#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

union intfloat {
	float f;
	int i;
};

int main()
{
	union intfloat a;
	union intfloat b;
	union intfloat c;
	union intfloat d;

	a.f = NAN;
	b.f = NAN;
	c.f = 1.1;
	d.f = 1.1;

	if(a.f == b.f) {
		printf("a == b\n");
	}

	if(a.f == NAN) {
		printf("a == NAN\n");
	}

	if(memcmp(&a, &b, sizeof(a)) == 0) {
		printf("a memcmp == NAN\n");
	}

	if(a.f != c.f) {
		printf("a != c\n");
	}

	if(memcmp(&a.f, &c, sizeof(a)) != 0) {
		printf("a != c\n");
		printf("memcmp: %d\n", memcmp(&a.f, &c, sizeof(a)));
	}

	if(memcmp(&c, &d, sizeof(c)) == 0) {
		printf("c == d\n");
	}

	printf("a: %08x, b: %08x, c: %08x, d: %08x\n", a.i, b.i, c.i, d.i);
	return 0;
}
