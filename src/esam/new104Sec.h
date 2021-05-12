

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

/*����Ϊ6�¹�Լʵʩ�淶*/
#define SCHEDULE_FT_DIR_STD                0x00010000
#define SCHEDULE_FT_FILE_READY_STD         0x00020000 
#define SCHEDULE_FT_SECTION_READY_STD      0x00040000
#define SCHEDULE_FT_DATA_STD               0x00080000
#define SCHEDULE_FT_LAST_SECTION_FILE_STD  0x00100000    
#define SCHEDULE_XSFILESYNFINISH          0x00400000   //�ļ�ͬ����cl 20180314

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
    INT16U  *AItype;  //�������ݵ����ͣ�1-�з��� 0-�޷���
    INT8U *AIporperty;//ai��������
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
    INT8U TypeID;       //��ʾ�������ͣ���Ϊվ�ٻ��͵���������
    BOOL First;
    INT8U COT;          //���д���ԭ���������������л��Ƿ����ٻ�
    INT16U PubAddr;
    INT16U DevIndex;
    INT16U GroupNo;     //�����1~8Ϊң�ţ�9~14Ϊң�⡣���ݸ���ſ��Ʒ���ң�Ż���ң�⡣104ÿ128��ң��Ϊһ��
    INT32U InfoAddr;    //��Ϣ���ַ������ȷ���Ƿ�����ɡ�
    INT8U Description;
    INT8U  HaveSendDBI; //DBI���ͱ�־����������˫��ң������
    INT16U SoeStartPtr; //soe���ٻ��Ŀ�ʼָ�롣��������

};
#define STANDARDFILETRANS104 1

/*���������������ͨѶ�ϸ�λ������£���ʱ30����ûͨ��λ����ʱ�����wjr 2010.5.30*/
#define EV_SYSRET         0x00010000
#define MAXBREAKNUM     600         /*��·ͨ�������10����û���յ�����ding20100115*/

//#define SFILETRANAPP104    1     /*�����ļ��������*/ 
#ifdef SFILETRANAPP104
#undef STANDARDFILETRANS104              //�����������ļ������й�ͬ�����ͱ�ʶ�����������֮ǰ���ļ��������ʱ����׼�ļ�������̲������á�
#define CSFileTran      New104Sec       //��������Ŀ�ģ��ǰ�CSFileTran���滻��New104Sec�࣬�������FileTranApp.h�ļ��е���������һ�飬ʵ����û��ʹ��FileTranApp.h�ļ� ll

/***********************************************/

#define SFTYPECALLYC            192         /*�ٻ�ң�⣨��չ���*/
#define SFTYPESELECTFILE        193         /*ѡ���ļ�����չ���*/

/*����Ϊ�ļ�������չ���*/
#define SFTFYC                  192         /*��ʷң���ļ�*/
#define SFTFDD                  193         /*��ʷ����ļ�*/
#define SFTFMAX                 194         /*����ֵ�ļ�*/
#define SFTFMIN                 195         /*��Сֵ�ļ�*/


#define	SF_SC_NA        		122         /*�ٻ�Ŀ¼�pѡ���ļ��p�ٻ��ļ��p�ٻ���*/
#define	SF_AF_NA        		124         /*ȷ���ļ��pȷ�Ͻ�*/
#define	SFREQ       		    5           /*����ԭ�򣭣����������*/
#define	SFACTCON		        7 	        /*����ȷ��*/
#define	SFACTTERM		        10 	        /*�������*/
#define	SFILE               	13 	        /*����ԭ�򣭣��ļ�����*/
#define SFUNKNOWNTYPEID         44          /*����ԭ�򣭣�δ֪�����ͱ�ʶ*/
#define SFUNKNOWNCOT            45          /*����ԭ�򣭣�δ֪�Ĵ���ԭ��*/
#define SFUNKNOWNPUBADDR        46          /*����ԭ�򣭣�δ֪��Ӧ�÷������ݵ�Ԫ������ַ*/
#define SFUNKNOWNTINFOADDR      47          /*����ԭ�򣭣�δ֪����Ϣ�����ַ*/
#define SFLAI                   0x4001

#define SFFILEHEADLEN           17          /*�ļ�ͷ����*/
#define SFSectionLen            64000       /*�ڳ���*/


enum FileStatus {FileOver=0,CallDir,SelectFile,CallFile,CallSection,
                 SendSegment,LastSegment,LastSection,LastFile};
enum CallYcStatus {CYNouse=0, CYAck, CYCall, CYEnd, CYError};

struct SFileInfo_t{         /*������Ϣ*/
    enum FileStatus FileStep;
    
    INT8U   RxID;
    INT8U   RxVsq;
    INT16U  RxCot;
    INT16U  RxPubAddr;
    INT32U  RxInfoAddr;
    
    INT32U  RxInfoAddrEnd;
    struct Iec101ClockTime_t    RxTimeStart;
    struct Iec101ClockTime_t    RxTimeEnd;
    
    INT16U  FileName;       /*�ļ���*/
    INT8U   SectionName;    /*����*/    
    
    /*����Ϊstruct FileOPData*/
    INT16U  FDataPer;           /*���ݵ�ʱ��������λ�����ӣ�*/
    INT16U  FDataNum;           /*��ϢԪ����*/
    INT32U  FDataClock;         /*��ʼ����ʱ��*/

    INT32U  FileLen;
    INT32U  SectLen;
    
    INT32U  FileCur;
    INT32U  SectCur;
    INT8U   FileChs;
    INT8U   SectChs;
    
    /*�ٻ�ң��*/
    INT16U  AllYCNum;
    enum CallYcStatus   CallYcStep;
    INT8U   RxYcLen;
    INT8U   RxYcData[250];
    INT8U   InfoNum;
    INT16U  InfoAddr[127];  /*֧��ң���0��65535*/
};

#endif  //end of #ifdef SFILETRANAPP104
/***********************************************/

#ifdef STANDARDFILETRANS104
enum FileStatus {FileOver=0,CallDir,SelectFile,CallFile,CallSection,
                 SendSegment,LastSegment,LastSection,LastFile};
enum AckStatus{NOTUSED=0,ACKFILEPOS,ACKFILENEG,ACKSECTPOS,ACKSECTNEG};  
#define	SFREQ       		    5           /*����ԭ�򣭣����������*/  
#define	SFILE               	13 	        /*����ԭ�򣭣��ļ�����*/             
struct StdFileInfo_t{         /*������Ϣ*/
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
    
    INT16U  FileName;       /*�ļ���*/
    INT8U   SectionName;    /*����*/    
    INT8U   SectionNum;
    
    /*����Ϊstruct FileOPData*/
    INT16U  FDataPer;           /*���ݵ�ʱ��������λ�����ӣ�*/
    INT16U  FDataNum;           /*��ϢԪ����*/
    INT32U  FDataClock;         /*��ʼ����ʱ��*/

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

    /*Ϊ���������·�����������ĸĶ� wjr 2010.5.21*/
    INT16U LinkConnect;       /*����ͨ�ϵı�־  wjr  2010.5.21*/
    INT16U LinkBreakCounter;  /*��·ͨ�������û���յ����ݵļ����� wjr  2010.5.21*/
    
    INT8U *RxMsg;
    INT8U *TxMsg;
    INT8U DBData[2048];//�����ݿ�ȡ����ʱʹ�õ���ʱ������
    
    UINT16  LBIinfoaddr;  //����ң����Ϣ���ַ       2008.11.5
    UINT16  LDBIinfoaddr; //˫��ң����Ϣ���ַ       2008.11.5
    
    struct BIEWithTimeData_t DBIDBData[100];        //�����ݿ�ȡ����ʱʹ�õ���ʱ������   wjr
    struct BIEWithoutTimeData_t DBICOSDBData[100];  //�����ݿ�ȡ����ʱʹ�õ���ʱ������   wjr
    INT16U DBISOEnum;   //�յ���˫��ң�ŵ�soe��
    INT16U DBICOSnum;   //�յ���˫��ң�ŵ�cos��
    INT16U DBIDevIndex;    //��Ҫ����˫��ң��soe���豸��
    INT16U DBICOSDevIndex;    //��Ҫ����˫��ң��cos���豸��

    
    INT16U AsduHeadLength;//ASDUͷ���ȣ����ͱ�־����Ϣ���ַ
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

    INT8U InitFlag;             // ��ʼ����־
    INT8U YKTypeID;
    
    BOOL   FirstCallAllData;   //��һ�����в�����Ϲ��ܣ�0xff-�Ѿ����й���0-δ���й�����1�����н������ٷ����������ݡ�
    INT16U WaitCallAllDelay;
    
    BOOL GYKZ2015Flag;        //2015��104��Լ��չ����涨����
    INT16U SendCOS;         //�ڹ�ѡ2015��չ��ʱ�������Ҫ����cos����
    //BOOL RMTParaGHSimple;       //debug 2017-3-1 Զ�̲����̻����̲����Я���Ĳ�����
    //BOOL HisFileTxt;            //debug ��ʷ�����ı���ʽ
    
    INT16U RMTHaveReadParaFlag;     //���ֶ�ȡʱ����Ϊ�Ѷ�ȡ��š�ȫ����ȡʱ��Ϊ��ȡλ�ñ��
    BOOL   RMTParaReadAllFlag;      //��ȫ��������־
    INT16U RMTSectionNo;            //��ʱ����һ����Ӧ����ǰ���š�
    INT16U RMTSectionNo2;
    INT16U RMTParaNum;              //����д�ĸ���
    INT16U RMTParaInfo[RMT_RW_MAXNUM];
    float  RMTParaValue[RMT_RW_MAXNUM];
    INT16U RMTTimeOut;                 //Ԥ�ñ�־��ʱ����
    BOOL   RMTParaYZ;               //Ԥ�ñ�־
    
    //ң����˫��ң�ŷ���-�㶫��ɽ
    BOOL  bSendAllDBI;
    
    //����Զ����ά
    INT8U Roi;
    INT8U Qpa;
    INT16U GXTimeOut;
    BOOL   GXParaYZ;               //����Ԥ�ñ�־
    INT16U GXParaNum;              //����д�ĸ���
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
    
    /****NEW�ļ�����START***********/
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
    
    void ProcFileSyn(void);//����ģ��2018��׼���ļ�ͬ����cl 20180314
    void SendFreezeEvent2Pri101(void);//��101��վ������Ϣ����˲ʱ����
    
    /****NEW�ļ�����END***********/
    
    
    
    /****6�·�NEW�ļ�����START***********/
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
    /****6�·�NEW�ļ�����END***********/
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
    
    void EnCodeNACK(INT16U Cot);//�񶨻ش�ӿں���  CL
    void ProcInitEnd(void); //��ʼ������֡������ʼ������ CL

    BOOL EnCodeNVA(void);
    BOOL EnCodeSOE(void);
    BOOL EnCodeBIENT(void);
    void EnCodeDBIENT(void);  //�༭˫��ң��COS   
    void EnCodeDBISOE(void); //�༭˫��ң��SOE  
    INT8U EnCodeCounter(INT16U BeginNo,INT16U EndNo,INT16U *pNum);
    void EnCodeReadData(void);

    void EnCodeGroupEnd(void);
    
    
    BOOL WritePara(void);
    void EnCodeClock(INT8U flag);
    BOOL EnCodeInitEnd(void);               //wjr��ʼ������
    void ProcXsFileSynFinish(void);
    
    /*****�㶫Զ�̲���*******/
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

    INT16U *LinkFlag;       /* 2010.5.30  ��ͨѶͨ��֮��6����û�����ݺ�Ͽ��������������֮����Сʱ֮��û�����Ͼ���Ϊ����û�лظ�����������*/
    
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




