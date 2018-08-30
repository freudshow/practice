#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef char elem_t;

typedef struct tree_node *tree_ptr;
typedef struct tree_node {
	elem_t e[256];
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
	t->child = NULL;
	t->sibling = NULL;
}

void addChild(tree_ptr s, tree_ptr t)
{
	if (NULL == s && NULL == t)
		return;

	if (NULL != s->sibling)
		return;

	s->sibling = t;
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
}

list_directory(tree_ptr t)
{
	list_dir(t, 0);
}

void print_name(tree_ptr t, int depth)
{
	int i = 0;
	for (i = 0; i < depth; i++)
		printf("-");
	printf(">%s\n", t->e);
}

int isLeaf(tree_ptr t)
{
	return (t->child == NULL);
}

void list_dir(tree_ptr t, unsigned int depth)
{
	tree_ptr p = NULL;

	if (isLeaf(t))
		print_name(depth, t);

	if (!isLeaf(t)) {
		print_name(depth, t);
		p = t->child;
		while (NULL != p) {
			list_dir(p, depth + 1);
			p = p->sibling;
		}
	}
}

int main(int argc, char** argv)
{
	tree_ptr t = NULL;


}
