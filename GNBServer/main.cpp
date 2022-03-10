#include <iostream>
#include "yc_net.hpp"
#include "yc_worker.hpp"
#include "packet_data.hpp"
#include "yc_time.hpp"
#include <map>
auto w = yc_net::create_worker();


void loop()
{
    yc::time::update_delta_time();

    static size_t fps = 0;
    static float dt = 0;
    fps++;
    if ((dt += yc::time::get_delta_time()) > 1.f)
    {
        printf("%lld\n", fps);
        fps = 0;
        dt = 0;
    }

    yc_net::add_sync_worker(w, loop);
}

int main()
{
    packet_data_load();

    std::map<yc::socket_t, std::string> users;

    server_setting = server_setting_t{
        .io_thread_number = 2,
        .worker_thread_number = 2,
        .port = 6666
    };

    yc_net::connect_callback = [&](yc::socket_t socket) {
        printf("connect! [SOCKET : %lld, ADDR : %s]\n", socket, yc_net::get_clnt_addrs(socket).c_str());
        users[socket] = yc_net::get_clnt_addrs(socket).c_str();
    };

    yc_net::disconnect_callback = [&](yc::socket_t socket) {
        printf("disconnect! [SOCKET : %lld, ADDR : %s]\n", socket, yc_net::get_clnt_addrs(socket).c_str());
        users.erase(socket);
    };
    bool stop = false;
    int a = 0;
    strand_run(stop);
    yc_net::bind_ev<p_chat_message_t>([&](p_chat_message_t* chat, yc::socket_t socket) {
        wprintf(std::format(L"SOCKET : {} - {}\n", socket, chat->msg).c_str());
        std::wstring s;
        s.assign(users[socket].begin(), users[socket].end());
        s.append(L" : ");
        s.append(chat->msg);
        std::copy(s.begin(), s.end(), chat->msg);
        for (auto& i : users) {
            yc_net::send(chat, i.first);
        }
        });

    auto send_all_user = [&](yc::socket_t sock) {
        p_chat_message_t p { };
        std::wstring s;
        s.assign(users[sock].begin(), users[sock].end());
        s.append(L"님이 SSR 등급 캐릭터를 뽑았습니다!");
        std::copy(s.begin(), s.end(), p.msg);
        for (auto& i : users) {
            yc_net::send(&p, i.first);
        }
    };


    yc_net::bind_ev<p_gacha_start_t>([&](p_gacha_start_t* gacha, yc::socket_t socket) {
        
        p_gacha_result_t gr;
        for (int i = 0; i < gacha->cnt; i++) {
            gr.r[i] = rand() % 100;
            wprintf(std::format(L"SOCKET : {}, gacha : {}\n", socket, (int)gr.r[i]).c_str());
            if (gr.r[i] < 10) send_all_user(socket);
        }

        yc_net::send(&gr, socket);

        });



    //yc_net::add_sync_worker(w, loop);

    main_server();
}