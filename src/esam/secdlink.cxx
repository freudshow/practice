#include "secdlink.h"


extern INT32U baudRateTable[];
//INT8U N101Encrptystyle=2;
extern INT16U fixmlen ;//
extern INT16U AuthEndflag;

/*���캯������ʼ��*/
CSecDLink::CSecDLink(INT16U appid,CSecAppSev *p)
{
	pSecApp=p;
	wAppID=appid;
}

CSecDLink::~CSecDLink()
{
}

BOOL CSecDLink::InitSecDLink(void)
{
    INT8U rc;
    struct TaskInfo_t *TI;
    struct PortInfo_t *PI;
    struct PSec101Pad *pSecPad=NULL;
    INT32U i;

    GetTaskInfo(wAppID,&TI);
    GetPortInfo(wAppID,&PI);
    
    if (PI->CodePad!=NULL)
    {
        pSecPad = (struct PSec101Pad *)PI->CodePad;
        TimeOutValue=pSecPad->TimeOut;
        if (TimeOutValue<100)
            TimeOutValue=100;

        DLIdleTime=TimeOutValue/100;//��·���м��ʱ�䲻��С��2��
        if((DLIdleTime<2)||(DLIdleTime>=4))
            DLIdleTime=2;
        
        if(pSecPad->control & CON_ISSENDE5) //ll 2010.9.16
            IsSendE5 = 1;       
        else
            IsSendE5 = 0;
           
        if(pSecPad->BalanceMode == 1)
        {
            BalanMode=TRUE;
            if(pSecPad->control & CON_DIRBITFLAG)   //ll 2010.9.16
                IEC_DIR=0x00;
            else    
                IEC_DIR=0x80;            
        }
        else
        {
            BalanMode=FALSE;
            IEC_DIR=0;
        }
        
        
        
        if(pSecPad->control & CON_ISNEEDDELAY)
            IsDoubleLinkInit = FALSE;
        else
            IsDoubleLinkInit = TRUE;    //֧��˫����·ͬ����ʼ��
        
        //��֧�ֹ�����Լ��չ������£��Զ�ʽ��������·
        /*if(pSecPad->control & CON_101GYKZ)
        {
           IsDoubleLinkInit = FALSE;   
        }*/    
        
        if(pSecPad->control & CON_NOFCVCTRL)
        {
            NoJudgeFCB = TRUE;
            logSysMsgNoTime("101��Լ���ж�FCB��ת",0,0,0,0);
        }
        else
            NoJudgeFCB = FALSE;
        
        HeartBeatIdleLimit = 0; //60;   //����������ʱ����
        HeartBeatIdleTime = 0;
        
        LinkAddrSize=pSecPad->LINKAddrSize;
        if(LinkAddrSize!=1)
            LinkAddrSize=2;//zzw
        
    }
    else
    {
        TimeOutValue=100;
        BalanMode=FALSE;
        IEC_DIR=0;
        LinkAddrSize=2;
        IsDoubleLinkInit = TRUE;
    }
    
    EndlessLoopInit();
    
    baudrate = baudRateTable[PI->Attri->Baud];

    dwAppTID=TI->Tid;//ϵͳ���������ID��

    DLPriStatus=PRIDISABLE;
    DLSecStatus=SECDISABLE;
    
    TlaConCode = 0;
    RlaConCode = 0;
    
    fixmlen = 4 + LinkAddrSize;//
    
    i=CheckDevType(PI->DevIDSet[0]);
    if (i==0)//0����I���߼��豸
    {
        struct AppLBConf_t AppLBConf;
        L_ReadBConf(PI->DevIDSet[i],wAppID,(INT8U*)&AppLBConf);
        SourceNo=AppLBConf.Address;//������ַΪԴվַ
    }
    else if (i==2)//2:II���߼��豸
    {
        struct AppSLBConf_t AppSLBConf;
        //SL_ReadBConf(PI->DevIDSet[i],wAppID,(INT8U*)&AppSLBConf);
        if(!SL_ReadBConf(PI->DevIDSet[i],wAppID,(INT8U*)&AppSLBConf))       //����coverity����
        {
            return false;
        }
        SourceNo=AppSLBConf.Address;//������ַΪԴվַ
    }
    
    IsEncrypt = 0;
    N101Encrptystyle = 0;
    #ifdef INCLUDE_ENCRYPT
    
        if(pSecPad != NULL)
        {
            //CON_ENCRYPT:CON_1161ENCRPTY = 1:0(�޼���)  0:0(11�氲ȫ����) 0:1(16�氲ȫ��������) 1:1(�Ƿ�)
            switch(pSecPad->control & (CON_ENCRYPT|CON_1161ENCRPTY|CON_1120ENCRPTY|CON_OLD1120ENCRPTY))
            {
            case 0:
                IsEncrypt = 1;
                myEnctyptInit(SourceNo, pSecPad->EncryptTimeout);
                logSysMsgNoTime("11�氲ȫ��������(SGC1126)-101",0,0,0,0);
                break;
            case CON_ENCRYPT|CON_1161ENCRPTY:
                //1161����оƬ
                N101Encrptystyle = 2;
                logSysMsgNoTime("15�氲ȫ������������(SGC1161)-101",0,0,0,0);
                myTaskDelay(2);    
                rc = EncrptyChiptest(1);
                if(rc != 1)
                {
                    logSysMsgNoTime("���ܷ�����оƬ���Ͳ�ƥ������оƬ������",0,0,0,0);
                }
                break;
            case CON_ENCRYPT|CON_1120ENCRPTY:
                //1120����оƬ����ũ������
                N101Encrptystyle = 3;
                logSysMsgNoTime("����ũ����ȫ��������(SGC1120a)-101",0,0,0,0);
                myTaskDelay(2);    
               rc = EncrptyChiptest(1);
               if(rc != 2)
               {
                   logSysMsgNoTime("���ܷ�����оƬ���Ͳ�ƥ������оƬ������",0,0,0,0);
               }
                break;		
            case CON_ENCRYPT|CON_OLD1120ENCRPTY:
                 N101Encrptystyle = 4;
                 logSysMsgNoTime("�Ϻ���ũ����ȫ��������(SGC1120a)-101",0,0,0,0);
                 myTaskDelay(2);    
                rc = EncrptyChiptest(1);
                if(rc != 2)
                {
                    logSysMsgNoTime("���ܷ�����оƬ���Ͳ�ƥ������оƬ������",0,0,0,0);
                }
                 break;      
            default:
                //�޼���
                logSysMsgNoTime("�ް�ȫ��������(͸������)-101",0,0,0,0);
                break;               
            }
        }
    #endif
    
    FlagData1=FALSE;
    TxdStatus=TXDSEND;
    RxdStatus=RXDSTART;

    FrameHead=0;
    TxdHead=0;
    TxdTail=0;
    RxdHead=0;
    RxdTail=0;
    RxdLength=0;
    RetryFlag=0;
 
    TxdHead_Sec=0;
    TxdTail_Sec=0;
    FrameHead_Sec=0;
    
    FrameHead_Pri=0;	
    TxdHead_Pri=0;
    TxdTail_Pri=0;
    
    EbMsgRxdHead = 0;
    EbMsgRxdTail = 0;
    //EbMsgRxdLength = 0;
    
    //StartDL=0xff;
    //RemoteDLOK=0;
    //IsSendLinkInitCmd = TRUE;
    
    LastControl=0xff;
    ScanFlag=FALSE;

    wChanNo=wAppID;

    TimeOutTick=TimeOutTick_Pri=TimeOutTick_Sec=TimeOutTickCopy=0;
    En_LinkAddrSize = 0;
    En_CotSize = 0;
    En_PubAddrSize = 0;
    En_InfoAddrSize = 0;

    if(LinkAddrSize==1)
        FixFrmLength=5;
    else
        FixFrmLength=6;
    FirstRecFCB = TRUE;     //wjr  2009.6.3 ��·��λ��ǣ�����ʶ��·��λ���һ���յ��Է��Ĵ�FCBλ�ı��ģ����ԶԷ���FCB��ʼλΪ��ʼ��־
    return	TRUE;
}

//��MISI����
void CSecDLink::RecMISIData(void)
{
	RecMISIDataDealFun(N101Encrptystyle);
}

/********************************************************************
*�������ƣ�RecMISIDataDealFun
*���ܣ��б�ǰ��Լ���ܷ�ʽ���������Ļ�����������
*���룺Enflag:�������ͣ�2��ʾ1161�������ܣ�3��ʾ
*                  1120a����ũ�����ܣ�����Ϊ���Ļ�168���Ĵ���
*�з��ˣ�����
*********************************************************************/

void CSecDLink::RecMISIDataDealFun(INT8U Enflag)
{
    INT16U RNum,FrameTailFlag;
    INT16U tmpau,ennum;
	
    if((Enflag!=2)&&(Enflag!=3))//����ͨѶ��ʽ
    {
        if(RxdHead<RxdTail)    //���ջ�����������δ���������
        {
            if (RxdHead!=0)
            {
                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);//��δ����������Ƶ�������ͷ
                RxdTail-=RxdHead;
                RxdHead=0;
            }
        }
        else
            RxdHead=RxdTail=0;

        RNum=(INT16U)MisiRead(wChanNo,RxdBuf+RxdTail,FRAMEBUFSIZE-RxdTail,&FrameTailFlag);
        if (RNum>0)
        {
            RxdTail+=RNum;
            SearchFrame();
        }
    }
    else
    {
        if(EbMsgRxdHead<EbMsgRxdTail)    //���ջ�����������δ���������
        {
            if (EbMsgRxdHead!=0)
            {
                memcpy(EbMsgRxdBuf,EbMsgRxdBuf+EbMsgRxdHead,EbMsgRxdTail-EbMsgRxdHead);//��δ����������Ƶ�������ͷ
                EbMsgRxdTail-=EbMsgRxdHead;
                EbMsgRxdHead=0;
            }
        }
        else
        {
            EbMsgRxdHead=EbMsgRxdTail=0;
        }
        
        RNum=(INT16U)MisiRead(wChanNo, EbMsgRxdBuf+EbMsgRxdTail, FRAMEBUFSIZE-EbMsgRxdTail, &FrameTailFlag);
        EbMsgRxdTail+=RNum;
		
        if(RxdHead<RxdTail)    //���ջ�����������δ���������
        {
            if (RxdHead!=0)
            {
                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);//��δ����������Ƶ�������ͷ
                RxdTail-=RxdHead;
                RxdHead=0;
            }
        }
        else
        {
            RxdHead=RxdTail=0;
        }

		tmpau = RxdTail;
		
		if(Enflag == 2)//����1161��ȫ����
		{
		    ennum = EbSafetySearchFrame(&EbMsgRxdBuf[0], RxdBuf+RxdTail, &RxdTail, EbMsgRxdTail-EbMsgRxdHead, wChanNo);
		}
		else
		{
		    ennum = Eb1120aSafetySearchFrame(&EbMsgRxdBuf[0], RxdBuf+RxdTail, &RxdTail, EbMsgRxdTail-EbMsgRxdHead, wChanNo);
		}
		
        if(AuthEndflag != wChanNo)
        {
            memset(RxdBuf+tmpau,0,RxdTail-tmpau);
            RNum = 0;
            //EbErrCodeSend(0x9107,0x1f,wChanNo);
        }
        
        EbMsgRxdHead += ennum;
        
        if (RNum>0)
        {
            SearchFrame();
        }		        
    }
}

//���ͻ������е����ݣ�дMISI�ӿ�
BOOL CSecDLink::SendDataToMISI(void)
{
    if(BalanMode)
        myTaskDelay(5);    //ƽ��ģʽ�����ӳ�50ms
    else
        myTaskDelay(20);   //��ƽ��ģʽ�����ӳ�200ms
        
	return SendMISIDataDealFun(N101Encrptystyle);
}
/********************************************************************
*�������ƣ� SqrLongFraFunI
*�������ܣ� �����ṩ��101ԭʼ������Ϣ��������ԭʼ
*           ���ĵ�ң��/ң��������һ֡�϶̵�101����
*�������룺
*           ori:ԭʼ���Ļ�������
*           Bakbuf��֡���Ļ�����
*           infodataptr:��ǰ��֡��ң��ң�����ʼ��ַ 
*           ctoalen:  ������asdu������ַ����   
*           num: ��Ҫ��֡��ң��/ң�����
*           infolen:��Ϣ���ַ����
*           count:֡����
*�з��ˣ�����
*********************************************************************/
void CSecDLink::SqrLongFraFunI(INT8U *ori,INT8U *Bakbuf,INT8U *infodataptr,INT8U ctoalen,INT8U num,INT8U infolen,INT8U count,INT8U Sq) 
{
    INT8U lenth,len,infonum;
    INT8U *ptr;
	INT8U *paddr;
	INT16U addr;
    if(Sq == 0)
    {
        len = ctoalen+num*(En_InfoAddrSize+infolen);
    }
    else
    {
        len = ctoalen+((num*infolen) + En_InfoAddrSize); //ֻ��¼��ͷ����Ϣ���ַ
    }
    lenth = len+6;
    ptr = Bakbuf;
    ptr[0]=ptr[3]=0x68; 
    ptr[1]=ptr[2]=len;
    memcpy((ptr+4),ori+4,ctoalen);
    if(Sq == 0)
    {
        ptr[4+(1+En_LinkAddrSize+1)] = (num&0x7F);//vsq 
    }
    else
    {
        ptr[4+(1+En_LinkAddrSize+1)] = (num|0x80);//
    }       
    memcpy((ptr+4+ctoalen),infodataptr,len-ctoalen);  //ң�ŵ�ַ+ֵ
    ptr[lenth-1]=0x16;	
	if((Sq!=0)&&(count!=0))
	{
            paddr = (ptr-170);
	    addr = paddr[4+ctoalen];
    	    infonum = paddr[fixmlen+2];
    	    addr = addr+(infonum&0x7F);
           ptr[4+ctoalen+1] = paddr[4+ctoalen+1];
	    ptr[4+ctoalen] = (INT8U)(addr&0x00FF);//��Ϣ���ַ
           if(addr>0xFF)
           {            
               ptr[4+ctoalen+1]++;                       
           }
	}
    if(count == 1)
    {
        if((ptr[4]&0x20)==0)//fcb��ת
        {
            ptr[4]|=0x20;
        }
        else
        {
            ptr[4]&=0xDF;
        }               
    }               
}
/********************************************************************
*�������ƣ�SqrLongFraFun
*�������ܣ��жϵ�ǰ֡�Ƿ���Ҫ�������������ĳ��ȴ���255
*           �����������ķֳ�3֡����
*�������룺
*    ori:ԭʼ���Ļ�������
*    Bakbuf��֡���Ļ�����
*�з��ˣ�����
*********************************************************************/
INT8U CSecDLink::SqrLongFraFun(INT8U *ori,INT8U *Bakbuf)
{
    INT8U vsq,sq,num,len1,rc;
    ////INT8U *ptr;
    INT8U *infodataptr;
    ////INT16U addr;
    rc = 0;
    if((ori[0] == 0x68))
    {
        switch(ori[6 +1])
        {
            case 0x01:
            case 0x03:
                if(ori[1]>236)
                {
                    vsq = (ori[fixmlen+2]&0x7F);
                    sq = (ori[fixmlen+2]&0x80);   
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//������asdu������ַ����    9  
                    rc = 1;
                }
                else
                {
                    break;
                }
                if(sq == 0)
                {
                    num = vsq/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,(num+vsq%3),1,0,0); 
					infodataptr = infodataptr+(num+vsq%3)*(En_InfoAddrSize+1);
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,1,1,0);
                    infodataptr = infodataptr+num*(En_InfoAddrSize+1);
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,num,1,2,0);                                        
                }
				/*
                else
                {
                    num = (vsq&0xEF)/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,(num+vsq%3),1,0,1); 
					infodataptr = infodataptr+(num+vsq%3);
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,1,1,1);
                    infodataptr = infodataptr+num;
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,num,1,2,1);     
				}
				*/
                break;
            case 0x1E:
            case 0x1F:
                if(ori[1]>236)
                {
                    vsq = (ori[fixmlen+2]&0x7F);
                    sq = (ori[fixmlen+2]&0x80);   
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//������asdu������ַ����    9  
                    rc = 1;
                }
                else
                {
                    break;
                }
                if(sq == 0)
                {
                    num = vsq/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,(num+vsq%3),8,0,0); 
					infodataptr = infodataptr+(num+vsq%3)*(En_InfoAddrSize+8);
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,8,1,0);
                    infodataptr = infodataptr+num*(En_InfoAddrSize+8);
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,num,8,2,0);
                }
				/*
                else
                {
                    num = (vsq&0x7F)/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,(num+vsq%3),8,0,1); 
					infodataptr = infodataptr+(num+vsq%3)*8;
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,8,1,1);
                    infodataptr = infodataptr+num*8;
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,num,8,2,1);   
                }
                */
                break;
            case 0x09:
            case 0x0B:
                if(ori[1]>236)
                {
                    vsq = (ori[fixmlen+2]&0x7F);
                    sq = (ori[fixmlen+2]&0x80);   
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//������asdu������ַ����    9  
                    rc = 1;
                }
                else
                {
                    break;
                }
                if(sq == 0)
                {
                    num = vsq/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,(num+vsq%3),3,0,0); 
					infodataptr = infodataptr+(num+vsq%3)*(En_InfoAddrSize+3);
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,3,1,0);
                    infodataptr = infodataptr+num*(En_InfoAddrSize+3);
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,num,3,2,0);           
                }
                else
                {
                    num = (vsq&0x7F)/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,num,3,0,1); 
					infodataptr = infodataptr+num*3;
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,3,1,1);
                    infodataptr = infodataptr+num*3;
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,(num+vsq%3),3,2,1);                       
                }
                break;
            case 0x0D:
                if(ori[1]>236)
                {
                    vsq = (ori[fixmlen+2]&0x7F);
                    sq = (ori[fixmlen+2]&0x80);   
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//������asdu������ַ����    9  
                    rc = 1;
                }
                else
                {
                    break;
                }
                if(sq == 0)
                {
                    num = vsq/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,(num+vsq%3),5,0,0); 
					infodataptr = infodataptr+(num+vsq%3)*(En_InfoAddrSize+5);
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,5,1,0);
                    infodataptr = infodataptr+num*(En_InfoAddrSize+5);
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,num,5,2,0);           
                }
                else
                {
                    num = (vsq&0x7F)/3;
                    infodataptr = (ori+4+len1);
                    SqrLongFraFunI(ori,Bakbuf,infodataptr,len1,num,5,0,1); 
					infodataptr = infodataptr+num*5;
                    SqrLongFraFunI(ori,Bakbuf+170,infodataptr,len1,num,5,1,1);
                    infodataptr = infodataptr+num*5;
                    SqrLongFraFunI(ori,Bakbuf+340,infodataptr,len1,(num+vsq%3),5,2,1);      
                }
                break;
            default:
                break;
        }
    }
    return rc;
 }
/********************************************************************
*�������ƣ�RecMISIDataDealFun
*���ܣ��б�ǰ��Լ���ܷ�ʽ���������Ļ�����������
*���룺Enflag:�������ͣ�2��ʾ1161�������ܣ�3��ʾ
*                  1120a����ũ�����ܣ�����Ϊ���Ļ�168���Ĵ���
*�з��ˣ�����
*********************************************************************/

BOOL CSecDLink::SendMISIDataDealFun(INT8U Enflag)
{
    INT16U SendLen,rc,eb101sendlen,length,i;
    INT8U *ptr;
    IdleTimeCount = 0;
    HeartBeatIdleTime = 0;
    ////memcpy(TxdBuf,bufttt,255);/////
    ////AuthEndflag = 2;

    if((Enflag!=2)&&(Enflag!=3)&&(Enflag!=4))//����ͨѶ��ʽ
    {
        if (TxdStatus==TXDSEND)//�����ݷ���
        {
            TxdHead=0;
            SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf,TxdTail-TxdHead,3);
            if (SendLen>0)//�����������ͣ���û�껹δ�жϣ�
                TxdStatus=TXDWAIT;
        }
        else//�ϴ�����δ�����꣬
        {
            SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf+TxdHead,TxdTail-TxdHead,1);
        }
    }
    else if(Enflag == 4)//��ũ������
    {
        if (TxdStatus==TXDSEND)//�����ݷ���
        {
            TxdHead=0;
            if((AuthEndflag!=wChanNo)&&(TxdBuf[0]==0x68))//��֤���֮ǰ������ҵ����
            {  
                switch(TxdBuf[7])//ң�ţ�ң�⣬ң�أ���Ϣ�ϱ���Ҫ����
                {
                    case 0x01:
                    case 0x03:
                    case 0x1E:
                    case 0x1F:
                    case 0x09:
                    case 0x0B:
                    case 0x0D:
                        memset(TxdBuf,0,TxdTail-TxdHead);
                        TxdHead = TxdTail = 0;
                        return 0;
                        //break;
                    default:
                        break;
                }
            }
            if((AuthEndflag==wChanNo)&&(TxdBuf[0]==0x68))
            {
                rc = SqrLongFraFun(TxdBuf,EbMsgTxdBuf);
                if(rc == 1)
                {
                    for(i=0;i<3;i++)
                    {
                        ptr = EbMsgTxdBuf+(i*170);
                        Packf68ToOld1120aEn(ptr);
                        if(ptr[0] == 0x69)
                        {
                            length = ptr[1]+6;
                            ptr[length-2] = CheckSum(ptr);
                            ptr[length-1] = ENDCODE;
                        }
                        SendLen=(INT16U)MisiWrite(wChanNo,ptr,length,3);
                    }
                    memset(TxdBuf,0,TxdTail-TxdHead);
                    TxdHead = TxdTail = 0;
                    return 0;
                }
                Packf68ToOld1120aEn(TxdBuf);
                if(TxdBuf[0] == 0x69)
                {
                    length = TxdBuf[1]+6;
                    TxdBuf[length-2] = CheckSum(TxdBuf);
                    TxdBuf[length-1] = ENDCODE;
                    ////saveRecord(EbMsgTxdBuf,length,TXSAVEMODE,1);
                    TxdTail = length;
                }
            }
            SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf,TxdTail-TxdHead,3);
            if (SendLen>0)//�����������ͣ���û�껹δ�жϣ�
                TxdStatus=TXDWAIT;
        }
        else//�ϴ�����δ�����꣬
        {
            SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf+TxdHead,TxdTail-TxdHead,1);
        }
    }
    else
    {
 	    if(AuthEndflag != wChanNo)//&&TxdBuf[0] != 0xEB)
		{
			memset(TxdBuf,0,TxdTail-TxdHead);
			TxdHead = TxdTail = 0;
			return 0;
		}
		
		if (TxdStatus==TXDSEND)//�����ݷ���
		{
			TxdHead=0;
			eb101sendlen = 0;
			memcpy(EbMsgTxdBuf,TxdBuf,(INT16U)(TxdTail-TxdHead));
			
			if(Enflag == 2)
			{
			    rc = Pack101msgtoEb(EbMsgTxdBuf,(INT16U)(TxdTail-TxdHead),&eb101sendlen,wChanNo);
			}
			else
			{
			    rc = Pack1120aFor101msgtoEb(EbMsgTxdBuf,(INT16U)(TxdTail-TxdHead),&eb101sendlen,wChanNo);
			}
			
			if(rc == 0)
			{
				return 0;
			}
			if((eb101sendlen - rc) > (TxdTail - TxdHead))//
			{
				return 0;
			}	
					
			SendLen=(INT16U)MisiWrite(wChanNo,EbMsgTxdBuf,rc,3);
			
			if(SendLen!=0xffff)
			{
				SendLen = TxdTail - (eb101sendlen - rc);
			}
			
	        UpgradeDataVerify(wChanNo);
	        
	        if (SendLen>0)//�����������ͣ���û�껹δ�жϣ�
				TxdStatus=TXDWAIT;	
	    }   
		else//�ϴ�����δ�����꣬
		{
			SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf+TxdHead,TxdTail-TxdHead,1);
		}
    }
    
    if(SendLen==0xffff)//д���󣬷���-1
    {
        SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf,0,1);//����֡β
        TxdStatus=TXDSEND;
        if(BalanMode)
        {
            myEventSend(myTaskIdSelf(),SENDOVER);
        }
        return TRUE;
    }
    else
    {
        TxdHead+=SendLen;
        if(TxdHead>=TxdTail)//�ô����������Ѿ����ꡣ
        {
            TxdStatus=TXDSEND;
            if(BalanMode)
            {
                myEventSend(myTaskIdSelf(),SENDOVER);
            }
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}

void CSecDLink::SendPriDataToMISI(void)//
{
    
    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;

    memcpy(TxdBuf+FrameHead,TxdBuf_Pri,TxdTail_Pri);
    TxdTail+=TxdTail_Pri;

    SendDataToMISI();
}

void CSecDLink::SendSecDataToMISI(void)
{
    
    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;

    memcpy(TxdBuf+FrameHead,TxdBuf_Sec,TxdTail_Sec);
    TxdTail+=TxdTail_Sec;

    SendDataToMISI();
}
/*------------------------------------------------------------------/
�������ƣ�  SendTestOnTime()
�������ܣ�  ��ʱ������·��Ĳ���֡��������
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecDLink::SendTestOnTime(void)
{
    if(DLSecStatus==SECDISABLE)
    {
        //δ�������Ӳ�ִ������ 
        HeartBeatIdleTime = 0;
        return;
    }
    
    if(HeartBeatIdleLimit == 0) //HeartBeatIdleLimitΪ0��ʾ����Ҫ��������
        return;  
    
    HeartBeatIdleTime++;
    if(HeartBeatIdleTime >= HeartBeatIdleLimit)
    {
        HeartBeatIdleTime = 0;
        
        EditFra10(DLTESTDL);  
        SendDataToMISI();
        
        DLPriStatus=PRISENDCON;
        
        TimeOutTick_Pri = TimeDelay(2* FixFrmLength);
        RetryCount      = MAXRETRYNUM; 
    }

}

/*------------------------------------------------------------------/
�������ƣ�  TimeOut()
�������ܣ�  ��ʱ������
����˵����  
���˵����  ÿ������һ�Σ���Ӧ�ò����
/------------------------------------------------------------------*/
void CSecDLink::TimeOut(void)
{
    //logSysMsgNoTime("1 PriTime=%d, Sectime=%d,",TimeOutTick_Pri,TimeOutTick_Sec,0,0);//dubug ll
    if(TimeOutTick_Pri)//��λ����
    {
        TimeOutTick_Pri--;
        if(!TimeOutTick_Pri)
        {
            TimeOutFun(1,1);
        }
    }

    if(BalanMode)//ƽ��ģʽ�£�Ϊ������·�������ڼ�ʱ����Ӧ�ò��һ�����������ڳ����趨�Ŀ���ʱ���ÿ����Ӧ�ò�һ�Ρ�
    {
        SendTestOnTime();   
        EndlessLoopMonitor();
        IdleTimeCount++;
        if((IdleTimeCount>=DLIdleTime)&&(DLPriStatus==PRIENABLE))//DLIdleTime��С����С��2
        {
            DLCommand=DL_SCAN1S;
            LengthIn=0;

            //Ӧ�ò����
            pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
            DLSendProc();
            //logSysMsgNoTime("����ʱ��ɨ��Ӧ�ò�",0,0,0,0);//  debug ll
        }
        
    }
    
    //�Ӷ�״̬Ϊ��ֹ����·��ʱ��ʼ��������Ҳ��Ϊ�Ӷ�����֪ͨ��·������Ӷ�״̬
    if(BalanMode &&(DLSecStatus == SECDISABLE))
    {
        if((pSecApp->APP_DLSecStatus == SECDISABLE) && (pSecApp->DLInitFinishFlag == TRUE))
        {
            pSecApp->APP_DLSecStatus = SECENABLE;
            DLSecStatus = SECENABLE;
            logSysMsgNoTime("�Ӷ�����ʱ��Ϊ����",0,0,0,0);
        }
    }
    
}

void CSecDLink::TimeOutFun(INT8U FailType,INT8U Prm)//��ʱ����
{
    if(!RetryCount)
        return;
    RetryCount--;
    
    if(RetryCount)
    {
        if(Prm)//����վ���ĳ�ʱδ�յ�Ӧ��
        {
            //logSysMsgNoTime("SEC ��ʱ�ط�,",0,0,0,0);//dubug ll
            SendPriDataToMISI();
            TimeOutTick_Pri=TimeOutTickCopy;
        }
        else//������ʵ���в���
        {
            SendSecDataToMISI();
            TimeOutTick_Sec=TimeOutTickCopy;
        }
        return;
    }
    else//�����ط����������½�����·����֪ͨӦ�ò㡣
    {
        if(BalanMode)
        {
            //logSysMsgNoTime(" Pri=%x, Sec=%x, Time=%d,R=%d,",DLPriStatus,DLSecStatus,TimeOutTick_Pri,RetryCount);//dubug ll
            
            //if(gxceshi == 0)
             //   CallDLStatus();
            
            //IsSendLinkInitCmd = TRUE;
            DLSecStatus = SECDISABLE;   //��·��Ӷ��������־λ������������Ϊ�ǳ�ʱ�ط���Ӧ��Ҳ��������̬����������塣

            DLCommand=DL_LINKDISABLE;
            LengthIn=0;
            pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
        }
    }
}


//������·����
void CSecDLink::CallDLStatus(void)
{
    //if(pSecApp->GYKZ2015Flag)  //����ǹ���Ҫ����չ��Լ����ô������������·��ʼ��
    //    return;
        
    //logSysMsgNoTime("4 CallDLStatus����������·����",0,0,0,0);   //  debug ll
    EditFra10(DLREQSTATUS);
    SendDataToMISI();

    DLPriStatus=PRIWAITSTATUS;
    
    TimeOutTick_Pri = TimeDelay(2* FixFrmLength);
    RetryCount=MAXRETRYNUM;
    
   
}

void CSecDLink::SearchFrame(void)
{
    BOOL Stop=FALSE;
    INT8U length;
    INT8U rc;
    
    //INT8U L2, L3;
    while ((RxdHead<RxdTail)&&(!Stop))
    {
        if(RxdStatus==RXDSTART)
        {
        #ifdef INCLUDE_ENCRYPT
            if(IsEncrypt)
            {
                while ((RxdBuf[RxdHead]!=STARTCODE68)
                        &&(RxdBuf[RxdHead]!=STARTCODE10)
                        &&(RxdBuf[RxdHead]!=MAINTSTARTCODE)
                        &&(RxdBuf[RxdHead]!= 0x16)
                        &&(RxdHead<RxdTail))
                {
                    RxdHead++;
                }
            }
            else
        #endif
            {
                while ((RxdBuf[RxdHead]!=STARTCODE68)
                        &&(RxdBuf[RxdHead]!=STARTCODE10)
                        &&(RxdBuf[RxdHead]!=MAINTSTARTCODE)
                        &&(RxdHead<RxdTail))
                {
                    RxdHead++;
                }
                
            }
            
            if (RxdHead<RxdTail)//�ҵ������ַ������������뻺�������롣
            {
                
                RxdStatus=RXDHEAD;
                if (RxdHead!=0)
                {
                    memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);
                    RxdTail-=RxdHead;
                    RxdHead=0;
                }
            }
            
        #ifdef INCLUDE_ENCRYPT
            
            RxdLength=RxdTail-RxdHead;
            if((RxdBuf[RxdHead] == 0x16)&& IsEncrypt )    //���ܱ��Ĵ���&& IsEncrypt
            {
                
                rc = CheckEncryptFramHead(&RxdBuf[RxdHead], RxdLength);
                switch(rc)
                {
                case 0: //���ļ����ȷ
                    RxdHead+=(RxdBuf[RxdHead+1]+6);//ͷָ����Ƶ����ĺ�
                    RxdStatus=RXDSTART;
                    Stop=TRUE;
                    //logSysMsgWithTime("�յ�ͨѶ���ܱ��Ŀ�ʼ����",0,0,0,0);   //debug
                    ExeDLFunCode16();
                    //logSysMsgWithTime("���Ĵ�����ϱ���111",0,0,0,0); //debug
                    break; 
                case 1: //֡ͷ����
                    RxdHead++;
                    RxdStatus=RXDSTART;
                    break;
                case 2: //δ����ȫ
                    Stop=TRUE;
                    RxdStatus=RXDSTART;
                    break; 
                case 3: //У��͡���ǩ����
                    RxdHead+=6;
                    //RxdHead++;
                    RxdStatus=RXDSTART;
                    break;
                  
                default:
                    //RxdHead++;
                    RxdStatus=RXDSTART;
                    break;    
                }
                
            }
        #endif //#ifdef INCLUDE_ENCRYPT    
        }
        RxdLength=RxdTail-RxdHead;
        if(RxdStatus==RXDHEAD)//���Ĺ�һ֡��
        {
            switch (RxdBuf[RxdHead])
            {
                
                case STARTCODE68:
                    if (RxdLength>=5+LinkAddrSize)//�����ַ�����·��ַ��6�ֽڡ�
                    {
                        if (!ExeDLFun68())//�ж��Ƿ���ͷ
                        {
                            RxdHead++;
                            RxdStatus=RXDSTART;
                        }
                        else
                        {
                            
                            RxdStatus=RXDCONTINUE;
                        }
                    }
                    else
                        Stop=TRUE;
                    break;
                case STARTCODE10:
                    if (RxdLength>=4+LinkAddrSize)
                    {
                        if (!ExeDLFun10())
                        {
                            RxdHead++;
                            RxdStatus=RXDSTART;
                        }
                        else
                            RxdStatus=RXDCONTINUE;
                    }
                    else
                        Stop=TRUE;
                    break;
                case MAINTSTARTCODE:
                    if (RxdLength>=3)
                    {
                        if((RxdBuf[RxdHead+1]!='S')||(RxdBuf[RxdHead+2]!='T'))
                        {
                            RxdHead++;
                            RxdStatus=RXDSTART;
                        }
                        else
                        {
                            if (RxdHead>0)
                            {
                                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);
                                RxdTail-=RxdHead;
                                RxdHead=0;
                            }
                            RxdStatus=RXDCONTINUE;
                        }
                    }
                    else if (RxdLength==2)
                    {
                        if (RxdBuf[RxdHead+1]!='S')
                        {
                            RxdHead++;
                            RxdStatus=RXDSTART;
                        }
                        Stop=TRUE;
                    }
                    else
                        Stop=TRUE;
                    break;
            }
        }
        if(RxdStatus==RXDCONTINUE)
        {
            switch (RxdBuf[RxdHead])
            {
                
                case STARTCODE68:
                    length=RxdBuf[RxdHead+1];
                    if (RxdLength>=length+6)//������ȫ�����д���
                    {
                        //logSysMsgNoTime("�յ�68����",0,0,0,0);//debug
                        if ( (RxdBuf[RxdHead+length+4]==CheckSum(RxdBuf+RxdHead))//У����ȷ
                                &&(RxdBuf[RxdHead+length+4+1]==ENDCODE))//��������ȷ
                        {
                            if (RxdHead>0)
                            {
                                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);//�����뻺��������
                                RxdTail-=RxdHead;
                                RxdHead=0;
                            }
                            
                        #ifdef INCLUDE_ENCRYPT    
                            //�Ƿ�֧�ּ��� RxdBuf+5+LinkAddrSize
                            if(IsEncrypt && 
                                ((RxdBuf[5+LinkAddrSize]==C_SC_NA) || (RxdBuf[5+LinkAddrSize]==C_DC_NA))
                              )
                            {
                                if(RxdLength>=length+10) 
                                {
                                    if((RxdBuf[length+6] != 0x16) || (RxdBuf[length+9] != 0x16))
                                    {
                                        logSysMsgNoTime("����֤����ͷ����������",0,0,0,0);
                                        RxdStatus=RXDSTART;
                                        //����֤����ͷ����������
                                        RxdHead+=(RxdBuf[1]+6);
                                        EditSecFra10(DLCON);
                                        SendDataToMISI();
                                        if (RxdTail>RxdHead)
                                            myEventSend(myTaskIdSelf(),NEXTFRAME);
                                        
                                        
                                    }
                                    else if (RxdLength>=length+6+RxdBuf[length+7]+6)//������ȫ�����д���
                                    {
                                        RxdStatus=RXDSTART;
                                        
                                        //logSysMsgWithTime("�յ����Ŀ�ʼ����222",0,0,0,0);   //debug 
                                        rc = CheckEncryptFrame(&RxdBuf[length+6], RxdBuf, length+6);
                                        if(rc == 0)
                                        {
                                           
                                            RxdHead+=(length+6+RxdBuf[length+7]+6);//ͷָ����Ƶ����ĺ� 
                                            //RxdStatus=RXDSTART;
                                            logSysMsgWithTime("�յ�����ң�ر���,��ȫ��֤������ȷ.", 0,0,0,0);
                                            ExeDLFunCode68();
                                            //logSysMsgWithTime("���Ĵ������222",0,0,0,0);   //debug
                                            if (RxdTail>RxdHead)
                                                myEventSend(myTaskIdSelf(),NEXTFRAME);
                                            
                                        }
                                        else
                                        {
                                            RxdHead+=(RxdBuf[1]+10);
                                            logSysMsgNoTime("��ȫ����У��(У���,ǩ��,ʱ���)����ȷ.rc=%d",rc,0,0,0);
                                        }
                                        
                                    }
                                    
                                }
                                                                
                                Stop=TRUE;
                                
                            }
                            else    //�������ʱ�Ĵ���
                         #endif
                            {
                                
                                RxdHead+=(RxdBuf[1]+6);//ͷָ����Ƶ����ĺ�
                                RxdStatus=RXDSTART;
                                Stop=TRUE;
    
                                ExeDLFunCode68();
                                
                                if (RxdTail>RxdHead)
                                    myEventSend(myTaskIdSelf(),NEXTFRAME);
                            }
                        }
                        else
                        {
                            //logSysMsgNoTime("У��Ͳ��ԣ�head=%d,cs=%d, length=%d",RxdHead,RxdBuf[RxdHead+length+4+1],RxdBuf[RxdHead+1],0);//debug
                            RxdHead+=6;
                            RxdStatus=RXDSTART;
                        }
                    }
                    else//����δ��ȫ�������������ȴ����ݡ�
                        Stop=TRUE;
                    break;
                case STARTCODE10:
                    if (RxdHead>0)
                    {
                        memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);
                        RxdTail-=RxdHead;
                        RxdHead=0;
                    }
                    RxdHead+=4+LinkAddrSize;
                    RxdStatus=RXDSTART;
                    Stop=TRUE;
                    ExeDLFunCode10();
                    if (RxdTail>RxdHead)
                        myEventSend(myTaskIdSelf(),NEXTFRAME);
                    break;
                case MAINTSTARTCODE:
                    if (RxdLength>=30)
                    {
                        if (!ExeMaint())
                        {
                            RxdHead++;
                            RxdStatus=RXDSTART;
                        }
                        else
                        {
                            if (RxdHead>0)
                            {
                                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);
                                RxdTail-=RxdHead;
                                RxdHead=0;
                            }
                            RxdHead+=30;
                            RxdStatus=RXDSTART;
                            Stop=TRUE;

                            if (RxdTail>RxdHead)
                                myEventSend(myTaskIdSelf(),NEXTFRAME);
                        }
                    }
                    else
                        Stop=TRUE;
                    break;
            }
        }
  	}
}

//���10֡��ȷ��
BOOL CSecDLink::ExeDLFun10(void)
{
    INT8U *p;
    INT16U addr=0;

    p=(RxdBuf+RxdHead);
    if(p[2+LinkAddrSize]!=CheckSum(p))
        return(FALSE);
    if(p[3+LinkAddrSize]!=ENDCODE)
        return(FALSE);
    if(LinkAddrSize==1)
        addr=p[2];
    else
        addr=(p[2]+(p[3]<<8));
    
#ifdef  PRO101_ADDR_STUDY
    struct PortInfo_t *PI;
    
    if(BalanMode)   //ll ����վַ��ѧϰ
    {
        
        if((p[1]&P101_FUNCODE) ==DLREQSTATUS)
        {
            if(SourceNo != addr)
            {
                GetPortInfo(wAppID,&PI);

                SourceNo = addr;
                pSecApp->DevList[0].Addr = addr; 
                StudyAddrSetAddr(SourceNo, PI->DevIDSet[0]);
            }
        }       
    }
#endif
   
    if(addr!=SourceNo)
        return(FALSE);
    return(TRUE);
}

//�ж�68֡ͷ��
BOOL CSecDLink::ExeDLFun68(void)
{
    INT8U *p;
    p=(RxdBuf+RxdHead);
    if((p[0]!=p[3])||(p[1]!=p[2]))
        return(FALSE);
    if(LinkAddrSize==1)
    {
        if((p[5]!=SourceNo)&&(p[5]!=0xff))
            return(FALSE);
    }
    else
    {
        if(((p[5]+(p[6]<<8))!=SourceNo)&&((p[5]+(p[6]<<8))!=0xffff))
            return(FALSE);
    }
    return(TRUE);
}


void CSecDLink::ExeDLFunCode10(void)
{
    INT8U Stop=0;
    INT8U Control;

    Control = RxdBuf[1];
    IdleTimeCount = 0;
    HeartBeatIdleTime = 0;
    
    
    if(Control&P101_PRM)//��վ��Ϊ�Ӷ�վ������վ������
    {
        
        //�Ӷ���·ִ�в���
        if(DLSecStatus==SECDISABLE) //����·״̬δ��Ч����λ��ʱ
        {
            if(((Control&P101_FUNCODE)!=DLREQSTATUS) 
                && ((Control&P101_FUNCODE)!=DLRESETRDL))   //���ش��κη�FC9��FC0�ı���
            {
                return ;   
            }
        }
        
        //ƽ��ģʽ�£��յ�FC10��FC11���������
        if(BalanMode)   
        {
            if(((Control&P101_FUNCODE)==DLREQDATA1) 
                || ((Control&P101_FUNCODE)==DLREQDATA2))   
            {
                return ;   
            }
        }
        //��ƽ��ģʽ�£��յ�FC2��������
        if(!BalanMode)
        {
            if((Control&P101_FUNCODE)==DLTESTDL)
                return;
        }
        //logSysMsgNoTime("SEC prm=1,�յ�����%d",Control&P101_FUNCODE,0,0,0);   //  debug ll 
        if ((!(Control&P101_FCV)) || ((Control&P101_FCB) !=((RlaConCode) & 0x20)) || (FirstRecFCB==TRUE))
        {
            if (Control&P101_FCV)//FCV��Ч����¼FCB״̬��
            {
                RlaConCode=RxdBuf[1];
                if(NoJudgeFCB == FALSE)
                {
                    if(FirstRecFCB==TRUE)
                        FirstRecFCB=FALSE;   
                }
                
            }
            switch(Control&P101_FUNCODE)
            {
                case DLREQSTATUS://FC9 �ٻ���·״̬
                    RecReqDLStatus();
                    while((RxdTail-RxdHead>=FixFrmLength)&&(Stop==0))//�յ��ٻ���·״̬����ʱ�����������ͬ���
                    {//���Ա��������ʵĹ��̣����콨����·��
                        if(memcmp(RxdBuf,RxdBuf+RxdHead,FixFrmLength)==0)
                            RxdHead+=FixFrmLength;
                        else
                            Stop=1;
                    }
                    break;
                case DLRESETRDL://FC0 ��λ��·
                    RecResetDL();
                    break;
                case DLTESTDL:  //FC2, ��·���ԣ� ll 2010/07/20 for��������
                    //ƽ��ģʽ���ܵ�������
                    RecTestDL();
                    break;
                case DLREQDATA1://FC10
                    //��ƽ��ģʽ���ܵ�����
                    DLCommand=DL_CALLDATA1;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    LengthIn=0;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);

                    DLSendProc();
                    
                    break;
                case DLREQDATA2://FC11
                    //��ƽ��ģʽ���ܵ�����
                    DLCommand=DL_CALLDATA2;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    LengthIn=0;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    
                    break;
                default:    //������FC0,FC2,FC9,FC10,FC11�⣬����FC�����
                    break;
            }
            return;
        }
        else //FCBδ��ת
        {
            //FCBδ��ת����ƽ��ģʽ���յ�FC2�ı��ģ���ʱ��ȡ�ط�����,һ�㷢�Ķ���ȷ�����
            //FCBδ��ת���ڷ�ƽ��ģʽ���յ�FC10/FC11�ı��ģ���ʱ��ȡ�ط����ԡ�
            SendSecDataToMISI();
            //logSysMsgNoTime("SEC 10֡δ��ת�ط�",0,0,0,0);   //  debug ll
            
        }
    }
    else//Prm=0 ƽ��ģʽ��վ��Ϊ����վ���յ���վ�����
    {
        //logSysMsgNoTime("SEC prm=0,�յ�����%d",Control&P101_FUNCODE,0,0,0);   //  debug ll
        switch(Control&P101_FUNCODE)
        {
            case DLCON:	//FC0 ȷ��
                RecConf10(Control);
                break;
            case DLSTATUSOK://FC11 Ӧ����·
                RecDLSta(Control);
                break;
            case DLNOWORK:  //FC14
            case DLNOFIN:   //FC15
                //����״̬�£��յ�FC14/FC15ʱ����ԭ״̬����
                /*if(DLPriStatus==PRISENDCON) //�ڵȴ��Է�ȷ��״̬ʱ���յ�FC14/FC15������·�����״̬
                {
                    DLPriStatus = PRIENABLE;
                    TimeOutTick_Pri = 0;
                }*/
                
                //ָʾӦ�ò㣬��·��δ�����ú�δ��ɡ�(Ӧ�ò���δ���)
                break;
            case DLNOCON:   //FC1 ��ȷ��Ӧ��
                //ֻ����PRIWAITRSCON״̬�£��յ�FC1������ȷ�ģ���ʱ���򲻸ı䵱ǰ״̬
                //����״̬���յ�FC1�����쳣���, ������Ӧ��    
                break;    
            default:
                
                break;
        }
    }
}
#ifdef INCLUDE_ENCRYPT
void CSecDLink::ExeDLFunCode16(void)
{
    INT8U len;
    
    if(DLSecStatus==SECDISABLE) //����·״̬δ��Ч����λ��ʱ, �������κ�68֡��ƽ���ƽ��ģʽͬ������
    {
        logSysMsgNoTime("ͨѶ����-��վδ������·����ִ������",0,0,0,0);
        return ;   
    }
    
    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;
    	
    ProcEncryptFrame(RxdBuf, TxdBuf+FrameHead, &len);
    
    if(len)
    {
        TxdTail += len;
        SendDataToMISI();
    }
    
    
}
#endif
void CSecDLink::ExeDLFunCode68(void)
{
    INT8U Control;
    //logSysMsgNoTime("SEC �յ�68֡���ĴӶ�״̬%d",DLSecStatus,0,0,0);   //  debug ll
    if(DLSecStatus==SECDISABLE) //����·״̬δ��Ч����λ��ʱ, �������κ�68֡��ƽ���ƽ��ģʽͬ������
    {
        return ;   
    }
    	
    Control=RxdBuf[4];
    IdleTimeCount = 0;
    HeartBeatIdleTime = 0;
    if(Control&P101_PRM)//��վ��Ϊ�Ӷ�վ������վ��·���ġ�
    {
        if ((!(Control&P101_FCV)) || ((Control&P101_FCB) !=((RlaConCode) & 0x20)) || (FirstRecFCB==TRUE))
        {
            if (Control&P101_FCV)//FCV��Ч����FCB��λ
            {
                RlaConCode=RxdBuf[4]; //�������µĹ����룬����ֻ�õ�FCB
                if(NoJudgeFCB == FALSE)
                {
                    if(FirstRecFCB==TRUE)
                        FirstRecFCB=FALSE;   //wjr 2009.6.3   �յ�һ��68֡�����ݺ�ͽ��ñ�־���
                }
            }
            	
            switch(Control&P101_FUNCODE)
            {
                case DLRESETUSE://1������վ��λ�Ӷ�վ�û�����
                    
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    EditSecFra10(DLCON);
                    SendDataToMISI();
                    
                    break;
                case DLTESTDL://2��������·
                                        
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    EditSecFra10(DLCON);
                    SendDataToMISI();
                    
                    break;
                case DLSENDCON://3������/ȷ�� ����
                    DLCommand=DL_SENDCON;//
                    
                    //EditSecFra10(DLCON);
                    //SendDataToMISI();
                    //logSysMsgNoTime("�յ�68֡, RxdBuf=%x, lenth=%d,pub=%d, cot=%d",RxdBuf[0],RxdBuf[1],pSecApp->PubAddrLocation,pSecApp->CotLocation);   //  debug ll
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);  //����SecAppProc������λ��Ӧ�ı�־�������������ݡ�
                    //logSysMsgNoTime("�յ�68֡��׼������%d��lenth=%d",AppCommand,LengthOut,0,0);   //  debug ll
                    DLSendProc();   //����������ȷ��֡
                    //logSysMsgNoTime("�յ�68֡ ����ȷ��",0,0,0,0);   //  debug ll
                    EditSecFra10(DLCON);    //����ȷ��֡
                    SendDataToMISI();
                    
                    break;
                case DLSENDNOCON://4������/�޻ش� ����
                    DLCommand=DL_SENDNOCON;//
                    
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    break;
                default:    //������FC1,FC2,FC3,FC4�⣬����FC�����
                    break;
            }
        }
        else  //FCB δ��ת  //change by wyj
        {
            SendSecDataToMISI();
          
        }
    }
    else  //Prm=0����վ��Ϊ����վ������վ��·����
    {
    }
    
}

INT8U CSecDLink::CheckSum(INT8U *RxdBuf)
{
    INT8U sum=0;
    int j;
    if(*(RxdBuf)==STARTCODE10)
    {
        for(j=0;j<LinkAddrSize+1;j++)
            sum+=*(RxdBuf+1+j);
    }
    else
    {
        INT8U *p;
        INT8U i;
        p=RxdBuf+4;
        i=*(RxdBuf+1);

        while(i--)
            sum += *p++ & 0xFF;
    }
    return(sum);
}

INT32U CSecDLink::TimeDelay(INT16U i)
{
    TimeOutTickCopy = ((INT32U)i*10/baudrate) + (TimeOutValue/100)+1;
    return(TimeOutTickCopy);
}
/*------------------------------------------------------------------/
�������ƣ�  RecDLSta()
�������ܣ�  ��Ӧ��ѯ��·
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecDLink::RecDLSta(INT8U Control)
{
    //FC11ֻ����PRIWAITSTATUS���״̬�³��ֲ���Ч�����������쳣���
    if(DLPriStatus == PRIWAITSTATUS)
    {
        //if((Control&P101_DFC)==0)   //��PRIWAITSTATUS״̬�£��յ�DFC=1��Ӧ��Ӧ�޷�Ӧ
        {
            EditFra10(DLRESETRDL);
            SendDataToMISI();
            
            TlaConCode &= 0xDF;  
            DLPriStatus=PRIWAITRSCON;
            //logSysMsgNoTime("���͸�λ�����",0,0,0,0);   //  debug ll
            TimeOutTick_Pri=TimeDelay(2* FixFrmLength);
            RetryCount=MAXRETRYNUM;
        }
        
    }
    //����״̬�£��յ�FC11�����������Ĵ�������Ӧ
    
}
/*------------------------------------------------------------------/
�������ƣ�  RecConf10()
�������ܣ�  ƽ��ģʽ��վ��Ϊ����վ���յ���վ��ȷ�ϱ���
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecDLink::RecConf10(INT8U Control)
{
    //FC0 ֻ����PRIWAITRSCON��PRISENDCON״̬�³��ֲ�����ȷ�ģ�����Ϊ�쳣���
    
    if (DLPriStatus==PRIWAITRSCON)
    {
        //logSysMsgNoTime("����������·ȷ��",0,0,0,0);    //dubug ll
        DLPriStatus=PRIENABLE;
        if(IsDoubleLinkInit == FALSE)   //����˫������·�����ôӶ�ҲΪ����Ӧ�ò����·�㣩��
        {
            DLSecStatus=SECENABLE; 
            pSecApp->APP_DLSecStatus = SECENABLE;
            
            RlaConCode &= 0xDF;     //��FCB  
            FirstRecFCB = TRUE;
        }
        TimeOutTick_Pri = 0;  //�յ�ȷ�����������λ���ʱʱ��
        
        //֪ͨӦ�ò㣬��ǰ��·״̬����
        DLCommand=DL_LINKENABLE;
        LengthIn=0;
        pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                            TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
        DLSendProc();
        
       
    }
    else if (DLPriStatus==PRISENDCON)//ƽ��ģʽ
    {
        //if((Control&P101_DFC)==0)
        {
            DLPriStatus = PRIENABLE;
            //logSysMsgNoTime("RecConf10=%d",DLPriStatus,0,0,0);   //  debug ll
            TimeOutTick_Pri = 0;
            //Ӧ�ò����
            DLCommand=DL_APPCON;
            LengthIn=0;
            pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
            DLSendProc();
        }
    }
    
    //����״̬���յ�FC0�����쳣�������Ӧ��
}

//�յ���λ��·����
void CSecDLink::RecResetDL(void)
{
    //��ֹ������ѭ����� ll 2014-8-11
    if(ELReplyFlag) //ELReplyFlag ����ʱ����1
    {
        ELClearTime = EL_NOREPLY_TIME;
        ELResetNum++;
    }
    
    DLSecStatus = SECENABLE;//��վ��·����
    if(IsDoubleLinkInit == FALSE)   //����˫������·���Ӷ�����������������ҲΪ����Ӧ�ò����·�㣩��
    {
       DLPriStatus = PRIENABLE; 
       pSecApp->APP_DLPriStatus = PRIENABLE;
    }
    
    RlaConCode &= 0xDF;     //��FCB  
        
    FirstRecFCB = TRUE;     //wjr  2009.6.3 ��·��λ��ǣ�����ʶ��·��λ���һ���յ��Է��Ĵ�FCBλ�ı��ģ����ԶԷ���FCB��ʼλΪ��ʼ��־
    
    //��Ӧ�ò���񣬼���Ƿ��ͳ�ʼ������,���ڷ�ƽ��ģʽ��
    if(ELReplyFlag)
    {
        DLCommand=DL_RESETDL;
        LengthIn=0;
        pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                            TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
        if(AppCommand&APP_HAVEDATA1)
            FlagData1=TRUE;
        else
            FlagData1=FALSE;
    }

    EditSecFra10(DLCON);	//�༭Ӧ��֡��
    SendDataToMISI();

    if(BalanMode && ELReplyFlag)   //����Ƿ��ͳ�ʼ������������ƽ��ģʽ��
    {
        if(IsDoubleLinkInit /*&& (IsSendLinkInitCmd==FALSE)*/) //����Ӷ���·�Ѿ���λ��,˵������վ���س�ʼ������,��Ҫ��λ�Է���·
        {
            CallDLStatus();
        }
        else
        {
            //IsSendLinkInitCmd=FALSE;
            DLCommand=DL_SCAN1S;
            LengthIn=0;
            pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
            DLSendProc();           //��������׼���õĳ�ʼ������֡
        }
    }
    
}

void CSecDLink::RecReqDLStatus(void)
{
    //������״̬�£��յ�������·״̬������ı䵱ǰ��·״̬
    EditSecFra10(DLSTATUSOK);
    SendDataToMISI();
}
/*------------------------------------------------------------------------
 Procedure:     RecTestDL
 Purpose:       ��Ӧ��·�����Ӧ������GPRSͨѶģʽ������
 Input:   
 author��       ll 2010/07/20      
------------------------------------------------------------------------*/
void CSecDLink::RecTestDL(void)
{
    
    EditSecFra10(DLCON);
    SendDataToMISI();
}

/*�༭10֡��������վ��10֡*/
void CSecDLink::EditFra10(INT8U Function)
{
    INT8U *p;

    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;
    p=TxdBuf+FrameHead;
    p[0]=STARTCODE10;
    p[1]=Function;
    LastControl=Function;
    
    switch(Function)
    {
    case DLRESETRDL:
    case DLREQACD:
    case DLREQSTATUS:
    case DLTESTDL:
        p[1]&=(~P101_FCV);
        break; 
    default:
        p[1]|=P101_FCV;
        break;    
    }
    
    /*if ((Function==DLRESETRDL)||(Function==DLREQACD)||(Function==DLREQSTATUS)||(Function==DLTESTDL))
        p[1]&=(~P101_FCV);
    else
        p[1]|=P101_FCV;*/

    if (p[1]&P101_FCV)//FCV��Ч����תFCB
    {
        if (TlaConCode&0x20)
        {
            p[1]&=(~P101_FCB);
            TlaConCode&=0xDF;
        }
        else
        {
            p[1]|=P101_FCB;
            TlaConCode|=0x20;
        }
    }
    else//FCV��Ч����FCB
        p[1]&=(~P101_FCB);
    p[1]|=P101_PRM;
    p[1]&=(~P101_DIR);//�巽��λ����ֹ���š�
    p[1]|=IEC_DIR;//��վ����վ���䣬ƽ��ģʽΪ1����ƽ��ģʽΪ0��
    for(int i=0;i<LinkAddrSize;i++)
        p[2+i]=((SourceNo>>(8*i))&0xff);
    p[2+LinkAddrSize]=CheckSum(p);
    p[3+LinkAddrSize]=ENDCODE;
    TxdTail+=FixFrmLength;//�ƶ�����βָ��

    memcpy(TxdBuf_Pri,p,FixFrmLength);//���������ݱ��浽����վ�ط�������
    FrameHead_Pri=0;
    TxdHead_Pri=0;
    TxdTail_Pri=FixFrmLength;
}

/*�༭68֡*/
//Function:��·������
//FrameLength:Ӧ�ò㱨�ĳ��ȣ������ͱ�ʶ��ʼ������Ӧ�ò���÷��ص�
void CSecDLink::EditFra68(INT8U Function,INT16U FrameLength)
{
    INT8U *p;
    INT16U length;

    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;
    p=TxdBuf+FrameHead;

    p[0]=p[3]=STARTCODE68;
    p[1]=p[2]=FrameLength+1+LinkAddrSize;
    p[4]=Function;
    LastControl=Function;
    if ((Function==DLSENDNOCON))
        p[4]&=(~P101_FCV);
    else
        p[4]|=P101_FCV;
    if (p[4]&P101_FCV)
    {
        if (TlaConCode&0x20)
        {
            p[4]&=(~P101_FCB);
            TlaConCode&=0xDF;
        }
        else
        {
            p[4]|=P101_FCB;
            TlaConCode|=0x20;
        }
        TlaConCode|=0x10;
    }
    else
    {
        p[4]&=(~P101_FCB);
        TlaConCode&=0xEF;
    }
    p[4]|=P101_PRM;
    p[4]&=(~P101_DIR);//�巽��λ����ֹ���š�
    p[4]|=IEC_DIR;//��վ����վ���䣬ƽ��ģʽΪ1����ƽ��ģʽΪ0��
    for(int i=0;i<LinkAddrSize;i++)
        p[5+i]=((SourceNo>>(8*i))&0xff);

	if((N101Encrptystyle == 4)&&(AuthEndflag==wChanNo))
	{
		Packf68ToOld1120aEn(p);
		FrameLength = p[1]-1-LinkAddrSize;
		LengthOut = FrameLength;  
	}
    length=7+LinkAddrSize+FrameLength;//ȫ�����ĳ���
    p[length-2]=CheckSum(p);
    p[length-1]=ENDCODE;
    TxdTail+=length;

    memcpy(TxdBuf_Pri,p,length);
    FrameHead_Pri=0;
    TxdHead_Pri=0;
    TxdTail_Pri=length;
}

void CSecDLink::EditSecFra68(INT8U Function,INT16U FrameLength)
{
    INT8U *p;
    INT16U length;

    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;

    p=TxdBuf+FrameHead;
    p[0]=p[3]=STARTCODE68;
    p[1]=p[2]=FrameLength+1+LinkAddrSize;
    p[4]=Function;
    LastControl=Function;
    if(FlagData1&&(!BalanMode))
        p[4]|=P101_ACD;
    else
        p[4]&=(~P101_ACD);
    p[4]&=(~P101_DFC);
    p[4]&=(~P101_PRM);
    p[4]&=(~P101_DIR);//�巽��λ����ֹ���š�
    p[4]|=IEC_DIR;//��վ����վ���䣬ƽ��ģʽΪ1����ƽ��ģʽΪ0��

    for(int i=0;i<LinkAddrSize;i++)
        p[5+i]=((SourceNo>>(8*i))&0xff);
	
	////if((N101Encrptystyle == 4)&&(AuthEndflag==wChanNo))
	////{
		////Packf68ToOld1120aEn(p);
		////FrameLength = p[1]-1-LinkAddrSize;
		////LengthOut = FrameLength;  
	////}
	
    length=7+LinkAddrSize+FrameLength;//ȫ�����ĳ���
    p[length-2]=CheckSum(p);
    p[length-1]=ENDCODE;

    TxdTail+=length;
    memcpy(TxdBuf_Sec,p,length);
    FrameHead_Sec=0;
    TxdHead_Sec=0;
    TxdTail_Sec=length;
}

void CSecDLink::EditSecFra10(INT8U Function)//�༭�Ӷ�վӦ��10֡
{
    INT8U *p;

    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;
    p=TxdBuf+FrameHead;
    p[0]=STARTCODE10;
    p[1]=Function;
    LastControl=Function;
    p[1]&=(~P101_DFC);
    p[1]&=(~P101_PRM);
    p[1]&=(~P101_DIR);//�巽��λ����ֹ���š�
    p[1]|=IEC_DIR;//��վ����վ���䣬ƽ��ģʽΪ1����ƽ��ģʽΪ0
    //if(FlagData1&&(!BalanMode)&&(Function!=DLSTATUSOK))//һ������λ
    if(FlagData1&&(!BalanMode))//һ������λ��Ϊ����Ӧʤ����������·�������ٴ��ٻ���·��Ҫ��
        p[1]|=P101_ACD;
    else
        p[1]&=(~P101_ACD);
    for(int i=0;i<LinkAddrSize;i++)
        p[2+i]=((SourceNo>>(8*i))&0xff);
    p[2+LinkAddrSize]=CheckSum(p);
    p[3+LinkAddrSize]=ENDCODE;
    TxdTail+=FixFrmLength;//�ƶ�����βָ��

    memcpy(TxdBuf_Sec,p,FixFrmLength);
    FrameHead_Sec=0;
    TxdHead_Sec=0;
    TxdTail_Sec=FixFrmLength;
}
void CSecDLink::EditE5(void)//�༭E5֡
{
    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;
    *(TxdBuf+FrameHead)=0xe5;
    LastControl=0xe5;
    TxdTail++;

    *(TxdBuf_Sec)=0xe5;
    FrameHead_Sec=0;
    TxdHead_Sec=0;
    TxdTail_Sec=1;
}

BOOL CSecDLink::ExeMaint(void)
{
    INT8U MaintCode[]={'I','S','T','H','I','S'};
    int i;
    INT8U lpc=0;
    unsigned int station;

    for(i=0;i<9;i++)
        lpc^=RxdBuf[RxdHead+i];

    if(lpc!=RxdBuf[RxdHead+9])
        return(FALSE);
    if(memcmp(MaintCode,RxdBuf+RxdHead,6))
        return(FALSE);

    station=MAKEWORD(RxdBuf[RxdHead+6],RxdBuf[RxdHead+7]);
    if((station!=SourceNo)&&(station!=0xFFFF))
        return(FALSE);

    if((memcmp(RxdBuf+RxdHead,RxdBuf+RxdHead+10,10))
            ||(memcmp(RxdBuf+RxdHead,RxdBuf+RxdHead+20,10))
            ||(memcmp(RxdBuf+RxdHead+10,RxdBuf+RxdHead+20,10)))
        return(1);

    for(i=0;i<3;i++)
    {
        EditMaintCon(station);
        SendDataToMISI();
    }

    //��Լ�л�����
    enterMaint(wAppID);//wAppID,�Դ������񣬾��Ǵ��ںš�
    return(1);
}

void CSecDLink::EditMaintCon(unsigned int station)
{
    int i;
    INT8U lpc=0;
    if (TxdStatus==TXDSEND)
        TxdTail=0;
    FrameHead=TxdTail;

    TxdBuf[FrameHead]='T';
    TxdBuf[FrameHead+1]='H';
    TxdBuf[FrameHead+2]='I';
    TxdBuf[FrameHead+3]='S';
    TxdBuf[FrameHead+4]='I';
    TxdBuf[FrameHead+5]='S';
    TxdBuf[FrameHead+6]=LOBYTE(station);
    TxdBuf[FrameHead+7]=HIBYTE(station);
    TxdBuf[FrameHead+8]='!';
    for(i=0;i<9;i++)
        lpc^=TxdBuf[FrameHead+i];
    TxdBuf[FrameHead+9]=lpc;
    TxdTail+=10;
}

void CSecDLink::DLSendProc(void)
{
    if(AppCommand&APP_HAVEDATA1)//����һ������
    {
        FlagData1=TRUE;
        AppCommand&=0x7FFF;
    }
    else
        FlagData1=FALSE;
    //logSysMsgNoTime("DLSendProc ����%d��״̬=%d",AppCommand,DLPriStatus,0,0);   //  debug ll
    switch(AppCommand)
    {
        case APP_APPCON:	//Ӧ�ò���յ���������Ӧ��
            //EditSecFra10(DLCON);
            //SendDataToMISI();
            break;
        case APP_APPDIABALE:	//Ӧ�ò���Ч
            break;
        case APP_SENDCON:	//����03����
            //logSysMsgNoTime("DLSendProc ����03����%d",LengthOut,0,0,0);   //  debug ll
            EditFra68(DLSENDCON,LengthOut);
            SendDataToMISI();
            
            TimeOutTick_Pri=TimeDelay(LengthOut+8+5);
            DLPriStatus=PRISENDCON;
            RetryCount=MAXRETRYNUM;
            break;
        case APP_SENDNOCON:	//����04���� ƽ��ģʽ
            EditFra68(DLSENDNOCON,LengthOut);
            SendDataToMISI();
            //logSysMsgNoTime("send04�ı�pritime",0,0,0,0);   //  debug ll
            TimeOutTick_Pri=0;
            break;
        case APP_SENDDATA://����08����
            EditSecFra68(DLRESDATA,LengthOut);
            SendDataToMISI();
            break;
        case APP_NODATA://Ӧ�ò�������
            if(!BalanMode)
            {
                if(!IsSendE5)
                    EditE5();
                else
                    EditSecFra10(DLNODATA);
                
                SendDataToMISI();
            }
            break;
        case APP_NOJOB://Ӧ�ò�������
            //logSysMsgNoTime("NOJOB",0,0,0,0);   //  debug ll
            //if(BalanMode&&(DLPriStatus==PRIWAITSTATUS))
            //    CallDLStatus();
            break;
        case APP_RESETMACHINE:
        	EditSecFra68(DLRESDATA,LengthOut);
            SendDataToMISI();
        	SystemReset(WARMRESET);
        	break;
        default:
            break;
    }
    AppCommand = APP_NOJOB; //�������ݷ��������������Ӧ�ò�����Ϊ������
}

void CSecDLink::CallUData(void)//ȡ��������
{
    ScanFlag=FALSE;//ȡ��Ԥ�Ƶ�Ӧ�ò����
    LengthIn=0;
    DLCommand=DL_CALLUDATA;
    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);

    DLSendProc();
}

//ƽ��ģʽ��ң�ع���
void CSecDLink::CallUMsg(void)//ȡң����Ϣ
{
    ScanFlag=FALSE;//ȡ��Ԥ�Ƶ�Ӧ�ò����
    LengthIn=0;
    DLCommand=DL_CALLDBMSG;
    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);

    if(LengthOut!=0)
    {
        EditFra68(DLSENDCON,LengthOut);
        SendDataToMISI();
        //logSysMsgNoTime("CallUMsg�ı�pritime",0,0,0,0);   //  debug ll
        TimeOutTick_Pri = TimeDelay(LengthOut+8+5);
        DLPriStatus=PRISENDCON;
        RetryCount=MAXRETRYNUM;
    }
}

void CSecDLink::SendDataEnd(void)  //���ͽ���,ƽ��ģʽ��
{
    if(DLPriStatus==PRIENABLE)
    {
        switch(LastControl)//���һ�η��͵�����
        {
            case DLSENDNOCON://ƽ��ģʽ������04��������Ӧ�ò����
            case DLCON:
                //logSysMsgNoTime("SendDataEndɨ��Ӧ�ò�",0,0,0,0);//  debug ll
                /*DLCommand=DL_SCAN1S;
                LengthIn=0;
                pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                    TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                DLSendProc();*/
                NotifyToAppSchedule();
                
                break;
            default:
                break;
        }
    }
}

//֪ͨӦ�ò�ɨ��
/*------------------------------------------------------------------/
�������ƣ�  NotifyToAppSchedule()
�������ܣ�  ����һ��Ӧ�ò�ɨ��
����˵����    
���˵����  
��ע��      
/------------------------------------------------------------------*/
void CSecDLink::NotifyToAppSchedule(void)  
{
    DLCommand=DL_SCAN1S;
    LengthIn=0;
    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
    DLSendProc();
}
//��ֹ������·��ѭ����ʼ��
void CSecDLink::EndlessLoopInit(void)
{
    ELReplyFlag = TRUE;
    ELClearTime = 0;
    ELNoReplyTime = 0;
    ELResetNum = 0;
    
}
//��ֹ������·��ѭ������
//���һ��ʱ���ڣ�EL_NOREPLY_TIME���յ���λ��·�������ڣ�EL_MAX_RECEIVE_RESETNUM���жϽ�����ѭ�����̣��򲻶Ը�λ��·����˫��λ��·��Ӧ��
//���ң�EL_NOREPLY_TIME��ʱ����ڲ�����Ӧ��վ��λ��·���
void CSecDLink::EndlessLoopMonitor(void)
{
    
    if(ELClearTime)     //�յ���·��λ�������ʱ�帴λ����
    {
        ELClearTime--;
        if(ELClearTime==0)
        {
            ELResetNum = 0;
        }   
    }
    
    if(ELNoReplyTime)   //�жϽ�����ѭ������ʱ����վ��λ���֧��˫������·
    {
        ELNoReplyTime--;
        if(ELNoReplyTime==0)
        {
            ELReplyFlag = TRUE;
            ELResetNum = 0;
        }
    }
    
    if(ELResetNum >= EL_MAX_RECEIVE_RESETNUM)
    {
               
        logSysMsgNoTime("���������ѭ����%d�벻֧��˫������·",EL_NOREPLY_TIME,0,0,0);
        
        ELResetNum = 0;
        ELReplyFlag = FALSE;
        ELNoReplyTime = EL_NOREPLY_TIME;
        
    }
    
    
    
}
