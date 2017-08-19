#include <stdio.h>
#include <stdlib.h>
#include "list.h"

int isEmpty(list L)
{
	return (L->pNext == NULL);
}

int isLast(position P, list L)
{
	return (P->pNext == NULL);
}

list makeEmpty(list L)
{
	if(L != NULL)
		deletelist(L);

	L = malloc(sizeof(node_s));
	if(L == NULL)
		FatalError("Out of memory!");

	L->pNext = NULL;
	return L;
}

position find(elementType X, list L)
{
	position p;

	p = L->pNext;
	while(p != NULL && p->element != X)
		p = p->pNext;

	return p;
}

void delete(elementType X, list L)
{
	position p, tmpCell;

	p = findPrevious(X, L);

	if(!isLast(p, L)) {
		tmpCell = p->pNext;
		p->pNext = tmpCell->pNext;
		free(tmpCell);
	}
}

/*
 * If X is not found,
 * then Next field of returned value is NULL
 * assumes a header
 */
position findPrevious(elementType X, list L)
{
    position p;

	p = L;
	while((p->pNext != NULL) && (p->pNext->element != X))
		p = p->pNext;

	return p;
}

/*
 * insert X after P
 * header implementation assumed
 */
void insert(elementType X, list L, position P)
{
	position tmpCell;

	tmpCell = malloc(sizeof(node_s));
	if(tmpCell == NULL)
		FatalError("out of space!!!");

	tmpCell->element = X;
	tmpCell->pNext = P->pNext;
	P->pNext = tmpCell;
}

void deletelist(list L)
{
    position p, tmp;

	p = L->pNext;  /* Header assumed */
	L->pNext = NULL;
	while(p != NULL) {
		tmp = p->pNext;
		free(p);
		p = tmp;
    }
}

position header(list L)
{
	return L;
}

position first(list L)
{
	return L->pNext;
}

position advance(position P)
{
	return P->pNext;
}

elementType retrieve(position P)
{
	return P->element;
}
