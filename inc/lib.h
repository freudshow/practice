#ifndef __LIB_H__
#define __LIB_H__

#include <stdio.h>
#include <stdlib.h>
#include "basedef.h"

#define	DEBUG_OUT(format, ...)	debug(FILE_LINE, 0, format, ##__VA_ARGS__)
#define DEBUG_TIME_LINE(format, ...) debug(FILE_LINE, 1, format, ##__VA_ARGS__)
#define	DEBUG_PRINT(format, ...)	fprintf(stderr, "[%s][%s][%d]"format"\n", , ##__VA_ARGS__)

#define	 INIT_STATE	-1
#define EQUAL_TO	0
#define GREAT_THAN	1
#define SMALL_THAN	2

#define TRUE	(0)
#define FALSE	(-1)

extern void debug(const char* file, const char* func, u32 line, u8 printEnter, const char *fmt, ...);
extern void showArray(int v[], int length);
extern int arrayCmp(int a, int b);
extern int binSearchInt(int elem, int array[], int length);
extern void xorSwap(int v[], int i, int j);
extern void swap(int v[], int i, int j);
extern void quikSort(int v[], int len);
extern int readFrm(char *str, u8 *buf, u32 *bufSize);
extern void printBuf(u8* buf, u32 bufSize);
extern int isDecimal(char* dec);

#endif// __LIB_H__
