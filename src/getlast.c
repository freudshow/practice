#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	printf("%s\n", getLastStr(argv[1], argv[2][0]));
	return 0;
}
