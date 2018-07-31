#include "stackli.h"
#include "fatal.h"
#include <stdlib.h>

int IsEmpty(Stack S) {
	return (S->Next == NULL);
}

Stack CreateStack(void) {
	Stack S;

	S = malloc(sizeof(stack_s));
	if (S == NULL)
		FatalError("Out of space!!!");
	S->Next = NULL;
	MakeEmpty(S);
	return S;
}

void MakeEmpty(Stack S) {
	if (S == NULL)
		Error("Must use CreateStack first");
	else
		while (!IsEmpty(S))
			Pop(S);
}

void DisposeStack(Stack S) {
	MakeEmpty(S);
	free(S);
}

void Push(elem_t X, Stack S) {
	PtrToNode TmpCell;

	TmpCell = malloc(sizeof(struct Node));
	if (TmpCell == NULL)
		FatalError("Out of space!!!");
	else {
		TmpCell->Element = X;
		TmpCell->Next = S->Next;
		S->Next = TmpCell;
//		printf("Push: %c\n", TmpCell->Element);
	}
}

elem_t Top(Stack S) {
	if (!IsEmpty(S))
		return S->Next->Element;
	Error("Empty stack");
	return (elem_t)0; /* Return value used to avoid warning */
}

elem_t Pop(Stack S) {
	PtrToNode FirstCell;
	elem_t e = (elem_t)0;

	if (IsEmpty(S))
		Error("Empty stack");
	else {
		FirstCell = S->Next;
		S->Next = S->Next->Next;
		e = FirstCell->Element;
//		printf("%c", FirstCell->Element);
		free(FirstCell);
	}
	return e;
}

void getStack(stack_s* s)
{
	s->createStack = CreateStack;
	s->isEmpty = IsEmpty;
	s->disposeStack = DisposeStack;
	s->makeEmpty = MakeEmpty;
	s->top = Top;
	s->push = Push;
	s->pop = Pop;
	s->s = s->createStack();
}
