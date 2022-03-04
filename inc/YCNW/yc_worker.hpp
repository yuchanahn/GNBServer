#pragma once
#include <thread>
#include <vector>
#include <ranges>
#include <mutex>
#include <functional>
#include <unordered_map>

#include "yc_function.hpp"
#include "yc_net.hpp"




namespace
{
    const BYTE kLOCKTYPE_NONE = 0;
    const BYTE kLOCKTYPE_EXCLUSIVE = 1;
    const BYTE kLOCKTYPE_SHARED = 2;
}

class CSimpleSRWLock
{
public:
    CSimpleSRWLock() : m_bInit(FALSE) { InitializeSRWLock(&m_SRWLock); m_bInit = TRUE; }
    ~CSimpleSRWLock() { }

    void Lock(BYTE byLockType) {
        switch (byLockType)
        {
        case kLOCKTYPE_EXCLUSIVE:
            AcquireSRWLockExclusive(&m_SRWLock);
            break;
        case kLOCKTYPE_SHARED:
            AcquireSRWLockShared(&m_SRWLock);
            break;
        }
    }
    void Unlock(BYTE byLockType) {
        switch (byLockType)
        {
        case kLOCKTYPE_EXCLUSIVE:
            ReleaseSRWLockExclusive(&m_SRWLock);
            break;
        case kLOCKTYPE_SHARED:
            ReleaseSRWLockShared(&m_SRWLock);
            break;
        }
    }
    BOOL IsInit() { return m_bInit; }
    BOOL TryLock(BYTE byLockType) {
        boolean bRet = 0;
        switch (byLockType)
        {
        case kLOCKTYPE_EXCLUSIVE:
            bRet = TryAcquireSRWLockExclusive(&m_SRWLock);
            break;
        case kLOCKTYPE_SHARED:
            bRet = TryAcquireSRWLockShared(&m_SRWLock);
            break;
        }
        return bRet != 0 ? TRUE : FALSE;
    }
private:
    SRWLOCK             m_SRWLock;
    BOOL                m_bInit;
};


/**
 * @brief Scope범위에서만 Lock이 설정되도록 하는 클래습니다.
 * @details 템플릿인자 클래스는 Lock 및 UnLock함수를 구현해야 합니다.
 * @author woong20123
 * @date 2020-02-15
 */
template<typename SimpleLockType>
class CScopeLock
{
public:
    CScopeLock(SimpleLockType* pcs, BYTE byLockType = kLOCKTYPE_EXCLUSIVE) : m_pcsObjcect(pcs), m_byLockType(byLockType)
    {
        if (m_pcsObjcect && m_pcsObjcect->IsInit())
        {
            m_pcsObjcect->Lock(m_byLockType);
        }
    }
    ~CScopeLock()
    {
        if (m_pcsObjcect && m_pcsObjcect->IsInit())
        {
            m_pcsObjcect->Unlock(m_byLockType);
        }
    }
private:
    // CScopeLock은 NonCopyable
    CScopeLock(const SimpleLockType&);
    const CScopeLock<SimpleLockType>& operator=(const SimpleLockType&);

    SimpleLockType* m_pcsObjcect;
    BYTE            m_byLockType;
};




namespace yc_net 
{
	struct worker_info_t {
		bool on_duty = false;
		bool is_active = true;
		CSimpleSRWLock m;
	};

	std::unordered_map<worker_info_t*, std::vector<std::function<void()>>> workers;
	std::unordered_map<std::thread::id, std::vector<worker_info_t*>> worker_info_thread_mem;

	worker_info_t* create_worker()
	{
		auto& worker_info = worker_info_thread_mem[std::this_thread::get_id()];
		worker_info_t* w = nullptr;

		for (auto& i : worker_info | std::views::filter(pis_act_false))
		{
			i->is_active = true;
			w = i;
		}
		if (w == nullptr)
		{
			w = new worker_info_t();
			worker_info.push_back(w);
		}
		return w;
	}

	auto add_sync_worker = [](worker_info_t* w, auto f) {
		CScopeLock<CSimpleSRWLock> ScopeSRWLock(&w->m, kLOCKTYPE_EXCLUSIVE);
		workers[w].push_back(f);
	};

	void run_wokers_in_this_thread(bool& stop_button)
	{
		while (!stop_button)
		{
			//Sleep(1);
			for (auto& i : workers  | std::views::filter([](auto& i) { return pis_act_true(i.first); })
							        | std::views::filter([](auto& i) { return i.second.size(); })
							        | std::views::filter([](auto& i) { return !(i.first->on_duty); })
							        | std::views::take(1)) {
                std::vector<std::function<void()>> jobs;
                {
                    CScopeLock<CSimpleSRWLock> ScopeSRWLock(&i.first->m, kLOCKTYPE_EXCLUSIVE);
                    if (i.first->on_duty)
                    {
                        continue;
                    }
                    i.first->on_duty = true;
                    jobs = i.second;
                    i.second.clear();
                }
				yc::invoke_all(jobs);
                {
                    CScopeLock<CSimpleSRWLock> ScopeSRWLock(&i.first->m, kLOCKTYPE_EXCLUSIVE);
                    i.first->on_duty = false;
                }
			}
		}
	}
}