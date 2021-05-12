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
#define LPDUSIZE        (255+6) //��·�㷢�ͱ��ĵ���󳤶�
#define FRAMEBUFSIZE    512     //��·����ջ���������󳤶�
#define NORETRYNUM      0
#define MINRETRYNUM     1
#define	MAXRETRYNUM     3       //�ط���������
#define APDUSIZE        255     //Ӧ�ò��շ�����������󳤶�
#define MAXFTUNUM       60      //ÿ����������FTU��

//��·��ʼ��վ�����
#define	DLRESETRDL      0       //��λԶ����·
#define DLRESETUSE      1       //��λ�û�����
#define DLTESTDL        2       //������·����
#define DLSENDCON       3       //����ȷ������
#define DLSENDNOCON     4       //���Ͳ�ȷ������
#define DLREQACD        8       //������Ӧȷ������״̬
#define DLREQSTATUS     9       //�ٻ���·״̬
#define DLREQDATA1      10       //�ٻ�1���û�����
#define DLREQDATA2      11       //�ٻ�2���û�����
#define MAINTSWITCH     12       //�л���ά����Լ��

//��·��Ӷ�վ�����
#define DLCON           0       //ȷ��
#define	DLNOCON         1       //��ȷ��
#define DLRESDATA       8       //��������Ӧ����֡
#define DLNODATA        9       //�����ٻ�������
#define DLSTATUSOK      11       //��Ӧ��·״̬��ش�����֡
#define DLNOWORK        14       //��·δ����
#define DLNOFIN         15       //��·δ���

//��·�㵽Ӧ�ò������(INT16U)DLCommand����
#define DL_RESETUSE     1       //��λ�û�����
#define DL_TESTDLINK    2       //���� ��·
#define	DL_SENDCON      3       //��·�յ�03����
#define	DL_SENDNOCON    4       //��·�յ�04����
#define	DL_LINKDISABLE  5       //��·��Ч
#define	DL_LINKENABLE   6       //��·��Ч,��·�յ�0��ȷ�ϣ���Ӧ�ò㷢������
#define	DL_NODATA       7       //��·���յ�������Ӧ��
#define	DL_SENDDATA     8       //��·�յ�08����
#define	DL_SCAN1S       9       //ƽ��ģʽ��·���в�ѯӦ�ò�����YK����ʱȫ���ݡ���ʱ���ӡ���ʱ��ȣ�
#define DL_CALLDATA1    10       //ȡһ������
#define DL_CALLDATA2    11       //ȡ��������
#define DL_CALLUDATA    12       //ȡ�������ݣ�ƽ��ģʽ��վ��������һ�����ݣ�COS��SOE����
#define DL_CALLDBMSG    13       //��վƽ��ģʽ����ȡ���ݿ���Ϣ���Ա㼰ʱ����ң�ص���Ϣ��
#define	DL_APPCON       14       //�յ���03�����ȷ��,��վƽ��ʽ
#define DL_RESETDL      15       //�յ���λ��·���Ӧ�ò�ȷ���Ƿ񷢳�ʼ����������վ��ƽ�⡣
#define DL_SECLINKENABLE 16     //�յ��Ӷ���λ���֪ͨӦ�ò�Ӷ���·��Ч

#define	DL_REC_ACD      0x8000   //��λ��1��·���յ���ACD=1������Ϊ0
#define	DL_FCBOK        0x4000   //FCB��ת��ȷ,=1��ȷ��

//Ӧ�ò㵽��·�������(INT16U)AppCommand
#define	APP_APPCON      0       //Ӧ�ò���յ���������Ӧ��
#define APP_RESETUSE    1       //��λ�û�����
#define APP_TESTDLINK   2       //���� ��·
#define	APP_SENDCON     3       //����03����
#define	APP_SENDNOCON   4       //����04����
#define	APP_SENDDATA    8       //����08����
#define APP_NODATA      9       //Ӧ�ò������ݣ���վ��ƽ��
#define	APP_CALLDATA1   10       //����һ������
#define	APP_CALLDATA2   11       //�����������
#define	APP_ALLDEVNOUSE 12       //Ӧ�ò������豸������
#define	APP_NOJOB       13       //Ӧ�ò�������//��վƽ��ģʽ��Ч
#define	APP_APPDIABALE  14       //Ӧ�ò���Ч
#define APP_LINKINIT    15       //������·��ʼ��
#define APP_RESETMACHINE 16

#define	APP_HAVEDATA1   0x8000   //��λ��1˵��Ӧ�ò���һ�����ݣ�����Ϊ0;��վ��ƽ�⡢ƽ��ģʽ���ٻ�������

//��·��֡����
#define	STARTCODE10     0x10       //��ʼ��
#define STARTCODE68     0x68       //��ʼ��
#define ENDCODE         0x16       //ֹͣ��
#define	SINGLECON       0xE5       //���ֽ�������ȷ��Ӧ��
#define MAINTSTARTCODE  'I'

#define M_DP_NA_ALLDBI     240     //�㶫Ҫ���ȫ˫��ң�ŷ��ͣ����⴦��
#define	M_SP_NA         1       //����ʱ��ĵ�����Ϣ
#define	M_SP_TA         2       //��ʱ��ĵ�����Ϣ
#define	M_DP_NA         3       //����ʱ���˫����Ϣ
#define	M_DP_TA         4       //��ʱ���˫����Ϣ
#define	M_ST_NA         5       //��λ����Ϣ
#define	M_ST_TA         6       //��ʱ��Ĳ�λ����Ϣ
#define	M_BO_NA         7       //��վԶ���ն�״̬
#define	M_ME_NA         9       //����ֵ,��һ��ֵ
#define	M_ME_TA         10       //��ʱ��Ĳ���ֵ����һ��ֵ
#define	M_ME_NB         11       //����ֵ����Ȼ�ֵ�����±�׼�У�����ĿǰӦʹ�õ����͡����ԭ����M_ME_NA
#define	M_ME_TB         12       //��ʱ��Ĳ���ֵ,��Ȼ�ֵ
#define	M_ME_NC         13       //����ֵ���̸�����
#define	M_ME_TC         14       //��ʱ�����ֵ���̸�����
#define	M_IT_NA         15       //�������������
#define	M_IT_TA         16       //��ʱ��ĵ������������
#define	M_EP_TA         17       //��ʱ��ļ̵籣�����غ�բ�豸�����¼�
#define	M_EP_TB         18       //��ʱ��ļ̵籣��װ�ó��������¼�
#define	M_EP_TC         19       //��ʱ��ļ̵籣��װ�ó��������·��Ϣ�¼�
#define	M_PS_NA         20       //����״̬��λ����ĳ��鵥����Ϣ
#define	M_ME_ND         21       //����Ʒ�������Ĳ���ֵ����һ��ֵ
#define	M_BD_NA         232      //BCD��ֵ

#define	M_EI_NA         70       //��ʼ������

//DL/T634.5101-2002��׼�������ͱ�ʶ
#define M_SP_TB         30       //��CP56Time2aʱ��ĵ�����Ϣ
#define M_DP_TB         31       //��CP56Time2aʱ���˫����Ϣ
#define M_ST_TB         32       //��CP56Time2aʱ��Ĳ�λ����Ϣ
#define M_BO_TB         33       //��CP56Time2aʱ���32λ���ش�
#define M_ME_TD         34       //��CP56Time2aʱ��Ĳ���ֵ����һ��ֵ
#define M_ME_TE         35       //��CP56Time2aʱ��Ĳ���ֵ����Ȼ�ֵ
#define M_ME_TF         36       //��CP56Time2aʱ��Ĳ���ֵ���̸�����
#define M_IT_TB         37       //��CP56Time2aʱ����ۼ���
#define M_EP_TD         38       //��CP56Time2aʱ��ļ̵籣���豸�¼�
#define M_EP_TE         39       //��CP56Time2aʱ��ļ̵籣���豸���������¼�
#define M_EP_TF         40       //��CP56Time2aʱ��ļ̵籣���豸���������·��Ϣ
#define	M_EI_NA         70       //��ʼ������֡

#define C_SC_NA         45       //����ң������
#define	M_CD_NA         106       //��ʱ�������
#define	M_CS_NA         103       //ʱ��ͬ������
//END

#define	C_DC_NA         46       //˫��ң������
#define	C_RC_NA         47       //��������
#define	C_SE_NA         48       //�趨����
#define	C_BO_NA         51       //32λ��λ��

#define	C_IC_NA         100       //�ٻ�����
#define	C_CI_NA         101       //���������ٻ�����
#define	C_RD_NA         102       //����������
#define	C_CS_NA         103       //ʱ��ͬ������
#define	C_TS_NA         104       //��������
#define	C_RP_NA         105       //��λ��������
#define	C_CD_NA         106       //��ʱ�������

#define	P_ME_NA         110       //װ�ز��������һ��ֵ
#define	P_ME_NB         111       //װ�ز��������Ȼ�ֵ
#define	P_ME_NC         112       //װ�ز�������̸�����
#define	P_AC_NA         113       //�������

#define	P_RS_NA_1_GX    108       //������վ�ٻ�����
#define	P_ME_NA_1_GX    112       //��������Ԥ�û�������ͣ�Զ�̲���Ԥ�ü���ȡ������������վ
#define	P_AC_NA_1_GX    113       //�����������

#define	F_FR_NA         120       //�ļ���׼����
#define	F_SR_NA         121       //����׼����
#define	F_SC_NA         122       //�ٻ�Ŀ¼�pѡ���ļ��p�ٻ��ļ��p�ٻ���
#define	F_LS_NA         123       //���Ľکp���Ķ�
#define	F_AF_NA         124       //ȷ���ļ��pȷ�Ͻ�
#define	F_SG_NA         125       //��
#define	F_DR_NA         126       //Ŀ¼

#define F_FY_NA         141       //�ļ�����

#define M_FA_TB         142       //���ϴ�������Ϣ

#define C_PF_NA         143       //������ֵ�趨�������ԭ��6Ϊ�趨��
#define P_PF_NA         144       //������ֵ�ٻ��������ԭ��5Ϊ����

#define M_PF_NA         145       //������ֵ�����������ԭ��5Ϊ������

//#define C_IT_TC         146       //�ٻ���ʷ���  ��վCOT=6 ���� ��վCOT=7����ȷ�� COT=10ֹͣ

//#define M_IT_TC         147       //��ʷ�������  COT=5������

#define E_SGC_F0       0xF0//����ũ������TI
#define E_SGC_F1       0xF1
#define E_SGC_F2       0xF2
#define E_SGC_F3       0xF3
#define E_SGC_F4       0xF4
#define E_SGC_F5       0xF5
#define E_SGC_F6       0xF6
#define E_SGC_FA       0xFA

/***********�㶫Զ�̲�������*****************/
#define GD_MUTIPARA_READ    108     //����������,�̸�������ͬ����������
#define GD_MUTIPARA_WRITE   55      //Ԥ��/�����������,�̸�  


/***********2016�¹�Լ����*****************/
#define M_FT_NA  42     //����ֵ��Ϣ
#define M_IT_NB  206
#define M_IT_TC  207

#define C_SR_NA     200    //�л���ֵ��
#define C_RR_NA     201    //����ֵ����
#define C_RS_NA     202    //�������Ͷ�ֵ
#define C_WS_NA     203    //д�����Ͷ�ֵ
#define F_FR_NA_N   210    //�ļ�����
#define F_SR_NA_N   211    //�������
#define F_FS_NA_N   212    //����ģ��2018��׼���ļ�ͬ����cl 20180314

#define PARA_DATA_TYPE_WORD   45  //�޷��Ŷ�����
#define PARA_DATA_TYPE_FLOAT  38  //�̸�����

//�ļ����������ʾ����
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

//����ԭ��
#define	PERCYC          1       //����/ѭ��
#define	BACK            2       //����ɨ��
#define	SPONT           3       //ͻ��
#define	INIT_101        4       //��ʼ��
#define	REQ             5       //���������
#define	ACT             6       //����
#define	ACTCON		    7       //����ȷ��
#define	DEACT           8       //ֹͣ����
#define	DEACTCON        9       //ֹͣ����ȷ��
#define	ACTTERM         10       //�������
#define	RETREM          11       //Զ����������ķ�����Ϣ
#define	RETLOC          12       //������������ķ�����Ϣ
#define	FILE_101        13       //�ļ�����
#define	INTROGEN        20       //��Ӧ���ٻ�
#define	INTRO1          21       //��Ӧ��1���ٻ�
#define	INTRO2          22       //��Ӧ��2���ٻ�
#define	INTRO3          23       //��Ӧ��3���ٻ�
#define	INTRO4          24       //��Ӧ��4���ٻ�
#define	INTRO5          25       //��Ӧ��5���ٻ�
#define	INTRO6          26       //��Ӧ��6���ٻ�
#define	INTRO7          27       //��Ӧ��7���ٻ�
#define	INTRO8          28       //��Ӧ��8���ٻ�
#define	INTRO9          29       //��Ӧ��9���ٻ�
#define	INTRO10         30       //��Ӧ��10���ٻ�
#define	INTRO11         31       //��Ӧ��11���ٻ�
#define	INTRO12         32       //��Ӧ��12���ٻ�
#define	INTRO13         33       //��Ӧ��13���ٻ�
#define	INTRO14         34       //��Ӧ��14���ٻ�
#define	INTRO15         35       //��Ӧ��15���ٻ�
#define	INTRO16         36       //��Ӧ��16���ٻ�
#define	REQCOGCN        37       //��Ӧ���������ٻ�
#define	REQCO1          38       //��Ӧ��1��������ٻ�
#define	REQCO2          39       //��Ӧ��2��������ٻ�
#define	REQCO3          40       //��Ӧ��3��������ٻ�
#define	REQCO4          41       //��Ӧ��4��������ٻ�
#define UNKNOWNTYPEID   44       //δ֪�����ͱ�ʶ
#define UNKNOWNCOT      45       //δ֪�Ĵ���ԭ��
#define UNKNOWNPUBADDR  46       //δ֪��Ӧ�÷������ݵ�Ԫ������ַ
#define UNKNOWNTINFOADDR   47      //δ֪����Ϣ�����ַ
#define COT_YKRYBERR     48         //��ѹ����� 0x30
#define COT_YKSJCERR     49         //ʱ������� ң��ִ��
#define COT_YKYQSBERR    50        //��ǩ���� ң��ִ��

/*��Ϣ����ʼ��ַ����*/
#define LBI       0x0001
#define HBI       0x1000
#define LDBI      0x0001         //wjr˫��ң��
#define HDBI      0x1000
#define LAI       0x4001
#define HAI       0x5000
#define LPARA	  0x5001    //ң�����
#define HPARA	  0x6000
#define LBO       0x6001
#define HBO       0x6200
#define	LSET	  0x6201
#define	HSET	  0x6400
#define LBCR      0x6401
#define HBCR      0x6600
#define LSPI      0x6601
#define HSPI      0x6700
#define LBCD      0x6701//..DL/T634.5101-2002��׼����BCD�루ˮλ������Ϣ�����ַ�Ķ���
#define HBCD      0x6800
#define RTUSTATUS 0x6801
#define LFILE	  0x6802
#define HFILE	  0x7000

#define RMTP_ORG_L      0X8001
#define RMTP_ORG_H      0X800A      //���в���
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
#define	TESTREASON  1	//1=�������� 0=δ����

#define	NEGAPPROV   1	//���Ͽ�
#define	POSAPPROV   0	//�϶��Ͽ�

#define	OFF	0 	//��
#define	ON	1 	//��

#define	GATEVAL		1 	//����ֵ
#define	FIWAVE		2	  //�˲�ϵ��
#define	MELOWER		3	  //����ֵ������ֵ
#define	MEUPPER		4	  //����ֵ������ֵ

#define	LPCCHANGE	  1	//���ز���δ�ı�
#define	LPCNOCHANGE	0	//���ز����ı�

#define	POPMOVE		0	//����������
#define	POPNOMOVE	1	//����δ����

#define	QRPRESET	1	//�����ܸ�λ
#define	QRPSOEIND	2	//��λ�¼��������ȴ�����Ĵ�ʱ�����Ϣ
#define QRPCOLD   128 //�临λ

//EV
#define DLFLAG          0x00000020
#define APPFLAG         0x00000040
#define APPTIMEFLAG     0x00000080
#define SCHEDULE        0x00000100
#define NEXTFRAME       0x00000200
#define SENDOVER        0x00000400
#define SAVEKWHFLAG     0x00000800 //��ʱ����������
#define FORCESCHEDULE   0x00010000  //ǿ�ƽ���һ��Ӧ�ò�ɨ��

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

/*������·��������ݶ���*/

/*��·��Ӷ�վ������״̬*/
enum	DLSECSTATUS
{
    SECDISABLE =0,	//�Ӷ�վ��·��û��λ
    SECENABLE,	//�Ӷ�վ��·�����ʹ��
    SECDLSTATUS,	//
    SECDLBUSY 	//�Ӷ�վ��·��æ
};


/*��·������վ������״̬*/
enum	DLPRISTATUS
{
    PRIDISABLE =0,	//����վ��·��û��λ
    PRIWAITSTATUS,	//����վ��·��ȴ��ش���·״̬(ִ��������·״̬)
    PRIWAITRSCON,	//����վ��·��ȴ���λ��·ȷ��
    PRIWAITINITEND,	//��վ�ȴ���վ��ʼ������֡???
    PRIENABLE,	//����վ��·�����ʹ��
    PRISENDCON,	//����վ��·��ȴ�����ȷ��
    PRIREQRES,	//����վ��·��ȴ�������Ӧ
    PRIDLBUSY 	//����վ��·��æ
};

/*���ͽ���״̬����*/
enum DLTXDSTATUS
{
    TXDSEND,	//���ͱ��ĵ�һ֡״̬
    TXDWAIT		//���ͱ��ĵ�һ֡״̬
};

/*��·�����״̬*/
enum DLRXDSTATUS
{
    RXDSTART=0,	//Ѱ����ʼ��־״̬
    RXDHEAD,	//Ѱ�ұ���ͷ
    RXDCONTINUE 	//���պ�������
};

struct DLDevInfo
{
    enum DLSECSTATUS DLSecStatus; //��·��Ӷ�վ״̬
    enum DLPRISTATUS DLPriStatus; //��·������վ״̬
    INT8U RlaConCode;             //���յĿ�����
    INT8U FCBNoTurnNum;           //FCBδ��ת����
    INT8U TlaConCode;             //���͵Ŀ�����
    INT16U DestAddr;              //Ŀ�ĵ�ַ��������վ��·��ַ
    INT16U SourceNo;              //Դ��ַ������������ַ
    INT8U FlagData1;              //һ�����ݱ�־

    //INT8U StartDL;                //��ʼ��·���̣�ƽ��ģʽ�ж���·�����Ƿ�����ı�־
    //INT8U RemoteDLOK;             //��λԶ����·�ɹ���ƽ��ģʽ
    //INT8U LocalDLOK;              //������·��λ�ɹ���ƽ��ģʽ
    
    BOOL IsSendLinkInitCmd;         //�Ƿ���������·�ϺͿ��������͹���·��ʼ������ FC9 0-û�з��͹�  1-���͹�������⵽��·�Ϻ���λ�ñ�־
                                    //���ڵ��յ��Է���·��λ������Ƿ��͸�λ����ı�־
    
    BOOL FirstRecFCB;             //����ʶ��·��λ���һ���յ��Է��Ĵ�FCBλ�ı��ģ����ԶԷ���FCB��ʼλΪ��ʼ��־
    
    BOOL IsEncrypt;             //�Ƿ�֧�ּ���ģʽ
    
    
};

/*��·�㶨�����*/

/*����ΪӦ�ò���ض���*/

#define VSQ_SQ 0x80         //sq=1 ˳����  SQ=0 ��ɢ����
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

//�̶�֡���ṹ
#define P101_FUNCODE 0x0F
#define P101_FCV     0x10
#define P101_DFC     0x10
#define P101_FCB     0x20
#define P101_ACD     0x20
#define P101_PRM     0x40
#define P101_DIR     0x80

#define P101_BL 0x10  //�Ƿ񱻷���
#define P101_SB 0x20  //�Ƿ�ȡ��
#define P101_NT 0x40  //�Ƿ�Ϊ��ǰֵ
#define P101_IV 0x80  //�Ƿ���Ч
#define P101_OV 0x01    //���

#define SPI 0x01
#define DPI 0x03

#define VTI_VALUE 0x7F
#define VTI_T     0x80

#define P101_OV 0x01


#define BCR_SQ 0x1F
#define BCR_CY 0x20
#define BCR_CA 0x40
#define BCR_IV 0x80

//��д�����Ĳ���������ʾ��
#define RP_PI_SE    0x80    //1-Ԥ��  0-ִ��
#define RP_PI_CR    0x40    //1-ȡ��Ԥ��  
#define RP_PI_CONT  0x01    //1-�к��� 0-�޺���

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
    INT8U  SendFlag;//���ͱ�־��0δ���ͣ�ff�Ѿ�����
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

#define FILELENGTH  (128*9)   //9���ܲ���ÿ�ܲ�64��128�ֽڡ�
#define LUBOINFOADDR 0x6901
#define LUBOFLAG 0x8000



#define ALLDATATIMER     30    //30����һ��ȫ����
#define COUNTERTIMER     60    //60����һ�ε����
#define CLOCKTIMER       5     //5���Ӷ���
#define SCANDATA2TIMER   3     //��������ɨ����3s



struct PPri101Pad
{
    UINT16 DriverID;
    UINT16 ControlPermit;       //ң������ 1-����0-������ ȱʡΪ1
    UINT16 SetTimePermit;       //�������� 1-����0-������ ȱʡΪ1
    UINT16 BalanceMode;         //ƽ��ģʽ 1-ƽ��ģʽ 0-��ƽ��ģʽ ȱʡΪ0
    UINT16 CallAllDataOnTime;   //��ʱ�ٻ�ȫ���� 1-����0-������ ȱʡΪ1
    UINT16 CallCountOnTime;     //��ʱ�ٻ����  1-����0-������ ȱʡΪ1
    UINT16 CallCountWithReset;  //�ٻ����ʱ����λ��1-����λ 0-������λ ȱʡΪ0
    UINT16 UseStandClock;       //ʹ�ñ�׼ʱ�Ӹ�ʽ 1-��׼ 0-�Ǳ�׼ ȱʡΪ1
    UINT16 SetTimeBroadCast;    //����ʹ�ù㲥���� 1-�㲥 0-���㲥 ȱʡ0
    UINT16 MaxErrorTimes;       //�����������дͨѶ���� ȱʡΪ3
    UINT16 AllDataInternal;     //��ʱ�ٻ�ȫ���ݼ�����֣� ȱʡ30
    UINT16 SetTimeInternal;     //��ʱ���Ӽ�����֣� ȱʡ5
    UINT16 CountInternal;       //��ʱ�ٻ���ȼ�����֣� ȱʡ60
    UINT16 TimeOut;             //��ʱʱ�䣨*10ms�� ȱʡ100

    UINT16 LINKAddrSize;//��·��ַ�ֽ��� ȱʡΪ2
    UINT16 COTSize;//����ԭ���ֽ��� ȱʡΪ1
    UINT16 PUBAddrSize;//�������ַ�ֽ��� ȱʡΪ2
    UINT16 INFOAddrSize;//��Ϣ���ַ�ֽ��� ȱʡΪ2
    
    UINT16 TimeDelay;   //����֮ǰ�Ƿ�Ҫ����
    UINT16  control;      //��־λ
                          /*D0 ƽ��ģʽ�£���վ��DIR��־λ�Ƿ���1��־λ��0-����Ҫ��1��1-��Ҫ��1��ȱʡֵ0
                            D1 ƽ��ģʽ�£��Ƿ�֧��˫����·��ʼ����(��֧��˫����·��ʼ��ʱ������������·����ʱ�з������ʱ����֧��ʱ�޷�����ʱ)��0-֧�֣�1-��֧�֣�ȱʡֵ0
                            D2 ֧������ģ��2018����׼��Ĭ�Ϲ�ѡ
                            D3 �ڶ�������ģ�飬Ĭ�ϲ���ѡ
                            D4 ˫��ң�أ�Ĭ�ϲ���ѡ 
                            D5
                            D6
                            D7 ֧��"������Լ��չ"-2015��   1-��ѡ 0-����ѡ Ĭ��1
                          */
};
//��վ����Լ���control����λ����
#define PRICON_DIRBITFLAG    0x0001
#define PRICON_ISNEEDDELAY   0x0002
#define PRICON_101GYKZ       0x0080      //AJ++170810 �������ٵ��ʱ�Ƿ���Ҫ�ȶ��� 1-������ 0-����
#define PRICON_XSBZ          0x0004      //֧������ģ��2018����׼   CL 20180525
#define PRICON_XSNUM2        0x0008      //�ڶ�������ģ��    CL 20180525  
#define PRICON_SDYK          0x0010      //˫��ң��   CL20180828

struct PSec101Pad
{
    UINT16 DriverID;
    UINT16 ControlPermit;       //ң������ 1-����0-������ ȱʡΪ1
    UINT16 SetTimePermit;  //�������� 1-����0-������ ȱʡΪ1
    UINT16 BalanceMode;//ƽ��ģʽ 1-ƽ��ģʽ 0-��ƽ��ģʽ ȱʡΪ0
    UINT16 SOEWithCP56Time;//SOEʹ�ó�ʱ���ʽ 1-56λ��ʱ�� 0-24λ��ʱ��  ȱʡΪ1
    UINT16 UseStandClock;//ʹ�ñ�׼ʱ�Ӹ�ʽ 1-��׼ 0-�Ǳ�׼ ȱʡΪ1
    UINT16 MaxALLen; //���Ӧ�ò㱨�ĳ��� ȱʡ250
    UINT16 AIDeadValue;//ң������ֵ��ǧ�ֱȣ� ȱʡ3
    UINT16 ScanData2;//��������ɨ�������룩 ȱʡ3
    UINT16 TimeOut;//��ʱʱ�䣨*10ms�� ȱʡ100

    UINT16 LINKAddrSize;//��·��ַ�ֽ��� ȱʡΪ2
    UINT16 COTSize;//����ԭ���ֽ��� ȱʡΪ1
    UINT16 PUBAddrSize;//�������ַ�ֽ��� ȱʡΪ2
    UINT16 INFOAddrSize;//��Ϣ���ַ�ֽ��� ȱʡΪ2
    UINT16 BackScanTime;//��������ɨ�������֣� ȱʡ20
    UINT16 CycScanTime;//����ѭ������ɨ�������룩 ȱʡ20
    UINT16 TypeID[20];//���ͱ�ʶ 0��7ȱʡΪ1��8��11ȱʡΪ11��12Ϊ0��13Ϊ11��14Ϊ5��16ȱʡΪ15 ������Ϊ0
    UINT16 GroupNum[20];//ÿ����Ϣ���� ȱʡΪ0��
    UINT16 HistoryDDPermit;//��ʷ��ȱ������� 1-���� 0-������ ȱʡ0
    UINT16 HistoryDDTime;//��ʷ��ȱ������ڣ��֣� ȱʡ60
    
    
    UINT16  YCNum;        /*ʵʱң��������ȱʡ0xFFFF��2008��3��10���򱱾���Ŀ����*/
    UINT16  LBIinfoaddr;  //����ң����Ϣ���ַ       2008.11.5
    UINT16  LDBIinfoaddr; //˫��ң����Ϣ���ַ       2008.11.5
    
    UINT16  control;      //��־λ
                          /*D0 ƽ��ģʽ�£���վ��DIR��־λ�Ƿ���1��־λ��0-��Ҫ��1��1-����Ҫ��1��ȱʡֵ0
                            D1 ��ƽ��ģʽ�£���������ʱ���Ƿ���E5��0-����  1-�����ͣ�ȱʡΪ0
                            D2 ƽ��ģʽ�£��Ƿ�֧��˫����·��ʼ����(��֧��˫����·��ʼ��ʱ������������·����ʱ�з������ʱ����֧��ʱ�޷�����ʱ)��0-֧�֣�1-��֧�֣�ȱʡֵ0
                            D3 �Ƿ�֧�ֱ���ģʽң��������ܴ���0-֧�� 1-��֧�� ȱʡΪ1
                            D4 �Ƿ�ÿ�γ�ʼ�����������ͳ�ʼ������֡��0-ÿ�η��� 1-��λ���� ȱʡΪ0
                            D5 ����FCB��ת��Ĭ���ж� 0-�ж� 1-���ж�
                            D6 GPRS���� 0-��ʹ��GPRS  1-ʹ��GPRS
                            D7 ֧��"������Լ��չ"-2015��   1-��ѡ 0-����ѡ Ĭ��1
                          */
    UINT16  EncryptTimeout; //ͨѶ����ʱ�����ʱʱ��
    //UINT16  HeartBeatIdleLimit; //�������ͼ��
};

//��վ����Լ���control����λ����
#define CON_DIRBITFLAG      0x0001
#define CON_ISSENDE5        0x0002
#define CON_ISNEEDDELAY     0x0004      //0-˫�򣨹�ѡ״̬�� 1-����
#define CON_ENCRYPT         0x0008      //ͨѶ����
#define CON_RSTSEND_INITEND 0x0010
#define CON_NOFCVCTRL       0x0020      //����FCB��ת

#define CON_101GYKZ         0x0080      //֧�ֹ���101��Լ��չ-2015�� 1-��ѡ 0-����ѡ Ĭ��1
#define CON_1161ENCRPTY     0x0100      //֧�ֹ���ȫ��Ϣ���ܰ�ȫ��������(sgc1161).��CON_ENCRYPT���ʹ�á�CON_ENCRYPT:CON_1161ENCRPTY = 1:0(�޼���)  0:0(11�氲ȫ����) 0:1(16�氲ȫ��������) 1:1(�Ƿ�)
#define CON_1120ENCRPTY     0x0200      //֧�ֺ���ũ������ͨѶ
#define CON_OLD1120ENCRPTY  0x0400      //֧���Ϻ���ũ������ͨѶ
#define CON_101SENDCOS      0x0800      //1-�ڹ�ѡ2015��չ��ʱ������Ҫ����cos
#define CON_ALLDBI_101      0x1000      //ң����˫��ң�ŷ���-�㶫��ɽ


void SetProtocalErr(void);  

#ifdef	__cplusplus
}
#endif

#endif
