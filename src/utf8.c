#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BIT_N(byte, n)				(((byte)>>(n))&0x01)
#define CLEAR_BIT(b, n)     ((b) &= (~(0x01<<(n))))
#define SET_BIT(b, n)       ((b) |= (0x01<<(n)))

#define debug()						printf("[%s][%d]\n", __FUNCTION__, __LINE__)

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef int s32;
typedef signed long long s64;
typedef float fp32;
typedef double fp64;

/*
 * 0xxxxxxx
 * 110xxxxx 10xxxxxx
 * 1110xxxx 10xxxxxx 10xxxxxx
 * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 */
u8 countByte(u8* s)
{
	u8 i = 0;
	u8 cnt = 0;
	
	if(BIT_N(*s, 7) == 0)
		return 1;

	for ( i = 7; i > 2; i-- ) {
		if(BIT_N(*s, i) == 1) {
			cnt++;
		} else {
			break;
		}
	}

	if(cnt > 4 || cnt == 1)
		return 0;

	for(i = 1; i < cnt; i++) {
		if((*(s+i) >> 6) != 2) {
			return 0;
		}
	}

	return cnt;
}

u8 clrMBits(u8 u, u8 n)
{
	u8 i = 7;
	u8 m = u;
	
	while ((7-i)<n) {
		m = CLEAR_BIT(m, i);
		i--;
	}
	return m;
}

u32 getUTFValue(u8 *s)
{
	u32 v = 0;
	u32 t = 0;
	u8 i = 0;
	u8 bytes = countByte(s);
	
	if(bytes == 1)
		return *s;
	
	v = clrMBits(*s, bytes);
	v = (v << 6*(bytes-1));
	
	for(i=1;i<bytes;i++) {
		t = clrMBits(*(s+i), 2);
		t = ( t << 6*(bytes-1-i));
		v |= t;
	}

	return v;
}

int main(void)
{
	u8 p[] = {0b11110010, 0b10000100, 0b10110100, 0b10111000}; 
	u8 u = 0b11111111;
	
	printf("bytes: %u\n", countByte(p));
	printf("%02X\n", clrMBits(u, 2));
	printf("%u\n", getUTFValue(p));

	exit(0);	
}