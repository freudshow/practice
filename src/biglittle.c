#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef int s32;
typedef signed long long s64;
typedef float fp32;
typedef double fp64;

typedef struct {
	u16 rate:15;				//通信速率
	u16 unit:1;					//速率单位标识
} rate_t;

typedef struct {
	u8 rate:7;					//通信速率
	u8 unit:1;					//速率单位标识
} unit_t;

#pragma pack(push)
#pragma pack(1)
typedef struct   
{  
   char a:2;  
   double i;  
   int c:4;  
}Node_bit;

typedef struct {
	struct {
		u8 routeId:1;				//路由标识
		u8 auxNodeId:1;				//附属模块标识
		u8 modelId:1;				//通信模块标识
		u8 conflictDetect:1;		//冲突检测
		u8 relayLevel:4;			//中继级别
	} commu;
	struct {
		u8 channelId:4;				//信道标识
		u8 errCorrectionId:4;		//纠错编码标识
	} chn;
	u8 expLen;						//预计应答字节数
	struct {
		u16 rate:15;				//通信速率
		u16 unit:1;					//速率单位标识
	} rate;
	u8 seq;							//报文序号
}qgdw3762infoDown_t;

typedef struct {
	struct {
		u8 routeId:1;				//路由标识
		u8 b1:1;					//常为0
		u8 modelId:1;				//通信模块标识
		u8 bit3:1;					//常为0
		u8 relayLevel:4;			//中继级别
	} commu;
	struct {
		u8 channelId:4;				//信道标识
		u8 bit528:4;				//5 to 8, 常为0
	} chn;
	struct {
		u8 phaseId:4;				//实测相线标识
		u8 meterRoute:4;			//电表通道特征
	} phase;
	struct {
		u8 cmd:4;					//末级命令信号品质
		u8 asw:4;					//末级应答信号品质
	} quality;
	struct {
		u8 rsv:7;					//保留
		u8 flag:1;					//事件标志
	} event;
	u8 seq;							//报文序号
}qgdw3762infoUp_t;

#pragma pack(pop)

typedef struct   
{  
   char a;   //8 bytes, align to double
   double i; //8 bytes 
   int c;  	 //8 bytes, align to double
}Node;



#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)

int main()
{
	rate_t r;
	r.unit=1;
	r.rate=0;
	unit_t u;
	u.unit=1;
	u.rate=0;
	printf("%04X\n", r);
	printf("%02X\n", u);
	
	Node node;
	printf("%d\n", sizeof(node));
	printf("a: %p, i: %p, c: %p\n", offsetof(Node, a), offsetof(Node, i), offsetof(Node, c));
	
	Node_bit nodet;
	printf("%d\n", sizeof(nodet));

	qgdw3762infoUp_t up;
	printf("%d\n", sizeof(up));
	printf("%d\n", sizeof(qgdw3762infoDown_t));

	exit(0);
}