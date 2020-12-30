#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * 以下3个日志相关的宏定义,
 * 根据自己需求定义.
 * 必须要定义, 否则预编译出错
 */
#define LOG_NAME	"mylog.txt" //日志名, 须使用绝对路径
#define LOG_SIZE	(5*1024*1024)						//日志文件最大长度
#define LOG_COUNT	10									//日志文件个数

#define DEBUG_OUT_TO_FILES //如果定义, 则打开写日志, 反之关闭写日志
#define DEBUG_OUT_TO_SCREEN //如果定义, 向屏幕输出, 反之关闭屏幕输出
#include "log.h"//这个包含语句, 须在上述宏定义之后, 否则预编译报错. 也可以将宏定义写在编译语句中的 "-D" 参数中


int main()
{

	char buf[] = { 0x01, 0x02, 0x68, 0xfe };

	char longbuf[] = { 0x68, 0x20, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x33, 0x14, 0x04, 0x00, 0x35, 0x00, 0x15, 0x12, 0x19, 0x20, 0x01,
			0x00, 0x08, 0x09, 0x03, 0x50, 0x00, 0x00, 0x01, 0x94, 0x42, 0xDF,
			0x16, 0x68, 0x20, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33,
			0x14, 0x04, 0x00, 0x35, 0x00, 0x15, 0x12, 0x19, 0x20, 0x01, 0x00,
			0x08, 0x09, 0x03, 0x50, 0x00, 0x00, 0x01, 0x94, 0x42, 0xDF, 0x16,
			0x68, 0x20, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x14,
			0x04, 0x00, 0x35, 0x00, 0x15, 0x12, 0x19, 0x20, 0x01, 0x00, 0x08,
			0x09, 0x03, 0x50, 0x00, 0x00, 0x01, 0x94, 0x42, 0xDF, 0x16, 0x68,
			0x20, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x14, 0x04,
			0x00, 0x35, 0x00, 0x15, 0x12, 0x19, 0x20, 0x01, 0x00, 0x08, 0x09,
			0x03, 0x50, 0x00, 0x00, 0x01, 0x94, 0x42, 0xDF, 0x16, 0x68, 0x20,
			0x00, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x14, 0x04, 0x00,
			0x35, 0x00, 0x15, 0x12, 0x19, 0x20, 0x01, 0x00, 0x08, 0x09, 0x03,
			0x50, 0x00, 0x00, 0x01, 0x94, 0x42, 0xDF, 0x16, 0x68, 0x20, 0x00,
			0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x14, 0x04, 0x00, 0x35,
			0x00, 0x15, 0x12, 0x19, 0x20, 0x01, 0x00, 0x08, 0x09, 0x03, 0x50,
			0x00, 0x00, 0x01, 0x94, 0x42, 0xDF, 0x16, 0x68, 0x20, 0x00, 0xC3,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x14, 0x04, 0x00, 0x35, 0x00,
			0x15, 0x12, 0x19, 0x20, 0x01, 0x00, 0x08, 0x09, 0x03, 0x50, 0x00,
			0x00, 0x01, 0x94, 0x42, 0xDF, 0x16, 0x68, 0x20, 0x00, 0xC3, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x34, 0x14, 0x04, 0x00, 0x03, 0x00, 0x15,
			0x12, 0x19, 0x20, 0x01, 0x00, 0x08, 0x0A, 0x03, 0x51, 0x00, 0x00,
			0x01, 0x95, 0x8D, 0xFC, 0x16, 0x68, 0x20, 0x00, 0xC3, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x34, 0x14, 0x04, 0x00, 0x03, 0x00, 0x15, 0x12,
			0x19, 0x20, 0x01, 0x00, 0x08, 0x0A, 0x03, 0x51, 0x00, 0x00, 0x01,
			0x95, 0x8D, 0xFC, 0x16, 0x68, 0x20, 0x00, 0xC3, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x34, 0x14, 0x04, 0x00, 0x03, 0x00, 0x15, 0x12, 0x19,
			0x20, 0x01, 0x00, 0x08, 0x0A, 0x03, 0x51, 0x00, 0x00, 0x01, 0x95,
			0x8D, 0xFC, 0x16, 0x68, 0x20, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x34, 0x14, 0x04, 0x00, 0x03, 0x00, 0x15, 0x12, 0x19, 0x20,
			0x01, 0x00, 0x08, 0x0A, 0x03, 0x51, 0x00, 0x00, 0x01, 0x95, 0x8D,
			0xFC, 0x16, 0x68, 0x20, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x34, 0x14, 0x04, 0x00, 0x03, 0x00, 0x15, 0x12, 0x19, 0x20, 0x01,
			0x00, 0x08, 0x0A, 0x03, 0x51, 0x00, 0x00, 0x01, 0x95, 0x8D, 0xFC,
			0x16, 0x68, 0x20, 0x00, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34,
			0x14, 0x04, 0x00, 0x03, 0x00, 0x15, 0x12, 0x19, 0x20, 0x01, 0x00,
			0x08, 0x0A, 0x03, 0x51, 0x00, 0x00, 0x01, 0x95, 0x8D, 0xFC, 0x16 };

//	DEBUG_BUFFFMT_TO_FILE(LOG_NAME, buf, sizeof(buf), "[hello %s!!!]", "world"); // 带报文的日志
//	DEBUG_TO_FILE(LOG_NAME, "[hello %s!!!]", "world") // 不带报文的日志
//	DEBUG_BUFF_FORMAT(buf, sizeof(buf), "[hello %s!!!]", "world"); // 带报文的打印信息
//	DEBUG_TIME_LINE("[hello %s!!!]", "world"); // 不带报文的打印信息

//	DEBUG_BUFFFMT_TO_FILE(LOG_NAME, longbuf, sizeof(longbuf), "[hello %s!!!]", "world"); // 带报文的日志
	DEBUG_BUFF_FORMAT(longbuf, sizeof(longbuf), "[hello %s!!!]", "world"); // 带报文的打印信息
	printf("-------------out done\n");
	exit(0);
}