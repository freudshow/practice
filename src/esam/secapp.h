#ifndef	_SECAPP_H
#define _SECAPP_H

#include "iec101def.h"
#include "..\newfiletran\FileTranNew.h"

#ifdef	__cplusplus
extern "C"{
#endif

//#define INFOADDR2BYTE       1   //�е���Ϣ���ַ��2�ֽ�

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
#define DATA2_XSFT_SYNACT        0x01000000     //�����ļ�ͬ������  CL 20180607
#define DATA2_XSFT_SYNACTFINISH  0x02000000     //�����ļ�ͳ�Ƽ�����ֹ
#define DATA2_RMT_READPARA_GD    0X04000000     //�㶫������
#define DATA2_RMT_WRITEPARA_GD   0X08000000     //�㶫д����

#define BIETFRAME   0x01
#define BIENTFRAME  0x02
#define FAPROCFRAME 0x04

#define WAIT_CALLALL_DELAY  10      /*��1�����н�ֹ����ϣ�����ƽ��ģʽ����������Ҫ��*/

/*Һ����Ŀ����궨��,����4����չ���ͱ�ʶ*/
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
{  //�仯ң��ṹ
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
    BOOL type;  //1-�з��� 0-�޷���
};


struct PDevData
{  //�豸�ɼ�����Ŀ
    INT16U AINum;
    INT16U BINum;
    INT16U DBINum;             //wjr
    INT16U CounterNum;
    INT16U BONum;
    INT16U SPINum;
    INT16U BCDNum;
    INT16U NvaNo;
    INT32U *LastCounterData;
    INT32U *HisCounterData;//������ʷ���
    INT16U  HisDDReadPtr;
    struct RealCounter_t *CounterData;
    struct PNva	*AIData;
    short *AIMaxVal;
    struct YcPara *AIPara;
        
};

struct PData1
{   //һ�����ݽṹ
    INT32U Flag;
    BOOL COT;
    INT16U SOENum;      //���͵�SOE��Ŀ
    INT16U BIENTNum;    //���͵ı�λYX��Ŀ
    INT16U FAProcNum;   //���͵�FA��Ŀ
    INT16U PubAddr;
    INT16U InfoAddr;	//��Ϣ���ַ����2

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
    struct PData1 Data1;//zzw2004/6/7�����ڼ�¼ÿ���豸��1��������Ϣ��
};

struct PGroupTrn
{
    INT8U TypeID;
    BOOL First;
    INT8U COT;
    INT16U PubAddr;
    INT16U DevIndex;//���鷢�͵ĵ�ǰ�豸���
    INT16U GroupNo;
    INT16U InfoAddr;	//��Ϣ���ַ����2
    INT8U Description;
    INT8U  HaveSendDBI; //DBI���ͱ�־����������˫��ң������
    INT16U SoeStartPtr; //soe���ٻ��Ŀ�ʼָ�롣��������
 };

extern struct config_t MyConfig;

//�����ļ���ؽṹ��
extern struct XSFileSynInfo_t XSFileSynInfo;

//��ʱ������ʷ���
#define TIMETOSAVEDATA	60   //ʱ����
#define SAVENUM	(24*30)//������ʷ��ȵ�ʱ�����

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

#define SFILETRANAPP101  1       /*�����ļ��������*/

#ifdef SFILETRANAPP101
#define CSFileTran      CSecAppSev


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
    INT8U InitFlag;     //��ʼ���Ƿ��͹���־��0xff��ʾδ���͹��� 0��ʾ���͹���

    //Ӧ�ò������ô���Ĳ���
    INT16U *AppCommand;
    INT16U DLCommand;
    INT16U LengthIn;
    INT16U *LengthOut;
    INT8U *RxMsg;	//Ӧ�ò��������ָ��
    INT8U *TxMsg;	//Ӧ�ò��������ָ��

    BOOL DDFreeze;       //wjr 

    INT8U DBData[2048];//�����ݿ�ȡ����ʱʹ�õ���ʱ������
    
    struct BIEWithTimeData_t DBIDBData[100];//�����ݿ�ȡ����ʱʹ�õ���ʱ������   wjr
    struct BIEWithoutTimeData_t DBICOSDBData[100];//�����ݿ�ȡ����ʱʹ�õ���ʱ������   wjr
    INT16U DBISOEnum;       //�յ���˫��ң�ŵ�soe��
    INT16U DBICOSnum;       //�յ���˫��ң�ŵ�cos��
    INT16U DBIDevIndex;     //��Ҫ����˫��ң��soe���豸��
    INT16U DBICOSDevIndex;  //��Ҫ����˫��ң��cos���豸��
    INT8U  IsDBISoeSend;   //�Ƿ���DBIsoe

    struct PData1 Data1;
    struct PGroupTrn GroupTrn;
    struct PGroupTrn LastGroupTrn;
    struct PGroupTrn GroupTrnDD;

    INT32U Data2Flag;

    UINT16  LBIinfoaddr;  //����ң����Ϣ���ַ       2008.11.5
    UINT16  LDBIinfoaddr; //˫��ң����Ϣ���ַ       2008.11.5

    INT16U DevCount;//������豸��Ŀ
    INT16U ActDevIndex;//
    INT16U LastDevIndex;//��¼���һ�η��ͱ�λYX��SOE��FA�����豸��š�
    INT16U BODevIndex;//��¼ң��������豸���
    //struct PDevInfo *DevList;//�豸��Ϣ
    INT16U NvaActDevNo;//���ͱ仯ң��ĵ�ǰ�豸��š�

    INT8U *YCGroupNo;   //��������ÿ��ң�����ţ��Ա�������ͱ�ʶ
    INT8U *YXGroupNo;   //��������ÿ��ң�ŵ���ţ��Ա�������ͱ�ʶ

    //enum PMASTERUseStatus MasterStatus;//zzw
    
    INT16U RestDLDelay;                 //������·������ɺ���ʱ30��Ӷ���δ������·��������Ӧ�ò㱨�� 
    
    BOOL   IsAllSendInitEnd;            //�Ƿ�ÿ���������Ӻ��ͳ�ʼ������֡
            
    BOOL   FirstCallAllData;              //��һ�����в�����Ϲ��ܣ�������ƽ��ģʽ��0xff-�Ѿ����й���0-δ���й����ն��յ��Է���·��λ�󣬸ñ�־Ϊ0����1�����н������ٷ����������ݡ�
    INT16U WaitCallAllDelay;
    
    //ң����˫��ң�ŷ���-�㶫��ɽ
    BOOL  bSendAllDBI;
    
    INT16U RDPubAddr;
    INT16U RDInfoAddr;

    INT16U ResetPubAddr;
    INT16U ResetInfoAddr;
    INT8U ResetGRP;
    INT16U ResetCount;
    INT8U ResetFlag;
   
    //�������ϸ���
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

    //����ң����ʱ��Ϣ
    INT8U YKTypeID;
    enum {NOTUSE=0,YKERROR,YKSETCON,YKEXECON,YKCANCELCON,YKTERM}YKStatus;
    INT16U YkStatusForTest; //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24
    INT8U DcoTemp;
    INT8U ScoTemp;
    //struct PDBBO PDBBOTemp;
    INT16U SwitchNoTemp;

    INT16U SDTTime;//�洢��վ���͵�SDTʱ��
    INT16U TrTime; //��վ�ӽ��յ�������ʱ��������ʱ����
    INT16U SendTime;//=SDTTime+TrTime
    INT16U TimeDelay;
    struct AbsTime_t SecSysTimeR;//��վ�յ�C_CD_NAʱ��ϵͳʱ��
    struct AbsTime_t SecSysTimeT;//��վ����M_CD_NAʱ��ϵͳʱ��
    struct AbsTime_t OldSysTime;//��վ����ʱ��ǰ��ϵͳʱ��

    INT8U EditDDCon;
    struct AbsTime_t CounterTime;//�����ȵ�ʱ��

    INT16U BackScanTime;//�֣���������ɨ����
    INT16U CycScanTime;//�룬����ѭ������ɨ����
    INT16U BackScanCount;
    INT16U CycScanCount;

    INT8U ReadTimeFlag;

    //���������
    BOOL HaveWrongData;
    INT16U WrongDataLength;
    INT8U WrongData[256];

    enum PFrmState Status;
    enum PFrmState LastStatus;
    INT8U BIFrame;
    INT8U EditAllDataCon;
    
    INT8U EditReadParaCon;
    INT8U ActiveParaCon;    //Զ�̲���������Ʋ�����1-��ȷ��֡ 2-�����ظ� 3-�������Ͳ���
    

    struct PDevInfo *pDev;
    enum PFrmState LastFrame;

    //�ļ��ϴ�;һ���ļ�����һ����������
    INT32U FileReadPtr;
    INT16U CurrentFileName;
    INT32U CurrentFileSize;
    INT16U CurrentInfoAddr;
    INT16U CurrentZBNo;
    struct DIRUNIT DirUnit;
    enum PFileStatus FileStep;
    INT8U FileCheckSum;

    //������ֵ
    INT8U ProtectValue[33];//���һ���ֽڣ����������ɹ���־

    //��ʷ��ȱ���ʱ��
    struct Iec101ClockTime_t HisDDTime;
    enum {Start=0,EditCon,SendData,SendOver}HisDDStatus;

    BOOL SetDevInfo(INT16U devid);
    void InitPara(void);
    void CheckPad(void);
    void SetDefaultPad(void);
    void ReadAIMaxVal(INT16U i);

    //��ʷ���
    INT16U HisDDDevNo;//��ǰ��ʷ����豸���

    //
    INT16U FixFrmLength;//�̶�֡����
    INT16U AsduHeadLength;//ASDUͷ���ȣ����ͱ�־����Ϣ���ַ
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

    short Data[150];//¼�������ݴ�
    
    INT8U *pRestType;   //��λ��Լ����״̬���� ll 2010/07/20   for ������Լ����
    
    
    
    INT16U RMTHaveReadParaFlag;     //���ֶ�ȡʱ����Ϊ�Ѷ�ȡ��š�ȫ����ȡʱ��Ϊ��ȡλ�ñ��
    BOOL   RMTParaReadAllFlag;      //��ȫ��������־
    INT16U RMTSectionNo;            //��ʱ����һ����Ӧ����ǰ���š�
    INT16U RMTParaNum;              //����д�ĸ���
    INT16U RMTParaInfo[RMT_RW_MAXNUM];
    float RMTParaValue[RMT_RW_MAXNUM];
    INT16U RMTTimeOut;              //Ԥ�ñ�־��ʱ����
    BOOL   RMTParaYZ;               //Ԥ�ñ�־
    INT8U  RMTReturnCot;            //��¼���صĴ���ԭ��
    INT8U  RMTReturnVsq;            //��¼����ʹ�õ�vsq
    
    INT8U  ProgramUpadateCot;      //������������COT
    INT8U  ProgramUpadateSE;
    
    //����Զ����ά
    INT8U Roi;
    INT8U Qpa;
    INT16U GXTimeOut;
    BOOL   GXParaYZ;               //����Ԥ�ñ�־
    INT16U GXParaNum;              //����д�ĸ���
    BOOL  GXvsqflag;
    INT8U GXReturnCot;
    INT8U GXParaControl;
    INT16U GXParaInfo[GX_RW_MAXNUM];
    float GXParaValue[GX_RW_MAXNUM];
    
    /****NEW�ļ�����START***********/
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
    
    /****NEW�ļ�����END***********/
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
    
    void ProcEncodeXSFileSynConf(void);//�����ļ�ͬ����ȷ����ظ�
    void ProcEncodeXSFileSynFinish(void);//�����ļ�ͬ����ȷ����ظ�          

    void ProcData1(void);
    void ProcData2(void);
    void ProcControl(void);  //����ң��
    void ProcSetNVA(void);
    void ProcLCdataCall(void);
    void ProcAllDataCall(void);
    void ProcDDCall(void);
    void ProcTimeDelay(void);
    void ProcClock(BOOL Conf); //�������
    void ProcReset(void);
    void ProcTest(void);
    void ProcReadData(void);
    void ProcParaSet(void);
    void EnCodeInitEnd(void);
    void ProcTaskMsg(void);//����ң�ط�У��Ϣ
    BOOL EnCodeCtrlRet(void);  //ң�ط�У
    void EditYKTerm(void);//ң�ؽ���
    BOOL GetActDevIndexByAddr(INT16U Addr);
    BOOL EnCodeBIENT(void);  //�༭COS  ��������Ҫ���ֵ���˫��yx
    void EnCodeDBIENT(void);  //�༭˫��ң��COS   wjr
    BOOL EnCodeSOE(void); //�༭SOE //��ʱ������˫��ң��
    void EnCodeDBISOE(void); //�༭˫��ң��SOE  wjr
    void EnCodeAllDataConf(void);//���ٻ�ȷ��֡
    BOOL ProcAllData(void); //����ȫ����
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
    void EnCodeGroupEnd(void);//����֡
    void EnCodeDDGroupEnd(void);//����֡
    BOOL GetNextDev(void); //�õ���һ���豸
    BOOL GetNextDDDev(void); //�õ���һ���豸
    BOOL EnCodeNVA(void);  //�༭�仯ң������;
    void FreezeCounter(void);
    void EnCodeCounterConf(void);//
    void ProcCounter(void) ;//������zzw
    INT8U EnCodeCounter(INT16U BeginNo,INT16U EndNo,INT16U *pNum);//�����ݿ�ȡ��ȣ�����󣬷�����·�㡣
    void EnCodeTimeDelay(void);
    void EnCodeClock(void);//������վ����ʱ��ǰ��ϵͳʱ��
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
    
      
    void ProcXSFileSyn(void); //����ģ���ļ�ͬ������ CL 20180608
   
    INT16U EnCodeAllLastSoe(INT16U BeginNo);  
        
    //�ļ��ϴ�
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
    //������ֵ
    void SetProtect(void);
    void SendProtectCon(void);
    void CallProtect(void);
    void SendProtectData(void);
#endif

    void CheckUDataFlag(INT16U i,INT8U Flag);//��������豸�Ƿ���SOE COS
    void ClearMsg(void);    //�����½�����·��������Ϣ�ѻ���Ŀǰ��Ϣֻ��ң�أ������κ�ԭ�����½������Ӻ󣬶���Ӧȥ�����ʱ��ң�ر��ġ�
    
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

    //��ʱ������
    //#ifdef	SAVEKWH
    struct SAVEDATABUF *hisDataPtr ;
    INT16U HisDDCycle;
    BOOL ReadKWHHistoryData(struct AbsTime_t absTime,long *KWH,INT16U KWHNum);
    //#endif
public:
    enum DLSECSTATUS APP_DLSecStatus;	//��·��Ӷ�վ״̬,Ӧ�ò��־  ll
    enum DLPRISTATUS APP_DLPriStatus;	//��·������վ״̬,Ӧ�ò��־  ll
    INT8U  DLInitpreState;                    //AJ++180416 ��·��ʼ������״̬����
    BOOL   DLInitFinishFlag;            //��·��ʼ�����̽�����־  1-���� 0-δ����
    
    BOOL   GYKZ2015Flag;        //2015��101��Լ��չ����涨����
    INT8U  SendCOS;
   
    struct PDevInfo *DevList;//�豸��Ϣ
    INT16U LCFlag;
    INT32U SaveKWHTimerID;
    INT16U CotLocation;
    INT16U PubAddrLocation;
    BOOL TimeRightFlag;        //������վ����ʱ��������ı���
    CSecAppSev(INT16U AppID);
    ~CSecAppSev();
    struct PSec101Pad Sec101Pad;    //��Լ���
    BOOL InitSecApp(void);
    void OnTimer(void);
    void SetUMsgFlag(void);
    void SetUDataFlag(void);
    void ProcXSFileSynFinish(void);//����ģ���ļ�ͬ����ֹ���� CL 20180612
    //SecAppProc����·�ӿں���
    //���������bufin:����Ļ��������ݵ�ַ,�����ͱ�ʶ��ʼ��
    //���������lengthinΪӦ�ò����ݳ��ȣ�
    //���������dlcommandΪ��·�㵽Ӧ�ò��Ĺ�����
    //���������bufout:����Ļ��������ݵ�ַ�������ͱ�ʶ��ʼ��
    //���������lengthoutΪӦ�ò����ݳ��ȣ�
    //���������appcommandΪӦ�ò㵽��·�������
    void SecAppProc(INT8U *bufin,INT16U lengthin,INT16U dlcommand,
                    INT8U* bufout,INT16U* lengthout,INT16U*appcommand);
    //��ʼ��NVRAM��ʷ�����
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
