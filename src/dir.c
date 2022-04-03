#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

void printDir(char *path)
{
	struct stat s_buf;

	stat(path,&s_buf);

	if(S_ISDIR(s_buf.st_mode))
	{
		printf("[%s] it is a dir\n", path);
		struct dirent *filename;
		DIR *dp = opendir(path);

		while(filename = readdir(dp))
		{
			char file_path[200] = {0};
			strcat(file_path,path);
			strcat(file_path,"/");
			strcat(file_path, filename->d_name);

			if (filename->d_name[0] != '.') {
				printDir(file_path);
			} else {
				printf("[%s] it is a dir\n", file_path);
			}
		}

		closedir(dp);
	}
	else if(S_ISREG(s_buf.st_mode))
	{
		printf("[%s] is a regular file\n",path);
	}
}

int main(int argc, char *argv[])
{
	char const *path = argv[1];
	
	printDir(argv[1]);
	return 0;
}
