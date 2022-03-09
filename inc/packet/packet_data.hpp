#pragma once
#include "yc_packet.hpp"

#pragma pack(push, 1)

struct p_chat_message_t {
	wchar_t msg[300];
};

#pragma pack(pop)


auto packet_data_load = [] {
	ioev::Map<p_chat_message_t>().To<0>();
};