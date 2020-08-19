#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define	FILE_LINE		__FILE__,__FUNCTION__,__LINE__

#define UNUSEDV(x)      (void)x

#ifdef DEBUG_OUT_TO_FILES
#define DEBUG_BUFFFMT_TO_FILE(fname, buf, bufSize, format, ...) do {\
																		FILE *fp = fopen(fname, "a+");\
																		if (fp != NULL) {\
																			debugBufFormat2fp(fp, FILE_LINE, buf, bufSize, format, ##__VA_ARGS__);\
																		}\
																		logLimit(fname, LOG_SIZE, LOG_COUNT);\
																	} while(0);

#define DEBUG_TO_FILE(fname, format, ...)	do {\
												FILE *fp = fopen(fname, "a+");\
												if (fp != NULL) {\
													debugBufFormat2fp(fp, FILE_LINE, NULL, 0, format, ##__VA_ARGS__);\
												}\
												logLimit(fname, LOG_SIZE, LOG_COUNT);\
											} while(0);

#else

#define DEBUG_BUFFFMT_TO_FILE(filename, buf, bufSize, format, ...)
#define DEBUG_TO_FILE(fname, format, ...)

#endif

#ifdef DEBUG_OUT_TO_SCREEN
#define	DEBUG_BUFF_FORMAT(buf, bufSize, format, ...)	debugBufFormat2fp(stdout, FILE_LINE, buf, bufSize, format, ##__VA_ARGS__)
#define	DEBUG_TIME_LINE(format, ...)	debugBufFormat2fp(stdout, FILE_LINE, NULL, 0, format, ##__VA_ARGS__)
#else
#define	DEBUG_BUFF_FORMAT(data, dataSize, format, ...)
#define	DEBUG_TIME_LINE(format, ...)
#endif


extern void debugBufFormat2fp(FILE *fp, const char *file, const char *func,
		int line, char* buf, int len, const char* fmt, ...);

/**
 * @fname: 文件名, 绝对路径
 * @logsize: 日志文件的最大值
 * @logCount: 日志文件个数
 * @return: 成功, 返回0;  失败, 返回-1
 */
extern int logLimit(char* fname, int logsize, int logCount);

#ifdef __cplusplus
}
#endif

#endif//LOG_H_
