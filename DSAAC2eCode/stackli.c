#include "stackli.h"
#include "fatal.h"
#include <stdlib.h>

int isEmpty(Stack S) {
	return (S->Next == NULL);
}

Stack createStack(void) {
	Stack S;

	S = malloc(sizeof(stack_s));
	if (S == NULL)
		FatalError("Out of space!!!");
	S->Next = NULL;
	MakeEmpty(S);
	return S;
}

void makeEmpty(Stack S) {
	if (S == NULL)
		Error("Must use CreateStack first");
	else
		while (!IsEmpty(S))
			Pop(S);
}

void disposeStack(Stack S) {
	MakeEmpty(S);
	free(S);
}

void push(elem_t X, Stack S) {
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

elem_t top(Stack S) {
	if (!IsEmpty(S))
		return S->Next->Element;
	Error("Empty stack");
	return (elem_t)0; /* Return value used to avoid warning */
}

elem_t pop(Stack S) {
	PtrToNode FirstCell;
	elem_t e = (elem_t)0;

	if (IsEmpty(S))
		Error("Empty stack");
	else {
		FirstCell = S->Next;
		S->Next = S->Next->Next;
		e = FirstCell->Element;
		free(FirstCell);
	}
	return e;
}

void getStack(stack_s* s)
{
	s->createStack = createStack;
	s->isEmpty = isEmpty;
	s->disposeStack = disposeStack;
	s->makeEmpty = makeEmpty;
	s->top = top;
	s->push = push;
	s->pop = pop;
	s->s = s->createStack();
}
