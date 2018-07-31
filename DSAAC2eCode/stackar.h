typedef char ElementType;
/* START: fig3_45.txt */
#ifndef _StackArray_h
#define _StackArray_h

struct StackArrayRecord;
typedef struct StackArrayRecord *StackArray;

struct StackArrayRecord {
	int Capacity;
	int TopOfStack;
	ElementType *Array;
};

int IsEmptyArray(StackArray S);
int IsFullArray(StackArray S);
StackArray CreateStackArray(int MaxElements);
void DisposeStackArray(StackArray S);
void MakeEmptyArray(StackArray S);
void PushArray(ElementType X, StackArray S);
ElementType TopArray(StackArray S);
void PopArray(StackArray S);
ElementType TopAndPop(StackArray S);

#endif  /* _StackArray_h */

/* END */
