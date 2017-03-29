#include "basedef.h"
#include "lib.h"
#include "showbytes.h"

int main(int argc, char* argv[])
{
	s_test test;

	show_int(3510593);
	show_int(3510593);
	show_float(3510593.0);
	show_struct(&test);
	DEBUG_OUT("%s %d", "fuck you", 1009);

	int v[] = {4, 6, 12, 19, 5, 34, 145, 68, 92};
	showArray(v, NELEM(v));
	fprintf(stderr, "idx: %d\n", binSearchInt(-67, v,  NELEM(v)));
	fprintf(stderr, "idx: %d\n", binSearchInt(145, v,  NELEM(v)));
	swap(v, 0, 3);
	showArray(v,  NELEM(v));
	swap(v, 4, 8);
	showArray(v,  NELEM(v));
	quikSort(v, NELEM(v));
	showArray(v,  NELEM(v));
	return 0;
}
