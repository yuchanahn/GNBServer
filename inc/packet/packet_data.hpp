#pragma once
#include "yc_packet.hpp"

#pragma pack(push, 1)

struct p_chat_message_t {
	wchar_t msg[300];
};

struct p_gacha_start_t {
	char cnt;
};

struct p_gacha_result_t {
	char r[10];
	char size;
};

#pragma pack(pop)

auto packet_data_load = [] {
	ioev::Map<p_chat_message_t>().To<0>();
	ioev::Map<p_gacha_start_t>().To<1>();
	ioev::Map<p_gacha_result_t>().To<2>();
};