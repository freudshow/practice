#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include "lib.h"

void get_local_time(char* buf, u32 bufSize)
{
	time_t rawtime;
	struct tm timeinfo;

	if ((bufSize < 20) || (NULL == buf))
		return;

	time(&rawtime);
	localtime_r(&rawtime, &timeinfo);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", (timeinfo.tm_year + 1900),
			(timeinfo.tm_mon + 1), timeinfo.tm_mday, timeinfo.tm_hour,
			timeinfo.tm_min, timeinfo.tm_sec);
}

void debug(const char* file, const char* func, u32 line, u8 printEnter, const char *fmt, ...)
{
	va_list ap;
	char buf[20] = { 0 };

	get_local_time(buf, sizeof(buf));
	fprintf(stderr, "[%s][%s][%s()][%d]: ", buf, file, func, line);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if(1 == printEnter)
		fprintf(stderr, "\n");
}

void showArray(int v[], int length)
{
	int i;

	for (i = 0; i < length; i++) {
		fprintf(stderr, "%d\t", v[i]);
	}
	fprintf(stderr, "\n");
}

/*
 * 功能: 将帧字符串转化为16进制字节串 *
 * @str: 帧字符串
 * @buf: 目标字节串
 * @bufSize: 目标字节串原本的长度
 */
int readFrm(char *str, u8 *buf, u32 *bufSize)
{
    int state = 0;//0, 初始状态; 1, 空格状态; 2, 字节高状态; 3, 字节低状态; 4, 错误状态.
    u8 high = 0;
    u8 low = 0;
    u32 destLen = 0;//已扫描过的字节个数
    char *p = str;
    u8 *pBuf = buf;
    int ret = TRUE;

    if (bufSize == NULL || buf == NULL || str == NULL)
        return FALSE;

    while (*p != '\0') {
        if (!(isHex(*p) || isDelim(*p))) {
            state = 4;
            goto final;
        }

        switch (state) {
		case 0://init state
			if (isDelim(*p)) {
				state = 1;
			} else if (isHex(*p)) {
				high = *p;
				state = 2;
			}
			break;
		case 1://space state
			if (isHex(*p)) {
				high = *p;
				state = 2;
			}
			break;
		case 2://high state
			if (isDelim(*p)) {
				state = 4;
				goto final;
			}

			if (destLen < (*bufSize)) {
				low = *p;
				high = ASCII_TO_HEX(high);
				low = ASCII_TO_HEX(low);

				*pBuf = (high << 4 | low);
				pBuf++;
				destLen++;
				high = low = 0;
				state = 3;
			} else {
				goto final;
			}
			break;
		case 3://low state
			if (isHex(*p)) {
				state = 4;
				goto final;
			} else {
				state = 1;
			}
			break;
		default:
			goto final;
        }

        p++;
    }

final:
    if (state == 4 || state == 2) {//高位状态和非法状态均为不可接受状态
        DEBUG_TIME_LINE("存在非法字符, 或字符串格式非法\n");
        if (destLen > 0) {
            memset(buf, 0, destLen);
            destLen = 0;
        }
        ret = FALSE;
    }
    *bufSize = destLen;
	printBuf(buf, destLen);

	return ret;
}

void printBuf(u8* buf, u32 bufSize)
{
	u32 i = 0;

	if (NULL == buf || 0 == bufSize) {
		return;
	}

	for (i=0; i < (bufSize-1); i++) {
		fprintf(stderr, "%02X ", buf[i]);
	}
	fprintf(stderr, "%02X\n", buf[i]);
}

int arrayCmp(int a, int b)
{
	if (a == b)
		return EQUAL_TO;
	else if (a > b)
		return GREAT_THAN;
	else if (a < b)
		return SMALL_THAN;

	return -2;
}

/*****************************
 * @elem	an element to be found
 * @array	an array sorted ascend
 * return the index of elem in array, if found;
 * reutrn -1 if not found.
 *****************************/
int binSearchInt(int elem, int array[], int length)
{
	int low, high, mid, cmp;
	low = 0;
	high = (length - 1);
	mid = (low+high)/2;
	cmp = INIT_STATE;
	while(low <= high) {
		mid = (low+high)/2;
		cmp = arrayCmp(elem, array[mid]);
		switch (cmp) {
		case GREAT_THAN:
			low = mid+1;
			break;
		case SMALL_THAN:
			high = mid-1;
			break;
		case EQUAL_TO:
			return mid;
		default:
			break;
		}
	}

	return NOT_FOUND;
}

void quikSort(int v[], int n)
{
	int i, last;

	if(n <= 1)
		return;
	fprintf(stderr, "before swap pivot:\n");
	showArray(v, n);
	SWAP(v[0], v[(rand()%n)]);
	DEBUG_OUT("after swap pivot:\n");
	showArray(v, n);
	last = 0;
	for(i=1; i < n; i++)
		if(v[i] < v[0]) {
			last++;
			SWAP(v[last], v[i]);
		}
	DEBUG_OUT("after select:\n");
	showArray(v, n);

	SWAP(v[0], v[last]);
	DEBUG_OUT("after restore pivot:\n");
	showArray(v, n);

	quikSort(v, last);
	quikSort(v+last+1, n-last-1);
}

int bufIsNULL(u8 *buf, u32 bufSize) {
    u32 i = 0;

    if (NULL == buf || 0 == bufSize) {
        return TRUE;
    }

    for (i = 0; i < bufSize; i++) {
        if (buf[i] != 0)
            return FALSE;
    }

    return TRUE;
}

int isDecimal(char* dec)
{
	if (NULL == dec)
		return FALSE;

	s8 ret = TRUE;
	u8 state = 0; //0-init state; 1-integer state; 2-dec point state; 3-fraction state; 4-error state.

	while ('\0' != *dec) {
		if (!isDigit(*dec) && (*dec != '.')) {
			state = 4;
			goto final;
		}

		switch (state) {
		case 0: //init state, only accepts digit
			if (isDigit(*dec)) {
				state = 1;
			} else {
				state = 4;
				goto final;
			}
			break;
		case 1: //integer state, only accepts digit or '.'
			if (*dec == '.') {
				state = 2;
			} else if (!isDigit(*dec)) {
				state = 4;
				goto final;
			}
			break;
		case 2: //dec point state, , only accepts digit
			if (!isDigit(*dec)) {
				state = 4;
				goto final;
			} else {
				state = 3;
			}
			break;
		case 3: //fraction state, only accepts digit
			if (!isDigit(*dec)) {
				state = 4;
				goto final;
			}
			break;
		default:
			break;
		}
	}

final:
	if (4 == state)
		ret = FALSE;

	return ret;
}

