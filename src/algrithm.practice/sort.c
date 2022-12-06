#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <math.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#define	FILE_LINE		__FILE__,__FUNCTION__,__LINE__

#define	DEBUG_BUFF_FORMAT(buf, bufSize, format, ...)	debugBufFormat2fp(stdout, FILE_LINE, (int*)buf, (int)bufSize, format, ##__VA_ARGS__)
#define	DEBUG_TIME_LINE(format, ...)	DEBUG_BUFF_FORMAT(NULL, 0, format, ##__VA_ARGS__)

typedef unsigned char boolean; /* bool value, 0-false, 1-true       */
typedef unsigned char u8; /* Unsigned  8 bit quantity          */
typedef char s8; /* Signed    8 bit quantity          */
typedef unsigned short u16; /* Unsigned 16 bit quantity          */
typedef signed short s16; /* Signed   16 bit quantity          */
typedef unsigned int u32; /* Unsigned 32 bit quantity          */
typedef signed int s32; /* Signed   32 bit quantity          */
typedef unsigned long long u64; /* Unsigned 64 bit quantity   	   */
typedef signed long long s64; /* Unsigned 64 bit quantity          */
typedef float fp32; /* Single precision floating point   */
typedef double fp64; /* Double precision floating point   */

/**************************************************
 * 功能描述: 获得当前时间字符串
 * ------------------------------------------------
 * 输入参数: buffer [out]: 当前时间的字符串, 长度
 *  	    至少为20个字节, 用以存储格式为"2020-07-01 16:13:33"
 * 输出参数: buffer
 * ------------------------------------------------
 * 返回值: 无
 * ------------------------------------------------
 * 修改日志:
 * 		1. 日期: 2020年7月19日
 创建函数
 **************************************************/
void get_local_time(char *buf, u32 bufLen)
{
	struct timeval systime;
	struct tm timeinfo;

	gettimeofday(&systime, NULL);
	localtime_r(&systime.tv_sec, &timeinfo);
	snprintf(buf, bufLen, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
			(timeinfo.tm_year + 1900), timeinfo.tm_mon + 1, timeinfo.tm_mday,
			timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
			systime.tv_usec / 1000);
}

/**************************************************
 * 功能描述: 将字节数组, 以16进制格式, 打印到字符串out中, 以空格
 * 分开每个字节
 * ------------------------------------------------
 * 输入参数: buf [in]: 字节数组
 * 输入参数: len [in]: 字节数组长度
 * 输出参数: out, 输出的字符串
 * ------------------------------------------------
 * 返回值: 无
 * ------------------------------------------------
 * 修改日志:
 * 		1. 日期: 2020年7月19日
 创建函数
 **************************************************/
void getBufString(int *buf, int len, char *out)
{
	int i = 0;
	char s[5] = { 0 };
	for (i = 0; i < len; i++)
	{
		sprintf(s, "%d ", buf[i]);
		strcat(out, s);
	}
}

/**************************************************
 * 功能描述: 将调试信息打印到文件描述符fp中
 * ------------------------------------------------
 * 输入参数: fp [in]: 文件描述符
 * 输入参数: file [in]: 调试信息所在的文件名
 * 输入参数: func [in]: 调试信息所在的函数名
 * 输入参数: line [in]: 调试信息所在的行号
 * 输出参数: 无
 * ------------------------------------------------
 * 返回值: 无
 * ------------------------------------------------
 * 修改日志:
 * 		1. 日期: 2020年7月19日
 创建函数
 **************************************************/
void debugBufFormat2fp(FILE *fp, const char *file, const char *func, int line,
		int *buf, int len, const char *fmt, ...)
{
	va_list ap;
	char bufTime[25] = { 0 };

	if (fp != NULL)
	{
		get_local_time(bufTime, sizeof(bufTime));
		fprintf(fp, "[%s][%s][%s()][%d]: ", bufTime, file, func, line);
		va_start(ap, fmt);
		vfprintf(fp, fmt, ap);
		va_end(ap);

		if (buf != NULL && len > 0)
		{
			char *s = (char*) calloc(1, 3 * (len + 1)); //多开3个字符的余量
			if (s != NULL)
			{
				getBufString(buf, len, s);
				fprintf(fp, "%s", s);
				free(s);
			}
		}
		fprintf(fp, "\n");
		fflush(fp);
		if (fp != stdout && fp != stderr)
			fclose(fp);
	}
}

void insertionSort(int *array, int len)
{
	if (len <= 1)
	{
		return;
	}

	int i = 0;
	int j = 0;
	int key = 0;
	for (i = 1; i < len; i++)
	{
		key = array[i];
		DEBUG_TIME_LINE("key: %d", key);

		// Insert array[i] into the sorted subarray array[1: i-1]
		j = i - 1;

		while (j >= 0 && array[j] > key)
		{
			array[j + 1] = array[j];
			j = j - 1;
		}

		array[j + 1] = key;

		DEBUG_BUFF_FORMAT(array, len, "step{-%d-}: ", i);
	}
}

int main(int argc, char **argv)
{
	int a[] = { 12, 17, 6, 5, 9, 6, 11, 15, 141, 98, 74, 1, 65 };
	int len = sizeof(a) / sizeof(a[0]);

	DEBUG_BUFF_FORMAT(a, len, "before sort: ");
	insertSort(a, len);
	DEBUG_BUFF_FORMAT(a, len, "after sort: ");

	exit(0);
}
