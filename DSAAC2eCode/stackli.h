typedef int ElementType;
/* START: fig3_39.txt */
#ifndef _Stack_h
#define _Stack_h

struct Node;
typedef struct Node *PtrToNode;
typedef PtrToNode Stack;

typedef struct Node {
	ElementType Element;
	PtrToNode Next;
} stack_s;




int IsEmpty(Stack S);
Stack CreateStack(void);
void DisposeStack(Stack S);
void MakeEmpty(Stack S);
void Push(ElementType X, Stack S);
ElementType Top(Stack S);
void Pop(Stack S);

#endif  /* _Stack_h */

/* END */
