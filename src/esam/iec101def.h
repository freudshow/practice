/*--------------- DONGFANG ELECTRONICS GROUP LTD.------------------*/
/*              MODULE:    101 Protocol Define head file           */
/*              SUBSYSTEM: DF9200 Software System.                 */
/*              AUTHOR:    MaJunHua                                */
/*              DATA:      1999.7                                  */
/*-----------------------------------------------------------------*/

#ifndef _IEC101DEF_H
#define _IEC101DEF_H

#include "publichead.h"
#include "database.h"

#ifdef	__cplusplus
extern "C"{
#endif


#define ASDULEN         250
#define LPDUSIZE        (255+6) //链路层发送报文的最大长度
#define FRAMEBUFSIZE    512     //链路层接收缓冲区的最大长度
#define NORETRYNUM      0
#define MINRETRYNUM     1
#define	MAXRETRYNUM     3       //重发的最大次数
#define APDUSIZE        255     //应用层收发缓冲区的最大长度
#define MAXFTUNUM       60      //每条线上最多的FTU数

//链路层始发站命令定义
#define	DLRESETRDL      0       //复位远方链路
#define DLRESETUSE      1       //复位用户过程
#define DLTESTDL        2       //测试链路功能
#define DLSENDCON       3       //发送确认命令
#define DLSENDNOCON     4       //发送不确认命令
#define DLREQACD        8       //请求响应确定访问状态
#define DLREQSTATUS     9       //召唤链路状态
#define DLREQDATA1      10       //召唤1级用户数据
#define DLREQDATA2      11       //召唤2级用户数据
#define MAINTSWITCH     12       //切换到维护规约。

//链路层从动站命令定义
#define DLCON           0       //确认
#define	DLNOCON         1       //否定确认
#define DLRESDATA       8       //以数据响应请求帧
#define DLNODATA        9       //无所召唤的数据
#define DLSTATUSOK      11       //响应链路状态或回答请求帧
#define DLNOWORK        14       //链路未工作
#define DLNOFIN         15       //链路未完成

//链路层到应用层的命令(INT16U)DLCommand内容
#define DL_RESETUSE     1       //复位用户进程
#define DL_TESTDLINK    2       //测试 链路
#define	DL_SENDCON      3       //链路收到03命令
#define	DL_SENDNOCON    4       //链路收到04命令
#define	DL_LINKDISABLE  5       //链路无效
#define	DL_LINKENABLE   6       //链路有效,链路收到0（确认）向应用层发此命令
#define	DL_NODATA       7       //链路层收到无数据应答
#define	DL_SENDDATA     8       //链路收到08命令
#define	DL_SCAN1S       9       //平衡模式链路例行查询应用层任务（YK、定时全数据、定时对钟、定时电度）
#define DL_CALLDATA1    10       //取一级数据
#define DL_CALLDATA2    11       //取二级数据
#define DL_CALLUDATA    12       //取紧急数据，平衡模式子站主动发送一级数据（COS、SOE）用
#define DL_CALLDBMSG    13       //子站平衡模式用于取数据库消息，以便及时处理遥控等信息。
#define	DL_APPCON       14       //收到对03命令的确认,子站平衡式
#define DL_RESETDL      15       //收到复位链路命令，应用层确定是否发初始化结束。子站非平衡。
#define DL_SECLINKENABLE 16     //收到从动复位命令，通知应用层从动链路有效

#define	DL_REC_ACD      0x8000   //该位置1链路接收到的ACD=1，否则为0
#define	DL_FCBOK        0x4000   //FCB反转正确,=1正确。

//应用层到链路层的命令(INT16U)AppCommand
#define	APP_APPCON      0       //应用层对收到的数据做应答
#define APP_RESETUSE    1       //复位用户进程
#define APP_TESTDLINK   2       //测试 链路
#define	APP_SENDCON     3       //发送03命令
#define	APP_SENDNOCON   4       //发送04命令
#define	APP_SENDDATA    8       //发送08命令
#define APP_NODATA      9       //应用层无数据，子站非平衡
#define	APP_CALLDATA1   10       //请求一级数据
#define	APP_CALLDATA2   11       //请求二级数据
#define	APP_ALLDEVNOUSE 12       //应用层所有设备不可用
#define	APP_NOJOB       13       //应用层无任务//主站平衡模式有效
#define	APP_APPDIABALE  14       //应用层无效
#define APP_LINKINIT    15       //发起链路初始化
#define APP_RESETMACHINE 16

#define	APP_HAVEDATA1   0x8000   //该位置1说明应用层有一级数据，否则为0;子站非平衡、平衡模式总召唤、对钟

//链路层帧内容
#define	STARTCODE10     0x10       //起始码
#define STARTCODE68     0x68       //起始码
#define ENDCODE         0x16       //停止码
#define	SINGLECON       0xE5       //单字节无数据确认应答。
#define MAINTSTARTCODE  'I'

#define M_DP_NA_ALLDBI     240     //广东要求的全双点遥信发送，特殊处理
#define	M_SP_NA         1       //不带时标的单点信息
#define	M_SP_TA         2       //带时标的单点信息
#define	M_DP_NA         3       //不带时标的双点信息
#define	M_DP_TA         4       //带时标的双点信息
#define	M_ST_NA         5       //步位置信息
#define	M_ST_TA         6       //带时标的步位置信息
#define	M_BO_NA         7       //子站远动终端状态
#define	M_ME_NA         9       //测量值,规一化值
#define	M_ME_TA         10       //带时标的测量值，规一化值
#define	M_ME_NB         11       //测量值，标度化值——新标准中，我们目前应使用的类型。替代原来的M_ME_NA
#define	M_ME_TB         12       //带时标的测量值,标度化值
#define	M_ME_NC         13       //测量值，短浮点数
#define	M_ME_TC         14       //带时标测量值，短浮点数
#define	M_IT_NA         15       //电能脉冲记数量
#define	M_IT_TA         16       //带时标的电能脉冲记数量
#define	M_EP_TA         17       //带时标的继电保护或重合闸设备单个事件
#define	M_EP_TB         18       //带时标的继电保护装置成组启动事件
#define	M_EP_TC         19       //带时标的继电保护装置成组输出电路信息事件
#define	M_PS_NA         20       //具有状态变位检出的成组单点信息
#define	M_ME_ND         21       //不带品质描述的测量值，规一化值
#define	M_BD_NA         232      //BCD码值

#define	M_EI_NA         70       //初始化结束

//DL/T634.5101-2002标准新增类型标识
#define M_SP_TB         30       //带CP56Time2a时标的单点信息
#define M_DP_TB         31       //带CP56Time2a时标的双点信息
#define M_ST_TB         32       //带CP56Time2a时标的步位置信息
#define M_BO_TB         33       //带CP56Time2a时标的32位比特串
#define M_ME_TD         34       //带CP56Time2a时标的测量值，规一化值
#define M_ME_TE         35       //带CP56Time2a时标的测量值，标度化值
#define M_ME_TF         36       //带CP56Time2a时标的测量值，短浮点数
#define M_IT_TB         37       //带CP56Time2a时标的累计量
#define M_EP_TD         38       //带CP56Time2a时标的继电保护设备事件
#define M_EP_TE         39       //带CP56Time2a时标的继电保护设备成组启动事件
#define M_EP_TF         40       //带CP56Time2a时标的继电保护设备成组输出电路信息
#define	M_EI_NA         70       //初始化结束帧

#define C_SC_NA         45       //单点遥控命令
#define	M_CD_NA         106       //延时获得命令
#define	M_CS_NA         103       //时钟同步命令
//END

#define	C_DC_NA         46       //双点遥控命令
#define	C_RC_NA         47       //升降命令
#define	C_SE_NA         48       //设定命令
#define	C_BO_NA         51       //32位的位串

#define	C_IC_NA         100       //召唤命令
#define	C_CI_NA         101       //电能脉冲召唤命令
#define	C_RD_NA         102       //读数据命令
#define	C_CS_NA         103       //时钟同步命令
#define	C_TS_NA         104       //测试命令
#define	C_RP_NA         105       //复位进程命令
#define	C_CD_NA         106       //延时获得命令

#define	P_ME_NA         110       //装载参数命令，规一化值
#define	P_ME_NB         111       //装载参数命令，标度化值
#define	P_ME_NC         112       //装载参数命令，短浮点数
#define	P_AC_NA         113       //激活参数

#define	P_RS_NA_1_GX    108       //广西主站召唤参数
#define	P_ME_NA_1_GX    112       //广西参数预置或参数上送，远程参数预置及读取参数后上送主站
#define	P_AC_NA_1_GX    113       //广西激活参数

#define	F_FR_NA         120       //文件已准备好
#define	F_SR_NA         121       //节已准备好
#define	F_SC_NA         122       //召唤目录﹑选择文件﹑召唤文件﹑召唤节
#define	F_LS_NA         123       //最后的节﹑最后的段
#define	F_AF_NA         124       //确认文件﹑确认节
#define	F_SG_NA         125       //段
#define	F_DR_NA         126       //目录

#define F_FY_NA         141       //文件传输

#define M_FA_TB         142       //故障处理结果信息

#define C_PF_NA         143       //保护定值设定命令，传送原因6为设定，
#define P_PF_NA         144       //保护定值召唤命令，传送原因5为请求

#define M_PF_NA         145       //保护定值发送命令，传送原因5为被请求

//#define C_IT_TC         146       //召唤历史电度  主站COT=6 激活 从站COT=7激活确认 COT=10停止

//#define M_IT_TC         147       //历史电度数据  COT=5被请求

#define E_SGC_F0       0xF0//湖南农网加密TI
#define E_SGC_F1       0xF1
#define E_SGC_F2       0xF2
#define E_SGC_F3       0xF3
#define E_SGC_F4       0xF4
#define E_SGC_F5       0xF5
#define E_SGC_F6       0xF6
#define E_SGC_FA       0xFA

/***********广东远程参数定义*****************/
#define GD_MUTIPARA_READ    108     //读参数命令,短浮点数，同广西功能码
#define GD_MUTIPARA_WRITE   55      //预置/激活参数命令,短浮  


/***********2016新规约定义*****************/
#define M_FT_NA  42     //故障值信息
#define M_IT_NB  206
#define M_IT_TC  207

#define C_SR_NA     200    //切换定值区
#define C_RR_NA     201    //读定值区号
#define C_RS_NA     202    //读参数和定值
#define C_WS_NA     203    //写参数和定值
#define F_FR_NA_N   210    //文件传输
#define F_SR_NA_N   211    //软件升级
#define F_FS_NA_N   212    //线损模块2018标准，文件同步；cl 20180314

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

//传送原因：
#define	PERCYC          1       //周期/循环
#define	BACK            2       //背景扫描
#define	SPONT           3       //突发
#define	INIT_101        4       //初始化
#define	REQ             5       //请求或被请求
#define	ACT             6       //激活
#define	ACTCON		    7       //激活确认
#define	DEACT           8       //停止激活
#define	DEACTCON        9       //停止激活确认
#define	ACTTERM         10       //激活结束
#define	RETREM          11       //远程命令引起的返送信息
#define	RETLOC          12       //当地命令引起的返送信息
#define	FILE_101        13       //文件传送
#define	INTROGEN        20       //响应总召唤
#define	INTRO1          21       //响应第1组召唤
#define	INTRO2          22       //响应第2组召唤
#define	INTRO3          23       //响应第3组召唤
#define	INTRO4          24       //响应第4组召唤
#define	INTRO5          25       //响应第5组召唤
#define	INTRO6          26       //响应第6组召唤
#define	INTRO7          27       //响应第7组召唤
#define	INTRO8          28       //响应第8组召唤
#define	INTRO9          29       //响应第9组召唤
#define	INTRO10         30       //响应第10组召唤
#define	INTRO11         31       //响应第11组召唤
#define	INTRO12         32       //响应第12组召唤
#define	INTRO13         33       //响应第13组召唤
#define	INTRO14         34       //响应第14组召唤
#define	INTRO15         35       //响应第15组召唤
#define	INTRO16         36       //响应第16组召唤
#define	REQCOGCN        37       //响应计数量总召唤
#define	REQCO1          38       //响应第1组计数量召唤
#define	REQCO2          39       //响应第2组计数量召唤
#define	REQCO3          40       //响应第3组计数量召唤
#define	REQCO4          41       //响应第4组计数量召唤
#define UNKNOWNTYPEID   44       //未知的类型标识
#define UNKNOWNCOT      45       //未知的传送原因
#define UNKNOWNPUBADDR  46       //未知的应用服务数据单元公共地址
#define UNKNOWNTINFOADDR   47      //未知的信息对象地址
#define COT_YKRYBERR     48         //软压板错误 0x30
#define COT_YKSJCERR     49         //时间戳错误 遥控执行
#define COT_YKYQSBERR    50        //验签错误 遥控执行

/*信息体起始地址定义*/
#define LBI       0x0001
#define HBI       0x1000
#define LDBI      0x0001         //wjr双点遥信
#define HDBI      0x1000
#define LAI       0x4001
#define HAI       0x5000
#define LPARA	  0x5001    //遥测参数
#define HPARA	  0x6000
#define LBO       0x6001
#define HBO       0x6200
#define	LSET	  0x6201
#define	HSET	  0x6400
#define LBCR      0x6401
#define HBCR      0x6600
#define LSPI      0x6601
#define HSPI      0x6700
#define LBCD      0x6701//..DL/T634.5101-2002标准中无BCD码（水位）的信息对象地址的定义
#define HBCD      0x6800
#define RTUSTATUS 0x6801
#define LFILE	  0x6802
#define HFILE	  0x7000

#define RMTP_ORG_L      0X8001
#define RMTP_ORG_H      0X800A      //固有参数
#define RMTP_RUN1_L     0X8020
#define RMTP_RUN1_H     0X8034
#define RMTP_RUN2_L     0X8040
#define RMTP_RUN2_H     0X8043
#define RMTP_RUN_NUM    16
#define RMTP_ACT1_L     0X8220
#define RMTP_ACT1_H     0X822F
#define RMTP_ACT2_L     0X8240
#define RMTP_ACT2_H     0X824E
#define RMTP_ACT_NUM    32

#define MAXBINUM   0x1000
#define GROUPBINUM 0x200
#define MAXAINUM   0x1000
#define GROUPAINUM 0x400
#define MAXSPINUM  0x100
#define MAXBONUM   0x200
#define MAXBCRNUM  0x200
#define GROUPBCRNUM 0x80


#define	SQSINGLE    0
#define	SQSEQUENCE  1
#define	TESTREASON  1	//1=正在试验 0=未试验

#define	NEGAPPROV   1	//否定认可
#define	POSAPPROV   0	//肯定认可

#define	OFF	0 	//开
#define	ON	1 	//合

#define	GATEVAL		1 	//门限值
#define	FIWAVE		2	  //滤波系数
#define	MELOWER		3	  //测量值的下限值
#define	MEUPPER		4	  //测量值的上限值

#define	LPCCHANGE	  1	//当地参数未改变
#define	LPCNOCHANGE	0	//当地参数改变

#define	POPMOVE		0	//参数在运行
#define	POPNOMOVE	1	//参数未运行

#define	QRPRESET	1	//进程总复位
#define	QRPSOEIND	2	//复位事件缓冲区等待处理的带时标的信息
#define QRPCOLD   128 //冷复位

//EV
#define DLFLAG          0x00000020
#define APPFLAG         0x00000040
#define APPTIMEFLAG     0x00000080
#define SCHEDULE        0x00000100
#define NEXTFRAME       0x00000200
#define SENDOVER        0x00000400
#define SAVEKWHFLAG     0x00000800 //定时保存电度事项
#define FORCESCHEDULE   0x00010000  //强制进行一次应用层扫描

#define  SUMMONBCRALL  5
#define  FREEZENORESET 0x40         //wjr
#define  FREEZERESET   0x80
#define	 READCOUNTER	0

#define COUNTERINTERVAL 60    //pre 1m

#define ALLDATAGROUPNUM  16
#define COUNTERGROUPNUM 4

#define TIMEOUT   1
#define TAIL_2      2
#define CRCERROR  3

/*以下链路层相关数据定义*/

/*链路层从动站的运行状态*/
enum	DLSECSTATUS
{
    SECDISABLE =0,	//从动站链路层没复位
    SECENABLE,	//从动站链路层可以使用
    SECDLSTATUS,	//
    SECDLBUSY 	//从动站链路层忙
};


/*链路层启动站的运行状态*/
enum	DLPRISTATUS
{
    PRIDISABLE =0,	//启动站链路层没复位
    PRIWAITSTATUS,	//启动站链路层等待回答链路状态(执行请求链路状态)
    PRIWAITRSCON,	//启动站链路层等待复位链路确认
    PRIWAITINITEND,	//主站等待子站初始化结束帧???
    PRIENABLE,	//启动站链路层可以使用
    PRISENDCON,	//启动站链路层等待数据确认
    PRIREQRES,	//启动站链路层等待请求响应
    PRIDLBUSY 	//启动站链路层忙
};

/*发送进程状态记忆*/
enum DLTXDSTATUS
{
    TXDSEND,	//发送报文第一帧状态
    TXDWAIT		//发送报文第一帧状态
};

/*链路层接收状态*/
enum DLRXDSTATUS
{
    RXDSTART=0,	//寻找起始标志状态
    RXDHEAD,	//寻找报文头
    RXDCONTINUE 	//接收后续报文
};

struct DLDevInfo
{
    enum DLSECSTATUS DLSecStatus; //链路层从动站状态
    enum DLPRISTATUS DLPriStatus; //链路层启动站状态
    INT8U RlaConCode;             //接收的控制码
    INT8U FCBNoTurnNum;           //FCB未翻转计数
    INT8U TlaConCode;             //发送的控制码
    INT16U DestAddr;              //目的地址——即子站链路地址
    INT16U SourceNo;              //源地址——即本机地址
    INT8U FlagData1;              //一级数据标志

    //INT8U StartDL;                //开始链路过程，平衡模式判断链路过程是否结束的标志
    //INT8U RemoteDLOK;             //复位远方链路成功，平衡模式
    //INT8U LocalDLOK;              //本地链路复位成功，平衡模式
    
    BOOL IsSendLinkInitCmd;         //是否主动（链路断和开机）发送过链路初始化命令 FC9 0-没有发送过  1-发送过，当检测到链路断后置位该标志
                                    //用于当收到对方链路复位命令后，是否发送复位命令的标志
    
    BOOL FirstRecFCB;             //来标识链路复位后第一次收到对方的带FCB位的报文，并以对方的FCB起始位为起始标志
    
    BOOL IsEncrypt;             //是否支持加密模式
    
    
};

/*链路层定义结束*/

/*以下为应用层相关定义*/

#define VSQ_SQ 0x80         //sq=1 顺序发送  SQ=0 离散发送
#define VSQ_NUM 0x7f
#define COT_REASON 0x3F
#define COT_PONO 0x40
#define COT_TEST 0x80

#define DCO_DCS 0x03
#define DCO_QU  0x7C
#define DCO_SE  0x80

#define SCO_SCS 0x01
#define SCO_QU  0x7C
#define SCO_SE  0x80

#define RCO_DCS 0x03
#define RCO_QU  0x7C
#define RCO_SE  0x80

#define QCC_RQT 0x3F
#define QCC_FRZ 0xC0

//固定帧长结构
#define P101_FUNCODE 0x0F
#define P101_FCV     0x10
#define P101_DFC     0x10
#define P101_FCB     0x20
#define P101_ACD     0x20
#define P101_PRM     0x40
#define P101_DIR     0x80

#define P101_BL 0x10  //是否被封锁
#define P101_SB 0x20  //是否被取代
#define P101_NT 0x40  //是否为当前值
#define P101_IV 0x80  //是否有效
#define P101_OV 0x01    //溢出

#define SPI 0x01
#define DPI 0x03

#define VTI_VALUE 0x7F
#define VTI_T     0x80

#define P101_OV 0x01


#define BCR_SQ 0x1F
#define BCR_CY 0x20
#define BCR_CA 0x40
#define BCR_IV 0x80

//读写参数的参数特征标示符
#define RP_PI_SE    0x80    //1-预置  0-执行
#define RP_PI_CR    0x40    //1-取消预置  
#define RP_PI_CONT  0x01    //1-有后续 0-无后续

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


struct	FILEUNIT
{
    INT8U  SendFlag;//发送标志，0未发送，ff已经发送
    INT16U  Name;
    INT32U Length;
    INT8U  Status;
    INT16U  InfoAddr;
    struct Iec101ClockTime_t Time;
};

struct  DIRUNIT
{
    INT8U FileNum;
    INT8U ReadPtr;
    INT8U WritePtr;

    struct FILEUNIT File[48];
};

#define FILELENGTH  (128*9)   //9个周波，每周波64点128字节。
#define LUBOINFOADDR 0x6901
#define LUBOFLAG 0x8000



#define ALLDATATIMER     30    //30分钟一次全数据
#define COUNTERTIMER     60    //60分钟一次电度量
#define CLOCKTIMER       5     //5分钟对钟
#define SCANDATA2TIMER   3     //二级数据扫描间隔3s



struct PPri101Pad
{
    UINT16 DriverID;
    UINT16 ControlPermit;       //遥控允许 1-允许，0-不允许 缺省为1
    UINT16 SetTimePermit;       //对钟允许 1-允许，0-不允许 缺省为1
    UINT16 BalanceMode;         //平衡模式 1-平衡模式 0-非平衡模式 缺省为0
    UINT16 CallAllDataOnTime;   //定时召唤全数据 1-允许，0-不允许 缺省为1
    UINT16 CallCountOnTime;     //定时召唤电度  1-允许，0-不允许 缺省为1
    UINT16 CallCountWithReset;  //召唤电度时带复位，1-带复位 0-不带复位 缺省为0
    UINT16 UseStandClock;       //使用标准时钟格式 1-标准 0-非标准 缺省为1
    UINT16 SetTimeBroadCast;    //对钟使用广播命令 1-广播 0-不广播 缺省0
    UINT16 MaxErrorTimes;       //最大错误次数，写通讯事项 缺省为3
    UINT16 AllDataInternal;     //定时召唤全数据间隔（分） 缺省30
    UINT16 SetTimeInternal;     //定时对钟间隔（分） 缺省5
    UINT16 CountInternal;       //定时召唤电度间隔（分） 缺省60
    UINT16 TimeOut;             //超时时间（*10ms） 缺省100

    UINT16 LINKAddrSize;//链路地址字节数 缺省为2
    UINT16 COTSize;//传送原因字节数 缺省为1
    UINT16 PUBAddrSize;//公共体地址字节数 缺省为2
    UINT16 INFOAddrSize;//信息体地址字节数 缺省为2
    
    UINT16 TimeDelay;   //对钟之前是否要进行
    UINT16  control;      //标志位
                          /*D0 平衡模式下，从站方DIR标志位是否置1标志位。0-不需要置1，1-需要置1，缺省值0
                            D1 平衡模式下，是否支持双向链路初始化，(当支持双向链路初始化时，启动方向链路建立时有防错的延时，不支持时无防错延时)。0-支持，1-不支持，缺省值0
                            D2 支持线损模块2018检测标准，默认勾选
                            D3 第二个线损模块，默认不勾选
                            D4 双点遥控，默认不勾选 
                            D5
                            D6
                            D7 支持"国网规约扩展"-2015版   1-勾选 0-不勾选 默认1
                          */
};
//主站方规约面板control控制位定义
#define PRICON_DIRBITFLAG    0x0001
#define PRICON_ISNEEDDELAY   0x0002
#define PRICON_101GYKZ       0x0080      //AJ++170810 区别总召电度时是否需要先冻结 1-不冻结 0-冻结
#define PRICON_XSBZ          0x0004      //支持线损模块2018检测标准   CL 20180525
#define PRICON_XSNUM2        0x0008      //第二个线损模块    CL 20180525  
#define PRICON_SDYK          0x0010      //双点遥控   CL20180828

struct PSec101Pad
{
    UINT16 DriverID;
    UINT16 ControlPermit;       //遥控允许 1-允许，0-不允许 缺省为1
    UINT16 SetTimePermit;  //对钟允许 1-允许，0-不允许 缺省为1
    UINT16 BalanceMode;//平衡模式 1-平衡模式 0-非平衡模式 缺省为0
    UINT16 SOEWithCP56Time;//SOE使用长时标格式 1-56位长时标 0-24位短时标  缺省为1
    UINT16 UseStandClock;//使用标准时钟格式 1-标准 0-非标准 缺省为1
    UINT16 MaxALLen; //最大应用层报文长度 缺省250
    UINT16 AIDeadValue;//遥测死区值（千分比） 缺省3
    UINT16 ScanData2;//二级数据扫描间隔（秒） 缺省3
    UINT16 TimeOut;//超时时间（*10ms） 缺省100

    UINT16 LINKAddrSize;//链路地址字节数 缺省为2
    UINT16 COTSize;//传送原因字节数 缺省为1
    UINT16 PUBAddrSize;//公共体地址字节数 缺省为2
    UINT16 INFOAddrSize;//信息体地址字节数 缺省为2
    UINT16 BackScanTime;//背景数据扫描间隔（分） 缺省20
    UINT16 CycScanTime;//周期循环数据扫描间隔（秒） 缺省20
    UINT16 TypeID[20];//类型标识 0到7缺省为1；8到11缺省为11；12为0；13为11；14为5；16缺省为15 ；其他为0
    UINT16 GroupNum[20];//每组信息数量 缺省为0。
    UINT16 HistoryDDPermit;//历史电度保存允许 1-允许 0-不允许 缺省0
    UINT16 HistoryDDTime;//历史电度保存周期（分） 缺省60
    
    
    UINT16  YCNum;        /*实时遥测数量，缺省0xFFFF；2008年3月10日因北京项目增加*/
    UINT16  LBIinfoaddr;  //单点遥信信息体地址       2008.11.5
    UINT16  LDBIinfoaddr; //双点遥信信息体地址       2008.11.5
    
    UINT16  control;      //标志位
                          /*D0 平衡模式下，从站方DIR标志位是否置1标志位。0-需要置1，1-不需要置1，缺省值0
                            D1 非平衡模式下，当无数据时，是否发送E5，0-发送  1-不发送，缺省为0
                            D2 平衡模式下，是否支持双向链路初始化，(当支持双向链路初始化时，启动方向链路建立时有防错的延时，不支持时无防错延时)。0-支持，1-不支持，缺省值0
                            D3 是否支持北京模式遥控命令加密处理。0-支持 1-不支持 缺省为1
                            D4 是否每次初始化结束都发送初始化结束帧。0-每次发送 1-复位后发送 缺省为0
                            D5 不判FCB翻转。默认判断 0-判断 1-不判断
                            D6 GPRS设置 0-不使用GPRS  1-使用GPRS
                            D7 支持"国网规约扩展"-2015版   1-勾选 0-不勾选 默认1
                          */
    UINT16  EncryptTimeout; //通讯加密时间戳超时时间
    //UINT16  HeartBeatIdleLimit; //心跳发送间隔
};

//从站方规约面板control控制位定义
#define CON_DIRBITFLAG      0x0001
#define CON_ISSENDE5        0x0002
#define CON_ISNEEDDELAY     0x0004      //0-双向（勾选状态） 1-单向
#define CON_ENCRYPT         0x0008      //通讯加密
#define CON_RSTSEND_INITEND 0x0010
#define CON_NOFCVCTRL       0x0020      //不判FCB翻转

#define CON_101GYKZ         0x0080      //支持国网101规约扩展-2015版 1-勾选 0-不勾选 默认1
#define CON_1161ENCRPTY     0x0100      //支持国网全信息加密安全防护方案(sgc1161).与CON_ENCRYPT组合使用。CON_ENCRYPT:CON_1161ENCRPTY = 1:0(无加密)  0:0(11版安全方案) 0:1(16版安全防护方案) 1:1(非法)
#define CON_1120ENCRPTY     0x0200      //支持湖南农网加密通讯
#define CON_OLD1120ENCRPTY  0x0400      //支持老湖南农网加密通讯
#define CON_101SENDCOS      0x0800      //1-在勾选2015扩展版时，还需要发送cos
#define CON_ALLDBI_101      0x1000      //遥信以双点遥信发送-广东佛山


void SetProtocalErr(void);  

#ifdef	__cplusplus
}
#endif

#endif
