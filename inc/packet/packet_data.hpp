#pragma once
#include "yc_packet.hpp"

#pragma pack(push, 1)

struct vec3d_t {
	float x;
	float y;
	float z;
};

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


struct p_mouse_event_t {
	bool right;
	vec3d_t start_pos;
	vec3d_t pos;
};

struct p_spawn_character_t {
	char number;
	bool IsMyCharacter;
	vec3d_t pos;
};

struct p_movement_character_t {
	char number;
	vec3d_t start_pos;
	vec3d_t pos;
};


struct p_attack_t {
	char number;
	vec3d_t start_pos;
};



#pragma pack(pop)

auto packet_data_load = [] {
	ioev::Map<p_chat_message_t>().To<0>();
	ioev::Map<p_gacha_start_t>().To<1>();
	ioev::Map<p_gacha_result_t>().To<2>();
	ioev::Map<p_mouse_event_t>().To<3>();
	ioev::Map<p_spawn_character_t>().To<4>();
	ioev::Map<p_movement_character_t>().To<5>();
	ioev::Map<p_attack_t>().To<6>();
};