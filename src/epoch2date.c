/*
 * epoch2date.c
 *
 *  Created on: 2019年7月1日
 *      Author: floyd
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../inc/basedef.h"

void usage()
{
	printf("usage: app \"unix second\"\n");
}

int main(int argc, char** argv)
{
	time_t timet = 0;
	struct tm tmp_tm;
	char *p = NULL;

	if(argc != 2) {
		usage();
		goto err;
	}

	p = argv[1];
	while(*p != '\0') {
		if(!isDigit(*p)) {
			usage();
			goto err;
		}
		p++;
	}

	timet = atol(argv[1]);
	localtime_r(&timet, &tmp_tm);
	printf("date: %04d-%02d-%02d %02d:%02d:%02d\n",
				tmp_tm.tm_year + 1900, tmp_tm.tm_mon + 1, tmp_tm.tm_mday,
				tmp_tm.tm_hour, tmp_tm.tm_min, tmp_tm.tm_sec);

	exit(0);

err:
	exit(1);
}
