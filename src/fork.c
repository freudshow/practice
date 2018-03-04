#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

char *message;
int g_int = 0;

int main(int argc, char * argv[])
{
	pid_t pid;

	int n;

	pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	}
	if (pid == 0) {
		message = "This is the child, ";
		fprintf(stdout, "<%d>This is the child, pid: %d, ppid: %d\n", __LINE__, getpid(), getppid());
		n = 6;
	} else {
		message = "This is the parent, ";
		fprintf(stdout, "<%d>This is the parent, pid: %d, childpid: %d\n",  __LINE__, getpid(), pid);
		n = 3;
	}

	pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	}
	if (pid == 0) {
		message = "This is the other child, ";
		fprintf(stdout, "<%d>%s, pid: %d, ppid: %d\n",  __LINE__, message, getpid(), getppid());
		n = 6;
	} else {
		message = "This is the other parent, ";
		fprintf(stdout, "<%d>%s, pid: %d, childpid: %d\n",  __LINE__, message, getpid(), pid);
		n = 3;
	}

	int i=0;
	g_int = getpid();
	for(i=0;1; ++i) {
		fprintf(stdout, "<%d><%d>%spid: %d, ppid: %d, g_int: %d\n", i,  __LINE__, message, getpid(), getppid(), g_int);
		sleep(3);
	}

	exit(0);
}
