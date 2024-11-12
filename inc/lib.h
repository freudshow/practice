#ifndef __LIB_H__
#define __LIB_H__

#include <stdio.h>
#include <stdlib.h>
#include "basedef.h"

#define	DEBUG_BUFF_FORMAT(buf, bufSize, format, ...)	debugBufFormat2fp(stdout, FILE_LINE, (char*)buf, (int)bufSize, format, ##__VA_ARGS__)
#define	DEBUG_TIME_LINE(format, ...)	DEBUG_BUFF_FORMAT(NULL, 0, format, ##__VA_ARGS__)

#define	 INIT_STATE	-1
#define EQUAL_TO	0
#define GREAT_THAN	1
#define SMALL_THAN	2

#define TRUE	(0)
#define FALSE	(-1)

extern void debugBufFormat2fp(FILE *fp, const char *file, const char *func, int line,
        char *buf, int len, const char *fmt, ...);
extern void showArray(int v[], int length);
extern int arrayCmp(int a, int b);
extern int binSearchInt(int elem, int array[], int length);
extern void xorSwap(int v[], int i, int j);
extern void swap(int v[], int i, int j);
extern void quikSort(int v[], int len);
extern int readFrame(char *str, u32 maxLen, u8 *buf);
extern int isDecimal(char* dec);

#endif// __LIB_H__
