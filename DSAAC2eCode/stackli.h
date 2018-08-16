/* START: fig3_39.txt */
#ifndef _Stack_h
#define _Stack_h

typedef char elem_t;
struct Node;
typedef struct Node *PtrToNode;
typedef PtrToNode Stack;

struct Node {
	elem_t Element;
	PtrToNode Next;
};

typedef int (*isEmpty_f)(Stack S);
typedef Stack (*createStack_f)(void);
typedef void (*disposeStack_f)(Stack S);
typedef void (*makeEmpty_f)(Stack S);
typedef void (*push_f)(elem_t X, Stack S);
typedef elem_t (*top_f)(Stack S);
typedef elem_t (*pop_f)(Stack S);

typedef struct {
	Stack s;
	createStack_f createStack;
	isEmpty_f isEmpty;
	disposeStack_f disposeStack;
	makeEmpty_f makeEmpty;
	top_f top;
	push_f push;
	pop_f pop;
}stack_s;


extern int isEmpty(Stack S);
extern Stack createStack(void);
extern void disposeStack(Stack S);
extern void makeEmpty(Stack S);
extern void push(elem_t X, Stack S);
extern elem_t top(Stack S);
extern elem_t pop(Stack S);
extern void getStack(stack_s* s);

#endif  /* _Stack_h */

/* END */
