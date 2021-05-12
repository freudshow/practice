#ifndef	_SECDLINK_H
#define _SECDLINK_H

#include "iec101def.h"
#include "secapp.h"

#ifdef	__cplusplus
extern "C"{
#endif

class	CSecDLink
{
private:
    enum DLSECSTATUS DLSecStatus;	//链路层从动站状态
    enum DLPRISTATUS DLPriStatus;	//链路层启动站状态
    enum DLTXDSTATUS TxdStatus;	//发送状态
    enum DLRXDSTATUS RxdStatus;	//接收状态

    INT32U baudrate;
    INT32U TimeOutValue;
    INT32U dwAppTID;
    INT32U wAppID;

    INT16U SourceNo;	//源地址——即本机地址
    INT8U RlaConCode;	//接收的控制码
    //INT8U FCBNoTurnNum;  //FCB未翻转计数
    BOOL FirstRecFCB;   //wjr 2009.6.3   链路复位标记，来标识链路复位后第一次收到对方的带FCB位的报文，并以对方的FCB起始位为起始标志
    INT8U TlaConCode;	//发送的控制码
    INT16U RetryFlag;	//重试标志
    INT16U wChanNo;	//通道号
    INT16U RetryCount;	//重试次数
    INT8U LastControl;
    BOOL HaveNextFrame;

    INT16U FrameHead;	//组帧时数据头在发送缓冲区的位置
    INT16U TxdHead;	//
    INT16U TxdTail;	//

    INT16U FrameHead_Pri;	//记录上帧启动站报文的数据头位置，重发时用
    INT16U TxdHead_Pri;
    INT16U TxdTail_Pri;

    INT16U FrameHead_Sec;	//记录上帧从动站报文的数据头位置，重发时用
    INT16U TxdHead_Sec;
    INT16U TxdTail_Sec;

    INT16U RxdHead;	//处理数据的头指针
    INT16U RxdTail;
    INT16U RxdLength;

    

    INT32U TimeOutTick;			//超时记数器
    INT32U TimeOutTick_Pri;		//启动站超时记数器
    INT32U TimeOutTick_Sec;		//从动超时记数器
    INT32U TimeOutTickCopy;		//超时记数器拷贝

    BOOL FlagData1;			//一级数据标志
    //INT8U ReqData;	//ACD=1时的临时记录。
    BOOL ScanFlag;

    //INT8U StartDL;	//开始链路过程，平衡模式判断链路过程是否结束的标志
    //INT8U RemoteDLOK;	//复位远方链路成功，平衡模式
    //INT8U LocalDLOK;	//用于当收到对方链路复位命令后，是否发送复位命令的标志 0xff-发送 0-不发送
    BOOL IsSendLinkInitCmd;         //是否主动（链路断和开机）发送过链路初始化命令 FC9 0-没有发送过  1-发送过，当检测到链路断后置位该标志
                                    //用于当收到对方链路复位命令后，是否发送复位命令的标志
    //INT8U  DoubleDlResetFlag;   //双向链路建立标志

    INT32U DLIdleTime;	//链路空闲时间，链路空闲超过这个时间，每1秒钟链路查询应用层一次，为了一级数据任务的及时处理
    INT32U IdleTimeCount;

    INT8U EbMsgRxdBuf[FRAMEBUFSIZE];
    INT16U EbMsgRxdHead;	//处理EB格式数据的头指针
    INT16U EbMsgRxdTail;
    INT8U EbMsgTxdBuf[2*LPDUSIZE];
    //INT16U EbMsgRxdLength;

    INT8U RxdBuf[FRAMEBUFSIZE];	        //接收缓冲区
    INT8U TxdBuf[2*LPDUSIZE];			//发送缓冲区
    INT8U TxdBuf_Pri[2*LPDUSIZE];		//发送缓冲区 ，暂存启动站发送的数据，重发时用
    INT8U TxdBuf_Sec[2*LPDUSIZE];		//发送缓冲区 ，暂存从动站发送的数据，重发时用
    INT8U IEC_DIR;	                    //方向位，平衡式有效，非平衡=0
    INT8U N101Encrptystyle;            //是否支持最新安全加密方案zhangliang 
	BOOL  IsDoubleLinkInit;             //平衡模式下，是否支持双向链路同步初始化 1-支持 0-不支持.(单向建立链接时，任一方向链接建立成功均成功)
    //INT32U rc;
    BOOL  IsSendE5;                     //当无数据时，是否发送E5，0-发送  1-不发送
    BOOL  IsEncrypt;                    //是否支持加密
    BOOL  NoJudgeFCB;                 //不判断FCB翻转
    
    INT16U HeartBeatIdleTime;           //心跳空闲时间
    INT16U HeartBeatIdleLimit;          //心跳发送间隔，单位秒

    CSecAppSev *pSecApp;	            //应用层指针

    //应用层服务调用需要的参数

    INT16U AppCommand;
    INT16U DLCommand;
    INT16U LengthIn;
    INT16U LengthOut;

    //
    INT16U FixFrmLength;//固定帧长度
    //INT16U AsduHeadLength;//ASDU头长度，类型标志到信息体地址
    INT16U LinkAddrSize;
    
    /*为解决101进入死循环所做的改动 ll 2014.8.11*/
    INT16U ELResetNum;              /*收到复位链路次数 */
    INT16U ELClearTime;             /*清0复位链路次数时间 */
    INT16U ELNoReplyTime;
    BOOL   ELReplyFlag;             //双向建立链接时是否恢复标志，在进入死循环后，该标志置0
    void EndlessLoopMonitor(void);
    void EndlessLoopInit(void);
    
    void SendTestOnTime(void);
    
    /*链路层接收启动站命令函数*/
    void RecResetDL(void);		//复位链路
    void RecReqDLStatus(void);		//请求链路状态
    void RecMISITail(void);	//？？

    /*链路层接收从动站命令函数*/
    void RecConf10(INT8U Control);		//确认
    void RecDLSta(INT8U Control);			//链路状态
    void RecTestDL(void);       //链路测试命令
        
    /*链路层发送启动站命令函数*/
    void EditE5(void);
    void EditFra10(INT8U Function);
    void EditFra68(INT8U Function,INT16U FrameLength);
    void TimeOutFun(INT8U FailType,INT8U Prm);

    /*数据接收处理*/
    BOOL ExeDLFun10(void);	//检测10帧的正确性
    BOOL ExeDLFun68(void);	//检测68帧的正确性
    void ExeDLFunCode10(void);		//处理10命令
    void ExeDLFunCode68(void);		//处理68命令
    void ExeE5(void);

    /*链路层发送从动站命令函数*/
    void EditSecFra10(INT8U Function);
    void EditSecFra68(INT8U Function,INT16U FrameLength);

    //规约切换处理
    BOOL ExeMaint(void);		//检测切换命令
    void EditMaintCon(unsigned int station);	//发送确认

    INT8U CheckSum(INT8U *);	//校验

    //环路延时计算
    INT32U TimeDelay(INT16U i);


    //重发启动站缓冲区数据
    void SendPriDataToMISI(void);

    //重发从动站缓冲区数据
    void SendSecDataToMISI(void);

    void DLSendProc(void);
public:
    INT8U BalanMode;			//平衡式=1 非平衡式=0

    INT16U En_LinkAddrSize;
    INT16U En_CotSize;
    INT16U En_PubAddrSize;
    INT16U En_InfoAddrSize;
    CSecDLink(INT16U AppID,CSecAppSev *p);
    ~CSecDLink();
    BOOL InitSecDLink(void);

    //检测有效数据帧
    void SearchFrame(void);

    //读MISI数据
    void RecMISIData(void);
    //规约通讯数据处理(明文、168号文、国网安全加密、湖南农网安全加密)
	void RecMISIDataDealFun(INT8U Enflag);
	//发送缓冲区中的数据，写MISI接口
    BOOL SendDataToMISI(void);
    //规约通讯数据发送前处理(明文、168号文、国网安全加密、湖南农网安全加密)
	BOOL SendMISIDataDealFun(INT8U Enflag);
    //平衡模式处理后续数据
    void SendDataEnd(void);
    void NotifyToAppSchedule(void);
    //超时处理函数
    void TimeOut(void);

    //启动链路过程
    void CallDLStatus(void);

    //链路状态检测
    BOOL DLStatusCheck(void){return (DLPriStatus==PRIENABLE);}

    void CallUData(void);//取紧急数据
    void CallUMsg(void);//取遥控消息
    
    BOOL ExeDLFun16(void);
    void ExeDLFunCode16(void);
    void SqrLongFraFunI(INT8U *ori,INT8U *Bakbuf,INT8U *infodataptr,INT8U ctoalen,INT8U num,INT8U infolen,INT8U count,INT8U Sq) ;
    INT8U SqrLongFraFun(INT8U *ori,INT8U *Bakbuf);

};

/*为解决101进入死循环所做的改动 */
#define EL_MAX_RECEIVE_RESETNUM     5
#define EL_CLEARRESETNUM_TIME       3*60
#define EL_NOREPLY_TIME             3*60

#ifdef INCLUDE_ENCRYPT
    extern int myEnctyptInit(unsigned long addr, unsigned short timeout);
    extern INT8U CheckEncryptFrame(unsigned char *pdata, unsigned char *psign, unsigned char len);
    extern char CheckEncryptFramHead(unsigned char *pdata, unsigned char rcvlen);
    extern void ProcEncryptFrame(unsigned char *pdata, unsigned char *poutdata, unsigned char *poutlen);
    extern void test30s(void);
#endif

#ifdef PRO101_ADDR_STUDY
    extern void StudyAddrSetAddr(INT16U addr, INT16U devid);
#endif
INT16U EbSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo);
void PackFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf);
void PackFra68ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo);
INT16U Pack101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
void EbErrCodeSend(INT16U SW,INT8U TypeId,INT16U wChanNo);
void saveRecord(BYTE *buf,WORD len,WORD mode, WORD flag);
INT8U UpgradeDataVerify(INT16U wChanNo);
INT8U CheckEncrptchip(INT8U CheckType);
INT16U Eb1120aSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo);
INT16U Pack1120aFor101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
INT8U Check1120aEncrptchip(INT8U CheckType);
INT8U EncrptyChiptest(INT8U type);
void Packf68ToOld1120aEn(INT8U *oribuf);

#ifdef	__cplusplus
}
#endif

#endif
