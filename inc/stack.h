#ifndef STACK_H
#define STACK_H

#include "basedef.h"

struct StackRecord;
typedef node_s* Stack;

extern int isEmpty(Stack S);
extern int isFull(Stack S);
extern Stack createStack(int MaxElements);
extern void disposeStack(Stack S);
extern void makeEmpty(Stack S);
extern void push(elementType X, Stack S);
extern elementType top(Stack S);
extern void pop(Stack S);
extern elementType topAndPop(Stack S);

#endif // STACK_H
