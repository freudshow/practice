#include <stdio.h>
#include <stdlib.h>

#define TODO()   do {\
                    printf ("\nAdd your code here: file \"%s\", line %d\n",\
                            __FILE__, __LINE__);\
                }while(0)
///////////////////////////////////////////////

// Data structures for the Sum language.

typedef enum {EXP_INT, EXP_SUM} Exp_e;

typedef struct
{
    Exp_e kind;
}Exp_s;

typedef struct
{
    Exp_e kind;
    int i;
}expInt_s;

typedef struct
{
    Exp_e kind;
    Exp_s *left;
    Exp_s *right;
}expSum_s;

// "constructors"
Exp_s *Exp_Int_new(int i)
{
    expInt_s *p = malloc(sizeof(*p));
    p->kind = EXP_INT;
    p->i = i;

    return (Exp_s *) p;
}

Exp_s *Exp_Sum_new(Exp_s *left, Exp_s *right)
{
    expSum_s *p = malloc(sizeof(*p));
    p->kind = EXP_SUM;
    p->left = left;
    p->right = right;

    return (Exp_s *) p;
}

// "printer"
void Exp_print(Exp_s *exp)
{
    switch (exp->kind) {
    case EXP_INT: {
        expInt_s *p = (expInt_s *) exp;
        printf("%d", p->i);
        break;
    }
    case EXP_SUM: {
        expSum_s *p = (expSum_s *) exp;
        Exp_print(p->left);
        printf("+");
        Exp_print(p->right);
        break;
    }
    default:
        break;
    }
}

//////////////////////////////////////////////

// Data structures for the Stack language.
typedef enum {
    STACK_ADD,
    STACK_PUSH
}stackKink_e;

typedef struct
{
    stackKink_e kind;
}stack_s;

typedef struct
{
    stackKink_e kind;
}stackAdd_s;

typedef struct
{
    stackKink_e kind;
    int i;
}stackPush_s;

// "constructors"
stack_s *Stack_Add_new()
{
    stackAdd_s *p = malloc(sizeof(*p));
    p->kind = STACK_ADD;

    return (stack_s *) p;
}

stack_s *Stack_Push_new(int i)
{
    stackPush_s *p = malloc(sizeof(*p));
    p->kind = STACK_PUSH;
    p->i = i;

    return (stack_s *) p;
}

/// instruction list
typedef struct List_t
{
    stack_s *instr;
    struct List_t *next;
}list_s;

struct List_t *List_new(stack_s *instr, struct List_t *next)
{
    struct List_t *p = malloc(sizeof(*p));
    p->instr = instr;
    p->next = next;

    return p;
}

// "printer"
void List_reverse_print(struct List_t *list)
{
    TODO();
}

//////////////////////////////////////////////////

// a compiler from Sum to Stack
struct List_t *all = 0;
void emit(stack_s *instr)
{
    all = List_new(instr, all);
}

void compile(Exp_s *exp)
{
    switch (exp->kind) {
    case EXP_INT: {
        expInt_s *p = (expInt_s *) exp;
        emit(Stack_Push_new(p->i));
        break;
    }
    case EXP_SUM: {
        TODO();
        break;
    }
    default:
        break;
    }
}

//////////////////////////////////////////////////

// program entry

int main()
{
    printf("Compile starting\n");
    // build an expression tree:
    //            +
    //           / 
    //          +   4
    //         / 
    //        2   3

    Exp_s *exp = Exp_Sum_new(Exp_Sum_new(Exp_Int_new(2)
    , Exp_Int_new(3))
    , Exp_Int_new(4));

    // print out this tree:
    printf("the expression is:\n");

    Exp_print(exp);

    // compile this tree to Stack machine instructions
    compile(exp);

    // print out the generated Stack instructons:
    List_reverse_print(all);
    printf("\nCompile finished\n");

    return 0;

}

