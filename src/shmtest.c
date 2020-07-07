#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char** argv) {
	printf("This is parent process%d\n", getpid());

	pid_t p1;
	int i;

	for (i = 0; i <= 20; i++) {
		if ((p1 = fork()) == 0) {
			while(1) {
				printf("This is child [%d] process [%d]\n", i, getpid());
				sleep(1);
			}
		} else {
			waitpid(p1, NULL, 0); //父进程等待p1子进程执行后才能继续fork其他子进程
			printf("This is parent process [%d]\n", getpid());
		}
	}
}
