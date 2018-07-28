#ifndef _STACK_H
#define _STACK_H

typedef int ElementType;

struct Node;
typedef stack_s *PtrToNode;
typedef PtrToNode Stack;

typedef struct Node {
	ElementType Element;
	PtrToNode Next;
}stack_s;


int IsEmpty(Stack S);
Stack CreateStack(void);
void DisposeStack(Stack S);
void MakeEmpty(Stack S);
void Push(ElementType X, Stack S);
ElementType Top(Stack S);
void Pop(Stack S);

#endif  //_STACK_H

