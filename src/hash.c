#include <stdio.h>
#include <stdlib.h>

unsigned hash(const char *s) {
	const unsigned char *p;
	unsigned hashval = 0;
	for (p = (const unsigned char *) s; *p; p++)
		hashval = *p + 137u * hashval;
	return hashval;
}

int main(int argc, char const *argv[]) {
	unsigned h = hash(argv[1]);
	fprintf(stderr, "%x\n", h);

	return 0;
}
