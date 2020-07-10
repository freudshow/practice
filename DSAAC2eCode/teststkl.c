#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stackli.h"
#include "stackar.h"
#include "basedef.h"

int chkBrace(int argc, char* argv[])
{
	Stack S;
	char *c = argv[1];
	char s = 0;

	S = CreateStack();

	printf("scan:\n");
	while (*c != '\0') {
		switch(*c) {
		case '(':
			Push(*c, S);
			printf("push left\n");
			break;
		case ')':
			s = Top(S);
			printf("chk left: %c\n", s);
			if(s != '(')
				FatalError("mismatch!!!");
			else
				Pop(S);
			break;
		default:
			printf("ignore: %c\n", *c);
			break;
		}
		c++;
	}

	if (!IsEmpty(S)) {
		printf("mismatch!!!\n");
	} else {
		printf("match!!!\n");
	}

	DisposeStack(S);
	return 0;
}

#define isOperater(c)	((c)=='+' || (c) == '-' || (c) == '*' || (c) == '/' || (c) == '^')
#define isOperand(c)    (isalpha((c)) || isdigit(c))

typedef enum {
	e_pInvalid = -1,
	e_p0 = 0,
	e_p1,
	e_p2
} prior_e;

prior_e getPrior(char o)
{
	switch(o) {
	case '+':
		return e_p0;
	case '-':
		return e_p0;
	case '*':
		return e_p1;
	case '/':
		return e_p1;
	case '^':
		return e_p2;
	default:
		break;
	}

	return e_pInvalid;
}

int calc(int l, int r, char op)
{
	switch(op) {
	case '+':
		return (l+r);
	case '-':
		return (l-r);
	default:
		return 0;
	}
}

void calcPost(int argc, char* argv[])
{
	Stack S;
	char *c = argv[1];
	int left = 0;
	int right = 0;
	int result = 0;

	S = CreateStack();

	printf("scan:\n");
	while (*c != '\0') {
		if (isdigit(*c)) {
			Push(ASCII_TO_HEX(*c), S);
		} else if (isOperater(*c)) {
			left = Top(S);
			Pop(S);
			right = Top(S);
			Pop(S);
			result = calc(left, right, *c);
			Push(result, S);
		}
		c++;
	}
	result = Top(S);
	Pop(S);
	if (!IsEmpty(S)) {
		printf("illegal!!!\n");
	} else {
		printf("result: %d\n", result);
	}
	DisposeStack(S);
}

void infix2postfix(int argc, char* argv[])
{
	char *c = NULL;
	char p[256] = {'\0'};
	stack_s s = { };
	StackArray sa = CreateStackArray(256);

	bzero(sa->Array, 256);
	getStack(&s);
	strcpy(p, argv[1]);
	if (p[0] != '(') {
		s.push('(', s.s);
		p[strlen(p)] = ')';
	}
	printf("p: %s\n", p);

	c = p;
	while (*c != '\0') {
		if (isOperand(*c)) {
			PushArray(*c, sa);
		} else if (*c == '(') {
			s.push(*c, s.s);
		} else if (isOperater(*c)) {
			while (isOperater(s.top(s.s)) && (getPrior(s.top(s.s)) >= getPrior(*c))) {
				PushArray(s.pop(s.s), sa);
			}
			s.push(*c, s.s);
		} else if (*c == ')') {
			while(isOperater(s.top(s.s))) {
				PushArray(s.pop(s.s), sa);
			}
			s.pop(s.s);
		}
		c++;
	}
	printf("[%s][%d]%s\n", __FUNCTION__, __LINE__, sa->Array);
	DisposeStackArray(sa);
	s.disposeStack(s.s);
}

int main(int argc, char* argv[])
{
	infix2postfix(argc, argv);
	exit(0);
}
