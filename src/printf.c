#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "../inc/basedef.h"

typedef char boolean;
#define TRUE 0
#define FALSE 1

void foo(int a, int b, int c)
{
	printf("a: %d, &a: %p\n", a, &a);
	printf("b: %d\n", b);
	printf("c: %d, &c: %p\n", c, &c);
}

int getMaxFileNo(char* fname)
{
	int ret = 0;
	char cmd[100] = { 0 }; //调用shell命令
	char result[128] = { 0 };
	FILE *fstream = NULL;

	sprintf(cmd, "ls -r %s*|cut -c %d-99", fname, strlen(fname) + 2); //最大两位数

	if (NULL == (fstream = popen(cmd, "r"))) { //打开管道
		fprintf(stderr, "execute command failed: %s", strerror(errno));
		return -1;
	}

	if (0 != fread(result, sizeof(char), sizeof(result), fstream)) { //读取执行结果
		printf("[%s][%s][%d]result: %s", FILE_LINE, result);

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

	if ((max + 1) == logCount) {
		sprintf(name, "%s.%d", fname, max);
		max -= 1;
		unlink(name);
		sync();
	}

	char cmd[100] = { 0 }; //调用shell命令
	for (i = max; i > 0; i--) {
		sprintf(cmd, "mv %s.%d %s.%d", fname, i, fname, i + 1);
		printf("------->>>>cmd: %s\n", cmd);
		system(cmd);
	}

	if (logCount > 1) {
		sprintf(cmd, "mv %s %s.%d", fname, fname, i + 1);
		system(cmd);
	}

	return 0;
}

int main(int argc, char* argv[])
{
	mvFiles(argv[1], atoi(argv[2]));
    exit(0);
}
