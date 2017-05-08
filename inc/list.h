#ifndef list_H
#define list_H

#include "basedef.h"

#pragma pack(push)
#pragma pack(1)


typedef int elementType;
struct node;
typedef struct node node_s;
typedef node_s* ptrToNode;
typedef ptrToNode list;
typedef ptrToNode position;

typedef struct node {
    elementType element;
    position pNext;
} node_s;



#pragma pack(pop)


extern list makeEmpty( list L );
extern int isEmpty( list L );
extern int isLast( position P, list L );
extern position find( elementType X, list L );
extern void delete( elementType X, list L );
extern position findPrevious( elementType X, list L );
extern void insert( elementType X, list L, position P );
extern void deletelist( list L );
extern position header( list L );
extern position first( list L );
extern position advance( position P );
extern elementType retrieve( position P );



#endif // list_H
