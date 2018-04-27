#include <unistd.h>
#include <stdio.h>

#if defined CCTII
#define S4851   		"/dev/ttyS2"
#define S4852   		"/dev/ttyS1"
#elif defined CJQIII
#define S4851   		"/dev/ttySA1"
#define S4852   		"/dev/ttySA5"
#define SMBUS1   		"/dev/ttySA3"
#define SMBUS2			"/dev/ttySA10"
#define SMBUS3			"/dev/ttySA6"
#define SMODULE1		"/dev/ttySA8"
#define SMODULE2		"/dev/ttySA2"
#define SMODULE3		"/dev/ttySA9"
#define SMODULE4		"/dev/ttySA7"
#define SINFRARED		"/dev/ttySA4"
#elif defined (CCTI) || (CCTIII)
#define S4851   		"/dev/ttyS1"
#define S4852   		"/dev/ttyS2"
#endif

int main(int argc, char * argv[]) {
	int aflag = 0, bflag = 0, cflag = 0;
	int ch;
	printf("optind:%d，opterr：%d\n", optind, opterr);
	printf("--------------------------\n");
	while ((ch = getopt(argc, argv, "ab:c:de::")) != -1) {
		printf("optind: %d,argc:%d,argv[%d]:%s\n", optind, argc, optind,
				argv[optind]);
		switch (ch) {
		case 'a':
			printf("HAVE option: -a\n\n");
			break;
		case 'b':
			printf("HAVE option: -b\n");
			printf("The argument of -b is %s\n\n", optarg);
			break;
		case 'c':
			printf("HAVE option: -c\n");
			printf("The argument of -c is %s\n\n", optarg);
			break;
		case 'd':
			printf("HAVE option: -d\n");
			break;
		case 'e':
			printf("HAVE option: -e\n");
			printf("The argument of -e is %s\n\n", optarg);
			break;
		case '?':
			printf("Unknown option: %c\n", (char) optopt);
			break;
		}
	}
	printf("----------------------------\n");
	printf("optind=%d,argv[%d]=%s\n", optind, optind, argv[optind]);

	printf("%s\n", S4851);
}
