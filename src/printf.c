#include <stdio.h>
#include <stdlib.h>

typedef char boolean;
#define TRUE 0
#define FALSE 1

void foo(int a, int b, int c)
{
	printf("a: %d, &a: %p\n", a, &a);
	printf("b: %d\n", b);
	printf("c: %d, &c: %p\n", c, &c);
}

int main(void)
{
	foo(1,2,3);
    exit(0);
}
