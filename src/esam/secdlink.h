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
    enum DLSECSTATUS DLSecStatus;	//��·��Ӷ�վ״̬
    enum DLPRISTATUS DLPriStatus;	//��·������վ״̬
    enum DLTXDSTATUS TxdStatus;	//����״̬
    enum DLRXDSTATUS RxdStatus;	//����״̬

    INT32U baudrate;
    INT32U TimeOutValue;
    INT32U dwAppTID;
    INT32U wAppID;

    INT16U SourceNo;	//Դ��ַ������������ַ
    INT8U RlaConCode;	//���յĿ�����
    //INT8U FCBNoTurnNum;  //FCBδ��ת����
    BOOL FirstRecFCB;   //wjr 2009.6.3   ��·��λ��ǣ�����ʶ��·��λ���һ���յ��Է��Ĵ�FCBλ�ı��ģ����ԶԷ���FCB��ʼλΪ��ʼ��־
    INT8U TlaConCode;	//���͵Ŀ�����
    INT16U RetryFlag;	//���Ա�־
    INT16U wChanNo;	//ͨ����
    INT16U RetryCount;	//���Դ���
    INT8U LastControl;
    BOOL HaveNextFrame;

    INT16U FrameHead;	//��֡ʱ����ͷ�ڷ��ͻ�������λ��
    INT16U TxdHead;	//
    INT16U TxdTail;	//

    INT16U FrameHead_Pri;	//��¼��֡����վ���ĵ�����ͷλ�ã��ط�ʱ��
    INT16U TxdHead_Pri;
    INT16U TxdTail_Pri;

    INT16U FrameHead_Sec;	//��¼��֡�Ӷ�վ���ĵ�����ͷλ�ã��ط�ʱ��
    INT16U TxdHead_Sec;
    INT16U TxdTail_Sec;

    INT16U RxdHead;	//�������ݵ�ͷָ��
    INT16U RxdTail;
    INT16U RxdLength;

    

    INT32U TimeOutTick;			//��ʱ������
    INT32U TimeOutTick_Pri;		//����վ��ʱ������
    INT32U TimeOutTick_Sec;		//�Ӷ���ʱ������
    INT32U TimeOutTickCopy;		//��ʱ����������

    BOOL FlagData1;			//һ�����ݱ�־
    //INT8U ReqData;	//ACD=1ʱ����ʱ��¼��
    BOOL ScanFlag;

    //INT8U StartDL;	//��ʼ��·���̣�ƽ��ģʽ�ж���·�����Ƿ�����ı�־
    //INT8U RemoteDLOK;	//��λԶ����·�ɹ���ƽ��ģʽ
    //INT8U LocalDLOK;	//���ڵ��յ��Է���·��λ������Ƿ��͸�λ����ı�־ 0xff-���� 0-������
    BOOL IsSendLinkInitCmd;         //�Ƿ���������·�ϺͿ��������͹���·��ʼ������ FC9 0-û�з��͹�  1-���͹�������⵽��·�Ϻ���λ�ñ�־
                                    //���ڵ��յ��Է���·��λ������Ƿ��͸�λ����ı�־
    //INT8U  DoubleDlResetFlag;   //˫����·������־

    INT32U DLIdleTime;	//��·����ʱ�䣬��·���г������ʱ�䣬ÿ1������·��ѯӦ�ò�һ�Σ�Ϊ��һ����������ļ�ʱ����
    INT32U IdleTimeCount;

    INT8U EbMsgRxdBuf[FRAMEBUFSIZE];
    INT16U EbMsgRxdHead;	//����EB��ʽ���ݵ�ͷָ��
    INT16U EbMsgRxdTail;
    INT8U EbMsgTxdBuf[2*LPDUSIZE];
    //INT16U EbMsgRxdLength;

    INT8U RxdBuf[FRAMEBUFSIZE];	        //���ջ�����
    INT8U TxdBuf[2*LPDUSIZE];			//���ͻ�����
    INT8U TxdBuf_Pri[2*LPDUSIZE];		//���ͻ����� ���ݴ�����վ���͵����ݣ��ط�ʱ��
    INT8U TxdBuf_Sec[2*LPDUSIZE];		//���ͻ����� ���ݴ�Ӷ�վ���͵����ݣ��ط�ʱ��
    INT8U IEC_DIR;	                    //����λ��ƽ��ʽ��Ч����ƽ��=0
    INT8U N101Encrptystyle;            //�Ƿ�֧�����°�ȫ���ܷ���zhangliang 
	BOOL  IsDoubleLinkInit;             //ƽ��ģʽ�£��Ƿ�֧��˫����·ͬ����ʼ�� 1-֧�� 0-��֧��.(����������ʱ����һ�������ӽ����ɹ����ɹ�)
    //INT32U rc;
    BOOL  IsSendE5;                     //��������ʱ���Ƿ���E5��0-����  1-������
    BOOL  IsEncrypt;                    //�Ƿ�֧�ּ���
    BOOL  NoJudgeFCB;                 //���ж�FCB��ת
    
    INT16U HeartBeatIdleTime;           //��������ʱ��
    INT16U HeartBeatIdleLimit;          //�������ͼ������λ��

    CSecAppSev *pSecApp;	            //Ӧ�ò�ָ��

    //Ӧ�ò���������Ҫ�Ĳ���

    INT16U AppCommand;
    INT16U DLCommand;
    INT16U LengthIn;
    INT16U LengthOut;

    //
    INT16U FixFrmLength;//�̶�֡����
    //INT16U AsduHeadLength;//ASDUͷ���ȣ����ͱ�־����Ϣ���ַ
    INT16U LinkAddrSize;
    
    /*Ϊ���101������ѭ�������ĸĶ� ll 2014.8.11*/
    INT16U ELResetNum;              /*�յ���λ��·���� */
    INT16U ELClearTime;             /*��0��λ��·����ʱ�� */
    INT16U ELNoReplyTime;
    BOOL   ELReplyFlag;             //˫��������ʱ�Ƿ�ָ���־���ڽ�����ѭ���󣬸ñ�־��0
    void EndlessLoopMonitor(void);
    void EndlessLoopInit(void);
    
    void SendTestOnTime(void);
    
    /*��·���������վ�����*/
    void RecResetDL(void);		//��λ��·
    void RecReqDLStatus(void);		//������·״̬
    void RecMISITail(void);	//����

    /*��·����մӶ�վ�����*/
    void RecConf10(INT8U Control);		//ȷ��
    void RecDLSta(INT8U Control);			//��·״̬
    void RecTestDL(void);       //��·��������
        
    /*��·�㷢������վ�����*/
    void EditE5(void);
    void EditFra10(INT8U Function);
    void EditFra68(INT8U Function,INT16U FrameLength);
    void TimeOutFun(INT8U FailType,INT8U Prm);

    /*���ݽ��մ���*/
    BOOL ExeDLFun10(void);	//���10֡����ȷ��
    BOOL ExeDLFun68(void);	//���68֡����ȷ��
    void ExeDLFunCode10(void);		//����10����
    void ExeDLFunCode68(void);		//����68����
    void ExeE5(void);

    /*��·�㷢�ʹӶ�վ�����*/
    void EditSecFra10(INT8U Function);
    void EditSecFra68(INT8U Function,INT16U FrameLength);

    //��Լ�л�����
    BOOL ExeMaint(void);		//����л�����
    void EditMaintCon(unsigned int station);	//����ȷ��

    INT8U CheckSum(INT8U *);	//У��

    //��·��ʱ����
    INT32U TimeDelay(INT16U i);


    //�ط�����վ����������
    void SendPriDataToMISI(void);

    //�ط��Ӷ�վ����������
    void SendSecDataToMISI(void);

    void DLSendProc(void);
public:
    INT8U BalanMode;			//ƽ��ʽ=1 ��ƽ��ʽ=0

    INT16U En_LinkAddrSize;
    INT16U En_CotSize;
    INT16U En_PubAddrSize;
    INT16U En_InfoAddrSize;
    CSecDLink(INT16U AppID,CSecAppSev *p);
    ~CSecDLink();
    BOOL InitSecDLink(void);

    //�����Ч����֡
    void SearchFrame(void);

    //��MISI����
    void RecMISIData(void);
    //��ԼͨѶ���ݴ���(���ġ�168���ġ�������ȫ���ܡ�����ũ����ȫ����)
	void RecMISIDataDealFun(INT8U Enflag);
	//���ͻ������е����ݣ�дMISI�ӿ�
    BOOL SendDataToMISI(void);
    //��ԼͨѶ���ݷ���ǰ����(���ġ�168���ġ�������ȫ���ܡ�����ũ����ȫ����)
	BOOL SendMISIDataDealFun(INT8U Enflag);
    //ƽ��ģʽ�����������
    void SendDataEnd(void);
    void NotifyToAppSchedule(void);
    //��ʱ������
    void TimeOut(void);

    //������·����
    void CallDLStatus(void);

    //��·״̬���
    BOOL DLStatusCheck(void){return (DLPriStatus==PRIENABLE);}

    void CallUData(void);//ȡ��������
    void CallUMsg(void);//ȡң����Ϣ
    
    BOOL ExeDLFun16(void);
    void ExeDLFunCode16(void);
    void SqrLongFraFunI(INT8U *ori,INT8U *Bakbuf,INT8U *infodataptr,INT8U ctoalen,INT8U num,INT8U infolen,INT8U count,INT8U Sq) ;
    INT8U SqrLongFraFun(INT8U *ori,INT8U *Bakbuf);

};

/*Ϊ���101������ѭ�������ĸĶ� */
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
