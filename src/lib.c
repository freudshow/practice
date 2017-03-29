#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "lib.h"

void get_local_time(char* buf, u32 bufSize)
{
	time_t rawtime;
	struct tm timeinfo;

	if ((bufSize < 20) || (NULL == buf))
		return;

	time(&rawtime);
	localtime_r(&rawtime, &timeinfo);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", (timeinfo.tm_year + 1900),
			(timeinfo.tm_mon + 1), timeinfo.tm_mday, timeinfo.tm_hour,
			timeinfo.tm_min, timeinfo.tm_sec);
}

void debug(const char* file, const char* func, u32 line, const char *fmt, ...)
{
	va_list ap;
	char buf[20] = { 0 };

	get_local_time(buf, sizeof(buf));
	fprintf(stderr, "[%s][%s][%s()][%d]: ", buf, file, func, line);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
}

void showArray(int v[], int length)
{
	int i;

	for (i = 0; i < length; i++) {
		fprintf(stderr, "%d\t", v[i]);
	}
	fprintf(stderr, "\n");
}

int arrayCmp(int a, int b)
{
	if (a == b)
		return EQUAL_TO;
	else if (a > b)
		return GREAT_THAN;
	else if (a < b)
		return SMALL_THAN;

	return -2;
}

/*****************************
 * @elem	an element to be found
 * @array	an array sorted ascend
 * return the index of elem in array, if found;
 * reutrn -1 if not found.
 *****************************/
int binSearchInt(int elem, int array[], int length)
{
	int low, high, mid, cmp;
	low = 0;
	high = (length - 1);
	mid = (low+high)/2;
	cmp = INIT_STATE;
	while(low <= high) {
		mid = (low+high)/2;
		cmp = arrayCmp(elem, array[mid]);
		switch (cmp) {
		case GREAT_THAN:
			low = mid+1;
			break;
		case SMALL_THAN:
			high = mid-1;
			break;
		case EQUAL_TO:
			return mid;
		default:
			break;
		}
	}

	return NOT_FOUND;
}

void swap(int v[], int i, int j)
{
	int temp;
	temp = v[i];
	v[i] = v[j];
	v[j] = temp;
}

void quikSort(int v[], int n)
{
	int i, last;

	if(n <= 1)
		return;
	fprintf(stderr, "before swap pivot:\n");
	showArray(v, n);
	swap(v, 0, (rand()%n));
	fprintf(stderr, "after swap pivot:\n");
	showArray(v, n);
	last = 0;
	for(i=1; i < n; i++)
		if(v[i] < v[0])
			swap(v, ++last, i);
	fprintf(stderr, "after select:\n");
	showArray(v, n);

	swap(v, 0, last);
	fprintf(stderr, "after restore pivot:\n");
	showArray(v, n);

	quikSort(v, last);
	quikSort(v+last+1, n-last-1);
}
