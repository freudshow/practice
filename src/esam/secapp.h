#ifndef	_SECAPP_H
#define _SECAPP_H

#include "iec101def.h"
#include "..\newfiletran\FileTranNew.h"

#ifdef	__cplusplus
extern "C"{
#endif

//#define INFOADDR2BYTE       1   //有的信息体地址是2字节

#define APP_DATA1_DEF (HaveInitEnd|HaveCOS|HaveSOE|HaveYK|CallAllData|CallTimeDelay|CallDD|HaveFA|CallSetNVA|ProtectCon|CallParaSet|FreezeDD|CallReadData|CallLCdata|DATA1_FT_FREVENT|LCSetPara|LCActivatePara|CallLightStatus|SummonInfoOnBoot)
#define APP_DATA2_DEF (CallClock|CallTest|HaveNVA|CallReset|DATA2_FT_DIR|DATA2_FT_FILEACT|DATA2_FT_FILEDATA|DATA2_FT_WTFILEACT|DATA2_FT_WTDATAACT|DATA2_RMT_SETSEC|DATA2_RMT_READSEC|DATA2_RMT_SETPARA|DATA2_RMT_READPARA|DATA2_PUP_PROGUP|DATA2_XSFT_SYNACT|DATA2_XSFT_SYNACTFINISH|DATA2_GX_READPARA|DATA2_GX_SETPARA|DATA2_RMT_READPARA_GD|DATA2_RMT_WRITEPARA_GD)

//Data1.Flag
#define CallLCdata          0x00000010
#define LCSetPara           0x00000020
#define LCActivatePara      0x00000040
#define CallLightStatus     0x00000080
#define HaveInitEnd         0x00000100
#define	HaveCOS	            0x00000200
#define HaveSOE	            0x00000400
#define HaveFA	            0x00000800
#define HaveYK              0x00001000
#define CallAllData         0x00002000
#define CallClock           0x00004000
#define CallDD              0x00008000
#define CallTimeDelay       0x00010000
#define CallReadData	    0x00020000
#define FreezeDD            0x00040000
#define CallParaSet         0x00080000
#define CallReset           0x00100000
#define CallSetNVA          0x00200000
#define CallTest            0x00400000
#define HaveNVA	            0x00800000
#define ProtectCon          0x01000000
#define HaveHisDD           0x02000000
#define DATA1_FT_FREVENT    0x04000000 
#define SummonInfoOnBoot    0x08000000
//Data2Flag
#define BackData	        0x0001
#define PerCycData	        0x0002
#define ProtectData 	    0x0004
#define UpLoadFile  	    0x0008
#define NewFiletran         0x0010
#define CallYCProc          0x0020

#define DATA2_FT_DIR        0x00000100
#define DATA2_FT_FILEACT    0x00000200
#define DATA2_FT_FILEDATA   0x00000400
#define DATA2_FT_DATA       0x00000800  
#define DATA2_FT_WTFILEACT  0x00001000   
#define DATA2_FT_WTDATAACT  0x00002000    
#define DATA2_RMT_READPARA  0x00010000
#define DATA2_RMT_SETPARA   0x00020000
#define DATA2_RMT_READSEC   0x00040000
#define DATA2_RMT_SETSEC    0x00080000
#define DATA2_PUP_PROGUP    0x00100000
#define DATA2_GX_READPARA   0x00200000
#define DATA2_GX_SETPARA    0x00400000
#define DATA2_GX_ACTIVATEPARA    0x00800000
#define DATA2_XSFT_SYNACT        0x01000000     //线损文件同步激活  CL 20180607
#define DATA2_XSFT_SYNACTFINISH  0x02000000     //线损文件统计激活终止
#define DATA2_RMT_READPARA_GD    0X04000000     //广东读参数
#define DATA2_RMT_WRITEPARA_GD   0X08000000     //广东写参数

#define BIETFRAME   0x01
#define BIENTFRAME  0x02
#define FAPROCFRAME 0x04

#define WAIT_CALLALL_DELAY  10      /*第1次总招禁止被打断，用于平衡模式，广西测试要求*/

/*液晶项目所需宏定义,包括4个扩展类型标识*/
#define C_LC_CALL_YC_YX 136
#define SUMMON_YX       49
#define SUMMON_YC       50

#define C_LC_CALL_SYSINFO_SOE 137
#define SUMMON_SYS_INFO       49
#define SUMMON_SOE            50

#define C_LC_CALL_NAME_VER_CLOCK 138
#define SUMMON_YX_NAME           49
#define SUMMON_YC_NAME           50
#define SUMMON_SOFTWARE_VERSION  51
#define SUMMON_CLOCK             52

#define C_LC_PANEL_DRIVER 139
#define FAULT_RESET      49

#define C_LC_SUMMON_PARA 150
#define C_LC_SET_PARA    151
#define C_LC_ACTIVATE_PARA    152

#define CONTROLLERAPPPARA 3
#define CONTROLLERPROTECTPARA 0
#define CONTROLLERFAULTDETECTPARA 1
#define CONTROLLERVOLTAGETIMEPARA 2
#define CONTROLLERVOLTAGECURRENTPARA 3
enum PMASTERUseStatus{NOUSE=0,ALLDATA,INUSE,WAITDLSTATUS,WAITCON};
enum PFrmState{Polling,ResetDL,Cdt,Group,BI,File};
enum PData2Seq {Soe=0,FAProcInfo,Nva,End};
enum PFileStatus{PFileOver=0,PCallDir,PSelectFile,PCallFile,PCallSection,
                 PSendSegment,PLastSegment,PLastSection,PLastFile};

struct PNva
{  //变化遥测结构
    INT8U Flag;
    short Value;
    short TempValue;
    BOOL WillSend;
};

struct YcPara
{
    INT16U DeadValue;
    INT16U UpLimit;
    INT16U LowLimit;
    INT16U porperty;
    BOOL type;  //1-有符号 0-无符号
};


struct PDevData
{  //设备采集量数目
    INT16U AINum;
    INT16U BINum;
    INT16U DBINum;             //wjr
    INT16U CounterNum;
    INT16U BONum;
    INT16U SPINum;
    INT16U BCDNum;
    INT16U NvaNo;
    INT32U *LastCounterData;
    INT32U *HisCounterData;//保存历史电度
    INT16U  HisDDReadPtr;
    struct RealCounter_t *CounterData;
    struct PNva	*AIData;
    short *AIMaxVal;
    struct YcPara *AIPara;
        
};

struct PData1
{   //一级数据结构
    INT32U Flag;
    BOOL COT;
    INT16U SOENum;      //发送的SOE数目
    INT16U BIENTNum;    //发送的变位YX数目
    INT16U FAProcNum;   //发送的FA数目
    INT16U PubAddr;
    INT16U InfoAddr;	//信息体地址——2

};

struct PDevInfo
{
    INT8U Flag;
    INT16U DevID;
    INT16U Addr;
    INT16U MAddr;
    struct PDevData DevData;
    struct LogDbaseWin_t *pDbaseWin;
    struct RealDbaseWin_t *RealWin;
    INT8U DLUseStatus;
    struct PData1 Data1;//zzw2004/6/7；用于记录每个设备的1级数据信息。
};

struct PGroupTrn
{
    INT8U TypeID;
    BOOL First;
    INT8U COT;
    INT16U PubAddr;
    INT16U DevIndex;//分组发送的当前设备序号
    INT16U GroupNo;
    INT16U InfoAddr;	//信息体地址——2
    INT8U Description;
    INT8U  HaveSendDBI; //DBI传送标志。用于启动双点遥信启动
    INT16U SoeStartPtr; //soe总召唤的开始指针。（广西）
 };

extern struct config_t MyConfig;

//线损文件相关结构体
extern struct XSFileSynInfo_t XSFileSynInfo;

//定时保存历史电度
#define TIMETOSAVEDATA	60   //时间间隔
#define SAVENUM	(24*30)//保存历史电度的时间点数

struct DATAFORMAT
{
    struct AbsTime_t time;
    INT32U *Data;
};


struct SAVEDATABUF
{
    struct DATAFORMAT  saveData[SAVENUM];
};

#define RMT_RW_MAXNUM 100

#define GX_RW_MAXNUM 64

#define SFILETRANAPP101  1       /*调用文件传输操作*/

#ifdef SFILETRANAPP101
#define CSFileTran      CSecAppSev


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

#endif
/***********************************************/


class	CSecAppSev
{
    INT16U wAppID;
    INT8U *LCBuf;
    struct PMySelf MySelf;
    struct PMessage *pMsg;
/*    PMsgManager *pMsgMan;
    struct PFileMsg FileMsg;*/

    
    BOOL BalanMode;
    INT8U InitFlag;     //初始化是否发送过标志。0xff表示未发送过。 0表示发送过。

    //应用层服务调用带入的参数
    INT16U *AppCommand;
    INT16U DLCommand;
    INT16U LengthIn;
    INT16U *LengthOut;
    INT8U *RxMsg;	//应用层接收数据指针
    INT8U *TxMsg;	//应用层输出数据指针

    BOOL DDFreeze;       //wjr 

    INT8U DBData[2048];//向数据库取数据时使用的临时缓冲区
    
    struct BIEWithTimeData_t DBIDBData[100];//向数据库取数据时使用的临时缓冲区   wjr
    struct BIEWithoutTimeData_t DBICOSDBData[100];//向数据库取数据时使用的临时缓冲区   wjr
    INT16U DBISOEnum;       //收到的双点遥信的soe数
    INT16U DBICOSnum;       //收到的双点遥信的cos数
    INT16U DBIDevIndex;     //需要发送双点遥信soe的设备号
    INT16U DBICOSDevIndex;  //需要发送双点遥信cos的设备号
    INT8U  IsDBISoeSend;   //是否发送DBIsoe

    struct PData1 Data1;
    struct PGroupTrn GroupTrn;
    struct PGroupTrn LastGroupTrn;
    struct PGroupTrn GroupTrnDD;

    INT32U Data2Flag;

    UINT16  LBIinfoaddr;  //单点遥信信息体地址       2008.11.5
    UINT16  LDBIinfoaddr; //双点遥信信息体地址       2008.11.5

    INT16U DevCount;//管理的设备数目
    INT16U ActDevIndex;//
    INT16U LastDevIndex;//记录最近一次发送变位YX，SOE，FA，的设备序号。
    INT16U BODevIndex;//记录遥控命令的设备序号
    //struct PDevInfo *DevList;//设备信息
    INT16U NvaActDevNo;//发送变化遥测的当前设备序号。

    INT8U *YCGroupNo;   //用来储存每个遥测的组号，以便检索类型标识
    INT8U *YXGroupNo;   //用来储存每个遥信的组号，以便检索类型标识

    //enum PMASTERUseStatus MasterStatus;//zzw
    
    INT16U RestDLDelay;                 //启动链路建立完成后，延时30秒从动还未建立链路，则启动应用层报文 
    
    BOOL   IsAllSendInitEnd;            //是否每次重新链接后发送初始化结束帧
            
    BOOL   FirstCallAllData;              //第一次总招不被打断功能，适用于平衡模式，0xff-已经总招过，0-未总招过。终端收到对方链路复位后，该标志为0，第1次总招结束后再发送其他数据。
    INT16U WaitCallAllDelay;
    
    //遥信以双点遥信发送-广东佛山
    BOOL  bSendAllDBI;
    
    INT16U RDPubAddr;
    INT16U RDInfoAddr;

    INT16U ResetPubAddr;
    INT16U ResetInfoAddr;
    INT8U ResetGRP;
    INT16U ResetCount;
    INT8U ResetFlag;
   
    //北京故障复归
    INT8U RFaultFlag;
    BOOL YKSetAlready;

    INT16U TestPubAddr;
    INT16U TestInfoAddr;
    INT16U TestFBP;

    INT16U SetPubAddr;
    INT16U SetInfoAddr;
    INT16U SetNvaWord;
    INT8U  SetQOS;

    INT16U ParaPubAddr;
    INT16U ParaInfoAddr;
    INT8U  ParaTypeID;
    INT16U ParaWord;
    float  ParaFloat;
    INT8U  ParaQPM;

    INT32U AllDataInterval;
    INT32U CounterInterval;
    INT32U NvaInterval;
    //BOOL   ScanData2Flag;
    INT32U ScanData2Count;
    INT32U CdtModeInterval;
    INT32U AllDataCount;
    INT32U CounterCount;
    INT32U WatchDogCount;

    //保存遥控临时信息
    INT8U YKTypeID;
    enum {NOTUSE=0,YKERROR,YKSETCON,YKEXECON,YKCANCELCON,YKTERM}YKStatus;
    INT16U YkStatusForTest; //ll 为广州测试临时修改 2012-3-24
    INT8U DcoTemp;
    INT8U ScoTemp;
    //struct PDBBO PDBBOTemp;
    INT16U SwitchNoTemp;

    INT16U SDTTime;//存储主站发送的SDT时间
    INT16U TrTime; //子站从接收到发送延时获得命令的时间间隔
    INT16U SendTime;//=SDTTime+TrTime
    INT16U TimeDelay;
    struct AbsTime_t SecSysTimeR;//子站收到C_CD_NA时的系统时间
    struct AbsTime_t SecSysTimeT;//子站发送M_CD_NA时的系统时间
    struct AbsTime_t OldSysTime;//子站设置时钟前的系统时间

    INT8U EditDDCon;
    struct AbsTime_t CounterTime;//冻结电度的时间

    INT16U BackScanTime;//分，背景数据扫描间隔
    INT16U CycScanTime;//秒，周期循环数据扫描间隔
    INT16U BackScanCount;
    INT16U CycScanCount;

    INT8U ReadTimeFlag;

    //错误命令处理
    BOOL HaveWrongData;
    INT16U WrongDataLength;
    INT8U WrongData[256];

    enum PFrmState Status;
    enum PFrmState LastStatus;
    INT8U BIFrame;
    INT8U EditAllDataCon;
    
    INT8U EditReadParaCon;
    INT8U ActiveParaCon;    //远程参数激活控制参数，1-回确认帧 2-参数回复 3-主动发送参数
    

    struct PDevInfo *pDev;
    enum PFrmState LastFrame;

    //文件上传;一个文件当做一个节来处理。
    INT32U FileReadPtr;
    INT16U CurrentFileName;
    INT32U CurrentFileSize;
    INT16U CurrentInfoAddr;
    INT16U CurrentZBNo;
    struct DIRUNIT DirUnit;
    enum PFileStatus FileStep;
    INT8U FileCheckSum;

    //保护定值
    INT8U ProtectValue[33];//最后一个字节，可以用做成功标志

    //历史电度保存时间
    struct Iec101ClockTime_t HisDDTime;
    enum {Start=0,EditCon,SendData,SendOver}HisDDStatus;

    BOOL SetDevInfo(INT16U devid);
    void InitPara(void);
    void CheckPad(void);
    void SetDefaultPad(void);
    void ReadAIMaxVal(INT16U i);

    //历史电度
    INT16U HisDDDevNo;//当前历史电度设备序号

    //
    INT16U FixFrmLength;//固定帧长度
    INT16U AsduHeadLength;//ASDU头长度，类型标志到信息体地址
    INT16U LinkAddrSize;
    INT16U CotSize;
    INT16U PubAddrSize;
    INT16U InfoAddrSize;
    
    INT16U InfoAddrLocation;
    INT16U BroadCastAddr;
    
    INT8U RxTypeID;
    INT8U StoreRxTypeID;
    INT8U Qoi;
    INT8U RxVsq;
    INT8U RxCot;
    INT16U RxPubAddr;
    INT32U RxInfoAddr;
    INT16U LCInforAddr;
    INT16U LCAmount;
    INT16U Addresses_Discrete[9];
    INT8U paramirrorbuf[255];
    INT8U totallength_m;
    INT8U filenum;
    INT8U *pRxData;
    
    
    //INT8U ReadParaCot;

    INT8U *pTxTypeID;
    INT8U *pTxVSQ;
    INT8U *pTxInfoAddr;
    INT8U *pTxData;

    short Data[150];//录波数据暂存
    
    INT8U *pRestType;   //复位规约进程状态记忆 ll 2010/07/20   for 广西规约测试
    
    
    
    INT16U RMTHaveReadParaFlag;     //部分读取时，作为已读取序号。全部读取时作为读取位置标号
    BOOL   RMTParaReadAllFlag;      //读全部参数标志
    INT16U RMTSectionNo;            //暂时定义一个，应付当前区号。
    INT16U RMTParaNum;              //读或写的个数
    INT16U RMTParaInfo[RMT_RW_MAXNUM];
    float RMTParaValue[RMT_RW_MAXNUM];
    INT16U RMTTimeOut;              //预置标志超时处理
    BOOL   RMTParaYZ;               //预置标志
    INT8U  RMTReturnCot;            //记录返回的传送原因
    INT8U  RMTReturnVsq;            //记录返回使用的vsq
    
    INT8U  ProgramUpadateCot;      //软件升级命令返回COT
    INT8U  ProgramUpadateSE;
    
    //广西远程运维
    INT8U Roi;
    INT8U Qpa;
    INT16U GXTimeOut;
    BOOL   GXParaYZ;               //广西预置标志
    INT16U GXParaNum;              //读或写的个数
    BOOL  GXvsqflag;
    INT8U GXReturnCot;
    INT8U GXParaControl;
    INT16U GXParaInfo[GX_RW_MAXNUM];
    float GXParaValue[GX_RW_MAXNUM];
    
    /****NEW文件传输START***********/
    struct FTFileTransfer_t  FtInfo;
    
    void ProcFileInit(void);
    
    void ProcFileTran(void);
    
    void ProcFT_ReadDir(void);
    BOOL ProcFT_EncodeReadDir(void);
    void ProcFT_ReadFileAct(void); 
    void ProcFT_EncodeFileActConf(void);
    BOOL ProcFT_EncodeFileData(void); 
    void ProcFT_EncodeWriteFileActConf(void);
    void ProcFT_EncodeWriteFileDataConf(void);
        
    void ProcFT_ProgramUpdate(void);
    void ProcFT_WriteFileAct(void);
    void ProcFT_WriteFileData(void);
    
    /****NEW文件传输END***********/
    void EnCodeFREvent(void);
    void CheckFREOnTime(void);  
    
    void EnCodeGXReadParaConf(void);
    void EnCodeGXReadParaEnd(void);
    BOOL ProcEncodeGXReadPara(void);
    void ProcReadParaGX(void);
    void ProcSetParaGX(void);
    void GXParaInit(void);
    void ProcActivateParaGX(void);
    void ProcEncodeGXSetPara(void);
    void ProcEncodeGXActivatePara(void);
    void ProcEncodeGXChangePara(void);
    INT16U GXRemoteParaCheck(void);
    void GXWatchLPChange(void);
    void GXParaYzCheck(void);
    BOOL ProcEncodeGXSendPara(void);
    
    
    void RMTParaYzCheck(void);
    void ProcReadPara(void);
    BOOL ProcEncodeRMTReadPara(void);
    BOOL RMTReadAllPara(INT8U *pbuf, INT8U *plen, INT8U *psendnum);
    void DeadValueRealTimeEffect(void);
    void ProcWritePara(void);
    void ProcEncodeRMTSetPara(void);
    void ProcEncodePUPupdateConf(void);
    void ProcSetSectionNo(void);
    void ProcEncodeSetSectionNo(void);
    void ProcReadSectionNo(void);
    void ProcEnCodeReadSectionNo(void) ;
    void RMTParaInit(void);
    
    void SetDevUseState(void);
    
    void SetSendData2Flag(INT32U flag);
    void EnCode101DLMsg(INT16U len, INT16U appcmd);
    void JudgeSendInitEnd(void);
    
    void ProcEncodeRMTSetPara_GD(void);
    void ProcWritePara_GD(void);
    BOOL ProcEncodeRMTReadPara_gd(void);
    void ProcReadParaGD(void);
    BOOL EnCodeBIENT_ALLDBI(void);
    BOOL EnCodeSOE_ALLDBI(void);
    
    void ProcEncodeXSFileSynConf(void);//线损文件同步的确认桢回复
    void ProcEncodeXSFileSynFinish(void);//线损文件同步的确认桢回复          

    void ProcData1(void);
    void ProcData2(void);
    void ProcControl(void);  //处理遥控
    void ProcSetNVA(void);
    void ProcLCdataCall(void);
    void ProcAllDataCall(void);
    void ProcDDCall(void);
    void ProcTimeDelay(void);
    void ProcClock(BOOL Conf); //处理对钟
    void ProcReset(void);
    void ProcTest(void);
    void ProcReadData(void);
    void ProcParaSet(void);
    void EnCodeInitEnd(void);
    void ProcTaskMsg(void);//处理遥控返校信息
    BOOL EnCodeCtrlRet(void);  //遥控返校
    void EditYKTerm(void);//遥控结束
    BOOL GetActDevIndexByAddr(INT16U Addr);
    BOOL EnCodeBIENT(void);  //编辑COS  将来这里要区分单点双点yx
    void EnCodeDBIENT(void);  //编辑双点遥信COS   wjr
    BOOL EnCodeSOE(void); //编辑SOE //暂时不考虑双点遥信
    void EnCodeDBISOE(void); //编辑双点遥信SOE  wjr
    void EnCodeAllDataConf(void);//总召唤确认帧
    BOOL ProcAllData(void); //处理全数据
    BOOL CheckAndModifyGroup(void);
    BOOL CheckDDGroup(void);
    INT8U EnCodeAllData(INT16U BeginNo,INT16U EndNo,INT16U *pNum);//zzw
    INT8U EnCodeAllYX(INT16U DevIndex,INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    void EnCodeLC();
    INT8U EnCodeLCYX_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YX_Addresses);
    INT8U EnCodeLCYX(INT16U DevIndex,INT16U FirstInfoAddr,INT16U Amount);
    INT8U EnCodeLCYC_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YC_Addresses);
    INT8U EnCodeLCYC(INT16U DevIndex,INT16U FirstInfoAddr,INT16U Amount);
    void EnCodeLCClock(INT16U DevIndex);
    void EnCodeLCSoftwareVer(INT16U DevIndex);
    INT8U EnCodeLCSOE(INT16U DevIndex,WORD OffSet,WORD Num);
    INT8U EnCodeLCSysInfo(INT16U DevIndex,WORD OffSet,WORD Num);
    INT8U EnCodeLCCOS(INT16U DevIndex,WORD OffSet,WORD Num);
    INT8U EnCodeLCYcName_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YcName_Addresses);
    INT8U EnCodeLCYcName(INT16U DevIndex,WORD OffSet,WORD Num);
    INT8U EnCodeLCYxName_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YxName_Addresses);
    INT8U EnCodeLCYxName(INT16U DevIndex,WORD OffSet,WORD Num);
    INT8U EnCodeAllYC(INT16U DevIndex,INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    INT8U EnCodeAllPara(INT16U DevIndex,INT16U BeginNo,INT16U EndNo,INT16U *pNum);
	void  WriteParaFile(INT16U Addr,INT16U ParaLength,INT8U Qoi,INT8U *p);
	void ProcLCSetPara(BOOL Conf);
	void ProcActivatePara(BOOL Conf);
	void EnCodeParaMirror(INT16U DevIndex,INT8U Length);
	void EnCodeActivatePara(INT16U DevIndex);
	void EnCodeLCFaultOrProtectPara(INT16U DevIndex,INT8U qoi,INT16U LCInforAddr);
	void EnCodeLCCommunicatePara(INT16U DevIndex,INT8U qoi,INT16U LCInforAddr);
    void EnCodeGroupEnd(void);//结束帧
    void EnCodeDDGroupEnd(void);//结束帧
    BOOL GetNextDev(void); //得到下一个设备
    BOOL GetNextDDDev(void); //得到下一个设备
    BOOL EnCodeNVA(void);  //编辑变化遥测数据;
    void FreezeCounter(void);
    void EnCodeCounterConf(void);//
    void ProcCounter(void) ;//处理电度zzw
    INT8U EnCodeCounter(INT16U BeginNo,INT16U EndNo,INT16U *pNum);//从数据库取电度，处理后，发到链路层。
    void EnCodeTimeDelay(void);
    void EnCodeClock(void);//发送子站设置时钟前的系统时钟
    void EnCodeReadData(void);
    void EnCodeReadDataCon(INT16U DevIndex,INT8U Cot);
    void EnCodeReadPara(INT16U DevIndex,INT16U InfoAddr);
    void ParaSetCon(void);
    void ProcLCPanelDriver(INT8U control,INT8U command);
    void ProcSummonLightStatus();
    void ProcSummonInfoOnBoot();
    void EnCodeLightStatus(INT16U DevIndex);
    void EnCodeSummonInfoOnBoot(INT16U DevIndex);
    void EnCodeReset(void);
    void EnCodeTest(void);
    void EnCodeSetNVA(void);
    BOOL ClearFlag(INT16U DevIndex,INT16U Flag);//
    INT8U EnCodeDevStatus(void);
    struct STEP GetSPIF(struct RealSPI_t DBSPIF);
    struct FAProcInfo_t GetFAProcInfo(struct FAProcInfo_t DBFAProcInfo);
    INT8U EditTestSPI(void);
    BOOL CheckNVA(void);
    void SendAllDataOnTime(void);
    
      
    void ProcXSFileSyn(void); //线损模块文件同步处理 CL 20180608
   
    INT16U EnCodeAllLastSoe(INT16U BeginNo);  
        
    //文件上传
    void ProcFileCall(void);
    void SendDir(void);
    void FileReady(void);
    void SectionReady(void);
    void SendSegment(void);
    void SendLastSegment(void);
    void SConfirm(void);
    void SendLastSection(void);
    void ReadDirList(void);

    void ReadDirList1(void);
    void ReadDirList2(void);
    void SendSegment1(void);
    void SendSegment2(void);

    void Sgcwronginf(INT16U enerrno);
    void ProDealF0(void);
    void ProDealF1(void);
    void ProDealF2(void);
    void ProDealF3(void);
    void ProDealF4(void);
    void ProDealF5();
    void ProDealF6();   	
	
#ifdef INCLUDE_DA
    //保护定值
    void SetProtect(void);
    void SendProtectCon(void);
    void CallProtect(void);
    void SendProtectData(void);
#endif

    void CheckUDataFlag(INT16U i,INT8U Flag);//检测其他设备是否有SOE COS
    void ClearMsg(void);    //在重新建立链路后，清理消息堆积。目前消息只有遥控，所以任何原因重新建立连接后，都不应去处理过时的遥控报文。
    
    /***********************************************/
    #ifdef SFILETRANAPP101
        
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

    //定时保存电度
    //#ifdef	SAVEKWH
    struct SAVEDATABUF *hisDataPtr ;
    INT16U HisDDCycle;
    BOOL ReadKWHHistoryData(struct AbsTime_t absTime,long *KWH,INT16U KWHNum);
    //#endif
public:
    enum DLSECSTATUS APP_DLSecStatus;	//链路层从动站状态,应用层标志  ll
    enum DLPRISTATUS APP_DLPriStatus;	//链路层启动站状态,应用层标志  ll
    INT8U  DLInitpreState;                    //AJ++180416 链路初始化过程状态缓存
    BOOL   DLInitFinishFlag;            //链路初始化过程结束标志  1-结束 0-未结束
    
    BOOL   GYKZ2015Flag;        //2015版101规约扩展特殊规定处理
    INT8U  SendCOS;
   
    struct PDevInfo *DevList;//设备信息
    INT16U LCFlag;
    INT32U SaveKWHTimerID;
    INT16U CotLocation;
    INT16U PubAddrLocation;
    BOOL TimeRightFlag;        //表明主站发来时间错误与否的变量
    CSecAppSev(INT16U AppID);
    ~CSecAppSev();
    struct PSec101Pad Sec101Pad;    //规约面板
    BOOL InitSecApp(void);
    void OnTimer(void);
    void SetUMsgFlag(void);
    void SetUDataFlag(void);
    void ProcXSFileSynFinish(void);//线损模块文件同步终止处理 CL 20180612
    //SecAppProc：链路接口函数
    //输入参数：bufin:输入的缓冲区数据地址,从类型标识开始，
    //输入参数：lengthin为应用层数据长度，
    //输入参数：dlcommand为链路层到应用层间的功能码
    //输出参数：bufout:输出的缓冲区数据地址，从类型标识开始，
    //输出参数：lengthout为应用层数据长度，
    //输出参数：appcommand为应用层到链路层的命令
    void SecAppProc(INT8U *bufin,INT16U lengthin,INT16U dlcommand,
                    INT8U* bufout,INT16U* lengthout,INT16U*appcommand);
    //初始化NVRAM历史电度区
    //#ifdef	SAVEKWH
    void InitHisDataBuf(void);
    BOOL SaveKWHToBuf(void);
    void RebootCheckUDataFlag(void);
    void PassSprValueToLink(INT16U *eparaI,INT16U *eparaII,INT16U *eparaIII,INT16U *eparaIV);
    //#endif
};

extern struct NameList *YcNameList;
extern struct NameList *YxNameList;
extern INT16U FirstFAyxInfoAdr;
extern struct ErrorInfo_t *ErrorBuf;

void new101sec(INT16U AppID);
extern INT8U *nvramMalloc(INT32U len);
extern void startCellMaint(void);
extern void ResetFaultInfoForCall_FaultCheck(void);
extern void getSysVersion(INT8U *buf);
extern void ResetFaultInfo(WORD FDIndex);
extern void fdFaultRstLook(void);

void BspYkRelease(void);
void ProgLogWrite2(INT8 *fmt,INT32 arg1,INT32 arg2,INT32 arg3,INT32 arg4, INT8U IsWriteSysinfo, INT8U type, INT8U status);

BOOL testGWFREvent(INT16U LogicDevId);
BOOL GWFREventRead(struct FaultRec_t *pfrevent, INT16U LogicDevId);
void GetTerminalPara(INT8U* buf,INT8U* len,INT16U addr,INT16U num);
extern void ProtectPara_LC_EachFeeder_W(INT8U index);
extern void VoltageTypePara_LC_EachFeeder_W(INT8U index,INT8U flag);
extern void WriteAdvancePara(INT8U *SourceAddr,INT8U index,INT16U ParaLength,INT8U flag);
extern void WriteCommunicationPara(INT8U *,INT16U,INT8U);
extern void WriteYcPara(INT8U *,INT16U,INT8U);
extern INT8U sizeofPara(INT8U);
extern void BspLCPanelDriver(INT8U control,INT8U command);
extern INT8U sizeofProtectLightStatus();
extern void ProtectLightStatusToASDU(INT8U *TxDataAdd);
extern INT8U sizeofInfoSummoned();
extern void InfoSummonedToASDU(INT8U *TxDataAdd);

extern void ParaToASDU(INT8U *,INT8U,INT8U);
INT16U SetTerminalPara(float value, INT16U addr);
void SaveTerminalPara(void);
void SaveRMTParaToSys(void);
INT16U GetRmtDeathvalue(INT8U ppty);
BOOLEAN GetSiQuChangeFlag(INT16U ParaFlag);
BOOL GetYKRYBState(void);
INT16U GetDBINum(void);
INT16U RmtParaChangeFloat2Word(INT16U info, float val);
void GetIDeadValuePara(INT8U* buf, INT8U* len, INT16U addr);


INT32U FT_GetDirID(struct FTFileTransfer_t *pFtinfo);
void ClearProgramUpdate(void);
void StartProgramUpdate(void);
INT8U Sgc1120aGetChipKeyVersion(INT8U *rcvbuf);
INT8U Sgc1120aGetChipSerialNumID(INT8U *rcvbuf);
INT8U Sgc1120aCalculateAuthRData(INT8U *rcvbuf);
INT8U Sgc1120aGetPKeyAuthData(INT8U Fid,INT8U *pdata,INT8U *rcvbuf);
int SGCOldPkeyUpdate(INT8U *pdata,INT8U *sdata);
int SGCOldSymkeyUpdate(INT8U *pdata);
INT8U Sgc1120aGetKeyConsultData(INT8U Fid,INT8U *pdata,INT8U *rcvbuf);
INT8U Sgc1120aGetRandomData(INT8U *rcvbuf);

//BOOL LogYkInfoRec(WORD DeviceID, WORD type, WORD dco, WORD infoaddr, WORD cot);

BOOL GXReadAllPara(INT8U *buf,INT8U *len,INT8U *sendnum,INT8U infosize,INT8U control);
BOOL GXReadCommonPara(INT8U *buf,INT8U *len,INT8U *sendnum,INT8U infosize);
BOOL GXReadJianGePara(INT8U *buf,INT8U *len,INT8U *sendnum,INT8U infosize,INT8U control);
INT16U SetTerminalParaGX(float value,INT16U addr);
void GXRemoteParaEffect();
INT16U GXParaSetCheck(float value,INT16U addr);
float GxGetParaValue(INT16U addr);
BOOL GxGetDeadValueFlag(INT16U flag);

INT16U L_GetETWptr(WORD DeviceID, WORD *pMaxNum);
INT16U GXGetChangeYxFlag(void);

INT16U GetFeederNum(void);
BOOL appGetJXStatus(void);
BOOL ReadRemoteParaSetEnableState(void);

#ifdef	__cplusplus
}
#endif

#endif
