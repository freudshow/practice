#include "basedef.h"
#include "lib.h"
#include "showbytes.h"

int main(int argc, char* argv[])
{
//	s_test test;

//	show_int(3510593);
//	show_int(3510593);
//	show_float(3510593.0);
//	show_struct(&test);

	int v[] = {4, 6, 12, 1, 5, 3, 145, 68, 92};//0-8

	showArray(v, NELEM(v));
	SWAP_NUM(v[0], v[4]);
	showArray(v, NELEM(v));

	return 0;
}
