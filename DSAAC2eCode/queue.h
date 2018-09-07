typedef int elem_t;
/* START: fig3_57.txt */
#ifndef _Queue_h
#define _Queue_h

struct queueRecord;
typedef struct queueRecord *queue;

extern int isEmpty(queue Q);
extern int isFull(queue Q);
extern int qCapacity(queue Q);
extern queue createQueue(int MaxElements);
extern void disposeQueue(queue Q);
extern void makeEmpty(queue Q);
extern void enqueue(elem_t X, queue Q);
extern elem_t front(queue Q);
extern void dequeue(queue Q);
extern elem_t frontAndDequeue(queue Q);

#endif  /* _Queue_h */
/* END */
