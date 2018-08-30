#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

#define QUEUE_SIZE 2048

int main(int argc, char** argv) {
	queue q;
	int i;
	int nelem = 0;

	if(argc == 2) {
		nelem = atoi(argv[1]);
	} else
		nelem = QUEUE_SIZE;

	q = createQueue(nelem);

	for (i = 0; i < qCapacity(q); i++)
		enqueue(i, q);

	while (!isEmpty(q))
		printf("%d\n", frontAndDequeue(q));

	disposeQueue(q);
	return 0;
}
