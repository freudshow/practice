#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char * argv[])
{
	pid_t pid;
	char *message;
	int n;

	pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	}
	if (pid == 0) {
		message = "This is the child\n";
		fprintf(stdout, "This is the child, pid: %d, ppid: %d\n", getpid(), getppid());
		n = 6;
	} else {
		message = "This is the parent\n";
		fprintf(stdout, "This is the parent, pid: %d, childpid: %d\n", getpid(), pid);
		n = 3;
	}

	for(;1; ) {
		printf(message);
		sleep(1);
	}

	exit(0);
}
