

#ifndef _NEW104SEC_H
#define _NEW104SEC_H

#include "new104def.h"
#include "new104dl.h"
#include "..\newfiletran\FileTranNew.h"
#include "..\newfiletran\StdFileTran.h"
#ifdef __cplusplus
extern "C" {
#endif


#define SCHEDULE_DATA1_BIENT       0x0001
#define SCHEDULE_DATA1_DEVSTATUS   0x0002
#define SCHEDULE_DATA1_READDATA    0x0004
#define SCHEDULE_MSG               0x0008
#define SCHEDULE_GROUP             0x0010
#define SCHEDULE_FILE              0x0020
#define SCHEDULE_PROC              0x0040
#define SCHEDULE_INITOK            0x0080
#define SCHEDULE_DATA1_BIET        0x0100
#define SCHEDULE_GXPARA            0x0200
#define SCHEDULE_GXPARAEND         0x0400
#define SCHEDULE_GXSENDPARA        0x0800
#define SCHEDULE_FT_DIR            0x1000
#define SCHEDULE_FT_DATA           0x2000  
#define SCHEDULE_FT_FREVENT        0x4000 
#define SCHEDULE_RMTPARA           0x8000

/*以下为6月规约实施规范*/
#define SCHEDULE_FT_DIR_STD                0x00010000
#define SCHEDULE_FT_FILE_READY_STD         0x00020000 
#define SCHEDULE_FT_SECTION_READY_STD      0x00040000
#define SCHEDULE_FT_DATA_STD               0x00080000
#define SCHEDULE_FT_LAST_SECTION_FILE_STD  0x00100000    
#define SCHEDULE_XSFILESYNFINISH          0x00400000   //文件同步；cl 20180314

#define PTL104_CLOCK_READ			0X01     
#define PTL104_CLOCK_WRITE			0X02 
#define PTL104_CLOCK_NACK			0X80 


struct PNva
{
    INT8U Flag;
    short Value;
    short TempValue;
    BOOL WillSend;
};

struct PDevData
{
    INT16U AINum;
    INT16U BINum;
    INT16U DBINum;             //wjr
    INT16U CounterNum;
    INT16U BONum;
    INT16U SPINum;
    INT16U BCDNum;
    INT16U NvaNo;
    INT32U *LastCounterData;
    struct RealCounter_t *CounterData;
    struct PNva	*AIData;
    short *AIMaxVal;
    short *AIMaxValTrue;
    INT16U  *AItype;  //发送数据的类型，1-有符号 0-无符号
    INT8U *AIporperty;//ai数据属性
};

struct PDevInfo
{
    INT8U Flag;
    INT16U DevID;
    INT16U Addr;
    INT16U MAddr;
    struct PDevData DevData;
    struct LogDbaseWin_t *DbaseWin;
    struct RealDbaseWin_t *RealWin;
};

struct PData1
{
    INT16U PubAddr;
    INT16U InfoAddr;
};

struct PGroupTrn
{
    INT8U TypeID;       //标示总招类型，分为站召唤和电能量总招
    BOOL First;
    INT8U COT;          //总招传送原因，用于区分是总招还是分组召唤
    INT16U PubAddr;
    INT16U DevIndex;
    INT16U GroupNo;     //组序号1~8为遥信，9~14为遥测。根据该序号控制发送遥信还是遥测。104每128个遥信为一组
    INT32U InfoAddr;    //信息体地址，用于确定是否发送完成。
    INT8U Description;
    INT8U  HaveSendDBI; //DBI传送标志。用于启动双点遥信启动
    INT16U SoeStartPtr; //soe总召唤的开始指针。（广西）

};
#define STANDARDFILETRANS104 1

/*重新启动后如果是通讯断复位的情况下，定时30分钟没通则复位，定时器事项。wjr 2010.5.30*/
#define EV_SYSRET         0x00010000
#define MAXBREAKNUM     600         /*网路通的情况下10分钟没有收到数据ding20100115*/

//#define SFILETRANAPP104    1     /*调用文件传输操作*/ 
#ifdef SFILETRANAPP104
#undef STANDARDFILETRANS104              //由于这两类文件传输有共同的类型标识符，因此在用之前的文件传输机制时，标准文件传输过程不起作用。
#define CSFileTran      New104Sec       //这样做的目的，是把CSFileTran类替换到New104Sec类，在下面把FileTranApp.h文件中的又声明了一遍，实际上没有使用FileTranApp.h文件 ll

/***********************************************/

#define SFTYPECALLYC            192         /*召唤遥测（扩展命令）*/
#define SFTYPESELECTFILE        193         /*选择文件（扩展命令）*/

/*以下为文件名（扩展命令）*/
#define SFTFYC                  192         /*历史遥测文件*/
#define SFTFDD                  193         /*历史电度文件*/
#define SFTFMAX                 194         /*极大值文件*/
#define SFTFMIN                 195         /*极小值文件*/


#define	SF_SC_NA        		122         /*召唤目录﹑选择文件﹑召唤文件﹑召唤节*/
#define	SF_AF_NA        		124         /*确认文件﹑确认节*/
#define	SFREQ       		    5           /*传送原因－－请求或被请求*/
#define	SFACTCON		        7 	        /*激活确认*/
#define	SFACTTERM		        10 	        /*激活结束*/
#define	SFILE               	13 	        /*传送原因－－文件传送*/
#define SFUNKNOWNTYPEID         44          /*传送原因－－未知的类型标识*/
#define SFUNKNOWNCOT            45          /*传送原因－－未知的传送原因*/
#define SFUNKNOWNPUBADDR        46          /*传送原因－－未知的应用服务数据单元公共地址*/
#define SFUNKNOWNTINFOADDR      47          /*传送原因－－未知的信息对象地址*/
#define SFLAI                   0x4001

#define SFFILEHEADLEN           17          /*文件头长度*/
#define SFSectionLen            64000       /*节长度*/


enum FileStatus {FileOver=0,CallDir,SelectFile,CallFile,CallSection,
                 SendSegment,LastSegment,LastSection,LastFile};
enum CallYcStatus {CYNouse=0, CYAck, CYCall, CYEnd, CYError};

struct SFileInfo_t{         /*运行信息*/
    enum FileStatus FileStep;
    
    INT8U   RxID;
    INT8U   RxVsq;
    INT16U  RxCot;
    INT16U  RxPubAddr;
    INT32U  RxInfoAddr;
    
    INT32U  RxInfoAddrEnd;
    struct Iec101ClockTime_t    RxTimeStart;
    struct Iec101ClockTime_t    RxTimeEnd;
    
    INT16U  FileName;       /*文件名*/
    INT8U   SectionName;    /*节名*/    
    
    /*以下为struct FileOPData*/
    INT16U  FDataPer;           /*数据的时间间隔（单位：分钟）*/
    INT16U  FDataNum;           /*信息元素数*/
    INT32U  FDataClock;         /*起始绝对时间*/

    INT32U  FileLen;
    INT32U  SectLen;
    
    INT32U  FileCur;
    INT32U  SectCur;
    INT8U   FileChs;
    INT8U   SectChs;
    
    /*召唤遥测*/
    INT16U  AllYCNum;
    enum CallYcStatus   CallYcStep;
    INT8U   RxYcLen;
    INT8U   RxYcData[250];
    INT8U   InfoNum;
    INT16U  InfoAddr[127];  /*支持遥测号0～65535*/
};

#endif  //end of #ifdef SFILETRANAPP104
/***********************************************/

#ifdef STANDARDFILETRANS104
enum FileStatus {FileOver=0,CallDir,SelectFile,CallFile,CallSection,
                 SendSegment,LastSegment,LastSection,LastFile};
enum AckStatus{NOTUSED=0,ACKFILEPOS,ACKFILENEG,ACKSECTPOS,ACKSECTNEG};  
#define	SFREQ       		    5           /*传送原因－－请求或被请求*/  
#define	SFILE               	13 	        /*传送原因－－文件传送*/             
struct StdFileInfo_t{         /*运行信息*/
    enum FileStatus FileStep;
    enum AckStatus  ackstatus;
    INT8U   RxID;
    INT8U   RxVsq;
    INT16U  RxCot;
    INT16U  RxPubAddr;
    INT32U  RxInfoAddr;
    
    INT32U  RxInfoAddrEnd;
    struct Iec101ClockTime_t    RxTimeStart;
    struct Iec101ClockTime_t    RxTimeEnd;
    
    INT16U  FileName;       /*文件名*/
    INT8U   SectionName;    /*节名*/    
    INT8U   SectionNum;
    
    /*以下为struct FileOPData*/
    INT16U  FDataPer;           /*数据的时间间隔（单位：分钟）*/
    INT16U  FDataNum;           /*信息元素数*/
    INT32U  FDataClock;         /*起始绝对时间*/

    INT32U  FileLen;
    INT32U  SectLen;
    
    INT32U  FileCur;
    INT32U  SectCur;
    INT8U   FileChs;
    INT8U   SectChs;
    
    
};
#endif

#define RMT_RW_MAXNUM   100
#define GX_RW_MAXNUM 64

enum PData2Seq {Soe=0,Nva,End};
extern struct config_t MyConfig;

class New104Sec
{
private:
    struct PMySelf MySelf;

    struct PMessage *pMsg;
    INT8U FrameLen;
    BOOL Revert;
    BOOL YKSetAlready;

    BOOL CallDDFlag;
    BOOL CallAllDataFlag;
    BOOL CallDD;
    INT16U YcBeginNo;
    INT16U DevCount;
    INT16U ActDevIndex;
    INT16U BODevIndex;
    struct PDevInfo *DevList;
    struct PSec104Pad Sec104Pad;

    /*为解决北京网路死问题所做的改动 wjr 2010.5.21*/
    INT16U LinkConnect;       /*网络通上的标志  wjr  2010.5.21*/
    INT16U LinkBreakCounter;  /*网路通的情况下没有收到数据的计数器 wjr  2010.5.21*/
    
    INT8U *RxMsg;
    INT8U *TxMsg;
    INT8U DBData[2048];//向数据库取数据时使用的临时缓冲区
    
    UINT16  LBIinfoaddr;  //单点遥信信息体地址       2008.11.5
    UINT16  LDBIinfoaddr; //双点遥信信息体地址       2008.11.5
    
    struct BIEWithTimeData_t DBIDBData[100];        //向数据库取数据时使用的临时缓冲区   wjr
    struct BIEWithoutTimeData_t DBICOSDBData[100];  //向数据库取数据时使用的临时缓冲区   wjr
    INT16U DBISOEnum;   //收到的双点遥信的soe数
    INT16U DBICOSnum;   //收到的双点遥信的cos数
    INT16U DBIDevIndex;    //需要发送双点遥信soe的设备号
    INT16U DBICOSDevIndex;    //需要发送双点遥信cos的设备号

    
    INT16U AsduHeadLength;//ASDU头长度，类型标志到信息体地址
    INT16U PubAddrSize;
    INT16U CotSize;
    INT16U InfoAddrSize;
    INT16U CotLocation;
    INT16U PubAddrLocation;
    INT16U InfoAddrLocation;
    INT16U BroadCastAddr;

    INT8U RxTypeID;
    INT8U RxVsq;
    INT8U RxCot;
    INT16U RxPubAddr;
    INT32U RxInfoAddr;
    INT8U *pRxData;

    INT8U *pTxTypeID;
    INT8U *pTxVSQ;
    INT8U *pTxInfoAddr;
    INT8U *pTxData;

    BOOL AllDataEnd;
    INT16U NvaActDevNo;
    enum PData2Seq Data2Seq;
    struct PData1  Data1;
    struct PGroupTrn GroupTrn;

    INT32U ScheduleFlag;

    INT32U AllDataInterval;
    INT32U CounterInterval;
    INT32U NvaInterval;
    INT32U AllDataCount;
    INT32U CounterCount;
    INT32U NvaCount;

    INT8U InitFlag;             // 初始化标志
    INT8U YKTypeID;
    
    BOOL   FirstCallAllData;   //第一次总招不被打断功能，0xff-已经总招过，0-未总招过。第1次总招结束后再发送其他数据。
    INT16U WaitCallAllDelay;
    
    BOOL GYKZ2015Flag;        //2015版104规约扩展特殊规定处理
    INT16U SendCOS;         //在勾选2015扩展版时如果还需要发送cos，则
    //BOOL RMTParaGHSimple;       //debug 2017-3-1 远程参数固化过程不检查携带的参数。
    //BOOL HisFileTxt;            //debug 历史数据文本方式
    
    INT16U RMTHaveReadParaFlag;     //部分读取时，作为已读取序号。全部读取时作为读取位置标号
    BOOL   RMTParaReadAllFlag;      //读全部参数标志
    INT16U RMTSectionNo;            //暂时定义一个，应付当前区号。
    INT16U RMTSectionNo2;
    INT16U RMTParaNum;              //读或写的个数
    INT16U RMTParaInfo[RMT_RW_MAXNUM];
    float  RMTParaValue[RMT_RW_MAXNUM];
    INT16U RMTTimeOut;                 //预置标志超时处理
    BOOL   RMTParaYZ;               //预置标志
    
    //遥信以双点遥信发送-广东佛山
    BOOL  bSendAllDBI;
    
    //广西远程运维
    INT8U Roi;
    INT8U Qpa;
    INT16U GXTimeOut;
    BOOL   GXParaYZ;               //广西预置标志
    INT16U GXParaNum;              //读或写的个数
    //BOOL GXvsqflag;
    INT8U GXReturnCot;
    INT8U GXParaControl;
    INT16U GXParaInfo[GX_RW_MAXNUM];
    float GXParaValue[GX_RW_MAXNUM];
    /***********************************************/
    #ifdef SFILETRANAPP104
        
    struct SFileInfo_t   SFileInfo;
    
    INT8U   TxData[256];

    void SFileCallProc(INT8U* pData);
    void SFileConfirm(INT8U* pData);
    void SFileSelectProc(INT8U* pData);
    BOOL SFileGetRcInfo(INT8U* pData, INT16U* Len);

    INT16U SFileReady(void);
    INT16U SSectionReady(void);
    INT16U SSendSegment(void);
    INT16U SSendLastSegment(void);
    INT16U SSendLastSection(void);
    INT16U SFileHead(INT8U *pData);
    INT32U SFileGetAbsClk(struct Iec101ClockTime_t *Time);
       
    void SCallYCProc(INT8U* pData);
    BOOL SCallYcGetRcInfo(INT8U* pData, INT16U* Len);
    INT16U SCallYCEncode(void);
    
    #endif
    /***********************************************/
    
    /****NEW文件传输START***********/
    struct FTFileTransfer_t  FtInfo;
    
    void ProcFileInit(void);
    
    void ProcFileTran(void);
    void ProcFT_ReadDir(void);
    void ProcFT_ReadFileAct(void);    
    
    void ProcFT_EncodeReadDir(void);
    void ProcFT_EncodeFileData(void);
    
    void ProcFT_ProgramUpdate(void);
    void ProcFT_WriteFileAct(void);
    void ProcFT_WriteFileData(void);
    void ProcFT_WriteFileDataConf(void);
    
    void ProcFileSyn(void);//线损模块2018标准，文件同步；cl 20180314
    void SendFreezeEvent2Pri101(void);//给101主站发送消息进行瞬时冻结
    
    /****NEW文件传输END***********/
    
    
    
    /****6月份NEW文件传输START***********/
    #ifdef STANDARDFILETRANS104
    struct StdFileInfo_t StdFileInfo; 
    struct StdFileTransfer_t  StdFtInfo;
    void StdGetFileInfo(INT8U*);
    void StdProcFileTran(void);
    void StdProcFT_ReadDir();
    void StdProcFT_EncodeReadDir(void);
    void StdProcFileAck(void);
    void StdProcFT_EncodeFileReady();
    void StdProcFT_EncodeSectionReady();
    void StdProcFT_EncodeSegment();
    void StdProcFT_EncodeLastSegSect();
    void StdProcFT_ReadFile();
    #endif
    /****6月份NEW文件传输END***********/
    void ProcSetSectionNo(void);
    void ProcReadSectionNo(void);
    void ProcReadPara(void);
    void EncodeRMTReadPara(void);
    BOOL RMTReadAllPara(INT8U *pbuf, INT8U *plen, INT8U *psendnum);
    void DeadValueRealTimeEffect(void);
    void ProcWritePara(void);
    void RMTParaInit(void);
    void RMTParaYzCheck(void);
    
    void EnCodeFREvent(void);
    void CheckFREOnTime(void);

    void SetAllVarNull(void);
    BOOL SetDevInfo(INT16U DevID);
    void InitPara(void);
    void CheckPad(void);
    void SetDefaultPad(void);
    void ReadAIMaxVal(INT16U i);

    BOOL SecResetUseP(void);
    BOOL SecTestDL(void);
    void SecCallData2(void);

    BOOL ToProc(void);
    void ProcControl(void);
    void ProcGroupTrn(void);
    void ProcAllData(void);
    void ProcClock(void);
    void ProcCounter(void);
    void ProcReadData(void);
    BOOL CheckAndModifyGroup(void);
    void ProcTaskMsg(void);
    BOOL ProcRoute(void);
    BOOL GetNextDev(void);
    BOOL GetActDevIndexByAddr(INT16U Addr);
    BOOL GetActDevIndexByDevID(INT16U DevID);

    void EnCodeData1(void);
    void EnCodeAllDataConf(void);
    void EnCodeCounterConf(void);
    BOOL EnCodeCtrlRet(void);
    BOOL EnCodeDLMsg(INT16U Len);

    INT16U EnCodeAllData(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    INT16U EnCodeAllYX(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    INT16U EnCodeAllYC(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    INT16U EnCodeAllST(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    INT16U EnCodeAllBCD(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    INT16U EnCodeAllSta(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    INT16U EnCodeAllLastSoe(INT16U BeginNo);
    
    void EnCodeNACK(INT16U Cot);//否定回答接口函数  CL
    void ProcInitEnd(void); //初始化结束帧处理：初始化结束 CL

    BOOL EnCodeNVA(void);
    BOOL EnCodeSOE(void);
    BOOL EnCodeBIENT(void);
    void EnCodeDBIENT(void);  //编辑双点遥信COS   
    void EnCodeDBISOE(void); //编辑双点遥信SOE  
    INT8U EnCodeCounter(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    void EnCodeReadData(void);

    void EnCodeGroupEnd(void);
    
    
    BOOL WritePara(void);
    void EnCodeClock(INT8U flag);
    BOOL EnCodeInitEnd(void);               //wjr初始化结束
    void ProcXsFileSynFinish(void);
    
    /*****广东远程参数*******/
    void ProcReadParaGD(void);
    void ProcWritePara_GD(void);
    void EncodeRMTReadPara_GD(void);  
    BOOL EnCodeBIENT_ALLDBI(void);  
    BOOL EnCodeSOE_ALLDBI(void);
            
    void ProcReadParaGX(void);
    void ProcSetParaGX(void);
    void ProcActivateParaGX(void);
    void ProcEncodeGXReadPara(void);
    void EnCodeGXReadParaEnd(void);
    void GXParaInit(void);
    void GXParaYzCheck(void);
    void ProcEncodeGXChangePara(void);
    INT16U GXRemoteParaCheck(void);
    void ProcEncodeGXSendPara(void);
    void GXWatchLPChange(void);
public:
    INT32U ScanTimerID;

    INT16U *LinkFlag;       /* 2010.5.30  当通讯通上之后6分钟没有数据后断开重启，如果启动之后半个小时之后还没连接上就认为错误没有回复再重新启动*/
    
    New104DataLink *pDLink;

    struct PDevInfo *pDev;

    New104Sec(INT16U AppID,struct PortInfo_t **ppport,struct TaskInfo_t **pptask,BOOL *pInitOK);
    ~New104Sec();

    void SendData1(INT32U Flag);
    void EnCodeNextFrame(void);
    void OnTimer(void);
    void OnMessage(void);
    void OnUData(void);
    void OnRxData(void);
    void OnCommState(void);
    void SetDevUseState(BOOL InUse);
    

    void ProcXSFileSynFinish(void);
};
void new104sec(INT16U AppID);
extern void startCellMaint(void);
extern void ResetFaultInfoForCall_FaultCheck(void);
void BspYkRelease(void);
void FT_Init(struct FTFileTransfer_t *pFtinfo);
void ProgLogWrite2(INT8 *fmt,INT32 arg1,INT32 arg2,INT32 arg3,INT32 arg4, INT8U IsWriteSysinfo, INT8U type, INT8U status);
//BOOL LogYkInfoRec(WORD DeviceID, WORD type, WORD dco, WORD infoaddr, WORD cot);

BOOL testGWFREvent(INT16U LogicDevId);
BOOL GWFREventRead(struct FaultRec_t *pfrevent, INT16U LogicDevId);
void GetTerminalPara(INT8U* buf,INT8U* len,INT16U addr,INT16U num);
INT16U SetTerminalPara(float value, INT16U addr);
BOOLEAN GetSiQuChangeFlag(INT16U ParaFlag);
void SaveTerminalPara(void);
void SaveRMTParaToSys(void);
void SaveRMTPadParaToSys(INT16U Appid,void *newpadpara);
INT16U GetRmtDeathvalue(INT8U ppty);

BOOL GetYKRYBState(void);
INT16U GetDBINum(void);
INT16U RmtParaChangeFloat2Word(INT16U info, float val);
void GetIDeadValuePara(INT8U* buf, INT8U* len, INT16U addr);

INT32U FT_GetDirID(struct FTFileTransfer_t *pFtinfo);
void ClearProgramUpdate(void);
void StartProgramUpdate(void);

INT16U GetWhLogicDevID(void);
INT8U CheckEncrptchip(INT8U CheckType);
INT8U Check1120aEncrptchip(INT8U CheckType);
INT8U EncrptyChiptest(INT8U type);
INT16U L_GetETWptr(WORD DeviceID, WORD *pMaxNum);
BOOL GXReadAllPara(INT8U *buf,INT8U *len,INT8U *sendnum,INT8U infosize,INT8U control);
BOOL GXReadCommonPara(INT8U *buf,INT8U *len,INT8U *sendnum,INT8U infosize);
BOOL GXReadJianGePara(INT8U *buf,INT8U *len,INT8U *sendnum,INT8U infosize,INT8U control);
INT16U SetTerminalParaGX(float value,INT16U addr);
void GXRemoteParaEffect();
INT16U GXParaSetCheck(float value,INT16U addr);
float GxGetParaValue(INT16U addr);
INT16U GXGetChangeYxFlag(void);
BOOL GxGetDeadValueFlag(INT16U flag);

INT16U GetFeederNum(void);
BOOL appGetJXStatus(void);
BOOL ReadRemoteParaSetEnableState(void);

#ifdef __cplusplus
}
#endif

#endif




