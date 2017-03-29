#ifndef __LIB_H__
#define __LIB_H__

#include <stdio.h>
#include <stdlib.h>
#include "basedef.h"

#define	DEBUG_OUT(format, ...)	debug(FILE_LINE, format, ##__VA_ARGS__)

#define	INIT_STATE	-1
#define EQUAL_TO	0
#define GREAT_THAN	1
#define SMALL_THAN	2

extern void debug(const char* file, const char* func, u32 line, const char *fmt, ...);
void showArray(int v[], int length);
extern int arrayCmp(int a, int b);
extern int binSearchInt(int elem, int array[], int length);
extern void swap(int v[], int i, int j);
extern void quikSort(int v[], int len);

#endif// __LIB_H__
