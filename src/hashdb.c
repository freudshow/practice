
#include<stdio.h>
#include<stdlib.h>

unsigned long _db_hash(const char *key)
{
	unsigned long		hval = 0;
	char				c;
	int					i;

	for (i = 1; (c = *key++) != 0; i++)
		hval += c * i;		/* ascii char times its 1-based index */
	return(hval % 137);
}

int main(int argc, char* argv[])
{
	printf("hash of %s: %u\n", argv[1], _db_hash(argv[1]));

	exit(0);
}