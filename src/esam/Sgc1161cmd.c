#include"sgc1161cmd.h"
#include "publichead.h"
#include <config.h>
#include "mcf_qspi.h"
#include "HardEncrypt.h"
#include "semLib.h"
#include "md5calc.h"

//#include "new104def.h"


#define SGCSIGNDATALEN 64
#define SGCRANDOMLEN   8
#define SGCSYMMETRYKEYID   0x01

#define EBKEYIDLEN   1      //��Կ�����ֽ���
#define EBHEADLEN   4
#define EBAUDATASTARTSITE   9   //��֤���̱���������ʼλ��

extern  SEM_ID  sem_qspiid;

extern INT8U *pHeSendBuf1,*pHeSendBuf2;
extern INT8U *ParaTempBuf;
extern struct encrypt_st  encrypt_data;//�����ж�ʱ�����ʱ


INT8U SgcSelfRandbuf[8];//оƬ��������������
INT8U SgcCerToolIDbuf[8];//֤�������ID���
INT8U SgcGetWayRandbuf[8];//�����·������
INT8U SgcMasterRandbuf[16];//��վ��֤ʱ�·������ R1+R1��λȡ��
INT8U SgcRdDatatoYWbuf[16];//֤���������֤�����R+Rȡ��

INT8U UpLoadtime[6];//������֤ʱ��
INT8U UpLoadRdata[8];//������֤�����    
INT8U UpLoadSdata[64];//������֤ǩ��
INT8U UpLoadKeyId;
INT8U UoLoadMD5[16];

INT8U SGCConsultrandom[8];//��ԿЭ�������
//INT8U SGCSymrandom[8];//���¶Գ���Կ�����
////INT8U g_EnDone = 0;

INT8U rmparaflag = 0;
INT8U CAnum = 0;
INT8U YWCernum = 0;
INT8U Upendflag = 0;


INT8U Sendnum = 0;
INT16U Sendlen = 0;
INT8U SendCount = 0;//�ѷ����˶���֡


INT16U LenFromEbMsg = 0;
INT16U AuthEndflag = 0;
INT16U YWAuthEndflag = 0;

INT16U fixmlen = 0;
INT16U enCAlen = 0;
INT16U YWCerlen = 0;

struct Sgc1120aFlag_t HnnwInf;

/********************************************************************
*�������ƣ�SgcGetChipSerialNumID
*���ܣ���ȡоƬ���к�
*���룺rcvbuf:���ݴ�Ż�����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161GetChipSerialNumID(INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x00;
    hecmd.ins  = 0xB0;
    hecmd.p1   = 0x99;
    hecmd.p2   = 0x05;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x02;
    pHeSendBuf2[0] = 0x00;   
    pHeSendBuf2[1] = 0x08;    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, 2);  //
    
    myTaskDelay(5);    
    
    rc = SGCReceiveData(rcvbuf,8);
    semGive(sem_qspiid);
    if(rc == 0)
    {
        //logSysMsgNoTime("��ȡоƬ���к���ȷ:",0,0,0,0);
    }
    else
    {
       // logSysMsgNoTime("��ȡоƬ���к�ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   
}

/********************************************************************
*�������ƣ�Sgc1161GetOriCerlen
*���ܣ���ȡ��ʼ֤�鳤��
*���룺rcvbuf:���ݴ�Ż�����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161GetOriCerlen(INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x00;
    hecmd.ins  = 0xB0;
    hecmd.p1   = 0x81;
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x02;
    pHeSendBuf2[0] = 0x00;   
    pHeSendBuf2[1] = 0x02;    
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    
    HESendCmd(&hecmd, pHeSendBuf2, 2);  //
    
    myTaskDelay(2);    
    
    rc = SGCReceiveData(rcvbuf,4);
    semGive(sem_qspiid);
        
    if(rc == 0)
    {
        //logSysMsgNoTime("��ȡоƬ���к���ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��ȡ��ʼ֤�鳤��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   
}

/********************************************************************
*�������ƣ�Sgc1161GetOriCerData
*���ܣ���ȡ��ʼ֤��
*���룺rcvbuf:���ݴ�Ż�����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161GetOriCerData(INT8U *rcvbuf,INT16U lenth)
{
    struct HESendCmd_t hecmd;
    int rc,i,j;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x00;
    hecmd.ins  = 0xB0;
    hecmd.p1   = 0x81;
    hecmd.p2   = 0x02;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x02;    

    pHeSendBuf2[0] = HIBYTE(lenth);  
    pHeSendBuf2[1] = LOBYTE(lenth);  
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    
    HESendCmd(&hecmd, pHeSendBuf2, 2);  //
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(rcvbuf,1024);
    semGive(sem_qspiid);
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(1);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        
        HESendCmd(&hecmd, pHeSendBuf2, 2);  // 
        
        myTaskDelay(2);    
        
        rc = SGCReceiveData(rcvbuf,1024);
        
        semGive(sem_qspiid);
        
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)
            {
                myTaskDelay(1);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, 2);  // 
                
                myTaskDelay(2);    
                
                rc = SGCReceiveData(rcvbuf,1024);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }
        
        j++;
        
        if(j > 3)
        {
            break;
        }
        
    }
    
    if(rc == 0)
    {
        //logSysMsgNoTime("��ȡоƬ���к���ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��ȡ��ʼ֤��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   
}

/********************************************************************
*�������ƣ�SgcGetChipKeyVersion
*���ܣ���ȡоƬ��Կ�汾��
*���룺pdata���ݽ��ջ�����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161GetChipKeyVersion(INT8U *pdata,INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;

    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x1A;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x00;
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    
    HESendCmd(&hecmd, NULL, 0);  //
    
    myTaskDelay(2);    
    
    rc = SGCReceiveData(rcvbuf,3);
    semGive(sem_qspiid);
    
    if(rc == 0)
    {
        //logSysMsgNoTime("��ȡ��Կ�汾����ȷ:",0,0,0,0);
    }
    else
    {
        //logSysMsgNoTime("��ȡ��Կ�汾��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;       
    
}
/********************************************************************
*�������ƣ�SgcVerify1161MasterSignData
*���ܣ���֤��վǩ��
*���룺pdataǩ������,KeyId��ǩ����Կ����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161VerifyMasterSignData(INT8U KeyId,INT8U* pdata)
{
    struct HESendCmd_t hecmd;
    int rc;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x18;
    hecmd.p1   = 0x00;
    hecmd.p2   = KeyId;
    hecmd.len1 = 0x00;
    hecmd.len2 = SGCSIGNDATALEN;
        
    memcpy(&pHeSendBuf2[0], pdata, SGCSIGNDATALEN);  
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, SGCSIGNDATALEN);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(NULL,350);
    semGive(sem_qspiid);
    
    if(rc == 0)
    {
        //logSysMsgNoTime("��֤��վǩ����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��֤��վǩ������rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;         
}
/********************************************************************
*�������ƣ�SgcGet1161RanSignData
*���ܣ������������ǩ��
*���룺pdata:��վ�����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161GetRanSignData(INT8U* pdata,INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U i,j;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x16;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x80;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x08;    
    
    memcpy(&pHeSendBuf2[0], pdata, SGCRANDOMLEN);   
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, SGCRANDOMLEN);  //      
    
    myTaskDelay(5);    
    
    rc = SGCReceiveData(rcvbuf,350);
    semGive(sem_qspiid);
    
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(2);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, SGCRANDOMLEN);  //      
        myTaskDelay(2);    
        rc = SGCReceiveData(rcvbuf,350);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)
            {
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, SGCRANDOMLEN);  // 
                
                myTaskDelay(2);    
                
                rc = SGCReceiveData(rcvbuf,350);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }
        
        j++;
        
        if(j > 3)
        {
            break;
        }
        
    }
    


    if(rc == 0)
    {
        //memcpy(SgcSelfRandbuf, pdata+2, SGCRANDOMLEN);    
        //logSysMsgNoTime("�����ǩ��������ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("�����ǩ������ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;          
}
/********************************************************************
*�������ƣ�Sgc1161EncryptData
*���ܣ����ݼ���
*���룺pdata:����������,lenth���������ݳ���
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161EncryptData(INT8U *pdata,INT16U lenth)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U i,j;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x26;
    hecmd.p1   = 0x60;
    hecmd.p2   = 0x01;
    hecmd.len1 = HIBYTE(lenth+16);
    hecmd.len2 = LOBYTE(lenth+16);     
    
    memcpy(&pHeSendBuf2[0], SgcMasterRandbuf, 16); 
    memcpy(&pHeSendBuf2[16], pdata, lenth);  
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1); 
    // //��������������������������ʱ���п��ܻᱻ¼��дflashӰ�����ָ��ķ���   
    HESendCmd(&hecmd, pHeSendBuf2, lenth+16);  //      
    
     myTaskDelay(3);    
    
    rc = SGCReceiveData(pdata,350);
    semGive(sem_qspiid);
    j = 0;
    
    while(rc == 0x11)
    {
        myTaskDelay(2);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth+16);  //      
        myTaskDelay(2);    
        rc = SGCReceiveData(pdata,350);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)//�������֧���ط�����
            {
            
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, lenth+16);  //      
                myTaskDelay(2);    
                rc = SGCReceiveData(pdata,350);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }
        
        j++;
        
        if(j > 3)
        {
            break;
        }
    }


    
    if(rc == 0)
    {
        //logSysMsgNoTime("���ݼ�����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("���ݼ���ʧ��!!!.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*�������ƣ�Sgc1161DecryptData
*���ܣ����ݽ���
*���룺pdata:����������,lenth���������ݳ���
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161DecryptData(INT8U *pdata,INT16U lenth,INT8U *dndata)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U i,j;
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x2C;
    hecmd.p1   = 0x60;
    hecmd.p2   = 0x01;
    hecmd.len1 = HIBYTE(lenth+16);
    hecmd.len2 = LOBYTE(lenth+16 );     
    
    //logSysMsgNoTime("len1=%xlen2=%x",hecmd.len1,hecmd.len2,0,0);
    
    memcpy(&pHeSendBuf2[0], SgcMasterRandbuf, 16); 
    memcpy(&pHeSendBuf2[16], pdata, lenth);    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth + 16);  //     
    
    //logSysMsgNoTime("lenth=%d",lenth,0,0,0);
    if(lenth > 500)
    {
        myTaskDelay(5);    
    }
    else
    {
        myTaskDelay(5);    
    }
    
    rc = SGCReceiveData(dndata,330);
    semGive(sem_qspiid);
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(1);    
        hecmd.cla  = 0x80;
        hecmd.ins  = 0x2C;
        hecmd.p1   = 0x60;
        hecmd.p2   = 0x01;
        hecmd.len1 = HIBYTE(lenth+16);
        hecmd.len2 = LOBYTE(lenth+16 );     
        
        //logSysMsgNoTime("len1=%xlen2=%x",hecmd.len1,hecmd.len2,0,0);
        
        memcpy(&pHeSendBuf2[0], SgcMasterRandbuf, 16); 
        memcpy(&pHeSendBuf2[16], pdata, lenth);  
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth + 16);  //    
        myTaskDelay(5);    
        rc = SGCReceiveData(dndata,1024);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)
            {
            
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, lenth + 16);  //          
                myTaskDelay(5);    
                rc = SGCReceiveData(dndata,1024);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }        
        }

        j++;
        //logSysMsgNoTime("���ݽ���ʧ��.rc=%d",rc,0,0,0);
        if(j > 3)
        {
            break;
        }
    }


    
    if(rc == 0)
    {
        //logSysMsgNoTime("���ݽ�����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("���ݽ���ʧ��.rc=%d",rc,0,0,0);
    }
    
   // semGive(sem_qspiid);
    
    return rc;        
}



/********************************************************************
*�������ƣ�Sgc1161VerifySigndata
*���ܣ���֤ǩ��
*���룺pdataǩ������+ǩ��,KeyId��ǩ����Կ����
*�з��ˣ�����
*********************************************************************/

INT8U Sgc1161VerifySigndata(INT8U *pdata,INT16U lenth,INT8U KeyId)
{
    struct HESendCmd_t hecmd;
    int rc;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x08;
    hecmd.p1   = 0x00;
    hecmd.p2   = KeyId;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth);    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(NULL,350);
    semGive(sem_qspiid);
    if(rc == 0)
    {
        //logSysMsgNoTime("��֤ǩ����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��֤ǩ��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;       
}
/********************************************************************
*�������ƣ�Sgc1161ObtainRandata
*���ܣ���ȡ�����
*���룺
*�з��ˣ�����
*********************************************************************/

INT8U Sgc1161ObtainRandata(void)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U i,j;
    INT8U buf[10];
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x00;
    hecmd.ins  = 0x84;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x08;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x00;
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, NULL, 0);  //
    
    myTaskDelay(2);    
    
    rc = SGCReceiveData(buf,8);
    semGive(sem_qspiid);
    
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(2);  
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, NULL, 0);  //
        myTaskDelay(2);    
        rc = SGCReceiveData(buf,8);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)//�������֧���ط�����
            {
                
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, NULL, 0);  //
                myTaskDelay(2);    
                rc = SGCReceiveData(buf,8);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }
        
        j++;
        //logSysMsgNoTime("��ȡоƬ�����ʧ��!1",0,0,0,0);
        if(j > 3)
        {
            break;
        }
        
    }


    
    if(rc == 0)
    {
        memcpy(SgcSelfRandbuf,buf+2,8);
        //logSysMsgNoTime("��ȡоƬ�������ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��ȡоƬ�����ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;     
}
/********************************************************************
*�������ƣ�Sgc1161LoadSymmetryKey
*���ܣ�д��Գ���Կ
*���룺pdata�Գ���Կ+ǩ��,KeyId��ǩ����Կ����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161LoadSymmetryKey(INT8U* pdata,INT16U lenth,INT8U KeyId)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U keyver = 0;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    keyver = pdata[0];
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x1C;
    if(keyver > 0)
    {
        hecmd.p1   = 0x00;//������Կ
    }
    else
    {
        hecmd.p1   = 0x01;//�ָ���Կ
    }
    
    hecmd.p2   = KeyId;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth);  
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(pdata,350);
    semGive(sem_qspiid);
    if(rc == 0)
    {
        //logSysMsgNoTime("�Գ���Կд����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("�Գ���Կд��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*�������ƣ�Sgc1161DeCerdata
*���ܣ�����֤��
*���룺pdata��֤��,KeyId���Գ���Կ��ʶ
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161DeCerdata(INT8U* pdata,INT16U lenth,INT8U KeyId)
{
    struct HESendCmd_t hecmd;
    int rc;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x04;
    hecmd.p1   = 0x00;
    hecmd.p2   = KeyId;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth);    
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(pdata,1024);
    semGive(sem_qspiid);
    if(rc == 0)
    {
        //logSysMsgNoTime("֤�������ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("֤�����ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161DeCerdata
*���ܣ�������վ�����أ��ն�֤��
*���룺pdata��֤��,CerId:֤��ID
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161LoadCerdata(INT8U* pdata,INT16U lenth,INT8U CerId)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U rbuf[7];
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x22;
    hecmd.p1   = CerId;
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth);  
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(5);    
    
    rc = SGCReceiveData(rbuf,7);
    semGive(sem_qspiid);
    if(rc == 0)
    {
       // logSysMsgNoTime("֤��д����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("֤��д��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161LoadTemSelfCerdata
*���ܣ������ն�֤��
*���룺pdata��֤��,
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161LoadTemSelfCerdata(INT8U* pdata,INT16U lenth )
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U rbuf[7];
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x24;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth);   
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(rbuf,7);
    semGive(sem_qspiid);
    if(rc == 0)
    {
       // logSysMsgNoTime("�ն�֤��д����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("�ն�֤��д��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161CheckoutCer
*���ܣ������ն�оƬ֤��
*���룺pdata��֤�鵼��������
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161CheckoutCer(INT8U *pdata)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U i,j;
    INT16U lenth = 0x0000;
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x30;
    hecmd.p1   = 0x01;
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], NULL, lenth);  
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(3);    
    
    rc = SGCReceiveData(pdata,1024);
    semGive(sem_qspiid);
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(2);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth);  //      
        myTaskDelay(2);    
        rc = SGCReceiveData(pdata,1024);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)//�������֧���ط�����
            {
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);  
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
                
                myTaskDelay(2);    
                
                rc = SGCReceiveData(pdata,1024);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }
        j++;
        
        if(j > 3)
        {
            break;
        }
    }

        
    if(rc == 0)
    {
       // logSysMsgNoTime("�ն�֤��д������ȷ:",0,0,0,0);
    }
    else
    {
        //logSysMsgNoTime("�ն�֤�鵼��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161CheckoutPKey
*���ܣ������ն�оƬ��Կ
*���룺pdata��Կ����������
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161CheckoutPKey(INT8U *pdata)
{
    struct HESendCmd_t hecmd;
    INT8U i,j;
    int rc;
    INT16U lenth = 0x0000;
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x30;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], NULL, lenth); 
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(5);    
    
    rc = SGCReceiveData(pdata,0x50);
    semGive(sem_qspiid);
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(2);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
        myTaskDelay(2);    
        rc = SGCReceiveData(pdata,0x50);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)//�������֧���ط�����
            {
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
                
                myTaskDelay(2);    
                
                rc = SGCReceiveData(pdata,1024);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }
        j++;
        
        if(j > 3)
        {
            break;
        }
    }

    if(rc == 0)
    {
       // logSysMsgNoTime("�ն�֤��д������ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("�����ն�оƬ��Կʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161VerifyMaintDevCer
*���ܣ���֤��ά�ն�֤��
*���룺pdata��֤��,
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161VerifyMaintDevCer(INT8U* pdata,INT16U lenth )
{
    struct HESendCmd_t hecmd;
    int rc;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x1E;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth);   
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(pdata,350);
    semGive(sem_qspiid);
    if(rc == 0)
    {
       // logSysMsgNoTime("��ά�ն�֤����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��ά�ն�֤�����.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161VerifyMaintDevSigndata
*���ܣ���֤��ά�ն�ǩ��
*���룺pdata��ǩ��
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161VerifyMaintDevSigndata(INT8U* pdata,INT16U lenth )
{
    struct HESendCmd_t hecmd;
    int rc;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x20;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = SGCSIGNDATALEN;
        
    memcpy(&pHeSendBuf2[0], pdata, SGCSIGNDATALEN); 
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, SGCSIGNDATALEN);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(pdata,350);
    semGive(sem_qspiid);
    if(rc == 0)
    {
        //logSysMsgNoTime("��ά�ն�֤����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��ά�ն�֤�����.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161VfyYWCer
*���ܣ���֤֤�������֤����Ч��
*���룺pdata��ǩ��
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161VfyYWCer(INT8U* pdata,INT16U lenth )
{
    struct HESendCmd_t hecmd;
    int rc,i;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x1E;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);

    memcpy(&pHeSendBuf2[0], pdata, lenth);   
    
    myTaskDelay(1);    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(3);    
    
    rc = SGCReceiveData(NULL,350);
    semGive(sem_qspiid);
    i = 0;
    while(rc == 77)
    {
        myTaskDelay(1);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
        
        myTaskDelay(3);    
        
        rc = SGCReceiveData(NULL,1024);
        semGive(sem_qspiid);
        i++;
        if(i>3)
        {
            break;
        }
    }
    
    if(rc == 0)
    {
        //logSysMsgNoTime("��ά�ն�֤����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��֤֤�������֤����Ч�Դ���.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   

}

/********************************************************************
*�������ƣ�Sgc1161EncryptPbKey
*���ܣ���Կ����(��֤������ߴ�������)
*���룺pdata:֤�������ID��R1���������ʼ����,lenth���������ݳ���
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161EncryptPbKey(INT8U *pdata,INT16U lenth)
{
    struct HESendCmd_t hecmd;
    int rc,i;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x26;
    hecmd.p1   = 0x62;
    hecmd.p2   = 0x02;    
    hecmd.len1 = HIBYTE(lenth+32);//ID + R1+IVData(R1+R1ȡ��)
    hecmd.len2 = LOBYTE(lenth+32);     
        
    memcpy(&pHeSendBuf2[0], SgcCerToolIDbuf, 8); 
    memcpy(&pHeSendBuf2[8], SgcRdDatatoYWbuf, 8); 
    memcpy(&pHeSendBuf2[16], SgcRdDatatoYWbuf, 16);    
    memcpy(&pHeSendBuf2[32], pdata, lenth); 
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth+32);  //   
    
     myTaskDelay(5);    
    
    rc = SGCReceiveData(pdata,350);
    semGive(sem_qspiid);
    i = 0;
    while(rc == 0x11)
    {
        myTaskDelay(1);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth+32);  //   
        myTaskDelay(3);    
        rc = SGCReceiveData(pdata,350);
        semGive(sem_qspiid);
        i++;
        
        if(i > 5)
        {
            break;
        }
    }
        
    if(rc == 0)
    {
        //logSysMsgNoTime("���ݼ�����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��Կ����ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*�������ƣ�Sgc1161DecryptYWFileData
*���ܣ�����֤�������֤����������
*���룺pdata:lenth���������ݳ���
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161DecryptYWFileData(INT8U *pdata,INT16U lenth,INT8U *dndata)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U i,j;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x2C;
    hecmd.p1   = 0x62;
    hecmd.p2   = 0x02;
    hecmd.len1 = HIBYTE(lenth+32);//ID + R1+IVData(R1+R1ȡ��)
    hecmd.len2 = LOBYTE(lenth+32 );     
    
    memcpy(&pHeSendBuf2[0], SgcCerToolIDbuf, 8); 
    memcpy(&pHeSendBuf2[8], SgcRdDatatoYWbuf, 8); 
    memcpy(&pHeSendBuf2[16], SgcRdDatatoYWbuf, 16);    
    memcpy(&pHeSendBuf2[32], pdata, lenth);   
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth + 32);  //     
    
    myTaskDelay(5);    
    
    rc = SGCReceiveData(dndata,1024);
    semGive(sem_qspiid);
    j =  0;
    while(rc == 0x11)
    {
        myTaskDelay(2);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth + 32);  //     
        myTaskDelay(2);    
        rc = SGCReceiveData(dndata,1024);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)
            {
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, lenth + 32);  //          
                myTaskDelay(2);    
                rc = SGCReceiveData(dndata,350);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }  

        j++;
        if(j > 3)
        {
            break;
        }
        
    }
    
      
        
    if(rc == 0)
    {
        //logSysMsgNoTime("���ݽ�����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("����֤�������֤����������ʧ��.rc=%d",rc,0,0,0);
    }
    
   // semGive(sem_qspiid);
    
    return rc;        
}
/********************************************************************
*�������ƣ�Sgc1161SignYWData
*���ܣ���֤�����������ǩ��
*���룺pdata��
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161SignYWData(INT8U* pdata,INT16U lenth )
{
    struct HESendCmd_t hecmd;
    int rc,i,j;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x0A;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x80;
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth);
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(5);    
    
    rc = SGCReceiveData(pdata,350);
    semGive(sem_qspiid);
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(1);    
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
        HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
        myTaskDelay(3);    
        rc = SGCReceiveData(pdata,350);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)
            {
                myTaskDelay(1);   
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
                myTaskDelay(3);    
                rc = SGCReceiveData(pdata,350);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }
        }  

        j++;
        if(j > 3)
        {
            break;
        }
    }
    
    if(rc == 0)
    {
       // logSysMsgNoTime("��ά�ն�֤����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("��ά�ն�֤�����.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*�������ƣ�Sgc1161RecoveryKeydata
*���ܣ�֤������߻ָ���Կ
*���룺pdata  
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1161RecoveryKeydata(INT8U* pdata,INT16U lenth)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U keyver = 0;
    
    //semTake(sem_qspiid, WAIT_FOREVER);
    keyver = pdata[0];
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x1C;
    hecmd.p1   = 0x02;//�ָ���Կ
    hecmd.p2   = 0x00;
    
    hecmd.len1 = HIBYTE(lenth);
    hecmd.len2 = LOBYTE(lenth);
        
    memcpy(&pHeSendBuf2[0], pdata, lenth); 
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, lenth);  //  
    
    myTaskDelay(10);    
    
    rc = SGCReceiveData(pdata,350);
    semGive(sem_qspiid);
    if(rc == 0)
    {
        //logSysMsgNoTime("�Գ���Կд����ȷ:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("�Գ���Կд��ʧ��.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*�������ƣ�GetEbMegCheckSum
*���ܣ������ۼӺ�
*���룺pdata������
*�з��ˣ�����
*********************************************************************/
INT8U GetEbMsgCheckSum(INT8U *pdata)
{
    INT8U sum = 0;
    INT8U *p;
    INT16U i;
    
    p = pdata + 4;
    i = pdata[2]+(pdata[1]<<8);
    
    while(i--)
    {
        sum += *p++ & 0xFF;
    }
    return sum;    
}
/********************************************************************
*�������ƣ�CheckIllfgalType
*���ܣ�������վ�·��ı���Ӧ�������Ƿ�Ϸ�
*���룺pdata�����ģ�type:Ӧ�����ͣ�wChanNo:��Լ�˿ں�
*�з��ˣ�����
*********************************************************************/

int CheckIllfgalType(INT8U *ptada,INT8U type,INT16U wChanNo)
{
    INT8U DncryptTi = 0;
    INT8U DncryptCot = 0;
    INT8U DncryptPI = 0;
    INT8U Filetype = 0;
    
    int rc = 1;
    
    if(wChanNo < 6)//101����
    {
    	
        /*
    	if(fixmlen == 6)//ȷ����ַ����һ���ֽڻ��������ֽ�
	    {
	        DncryptTi = ptada[7];
	        DncryptCot = ptada[9]&0x3F;
	    }
	    else
	    {
	        DncryptTi = ptada[6];
	        DncryptCot = ptada[8]&0x3F;
	    }
	    */
	    
	    DncryptTi = ptada[fixmlen + 1];
	    DncryptCot = ptada[fixmlen + 3]&0x3F;
	    DncryptPI = ptada[fixmlen +9];
           if(DncryptTi == 210)
           {
               Filetype = ptada[fixmlen +10];
           
           }
    }
    else if(wChanNo > 40)//104����
    {
           DncryptTi = ptada[6];
	    DncryptCot = ptada[8]&0x3F;
	    if(DncryptTi == 203)
	    {
	        DncryptPI = ptada[14];
//�޸Ĳ�����������ʶ��ң�غ����������ǰһ���ֽڣ���Ϊ��ֵ����ֻ�������ֽڣ�
	    }
	    else
	    {
	        DncryptPI = ptada[15];
	    }
        
           if(DncryptTi == 210)
           {
               Filetype = ptada[16];
           }
    }
    switch(DncryptTi)
    {
    	case 45:
    	case 46://ң��TI
    	    if(((DncryptPI&0x80) != 0)||(DncryptCot == 8))//ң��ѡ��/����
    	    {
    	    	if(type != 0x05)
    	    	{
    	    	    rc = -1;
    	        }
    	    }
    	    else//ң��ִ��
    	    {
    	    	if(type != 0x07)
    	    	{
    	    	    rc = -1;
    	        }    	    	
    	    }
    	    break;
    	case 200://�л���ֵ��TI
    	
    	    if(DncryptCot == 6)
    	    {
                if(type != 0x01)
                {
                    rc = -1;
                }  
    	    }
           break;
           
    	case 210://д�ļ�����

    	    if((DncryptCot == 6)&&(Filetype == 0x07))//
    	    {
  	    	 if(type != 0x01)
    	    	 {
    	    	    rc = -1;
    	        }      	    	
    	    }
    	
    	    break;    	
    	case 203:    	    
    	    if((DncryptPI&0x80) != 0)//����Ԥ��(��ֹ�Ļ���0x40)
    	    {
  	    	if(type != 0x01)
    	    	{
    	    	    rc = -1;
    	       }
              rmparaflag = 1;
    	    }
    	    else if((DncryptCot == 0x06)&&((DncryptPI&0x40) == 0))//�����̻�
    	    {
  	    	if(type != 0x03)
    	    	{
    	    	    rc = -1;
    	       }  
              rmparaflag = 2;
    	    }
           else if((DncryptPI&0x40) != 0)
           {
               if((rmparaflag == 1)&&(type != 0x01))//����Ԥ��ȡ��
               {
                   rc = -1;
               }
               else if((rmparaflag == 2)&&(type != 0x03))//�����̻�ȡ��
               {
                   rc = -1;
               }
               rmparaflag = 0;
           }
    	    break;
        /*    	
        case 210:
    	    if(DncryptCot == 6)//д�ļ�����ֻ��Ҫ�ж�cot���ɣ�ֻ��д�ļ�������6
������ȷ��7�����ݴ���5��
    	    {
  	    	    if(type != 0x01)
    	    	{
    	    	    rc = -1;
    	        }      	    	
    	    }        
        
        break;
        */
        case 211:
    	    if(((DncryptPI&0x80) != 0)&&(DncryptCot == 6))//��������
    	    {
    	    	if(type != 0x01)
    	    	{
    	    	    rc = -1;
    	        }
    	    }
    	    //else//����ִ��
    	    //{
    	    	//if(type != 0x03)
    	    	//{
    	    	   // rc = -1;
    	        //}    	    	
    	    //}
    	    break;            
    	
    	default:
    	    break;
    } 
    return rc;   
}

/********************************************************************
*�������ƣ�EbEncpytDataAnaly
*���ܣ�������վ�·����������ݣ����ܲ�����
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U EbEncpytDataAnaly(INT8U* pdata,INT16U lenth,INT8U *dedata,INT16U wChanNo)
{
    INT8U ddatalen,bwlen;
    INT8U *p;
    INT8U sgcbuf[330];
    //int typid = 0;
    int rc;
    
    bwlen = Sgc1161DecryptData(pdata,lenth,sgcbuf);//���ܻ����������
    /*
    if(bwlen != 0)
    {
        myTaskDelay(10);   
        
        bwlen = Sgc1161DecryptData(pdata,lenth,sgcbuf);//����ʧ�����½���

        if(bwlen != 0)
        {
            myTaskDelay(10);   
            
            bwlen = Sgc1161DecryptData(pdata,lenth,sgcbuf);//����ʧ�����½���
        }
        
        if(bwlen != 0)
        {
            EbErrCodeSend(0x9103,0x1f,wChanNo);//����ʧ��
        }
    }
    */
    if(bwlen != 0)
    {
        EbErrCodeSend(0x9103,0x1f,wChanNo);//����ʧ��
    }
    //p = sgcbuf + 4;//sgcbuf��ǰ�����ֽ��ǰ�ȫоƬ���ص����ݳ��ȣ��������ĸ��ֽ�����վ��֡ʱ��ӵĳ���?

    p = sgcbuf + 2;

    ddatalen = sgcbuf[1]+(sgcbuf[0]<<8);//���ı��ĳ���
    saveRecord(sgcbuf,ddatalen+2,RXSAVEMODE,0);
    
    rc = CheckIllfgalType(p+2,p[0],wChanNo);
    
    if(rc  < 0)
    {
        EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
        bwlen = 0;
        logSysMsgNoTime("ҵ�����ʹ���",0,0,0,0);
        return bwlen;
    }


    switch(p[0])//�ж�ҵ��Ӧ�����ͣ�Ŀǰֻ�õ���00 01 02 03 05 07
    {
	    case 0x00:
			bwlen = p[1];//101���ĳ���
			memcpy(dedata,p+2,bwlen);
			break;
	    case 0x01:
			bwlen = EbMsgWithSData(p,dedata,ddatalen,wChanNo);
			break;
	    case 0x02://�ն˶���վ��ȷ�ϱ��ģ���վ�����·�02
                    EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
                    bwlen = 0;
			break;
           case 0x03:
			bwlen = EbMsgWithRandSData(p,dedata,ddatalen,wChanNo);
			break; 
	    case 0x05:
			bwlen = EbMsgWithTandSData(p,dedata,ddatalen,wChanNo);
			break;			
	    case 0x07:
                    bwlen = EbMsgWithAllData(p,dedata,ddatalen,wChanNo);
			break;
           case 0x08:
                    bwlen = EbMsgUpLoadData(p,dedata,ddatalen,wChanNo);
                break;
           default:            
                    EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
                    bwlen = 0;
			break;
    }
    return bwlen;
	
}
/********************************************************************
*�������ƣ�EbMsgWithAllData
*���ܣ������ʱ��������Լ�ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/

INT8U EbMsgWithAllData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U timebuf[6];
    INT8U randombuf[8];
	//INT8U signdbuf[SGCSIGNDATALEN];
    INT8U verbuf[255];
    struct SysTime_t Time;
    
	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//101���ĳ���
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//��չ����������

	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);
	memcpy(verbuf+tmplen,pdata+seek,extlen-1);//��չ������ƴ����101���ĺ󣬼���ǩ����
	seek += 6;
	memcpy(randombuf,pdata+seek,8);
    
       if(CheckTimeAging(timebuf) != 0)
       {
           EbErrCodeSend(0x9105,0x1f,wChanNo);//
           logSysMsgNoTime("07����ʱ���У�����:",0,0,0,0);
           return 0;
       }
    
       if(memcmp(randombuf,SgcSelfRandbuf,8) != 0)
       {
           EbErrCodeSend(0x9104,0x1f,wChanNo);//
           logSysMsgNoTime("07�����У�����:",0,0,0,0);         
           return 0;
       }
        
	//seek += 8;
	//memcpy(signdbuf,pdata+seek,SGCSIGNDATALEN);

	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
           EbErrCodeSend(0x9102,0x1f,wChanNo);//
           logSysMsgNoTime("07������֤ǩ������:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*�������ƣ�EbMsgUpLoadData
*���ܣ������ʱ��������Լ�ǩ������������֤����
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/
INT8U EbMsgUpLoadData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
    INT8U rc,seek;
    INT16U tmplen,extlen;
    //INT8U timebuf[6];
    //INT8U randombuf[8];
	//INT8U signdbuf[SGCSIGNDATALEN];
    //INT8U verbuf[255];
    struct SysTime_t Time;
    
	tmplen = pdata[2]+(pdata[1]<<8);
    seek = 3;
        
	memcpy(UpLoadtime,pdata+seek,6);
	seek += 6;
	memcpy(UpLoadRdata,pdata+seek,8);
	seek += 8;
	memcpy(UpLoadSdata,pdata+seek,64);
	seek += 64;
    UpLoadKeyId = pdata[seek];
       
       /*    
       if(CheckTimeAging(UpLoadtime) != 0)
       {
           EbErrCodeSend(0x9105,0x1f,wChanNo);//
           logSysMsgNoTime("08����ʱ���У�����:",0,0,0,0);
           return 0;
       }
    
       if(memcmp(UpLoadRdata,SgcSelfRandbuf,8) != 0)
       {
           EbErrCodeSend(0x9104,0x1f,wChanNo);//
           logSysMsgNoTime("08�����У��������:",0,0,0,0);         
           return 0;
       }
    
	//seek += 8;
	//memcpy(signdbuf,pdata+seek,SGCSIGNDATALEN);

	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
           EbErrCodeSend(0x9102,0x1f,wChanNo);//
           logSysMsgNoTime("07������֤ǩ������:",0,0,0,0);
	    return 0;
	}
*/	
	return 0;//���� 
}

/********************************************************************
*�������ƣ�SGCVerifyUpLoadData
*���ܣ���֤��������֤����
*���룺pdata��
*�з��ˣ�����
*********************************************************************/

INT8U SGCVerifyUpLoadData(INT16U wChanNo)
{
    INT8U rc;
    INT8U verbuf[94];
    INT16U len;
    
    if(CheckTimeAging(UpLoadtime) != 0)
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9105,0x1f,wChanNo);//
        logSysMsgNoTime("08����ʱ���У�����:",0,0,0,0);
        return 0;
    }
    
    if(memcmp(UpLoadRdata,SgcSelfRandbuf,8) != 0)
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        logSysMsgNoTime("08�����У�����:",0,0,0,0);         
        return 0;
    }
    
    memcpy(verbuf,UoLoadMD5,16);
    memcpy(verbuf+16,UpLoadtime,6);
    memcpy(verbuf+22,UpLoadRdata,8);
    memcpy(verbuf+30,UpLoadSdata,64);
    
    rc = Sgc1161VerifySigndata(verbuf,94,UpLoadKeyId);//16+6+8+64
    
    if(rc == 0)
    {
       // EbErrCodeSend(0x9000,0x1f,wChanNo);//�ɹ������ش�����
        StartProgramUpdate();
        return 0;
    }
    else
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("08������֤ǩ������:",0,0,0,0);
	 return 0;
    }
    
}

/********************************************************************
*�������ƣ�SGCVerifyMD5UpLoadData
*���ܣ���ȡMD5ֵ��У��ʱ��ǩ�������
*���룺pdata��
*�з��ˣ�����
*********************************************************************/
INT8U UpgradeDataVerify(INT16U wChanNo)
{
    if(Upendflag == 0)
    {
        return 0;
    }
    if(UpdateProgramMd5() == TRUE)
    {
        if(Upendflag == 1)
        {
			SGCVerifyUpLoadData(wChanNo);
        }
		else if(Upendflag == 2)
		{
			SGC1120aVerifyUpLoadData(wChanNo);		    
		}
    }
    else
    {
        ClearProgramUpdate();
    }
    Upendflag = 0;
	return 1;
}
/********************************************************************
*�������ƣ�EbMsgWithRandSData
*���ܣ������������Լ�ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/

INT8U EbMsgWithRandSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
/*
    INT8U rc;
    INT16U tmplen;
	
	tmplen = pdata[1];//101���ĳ���	
	rc = VerifyMsgWithExt(pdata,len);
*/
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U randbuf[8];
    INT8U verbuf[1024];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//��Ч���ݳ���
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//��չ����������

	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(randbuf,pdata+seek,8);
    
       if(memcmp(randbuf,SgcSelfRandbuf,8) != 0)
       {
           EbErrCodeSend(0x9104,0x1f,wChanNo);//
           //logSysMsgNoTime("03�����У�����:",0,0,0,0);
          // logSysMsgNoTime("Rand,%x,%x,%x,%x",randbuf[0],randbuf[1],randbuf[2],randbuf[3]); 
           //logSysMsgNoTime("Rand,%x,%x,%x,%x",randbuf[4],randbuf[5],randbuf[6],randbuf[7]); 
           //logSysMsgNoTime("Rand,%x,%x,%x,%x",SgcSelfRandbuf[0],SgcSelfRandbuf[1],SgcSelfRandbuf[2],SgcSelfRandbuf[3]); 
          // logSysMsgNoTime("Rand,%x,%x,%x,%x",SgcSelfRandbuf[4],SgcSelfRandbuf[5],SgcSelfRandbuf[6],SgcSelfRandbuf[7]); 
           
           return 0;
       }

    
	memcpy(verbuf+tmplen,pdata+seek,extlen-1);//��չ������ƴ����101���ĺ󣬼���ǩ����
  
	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);

	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
	    
           EbErrCodeSend(0x9102,0x1f,wChanNo);//
           logSysMsgNoTime("03������֤ǩ������:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*�������ƣ�CheckTimeAging
*���ܣ����ʱ�����ʱЧ��
*���룺pdata��ʱ�������
*�з��ˣ�����
*********************************************************************/
int CheckTimeAging(INT8U *pdta)
{

    //INT8U timedata[6];
    INT8U selftimebuf[6];
    INT16U delay,mtime;
    struct SysTime_t Time;
    
    GetSysTime((void *)&Time,SYSTEMTIME);
    
    selftimebuf[0] = (INT8U)(Time.Year - 2000); 
    selftimebuf[1] = Time.Month;
    selftimebuf[2] = Time.Day;
    selftimebuf[3] = Time.Hour;
    selftimebuf[4] = Time.Minute;
    selftimebuf[5] = Time.Second;
    
    if(memcmp(pdta,selftimebuf,4) != 0)
    {
        logSysMsgNoTime("t err,%x,%x,%x,%x",pdta[0],pdta[1],pdta[2],pdta[3]); 
        logSysMsgNoTime("t err,%x,%x",pdta[4],pdta[5],0,0); 
        logSysMsgNoTime("t err,%x,%x,%x,%x",selftimebuf[0],selftimebuf[1],selftimebuf[2],selftimebuf[3]); 
        logSysMsgNoTime("t err,%x,%x",selftimebuf[4],selftimebuf[5],0,0); 
        return -1;
    }
    else
    {
        mtime = (INT16U)pdta[4]*60 + pdta[5];
        delay = (INT16U)selftimebuf[4]*60 + selftimebuf[5];

        if(mtime > delay)
        {
            delay = mtime - delay;
        }
        else
        {
            delay = delay - mtime;
        }
        
        if(delay > 60)  //Ŀǰ���Ժ�涨��һ���ӣ�����д���ڳ�����
        {
            
            logSysMsgNoTime("t err,%x,%x,%x,%x",pdta[0],pdta[1],pdta[2],pdta[3]); 
            logSysMsgNoTime("t err,%x,%x",pdta[4],pdta[5],0,0); 

            logSysMsgNoTime("t err,%x,%x,%x,%x",selftimebuf[0],selftimebuf[1],selftimebuf[2],selftimebuf[3]); 
            logSysMsgNoTime("t err,%x,%x",selftimebuf[4],selftimebuf[5],0,0); 
            return -1;
        }
    }
    
    return 0;
}

/********************************************************************
*�������ƣ�EbMsgWithTandSData
*���ܣ������ʱ���Լ�ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/

INT8U EbMsgWithTandSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
/*
    INT8U rc;
    INT16U tmplen;
	
	tmplen = pdata[1];//101���ĳ���	
	rc = VerifyMsgWithExt(pdata,len);
*/
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U timebuf[6];
    //INT8U timedata[6];
    INT8U verbuf[1024];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//��Ч���ݳ���
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//��չ����������

	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);
	memcpy(verbuf+tmplen,pdata+seek,extlen-1);//��չ������ƴ����101���ĺ󣬼���ǩ����


     if(CheckTimeAging(timebuf) != 0)
     {
         EbErrCodeSend(0x9105,0x1f,wChanNo);//
         logSysMsgNoTime("05����ʱ���У�����:",0,0,0,0);
         return 0;
     }
	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);

	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
	    
           EbErrCodeSend(0x9102,0x1f,wChanNo);//
           logSysMsgNoTime("05������֤ǩ������:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}
#if 0
/********************************************************************
*�������ƣ�VerifyMsgWithExt
*���ܣ���֤����չ�����ݱ���
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U VerifyMsgWithExt(INT8U *pdata,INT16U len )
{
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U verbuf[255];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//101���ĳ���	
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//��չ����������
	
	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(verbuf+tmplen,pdata+seek,extlen - 1);
	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);
	return rc;

}
#endif
/********************************************************************
*�������ƣ�EbMsgWithSData
*���ܣ�����ֻ��ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/

INT8U EbMsgWithSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U verbuf[350];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//101/104���ĳ���
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);
	
	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(verbuf+tmplen,pdata+seek,extlen - 1);
	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);
    
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
           EbErrCodeSend(0x9102,0x1f,wChanNo);//
           logSysMsgNoTime("01������֤ǩ������:",0,0,0,0);
           return 0;
	}
	return tmplen;
}


//EbSafetySearchFrame(EbMsgRxdBuf,RxdBuf+RxdTail,&RxdTail,RxdTail - TxdHead,wChanNo);

/********************************************************************
*�������ƣ�EbSafetySearchFrame
*���ܣ������յ���EB���ģ���������ת��Ϊ101/104֡
*���룺oribuf��EB���Ļ�����,validbuf:101/104���Ĵ�Ż�������
*validtaillen:RxdTail��len�����յ���EB����������,wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT16U EbSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo)
{
    //INT8U len;
    INT16U eblenth,i;
    INT16U checklen = 0;
    INT16U tail = 0; 
    //INT8U buf[18] = {0x01,0x0F,0x68,0x0D,0x0E,0x00,0x24,0x00,0xCB,0x00,0x06,0x00,0x01,0x00,0x01,0x00,0x00,0x00};
    


    i = 0;
    //len = oribuf[1];
    //Pack101msgtoEb(oribuf,len,validbuf,wChanNo);
   //EbmsgAnalysis(oribuf,validbuf,wChanNo);
   // CheckIllfgalType(buf+2,buf[0],42);
    //EnMsgBymasterHandle(oribuf,validbuf,wChanNo);

    //SgcYWToolReWritrOriCA(wChanNo);
    while (i < len)
    {
        if(oribuf[i] == 0xEb)
        {
            if(i +4 > len )
            {
                return i ;
            }
        }
        
        if(oribuf[i] == 0xEb&&oribuf[i + 3] == 0xEb)
        {

            checklen = oribuf[i+2]+(oribuf[i+1]<<8);

            if(checklen > (512 -6))//���checklen>256����� i ����ִ��++
            {
                i++;
            }
            else
            {
                if(checklen > (len- i))
                {
                    return i ;
                }
             
                if(CheckEbMsgSty((oribuf+ i)))
                {
    	            eblenth = oribuf[i+2]+(oribuf[i+1]<<8);
    	            EbmsgAnalysis(oribuf+i,((INT8U*)validbuf + tail),wChanNo);
    		        i += (eblenth + 6);
                    (*validtaillen) += LenFromEbMsg;
                    tail += LenFromEbMsg;
                    LenFromEbMsg = 0;
                    //saveRecord(validbuf,LenFromEbMsg,RXSAVEMODE,0x8000);
                }
                else
                {
                    i++;
                }
            }
        }
        else
        {
            i++;
        }    
        
    } 
    
    return i;
}
/********************************************************************
*�������ƣ�CheckEbMegSty
*���ܣ������Ƿ�EB����
*���룺pdata������
*�з��ˣ�����
*********************************************************************/
BOOL CheckEbMsgSty(INT8U *pdata)
{
    INT8U sum = 0;
    INT16U i = 0;
	
    if((pdata[0] == 0xEB)&&(pdata[3] == 0xEB))
    {
        //i = (INT16U)pdata[2]+((INT16U)pdata[1]<<8);
        //sum = pdata[i + 4];//
        i = (INT16U)pdata[1];
		i = (i<<8)+(INT16U)pdata[2];
		
        sum = GetEbMsgCheckSum(pdata);
        if((sum == pdata[i + 4])&&(pdata[i + 5] == 0xD7))
        {
            return TRUE;
        }
        else
        {
            logSysMsgNoTime("checksum err1 sum = %x",sum,0,0,0);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    
}

/********************************************************************
*�������ƣ�EbmegAnalysis
*���ܣ�����Eb��ʽ����
*���룺pdata�����ģ�rxbuff:101/104���Ĵ�Ż�������wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
void EbmsgAnalysis(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    

    switch(pdata[5] & 0xF0)
    {
        case 0x80://��������
            EnMsgByGetwayHandle(pdata,rxbuff,wChanNo);           
            break;
        case 0x40://��ά�ն�
            EnMsgByYWTool(pdata,rxbuff,wChanNo); 
            break;
            
        case 0x00://��վ
            LenFromEbMsg = EnMsgBymasterHandle(pdata,rxbuff,wChanNo);
            break;        
             
        case 0x0C://����
            break;        
        default:
            EbErrCodeSend(0x9110,0x1f,wChanNo);
            break;
    }
    
}

/********************************************************************
*�������ƣ�EbmegAnalysis
*���ܣ�����Eb��ʽ����
*���룺pdata�����ģ�rxbuff:101/104���Ĵ�Ż�������wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
void EbErrCodeSend(INT16U SW,INT8U TypeId,INT16U wChanNo)
{
        INT8U signbuf[17];
        INT8U replybuf[17];
        
        signbuf[0] = (INT8U)((SW>>8)&0x00FF);
        signbuf[1] = (INT8U)(SW&0x00FF);
        
        EbEditmsg(replybuf,signbuf,7, 0,TypeId,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo); 
}



/********************************************************************
*�������ƣ�EnMsgByGetwayHandle
*���ܣ����������·�������
*���룺pdata��EB���Ļ�������rxbuff:101/104���Ĵ�Ż�����wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U EnMsgByGetwayHandle(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    INT16U wEblenth;
	INT8U bwlen = 0;
	
    wEblenth = pdata[2]+(pdata[1]<<8);//���ĳ���(����ͷ��У����)

	switch(pdata[6])
	{
	    case 0x20://������֤������
	            AuthEndflag = 0;//���ӶϿ���������֤ʱ��Ҫ�Ƚ���֤��־λ��0
	            SgcGetwayauthenStepI(pdata+6,wEblenth,wChanNo);
                   myTaskDelay(10);
	            break;
	    case 0x22://
		      SgcGetwayauthenStepII(pdata+6,wEblenth,wChanNo);
                   myTaskDelay(10);
	            break;                                                    
	    default:
	            break;              
	        
	}
	return bwlen;
}

/********************************************************************
*�������ƣ�SgcGetwayauthenStepI
*���ܣ��������ն������֤��һ��
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U SgcGetwayauthenStepI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,wholelen,sdatalen;
    INT8U signbuf[100];
    INT8U replybuf[120];
    
    //datalen = pdata[2]+(pdata[1]<<8);//Ӧ�����ݳ���
    datalen = (INT16U)pdata[1];
	datalen = (datalen<<8)+(INT16U)pdata[2];
		
    memset(signbuf,0,100);
    memset(replybuf,0,120);
    memcpy(SgcMasterRandbuf,(pdata + 3),datalen);
    
    Sgc1161GetRanSignData(SgcMasterRandbuf,signbuf);

    sdatalen = signbuf[1]+(signbuf[0]<<8) + EBKEYIDLEN;
    wholelen = sdatalen + 5;//EB����ͷ�г���
    
    memcpy(signbuf,signbuf + 2,sdatalen);
    signbuf[sdatalen-1] = 0x01;//��β�����Կ����
    
    EbEditmsg(replybuf,signbuf,wholelen, 0X0080,0x21,sdatalen);
	
	//TxdHead=0;
	//TxdTail = wholelen + 6;
    //memcpy(TxdBuf,replybuf,TxdTail);
	
	//SendDataToMISI();
	SendAuthDataToMISI(replybuf,wholelen + 6,wChanNo);

    return 0;
}
/********************************************************************
*�������ƣ�SgcGetwayauthenStepI
*���ܣ��������ն������֤��2��
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U SgcGetwayauthenStepII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U Keyno,rc;
    //INT16U msglen;
    INT8U replybuf[30];
    INT8U signbuf[2];
    
    //msglen = pdata[2]+(pdata[1]<<8);
    //Keyno = pdata[msglen + EBHEADLEN - 1];
    
    //logSysMsgNoTime("��֤��վǩ����",0,0,0,0);
    rc = Sgc1161VerifyMasterSignData( 0x05, (pdata + 3));
    if(rc == 0)
    {    
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x90;
    }
    
	EbEditmsg(replybuf,signbuf,7, 0X0080,0x23,2);
	//TxdHead=0;
	//TxdTail = 7 + 6;
    //memcpy(TxdBuf,replybuf,TxdTail);
	
	//SendDataToMISI();	
    SendAuthDataToMISI(replybuf,7 + 6,wChanNo);

    return 0;
}
/********************************************************************
*�������ƣ�EnMsgBymasterHandle
*���ܣ�������վ�·�������
*���룺pdata������,rxbuff:101/104���Ĵ�Ż�������wChanNo���˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U EnMsgBymasterHandle(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    BOOL  Encrptyflag;
    INT16U wEblenth;
    INT8U bwlen = 0;
	
    wEblenth = pdata[2]+(pdata[1]<<8);//���ĳ���(����ͷ��У����)
    //SKeyId = pdata[EBHEADLEN + 1]&0x07;//�Գ���Կ��ʶ
    
    if(pdata[EBHEADLEN + 1]&0x08)//�Ƿ����
    {
        Encrptyflag = TRUE;
    }
    else
    {
        Encrptyflag = FALSE;        
    }
	
    if(Encrptyflag)
    {
    
        if( AuthEndflag == 0)
        {
    
            EbErrCodeSend(0x9107,0x1f,wChanNo);
            bwlen = 0;
        }
        
        bwlen = EbEncpytDataAnaly(pdata + 6, wEblenth - 2,rxbuff,wChanNo);//���ܻ����������
    
            //if(bwlen == 0)//Ӧ�����Ͳ���
            //{
               // EbErrCodeSend(0x9101,0x1f,wChanNo);//
           // }
    }
    else
    {
	    if( AuthEndflag == 0)
	    {
	        if(pdata[6]<0x50)//||pdata[6]>0x60)
	        {
	             if(pdata[6] == 0x1f)
	             {
	                 return 0;
	             }
	             EbErrCodeSend(0x9107,0x1f,wChanNo);
                    return 0;
	        }
	    }
	    switch(pdata[6])
	    {
	        case 0x50://��վ��֤������
                   AuthEndflag = 0;
	            SgcMasterauthenStepI(pdata,wEblenth,wChanNo);
                   myTaskDelay(2);
	            break;
	        case 0x52://
	            SgcMasterauthenStepII(pdata,wEblenth,wChanNo);
                  myTaskDelay(2);
	            break;     
	        case 0x54:
	            SgcMasterauthenStepIII(0x55,wChanNo);
                    myTaskDelay(2);
                    AuthEndflag = wChanNo;
                    /*
                    if(wChanNo < 6)//101�˿�
                    {
                         AuthEndflag = 1;
                    }
                    else if(wChanNo > 40)//104
                    {
                        AuthEndflag = 2;
                    }
                    */
	            break;

               case 0x60://��վԶ����Կ����
                   
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("ԽȨ������Կ1",0,0,0,0);
                       EbErrCodeSend(0x9091,0x61,wChanNo);
                       return 0;
                   }
                        
                   SgcKeymanageStepI(pdata,wEblenth,wChanNo);
                
                   break;
               case 0x62:
                
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("ԽȨ������Կ2",0,0,0,0);
                       EbErrCodeSend(0x9091,0x63,wChanNo);
                       return 0;
                   }
                   SgcKeymanageStepII(pdata,wEblenth,wChanNo,0x62);
                   myTaskDelay(3);
                   break;

               case 0x64://��վԶ����Կ�ָ�
               
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("ԽȨ�ָ���Կ",0,0,0,0);
                       EbErrCodeSend(0x9092,0x65,wChanNo);
                       return 0;
                   }
                   
                   SgcKeymanageStepII(pdata,wEblenth,wChanNo,0x64);
                   myTaskDelay(3);
                   break;

               case 0x70://��վԶ��֤�����
               
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("ԽȨ�·�֤��1",0,0,0,0);
                       EbErrCodeSend(0x9097,0x71,wChanNo);
                       return 0;
                   }
                   SgcCAmanageStepI(pdata,wEblenth,wChanNo);
                   myTaskDelay(2);
                   break;

               case 0x72:
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("ԽȨ�·�֤��2",0,0,0,0);
                       EbErrCodeSend(0x9097,0x73,wChanNo);
                       return 0;
                   }
                   SgcCAmanageStepII(pdata,wEblenth,wChanNo);
                   myTaskDelay(2);
                   break;
                   
              case 0x74: //��վ��ȡ�ն�֤��         
                  if(AuthEndflag == 0)
                  {
                      logSysMsgNoTime("ԽȨ��ȡ֤��",0,0,0,0);
                      EbErrCodeSend(0x9095,0x75,wChanNo);
                      return 0;
                  }
                   SgcCAmanageStepIII(pdata,wEblenth,wChanNo);
                   myTaskDelay(3);
                  break;

               case 0x76:
                   //SgcCAmanageStepII(pdata,wEblenth,wChanNo);
                   myTaskDelay(1);
                   break;
               /*    
               case 0x05://���������������+ʱ��+ǩ������ʽ�·�
                   
                   bwlen = EbMsgWithTandSData(pdata+6,rxbuff,wEblenth,wChanNo);
                   break;  
               */    
	        case 0x00:  
                   //if(AuthEndflag == 0)
                   //{
                      // EbErrCodeSend(0x9107,0x1f,wChanNo);
                       //break;
                   //}
                   
	            bwlen = pdata[7];
                   if(wChanNo < 6)//101�˿�
                   {
                       if(bwlen > fixmlen)//�Ƕ���֡ʹ��������
                       {
                           EbErrCodeSend(0x9106,0x1f,wChanNo);
                           return 0;
                       }
                   }
                   else if(wChanNo > 40)
                   {
                       if(bwlen > 6)//�Ƕ���֡ʹ��������
                       {
                           EbErrCodeSend(0x9106,0x1f,wChanNo);
                           return 0;
                       }
                   }
                   
	            memcpy(rxbuff,pdata+8,bwlen);
	            break; 
                
	        default:
             
	            bwlen = pdata[7];
                   if(wChanNo < 6)//101�˿�
                   {
                       if(bwlen > fixmlen)//�Ƕ���֡ʹ��������
                       {
                           EbErrCodeSend(0x9106,0x1f,wChanNo);
                           return 0;
                       }
                   }
                   else if(wChanNo > 40)
                   {
                       if(bwlen > 6)//�Ƕ���֡ʹ��������
                       {
                           EbErrCodeSend(0x9106,0x1f,wChanNo);
                           return 0;
                       }
                   }
	            break;              
	        
	    }
	}
	return bwlen;
}
/********************************************************************
*�������ƣ�SgcMasterauthenStepI
*���ܣ���վ���ն������֤��һ��
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U SgcMasterauthenStepI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,wholelen,sdatalen;
    INT8U signbuf[100];
    INT8U replybuf[120];
    INT8U i ;
        
    datalen = pdata[8]+(pdata[7]<<8);//Ӧ�����ݳ���
    
    memset(signbuf,0,100);
    memset(replybuf,0,120);
    memcpy(SgcMasterRandbuf,pdata + EBAUDATASTARTSITE,datalen);

    //Sgc1161GetChipSerialNumID(signbuf);
    for(i = 0;i < SGCRANDOMLEN;i++)
    {
        SgcMasterRandbuf[8 + i] = ~SgcMasterRandbuf[i];//����վR1��λȡ��
    }
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[0],SgcMasterRandbuf[1],SgcMasterRandbuf[2],SgcMasterRandbuf[3]);
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[4],SgcMasterRandbuf[5],SgcMasterRandbuf[6],SgcMasterRandbuf[7]);
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[8],SgcMasterRandbuf[9],SgcMasterRandbuf[10],SgcMasterRandbuf[11]);
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[12],SgcMasterRandbuf[13],SgcMasterRandbuf[14],SgcMasterRandbuf[15]);
    Sgc1161GetRanSignData(pdata + EBAUDATASTARTSITE,signbuf);

    sdatalen = signbuf[1]+(signbuf[0]<<8) + EBKEYIDLEN;
    wholelen = sdatalen + 5;//����ͷ�г���
    
    memcpy(signbuf,signbuf + 2,sdatalen);
    signbuf[sdatalen-1] = 0x01;//��β�����Կ����
    
    EbEditmsg(replybuf,signbuf,wholelen, 0,0x51,sdatalen);
	
	//TxdHead=0;
	//TxdTail = wholelen + 6;
    //memcpy(TxdBuf,replybuf,TxdTail);
	
	//SendDataToMISI();
    SendAuthDataToMISI(replybuf,wholelen + 6,wChanNo);	
	
    return 0;

}



INT8U SgcMasterauthenStepII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U Keyno,rc;
    INT16U msglen;
    INT8U replybuf[30];
    INT8U signbuf[2];
    
    msglen = pdata[2]+(pdata[1]<<8);
    Keyno = pdata[msglen + EBHEADLEN - 1];
    //logSysMsgNoTime("��֤��վǩ����Keyno=%d",Keyno,0,0,0);
    rc = Sgc1161VerifyMasterSignData( Keyno, (pdata + EBAUDATASTARTSITE));
    if(rc == 0)
    {    
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
    }
    else
    {
        signbuf[0] = 0x90;
	 signbuf[1] = 0x90;
    }
    
    EbEditmsg(replybuf,signbuf,7, 0,0x53,2);

	//TxdHead=0;
	//TxdTail = 7 + 6;
    //memcpy(TxdBuf,replybuf,TxdTail);
	
	//SendDataToMISI();	
    SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

    return 0;
}

/********************************************************************
*�������ƣ�SgcMasterauthenStepIII
*���ܣ���ȡоƬ���к�
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U SgcMasterauthenStepIII(INT8U TypeID,INT16U wChanNo)
{
    INT8U rbuf[50];
    INT8U replybuf[50];
    INT16U wholelen,sdatalen,msgtype;

    Sgc1161GetChipSerialNumID(rbuf);

    sdatalen = rbuf[1]+(rbuf[0]<<8);
    wholelen = sdatalen + 5;//����ͷ�г���
	
    memcpy(rbuf,rbuf + 2,sdatalen);
    if(TypeID == 0x55)
    {
        msgtype = 0x0000;
    }
    else
    {
        msgtype = 0x0040;
    }
    EbEditmsg(replybuf,rbuf,wholelen,msgtype,TypeID,sdatalen);

	//TxdHead=0;
	//TxdTail = wholelen + 6;
    //memcpy(TxdBuf,replybuf,TxdTail);
	
	//SendDataToMISI();	
    SendAuthDataToMISI(replybuf,wholelen + 6,wChanNo);	

    return 0;
}

/********************************************************************
*�������ƣ�SgcKeymanageStepI
*���ܣ���վ���ն���Կ���µ�һ��,��ȡоƬ��Կ�汾��+�����
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U SgcKeymanageStepI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U KeyVersion = 0;
    INT16U datalen,wholelen,sdatalen,rc;
    INT8U infobuf[9];
    INT8U replybuf[25];
        
    rc = Sgc1161GetChipKeyVersion(NULL,infobuf);   
    
    KeyVersion = infobuf[2];
    memset(infobuf, 0,9);
    
    if(rc != 0)
    {
        EbErrCodeSend(0x9091,0x61,wChanNo);
        return 0;   
    }
    
    rc = Sgc1161ObtainRandata();
    if(rc != 0)
    {
        EbErrCodeSend(0x9091,0x61,wChanNo);
        return 0;   
    }

    memset(infobuf,0,9);
    memset(replybuf,0,25);
    
    infobuf[0] = KeyVersion;//��Կ�汾��
    memcpy(infobuf+1,SgcSelfRandbuf,8); //�ն������   

    EbEditmsg(replybuf,infobuf,9 + 5, 0,0x61,9);
	
    SendAuthDataToMISI(replybuf,14 + 6,wChanNo)	;

    return 0;   
}

/********************************************************************
*�������ƣ�SgcKeymanageStepII
*���ܣ���վ���ն���Կ����/�ָ��ڶ���,ִ����Կ����/�ָ�
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U SgcKeymanageStepII(INT8U *pdata,INT16U len,INT16U wChanNo,INT8U typeid)
{
    INT8U Keyno,rc;
    INT8U replybuf[30];
    INT8U infobuf[2];
    
    Keyno = pdata[len + EBHEADLEN - 1];
    
    rc = Sgc1161LoadSymmetryKey(pdata + EBAUDATASTARTSITE,len - 6 ,Keyno);
    if(rc == 0)
    {    
        infobuf[0] = 0x90;
        infobuf[1] = 0x00;
    }
    else
    {
        infobuf[0] = 0x90;
        if(typeid == 0x62)
        {
            infobuf[1] = 0x91;
        }
        else
        {
            infobuf[1] = 0x92;   
        }
        logSysMsgNoTime("��Կ����ʧ��.rc=%d",rc,0,0,0);
	 
    }
    
    EbEditmsg(replybuf,infobuf,7, 0,(typeid+1),2);

    SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

    return 0;   
}

/********************************************************************
*�������ƣ�SgcCAmanageStepI
*���ܣ�֤������һ��
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U SgcCAmanageStepI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U CArID = 0,KeyID = 0;
    INT8U *p;
    INT8U *signbuf;
    INT16U datalen,CAlen,sdatalen,rc,i;
    //INT8U signbuf[1024];
    INT8U replybuf[30];
    //INT8U CAbuf[];
    INT8U timebuf[6];

    //CAVersion = pdata[EBAUDATASTARTSITE];
    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA�������ݰ�����
    signbuf = ParaTempBuf + 2048;
    memset(signbuf,0,1024);
	
    if(pdata[EBAUDATASTARTSITE + 2] == (CAnum + 1))
    {
        memcpy(ParaTempBuf+enCAlen,(pdata+ EBAUDATASTARTSITE + 3),datalen);
        enCAlen += datalen;
        CAnum++ ;
    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x96;
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
        enCAlen = 0;
        CAnum = 0;
        return 0;
    }
    
    if(CAnum != pdata[EBAUDATASTARTSITE + 1])//��δ�������
    {
        return 0;
    }
    
    saveRecord(ParaTempBuf,enCAlen,RXSAVEMODE,0);
    
    rc = Sgc1161DecryptData(ParaTempBuf,enCAlen,signbuf);//���ܻ����������
    
    if(rc != 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x97;
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

        logSysMsgNoTime("֤�����ʧ��",0,0,0,0);
        enCAlen = 0;
        CAnum = 0;
        return 0;
    }
    
    sdatalen = signbuf[1]+(signbuf[0]<<8);//���ı��ĳ���
  
    p = signbuf + 2;//sgcbuf��ǰ�����ֽ��ǰ�ȫоƬ���ص����ݳ���
    saveRecord(signbuf,sdatalen+2,RXSAVEMODE,0);
    
    enCAlen = 0;
    CAnum = 0;
    
    CArID = p[0];
    CAlen = sdatalen -1 -6 - 65;//��Կ��ʶ��ʱ����Ϣ��ǩ�����+ǩ����Կ��ʶ
    KeyID = p[sdatalen - 1];
    

    memcpy(timebuf,p+CAlen+1,6);
    
    if(CheckTimeAging(timebuf) != 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x97;
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

        //logSysMsgNoTime("70����ʱ���У�����:",0,0,0,0);
        return 0;
    }
    
    rc = Sgc1161VerifySigndata(p,sdatalen - 1,KeyID);

    if(rc == 0)
    {
        rc = Sgc1161LoadCerdata(p + 1, CAlen,CArID);
    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x97;//
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	; 
        logSysMsgNoTime("��֤��վ�·�֤��ǩ��ʧ��",0,0,0,0);
        return 0;   
    }
    
    if(rc == 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	; 

    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x97;//
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	; 
    }
    return 0;   
}

/********************************************************************
*�������ƣ�CheckCerDecryptData
*���ܣ�֤�����Ľ��ܱ��ļ��
*���룺pdata��
*�з��ˣ�����
*********************************************************************/
INT8U CheckCerDecryptData(INT8U *pdata,INT16U len)
{
    INT16U i;
    
    for(i=0;i<len;i++)
    {
        if((pdata[i] == 0x55)&&(pdata[i + 1] == 0x90)&&(pdata[i + 2] == 0x00))
        {
            //if(i < (sdatalen - 2))
            //{
                // if(memcmp(signbuf,signbuf+3,(sdatalen - i -3)) != 0)
            //}
            return 1;
        }
    }
    return 0;
}

INT8U SgcCAmanageStepII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U CArID= 0;
    INT8U *p;
    INT16U datalen,CAlen,sdatalen,rc;
    INT8U signbuf[10];
    INT8U replybuf[20];
    //INT8U CAbuf[];
    

    //CAVersion = pdata[EBAUDATASTARTSITE];
    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA���ݰ��ְ�����
    
    if(pdata[EBAUDATASTARTSITE + 2] == (CAnum + 1))
    {
        memcpy(ParaTempBuf+enCAlen,(pdata+EBAUDATASTARTSITE + 3),datalen);
        enCAlen += datalen;
        CAnum++ ;
    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x96;
        EbEditmsg(replybuf,signbuf,7, 0,0x73,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;   
        enCAlen = 0;
        CAnum = 0;
        return 0;
    }
    if(CAnum != pdata[EBAUDATASTARTSITE + 1])//��δ�������
    {
        return 0;
    }

    rc =  Sgc1161LoadTemSelfCerdata(ParaTempBuf,enCAlen );
    enCAlen = 0;
    CAnum = 0;
    
    if(rc == 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
        EbEditmsg(replybuf,signbuf,7, 0,0x73,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	; 

    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x97;
        EbEditmsg(replybuf,signbuf,7, 0,0x73,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	; 
    }
    return 0;   
}

INT8U SgcCAmanageStepIII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U i,rc;
    INT8U msgnum = 0;
    INT8U CAbuf[210];
    INT8U sendbuf[255];
    INT16U datalen,CAlen,sdatalen;
    
    rc = Sgc1161CheckoutCer(ParaTempBuf);
    
    if(rc != 0)
    {
         EbErrCodeSend(0x9095,0x75,wChanNo);
         return 0;
    }
    
    CAlen = ParaTempBuf[1]+(ParaTempBuf[0]<<8);
    
    if(CAlen%200 == 0)
    {
        msgnum = CAlen/200;
    }
    else
    {
        msgnum = (CAlen/200) + 1;
    }
    
    CAbuf[0] = 0x06;//�ն�֤���ʶ(����Ժ��ͨ��֪��ʶ�̶�Ϊ1)
    CAbuf[1] = msgnum;//֤�鱨����֡��
    
    for(i = 1;i <= msgnum; i++)
    {
    
        CAbuf[2] = i;//��ǰ֡���
        
        if(i < msgnum)
        {
            memcpy(CAbuf+3,ParaTempBuf+2+((i - 1) * 200),200);//ÿ֡����200�ֽ�ca����
            EbEditmsg(sendbuf,CAbuf, 208,0,0x75,203);
            SendAuthDataToMISI(sendbuf,208+ 6,wChanNo); 
        }
        else
        {
            memcpy(CAbuf+3,ParaTempBuf+2+((i - 1) * 200),(CAlen%200));//���һ֡ca����
            EbEditmsg(sendbuf,CAbuf, (CAlen%200)+3+5 ,0,0x75,(CAlen%200)+3);
            SendAuthDataToMISI(sendbuf,(CAlen%200)+3+5+ 6,wChanNo); 
        }
        
        
        myTaskDelay(2);
    }
	return 0;
}

/********************************************************************
*�������ƣ�EnMsgByYWTool
*���ܣ������ֳ���ά�����·�������
*���룺pdata��EB���Ļ�������rxbuff:���ݴ�Ż�����wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U EnMsgByYWTool(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    INT16U wEblenth,datalen,rc;
    INT8U bwlen = 0;
    INT8U rpbuf[25];
	
    wEblenth = pdata[2]+(pdata[1]<<8);//���ĳ���(����ͷ��У����)

    if((pdata[5]&0x08) != 0)//�Ƿ����
    {
        rc = Sgc1161DecryptYWFileData(pdata + 6,wEblenth- 2,pdata + 6);
        datalen = pdata[7]+(pdata[6]<<8);//���ܺ��������ݳ���
        memcpy(pdata+6,pdata + 8,datalen);
    }

    if(YWAuthEndflag != 1)
    {
        if(pdata[6] > 0x32)
        {
            if(pdata[6] == 0x46)
            {
                rpbuf[9] = 0x90;
                rpbuf[10] = 0x92;
                EbEditmsg(rpbuf,rpbuf+9,7, 0x0040,0x47,2);
                SendAuthDataToMISI(rpbuf,7 + 6,wChanNo)  ;
            }
            //else if(pdata[6] == 0x34)
            else if((pdata[6] == 0x3E)||(pdata[6] == 0x42)||(pdata[6] == 0x44)||(pdata[6] == 0x45))
            {
                //?
            }
            else
            {
                rpbuf[9] = 0x91;
                rpbuf[10] = 0x07;
                EbEditmsg(rpbuf,rpbuf+9,7, 0x0040,pdata[6]+1 ,2);
                SendAuthDataToMISI(rpbuf,7 + 6,wChanNo)  ;
                return 0;
            }
        }
    }
    
    switch(pdata[6])
    {
	    case 0x30://֤���������֤������
	        YWAuthEndflag = 0;
               SgcYWToolAuthReq(pdata,wEblenth,wChanNo);
               myTaskDelay(2);
	        break;
	    case 0x32://
		     SgcYWToolAuthSdata(pdata,wEblenth,wChanNo);
               myTaskDelay(2);
	        break;   
           case 0x34://
               //if(AuthEndflag == 0)
               //{
                  // EbErrCodeSend(0x9091,0x61,wChanNo);
                   //return 0;
               //}
               SgcKeyVerforYWTool(wChanNo);
               break;
           case 0x36://
               HostSerialNumforYWTool(wChanNo);
                   break;
           case 0x38://
               SgcMasterauthenStepIII(0x39,wChanNo);
               break;
           case 0x3A://
               SgcGetPbKeyforYWTool(wChanNo);
               break;
           case 0x3C://
               SgcSignYWtoolReqfile(pdata, wEblenth, wChanNo);
                   break;
           case 0x3E://
               SgcYWToolCAmanage(pdata, wEblenth, wChanNo);
                   break;
           case 0x40://
               SgcYWToolReWritrOriCA(wChanNo);
                   break;
           case 0x42://
               SgcYWToolGetPbKeyCer(wChanNo);
                   break;
           case 0x44://
                   //֤������߷���֤�鷵�ؽ��
                   break;
           case 0x45://
                SgcYWWaittoSend(wChanNo);
                   break;
           case 0x46://�ָ���Կ
           
               SgcYWToolHFDCKey(pdata,wEblenth, wChanNo);
                               
                   break;
           case 0x48://
                                       
                   break;

	    default:
	            break;              
	        
    }
    return bwlen;
}

/********************************************************************
*�������ƣ�SgcYWToolAuthReq
*���ܣ�֤�鹤�����ն������֤��һ��
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U SgcYWToolAuthReq(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    
    INT8U i;
    INT16U datalen,rc;
    INT8U signbuf[10];
    INT8U replybuf[100];
    INT8U *p;

    p = ParaTempBuf+1024;//�����101/104��Լ�·�֤��ʱ��ͻ
    datalen = pdata[8]+(pdata[7]<<8) - 11;//Cer���ݰ�����
    
    if(pdata[EBAUDATASTARTSITE + 2] == (YWCernum + 1))
    {
        memcpy(p+YWCerlen,(pdata+ EBAUDATASTARTSITE + 3),datalen);
        YWCerlen += datalen;
        YWCernum++ ;
        
        if(YWCernum == 1)//֤�������IDֻ��һ�μ���
        {
            memcpy(SgcCerToolIDbuf,(pdata+ EBAUDATASTARTSITE + 3+ datalen),8);
        }
    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x96;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x45,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
        YWCerlen = 0;
        YWCernum = 0;
        return 0;
    }
    
    if(YWCernum != pdata[EBAUDATASTARTSITE + 1])//��δ�������
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x45,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
        return 0;
    }
    /*
    rc = Sgc1161GetChipSerialNumID(signbuf);
    if(rc != 0)
    {
        rc = Sgc1161GetChipSerialNumID(signbuf);
    }
    */
    rc = Sgc1161VfyYWCer(p,YWCerlen);
    YWCerlen = 0;
    YWCernum = 0;
    
    if(rc != 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x90;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x31,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
        return 0;
    }
    
    rc = Sgc1161ObtainRandata();
    
    if(rc != 0)
    {
        //signbuf[0] = 0x91;
        //signbuf[1] = 0x09;
        //EbEditmsg(replybuf,signbuf,7, 0x0040,0x31,2);
        //SendAuthDataToMISI(replybuf,7 + 6,wChanNo);
        return 0;
    }
    
    memcpy(SgcRdDatatoYWbuf,SgcSelfRandbuf,8);
    for(i = 0;i < SGCRANDOMLEN;i++)
    {
        SgcRdDatatoYWbuf[8 + i] = ~SgcRdDatatoYWbuf[i];//����վR1��λȡ��
    }
    
    EbEditmsg(replybuf,SgcRdDatatoYWbuf,SGCRANDOMLEN + 5, 0x0040,0x31,SGCRANDOMLEN);
    SendAuthDataToMISI(replybuf,SGCRANDOMLEN + 5 + 6,wChanNo);	
    return 0;
}

/********************************************************************
*�������ƣ�SgcYWToolAuthSdata
*���ܣ���ά������֤ ǩ��
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U SgcYWToolAuthSdata(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,rc;
    INT8U signbuf[10];
    INT8U replybuf[20];
    
    datalen = pdata[8]+(pdata[7]<<8) - 11;//֤�鹤��ǩ�����ݰ�����
    
    rc = Sgc1161VerifyMaintDevSigndata(pdata + EBAUDATASTARTSITE,datalen );

    if(rc != 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x90;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x33,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x33,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

        YWAuthEndflag = 1;
        if(AuthEndflag == wChanNo)
        {
            AuthEndflag = 0;
        }
    }
    return 0;
}

/********************************************************************
*�������ƣ�SgcKeyVerforYWTool
*���ܣ�֤���������ȡ�ն�ID
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
/*
INT8U GetTerminalIdforYWTool(INT16U wChanNo)
{

    INT8U lenth;
    INT8U IDbuf[24];
    INT8U replybuf[40];
    
        
    GetTerminalId(IDbuf,&lenth);
    
    
    memset(IDbuf,0,24);
    
    memcpy(IDbuf,SgcSelfRandbuf,8); //�ն������   

    EbEditmsg(replybuf,IDbuf,lenth + 5, 0x0040,0x37,lenth);
	
    SendAuthDataToMISI(replybuf,lenth + 5 + 6,wChanNo)	;

    return 0;   

}
*/
/********************************************************************
*�������ƣ�SgcKeyVerforYWTool
*���ܣ�֤���������ȡ��Կ�汾��
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U SgcKeyVerforYWTool(INT16U wChanNo)
{

    INT8U KeyVersion = 0;
    INT16U datalen,wholelen,sdatalen,rc;
    INT8U infobuf[9];
    INT8U replybuf[25];
        
    rc = Sgc1161GetChipKeyVersion(NULL,infobuf);  
    
    KeyVersion = infobuf[2];
    memset(infobuf, 0,9);
    if(rc != 0)
    {
        //infobuf[0] = 0x91;
        //infobuf[1] = 0x09;
        //EbEditmsg(replybuf,infobuf,7, 0x0040,0x35,2);
        //SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
        return 0;   
    }
    
    rc = Sgc1161ObtainRandata();
    
    if(rc != 0)
    {
        //infobuf[0] = 0x91;
        //infobuf[1] = 0x09;
        //EbEditmsg(replybuf,infobuf,7, 0x0040,0x35,2);
        //SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
        return 0;   
    }
    
    memset(infobuf,0,9);
    memset(replybuf,0,25);
    
    infobuf[0] = KeyVersion;//��Կ�汾��
    memcpy(infobuf+1,SgcSelfRandbuf,8); //�ն������   

    EbEditmsg(replybuf,infobuf,9 + 5, 0x0040,0x35,9);
	
    SendAuthDataToMISI(replybuf,14 + 6,wChanNo)	;

    return 0;   

}

/********************************************************************
*�������ƣ�SgcSerialNumforYWTool
*���ܣ�֤���������ȡ�ն����к�
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT8U HostSerialNumforYWTool(INT16U wChanNo)
{
    char rbuf[24];//�ն����к�
    INT8U replybuf[40];
    INT8U lenth;
    INT16U wholelen,sdatalen;

    //Sgc1161GetChipSerialNumID(rbuf);
    //rbuf[25] = 77;
    GetTerminalId(rbuf,&lenth);
    sdatalen = lenth - 1;
    wholelen = sdatalen + 5;//����ͷ�г���
	
    //memcpy(rbuf,rbuf + 2,sdatalen);
    EbEditmsg(replybuf,(INT8U *)rbuf,wholelen, 0x0040,0x37,sdatalen);
	
    SendAuthDataToMISI(replybuf,wholelen + 6,wChanNo);	

    return 0;
}
/********************************************************************
*�������ƣ�SgcGetPbKeyforYWTool
*���ܣ�֤���������ȡ��Կ
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U SgcGetPbKeyforYWTool(INT16U wChanNo)
{
    INT8U rc;
    INT8U sum = 0;
    INT8U PbKeybuf[160];
    //INT8U sendbuf[255];
    INT16U endatalen;
        
    rc =  Sgc1161CheckoutPKey((PbKeybuf + 1));
    
    if(rc != 0)
    {
        //PbKeybuf[9] = 0x91;
        //PbKeybuf[10] = 0x09;
        //EbEditmsg(PbKeybuf,PbKeybuf+9,7, 0x0040,0x3B,2);
        //SendAuthDataToMISI(PbKeybuf,7 + 6,wChanNo)	;
         return 0;
    }

    //memcpy(PbKeybuf+3,PbKeybuf+5,0x40);
    
    PbKeybuf[0] = 0x3B;
    //PbKeybuf[1] = 0x00;
   // PbKeybuf[2] = 0x40;
    
    rc = Sgc1161EncryptPbKey(PbKeybuf,0x43);
    
    endatalen = PbKeybuf[1]+(PbKeybuf[0]<<8);
    memcpy(PbKeybuf + 6, PbKeybuf + 2, endatalen);//�����ܺ�����ݷ���Ӧ������֮��
    endatalen += 2;//���ϱ����������ֽ���ΪEb���ĵĳ����ֽ�
    
    PbKeybuf[0] = PbKeybuf[3] = 0xEB;
    PbKeybuf[1] = HIBYTE(endatalen);
    PbKeybuf[2] = LOBYTE(endatalen);
    PbKeybuf[4] = 0x00;
    PbKeybuf[5] = 0x48;
    //PbKeybuf[6] = 0x3B;
    //PbKeybuf[7] = 0x00;
    //PbKeybuf[8] = 0x40;
    
    sum = GetEbMsgCheckSum(PbKeybuf);
    PbKeybuf[endatalen +  4] = sum;
    PbKeybuf[endatalen +  5] = 0xD7;
    
    SendAuthDataToMISI(PbKeybuf,endatalen + 6,wChanNo);	
    return 0;
}
/********************************************************************
*�������ƣ�SgcSignYWtoolReqfile
*���ܣ���֤������ߵ�֤�������ļ�ǩ��
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT8U SgcSignYWtoolReqfile(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    
    INT8U i;
    INT16U datalen,rc;
    INT8U signbuf[128];
    //INT8U replybuf[1024];
    INT8U *p;

    p = ParaTempBuf+1024;//�����101/104��Լ�·�֤��ʱ��ͻ
    
    datalen = pdata[8]+(pdata[7]<<8) - 2;//֤���������ݳ���
    
    if(pdata[EBAUDATASTARTSITE + 1] == (YWCernum + 1))
    {
        memcpy(p+YWCerlen,(pdata+ EBAUDATASTARTSITE + 2),datalen);
        YWCerlen += datalen;
        YWCernum++ ;
    }
    else
    {
        signbuf[9] = 0x90;
        signbuf[10] = 0x96;
        EbEditmsg(signbuf,signbuf+9,7, 0x0040,0x45,2);
        SendAuthDataToMISI(signbuf,7 + 6,wChanNo);
        YWCerlen = 0;
        YWCernum = 0;
        return 0;
    }
    
    if(YWCernum != pdata[EBAUDATASTARTSITE])//��δ�������
    {
        signbuf[9] = 0x90;
        signbuf[10] = 0x00;
        EbEditmsg(signbuf,signbuf+9,7, 0x0040,0x45,2);
        SendAuthDataToMISI(signbuf,7 + 6,wChanNo);

        return 0;
    }
    
    //rc = Sgc1161VfyYWCer(p,YWCerlen);
    rc = Sgc1161SignYWData(p,YWCerlen);
    YWCerlen = 0;
    YWCernum = 0;
    
    if(rc != 0)
    {
        //signbuf[9] = 0x91;
        //signbuf[10] = 0x09;
        //EbEditmsg(signbuf,signbuf+9,7, 0x0040,0x3D,2);
        //SendAuthDataToMISI(signbuf,7 + 6,wChanNo)	;
        return 0;
    } 

    datalen = p[1]+(p[0]<<8);//֤���������ݳ���
    //memcpy(signbuf +9 ,p+2,datalen);
    
    EbEditmsg(signbuf,p+2,datalen + 5, 0x0040,0x3D,datalen);
    SendAuthDataToMISI(signbuf,datalen + 5 + 6,wChanNo);	
    return 0;
}

/********************************************************************
*�������ƣ�SgcYWToolCAmanage
*���ܣ���֤������ߵ���֤�����ݴ���
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT8U SgcYWToolCAmanage(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U CArID= 0,i = 0;
    INT16U datalen,CAlen,rc;
    INT8U signbuf[10];
    INT8U replybuf[120];
    //INT8U CAbuf[];
    INT8U *p;

    p = ParaTempBuf+1024;//�����101/104��Լ�·�֤��ʱ��ͻ

    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA���ݰ��ְ�����
    
    if(pdata[EBAUDATASTARTSITE + 2] == (YWCernum + 1))
    {
        memcpy(p+YWCerlen,(pdata+EBAUDATASTARTSITE + 3),datalen);
        YWCerlen += datalen;
        YWCernum++ ;
    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x96;
        EbEditmsg(replybuf,signbuf,7,0x0040,0x45,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)  ;   
        YWCerlen = 0;
        YWCernum = 0;
        return 0;
    }
    if(YWCernum != pdata[EBAUDATASTARTSITE + 1])//��δ�������
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x45,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)  ;
        return 0;
    }
    
    CArID = pdata[EBAUDATASTARTSITE];
    
    if(CArID != 6)
    {
        rc = Sgc1161LoadCerdata( p, YWCerlen,CArID);
    }
    else
    {
        rc =  Sgc1161LoadTemSelfCerdata( p,YWCerlen );
    }
    while(i < 3)
    {
        i++;
        if(rc  != 0)
        {
            myTaskDelay(2);    
            if(CArID != 6)
            {
                CArID = pdata[EBAUDATASTARTSITE];
                rc = Sgc1161LoadCerdata( p, YWCerlen,CArID);
            }
            else
            {
                rc =  Sgc1161LoadTemSelfCerdata( p,YWCerlen );
            }
        }
        else
        {
            break;
        }
    }
    YWCerlen = 0;
    YWCernum = 0;
    
    if(rc == 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
        EbEditmsg(replybuf,signbuf,7,0x0040,0x3F,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)  ; 

    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x93;
        EbEditmsg(replybuf,signbuf,7,0x0040,0x3F,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)  ; 
    }
    return 0;   
}


/*
INT8U SgcYWToolCAmanage(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U CArID= 0,i = 0;
    INT16U datalen,CAlen,rc;
    INT8U cerbuf[1024];
    INT8U replybuf[120];
    //INT8U CAbuf[];
    INT8U *p;

    p = cerbuf;//�����101/104��Լ�·�֤��ʱ��ͻ

    //CAVersion = pdata[EBAUDATASTARTSITE];
    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA���ݰ��ְ�����
    
    if(pdata[EBAUDATASTARTSITE + 2] == (YWCernum + 1))
    {
        memcpy(cerbuf+YWCerlen,(pdata+EBAUDATASTARTSITE + 3),datalen);
        YWCerlen += datalen;
        YWCernum++ ;
    }
    else
    {
        replybuf[9] = 0x90;
        replybuf[10] = 0x96;
        EbEditmsg(replybuf,replybuf+9,7, 0x0040,0x45,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;        
        YWCerlen = 0;
        YWCernum = 0;
        return 0;
    }
    if(YWCernum != pdata[EBAUDATASTARTSITE + 1])//��δ�������
    {
        replybuf[9] = 0x90;
        replybuf[10] = 0x00;
        EbEditmsg(replybuf,replybuf+9,7, 0x0040,0x45,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
        return 0;
    }

    if(CArID != 6)
    {
        CArID = pdata[EBAUDATASTARTSITE];
        rc = Sgc1161LoadCerdata( cerbuf, YWCerlen,CArID);
    }
    else
    {
        rc =  Sgc1161LoadTemSelfCerdata( cerbuf,YWCerlen );
    }
    while(i < 3)
    {
        i++;
        if(rc  != 0)
        {
            myTaskDelay(2);    
            if(CArID != 6)
            {
                CArID = pdata[EBAUDATASTARTSITE];
                rc = Sgc1161LoadCerdata( cerbuf, YWCerlen,CArID);
            }
            else
            {
                rc =  Sgc1161LoadTemSelfCerdata( cerbuf,YWCerlen );
            }
        }
        else
        {
            break;
        }
    }
    YWCerlen = 0;
    YWCernum = 0;
    
    if(rc == 0)
    {
        replybuf[9] = 0x90;
        replybuf[10] = 0x00;
        EbEditmsg(replybuf,replybuf+9,7, 0x0040,0x3F,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
    }
    else
    {
        replybuf[9] = 0x90;
        replybuf[10] = 0x93;
        EbEditmsg(replybuf,replybuf+9,7, 0x0040,0x3F,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

    }
    return 0;   
}
*/
/********************************************************************
*�������ƣ�SgcYWToolReWritrOriCA
*���ܣ���ʼ֤���д
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT8U SgcYWToolReWritrOriCA(INT16U wChanNo)
{
    INT16U OriCalen,rc;
    INT8U oricabuf[1024];
    
    rc =  Sgc1161GetOriCerlen(oricabuf);
    if(rc != 0)
    {
        oricabuf[9] = 0x90;
        oricabuf[10] = 0x93;
        EbEditmsg(oricabuf,oricabuf+9,7, 0x0040,0x41,2);
        SendAuthDataToMISI(oricabuf,7 + 6,wChanNo)	;
        return 0;
    } 
   
    OriCalen = oricabuf[3]+(oricabuf[2]<<8);//��ʼ֤�鳤��
    
    rc = Sgc1161GetOriCerData(oricabuf,OriCalen);
    //rc = Sgc1161CheckoutCer(oricabuf);
    if(rc != 0)
    {
        oricabuf[9] = 0x90;
        oricabuf[10] = 0x93;
        EbEditmsg(oricabuf,oricabuf+9,7, 0x0040,0x41,2);
        SendAuthDataToMISI(oricabuf,7 + 6,wChanNo)	;
        return 0;
    } 
    
    rc =  Sgc1161LoadTemSelfCerdata( oricabuf + 2,OriCalen);
    
    if(rc != 0)
    {
        oricabuf[9] = 0x90;
        oricabuf[10] = 0x93;
        EbEditmsg(oricabuf,oricabuf+9,7, 0x0040,0x41,2);
        SendAuthDataToMISI(oricabuf,7 + 6,wChanNo);
        return 0;
    }
    else
    {
        oricabuf[9] = 0x90;
        oricabuf[10] = 0x00;
        EbEditmsg(oricabuf,oricabuf+9,7, 0x0040,0x41,2);
        SendAuthDataToMISI(oricabuf,7 + 6,wChanNo)	;
        return 0;
    }
    
}

/********************************************************************
*�������ƣ�SgcYWToolGetPbKeyCer
*���ܣ�������Կ֤��(�ն�֤��)��֤�������
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT8U SgcYWToolGetPbKeyCer(INT16U wChanNo)
{
     
     INT8U i,rc;
     INT8U *p;
     INT8U CAbuf[255];
     //INT8U sendbuf[255];
     INT16U datalen,sdatalen;

     p = ParaTempBuf+1024;
     rc = Sgc1161CheckoutCer(p);
     
     if(rc != 0)
     {
         CAbuf[9] = 0x90;
         CAbuf[10] = 0x94;
         EbEditmsg(CAbuf,CAbuf+9,7, 0x0040,0x43,2);
         SendAuthDataToMISI(CAbuf,7 + 6,wChanNo)  ;
         return 0;

     }
     
     Sendlen = p[1]+(p[0]<<8);
     
     if(Sendlen%200 == 0)
     {
         Sendnum = Sendlen/200;
     }
     else
     {
         Sendnum = (Sendlen/200) + 1;
     }
     
     CAbuf[0] = 0x06;//
     CAbuf[1] = Sendnum;//֤�鱨����֡��
     SendCount = 1 ;
     CAbuf[2] = SendCount;//��ǰ֡���
     
     if(Sendnum > 1)
     {
         memcpy(CAbuf+3,p+2,200);//ÿ֡����200�ֽ�ca����
         memcpy(CAbuf +9,CAbuf,203);
         
         EbEditmsg(CAbuf,CAbuf+9, 208,0x0040,0x43,203);
         SendAuthDataToMISI(CAbuf,208+ 6,wChanNo); 
     }
     else
     {
         memcpy(CAbuf+3,p+2,Sendlen);//���һ֡ca����
         memcpy(CAbuf +9,CAbuf,Sendlen+3);
         
         EbEditmsg(CAbuf,CAbuf+9, Sendlen+3+5 ,0x0040,0x43,Sendlen+3);
         SendAuthDataToMISI(CAbuf,Sendlen+3+5+ 6,wChanNo); 
     }
     
/*     
     for(i = 1;i <= msgnum; i++)
     {
     
         CAbuf[2] = i;//��ǰ֡���
         
         if(i < msgnum)
         {
             memcpy(CAbuf+3,p+2+((i - 1) * 200),200);//ÿ֡����200�ֽ�ca����
             EbEditmsg(sendbuf,CAbuf, 208,0x0040,0x43,203);
             SendAuthDataToMISI(sendbuf,208+ 6,wChanNo); 
         }
         else
         {
             memcpy(CAbuf+3,p+2+((i - 1) * 200),(CAlen%200));//���һ֡ca����
             EbEditmsg(sendbuf,CAbuf, (CAlen%200)+3+5 ,0x0040,0x43,(CAlen%200)+3);
             SendAuthDataToMISI(sendbuf,(CAlen%200)+3+5+ 6,wChanNo); 
         }
         
         
         myTaskDelay(2);
     }
     */
     return 0;
}
/********************************************************************
*�������ƣ�SgcYWWaittoSend
*���ܣ�������֡������
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT8U SgcYWWaittoSend(INT16U wChanNo)
{
    INT8U *p;
    INT8U CAbuf[255];

    if(Sendlen == 0)
    {
        return 0;
    }
    p = ParaTempBuf+1024;

    CAbuf[0] = 0x06;//
    CAbuf[1] = Sendnum;//֤�鱨����֡��

    SendCount++;
    CAbuf[2] = SendCount;//��ǰ֡���
    
    if(Sendnum > SendCount)
    {
        memcpy(CAbuf+3,p+2+((SendCount - 1) * 200),200);//ÿ֡����200�ֽ�ca����
        memcpy(CAbuf +9,CAbuf,203);
        
        EbEditmsg(CAbuf,CAbuf+9, 208,0x0040,0x43,203);
        SendAuthDataToMISI(CAbuf,208+ 6,wChanNo); 
    }
    else
    {
        memcpy(CAbuf+3,p+2+((SendCount - 1) * 200),(Sendlen%200));//���һ֡ca����
        memcpy(CAbuf +9,CAbuf,((Sendlen%200) +3));
        
        EbEditmsg(CAbuf,CAbuf+9, (Sendlen%200)+3+5 ,0x0040,0x43,(Sendlen%200)+3);
        SendAuthDataToMISI(CAbuf,(Sendlen%200)+3+5+ 6,wChanNo); 
        Sendnum = 0;
        SendCount = 0;
        Sendlen = 0;
    }
    return 0;
}

/********************************************************************
*�������ƣ�SgcYWToolHFDCKey
*���ܣ���֤������߻ָ��ն˶Գ���Կ
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT8U SgcYWToolHFDCKey(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,rc;
    INT8U signbuf[10];
    INT8U replybuf[20];

    rc = Sgc1161RecoveryKeydata(pdata + EBAUDATASTARTSITE, 185);//��Կ�ָ����ֽڹ̶�185��

    if(rc != 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x92;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x47,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

    }
    else
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
        EbEditmsg(replybuf,signbuf,7, 0x0040,0x47,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;
    }
    return 0;
}

/********************************************************************
*�������ƣ�GetYWTooldataFromWHBuf
*���ܣ���ά���ڻ�����ժȡ֤��������·�������
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
void GetYWTooldataFromWHBuf(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U eblenth,i;
    INT16U checklen = 0;
    INT16U totollen = 0; 

    i = 0;
    while (i < len)
    {
        if(pdata[i] == 0xEb)
        {
            if(i +4 > len )
            {
                return  ;
            }
        }
        
        if(pdata[i] == 0xEb&&pdata[i + 3] == 0xEb)
        {

            checklen = pdata[i+2]+(pdata[i+1]<<8);
            
            if(checklen+6 > (len- i))
            {
                return  ;
            }
         
            if(CheckEbMsgSty((pdata+ i)))
            {
	         eblenth = pdata[i+2]+(pdata[i+1]<<8);
             
	         EbmsgAnalysis(pdata+i,NULL,wChanNo);

                memcpy(pdata+i,(pdata+eblenth + 6 +i),(eblenth + 6));
                i += (eblenth + 6);
                totollen += (eblenth + 6);//���߹������ۼƳ���
            }
            else
            {
                i++;
            }
        }
        else
        {
            i++;
        }    
        
    } 
    
}


#if 0
/********************************************************************
*�������ƣ�SgcKeymanageStepI
*���ܣ���վ�ָ��ն���Կ
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

SgcKeymanageStepIII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U Keyno,rc;
    INT8U replybuf[30];
    INT8U signbuf[2];
    
    Keyno = pdata[len + EBHEADLEN - 1];
    
    rc = Sgc1161LoadSymmetryKey(pdata + EBAUDATASTARTSITE,len - 1,Keyno);
    if(rc == 0)
    {    
        signbuf[0] = 0x90;
        signbuf[1] = 0x00;
    }
    else
    {
        signbuf[0] = 0x90;
	 signbuf[1] = 0x92;
    }
    
    EbEditmsg(replybuf,signbuf,7, 0,0x65,2);

    SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

    return 0;   
}
#endif

/********************************************************************
*�������ƣ�SendAuthDataToMISI
*���ܣ�������֤����
*���룺sendbuf���������ݣ�len��EB���ĵĳ��ȣ�����������У���룩
*      wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
BOOL SendAuthDataToMISI(INT8U *sendbuf,INT16U len,INT16U wChanNo)
{
    INT16U SendLen;
    SendLen=(INT16U)MisiWrite(wChanNo,sendbuf,len,3);
    
    //myTaskDelay(5);
    if(SendLen==0xffff)//д����
    {
        //SendLen=(INT16U)MisiWrite(wChanNo,sendbuf,len,3);
        myTaskDelay(10);
    }
    
    if(SendLen==0xffff)//д����
    {
        return FALSE;
    }
    else
    {
        return TRUE;        
    }

}



/********************************************************************
*�������ƣ�EbEditmsg
*���ܣ��༭Eb��ȫ����
*���룺pdata�����Ļ�������sdatabuf���ݻ�������slen���ĳ���
*�з��ˣ�����
*********************************************************************/
void EbEditmsg(INT8U *pdata,INT8U* sdatabuf,INT16U slen, INT16U ebtype,INT8U typeId,INT16U msglen)
{
    INT8U sum = 0;
    
    pdata[0] = pdata[3] = 0xEB;
    pdata[1] = HIBYTE(slen);
    pdata[2] = LOBYTE(slen);
    pdata[4] = HIBYTE(ebtype);
    pdata[5] = LOBYTE(ebtype);
    pdata[6] = typeId;
    pdata[7] = HIBYTE(msglen);
    pdata[8] = LOBYTE(msglen); 
       
    memcpy(pdata + 9, sdatabuf, msglen);
    sum = GetEbMsgCheckSum(pdata);
    //pdata[slen +  4] = 0;
    pdata[slen +  4] = sum;
    pdata[slen +  5] = 0xD7;
}
/********************************************************************
*�������ƣ�Pack104msgtoEb
*���ܣ���104����ת��ΪEB��ȫ����
*���룺buf�����ģ�SEBtaillen:��ȫ���ݻ�������len���ĳ���
*�з��ˣ�����
*********************************************************************/

INT16U Pack104msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo)
{
    INT16U i,templen,rc;
    INT8U ebbuf[355];
    INT8U len104 = 0;
    INT8U tmp = 0;
    
    i = 0;
    rc = 0;
    memset(ebbuf,0,355);
    
	while(i < len)
	{
	    if((buf[i] == 0xEB)&&(buf[i+3] == 0xEB))
	    {
	        templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB���ĳ��� 
	        i += templen;
               rc = i;
	    }
        
           if(buf[i] == 0x68)
           {
               if(buf[i+1] != 0x04)
               {
               //templen = (INT16U)buf[i+1]&0x00FF;
                   len104 = buf[i+1] + 2;
                   saveRecord(buf+i,len104,TXSAVEMODE,1);
                   PackFra104ToEb(buf+i ,len104,ebbuf,wChanNo);

                    templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ���
                    
                    if(templen > 6)
     			    {
                        tmp = i + len104;
                        //i += len104;
                        if(tmp < len)
                        {
                            memcpy(buf+i+templen ,buf+tmp,len - tmp);
                            len = len -len104 + templen;
                        }    
                        memcpy(buf+i,ebbuf,templen);
                        i += templen;
                        rc += templen;
     			    }
                    else
    				{
    				    tmp = i + len104;
    				    if(tmp<len)
    				    {
    						memcpy(ebbuf,buf+tmp,len - tmp);
    						memcpy(buf+i,ebbuf,len - tmp);
    						len = len - len104;
    				    }
    					else
    					{
    						i += len104; 
    					}
    				}
               }
               else
               {
                   PackFixed104ToEb(buf+i,6,ebbuf);
                   tmp = i + 6;
                   templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
                   if(tmp < len)
                   {
                       memcpy(buf+i+templen ,buf+tmp,len - tmp);
                       len = len -6 + templen;
                   }                     
                   memcpy(buf+i,ebbuf,templen);
                   rc += templen;
	            i += templen;
               }
           }
           else
           {
               i++;
           }
        /*
	    if((buf[i] == 0x10)&&(buf[i+5] == 0x16))
	    {
	        PackFra10ToEb(buf+i,fixmlen,ebbuf);
               templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
               memcpy(buf+rc,ebbuf,templen);
               rc += templen;
	        i += fixmlen;
	    }
		if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]=buf[i+2]))
	    {

	        templen = (INT16U)buf[i+1]&0x00FF;
               templen = templen + 6;
               
               PackFra68ToEb(buf+i ,templen,ebbuf);
               templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
               memcpy(buf+rc,ebbuf,templen);
               rc += templen;
               
	    }
	    */
	}
       if(rc > 0)
       {
           (*SEBtaillen) = i;
       }
       return rc;
}

/********************************************************************
*�������ƣ�PackFra104ToEb
*���ܣ���װ104������EB��
*���룺pdata������,len:68���ĳ��ȣ�ebbuf:EB���Ĵ�Ż�����
*�з��ˣ�����
*********************************************************************/

void PackFra104ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo)
{

    INT8U sum,ensureflag,encot,rc;
    INT16U eblen = 0;
    INT8U tsgcbuf[355];

    ebbuf[0] = ebbuf[3] = 0xEB;
    ensureflag = 0;
    if(pdata[6] == 203)
    {
        ensureflag = pdata[14];
        
        if(ensureflag == 0x40)
        {
            ensureflag = 0x80;
        }
        
        if(rmparaflag == 2)
        {
            ensureflag = 0;
        }
    }
    else if((pdata[6] == 46)||(pdata[6] == 0X2D))
    {
        ensureflag = pdata[15];
    }
    else if(pdata[6] == 211)
    {
        ensureflag = pdata[15];
        
	    encot = pdata[8]&0x3F;
        if((encot == 7)&&((ensureflag & 0x80) == 0))//��������ȷ��
        {
            Upendflag = 1;
        /*
            if(UpdateProgramMd5() == TRUE)
            {
                SGCVerifyUpLoadData(wChanNo);
            }
            else
            {
                ClearProgramUpdate();
            }
            */
        }
    }
    //ensureflag = pdata[len - 1];

    memcpy(tsgcbuf+2,pdata,len);
    
    if(ensureflag & 0x80)
    {
        //Sgc1161ObtainRandata();
        rc = Sgc1161ObtainRandata();
        
        if(rc != 0)
        {
            memset(ebbuf,0,len);
            EbErrCodeSend(0x9109,0x1f,wChanNo);
            return ;
        }
        
        tsgcbuf[0] = 0x02;
        tsgcbuf[1] = len;        
        tsgcbuf[len + 2] = 0x00;
        tsgcbuf[len + 3] = 0x08;
        
        memcpy(tsgcbuf+len + 4,SgcSelfRandbuf,8);     
        
        //logSysMsgNoTime("Rand,%x,%x,%x,%x",SgcSelfRandbuf[0],SgcSelfRandbuf[1],SgcSelfRandbuf[2],SgcSelfRandbuf[3]); 
        //logSysMsgNoTime("Rand,%x,%x,%x,%x",SgcSelfRandbuf[4],SgcSelfRandbuf[5],SgcSelfRandbuf[6],SgcSelfRandbuf[7]); 
        
        rc = Sgc1161EncryptData(tsgcbuf,len+12);
        
    }
    else
    {
        tsgcbuf[0] = 0x00;
        tsgcbuf[1] = len;   
        tsgcbuf[len + 2] = 0x00;
        tsgcbuf[len + 3] = 0x00;
        
        rc = Sgc1161EncryptData(tsgcbuf,len+4); //WY
    }
    
    if(rc == 0)
    {
        eblen = tsgcbuf[1]+(tsgcbuf[0]<<8) + 2;
        ebbuf[1] = HIBYTE(eblen);
        ebbuf[2] = LOBYTE(eblen);   
            
        ebbuf[4] = 0x00;   
        ebbuf[5] = 0x09;
        memcpy(ebbuf+6,tsgcbuf+2,eblen - 2);

    }
    else
    {
        memset(ebbuf,0,len);
        EbErrCodeSend(0x9109,0x1f,wChanNo);
        return ;
    }
        //tsgcbuf:���ֽ�68���ĳ���+68����+���ֽ����������+�����

    sum = GetEbMsgCheckSum(ebbuf);
    
    ebbuf[eblen +  4] = sum;
    ebbuf[eblen +  5] = 0xD7;

}
/********************************************************************
*�������ƣ�PackFixed104ToEb
*���ܣ���װ10֡������EB��
*���룺pdata������,len:10���ĳ��ȣ�ebbuf:EB���Ĵ�Ż�����
*�з��ˣ�����
*********************************************************************/

void PackFixed104ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{

    INT8U sum;
    INT16U slen;

	slen = len + 6;//��������2��Ӧ������1��101�����ֽ�1 ���ֽ���չ������ 00 00

    ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + EBAUDATASTARTSITE - 1, pdata, len);//101���ĳ����ֽ�ֻ��1��������Ҫ��һ
    ebbuf[EBAUDATASTARTSITE + len ] =0x00;
    ebbuf[EBAUDATASTARTSITE + len + 1] =0x00;

    sum = GetEbMsgCheckSum(ebbuf);
    ebbuf[slen +  4] = sum;
    ebbuf[slen +  5] = 0xD7;
	
}
/********************************************************************
*�������ƣ�Pack101msgtoEb
*���ܣ���101����ת��ΪEB��ȫ����
*���룺buf�����ģ�SEBtaillen:��ȫ���ݻ�������len���ĳ���
*�з��ˣ�����
*********************************************************************/

INT16U Pack101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo)
{
    INT16U i,rc,tmp;
    INT8U ebbuf[350];
    INT16U templen;
    
    i = 0;
    rc = 0;
    memset(ebbuf,0,350);
    
    if(fixmlen == 0)
    {
        fixmlen = 6;
    }
    
    while(i < len)
    {
       //logSysMsgNoTime("Pack101msgtoEb!",0,0,0,0);
	if((buf[i] == 0xEB)&&(buf[i+3] == 0xEB))
	{
	    templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB���ĳ��� 
	    i += templen;
           rc = i;
	}
       ///////////����101����
       if((buf[i] == 0x10)&&(buf[i+fixmlen-1] == 0x16))
	{
	    PackFra10ToEb(buf+i,fixmlen,ebbuf);
        
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
           tmp = i + fixmlen;
               
           if( tmp < len)
           {
               memcpy((buf+i+templen) ,buf+tmp,len - tmp);//���������ĺ���
               len = len - fixmlen + templen;
           }
           memcpy((buf+i ),ebbuf,templen);
           i += templen;
           rc += templen;
	}
       else if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]==buf[i+2]))////////�Ƕ���101����
	{

	    templen = (INT16U)buf[i+1]&0x00FF;
           templen = templen + 6;//101���ĳ���
           //buf[i+templen-2] = 0x68;           
           saveRecord(buf+i,templen,TXSAVEMODE,1);
           PackFra68ToEb(buf+i ,(INT8U)templen,ebbuf,wChanNo);
           
           tmp = i + templen;
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
               
           if(tmp < len)
           {
               memcpy(buf+i+templen ,buf+tmp,len - tmp);
               len = len - tmp + i + templen;
           }
           
           memcpy(buf+i,ebbuf,templen);
               
           i += templen;//
           rc += templen;
               
	 }
       else
       {
           i++;
       }
    }
    if(rc > 0)
    {
        (*SEBtaillen) = i;
    }
    return rc;
}
/********************************************************************
*�������ƣ�PackFra68ToEb
*���ܣ���װ68֡������EB��
*���룺pdata������,len:68���ĳ��ȣ�ebbuf:EB���Ĵ�Ż�����
*�з��ˣ�����
*********************************************************************/
//���ڿ��Կ��ǽ�tsgcbuf�����ZHANGLIANG
void PackFra68ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo)
{

    INT8U sum,ensureflag,rc,tilocation,encot;
    INT16U eblen;
    INT8U tsgcbuf[350];
    
    ebbuf[0] = ebbuf[3] = 0xEB;
    ensureflag = 0;
    tilocation = fixmlen +1; 

    if(pdata[tilocation] == 203)
    {
        ensureflag = pdata[fixmlen + 9];//������ʶ��()
        if(rmparaflag == 2)
        {
            ensureflag = 0;
        }
    }
    else if((pdata[tilocation] == 46)||(pdata[tilocation] == 0X2D))
    {
        ensureflag = pdata[len -3];
    }
    else if(pdata[tilocation] == 211)
    {
        ensureflag = pdata[len -3];
        encot = pdata[fixmlen + 3]&0x3F;//����ԭ��
        if((encot == 7)&&((ensureflag & 0x80) == 0))//��������ȷ��
        {

            Upendflag = 1;
        /*
            if(UpdateProgramMd5() == TRUE)
            {
                SGCVerifyUpLoadData(wChanNo);
            }
            else
            {
                ClearProgramUpdate();
            }
            */
        }
    }
    
    memcpy(tsgcbuf+2,pdata,len);
    
    if(ensureflag & 0xC0)
    {
        rc = Sgc1161ObtainRandata();
        
        if(rc != 0)
        {
            memset(ebbuf,0,len);
            EbErrCodeSend(0x9109,0x1f,wChanNo);
            return ;
        }
        
        tsgcbuf[0] = 0x02;
        tsgcbuf[1] = len;
        tsgcbuf[len + 2] = 0x00;
        tsgcbuf[len + 3] = 0x08;
        
        memcpy(tsgcbuf+len + 4,SgcSelfRandbuf,8);     

        rc = Sgc1161EncryptData(tsgcbuf,len+12);
        
    }
    else
    {
        tsgcbuf[0] = 0x00;
        tsgcbuf[1] = len;    
        tsgcbuf[len + 2] = 0x00;
        tsgcbuf[len + 3] = 0x00;
        rc = Sgc1161EncryptData(tsgcbuf,len+4);
    }
    
    if(rc == 0)
    {
        eblen = tsgcbuf[1]+(tsgcbuf[0]<<8) + 2;
        ebbuf[1] = HIBYTE(eblen);
        ebbuf[2] = LOBYTE(eblen);   
            
        ebbuf[4] = 0x00;   
        ebbuf[5] = 0x09;
        memcpy(ebbuf+6,tsgcbuf+2,eblen - 2);

    }
    else
    {
        memset(ebbuf,0,len);
        EbErrCodeSend(0x9109,0x1f,wChanNo);
        return ;
    }
        //tsgcbuf:���ֽ�68���ĳ���+68����+���ֽ����������+�����

    sum = GetEbMsgCheckSum(ebbuf);
    
    ebbuf[eblen +  4] = sum;
    ebbuf[eblen +  5] = 0xD7;

}
/********************************************************************
*�������ƣ�PackFra10ToEb
*���ܣ���װ10֡������EB��
*���룺pdata������,len:10���ĳ��ȣ�ebbuf:EB���Ĵ�Ż�����
*�з��ˣ�����
*********************************************************************/

void PackFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{

    INT8U sum = 0;
    INT16U slen;

    slen = len + 6;//��������2��Ӧ������1��101�����ֽ�1 ��ȫ��չ�������ֽ�2

    ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + EBAUDATASTARTSITE - 1, pdata, len);//101���ĳ����ֽ�ֻ��1��������Ҫ��һ
    ebbuf[EBAUDATASTARTSITE + len ] =0x00;
    ebbuf[EBAUDATASTARTSITE + len + 1] =0x00;
    
    sum = GetEbMsgCheckSum(ebbuf);
    ebbuf[slen +  4] = sum;
    ebbuf[slen +  5] = 0xD7;
	
}

/*
INT8U EbMsgRxdBuf[FRAMEBUFSIZE];

void EbSafetySearchFrame(void)
{
    INT16U eblenth;
    while (RxdHead<RxdTail)
    {
        if(EbMsgRxdBuf[RxdHead] == 0xEb)
        {
            if(CheckEbMsgSty((EbMsgRxdBuf+ RxdHead)))
            {
				EbmsgAnalysis();
				eblenth = EbMsgRxdBuf[2]+(EbMsgRxdBuf[1]<<8);
				RxdHead += (eblenth + 6);
                break;
            }
            else
            {
                RxdHead++;
            }
        }
        else
        {
            RxdHead++;
        }       
    }    
    
}

void PackFra68ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{
    
}

void PackFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{
    INT8U sum;
    INT16U slen;

	slen = len + 4;//��������2��Ӧ������1��101�����ֽ�1

	ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + 9 - 1, pdata, len);//101���ĳ����ֽ�ֻ��1��������Ҫ��һ
    sum = GetEbMsgCheckSum(ebbuf);
	ebbuf[slen +  4] = sum;
    ebbuf[slen +  5] = 0xD7;
	
}
	
*/
/********************************************************************
*�������ƣ�SGCReceiveData
*���ܣ�����SGC1161��ȫ����оƬ���ص�����
*���룺prcv�����ݻ�����,len:����
*�з��ˣ�����
*********************************************************************/

INT16U SGCReceiveData(INT8U *prcv, INT16U len)
{
    INT8U data[17], display[17];
    INT16U i,j, readlen;
    INT8U rc = FALSE;
    //INT8U msg[356];
    INT8U lrc;

    ////semTake(sem_qspiid, WAIT_FOREVER);
    //if(pHeReceivebBuf == NULL)
    //    return -5;
    memset(data,0, 17);
    
    //����0x55ͷ,����10��,ÿ��û�ҵ����ӳ�100ms
    for(j=0;j<10;j++)
    {

        for(i=0;i<16;i++)
        { 
            data[0] = SGC_SPI_READ_CODE;
            //SGCspiRead(&data[0], 1);
            Mcf5275QspiSend8(&data[0], 1, QSPI_CS3, FALSE, TRUE);
            display[i] = data[0];
            if(data[0] == SGC_HEAD)
            {
                break;
            }
            
        }
        if(i<16)
        {
            break;   
        }
        else
        {
            //myTaskDelay(10);
        }
    }
    

    if(j>=10)
    {
        logSysMsgNoTime("û�ж���0x55ͷdebug ",0,0,0,0);
        HEPowerReset(10);//����SC1161оƬ
        prcv[0] = 0x00;
        prcv[1] = 0x01;
        prcv[2] = 0x91;
        prcv[3] = 0x09;
        //myTaskDelay(15);    
        ////semGive(sem_qspiid);
        return -1;  
    }
    
    //SGCspiRead(&data[1], 4);
    Mcf5275QspiSend8(&data[1], 4, QSPI_CS3, FALSE, TRUE);
    rc = 0;
    readlen = 0;
    switch(data[1] & SGC_SW1_HIGH)
    {
    case 0x90:
        if((data[1] == 0x90) && (data[2] == 0x00))
        {
            readlen = (data[3]<<8)+data[4];
            if(readlen<=len)
            {    
                rc = 0;
            }
            else
            {
                
                rc = 4;        //��ȡ�������������������
                logSysMsgNoTime("оƬ������=%x,��������=%x��оƬ���ݳ��Ȳ���",readlen,len,0,0);
                readlen = len;
            }
        }
        else if(data[2] == 0x86)
        {

            logSysMsgNoTime("��ǩʧ��,sw1=%x, sw2=%x",data[1],data[2],0,0);
            rc = 1;
        }
        else
        {
            rc = 4;
            logSysMsgNoTime("оƬ����ֵ�쳣��sw1=%x,sw2=%x",data[1],data[2],0,0); 
        }
        break;
    case 0x60:
        if((data[1] == 0x67)&&(data[1] == 0x00))
        {
            logSysMsgNoTime("У����򳤶ȴ���,sw1=%x, sw2=%x",data[1],data[2],0,0);
            rc = 5;
        }
        else if((data[1] == 0x6A)&&(data[2] == 0x90))
        {
            rc = 77;
            logSysMsgNoTime("�������,sw1=%x, sw2=%x",data[1],data[2],0,0);
        }
        else
        {
            logSysMsgNoTime("оƬ������������,sw1=%x, sw2=%x",data[1],data[2],0,0);
            rc = 2;
        }
        //readlen = (data[3]<<8)+data[4];
        break;
    default: 
        readlen = 0;
        logSysMsgNoTime("оƬӦ�����,sw1=%x, sw2=%x",data[1],data[2],0,0);
        HEClearNotReadData();   //��δ���������ݶ��� 
        
        ////HEPowerReset(10);//����SC1161оƬ
        prcv[0] = 0x00;
        prcv[1] = 0x01;
        prcv[2] = 0x91;
        prcv[3] = 0x09;
        //myTaskDelay(15);    
        ////semGive(sem_qspiid);
        return 1;  
        break;
    }
    
    if(prcv)
    {
        memset(prcv, SGC_SPI_READ_CODE, readlen);
        //SGCspiRead((prcv+2), readlen);
        Mcf5275QspiSend8((prcv+4), readlen, QSPI_CS3, FALSE, TRUE);
        //memcpy(msg,(data+1),4);
        //memcpy((msg+4),(prcv+2),readlen);
        //SGCspiRead(&msg[4+readlen], 1);
        //Mcf5275QspiSend8(&msg[4+readlen], 1, QSPI_CS3, TRUE, TRUE);
        
        memcpy(prcv,data+1,4); 
        
        Mcf5275QspiSend8(&data[5], 1, QSPI_CS3, TRUE, TRUE);
        lrc = HECalLRC(prcv,readlen+4);
        
        if (lrc != data[5])
        {
            //logSysMsgNoTime("SGCCalLRC err", 0,0,0,0);
            ////semGive(sem_qspiid);
            return 0x11;
        }
        //{
        memcpy(prcv,prcv+2,readlen+2);
            //logSysMsgNoTime("SGCCalLRC err", 0,0,0,0);
        //}
        //else
        //{
          //   memcpy(prcv,(data+3),2);
        //}
    }
    else
    {
        //��ʣ�����ݲ�����Ƭѡ
        for(i=0;i<readlen;i++)
        { 
            data[0] = HE_SPI_READ_CODE;
            Mcf5275QspiSend8(&data[0], 1, QSPI_CS3, FALSE, TRUE);
        }
        Mcf5275QspiSend8(&data[5], 1, QSPI_CS3, TRUE, TRUE);

        //SGCspiRead(&data[5], 1);
    } 
    ////semGive(sem_qspiid);
    return rc;
}
/*------------------------------------------------------------------/
�������ƣ�  UpdateProgramMd5()
�������ܣ�  ��������ļ���MD5ֵ
����˵����  
���˵����  TRUE ��ʾ��MD5ֵ  FALSE ��ʾû��MD5ֵ
��ע��      �ڷ�����վ����������ȷ��֡�󣬿�ʼ���㲢У��MD5
/------------------------------------------------------------------*/
BOOL UpdateProgramMd5(void)
{
    INT8U   *p8;
    MD5state temp ;
    INT32U  len;
    
    p8 = GetImageInfo(&len); 
    
    if((len==0) || (p8==NULL))
    {
        return FALSE;
    }
    memset(&temp, 0, sizeof(MD5state));
    GenerateMD5(p8, len, UoLoadMD5, &temp);
    logSysMsgNoTime("MD5ֵ1 %x-%x-%x-%x", UoLoadMD5[0],UoLoadMD5[1],UoLoadMD5[2],UoLoadMD5[3]);
    logSysMsgNoTime("MD5ֵ2 %x-%x-%x-%x", UoLoadMD5[12],UoLoadMD5[13],UoLoadMD5[14],UoLoadMD5[15]);
    
    return TRUE;
    
}   
/*------------------------------------------------------------------/
�������ƣ�  CheckEncrptchip()
�������ܣ�  ������оƬ�Ƿ������ʹ��
����˵����  CheckType:0��ʾά��������ã�������ʾ�����Լ�
���˵����  0��ʾоƬ����������
��ע��      
/------------------------------------------------------------------*/
INT8U CheckEncrptchip(INT8U CheckType)
{
    INT8U rc = 0;
    INT8U i = 0;
    INT8U KeyVersion = 0;
    char msgbuf[50];
    INT8U *p;
    
    p = ParaTempBuf+1024;   
    //myTaskDelay(5);    
    rc = Sgc1161GetChipSerialNumID(p);
    while(rc != 0)
    {
        
        myTaskDelay(2);    
        rc = Sgc1161GetChipSerialNumID(p);
        i++;
        if(i > 1)
        {
            //logSysMsgNoTime("����оƬ����쳣.rc=%d",rc,0,0,0);
            i = 0;
            
            return 1;
        }
    }
    
    if(CheckType == 0)
    {
        memset(msgbuf,0,50);
        sprintf(msgbuf,"����оƬ1161���к�:%02x%02x%2x%02x%02x%02x%02x%02x\r\n",p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
        logSysMsgNoTime(msgbuf,0,0,0,0);
    }

    rc = Sgc1161GetChipKeyVersion(NULL,p);   
    while(rc != 0)
    {
        myTaskDelay(2);    
        rc = Sgc1161GetChipKeyVersion(NULL,p);
        i++;
        if(i > 1)
        {
            //logSysMsgNoTime("����оƬ����쳣.rc=%d",rc,0,0,0);
            i = 0;
            
            return 2;
        }
    }
    
    KeyVersion = p[2];
    if(CheckType == 0)
    {
        logSysMsgNoTime("����оƬ1161��Կ�汾��:%d",KeyVersion,0,0,0);
    }
    else
    {
        logSysMsgNoTime("����оƬ1161�Լ�ɹ�����Կ�汾��:%d",KeyVersion,0,0,0);
        return 0;
    }

    
    rc = Sgc1161CheckoutCer(p);
    while(rc != 0)
    {
        myTaskDelay(2);    
        rc = Sgc1161CheckoutCer(p);
        i++;
        if(i > 3)
        {
            //logSysMsgNoTime("����оƬ����쳣.rc=%d",rc,0,0,0);
            i = 0;
            
            return 3;
        }
    }
    //logSysMsgNoTime("����оƬ�������.rc=%d",rc,0,0,0);
    return rc;
}

/********************************************************************
*�������ƣ�SgcYWToolGetPbKeyCer
*���ܣ�������Կ֤��(�ն�֤��)��֤�������
*���룺pdata�����ģ�len��EB���ĵĳ��ȣ�����������У���룩wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT8U SgcMaintGetPbKeyCer(INT8U *p)
{
     
     INT8U rc;
     //INT8U *p;
     
     //p = ParaTempBuf+1024;

     rc = Sgc1161CheckoutCer(p);
     
     if(rc != 0)
     {

         logSysMsgNoTime("�ն�֤�鵼��ʧ��.rc=%d",rc,0,0,0);
         p = NULL;
         return 0;

     }

     return 1;
}	

#if 0
#endif
////////////////////////////////////////////////////////////////
//����ũ�����ܽӿ�sgc1120a
//����
////////////////////////////////////////////////////////////////

/********************************************************************
*�������ƣ�Sgc1120aGetChipSerialNumID
*���ܣ���ȡоƬ���к�
*���룺rcvbuf:���ݴ�Ż�����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aGetChipSerialNumID(INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
	
    if(rcvbuf == NULL)
    {
        return 0;
    }
    hecmd.cla  = 0x00;
    hecmd.ins  = 0xB0;
    hecmd.p1   = 0x99;
    hecmd.p2   = 0x05;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x08;
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, 8);  //
    
    myTaskDelay(1);    
    
    rc = SGCReceiveData(rcvbuf,8);
    semGive(sem_qspiid);
    return rc;   
}

/********************************************************************
*�������ƣ�Sgc1120aGetChipKeyVersion
*���ܣ���ȡоƬ��Կ�汾��
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aGetChipKeyVersion(INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
	
    if(rcvbuf == NULL)
    {
        return 0;
    }
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x5E;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x00;
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, NULL, 0);  //
    
    myTaskDelay(1);    
    
    rc = SGCReceiveData(rcvbuf,3);
    semGive(sem_qspiid);
    return rc;       
    
}

/********************************************************************
*�������ƣ�Sgc1120aGetRandomData
*���ܣ���ȡоƬ�����
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aGetRandomData(INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
	
    if(rcvbuf == NULL)
    {
        return 0;
    }
    hecmd.cla  = 0x00;
    hecmd.ins  = 0x84;
    hecmd.p1   = 0x00;
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x08;
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, 8);  //
    
    myTaskDelay(1);    
    
    rc = SGCReceiveData(rcvbuf,8);
    semGive(sem_qspiid);
    return rc;       
}

/********************************************************************
*�������ƣ�SgcCalculateAuthRData
*���ܣ����������֤����
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aCalculateAuthRData(INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
	
    if(rcvbuf == NULL)
    {
        return 0;
    }
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x5C;
    hecmd.p1   = 0x01;
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x08;
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, SgcMasterRandbuf, 8);  //
    
    myTaskDelay(5);    
    
    rc = SGCReceiveData(rcvbuf,0x12);
    semGive(sem_qspiid);
    return rc;       
}

/********************************************************************
*�������ƣ�SgcGetPKeyAuthData
*���ܣ���ȡ��Կ���ܽ��
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aGetPKeyAuthData(INT8U Fid,INT8U *pdata,INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;

	if((Fid>4) || (pdata==NULL)|| (rcvbuf==NULL))
	{
	   return 1; 
	}
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x4E;
    hecmd.p1   = 0x80+Fid;
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x08;
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd,pdata,0x08);  //��վ�·��Ĺ�Կ��֤�����
    
    myTaskDelay(50);    
    
    rc = SGCReceiveData(rcvbuf,0x6A);
    semGive(sem_qspiid);
    return rc;       
}

/********************************************************************
*�������ƣ�SgcGetKeyConsultData
*���ܣ���ԿЭ��
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aGetKeyConsultData(INT8U Fid,INT8U *pdata,INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;

	if((Fid>4)||(pdata==NULL)||(rcvbuf==NULL))
	{
	   return 1; 
	}
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x60;
    hecmd.p1   = 0x80+Fid;
    hecmd.p2   = 0x01;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x48;
	
    memcpy(SGCConsultrandom,pdata,SGCRANDOMLEN);
    memcpy(&pHeSendBuf2[0],SGCConsultrandom,SGCRANDOMLEN);
    memcpy(&pHeSendBuf2[SGCRANDOMLEN],pdata+SGCRANDOMLEN,0x40);
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2,0x48);  
    
    myTaskDelay(50);

    rc = SGCReceiveData(rcvbuf,0x7A);
    semGive(sem_qspiid);
	return rc;
}

/********************************************************************
*�������ƣ�SGC1120aSginVerify
*���ܣ���֤��Կ����ǩ��
*���룺
*�з��ˣ�����
*********************************************************************/
/*
INT8U Sgc1120aUploadPKeySignData(INT8U Kid,INT8U *pdata,INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
	INT8U Pid;
	
	if((Kid>4) || (pdata==NULL)|| (rcvbuf==NULL))
	{
	   return 1; 
	}
	
	Pid = pdata[0]+0x80;
	
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x5A;
    hecmd.p1   = 0x80+Kid;//ǩ����Կ����
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x89;
	
	pHeSendBuf2[0] = Pid;
    memcpy((&pHeSendBuf2[0]+1), pdata+1, SGCRANDOMLEN);//���¹�Կ����+ �����0x01+0x08
    memcpy((&pHeSendBuf2[0]+9), pdata+9, 0x80);//��Կֵ+ǩ�����0x40+0x40
	HESendCmd(&hecmd, pHeSendBuf2,0x89); 
	
    myTaskDelay(5);

	rc = SGCReceiveData(rcvbuf,4);
	return rc;
}
*/
	
/*------------------------------------------------------------------/
�������ƣ�	SGCSginVerify()
�������ܣ�	��ǩ()
����˵����	
			pkey 64�ֽڹ�Կָ��  keyno ��Կ��ţ�0~3 ��ʾ1~4�Ź�Կ)
			pucDataInput ǩ������Դ 
			pucsign ǩ��ֵ 
���˵����	0 �ɹ� 
			-10 ��������
			���� ʧ��
��ע��		
/------------------------------------------------------------------*/

INT8U SGC1120aSginVerify(INT8U *DataInput, INT16U inputlen, INT8U *pucsign, INT16U signlen, INT8U keyno)
{
    struct HESendCmd_t hecmd;
    INT8U rc;
    
    if((keyno>4) || (DataInput==NULL)|| (pucsign==NULL))
    {
        return 1;
    }
    
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x5A;
    hecmd.p1   = 0x80+keyno;
    hecmd.p2   = 0;
    hecmd.len1 = HIBYTE(inputlen+signlen);
    hecmd.len2 = LOBYTE(inputlen+signlen);
    
    memcpy(&pHeSendBuf2[0],DataInput,inputlen);
    memcpy(&pHeSendBuf2[inputlen],pucsign,signlen);  
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd,pHeSendBuf2,(inputlen+signlen));  //64�ֽ�
    
    myTaskDelay(50);    
    
    rc = SGCReceiveData(NULL,0);
    semGive(sem_qspiid);
    
    if(rc == 0)
    {
        logSysMsgNoTime("SGCSginVerify:��ǩ��ȷ",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("SGCSginVerify:��ǩʧ��.rc=%d",rc,0,0,0);
    }   
    return rc;   
}

/********************************************************************
*�������ƣ�SgcUploadPKeyDataToChip
*���ܣ����¹�Կ��оƬ
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aUploadPKeyDataToChip(INT8U *pdata,INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;
	INT8U Pid;

	if((pdata==NULL)|| (rcvbuf==NULL))
	{
	   return 1; 
	}
	
	Pid = pdata[0];//+0x80;
	
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x52;
    hecmd.p1   = 0x80+Pid;//���¹�Կ����
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x40;
	
    memcpy(&pHeSendBuf2[0],pdata+9, 0x40);
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
	HESendCmd(&hecmd, pHeSendBuf2,0x40); 
	
    myTaskDelay(50);

	rc = SGCReceiveData(rcvbuf,4);
    semGive(sem_qspiid);
	return rc;
}
/********************************************************************
*�������ƣ�SgcUploadSymKeyDataToChip
*���ܣ����¶Գ���Կ��оƬ
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aUploadSymKeyDataToChip(INT8U *pdata,INT8U *rcvbuf)
{
    struct HESendCmd_t hecmd;
    int rc;

	if((pdata==NULL)|| (rcvbuf==NULL))
	{
	   return 1; 
	}
	
    hecmd.cla  = 0x84;
    hecmd.ins  = 0xD4;
    hecmd.p1   = 0x01;//���¹�Կ����
    hecmd.p2   = 0xFF;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x84;
	
    memcpy(&pHeSendBuf2[0], pdata, 0x84);
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
	HESendCmd(&hecmd, pHeSendBuf2,0x84); 
	
    myTaskDelay(90);

	rc = SGCReceiveData(rcvbuf,4);
    semGive(sem_qspiid);
	return rc;
}

/********************************************************************
*�������ƣ�Sgc1120aDectyData
*���ܣ�����
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aDectyData(INT8U *pdata,INT8U *rcvbuf,INT16U len)
{
    struct HESendCmd_t hecmd;
    int rc;
    INT8U i,j;
	if((pdata==NULL)|| (rcvbuf==NULL))
	{
	   return 1; 
	}
	
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x64;
    hecmd.p1   = 0x00;//���¹�Կ����
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE(len);
    hecmd.len2 = LOBYTE(len);
	
    memcpy(&pHeSendBuf2[0], pdata, len);
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
	HESendCmd(&hecmd, pHeSendBuf2,len); 
	
    myTaskDelay(15);

	rc = SGCReceiveData(rcvbuf,330);
    semGive(sem_qspiid);
    j = 0;
    while(rc == 0x11)
    {
        myTaskDelay(1);    
 
		hecmd.cla	= 0x80;
		hecmd.ins  = 0x64;
		hecmd.p1	 = 0x00;//���¹�Կ����
		hecmd.p2	 = 0x00;
		hecmd.len1 = HIBYTE(len);
		hecmd.len2 = LOBYTE(len);

		memcpy(&pHeSendBuf2[0], pdata, len);
        
        semTake(sem_qspiid, WAIT_FOREVER);
        myTaskDelay(1);    
		HESendCmd(&hecmd, pHeSendBuf2,len); 

		myTaskDelay(15);

		rc = SGCReceiveData(rcvbuf,330);
        semGive(sem_qspiid);
        for(i = 0;i < 3;i++)
        {
            if(rc == 77)
            {
            
                myTaskDelay(2);    
                semTake(sem_qspiid, WAIT_FOREVER);
                myTaskDelay(1);    
                HESendCmd(&hecmd, pHeSendBuf2, len);  //          
                myTaskDelay(15);    
                rc = SGCReceiveData(rcvbuf,330);
                semGive(sem_qspiid);
            }
            else
            { 
                break;
            }        
        }

        j++;
        if(j > 3)
        {
            break;
        }
    }
	return rc;
}

/********************************************************************
*�������ƣ�Sgc1120aEnctyData
*���ܣ�����
*���룺
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aEnctyData(INT8U *pdata,INT16U len)
{
    struct HESendCmd_t hecmd;
    int rc;
    
    if(pdata==NULL)
    {
         logSysMsgNoTime("pdata=NULL!!!",0,0,0,0);
	   return 1; 
    }
	
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x62;
    hecmd.p1   = 0x00;//���¹�Կ����
    hecmd.p2   = 0x00;
    hecmd.len1 = HIBYTE((len+8));
    hecmd.len2 = LOBYTE((len+8));
	
    memcpy(&pHeSendBuf2[0],SGCConsultrandom,SGCRANDOMLEN);
    memcpy((&pHeSendBuf2[0]+8),pdata,len);
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
	HESendCmd(&hecmd, pHeSendBuf2,(len+8)); 
    
    myTaskDelay(15);
    ////DcaiGetValue(0); 
    ////dataflash_read_ID(QSPI_CS0);
	rc = SGCReceiveData(pdata,255);
    semGive(sem_qspiid);
	return rc;
}

/********************************************************************
*�������ƣ�EbSafetySearchFrame
*���ܣ������յ���EB���ģ���������ת��Ϊ101/104֡
*���룺oribuf��EB���Ļ�����,validbuf:101/104���Ĵ�Ż�������
*validtaillen:RxdTail��len�����յ���EB����������,wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/

INT16U Eb1120aSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo)
{
    INT16U eblenth,i;
    INT16U checklen = 0;
    INT16U tail = 0; 

    i = 0;
	if(len > 512)
	{
	    len = 512;
	}
    while (i < len)
    {
        if(oribuf[i] == 0xEb)
        {
            if((i+4) > len )
            {
                return i ;
            }
        }
        
        if(oribuf[i] == 0xEb&&oribuf[i + 3] == 0xEb)
        {

            checklen = oribuf[i+2]+(oribuf[i+1]<<8);

            if(checklen > (512 -6))//���checklen>512����� i ����ִ��++
            {
                i++;
            }
            else
            {
                if(checklen > (len- i))
                {
                    return i ;
                }
             
                if(Check1120aEbMsgSty((oribuf+ i)))
                {
    	            eblenth = oribuf[i+2]+(oribuf[i+1]<<8);
    	            LenFromEbMsg = Eb1120amsgAnalysis(oribuf+i,((INT8U*)validbuf+tail),wChanNo);
    		        i += (eblenth+6);
                    (*validtaillen) += LenFromEbMsg;
                    tail += LenFromEbMsg;
                    LenFromEbMsg = 0;
                }
                else
                {
                    i++;
                }
            }
        }
        else
        {
            i++;
        }    
        
    } 
    
    return i;
}
/********************************************************************
*�������ƣ�CheckEbMegSty
*���ܣ������Ƿ�EB����
*���룺pdata������
*�з��ˣ�����
*********************************************************************/
BOOL Check1120aEbMsgSty(INT8U *pdata)
{
    INT8U sum = 0;
    INT16U i = 0;
	
    if((pdata[0] == 0xEB)&&(pdata[3] == 0xEB))
    {
        i = (INT16U)pdata[1];
		i = (i<<8)+(INT16U)pdata[2];
		
        sum = GetEbMsgCheckSum(pdata);
        if((sum == pdata[i + 4])&&(pdata[i + 5] == 0xD7))
        {
            return TRUE;
        }
        else
        {
            logSysMsgNoTime("checksum err1 sum = %x",sum,0,0,0);
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    
}

/********************************************************************
*�������ƣ�EbmegAnalysis
*���ܣ�����Eb��ʽ����
*���룺pdata�����ģ�rxbuff:101/104���Ĵ�Ż�������wChanNo:�˿ں�
*�з��ˣ�����
*********************************************************************/
INT16U Eb1120amsgAnalysis(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    BOOL  Encrptyflag;
    INT8U lenth,rc;
    INT8U bwlen = 0;
    INT16U wEblenth;
	
    wEblenth = pdata[2]+(pdata[1]<<8);//���ĳ���(����ͷ��У����)

	if(pdata[5]&0x08)//�Ƿ�����
    {
		Encrptyflag = TRUE;
    }
	else//�Ǽ��ܱ��Ĵ���
	{
		Encrptyflag = FALSE;
	}

    if(Encrptyflag)
    {
        if( AuthEndflag == 0)
        {
    
            EbErrCodeSend(0x9107,0x1f,wChanNo);
            bwlen = 0;
        }
        
        bwlen = Eb1120aEncpytDataAnaly((pdata+6),(wEblenth-2),rxbuff,wChanNo);//���ܻ����������
    }
    else
    {
	    if( AuthEndflag == 0)
	    {
	        if(pdata[6]<0x50)
	        {
	             if(pdata[6] == 0x1f)
	             {
	                 return 0;
	             }
	             EbErrCodeSend(0x9107,0x1f,wChanNo);
                 return 0;
	        }
	    }
	    switch(pdata[6])
	    {
	        case 0x50://��վ��ȡ�ն�оƬ��Ϣ
	            Sgc1120aMasterAuthI(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
	            break;
	        case 0x52://ϵͳ�����֤
                AuthEndflag = 0;
	            Sgc1120aMasterAuthII(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
	            break;     
	        case 0x54://��վ���ն˷�����֤�������ʱ��������
	            //SgcMasterauthenStepIII(0x55,wChanNo);
                //myTaskDelay(2);
                //AuthEndflag = wChanNo;  
	            break;
			case 0x55://��Կ��֤
				Sgc1120aMasterAuthIV(pdata,wEblenth,wChanNo);
				myTaskDelay(1);
				break;
			case 0x57://��վ���ն˷�����֤�������ʱ��������
				break;
			case 0x58://��ԿЭ��
				Sgc1120aMasterAuthV(pdata,wEblenth,wChanNo);
				myTaskDelay(1);
				break;
			case 0x5A:
				Sgc1120aMasterAuthVI(pdata,wEblenth,wChanNo);
				break;
            case 0x60://��Կ����
                //if(AuthEndflag == 0)
                {
                    //logSysMsgNoTime("ԽȨ������Կ1",0,0,0,0);
                    //EbErrCodeSend(0x9091,0x61,wChanNo);
                    //return 0;
                }
                Sgc1120aPKeyUpload(pdata,wEblenth,wChanNo);
                break;
            case 0x62://��վ���ص���֤�������ʱ������
                //if(AuthEndflag == 0)
                {
                    //logSysMsgNoTime("ԽȨ������Կ2",0,0,0,0);
                    //EbErrCodeSend(0x9091,0x63,wChanNo);
                    //return 0;
                }
                //SgcKeymanageStepII(pdata,wEblenth,wChanNo,0x62);
                break;
			case 0x63://�Գ���Կ����
				Sgc1120aSymKeyUploadI(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
				break;
            case 0x65://
            
                //if(AuthEndflag == 0)
                {
                    //logSysMsgNoTime("ԽȨ�ָ���Կ",0,0,0,0);
                    //EbErrCodeSend(0x9092,0x65,wChanNo);
                    //return 0;
                }
                Sgc1120aSymKeyUploadII(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
                break;
		   case 0x66:
		   	    break;
	       case 0x00: //���ı����Ҳ�����ȫ��չ������ 
			   ////rc = Sgc1120aJudgeWhetherEn((pdata+8),wChanNo);
               ////if(rc != 0)
               {
				 //// logSysMsgNoTime("���ܱ���ʹ�����Ĵ��䣬���Ϲ�",0,0,0,0);
				  ////EbErrCodeSend(0x9106,0x1f,wChanNo);
				   ////bwlen = 0;
               }
			  ////else
			   {
				   bwlen = pdata[7];
				   memcpy(rxbuff,pdata+8,bwlen);
			   }
	           break; 
              case 0x01://д�ļ�������ĵ��Ǵ�ǩ��
              /*ZHANGLIANG 20180211
	            rc = Sgc1120aJudgeWhetherEn((pdata+8),wChanNo);
                  if(rc != 0)
                  {
				   logSysMsgNoTime("���ܱ���ʹ�����Ĵ��䣬���Ϲ�1",0,0,0,0);
				   EbErrCodeSend(0x9106,0x1f,wChanNo);
				   bwlen = 0;
                  }
  		   else
  		   */
  		   {
  		       //if((pdata[8+fixmlen+9]==0x07)&&((pdata[8+fixmlen+3]&0x3F)==6))//д�ļ�����
  		       {
  			       bwlen = Eb1120aMsgWithSData((pdata+7),rxbuff,wChanNo);
  		       }
  			  // else
  			  // {
  				   //EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
  				   //logSysMsgNoTime("д�ļ�����ҵ�����ʹ���",0,0,0,0);
  				   //bwlen = 0;
  			   //}
  		   }
	           break;
		   case 0x08:
			   bwlen = Eb1120aMsgUpLoadData((pdata+7));
		   	   break;
	       default:
			   EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
			   logSysMsgNoTime("����ҵ�����ʹ���",0,0,0,0);
			   bwlen = 0;
               break;              
	    }
    }
	return bwlen;
}
/********************************************************************
*�������ƣ�Sgc1120aMasterAuthI
*���ܣ���ȡ�ն�оƬ��Ϣ
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aMasterAuthI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc;
    INT8U rbuf[10];
    INT8U replybuf[30];
    INT16U wholelen,sdatalen;
	
    rc = Sgc1120aGetChipKeyVersion(replybuf);  
	if(rc != 0)
	{
		EbErrCodeSend(0x9001,0x1f,wChanNo);
		return rc;
	}
	
    rbuf[0] = replybuf[2];//��Կ�汾��
    rc = Sgc1120aGetChipSerialNumID(replybuf);
	
	if(rc != 0)
	{
		EbErrCodeSend(0x9001,0x1f,wChanNo);
		return rc;
	}
	
    sdatalen = replybuf[1]+(replybuf[0]<<8)+1;//���кų���+��Կ�汾�ų���
    wholelen = sdatalen+5;//����ͷ�г���
	
    memcpy((rbuf+1),(replybuf+2),(sdatalen-1));
		
    EbEditmsg(replybuf,rbuf,wholelen,0,0x51,sdatalen);

    SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	

    return 0;
}
/********************************************************************
*�������ƣ�Sgc1120aMasterAuthII
*���ܣ��ն�������֤����
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aMasterAuthII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc;
	INT8U authbuf[20];
    INT8U replybuf[50];
    INT16U wholelen,sdatalen;
	
	memset(replybuf,0,50);
	memcpy(SgcMasterRandbuf,(pdata+EBAUDATASTARTSITE),SGCRANDOMLEN);

	rc = Sgc1120aCalculateAuthRData(authbuf);
	if(rc != 0)
	{
	    logSysMsgNoTime("��ȡ��֤����ʧ��",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);//��ȡ��֤����ʧ��
		return rc;
	}
	
    sdatalen = authbuf[1]+(authbuf[0]<<8);
    wholelen = sdatalen + 5;//����ͷ�г���
    
    memcpy(authbuf,(authbuf+2),sdatalen);
	
    EbEditmsg(replybuf,authbuf,wholelen,0,0x53,sdatalen);
    SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}
/********************************************************************
*�������ƣ�Sgc1120aMasterAuthIV
*���ܣ�����Կ��֤����
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aMasterAuthIV(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc,PKeyno;
    INT8U PKeyENCbuf[150];
    INT8U replybuf[150];
    INT16U wholelen,sdatalen;

	PKeyno = pdata[EBAUDATASTARTSITE];//��Կ����
	
	rc = Sgc1120aGetPKeyAuthData(PKeyno,(pdata+10),PKeyENCbuf);
	if(rc != 0)
	{
	    logSysMsgNoTime("��ȡ��Կ���ܽ��ʧ��",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	
	memset(replybuf,0,150);
    sdatalen = PKeyENCbuf[1] + (PKeyENCbuf[0]<<8);
    wholelen = sdatalen + 5;//����ͷ�г���
    
    memcpy(PKeyENCbuf,PKeyENCbuf+2,sdatalen);

	EbEditmsg(replybuf,PKeyENCbuf,wholelen,0,0x56,sdatalen);
	SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}

/********************************************************************
*�������ƣ�Sgc1120aMasterAuthV
*���ܣ�������ԿЭ������
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aMasterAuthV(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc,SignKeyno;
    INT8U Consultbuf[150];
    INT8U replybuf[150];
    INT16U wholelen,sdatalen;

	SignKeyno = pdata[EBAUDATASTARTSITE+SGCRANDOMLEN+SGCSIGNDATALEN];//ǩ����Կ����

	rc = Sgc1120aGetKeyConsultData(SignKeyno,(pdata+EBAUDATASTARTSITE),Consultbuf);
	//rc = SGC1120aSKeyConsult(INT8U * pdata,INT8U * cdata,INT16U cdatalen,INT8U *signdata,INT16U signdatalen,INT8U keyno)
	if(rc != 0)
	{
	    logSysMsgNoTime("��ȡ��ԿЭ������ʧ��",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	
	memset(replybuf,0,150);
    sdatalen = Consultbuf[1] + (Consultbuf[0]<<8);
    wholelen = sdatalen + 5;//����ͷ�г���
    memcpy(Consultbuf,Consultbuf+2,sdatalen);

	EbEditmsg(replybuf,Consultbuf,wholelen,0,0x59,sdatalen);
	SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}
/********************************************************************
*�������ƣ�Sgc1120aMasterAuthVI
*���ܣ���ԿЭ�̽������
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aMasterAuthVI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    if((pdata[EBAUDATASTARTSITE]==0x90)&&(pdata[EBAUDATASTARTSITE+1]==0x00))
    {
		AuthEndflag = wChanNo;	//��֤���̳ɹ����꣬���ñ�־λ
    }
	return 0;
}

/********************************************************************
*�������ƣ�Sgc1120aPKeyUpload
*���ܣ���Կ����ָ��
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aPKeyUpload(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U NPKeyno,SKeyno,rc,*p;
	INT8U replybuf[150];
	INT8U EnRanbuf[150];
    INT16U wholelen,sdatalen,eblen,slen;

	p = pdata+EBAUDATASTARTSITE;
	NPKeyno = p[0];
	eblen = EBAUDATASTARTSITE+1+SGCRANDOMLEN+(SGCSIGNDATALEN*2);
	SKeyno = pdata[eblen];
	
	slen = 1+SGCRANDOMLEN+SGCSIGNDATALEN;//ǩ��Դ���ݳ���
	//rc = Sgc1120aUploadPKeySignData(SKeyno,(pdata+EBAUDATASTARTSITE),EnRanbuf);
	//rc = SGC1120aSginVerify((pdata+EBAUDATASTARTSITE),slen,(pdata+9+1+8+64),SGCSIGNDATALEN,SKeyno);

	rc = SGC1120aSginVerify(p,slen,(p+slen),SGCSIGNDATALEN,SKeyno);
	if(rc != 0)
	{
	    logSysMsgNoTime("��Կ����ǩ����֤ʧ��",0,0,0,0);
		EbErrCodeSend(0x9004,0x1f,wChanNo);
		return rc;
	}
	
	rc = Sgc1120aUploadPKeyDataToChip(p,EnRanbuf);
	if(rc != 0)
	{
		logSysMsgNoTime("��Կд��ʧ��",0,0,0,0);
		EbErrCodeSend(0x9004,0x1f,wChanNo);
		return rc;
	}
	rc = Sgc1120aGetPKeyAuthData(NPKeyno,(p+1),EnRanbuf);
	if(rc != 0)
	{
		logSysMsgNoTime("�¹�Կ����ʧ��",0,0,0,0);
		EbErrCodeSend(0x9004,0x1f,wChanNo);
		return rc;
	}
	
	memset(replybuf,0,150);
    sdatalen = EnRanbuf[1] + (EnRanbuf[0]<<8);
    wholelen = sdatalen + 5;//����ͷ�г���
    memcpy(EnRanbuf,EnRanbuf+2,sdatalen);

	EbEditmsg(replybuf,EnRanbuf,wholelen,0,0x61,sdatalen);
	SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);
	return rc;
}
/********************************************************************
*�������ƣ�Sgc1120aSymKeyUploadI
*���ܣ��Գ���Կ����ָ��
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aSymKeyUploadI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc;
    INT8U rbuf[10];
    INT8U replybuf[30];
    INT16U wholelen,sdatalen;
	
    rc = Sgc1120aGetRandomData(rbuf);  
	if(rc != 0)
	{
	    logSysMsgNoTime("��ȡ�����ʧ��",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	
    sdatalen = rbuf[1]+(rbuf[0]<<8);
    wholelen = sdatalen + 5;//����ͷ�г���
    
    memcpy(rbuf,(rbuf+2),sdatalen);
	
    EbEditmsg(replybuf,rbuf,wholelen,0,0x64,sdatalen);
    SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}
/********************************************************************
*�������ƣ�Sgc1120aSymKeyUploadII
*���ܣ��Գ���Կ�������ݴ���
*���룺
*�з��ˣ�����
*********************************************************************/
INT16U Sgc1120aSymKeyUploadII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc,SKeyno,*p;
	INT8U resbuf[2];
    INT8U replybuf[30];
    INT16U wholelen,sdatalen,eblen;

	p = pdata+EBAUDATASTARTSITE;
	
	eblen = EBAUDATASTARTSITE+0x84+SGCSIGNDATALEN;
	SKeyno = pdata[eblen];

	rc = SGC1120aSginVerify(p,0x84,(p+0x84),SGCSIGNDATALEN,SKeyno);
	if(rc != 0)
	{
	    logSysMsgNoTime("�Գ���Կ������ǩʧ��",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	rc = Sgc1120aUploadSymKeyDataToChip(p,replybuf);
	if(rc != 0)
	{
	    logSysMsgNoTime("�Գ���Կ����д��ʧ��",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
	}
	else
	{
        resbuf[0] = 0x90;	
        resbuf[1] = 0x00;	
		
		EbEditmsg(replybuf,resbuf,7, 0,0x66,2);
		SendAuthDataToMISI(replybuf,7 + 6,wChanNo);
	}
	
	return rc;
}
/********************************************************************
*�������ƣ�Pack101msgtoEb
*���ܣ���101����ת��ΪEB��ȫ����
*���룺buf�����ģ�SEBtaillen:��ȫ���ݻ�������len���ĳ���
*�з��ˣ�����
*********************************************************************/
INT16U Pack1120a101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo)
{
    INT16U i,rc,tmp;
    INT8U ebbuf[350];
    INT16U templen;
    
    i = 0;
    rc = 0;
    memset(ebbuf,0,350);
    
    if(fixmlen == 0)
    {
        fixmlen = 6;
    }
    
    while(i < len)
    {
       //logSysMsgNoTime("Pack101msgtoEb!",0,0,0,0);
		if((buf[i] == 0xEB)&&(buf[i+3] == 0xEB))
		{
		    templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB���ĳ��� 
		    i += templen;
	        rc = i;
		}
	    ///////////����101����
	    if((buf[i] == 0x10)&&(buf[i+fixmlen-1] == 0x16))
		{
		   PackFra10ToEb(buf+i,fixmlen,ebbuf);
	        
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
           tmp = i + fixmlen;
               
           if( tmp < len)
           {
               memcpy((buf+i+templen) ,buf+tmp,len - tmp);//���������ĺ���
               len = len - fixmlen + templen;
           }
           memcpy((buf+i ),ebbuf,templen);
           i += templen;
           rc += templen;
		}
	    else if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]==buf[i+2]))////////�Ƕ���101����
		{

		   templen = (INT16U)buf[i+1]&0x00FF;
           templen = templen + 6;//101���ĳ���
           //buf[i+templen-2] = 0x68;           
           saveRecord(buf+i,templen,TXSAVEMODE,1);
           PackFra68ToEb(buf+i ,(INT8U)templen,ebbuf,wChanNo);
           
           tmp = i + templen;
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
               
           if(tmp < len)
           {
               memcpy(buf+i+templen ,buf+tmp,len - tmp);
               len = len - tmp + i + templen;
           }
           
           memcpy(buf+i,ebbuf,templen);
               
           i += templen;//
           rc += templen;
               
	 	}
        else
        {
            i++;
        }
    }
    if(rc > 0)//�÷��͵����ĳ���
    {
        (*SEBtaillen) = i;//���ܺ󻺳����ܳ���(�Ѽ��ܱ���+ʣ��δ��������)
    }
    return rc;
}

/********************************************************************
*�������ƣ�PackFra10ToEb
*���ܣ���װ10֡������EB��
*���룺pdata������,len:10���ĳ��ȣ�ebbuf:EB���Ĵ�Ż�����
*�з��ˣ�����
*********************************************************************/

void Pack1120aFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{

    INT8U sum = 0;
    INT16U slen;

    slen = len + 6;//��������2��Ӧ������1��101�����ֽ�1 ��ȫ��չ�������ֽ�2

    ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + EBAUDATASTARTSITE - 1, pdata, len);//101���ĳ����ֽ�ֻ��1��������Ҫ��һ
    ebbuf[EBAUDATASTARTSITE + len ] =0x00;
    ebbuf[EBAUDATASTARTSITE + len + 1] =0x00;
    
    sum = GetEbMsgCheckSum(ebbuf);
    ebbuf[slen +  4] = sum;
    ebbuf[slen +  5] = 0xD7;
	
}

/*------------------------------------------------------------------/
�������ƣ�  SGCKeyConsult()
�������ܣ�  ��ԿЭ��
����˵����  cdata Э�����ݣ���վ�������
                signdata ǩ��  
                keyno ��Կ���
���˵����  0 �ɹ� ���� ʧ��
��ע��      
/------------------------------------------------------------------*/
INT8U SGC1120aSKeyConsult(INT8U * pdata,INT8U * cdata,INT16U cdatalen,INT8U *signdata,INT16U signdatalen,INT8U keyno)
{
    struct HESendCmd_t hecmd;
    int rc;
    
    if((keyno>3) || (cdata==NULL)|| (signdata==NULL))
        return 10;
    hecmd.cla  = 0x80;
    hecmd.ins  = 0x60;
    hecmd.p1   = 0x81+keyno;
    hecmd.p2   = 0x01;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x48;
    
    memcpy(SGCConsultrandom,cdata,cdatalen);
    memcpy(&pHeSendBuf2[0], cdata, cdatalen);
    memcpy(&pHeSendBuf2[cdatalen], signdata, signdatalen);
    
    semTake(sem_qspiid, WAIT_FOREVER);
    myTaskDelay(1);    
    HESendCmd(&hecmd, pHeSendBuf2, (cdatalen+signdatalen));  
    
    myTaskDelay(15);
    rc = SGCReceiveData(pdata,120);
    semGive(sem_qspiid);
    if(rc==0)
    {
        logSysMsgNoTime("Э�̳ɹ�",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("Э��ʧ��.rc=%d",rc,0,0,0);
    }  
    return rc;    
}

/********************************************************************
*�������ƣ�Eb1120aEncpytDataAnaly
*���ܣ��������ũ����վ�·����������ݣ����ܲ�����
*���룺pdata������
*�з��ˣ�����
*********************************************************************/

INT8U Eb1120aEncpytDataAnaly(INT8U* pdata,INT16U lenth,INT8U *dedata,INT16U wChanNo)
{
	INT8U ddatalen,bwlen;
	INT8U *p;
	INT8U sgcbuf[330];
	int rc;
	 
	rc = Sgc1120aDectyData(pdata,sgcbuf,lenth);//���ܻ����������

	if(rc != 0)
	{
		EbErrCodeSend(0x9103,0x1f,wChanNo);//����ʧ��
		return 0;
	}
		
    p = sgcbuf + 2;
	
    ddatalen = sgcbuf[1]+(sgcbuf[0]<<8);//���ı��ĳ���
    saveRecord(sgcbuf,ddatalen+2,RXSAVEMODE,0);
	
	rc = Sgc1120aJudgeWhetherEn((p+2),wChanNo);
	if(rc == 0)
	{
		logSysMsgNoTime("���ı���ʹ�����Ĵ���,���Ϲ�",0,0,0,0);
		EbErrCodeSend(0x9106,0x1f,wChanNo);
        return 0;
	}
	
	rc = Check1120AIllfgalType((p+2),p[0],wChanNo);//P��һ��ֱ����Ӧ�����ͣ��ڶ���ֱ����101���ĳ���
    if(rc != 0)
    {
        EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
        logSysMsgNoTime("ҵ�����ʹ���",0,0,0,0);
        return 0;
    }
	
    switch(p[0])//�ж�ҵ��Ӧ�����ͣ�Ŀǰֻ�õ���00 01 02 03 05 07
    {
	    case 0x00:
			bwlen = p[1];//101���ĳ���
			memcpy(dedata,p+2,bwlen);
			break;
	    case 0x01:
			//bwlen = EbMsgWithSData(p,dedata,ddatalen,wChanNo);
			bwlen = Eb1120aMsgWithSData((p+1),dedata,wChanNo);
			break;
	    case 0x02://�ն˶���վ��ȷ�ϱ��ģ���վ�����·�02
            EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
            bwlen = 0;
			break;
       case 0x03:
			bwlen = Eb1120aMsgWithRandSData(p,dedata,wChanNo);
			break; 
	    case 0x05:
			bwlen = Eb1120aMsgWithTandSData(p,dedata,wChanNo);
			break;			
	    case 0x07:
            bwlen = Eb1120aMsgWithAllData(p,dedata,wChanNo);
			break;
       case 0x08:
            //bwlen = EbMsgUpLoadData(p,dedata,ddatalen,wChanNo);
            EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
            bwlen = 0;
			break;
       default:            
            EbErrCodeSend(0x9101,0x1f,wChanNo);//Ӧ�����ʹ���
            bwlen = 0;
			break;
    }
    return bwlen;
	
}
/********************************************************************
*�������ƣ�Check1120AIllfgalType
*���ܣ�������վ�·��ı���Ӧ�������Ƿ�Ϸ�
*���룺pdata�����ģ�type:Ӧ�����ͣ�wChanNo:��Լ�˿ں�
*�з��ˣ�����
*********************************************************************/

int Check1120AIllfgalType(INT8U *ptada,INT8U type,INT16U wChanNo)
{
    INT8U DncryptTi = 0;
    INT8U DncryptCot = 0;
    INT8U DncryptPI = 0;
    INT8U Filetype = 0;
    
    int rc = 0;
    
    if(wChanNo < 6)//101����
    {
	    DncryptTi = ptada[fixmlen + 1];
	    DncryptCot = ptada[fixmlen + 3]&0x3F;
	    DncryptPI = ptada[fixmlen +9];
        if(DncryptTi == 210)
        {
            Filetype = ptada[fixmlen +10];
        }
    }
    else if(wChanNo > 40)//104����
    {
        DncryptTi = ptada[6];
	    DncryptCot = ptada[8]&0x3F;
	    if(DncryptTi == 203)
	    {
	        DncryptPI = ptada[14];
//�޸Ĳ�����������ʶ��ң�غ����������ǰһ���ֽڣ���Ϊ��ֵ����ֻ�������ֽڣ�
	    }
	    else
	    {
	        DncryptPI = ptada[15];
	    }
        
        if(DncryptTi == 210)
        {
            Filetype = ptada[16];
        }
    }
    switch(DncryptTi)
    {
    	case 45:
    	case 46://ң��TI
			//Ԥ�� SE=1��cot=6
			//ִ�� SE=0��cot=6
			//���� SE=0��cot=8
    	    if(((DncryptPI&0x80) != 0)||(DncryptCot == 8))//ң��ѡ��/����
    	    {
    	    	if(type != 0x05)
    	    	{
    	    	    rc = 1;
    	        }
    	    }
    	    else//ң��ִ����վ�·���ң��ִ��PI = 0
    	    {
    	    	if(type != 0x07)
    	    	{
    	    	    rc = 1;
    	        }    	    	
    	    }
    	    break;           
    	case 210://д�ļ�����
    	    if((DncryptCot == 6)&&(Filetype == 0x07))//
    	    {
  	    	 if(type != 0x01)
    	    	 {
    	    	    rc = 1;
    	        }      	    	
    	    }
    	    break;    	
    	case 203:  
			//Ԥ�� SE=1��CR=0��cot=6
			//ִ�� SE=0��CR=0��cot=6
			//���� SE=0��CR=1��cot=8
    	    if((DncryptPI&0x80) != 0)//����Ԥ��(��ֹ�Ļ���0x40)
    	    {
  	    	    if(type != 0x01)
    	    	{
    	    	    rc = 1;
    	        }
                rmparaflag = 1;
                logSysMsgNoTime("����Ԥ��",0,0,0,0);         
    	    }
    	    else if((DncryptCot == 0x06)&&((DncryptPI&0x40) == 0))//�����̻�
    	    {
  	    	    if(type != 0x03)
    	    	{
    	    	    rc = 1;
    	        }  
                rmparaflag = 2;
                
                logSysMsgNoTime("�����̻�",0,0,0,0);         
    	    }
            else if((DncryptPI&0x40) != 0)
            {
               //if((rmparaflag == 1)&&(type != 0x01))//����Ԥ��ȡ��
               // �ֳ��ķ���վ�·���0x3,��ʱ�޸�
               if((rmparaflag == 1)&&(type != 0x03))//����Ԥ��ȡ�� 
               {
                   rc = 1;
                   logSysMsgNoTime("����Ԥ��ȡ��",0,0,0,0);         
               }
               else if((rmparaflag == 2)&&(type != 0x03))//�����̻�ȡ��
               {
                   rc = 1;
                   logSysMsgNoTime("�����̻�ȡ��",0,0,0,0);         
               }
               rmparaflag = 0;
           }
    	   break;
        case 211:
    	    if(((DncryptPI&0x80) != 0)&&(DncryptCot == 6))//��������
    	    {
    	    	if(type != 0x01)
    	    	{
    	    	    rc = 1;
    	        }
    	    }
    	    break;            
    	default:
    	    break;
    } 
    return rc;   
}
/********************************************************************
*�������ƣ�Sgc1120aJudgeWhetherEn
*���ܣ��жϸñ����Ƿ�Ӧ�ü���
*���룺Pdata:���ݴ�Ż�����
*�����rc = 1;������Ҫ���ܣ�rc = 0;����Ҫ����
*�з��ˣ�����
*********************************************************************/
INT8U Sgc1120aJudgeWhetherEn(INT8U *pdata,INT16U wChanNo)
{
    INT8U DncryptTi = 0;
    INT8U DncryptCot = 0;
    INT8U DncryptPI = 0;
    INT8U Filetype = 0;
    INT8U rc = 0;

	if(pdata == NULL)
	{
	    return 0;
	}
    if(wChanNo < 6)//101����
    {
	    DncryptTi = pdata[fixmlen + 1];
	    DncryptCot = pdata[fixmlen + 3]&0x3F;
	    DncryptPI = pdata[fixmlen +9];
           if(DncryptTi == 210)
           {
               Filetype = pdata[fixmlen +10];
           }
    }
    else if(wChanNo > 40)//104����
    {
        DncryptTi = pdata[6];
	    DncryptCot = pdata[8]&0x3F;
	    if(DncryptTi == 203)
	    {
	        DncryptPI = pdata[14];
           //�޸Ĳ�����������ʶ��ң�غ����������ǰһ���ֽڣ���Ϊ��ֵ����ֻ�������ֽڣ�
	    }
	    else
	    {
	        DncryptPI = pdata[15];
	    }
           if(DncryptTi == 210)
           {
               Filetype = pdata[16];
           }
    }
    
    switch(DncryptTi)
    {
        case 45:
    	case 46://ң��TI
    	    rc = 1;
            break;
        case 203:
            rc = 1;
            break;
        case 210:
    	     if((DncryptCot == 6)&&(Filetype == 0x07))//д�ļ������ֳ�Ϊ�����·�
    	     {
                 rc = 1;//ZHANGLIANG  20180211
    	     }
            break;
        case 211:
            if((DncryptCot == 6)&&((DncryptPI&0x80) != 0))//��������
            {
                HnnwInf.UpgradeFlag = 1;
                rc = 1;
            }
            else if((DncryptCot == 7)&&((DncryptPI&0x80) != 0))
            {
                if(HnnwInf.UpgradeFlag == 1)
                {
                    rc = 1;
                }
                else
                {
                    rc = 0;
                }
            }
            break;  
        default:
            rc = 0;
            break;          
    }
    
    return rc;
}

/********************************************************************
*�������ƣ�Eb1120aMsgWithSData
*���ܣ�1120a����ֻ��ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/
INT8U Eb1120aMsgWithSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo)
{
    INT8U KeyId,rc;
    INT16U tmplen,extlen,seek;
    INT8U verbuf[64];

	tmplen = pdata[0];//101/104���ĳ���
	extlen = pdata[tmplen+2]+(pdata[tmplen+1]<<8)-1;//ǩ������

	seek = tmplen+1+extlen+2;//��������������(1�ֽڳ���+ԭʼ����+2�ֽڳ���+��ȫ��չ������)
	KeyId = pdata[seek];
	
	seek = 1;//һ���ֽ�ԭʼ���ĳ���
	seek += tmplen;
	seek += 2;
	memcpy(verbuf,pdata+seek,extlen);
	
	rc = SGC1120aSginVerify((pdata+1),tmplen, verbuf, extlen, KeyId);

	if(rc == 0)
	{
	    memcpy(decpbuf,(pdata+1),tmplen);
	}
	else
	{
       EbErrCodeSend(0x9102,0x1f,wChanNo);//
       logSysMsgNoTime("������֤ǩ������:",0,0,0,0);
       return 0;
	}
	
	return tmplen;
}

/********************************************************************
*�������ƣ�Eb1120aMsgUpLoadData
*���ܣ�1120a�����ʱ��������Լ�ǩ������������֤����
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/
INT8U Eb1120aMsgUpLoadData(INT8U *pdata)
{
    INT8U rc;
    INT16U tmplen,extlen,seek;
    struct SysTime_t Time;
	
    if(pdata==NULL)
    {
        return 0;
    }
	tmplen = pdata[1]+(pdata[0]<<8);
    seek = 2;
        
	memcpy(UpLoadtime,pdata+seek,6);
	seek += 6;
	memcpy(UpLoadRdata,pdata+seek,8);
	seek += 8;
	memcpy(UpLoadSdata,pdata+seek,64);
	seek += 64;
    UpLoadKeyId = pdata[seek];
	
	return 0;//���� 
}

/********************************************************************
*�������ƣ�EbMsgWithRandSData
*���ܣ������������Լ�ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/
INT8U Eb1120aMsgWithRandSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo)
{

    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U randbuf[8];
    INT8U verbuf[72];

	seek = 2;
	tmplen = pdata[1];//101��Ч���ݳ���
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8)-1;//��չ���������ȼ�һ���ֽ���������

	seek += tmplen;
	seek += 2;
	memcpy(randbuf,pdata+seek,8);
    if(memcmp(randbuf,SgcSelfRandbuf,8) != 0)
    {
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        return 0;
    }
	memcpy(verbuf,pdata+seek,extlen);//�������ǩ��ֵ
    seek += extlen;
	KeyId = pdata[seek];//ǩ����Կ����
	
	rc = SGC1120aSginVerify((pdata+2),tmplen, verbuf, extlen, KeyId);
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("������֤ǩ������:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*�������ƣ�Eb1120aMsgWithTandSData
*���ܣ�1120a�����ʱ���Լ�ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/
INT8U Eb1120aMsgWithTandSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo)
{
    INT8U KeyId,rc;
    INT16U tmplen,extlen,seek;
    INT8U timebuf[6];
    INT8U verbuf[70];

	seek = 2;
	//KeyId = pdata[len - 1];
	tmplen = pdata[1];//101��Ч���ݳ���
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8)-1;//��չ���������ȼ�һ���ֽڵ���������
	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);

    if(CheckTimeAging(timebuf) != 0)
    {
        EbErrCodeSend(0x9105,0x1f,wChanNo);//
        logSysMsgNoTime("����ʱ���У�����:",0,0,0,0);
        return 0;
    }
	
	memcpy(verbuf,pdata+seek,extlen);//
    seek += extlen;
	KeyId = pdata[seek];//ǩ����Կ����
	
	rc = SGC1120aSginVerify((pdata+2),tmplen, verbuf,extlen, KeyId);
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("������֤ǩ������:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}
/********************************************************************
*�������ƣ�EbMsgWithAllData
*���ܣ������ʱ��������Լ�ǩ���Ľ��ܱ���
*���룺pdata������decpbuf:�������ݴ��
*�з��ˣ�����
*********************************************************************/
INT8U Eb1120aMsgWithAllData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo)
{
    INT8U KeyId,rc;
    INT16U tmplen,extlen,seek;
    INT8U timebuf[6];
    INT8U randombuf[8];
	//INT8U signdbuf[SGCSIGNDATALEN];
    INT8U verbuf[255];
    struct SysTime_t Time;
    
	seek = 2;
	//KeyId = pdata[len - 1];
	tmplen = pdata[1];//101���ĳ���
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8)-1;//��չ����������

	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);
	memcpy(verbuf,pdata+seek,extlen);//��չ������ƴ����101���ĺ󣬼���ǩ����
	seek += 6;
	memcpy(randombuf,pdata+seek,8);
	seek += 8;
	seek += 64;
	KeyId = pdata[seek];
	
    if(CheckTimeAging(timebuf) != 0)
    {
        EbErrCodeSend(0x9105,0x1f,wChanNo);//
        logSysMsgNoTime("07����ʱ���У�����:",0,0,0,0);
        return 0;
    }
 
    if(memcmp(randombuf,SgcSelfRandbuf,8) != 0)
    {
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        logSysMsgNoTime("07�����У�����:",0,0,0,0);         
        return 0;
    }
	//seek += 8;
	//memcpy(signdbuf,pdata+seek,SGCSIGNDATALEN);
	
	rc = SGC1120aSginVerify((pdata+2),tmplen, verbuf,extlen, KeyId);
    
       //logSysMsgNoTime("%x,%x,%x,%x:",pdata[2],tmplen,extlen,KeyId);       
       //logSysMsgNoTime("%x,%x,%x,%x:",verbuf[0],verbuf[1],verbuf[2],verbuf[3]);       
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("������֤ǩ������:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*�������ƣ�Pack1120aFor101msgtoEb
*���ܣ�1120a��101����ת��ΪEB��ȫ����
*���룺buf�����ģ�SEBtaillen:��ȫ���ݻ�������len���ĳ���
*�з��ˣ�����
*********************************************************************/

INT16U Pack1120aFor101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo)
{
    INT16U i,rc,tmp;
    INT8U ebbuf[350];
    INT16U templen;
    
    i = 0;
    rc = 0;
    memset(ebbuf,0,350);
    
    if(fixmlen == 0)
    {
        fixmlen = 6;
    }
    
    while(i < len)
    {
		if((buf[i] == 0xEB)&&(buf[i+3] == 0xEB))
		{
		    templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB���ĳ��� 
		    i += templen;
	        rc = i;
		}
       ///////////����101����
        if((buf[i] == 0x10)&&(buf[i+fixmlen-1] == 0x16))
	    {
	        PackFra10ToEb(buf+i,fixmlen,ebbuf);
        
            templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
            tmp = i + fixmlen;
                
            if( tmp < len)
            {
                memcpy((buf+i+templen) ,buf+tmp,len - tmp);//���������ĺ���
                len = len - fixmlen + templen;
            }
            memcpy((buf+i ),ebbuf,templen);
            i += templen;
            rc += templen;
	    }
        else if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]==buf[i+2]))////////�Ƕ���101����
	    {

	        templen = (INT16U)buf[i+1]&0x00FF;
            templen = templen + 6;//101���ĳ���
            saveRecord(buf+i,templen,TXSAVEMODE,1);
            Pack1120aFra68ToEb(buf+i,(INT8U)templen,ebbuf,wChanNo);
            
            tmp = i + templen;
            templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
                
            if(tmp < len)
            {
                memcpy(buf+i+templen ,buf+tmp,len - tmp);
                len = len - tmp + i + templen;
            }
            
            memcpy(buf+i,ebbuf,templen);
                
            i += templen;//
            rc += templen;
                
	    }
        else
        {
            i++;
        }
    }
    if(rc > 0)
    {
        (*SEBtaillen) = i;
    }
    return rc;
}
/********************************************************************
*�������ƣ�Pack1120aFra68ToEb
*���ܣ�1120a��װ68֡������EB��
*���룺pdata������,len:68���ĳ��ȣ�ebbuf:EB���Ĵ�Ż�����
*�з��ˣ�����
*********************************************************************/
void Pack1120aFra68ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo)
{

    INT8U sum,ensureflag,rc,rcI,*p;
    INT8U DncryptTi = 0;
    INT8U DncryptCot = 0;
    INT8U DncryptPI = 0;
    INT16U eblen;
    INT8U randbuf[10];
    
    ebbuf[0] = ebbuf[3] = 0xEB;
    ensureflag = 0;
	p = (ebbuf+6);
		
    DncryptTi = pdata[fixmlen + 1]; 
    DncryptCot = pdata[fixmlen + 3]&0x3F;
    DncryptPI = pdata[fixmlen +9];
	
    if(DncryptTi == 203)
    {
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//����Ԥ��ȷ��
        {
            ensureflag = 1;
        }
    }
    else if((DncryptTi == 46)||(DncryptTi == 0X2D))
    {
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//ң��Ԥ��ȷ��
        {
            ensureflag = 1;
        }
    }
    else if(DncryptTi == 211)
    {
   
  	    if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//
  	    {
  	        if(HnnwInf.UpgradeFlag == 1)//��������ȷ��
  	        {
				ensureflag = 1;
				HnnwInf.UpgradeFlag = 0;
  	        }
			else//��������ȷ��
			{
				ensureflag = 0;
				Upendflag = 2;
			}
  	    }
    }
    
    memcpy(p+2,pdata,len);
    
    if(ensureflag)//Ӧ������Ϊ02
    {
        rc = Sgc1120aGetRandomData(randbuf);
        if(rc != 0)
        {
            memset(ebbuf,0,len);
            EbErrCodeSend(0x9109,0x1f,wChanNo);
            return ;
        }
        memcpy(SgcSelfRandbuf,(randbuf+2),8);   
		
        p[0] = 0x02;
        p[1] = len;
        p[len + 2] = 0x00;
        p[len + 3] = 0x08;
        
        memcpy(p+len + 4,SgcSelfRandbuf,8);     

		rc = Sgc1120aEnctyData(p,(len+12));
    }
    else
    {
		p[0] = 0x00;
		p[1] = len;  
		p[len + 2] = 0x00;
		p[len + 3] = 0x00;
        rcI = Sgc1120aJudgeWhetherEn(pdata,wChanNo);
		if(rcI == 1)//��Ҫ����
		{
			rc = Sgc1120aEnctyData(p,(len+4));
		}
    }
	
    if(rcI == 0)//�������
    {
        eblen = len + 6;
        ebbuf[1] = HIBYTE(eblen);
        ebbuf[2] = LOBYTE(eblen); 
        ebbuf[4] = 0x00;   
        ebbuf[5] = 0x00;
        memcpy(ebbuf+6,p,(len+4));
    }
    else if(rc == 0)
    {
        eblen = p[1]+(p[0]<<8) + 2;
        ebbuf[1] = HIBYTE(eblen);
        ebbuf[2] = LOBYTE(eblen);   
            
        ebbuf[4] = 0x00;   
        ebbuf[5] = 0x08;//��������
        memcpy(ebbuf+6,p+2,eblen - 2);
    }
    else
    {
        memset(ebbuf,0,len);
        EbErrCodeSend(0x9109,0x1f,wChanNo);
        return ;
    }

    sum = GetEbMsgCheckSum(ebbuf);
    
    ebbuf[eblen +  4] = sum;
    ebbuf[eblen +  5] = 0xD7;

}
/********************************************************************
*�������ƣ�Pack1120afor104msgtoEb
*���ܣ���104����ת��ΪEB��ȫ����
*���룺buf�����ģ�SEBtaillen:��ȫ���ݻ�������len���ĳ���
*�з��ˣ�����
*********************************************************************/

INT16U Pack1120afor104msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo)
{
    INT16U i,templen,rc;
    INT8U ebbuf[355];
    INT8U len104 = 0;
    INT8U tmp = 0;
    
    i = 0;
    rc = 0;
    memset(ebbuf,0,355);
    
	while(i < len)
	{
	    if((buf[i] == 0xEB)&&(buf[i+3] == 0xEB))
	    {
	        templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB���ĳ��� 
	        i += templen;
               rc = i;
	    }
        
           if(buf[i] == 0x68)
           {
               if(buf[i+1] != 0x04)
               {
                   len104 = buf[i+1] + 2;
                   saveRecord(buf+i,len104,TXSAVEMODE,1);
                   Pack1120aFra104ToEb(buf+i ,len104,ebbuf,wChanNo);

                   tmp = i + len104;
                   
                   templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ���    
                   if(tmp < len)
                   {
                       memcpy(buf+i+templen ,buf+tmp,len - tmp);
                       len = len -len104 + templen;
                   }    
                   memcpy(buf+i,ebbuf,templen);
                   i += templen;
                   rc += templen;
               }
               else
               {
                   PackFixed104ToEb(buf+i,6,ebbuf);
                   tmp = i + 6;
                   templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB���ĳ��� 
                   if(tmp < len)
                   {
                       memcpy(buf+i+templen ,buf+tmp,len - tmp);
                       len = len -6 + templen;
                   }                     
                   memcpy(buf+i,ebbuf,templen);
                   rc += templen;
	            i += templen;
               }
           }
           else
           {
               i++;
           }
	}
       if(rc > 0)
       {
           (*SEBtaillen) = i;
       }
       return rc;
}
/********************************************************************
*�������ƣ�Pack1120aFra104ToEb
*���ܣ���װ104������EB��
*���룺pdata������,len:68���ĳ��ȣ�ebbuf:EB���Ĵ�Ż�����
*�з��ˣ�����
*********************************************************************/

void Pack1120aFra104ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo)
{

    INT8U sum,ensureflag,rc,rcI,*p;
    INT8U DncryptTi = 0;
    INT8U DncryptCot = 0;
    INT8U DncryptPI = 0;
    INT16U eblen = 0;
    //INT8U tsgcbuf[355];
    INT8U randbuf[10];

	
    ebbuf[0] = ebbuf[3] = 0xEB;
    ensureflag = 0;
    p = (ebbuf+6);
	
    DncryptTi = pdata[6];
    DncryptCot = pdata[8]&0x3F;;
	
    if(pdata[6] == 203)
    {
        DncryptPI = pdata[14];
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//����Ԥ��ȷ��
        {
            ensureflag = 1;
        }
    }
    else if((pdata[6] == 46)||(pdata[6] == 0X2D))
    {
	 DncryptPI = pdata[15];
        //ensureflag = pdata[15];
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//ң��Ԥ��ȷ��
        {
            ensureflag = 1;
        }
    }
    else if(pdata[6] == 211)
    {
           DncryptPI = pdata[15];
		
	    if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//
  	    {
  	        if(HnnwInf.UpgradeFlag == 1)//��������ȷ��
  	        {
		      ensureflag = 1;
		      HnnwInf.UpgradeFlag = 0;
  	        }
		 else//��������ȷ��
		 {
		     ensureflag = 0;
		     Upendflag = 2;
		 }
  	    }
        //if((DncryptCot == 7)&&((ensureflag & 0x80) == 0))//��������ȷ��
        //{
            //Upendflag = 1;
        //}
    }


    memcpy(p+2,pdata,len);
    
    if(ensureflag)//Ӧ������Ϊ02
    {
        rc = Sgc1120aGetRandomData(randbuf);
        
        if(rc != 0)
        {
            memset(ebbuf,0,eblen+3);
            EbErrCodeSend(0x9109,0x1f,wChanNo);
            return ;
        }
		
		memcpy(SgcSelfRandbuf,(randbuf+2),8); 
		
        p[0] = 0x02;
        p[1] = len;        
        p[len + 2] = 0x00;
        p[len + 3] = 0x08;
        
        memcpy(p+len+4,SgcSelfRandbuf,8);     
        rc = Sgc1120aEnctyData(p,(len+12));
        
    }
    else
    {
        p[0] = 0x00;
        p[1] = len;   
        p[len + 2] = 0x00;
        p[len + 3] = 0x00;
		
        rcI = Sgc1120aJudgeWhetherEn(pdata,wChanNo);
  	 if(rcI == 1)
  	 {
  		rc = Sgc1120aEnctyData(p,len+4);
  	 }
    }
    
    if(rcI == 0)//�������
    {
        eblen = len + 6;
        ebbuf[1] = HIBYTE(eblen);
        ebbuf[2] = LOBYTE(eblen);   
            
        ebbuf[4] = 0x00;   
        ebbuf[5] = 0x00;
        p[0] = 0x00;
        p[1] = len;
        //p[len + 2] = 0x00;
        //p[len + 3] = 0x00;
        memcpy(ebbuf+6,p,(len+4));

    }
    else if(rc == 0)
    {
        eblen = p[1]+(p[0]<<8) + 2;
        ebbuf[1] = HIBYTE(eblen);
        ebbuf[2] = LOBYTE(eblen);   
            
        ebbuf[4] = 0x00;   
        ebbuf[5] = 0x08;//��������
        memcpy(ebbuf+6,p+2,eblen - 2);
    }
    else
    {
        memset(ebbuf,0,len);
        EbErrCodeSend(0x9109,0x1f,wChanNo);
        return ;
    }
        //tsgcbuf:���ֽ�68���ĳ���+68����+���ֽ����������+�����

    sum = GetEbMsgCheckSum(ebbuf);
    
    ebbuf[eblen +  4] = sum;
    ebbuf[eblen +  5] = 0xD7;

}

/********************************************************************
*�������ƣ�SGC1120aVerifyUpLoadData
*���ܣ�1120a��֤��������֤����
*���룺pdata��
*�з��ˣ�����
*********************************************************************/

INT8U SGC1120aVerifyUpLoadData(INT16U wChanNo)
{
    INT8U rc;
    INT8U verbuf[30];
    INT16U len;
    
    if(CheckTimeAging(UpLoadtime) != 0)
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9105,0x1f,wChanNo);//
        logSysMsgNoTime("08����ʱ���У�����:",0,0,0,0);
        return 0;
    }
    
    if(memcmp(UpLoadRdata,SgcSelfRandbuf,8) != 0)
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        logSysMsgNoTime("08�����У�����:",0,0,0,0);         
        return 0;
    }
    
    memcpy(verbuf,UoLoadMD5,16);
    memcpy(verbuf+16,UpLoadtime,6);
    memcpy(verbuf+22,UpLoadRdata,8);
    
	rc = SGC1120aSginVerify(verbuf,30,UpLoadSdata,64,UpLoadKeyId);
    if(rc == 0)
    {
       // EbErrCodeSend(0x9000,0x1f,wChanNo);//�ɹ������ش�����
        StartProgramUpdate();
        return 0;
    }
    else
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("08������֤ǩ������:",0,0,0,0);
	 return 0;
    }
    
}
/*------------------------------------------------------------------/
�������ƣ�  Check1120aEncrptchip()
�������ܣ�  ������оƬ1120a�Ƿ������ʹ��
����˵����  CheckType:0��ʾά��������ã�������ʾ�����Լ�
���˵����  0��ʾоƬ����������
��ע��      
/------------------------------------------------------------------*/
INT8U Check1120aEncrptchip(INT8U CheckType)
{
    INT8U rc = 0;
    INT8U i = 0;
    INT8U KeyVersion = 0;
    char msgbuf[50];
    INT8U *p;
    
    p = ParaTempBuf+1024;   
    //myTaskDelay(5);    

    rc = Sgc1120aGetChipKeyVersion(p);   
    while(rc != 0)
    {
        myTaskDelay(20);    
        rc = Sgc1120aGetChipKeyVersion(p);
        i++;
        if(i > 1)
        {
            //logSysMsgNoTime("����оƬ����쳣.rc=%d",rc,0,0,0);
            i = 0;
            
            return 2;
        }
    }
    
    KeyVersion = p[2];
    if(CheckType == 0)
    {
        logSysMsgNoTime("����оƬ1120a��Կ�汾��:%d",KeyVersion,0,0,0);
    }
    else
    {
        logSysMsgNoTime("����оƬ1120a�Լ�ɹ�����Կ�汾��:%d",KeyVersion,0,0,0);
        return 0;
    }
    myTaskDelay(20);    
    rc = Sgc1120aGetChipSerialNumID(p);
    while(rc != 0)
    {
        
        myTaskDelay(2);    
        rc = Sgc1120aGetChipSerialNumID(p);
        i++;
        if(i > 1)
        {
            //logSysMsgNoTime("����оƬ����쳣.rc=%d",rc,0,0,0);
            i = 0;
            
           return 1;
        }
    }
    
    //if(CheckType == 1)
    {
        memset(msgbuf,0,50);
        sprintf(msgbuf,"����оƬ1120a���к�:%02x%02x%2x%02x%02x%02x%02x%02x\r\n",p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
        logSysMsgNoTime(msgbuf,0,0,0,0);
    }
    myTaskDelay(20);    
    rc = Sgc1120aGetRandomData(p);
   // if(rc  == 0)
    //{
        //memset(msgbuf,0,50);
        //sprintf(msgbuf,"����оƬ�����:%02x%02x%2x%02x%02x%02x%02x%02x\r\n",p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
        //logSysMsgNoTime(msgbuf,0,0,0,0);
    //}
    //else
    //{
        //logSysMsgNoTime("�������ȡʧ��",0,0,0,0);
    //}
    return rc;
}

/*------------------------------------------------------------------/
�������ƣ�  EncrptyChiptest(INT8U type)
�������ܣ�  ��ⰲȫ����оƬ����
����˵����  
���˵���� 1:1161,2:1120a,0:�޼���оƬ
��ע��      
/------------------------------------------------------------------*/
INT8U EncrptyChiptest(INT8U type)
{
    INT8U rc = 0;

	rc = CheckEncrptchip(type); 

	if(rc != 0)
	{
           myTaskDelay(30);    
	    rc = Check1120aEncrptchip(type);
	}
	else
	{
	    ////logSysMsgNoTime("����оƬ�ͺ�1161",0,0,0,0);
	    return 1;
	}
	if(rc == 0)
	{
		////logSysMsgNoTime("����оƬ�ͺ�1120a",0,0,0,0);
	    return 2;
	}
	logSysMsgNoTime("����оƬ������",0,0,0,0);
	return 0;
}

/*------------------------------------------------------------------/
�������ƣ�  Packf68ToOld1120aEn(INT8U type)
�������ܣ�  ��װ�����ϼ��ܱ���
����˵����  
���˵���� 
��ע��      
/------------------------------------------------------------------*/
void Packf68ToOld1120aEn(INT8U *oribuf)
{
    INT8U tilocation,tivalue,len,enlen,rc;
    INT8U buf[255];
	INT8U tmpbuf[255];
	tilocation = fixmlen +1; 
	tivalue = oribuf[tilocation];
	len = oribuf[1]-3;
       ////g_EnDone = 1;
       buf[0] = len;//��Ҫ�����������ݳ���д������
	memcpy((buf+1),oribuf+7,len);
	switch(tivalue)//ң�ţ�ң�⣬ң�أ���Ϣ�ϱ���Ҫ����
	{
		case 0x01:
		case 0x03:
		case 0x1E:
		case 0x1F:
		case 0x09:
		case 0x0B:
		case 0x0D:
                    saveRecord(oribuf,(len+9),TXSAVEMODE,1);
			rc = Sgc1120aEnctyData(buf,(len+1));
			if(rc == 0)
			{
				oribuf[0]=oribuf[3]=0x69;
				enlen = (buf[0]<<8)+buf[1];//���ܺ�����+MAC����

				oribuf[1]=oribuf[2]=enlen+3;
				memcpy((oribuf+7),(buf+2),enlen);
			}
			else
			{
                           myTaskDelay(20);
                           logSysMsgNoTime("����ʧ��I!rc==%d",rc,0,0,0);
                           memcpy((buf+1),oribuf+7,len);
                           rc = Sgc1120aEnctyData(buf,(len+1));
                           if(rc == 0)
                           {
                               oribuf[0]=oribuf[3]=0x69;
                               enlen = (buf[0]<<8)+buf[1];//���ܺ�����+MAC����
                       
                               oribuf[1]=oribuf[2]=enlen+3;
                               memcpy((oribuf+7),(buf+2),enlen);
                           }
                           else
                           {
                               myTaskDelay(20);
                               logSysMsgNoTime("����ʧ��II!rc==%d",rc,0,0,0);
                               memcpy((buf+1),oribuf+7,len);
                               rc = Sgc1120aEnctyData(buf,(len+1));
                               if(rc == 0)
                               {
                                   oribuf[0]=oribuf[3]=0x69;
                                   enlen = (buf[0]<<8)+buf[1];//���ܺ�����+MAC����
                           
                                   oribuf[1]=oribuf[2]=enlen+3;
                                   memcpy((oribuf+7),(buf+2),enlen);
                               }                       
                           }
                           
                           if(rc != 0)
                           {
                               logSysMsgNoTime("����ʧ��III!rc==%d",rc,0,0,0);
                               oribuf[0]=oribuf[3]=0;
                               oribuf[1]=oribuf[2]=0;
    				    ////oribuf[1]=oribuf[2]=12;
    				    ////oribuf[7] = 0xFA;
    				    ////oribuf[8] = 0x01;
    				    ////oribuf[9] = 0x07;
    				    ////oribuf[12] = 0x00;
    				    ////oribuf[13] = 0x00;
    				    ////oribuf[14] = 0x02;
    				    ////oribuf[15] = 0x00;
                           }
			}

			break;
		default:
			break;
	}
	////g_EnDone = 0;
}

/*------------------------------------------------------------------/
�������ƣ�  SGCOldPkeyUpdate()
�������ܣ�  ��Կ����
����˵����  pdata:��վ��������
            sdata ��Կ���ܺ�õ�������

���˵����  0 �ɹ� ���� ʧ�� -10 ��������
��ע��      
/------------------------------------------------------------------*/
int SGCOldPkeyUpdate(INT8U *pdata,INT8U *sdata)
{
    int rc;
    int loc;
    int signkeyno;
    int newkeyno;
    
    INT8U signdata[64];
    INT8U keydata[64];
    INT8U random[8];
	INT8U tmp[80];
    //INT8U encryptdata[68];
    
    loc = 0;
    signkeyno = (int)*(pdata);
    if(signkeyno >= 0x81)
    {
        signkeyno = signkeyno - 0x81;
    }
    loc++;
    newkeyno = (int)*(pdata + loc);
    if(newkeyno >= 0x81)
    {
        newkeyno = newkeyno - 0x81;
    }
    loc++;
    memcpy(keydata,pdata+loc,64);
    loc = loc+64;
    memcpy(signdata,pdata+loc,64);
    loc = loc+64;
    memcpy(random,pdata+loc,8);
                 
	rc = SGC1120aSginVerify(keydata,64,signdata,SGCSIGNDATALEN,signkeyno);

	if(rc != 0)
    {
    	return -1;
    }
    //rc = SGCLoadOneKey(keydata,newkeyno);
    tmp[0] = newkeyno;
	memcpy((tmp+9),keydata,SGCSIGNDATALEN);
	rc = Sgc1120aUploadPKeyDataToChip(tmp,sdata);
    if(rc != 0)
    {
        logSysMsgNoTime("��Կд�����",0,0,0,0);  	
    	return -2;
    }
    //memset(encryptdata,0,68);
    ////rc = SGCPKeyEncrypt(sdata,random,8,newkeyno);
	rc = Sgc1120aGetPKeyAuthData(newkeyno,random,sdata);
    if(rc != 0)
    {
        logSysMsgNoTime("��Կ����ʧ��",0,0,0,0);  	
    	return -3;
    }
    return rc;

}

/*------------------------------------------------------------------/
�������ƣ�  SGCOldSymkeyUpdate()
�������ܣ�  �Գ���Կ����
����˵����  pdata:��վ��������

���˵����  0 �ɹ� ���� ʧ�� -10 ��������
��ע��      
/------------------------------------------------------------------*/
int SGCOldSymkeyUpdate(INT8U *pdata)
{
	int rc;
	int loc;
	int keyno;
	INT8U ekeydata[132];
	INT8U signdata[64];
	
	memset(ekeydata,0,132);
	memset(signdata,0,64);
	loc = 0;
	keyno = (int)*(pdata);
	loc++;
    if(keyno >= 0x80)
    {
        keyno = keyno - 0x80;
    }
    memcpy(ekeydata,pdata+loc,132);//�����Գ���Կ�������ݼ�MACУ������
    loc = loc + 132;
    memcpy(signdata,pdata+loc,64);//ǩ��
    	    
	//rc = SGCSginVerify(ekeydata,132,signdata,64,keyno);
	rc = SGC1120aSginVerify(ekeydata,0x84,signdata,SGCSIGNDATALEN,keyno);
	if(rc != 0)
    {
    	return -1;
    }
    //rc = SGCLoadSymKey(ekeydata);
	rc = Sgc1120aUploadSymKeyDataToChip(ekeydata,signdata);
    if(rc != 0)
    {
    	logSysMsgNoTime("�Գ���Կд��ʧ��",0,0,0,0);  	
    	return -2;
    }
    return rc;
}

