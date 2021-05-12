#include "secdlink.h"


extern INT32U baudRateTable[];
//INT8U N101Encrptystyle=2;
extern INT16U fixmlen ;//
extern INT16U AuthEndflag;

/*构造函数：初始化*/
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

        DLIdleTime=TimeOutValue/100;//线路空闲检测时间不能小于2秒
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
            IsDoubleLinkInit = TRUE;    //支持双向链路同步初始化
        
        //在支持国网规约扩展的情况下，自动式单向建立链路
        /*if(pSecPad->control & CON_101GYKZ)
        {
           IsDoubleLinkInit = FALSE;   
        }*/    
        
        if(pSecPad->control & CON_NOFCVCTRL)
        {
            NoJudgeFCB = TRUE;
            logSysMsgNoTime("101规约不判断FCB翻转",0,0,0,0);
        }
        else
            NoJudgeFCB = FALSE;
        
        HeartBeatIdleLimit = 0; //60;   //心跳功能暂时不加
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

    dwAppTID=TI->Tid;//系统分配的任务ID。

    DLPriStatus=PRIDISABLE;
    DLSecStatus=SECDISABLE;
    
    TlaConCode = 0;
    RlaConCode = 0;
    
    fixmlen = 4 + LinkAddrSize;//
    
    i=CheckDevType(PI->DevIDSet[0]);
    if (i==0)//0——I类逻辑设备
    {
        struct AppLBConf_t AppLBConf;
        L_ReadBConf(PI->DevIDSet[i],wAppID,(INT8U*)&AppLBConf);
        SourceNo=AppLBConf.Address;//本机地址为源站址
    }
    else if (i==2)//2:II类逻辑设备
    {
        struct AppSLBConf_t AppSLBConf;
        //SL_ReadBConf(PI->DevIDSet[i],wAppID,(INT8U*)&AppSLBConf);
        if(!SL_ReadBConf(PI->DevIDSet[i],wAppID,(INT8U*)&AppSLBConf))       //根据coverity更改
        {
            return false;
        }
        SourceNo=AppSLBConf.Address;//本机地址为源站址
    }
    
    IsEncrypt = 0;
    N101Encrptystyle = 0;
    #ifdef INCLUDE_ENCRYPT
    
        if(pSecPad != NULL)
        {
            //CON_ENCRYPT:CON_1161ENCRPTY = 1:0(无加密)  0:0(11版安全方案) 0:1(16版安全防护方案) 1:1(非法)
            switch(pSecPad->control & (CON_ENCRYPT|CON_1161ENCRPTY|CON_1120ENCRPTY|CON_OLD1120ENCRPTY))
            {
            case 0:
                IsEncrypt = 1;
                myEnctyptInit(SourceNo, pSecPad->EncryptTimeout);
                logSysMsgNoTime("11版安全防护方案(SGC1126)-101",0,0,0,0);
                break;
            case CON_ENCRYPT|CON_1161ENCRPTY:
                //1161加密芯片
                N101Encrptystyle = 2;
                logSysMsgNoTime("15版安全防护方案启用(SGC1161)-101",0,0,0,0);
                myTaskDelay(2);    
                rc = EncrptyChiptest(1);
                if(rc != 1)
                {
                    logSysMsgNoTime("加密方案和芯片类型不匹配或加密芯片不存在",0,0,0,0);
                }
                break;
            case CON_ENCRYPT|CON_1120ENCRPTY:
                //1120加密芯片湖南农网加密
                N101Encrptystyle = 3;
                logSysMsgNoTime("湖南农网安全防护方案(SGC1120a)-101",0,0,0,0);
                myTaskDelay(2);    
               rc = EncrptyChiptest(1);
               if(rc != 2)
               {
                   logSysMsgNoTime("加密方案和芯片类型不匹配或加密芯片不存在",0,0,0,0);
               }
                break;		
            case CON_ENCRYPT|CON_OLD1120ENCRPTY:
                 N101Encrptystyle = 4;
                 logSysMsgNoTime("老湖南农网安全防护方案(SGC1120a)-101",0,0,0,0);
                 myTaskDelay(2);    
                rc = EncrptyChiptest(1);
                if(rc != 2)
                {
                    logSysMsgNoTime("加密方案和芯片类型不匹配或加密芯片不存在",0,0,0,0);
                }
                 break;      
            default:
                //无加密
                logSysMsgNoTime("无安全防护方案(透明传输)-101",0,0,0,0);
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
    FirstRecFCB = TRUE;     //wjr  2009.6.3 链路复位标记，来标识链路复位后第一次收到对方的带FCB位的报文，并以对方的FCB起始位为起始标志
    return	TRUE;
}

//读MISI数据
void CSecDLink::RecMISIData(void)
{
	RecMISIDataDealFun(N101Encrptystyle);
}

/********************************************************************
*函数名称：RecMISIDataDealFun
*功能：判别当前规约加密方式，处理密文或者明文数据
*输入：Enflag:加密类型，2表示1161国网加密，3表示
*                  1120a湖南农网加密，其他为明文或168号文处理
*研发人：张良
*********************************************************************/

void CSecDLink::RecMISIDataDealFun(INT8U Enflag)
{
    INT16U RNum,FrameTailFlag;
    INT16U tmpau,ennum;
	
    if((Enflag!=2)&&(Enflag!=3))//明文通讯方式
    {
        if(RxdHead<RxdTail)    //接收缓冲区中有尚未处理的数据
        {
            if (RxdHead!=0)
            {
                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);//将未处理的数据移到缓冲区头
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
        if(EbMsgRxdHead<EbMsgRxdTail)    //接收缓冲区中有尚未处理的数据
        {
            if (EbMsgRxdHead!=0)
            {
                memcpy(EbMsgRxdBuf,EbMsgRxdBuf+EbMsgRxdHead,EbMsgRxdTail-EbMsgRxdHead);//将未处理的数据移到缓冲区头
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
		
        if(RxdHead<RxdTail)    //接收缓冲区中有尚未处理的数据
        {
            if (RxdHead!=0)
            {
                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);//将未处理的数据移到缓冲区头
                RxdTail-=RxdHead;
                RxdHead=0;
            }
        }
        else
        {
            RxdHead=RxdTail=0;
        }

		tmpau = RxdTail;
		
		if(Enflag == 2)//国网1161安全加密
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

//发送缓冲区中的数据，写MISI接口
BOOL CSecDLink::SendDataToMISI(void)
{
    if(BalanMode)
        myTaskDelay(5);    //平衡模式发送延迟50ms
    else
        myTaskDelay(20);   //非平衡模式发送延迟200ms
        
	return SendMISIDataDealFun(N101Encrptystyle);
}
/********************************************************************
*函数名称： SqrLongFraFunI
*函数功能： 根据提供的101原始报文信息，将部分原始
*           报文的遥信/遥测数据组一帧较短的101报文
*函数输入：
*           ori:原始报文缓冲区，
*           Bakbuf分帧密文缓冲区
*           infodataptr:当前组帧的遥信遥测的起始地址 
*           ctoalen:  控制域到asdu公共地址长度   
*           num: 需要组帧的遥信/遥测个数
*           infolen:信息体地址长度
*           count:帧计数
*研发人：张良
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
        len = ctoalen+((num*infolen) + En_InfoAddrSize); //只记录开头的信息体地址
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
    memcpy((ptr+4+ctoalen),infodataptr,len-ctoalen);  //遥信地址+值
    ptr[lenth-1]=0x16;	
	if((Sq!=0)&&(count!=0))
	{
            paddr = (ptr-170);
	    addr = paddr[4+ctoalen];
    	    infonum = paddr[fixmlen+2];
    	    addr = addr+(infonum&0x7F);
           ptr[4+ctoalen+1] = paddr[4+ctoalen+1];
	    ptr[4+ctoalen] = (INT8U)(addr&0x00FF);//信息体地址
           if(addr>0xFF)
           {            
               ptr[4+ctoalen+1]++;                       
           }
	}
    if(count == 1)
    {
        if((ptr[4]&0x20)==0)//fcb翻转
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
*函数名称：SqrLongFraFun
*函数功能：判断当前帧是否需要生成密文且密文长度大于255
*           并将过长报文分成3帧报文
*函数输入：
*    ori:原始报文缓冲区，
*    Bakbuf分帧报文缓冲区
*研发人：张良
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
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//控制域到asdu公共地址长度    9  
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
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//控制域到asdu公共地址长度    9  
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
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//控制域到asdu公共地址长度    9  
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
                    len1 = 1+En_LinkAddrSize+1+1+En_CotSize+En_PubAddrSize;//控制域到asdu公共地址长度    9  
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
*函数名称：RecMISIDataDealFun
*功能：判别当前规约加密方式，处理密文或者明文数据
*输入：Enflag:加密类型，2表示1161国网加密，3表示
*                  1120a湖南农网加密，其他为明文或168号文处理
*研发人：张良
*********************************************************************/

BOOL CSecDLink::SendMISIDataDealFun(INT8U Enflag)
{
    INT16U SendLen,rc,eb101sendlen,length,i;
    INT8U *ptr;
    IdleTimeCount = 0;
    HeartBeatIdleTime = 0;
    ////memcpy(TxdBuf,bufttt,255);/////
    ////AuthEndflag = 2;

    if((Enflag!=2)&&(Enflag!=3)&&(Enflag!=4))//明文通讯方式
    {
        if (TxdStatus==TXDSEND)//新数据发送
        {
            TxdHead=0;
            SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf,TxdTail-TxdHead,3);
            if (SendLen>0)//数据正常发送（完没完还未判断）
                TxdStatus=TXDWAIT;
        }
        else//上次数据未发送完，
        {
            SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf+TxdHead,TxdTail-TxdHead,1);
        }
    }
    else if(Enflag == 4)//老农网加密
    {
        if (TxdStatus==TXDSEND)//新数据发送
        {
            TxdHead=0;
            if((AuthEndflag!=wChanNo)&&(TxdBuf[0]==0x68))//认证完成之前不处理业务报文
            {  
                switch(TxdBuf[7])//遥信，遥测，遥控，信息上报都要加密
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
            if (SendLen>0)//数据正常发送（完没完还未判断）
                TxdStatus=TXDWAIT;
        }
        else//上次数据未发送完，
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
		
		if (TxdStatus==TXDSEND)//新数据发送
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
	        
	        if (SendLen>0)//数据正常发送（完没完还未判断）
				TxdStatus=TXDWAIT;	
	    }   
		else//上次数据未发送完，
		{
			SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf+TxdHead,TxdTail-TxdHead,1);
		}
    }
    
    if(SendLen==0xffff)//写错误，返回-1
    {
        SendLen=(INT16U)MisiWrite(wChanNo,TxdBuf,0,1);//发送帧尾
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
        if(TxdHead>=TxdTail)//该次任务数据已经发完。
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
函数名称：  SendTestOnTime()
函数功能：  定时上送链路层的测试帧（心跳）
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecDLink::SendTestOnTime(void)
{
    if(DLSecStatus==SECDISABLE)
    {
        //未建立连接不执行心跳 
        HeartBeatIdleTime = 0;
        return;
    }
    
    if(HeartBeatIdleLimit == 0) //HeartBeatIdleLimit为0表示不需要进行心跳
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
函数名称：  TimeOut()
函数功能：  超时处理函数
输入说明：  
输出说明：  每秒运行一次，由应用层调用
/------------------------------------------------------------------*/
void CSecDLink::TimeOut(void)
{
    //logSysMsgNoTime("1 PriTime=%d, Sectime=%d,",TimeOutTick_Pri,TimeOutTick_Sec,0,0);//dubug ll
    if(TimeOutTick_Pri)//单位是秒
    {
        TimeOutTick_Pri--;
        if(!TimeOutTick_Pri)
        {
            TimeOutFun(1,1);
        }
    }

    if(BalanMode)//平衡模式下，为了在链路空闲期内及时处理应用层的一级数据任务，在超过设定的空闲时间后每秒检测应用层一次。
    {
        SendTestOnTime();   
        EndlessLoopMonitor();
        IdleTimeCount++;
        if((IdleTimeCount>=DLIdleTime)&&(DLPriStatus==PRIENABLE))//DLIdleTime最小不能小于2
        {
            DLCommand=DL_SCAN1S;
            LengthIn=0;

            //应用层调用
            pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
            DLSendProc();
            //logSysMsgNoTime("空闲时刻扫描应用层",0,0,0,0);//  debug ll
        }
        
    }
    
    //从动状态为禁止，链路延时初始化结束，也认为从动允许，通知链路层允许从动状态
    if(BalanMode &&(DLSecStatus == SECDISABLE))
    {
        if((pSecApp->APP_DLSecStatus == SECDISABLE) && (pSecApp->DLInitFinishFlag == TRUE))
        {
            pSecApp->APP_DLSecStatus = SECENABLE;
            DLSecStatus = SECENABLE;
            logSysMsgNoTime("从动方向超时判为允许",0,0,0,0);
        }
    }
    
}

void CSecDLink::TimeOutFun(INT8U FailType,INT8U Prm)//超时处理
{
    if(!RetryCount)
        return;
    RetryCount--;
    
    if(RetryCount)
    {
        if(Prm)//启动站报文超时未收到应答
        {
            //logSysMsgNoTime("SEC 超时重发,",0,0,0,0);//dubug ll
            SendPriDataToMISI();
            TimeOutTick_Pri=TimeOutTickCopy;
        }
        else//这里其实运行不到
        {
            SendSecDataToMISI();
            TimeOutTick_Sec=TimeOutTickCopy;
        }
        return;
    }
    else//超过重发次数后，重新建立链路，并通知应用层。
    {
        if(BalanMode)
        {
            //logSysMsgNoTime(" Pri=%x, Sec=%x, Time=%d,R=%d,",DLPriStatus,DLSecStatus,TimeOutTick_Pri,RetryCount);//dubug ll
            
            //if(gxceshi == 0)
             //   CallDLStatus();
            
            //IsSendLinkInitCmd = TRUE;
            DLSecStatus = SECDISABLE;   //链路层从动方向清标志位，主动方向因为是超时重发无应答，也不是允许态，因此无需清。

            DLCommand=DL_LINKDISABLE;
            LengthIn=0;
            pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
        }
    }
}


//启动链路过程
void CSecDLink::CallDLStatus(void)
{
    //if(pSecApp->GYKZ2015Flag)  //如果是国网要求扩展规约，那么不主动进行链路初始化
    //    return;
        
    //logSysMsgNoTime("4 CallDLStatus发送请求链路命令",0,0,0,0);   //  debug ll
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
            
            if (RxdHead<RxdTail)//找到启动字符，并将报文与缓冲区对齐。
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
            if((RxdBuf[RxdHead] == 0x16)&& IsEncrypt )    //加密报文处理&& IsEncrypt
            {
                
                rc = CheckEncryptFramHead(&RxdBuf[RxdHead], RxdLength);
                switch(rc)
                {
                case 0: //报文检查正确
                    RxdHead+=(RxdBuf[RxdHead+1]+6);//头指针后移到报文后
                    RxdStatus=RXDSTART;
                    Stop=TRUE;
                    //logSysMsgWithTime("收到通讯加密报文开始处理",0,0,0,0);   //debug
                    ExeDLFunCode16();
                    //logSysMsgWithTime("报文处理完毕报文111",0,0,0,0); //debug
                    break; 
                case 1: //帧头不对
                    RxdHead++;
                    RxdStatus=RXDSTART;
                    break;
                case 2: //未收完全
                    Stop=TRUE;
                    RxdStatus=RXDSTART;
                    break; 
                case 3: //校验和、解签错误
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
        if(RxdStatus==RXDHEAD)//报文够一帧则
        {
            switch (RxdBuf[RxdHead])
            {
                
                case STARTCODE68:
                    if (RxdLength>=5+LinkAddrSize)//启动字符到链路地址共6字节。
                    {
                        if (!ExeDLFun68())//判断是否报文头
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
                    if (RxdLength>=length+6)//报文收全，进行处理
                    {
                        //logSysMsgNoTime("收到68命令",0,0,0,0);//debug
                        if ( (RxdBuf[RxdHead+length+4]==CheckSum(RxdBuf+RxdHead))//校验正确
                                &&(RxdBuf[RxdHead+length+4+1]==ENDCODE))//结束码正确
                        {
                            if (RxdHead>0)
                            {
                                memcpy(RxdBuf,RxdBuf+RxdHead,RxdTail-RxdHead);//报文与缓冲区对齐
                                RxdTail-=RxdHead;
                                RxdHead=0;
                            }
                            
                        #ifdef INCLUDE_ENCRYPT    
                            //是否支持加密 RxdBuf+5+LinkAddrSize
                            if(IsEncrypt && 
                                ((RxdBuf[5+LinkAddrSize]==C_SC_NA) || (RxdBuf[5+LinkAddrSize]==C_DC_NA))
                              )
                            {
                                if(RxdLength>=length+10) 
                                {
                                    if((RxdBuf[length+6] != 0x16) || (RxdBuf[length+9] != 0x16))
                                    {
                                        logSysMsgNoTime("无认证报文头，丢弃报文",0,0,0,0);
                                        RxdStatus=RXDSTART;
                                        //无认证报文头，丢弃报文
                                        RxdHead+=(RxdBuf[1]+6);
                                        EditSecFra10(DLCON);
                                        SendDataToMISI();
                                        if (RxdTail>RxdHead)
                                            myEventSend(myTaskIdSelf(),NEXTFRAME);
                                        
                                        
                                    }
                                    else if (RxdLength>=length+6+RxdBuf[length+7]+6)//报文收全，进行处理
                                    {
                                        RxdStatus=RXDSTART;
                                        
                                        //logSysMsgWithTime("收到报文开始处理222",0,0,0,0);   //debug 
                                        rc = CheckEncryptFrame(&RxdBuf[length+6], RxdBuf, length+6);
                                        if(rc == 0)
                                        {
                                           
                                            RxdHead+=(length+6+RxdBuf[length+7]+6);//头指针后移到报文后 
                                            //RxdStatus=RXDSTART;
                                            logSysMsgWithTime("收到加密遥控报文,安全认证报文正确.", 0,0,0,0);
                                            ExeDLFunCode68();
                                            //logSysMsgWithTime("报文处理结束222",0,0,0,0);   //debug
                                            if (RxdTail>RxdHead)
                                                myEventSend(myTaskIdSelf(),NEXTFRAME);
                                            
                                        }
                                        else
                                        {
                                            RxdHead+=(RxdBuf[1]+10);
                                            logSysMsgNoTime("安全报文校验(校验和,签名,时间戳)不正确.rc=%d",rc,0,0,0);
                                        }
                                        
                                    }
                                    
                                }
                                                                
                                Stop=TRUE;
                                
                            }
                            else    //无需加密时的处理
                         #endif
                            {
                                
                                RxdHead+=(RxdBuf[1]+6);//头指针后移到报文后
                                RxdStatus=RXDSTART;
                                Stop=TRUE;
    
                                ExeDLFunCode68();
                                
                                if (RxdTail>RxdHead)
                                    myEventSend(myTaskIdSelf(),NEXTFRAME);
                            }
                        }
                        else
                        {
                            //logSysMsgNoTime("校验和不对，head=%d,cs=%d, length=%d",RxdHead,RxdBuf[RxdHead+length+4+1],RxdBuf[RxdHead+1],0);//debug
                            RxdHead+=6;
                            RxdStatus=RXDSTART;
                        }
                    }
                    else//报文未收全，不处理，继续等待数据。
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

//检测10帧正确性
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
    
    if(BalanMode)   //ll 增加站址自学习
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

//判断68帧头。
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
    
    
    if(Control&P101_PRM)//子站作为从动站接收主站的命令
    {
        
        //从动链路执行部分
        if(DLSecStatus==SECDISABLE) //当链路状态未有效（复位）时
        {
            if(((Control&P101_FUNCODE)!=DLREQSTATUS) 
                && ((Control&P101_FUNCODE)!=DLRESETRDL))   //不回答任何非FC9和FC0的报文
            {
                return ;   
            }
        }
        
        //平衡模式下，收到FC10和FC11命令，不处理
        if(BalanMode)   
        {
            if(((Control&P101_FUNCODE)==DLREQDATA1) 
                || ((Control&P101_FUNCODE)==DLREQDATA2))   
            {
                return ;   
            }
        }
        //非平衡模式下，收到FC2，不处理
        if(!BalanMode)
        {
            if((Control&P101_FUNCODE)==DLTESTDL)
                return;
        }
        //logSysMsgNoTime("SEC prm=1,收到报文%d",Control&P101_FUNCODE,0,0,0);   //  debug ll 
        if ((!(Control&P101_FCV)) || ((Control&P101_FCB) !=((RlaConCode) & 0x20)) || (FirstRecFCB==TRUE))
        {
            if (Control&P101_FCV)//FCV有效，记录FCB状态。
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
                case DLREQSTATUS://FC9 召唤链路状态
                    RecReqDLStatus();
                    while((RxdTail-RxdHead>=FixFrmLength)&&(Stop==0))//收到召唤链路状态命令时，清掉后续相同命令。
                    {//可以避免答非所问的过程，尽快建立链路。
                        if(memcmp(RxdBuf,RxdBuf+RxdHead,FixFrmLength)==0)
                            RxdHead+=FixFrmLength;
                        else
                            Stop=1;
                    }
                    break;
                case DLRESETRDL://FC0 复位链路
                    RecResetDL();
                    break;
                case DLTESTDL:  //FC2, 链路测试， ll 2010/07/20 for广西测试
                    //平衡模式才能到这里来
                    RecTestDL();
                    break;
                case DLREQDATA1://FC10
                    //非平衡模式才能到这里
                    DLCommand=DL_CALLDATA1;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    LengthIn=0;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);

                    DLSendProc();
                    
                    break;
                case DLREQDATA2://FC11
                    //非平衡模式才能到这里
                    DLCommand=DL_CALLDATA2;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    LengthIn=0;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    
                    break;
                default:    //除上述FC0,FC2,FC9,FC10,FC11外，其他FC不理会
                    break;
            }
            return;
        }
        else //FCB未翻转
        {
            //FCB未翻转，在平衡模式下收到FC2的报文，这时采取重发策略,一般发的都是确认命令。
            //FCB未翻转，在非平衡模式下收到FC10/FC11的报文，这时采取重发策略。
            SendSecDataToMISI();
            //logSysMsgNoTime("SEC 10帧未翻转重发",0,0,0,0);   //  debug ll
            
        }
    }
    else//Prm=0 平衡模式子站作为启动站接收到主站的命令。
    {
        //logSysMsgNoTime("SEC prm=0,收到报文%d",Control&P101_FUNCODE,0,0,0);   //  debug ll
        switch(Control&P101_FUNCODE)
        {
            case DLCON:	//FC0 确认
                RecConf10(Control);
                break;
            case DLSTATUSOK://FC11 应答链路
                RecDLSta(Control);
                break;
            case DLNOWORK:  //FC14
            case DLNOFIN:   //FC15
                //其他状态下，收到FC14/FC15时保持原状态不变
                /*if(DLPriStatus==PRISENDCON) //在等待对方确认状态时，收到FC14/FC15返回链路层可用状态
                {
                    DLPriStatus = PRIENABLE;
                    TimeOutTick_Pri = 0;
                }*/
                
                //指示应用层，链路层未起作用和未完成。(应用层如何处理？)
                break;
            case DLNOCON:   //FC1 否定确认应答
                //只有在PRIWAITRSCON状态下，收到FC1才是正确的，这时程序不改变当前状态
                //其他状态下收到FC1属于异常情况, 程序不做应答。    
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
    
    if(DLSecStatus==SECDISABLE) //当链路状态未有效（复位）时, 不接受任何68帧，平衡非平衡模式同样处理
    {
        logSysMsgNoTime("通讯加密-主站未建立链路，不执行命令",0,0,0,0);
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
    //logSysMsgNoTime("SEC 收到68帧报文从动状态%d",DLSecStatus,0,0,0);   //  debug ll
    if(DLSecStatus==SECDISABLE) //当链路状态未有效（复位）时, 不接受任何68帧，平衡非平衡模式同样处理
    {
        return ;   
    }
    	
    Control=RxdBuf[4];
    IdleTimeCount = 0;
    HeartBeatIdleTime = 0;
    if(Control&P101_PRM)//子站作为从动站接收主站链路报文。
    {
        if ((!(Control&P101_FCV)) || ((Control&P101_FCB) !=((RlaConCode) & 0x20)) || (FirstRecFCB==TRUE))
        {
            if (Control&P101_FCV)//FCV有效而且FCB变位
            {
                RlaConCode=RxdBuf[4]; //保存最新的功能码，这里只用到FCB
                if(NoJudgeFCB == FALSE)
                {
                    if(FirstRecFCB==TRUE)
                        FirstRecFCB=FALSE;   //wjr 2009.6.3   收到一次68帧的数据后就讲该标志清掉
                }
            }
            	
            switch(Control&P101_FUNCODE)
            {
                case DLRESETUSE://1，启动站复位从动站用户过程
                    
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    EditSecFra10(DLCON);
                    SendDataToMISI();
                    
                    break;
                case DLTESTDL://2，测试链路
                                        
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    EditSecFra10(DLCON);
                    SendDataToMISI();
                    
                    break;
                case DLSENDCON://3，发送/确认 命令
                    DLCommand=DL_SENDCON;//
                    
                    //EditSecFra10(DLCON);
                    //SendDataToMISI();
                    //logSysMsgNoTime("收到68帧, RxdBuf=%x, lenth=%d,pub=%d, cot=%d",RxdBuf[0],RxdBuf[1],pSecApp->PubAddrLocation,pSecApp->CotLocation);   //  debug ll
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);  //调用SecAppProc函数置位相应的标志，并不发送数据。
                    //logSysMsgNoTime("收到68帧，准备发送%d，lenth=%d",AppCommand,LengthOut,0,0);   //  debug ll
                    DLSendProc();   //基本不发送确认帧
                    //logSysMsgNoTime("收到68帧 发送确认",0,0,0,0);   //  debug ll
                    EditSecFra10(DLCON);    //发送确认帧
                    SendDataToMISI();
                    
                    break;
                case DLSENDNOCON://4，发送/无回答 命令
                    DLCommand=DL_SENDNOCON;//
                    
                    LengthIn=RxdBuf[1]-1-LinkAddrSize;
                    if((!BalanMode)&&(Control&P101_FCV))
                        DLCommand|=DL_FCBOK;
                    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
                    DLSendProc();
                    break;
                default:    //除上述FC1,FC2,FC3,FC4外，其他FC不理会
                    break;
            }
        }
        else  //FCB 未翻转  //change by wyj
        {
            SendSecDataToMISI();
          
        }
    }
    else  //Prm=0；子站作为启动站接收主站链路报文
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
函数名称：  RecDLSta()
函数功能：  响应查询链路
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecDLink::RecDLSta(INT8U Control)
{
    //FC11只有在PRIWAITSTATUS这个状态下出现才有效，其他都属异常情况
    if(DLPriStatus == PRIWAITSTATUS)
    {
        //if((Control&P101_DFC)==0)   //在PRIWAITSTATUS状态下，收到DFC=1的应答，应无反应
        {
            EditFra10(DLRESETRDL);
            SendDataToMISI();
            
            TlaConCode &= 0xDF;  
            DLPriStatus=PRIWAITRSCON;
            //logSysMsgNoTime("发送复位命令报文",0,0,0,0);   //  debug ll
            TimeOutTick_Pri=TimeDelay(2* FixFrmLength);
            RetryCount=MAXRETRYNUM;
        }
        
    }
    //其他状态下，收到FC11，都丢弃报文处理，不反应
    
}
/*------------------------------------------------------------------/
函数名称：  RecConf10()
函数功能：  平衡模式子站作为启动站接收到主站的确认报文
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecDLink::RecConf10(INT8U Control)
{
    //FC0 只有在PRIWAITRSCON和PRISENDCON状态下出现才是正确的，其他为异常情况
    
    if (DLPriStatus==PRIWAITRSCON)
    {
        //logSysMsgNoTime("启动方向链路确认",0,0,0,0);    //dubug ll
        DLPriStatus=PRIENABLE;
        if(IsDoubleLinkInit == FALSE)   //不是双向建立链路，就让从动也为允许（应用层和链路层）。
        {
            DLSecStatus=SECENABLE; 
            pSecApp->APP_DLSecStatus = SECENABLE;
            
            RlaConCode &= 0xDF;     //清FCB  
            FirstRecFCB = TRUE;
        }
        TimeOutTick_Pri = 0;  //收到确认命令，清请求复位命令超时时间
        
        //通知应用层，当前链路状态可用
        DLCommand=DL_LINKENABLE;
        LengthIn=0;
        pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                            TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
        DLSendProc();
        
       
    }
    else if (DLPriStatus==PRISENDCON)//平衡模式
    {
        //if((Control&P101_DFC)==0)
        {
            DLPriStatus = PRIENABLE;
            //logSysMsgNoTime("RecConf10=%d",DLPriStatus,0,0,0);   //  debug ll
            TimeOutTick_Pri = 0;
            //应用层调用
            DLCommand=DL_APPCON;
            LengthIn=0;
            pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                                TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
            DLSendProc();
        }
    }
    
    //其他状态下收到FC0，属异常情况，不应答
}

//收到复位链路命令
void CSecDLink::RecResetDL(void)
{
    //防止进入死循环添加 ll 2014-8-11
    if(ELReplyFlag) //ELReplyFlag 正常时都是1
    {
        ELClearTime = EL_NOREPLY_TIME;
        ELResetNum++;
    }
    
    DLSecStatus = SECENABLE;//从站链路可用
    if(IsDoubleLinkInit == FALSE)   //不是双向建立链路，从动方向建立，启动方向也为允许（应用层和链路层）。
    {
       DLPriStatus = PRIENABLE; 
       pSecApp->APP_DLPriStatus = PRIENABLE;
    }
    
    RlaConCode &= 0xDF;     //清FCB  
        
    FirstRecFCB = TRUE;     //wjr  2009.6.3 链路复位标记，来标识链路复位后第一次收到对方的带FCB位的报文，并以对方的FCB起始位为起始标志
    
    //调应用层服务，检测是否发送初始化结束,用于非平衡模式下
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

    EditSecFra10(DLCON);	//编辑应答帧。
    SendDataToMISI();

    if(BalanMode && ELReplyFlag)   //检查是否发送初始化结束，用于平衡模式下
    {
        if(IsDoubleLinkInit /*&& (IsSendLinkInitCmd==FALSE)*/) //如果从动链路已经复位过,说明是主站本地初始化过程,需要复位对方链路
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
            DLSendProc();           //发送上面准备好的初始化结束帧
        }
    }
    
}

void CSecDLink::RecReqDLStatus(void)
{
    //在任意状态下，收到请求链路状态命令，不改变当前链路状态
    EditSecFra10(DLSTATUSOK);
    SendDataToMISI();
}
/*------------------------------------------------------------------------
 Procedure:     RecTestDL
 Purpose:       响应链路层测试应答，用于GPRS通讯模式的心跳
 Input:   
 author：       ll 2010/07/20      
------------------------------------------------------------------------*/
void CSecDLink::RecTestDL(void)
{
    
    EditSecFra10(DLCON);
    SendDataToMISI();
}

/*编辑10帧——启动站的10帧*/
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

    if (p[1]&P101_FCV)//FCV有效，翻转FCB
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
    else//FCV无效，清FCB
        p[1]&=(~P101_FCB);
    p[1]|=P101_PRM;
    p[1]&=(~P101_DIR);//清方向位，防止干扰。
    p[1]|=IEC_DIR;//子站向主站传输，平衡模式为1，非平衡模式为0。
    for(int i=0;i<LinkAddrSize;i++)
        p[2+i]=((SourceNo>>(8*i))&0xff);
    p[2+LinkAddrSize]=CheckSum(p);
    p[3+LinkAddrSize]=ENDCODE;
    TxdTail+=FixFrmLength;//移动发送尾指针

    memcpy(TxdBuf_Pri,p,FixFrmLength);//将发送数据保存到启动站重发数据区
    FrameHead_Pri=0;
    TxdHead_Pri=0;
    TxdTail_Pri=FixFrmLength;
}

/*编辑68帧*/
//Function:链路功能码
//FrameLength:应用层报文长度（从类型标识开始），是应用层调用返回的
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
    p[4]&=(~P101_DIR);//清方向位，防止干扰。
    p[4]|=IEC_DIR;//子站向主站传输，平衡模式为1，非平衡模式为0。
    for(int i=0;i<LinkAddrSize;i++)
        p[5+i]=((SourceNo>>(8*i))&0xff);

	if((N101Encrptystyle == 4)&&(AuthEndflag==wChanNo))
	{
		Packf68ToOld1120aEn(p);
		FrameLength = p[1]-1-LinkAddrSize;
		LengthOut = FrameLength;  
	}
    length=7+LinkAddrSize+FrameLength;//全部报文长度
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
    p[4]&=(~P101_DIR);//清方向位，防止干扰。
    p[4]|=IEC_DIR;//子站向主站传输，平衡模式为1，非平衡模式为0。

    for(int i=0;i<LinkAddrSize;i++)
        p[5+i]=((SourceNo>>(8*i))&0xff);
	
	////if((N101Encrptystyle == 4)&&(AuthEndflag==wChanNo))
	////{
		////Packf68ToOld1120aEn(p);
		////FrameLength = p[1]-1-LinkAddrSize;
		////LengthOut = FrameLength;  
	////}
	
    length=7+LinkAddrSize+FrameLength;//全部报文长度
    p[length-2]=CheckSum(p);
    p[length-1]=ENDCODE;

    TxdTail+=length;
    memcpy(TxdBuf_Sec,p,length);
    FrameHead_Sec=0;
    TxdHead_Sec=0;
    TxdTail_Sec=length;
}

void CSecDLink::EditSecFra10(INT8U Function)//编辑从动站应答10帧
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
    p[1]&=(~P101_DIR);//清方向位，防止干扰。
    p[1]|=IEC_DIR;//子站向主站传输，平衡模式为1，非平衡模式为0
    //if(FlagData1&&(!BalanMode)&&(Function!=DLSTATUSOK))//一级数据位
    if(FlagData1&&(!BalanMode))//一级数据位，为了适应胜利油田在链路过程总再次召唤链路的要求
        p[1]|=P101_ACD;
    else
        p[1]&=(~P101_ACD);
    for(int i=0;i<LinkAddrSize;i++)
        p[2+i]=((SourceNo>>(8*i))&0xff);
    p[2+LinkAddrSize]=CheckSum(p);
    p[3+LinkAddrSize]=ENDCODE;
    TxdTail+=FixFrmLength;//移动发送尾指针

    memcpy(TxdBuf_Sec,p,FixFrmLength);
    FrameHead_Sec=0;
    TxdHead_Sec=0;
    TxdTail_Sec=FixFrmLength;
}
void CSecDLink::EditE5(void)//编辑E5帧
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

    //规约切换调用
    enterMaint(wAppID);//wAppID,对串口任务，就是串口号。
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
    if(AppCommand&APP_HAVEDATA1)//还有一级数据
    {
        FlagData1=TRUE;
        AppCommand&=0x7FFF;
    }
    else
        FlagData1=FALSE;
    //logSysMsgNoTime("DLSendProc 命令%d，状态=%d",AppCommand,DLPriStatus,0,0);   //  debug ll
    switch(AppCommand)
    {
        case APP_APPCON:	//应用层对收到的数据做应答
            //EditSecFra10(DLCON);
            //SendDataToMISI();
            break;
        case APP_APPDIABALE:	//应用层无效
            break;
        case APP_SENDCON:	//发送03命令
            //logSysMsgNoTime("DLSendProc 发送03命令%d",LengthOut,0,0,0);   //  debug ll
            EditFra68(DLSENDCON,LengthOut);
            SendDataToMISI();
            
            TimeOutTick_Pri=TimeDelay(LengthOut+8+5);
            DLPriStatus=PRISENDCON;
            RetryCount=MAXRETRYNUM;
            break;
        case APP_SENDNOCON:	//发送04命令 平衡模式
            EditFra68(DLSENDNOCON,LengthOut);
            SendDataToMISI();
            //logSysMsgNoTime("send04改变pritime",0,0,0,0);   //  debug ll
            TimeOutTick_Pri=0;
            break;
        case APP_SENDDATA://发送08命令
            EditSecFra68(DLRESDATA,LengthOut);
            SendDataToMISI();
            break;
        case APP_NODATA://应用层无数据
            if(!BalanMode)
            {
                if(!IsSendE5)
                    EditE5();
                else
                    EditSecFra10(DLNODATA);
                
                SendDataToMISI();
            }
            break;
        case APP_NOJOB://应用层无任务
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
    AppCommand = APP_NOJOB; //本次数据发送任务结束，清应用层命令为无任务
}

void CSecDLink::CallUData(void)//取紧急数据
{
    ScanFlag=FALSE;//取消预计的应用层调用
    LengthIn=0;
    DLCommand=DL_CALLUDATA;
    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);

    DLSendProc();
}

//平衡模式的遥控过程
void CSecDLink::CallUMsg(void)//取遥控消息
{
    ScanFlag=FALSE;//取消预计的应用层调用
    LengthIn=0;
    DLCommand=DL_CALLDBMSG;
    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);

    if(LengthOut!=0)
    {
        EditFra68(DLSENDCON,LengthOut);
        SendDataToMISI();
        //logSysMsgNoTime("CallUMsg改变pritime",0,0,0,0);   //  debug ll
        TimeOutTick_Pri = TimeDelay(LengthOut+8+5);
        DLPriStatus=PRISENDCON;
        RetryCount=MAXRETRYNUM;
    }
}

void CSecDLink::SendDataEnd(void)  //发送结束,平衡模式下
{
    if(DLPriStatus==PRIENABLE)
    {
        switch(LastControl)//最近一次发送的命令
        {
            case DLSENDNOCON://平衡模式，发送04命令后调用应用层服务
            case DLCON:
                //logSysMsgNoTime("SendDataEnd扫描应用层",0,0,0,0);//  debug ll
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

//通知应用层扫描
/*------------------------------------------------------------------/
函数名称：  NotifyToAppSchedule()
函数功能：  触发一次应用层扫描
输入说明：    
输出说明：  
备注：      
/------------------------------------------------------------------*/
void CSecDLink::NotifyToAppSchedule(void)  
{
    DLCommand=DL_SCAN1S;
    LengthIn=0;
    pSecApp->SecAppProc(RxdBuf+5+LinkAddrSize,LengthIn,DLCommand,
                        TxdBuf+5+LinkAddrSize,&LengthOut,&AppCommand);
    DLSendProc();
}
//防止建立链路死循环初始化
void CSecDLink::EndlessLoopInit(void)
{
    ELReplyFlag = TRUE;
    ELClearTime = 0;
    ELNoReplyTime = 0;
    ELResetNum = 0;
    
}
//防止建立链路死循环监视
//如果一段时间内（EL_NOREPLY_TIME）收到复位链路次数大于（EL_MAX_RECEIVE_RESETNUM）判断进入死循环过程，则不对复位链路进行双向复位链路响应。
//并且（EL_NOREPLY_TIME）时间段内不再响应主站复位链路命令。
void CSecDLink::EndlessLoopMonitor(void)
{
    
    if(ELClearTime)     //收到链路复位命令后，延时清复位次数
    {
        ELClearTime--;
        if(ELClearTime==0)
        {
            ELResetNum = 0;
        }   
    }
    
    if(ELNoReplyTime)   //判断进入死循环后，延时对主站复位命令不支持双向建立链路
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
               
        logSysMsgNoTime("程序进入死循环，%d秒不支持双向建立链路",EL_NOREPLY_TIME,0,0,0);
        
        ELResetNum = 0;
        ELReplyFlag = FALSE;
        ELNoReplyTime = EL_NOREPLY_TIME;
        
    }
    
    
    
}
