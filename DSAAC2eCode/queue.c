#include "queue.h"
#include "fatal.h"
#include <stdlib.h>

#define MinQueueSize ( 2 )

/*
 * 为安全操作起见,结构
 * 体不能暴露给调用者.
 */
struct queueRecord {
	int capacity; //max elements
	int front;
	int rear;
	int size; //current elements
	elem_t *array;
	queue this;
};

int isEmpty(queue q)
{
	return q->size == 0;
}

int isFull(queue q)
{
	return q->size == q->capacity;
}

int qCapacity(queue q)
{
	return q->capacity;
}

queue createQueue(int MaxElements)
{
	queue q;

	if (MaxElements < MinQueueSize)
		Error("Queue size is too small");

	q = calloc(1, sizeof(struct queueRecord));
	if (q == NULL)
		FatalError("Out of space!!!");
	q->this = q;

	q->this->array = calloc(1, sizeof(elem_t) * MaxElements);
	if (q->this->array == NULL)
		FatalError("Out of space!!!");

	q->this->capacity = MaxElements;
	makeEmpty(q->this);

	return q;
}

/*
 * 因为入列前, rear要先后移1次,
 * 再赋值,所以初始状态下, rear要比
 * front靠前1个位置, 这样在出列时,
 * front的位置才能指向第0个元素.
 * 如果rear先赋值再后移, 则不符合
 * rear的语义了, 即rear总是指向
 * 队列的最后一个有效元素. 如果
 * rear先赋值, 则rear的意义则变成
 * 总是指向队列最后一个有效元素的
 * 下一个位置了.
 */
void makeEmpty(queue q)
{
	q->size = 0;
	q->front = 1;
	q->rear = 0;
}

void disposeQueue(queue q)
{
	if (q != NULL) {
		free(q->array);
		free(q);
	}
}

void enqueue(elem_t e, queue q)
{
	if (isFull(q))
		Error("Full queue");
	else {
		q->size++;
		q->rear = ((q->rear + 1) % q->capacity);
		q->array[q->rear] = e;
	}
}

elem_t front(queue q)
{
	if (!isEmpty(q))
		return q->array[q->front];
	Error("Empty queue");
	return 0;
}

void dequeue(queue q)
{
	if (isEmpty(q))
		Error("Empty queue");
	else {
		q->size--;
		q->front = ((q->front + 1) % q->capacity);
	}
}

elem_t frontAndDequeue(queue q)
{
	elem_t e = 0;

	if (isEmpty(q))
		Error("Empty queue");
	else {
		q->size--;
		e = q->array[q->front];
		q->front = ((q->front + 1) % q->capacity);
	}

	return e;
}
