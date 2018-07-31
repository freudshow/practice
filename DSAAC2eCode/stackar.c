#include "stackar.h"
#include "fatal.h"
#include <stdlib.h>

#define EmptyTOS ( -1 )
#define MinStackArraySize ( 5 )

/* START: fig3_48.txt */
int IsEmptyArray(StackArray S) {
	return S->TopOfStack == EmptyTOS;
}
/* END */

int IsFullArray(StackArray S) {
	return S->TopOfStack == S->Capacity - 1;
}

/* START: fig3_46.txt */
StackArray CreateStackArray(int MaxElements) {
	StackArray S;

	/* 1*/if (MaxElements < MinStackArraySize)
		/* 2*/Error("StackArray size is too small");

	/* 3*/S = malloc(sizeof(struct StackArrayRecord));
	/* 4*/if (S == NULL)
		/* 5*/FatalError("Out of space!!!");

	/* 6*/S->Array = malloc(sizeof(ElementType) * MaxElements);
	/* 7*/if (S->Array == NULL)
		/* 8*/FatalError("Out of space!!!");
	/* 9*/S->Capacity = MaxElements;
	/*10*/MakeEmptyArray(S);

	/*11*/return S;
}
/* END */

/* START: fig3_49.txt */
void MakeEmptyArray(StackArray S) {
	S->TopOfStack = EmptyTOS;
}
/* END */

/* START: fig3_47.txt */
void DisposeStackArray(StackArray S) {
	if (S != NULL) {
		free(S->Array);
		free(S);
	}
}
/* END */

/* START: fig3_50.txt */
void PushArray(ElementType X, StackArray S) {
	if (IsFullArray(S))
		Error("Full StackArray");
	else
		S->Array[++(S->TopOfStack)] = X;
}
/* END */

/* START: fig3_51.txt */
ElementType TopArray(StackArray S) {
	if (!IsEmptyArray(S))
		return S->Array[S->TopOfStack];
	Error("Empty StackArray");
	return 0; /* Return value used to avoid warning */
}
/* END */

/* START: fig3_52.txt */
void PopArray(StackArray S) {
	if (IsEmptyArray(S))
		Error("Empty StackArray");
	else
		S->TopOfStack--;
}
/* END */

/* START: fig3_53.txt */
ElementType TopAndPop(StackArray S) {
	if (!IsEmptyArray(S))
		return S->Array[S->TopOfStack--];
	Error("Empty StackArray");
	return 0; /* Return value used to avoid warning */
}
/* END */
