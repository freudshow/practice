#include <stdio.h>
#include <stdlib.h>

typedef struct {
	int a;
	char b;
	short c[4];
} test_s;

int main(int argc, char* argv[])
{
	test_s s = {};

	printf("%d, %d, %d, %d, %d, %d\n", s.a, s.b, s.c[0], s.c[1], s.c[2], s.c[3]);
	exit(0);
}