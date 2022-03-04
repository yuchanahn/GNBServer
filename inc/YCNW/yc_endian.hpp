#pragma once 
#if (BYTE_ORDER == LITTLE_ENDIAN || __BYTE_ORDER == LITTLE_ENDIAN ||\
 __BYTE_ORDER == __LITTLE_ENDIAN) 
#define NTOH(x) reverse((x)) 
#define HTON(x) reverse((x)) 
#else 
#if (BYTE_ORDER == BIG_ENDIAN || __BYTE_ORDER == BIG_ENDIAN ||\
 __BYTE_ORDER == __BIG_ENDIAN)
#define NTOH(x) (x) 
#define HTON(x) (x) 
#else 
#error "No Support BYTE ORDER." 
#endif 
#endif 
inline bool is_big_endian()
{
	char buf[2] = { 0,1 };
	unsigned short* val = reinterpret_cast<unsigned short*>(buf);
	return *val == 1;
}

template <class T>
T reverse(T from)
{
	T to;
	char* s_from = reinterpret_cast<char*>(&from);
	char* s_to = reinterpret_cast<char*>(&to);
	for (int i = 0; i < sizeof(T); ++i)
		s_to[i] = s_from[sizeof(T) - i - 1];
	return to;
}