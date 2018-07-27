#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>

typedef unsigned    char            u8;
typedef unsigned    short           u16;
typedef unsigned    int             u32;

#pragma pack(push)
#pragma pack(1)


typedef  struct  {

    u8  buf[25];
    u32  len;
}  a;
#pragma pack(pop)

void readdata(u8* buf,  u32* len)
{
    u8* p = (u8*)len-10;
        int j = 0;
    
    for(j=0;j<20;j++) {
        printf("%p, %02X\n", &p[j], p[j]);
    }
    printf("[%s][%d]%p , %u,  %08X\n",  __FUNCTION__, __LINE__, len, *len, *len);   
    
    u32 i;
    memcpy(&i, len, sizeof(i));
    printf("%08X, %u", i, i);
}

int main(void)
{
    a i;
    i.len = 0xabcd1234;
    u8* p = (u8*)&i.len-10;
    int j = 0;
    
    for(j=0;j<20;j++) {
        printf("%p, %02X\n", &p[j], p[j]);
    }
    
   
    printf("%p , %u,  %08X\n",  &i.len,  i.len, i.len); 
    readdata(i.buf, &i.len);

    return (0);
}