typedef char elem_t;
/* START: fig3_39.txt */
#ifndef _Stack_h
#define _Stack_h

struct Node;
typedef struct Node *PtrToNode;
typedef PtrToNode Stack;

typedef struct Node {
	elem_t Element;
	PtrToNode Next;
} stack_s;




int IsEmpty(Stack S);
Stack CreateStack(void);
void DisposeStack(Stack S);
void MakeEmpty(Stack S);
void Push(elem_t X, Stack S);
elem_t Top(Stack S);
void Pop(Stack S);

#endif  /* _Stack_h */

/* END */
