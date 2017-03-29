#ifndef	__SHOWBYTES_H__
#define __SHOWBYTES_H__

#include "basedef.h"

typedef u8 *byte_pointer;

#pragma	pack(push)
#pragma pack(1)

typedef struct {
	u8 a;
	u32 b;
	u8 c;
	u16 d;
} s_child;

typedef struct {
	u8 a;
	s_child b;
	u32 c;
	u16 d;
} s_test;

#pragma pack(pop)

extern void show_bytes(byte_pointer start, int len);
extern void show_int(int x);
extern void show_uint(unsigned int x);
extern void show_float(float x);
extern void show_pointer(void *x);
extern void show_struct(void *x);

#endif //__SHOWBYTES_H__
