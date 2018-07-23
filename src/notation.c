#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef int s32;

/* pack: pack binary items i n t o buf, return length */
int pack(u8 *buf, char *fmt, ...)
{
	va_list args;
	u8 *bp;
	char *p;
	u16 s;
	u32 l;

	bp = buf;
	va_start(args, fmt);

	for (p = fmt; *p != '\0'; p++) {
		switch (*p) {
		case 'c': /* char */
			*bp++ = va_arg(args, int);
			break;
		case 's': /* short */
			s = va_arg(args, int);
			*bp++ = s >> 8;
			*bp++ = s;
			break;
		case 'l': /* long */
			l = va_arg(args, unsigned long);
			*bp++ = l >> 24;
			*bp++ = l >> 16;
			*bp++ = l >> 8;
			*bp++ = l;
			break;
		default: /* illegal type character */
			va_end(args);
			return -1;
		}
	}
	va_end(args);

	return (bp - buf);
}

int unpack(u8* buf, char* fmt, ...)
{
	va_list args;
	char *p;
	u8 *bp, *pc;
	u16 *ps;
	u32 *pl;

	bp = buf;
	va_start(args, fmt);

	for (p = fmt; *p != '\0'; p++) {
		switch (*p) {
		case 'c': /* char */
			pc = va_arg(args, u8*);
			*pc = *bp++;
			break;
		case 's': /* short */
			ps = va_arg(args, u16*);
			*ps = *bp++ << 8;
			*ps |= *bp++;
			break;
		case 'l': /* long */
			pl = va_arg(args, u32*);
			*pl = *bp++ << 24;
			*pl |= *bp++ << 16;
			*pl |= *bp++ << 8;
			*pl |= *bp++;;
			break;
		default: /* illegal type character */
			va_end(args);
			return -1;
		}
	}

	va_end(args);
	return (bp - buf);
}

int main()
{
	u8 ibuf[] = {0x05, 0xe4, 0xa2, 'd', 0x4f, 0x04, 0x10, 0x45};
	u8 obuf[256] = {0};
	u8 type = 0x01;
	u16 u16b = 0xe234;
	u32 u32b = 0x875fe234;
	u8 c = 'f';

	pack(obuf, "cscl", type, u16b, c, u32b);

	int i = 0;
	for(i=0;i<sizeof(obuf)&&obuf[i]>0;i++){
		printf("%02X ", obuf[i]);
	}
	printf("\n");



	exit(0);
}
