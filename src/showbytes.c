#include <stdio.h>
#include "lib.h"
#include "showbytes.h"

void show_bytes(byte_pointer start, int len)
{
	int i;
	printf("length: %d\n", len);
	for (i = 0; i < len; i++)
		printf(" %.2x", start[i]);
	printf("\n");
}

void show_int(int x)
{
	show_bytes((byte_pointer) &x, sizeof(int));
}

void show_uint(unsigned int x)
{
	show_bytes((byte_pointer) &x, sizeof(int));
}

void show_float(float x)
{
	show_bytes((byte_pointer) &x, sizeof(float));
}

void show_pointer(void *x)
{
	show_bytes((byte_pointer) &x, sizeof(void *));
}

void show_struct(void *x)
{
	s_test* p = (s_test*) x;
	show_bytes((byte_pointer) &x, sizeof(s_test));
	printf("struct: %p, a: %p, b: %p, c: %p, d: %p\n", p, &(p->a), &(p->b),
			&(p->c), &(p->d));
}
