#ifndef __BASEDEF_H__
#define __BASEDEF_H__

typedef	unsigned	char	u8;
typedef	unsigned	short	u16;
typedef	unsigned	int		u32;

typedef	char	s8;
typedef	short	s16;
typedef	int		s32;

typedef int elementType;
struct node;
typedef struct node node_s;
typedef node_s* ptrToNode;
typedef ptrToNode list;
typedef ptrToNode position;

#pragma pack(push)
#pragma pack(1)

typedef struct node {
    elementType element;
    position pNext;
};

typedef struct {
	char*	name;
	int		value;
} nameVal;

#pragma pack(pop)

#define	FILE_LINE   __FILE__,__FUNCTION__,__LINE__
#define	DEBUG_PRINT(format, ...)	fprintf(stderr, "[%s][%s][%d]"format"\n", , ##__VA_ARGS__)

#define HEX_TO_BCD(x) (((x)/0x0A)*0x10+((x)%0x0A))
#define BCD_TO_HEX(x) (((x)/0x10)*0x0A+((x)%0x10))

#define MERGE_TWO_BYTE(byt_h,byt_l)	(((u16)(byt_h)<<8)+byt_l)
#define LOW_BYTE_OF(dbyte)			((u8)(dbyte))
#define HIGH_BYTE_OF(dbyte)			((u8)((dbyte)>>8))

#define	HEX_TO_ASCII(x)				((x<=0x09)?(x+0x30):(x+0x37))

#define ASCII_TO_HEX(c)				((c >='0' && c <='9') ? (c-'0'): (\
										(c>='A'&&c<='F') ? (c-'A'+10): (\
												(c>='a'&&c<='f')?(c-'a'+10):0\
											)\
										)\
									)

#define U8_TO_ASCII_H(x)			HEX_TO_ASCII(((x)&0x0F))
#define U8_TO_ASCII_L(x)			HEX_TO_ASCII(((x)&0x0F))
#define BIT_N(byte, n)				(((byte)>>(n))&0x01)

#define isdigit(c)	((unsigned) ((c)-'0') < 10)

#define	NELEM(array)	(sizeof(array)/sizeof(array[0]))
#define	NOT_FOUND	-1

#define SWAP_NUM(a, b)	do{\
							if((a) ^ (b)) {\
								(a) = (a)^(b);\
								(b) = (a)^(b);\
								(a) = (a)^(b);\
							}\
						}while(0)

#define Error(Str)        FatalError(Str)
#define FatalError(Str)   fprintf(stderr, "%s\n", Str), exit(1)

#endif//__BASEDEF_H__
