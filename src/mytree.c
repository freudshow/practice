#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef char elem_t;

typedef struct tree_node *tree_ptr;
typedef struct tree_node {
	elem_t e[256];
	elem_t size;
	tree_ptr father;
	tree_ptr child;
	tree_ptr sibling;
} tNode_s;

void makeEmpty(tree_ptr t);

tree_ptr creatTree(const char* name)
{
	tree_ptr t = NULL;
	t = calloc(1, sizeof(tNode_s));
	if (NULL != t) {
		makeEmpty(t);
		strcpy(t->e, name);
	}
	return t;
}

void makeEmpty(tree_ptr t)
{
	if (NULL == t)
		return;

	bzero(t->e, sizeof(t->e));
    t->size = 0;
	t->father = NULL;
	t->child = NULL;
	t->sibling = NULL;
}

void addChild(tree_ptr s, tree_ptr t)
{
	if (NULL == s && NULL == t)
		return;

	if (NULL != s->child)
		return;

	s->child = t;
	t->father = s;
}

void addSibling(tree_ptr s, tree_ptr t)
{
	tree_ptr p = NULL;

	if (NULL == s && NULL == t)
		return;

	p = s;
	while (NULL != p->sibling) {
		p = p->sibling;
	}

	s->sibling = t;
	t->father = s->father;
}

void print_name(tree_ptr t, int depth)
{
	int i = 0;
//	for (i = 0; i < depth; i++)
//		printf("\t");
//	printf("/%s\n", t->e);
//
	tree_ptr p = NULL;
	tree_ptr list[256] = {};

	p = t->father;
	while(p) {
		list[i++] = p;
		p = p->father;
	}

	while (i--) {//print father one by one with '/'
		printf("/%s", list[i]->e);
	}
	printf("/%s\n", t->e);//print itself with '/'
}

int isLeaf(tree_ptr t)
{
	return (t->child == NULL);
}

void list_dir(tree_ptr t, unsigned int depth)
{
	tree_ptr p = NULL;

	if (isLeaf(t))
		print_name(t, depth);

	if (!isLeaf(t)) {
		print_name(t, depth);
		p = t->child;
		while (NULL != p) {
			list_dir(p, depth + 1);
			p = p->sibling;
		}
	}
}

void list_directory(tree_ptr t)
{
	list_dir(t, 0);
}

int main(int argc, char** argv)
{
	tree_ptr root = NULL;
	tree_ptr t0 = NULL;
	tree_ptr t1 = NULL;
	tree_ptr t2 = NULL;
	tree_ptr t3 = NULL;
	tree_ptr t4 = NULL;
	tree_ptr t5 = NULL;
	tree_ptr t6 = NULL;
	tree_ptr t7 = NULL;
	tree_ptr t8 = NULL;
	tree_ptr t9 = NULL;
	tree_ptr t10 = NULL;
	tree_ptr t11 = NULL;
	tree_ptr t12 = NULL;
	tree_ptr t13 = NULL;
	tree_ptr t14 = NULL;

	root = creatTree("usr");
	t0 = creatTree("inc");
	t1 = creatTree("src");
	t2 = creatTree("local");
	t3 = creatTree("std");
	t4 = creatTree("str.h");
	t5 = creatTree("std");
	t6 = creatTree("str.c");
	t7 = creatTree("bin");
	t8 = creatTree("share");
	t9 = creatTree("io.h");
	t10 = creatTree("io.c");
	t11 = creatTree("cp");
	t12 = creatTree("qmake");
	t13 = creatTree("unistd.h");
	t14 = creatTree("make");

	addChild(root, t0);
	addSibling(t0, t1);
	addSibling(t1, t2);

	addChild(t0, t3);
	addSibling(t3, t4);
	addChild(t3, t9);
	addSibling(t9, t13);

	addChild(t1, t5);
	addSibling(t5, t6);
	addChild(t5, t10);

	addChild(t2, t7);
	addChild(t7, t11);
	addSibling(t7, t8);
	addSibling(t11, t14);
	addChild(t8, t12);

	list_directory(root);

	exit(0);
}
