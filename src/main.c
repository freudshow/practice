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

	float v[] = {4.5, 6, 12, 1.9, 5, 3.4, 145, 68, 92};//0-8
	showArrayf(v, NELEM(v));
	SWAP_NUM(v[0], v[4]);
	showArrayf(v, NELEM(v));
	return 0;
}
