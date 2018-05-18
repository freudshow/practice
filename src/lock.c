#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int lock_set(int fd, int type) {
	struct flock lock;
	lock.l_whence = SEEK_SET;//at the beginning of file
	lock.l_start = 0;//0 bytes offset to l_whence
	lock.l_len = 0;//from l_start to the end of the file
	lock.l_type = type;//F_RDLCK, F_WRLCK, or F_UNLCK

	//judge if range of file is set a lock
	fcntl(fd, F_GETLK, &lock);//fcntl has already updated lock.l_type
	if (lock.l_type != F_UNLCK) {
		if (lock.l_type == F_RDLCK) {
			printf("!= F_UNLCK Read lock already set by %d .\n", lock.l_pid);
		} else if (lock.l_type == F_WRLCK) {
			printf("!= F_UNLCK Write lock already set by %d .\n", lock.l_pid);
		}
	}

	lock.l_type = type;

	if ((fcntl(fd, F_SETLKW, &lock)) < 0) {//F_SETLKW means SET LocK Wait
		printf("Lock failed:type = %d\n", lock.l_type);
		return 1;
	}

	switch (lock.l_type) {
	case F_RDLCK:
		printf("Read lock set by %d \n", getpid());
		break;
	case F_WRLCK:
		printf("Write lock set by %d \n", getpid());
		break;
	case F_UNLCK:
		printf("Release lock by %d \n", getpid());
		return 1;
		break;
	default:
		break;
	}
	return 0;
}

int main() {
	int fd;

	fd = open("hello", O_RDWR | O_CREAT, 0644);
	if (fd < 0) {
		printf("Open file error!\n");
		exit(1);
	}

	lock_set(fd, F_WRLCK);
	getchar();

	lock_set(fd, F_UNLCK);
	getchar();
	close(fd);
	exit(0);
}

