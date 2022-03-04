#pragma once
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <functional>
#include <vector>
#include <chrono>
#include <ranges>

using namespace std::chrono;
namespace yc_timer {

	static std::thread* timers = nullptr;
	static bool stop = false;

	std::promise<void> stop_p;

	void shutdown_timer() {
		std::shared_future<void> stop_f = stop_p.get_future();
		stop = true;
		stop_f.get();
		if (timers->joinable())
		{
			timers->join();
		}
	}

	struct timer_t
	{
		size_t id;
		float t;
		float dt;
		std::function<void()> f;
		bool active = true;
	};
	static std::vector<timer_t> used_timer;
	std::thread& loop() {
		if (!timers) {
			timers = new std::thread([] {
				while (!stop) {
					static system_clock::time_point* lastT = new std::chrono::system_clock::time_point;
					static bool frs = true;
					if (frs) {
						*lastT = std::chrono::system_clock::now();
						std::this_thread::sleep_for(std::chrono::microseconds(10));
						frs = false;
					}
					auto dt = ((std::chrono::duration<float>)(std::chrono::system_clock::now() - *lastT)).count();
					*lastT = std::chrono::system_clock::now();

					for (auto& i : used_timer)
					{
						i.dt += dt;
					}

					for (auto& i : used_timer | std::views::filter([](auto& i) { return i.active; })
											  | std::views::filter([](auto& i) { return i.dt >= i.t; }))
					{
						i.f();
						i.active = false;
					}
					std::this_thread::sleep_for(std::chrono::microseconds(1));
				}
				stop_p.set_value();
			});
		}
		return *timers;
	}

	template <typename F>
	void timer_event(F&& f, float t_sec) {
		for (auto& i : used_timer | std::views::filter([](auto& i) { return !i.active; }))
		{
			i.t = t_sec;
			i.dt = 0;
			i.f = f;
			i.active = true;
			return;
		}
		used_timer.push_back(timer_t { used_timer.size(), t_sec, 0, f, true });
		loop();
	}
}