#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
 
#define SINGLE_ONE_BIT 0x80     //相当于10000000b
#define BLOCK_SIZE 512
#define MOD_SIZE 448
#define APP_SIZE 64
#define BITS 8
 
// MD5 Chaining Variable
#define A 0x67452301UL     //相当于字符串0123456789ABCDEFEDCBA9876543210
#define B 0xEFCDAB89UL
#define C 0x98BADCFEUL
#define D 0x10325476UL
 
// Creating own types 创建我们的类型
#ifdef UINT64
#undef UINT64
#endif
 
#ifdef UINT32
#undef UINT32
#endif
 
typedef unsigned long long UINT64;
typedef unsigned long UINT32;
typedef unsigned char UINT8;
 
typedef struct{
     char * message;
     UINT64 length;
}STRING;
 
// g_X表示块的数索引，左边代码初始值，游标代表增加值，由于块数为16，故需要取mod
const UINT32 g_X[4][2] = {{0, 1}, {1, 5}, {5, 3}, {0, 7}};
 
// MD5变换的常数(左移位数)
const UINT32 g_S[4][4] = { { 7, 12, 17, 22 },
                         { 5, 9, 14, 20 },
                         { 4, 11, 16, 23 },
                         { 6, 10, 15, 21 }};
 
// F, G, H and I are basic MD5 functions. 四个基础函数
UINT32 F( UINT32 X, UINT32 Y, UINT32 Z ){
     return ( X & Y ) | ( ~X & Z );
}
UINT32 G( UINT32 X, UINT32 Y, UINT32 Z ){
     return ( X & Z ) | ( Y & ~Z );
}
UINT32 H( UINT32 X, UINT32 Y, UINT32 Z ){
     return X ^ Y ^ Z;
}
UINT32 I( UINT32 X, UINT32 Y, UINT32 Z ){
     return Y ^ ( X | ~Z );
}
 
//  X循环左移s位
UINT32 rotate_left( UINT32 x, UINT32 s )
{
     return ( x << s ) | ( x >> ( 32 - s ) );
}
 
// Pre-processin 预处理
// 计算需要填充0部分的字节长度
UINT32 count_padding_bits ( UINT32 length )  //需填充字节数
{
     UINT32 mod = length * BITS % BLOCK_SIZE;     //模
 
     //需要填充的位数
     UINT32 c_bits = ( MOD_SIZE + BLOCK_SIZE - mod ) % BLOCK_SIZE;
     return c_bits / BITS;
}
 
//补充消息字符串
STRING append_padding_bits ( char * msg )
{
     UINT32 msg_length = strlen ( msg );
     UINT32 bit_length = count_padding_bits ( msg_length );
     UINT64 app_length = msg_length * BITS;
 
     STRING string;
     string.message = (char *)malloc(msg_length + bit_length + APP_SIZE / BITS);
     memcpy(string.message, msg, msg_length);   // Save message
 
     // Pad out to mod 64. 填充bit_length字节个0
     memset (string.message + msg_length, 0, bit_length );
     string.message [msg_length] = SINGLE_ONE_BIT;
 
     // Append length (before padding).填充原字符串长度64位
     memmove (string.message + msg_length + bit_length, (char *)&app_length, sizeof( UINT64 ) );
     string.length = msg_length + bit_length + sizeof( UINT64 );
     return string;
}
 
int main ( int argc, char *argv[] )
{
     STRING string;
     UINT32 chain[4];      //中间结果
     UINT32 state[4];
     // state[0] - a
     // state[1] - b
     // state[2] - c
     // state[3] - d
 
     UINT8 result[16];     //结果放置
     UINT32 ( *auxi[ 4 ])( UINT32, UINT32, UINT32 ) = { F, G, H, I }; //函数指针数组
 
     if ( argc != 2 )
     {
          fprintf ( stderr, "usage: %s <src string>\n", argv[ 0 ] );
          return EXIT_FAILURE;
     }
 
     string = append_padding_bits (argv[1]);
 
     // MD5 initialization.MD5初始化
     chain[0] = A;
     chain[1] = B;
     chain[2] = C;
     chain[3] = D;
 
     UINT32 block[16];
     int wIdx;
     for (UINT64 j = 0; j < string.length; j += BLOCK_SIZE / BITS)  //主循环(每512位来一次)
     {
         // 取其中一块
         memmove ( (char *)block, string.message + j, BLOCK_SIZE / BITS );
         memmove ( state, chain, sizeof(chain) );
 
         // 4 * 16循环
         for (int roundIdx = 0; roundIdx < 4; roundIdx++ )
         {
             wIdx = g_X[ roundIdx ][ 0 ];
 
             int sIdx = 0;
             for (int i = 0; i < 16; i++ )
             {
                 // FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.根据轮数确定函数是哪个
                 // Rotation is separate from addition to prevent recomputation.
 
                 //a = b + ( a + auxi[i](b ,c , d) + mj + ti) <<< w
                 state[sIdx] = state [ (sIdx + 1) % 4 ] +
                         rotate_left ( state[sIdx] +
                                       (*auxi[roundIdx])
                                       ( state[(sIdx+1) % 4], state[(sIdx+2) % 4], state[(sIdx+3) % 4]) +
                         block[ wIdx ] + // 块内容
                         (UINT32)floor( (1ULL << 32) * fabs(sin( roundIdx * 16 + i + 1 )) ), // 常数
 
                         g_S[ roundIdx ][ i % 4 ]);// w
 
                 sIdx = ( sIdx + 3 ) % 4;
 
                 wIdx = ( wIdx + g_X[ roundIdx ][ 1 ] ) & 0xF;
             }
         }
         chain[0] += state[0];
         chain[1] += state[1];
         chain[2] += state[2];
         chain[3] += state[3];
     }
 
     // 保存最终结果到result
     memmove ( result + 0, (char *)&chain[0], sizeof(UINT32) );
     memmove ( result + 4, (char *)&chain[1], sizeof(UINT32) );
     memmove ( result + 8, (char *)&chain[2], sizeof(UINT32) );
     memmove ( result + 12, (char *)&chain[3], sizeof(UINT32) );
 
     for (int i = 0; i < 16; i++ )
          printf ( "%02x", result[i] );
     putchar ( '\n' );
 
     return EXIT_SUCCESS;
}