#ifndef _NEW104DLINK_H
#define _NEW104DLINK_H

#include "new104def.h"
#include "encrypt.h"

#ifdef	__cplusplus
extern "C"{
#endif

#define MAXRXLEN APCILEN

enum PRxState{Start=1,Head,Continue};
enum PTxState{Send=1,Wait};
enum PDARet{DA_NULL=1,DA_NODATA,DA_SUCCESS};
enum PTxUnitState{NoUse=1,WaitSend,WaitAck};

struct PTick
{
    BOOL IsUse;
    INT16U Count;
    INT16U Value;
};

struct PTxBufUnit
{
    PTxUnitState State;
    INT8U Priority;
    INT8U TxHead;
    INT8U TxTail;
    INT8U *BufStart;
};



class New104DataLink
{
private:
    struct PMySelf *pMySelf;
    struct PTxBufUnit TxBufUnit[K+2];

    INT16U RxHead;
    INT16U RxTail;

    INT16U EBRxHead;
    INT16U EBRxTail;
    
    INT16U TxHead;
    INT16U TxTail;
    INT8U *TxStart;
    short CurTxUnit;
    BOOL FrameTailFlag;
    INT16U NextFrmHeadPos;

    enum PRxState RxState;
    enum PTxState TxState;

    struct PAPCI apci;
    INT8U *pAPCI;

    INT16U PeerNoAckNum;
    INT16U RxFrmNum;
    INT32U *pScheduleFlag;
    
    
    void BeginDT(void);
    void StopTick(INT8U i);
    void CloseTCP(void);
    void ProcUFrame(void);
    BOOL ProcPeerNoAckNum(INT16U PeerNR);
    void BeginTick(INT8U i);
    void FindTxUnitToSend(void);
    void SendToTeam(void);
	void SendToTeamEnDealFun(INT8U Enflag);
public:
	INT16U NR;
    INT16U NS;
    INT8U *RxBuf;
    INT8U *TxBuf;

    INT8U *EBRxBuf;//ZHANGLIANG
    INT8U *EBTxBuf;
	
    BOOL CommConnect;
    struct PTick Tick[4];
    
    INT16U YkStatusForTest2;    //ll 为广州测试临时修改 2012-3-24
    
    INT16U NoJudgeFramNo;       //1-不判帧序号 
    INT16U RsvStartClearRSno;   //收到start帧收发序号清0
    
    
    //通讯加密相关
    BOOL  IsEncrypt;                    //是否支持加密 ll
	INT8U N104Encrptystyle;
	INT8U EncryptBuf[255];
    void ExeDLFunCode16(void);    
    
    New104DataLink(struct PMySelf *pMySelf,INT32U *pScheduleFlag,BOOL *IsOK);
    ~New104DataLink();

    void ReferToMisi(void);
    void SendCtrlFrame(INT16U FrameType);
    BOOL GetFreeTxUnit(INT8U Priority,INT8U **pTxMsg);
    BOOL SetThisUse(INT8U *BufStart);
    void NotifyToAppSchedule(void);
    void ConfS(void);

    void StopDT(BOOL closetcp);
    INT8U *GetASDU(INT8U *FrameLen);

    void  RxData(void);
	void RxDataEnDealFun(INT8U Enflag);
    PDARet SearchFrm(BOOL *HaveData);
    void TxData(void);
    void TimeOut(void);
};

 INT16U EbSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo);
 INT16U Pack104msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
 void EbErrCodeSend(INT16U SW,INT8U TypeId,INT16U wChanNo);
 INT8U UpgradeDataVerify(INT16U wChanNo);
 extern void saveRecord(BYTE *buf,WORD len,WORD mode, WORD flag);
 INT16U Pack1120afor104msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
 void Pack1120aFra104ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo);
 INT16U Eb1120aSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo);

#ifdef	__cplusplus
}
#endif


#endif
