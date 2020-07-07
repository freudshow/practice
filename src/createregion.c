#include "shm.h"

int rand(void);
int rand_r(unsigned int *seedp);
void srand(unsigned int seed);

int main()
{
	init_sem_set();
	init_region();

	exit(0);
}

