#include <stdio.h>
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
	position p = NULL;

	while(!isEmpty(L)) {
		p = L->pNext;
		L = p->pNext;
		free(p);
	}
}

position find(elementType X, list L)
{

}

void delete(elementType X, list L)
{
	position P, tmpCell;

	P = findPrevious(X, L);

	if(!isLast(P, L)) {	/* Assumption of header use */
	                     	 /* X is found; delete it */
		tmpCell = P->pNext;
		P->pNext = tmpCell->pNext;  /* Bypass deleted cell */
		free(tmpCell);
	}
}

/*
 * If X is not found,
 * then Next field of returned value is NULL
 * Assumes a header
 */
position findPrevious(elementType X, list L)
{
    position p;

	p = L;
	while( (p->pNext != NULL) && (p->pNext->element != X ))
		p = p->pNext;

	return p;
}

/*
 * insert X after P
 * If X is not found,
 * then Next field of returned value is NULL
 * Assumes a header
 */
void insert(elementType X, list L, position P)
{

}

void deletelist(list L)
{
    position P, Tmp;

	P = L->pNext;  /* Header assumed */
	L->pNext = NULL;
	while( P != NULL ) {
		Tmp = P->pNext;
		free( P );
		P = Tmp;
    }
}

position header(list L)
{

}

position first(list L)
{

}

position advance(position P)
{

}

elementType retrieve(position P)
{

}
