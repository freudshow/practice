
#include "new104dl.h"

extern INT16U AuthEndflag;
//void EbSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo);
//INT16U Pack104msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen);

New104DataLink::New104DataLink(struct PMySelf *pmyself,INT32U *pscheduleflag,BOOL *IsOK)
{
    int i;
    RxBuf=new INT8U[K*APDULEN];
    TxBuf=new INT8U[MAXRXLEN+(K+1)*APDULEN];

    EBRxBuf=new INT8U[K*APDULEN];
	EBTxBuf = new INT8U[APDULEN+100];//密文发送缓冲区初始化

    if (!(RxBuf&&TxBuf&&EBRxBuf&&EBTxBuf))
        *IsOK=FALSE;
    else
        *IsOK=TRUE;

    if (*IsOK==FALSE)
        return;

    RxHead=RxTail=0;
    EBRxHead=EBRxTail=0;
    FrameTailFlag=FALSE;
    pMySelf=pmyself;
    pScheduleFlag=pscheduleflag;

    INT8U *p=TxBuf;
    for(i=0;i<K+2;i++)
    {
        TxBufUnit[i].State=NoUse;
        if (i==0)
            TxBufUnit[i].Priority=PRIORITY_0;
        else if (i==1)
            TxBufUnit[i].Priority=PRIORITY_1;
        else
            TxBufUnit[i].Priority=PRIORITY_2;
        TxBufUnit[i].TxHead=TxBufUnit[i].TxTail=0;
        TxBufUnit[i].BufStart=p;
        if (i==0)
            p+=MAXRXLEN;
        else
            p+=APDULEN;
    }

    Tick[1].IsUse=FALSE;
    Tick[2].IsUse=FALSE;
    Tick[3].IsUse=FALSE;
    NR=0;
    NS=0;
    PeerNoAckNum=0;
    RxState=Start;
    CommConnect=FALSE;
    CurTxUnit=-1;
    IsEncrypt = 0;
	N104Encrptystyle = 0;
    YkStatusForTest2 = 0;
    
    NoJudgeFramNo = 0;
    RsvStartClearRSno = 0;
}

New104DataLink::~New104DataLink()
{
    if (RxBuf)
        delete RxBuf;
    if (TxBuf)
        delete TxBuf;
}

void New104DataLink::CloseTCP(void)
{
    MisiDisconnect(pMySelf->AppID);
}

void New104DataLink::BeginDT(void)  //开始数据传输
{
    INT8U TypeID;
    if (RxHead>=RxTail)
        RxHead=RxTail=0;
    //NR=NS=0;                        by wjr  080407
    PeerNoAckNum=0;
    CommConnect=TRUE;
    CurTxUnit=-1;                   
    
    YkStatusForTest2 = 0;   //ll 为广州测试临时修改 2012-3-24
   
    if(RsvStartClearRSno == 1)
    {
        Tick[1].IsUse=FALSE;
        Tick[2].IsUse=FALSE;
        Tick[3].IsUse=FALSE;
        NR=0;
        NS=0; 
        
        for (int i=0;i<K+2;i++)
            TxBufUnit[i].State=NoUse;
        
        ProgLogWrite2("端口%d,收到启动帧收发序号清0",pMySelf->AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 1);
    }
    for (int i=0;i<K+2;i++)
    {
        if ((TxBufUnit[i].State==WaitSend)||(TxBufUnit[i].State==WaitAck))
        {
            TypeID = TxBufUnit[i].BufStart[APCILEN];
            
            if(TypeID==M_SP_TB)
            {
                TxBufUnit[i].State=WaitSend;
            }
            else
                TxBufUnit[i].State=NoUse;
        }
        
    }

    BeginTick(3);
}

void New104DataLink::StopDT(BOOL closetcp)  //停止数据传输
{
    StopTick(1);
    StopTick(2);
    StopTick(3);
    CommConnect=FALSE;
    RxHead=RxTail=0;
    RxState=Start;
    if (closetcp)
        CloseTCP();
}

void New104DataLink::RxData(void)
{
    RxDataEnDealFun(N104Encrptystyle);   
}
/********************************************************************
*函数名称：RxDataEnDealFun
*功能：判别当前规约加密方式，处理密文或者明文数据
*输入：Enflag:加密类型，2表示1161国网加密，3表示
*                  1120a湖南农网加密，其他为明文或168号文处理
*研发人：张良
*********************************************************************/
void New104DataLink::RxDataEnDealFun(INT8U Enflag)
{
    INT16U RNum,TailFlag;
    INT16U tmpau = 0;
    INT16U Ennum = 0;
    if((Enflag!=2)&&(Enflag!=3))//明文通讯方式
    {
        if(RxHead<RxTail)
        {
            if (RxHead!=0)
            {
                memcpy(RxBuf, RxBuf+RxHead, (unsigned int)(RxTail-RxHead));
                RxTail-=RxHead;
                RxHead=0;
            }
        }
        else
        {
            RxHead=RxTail=0;
        }
        RNum=(INT16U)MisiRead(pMySelf->PortID,RxBuf+RxTail,(INT16U)(K*APDULEN-RxTail),&TailFlag);
        if (TailFlag&RX_FRAMETAIL)
        {
            FrameTailFlag=TRUE;
            NextFrmHeadPos=(TailFlag&(~RX_FRAMETAIL))+RxTail+1;
        }
        else
        {
            TailFlag=FALSE;
        }
        RxTail+=RNum;
    }
    else
    {
        if(EBRxHead<EBRxTail)
        {
            if (EBRxHead!=0)
            {
                memcpy(EBRxBuf, EBRxBuf+EBRxHead, (unsigned int)(EBRxTail-EBRxHead));
                EBRxTail-=EBRxHead;
                EBRxHead=0;
            }
        }
        else
        {
            EBRxHead=EBRxTail=0;   
        }     

        RNum=(INT16U)MisiRead(pMySelf->PortID,EBRxBuf+EBRxTail,(INT16U)(K*APDULEN-EBRxTail),&TailFlag);

        EBRxTail+=RNum;

        if(RxHead<RxTail)
        {
            if (RxHead!=0)
            {
                memcpy(RxBuf, RxBuf+RxHead, (unsigned int)(RxTail-RxHead));
                RxTail-=RxHead;
                RxHead=0;
            }
        }
        else
        {
            RxHead=RxTail=0;
        }
        
        tmpau = RxTail;
        
        if(Enflag == 2)
        {
            Ennum = EbSafetySearchFrame(EBRxBuf, RxBuf+RxTail, &RxTail, EBRxTail - EBRxHead, pMySelf->PortID);
        }
        else
        {
            Ennum = Eb1120aSafetySearchFrame(EBRxBuf, RxBuf+RxTail, &RxTail, EBRxTail - EBRxHead, pMySelf->PortID);
        }
        
         if (TailFlag&RX_FRAMETAIL)
        {
            FrameTailFlag=TRUE;
            NextFrmHeadPos=(TailFlag&(~RX_FRAMETAIL))+RxTail+1;
        }
        else
        {
            TailFlag=FALSE;
        }
        
        if(AuthEndflag != pMySelf->PortID)
        {
            memset(RxBuf+tmpau,0,RxTail-tmpau);           
        }
        
        EBRxHead += Ennum;       
    }  
}

#ifdef INCLUDE_ENCRYPT
void New104DataLink::ExeDLFunCode16(void)
{
    INT8U len;

    ProcEncryptFrame(&RxBuf[RxHead],EncryptBuf, &len);
    
    if(len)
        MisiWrite(pMySelf->PortID,(INT8U*)EncryptBuf, len,3);//帧首帧尾
   
}
#endif

PDARet New104DataLink::SearchFrm(BOOL *HaveData)
{
    INT16U RxLen,PeerNR;
    PDARet DARet=DA_NULL;
    INT8U rc;
    
    if (RxState==Start)
    {
    #ifdef INCLUDE_ENCRYPT
        if(IsEncrypt)
        {
            while ((RxBuf[RxHead]!=STARTCODE)
                    &&(RxBuf[RxHead]!= 0x16)
                    &&(RxHead<RxTail))
            {
                RxHead++;
            }
        }
        else
    #endif
        {
            while ((RxBuf[RxHead]!=STARTCODE)&&(RxHead<RxTail))
                RxHead++;
        }
        if (RxHead<RxTail)
            RxState=Head;
            
    #ifdef INCLUDE_ENCRYPT
        
        RxLen=RxTail-RxHead;
        if((RxBuf[RxHead] == 0x16)&& IsEncrypt )    //加密报文处理&& IsEncrypt
        { 
            BeginTick(3);  
            //logSysMsgWithTime("收到报文开始处理111",0,0,0,0);   //debug
            rc = CheckEncryptFramHead(&RxBuf[RxHead], RxLen);
            switch(rc)
            {
            case 0: //报文检查正确
                //logSysMsgWithTime("报文处理开始222",0,0,0,0);
                ExeDLFunCode16();
                //logSysMsgWithTime("报文处理完毕报文222",0,0,0,0); //debug
                RxHead+=(RxBuf[RxHead+1]+6);//头指针后移到报文后
                RxState=Start;
                
                break; 
            case 1: //帧头不对
                RxHead++;
                RxState=Start;
                //logSysMsgWithTime("帧头不对",0,0,0,0); //debug
                break;
            case 2: //未收完全
                RxState=Start;
                DARet=DA_NODATA;
                //logSysMsgWithTime("未收完",0,0,0,0); //debug
                break; 
            case 3: //校验和、解签错误
                RxHead+=6;
                RxState=Start;
                break;
              
            default:
                RxHead++;
                RxState=Start;
                break;    
            }
            
        }
    #endif //#ifdef INCLUDE_ENCRYPT 
    
    }

    
    RxLen=RxTail-RxHead;
    if (RxState==Head)
    {
        if (RxLen>=APCILEN) //APCILEN=6
        {
            pAPCI=(RxBuf+RxHead);
            apci.StartCode = pAPCI[0];
            apci.Length = pAPCI[1];
            apci.NS = MAKEWORD(pAPCI[2],pAPCI[3]);
            apci.NR = MAKEWORD(pAPCI[4],pAPCI[5]);
            
            if (apci.NS&0x01)
            {
                if(apci.Length == 4)    //ll 判下U和S帧的数据长度 
                {    
                    StopTick(1);   //figure 12
                    BeginTick(3);  //figure 13
                    if ((apci.NS&U_FRAME)==U_FRAME)  //U Frame
                        ProcUFrame();
                    else  //S Frame
                    {
                    
                        PeerNR=apci.NR>>1;
                        if (!ProcPeerNoAckNum(PeerNR))
                        {
                            //MyErrorInfo(104,"163 stopdt");
                            StopDT(TRUE);
                            *HaveData=FALSE;
                            return(DA_NULL);
                        }
                    }
                    RxState=Start;
                    RxHead+=APCILEN;
                }
                else
                {
                
                    //U和S帧长度不对，丢弃
                    RxHead++;
                    RxState=Start;   
                }
            }
            else   //I Frmae
            {
                if(apci.NR&0x01)
                {
                    //I帧的收序号异常   //ll 2012-9-5 解决收到无用加密报文时检帧的问题
                    RxHead++;
                    RxState=Start;
                    //logSysMsgNoTime("I帧收序号错误",0,0,0,0);
                }
                else
                {
                    RxState=Continue;
                }
            }
        }
        else
            DARet=DA_NODATA;
    };
    if (RxState==Continue)
    {
        pAPCI=(RxBuf+RxHead);
        apci.StartCode = pAPCI[0];
        apci.Length = pAPCI[1];
        apci.NS = MAKEWORD(pAPCI[2],pAPCI[3]);
        apci.NR = MAKEWORD(pAPCI[4],pAPCI[5]);
        if (RxLen>=(apci.Length+2))
        {
            PeerNR=apci.NR>>1;
            if (!ProcPeerNoAckNum(PeerNR))
            {
                StopDT(TRUE);
                *HaveData=FALSE;
                return(DA_NULL);
            }
            
            if(NoJudgeFramNo == 0)
            {
                //判帧序号
                if ((apci.NS)!= ((NR<<1) & 0xfffe))
                {
                    logSysMsgNoTime("收到I帧的发送序号错误，发送序号=%d,期待序号=%d",apci.NS>>1,NR,0,0); // ll
                    StopDT(TRUE);
                    *HaveData=FALSE;
                    return(DA_NULL);
                }
            }
            else
            {
                //不判帧序号
                if ((apci.NS)!= ((NR<<1) & 0xfffe))
                {
                    NR = apci.NS>>1;
                    logSysMsgNoTime("调整I帧的接收序号，主站发送序号=%d,接收序号=%d",apci.NS>>1,NR,0,0); // ll
                    
                }
                
            }
            
        #ifdef INCLUDE_ENCRYPT    
            //是否支持加密 
            if(IsEncrypt && 
                ((pAPCI[APCILEN]==C_SC_NA) || (pAPCI[APCILEN]==C_DC_NA))
              )
            {
                if(RxLen>=apci.Length+6) 
                {
                    if((pAPCI[apci.Length+2] != 0x16) || (pAPCI[apci.Length+5] != 0x16))
                    {
                        logSysMsgNoTime("104 无认证报文头，丢弃报文",0,0,0,0);
                        RxState=Start;
                        //无认证报文头，丢弃报文
                        RxHead += (apci.Length+2);
                        if (CommConnect)
                        {
                            StopTick(1);   //figure 12
                            BeginTick(2);  //figure 10
                            BeginTick(3);  //figure 13
                            NR++;
                            RxFrmNum++;    //应用层中对无I应答的数据要判RxFrmNum决定是否应答S
                            DARet=DA_NODATA;
                        }
                        
                        
                    }
                    else if (RxLen>=apci.Length+2+pAPCI[apci.Length+3]+6)//报文收全，进行处理
                    {
                        RxState=Start;
                        
                        if (CommConnect)
                        {
                            StopTick(1);   //figure 12
                            BeginTick(2);  //figure 10
                            BeginTick(3);  //figure 13
                            NR++;
                            RxFrmNum++;    //应用层中对无I应答的数据要判RxFrmNum决定是否应答S
                        }
                        //logSysMsgWithTime("收到报文开始处理222",0,0,0,0);   //debug 
                        rc = CheckEncryptFrame(&pAPCI[apci.Length+2], pAPCI, apci.Length+2);
                        if(rc == 0)
                        {
                           
                            RxHead+=(apci.Length+2+pAPCI[apci.Length+3]+6);//头指针后移到报文后 
                            
                            logSysMsgWithTime("104 收到主站加密遥控报文，验签正确.", 0,0,0,0);
                            
                            if (CommConnect)
                            {
                                DARet=DA_SUCCESS;
                            }
                            
                        }
                        else
                        {
                            RxHead+=(apci.Length+6);
                            logSysMsgNoTime("安全报文校验(校验和,签名,时间戳)不正确.rc=%d",rc,0,0,0);
                            if (CommConnect)
                            {
                                DARet=DA_NODATA;
                            }
                            
                        }
                        
                    }
                    else
                        DARet=DA_NODATA;
                }
                else
                    DARet=DA_NODATA;                                
                
                
            }
            else
        #endif        
            {
            
                if (CommConnect)
                {
                    StopTick(1);   //figure 12
                    BeginTick(2);  //figure 10
                    BeginTick(3);  //figure 13
                    NR++;
                    RxFrmNum++;    //应用层中对无I应答的数据要判RxFrmNum决定是否应答S
                    DARet=DA_SUCCESS;
                }
                RxHead+=(apci.Length+2);
            }
            RxState=Start;
        }
        else
            DARet=DA_NODATA;
    }

    if (DARet==DA_NODATA)
    {
    
        if (FrameTailFlag)
        {
            FrameTailFlag=FALSE;
            RxState=Start;
            /*RxHead=NextFrmHeadPos;
            
            if (RxTail>RxHead)
                *HaveData=TRUE;
            else*/
                *HaveData=FALSE;
        }
        else
            *HaveData=FALSE;
    }
    else
    {
    
        if (FrameTailFlag)
        {
            FrameTailFlag=FALSE;
            if (RxHead>NextFrmHeadPos)
                RxHead=NextFrmHeadPos;
        }
        
        if (RxTail>RxHead)
            *HaveData=TRUE;
        else
            *HaveData=FALSE;
    }
    return(DARet);
}

INT8U *New104DataLink::GetASDU(INT8U *FrameLen)
{
    *FrameLen=apci.Length-APCILEN;
    return(pAPCI+APCILEN);
}

void New104DataLink::ProcUFrame(void)  //处理U帧
{
   
    switch (LOBYTE(apci.NS))
    {
        case U_STARTDTACT:
            BeginDT();
            SendCtrlFrame(U_STARTDTCON);
            (*pScheduleFlag)|=SCHEDULE_INITOK;      //wjr初始化结束
            NotifyToAppSchedule();
            break;
        case U_STARTDTCON:
            BeginDT();
            myEventSend(pMySelf->AppTID,LINKINITOKFLAG);
            NotifyToAppSchedule();
            break;
        case U_STOPDTACT:
            SendCtrlFrame(U_STOPDTCON);
           // StopDT(TRUE);              wjr 080409
            StopDT(FALSE);
            break;
        case U_STOPDTCON:
            //StopDT(TRUE);            wjr 080409
            StopDT(FALSE);
            break;
        case U_TESTFRACT:
            if (PeerNoAckNum>=K)
            {
                //SendCtrlFrame(U_STOPDTACT);
                logSysMsgNoTime("超过%d帧报文未被确认,断开TCP",PeerNoAckNum,0,0,0); // ll
                StopDT(TRUE);
            }
            else
            {
                SendCtrlFrame(U_TESTFRCON);
                NotifyToAppSchedule();
            }
            break;
    }
}

BOOL New104DataLink::ProcPeerNoAckNum(INT16U PeerNR)  //处理对方确认帧
{
    INT16U TempPeerNoAckNum,i,j;
    struct PAPCI APCI;
    if (!CommConnect)
        return(TRUE);
    if (PeerNR<=NS)
        TempPeerNoAckNum=NS-PeerNR;
    else
        TempPeerNoAckNum=32767-PeerNR+NS;
    
    if(NoJudgeFramNo == 0)
    {  
        //判帧序号，原程序正常处理 
        if (TempPeerNoAckNum>PeerNoAckNum)  //TempPeerNoAckNum 还未确认的帧 ll remark
        {
            logSysMsgNoTime("本机发送序号=%d,对方确认序号=%d,未确认帧数=%d",NS,PeerNR,PeerNoAckNum,0); // ll
            return(FALSE);
        }
        
        //释放缓冲区中已经确认的帧 ll remark
        for(i=0;i<PeerNoAckNum-TempPeerNoAckNum;i++)    //PeerNoAckNum-TempPeerNoAckNum已经确认帧的个数 ll remark
        {
            if (PeerNR==0)
                PeerNR=32767;
            else
                PeerNR--;
            for(j=1;j<K+2;j++)
            {
                APCI.NS = MAKEWORD(TxBufUnit[j].BufStart[2],TxBufUnit[j].BufStart[3]);
                if ((APCI.NS>>1)==PeerNR)
                    TxBufUnit[j].State=NoUse;
            }
        }
        PeerNoAckNum=TempPeerNoAckNum;
    }
    else
    {
        //不判断帧序号，无需对方确认，把所有待确认的释放 
        for (i=0; i<K+2;i++)
        {
            if (TxBufUnit[i].State==WaitAck)
            {
                //APCI.NS = MAKEWORD(TxBufUnit[j].BufStart[2],TxBufUnit[j].BufStart[3]);
                TxBufUnit[i].State=NoUse;
            }
                
        }
        PeerNoAckNum = 0;
            
    }
    
    NotifyToAppSchedule();
    return(TRUE);
}

void New104DataLink::BeginTick(INT8U i)
{
    if ((i>3)||(!CommConnect))
        return;
    Tick[i].Count=Tick[i].Value;
    Tick[i].IsUse=TRUE;
}

void New104DataLink::StopTick(INT8U i)
{
    if ((i>3)||(!CommConnect))
        return;
    Tick[i].IsUse=FALSE;
}

void New104DataLink::SendCtrlFrame(INT16U FrameType)    //根据coverity更改
{
    TxBufUnit[0].State=WaitSend;
    INT8U *TxMsg = TxBufUnit[0].BufStart;

    TxMsg[0] = STARTCODE;
    TxMsg[1] = 4;
    TxMsg[2] = LOBYTE(FrameType);
    TxMsg[3] = HIBYTE(FrameType);
    TxMsg[4] = 0;
    TxMsg[5] = 0;

    ReferToMisi();
}

void New104DataLink::ReferToMisi(void)
{
    if (CurTxUnit>=0)
        return;
    else
        FindTxUnitToSend();
}

void New104DataLink::FindTxUnitToSend(void) //查找发送单元发送
{
    int i;
    struct PAPCI TxMsg;
    for (i=0;i<K+2;i++)
    {
        if (TxBufUnit[i].State==WaitSend)
        {
            TxBufUnit[i].State=WaitAck;
            CurTxUnit=i;
            TxStart=TxBufUnit[i].BufStart;
            TxMsg.Length = TxStart[1];
            TxMsg.NS = MAKEWORD(TxStart[2],TxStart[3]);
            TxMsg.NR = MAKEWORD(TxStart[4],TxStart[5]);

            TxHead=0;
            TxTail=TxMsg.Length+2;

            if (i==0)
            {
                if ((TxMsg.NS&U_FRAME)!=U_FRAME)  //S Frame
                {
                    TxMsg.NR=NR<<1;
                    RxFrmNum=0;
                    StopTick(2);
                }
                else
                {
                    if ((TxMsg.NS==U_STARTDTACT)||(TxMsg.NS==U_TESTFRACT))
                        BeginTick(1);
                    else
                        BeginTick(3);
                }
            }
            else
            {
                TxMsg.NS=(NS<<1)&0xFFFE;
                TxMsg.NR=NR<<1;
                RxFrmNum=0;
                NS++;
                NS=NS&0x7FFF;
                PeerNoAckNum++;
                BeginTick(1);   // 增加正常帧的发送无确认计时  ll 2014-3-14 用于快速检测通讯中断
                StopTick(2);
                BeginTick(3);
            }

            TxStart[2] = LOBYTE(TxMsg.NS);
            TxStart[3] = HIBYTE(TxMsg.NS);
            TxStart[4] = LOBYTE(TxMsg.NR);
            TxStart[5] = HIBYTE(TxMsg.NR);

            TxState=Send;
            SendToTeam();
            return;
        }
    }
    CurTxUnit=-1;
}

void New104DataLink::SendToTeam(void)
{
    SendToTeamEnDealFun(N104Encrptystyle);    
}
/********************************************************************
*函数名称：RecMISIDataDealFun
*功能：判别当前规约加密方式，处理密文或者明文数据
*输入：Enflag:加密类型，2表示1161国网加密，3表示
*                  1120a湖南农网加密，其他为明文或168号文处理
*研发人：张良
*********************************************************************/
void New104DataLink::SendToTeamEnDealFun(INT8U Enflag)
{
    INT16U i,rc,ebsendlen;
    INT8U *Packstar;
    
    if((Enflag!=2)&&(Enflag!=3))//明文通讯方式
    {
        if (TxState==Send)
        {
            i=(INT16U)MisiWrite(pMySelf->PortID,(INT8U*)(TxStart+TxHead),(INT16U)(TxTail-TxHead),3);//帧首帧尾
            if (i)
            {
                TxState=Wait;
            }
        }
        else
        {
            i=(INT16U)MisiWrite(pMySelf->PortID,(INT8U*)(TxStart+TxHead),(INT16U)(TxTail-TxHead),1);//有帧尾，无帧首
        }
        
        myTaskDelay(10);
        
        TxHead+=i;
        if (TxHead>=TxTail)
        {
            FindTxUnitToSend();
        }
    }
    else
    {
		ebsendlen = 0;
        if(AuthEndflag != pMySelf->PortID)//&&TxStart[0] != 0xEB)//主站认证完成之前不发送规约数据，否则会导致重新认证
        {
            memset(TxStart+TxHead,0,TxTail-TxHead);
            TxHead = TxTail = 0;
            return ;
        }
        if (TxState==Send)
        {
            Packstar = (INT8U*)(TxStart+TxHead);
            memcpy(EBTxBuf,Packstar,(INT16U)(TxTail-TxHead));
            
			if(Enflag == 2)
			{
			    rc = Pack104msgtoEb(EBTxBuf,(INT16U)(TxTail-TxHead),&ebsendlen,pMySelf->PortID);
			}
			else
			{
			    rc = Pack1120afor104msgtoEb(EBTxBuf,(INT16U)(TxTail-TxHead),&ebsendlen,pMySelf->PortID);
			}

			if(rc == 0)
            {
                return ;
            }
			if((ebsendlen - rc) > (TxTail-TxHead))
			{
                return ;
			}
			
            i=(INT16U)MisiWrite(pMySelf->PortID,(INT8U*)(EBTxBuf),(INT16U)(rc),3);//帧首帧尾
            if (i)
            {
                TxState=Wait;
            }
			
			i = TxTail - (ebsendlen - rc);
		}
		else
		{
		    i=(INT16U)MisiWrite(pMySelf->PortID,(INT8U*)(TxStart+TxHead),(INT16U)(TxTail-TxHead),1);//有帧尾，无帧首
		}
		
        UpgradeDataVerify(pMySelf->PortID);
        
        myTaskDelay(1);
        TxHead+=i;	//LW++180416
        if (TxHead>=TxTail)
        {
            FindTxUnitToSend();
        }        
    }
}

void New104DataLink::TxData(void)
{
    if (TxHead>=TxTail)
        FindTxUnitToSend();
    else
        SendToTeam();
}

void New104DataLink::TimeOut(void)
{
    if (Tick[1].IsUse)
    {
        Tick[1].Count--;
        if (Tick[1].Count==0)
        {
            StopTick(1);
            logSysMsgWithTime("发送帧超过T1时间未得到确认,断开TCP",0,0,0,0);  // ll
            StopDT(TRUE);
        }
    }
    if (Tick[2].IsUse)
    {
        Tick[2].Count--;
        if (Tick[2].Count==0)
        {
            StopTick(2);
            SendCtrlFrame(S_FRAME);
        }
    }
    if (Tick[3].IsUse)
    {
        Tick[3].Count--;
        if (Tick[3].Count==0)
        {
            StopTick(3);
            if (!PeerNoAckNum)
                SendCtrlFrame(U_TESTFRACT);
            else if (!Tick[1].IsUse)
                BeginTick(1);
        }
    }
}

BOOL New104DataLink::GetFreeTxUnit(INT8U Priority,INT8U **pTxMsg)  //得到空闲发送单元
{
    INT16U i,j=0;
    if (!CommConnect)
        return(FALSE);
    for (i=1;i<K+2;i++)
        if (TxBufUnit[i].State!=NoUse)
            j++;
    if (j>=K)
        return(FALSE);
    for (i=1;i<K+2;i++)
    {
        if ((TxBufUnit[i].State==NoUse)&&(TxBufUnit[i].Priority>=Priority))
        {
            *pTxMsg=(TxBufUnit[i].BufStart+APCILEN);
            return(TRUE);
        }
    }
    return(FALSE);
}

BOOL New104DataLink::SetThisUse(INT8U *BufStart)
{
    INT16U i;
    for (i=1;i<K+2;i++)
    {
        if (TxBufUnit[i].BufStart==BufStart)
        {
            TxBufUnit[i].State=WaitSend;
            return(TRUE);
        }
    }
    return(FALSE);
}

void New104DataLink::NotifyToAppSchedule(void)
{
    if (*pScheduleFlag)
        myEventSend(pMySelf->AppTID,SCHEDULEFLAG);
}

//发送确认帧
void New104DataLink::ConfS(void)
{
    if (RxFrmNum>=W)
    {
        StopTick(2);
        SendCtrlFrame(S_FRAME);
    }
}



