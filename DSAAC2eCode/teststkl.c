#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "stackli.h"
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

#define isOprater(c)	((c)=='+' || (c) == '-')

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
		} else if (isOprater(*c)) {
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
}

int main(int argc, char* argv[])
{
	calcPost(argc, argv);
	exit(0);
}
