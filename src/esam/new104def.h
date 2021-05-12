
#ifndef _NEW104DEF_H
#define _NEW104DEF_H

#include "publichead.h"
#include "database.h"

//define EV
#define SCHEDULEFLAG    0x40  //���ȱ�־
#define LINKINITOKFLAG  0x80  //��·���ʼ���ɹ���־��

#define APCILEN     6
#define APDULEN    255
#define ASDULEN    249
#define K          12
#define W          8

#define STARTCODE  0x68		//��ʼ��

#define M_DP_NA_ALLDBI  240     //�㶫Ҫ���ȫ˫��ң�ŷ��ͣ����⴦��
#define	M_SP_NA		1 	//����ʱ��ĵ�����Ϣ
#define	M_SP_TA		2 	//��ʱ��ĵ�����Ϣ
#define	M_DP_NA		3 	//����ʱ���˫����Ϣ
#define	M_ST_NA		5 	//��λ����Ϣ
#define	M_BO_NA		7 	//��վԶ���ն�״̬
#define	M_ME_NA		9 	//����ֵ��һ��ֵ
#define	M_ME_NB		11	//��Ȼ�ֵ
#define	M_ME_NC         13       //����ֵ���̸�����
#define	M_IT_NA		15 	//�������������
#define	M_IT_TA		16 	//��ʱ��ĵ������������
#define	M_PS_NA		20 	//����״̬��λ����ĳ��鵥����Ϣ
#define	M_ME_ND		21 	//����Ʒ�������Ĳ���ֵ
#define	M_BD_NA		232 //BCD��ֵ

#define M_SP_TB   30  //����ʱ��ĵ�����Ϣ
#define M_DP_TB   31  //����ʱ���˫����Ϣ
#define M_ST_TB   32  //����ʱ��Ĳ�λ����Ϣ
#define M_BO_TB   33  //����ʱ���32λλ��
#define M_ME_TD   34  //����ʱ��Ĳ���ֵ

#define	M_EI_NA		70 	//��ʼ������
#define C_SC_NA     45       //����ң������
#define	C_DC_NA		46 	//˫��ң������
#define	C_RC_NA		47 	//��������
#define	C_SE_NA		48 	//�趨����

#define	C_IC_NA		100 	//�ٻ�����
#define	C_CI_NA		101 	//���������ٻ�����
#define	C_RD_NA		102 	//����������
#define	C_CS_NA		103 	//ʱ��ͬ������
#define	C_RP_NA		105 	//��λ��������
#define C_TS_TA     107   //��ʱ��Ĳ�������

#define	P_ME_NA		110 	//װ�ز�������
#define P_ME_NB		111	//
#define	P_AC_NA		113 	//�������

#define	P_RS_NA_1_GX    108       //������վ�ٻ�����
#define	P_ME_NA_1_GX    112       //��������Ԥ�û�������ͣ�Զ�̲���Ԥ�ü���ȡ������������վ
#define	P_AC_NA_1_GX    113       //�����������

#define	F_FR_NA		120 	//�ļ���׼����
#define	F_SR_NA		121 	//����׼����
#define	F_SC_NA		122 	//�ٻ�Ŀ¼�pѡ���ļ��p�ٻ��ļ��p�ٻ���
#define	F_LS_NA		123 	//���Ľکp���Ķ�
#define	F_AF_NA		124 	//ȷ���ļ��pȷ�Ͻ�
#define	F_SG_NA		125 	//��
#define	F_DR_NA		126 	//Ŀ¼
#define	C_PS_NA		138 	//��Լ�л�

#define F_FY_NA   141   //�ļ�����

#define M_FA_TB   142   //���ϴ�������Ϣ

#define M_OT_NA   143   //͸�����ݴ���

/***********�㶫Զ�̲�������*****************/
#define GD_MUTIPARA_READ    108     //����������,�̸�������ͬ����������
#define GD_MUTIPARA_WRITE   55      //Ԥ��/�����������,�̸�    

/***********2016�¹�Լ����*****************/
#define M_FT_NA  42     //����ֵ��Ϣ
#define M_IT_NB  206
#define M_IT_TC  207

#define C_SR_NA  200    //�л���ֵ��
#define C_RR_NA  201    //����ֵ����
#define C_RS_NA  202    //�������Ͷ�ֵ
#define C_WS_NA  203    //д�����Ͷ�ֵ
#define F_FR_NA_N  210    //�ļ�����(��ԭ����F_FR_NA����
#define F_SR_NA_N  211    //�������
#define F_FS_NA_N  212    //����ģ��2018��׼���ļ�ͬ����cl 20180314

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

/***********2016�¹�Լ���� end*****************/

//����ԭ��
#define	PERCYC	      1 	//����/ѭ��
#define	BACK		  2 	//����ɨ��
#define	SPONT		  3 	//ͻ��
#define	INIT_101  4 	//��ʼ��
#define	REQ		    5 	//���������
#define	ACT		    6 	//����
#define	ACTCON		7 	//����ȷ��
#define	DEACT		  8 	//ֹͣ����
#define	DEACTCON	9 	//ֹͣ����ȷ��
#define	ACTTERM		10 	//�������
#define	RETREM		11 	//Զ����������ķ�����Ϣ
#define	RETLOC		12 	//������������ķ�����Ϣ
#define	FILE_101	13 	//�ļ�����
#define	INTROGEN	20 	//��Ӧ���ٻ�
#define	INTRO1		21 	//��Ӧ��1���ٻ�
#define	INTRO2		22 	//��Ӧ��2���ٻ�
#define	INTRO3		23 	//��Ӧ��3���ٻ�
#define	INTRO4		24 	//��Ӧ��4���ٻ�
#define	INTRO5		25 	//��Ӧ��5���ٻ�
#define	INTRO6		26 	//��Ӧ��6���ٻ�
#define	INTRO7		27 	//��Ӧ��7���ٻ�
#define	INTRO8		28 	//��Ӧ��8���ٻ�
#define	INTRO9		29 	//��Ӧ��9���ٻ�
#define	INTRO10		30  //��Ӧ��10���ٻ�
#define	INTRO11		31 	//��Ӧ��11���ٻ�
#define	INTRO12		32 	//��Ӧ��12���ٻ�
#define	INTRO13		33 	//��Ӧ��13���ٻ�
#define	INTRO14		34 	//��Ӧ��14���ٻ�
#define	INTRO15		35 	//��Ӧ��15���ٻ�
#define	INTRO16		36  //��Ӧ��16���ٻ�
#define	REQCOGCN	37  //��Ӧ���������ٻ�
#define	REQCO1		38 	//��Ӧ��1��������ٻ�
#define	REQCO2		39 	//��Ӧ��2��������ٻ�
#define	REQCO3		40 	//��Ӧ��3��������ٻ�
#define	REQCO4		41 	//��Ӧ��4��������ٻ�
#define UNKNOWNTYPEID   44       //δ֪�����ͱ�ʶ
#define UNKNOWNCOT      45       //δ֪�Ĵ���ԭ��
#define UNKNOWNPUBADDR  46       //δ֪��Ӧ�÷������ݵ�Ԫ������ַ
#define UNKNOWNTINFOADDR   47      //δ֪����Ϣ�����ַ
#define COT_YKRYBERR     48         //��ѹ�����
#define COT_YKSJCERR     49         //ʱ������� ң��ִ��
#define COT_YKYQSBERR    50        //��ǩ���� ң��ִ��



#define	QRPRESET	1	//�����ܸ�λ
#define	QRPSOEIND	2	//��λ�¼��������ȴ�����Ĵ�ʱ�����Ϣ
#define QRPCOLD   128 //�临λ

#define  SUMMONBCRALL  5
#define  FREEZENORESET 0x40//1
#define  FREEZERESET   0x80//2

#define LBI       0x0001
#define HBI       0x1000
#define LDBI      0x0001         //wjr˫��ң��
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
#define RMTP_ORG_H      0X800A      //���в���
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


#define PRIORITY_0   0  //����U/S֡
#define PRIORITY_1   1  //�������ȼ��ߵ�֡��ң�ص�
#define PRIORITY_2   2  //����һ��֡

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

#define P101_BL 0x10  //�Ƿ񱻷���
#define P101_SB 0x20  //�Ƿ�ȡ��
#define P101_NT 0x40  //�Ƿ�Ϊ��ǰֵ
#define P101_IV 0x80  //�Ƿ���Ч

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


#define OTH_BH      0x01  //������Ϣ

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

#define RP_PI_SE    0x80    //1-Ԥ��  0-ִ��
#define RP_PI_CR    0x40    //1-ȡ��Ԥ��  
#define RP_PI_CONT  0x01    //1-�к��� 0-�޺���


#define ALLDATATIMER 30 //30����һ��ȫ����
#define COUNTERTIMER 60 //60����һ�ε����
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
    UINT16 ControlPermit;       //ң������ 1-����0-������ ȱʡΪ1
    UINT16 SetTimePermit;  //�������� 1-����0-������ ȱʡΪ1
    UINT16 SendAllDataOnTime;  //��ʱ����ȫ���� 1-����0-������ ȱʡΪ1
    UINT16 SendCountOnTime;//��ʱ���͵��  1-����0-������ ȱʡΪ1
    UINT16 SendCountWithReset;//���͵��ʱ����λ��1-����λ 0-������λ ȱʡΪ0
    UINT16 UseStandClock;//ʹ�ñ�׼ʱ�Ӹ�ʽ 1-��׼ 0-�Ǳ�׼ ȱʡΪ1
    UINT16 AllDataInternal;//��ʱ����ȫ���ݼ�����֣� ȱʡ30
    UINT16 ScanData2;//��������ɨ�������룩 ȱʡ3
    UINT16 CountInternal;//��ʱ���͵�ȼ�����֣� ȱʡ60
    UINT16 TickValue[4];//TickValue[0]����
                        //TickValue[1]ȷ���޻ش�ʱ�������룩 ȱʡ15
                        //TickValue[2]����ȷ��֡ʱ�������룩 ȱʡ5
                        //TickValue[3]���Ͳ���֡ʱ�������룩 ȱʡ30
    UINT16 AIDeadValue;//ң������ֵ��ǧ�ֱȣ� ȱʡ3
    UINT16 SBIType;//����ң�����ͱ�ʶ ȱʡ ����ʱ��ĵ�����Ϣ
    UINT16 AIType;//ң�����ͱ�ʶ ȱʡ ����Ʒ�������Ĳ���ֵ
    
    UINT16 YCNum;   /*ʵʱң��������ȱʡ0xFFFF��2007��11��1���򱱾���Ŀ����*/
    UINT16  LBIinfoaddr;  //����ң����Ϣ���ַ       2008.11.5
    UINT16  LDBIinfoaddr; //˫��ң����Ϣ���ַ       2008.11.5
    
    UINT16  control;      //��־λ
                          /*D0 �Ƿ�֧�ֱ���ģʽң��������ܴ���0-֧�� 1-��֧�� ȱʡΪ1
                            D1 ֧��"������Լ��չ"-2015��   1-��ѡ 0-����ѡ Ĭ��1
                            D2 
                            D3 
                            D4
                            D5
                            D6
                          */
    UINT16  EncryptTimeout; //ͨѶ����ʱ�����ʱʱ��
    
};
//��վ����Լ���control����λ����
#define CON_ENCRYPT         0x0001      //���������֤���ܷ���
#define CON_104GYKZ         0x0002      //֧�ֹ���104��Լ��չ-2015�� 1-��ѡ 0-����ѡ Ĭ��1
#define CON_1161ENCRPTY     0x0004      //˫�������֤���ܷ�����1161����оƬ��
#define CON_1120ENCRPTY     0x0008      //����ũ�����ܷ�����1120����оƬ��
#define CON_OLD1120ENCRPTY  0x0010      //�Ϻ���ũ������ͨѶ                        ռλ(��ʱ���ã�
#define CON_CLEARRSNO       0x0020      //1-�յ�start������շ����               ռλ(��ʱ���ã�
#define CON_NOJUDGERSNO     0x0040      //1-�����շ����                            ռλ(��ʱ���ã�
#define CON_SENDCOS         0x0080      //1-�ڹ�ѡ2015��չ��ʱ������Ҫ����cos��     ռλ(��ʱ���ã�
#define CON_RSTSEND_INITEND  0x0100      //1-��λ��ŷ��ͳ�ʼ������֡  
#define CON_FREVENT_2       0x0200      //1-�����¼�����Ϣ���ַ��2�ֽ� 0-3�ֽ�
#define CON_ALLDBI_104      0x0400      //ң����˫��ң�ŷ���-�㶫��ɽ

struct PPri104Pad
{
    UINT16 DriverID;
    UINT16 ControlPermit;       //ң������ 1-����0-������ ȱʡΪ1
    UINT16 SetTimePermit;  //�������� 1-����0-������ ȱʡΪ1
    UINT16 CallAllDataOnTime;  //��ʱ�ٻ�ȫ���� 1-����0-������ ȱʡΪ1
    UINT16 CallCountOnTime;//��ʱ�ٻ����  1-����0-������ ȱʡΪ1
    UINT16 CallCountWithReset;//�ٻ����ʱ����λ��1-����λ 0-������λ ȱʡΪ0
    UINT16 UseStandClock;//ʹ�ñ�׼ʱ�Ӹ�ʽ 1-��׼ 0-�Ǳ�׼ ȱʡΪ1
    UINT16 AllDataInternal;//��ʱ�ٻ�ȫ���ݼ�����֣� ȱʡ30
    UINT16 SetTimeInternal;//��ʱ���Ӽ�����֣� ȱʡ5
    UINT16 CountInternal;//��ʱ�ٻ���ȼ�����֣� ȱʡ60
    UINT16 TickValue[4];//TickValue[0]����
                        //TickValue[1]ȷ���޻ش�ʱ�������룩 ȱʡ15
                        //TickValue[2]����ȷ��֡ʱ�������룩 ȱʡ5
                        //TickValue[3]���Ͳ���֡ʱ�������룩 ȱʡ30
    UINT16 Control;//��־λ    
                     //[D0]�Ƿ��ٻ�¼��
    UINT16 DTUBasedFdNo;//�ٻ�¼��ʱ����DTU�Ļ������ߺ� Ĭ����0 ��ʾ�����ü���¼��  ����0��ʾ���ü�����DTU
    UINT16 Rsv[30];
};

#define CON_CALLLBDATA         0x0001      //֧���ٻ�¼��  ɽ���߱�׼   20180725


#ifdef	__cplusplus
extern "C"{
#endif

void SetProtocalErr(void);

#ifdef	__cplusplus
}
#endif

#endif



