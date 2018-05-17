#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>

#define SEMNAME_SPI0_0		"sem_spi0.0"//专变、I型集中器交采和esam的spi通信互斥信号量
#define SEMNAME_PARA_SAVE		"sem_para_save"

# define __FDS_BITS(set) ((set)->__fds_bits)

char* getLastStr(char* s, char delim)
{
	char* p = NULL;

	if(s == NULL)
		return NULL;

	p = s+strlen(s);
	printf("delim: %c\n", delim);
	while(*p!=delim && p != s) {
		printf("%c", *p);
		p--;
	}
	printf("\n");

	if(s == p)
		return s;
	else
		return (p+1);
}

int main(int argc, char* argv[])
{
	int value = 10;
	int ret = 0;
	sem_t* sem;

	sem_unlink(SEMNAME_SPI0_0);
	sem = sem_open(SEMNAME_SPI0_0, O_CREAT, O_RDWR, 5);
	while(1){
		sem_wait(sem);
		ret = sem_getvalue(sem, &value);
		if(ret == 0) {
			printf("%ld, %d\n", sem->__align, value);
		}

		sleep(1);
//		sem_post(p);
	}
	sem_unlink(SEMNAME_SPI0_0);
	return 0;
}
