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

#define EBKEYIDLEN   1      //密钥索引字节数
#define EBHEADLEN   4
#define EBAUDATASTARTSITE   9   //认证过程报文数据起始位置

extern  SEM_ID  sem_qspiid;

extern INT8U *pHeSendBuf1,*pHeSendBuf2;
extern INT8U *ParaTempBuf;
extern struct encrypt_st  encrypt_data;//用于判断时间戳超时


INT8U SgcSelfRandbuf[8];//芯片自生成随机数存放
INT8U SgcCerToolIDbuf[8];//证书管理工具ID存放
INT8U SgcGetWayRandbuf[8];//网关下发随机数
INT8U SgcMasterRandbuf[16];//主站认证时下发随机数 R1+R1按位取反
INT8U SgcRdDatatoYWbuf[16];//证书管理工具认证随机数R+R取反

INT8U UpLoadtime[6];//升级验证时间
INT8U UpLoadRdata[8];//升级验证随机数    
INT8U UpLoadSdata[64];//升级验证签名
INT8U UpLoadKeyId;
INT8U UoLoadMD5[16];

INT8U SGCConsultrandom[8];//密钥协商随机数
//INT8U SGCSymrandom[8];//更新对称密钥随机数
////INT8U g_EnDone = 0;

INT8U rmparaflag = 0;
INT8U CAnum = 0;
INT8U YWCernum = 0;
INT8U Upendflag = 0;


INT8U Sendnum = 0;
INT16U Sendlen = 0;
INT8U SendCount = 0;//已发送了多少帧


INT16U LenFromEbMsg = 0;
INT16U AuthEndflag = 0;
INT16U YWAuthEndflag = 0;

INT16U fixmlen = 0;
INT16U enCAlen = 0;
INT16U YWCerlen = 0;

struct Sgc1120aFlag_t HnnwInf;

/********************************************************************
*函数名称：SgcGetChipSerialNumID
*功能：获取芯片序列号
*输入：rcvbuf:数据存放缓冲区
*研发人：张良
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
        //logSysMsgNoTime("获取芯片序列号正确:",0,0,0,0);
    }
    else
    {
       // logSysMsgNoTime("获取芯片序列号失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   
}

/********************************************************************
*函数名称：Sgc1161GetOriCerlen
*功能：获取初始证书长度
*输入：rcvbuf:数据存放缓冲区
*研发人：张良
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
        //logSysMsgNoTime("获取芯片序列号正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("获取初始证书长度失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   
}

/********************************************************************
*函数名称：Sgc1161GetOriCerData
*功能：获取初始证书
*输入：rcvbuf:数据存放缓冲区
*研发人：张良
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
        //logSysMsgNoTime("获取芯片序列号正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("获取初始证书失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   
}

/********************************************************************
*函数名称：SgcGetChipKeyVersion
*功能：获取芯片密钥版本号
*输入：pdata数据接收缓冲区
*研发人：张良
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
        //logSysMsgNoTime("获取密钥版本号正确:",0,0,0,0);
    }
    else
    {
        //logSysMsgNoTime("获取密钥版本号失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;       
    
}
/********************************************************************
*函数名称：SgcVerify1161MasterSignData
*功能：验证主站签名
*输入：pdata签名数据,KeyId：签名密钥索引
*研发人：张良
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
        //logSysMsgNoTime("验证主站签名正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("验证主站签名错误。rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;         
}
/********************************************************************
*函数名称：SgcGet1161RanSignData
*功能：对随机数生成签名
*输入：pdata:主站随机数
*研发人：张良
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
        //logSysMsgNoTime("随机数签名生成正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("随机数签名生成失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;          
}
/********************************************************************
*函数名称：Sgc1161EncryptData
*功能：数据加密
*输入：pdata:待加密数据,lenth：加密数据长度
*研发人：张良
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
    // //测试现象来看，如果不加这个延时，有可能会被录播写flash影响加密指令的发送   
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
            if(rc == 77)//传输错误，支持重发三次
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
        //logSysMsgNoTime("数据加密正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("数据加密失败!!!.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*函数名称：Sgc1161DecryptData
*功能：数据解密
*输入：pdata:待解密数据,lenth：解密数据长度
*研发人：张良
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
        //logSysMsgNoTime("数据解密失败.rc=%d",rc,0,0,0);
        if(j > 3)
        {
            break;
        }
    }


    
    if(rc == 0)
    {
        //logSysMsgNoTime("数据解密正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("数据解密失败.rc=%d",rc,0,0,0);
    }
    
   // semGive(sem_qspiid);
    
    return rc;        
}



/********************************************************************
*函数名称：Sgc1161VerifySigndata
*功能：验证签名
*输入：pdata签名数据+签名,KeyId：签名密钥索引
*研发人：张良
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
        //logSysMsgNoTime("验证签名正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("验证签名失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;       
}
/********************************************************************
*函数名称：Sgc1161ObtainRandata
*功能：获取随机数
*输入：
*研发人：张良
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
            if(rc == 77)//传输错误，支持重发三次
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
        //logSysMsgNoTime("获取芯片随机数失败!1",0,0,0,0);
        if(j > 3)
        {
            break;
        }
        
    }


    
    if(rc == 0)
    {
        memcpy(SgcSelfRandbuf,buf+2,8);
        //logSysMsgNoTime("获取芯片随机数正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("获取芯片随机数失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;     
}
/********************************************************************
*函数名称：Sgc1161LoadSymmetryKey
*功能：写入对称密钥
*输入：pdata对称密钥+签名,KeyId：签名密钥索引
*研发人：张良
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
        hecmd.p1   = 0x00;//更新密钥
    }
    else
    {
        hecmd.p1   = 0x01;//恢复密钥
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
        //logSysMsgNoTime("对称密钥写入正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("对称密钥写入失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*函数名称：Sgc1161DeCerdata
*功能：解析证书
*输入：pdata：证书,KeyId：对称密钥标识
*研发人：张良
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
        //logSysMsgNoTime("证书解密正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("证书解密失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161DeCerdata
*功能：更新主站，网关，终端证书
*输入：pdata：证书,CerId:证书ID
*研发人：张良
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
       // logSysMsgNoTime("证书写入正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("证书写入失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161LoadTemSelfCerdata
*功能：更新终端证书
*输入：pdata：证书,
*研发人：张良
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
       // logSysMsgNoTime("终端证书写入正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("终端证书写入失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161CheckoutCer
*功能：导出终端芯片证书
*输入：pdata：证书导出缓冲区
*研发人：张良
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
            if(rc == 77)//传输错误，支持重发三次
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
       // logSysMsgNoTime("终端证书写导出正确:",0,0,0,0);
    }
    else
    {
        //logSysMsgNoTime("终端证书导出失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161CheckoutPKey
*功能：导出终端芯片公钥
*输入：pdata公钥导出缓冲区
*研发人：张良
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
            if(rc == 77)//传输错误，支持重发三次
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
       // logSysMsgNoTime("终端证书写导出正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("导出终端芯片公钥失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161VerifyMaintDevCer
*功能：验证运维终端证书
*输入：pdata：证书,
*研发人：张良
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
       // logSysMsgNoTime("运维终端证书正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("运维终端证书错误.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161VerifyMaintDevSigndata
*功能：验证运维终端签名
*输入：pdata：签名
*研发人：张良
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
        //logSysMsgNoTime("运维终端证书正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("运维终端证书错误.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161VfyYWCer
*功能：验证证书管理工具证书有效性
*输入：pdata：签名
*研发人：张良
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
        //logSysMsgNoTime("运维终端证书正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("验证证书管理工具证书有效性错误.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;   

}

/********************************************************************
*函数名称：Sgc1161EncryptPbKey
*功能：公钥加密(对证书管理工具传输密文)
*输入：pdata:证书管理工具ID，R1随机数，初始向量,lenth：加密数据长度
*研发人：张良
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
    hecmd.len1 = HIBYTE(lenth+32);//ID + R1+IVData(R1+R1取反)
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
        //logSysMsgNoTime("数据加密正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("公钥加密失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*函数名称：Sgc1161DecryptYWFileData
*功能：解密证书管理工具证书请求数据
*输入：pdata:lenth：解密数据长度
*研发人：张良
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
    hecmd.len1 = HIBYTE(lenth+32);//ID + R1+IVData(R1+R1取反)
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
        //logSysMsgNoTime("数据解密正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("解密证书管理工具证书请求数据失败.rc=%d",rc,0,0,0);
    }
    
   // semGive(sem_qspiid);
    
    return rc;        
}
/********************************************************************
*函数名称：Sgc1161SignYWData
*功能：对证书管理工具数据签名
*输入：pdata：
*研发人：张良
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
       // logSysMsgNoTime("运维终端证书正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("运维终端证书错误.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}

/********************************************************************
*函数名称：Sgc1161RecoveryKeydata
*功能：证书管理工具恢复密钥
*输入：pdata  
*研发人：张良
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
    hecmd.p1   = 0x02;//恢复密钥
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
        //logSysMsgNoTime("对称密钥写入正确:",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("对称密钥写入失败.rc=%d",rc,0,0,0);
    }
    
    //semGive(sem_qspiid);
    
    return rc;        
}


/********************************************************************
*函数名称：GetEbMegCheckSum
*功能：计算累加和
*输入：pdata：报文
*研发人：张良
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
*函数名称：CheckIllfgalType
*功能：处理主站下发的报文应用类型是否合法
*输入：pdata：报文，type:应用类型，wChanNo:规约端口号
*研发人：张良
*********************************************************************/

int CheckIllfgalType(INT8U *ptada,INT8U type,INT16U wChanNo)
{
    INT8U DncryptTi = 0;
    INT8U DncryptCot = 0;
    INT8U DncryptPI = 0;
    INT8U Filetype = 0;
    
    int rc = 1;
    
    if(wChanNo < 6)//101报文
    {
    	
        /*
    	if(fixmlen == 6)//确定地址域是一个字节还是两个字节
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
    else if(wChanNo > 40)//104报文
    {
           DncryptTi = ptada[6];
	    DncryptCot = ptada[8]&0x3F;
	    if(DncryptTi == 203)
	    {
	        DncryptPI = ptada[14];
//修改参数的特征标识比遥控和软件升级往前一个字节（因为定值区号只有两个字节）
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
    	case 46://遥控TI
    	    if(((DncryptPI&0x80) != 0)||(DncryptCot == 8))//遥控选择/撤销
    	    {
    	    	if(type != 0x05)
    	    	{
    	    	    rc = -1;
    	        }
    	    }
    	    else//遥控执行
    	    {
    	    	if(type != 0x07)
    	    	{
    	    	    rc = -1;
    	        }    	    	
    	    }
    	    break;
    	case 200://切换定值区TI
    	
    	    if(DncryptCot == 6)
    	    {
                if(type != 0x01)
                {
                    rc = -1;
                }  
    	    }
           break;
           
    	case 210://写文件激活

    	    if((DncryptCot == 6)&&(Filetype == 0x07))//
    	    {
  	    	 if(type != 0x01)
    	    	 {
    	    	    rc = -1;
    	        }      	    	
    	    }
    	
    	    break;    	
    	case 203:    	    
    	    if((DncryptPI&0x80) != 0)//参数预置(终止的话是0x40)
    	    {
  	    	if(type != 0x01)
    	    	{
    	    	    rc = -1;
    	       }
              rmparaflag = 1;
    	    }
    	    else if((DncryptCot == 0x06)&&((DncryptPI&0x40) == 0))//参数固化
    	    {
  	    	if(type != 0x03)
    	    	{
    	    	    rc = -1;
    	       }  
              rmparaflag = 2;
    	    }
           else if((DncryptPI&0x40) != 0)
           {
               if((rmparaflag == 1)&&(type != 0x01))//参数预置取消
               {
                   rc = -1;
               }
               else if((rmparaflag == 2)&&(type != 0x03))//参数固化取消
               {
                   rc = -1;
               }
               rmparaflag = 0;
           }
    	    break;
        /*    	
        case 210:
    	    if(DncryptCot == 6)//写文件激活只需要判断cot即可（只有写文件激活是6
，激活确认7，数据传输5）
    	    {
  	    	    if(type != 0x01)
    	    	{
    	    	    rc = -1;
    	        }      	    	
    	    }        
        
        break;
        */
        case 211:
    	    if(((DncryptPI&0x80) != 0)&&(DncryptCot == 6))//升级启动
    	    {
    	    	if(type != 0x01)
    	    	{
    	    	    rc = -1;
    	        }
    	    }
    	    //else//升级执行
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
*函数名称：EbEncpytDataAnaly
*功能：处理主站下发的密文数据，解密并分析
*输入：pdata：报文
*研发人：张良
*********************************************************************/

INT8U EbEncpytDataAnaly(INT8U* pdata,INT16U lenth,INT8U *dedata,INT16U wChanNo)
{
    INT8U ddatalen,bwlen;
    INT8U *p;
    INT8U sgcbuf[330];
    //int typid = 0;
    int rc;
    
    bwlen = Sgc1161DecryptData(pdata,lenth,sgcbuf);//解密获得明文数据
    /*
    if(bwlen != 0)
    {
        myTaskDelay(10);   
        
        bwlen = Sgc1161DecryptData(pdata,lenth,sgcbuf);//解密失败重新解密

        if(bwlen != 0)
        {
            myTaskDelay(10);   
            
            bwlen = Sgc1161DecryptData(pdata,lenth,sgcbuf);//解密失败重新解密
        }
        
        if(bwlen != 0)
        {
            EbErrCodeSend(0x9103,0x1f,wChanNo);//解密失败
        }
    }
    */
    if(bwlen != 0)
    {
        EbErrCodeSend(0x9103,0x1f,wChanNo);//解密失败
    }
    //p = sgcbuf + 4;//sgcbuf中前两个字节是安全芯片返回的数据长度，第三第四个字节是主站组帧时添加的长度?

    p = sgcbuf + 2;

    ddatalen = sgcbuf[1]+(sgcbuf[0]<<8);//明文报文长度
    saveRecord(sgcbuf,ddatalen+2,RXSAVEMODE,0);
    
    rc = CheckIllfgalType(p+2,p[0],wChanNo);
    
    if(rc  < 0)
    {
        EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
        bwlen = 0;
        logSysMsgNoTime("业务类型错误",0,0,0,0);
        return bwlen;
    }


    switch(p[0])//判断业务应用类型，目前只用到了00 01 02 03 05 07
    {
	    case 0x00:
			bwlen = p[1];//101报文长度
			memcpy(dedata,p+2,bwlen);
			break;
	    case 0x01:
			bwlen = EbMsgWithSData(p,dedata,ddatalen,wChanNo);
			break;
	    case 0x02://终端对主站的确认报文，主站不会下发02
                    EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
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
                    EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
                    bwlen = 0;
			break;
    }
    return bwlen;
	
}
/********************************************************************
*函数名称：EbMsgWithAllData
*功能：处理带时间随机数以及签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
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
	tmplen = pdata[1];//101报文长度
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//扩展数据区长度

	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);
	memcpy(verbuf+tmplen,pdata+seek,extlen-1);//扩展区数据拼接至101报文后，计算签名用
	seek += 6;
	memcpy(randombuf,pdata+seek,8);
    
       if(CheckTimeAging(timebuf) != 0)
       {
           EbErrCodeSend(0x9105,0x1f,wChanNo);//
           logSysMsgNoTime("07数据时间戳校验错误:",0,0,0,0);
           return 0;
       }
    
       if(memcmp(randombuf,SgcSelfRandbuf,8) != 0)
       {
           EbErrCodeSend(0x9104,0x1f,wChanNo);//
           logSysMsgNoTime("07随机出校验错误:",0,0,0,0);         
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
           logSysMsgNoTime("07数据验证签名错误:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*函数名称：EbMsgUpLoadData
*功能：处理带时间随机数以及签名的升级包验证报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
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
           logSysMsgNoTime("08数据时间戳校验错误:",0,0,0,0);
           return 0;
       }
    
       if(memcmp(UpLoadRdata,SgcSelfRandbuf,8) != 0)
       {
           EbErrCodeSend(0x9104,0x1f,wChanNo);//
           logSysMsgNoTime("08随机出校验错误错误:",0,0,0,0);         
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
           logSysMsgNoTime("07数据验证签名错误:",0,0,0,0);
	    return 0;
	}
*/	
	return 0;//待定 
}

/********************************************************************
*函数名称：SGCVerifyUpLoadData
*功能：验证升级包验证数据
*输入：pdata：
*研发人：张良
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
        logSysMsgNoTime("08数据时间戳校验错误:",0,0,0,0);
        return 0;
    }
    
    if(memcmp(UpLoadRdata,SgcSelfRandbuf,8) != 0)
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        logSysMsgNoTime("08随机出校验错误:",0,0,0,0);         
        return 0;
    }
    
    memcpy(verbuf,UoLoadMD5,16);
    memcpy(verbuf+16,UpLoadtime,6);
    memcpy(verbuf+22,UpLoadRdata,8);
    memcpy(verbuf+30,UpLoadSdata,64);
    
    rc = Sgc1161VerifySigndata(verbuf,94,UpLoadKeyId);//16+6+8+64
    
    if(rc == 0)
    {
       // EbErrCodeSend(0x9000,0x1f,wChanNo);//成功不返回处理结果
        StartProgramUpdate();
        return 0;
    }
    else
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("08数据验证签名错误:",0,0,0,0);
	 return 0;
    }
    
}

/********************************************************************
*函数名称：SGCVerifyMD5UpLoadData
*功能：获取MD5值并校验时间签名随机数
*输入：pdata：
*研发人：张良
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
*函数名称：EbMsgWithRandSData
*功能：处理带随机数以及签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
*********************************************************************/

INT8U EbMsgWithRandSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
/*
    INT8U rc;
    INT16U tmplen;
	
	tmplen = pdata[1];//101报文长度	
	rc = VerifyMsgWithExt(pdata,len);
*/
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U randbuf[8];
    INT8U verbuf[1024];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//有效数据长度
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//扩展数据区长度

	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(randbuf,pdata+seek,8);
    
       if(memcmp(randbuf,SgcSelfRandbuf,8) != 0)
       {
           EbErrCodeSend(0x9104,0x1f,wChanNo);//
           //logSysMsgNoTime("03随机出校验错误:",0,0,0,0);
          // logSysMsgNoTime("Rand,%x,%x,%x,%x",randbuf[0],randbuf[1],randbuf[2],randbuf[3]); 
           //logSysMsgNoTime("Rand,%x,%x,%x,%x",randbuf[4],randbuf[5],randbuf[6],randbuf[7]); 
           //logSysMsgNoTime("Rand,%x,%x,%x,%x",SgcSelfRandbuf[0],SgcSelfRandbuf[1],SgcSelfRandbuf[2],SgcSelfRandbuf[3]); 
          // logSysMsgNoTime("Rand,%x,%x,%x,%x",SgcSelfRandbuf[4],SgcSelfRandbuf[5],SgcSelfRandbuf[6],SgcSelfRandbuf[7]); 
           
           return 0;
       }

    
	memcpy(verbuf+tmplen,pdata+seek,extlen-1);//扩展区数据拼接至101报文后，计算签名用
  
	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);

	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
	    
           EbErrCodeSend(0x9102,0x1f,wChanNo);//
           logSysMsgNoTime("03数据验证签名错误:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*函数名称：CheckTimeAging
*功能：检查时间戳的时效性
*输入：pdata：时间戳数据
*研发人：张良
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
        
        if(delay > 60)  //目前电科院规定是一分钟，所以写死在程序里
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
*函数名称：EbMsgWithTandSData
*功能：处理带时间以及签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
*********************************************************************/

INT8U EbMsgWithTandSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
/*
    INT8U rc;
    INT16U tmplen;
	
	tmplen = pdata[1];//101报文长度	
	rc = VerifyMsgWithExt(pdata,len);
*/
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U timebuf[6];
    //INT8U timedata[6];
    INT8U verbuf[1024];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//有效数据长度
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//扩展数据区长度

	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);
	memcpy(verbuf+tmplen,pdata+seek,extlen-1);//扩展区数据拼接至101报文后，计算签名用


     if(CheckTimeAging(timebuf) != 0)
     {
         EbErrCodeSend(0x9105,0x1f,wChanNo);//
         logSysMsgNoTime("05数据时间戳校验错误:",0,0,0,0);
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
           logSysMsgNoTime("05数据验证签名错误:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}
#if 0
/********************************************************************
*函数名称：VerifyMsgWithExt
*功能：验证带扩展区数据报文
*输入：pdata：报文
*研发人：张良
*********************************************************************/

INT8U VerifyMsgWithExt(INT8U *pdata,INT16U len )
{
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U verbuf[255];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//101报文长度	
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8);//扩展数据区长度
	
	memcpy(verbuf,pdata+seek,tmplen);
	seek += tmplen;
	seek += 2;
	memcpy(verbuf+tmplen,pdata+seek,extlen - 1);
	
	rc = Sgc1161VerifySigndata(verbuf,tmplen + extlen - 1,KeyId);
	return rc;

}
#endif
/********************************************************************
*函数名称：EbMsgWithSData
*功能：处理只带签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
*********************************************************************/

INT8U EbMsgWithSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo)
{
    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U verbuf[350];

	seek = 2;
	KeyId = pdata[len - 1];
	tmplen = pdata[1];//101/104报文长度
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
           logSysMsgNoTime("01数据验证签名错误:",0,0,0,0);
           return 0;
	}
	return tmplen;
}


//EbSafetySearchFrame(EbMsgRxdBuf,RxdBuf+RxdTail,&RxdTail,RxdTail - TxdHead,wChanNo);

/********************************************************************
*函数名称：EbSafetySearchFrame
*功能：解析收到的EB报文，并将报文转化为101/104帧
*输入：oribuf：EB报文缓冲区,validbuf:101/104报文存放缓冲区，
*validtaillen:RxdTail，len：接收到的EB缓冲区长度,wChanNo:端口号
*研发人：张良
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

            if(checklen > (512 -6))//如果checklen>256会出现 i 不再执行++
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
*函数名称：CheckEbMegSty
*功能：检验是否EB报文
*输入：pdata：报文
*研发人：张良
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
*函数名称：EbmegAnalysis
*功能：解析Eb格式报文
*输入：pdata：报文，rxbuff:101/104报文存放缓冲区，wChanNo:端口号
*研发人：张良
*********************************************************************/
void EbmsgAnalysis(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    

    switch(pdata[5] & 0xF0)
    {
        case 0x80://隔离网关
            EnMsgByGetwayHandle(pdata,rxbuff,wChanNo);           
            break;
        case 0x40://运维终端
            EnMsgByYWTool(pdata,rxbuff,wChanNo); 
            break;
            
        case 0x00://主站
            LenFromEbMsg = EnMsgBymasterHandle(pdata,rxbuff,wChanNo);
            break;        
             
        case 0x0C://备用
            break;        
        default:
            EbErrCodeSend(0x9110,0x1f,wChanNo);
            break;
    }
    
}

/********************************************************************
*函数名称：EbmegAnalysis
*功能：解析Eb格式报文
*输入：pdata：报文，rxbuff:101/104报文存放缓冲区，wChanNo:端口号
*研发人：张良
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
*函数名称：EnMsgByGetwayHandle
*功能：处理网关下发的数据
*输入：pdata：EB报文缓冲区，rxbuff:101/104报文存放缓冲区wChanNo:端口号
*研发人：张良
*********************************************************************/

INT8U EnMsgByGetwayHandle(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    INT16U wEblenth;
	INT8U bwlen = 0;
	
    wEblenth = pdata[2]+(pdata[1]<<8);//报文长度(报文头至校验码)

	switch(pdata[6])
	{
	    case 0x20://网关认证请求报文
	            AuthEndflag = 0;//链接断开后重新认证时需要先将认证标志位清0
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
*函数名称：SgcGetwayauthenStepI
*功能：网关与终端身份认证第一步
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/

INT8U SgcGetwayauthenStepI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,wholelen,sdatalen;
    INT8U signbuf[100];
    INT8U replybuf[120];
    
    //datalen = pdata[2]+(pdata[1]<<8);//应用数据长度
    datalen = (INT16U)pdata[1];
	datalen = (datalen<<8)+(INT16U)pdata[2];
		
    memset(signbuf,0,100);
    memset(replybuf,0,120);
    memcpy(SgcMasterRandbuf,(pdata + 3),datalen);
    
    Sgc1161GetRanSignData(SgcMasterRandbuf,signbuf);

    sdatalen = signbuf[1]+(signbuf[0]<<8) + EBKEYIDLEN;
    wholelen = sdatalen + 5;//EB报文头中长度
    
    memcpy(signbuf,signbuf + 2,sdatalen);
    signbuf[sdatalen-1] = 0x01;//结尾添加密钥索引
    
    EbEditmsg(replybuf,signbuf,wholelen, 0X0080,0x21,sdatalen);
	
	//TxdHead=0;
	//TxdTail = wholelen + 6;
    //memcpy(TxdBuf,replybuf,TxdTail);
	
	//SendDataToMISI();
	SendAuthDataToMISI(replybuf,wholelen + 6,wChanNo);

    return 0;
}
/********************************************************************
*函数名称：SgcGetwayauthenStepI
*功能：网关与终端身份认证第2步
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/

INT8U SgcGetwayauthenStepII(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U Keyno,rc;
    //INT16U msglen;
    INT8U replybuf[30];
    INT8U signbuf[2];
    
    //msglen = pdata[2]+(pdata[1]<<8);
    //Keyno = pdata[msglen + EBHEADLEN - 1];
    
    //logSysMsgNoTime("验证主站签名。",0,0,0,0);
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
*函数名称：EnMsgBymasterHandle
*功能：处理主站下发的数据
*输入：pdata：报文,rxbuff:101/104报文存放缓冲区，wChanNo：端口号
*研发人：张良
*********************************************************************/

INT8U EnMsgBymasterHandle(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    BOOL  Encrptyflag;
    INT16U wEblenth;
    INT8U bwlen = 0;
	
    wEblenth = pdata[2]+(pdata[1]<<8);//报文长度(报文头至校验码)
    //SKeyId = pdata[EBHEADLEN + 1]&0x07;//对称密钥标识
    
    if(pdata[EBHEADLEN + 1]&0x08)//是否加密
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
        
        bwlen = EbEncpytDataAnaly(pdata + 6, wEblenth - 2,rxbuff,wChanNo);//解密获得明文数据
    
            //if(bwlen == 0)//应用类型不明
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
	        case 0x50://主站认证请求报文
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
                    if(wChanNo < 6)//101端口
                    {
                         AuthEndflag = 1;
                    }
                    else if(wChanNo > 40)//104
                    {
                        AuthEndflag = 2;
                    }
                    */
	            break;

               case 0x60://主站远程密钥管理
                   
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("越权更新密钥1",0,0,0,0);
                       EbErrCodeSend(0x9091,0x61,wChanNo);
                       return 0;
                   }
                        
                   SgcKeymanageStepI(pdata,wEblenth,wChanNo);
                
                   break;
               case 0x62:
                
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("越权更新密钥2",0,0,0,0);
                       EbErrCodeSend(0x9091,0x63,wChanNo);
                       return 0;
                   }
                   SgcKeymanageStepII(pdata,wEblenth,wChanNo,0x62);
                   myTaskDelay(3);
                   break;

               case 0x64://主站远程密钥恢复
               
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("越权恢复密钥",0,0,0,0);
                       EbErrCodeSend(0x9092,0x65,wChanNo);
                       return 0;
                   }
                   
                   SgcKeymanageStepII(pdata,wEblenth,wChanNo,0x64);
                   myTaskDelay(3);
                   break;

               case 0x70://主站远程证书管理
               
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("越权下发证书1",0,0,0,0);
                       EbErrCodeSend(0x9097,0x71,wChanNo);
                       return 0;
                   }
                   SgcCAmanageStepI(pdata,wEblenth,wChanNo);
                   myTaskDelay(2);
                   break;

               case 0x72:
                   if(AuthEndflag == 0)
                   {
                       logSysMsgNoTime("越权下发证书2",0,0,0,0);
                       EbErrCodeSend(0x9097,0x73,wChanNo);
                       return 0;
                   }
                   SgcCAmanageStepII(pdata,wEblenth,wChanNo);
                   myTaskDelay(2);
                   break;
                   
              case 0x74: //主站提取终端证书         
                  if(AuthEndflag == 0)
                  {
                      logSysMsgNoTime("越权提取证书",0,0,0,0);
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
               case 0x05://软件升级包以明文+时间+签名的形式下发
                   
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
                   if(wChanNo < 6)//101端口
                   {
                       if(bwlen > fixmlen)//非定长帧使用了明文
                       {
                           EbErrCodeSend(0x9106,0x1f,wChanNo);
                           return 0;
                       }
                   }
                   else if(wChanNo > 40)
                   {
                       if(bwlen > 6)//非定长帧使用了明文
                       {
                           EbErrCodeSend(0x9106,0x1f,wChanNo);
                           return 0;
                       }
                   }
                   
	            memcpy(rxbuff,pdata+8,bwlen);
	            break; 
                
	        default:
             
	            bwlen = pdata[7];
                   if(wChanNo < 6)//101端口
                   {
                       if(bwlen > fixmlen)//非定长帧使用了明文
                       {
                           EbErrCodeSend(0x9106,0x1f,wChanNo);
                           return 0;
                       }
                   }
                   else if(wChanNo > 40)
                   {
                       if(bwlen > 6)//非定长帧使用了明文
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
*函数名称：SgcMasterauthenStepI
*功能：主站与终端身份认证第一步
*输入：pdata：报文
*研发人：张良
*********************************************************************/

INT8U SgcMasterauthenStepI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,wholelen,sdatalen;
    INT8U signbuf[100];
    INT8U replybuf[120];
    INT8U i ;
        
    datalen = pdata[8]+(pdata[7]<<8);//应用数据长度
    
    memset(signbuf,0,100);
    memset(replybuf,0,120);
    memcpy(SgcMasterRandbuf,pdata + EBAUDATASTARTSITE,datalen);

    //Sgc1161GetChipSerialNumID(signbuf);
    for(i = 0;i < SGCRANDOMLEN;i++)
    {
        SgcMasterRandbuf[8 + i] = ~SgcMasterRandbuf[i];//对主站R1按位取反
    }
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[0],SgcMasterRandbuf[1],SgcMasterRandbuf[2],SgcMasterRandbuf[3]);
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[4],SgcMasterRandbuf[5],SgcMasterRandbuf[6],SgcMasterRandbuf[7]);
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[8],SgcMasterRandbuf[9],SgcMasterRandbuf[10],SgcMasterRandbuf[11]);
    //logSysMsgNoTime("Rand:%x,%x,%x,%x",SgcMasterRandbuf[12],SgcMasterRandbuf[13],SgcMasterRandbuf[14],SgcMasterRandbuf[15]);
    Sgc1161GetRanSignData(pdata + EBAUDATASTARTSITE,signbuf);

    sdatalen = signbuf[1]+(signbuf[0]<<8) + EBKEYIDLEN;
    wholelen = sdatalen + 5;//报文头中长度
    
    memcpy(signbuf,signbuf + 2,sdatalen);
    signbuf[sdatalen-1] = 0x01;//结尾添加密钥索引
    
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
    //logSysMsgNoTime("验证主站签名。Keyno=%d",Keyno,0,0,0);
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
*函数名称：SgcMasterauthenStepIII
*功能：获取芯片序列号
*输入：pdata：报文
*研发人：张良
*********************************************************************/

INT8U SgcMasterauthenStepIII(INT8U TypeID,INT16U wChanNo)
{
    INT8U rbuf[50];
    INT8U replybuf[50];
    INT16U wholelen,sdatalen,msgtype;

    Sgc1161GetChipSerialNumID(rbuf);

    sdatalen = rbuf[1]+(rbuf[0]<<8);
    wholelen = sdatalen + 5;//报文头中长度
	
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
*函数名称：SgcKeymanageStepI
*功能：主站与终端密钥更新第一步,获取芯片密钥版本号+随机数
*输入：pdata：报文
*研发人：张良
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
    
    infobuf[0] = KeyVersion;//密钥版本号
    memcpy(infobuf+1,SgcSelfRandbuf,8); //终端随机数   

    EbEditmsg(replybuf,infobuf,9 + 5, 0,0x61,9);
	
    SendAuthDataToMISI(replybuf,14 + 6,wChanNo)	;

    return 0;   
}

/********************************************************************
*函数名称：SgcKeymanageStepII
*功能：主站与终端密钥更新/恢复第二步,执行密钥更新/恢复
*输入：pdata：报文
*研发人：张良
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
        logSysMsgNoTime("密钥更新失败.rc=%d",rc,0,0,0);
	 
    }
    
    EbEditmsg(replybuf,infobuf,7, 0,(typeid+1),2);

    SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

    return 0;   
}

/********************************************************************
*函数名称：SgcCAmanageStepI
*功能：证书管理第一步
*输入：pdata：报文
*研发人：张良
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
    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA密文数据包长度
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
    
    if(CAnum != pdata[EBAUDATASTARTSITE + 1])//还未接收完毕
    {
        return 0;
    }
    
    saveRecord(ParaTempBuf,enCAlen,RXSAVEMODE,0);
    
    rc = Sgc1161DecryptData(ParaTempBuf,enCAlen,signbuf);//解密获得明文数据
    
    if(rc != 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x97;
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

        logSysMsgNoTime("证书解密失败",0,0,0,0);
        enCAlen = 0;
        CAnum = 0;
        return 0;
    }
    
    sdatalen = signbuf[1]+(signbuf[0]<<8);//明文报文长度
  
    p = signbuf + 2;//sgcbuf中前两个字节是安全芯片返回的数据长度
    saveRecord(signbuf,sdatalen+2,RXSAVEMODE,0);
    
    enCAlen = 0;
    CAnum = 0;
    
    CArID = p[0];
    CAlen = sdatalen -1 -6 - 65;//密钥标识，时间信息，签名结果+签名密钥标识
    KeyID = p[sdatalen - 1];
    

    memcpy(timebuf,p+CAlen+1,6);
    
    if(CheckTimeAging(timebuf) != 0)
    {
        signbuf[0] = 0x90;
        signbuf[1] = 0x97;
        EbEditmsg(replybuf,signbuf,7, 0,0x71,2);
        SendAuthDataToMISI(replybuf,7 + 6,wChanNo)	;

        //logSysMsgNoTime("70数据时间戳校验错误:",0,0,0,0);
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
        logSysMsgNoTime("验证主站下发证书签名失败",0,0,0,0);
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
*函数名称：CheckCerDecryptData
*功能：证书密文解密报文检查
*输入：pdata：
*研发人：张良
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
    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA数据包分包长度
    
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
    if(CAnum != pdata[EBAUDATASTARTSITE + 1])//还未接收完毕
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
    
    CAbuf[0] = 0x06;//终端证书标识(与电科院沟通得知标识固定为1)
    CAbuf[1] = msgnum;//证书报文总帧数
    
    for(i = 1;i <= msgnum; i++)
    {
    
        CAbuf[2] = i;//当前帧序号
        
        if(i < msgnum)
        {
            memcpy(CAbuf+3,ParaTempBuf+2+((i - 1) * 200),200);//每帧传输200字节ca数据
            EbEditmsg(sendbuf,CAbuf, 208,0,0x75,203);
            SendAuthDataToMISI(sendbuf,208+ 6,wChanNo); 
        }
        else
        {
            memcpy(CAbuf+3,ParaTempBuf+2+((i - 1) * 200),(CAlen%200));//最后一帧ca报文
            EbEditmsg(sendbuf,CAbuf, (CAlen%200)+3+5 ,0,0x75,(CAlen%200)+3);
            SendAuthDataToMISI(sendbuf,(CAlen%200)+3+5+ 6,wChanNo); 
        }
        
        
        myTaskDelay(2);
    }
	return 0;
}

/********************************************************************
*函数名称：EnMsgByYWTool
*功能：处理现场运维工具下发的数据
*输入：pdata：EB报文缓冲区，rxbuff:数据存放缓冲区wChanNo:端口号
*研发人：张良
*********************************************************************/

INT8U EnMsgByYWTool(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    INT16U wEblenth,datalen,rc;
    INT8U bwlen = 0;
    INT8U rpbuf[25];
	
    wEblenth = pdata[2]+(pdata[1]<<8);//报文长度(数据头至校验码)

    if((pdata[5]&0x08) != 0)//是否加密
    {
        rc = Sgc1161DecryptYWFileData(pdata + 6,wEblenth- 2,pdata + 6);
        datalen = pdata[7]+(pdata[6]<<8);//解密后明文数据长度
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
	    case 0x30://证书管理工具认证请求报文
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
                   //证书管理工具返回证书返回结果
                   break;
           case 0x45://
                SgcYWWaittoSend(wChanNo);
                   break;
           case 0x46://恢复密钥
           
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
*函数名称：SgcYWToolAuthReq
*功能：证书工具与终端身份认证第一步
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/

INT8U SgcYWToolAuthReq(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    
    INT8U i;
    INT16U datalen,rc;
    INT8U signbuf[10];
    INT8U replybuf[100];
    INT8U *p;

    p = ParaTempBuf+1024;//避免和101/104规约下发证书时冲突
    datalen = pdata[8]+(pdata[7]<<8) - 11;//Cer数据包长度
    
    if(pdata[EBAUDATASTARTSITE + 2] == (YWCernum + 1))
    {
        memcpy(p+YWCerlen,(pdata+ EBAUDATASTARTSITE + 3),datalen);
        YWCerlen += datalen;
        YWCernum++ ;
        
        if(YWCernum == 1)//证书管理工具ID只存一次即可
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
    
    if(YWCernum != pdata[EBAUDATASTARTSITE + 1])//还未接收完毕
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
        SgcRdDatatoYWbuf[8 + i] = ~SgcRdDatatoYWbuf[i];//对主站R1按位取反
    }
    
    EbEditmsg(replybuf,SgcRdDatatoYWbuf,SGCRANDOMLEN + 5, 0x0040,0x31,SGCRANDOMLEN);
    SendAuthDataToMISI(replybuf,SGCRANDOMLEN + 5 + 6,wChanNo);	
    return 0;
}

/********************************************************************
*函数名称：SgcYWToolAuthSdata
*功能：运维工具验证 签名
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/

INT8U SgcYWToolAuthSdata(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,rc;
    INT8U signbuf[10];
    INT8U replybuf[20];
    
    datalen = pdata[8]+(pdata[7]<<8) - 11;//证书工具签名数据包长度
    
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
*函数名称：SgcKeyVerforYWTool
*功能：证书管理工具提取终端ID
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/
/*
INT8U GetTerminalIdforYWTool(INT16U wChanNo)
{

    INT8U lenth;
    INT8U IDbuf[24];
    INT8U replybuf[40];
    
        
    GetTerminalId(IDbuf,&lenth);
    
    
    memset(IDbuf,0,24);
    
    memcpy(IDbuf,SgcSelfRandbuf,8); //终端随机数   

    EbEditmsg(replybuf,IDbuf,lenth + 5, 0x0040,0x37,lenth);
	
    SendAuthDataToMISI(replybuf,lenth + 5 + 6,wChanNo)	;

    return 0;   

}
*/
/********************************************************************
*函数名称：SgcKeyVerforYWTool
*功能：证书管理工具提取密钥版本号
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
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
    
    infobuf[0] = KeyVersion;//密钥版本号
    memcpy(infobuf+1,SgcSelfRandbuf,8); //终端随机数   

    EbEditmsg(replybuf,infobuf,9 + 5, 0x0040,0x35,9);
	
    SendAuthDataToMISI(replybuf,14 + 6,wChanNo)	;

    return 0;   

}

/********************************************************************
*函数名称：SgcSerialNumforYWTool
*功能：证书管理工具提取终端序列号
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/
INT8U HostSerialNumforYWTool(INT16U wChanNo)
{
    char rbuf[24];//终端序列号
    INT8U replybuf[40];
    INT8U lenth;
    INT16U wholelen,sdatalen;

    //Sgc1161GetChipSerialNumID(rbuf);
    //rbuf[25] = 77;
    GetTerminalId(rbuf,&lenth);
    sdatalen = lenth - 1;
    wholelen = sdatalen + 5;//报文头中长度
	
    //memcpy(rbuf,rbuf + 2,sdatalen);
    EbEditmsg(replybuf,(INT8U *)rbuf,wholelen, 0x0040,0x37,sdatalen);
	
    SendAuthDataToMISI(replybuf,wholelen + 6,wChanNo);	

    return 0;
}
/********************************************************************
*函数名称：SgcGetPbKeyforYWTool
*功能：证书管理工具提取公钥
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
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
    memcpy(PbKeybuf + 6, PbKeybuf + 2, endatalen);//将加密后的数据放至应用类型之后
    endatalen += 2;//加上报文类型两字节作为Eb报文的长度字节
    
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
*函数名称：SgcSignYWtoolReqfile
*功能：对证书管理工具的证书请求文件签名
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/

INT8U SgcSignYWtoolReqfile(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    
    INT8U i;
    INT16U datalen,rc;
    INT8U signbuf[128];
    //INT8U replybuf[1024];
    INT8U *p;

    p = ParaTempBuf+1024;//避免和101/104规约下发证书时冲突
    
    datalen = pdata[8]+(pdata[7]<<8) - 2;//证书请求数据长度
    
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
    
    if(YWCernum != pdata[EBAUDATASTARTSITE])//还未接收完毕
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

    datalen = p[1]+(p[0]<<8);//证书请求数据长度
    //memcpy(signbuf +9 ,p+2,datalen);
    
    EbEditmsg(signbuf,p+2,datalen + 5, 0x0040,0x3D,datalen);
    SendAuthDataToMISI(signbuf,datalen + 5 + 6,wChanNo);	
    return 0;
}

/********************************************************************
*函数名称：SgcYWToolCAmanage
*功能：对证书管理工具导入证书数据处理
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/
INT8U SgcYWToolCAmanage(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U CArID= 0,i = 0;
    INT16U datalen,CAlen,rc;
    INT8U signbuf[10];
    INT8U replybuf[120];
    //INT8U CAbuf[];
    INT8U *p;

    p = ParaTempBuf+1024;//避免和101/104规约下发证书时冲突

    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA数据包分包长度
    
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
    if(YWCernum != pdata[EBAUDATASTARTSITE + 1])//还未接收完毕
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

    p = cerbuf;//避免和101/104规约下发证书时冲突

    //CAVersion = pdata[EBAUDATASTARTSITE];
    datalen = pdata[8]+(pdata[7]<<8) - 3;//CA数据包分包长度
    
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
    if(YWCernum != pdata[EBAUDATASTARTSITE + 1])//还未接收完毕
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
*函数名称：SgcYWToolReWritrOriCA
*功能：初始证书回写
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
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
   
    OriCalen = oricabuf[3]+(oricabuf[2]<<8);//初始证书长度
    
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
*函数名称：SgcYWToolGetPbKeyCer
*功能：导出公钥证书(终端证书)给证书管理工具
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
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
     CAbuf[1] = Sendnum;//证书报文总帧数
     SendCount = 1 ;
     CAbuf[2] = SendCount;//当前帧序号
     
     if(Sendnum > 1)
     {
         memcpy(CAbuf+3,p+2,200);//每帧传输200字节ca数据
         memcpy(CAbuf +9,CAbuf,203);
         
         EbEditmsg(CAbuf,CAbuf+9, 208,0x0040,0x43,203);
         SendAuthDataToMISI(CAbuf,208+ 6,wChanNo); 
     }
     else
     {
         memcpy(CAbuf+3,p+2,Sendlen);//最后一帧ca报文
         memcpy(CAbuf +9,CAbuf,Sendlen+3);
         
         EbEditmsg(CAbuf,CAbuf+9, Sendlen+3+5 ,0x0040,0x43,Sendlen+3);
         SendAuthDataToMISI(CAbuf,Sendlen+3+5+ 6,wChanNo); 
     }
     
/*     
     for(i = 1;i <= msgnum; i++)
     {
     
         CAbuf[2] = i;//当前帧序号
         
         if(i < msgnum)
         {
             memcpy(CAbuf+3,p+2+((i - 1) * 200),200);//每帧传输200字节ca数据
             EbEditmsg(sendbuf,CAbuf, 208,0x0040,0x43,203);
             SendAuthDataToMISI(sendbuf,208+ 6,wChanNo); 
         }
         else
         {
             memcpy(CAbuf+3,p+2+((i - 1) * 200),(CAlen%200));//最后一帧ca报文
             EbEditmsg(sendbuf,CAbuf, (CAlen%200)+3+5 ,0x0040,0x43,(CAlen%200)+3);
             SendAuthDataToMISI(sendbuf,(CAlen%200)+3+5+ 6,wChanNo); 
         }
         
         
         myTaskDelay(2);
     }
     */
     return 0;
}
/********************************************************************
*函数名称：SgcYWWaittoSend
*功能：发出分帧处理报文
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
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
    CAbuf[1] = Sendnum;//证书报文总帧数

    SendCount++;
    CAbuf[2] = SendCount;//当前帧序号
    
    if(Sendnum > SendCount)
    {
        memcpy(CAbuf+3,p+2+((SendCount - 1) * 200),200);//每帧传输200字节ca数据
        memcpy(CAbuf +9,CAbuf,203);
        
        EbEditmsg(CAbuf,CAbuf+9, 208,0x0040,0x43,203);
        SendAuthDataToMISI(CAbuf,208+ 6,wChanNo); 
    }
    else
    {
        memcpy(CAbuf+3,p+2+((SendCount - 1) * 200),(Sendlen%200));//最后一帧ca报文
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
*函数名称：SgcYWToolHFDCKey
*功能：对证书管理工具恢复终端对称密钥
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/
INT8U SgcYWToolHFDCKey(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT16U datalen,rc;
    INT8U signbuf[10];
    INT8U replybuf[20];

    rc = Sgc1161RecoveryKeydata(pdata + EBAUDATASTARTSITE, 185);//密钥恢复包字节固定185个

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
*函数名称：GetYWTooldataFromWHBuf
*功能：从维护口缓冲区摘取证书管理工具下发的数据
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
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
                totollen += (eblenth + 6);//工具管理报文累计长度
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
*函数名称：SgcKeymanageStepI
*功能：主站恢复终端密钥
*输入：pdata：报文
*研发人：张良
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
*函数名称：SendAuthDataToMISI
*功能：发送认证数据
*输入：sendbuf：发送数据，len：EB报文的长度（报文类型至校验码）
*      wChanNo:端口号
*研发人：张良
*********************************************************************/
BOOL SendAuthDataToMISI(INT8U *sendbuf,INT16U len,INT16U wChanNo)
{
    INT16U SendLen;
    SendLen=(INT16U)MisiWrite(wChanNo,sendbuf,len,3);
    
    //myTaskDelay(5);
    if(SendLen==0xffff)//写错误
    {
        //SendLen=(INT16U)MisiWrite(wChanNo,sendbuf,len,3);
        myTaskDelay(10);
    }
    
    if(SendLen==0xffff)//写错误
    {
        return FALSE;
    }
    else
    {
        return TRUE;        
    }

}



/********************************************************************
*函数名称：EbEditmsg
*功能：编辑Eb安全报文
*输入：pdata：报文缓冲区，sdatabuf数据缓冲区，slen报文长度
*研发人：张良
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
*函数名称：Pack104msgtoEb
*功能：将104报文转换为EB安全报文
*输入：buf：报文，SEBtaillen:安全数据缓冲区，len报文长度
*研发人：张良
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
	        templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB报文长度 
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

                    templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度
                    
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
                   templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
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
               templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
               memcpy(buf+rc,ebbuf,templen);
               rc += templen;
	        i += fixmlen;
	    }
		if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]=buf[i+2]))
	    {

	        templen = (INT16U)buf[i+1]&0x00FF;
               templen = templen + 6;
               
               PackFra68ToEb(buf+i ,templen,ebbuf);
               templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
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
*函数名称：PackFra104ToEb
*功能：封装104报文至EB包
*输入：pdata：报文,len:68报文长度，ebbuf:EB报文存放缓冲区
*研发人：张良
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
        if((encot == 7)&&((ensureflag & 0x80) == 0))//升级结束确认
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
        //tsgcbuf:两字节68报文长度+68报文+两字节随机数长度+随机数

    sum = GetEbMsgCheckSum(ebbuf);
    
    ebbuf[eblen +  4] = sum;
    ebbuf[eblen +  5] = 0xD7;

}
/********************************************************************
*函数名称：PackFixed104ToEb
*功能：封装10帧报文至EB包
*输入：pdata：报文,len:10报文长度，ebbuf:EB报文存放缓冲区
*研发人：张良
*********************************************************************/

void PackFixed104ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{

    INT8U sum;
    INT16U slen;

	slen = len + 6;//报文类型2，应用类型1，101长度字节1 两字节扩展区长度 00 00

    ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + EBAUDATASTARTSITE - 1, pdata, len);//101报文长度字节只有1个，所以要减一
    ebbuf[EBAUDATASTARTSITE + len ] =0x00;
    ebbuf[EBAUDATASTARTSITE + len + 1] =0x00;

    sum = GetEbMsgCheckSum(ebbuf);
    ebbuf[slen +  4] = sum;
    ebbuf[slen +  5] = 0xD7;
	
}
/********************************************************************
*函数名称：Pack101msgtoEb
*功能：将101报文转换为EB安全报文
*输入：buf：报文，SEBtaillen:安全数据缓冲区，len报文长度
*研发人：张良
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
	    templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB报文长度 
	    i += templen;
           rc = i;
	}
       ///////////定长101报文
       if((buf[i] == 0x10)&&(buf[i+fixmlen-1] == 0x16))
	{
	    PackFra10ToEb(buf+i,fixmlen,ebbuf);
        
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
           tmp = i + fixmlen;
               
           if( tmp < len)
           {
               memcpy((buf+i+templen) ,buf+tmp,len - tmp);//将后续报文后移
               len = len - fixmlen + templen;
           }
           memcpy((buf+i ),ebbuf,templen);
           i += templen;
           rc += templen;
	}
       else if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]==buf[i+2]))////////非定长101报文
	{

	    templen = (INT16U)buf[i+1]&0x00FF;
           templen = templen + 6;//101报文长度
           //buf[i+templen-2] = 0x68;           
           saveRecord(buf+i,templen,TXSAVEMODE,1);
           PackFra68ToEb(buf+i ,(INT8U)templen,ebbuf,wChanNo);
           
           tmp = i + templen;
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
               
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
*函数名称：PackFra68ToEb
*功能：封装68帧报文至EB包
*输入：pdata：报文,len:68报文长度，ebbuf:EB报文存放缓冲区
*研发人：张良
*********************************************************************/
//后期可以考虑将tsgcbuf精简掉ZHANGLIANG
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
        ensureflag = pdata[fixmlen + 9];//特征标识符()
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
        encot = pdata[fixmlen + 3]&0x3F;//传输原因
        if((encot == 7)&&((ensureflag & 0x80) == 0))//升级结束确认
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
        //tsgcbuf:两字节68报文长度+68报文+两字节随机数长度+随机数

    sum = GetEbMsgCheckSum(ebbuf);
    
    ebbuf[eblen +  4] = sum;
    ebbuf[eblen +  5] = 0xD7;

}
/********************************************************************
*函数名称：PackFra10ToEb
*功能：封装10帧报文至EB包
*输入：pdata：报文,len:10报文长度，ebbuf:EB报文存放缓冲区
*研发人：张良
*********************************************************************/

void PackFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{

    INT8U sum = 0;
    INT16U slen;

    slen = len + 6;//报文类型2，应用类型1，101长度字节1 安全扩展区长度字节2

    ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + EBAUDATASTARTSITE - 1, pdata, len);//101报文长度字节只有1个，所以要减一
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

	slen = len + 4;//报文类型2，应用类型1，101长度字节1

	ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + 9 - 1, pdata, len);//101报文长度字节只有1个，所以要减一
    sum = GetEbMsgCheckSum(ebbuf);
	ebbuf[slen +  4] = sum;
    ebbuf[slen +  5] = 0xD7;
	
}
	
*/
/********************************************************************
*函数名称：SGCReceiveData
*功能：接收SGC1161安全加密芯片返回的数据
*输入：prcv：数据缓冲区,len:长度
*研发人：张良
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
    
    //查找0x55头,查找10次,每次没找到都延迟100ms
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
        logSysMsgNoTime("没有读到0x55头debug ",0,0,0,0);
        HEPowerReset(10);//重启SC1161芯片
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
                
                rc = 4;        //读取缓冲区不够大，容易溢出
                logSysMsgNoTime("芯片读长度=%x,期望长度=%x，芯片数据长度不对",readlen,len,0,0);
                readlen = len;
            }
        }
        else if(data[2] == 0x86)
        {

            logSysMsgNoTime("验签失败,sw1=%x, sw2=%x",data[1],data[2],0,0);
            rc = 1;
        }
        else
        {
            rc = 4;
            logSysMsgNoTime("芯片返回值异常，sw1=%x,sw2=%x",data[1],data[2],0,0); 
        }
        break;
    case 0x60:
        if((data[1] == 0x67)&&(data[1] == 0x00))
        {
            logSysMsgNoTime("校验码或长度错误,sw1=%x, sw2=%x",data[1],data[2],0,0);
            rc = 5;
        }
        else if((data[1] == 0x6A)&&(data[2] == 0x90))
        {
            rc = 77;
            logSysMsgNoTime("传输错误,sw1=%x, sw2=%x",data[1],data[2],0,0);
        }
        else
        {
            logSysMsgNoTime("芯片报告其他错误,sw1=%x, sw2=%x",data[1],data[2],0,0);
            rc = 2;
        }
        //readlen = (data[3]<<8)+data[4];
        break;
    default: 
        readlen = 0;
        logSysMsgNoTime("芯片应答错误,sw1=%x, sw2=%x",data[1],data[2],0,0);
        HEClearNotReadData();   //把未读出的数据读出 
        
        ////HEPowerReset(10);//重启SC1161芯片
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
        //清剩余数据并拉高片选
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
函数名称：  UpdateProgramMd5()
函数功能：  计算程序文件的MD5值
输入说明：  
输出说明：  TRUE 表示有MD5值  FALSE 表示没有MD5值
备注：      在发送主站的升级结束确认帧后，开始计算并校验MD5
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
    logSysMsgNoTime("MD5值1 %x-%x-%x-%x", UoLoadMD5[0],UoLoadMD5[1],UoLoadMD5[2],UoLoadMD5[3]);
    logSysMsgNoTime("MD5值2 %x-%x-%x-%x", UoLoadMD5[12],UoLoadMD5[13],UoLoadMD5[14],UoLoadMD5[15]);
    
    return TRUE;
    
}   
/*------------------------------------------------------------------/
函数名称：  CheckEncrptchip()
函数功能：  检测加密芯片是否可正常使用
输入说明：  CheckType:0表示维护软件调用，其他表示开机自检
输出说明：  0表示芯片可正常工作
备注：      
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
            //logSysMsgNoTime("加密芯片检测异常.rc=%d",rc,0,0,0);
            i = 0;
            
            return 1;
        }
    }
    
    if(CheckType == 0)
    {
        memset(msgbuf,0,50);
        sprintf(msgbuf,"加密芯片1161序列号:%02x%02x%2x%02x%02x%02x%02x%02x\r\n",p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
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
            //logSysMsgNoTime("加密芯片检测异常.rc=%d",rc,0,0,0);
            i = 0;
            
            return 2;
        }
    }
    
    KeyVersion = p[2];
    if(CheckType == 0)
    {
        logSysMsgNoTime("加密芯片1161密钥版本号:%d",KeyVersion,0,0,0);
    }
    else
    {
        logSysMsgNoTime("加密芯片1161自检成功，密钥版本号:%d",KeyVersion,0,0,0);
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
            //logSysMsgNoTime("加密芯片检测异常.rc=%d",rc,0,0,0);
            i = 0;
            
            return 3;
        }
    }
    //logSysMsgNoTime("加密芯片检测正常.rc=%d",rc,0,0,0);
    return rc;
}

/********************************************************************
*函数名称：SgcYWToolGetPbKeyCer
*功能：导出公钥证书(终端证书)给证书管理工具
*输入：pdata：报文，len：EB报文的长度（报文类型至校验码）wChanNo:端口号
*研发人：张良
*********************************************************************/
INT8U SgcMaintGetPbKeyCer(INT8U *p)
{
     
     INT8U rc;
     //INT8U *p;
     
     //p = ParaTempBuf+1024;

     rc = Sgc1161CheckoutCer(p);
     
     if(rc != 0)
     {

         logSysMsgNoTime("终端证书导出失败.rc=%d",rc,0,0,0);
         p = NULL;
         return 0;

     }

     return 1;
}	

#if 0
#endif
////////////////////////////////////////////////////////////////
//湖南农网加密接口sgc1120a
//张良
////////////////////////////////////////////////////////////////

/********************************************************************
*函数名称：Sgc1120aGetChipSerialNumID
*功能：获取芯片序列号
*输入：rcvbuf:数据存放缓冲区
*研发人：张良
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
*函数名称：Sgc1120aGetChipKeyVersion
*功能：获取芯片密钥版本号
*输入：
*研发人：张良
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
*函数名称：Sgc1120aGetRandomData
*功能：获取芯片随机数
*输入：
*研发人：张良
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
*函数名称：SgcCalculateAuthRData
*功能：计算身份认证数据
*输入：
*研发人：张良
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
*函数名称：SgcGetPKeyAuthData
*功能：获取公钥加密结果
*输入：
*研发人：张良
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
    HESendCmd(&hecmd,pdata,0x08);  //主站下发的公钥验证随机数
    
    myTaskDelay(50);    
    
    rc = SGCReceiveData(rcvbuf,0x6A);
    semGive(sem_qspiid);
    return rc;       
}

/********************************************************************
*函数名称：SgcGetKeyConsultData
*功能：密钥协商
*输入：
*研发人：张良
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
*函数名称：SGC1120aSginVerify
*功能：验证公钥更新签名
*输入：
*研发人：张良
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
    hecmd.p1   = 0x80+Kid;//签名密钥索引
    hecmd.p2   = 0x00;
    hecmd.len1 = 0x00;
    hecmd.len2 = 0x89;
	
	pHeSendBuf2[0] = Pid;
    memcpy((&pHeSendBuf2[0]+1), pdata+1, SGCRANDOMLEN);//更新公钥索引+ 随机数0x01+0x08
    memcpy((&pHeSendBuf2[0]+9), pdata+9, 0x80);//公钥值+签名结果0x40+0x40
	HESendCmd(&hecmd, pHeSendBuf2,0x89); 
	
    myTaskDelay(5);

	rc = SGCReceiveData(rcvbuf,4);
	return rc;
}
*/
	
/*------------------------------------------------------------------/
函数名称：	SGCSginVerify()
函数功能：	验签()
输入说明：	
			pkey 64字节公钥指针  keyno 公钥序号（0~3 表示1~4号公钥)
			pucDataInput 签名数据源 
			pucsign 签名值 
输出说明：	0 成功 
			-10 参数错误
			其他 失败
备注：		
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
    HESendCmd(&hecmd,pHeSendBuf2,(inputlen+signlen));  //64字节
    
    myTaskDelay(50);    
    
    rc = SGCReceiveData(NULL,0);
    semGive(sem_qspiid);
    
    if(rc == 0)
    {
        logSysMsgNoTime("SGCSginVerify:验签正确",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("SGCSginVerify:验签失败.rc=%d",rc,0,0,0);
    }   
    return rc;   
}

/********************************************************************
*函数名称：SgcUploadPKeyDataToChip
*功能：更新公钥至芯片
*输入：
*研发人：张良
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
    hecmd.p1   = 0x80+Pid;//更新公钥索引
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
*函数名称：SgcUploadSymKeyDataToChip
*功能：更新对称密钥至芯片
*输入：
*研发人：张良
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
    hecmd.p1   = 0x01;//更新公钥索引
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
*函数名称：Sgc1120aDectyData
*功能：解密
*输入：
*研发人：张良
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
    hecmd.p1   = 0x00;//更新公钥索引
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
		hecmd.p1	 = 0x00;//更新公钥索引
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
*函数名称：Sgc1120aEnctyData
*功能：加密
*输入：
*研发人：张良
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
    hecmd.p1   = 0x00;//更新公钥索引
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
*函数名称：EbSafetySearchFrame
*功能：解析收到的EB报文，并将报文转化为101/104帧
*输入：oribuf：EB报文缓冲区,validbuf:101/104报文存放缓冲区，
*validtaillen:RxdTail，len：接收到的EB缓冲区长度,wChanNo:端口号
*研发人：张良
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

            if(checklen > (512 -6))//如果checklen>512会出现 i 不再执行++
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
*函数名称：CheckEbMegSty
*功能：检验是否EB报文
*输入：pdata：报文
*研发人：张良
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
*函数名称：EbmegAnalysis
*功能：解析Eb格式报文
*输入：pdata：报文，rxbuff:101/104报文存放缓冲区，wChanNo:端口号
*研发人：张良
*********************************************************************/
INT16U Eb1120amsgAnalysis(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo)
{
    BOOL  Encrptyflag;
    INT8U lenth,rc;
    INT8U bwlen = 0;
    INT16U wEblenth;
	
    wEblenth = pdata[2]+(pdata[1]<<8);//报文长度(报文头至校验码)

	if(pdata[5]&0x08)//是否密文
    {
		Encrptyflag = TRUE;
    }
	else//非加密报文处理
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
        
        bwlen = Eb1120aEncpytDataAnaly((pdata+6),(wEblenth-2),rxbuff,wChanNo);//解密获得明文数据
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
	        case 0x50://主站获取终端芯片信息
	            Sgc1120aMasterAuthI(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
	            break;
	        case 0x52://系统身份认证
                AuthEndflag = 0;
	            Sgc1120aMasterAuthII(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
	            break;     
	        case 0x54://主站向终端返回认证结果，暂时不做处理
	            //SgcMasterauthenStepIII(0x55,wChanNo);
                //myTaskDelay(2);
                //AuthEndflag = wChanNo;  
	            break;
			case 0x55://公钥验证
				Sgc1120aMasterAuthIV(pdata,wEblenth,wChanNo);
				myTaskDelay(1);
				break;
			case 0x57://主站向终端返回认证结果，暂时不做处理
				break;
			case 0x58://密钥协商
				Sgc1120aMasterAuthV(pdata,wEblenth,wChanNo);
				myTaskDelay(1);
				break;
			case 0x5A:
				Sgc1120aMasterAuthVI(pdata,wEblenth,wChanNo);
				break;
            case 0x60://公钥更新
                //if(AuthEndflag == 0)
                {
                    //logSysMsgNoTime("越权更新密钥1",0,0,0,0);
                    //EbErrCodeSend(0x9091,0x61,wChanNo);
                    //return 0;
                }
                Sgc1120aPKeyUpload(pdata,wEblenth,wChanNo);
                break;
            case 0x62://主站返回的验证结果，暂时不处理
                //if(AuthEndflag == 0)
                {
                    //logSysMsgNoTime("越权更新密钥2",0,0,0,0);
                    //EbErrCodeSend(0x9091,0x63,wChanNo);
                    //return 0;
                }
                //SgcKeymanageStepII(pdata,wEblenth,wChanNo,0x62);
                break;
			case 0x63://对称密钥更新
				Sgc1120aSymKeyUploadI(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
				break;
            case 0x65://
            
                //if(AuthEndflag == 0)
                {
                    //logSysMsgNoTime("越权恢复密钥",0,0,0,0);
                    //EbErrCodeSend(0x9092,0x65,wChanNo);
                    //return 0;
                }
                Sgc1120aSymKeyUploadII(pdata,wEblenth,wChanNo);
                myTaskDelay(1);
                break;
		   case 0x66:
		   	    break;
	       case 0x00: //明文报文且不带安全扩展区数据 
			   ////rc = Sgc1120aJudgeWhetherEn((pdata+8),wChanNo);
               ////if(rc != 0)
               {
				 //// logSysMsgNoTime("加密报文使用明文传输，不合规",0,0,0,0);
				  ////EbErrCodeSend(0x9106,0x1f,wChanNo);
				   ////bwlen = 0;
               }
			  ////else
			   {
				   bwlen = pdata[7];
				   memcpy(rxbuff,pdata+8,bwlen);
			   }
	           break; 
              case 0x01://写文件激活，明文但是带签名
              /*ZHANGLIANG 20180211
	            rc = Sgc1120aJudgeWhetherEn((pdata+8),wChanNo);
                  if(rc != 0)
                  {
				   logSysMsgNoTime("加密报文使用明文传输，不合规1",0,0,0,0);
				   EbErrCodeSend(0x9106,0x1f,wChanNo);
				   bwlen = 0;
                  }
  		   else
  		   */
  		   {
  		       //if((pdata[8+fixmlen+9]==0x07)&&((pdata[8+fixmlen+3]&0x3F)==6))//写文件激活
  		       {
  			       bwlen = Eb1120aMsgWithSData((pdata+7),rxbuff,wChanNo);
  		       }
  			  // else
  			  // {
  				   //EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
  				   //logSysMsgNoTime("写文件激活业务类型错误",0,0,0,0);
  				   //bwlen = 0;
  			   //}
  		   }
	           break;
		   case 0x08:
			   bwlen = Eb1120aMsgUpLoadData((pdata+7));
		   	   break;
	       default:
			   EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
			   logSysMsgNoTime("明文业务类型错误",0,0,0,0);
			   bwlen = 0;
               break;              
	    }
    }
	return bwlen;
}
/********************************************************************
*函数名称：Sgc1120aMasterAuthI
*功能：获取终端芯片信息
*输入：
*研发人：张良
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
	
    rbuf[0] = replybuf[2];//密钥版本号
    rc = Sgc1120aGetChipSerialNumID(replybuf);
	
	if(rc != 0)
	{
		EbErrCodeSend(0x9001,0x1f,wChanNo);
		return rc;
	}
	
    sdatalen = replybuf[1]+(replybuf[0]<<8)+1;//序列号长度+密钥版本号长度
    wholelen = sdatalen+5;//报文头中长度
	
    memcpy((rbuf+1),(replybuf+2),(sdatalen-1));
		
    EbEditmsg(replybuf,rbuf,wholelen,0,0x51,sdatalen);

    SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	

    return 0;
}
/********************************************************************
*函数名称：Sgc1120aMasterAuthII
*功能：终端生成认证数据
*输入：
*研发人：张良
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
	    logSysMsgNoTime("获取认证数据失败",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);//获取认证数据失败
		return rc;
	}
	
    sdatalen = authbuf[1]+(authbuf[0]<<8);
    wholelen = sdatalen + 5;//报文头中长度
    
    memcpy(authbuf,(authbuf+2),sdatalen);
	
    EbEditmsg(replybuf,authbuf,wholelen,0,0x53,sdatalen);
    SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}
/********************************************************************
*函数名称：Sgc1120aMasterAuthIV
*功能：处理公钥验证请求
*输入：
*研发人：张良
*********************************************************************/
INT16U Sgc1120aMasterAuthIV(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc,PKeyno;
    INT8U PKeyENCbuf[150];
    INT8U replybuf[150];
    INT16U wholelen,sdatalen;

	PKeyno = pdata[EBAUDATASTARTSITE];//公钥索引
	
	rc = Sgc1120aGetPKeyAuthData(PKeyno,(pdata+10),PKeyENCbuf);
	if(rc != 0)
	{
	    logSysMsgNoTime("获取公钥加密结果失败",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	
	memset(replybuf,0,150);
    sdatalen = PKeyENCbuf[1] + (PKeyENCbuf[0]<<8);
    wholelen = sdatalen + 5;//报文头中长度
    
    memcpy(PKeyENCbuf,PKeyENCbuf+2,sdatalen);

	EbEditmsg(replybuf,PKeyENCbuf,wholelen,0,0x56,sdatalen);
	SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}

/********************************************************************
*函数名称：Sgc1120aMasterAuthV
*功能：处理密钥协商请求
*输入：
*研发人：张良
*********************************************************************/
INT16U Sgc1120aMasterAuthV(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    INT8U rc,SignKeyno;
    INT8U Consultbuf[150];
    INT8U replybuf[150];
    INT16U wholelen,sdatalen;

	SignKeyno = pdata[EBAUDATASTARTSITE+SGCRANDOMLEN+SGCSIGNDATALEN];//签名密钥索引

	rc = Sgc1120aGetKeyConsultData(SignKeyno,(pdata+EBAUDATASTARTSITE),Consultbuf);
	//rc = SGC1120aSKeyConsult(INT8U * pdata,INT8U * cdata,INT16U cdatalen,INT8U *signdata,INT16U signdatalen,INT8U keyno)
	if(rc != 0)
	{
	    logSysMsgNoTime("获取密钥协商数据失败",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	
	memset(replybuf,0,150);
    sdatalen = Consultbuf[1] + (Consultbuf[0]<<8);
    wholelen = sdatalen + 5;//报文头中长度
    memcpy(Consultbuf,Consultbuf+2,sdatalen);

	EbEditmsg(replybuf,Consultbuf,wholelen,0,0x59,sdatalen);
	SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}
/********************************************************************
*函数名称：Sgc1120aMasterAuthVI
*功能：密钥协商结果处理
*输入：
*研发人：张良
*********************************************************************/
INT16U Sgc1120aMasterAuthVI(INT8U *pdata,INT16U len,INT16U wChanNo)
{
    if((pdata[EBAUDATASTARTSITE]==0x90)&&(pdata[EBAUDATASTARTSITE+1]==0x00))
    {
		AuthEndflag = wChanNo;	//认证流程成功走完，设置标志位
    }
	return 0;
}

/********************************************************************
*函数名称：Sgc1120aPKeyUpload
*功能：公钥更新指令
*输入：
*研发人：张良
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
	
	slen = 1+SGCRANDOMLEN+SGCSIGNDATALEN;//签名源数据长度
	//rc = Sgc1120aUploadPKeySignData(SKeyno,(pdata+EBAUDATASTARTSITE),EnRanbuf);
	//rc = SGC1120aSginVerify((pdata+EBAUDATASTARTSITE),slen,(pdata+9+1+8+64),SGCSIGNDATALEN,SKeyno);

	rc = SGC1120aSginVerify(p,slen,(p+slen),SGCSIGNDATALEN,SKeyno);
	if(rc != 0)
	{
	    logSysMsgNoTime("公钥更新签名验证失败",0,0,0,0);
		EbErrCodeSend(0x9004,0x1f,wChanNo);
		return rc;
	}
	
	rc = Sgc1120aUploadPKeyDataToChip(p,EnRanbuf);
	if(rc != 0)
	{
		logSysMsgNoTime("公钥写入失败",0,0,0,0);
		EbErrCodeSend(0x9004,0x1f,wChanNo);
		return rc;
	}
	rc = Sgc1120aGetPKeyAuthData(NPKeyno,(p+1),EnRanbuf);
	if(rc != 0)
	{
		logSysMsgNoTime("新公钥加密失败",0,0,0,0);
		EbErrCodeSend(0x9004,0x1f,wChanNo);
		return rc;
	}
	
	memset(replybuf,0,150);
    sdatalen = EnRanbuf[1] + (EnRanbuf[0]<<8);
    wholelen = sdatalen + 5;//报文头中长度
    memcpy(EnRanbuf,EnRanbuf+2,sdatalen);

	EbEditmsg(replybuf,EnRanbuf,wholelen,0,0x61,sdatalen);
	SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);
	return rc;
}
/********************************************************************
*函数名称：Sgc1120aSymKeyUploadI
*功能：对称密钥更新指令
*输入：
*研发人：张良
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
	    logSysMsgNoTime("获取随机数失败",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	
    sdatalen = rbuf[1]+(rbuf[0]<<8);
    wholelen = sdatalen + 5;//报文头中长度
    
    memcpy(rbuf,(rbuf+2),sdatalen);
	
    EbEditmsg(replybuf,rbuf,wholelen,0,0x64,sdatalen);
    SendAuthDataToMISI(replybuf,(wholelen+6),wChanNo);	
	return rc;
}
/********************************************************************
*函数名称：Sgc1120aSymKeyUploadII
*功能：对称密钥更新数据处理
*输入：
*研发人：张良
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
	    logSysMsgNoTime("对称密钥更新验签失败",0,0,0,0);
		EbErrCodeSend(0x9109,0x1f,wChanNo);
		return rc;
	}
	rc = Sgc1120aUploadSymKeyDataToChip(p,replybuf);
	if(rc != 0)
	{
	    logSysMsgNoTime("对称密钥更新写入失败",0,0,0,0);
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
*函数名称：Pack101msgtoEb
*功能：将101报文转换为EB安全报文
*输入：buf：报文，SEBtaillen:安全数据缓冲区，len报文长度
*研发人：张良
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
		    templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB报文长度 
		    i += templen;
	        rc = i;
		}
	    ///////////定长101报文
	    if((buf[i] == 0x10)&&(buf[i+fixmlen-1] == 0x16))
		{
		   PackFra10ToEb(buf+i,fixmlen,ebbuf);
	        
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
           tmp = i + fixmlen;
               
           if( tmp < len)
           {
               memcpy((buf+i+templen) ,buf+tmp,len - tmp);//将后续报文后移
               len = len - fixmlen + templen;
           }
           memcpy((buf+i ),ebbuf,templen);
           i += templen;
           rc += templen;
		}
	    else if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]==buf[i+2]))////////非定长101报文
		{

		   templen = (INT16U)buf[i+1]&0x00FF;
           templen = templen + 6;//101报文长度
           //buf[i+templen-2] = 0x68;           
           saveRecord(buf+i,templen,TXSAVEMODE,1);
           PackFra68ToEb(buf+i ,(INT8U)templen,ebbuf,wChanNo);
           
           tmp = i + templen;
           templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
               
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
    if(rc > 0)//该发送的密文长度
    {
        (*SEBtaillen) = i;//加密后缓冲区总长度(已加密报文+剩余未加密明文)
    }
    return rc;
}

/********************************************************************
*函数名称：PackFra10ToEb
*功能：封装10帧报文至EB包
*输入：pdata：报文,len:10报文长度，ebbuf:EB报文存放缓冲区
*研发人：张良
*********************************************************************/

void Pack1120aFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf)
{

    INT8U sum = 0;
    INT16U slen;

    slen = len + 6;//报文类型2，应用类型1，101长度字节1 安全扩展区长度字节2

    ebbuf[0] = ebbuf[3] = 0xEB;
    ebbuf[1] = HIBYTE(slen);
    ebbuf[2] = LOBYTE(slen);
    ebbuf[4] = 0x00;
    ebbuf[5] = 0x00;
    ebbuf[6] = 0x00;
    ebbuf[7] = (INT8U)len;
	
    memcpy(ebbuf + EBAUDATASTARTSITE - 1, pdata, len);//101报文长度字节只有1个，所以要减一
    ebbuf[EBAUDATASTARTSITE + len ] =0x00;
    ebbuf[EBAUDATASTARTSITE + len + 1] =0x00;
    
    sum = GetEbMsgCheckSum(ebbuf);
    ebbuf[slen +  4] = sum;
    ebbuf[slen +  5] = 0xD7;
	
}

/*------------------------------------------------------------------/
函数名称：  SGCKeyConsult()
函数功能：  密钥协商
输入说明：  cdata 协商数据（主站随机数）
                signdata 签名  
                keyno 公钥序号
输出说明：  0 成功 其他 失败
备注：      
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
        logSysMsgNoTime("协商成功",0,0,0,0);
    }
    else
    {
        logSysMsgNoTime("协商失败.rc=%d",rc,0,0,0);
    }  
    return rc;    
}

/********************************************************************
*函数名称：Eb1120aEncpytDataAnaly
*功能：处理湖南农网主站下发的密文数据，解密并分析
*输入：pdata：报文
*研发人：张良
*********************************************************************/

INT8U Eb1120aEncpytDataAnaly(INT8U* pdata,INT16U lenth,INT8U *dedata,INT16U wChanNo)
{
	INT8U ddatalen,bwlen;
	INT8U *p;
	INT8U sgcbuf[330];
	int rc;
	 
	rc = Sgc1120aDectyData(pdata,sgcbuf,lenth);//解密获得明文数据

	if(rc != 0)
	{
		EbErrCodeSend(0x9103,0x1f,wChanNo);//解密失败
		return 0;
	}
		
    p = sgcbuf + 2;
	
    ddatalen = sgcbuf[1]+(sgcbuf[0]<<8);//明文报文长度
    saveRecord(sgcbuf,ddatalen+2,RXSAVEMODE,0);
	
	rc = Sgc1120aJudgeWhetherEn((p+2),wChanNo);
	if(rc == 0)
	{
		logSysMsgNoTime("明文报文使用密文传输,不合规",0,0,0,0);
		EbErrCodeSend(0x9106,0x1f,wChanNo);
        return 0;
	}
	
	rc = Check1120AIllfgalType((p+2),p[0],wChanNo);//P第一个直接是应用类型，第二个直接是101报文长度
    if(rc != 0)
    {
        EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
        logSysMsgNoTime("业务类型错误",0,0,0,0);
        return 0;
    }
	
    switch(p[0])//判断业务应用类型，目前只用到了00 01 02 03 05 07
    {
	    case 0x00:
			bwlen = p[1];//101报文长度
			memcpy(dedata,p+2,bwlen);
			break;
	    case 0x01:
			//bwlen = EbMsgWithSData(p,dedata,ddatalen,wChanNo);
			bwlen = Eb1120aMsgWithSData((p+1),dedata,wChanNo);
			break;
	    case 0x02://终端对主站的确认报文，主站不会下发02
            EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
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
            EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
            bwlen = 0;
			break;
       default:            
            EbErrCodeSend(0x9101,0x1f,wChanNo);//应用类型错误
            bwlen = 0;
			break;
    }
    return bwlen;
	
}
/********************************************************************
*函数名称：Check1120AIllfgalType
*功能：处理主站下发的报文应用类型是否合法
*输入：pdata：报文，type:应用类型，wChanNo:规约端口号
*研发人：张良
*********************************************************************/

int Check1120AIllfgalType(INT8U *ptada,INT8U type,INT16U wChanNo)
{
    INT8U DncryptTi = 0;
    INT8U DncryptCot = 0;
    INT8U DncryptPI = 0;
    INT8U Filetype = 0;
    
    int rc = 0;
    
    if(wChanNo < 6)//101报文
    {
	    DncryptTi = ptada[fixmlen + 1];
	    DncryptCot = ptada[fixmlen + 3]&0x3F;
	    DncryptPI = ptada[fixmlen +9];
        if(DncryptTi == 210)
        {
            Filetype = ptada[fixmlen +10];
        }
    }
    else if(wChanNo > 40)//104报文
    {
        DncryptTi = ptada[6];
	    DncryptCot = ptada[8]&0x3F;
	    if(DncryptTi == 203)
	    {
	        DncryptPI = ptada[14];
//修改参数的特征标识比遥控和软件升级往前一个字节（因为定值区号只有两个字节）
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
    	case 46://遥控TI
			//预置 SE=1，cot=6
			//执行 SE=0，cot=6
			//撤销 SE=0，cot=8
    	    if(((DncryptPI&0x80) != 0)||(DncryptCot == 8))//遥控选择/撤销
    	    {
    	    	if(type != 0x05)
    	    	{
    	    	    rc = 1;
    	        }
    	    }
    	    else//遥控执行主站下发的遥控执行PI = 0
    	    {
    	    	if(type != 0x07)
    	    	{
    	    	    rc = 1;
    	        }    	    	
    	    }
    	    break;           
    	case 210://写文件激活
    	    if((DncryptCot == 6)&&(Filetype == 0x07))//
    	    {
  	    	 if(type != 0x01)
    	    	 {
    	    	    rc = 1;
    	        }      	    	
    	    }
    	    break;    	
    	case 203:  
			//预置 SE=1，CR=0，cot=6
			//执行 SE=0，CR=0，cot=6
			//撤销 SE=0，CR=1，cot=8
    	    if((DncryptPI&0x80) != 0)//参数预置(终止的话是0x40)
    	    {
  	    	    if(type != 0x01)
    	    	{
    	    	    rc = 1;
    	        }
                rmparaflag = 1;
                logSysMsgNoTime("参数预置",0,0,0,0);         
    	    }
    	    else if((DncryptCot == 0x06)&&((DncryptPI&0x40) == 0))//参数固化
    	    {
  	    	    if(type != 0x03)
    	    	{
    	    	    rc = 1;
    	        }  
                rmparaflag = 2;
                
                logSysMsgNoTime("参数固化",0,0,0,0);         
    	    }
            else if((DncryptPI&0x40) != 0)
            {
               //if((rmparaflag == 1)&&(type != 0x01))//参数预置取消
               // 现场四方主站下发的0x3,暂时修改
               if((rmparaflag == 1)&&(type != 0x03))//参数预置取消 
               {
                   rc = 1;
                   logSysMsgNoTime("参数预置取消",0,0,0,0);         
               }
               else if((rmparaflag == 2)&&(type != 0x03))//参数固化取消
               {
                   rc = 1;
                   logSysMsgNoTime("参数固化取消",0,0,0,0);         
               }
               rmparaflag = 0;
           }
    	   break;
        case 211:
    	    if(((DncryptPI&0x80) != 0)&&(DncryptCot == 6))//升级启动
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
*函数名称：Sgc1120aJudgeWhetherEn
*功能：判断该报文是否应该加密
*输入：Pdata:数据存放缓冲区
*输出：rc = 1;报文需要加密，rc = 0;不需要加密
*研发人：张良
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
    if(wChanNo < 6)//101报文
    {
	    DncryptTi = pdata[fixmlen + 1];
	    DncryptCot = pdata[fixmlen + 3]&0x3F;
	    DncryptPI = pdata[fixmlen +9];
           if(DncryptTi == 210)
           {
               Filetype = pdata[fixmlen +10];
           }
    }
    else if(wChanNo > 40)//104报文
    {
        DncryptTi = pdata[6];
	    DncryptCot = pdata[8]&0x3F;
	    if(DncryptTi == 203)
	    {
	        DncryptPI = pdata[14];
           //修改参数的特征标识比遥控和软件升级往前一个字节（因为定值区号只有两个字节）
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
    	case 46://遥控TI
    	    rc = 1;
            break;
        case 203:
            rc = 1;
            break;
        case 210:
    	     if((DncryptCot == 6)&&(Filetype == 0x07))//写文件激活现场为密文下发
    	     {
                 rc = 1;//ZHANGLIANG  20180211
    	     }
            break;
        case 211:
            if((DncryptCot == 6)&&((DncryptPI&0x80) != 0))//升级启动
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
*函数名称：Eb1120aMsgWithSData
*功能：1120a处理只带签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
*********************************************************************/
INT8U Eb1120aMsgWithSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo)
{
    INT8U KeyId,rc;
    INT16U tmplen,extlen,seek;
    INT8U verbuf[64];

	tmplen = pdata[0];//101/104报文长度
	extlen = pdata[tmplen+2]+(pdata[tmplen+1]<<8)-1;//签名长度

	seek = tmplen+1+extlen+2;//整个数据区长度(1字节长度+原始报文+2字节长度+安全扩展区数据)
	KeyId = pdata[seek];
	
	seek = 1;//一个字节原始报文长度
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
       logSysMsgNoTime("数据验证签名错误:",0,0,0,0);
       return 0;
	}
	
	return tmplen;
}

/********************************************************************
*函数名称：Eb1120aMsgUpLoadData
*功能：1120a处理带时间随机数以及签名的升级包验证报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
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
	
	return 0;//待定 
}

/********************************************************************
*函数名称：EbMsgWithRandSData
*功能：处理带随机数以及签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
*********************************************************************/
INT8U Eb1120aMsgWithRandSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo)
{

    INT8U KeyId,rc,seek;
    INT16U tmplen,extlen;
    INT8U randbuf[8];
    INT8U verbuf[72];

	seek = 2;
	tmplen = pdata[1];//101有效数据长度
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8)-1;//扩展数据区长度减一个字节索引长度

	seek += tmplen;
	seek += 2;
	memcpy(randbuf,pdata+seek,8);
    if(memcmp(randbuf,SgcSelfRandbuf,8) != 0)
    {
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        return 0;
    }
	memcpy(verbuf,pdata+seek,extlen);//随机数加签名值
    seek += extlen;
	KeyId = pdata[seek];//签名密钥索引
	
	rc = SGC1120aSginVerify((pdata+2),tmplen, verbuf, extlen, KeyId);
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("数据验证签名错误:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*函数名称：Eb1120aMsgWithTandSData
*功能：1120a处理带时间以及签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
*********************************************************************/
INT8U Eb1120aMsgWithTandSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo)
{
    INT8U KeyId,rc;
    INT16U tmplen,extlen,seek;
    INT8U timebuf[6];
    INT8U verbuf[70];

	seek = 2;
	//KeyId = pdata[len - 1];
	tmplen = pdata[1];//101有效数据长度
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8)-1;//扩展数据区长度减一个字节的索引长度
	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);

    if(CheckTimeAging(timebuf) != 0)
    {
        EbErrCodeSend(0x9105,0x1f,wChanNo);//
        logSysMsgNoTime("数据时间戳校验错误:",0,0,0,0);
        return 0;
    }
	
	memcpy(verbuf,pdata+seek,extlen);//
    seek += extlen;
	KeyId = pdata[seek];//签名密钥索引
	
	rc = SGC1120aSginVerify((pdata+2),tmplen, verbuf,extlen, KeyId);
	if(rc == 0)
	{
	    memcpy(decpbuf,pdata+2,tmplen);
	}
	else
	{
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("数据验证签名错误:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}
/********************************************************************
*函数名称：EbMsgWithAllData
*功能：处理带时间随机数以及签名的解密报文
*输入：pdata：报文decpbuf:报文数据存放
*研发人：张良
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
	tmplen = pdata[1];//101报文长度
	extlen = pdata[tmplen+3]+(pdata[tmplen+2]<<8)-1;//扩展数据区长度

	seek += tmplen;
	seek += 2;
	memcpy(timebuf,pdata+seek,6);
	memcpy(verbuf,pdata+seek,extlen);//扩展区数据拼接至101报文后，计算签名用
	seek += 6;
	memcpy(randombuf,pdata+seek,8);
	seek += 8;
	seek += 64;
	KeyId = pdata[seek];
	
    if(CheckTimeAging(timebuf) != 0)
    {
        EbErrCodeSend(0x9105,0x1f,wChanNo);//
        logSysMsgNoTime("07数据时间戳校验错误:",0,0,0,0);
        return 0;
    }
 
    if(memcmp(randombuf,SgcSelfRandbuf,8) != 0)
    {
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        logSysMsgNoTime("07随机出校验错误:",0,0,0,0);         
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
        logSysMsgNoTime("数据验证签名错误:",0,0,0,0);
	    return 0;
	}
	
	return tmplen;
}

/********************************************************************
*函数名称：Pack1120aFor101msgtoEb
*功能：1120a将101报文转换为EB安全报文
*输入：buf：报文，SEBtaillen:安全数据缓冲区，len报文长度
*研发人：张良
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
		    templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB报文长度 
		    i += templen;
	        rc = i;
		}
       ///////////定长101报文
        if((buf[i] == 0x10)&&(buf[i+fixmlen-1] == 0x16))
	    {
	        PackFra10ToEb(buf+i,fixmlen,ebbuf);
        
            templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
            tmp = i + fixmlen;
                
            if( tmp < len)
            {
                memcpy((buf+i+templen) ,buf+tmp,len - tmp);//将后续报文后移
                len = len - fixmlen + templen;
            }
            memcpy((buf+i ),ebbuf,templen);
            i += templen;
            rc += templen;
	    }
        else if((buf[i] == 0x68)&&(buf[i+3] == 0x68)&&(buf[i+1]==buf[i+2]))////////非定长101报文
	    {

	        templen = (INT16U)buf[i+1]&0x00FF;
            templen = templen + 6;//101报文长度
            saveRecord(buf+i,templen,TXSAVEMODE,1);
            Pack1120aFra68ToEb(buf+i,(INT8U)templen,ebbuf,wChanNo);
            
            tmp = i + templen;
            templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
                
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
*函数名称：Pack1120aFra68ToEb
*功能：1120a封装68帧报文至EB包
*输入：pdata：报文,len:68报文长度，ebbuf:EB报文存放缓冲区
*研发人：张良
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
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//参数预置确认
        {
            ensureflag = 1;
        }
    }
    else if((DncryptTi == 46)||(DncryptTi == 0X2D))
    {
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//遥控预置确认
        {
            ensureflag = 1;
        }
    }
    else if(DncryptTi == 211)
    {
   
  	    if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//
  	    {
  	        if(HnnwInf.UpgradeFlag == 1)//升级启动确认
  	        {
				ensureflag = 1;
				HnnwInf.UpgradeFlag = 0;
  	        }
			else//升级结束确认
			{
				ensureflag = 0;
				Upendflag = 2;
			}
  	    }
    }
    
    memcpy(p+2,pdata,len);
    
    if(ensureflag)//应用类型为02
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
		if(rcI == 1)//需要加密
		{
			rc = Sgc1120aEnctyData(p,(len+4));
		}
    }
	
    if(rcI == 0)//不需加密
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
        ebbuf[5] = 0x08;//密文数据
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
*函数名称：Pack1120afor104msgtoEb
*功能：将104报文转换为EB安全报文
*输入：buf：报文，SEBtaillen:安全数据缓冲区，len报文长度
*研发人：张良
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
	        templen = buf[i+2]+(buf[i+1]<<8) + 6;//EB报文长度 
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
                   
                   templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度    
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
                   templen = ebbuf[2]+(ebbuf[1]<<8) + 6;//EB报文长度 
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
*函数名称：Pack1120aFra104ToEb
*功能：封装104报文至EB包
*输入：pdata：报文,len:68报文长度，ebbuf:EB报文存放缓冲区
*研发人：张良
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
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//参数预置确认
        {
            ensureflag = 1;
        }
    }
    else if((pdata[6] == 46)||(pdata[6] == 0X2D))
    {
	 DncryptPI = pdata[15];
        //ensureflag = pdata[15];
        if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//遥控预置确认
        {
            ensureflag = 1;
        }
    }
    else if(pdata[6] == 211)
    {
           DncryptPI = pdata[15];
		
	    if((DncryptCot == 7)&&((DncryptPI&0x80)!=0))//
  	    {
  	        if(HnnwInf.UpgradeFlag == 1)//升级启动确认
  	        {
		      ensureflag = 1;
		      HnnwInf.UpgradeFlag = 0;
  	        }
		 else//升级结束确认
		 {
		     ensureflag = 0;
		     Upendflag = 2;
		 }
  	    }
        //if((DncryptCot == 7)&&((ensureflag & 0x80) == 0))//升级结束确认
        //{
            //Upendflag = 1;
        //}
    }


    memcpy(p+2,pdata,len);
    
    if(ensureflag)//应用类型为02
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
    
    if(rcI == 0)//不需加密
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
        ebbuf[5] = 0x08;//密文数据
        memcpy(ebbuf+6,p+2,eblen - 2);
    }
    else
    {
        memset(ebbuf,0,len);
        EbErrCodeSend(0x9109,0x1f,wChanNo);
        return ;
    }
        //tsgcbuf:两字节68报文长度+68报文+两字节随机数长度+随机数

    sum = GetEbMsgCheckSum(ebbuf);
    
    ebbuf[eblen +  4] = sum;
    ebbuf[eblen +  5] = 0xD7;

}

/********************************************************************
*函数名称：SGC1120aVerifyUpLoadData
*功能：1120a验证升级包验证数据
*输入：pdata：
*研发人：张良
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
        logSysMsgNoTime("08数据时间戳校验错误:",0,0,0,0);
        return 0;
    }
    
    if(memcmp(UpLoadRdata,SgcSelfRandbuf,8) != 0)
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9104,0x1f,wChanNo);//
        logSysMsgNoTime("08随机出校验错误:",0,0,0,0);         
        return 0;
    }
    
    memcpy(verbuf,UoLoadMD5,16);
    memcpy(verbuf+16,UpLoadtime,6);
    memcpy(verbuf+22,UpLoadRdata,8);
    
	rc = SGC1120aSginVerify(verbuf,30,UpLoadSdata,64,UpLoadKeyId);
    if(rc == 0)
    {
       // EbErrCodeSend(0x9000,0x1f,wChanNo);//成功不返回处理结果
        StartProgramUpdate();
        return 0;
    }
    else
    {
        ClearProgramUpdate();
        EbErrCodeSend(0x9102,0x1f,wChanNo);//
        logSysMsgNoTime("08数据验证签名错误:",0,0,0,0);
	 return 0;
    }
    
}
/*------------------------------------------------------------------/
函数名称：  Check1120aEncrptchip()
函数功能：  检测加密芯片1120a是否可正常使用
输入说明：  CheckType:0表示维护软件调用，其他表示开机自检
输出说明：  0表示芯片可正常工作
备注：      
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
            //logSysMsgNoTime("加密芯片检测异常.rc=%d",rc,0,0,0);
            i = 0;
            
            return 2;
        }
    }
    
    KeyVersion = p[2];
    if(CheckType == 0)
    {
        logSysMsgNoTime("加密芯片1120a密钥版本号:%d",KeyVersion,0,0,0);
    }
    else
    {
        logSysMsgNoTime("加密芯片1120a自检成功，密钥版本号:%d",KeyVersion,0,0,0);
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
            //logSysMsgNoTime("加密芯片检测异常.rc=%d",rc,0,0,0);
            i = 0;
            
           return 1;
        }
    }
    
    //if(CheckType == 1)
    {
        memset(msgbuf,0,50);
        sprintf(msgbuf,"加密芯片1120a序列号:%02x%02x%2x%02x%02x%02x%02x%02x\r\n",p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
        logSysMsgNoTime(msgbuf,0,0,0,0);
    }
    myTaskDelay(20);    
    rc = Sgc1120aGetRandomData(p);
   // if(rc  == 0)
    //{
        //memset(msgbuf,0,50);
        //sprintf(msgbuf,"加密芯片随机数:%02x%02x%2x%02x%02x%02x%02x%02x\r\n",p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9]);
        //logSysMsgNoTime(msgbuf,0,0,0,0);
    //}
    //else
    //{
        //logSysMsgNoTime("随机数获取失败",0,0,0,0);
    //}
    return rc;
}

/*------------------------------------------------------------------/
函数名称：  EncrptyChiptest(INT8U type)
函数功能：  检测安全加密芯片类型
输入说明：  
输出说明： 1:1161,2:1120a,0:无加密芯片
备注：      
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
	    ////logSysMsgNoTime("加密芯片型号1161",0,0,0,0);
	    return 1;
	}
	if(rc == 0)
	{
		////logSysMsgNoTime("加密芯片型号1120a",0,0,0,0);
	    return 2;
	}
	logSysMsgNoTime("加密芯片不存在",0,0,0,0);
	return 0;
}

/*------------------------------------------------------------------/
函数名称：  Packf68ToOld1120aEn(INT8U type)
函数功能：  封装湖南老加密报文
输入说明：  
输出说明： 
备注：      
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
       buf[0] = len;//需要将待加密数据长度写入密文
	memcpy((buf+1),oribuf+7,len);
	switch(tivalue)//遥信，遥测，遥控，信息上报都要加密
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
				enlen = (buf[0]<<8)+buf[1];//加密后密文+MAC长度

				oribuf[1]=oribuf[2]=enlen+3;
				memcpy((oribuf+7),(buf+2),enlen);
			}
			else
			{
                           myTaskDelay(20);
                           logSysMsgNoTime("加密失败I!rc==%d",rc,0,0,0);
                           memcpy((buf+1),oribuf+7,len);
                           rc = Sgc1120aEnctyData(buf,(len+1));
                           if(rc == 0)
                           {
                               oribuf[0]=oribuf[3]=0x69;
                               enlen = (buf[0]<<8)+buf[1];//加密后密文+MAC长度
                       
                               oribuf[1]=oribuf[2]=enlen+3;
                               memcpy((oribuf+7),(buf+2),enlen);
                           }
                           else
                           {
                               myTaskDelay(20);
                               logSysMsgNoTime("加密失败II!rc==%d",rc,0,0,0);
                               memcpy((buf+1),oribuf+7,len);
                               rc = Sgc1120aEnctyData(buf,(len+1));
                               if(rc == 0)
                               {
                                   oribuf[0]=oribuf[3]=0x69;
                                   enlen = (buf[0]<<8)+buf[1];//加密后密文+MAC长度
                           
                                   oribuf[1]=oribuf[2]=enlen+3;
                                   memcpy((oribuf+7),(buf+2),enlen);
                               }                       
                           }
                           
                           if(rc != 0)
                           {
                               logSysMsgNoTime("加密失败III!rc==%d",rc,0,0,0);
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
函数名称：  SGCOldPkeyUpdate()
函数功能：  公钥更新
输入说明：  pdata:主站接收数据
            sdata 公钥加密后得到的密文

输出说明：  0 成功 其他 失败 -10 参数错误
备注：      
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
        logSysMsgNoTime("公钥写入错误",0,0,0,0);  	
    	return -2;
    }
    //memset(encryptdata,0,68);
    ////rc = SGCPKeyEncrypt(sdata,random,8,newkeyno);
	rc = Sgc1120aGetPKeyAuthData(newkeyno,random,sdata);
    if(rc != 0)
    {
        logSysMsgNoTime("公钥加密失败",0,0,0,0);  	
    	return -3;
    }
    return rc;

}

/*------------------------------------------------------------------/
函数名称：  SGCOldSymkeyUpdate()
函数功能：  对称密钥更新
输入说明：  pdata:主站接收数据

输出说明：  0 成功 其他 失败 -10 参数错误
备注：      
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
    memcpy(ekeydata,pdata+loc,132);//四条对称密钥密文数据及MAC校验数据
    loc = loc + 132;
    memcpy(signdata,pdata+loc,64);//签名
    	    
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
    	logSysMsgNoTime("对称密钥写入失败",0,0,0,0);  	
    	return -2;
    }
    return rc;
}

