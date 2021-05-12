
#ifndef _NEW104DEF_H
#define _NEW104DEF_H

#include "publichead.h"
#include "database.h"

//define EV
#define SCHEDULEFLAG    0x40  //调度标志
#define LINKINITOKFLAG  0x80  //链路层初始化成功标志　

#define APCILEN     6
#define APDULEN    255
#define ASDULEN    249
#define K          12
#define W          8

#define STARTCODE  0x68		//起始码

#define M_DP_NA_ALLDBI  240     //广东要求的全双点遥信发送，特殊处理
#define	M_SP_NA		1 	//不带时标的单点信息
#define	M_SP_TA		2 	//带时标的单点信息
#define	M_DP_NA		3 	//不带时标的双点信息
#define	M_ST_NA		5 	//步位置信息
#define	M_BO_NA		7 	//子站远动终端状态
#define	M_ME_NA		9 	//测量值规一化值
#define	M_ME_NB		11	//标度化值
#define	M_ME_NC         13       //测量值，短浮点数
#define	M_IT_NA		15 	//电能脉冲记数量
#define	M_IT_TA		16 	//带时标的电能脉冲记数量
#define	M_PS_NA		20 	//具有状态变位检出的成组单点信息
#define	M_ME_ND		21 	//不带品质描述的测量值
#define	M_BD_NA		232 //BCD码值

#define M_SP_TB   30  //带长时标的单点信息
#define M_DP_TB   31  //带长时标的双点信息
#define M_ST_TB   32  //带长时标的步位置信息
#define M_BO_TB   33  //带长时标的32位位串
#define M_ME_TD   34  //带长时标的测量值

#define	M_EI_NA		70 	//初始化结束
#define C_SC_NA     45       //单点遥控命令
#define	C_DC_NA		46 	//双点遥控命令
#define	C_RC_NA		47 	//升降命令
#define	C_SE_NA		48 	//设定命令

#define	C_IC_NA		100 	//召唤命令
#define	C_CI_NA		101 	//电能脉冲召唤命令
#define	C_RD_NA		102 	//读数据命令
#define	C_CS_NA		103 	//时钟同步命令
#define	C_RP_NA		105 	//复位进程命令
#define C_TS_TA     107   //带时标的测试命令

#define	P_ME_NA		110 	//装载参数命令
#define P_ME_NB		111	//
#define	P_AC_NA		113 	//激活参数

#define	P_RS_NA_1_GX    108       //广西主站召唤参数
#define	P_ME_NA_1_GX    112       //广西参数预置或参数上送，远程参数预置及读取参数后上送主站
#define	P_AC_NA_1_GX    113       //广西激活参数

#define	F_FR_NA		120 	//文件已准备好
#define	F_SR_NA		121 	//节已准备好
#define	F_SC_NA		122 	//召唤目录﹑选择文件﹑召唤文件﹑召唤节
#define	F_LS_NA		123 	//最后的节﹑最后的段
#define	F_AF_NA		124 	//确认文件﹑确认节
#define	F_SG_NA		125 	//段
#define	F_DR_NA		126 	//目录
#define	C_PS_NA		138 	//规约切换

#define F_FY_NA   141   //文件传输

#define M_FA_TB   142   //故障处理结果信息

#define M_OT_NA   143   //透明数据传输

/***********广东远程参数定义*****************/
#define GD_MUTIPARA_READ    108     //读参数命令,短浮点数，同广西功能码
#define GD_MUTIPARA_WRITE   55      //预置/激活参数命令,短浮    

/***********2016新规约定义*****************/
#define M_FT_NA  42     //故障值信息
#define M_IT_NB  206
#define M_IT_TC  207

#define C_SR_NA  200    //切换定值区
#define C_RR_NA  201    //读定值区号
#define C_RS_NA  202    //读参数和定值
#define C_WS_NA  203    //写参数和定值
#define F_FR_NA_N  210    //文件传输(与原来的F_FR_NA区别）
#define F_SR_NA_N  211    //软件升级
#define F_FS_NA_N  212    //线损模块2018标准，文件同步；cl 20180314

#define PARA_DATA_TYPE_WORD   45  //无符号短整形
#define PARA_DATA_TYPE_FLOAT  38  //短浮点数

//文件传输操作标示定义
#define FR_RD_DIR             1
#define FR_RD_DIR_CON         2
#define FR_RD_FILE_ACT        3
#define FR_RD_FILE_ACTCON     4
#define FR_RD_FILE_DATA       5
#define FR_RD_FILE_DATACON    6
#define FR_WR_FILE_ACT        7
#define FR_WR_FILE_ACTCON     8
#define FR_WR_FILE_DATA       9
#define FR_WR_FILE_DATACON    10

/***********2016新规约定义 end*****************/

//传送原因：
#define	PERCYC	      1 	//周期/循环
#define	BACK		  2 	//背景扫描
#define	SPONT		  3 	//突发
#define	INIT_101  4 	//初始化
#define	REQ		    5 	//请求或被请求
#define	ACT		    6 	//激活
#define	ACTCON		7 	//激活确认
#define	DEACT		  8 	//停止激活
#define	DEACTCON	9 	//停止激活确认
#define	ACTTERM		10 	//激活结束
#define	RETREM		11 	//远程命令引起的返送信息
#define	RETLOC		12 	//当地命令引起的返送信息
#define	FILE_101	13 	//文件传送
#define	INTROGEN	20 	//响应总召唤
#define	INTRO1		21 	//响应第1组召唤
#define	INTRO2		22 	//响应第2组召唤
#define	INTRO3		23 	//响应第3组召唤
#define	INTRO4		24 	//响应第4组召唤
#define	INTRO5		25 	//响应第5组召唤
#define	INTRO6		26 	//响应第6组召唤
#define	INTRO7		27 	//响应第7组召唤
#define	INTRO8		28 	//响应第8组召唤
#define	INTRO9		29 	//响应第9组召唤
#define	INTRO10		30  //响应第10组召唤
#define	INTRO11		31 	//响应第11组召唤
#define	INTRO12		32 	//响应第12组召唤
#define	INTRO13		33 	//响应第13组召唤
#define	INTRO14		34 	//响应第14组召唤
#define	INTRO15		35 	//响应第15组召唤
#define	INTRO16		36  //响应第16组召唤
#define	REQCOGCN	37  //响应计数量总召唤
#define	REQCO1		38 	//响应第1组计数量召唤
#define	REQCO2		39 	//响应第2组计数量召唤
#define	REQCO3		40 	//响应第3组计数量召唤
#define	REQCO4		41 	//响应第4组计数量召唤
#define UNKNOWNTYPEID   44       //未知的类型标识
#define UNKNOWNCOT      45       //未知的传送原因
#define UNKNOWNPUBADDR  46       //未知的应用服务数据单元公共地址
#define UNKNOWNTINFOADDR   47      //未知的信息对象地址
#define COT_YKRYBERR     48         //软压板错误
#define COT_YKSJCERR     49         //时间戳错误 遥控执行
#define COT_YKYQSBERR    50        //验签错误 遥控执行



#define	QRPRESET	1	//进程总复位
#define	QRPSOEIND	2	//复位事件缓冲区等待处理的带时标的信息
#define QRPCOLD   128 //冷复位

#define  SUMMONBCRALL  5
#define  FREEZENORESET 0x40//1
#define  FREEZERESET   0x80//2

#define LBI       0x0001
#define HBI       0x1000
#define LDBI      0x0001         //wjr双点遥信
#define HDBI      0x1000
#define LAI       0x4001
#define HAI       0x5000
#define LBO       0x6001
#define HBO       0x6200
#define LBCR      0x6401
#define HBCR      0x6600
#define LSPI      0x6601
#define HSPI      0x6700
#define LBCD      0x6701
#define HBCD      0x6800
#define RTUSTATUS 0x6801

#define RMTP_ORG_L      0X8001
#define RMTP_ORG_H      0X800A      //固有参数
#define RMTP_RUN1_L     0X8020
#define RMTP_RUN1_H     0X8036 
#define RMTP_RUN2_L     0X8040
#define RMTP_RUN2_H     0X8043
#define RMTP_RUN_NUM    16
#define RMTP_ACT1_L     0X8220
#define RMTP_ACT1_H     0X822F
#define RMTP_ACT2_L     0X8240
#define RMTP_ACT2_H     0X824E
#define RMTP_ACT_NUM    32

#define BROADCASTADDR 0xFFFF


#define PRIORITY_0   0  //发送U/S帧
#define PRIORITY_1   1  //发送优先级高的帧如遥控等
#define PRIORITY_2   2  //发送一般帧

#define S_FRAME       0x01
#define U_FRAME       0x03
#define U_STARTDTACT  0x07
#define U_STARTDTCON  0x0B
#define U_STOPDTACT   0x13
#define U_STOPDTCON   0x23
#define U_TESTFRACT   0x43
#define U_TESTFRCON   0x83

//For VSQ
#define VSQ_NUM     0x7F
#define VSQ_SQ      0x80

//For COT
#define COT_REASON 0x3F
#define COT_PONO   0x40
#define COT_TEST   0x80

#undef COTByte
#undef PUBADDRBYTE
#define INFOADDR3BYTE

#define P101_BL 0x10  //是否被封锁
#define P101_SB 0x20  //是否被取代
#define P101_NT 0x40  //是否为当前值
#define P101_IV 0x80  //是否有效

#define SPI 0x01


#define DPI 0x03

#define VTI_VALUE 0x7F
#define VTI_T     0x80

#define P101_OV 0x01

#define BCR_SQ 0x1F
#define BCR_CY 0x20
#define BCR_CA 0x40
#define BCR_IV 0x80

#define QCC_RQT 0x3F
#define QCC_FRZ 0xC0


#define OTH_BH      0x01  //保护信息

#define OTHCON_END  0x80

//File

#define MAXFILETRANLEN 240

//Command
#define FILE_READINIT   0
#define FILE_READ       1
#define FILE_WRITEINIT  2
#define FILE_WRITE      3
#define FILE_DELETE     4
#define FILE_RENAME     5

//Type
#define TYPE_FILE       0
#define TYPE_DIR        0x80

//Status
#define STA_OK          0
#define STA_END         1
#define STA_INEXIST     2
#define STA_BUSY        3
#define STA_CHECKERR    4
#define STA_ERROR       5

#define QFT_CMD 0x7F
#define QFT_TYP 0x80

#define SOF_STATUS 0x1F
#define SOF_RES    0x20
#define SOF_FOR    0x40
#define SOF_FA     0x80


#define COMMCONNECT   0x0001
#define RX_FRAMETAIL  0x8000

#define DCO_DCS 0x03
#define DCO_QU  0x7C
#define DCO_SE  0x80

#define SCO_SCS 0x01
#define SCO_QU  0x7C
#define SCO_SE  0x80

#define LASTSEG 3
#define LASTSECT 1

struct PAPCI
{
    INT8U StartCode;
    INT8U Length;
    INT16U NS;
    INT16U NR;
};

#define RP_PI_SE    0x80    //1-预置  0-执行
#define RP_PI_CR    0x40    //1-取消预置  
#define RP_PI_CONT  0x01    //1-有后续 0-无后续


#define ALLDATATIMER 30 //30分钟一次全数据
#define COUNTERTIMER 60 //60分钟一次电度量
#define CLOCKTIMER   5
#define LIMITTIMER   5

#define SCANDATA2TIMER   3  //s

//t2<t1  t3>t1
#define T0              30
#define T1              15   //No Ack CloseTCP
#define T2              5    //Send S
#define T3              30   //Send Test U
#define MINT0           30
#define MINT1           3   //15   //No Ack CloseTCP
#define MINT2           1    //Send S
#define MINT3           10   //Send Test U


struct PSec104Pad
{
    UINT16 DriverID;
    UINT16 ControlPermit;       //遥控允许 1-允许，0-不允许 缺省为1
    UINT16 SetTimePermit;  //对钟允许 1-允许，0-不允许 缺省为1
    UINT16 SendAllDataOnTime;  //定时发送全数据 1-允许，0-不允许 缺省为1
    UINT16 SendCountOnTime;//定时发送电度  1-允许，0-不允许 缺省为1
    UINT16 SendCountWithReset;//发送电度时带复位，1-带复位 0-不带复位 缺省为0
    UINT16 UseStandClock;//使用标准时钟格式 1-标准 0-非标准 缺省为1
    UINT16 AllDataInternal;//定时发送全数据间隔（分） 缺省30
    UINT16 ScanData2;//二级数据扫描间隔（秒） 缺省3
    UINT16 CountInternal;//定时发送电度间隔（分） 缺省60
    UINT16 TickValue[4];//TickValue[0]无用
                        //TickValue[1]确认无回答时间间隔（秒） 缺省15
                        //TickValue[2]发送确认帧时间间隔（秒） 缺省5
                        //TickValue[3]发送测试帧时间间隔（秒） 缺省30
    UINT16 AIDeadValue;//遥测死区值（千分比） 缺省3
    UINT16 SBIType;//单点遥信类型标识 缺省 不带时标的单点信息
    UINT16 AIType;//遥测类型标识 缺省 不带品质描述的测量值
    
    UINT16 YCNum;   /*实时遥测数量，缺省0xFFFF；2007年11月1日因北京项目增加*/
    UINT16  LBIinfoaddr;  //单点遥信信息体地址       2008.11.5
    UINT16  LDBIinfoaddr; //双点遥信信息体地址       2008.11.5
    
    UINT16  control;      //标志位
                          /*D0 是否支持北京模式遥控命令加密处理。0-支持 1-不支持 缺省为1
                            D1 支持"国网规约扩展"-2015版   1-勾选 0-不勾选 默认1
                            D2 
                            D3 
                            D4
                            D5
                            D6
                          */
    UINT16  EncryptTimeout; //通讯加密时间戳超时时间
    
};
//从站方规约面板control控制位定义
#define CON_ENCRYPT         0x0001      //单向身份认证加密方案
#define CON_104GYKZ         0x0002      //支持国网104规约扩展-2015版 1-勾选 0-不勾选 默认1
#define CON_1161ENCRPTY     0x0004      //双向身份认证加密方案（1161加密芯片）
#define CON_1120ENCRPTY     0x0008      //湖南农网加密方案（1120加密芯片）
#define CON_OLD1120ENCRPTY  0x0010      //老湖南农网加密通讯                        占位(暂时不用）
#define CON_CLEARRSNO       0x0020      //1-收到start命令，清收发序号               占位(暂时不用）
#define CON_NOJUDGERSNO     0x0040      //1-不判收发序号                            占位(暂时不用）
#define CON_SENDCOS         0x0080      //1-在勾选2015扩展版时，还需要发送cos，     占位(暂时不用）
#define CON_RSTSEND_INITEND  0x0100      //1-复位后才发送初始化结束帧  
#define CON_FREVENT_2       0x0200      //1-故障事件（信息体地址）2字节 0-3字节
#define CON_ALLDBI_104      0x0400      //遥信以双点遥信发送-广东佛山

struct PPri104Pad
{
    UINT16 DriverID;
    UINT16 ControlPermit;       //遥控允许 1-允许，0-不允许 缺省为1
    UINT16 SetTimePermit;  //对钟允许 1-允许，0-不允许 缺省为1
    UINT16 CallAllDataOnTime;  //定时召唤全数据 1-允许，0-不允许 缺省为1
    UINT16 CallCountOnTime;//定时召唤电度  1-允许，0-不允许 缺省为1
    UINT16 CallCountWithReset;//召唤电度时带复位，1-带复位 0-不带复位 缺省为0
    UINT16 UseStandClock;//使用标准时钟格式 1-标准 0-非标准 缺省为1
    UINT16 AllDataInternal;//定时召唤全数据间隔（分） 缺省30
    UINT16 SetTimeInternal;//定时对钟间隔（分） 缺省5
    UINT16 CountInternal;//定时召唤电度间隔（分） 缺省60
    UINT16 TickValue[4];//TickValue[0]无用
                        //TickValue[1]确认无回答时间间隔（秒） 缺省15
                        //TickValue[2]发送确认帧时间间隔（秒） 缺省5
                        //TickValue[3]发送测试帧时间间隔（秒） 缺省30
    UINT16 Control;//标志位    
                     //[D0]是否召唤录波
    UINT16 DTUBasedFdNo;//召唤录波时级联DTU的基础馈线号 默认是0 表示不启用级联录波  不是0表示启用级联的DTU
    UINT16 Rsv[30];
};

#define CON_CALLLBDATA         0x0001      //支持召唤录波  山东高标准   20180725


#ifdef	__cplusplus
extern "C"{
#endif

void SetProtocalErr(void);

#ifdef	__cplusplus
}
#endif

#endif



