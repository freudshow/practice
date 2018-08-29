#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

#define QUEUE_SIZE 2048

int main(int argc, char** argv) {
	Queue Q;
	int i;
	int cap = 0;

	if(argc == 2) {
		cap = atoi(argv[1]);
	} else
		cap = QUEUE_SIZE;

	Q = CreateQueue(cap);

	for (i = 0; i < cap; i++)
		Enqueue(i, Q);

	while (!IsEmpty(Q)) {
		printf("%d\n", Front(Q));
		Dequeue(Q);
	}

	for (i = 0; i < cap; i++)
		Enqueue(i, Q);

	while (!IsEmpty(Q)) {
		printf("%d\n", Front(Q));
		Dequeue(Q);
	}

	DisposeQueue(Q);
	return 0;
}
