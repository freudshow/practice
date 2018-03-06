/******************************************************************************
 * file:    basedef.h
 * author:  s_baoshan <s_baoshan@163.com>
 * created: 2018-02-25
 * updated: 2018-02-25
 * contents: define base data types, some useful macros.
 *****************************************************************************/
#ifndef _BASEDEF_H
#define _BASEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

/**********base data types***********/
typedef unsigned	char			u8;
typedef unsigned	short			u16;
typedef unsigned	int				u32;
typedef unsigned long long			u64;
typedef char						s8;
typedef short						s16;
typedef int							s32;
typedef signed long long			s64;
typedef float						fp32;
typedef double						fp64;

#define BOOLEAN		u8		/* bool                                                */
#define INT8U		u8		/* Unsigned  8 bit quantity                           */
#define INT8S		s8		/* Signed    8 bit quantity                           */
#define INT16U		u16		/* Unsigned 16 bit quantity                           */
#define INT16S		s16		/* Signed   16 bit quantity                           */
#define INT32U		u32		/* Unsigned 32 bit quantity                           */
#define INT32S		s32		/* Signed   32 bit quantity                           */
#define INT64U		u64		/* Unsigned 64 bit quantity                           */
#define INT64S		s64		/* Unsigned 64 bit quantity                           */
#define FP32		fp32	/* Single precision floating point                    */
#define FP64		fp64	/* Double precision floating point                    */

#define	FILE_LINE   __FILE__,__FUNCTION__,__LINE__

/**********byte macros***********/
#define HEX_TO_BCD(x) (((x)/0x0A)*0x10+((x)%0x0A))
#define BCD_TO_HEX(x) (((x)/0x10)*0x0A+((x)%0x10))

#define MERGE_TWO_BYTE(byt_h,byt_l)	(((u16)((byt_h)<<8))+byt_l)
#define LOW_BYTE_OF(dbyte)				((u8)((u16)(dbyte)))
#define HIGH_BYTE_OF(dbyte)			((u8)(((u16)(dbyte))>>8))

/*************************************************************
 * convert hex char to ascii code
 * note: 'x' must be in interval [0x00, 0x0F]
 * example: HEX_TO_ASCII(0xc), return 'C'
 * ***********************************************************/
#define HEX_TO_ASCII(x)				((x<=0x09)?(x+0x30):(x+0x37))

/*************************************************************
 * convert ascii code to hex char
 * note: 'x' must be in interval:
 * {['0', '9'] U ['a', 'f'] U ['A', 'F']}
 * * example: ASCII_TO_HEX('4'), return 0x04,
 * 			  ASCII_TO_HEX('e'), return 0x0e,
 * 			  ASCII_TO_HEX('B'), return 0x0B, etc.
 * ***********************************************************/
#define ASCII_TO_HEX(c)				((c >='0' && c <='9') ? (c-'0'): (\
											(c>='A'&&c<='F') ? (c-'A'+10): (\
													(c>='a'&&c<='f')?(c-'a'+10):0\
												)\
											)\
										)


/*************************************************************
 * convert BCD char to ascii code
 * example: BCD_TO_ASCII_H(0x56), return '5',
 * 			BCD_TO_ASCII_L(0x56), return '6'.
 * ***********************************************************/
#define BCD_TO_ASCII_H(x)			HEX_TO_ASCII((((x)>>4)&0x0F))
#define BCD_TO_ASCII_L(x)			HEX_TO_ASCII(((x)&0x0F))

/*************************************************************
 * get n binary of byte, n is 1 based.
 * example: BIT_N(0b10010010, 5), return 1,
 * 			BIT_N(0b10010010, 1), return 0.
 * ***********************************************************/
#define BIT_N(byte, n)				(((byte)>>(n))&0x01)

/*************************************************************
 * juge if a char is a digit of decimal
 * ***********************************************************/
#define isDigit(c)					((unsigned) ((c)-'0') < 10)
/*************************************************************
 * juge if a char is a digit of hexdecimal
 * ***********************************************************/
#define isHex(c) (((unsigned) ((c)-'0') < 10) || ((unsigned) ((c)-'A') < 6) || ((unsigned) ((c)-'a') < 6) )

/*************************************************************
 * juge if a char is a delimeter
 * ***********************************************************/
#define isDelim(c) (c == ' ' || c == '\n' || c == 't' || c == '\r')

/*************************************************************
 * get number of elememnts in array
 * ***********************************************************/
#define ELEM_NUM(array)			(sizeof(array)/sizeof(array[0]))

#define	NOT_FOUND	-1

/*************************************************************
 * swap two number(u8/s8, u16/s16, u32/s32).
 * note: 'a' and 'b' must be the same datatype.
 * if a == b, then this macro do not swap them.
 * ***********************************************************/
#define SWAP(a, b)		do{\
							if((a) ^ (b)) {\
								(a) = (a)^(b);\
								(b) = (a)^(b);\
								(a) = (a)^(b);\
							}\
						}while(0)

#define min(X, Y)  ((X) < (Y) ? (X) : (Y))
#define max(X, Y)  ((X) > (Y) ? (X) : (Y))

#define Error(Str)        FatalError(Str)
#define FatalError(Str)   fprintf(stderr, "%s\n", Str), exit(1)


#ifdef __cplusplus
}
#endif

#endif//__BASEDEF_H__
