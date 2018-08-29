#include "queue.h"
#include "fatal.h"
#include <stdlib.h>

#define MinQueueSize ( 5 )

struct QueueRecord {
	int Capacity;//max elements
	int Front;
	int Rear;
	int Size;//current elements
	ElementType *Array;
	Queue this;
};

int IsEmpty(Queue Q) {
	return Q->Size == 0;
}

int IsFull(Queue Q) {
	return Q->Size == Q->Capacity;
}

Queue CreateQueue(int MaxElements) {
	Queue Q;

	if (MaxElements < MinQueueSize)
		Error("Queue size is too small");

	Q = calloc(1, sizeof(struct QueueRecord));
	if (Q == NULL)
		FatalError("Out of space!!!");
	Q->this = Q;

	Q->this->Array = calloc(1, sizeof(ElementType) * MaxElements);
	if (Q->this->Array == NULL)
		FatalError("Out of space!!!");
	Q->this->Capacity = MaxElements;
	MakeEmpty(Q->this);

	return Q;
}

void MakeEmpty(Queue Q) {
	Q->Size = 0;
	Q->Front = 1;
	Q->Rear = 0;
}

void DisposeQueue(Queue Q) {
	if (Q != NULL) {
		free(Q->Array);
		free(Q);
	}
}

void Enqueue(ElementType X, Queue Q) {
	if (IsFull(Q))
		Error("Full queue");
	else {
		Q->Size++;
		Q->Rear = ((Q->Rear+1)%Q->Capacity);
		Q->Array[Q->Rear] = X;
	}
}

ElementType Front(Queue Q) {
	if (!IsEmpty(Q))
		return Q->Array[Q->Front];
	Error("Empty queue");
	return 0;
}

void Dequeue(Queue Q) {
	if (IsEmpty(Q))
		Error("Empty queue");
	else {
		Q->Size--;
		Q->Front = ((Q->Front+1)%Q->Capacity);
	}
}

ElementType FrontAndDequeue(Queue Q) {
	ElementType X = 0;

	if (IsEmpty(Q))
		Error("Empty queue");
	else {
		Q->Size--;
		X = Q->Array[Q->Front];
		Q->Front = ((Q->Front+1)%Q->Capacity);
	}

	return X;
}
