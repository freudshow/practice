#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "log.h"

void get_local_time(char *buffer)
{
	time_t rawtime;
	struct tm *timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d", (timeinfo->tm_year + 1900),
			timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour,
			timeinfo->tm_min, timeinfo->tm_sec);
}

int getMaxFileNo(char* fname)
{
	int ret = 0;
	char cmd[100] = { 0 }; //调用shell命令
	char result[128] = { 0 };
	FILE *fstream = NULL;

	sprintf(cmd, "ls -r %s*|cut -c %d-99", fname, strlen(fname) + 2); //最大两位数

	if (NULL == (fstream = popen(cmd, "r"))) { //打开管道
		fprintf(stdout, "execute command failed: %s", strerror(errno));
		return -1;
	}

	if (0 != fread(result, sizeof(char), sizeof(result), fstream)) { //读取执行结果
		int i = 0;
		while (result[i] != '\n') //读取第一个数字, 'ls -r' 命令默认降序排列
			i++;

		result[i] = 0;//字符串结束符
		ret = atoi(&result[0]);
	} else {
		ret = -1;
	}

	pclose(fstream);

	return ret;
}

int mvFiles(char* fname, int logCount)
{
	if ((access(fname, F_OK)) != 0) {
		return -1;
	}

	char name[100] = { 0 };
	int max = getMaxFileNo(fname);
	int i = 0;

	if(max < 0)
		return -1;

	if ((max + 1) == logCount) {
		sprintf(name, "%s.%d", fname, max);
		max -= 1;
		unlink(name);
		sync();
	}

	char cmd[200] = { 0 };
	for (i = max; i > 0; i--) {
		sprintf(cmd, "mv %s.%d %s.%d", fname, i, fname, i + 1);
		system(cmd);
	}

	if (logCount > 1) {
		sprintf(cmd, "mv %s %s.%d", fname, fname, i + 1);
		system(cmd);
	}

	return 0;
}

/**
 * @fname: 文件名, 绝对路径
 * @logsize: 日志文件的最大值
 * @logCount: 日志文件个数
 * @return: 成功, 返回0;  失败, 返回-1
 */
int logLimit(char* fname, int logsize, int logCount)
{
	char cmd[128] = { 0 };
	struct stat fileInfo;
	int ret = 0;

	if (stat(fname, &fileInfo) == -1) {
		sprintf(cmd, "rm -f %s", fname);
		system(cmd);
		return -1;
	}

	/**
	 * 检查当前日志文件名的最大值(以logCount为参照)
	 * 比如, logCount = 10, 如果当前文件名有'9'
	 * 结尾的, 那么就将'fname9'删掉, 依次将'fname8'复制
	 * 为'fname9', 'fname7'复制为'fname8'...'fname'
	 * 复制为'fname0'; 如果当前文件没有达到最大值,
	 * 则依次复制当前最大值的文件名, 往后累加一个.
	 * 所以, 最关键的是获取当前最大的文件名.
	 */
	if (fileInfo.st_size > logsize) {
		ret = mvFiles(fname, logCount);
	}

	return ret;
}

void getBufString(char* buf, int len, char* out)
{
	int i=0;
	char s[5] = {0};

	if(NULL == buf || len <= 0)
		return;

	for(i=0;i<len;i++){
		sprintf(s, "%02X ", (unsigned char)buf[i]);
		strcat(out, s);
	}
}

void debugBufFormat2fp(FILE *fp, const char *file, const char *func,
		int line, char* buf, int len, const char* fmt, ...)
{
	va_list ap;
	char s[1024] = {0};
	char bufTime[20] = { 0 };

	if (fp != NULL) {
		get_local_time(bufTime);
		fprintf(fp, "\n[%s][%s][%s()][%d]: ", bufTime, file, func, line);
		va_start(ap, fmt);
		vfprintf(fp, fmt, ap);
		va_end(ap);
		getBufString(buf, len, s);
		fprintf(fp, "%s\n", s);
		fflush(fp);
		fclose(fp);
	}
}
