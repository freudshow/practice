#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "lib.h"

void get_local_time(char* buf, u32 bufSize)
{
    struct timeval systime;
    struct tm timeinfo;

    gettimeofday(&systime, NULL);
    localtime_r(&systime.tv_sec, &timeinfo);
    snprintf(buf, bufSize - 1, "%04d-%02d-%02d %02d:%02d:%02d.%03ld", (timeinfo.tm_year + 1900),
            timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour,
            timeinfo.tm_min, timeinfo.tm_sec, systime.tv_usec/1000);
}

void debugBufFormat2fp(FILE *fp, const char *file, const char *func, int line,
        char *buf, int len, const char *fmt, ...)
{
    va_list ap;
    char bufTime[25] = { 0 };

    if (fp != NULL) {
        get_local_time(bufTime, sizeof(bufTime));
        fprintf(fp, "[%s][%s][%s()][%d]: ", bufTime, file, func, line);

        va_start(ap, fmt);
        vfprintf(fp, fmt, ap);
        va_end(ap);

        if (buf != NULL && len > 0) {
            int i = 0;
            for (i = 0; i < len; i++) {
                fprintf(fp, "%02X ", (u8)buf[i]);
            }
        }

        fprintf(fp, "\n");
        fflush(fp);
        if (fp != stdout && fp != stderr)
            fclose(fp);
    }
}

void showArray(int v[], int length)
{
	int i;

	for (i = 0; i < length; i++) {
		fprintf(stderr, "%d\t", v[i]);
	}
	fprintf(stderr, "\n");
}

/**************************************************
 * 功能描述: 将帧字符串转化为16进制字节串
 * ------------------------------------------------
 * 输入参数: str - 帧字符串, 格式必须为
 *        "0B04 0CE4 00 16 32 09", 或者
 *        "0B040CE400163209", 或者
 *        "0B04 0CE4\r\t\n 00 16 32 09", 或者
 *        "0B 04 0C E4 00 16 32 09", 即1个字节
 *        必须用2个字符表示(当前位的数值为0时, 必须用'0'补足). 每个
 *        字节之间可以有分隔符, 可以有连续多个分隔符, 也可以没有分隔符.
 *        分隔符可以是 ' ', '\t', '\r', '\n'
 *        maxLen - 用于装载输出结果的缓冲区的最大长度.
 * 输出参数: buf - 用于装载输出结果的缓冲区.
 *        bufSize - 输出结果的长度
 * ------------------------------------------------
 * 返回值: 成功, 返回 输出结果的缓冲区长度;
 *       失败, 返回 -1
 * ------------------------------------------------
 * 修改日志:
 * 		1. 日期: 2020年7月20日
 *				创建函数
 **************************************************/
int readFrame(char *str, u32 maxLen, u8 *buf)
{
    int state = 0; //0, 初始状态; 1, 空格状态; 2, 字节高状态; 3, 字节低状态; 4, 错误状态.
    u8 high = 0;
    u8 low = 0;
    u32 destLen = 0; //已扫描过的字节个数
    char *p = str;

    if (buf == NULL || str == NULL)
    {
        return -1;
    }

    while (*p != '\0')
    {
        if (!(isHex(*p) || isDelim(*p)))
        {
            state = 4;
            goto final;
        }

        switch (state)
        {
            case 0: //init state
                if (isDelim(*p))
                {
                    state = 1;
                }
                else if (isHex(*p))
                {
                    high = ASCII_TO_HEX(*p);
                    state = 2;
                }

                break;
            case 1: //space state
                if (isHex(*p))
                {
                    high = ASCII_TO_HEX(*p);
                    state = 2;
                }

                break;
            case 2: //high state
                if (isDelim(*p))
                {
                    state = 4;
                    goto final;
                }

                if (destLen < maxLen && isHex(*p))
                {
                    low = ASCII_TO_HEX(*p);

                    buf[destLen++] = (high << 4 | low);
                    high = low = 0;
                    state = 3;
                }
                else
                {
                    DEBUG_TIME_LINE("buf over flow");
                    goto final;
                }

                break;
            case 3: //low state
                if (isHex(*p))
                {
                    high = ASCII_TO_HEX(*p);
                    state = 2;
                }
                else if (isDelim(*p))
                {
                    state = 1;
                }
                break;
            default:
                goto final;
        }

        p++;
    }

final:
    //高位状态和非法状态均为不可接收状态
    if (state == 4 || state == 2)
    {
        DEBUG_TIME_LINE("存在非法字符, 或字符串格式非法\n");
        if (destLen > 0)
        {
            memset(buf, 0, destLen);
            destLen = 0;
        }

        return -1;
    }

    return destLen;
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
	DEBUG_TIME_LINE("after swap pivot:\n");
	showArray(v, n);
	last = 0;
	for(i=1; i < n; i++)
		if(v[i] < v[0]) {
			last++;
			SWAP(v[last], v[i]);
		}
	DEBUG_TIME_LINE("after select:\n");
	showArray(v, n);

	SWAP(v[0], v[last]);
	DEBUG_TIME_LINE("after restore pivot:\n");
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

