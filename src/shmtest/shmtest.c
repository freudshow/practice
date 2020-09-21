#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "shm.h"

void doTask(int i) {
	region_t r;
	int idx = 0;
	int random = 0;

	printf("This is child [%d] process [%d]\n", i, getpid());

	random = rand();
	idx = (rand() % region_count);
	if (random % 2) { //odd
		r.longth = rand();
		r.width = rand();
		r.height = rand();

		printf("write region: longth-[%d], width-[%d], height-[%d], idx-{%d}\n",
				r.longth, r.width, r.height, idx);
		write_region(&r, idx);
	} else { //even
		read_region(&r, idx);
		printf("read region: longth-[%d], width-[%d], height-[%d], idx-{%d}\n",
				r.longth, r.width, r.height, idx);

	}
}

int main(int argc, char **argv) {
	pid_t p1[20];
	int i;
	srand(time(0));
//	printf("%d, %d, %d\n",!(1111), !(0), !(-1));

	/*
	 * C provides a compile-time unary operator called sizeof that can be used to compute the size
	 * of any object -- k&r c, P. 120
	 */
	for (i = 0; i <= sizeof(p1) / sizeof(p1[0]); i++) {
		if ((p1[i] = fork()) == 0) {
			while (1) {
				doTask(i);
				usleep(100);
			}
		}
	}
}
