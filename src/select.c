#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

typedef struct {
	int a;
	int b;
	char c;
	float f;
	int a3;
	int b3;
	char c3;
	float f3;
} foo_s;

int main(int argc, char* argv[])
{
//    int fd = -1;
//    fd_set rfds, wfds;
//    struct timeval tv;
//
//    if((fd = open(argv[1], O_RDONLY)) < 0)
//        perror("open error");
//
//    tv.tv_sec = 10;
//    tv.tv_usec = 0;
//    FD_ZERO(&rfds);
//    FD_SET(fd, &rfds);
//    select(10, &rfds, NULL, NULL, &tv);
//    if(FD_ISSET(fd, &rfds))
//        printf("file is ready\n");
	foo_s *p, *q;
	p = (foo_s * ) malloc (10*sizeof(foo_s));
	q=p;
	p = (foo_s* ) realloc (p,20);
	printf("p: %p, q: %p\n", p, q);
	printf("sizeof(*p): %ld\n", sizeof(*p));
	printf("sizeof(void):%ld, sizeof(void*):%ld\n", sizeof(void), sizeof(void*));
    exit(0);
}
