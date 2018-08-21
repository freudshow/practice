#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
	int fd = -1;
	struct iovec	iov[2];
	char vec1[10] = {0};
	char vec2[13] = {0};
	int i = 0;
	
	iov[0].iov_base = vec1;
	iov[0].iov_len  = sizeof(vec1)-1;
	iov[1].iov_base = vec2;
	iov[1].iov_len  = sizeof(vec2)-1;
	
	fd = open(argv[1],O_RDONLY);
	if(fd < 0)
		exit(1);
	i = readv(fd, &iov[0], 2);
	vec1[sizeof(vec1)-1] = '\0';
	vec2[sizeof(vec2)-1] = '\0';
	printf("%s %s\n", vec1, vec2);
	close(fd);
	exit(0);
}
