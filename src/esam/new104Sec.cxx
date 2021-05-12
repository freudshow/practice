/*********************************************************************************************/
/*  名称：                  IEC60870-5-104 从站端程序                                        */
/*  版权：                  烟台东方电子信息产业股份有限公司                                 */
/*  作者：                  岳振东                                                           */
/*  版本：                  v1.0                                                             */
/*  日期：                  2005/6/6                                                         */
/*                                                                                           */
/*                                                                                           */
/*  主要函数及功能：                                                                         */
/*                 new104sec(INT16U AppID)                      任务入口函数                   */
/*                 New104Sec(INT16U AppID,struct PortInfo_t **ppport,                          */
/*                           struct TaskInfo_t **pptask,BOOL *pInitOK)                       */
/*                                                            任务初始化函数                 */
/*                 OnRxData()                                 接收数据处理                   */
/*                 OnTimer()                                  定时器函数                     */
/*                 Schedule()                                 发送帧编辑函数                 */
/*                 OnCommState()                              接收通信状态变化事项           */
/*                 OnMessage()                                接收数据库发送的信息           */
/*                 SetDevInfo()                               设置设备信息                   */
/*                 InitPara()                                 任务参数初始化                 */
/*                                                                                           */
/*  修改历史：                                                                               */
/*           1.修改日期：                                                                    */
/*           2.修改者：                                                                      */
/*           3.修改内容：                                                                    */
/*********************************************************************************************/
#include "new104sec.h"
//线损文件同步  CL 20180504
#include "..\newhis\XSDataProc.h"
//线损文件同步  CL 20180504



extern struct DevConf_t *Device;
extern struct config_t MyConfig;
extern INT8U IsMainSta;
INT16U whTransLogID = 0xffff;
extern INT16U AuthEndflag;
extern struct XSFileSynInfo_t XSFileSynInfo;
//INT8U N104Encrptystyle=2;

void new104sec(INT16U AppID)
{
    UINT32 dwEvent;
    //UINT32 Resettimer;
    BOOL InitOk=TRUE;
    struct PortInfo_t *pport;
    struct TaskInfo_t *ptask;
    New104Sec *p104Sec;
    //myTaskDelay(3000); 
    p104Sec=new New104Sec(AppID,&pport,&ptask,&InitOk);
    if (!(p104Sec&&InitOk))
    {
        ptask->Status=0;
        #ifdef _CHINESE_
        logSysMsgNoTime("new104sec 任务创建失败",0,0,0,0);
        #else
        logSysMsgNoTime("Init Fail,new104sec Delete",0,0,0,0);
        #endif
        myTaskSuspendItself();
    }

    if(!MisiPortOpen(AppID,RX_AVAIL,TX_AVAIL,EX_STATE))
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("网络打开错误，new104sec 任务删除",0,0,0,0);
        #else
        logSysMsgNoTime("Net Open Fail,new104sec Delete",0,0,0,0);
        #endif
        delete p104Sec;
        myTaskSuspendItself();
    }
    
    /*wjr 2010.5.30  当通讯通上之后6分钟没有数据后断开重启，如果启动之后半个小时之后还没连接上就认为错误没有回复再重新启动*/
    p104Sec->LinkFlag = (INT16U *)nvramMalloc(sizeof(INT16U));
    
    if(((*(p104Sec->LinkFlag)) != 0x5555) && ((*(p104Sec->LinkFlag)) != 0xcccc))
    {
        (*(p104Sec->LinkFlag)) = 0xcccc;    
    }    
    //logSysMsgWithTime("标志地址%d 值%d",(INT16U)p104Sec->LinkFlag,*(p104Sec->LinkFlag),0,0);
    //tm_evafter(SYSCLOCKTICKS * 1800, EV_SYSRET, &Resettimer);
    /*wjr 2010.5.30 */
    if (!MisiGetLinkStatus(AppID))//检测链路是否有效，等待连接建立成功
    {
        for(;;)
        {
            
            myEventReceive(EX_STATE|EV_SYSRET,MY_EVENTS_WAIT_ANY|MY_EVENTS_KEEP_UNWANTED,WAIT_FOREVER,&dwEvent);
            {
                if(dwEvent&EX_STATE)
                    if (MisiGetLinkStatus(AppID))
                    {    
                        (*(p104Sec->LinkFlag)) = 0xcccc;        /*wjr 2010.5.30 */
                        logSysMsgWithTime("端口%d,104从站TCP链接建立,收发序号清0",AppID,0,0,0);  // ll
                        break;
                    }
                /*if(dwEvent&EV_SYSRET)          wjr 2010.5.30 
                {
                    if((*(p104Sec->LinkFlag)) == 0x5555)
                    {    
                        logSysMsgWithTime("reset net104 not link",0,0,0,0);
                        myTaskDelay(600);
                        SystemReset(WARMRESET);    
                    }
                }*/    
            }
        }
    }

    tm_evevery((INT32U)(SYSCLOCKTICKS),TIMERFLAG,&p104Sec->ScanTimerID);
    for(;;)
    {
        myEventReceive(SCHEDULEFLAG|RX_AVAIL|TX_AVAIL|EX_STATE|TIMERFLAG|UDATAFLAG|UMSGFLAG|XSFILESYNFINISH/*|CIDEXPLAINFINISHFLAG*/,
                                    MY_EVENTS_WAIT_ANY|MY_EVENTS_KEEP_UNWANTED,WAIT_FOREVER,&dwEvent);
        if(dwEvent&SCHEDULEFLAG)
            p104Sec->EnCodeNextFrame();

        if(dwEvent&UDATAFLAG)
            p104Sec->OnUData();

        if (dwEvent&UMSGFLAG)
            p104Sec->OnMessage();

        if(dwEvent&RX_AVAIL)
            p104Sec->OnRxData();

        if(dwEvent&TX_AVAIL)
            p104Sec->pDLink->TxData();

        if(dwEvent&EX_STATE)
            p104Sec->OnCommState();

        if(dwEvent&TIMERFLAG)
        {
            p104Sec->pDLink->TimeOut();
            p104Sec->OnTimer();
        }
	if(dwEvent & XSFILESYNFINISH)
        {
            p104Sec->ProcXSFileSynFinish();
        }
    }
}

New104Sec::New104Sec(INT16U AppID,struct PortInfo_t **ppport,struct TaskInfo_t **pptask,BOOL *pInitOK)
{
    INT8U rc;
    SetAllVarNull();

    if (GetTaskInfo(AppID,pptask)==TRUE)
    {
        MySelf.QID=(*pptask)->Qid;
        MySelf.AppTID=(*pptask)->Tid;
        MySelf.AppID=AppID;
        MySelf.PortID=(*pptask)->PortIDSet[0];
    }
    else
    {
        *pInitOK=FALSE;
        return;
    }

    pMsg=new PMessage;
    pDLink=new New104DataLink(&MySelf,&ScheduleFlag,pInitOK);

    if (*pInitOK==FALSE)
        return;

    if (!(pMsg&&pDLink))
    {
        *pInitOK=FALSE;
        return;
    }

    if (GetPortInfo(MySelf.PortID,ppport)==TRUE)
    {
        if ((*ppport)->DevNum<=0)
        {
            *pInitOK=FALSE;
            return;
        }
        pDev=new PDevInfo;
        if(!pDev)
        {
            *pInitOK=FALSE;
            return;
        }

        pDev->DevID=(*ppport)->DevIDSet[0];
        if ((*ppport)->CodePad!=NULL)
        {
            memcpy(&Sec104Pad,(*ppport)->CodePad,sizeof(struct	PSec104Pad));
            CheckPad();
        }
        else
        {
            #ifdef _CHINESE_
            logSysMsgNoTime("iec104Sec 无规约面板！请配置",0,0,0,0);
            #else
            logSysMsgNoTime("iec104Sec No CodePad! Please Check. ",0,0,0,0);
            #endif
            SetProtocalErr();
            SetDefaultPad();
        }

        if(!SetDevInfo(pDev->DevID))
        {
            *pInitOK=FALSE;
            return ;
        }
        InitPara();
    }
    else
    {
        *pInitOK=FALSE;
        return;
    }
    
    pDLink->IsEncrypt = 0;
    pDLink->N104Encrptystyle = 0;
    #ifdef INCLUDE_ENCRYPT    
        
        //CON_ENCRYPT:CON_1161ENCRPTY = 1:0(无加密)  0:0(11版安全方案) 1:1(16版安全防护方案)  0:1(非法)    
        switch(Sec104Pad.control & (CON_ENCRYPT|CON_1161ENCRPTY|CON_1120ENCRPTY))
        {
        case 0:
            pDLink->IsEncrypt = 1;
            myEnctyptInit(DevList[0].Addr, Sec104Pad.EncryptTimeout);
            logSysMsgNoTime("11版安全防护方案(SGC1126)-104",0,0,0,0);
            break;
        case CON_ENCRYPT|CON_1161ENCRPTY:
            //1161加密芯片
            pDLink->N104Encrptystyle = 2;
            logSysMsgNoTime("15版安全防护方案(SGC1161)-104",0,0,0,0);
            myTaskDelay(2);    
            rc = EncrptyChiptest(1);
            if(rc != 1)
            {
                logSysMsgNoTime("加密方案和芯片类型不匹配或加密芯片不存在",0,0,0,0);
            }
            break;
	 case CON_ENCRYPT|CON_1120ENCRPTY:
		//1120加密芯片
	     pDLink->N104Encrptystyle = 3;
           logSysMsgNoTime("湖南农网安全防护方案(SGC1120a)-104",0,0,0,0);
            myTaskDelay(2);    
	     rc = EncrptyChiptest(1);
           if(rc != 2)
           {
               logSysMsgNoTime("加密方案和芯片类型不匹配或加密芯片不存在",0,0,0,0);
           }
	     break;
        default:
            //无加密
            logSysMsgNoTime("无安全防护方案(透明传输)-104",0,0,0,0);
            break;               
        }
    #endif
    
    //支持"国网规约扩展"-2015版
    GYKZ2015Flag = FALSE;
    SendCOS = 1;
    if((Sec104Pad.control & CON_104GYKZ))
    {
        GYKZ2015Flag = TRUE;
        SendCOS = 0;
        if(Sec104Pad.control & CON_SENDCOS)
        {
            SendCOS = 1;
        }
        logSysMsgNoTime("104支持GY2015扩展",0,0,0,0);
    }
    
    if(Sec104Pad.control & CON_NOJUDGERSNO)
    {
        pDLink->NoJudgeFramNo = 1;
        logSysMsgNoTime("104规约不判帧序号",0,0,0,0);
    }
    
    if(Sec104Pad.control & CON_CLEARRSNO)
    {
        pDLink->RsvStartClearRSno = 1;
        logSysMsgNoTime("104规约收到启动命令清帧序号",0,0,0,0);
    }
    
    bSendAllDBI = FALSE;
    if(Sec104Pad.control & CON_ALLDBI_104)
        bSendAllDBI = TRUE;
    
    
    //新文件传输参数初始化
    ProcFileInit();
    RMTSectionNo2 = 1;
    RMTParaInit();
    GXParaInit();
    
}

New104Sec::~New104Sec()
{
    tm_cancel(ScanTimerID);

    if (pDev)
        delete pDev;
    if (pMsg)
        delete pMsg;
    if (pDLink)
        delete pDLink;
    if (DevList)
        delete []DevList;
}

void New104Sec::SetAllVarNull(void)
{
    pDev=NULL;
    pMsg=NULL;
    pDLink=NULL;
    DevList=NULL;
}

BOOL New104Sec::SetDevInfo(INT16U DevID)
{
    INT16U i,*temp;
    INT16 DevType;
    struct AppLBConf_t AppLBConf;
    struct AppSLBConf_t AppSLBConf;
    struct AppRBConf_t AppRBConf;

    DevType = CheckDevType(DevID);
    if(DevType < 0)
       return FALSE;
    
    if (DevType == 2)//二类逻辑设备
    {
        if(!SL_ReadBConf(DevID, MySelf.AppID, (INT8U *) &AppSLBConf))
            return FALSE;
        
        pDev->Flag=AppSLBConf.Flag;
        pDev->Addr=AppSLBConf.Address;
        pDev->MAddr=AppSLBConf.MAddress;
        pDev->DbaseWin=AppSLBConf.DbaseWin;
        pDev->RealWin=NULL;

        pDev->DevData.AINum=0;
        pDev->DevData.BINum=0;
        pDev->DevData.DBINum=0;
        pDev->DevData.CounterNum=0;
        pDev->DevData.BONum=0;
        pDev->DevData.SPINum=0;
        pDev->DevData.BCDNum=0;
        pDev->DevData.NvaNo=0;

        //管理的设备数目
        DevCount=AppSLBConf.DevNum;
        if (DevCount<=0)
        {
            return(FALSE);
        }

        DevList=new PDevInfo[DevCount];
        if(!DevList)
        {
            return(FALSE);
        }

        //取所有设备ID
        temp = (INT16U *)DBData;
        SL_ReadDevID(DevID, DevCount, temp);
        for(i=0;i<DevCount;i++)
            DevList[i].DevID=temp[i];

        //取设备信息
        for(i=0;i<DevCount;i++)
        {
            DevType = CheckDevType(DevList[i].DevID);
            if(DevType == 1)//实际设备
            {
                R_ReadBConf(DevList[i].DevID, MySelf.AppID,(INT8U *) &AppRBConf,1);

                DevList[i].Flag=AppRBConf.Flag;
                DevList[i].Addr=AppRBConf.Address;
                DevList[i].MAddr=AppRBConf.MAddress;
                DevList[i].DevData.AINum=AppRBConf.AINum;
                DevList[i].DevData.BINum=AppRBConf.HardBINum+AppRBConf.SoftBINum;
                DevList[i].DevData.CounterNum=AppRBConf.CounterNum;
                DevList[i].DevData.BONum=AppRBConf.BONum;
                //DevList[i].DevData.SPINum=AppRBConf.SPINum;
                //DevList[i].DevData.BCDNum=AppRBConf.BCDNum;
                DevList[i].DevData.SPINum=0;
                DevList[i].DevData.DBINum=0;           //wjr
                DevList[i].DevData.BCDNum=0;
                DevList[i].DevData.NvaNo=0;
                DevList[i].RealWin=AppRBConf.RealWin;
                DevList[i].DbaseWin=NULL;

            }
            else if(DevType == 0)//I类逻辑设备
            {
                L_ReadBConf(DevList[i].DevID, MySelf.AppID,(INT8U *) &AppLBConf);

                DevList[i].Flag=AppLBConf.Flag;
                DevList[i].Addr=AppLBConf.Address;
                DevList[i].MAddr=AppLBConf.MAddress;
                DevList[i].DevData.AINum=AppLBConf.AINum;
                DevList[i].DevData.BINum=AppLBConf.BINum;       
                DevList[i].DevData.CounterNum=AppLBConf.CounterNum;
                DevList[i].DevData.BONum=AppLBConf.BONum;
                //DevList[i].DevData.SPINum=AppLBConf.SPINum;
                //DevList[i].DevData.BCDNum=AppLBConf.BCDNum;
                DevList[i].DevData.SPINum=0;
                DevList[i].DevData.BCDNum=0;
                DevList[i].DevData.NvaNo=0;
                DevList[i].DbaseWin=AppLBConf.DbaseWin;
                DevList[i].RealWin=NULL;
                
                DevList[i].DevData.DBINum = 0;      //新版程序对双点遥信处理策略有了更改，使用原有参数会造成发送点号错乱。 ll 2017-7-19
                if(AppLBConf.DBINum > 0 )
                {
                    logSysMsgNoTime("新版程序对双点遥信参数设置要求不同，请按新要求修改双点遥信相关参数",0,0,0,0);   
                    return FALSE;
                }

            }
            else
            {
                return FALSE;
            }

            if(DevList[i].DevData.CounterNum)
            {
                DevList[i].DevData.CounterData=new RealCounter_t[DevList[i].DevData.CounterNum];
                if(DevList[i].DevData.CounterData==NULL)
                {
                    return(FALSE);
                }
                DevList[i].DevData.LastCounterData=new INT32U[DevList[i].DevData.CounterNum];
                if(DevList[i].DevData.LastCounterData==NULL)
                {
                    return(FALSE);
                }
            }
            else
            {
                DevList[i].DevData.CounterData=NULL;
                DevList[i].DevData.LastCounterData=NULL;
            }

            if(DevList[i].DevData.AINum)
            {
                DevList[i].DevData.AIMaxValTrue=new short[DevList[i].DevData.AINum];
                DevList[i].DevData.AIMaxVal=new short[DevList[i].DevData.AINum];
                DevList[i].DevData.AItype  =new INT16U[DevList[i].DevData.AINum];
                DevList[i].DevData.AIporperty  =new INT8U[DevList[i].DevData.AINum];
                DevList[i].DevData.AIData=new PNva[DevList[i].DevData.AINum];
                if(!(DevList[i].DevData.AIMaxVal&&DevList[i].DevData.AIData&&DevList[i].DevData.AIMaxValTrue))
                {
                    return(FALSE);
                }

                ReadAIMaxVal(i);
            }
            else
            {
                DevList[i].DevData.AIMaxVal=NULL;
                DevList[i].DevData.AIData=NULL;
            }
        }
        
        
        #ifdef SFILETRANAPP104
        
        if ((DevCount == 1) && (DevType == 0) && (Sec104Pad.YCNum <= DevList[0].DevData.AINum))
        {
            SFileInfo.AllYCNum = DevList[0].DevData.AINum;
            DevList[0].DevData.AINum = Sec104Pad.YCNum;
        }

        #endif
       
        
        return(TRUE);
    }
    else
    {
        return FALSE;
    }
}

void New104Sec::InitPara(void)
{
    INT32U rc=0;
    AllDataCount=0;
    CounterCount=0;
    NvaCount=0;
    NvaActDevNo=0;
    ScheduleFlag = 0;
    AllDataEnd=FALSE;
    CallDDFlag=FALSE;
    CallDD=FALSE;
    CallAllDataFlag=FALSE;
    InitFlag=0xFF;        //wjr
    ActDevIndex=0;
    
    LinkConnect=FALSE;            /*网络通上的标志  wjr  2010.5.21*/
    LinkBreakCounter = 0;         /*网路通的情况下没有收到数据的计数器 wjr  2010.5.21*/
    
    FirstCallAllData = 0;
    WaitCallAllDelay = 5;
    

    while(!rc)
        rc=ReceiveMSG(MySelf.QID,(void*)pMsg,MAXMSGLENGTH,ALWAYSRETURN);

    #ifdef	COTByte
    CotSize=1;
    #else
    CotSize=2;
    #endif

    #ifdef PUBADDRBYTE
    PubAddrSize=1;
    #else
    PubAddrSize=2;
    #endif

    #ifdef INFOADDR3BYTE
    InfoAddrSize=3;
    #else
    InfoAddrSize=2;
    #endif

    CotLocation=2;//COT在ASDU中的位置
    PubAddrLocation=CotLocation+CotSize;//PUBADDR在ASDU中的位置
    InfoAddrLocation=PubAddrLocation+PubAddrSize;//INFOADDR在ASDU中的位置
    AsduHeadLength=InfoAddrLocation+InfoAddrSize;//ASDU头的长度

    if(PubAddrSize==1)
        BroadCastAddr=0xff;
    else
        BroadCastAddr=0xffff;
    Revert=0;
    YKSetAlready = FALSE;
    //zzw
    pDLink->NR=0;
    pDLink->NS=0;                     //   by wjr  080407
    
    DBISOEnum=0;         
    DBICOSnum=0; 
    
    YKTypeID = 0;
    
}

void New104Sec::CheckPad(void)
{
    int i;

    if (!Sec104Pad.AllDataInternal)
        Sec104Pad.SendAllDataOnTime = 0;
    if (Sec104Pad.SendAllDataOnTime == 1)
        AllDataInterval=Sec104Pad.AllDataInternal*60;
    if (!pDev->DevData.CounterNum)
        Sec104Pad.SendCountOnTime = 0;
    if (!Sec104Pad.CountInternal)
        Sec104Pad.SendCountOnTime = 0;
    if (Sec104Pad.SendCountOnTime == 1)
        CounterInterval=Sec104Pad.CountInternal*60;

    if ((Sec104Pad.ScanData2==0)||(Sec104Pad.ScanData2>30))
        NvaInterval=SCANDATA2TIMER;
    else
        NvaInterval=Sec104Pad.ScanData2;

    if ((Sec104Pad.SBIType!=M_SP_NA)&&(Sec104Pad.SBIType!=M_PS_NA))
        Sec104Pad.SBIType=M_SP_NA;
    if ((Sec104Pad.AIType!=M_ME_ND)&&(Sec104Pad.AIType!=M_ME_NA)&&(Sec104Pad.AIType!=M_ME_NB)&& (Sec104Pad.AIType!=M_ME_NC))
        Sec104Pad.AIType=M_ME_NB;

    if((Sec104Pad.LBIinfoaddr<LBI)||(Sec104Pad.LBIinfoaddr>HBI))        //遥信信息体地址可设置 2008.11.5  wjr
        LBIinfoaddr=LBI;
    else
        LBIinfoaddr=Sec104Pad.LBIinfoaddr;
        
    if((Sec104Pad.LDBIinfoaddr<LDBI)||(Sec104Pad.LDBIinfoaddr>HDBI))
        LDBIinfoaddr=LDBI;
    else
        LDBIinfoaddr=Sec104Pad.LDBIinfoaddr;
    
    //LBIinfoaddr=LBI;    //新版程序对双点遥信处理策略有了更改，不再使用规约面板参数。 ll 2017-7-19
    //LDBIinfoaddr=LDBI;

    for (i=0;i<4;i++)
    {
        pDLink->Tick[i].IsUse=FALSE;
        pDLink->Tick[i].Value=Sec104Pad.TickValue[i];
    }
    //t2<t1  t3>t1
    if (pDLink->Tick[0].Value<MINT0)
        pDLink->Tick[0].Value=T0;
    if (pDLink->Tick[1].Value<MINT1)
        pDLink->Tick[1].Value=T1;
    if (pDLink->Tick[2].Value<MINT2)
        pDLink->Tick[2].Value=T2;
    if (pDLink->Tick[3].Value<MINT3)
        pDLink->Tick[3].Value=T3;
    /*if (pDLink->Tick[2].Value>=pDLink->Tick[1].Value)
    {
        pDLink->Tick[1].Value=T1;    //pre 1s
        pDLink->Tick[2].Value=T2;    //pre 1s
    }*/ //ll 2014-3-14 封 这样判断影响参数调整（十分不方便）。
    /*if (pDLink->Tick[3].Value<=pDLink->Tick[1].Value)
    {
        pDLink->Tick[1].Value=T1;    //pre 1s
        pDLink->Tick[3].Value=T3;    //pre 1s
    }*/
}

void New104Sec::SetDefaultPad(void)
{
    int i;

    Sec104Pad.ControlPermit = 1;       //遥控允许 1-允许，0-不允许 缺省为1
    Sec104Pad.SetTimePermit = 1;  //对钟允许 1-允许，0-不允许 缺省为1

    Sec104Pad.SendCountWithReset = 0;//发送电度时带复位，1-带复位 0-不带复位 缺省为0
    Sec104Pad.UseStandClock = 1;//使用标准时钟格式 1-标准 0-非标准 缺省为1
    Sec104Pad.AllDataInternal = ALLDATATIMER;//定时发送全数据间隔（分） 缺省30
    Sec104Pad.ScanData2=SCANDATA2TIMER;//二级数据扫描间隔
    Sec104Pad.CountInternal = COUNTERTIMER;//定时发送电度间隔（分） 缺省60
    Sec104Pad.TickValue[0] = T0;//TickValue[0]无用
    Sec104Pad.TickValue[1] = T1;//TickValue[1]确认无回答时间间隔（秒） 缺省15
    Sec104Pad.TickValue[2] = T2;//TickValue[2]发送确认帧时间间隔（秒） 缺省5
    Sec104Pad.TickValue[3] = T3;//TickValue[3]发送测试帧时间间隔（秒） 缺省30
    Sec104Pad.AIDeadValue=3;
    Sec104Pad.SBIType=M_SP_NA;//M_PS_NA;
    Sec104Pad.AIType=M_ME_NB;//M_ME_NA;

    AllDataInterval=Sec104Pad.AllDataInternal*60;
    CounterInterval=Sec104Pad.CountInternal*60;
    NvaInterval=Sec104Pad.ScanData2;

    for (i=0;i<4;i++)
    {
        pDLink->Tick[i].IsUse=FALSE;
        pDLink->Tick[i].Value=Sec104Pad.TickValue[i];
    }
    
    LBIinfoaddr=LBI;                     //遥信信息体地址可设置 2008.11.5  
    LDBIinfoaddr=LDBI;
}

void New104Sec::ReadAIMaxVal(INT16U DevIndex)  //读遥测满值——设置死区值
{
    INT16U i;
    INT16U DevID;
    INT16U val;
    INT16U deathval;
    INT8U ppty;
    
    DevID = DevList[DevIndex].DevID;

    for (i=0;i<DevList[DevIndex].DevData.AINum;i++)
    {
        if(DevList[DevIndex].Flag == 1)     //=1实际设备
            val = SRSendMax_ReadAI(DevID,i);
        else
            val = SLMax_ReadAI(DevID,i);
        
        DevList[DevIndex].DevData.AItype[i] = SL_ReadAI_Type(DevID,i); //读取当前数据的类型（有符号还是无符号）
        DevList[DevIndex].DevData.AIporperty[i] = SL_ReadAI_Porperty(DevID,i); //读取当前数据的属性(电流，电压，功率，频率)
        DevList[DevIndex].DevData.AIMaxValTrue[i] = val;
               
        
        //逐点设置死区值
        if(DevList[DevIndex].Flag == 1)
            deathval = SRDead_ReadAI(DevID,i);
        else
            deathval = SLDead_ReadAI(DevID,i);
        
        if(deathval > 1)    //非0，非1即认为有效
        {
            DevList[DevIndex].DevData.AIMaxVal[i] = deathval;   //不是千分比单位
            
        }
        else
        {
            ppty = DevList[DevIndex].DevData.AIporperty[i];
            deathval = GetRmtDeathvalue(ppty);
            if(deathval > 0)//远程参数死区值
            {
                DevList[DevIndex].DevData.AIMaxVal[i] = ((INT32U)val*deathval)/1000;
                
            }
            else //规约面板死区值
            {
                 DevList[DevIndex].DevData.AIMaxVal[i] = ((INT32U)val*Sec104Pad.AIDeadValue)/1000;
                 
            }
        }
        
        if (DevList[DevIndex].DevData.AIMaxVal[i]==0)
            DevList[DevIndex].DevData.AIMaxVal[i]=1;
    }
}

void New104Sec::OnRxData(void)
{
    int i;
    PDARet DARet;
    BOOL HaveData,Do=TRUE;
    
    /*为北京网络死的问题增加   2010.5.21*/
    if(LinkConnect == FALSE)
    {
        LinkConnect = TRUE;    
    }    
    else
    {
        LinkBreakCounter = 0;         /*网路通的情况下没有收到数据的计数器清零   2010.5.21*/	
    }
    
    pDLink->RxData();
    HaveData=TRUE;
    while ((HaveData==TRUE)&&(Do==TRUE))
    {
        DARet=pDLink->SearchFrm(&HaveData);
        if (DARet==DA_SUCCESS)
        {
            RxMsg=pDLink->GetASDU(&FrameLen);

            RxTypeID = RxMsg[0];
            RxVsq = RxMsg[1];
            RxCot=RxMsg[CotLocation];
            pRxData=RxMsg+AsduHeadLength;

            RxPubAddr=0;
            for(i=0;i<PubAddrSize;i++)
                RxPubAddr+=(RxMsg[PubAddrLocation+i]<<(8*i));
            if ((!GetActDevIndexByAddr(RxPubAddr))&&(RxPubAddr!=BroadCastAddr))
            {
                if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
                memcpy(TxMsg,RxMsg,FrameLen+2);

                TxMsg[2]=UNKNOWNPUBADDR;

                TxMsg[2]|=COT_PONO;

                EnCodeDLMsg(FrameLen+2);
                return;
            }

            RxInfoAddr=0;
            for(i=0;i<2;i++)//InfoAddrSize如果为3，也只取前2个字节。
                RxInfoAddr+=(RxMsg[InfoAddrLocation+i]<<(8*i));
            
            Do=ToProc();
        }
    }
}

BOOL New104Sec::ToProc(void)
{
    #ifdef SFILETRANAPP104
    BOOL    rc = TRUE;
    INT16U  len;
    #endif
    
    if (!pDLink->CommConnect)
        return(TRUE);
    
    pDLink->ConfS();
    
    switch(RxTypeID)
    {
        case C_RP_NA:
            return(SecResetUseP());
        case C_TS_TA:
            return(SecTestDL());
        case C_IC_NA:   //alldata
            EnCodeAllDataConf();
            if(FirstCallAllData==0xff)  //第一次总招不被打断 0xff-已经总招过，0-未总招过
                return(TRUE);
            else
                return(FALSE);
        case C_CI_NA:   //Counter
            EnCodeCounterConf();
            return(TRUE);
        case C_CS_NA:
            ProcClock();
            return(TRUE);
        case C_DC_NA:
        case C_RC_NA:
        case C_SE_NA:
        case C_SC_NA:    
            ProcControl();
            return(TRUE);
        case C_RD_NA:
            ProcReadData();
            return(TRUE);
        /*case P_ME_NA:
        case P_ME_NB:
            WritePara();*/
        /*    return(TRUE);*/
            //break;
        //广东远程参数
        case GD_MUTIPARA_READ:
            //if(IsRMTforGuangdong())
            ProcReadParaGD();
            break;
        case GD_MUTIPARA_WRITE:
            ProcWritePara_GD();
            break;
        //广西远程运维
        /*case P_RS_NA_1_GX:              //广西主站读参数
            ProcReadParaGX();
            break;
        case P_ME_NA_1_GX:               //广西主站预置参数
            ProcSetParaGX();
            break;
        case P_AC_NA_1_GX:               //广西主站激活参数
            ProcActivateParaGX();
            break;
        */
        case C_SR_NA:   //切换定值区号
            ProcSetSectionNo();
            break; 
        case C_RR_NA:
            ProcReadSectionNo();    //读定值区号
            break; 
        case C_RS_NA:       //读参数区
            ProcReadPara();
            break;  
        case C_WS_NA:       //写参数区
            ProcWritePara();
            break;
               
        case F_FR_NA_N:
            ProcFileTran(); //文件传输
            break; 
        case F_SR_NA_N:
            ProcFT_ProgramUpdate();
            break;   
        case F_FS_NA_N:
            ProcFileSyn();//文件同步 CL 20180306
            SetFileSynInfoTaskIDSubstation(MySelf.AppTID);
            if(XSFileSynInfo.TaskIDPri101[0]!=0)//代表第一个线损模块起的任务
            {
                myEventSend(GetFileSynInfoTaskID101(0),XSFILESYN);//给101主站任务发送消息 暂时先发给第一个101任务，后续是通过维护软件面板参数确认的。
            }
            else
            {
                logSysMsgWithTime("无支持2018标准的线损模块！",0,0,0,0);
            }
            if(XSFileSynInfo.TaskIDPri101[1]!=0)//代表有第二个线损模块起的任务
            {
                myEventSend(GetFileSynInfoTaskID101(1),XSFILESYN);
            }
            break;
        
        
        #ifdef SFILETRANAPP104
        case SFTYPESELECTFILE:
            SFileSelectProc(RxMsg);
            while(rc)
            {
                rc = SFileGetRcInfo(TxMsg, &len);
                if (len)
                {
                    EnCodeDLMsg(len);
                }
            }
            break;
        
        case SF_SC_NA:
            SFileCallProc(RxMsg);
            while(rc)
            {
                rc = SFileGetRcInfo(TxMsg, &len);
                if (len)
                {
                    EnCodeDLMsg(len);
                }
            }
            break;
        
        case SF_AF_NA:
            SFileConfirm(RxMsg);
            while(rc)
            {
                rc = SFileGetRcInfo(TxMsg, &len);
                if (len)
                {
                    EnCodeDLMsg(len);
                }
            }
            break;
        
        case SFTYPECALLYC:
            SCallYCProc(RxMsg);
            while(rc)
            {
                rc = SCallYcGetRcInfo(TxMsg, &len);
                if (len)
                {
                    EnCodeDLMsg(len);
                }
            }
            break;

        #endif
        
        #ifdef STANDARDFILETRANS104
        case F_SC_NA:
            StdGetFileInfo(RxMsg);
            StdProcFileTran();
            break;
        case F_AF_NA:
            StdGetFileInfo(RxMsg);
            StdProcFileAck();          
            break;
        #endif
        
        default:
        	if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
            {
                pDLink->ConfS();
                return(TRUE);
            }
            RxMsg[CotLocation]=UNKNOWNTYPEID;
            RxMsg[CotLocation]|=0x40;
            memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
            EnCodeDLMsg(FrameLen+2);
            break;
    }
    
    return(TRUE);
}

BOOL New104Sec::WritePara(void)
{
    INT8U Len;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        ScheduleFlag|=SCHEDULE_PROC;
        pDLink->ConfS();
        return(FALSE);
    }

    TxMsg[0]=RxTypeID;
    TxMsg[1]=RxVsq;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;

    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);

    TxMsg[InfoAddrLocation] = LOBYTE(RxInfoAddr);
    TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

	TxMsg[AsduHeadLength]=pRxData[0];
	TxMsg[AsduHeadLength+1]=pRxData[1];
	TxMsg[AsduHeadLength+2]=pRxData[2];

	Len=AsduHeadLength+3;
	EnCodeDLMsg(Len);
	return(TRUE);
 }

BOOL New104Sec::SecResetUseP(void)
{
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        ScheduleFlag|=SCHEDULE_PROC;
        pDLink->ConfS();
        return(FALSE);
    }

    TxMsg[0]=C_RP_NA;
    TxMsg[1]=1;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    switch(pRxData[0])
    {
        case QRPRESET:

            TxMsg[AsduHeadLength] = QRPRESET;
            EnCodeDLMsg(AsduHeadLength+1);
            //if (RxPubAddr==BroadCastAddr)        2009.4.2 北京测试时要求复位，不做地址判断
            {
                myTaskDelay(100);
                SystemReset(WARMRESET);
            }
            break;
        case QRPSOEIND:
            TxMsg[AsduHeadLength] = QRPSOEIND;
            EnCodeDLMsg(AsduHeadLength+1);
           // if (RxPubAddr==BroadCastAddr)   2009.4.2 北京测试时要求复位，不做地址判断
            {
                myTaskDelay(600);
                SystemReset(COLDRESET);
            }
            break;
        case QRPCOLD:
            TxMsg[AsduHeadLength] = QRPCOLD;
            EnCodeDLMsg(AsduHeadLength+1);
          //  if (RxPubAddr==BroadCastAddr)   2009.4.2 北京测试时要求复位，不做地址判断
            {
                myTaskDelay(600);
                SystemReset(COLDRESET);
            }
            break;
        default:
            TxMsg[2]|=COT_PONO;
            TxMsg[AsduHeadLength] = pRxData[0];
            if (RxPubAddr==BroadCastAddr)
                EnCodeDLMsg(AsduHeadLength+1);
            break;
    }
    return(TRUE);
}

BOOL New104Sec::SecTestDL(void)
{
    INT8U Len;
    struct Iec101ClockTime_t time;

    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        ScheduleFlag|=SCHEDULE_PROC;
        pDLink->ConfS();
        return(FALSE);
    }

    if (Sec104Pad.UseStandClock == 1)
        GetSysTime((void *)&time,IEC101CLOCKTIME);
    else
        GetSysTime((void *)&time,IEC101EXTCLOCKTIME);

    TxMsg[0]=C_TS_TA;              //2009.3.17修改，原来回的类型为C_RP_NA
    TxMsg[1]=1;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;
    TxMsg[AsduHeadLength]=0xAA;
	TxMsg[AsduHeadLength+1]=0x55;

    TxMsg[AsduHeadLength+2] = LOBYTE(time.MSecond);
    TxMsg[AsduHeadLength+3] = HIBYTE(time.MSecond);
    TxMsg[AsduHeadLength+4] = time.Minute;
    TxMsg[AsduHeadLength+5] = time.Hour;
    TxMsg[AsduHeadLength+6] = time.Day;
    TxMsg[AsduHeadLength+7] = time.Month;
    TxMsg[AsduHeadLength+8] = time.Year;

    Len=AsduHeadLength+2+7;
    EnCodeDLMsg(Len);
    return(TRUE);
}

void New104Sec::SecCallData2(void)  //召唤二级数据
{
    int i=Soe;      
    BOOL Stop=FALSE;
    
    Data2Seq=(enum PData2Seq)i;
    while(Stop==FALSE)
    {
        switch (Data2Seq)
        {
            case Soe:   //soe扫描不再使用定时扫描
                if (EnCodeSOE())        
                    Stop=TRUE;
                else
                {
                    i++;
                    Data2Seq=(enum PData2Seq)i;
                }
                break;
            case Nva:
                if (EnCodeNVA())
                    Stop=TRUE;
                else
                {
                    i++;
                    Data2Seq=(enum PData2Seq)i;
                }
                break;
            case End:
                Stop=TRUE;
                break;
        }
    }
}

BOOL New104Sec::GetActDevIndexByAddr(INT16U Addr)
{
    for (INT16U i=0;i<DevCount;i++)
    if (DevList[i].Addr==Addr)
    {
        ActDevIndex=i;
        return(TRUE);
    }
    return(FALSE);
}

BOOL New104Sec::GetActDevIndexByDevID(INT16U DevID)
{
    for (INT16U i=0;i<DevCount;i++)
    {
        if (DevList[i].DevID==DevID)
        {
            ActDevIndex=i;
            return(TRUE);
        }
    }
    return(FALSE);
}

void New104Sec::ProcControl(void)  //处理遥控
{
    INT8U sco,dco,OnOff;
    INT16U SwitchNo;
    Revert=0;
    YKTypeID = RxTypeID;                        
    
    SwitchNo = RxInfoAddr-LBO+1;
    
    //LogYkInfoRec(DevList[ActDevIndex].DevID, RxTypeID, *pRxData, RxInfoAddr, RxCot);  //记录主站发送的所有遥控信息
    
    if((RxCot&COT_TEST)==COT_TEST)//正常命令，（测试位为1则不能实际操作遥控）
    {
        if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
            return;
        TxMsg[0]=RxTypeID;
    	TxMsg[1]=1;
    	if((RxCot&COT_REASON)==ACT)
            TxMsg[2]=ACTCON;
    	else if((RxCot&COT_REASON)==DEACT)
            TxMsg[2]=DEACTCON;
    	else
        {
           	memcpy(TxMsg,RxMsg,AsduHeadLength+1);
            TxMsg[2]=UNKNOWNCOT;
            TxMsg[2]|=COT_PONO;
            EnCodeDLMsg(AsduHeadLength+1);
            return;
        }
    	if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;
    	for(int jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    	TxMsg[InfoAddrLocation]   = LOBYTE(RxInfoAddr);
    	TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
    	if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
            TxMsg[InfoAddrLocation+2]=0;
    	TxMsg[AsduHeadLength] = *pRxData;
    	EnCodeDLMsg(AsduHeadLength+1);
    	return;
    }
                        
    if(GetActDevIndexByAddr(RxPubAddr))
    {    
        if((SwitchNo-1)*2==DevList[ActDevIndex].DevData.BONum)                     //蓄电池维护
        {
            pDLink->ConfS();
            if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
            
            TxMsg[0]=RxTypeID;
            TxMsg[1]=1;
            if((RxCot&COT_REASON)==ACT)
                TxMsg[2]=ACTCON;
            else if((RxCot&COT_REASON)==DEACT)
                TxMsg[2]=DEACTCON;
            else
            {
            	memcpy(TxMsg,RxMsg,AsduHeadLength+1);
                TxMsg[2]=UNKNOWNCOT;
                TxMsg[2]|=COT_PONO;
                EnCodeDLMsg(AsduHeadLength+1);
                return;
            }
            if(CotSize==2)//传送原因为2字节时，高位固定为0。
                TxMsg[CotLocation+1]=0;
            for(int jj=0;jj<PubAddrSize;jj++)
                TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
        	TxMsg[InfoAddrLocation]   = LOBYTE(RxInfoAddr);
        	TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
        	if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxMsg[InfoAddrLocation+2]=0;
            TxMsg[AsduHeadLength] = *pRxData;
        	EnCodeDLMsg(AsduHeadLength+1);
        	if(((*pRxData)&DCO_SE)==0)  //执行
            {
                if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                    startCellMaint();
                TxMsg[2]=ACTTERM;
            	EnCodeDLMsg(AsduHeadLength+1);
            }
            
            return;                                                     //蓄电池维护结束
        }
        else if((SwitchNo-1)*2==DevList[ActDevIndex].DevData.BONum+2)  //复归级联设备  
        {
            pDLink->ConfS();
            if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
            	return;
            
            TxMsg[0]=RxTypeID;
            TxMsg[1]=1;
            if((RxCot&COT_REASON)==ACT)
                TxMsg[2]=ACTCON;
            else if((RxCot&COT_REASON)==DEACT)
                TxMsg[2]=DEACTCON;
            else
            {
            	memcpy(TxMsg,RxMsg,AsduHeadLength+1);
                TxMsg[2]=UNKNOWNCOT;
                TxMsg[2]|=COT_PONO;
                EnCodeDLMsg(AsduHeadLength+1);
                return;
            }
            if(CotSize==2)//传送原因为2字节时，高位固定为0。
                TxMsg[CotLocation+1]=0;
            for(int jj=0;jj<PubAddrSize;jj++)
                TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
        	TxMsg[InfoAddrLocation]   = LOBYTE(RxInfoAddr);
        	TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
        	if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxMsg[InfoAddrLocation+2]=0;
            TxMsg[AsduHeadLength] = *pRxData;
        	EnCodeDLMsg(AsduHeadLength+1);
        	if(((*pRxData)&DCO_SE)==0)  //执行
            {
                if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                {
                	ResetFaultInfoForCall_FaultCheck();
                	logSysMsgNoTime("复归级联设备",0,0,0,0);
                }
                    
                TxMsg[2]=ACTTERM;
            	EnCodeDLMsg(AsduHeadLength+1);
            }
            
            return;                                                     
        }
    }
    if((RxInfoAddr<LBO)||(RxInfoAddr>(LBO+(DevList[ActDevIndex].DevData.BONum/2)-1)))
    {
    	if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
            return;
        RxMsg[CotLocation]=UNKNOWNTINFOADDR;
        RxMsg[CotLocation]|=0x40;
        memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
        return;
    }     
    switch (RxTypeID)
    {
           
        case C_SC_NA:    //单点遥控命令
            sco = *pRxData;
            SwitchNo = RxInfoAddr-LBO+1;
            if ((sco & SCO_SCS) == 0)       //分
                OnOff = 2;
            else if ((sco & SCO_SCS) == 1)  //合
                OnOff = 1;
            if(Sec104Pad.ControlPermit == 0)//参数设置为不允许遥控
            {
                EnCodeNACK(UNKNOWNTYPEID); //否定确认 
                /*if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
                memcpy(TxMsg,RxMsg,AsduHeadLength+1);

                TxMsg[2]=UNKNOWNTYPEID;

                TxMsg[2]|=COT_PONO;

                EnCodeDLMsg(AsduHeadLength+1);*/
                return;
            }
            
            BODevIndex = ActDevIndex;
            
            if ((RxCot&COT_REASON)==ACT)//6，激活
            {
                if ((sco & SCO_SE) == SCO_SE)   //1，select
                {
                    
                    if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        SetYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    
                }
                else//0，执行
                {
                    if(pDLink->YkStatusForTest2 == 0)                    
                    {
                        EnCodeNACK(ACTCON); //否定激活确认                         
                        return;         //ll 为广州测试临时修改 执行之前必须预置 2012-3-24
                    }
                    pDLink->YkStatusForTest2 = 0;
                    
                    if(YKSetAlready == TRUE)
                        YKSetAlready = FALSE;
                    if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        ExecuteYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    
                }
            }
            else//
            {
                if((RxCot&COT_REASON)==DEACT)       //撤消
                    CancelYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                else
                {
                    EnCodeNACK(UNKNOWNCOT); //否定激活确认
                	/*if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                        return;
                    memcpy(TxMsg,RxMsg,AsduHeadLength+1);
    
                    TxMsg[2]=UNKNOWNCOT;
    
                    TxMsg[2]|=COT_PONO;
    
                    EnCodeDLMsg(AsduHeadLength+1);*/
                    return;
                }
            }
            pDLink->ConfS();
            break;
        case C_DC_NA:
            if (Sec104Pad.ControlPermit == 0)
            {
              /*  if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
                memcpy(TxMsg,RxMsg,AsduHeadLength+1);

                if (RxMsg[2] == DEACT)
                    TxMsg[2]=DEACTCON;
                else
                    TxMsg[2]=ACTCON;

                TxMsg[2]|=COT_PONO;

                EnCodeDLMsg(AsduHeadLength+1);*/
                if (RxMsg[2] == DEACT)
                {
                    EnCodeNACK(DEACTCON);
                }
                else
                {
                    EnCodeNACK(ACTCON);
                }
                return;
            }

            if (!GetActDevIndexByAddr(RxPubAddr))
            {
               /*if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
                memcpy(TxMsg,RxMsg,AsduHeadLength+1);

                if (RxMsg[2] == DEACT)
                    TxMsg[2]=DEACTCON;
                else
                    TxMsg[2]=ACTCON;

                TxMsg[2]|=COT_PONO;

                EnCodeDLMsg(AsduHeadLength+1);*/
                if (RxMsg[2] == DEACT)
                {
                    EnCodeNACK(DEACTCON);
                }
                else
                {
                    EnCodeNACK(ACTCON);
                } 
                return;
            }

            BODevIndex = ActDevIndex;
            dco = *pRxData;
           // SwitchNo = RxInfoAddr-LBO+1;
            if ((dco&DCO_DCS)==1)        //分
                OnOff = 2;
            else if ((dco&DCO_DCS)==2)  //合
                OnOff = 1;
            else
            {
            	if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
                memcpy(TxMsg,RxMsg,AsduHeadLength+1);

                TxMsg[2]|=COT_PONO;

                EnCodeDLMsg(AsduHeadLength+1);
                return;
            }

            if ((RxCot&COT_REASON)==ACT)//6，激活
            {
                if ((dco&DCO_SE) == DCO_SE)   //1，select
                {
                    
                    
                    if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        SetYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    if((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2))//北京故障复归，最后一个遥控的合
                    {
                        YKSetAlready = TRUE;
                    }
                }
                else//0，执行
                {
                    if(pDLink->YkStatusForTest2 == 0)
                    {
                        EnCodeNACK(ACTCON); //否定激活确认     
                        return;   //ll 为广州测试临时修改 2012-3-24
                    }
                    pDLink->YkStatusForTest2 = 0;
                    
                    if((YKSetAlready == FALSE)&&((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2)))//北京故障复归，最后一个遥控的合
                    {
                        Revert=1;
                        SetYK(MySelf.AppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    }
                    else
                    {
                        if(YKSetAlready == TRUE)
                            YKSetAlready = FALSE;
                        if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                            ExecuteYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    }
                }
            }
            else//撤消
            {
                if(((RxCot&COT_REASON)==DEACT)&&((RxCot&COT_TEST)==0))       //撤消
                    CancelYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                else
                {
                    if((RxCot&COT_TEST)==COT_TEST)
                    {
                        
                    }
                    EnCodeNACK(UNKNOWNCOT);
                    return;
                }
            }
            pDLink->ConfS();
            break;
        case C_RC_NA:
            break;
        case C_SE_NA:
            break;
    }
}

void New104Sec::ProcReadData(void)
{
    Data1.PubAddr=RxPubAddr;
    Data1.InfoAddr=RxInfoAddr;
    SendData1(SCHEDULE_DATA1_READDATA);
}

void New104Sec::ProcClock(void)  //处理对钟
{
    struct Iec101ClockTime_t Time;
    
    BOOL flag = FALSE;     //flag为1代表正常时间，为0代表异常时间，不判断星期几
    
    if((RxCot == REQ) || ((pRxData[6]+pRxData[5]+pRxData[4])==0))  //判断年月日为0
    {
        //读时钟
        EnCodeClock(PTL104_CLOCK_READ);
        return;
    }
    
    //后面是写时钟
    if (Sec104Pad.SetTimePermit == 1)
    {
        Time.MSecond = MAKEWORD(pRxData[0],pRxData[1]);
        Time.Minute  = (pRxData[2] & 0x3f);
        Time.Hour    = (pRxData[3] & 0x1f);
        Time.Day     = (pRxData[4] & 0x1f);
        Time.Month   = (pRxData[5] & 0xf);
        Time.Year    = (pRxData[6] & 0x7f);
    
        flag = IEC101TimeIsOK(&Time);
    }
    //if(flag && Sec104Pad.SetTimePermit)                    //如果flag为真，则Sec104Pad.SetTimePermit肯定为真，所以条件更改成只判断flag就可以
    if(flag)
    {
        
        
        if (Sec104Pad.UseStandClock == 1)
            SetSysTime(&Time,IEC101CLOCKTIME);
        else
            SetSysTime(&Time,IEC101EXTCLOCKTIME);  
        
        EnCodeClock(PTL104_CLOCK_WRITE);    //返回修改后的时钟
              
    }
    else
    {
        EnCodeClock(PTL104_CLOCK_WRITE|PTL104_CLOCK_NACK);                               //在从站不允许有对钟或者对钟时间有错误时发送传送原因为0x47,否定回答
    }    
}

void New104Sec::EnCodeNextFrame(void)
{          
    if(ScheduleFlag & SCHEDULE_INITOK)
    {
        ProcInitEnd();       
    }
    
    if ((ScheduleFlag&SCHEDULE_GROUP) && (FirstCallAllData==0))
        ProcGroupTrn();
    
    if((FirstCallAllData == 0))    //第1次总招不能被打断的处理时。
    {
        return; 
    }
    
    if ((ScheduleFlag&SCHEDULE_DATA1_BIENT)
        ||(ScheduleFlag&SCHEDULE_DATA1_DEVSTATUS)
        ||(ScheduleFlag&SCHEDULE_DATA1_BIET)
        ||(ScheduleFlag&SCHEDULE_DATA1_READDATA))
        EnCodeData1();

    if (ScheduleFlag & SCHEDULE_MSG)
        ProcTaskMsg();

    if (ScheduleFlag & SCHEDULE_PROC)
    {
        ScheduleFlag&=(~SCHEDULE_PROC);
        ToProc();
    }

    if (ScheduleFlag & SCHEDULE_GROUP)
        ProcGroupTrn();
    
    if(ScheduleFlag & SCHEDULE_FT_FREVENT)
        EnCodeFREvent();
    
    if (ScheduleFlag & SCHEDULE_RMTPARA)
        EncodeRMTReadPara();
    if (ScheduleFlag & SCHEDULE_GXPARA)
        ProcEncodeGXReadPara();
    if (ScheduleFlag & SCHEDULE_GXPARAEND)
        EnCodeGXReadParaEnd();
    if (ScheduleFlag & SCHEDULE_GXSENDPARA)
        ProcEncodeGXSendPara();    
        
        
    if (ScheduleFlag & SCHEDULE_FT_DATA)
        ProcFT_EncodeFileData();
    if (ScheduleFlag & SCHEDULE_FT_DIR)
        ProcFT_EncodeReadDir();
        
    #ifdef STANDARDFILETRANS104
    if (ScheduleFlag & SCHEDULE_FT_DIR_STD)
        StdProcFT_EncodeReadDir();
    if (ScheduleFlag & SCHEDULE_FT_FILE_READY_STD)
        StdProcFT_EncodeFileReady();
    if (ScheduleFlag & SCHEDULE_FT_SECTION_READY_STD)
        StdProcFT_EncodeSectionReady();
    if (ScheduleFlag & SCHEDULE_FT_DATA_STD)
        StdProcFT_EncodeSegment();
    if (ScheduleFlag & SCHEDULE_FT_LAST_SECTION_FILE_STD)
        StdProcFT_EncodeLastSegSect();
    #endif
    if (ScheduleFlag & SCHEDULE_XSFILESYNFINISH)
    {
        ProcXsFileSynFinish();
        ScheduleFlag&=(~SCHEDULE_XSFILESYNFINISH);
    }
    
}

void New104Sec::ProcGroupTrn(void)
{
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        return;
    }
    if (GroupTrn.TypeID==C_IC_NA)
        ProcAllData();
    else
        ProcCounter();
}

void New104Sec::ProcAllData(void)
{
    INT16U BeginNo,EndNo,Num,GroupNo = 17;   //根据coverity更改

    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))  //遥信组
    {
        if (CheckAndModifyGroup())
        {
            //GroupTrn.InfoAddr 是从LBIinfoaddr开始的。这样BeginNo总招是从0开始的，分组召唤不是从0
            //EndNo是一帧最大127个或BINum个 0x80的目的是控制128个数据一组

            BeginNo = GroupTrn.InfoAddr-LBIinfoaddr;
            if((DevList[GroupTrn.DevIndex].DevData.DBINum>0)&&(GroupTrn.GroupNo==1))
               	EndNo=DevList[GroupTrn.DevIndex].DevData.DBINum-1;
            else
                EndNo=GroupTrn.GroupNo*0x7f-1;
            
            if ((EndNo+1)>DevList[GroupTrn.DevIndex].DevData.BINum)
                EndNo=DevList[GroupTrn.DevIndex].DevData.BINum-1;
            
            if (EnCodeAllData(BeginNo,EndNo,&Num))
            {
                GroupTrn.First = FALSE;
                BeginNo += Num;
                GroupTrn.InfoAddr = BeginNo%0x1000 + LBIinfoaddr;
                    
                if(DevList[GroupTrn.DevIndex].DevData.DBINum>0)     //wjr 2009.8.27    双点遥信在第一组
                {
                    if((GroupTrn.GroupNo==1) && (BeginNo>=DevList[GroupTrn.DevIndex].DevData.DBINum))
                    {    
                        GroupNo=GroupTrn.GroupNo;
                        GroupNo++;  
                    }
                    else
                    {
                        if(GroupTrn.GroupNo>1)
                        {
                            GroupNo=GroupTrn.InfoAddr-LBIinfoaddr-DevList[GroupTrn.DevIndex].DevData.DBINum;
                            GroupNo=GroupNo/0x80+1;
                            GroupNo++;    
                        }    
                    }          
                }
                else
                {    
                    GroupNo  = GroupTrn.InfoAddr-LBIinfoaddr;
                    GroupNo /= 0x7f;
                    GroupNo++;
                }
                
                if ((GroupTrn.GroupNo!=GroupNo) && (BeginNo<DevList[GroupTrn.DevIndex].DevData.BINum))
                {
                    GroupTrn.GroupNo=GroupNo;
                    if ((GroupTrn.COT!=INTROGEN)&&(GroupTrn.COT!=BACK))//如果是分组召唤，则说明主站召唤的那组数据已经发完。
                    {
                        GroupTrn.GroupNo=17;    //发结束帧
                    }    
                }    
                    
                return;
            }
            else
            {
                if ((GroupTrn.COT==PERCYC)||(GroupTrn.COT==INTROGEN))
                    GroupTrn.GroupNo=9;
                else
                    GroupTrn.GroupNo=17;
            }
        }
    }
    if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=14)) //遥测组
    {
        if (CheckAndModifyGroup())
        {
            BeginNo=GroupTrn.InfoAddr-LAI;
            EndNo=(GroupTrn.GroupNo-8)*0x80-1;
            if ((EndNo+1)>DevList[GroupTrn.DevIndex].DevData.AINum)
                EndNo=DevList[GroupTrn.DevIndex].DevData.AINum-1;
            if (EnCodeAllData(BeginNo,EndNo,&Num))
            {
                GroupTrn.First=FALSE;
                BeginNo+=Num;
                GroupTrn.InfoAddr=BeginNo%0x1000+LAI;
                GroupNo=GroupTrn.InfoAddr-LAI;
                GroupNo/=0x80;
                GroupNo+=9;
                if (GroupTrn.GroupNo!=GroupNo)
                    GroupTrn.GroupNo=GroupNo;
                return;
            }
            else
            {
                if ((GroupTrn.COT==PERCYC)||(GroupTrn.COT==INTROGEN))
                    GroupTrn.GroupNo=17;
                else
                    GroupTrn.GroupNo=17;
            }
        }
    }

    if (GroupTrn.GroupNo==15)
    {
        if (CheckAndModifyGroup())
        {
            BeginNo=GroupTrn.InfoAddr-LBCD;
            EndNo=0x100;
            if ((EndNo+1)>DevList[GroupTrn.DevIndex].DevData.BCDNum)
                EndNo=DevList[GroupTrn.DevIndex].DevData.BCDNum-1;
            if (EnCodeAllData(BeginNo,EndNo,&Num))
            {
                GroupTrn.First=FALSE;
                BeginNo+=Num;
                GroupTrn.InfoAddr=BeginNo%0x100+LBCD;
                return;
            }
            else
            {
                if ((GroupTrn.COT==PERCYC)||(GroupTrn.COT==INTROGEN))
                    /*GroupTrn.GroupNo=15;
                else*/
                    GroupTrn.GroupNo=17;
            }
        }
    }
    if (GroupTrn.GroupNo==16)
    {
        //增加广西要求的送soe
        if (CheckAndModifyGroup())
        {
            if(EnCodeAllData(GroupTrn.SoeStartPtr,0,&Num) )
                return;
            else
                GroupTrn.GroupNo=17;
        }
        else
            GroupTrn.GroupNo=17;
        
    }
    if (GroupTrn.GroupNo>=17)
    {
        if (GetNextDev())
            ProcGroupTrn();
        else
        {
           
            AllDataEnd=TRUE;
            ScheduleFlag&=(~SCHEDULE_GROUP);
//           	pDLink->ConfS();
            if (GroupTrn.COT==INTROGEN)
            {
                FirstCallAllData = 0xff;
                EnCodeGroupEnd();
            }
            if (GroupTrn.COT==INTRO16)
            {
                EnCodeGroupEnd();
            }
            if(CallDDFlag)
            {
                EnCodeCounterConf();
                CallDDFlag=FALSE;
            }
        }
    }
}

void New104Sec::ProcCounter(void)
{
    INT8U len;
    INT16U BeginNo,EndNo,Num,GroupNo,i;
    for (i=0;i<DevCount;i++)
    {
        if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=15))
        {
            if (CheckAndModifyGroup())
            {
                BeginNo=GroupTrn.InfoAddr-LBCR;
                EndNo=GroupTrn.GroupNo*32-1;

                if ((EndNo+1)>DevList[GroupTrn.DevIndex].DevData.CounterNum)
                    EndNo=DevList[GroupTrn.DevIndex].DevData.CounterNum-1;

                len = EnCodeCounter(BeginNo,EndNo,&Num);

                if (len)
                {
                    GroupTrn.First=TRUE;
                    BeginNo+=Num;
                    GroupTrn.InfoAddr=BeginNo%0x200+LBCR;
                    GroupNo=GroupTrn.InfoAddr-LBCR;
                    GroupNo/=32;
                    GroupNo++;
                    if (GroupTrn.GroupNo!=GroupNo)
                        GroupTrn.GroupNo=GroupNo;
                    return;
                }
                else
                    GroupTrn.GroupNo=17;
            }
        }
        if(!GetNextDev())
        {
            AllDataEnd=TRUE;
            ScheduleFlag&=(~SCHEDULE_GROUP);
//           	pDLink->ConfS();
            if(!CallDD)
                return;
            CallDD=FALSE;
            CallDDFlag=FALSE;
            EnCodeGroupEnd();
            if(CallAllDataFlag)
            {
                EnCodeAllDataConf();
                CallAllDataFlag=FALSE;
            }
            return;
        }
    }
}

/*------------------------------------------------------------------/
函数名称：  CheckAndModifyGroup()
函数功能：  检测修改GroupTrn结构体，控制总招过程
输入说明：  
输出说明：  TRUE 有数据需要发送   FALSE 无数据发送
备注：      
/------------------------------------------------------------------*/
BOOL New104Sec::CheckAndModifyGroup(void)
{
    INT16U Num;
    INT16U wptr;
    
    switch (GroupTrn.TypeID)
    {
        case C_IC_NA:       //总招命令
            if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))
            {
                if(DevList[GroupTrn.DevIndex].DevData.DBINum>0)
                {
                    if(GroupTrn.GroupNo==1)
                    {
                    	if (GroupTrn.InfoAddr<LBIinfoaddr)
                            GroupTrn.InfoAddr=LBIinfoaddr;
                        else if (GroupTrn.InfoAddr>=DevList[GroupTrn.DevIndex].DevData.DBINum/2+LBIinfoaddr)
                        {
                            GroupTrn.GroupNo++;
                        }
                    }
                    else
                    {
                    	if (GroupTrn.InfoAddr<DevList[GroupTrn.DevIndex].DevData.DBINum+(GroupTrn.GroupNo-2)*0x80+LBIinfoaddr)
                            GroupTrn.InfoAddr=DevList[GroupTrn.DevIndex].DevData.DBINum+(GroupTrn.GroupNo-2)*0x80+LBIinfoaddr;
                        else if (GroupTrn.InfoAddr>=DevList[GroupTrn.DevIndex].DevData.DBINum+(GroupTrn.GroupNo-1)*0x80+LBIinfoaddr)
                        {
                            GroupTrn.GroupNo=GroupTrn.InfoAddr-LBIinfoaddr;
                            GroupTrn.GroupNo=(GroupTrn.GroupNo-DevList[GroupTrn.DevIndex].DevData.DBINum)/0x80+1;
                            GroupTrn.GroupNo++;
                        }
                      }
                }
                else
                {
                    if (GroupTrn.InfoAddr < (GroupTrn.GroupNo-1)*0x7f+LBIinfoaddr)  //用于分组召唤，调整到具体组那里？
                        GroupTrn.InfoAddr = (GroupTrn.GroupNo-1)*0x7f+LBIinfoaddr;
                    else if (GroupTrn.InfoAddr>=GroupTrn.GroupNo*0x7f+LBIinfoaddr)
                    {
                        GroupTrn.GroupNo=GroupTrn.InfoAddr-LBIinfoaddr;
                        GroupTrn.GroupNo/=0x7f;
                        GroupTrn.GroupNo++;
                    }
                }
                
                Num=GroupTrn.InfoAddr-LBIinfoaddr+1;
                if (Num>DevList[GroupTrn.DevIndex].DevData.BINum)
                {
                    if(GetDBINum()) //检测是否配置双点遥信
                    {
                        if((GroupTrn.COT==INTROGEN) && (GroupTrn.HaveSendDBI==FALSE))
                        {
                            //如果是总招且双点遥信没送，则组织双点遥信。分组召唤不送双点遥信
                            GroupTrn.InfoAddr = LBIinfoaddr;
                            GroupTrn.HaveSendDBI = TRUE;
                            
                            return TRUE;
                        }
                        
                    }
                    
                    
                    if ((GroupTrn.COT==PERCYC)||(GroupTrn.COT==INTROGEN))
                        GroupTrn.GroupNo=9;
                    else
                        GroupTrn.GroupNo=17;
                    return(FALSE);
                    
                }
                
                return(TRUE);
                
            }
            if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=14))
            {
                if (GroupTrn.InfoAddr<(GroupTrn.GroupNo-9)*0x80+LAI)
                    GroupTrn.InfoAddr=(GroupTrn.GroupNo-9)*0x80+LAI;
                else if (GroupTrn.InfoAddr>=GroupTrn.GroupNo*0x80+LAI)
                {
                    GroupTrn.GroupNo=GroupTrn.InfoAddr-LAI;
                    GroupTrn.GroupNo/=0x80;
                    GroupTrn.GroupNo+=9;
                }
                Num=GroupTrn.InfoAddr-LAI+1;
                if (Num>DevList[GroupTrn.DevIndex].DevData.AINum)
                {
                    if ((GroupTrn.COT==PERCYC)||(GroupTrn.COT==INTROGEN))
                        GroupTrn.GroupNo=17;
                    else
                        GroupTrn.GroupNo=17;
                    return(FALSE);
                }
                return(TRUE);
            }
            /*if (GroupTrn.GroupNo==13)
            {
                if (GroupTrn.InfoAddr<LSPI)
                    GroupTrn.InfoAddr=LSPI;
                Num=GroupTrn.InfoAddr-LSPI+1;
                if (Num>DevList[GroupTrn.DevIndex].DevData.SPINum)
                {
                    if ((GroupTrn.COT==PERCYC)||(GroupTrn.COT==INTROGEN))
                        GroupTrn.GroupNo=14;
                    else
                        GroupTrn.GroupNo=17;
                    return(FALSE);
                }
                return(TRUE);
            }
            if (GroupTrn.GroupNo==14)
            {
                if (GroupTrn.InfoAddr<LBCD)
                    GroupTrn.InfoAddr=LBCD;
                Num=GroupTrn.InfoAddr-LBCD+1;
                if (Num>DevList[GroupTrn.DevIndex].DevData.BCDNum)
                {
                    if ((GroupTrn.COT==PERCYC)||(GroupTrn.COT==INTROGEN))
                        GroupTrn.GroupNo=15;
                    else
                        GroupTrn.GroupNo=17;
                    return(FALSE);
                }
                return(TRUE);
            }     
            if (GroupTrn.GroupNo==15)
            {
                    GroupTrn.GroupNo=17;
                    return(TRUE);
            }*/
            if(GroupTrn.GroupNo == 16)
            {
                if(GroupTrn.First == TRUE)
                {
                    GroupTrn.First = FALSE;
                    
                    wptr = L_GetETWptr(DevList[0].DevID, &Num);
                    if((Num > 100) && (wptr != 0))
                    {
                        if(wptr > Num)
                        {
                            wptr = wptr%Num;
                            //最近100条
                            if(wptr >100) 
                                GroupTrn.SoeStartPtr = wptr-100;
                            else
                                GroupTrn.SoeStartPtr = wptr + Num -100;
                        }
                        else
                        {
                            if(wptr >100) 
                                GroupTrn.SoeStartPtr = wptr-100;
                            else
                                GroupTrn.SoeStartPtr = 0;

                        }
                        
                        logSysMsgNoTime("104soe召唤调试 start=%d, wptr=%d,max=%d",GroupTrn.SoeStartPtr,wptr,Num,0);
                    }
                    else
                    {
                        //错误
                        logSysMsgNoTime("104soe召唤错误 wptr=%d,max=%d",wptr,Num,0,0);
                        return FALSE;
                    }
                    
                }
                
                return(TRUE);
                 
            }
                        
            break;
        case C_CI_NA:
            if (GroupTrn.InfoAddr<(GroupTrn.GroupNo-1)*32+LBCR)
                GroupTrn.InfoAddr=(GroupTrn.GroupNo-1)*32+LBCR;
            else if (GroupTrn.InfoAddr>=GroupTrn.GroupNo*32+LBCR)
            {
                GroupTrn.GroupNo=GroupTrn.InfoAddr-LBCR;            
                GroupTrn.GroupNo/=32;
                GroupTrn.GroupNo++;
            }
            Num=GroupTrn.InfoAddr-LBCR+1;
            if (Num>DevList[GroupTrn.DevIndex].DevData.CounterNum)
            {
                GroupTrn.GroupNo=16;
                return(FALSE);
            }
            return(TRUE);
    }
    return(FALSE);
}

BOOL New104Sec::GetNextDev(void) //得到下一个轮询的设备
{
    if (GroupTrn.PubAddr==BroadCastAddr)
    {
        GroupTrn.DevIndex++;
        if (GroupTrn.DevIndex>=DevCount)
        {
            GroupTrn.DevIndex--;
            return(FALSE);
        }
        else
        {
            if (GroupTrn.TypeID==C_IC_NA)  //alldata
            {
                GroupTrn.GroupNo=1;
                GroupTrn.InfoAddr=LBIinfoaddr;
            }
            else
            {
                GroupTrn.GroupNo=1;
                GroupTrn.InfoAddr=LBCR;
            }
            return(TRUE);
        }
    }
    else
        return(FALSE);
}

void New104Sec::OnMessage(void)
{
    ScheduleFlag|=SCHEDULE_MSG;
    EnCodeNextFrame();
}

void New104Sec::OnUData(void)
{
    BOOL rc;

    rc = appGetJXStatus();
    if(rc)
    {
        clear_flag(pDev->DevID,BIENTFLAG);
        clear_flag(pDev->DevID,BIETFLAG);
        
        clear_SOE(pDev->DevID,BIENTFLAG);
        clear_SOE(pDev->DevID,BIETFLAG);
        return;
    }
    
    if (test_flag(pDev->DevID,BIENTFLAG))
    {
        clear_flag(pDev->DevID,BIENTFLAG);
        if(SendCOS == 1)
            SendData1(SCHEDULE_DATA1_BIENT);  
    }
    if (test_flag(pDev->DevID,BIETFLAG))
    {
        clear_flag(pDev->DevID,BIETFLAG);
        SendData1(SCHEDULE_DATA1_BIET);
    }
}


void New104Sec::OnCommState(void)
{
    if (MisiGetLinkStatus(MySelf.PortID))
        SetDevUseState(TRUE);
    else
        SetDevUseState(FALSE);
}

//设置设备使用状态
void New104Sec::SetDevUseState(BOOL InUse)
{
    INT32U rc=0;
    if (InUse)
    {
        pDLink->Tick[1].IsUse=FALSE;
        pDLink->Tick[2].IsUse=FALSE;
        pDLink->Tick[3].IsUse=FALSE;
        pDLink->NR=0;
        pDLink->NS=0;                  
        //InitFlag=0xff;
        FirstCallAllData = 0;
        ProcFileInit();   //清文件传输的参数
        //logSysMsgWithTime("104从站TCP链接建立,收发序号清0",0,0,0,0);  // ll
        ProgLogWrite2("端口%d,104从站TCP链接建立 收发序号清0",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 1);
        
    }
    else
    {
        //ll 检测通讯中断，撤销遥控。为广州测试修改 2014-3-14
        if(pDLink->YkStatusForTest2)
        {
            pDLink->YkStatusForTest2 = 0;    
            BspYkRelease();
            logSysMsgNoTime("104通讯中断撤销遥控",0,0,0,0);
        }
        
        ScheduleFlag = 0;   //断开链接后，清所有标志
        
        ProgLogWrite2("端口%d,104从站TCP断开链接",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 0);

        AllDataEnd=FALSE;
        while(!rc)
            rc=ReceiveMSG(MySelf.QID,(void*)pMsg,MAXMSGLENGTH,ALWAYSRETURN);
        if (pDLink->CommConnect!=InUse)
            pDLink->StopDT(FALSE);
    }
}
//每秒运行一次
void New104Sec::OnTimer(void)
{
    /*为北京网络死的问题增加  wjr 2010.5.21*/
    /*if(LinkConnect == TRUE)
    {
        LinkBreakCounter++;
        if(LinkBreakCounter > MAXBREAKNUM)
        {
            (*LinkFlag) = 0x5555;
            logSysMsgWithTime("net104 not rec",0,0,0,0);
            //logSysMsgWithTime("标志地址%d 值%d",(INT16U)LinkFlag,*LinkFlag,0,0);
            myTaskDelay(600);
            SystemReset(WARMRESET);    
        }        
    }*/
    

    //if ((!pDLink->CommConnect)||(!AllDataEnd))
    //if ((!pDLink->CommConnect)||(!FirstCallAllData))    //第1次总招不被打断 ll 2016-9-16
    //    return;
        
    if(WaitCallAllDelay)    //等待总招结束延时
    {
        WaitCallAllDelay--;
        if(FirstCallAllData)
            WaitCallAllDelay = 0;
    }
    else
        FirstCallAllData = 0xff;        
        
    if (Sec104Pad.SendCountOnTime == 1)
    {
        CounterCount++;
        if (CounterCount>=CounterInterval)
        {
            CounterCount=0;
            ScheduleFlag|=SCHEDULE_GROUP;
            GroupTrn.First=TRUE;
            GroupTrn.TypeID=C_CI_NA;
            GroupTrn.COT=PERCYC;
            GroupTrn.PubAddr=BroadCastAddr;
            GroupTrn.GroupNo=1;
            GroupTrn.InfoAddr=LBCR;
            GroupTrn.DevIndex=0;
            if (Sec104Pad.SendCountWithReset == 1)
            {
                GroupTrn.Description=FREEZERESET;
            }                
            else
            {
                if ((Sec104Pad.control & CON_104GYKZ))  //xwm 2017-08-02
                {
                    GroupTrn.Description = 0;
                }
                else
                {
                    GroupTrn.Description = FREEZENORESET;
                }
            }
                
            EnCodeNextFrame();
        }
    }
    if (Sec104Pad.SendAllDataOnTime == 1)
    {
        AllDataCount++;
        if (AllDataCount>=AllDataInterval)
        {
            AllDataCount=0;
            if (ScheduleFlag&SCHEDULE_GROUP)
                return;
            ScheduleFlag|=SCHEDULE_GROUP;
            GroupTrn.TypeID=C_IC_NA;
            GroupTrn.COT=PERCYC;
            GroupTrn.PubAddr=BroadCastAddr;
            GroupTrn.GroupNo=1;
            GroupTrn.InfoAddr=LBIinfoaddr;
            GroupTrn.DevIndex=0;
            EnCodeNextFrame();
        }
    }

    if ((ScheduleFlag&SCHEDULE_DATA1_BIENT)
        ||(ScheduleFlag&SCHEDULE_DATA1_DEVSTATUS)
        ||(ScheduleFlag&SCHEDULE_DATA1_BIET)
        ||(ScheduleFlag&SCHEDULE_DATA1_READDATA))
        EnCodeData1();

    if (ScheduleFlag&SCHEDULE_MSG)
        ProcTaskMsg();

    NvaCount++;
    if(NvaCount>NvaInterval)
    {
        NvaCount=0;
        //if ((ScheduleFlag==FALSE)&&(AllDataEnd==TRUE))    // 前面已经有防护了，这里就不需要判断总招了 ll 2016-9-16
        if((ScheduleFlag & 0x0fff) == FALSE)  //如果低24位有值不扫描
            SecCallData2();
    }
    
    CheckFREOnTime();
    RMTParaYzCheck();
    GXParaYzCheck();
    GXWatchLPChange();
    
    if (ScheduleFlag&SCHEDULE_FT_DATA)
        ProcFT_EncodeFileData();
    if (ScheduleFlag&SCHEDULE_FT_DIR)
        ProcFT_EncodeReadDir();
        
    #ifdef STANDARDFILETRANS104
    if (ScheduleFlag & SCHEDULE_FT_DIR_STD)
        StdProcFT_EncodeReadDir();
    if (ScheduleFlag & SCHEDULE_FT_FILE_READY_STD)
        StdProcFT_EncodeFileReady();
    if (ScheduleFlag & SCHEDULE_FT_SECTION_READY_STD)
        StdProcFT_EncodeSectionReady();
    if (ScheduleFlag & SCHEDULE_FT_DATA_STD)
        StdProcFT_EncodeSegment();
    if (ScheduleFlag & SCHEDULE_FT_LAST_SECTION_FILE_STD)
        StdProcFT_EncodeLastSegSect();
    #endif
}

void New104Sec::ProcTaskMsg(void)
{
    INT32U rc;
    BOOL Stop=FALSE;
    while(!Stop)
    {
        rc=ReceiveMSG(MySelf.QID,(void*)pMsg,MAXMSGLENGTH,ALWAYSRETURN);
        if (!rc)
        {
            if (pMsg->Head.Length<=sizeof(struct MSGHead_t))
                continue;
            if (!(pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg)&&(pDLink->CommConnect)))
                continue;
            switch (pMsg->Head.Command)
            {
                case SELECT:
                case OPERATE:
                    EnCodeCtrlRet();
                    break;
                default:
                    break;
            }
        }
        else
        {
            ScheduleFlag&=(~SCHEDULE_MSG);
            Stop=TRUE;
        }
    }
}

void New104Sec::SendData1(INT32U Flag)
{
    ScheduleFlag|=Flag;
    EnCodeNextFrame();
}

void New104Sec::EnCodeData1(void)
{
    BOOL rc;
    
    if (ScheduleFlag&SCHEDULE_DATA1_BIENT)
    {
        if(bSendAllDBI)
            rc = EnCodeBIENT_ALLDBI();
        else
            rc = EnCodeBIENT(); 
        if(rc == 0)
            ScheduleFlag&=(~SCHEDULE_DATA1_BIENT);
        else
            return;
    }
    
    if(ScheduleFlag & SCHEDULE_DATA1_BIET)
    {
        if(bSendAllDBI)
            rc = EnCodeSOE_ALLDBI();
        else
            rc = EnCodeSOE(); 
        if(rc == 0)
            ScheduleFlag&=(~SCHEDULE_DATA1_BIET);
        else
            return;
    }
    
    if (ScheduleFlag&SCHEDULE_DATA1_READDATA)
    {
        if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
            return;
        //ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //应答完数据再清除此标志 wjr 2009.3.31
        EnCodeReadData();
    }
}

void New104Sec::EnCodeAllDataConf(void) //全数据确认
{
    INT8U Len;
    
    if (ScheduleFlag&SCHEDULE_GROUP)
    {
        CallAllDataFlag=TRUE;
        pDLink->ConfS();
        
        return;
    }
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }

    TxMsg[0]=C_IC_NA;
    TxMsg[1]=1;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    TxMsg[AsduHeadLength] = RxMsg[AsduHeadLength];
    Len=AsduHeadLength+1;

    GroupTrn.PubAddr=RxPubAddr;
    if (GroupTrn.PubAddr==BroadCastAddr)
    {
        EnCodeDLMsg(Len);
        ScheduleFlag|=SCHEDULE_GROUP;
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.COT=INTROGEN;
        GroupTrn.PubAddr=DevList[0].Addr;
        GroupTrn.GroupNo=1;
        GroupTrn.InfoAddr=LBIinfoaddr;

        GroupTrn.DevIndex=0;
        EnCodeNextFrame();
    }
    else
    {
        if (GetActDevIndexByAddr(GroupTrn.PubAddr))
        {
            EnCodeDLMsg(Len);
            if(RxMsg[AsduHeadLength] == INTRO16)    //组16 单独处理 广西修改
            {
                logSysMsgWithTime("收到组16命令",0,0,0,0);
                ScheduleFlag   |= SCHEDULE_GROUP;
                GroupTrn.TypeID = C_IC_NA;
                GroupTrn.COT    =INTRO16;
                GroupTrn.PubAddr=RxPubAddr;
                GroupTrn.GroupNo=16;
                GroupTrn.InfoAddr=LBIinfoaddr;
                GroupTrn.HaveSendDBI = FALSE;
                GroupTrn.First  = TRUE;
                

                GroupTrn.DevIndex=ActDevIndex;
                EnCodeNextFrame();
            }
            else if(RxMsg[AsduHeadLength]>INTROGEN)      //wjr增加分组召唤 2009.3.31 
            {
                GroupTrn.DevIndex=ActDevIndex;
                GroupTrn.TypeID=C_IC_NA;
                GroupTrn.GroupNo=RxMsg[AsduHeadLength]-INTROGEN;
                GroupTrn.PubAddr=RxPubAddr;
                GroupTrn.InfoAddr=LBIinfoaddr;
                GroupTrn.COT=RxMsg[AsduHeadLength];
                GroupTrn.First=TRUE; 
                GroupTrn.HaveSendDBI = FALSE;
                
                ProcAllData();
                
                //组召唤 发结束帧
                TxMsg[0]=GroupTrn.TypeID;
                TxMsg[1]=1;
                TxMsg[2]=ACTTERM;
                if(CotSize==2)//传送原因为2字节时，高位固定为0。
                    TxMsg[CotLocation+1]=0;
                for(int jj=0;jj<PubAddrSize;jj++)
                    TxMsg[PubAddrLocation+jj]=GroupTrn.PubAddr>>(8*jj);
                TxMsg[InfoAddrLocation] = 0;
                TxMsg[InfoAddrLocation+1] = 0;
                if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                    TxMsg[InfoAddrLocation+2]=0;
                TxMsg[AsduHeadLength] = RxMsg[AsduHeadLength];
                
                EnCodeDLMsg(AsduHeadLength+1);
            }                                                                //增加结束2009.3.31
            else
            {
                ScheduleFlag|=SCHEDULE_GROUP;
                GroupTrn.TypeID=C_IC_NA;
                GroupTrn.COT=INTROGEN;
                GroupTrn.PubAddr=RxPubAddr;
                GroupTrn.GroupNo=1;
                GroupTrn.InfoAddr=LBIinfoaddr;
                GroupTrn.HaveSendDBI = FALSE;

                GroupTrn.DevIndex=ActDevIndex;
                EnCodeNextFrame();
            }
        }
    }
}

void New104Sec::EnCodeCounterConf(void) //电度确认
{
    INT8U Len;
    INT8U qcc;
    if (!AllDataEnd)
        return;
    if (ScheduleFlag&SCHEDULE_GROUP)
    {
        CallDDFlag=TRUE;
        pDLink->ConfS();
        return;
    }
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        return;
    }
    
    //转发给101主站进行瞬时冻结
    if((pRxData[0]&QCC_FRZ) == FREEZENORESET)  //FRZ=1 冻结不带复位功能
    {
        SendFreezeEvent2Pri101();
    }
    //转发给101主站进行瞬时冻结
    
    CallDD=TRUE;

    TxMsg[0]=C_CI_NA;
    TxMsg[1]=1;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    TxMsg[AsduHeadLength] = pRxData[0];
    Len=AsduHeadLength+1;

    GroupTrn.PubAddr=RxPubAddr;
    qcc = pRxData[0];
    if ((qcc&QCC_FRZ)==FREEZERESET)
    {
        GroupTrn.Description=FREEZERESET;
    }
    else
    {
        if (Sec104Pad.SendCountWithReset == 1)
        {
            GroupTrn.Description=FREEZERESET;
        }            
        else
        {
            if ((Sec104Pad.control & CON_104GYKZ))              // xwm  2017-08-02
            {
                GroupTrn.Description = 0;
            }
            else
            {
                GroupTrn.Description = FREEZENORESET;
            }
        }

    }
    if (GroupTrn.PubAddr==BroadCastAddr)
    {
        EnCodeDLMsg(Len);
        ScheduleFlag|=SCHEDULE_GROUP;
        GroupTrn.First=TRUE;
        GroupTrn.TypeID=C_CI_NA;
        GroupTrn.COT=REQCOGCN;
        GroupTrn.PubAddr=RxPubAddr;
        GroupTrn.GroupNo=1;
        GroupTrn.InfoAddr=LBCR;
        GroupTrn.DevIndex=0;
        EnCodeNextFrame();
    }
    else
    {
        if (GetActDevIndexByAddr(GroupTrn.PubAddr))
        {
            EnCodeDLMsg(Len);
            ScheduleFlag|=SCHEDULE_GROUP;
            GroupTrn.First=TRUE;
            GroupTrn.TypeID=C_CI_NA;
            GroupTrn.COT=REQCOGCN;
            GroupTrn.PubAddr=RxPubAddr;
            GroupTrn.GroupNo=1;
            GroupTrn.InfoAddr=LBCR;
            GroupTrn.DevIndex=ActDevIndex;
            EnCodeNextFrame();
        }
    }
}

void New104Sec::EnCodeGroupEnd(void)
{

    TxMsg[0]=GroupTrn.TypeID;
    TxMsg[1]=1;
    TxMsg[2]=ACTTERM;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=GroupTrn.PubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    if (TxMsg[0]==C_IC_NA) //alldata
        TxMsg[AsduHeadLength] = GroupTrn.COT;    //INTROGEN;
    else
        TxMsg[AsduHeadLength] = GroupTrn.Description|SUMMONBCRALL;

    EnCodeDLMsg(AsduHeadLength+1);
}

void New104Sec::EnCodeClock(INT8U flag)
{
    INT16U Len;
    struct Iec101ClockTime_t Time;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        return;
    }

    if (Sec104Pad.UseStandClock == 1)
        GetSysTime((void*)&Time,IEC101CLOCKTIME);
    else
        GetSysTime((void*)&Time,IEC101EXTCLOCKTIME);

    TxMsg[0]=C_CS_NA;
    TxMsg[1]=1;
    
    if(flag & PTL104_CLOCK_READ)
    {
        if(RxCot == REQ)
        {
            TxMsg[2] = REQ;
        }
        else
        {
            TxMsg[2]=UNKNOWNCOT;
            TxMsg[2]|=COT_PONO;
        }
        
    }
    else
    {
        if(RxCot == ACT )
        {
            if(flag & PTL104_CLOCK_NACK)
            {
            	TxMsg[2] = ACTCON| 0x40;
            }
            else
            {
            	TxMsg[2] = ACTCON ;
            }
        }
        else
        {
            TxMsg[2]=UNKNOWNCOT;
            TxMsg[2]|=COT_PONO;
        }
        
    }    
    
    
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    TxMsg[AsduHeadLength  ] = LOBYTE(Time.MSecond);
    TxMsg[AsduHeadLength+1] = HIBYTE(Time.MSecond);
    TxMsg[AsduHeadLength+2] = Time.Minute;
    TxMsg[AsduHeadLength+3] = Time.Hour;
    TxMsg[AsduHeadLength+4] = Time.Day;
    TxMsg[AsduHeadLength+5] = Time.Month;
    TxMsg[AsduHeadLength+6] = Time.Year;

    Len=AsduHeadLength+7;
    EnCodeDLMsg(Len);
}

BOOL New104Sec::EnCodeCtrlRet(void)  //遥控返校
{
    INT8U OnOff;
    INT8U dco=0;
    INT8U sco=0;
    INT8U Cmd;
    INT16U DeviceID,SwitchNo;

    short RetNo;
    struct YKMessage *YKMsgBuf = (struct YKMessage *)pMsg;

    ActDevIndex = BODevIndex;
    RetNo = CheckYK(&DeviceID,&SwitchNo,&OnOff,(INT8U *)YKMsgBuf);
    if(RetNo > 0)
    {
        switch(RetNo)
        {
            case 1://1-遥控预置成功
                Cmd = SELECT;
                pDLink->YkStatusForTest2 = 1;   //ll 为广州测试临时修改 2012-3-24
                break;
            case 2://2-遥控预置失败
                Cmd = SELECT;
                break;
            case 3://3-遥控执行成功
                Cmd = OPERATE;
                break;
            case 4://4-遥控执行失败
                Cmd = OPERATE;
                break;
            case 5://5-遥控撤消成功
                Cmd = SELECT;
                break;
            case 6://6-遥控撤消失败
                Cmd = SELECT;
                break;
            default:
                return(FALSE);
        }
    }
    else
    {
        return(FALSE);
    }

    TxMsg[0]=C_DC_NA;
    switch(YKTypeID)
    {
        case C_SC_NA:
            TxMsg[0]=C_SC_NA;
            break;
        case C_DC_NA:
            TxMsg[0]=C_DC_NA;
            break;
   }
    TxMsg[1]=1;

    if (Cmd == SELECT)
    {
        if((RetNo == 1)&&(Revert==1)&&(OnOff == 1)&&(SwitchNo==DevList[BODevIndex].DevData.BONum/2))//北京故障复归执行
        {
            Revert=0;
            ExecuteYK(MySelf.AppID,DeviceID,SwitchNo,OnOff);
            return  FALSE;
        }

        if((RetNo == 5)||(RetNo == 6))//撤消
        {
            TxMsg[2]=DEACTCON;
        }
        else
        {
            TxMsg[2]=ACTCON;
        }

        if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;

        if(YKTypeID==C_SC_NA)
            sco |= SCO_SE;//0x80 预置标志位
        else
            dco |= DCO_SE;
    }
    else
    {
        TxMsg[2]=ACTCON;
        if(YKTypeID==C_SC_NA)
            sco &= ~SCO_SE;
        else
            dco &= ~DCO_SE;
    }

    if ((RetNo & 1) == 0)//2,4,6 失败
    {
        if(GetYKRYBState() == TRUE)
            TxMsg[2]|=0x40;
        else
        {
            TxMsg[2] = COT_YKRYBERR;    //返回遥控软压板错误 ll 
            TxMsg[2] |= COT_PONO;
        }
    }
    else//1,3,5成功
        TxMsg[2]&=(~0x40);

    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation]   = LOBYTE((SwitchNo%0x80)+LBO-1);
    TxMsg[InfoAddrLocation+1] = HIBYTE((SwitchNo%0x80)+LBO-1);;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    if(YKTypeID==C_SC_NA)
    {
        if (OnOff == 1)  //he
            sco |=1;
        else if (OnOff == 2)  //fen
            sco &=0xfe;

        TxMsg[AsduHeadLength] = sco;
    }
    else
    {
        if (OnOff==1)  //he
            dco |= 2;
        else if (OnOff==2)  //fen
            dco |= 1;
        else
            dco |= OnOff;
        TxMsg[AsduHeadLength] = dco;
    }
    EnCodeDLMsg(AsduHeadLength+1);
    if(Cmd==OPERATE)
    {
        TxMsg[2]=ACTTERM;
        EnCodeDLMsg(AsduHeadLength+1);
    }
    if(Cmd==SELECT)
    {
        if((RetNo == 5)||(RetNo == 6))
        {
            TxMsg[2]=ACTTERM;
            EnCodeDLMsg(AsduHeadLength+1); 
        }   
    }
    return(TRUE);

}

void New104Sec::EnCodeReadData(void) //读数据
{
    int No;
    INT16U Num;
    
    GroupTrn.DevIndex=ActDevIndex;
    GroupTrn.PubAddr=Data1.PubAddr;
    GroupTrn.InfoAddr=Data1.InfoAddr;
    GroupTrn.COT=REQ;
    GroupTrn.First=TRUE;
    No=-1;
    if ((GroupTrn.InfoAddr>=LDBIinfoaddr)&&(GroupTrn.InfoAddr<=HDBI))
    {
        GroupTrn.TypeID=C_IC_NA;
        if(GroupTrn.InfoAddr>=(LDBIinfoaddr+DevList[GroupTrn.DevIndex].DevData.DBINum/2))   /*读的数据为单点遥信则   wjr2009.8.25*/
        {
            if((GroupTrn.InfoAddr<LBIinfoaddr) || (GroupTrn.InfoAddr>=LBIinfoaddr+DevList[GroupTrn.DevIndex].DevData.BINum))
            {
                ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //应答完数据再清除此标志 wjr 2009.3.31
                if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
            
                  RxMsg[CotLocation]=UNKNOWNTINFOADDR;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
                EnCodeDLMsg(FrameLen+2);
                return;
            }
            else
            {    
                No=GroupTrn.InfoAddr-LBIinfoaddr;
                No=No+DevList[GroupTrn.DevIndex].DevData.DBINum;
                GroupTrn.InfoAddr += DevList[GroupTrn.DevIndex].DevData.DBINum;
                if(DevList[GroupTrn.DevIndex].DevData.DBINum>0)
                    GroupTrn.GroupNo=(GroupTrn.InfoAddr-LBIinfoaddr)/0x80+2;
                else
                    GroupTrn.GroupNo=(GroupTrn.InfoAddr-LBIinfoaddr)/0x80+1;    
            }
        }    
        else
        {                                                                                 /*读的数据为双点遥信*/
            No=GroupTrn.InfoAddr-LDBIinfoaddr;
            GroupTrn.GroupNo=1;
        }
    }
    else if ((GroupTrn.InfoAddr>=LAI)&&(GroupTrn.InfoAddr<=LAI+DevList[GroupTrn.DevIndex].DevData.AINum-1))
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=(GroupTrn.InfoAddr-LAI)/0x80+9;
        No=GroupTrn.InfoAddr-LAI;
    }
    else if ((GroupTrn.InfoAddr>=LBCR)&&(GroupTrn.InfoAddr<(LBCR+DevList[GroupTrn.DevIndex].DevData.CounterNum)))
    {
        GroupTrn.TypeID=C_CI_NA;
        GroupTrn.GroupNo=(GroupTrn.InfoAddr-LBCR)/32+1;
        No=GroupTrn.InfoAddr-LBCR;
    }
    else if ((GroupTrn.InfoAddr>=LSPI)&&(GroupTrn.InfoAddr<=HSPI))
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=13;
        No=GroupTrn.InfoAddr-LSPI;
    }
    else if ((GroupTrn.InfoAddr>=LBCD)&&(GroupTrn.InfoAddr<=HBCD))
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=14;
        No=GroupTrn.InfoAddr-LBCD;
    }
    else if (GroupTrn.InfoAddr==RTUSTATUS)
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=15;
        No=0;
    }
    else
    {
        ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //应答完数据再清除此标志 wjr 2009.3.31
        if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
            return;
            
    	RxMsg[CotLocation]=UNKNOWNTINFOADDR;
        RxMsg[CotLocation]|=0x40;
        memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
        return;
    }

    if ((No>=0)&&CheckAndModifyGroup())
    {
        if(GroupTrn.TypeID==C_IC_NA)
            EnCodeAllData((INT16U)No,(INT16U)No,&Num);
        else
              EnCodeCounter((INT16U)No,(INT16U)No,&Num);                                 
        ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //应答完数据再清除此标志 wjr 2009.3.31
        return;
       
    }
    ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //应答完数据再清除此标志 wjr 2009.3.31
    
    pDLink->ConfS();
}

BOOL New104Sec::EnCodeDLMsg(INT16U Len)
{
    INT8U *pAPCI=TxMsg-sizeof(struct PAPCI);
    pAPCI[0]=STARTCODE;
    pAPCI[1]=Len+4;
    if(!pDLink->SetThisUse((INT8U *)pAPCI))
        return(FALSE);
    pDLink->ReferToMisi();
    pDLink->NotifyToAppSchedule();
    return(TRUE);
}


//ReadDB
INT16U New104Sec::EnCodeAllData(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT16U Len=0;
    
    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))
    {
        Len=EnCodeAllYX(BeginNo,EndNo,pNum);
    }
    else if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=14))
    {
        Len=EnCodeAllYC(BeginNo,EndNo,pNum);
    }
/*    else if (GroupTrn.GroupNo==13)
    {
        Len=EnCodeAllST(BeginNo,EndNo,pNum);
    }
    else if (GroupTrn.GroupNo==14)
    {
        Len=EnCodeAllBCD(BeginNo,EndNo,pNum);
    }*/
    else if (GroupTrn.GroupNo==15)
    {
        Len=EnCodeAllSta(BeginNo,EndNo,pNum);
    }
    else if (GroupTrn.GroupNo==16)
    {
        Len = EnCodeAllLastSoe(BeginNo);  //召唤最近100条SOE  Len 表示本次是否有数据发送
    }
    
    return(Len);
}
/*------------------------------------------------------------------/
函数名称：  EnCodeLastSoe()
函数功能：  上送最近的100条SOE（广西远程维护）
输入说明：  BeginNo soe起始指针
            
输出说明：  FALSE 表示没数据了，清标志， 
            TRUE 表示还有数据要发送
备注：      BeginNo 初始化时确定好了，本函数从BeginNo发送到结束即可，每帧最多18帧
/------------------------------------------------------------------*/
INT16U New104Sec::EnCodeAllLastSoe(INT16U BeginNo)
{
    INT8U Len=0,Status;
    short Num,FramePos;
    INT8U *TxData;
    INT16U SendNum,Length,WritePtr,i,j,jj;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    for (i=0;i<DevCount;i++)
    {
        
        if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
            return(TRUE);

        TxMsg[0]=M_SP_TB;   //带时标的单点信息
        TxMsg[1]=0;
        TxMsg[2]=INTRO16;  //通过组16召唤的，所以传送原因位组16
        if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);

        TxData = TxMsg+AsduHeadLength;
        Len = 0;
        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIET(DevList[i].DevID, BeginNo, 20, (struct BIEWithTimeData_t *)DBData, &WritePtr);//
        else
            Num=L_ReadSBIET(DevList[i].DevID, BeginNo, 20, (struct BIEWithTimeData_t *)DBData, &WritePtr);
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;
            p=(struct BIEWithTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-12;//250-9-12=229为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {
                /*if(p->Status & BIDBI_STATUSE)    //SOE的状态位BIDBI_STATUSE（0x10）表示是双点遥信
                {
                    DBIDBData[DBISOEnum]=(struct BIEWithTimeData_t)(*p);
                    DBISOEnum++;
                	j++;
                	p++;
                }
                else*/  //暂时不支持双点遥信，如果要支持再考虑打开
                {
                    
                    if(p->Status&0x80)
                        Status=1;
                    else
                        Status=0;
    
                    if (FramePos < 0)
                    {
                        TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LBIinfoaddr));
                        TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LBIinfoaddr));
                        if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                            TxMsg[InfoAddrLocation+2]=0;
                    }
                    else
                    {
                        TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//信息体地址
                        TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                        if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                            TxData[FramePos+2] = 0;
                    }
                    FramePos+=InfoAddrSize;
    
                    if((p->Status&BIACTIVEFLAG)==0)
                        TxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                    else
                        TxData[FramePos]=Status;//设置遥信状态字节
                        
                    if(p->Status&SUBSTITUTEDFLAG)
                        TxData[FramePos]|=P101_SB;//设置遥信状态字节
                    FramePos++;
    
                    AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
    
                    TxData[FramePos++] = LOBYTE(time.MSecond);
                    TxData[FramePos++] = HIBYTE(time.MSecond);
                    TxData[FramePos++] = time.Minute;
                    TxData[FramePos++] = time.Hour;
                    TxData[FramePos++] = time.Day;
                    TxData[FramePos++] = time.Month;
                    TxData[FramePos++] = time.Year;
    
                    SendNum++;//发送个数
                    p++;
                    j++;
                    if(FramePos>=Length)
                        break;
                }
                
            }
            
            //if(DBISOEnum>0)
            //   DBIDevIndex=i;
            
            if(SendNum>0)
            {
                //有单点遥信需要发送，发送完后继续检查是否有后续数据
                TxMsg[1] = SendNum;
                Len = FramePos+AsduHeadLength;//应用层报文总长度
                
                GroupTrn.SoeStartPtr +=  SendNum;
                
                EnCodeDLMsg(Len);
                return TRUE;
            }
            else
            {
                /*if(DBISOEnum>0)
                {
                    EnCodeDBISOE();
                    myTaskLock ();
                    if (DevList[i].Flag)
                    {
                        DevList[i].RealWin->BIETimRP=DevList[i].RealWin->BIETimRP+j;
                        clear_flag(DevList[i].DevID,BIETFLAG);
                    }
                    else
                    {
                        DevList[i].DbaseWin->BIETimRP=DevList[i].DbaseWin->BIETimRP+j;
                        clear_flag(DevList[i].DevID,BIETFLAG);
                    }
                    myTaskUnlock ();
                    
                    if(DBISOEnum)
                        return TRUE;    //表示还有未发送数据
                }
                else
                {
                	if(j>0) //j是已经处理过的个数
                    {
                    	myTaskLock ();
                        if (DevList[i].Flag)
                        {
                            DevList[i].RealWin->BIETimRP=DevList[i].RealWin->BIETimRP+j;
                            clear_flag(DevList[i].DevID,BIETFLAG);
                        }
                        else
                        {
                            DevList[i].DbaseWin->BIETimRP=DevList[i].DbaseWin->BIETimRP+j;
                            clear_flag(DevList[i].DevID,BIETFLAG);
                        }
                        myTaskUnlock ();
                    }
                }*/
            }
        }
    }
    
    return FALSE;
}

INT16U New104Sec::EnCodeAllYX(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT8U Status;
    INT8U *TxData;
    INT16U i,SBINum,devid,Len = 0;
    short ByteNum,Length,SendNum,FramePos,YxNum;    //SendNum是已经处理过的遥信个数，YxNum是发送的遥信个数
    INT16U yxSendno;

    TxMsg[0]=Sec104Pad.SBIType;
    TxMsg[1]=VSQ_SQ;
    //if(/*(DevList[GroupTrn.DevIndex].DevData.DBINum>0)&&*/(GroupTrn.GroupNo==8))
    if(GroupTrn.HaveSendDBI == TRUE)
    {
    	TxMsg[0]=M_DP_NA;
    }
    //广东要求的全双点遥信发送，特殊处理
    if(bSendAllDBI)
        TxMsg[0] = M_DP_NA_ALLDBI;
    
    TxMsg[2]=GroupTrn.COT;  //wjr  2009.8.26 传送原因直接等于组里面的传送原因 
    
    if(CotSize==2)              //传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    
    TxMsg[InfoAddrLocation] = LOBYTE(GroupTrn.InfoAddr);
    TxMsg[InfoAddrLocation+1] = HIBYTE(GroupTrn.InfoAddr);
    
    if(TxMsg[0]==M_DP_NA)
    {
    	TxMsg[InfoAddrLocation] = LOBYTE(BeginNo+LDBIinfoaddr);
        TxMsg[InfoAddrLocation+1] = HIBYTE(BeginNo+LDBIinfoaddr);
    }
    else
    {
        if(DevList[GroupTrn.DevIndex].DevData.DBINum > 0)   //wjr
        {
            TxMsg[InfoAddrLocation]=LOBYTE((GroupTrn.InfoAddr-DevList[GroupTrn.DevIndex].DevData.DBINum));
            TxMsg[InfoAddrLocation+1]=HIBYTE((GroupTrn.InfoAddr-DevList[GroupTrn.DevIndex].DevData.DBINum));
        }
    }     
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    TxData = TxMsg+AsduHeadLength;
    *pNum = 0;

    devid=DevList[GroupTrn.DevIndex].DevID;
    FramePos=0;
    SendNum=0;
    YxNum=0;
    Len = 0;
    Length=ASDULEN-AsduHeadLength-10;//250-7-8-2=233为应用层发送信息最大长度
    if(TxMsg[0] == M_PS_NA)//20-成组的bit
    {
        if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBIT_ReadSBI(devid,BeginNo,EndNo,DBData, &SBINum);
        else
            ByteNum=CLBIT_ReadSBI(devid,BeginNo,EndNo,DBData, &SBINum);
        if(ByteNum<1)
            return(0);
        if((ByteNum%2)==1)
            DBData[ByteNum]=0;
        for(i=0;i<(ByteNum+1)/2;i++)
        {
            TxData[FramePos++]=DBData[2*i];
            TxData[FramePos++]=DBData[2*i+1];
            TxData[FramePos++]=0;
            TxData[FramePos++]=0;
            TxData[FramePos++]=0;
            YxNum++;
            SendNum+=16;
            if((FramePos>=Length)||(SendNum>=127))
                break;
        }
    }
    else if(TxMsg[0]==M_SP_NA)//1-单点byte
    {
        if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        //单点遥信发送策略，在本组查询中，如果遇到双点遥信则停止发送，但停止发送必须至少有一个单点遥信要发送
        for(i=0;i<ByteNum;i++)
        {
            SendNum++;
            if((DBData[i] & BIDBI_STATUSE) == 0)    //非双点遥信，则按单点遥信发送
            {
                if((DBData[i]&BIACTIVEFLAG)==0)
                    TxData[FramePos]=((DBData[i]&0x80)>>7)|P101_IV;   //数据库D7为遥信状态,置无效位
                else
                    TxData[FramePos]=((DBData[i]&0x80)>>7);           //数据库D7为遥信状态
                    
                if(DBData[i]&SUBSTITUTEDFLAG)
                    TxData[FramePos]|=P101_SB;   
                
                    
                FramePos++;
                YxNum++;
                
                if(YxNum == 1)  //检测到第1个单点遥信，则修正信息体地址（因为有可能第1个是双点遥信）
                {
                    TxMsg[InfoAddrLocation] = LOBYTE(GroupTrn.InfoAddr+i);
                    TxMsg[InfoAddrLocation+1] = HIBYTE(GroupTrn.InfoAddr+i);
                }
            }
            else
            {
                if(YxNum)   //检测到双点遥信后，则停止后续发送（因为不连续）
                    break;
            }
            
            if((FramePos>=Length)||(SendNum>=127))
                break;
        }
    }
    else if(TxMsg[0]==M_DP_NA)//3-双点byte      wjr
    {
        //测试有双点遥信，但是没配到发送表的情况。
        //测试有双点遥信，一帧发送不完的情况
        //测试有双点遥信，分组召唤情况
        //测试cos的情况
        
        TxMsg[1]= 0;    //非顺序元素

    	EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1; //如果遥信个数过大，超过DBData缓冲区，那么需要循环读
    	if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        FramePos = InfoAddrLocation;       //调整到信息体地址位置 ll 2017-7-19
        for(i=0; i<(ByteNum); i++)
        {
            SendNum++;  //这里记录已经读取过的遥信个数
            
            if(DBData[i] & BIDBI_STATUSE)
            {
                yxSendno = GroupTrn.InfoAddr+i-LBIinfoaddr;   //计算本次的遥信发送序号，从0开始计算（GroupTrn.InfoAddr目前是记录的从LBIinfoaddr开始的序号，用于控制传送遥信的当前位置） ll 21-03-28
                TxMsg[FramePos++] = LOBYTE((yxSendno+LDBIinfoaddr));//信息体地址
                TxMsg[FramePos++] = HIBYTE((yxSendno+LDBIinfoaddr));
                TxMsg[FramePos++] = 0;
                if((DBData[i]&BIACTIVEFLAG)==0)
                    TxMsg[FramePos] = ((DBData[i]&0x60)>>5)|P101_IV;
                else
                    TxMsg[FramePos] = ((DBData[i]&0x60)>>5);
                    
                if(DBData[i]&SUBSTITUTEDFLAG)
                    TxMsg[FramePos] |= P101_SB;
                
                    
                FramePos++;
                YxNum++;
                if((FramePos>=Length) ||(YxNum>=127))
                    break;
            }
        }
        
        FramePos -= AsduHeadLength;     //为兼容单点遥信程序，调整FramePos的大小为去掉AsduHeadLength大小  ll 2017-7-19
    }
    else if(TxMsg[0] == M_DP_NA_ALLDBI)//广东要求的全双点遥信发送，特殊处理
    {
        TxMsg[0]=M_DP_NA;
        
        if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        for(i=0; i<ByteNum; i++)
        {
            SendNum++;
            
            if(DBData[i]&0x80)
                Status=(BIDBI_YXH>>5);
            else
                Status=(BIDBI_YXF>>5);
            
            if((DBData[i]&BIACTIVEFLAG)==0)
                TxData[FramePos] = Status|P101_IV;   //数据库D7为遥信状态,置无效位
            else
                TxData[FramePos] = Status;           //数据库D7为遥信状态
                
            if(DBData[i]&SUBSTITUTEDFLAG)
                TxData[FramePos] |= P101_SB;   
              
            FramePos++;
            YxNum++;
            
            if((FramePos>=Length)||(SendNum>=127))
                break;
        }
    }
        
    if(SendNum > 0) //
    {
        *pNum = SendNum;
        
        if(YxNum > 0)
        {
            TxMsg[1] |= YxNum;
            Len=FramePos+AsduHeadLength;
            EnCodeDLMsg(Len);
        }
        else
        {
            Len = SendNum;    //返回真值下步继续处理
            pDLink->NotifyToAppSchedule();  //由于没发任何数据，调度进行下一次发送 ll 2017-7-19
        }
    }

    return(Len);
}


INT16U New104Sec::EnCodeAllYC(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT8U *TxData;
    INT16U i,No,devid,Len = 0;
    short Value,AINum,Length,SendNum,FramePos;
    struct RealAI_t *pAIValue;
    float temp;
    INT8U *p, *pdd;
    INT32U dd;
    BOOL Stop = FALSE;

    TxData = TxMsg+AsduHeadLength;

    TxMsg[0]=Sec104Pad.AIType;
    TxMsg[1]=VSQ_SQ;
    TxMsg[2]=GroupTrn.COT;           //wjr  2009.8.26 传送原因直接等于组里面的传送原因 
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation]   = LOBYTE(GroupTrn.InfoAddr);
    TxMsg[InfoAddrLocation+1] = HIBYTE(GroupTrn.InfoAddr);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    //取出数据
    No=BeginNo;
    devid=DevList[GroupTrn.DevIndex].DevID;
    while (No<=EndNo)
    {
        if(DevList[GroupTrn.DevIndex].Flag==1)
            AINum=CRFSend_ReadAI(devid,No,No+511,(struct RealAI_t *)DBData);
        else
            AINum=CLF_ReadAI(devid,No,No+511,(struct RealAI_t *)DBData);

        if (AINum<0)
            break;
        pAIValue=(struct RealAI_t *)DBData;
        for(i=0;i<AINum;i++)
        {
            Value=pAIValue->Value;
            switch(TxMsg[0])
            {
                case M_ME_NA:     //9测量值，规一化值
                case M_ME_ND:
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag=pAIValue->Flag;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Value=Value;
                    //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxValTrue[No];
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue=Value;
                    break;
                case M_ME_NB:     //11测量值，标度值
                case M_ME_NC:    //13测量值，短浮点数
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag=pAIValue->Flag;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue=Value;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Value=Value;
                    break;
            }
            No++;
            pAIValue++;
        }
    }//end of (while (No<=EndNo))

    //组帧发送
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
    FramePos=0;
    No=BeginNo;
    *pNum=0;
    SendNum = 0;
    while ((FramePos<Length)&&(!Stop))
    {
        switch(TxMsg[0])
        {
            case M_ME_NA:     //9测量值，规一化值
            case M_ME_NB:     //11测量值，标度值
                Value = (short)SL_ReadAI_S(DevList[GroupTrn.DevIndex].DevID, No, DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue);
                //Value=DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue;
                TxData[FramePos++]=LOBYTE(Value);
                TxData[FramePos++]=HIBYTE(Value);
                
                TxData[FramePos]=0;
                if((DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIACTIVEFLAG)==0)
                    TxData[FramePos] |= P101_IV;
                if (DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIOVERAGE)
                    TxData[FramePos] |= P101_OV;
                FramePos++;
                
                break;
            case M_ME_NC:    //13测量值，短浮点数
                temp = SL_ReadAI_S(DevList[GroupTrn.DevIndex].DevID, No, DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue);
                //temp = (float)DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue;
                p = (INT8U*)(&temp);
                pdd = (INT8U*)(&dd);
                *(pdd++) = *(p++);
                *(pdd++) = *(p++);
                *(pdd++) = *(p++);
                *(pdd++) = *(p++);
                TxData[FramePos++]=LLBYTE(dd);
                TxData[FramePos++]=LHBYTE(dd);
                TxData[FramePos++]=HLBYTE(dd);
                TxData[FramePos++]=HHBYTE(dd);
                
                TxData[FramePos]=0;
                if((DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIACTIVEFLAG)==0)
                    TxData[FramePos] |= P101_IV;
                if (DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIOVERAGE)
                    TxData[FramePos] |= P101_OV;
                FramePos++;

                break;
            case M_ME_ND:
                Value = (short)SL_ReadAI_S(DevList[GroupTrn.DevIndex].DevID, No, DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue);
                //Value=DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue;
                TxData[FramePos++]=LOBYTE(Value);
                TxData[FramePos++]=HIBYTE(Value);

                break;
        }
        No++;
        SendNum++;
        if(No>EndNo)
            Stop=TRUE;
        if (SendNum>=127)//每帧不超过127个数据单元
            Stop=TRUE;
    }

    if(SendNum > 0)
    {
        *pNum = SendNum;
        TxMsg[1] |= SendNum;

        Len=FramePos+AsduHeadLength;//应用层报文的总长度
        EnCodeDLMsg(Len);
    }
    return(Len);
}

INT16U New104Sec::EnCodeAllST(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    return 0;
}

INT16U New104Sec::EnCodeAllBCD(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    return 0;
}

INT16U New104Sec::EnCodeAllSta(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT8U Len ,*TxData;

    TxData = TxMsg+AsduHeadLength;

    TxMsg[0]=M_BO_NA;
    TxMsg[1]=1;
    TxMsg[2]=GroupTrn.COT;                       //传送原因直接等于组里面的传送原因 wjr 2009.8.26
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(RTUSTATUS);
    TxMsg[InfoAddrLocation+1] = HIBYTE(RTUSTATUS);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    TxData[0] = 0;
    TxData[1] = 0;
    TxData[2] = 0;
    TxData[3] = 0;
    TxData[4] = 0;
    Len = AsduHeadLength+5;
    if (Len)
    {
        EnCodeDLMsg(Len);
    }
    return(Len);
}

BOOL New104Sec::EnCodeNVA(void)  //编辑变化遥测数据;
{
    BOOL Stop=FALSE;
    int  FramePos,i,j,jj;
    int No,k;
    short value, AINum;
    INT16U devid,Length,Len;
    float temp;
    INT8U *p,*pTxData, *pdd;
    INT32U dd;
    struct RealAI_t *pAIValue;
    INT16U NvaVal;
    
    AINum=0;
    for (j=0;j<DevCount;j++)//
    {
        if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
        {
            return(TRUE);
        }

        Stop=FALSE;
        FramePos=0-InfoAddrSize;
        pTxData = TxMsg+AsduHeadLength;
        TxMsg[0]=Sec104Pad.AIType;
        TxMsg[1]=0;
        TxMsg[2]=SPONT;
        if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[NvaActDevNo].Addr>>(8*jj);

        for (No=0;No<=DevList[NvaActDevNo].DevData.AINum-1;No++)
            DevList[NvaActDevNo].DevData.AIData[No].WillSend=FALSE;
        
        No=0;
        devid=DevList[NvaActDevNo].DevID;
        while (No<=(int)(DevList[NvaActDevNo].DevData.AINum-1))//取遥测值，设置发送标志
        {
            if(DevList[NvaActDevNo].Flag==1)
                AINum=CRFSend_ReadAI(devid,No,No+511,(struct RealAI_t *)DBData);
            else
                AINum=CLF_ReadAI(devid,No,No+511,(struct RealAI_t *)DBData);
            
            if (AINum<=0)
                return FALSE;
            pAIValue=(struct RealAI_t*)DBData;
            for(i=0;i<AINum;i++)
            {
                value=pAIValue->Value;
                
                if(DevList[NvaActDevNo].DevData.AItype[No])
                {
                    NvaVal = abs((INT16U)DevList[NvaActDevNo].DevData.AIData[No].Value - (INT16U)value);
                }
                else
                {
                    NvaVal = abs(DevList[NvaActDevNo].DevData.AIData[No].Value-value);
                }

                if (NvaVal >=DevList[NvaActDevNo].DevData.AIMaxVal[No])//比较变化值与死区值大小
                {
                    DevList[NvaActDevNo].DevData.AIData[No].WillSend=TRUE;//设置发送标志
                }
                DevList[NvaActDevNo].DevData.AIData[No].Flag=pAIValue->Flag;
                DevList[NvaActDevNo].DevData.AIData[No].TempValue=value;
                No++;
                pAIValue++;
            }
        }
        if (AINum<=0)
            return FALSE;
        No=DevList[NvaActDevNo].DevData.NvaNo; //从上次发完的遥测序号开始发送
        k=0;
        Length=ASDULEN-AsduHeadLength-5-sizeof(INT16U);//可以发送数据的报文长度
        while ((FramePos<Length)&&(!Stop))//组发送数据帧
        {
            if (DevList[NvaActDevNo].DevData.AIData[No].WillSend)
            {
                if (FramePos < 0)
                {
                    TxMsg[InfoAddrLocation]   = LOBYTE((No+LAI));
                    TxMsg[InfoAddrLocation+1] = HIBYTE((No+LAI));
                    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                        TxMsg[InfoAddrLocation+2]=0;
                }
                else
                {
                    pTxData[FramePos]   = LOBYTE((No+LAI));
                    pTxData[FramePos+1] = HIBYTE((No+LAI));
                    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                        pTxData[FramePos+2] = 0;
                }
                FramePos+=InfoAddrSize;

                switch (TxMsg[0])
                {
                    case M_ME_NA:     //9测量值，规一化值
                        value = (short)SL_ReadAI_S(DevList[NvaActDevNo].DevID, No, DevList[NvaActDevNo].DevData.AIData[No].TempValue);
                        //value=DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        //value=(long)value*0x3FFF/(long)DevList[NvaActDevNo].DevData.AIMaxValTrue[No];
                        pTxData[FramePos++]=LOBYTE(value);
                        pTxData[FramePos++]=HIBYTE(value);
                                                
                        pTxData[FramePos]=0;
                        if((DevList[NvaActDevNo].DevData.AIData[No].Flag & AIACTIVEFLAG)==0)
                            pTxData[FramePos] |= P101_IV;
                        if (DevList[NvaActDevNo].DevData.AIData[No].Flag & AIOVERAGE)
                            pTxData[FramePos] |= P101_OV;
                        FramePos++;


                        DevList[NvaActDevNo].DevData.AIData[No].Value
                                      =DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        break;
                    case M_ME_NB:     //11测量值，标度化
                        value = (short)SL_ReadAI_S(DevList[NvaActDevNo].DevID, No, DevList[NvaActDevNo].DevData.AIData[No].TempValue);
                        //value=DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        pTxData[FramePos++]=LOBYTE(value);
                        pTxData[FramePos++]=HIBYTE(value);
                        
                        pTxData[FramePos]=0;
                        if((DevList[NvaActDevNo].DevData.AIData[No].Flag & AIACTIVEFLAG)==0)
                            pTxData[FramePos] |= P101_IV;
                        if (DevList[NvaActDevNo].DevData.AIData[No].Flag & AIOVERAGE)
                            pTxData[FramePos] |= P101_OV;
                        FramePos++;


                        DevList[NvaActDevNo].DevData.AIData[No].Value=DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        break;
                    case M_ME_NC:    //13测量值，短浮点数
                        temp = SL_ReadAI_S(DevList[NvaActDevNo].DevID, No, DevList[NvaActDevNo].DevData.AIData[No].TempValue);
                        //temp = (float)DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        p = (INT8U*)(&temp);
                        pdd = (INT8U*)(&dd);
                        *(pdd++) = *(p++);
                        *(pdd++) = *(p++);
                        *(pdd++) = *(p++);
                        *(pdd++) = *(p++);
                        pTxData[FramePos++]=LLBYTE(dd);
                        pTxData[FramePos++]=LHBYTE(dd);
                        pTxData[FramePos++]=HLBYTE(dd);
                        pTxData[FramePos++]=HHBYTE(dd);
                        pTxData[FramePos]=0;
                        if((DevList[NvaActDevNo].DevData.AIData[No].Flag & AIACTIVEFLAG)==0)
                            pTxData[FramePos] |= P101_IV;
                        if (DevList[NvaActDevNo].DevData.AIData[No].Flag & AIOVERAGE)
                            pTxData[FramePos] |= P101_OV;
                        FramePos++;

                        DevList[NvaActDevNo].DevData.AIData[No].Value
                                       =DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        break;
                    case M_ME_ND:        //21不带品质描述的测量值，归一化
                        value = (short)SL_ReadAI_S(DevList[NvaActDevNo].DevID, No, DevList[NvaActDevNo].DevData.AIData[No].TempValue);
                        //value=DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        //value=(long)value*0x3FFF/(long)DevList[NvaActDevNo].DevData.AIMaxValTrue[No];
                        pTxData[FramePos++]=LOBYTE(value);
                        pTxData[FramePos++]=HIBYTE(value);

                        DevList[NvaActDevNo].DevData.AIData[No].Value
                                       =DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                        break;
                }
                TxMsg[1]++;
                if (TxMsg[1]>=127)
                    Stop=TRUE;
            }
            No++;
            if (No>=DevList[NvaActDevNo].DevData.AINum)
                No=0;
            k++;
            if (k >= DevList[NvaActDevNo].DevData.AINum)
                Stop=TRUE;
        }
        DevList[NvaActDevNo].DevData.NvaNo=No;
        if (FramePos>0)
        {
            Len=(FramePos+AsduHeadLength);
            EnCodeDLMsg(Len);
        }
        NvaActDevNo++;
        if (NvaActDevNo>=DevCount)
            NvaActDevNo=0;
    }
    return(FALSE);
}


void New104Sec::EnCodeDBISOE(void)
{
    INT8U Len=0,Status0,Status1;
    short Num,FramePos;
    INT8U *TxData;
    INT16U SendNum,Length,i,j,jj;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;

    TxMsg[0]=M_DP_TB;   //带时标的单点信息
    TxMsg[1]=0;
    TxMsg[2]=SPONT;  //
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DBIDevIndex].Addr>>(8*jj);

    TxData = TxMsg+AsduHeadLength;
    Len = 0;
    FramePos=0-InfoAddrSize;
    p=(struct BIEWithTimeData_t *)DBIDBData;
    Length=ASDULEN-AsduHeadLength-12;//250-9-12=229为应用层发送信息最大长度
    
    
    Num=0;    
    SendNum=0;
    
    for(j=0; j<DBISOEnum; j++)
    {
      //状态转换：数据库中的遥信D7为状态，规约中D0为状态
        /*if((p->No%2)==0)
        {
            if(p->Status&0x80)
                Status1=2;
            else
                Status1=0;
            if((p+1)->Status&0x80)
               Status0=1;
            else
                Status0=0;     
        }
        else
        {
        	if(p->Status&0x80)
                Status0=1;
            else
                Status0=0;
            if((p+1)->Status&0x80)
               Status1=2;
            else
                Status1=0;                 
        }*/
        Status0 = 0;
        Status1 = (p->Status&0x60)>>5;
        
        if (FramePos < 0)
        {
            TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LDBIinfoaddr));
            TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LDBIinfoaddr));
            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxMsg[InfoAddrLocation+2]=0;
        }
        else
        {
            TxData[FramePos]   = LOBYTE((p->No + LDBIinfoaddr));//信息体地址
            TxData[FramePos+1] = HIBYTE((p->No + LDBIinfoaddr));
            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxData[FramePos+2] = 0;
        }
        FramePos+=InfoAddrSize;

        if((p->Status&BIACTIVEFLAG)==0)
            TxData[FramePos]=Status1|Status0|P101_IV;//设置遥信状态字节
        else
            TxData[FramePos]=Status1|Status0;//设置遥信状态字节
            
        if(p->Status&SUBSTITUTEDFLAG)
            TxData[FramePos]|=P101_SB;//设置遥信状态字节
        
        FramePos++;
        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

        TxData[FramePos++] = LOBYTE(time.MSecond);
        TxData[FramePos++] = HIBYTE(time.MSecond);
        TxData[FramePos++] = time.Minute;
        TxData[FramePos++] = time.Hour;
        TxData[FramePos++] = time.Day;
        TxData[FramePos++] = time.Month;
        TxData[FramePos++] = time.Year;

                
        SendNum++;//发送个数
        p++;
        if(FramePos>=Length)
            break;
     }
     
     if(SendNum>0)
     {
         TxMsg[1]=SendNum;
         Len=FramePos+AsduHeadLength;//应用层报文总长度
         EnCodeDLMsg(Len);
                
         
         Num = SendNum;
         
         if(Num<DBISOEnum)
         {
         	i=0;
         	while(Num<DBISOEnum)
             {
             	DBIDBData[i]=DBIDBData[Num];
                 //DBIDBData[2*i+1]=DBIDBData[2*Num+1];
             	i++;
             	Num++;
             }
         	DBISOEnum = DBISOEnum-SendNum;
         }
         else
             DBISOEnum=0;
             
         
     }
   
    
}
/*------------------------------------------------------------------/
函数名称：  EnCodeSOE()
函数功能：  编辑SOE
输入说明：  
输出说明：  FALSE 表示没数据了，清标志， TRUE 表示还有数据要发送
            SOE由事件驱动(OnUData)，通过置标志位调用EnCodeSOE
/------------------------------------------------------------------*/
BOOL New104Sec::EnCodeSOE(void) 
{
    INT8U Len=0,Status;
    short Num,FramePos;
    INT8U *TxData;
    INT16U SendNum,Length,WritePtr,i,j,jj;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    if(DBISOEnum>0)
    {
        EnCodeDBISOE();
        if(DBISOEnum)
            return TRUE; 
        else
            return FALSE;   
    } 
      
    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIETFLAG))
            continue;

        if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
            return(TRUE);

        TxMsg[0]=M_SP_TB;   //带时标的单点信息
        TxMsg[1]=0;
        TxMsg[2]=SPONT;  //
        if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);

        TxData = TxMsg+AsduHeadLength;
        Len = 0;
        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIET(DevList[i].DevID,DevList[i].RealWin->BIETimRP,18,(struct BIEWithTimeData_t *)DBData,&WritePtr);//
        else
            Num=L_ReadSBIET(DevList[i].DevID,DevList[i].DbaseWin->BIETimRP,18,(struct BIEWithTimeData_t *)DBData,&WritePtr);
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;
            p=(struct BIEWithTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-12;//250-9-12=229为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {
                
                if(DevList[i].DevData.DBINum>0)     //不再使用DevList[i].DevData.DBINum判断是否有双点遥信，这段代码不再使用 ll 2017-7-19
                {
                    if((p->No)<DevList[i].DevData.DBINum)
                    {
     
                            DBIDBData[DBISOEnum*2]=(struct BIEWithTimeData_t)(*p);
                            DBIDBData[DBISOEnum*2+1]=(struct BIEWithTimeData_t)(*(p+1));
                            DBISOEnum++;
                        	j=j+2;
                        	p=p+2;
                    }
                    else
                    {
                        if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
        
                        if (FramePos < 0)
                        {
                            TxMsg[InfoAddrLocation]   = LOBYTE((p->No - DevList[i].DevData.DBINum + LBIinfoaddr));
                            TxMsg[InfoAddrLocation+1] = HIBYTE((p->No - DevList[i].DevData.DBINum + LBIinfoaddr));
                            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                TxMsg[InfoAddrLocation+2]=0;
                        }
                        else
                        {
                            TxData[FramePos]   = LOBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));//信息体地址
                            TxData[FramePos+1] = HIBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));
                            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                TxData[FramePos+2] = 0;
                        }
                        FramePos+=InfoAddrSize;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            TxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                        else
                            TxData[FramePos]=Status;//设置遥信状态字节
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            TxData[FramePos]|=P101_SB;//设置遥信状态字节
                        
                        FramePos++;
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
        
                        TxData[FramePos++] = LOBYTE(time.MSecond);
                        TxData[FramePos++] = HIBYTE(time.MSecond);
                        TxData[FramePos++] = time.Minute;
                        TxData[FramePos++] = time.Hour;
                        TxData[FramePos++] = time.Day;
                        TxData[FramePos++] = time.Month;
                        TxData[FramePos++] = time.Year;
        
                        SendNum++;//发送个数
                        p++;
                        j++;
                        if(FramePos>=Length)
                            break;
                    }
                }
                else
                {
                    if(p->Status & BIDBI_STATUSE)    //SOE的状态位BIDBI_STATUSE（0x10）表示是双点遥信
                    {
                        DBIDBData[DBISOEnum]=(struct BIEWithTimeData_t)(*p);
                        DBISOEnum++;
                    	j++;
                    	p++;
                    }
                    else
                    {
                        
                        if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
        
                        if (FramePos < 0)
                        {
                            TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LBIinfoaddr));
                            TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LBIinfoaddr));
                            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                TxMsg[InfoAddrLocation+2]=0;
                        }
                        else
                        {
                            TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//信息体地址
                            TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                TxData[FramePos+2] = 0;
                        }
                        FramePos+=InfoAddrSize;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            TxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                        else
                            TxData[FramePos]=Status;//设置遥信状态字节
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            TxData[FramePos]|=P101_SB;//设置遥信状态字节
                        
                        FramePos++;        
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
        
                        TxData[FramePos++] = LOBYTE(time.MSecond);
                        TxData[FramePos++] = HIBYTE(time.MSecond);
                        TxData[FramePos++] = time.Minute;
                        TxData[FramePos++] = time.Hour;
                        TxData[FramePos++] = time.Day;
                        TxData[FramePos++] = time.Month;
                        TxData[FramePos++] = time.Year;
        
                        SendNum++;//发送个数
                        p++;
                        j++;
                        if(FramePos>=Length)
                            break;
                    }
                }
            }
            
            if(DBISOEnum>0)
                DBIDevIndex=i;
            
            if(SendNum>0)
            {
                //有单点遥信需要发送，发送完后继续检查是否有后续数据
                TxMsg[1] = SendNum;
                Len = FramePos+AsduHeadLength;//应用层报文总长度
                myTaskLock ();
                if (DevList[i].Flag)
                {
                    DevList[i].RealWin->BIETimRP=DevList[i].RealWin->BIETimRP+j;
                    clear_flag(DevList[i].DevID,BIETFLAG);
                }
                else
                {
                    DevList[i].DbaseWin->BIETimRP=DevList[i].DbaseWin->BIETimRP+j;
                    clear_flag(DevList[i].DevID,BIETFLAG);
                }
                myTaskUnlock ();

                EnCodeDLMsg(Len);
                return TRUE;
            }
            else
            {
                if(DBISOEnum>0)
                {
                    EnCodeDBISOE();
                    myTaskLock ();
                    if (DevList[i].Flag)
                    {
                        DevList[i].RealWin->BIETimRP=DevList[i].RealWin->BIETimRP+j;
                        clear_flag(DevList[i].DevID,BIETFLAG);
                    }
                    else
                    {
                        DevList[i].DbaseWin->BIETimRP=DevList[i].DbaseWin->BIETimRP+j;
                        clear_flag(DevList[i].DevID,BIETFLAG);
                    }
                    myTaskUnlock ();
                    
                    if(DBISOEnum)
                        return TRUE;    //表示还有未发送数据
                }
                else
                {
                	if(j>0) //j是已经处理过的个数
                    {
                    	myTaskLock ();
                        if (DevList[i].Flag)
                        {
                            DevList[i].RealWin->BIETimRP=DevList[i].RealWin->BIETimRP+j;
                            clear_flag(DevList[i].DevID,BIETFLAG);
                        }
                        else
                        {
                            DevList[i].DbaseWin->BIETimRP=DevList[i].DbaseWin->BIETimRP+j;
                            clear_flag(DevList[i].DevID,BIETFLAG);
                        }
                        myTaskUnlock ();
                    }
                }
            }
        }
    }
    return(FALSE);
}
/*------------------------------------------------------------------/
函数名称：  EnCodeSOE_ALLDBI()
函数功能：  编辑SOE
输入说明：  
输出说明：  FALSE 表示没数据了，清标志， TRUE 表示还有数据要发送
            SOE由事件驱动(OnUData)，通过置标志位调用EnCodeSOE
/------------------------------------------------------------------*/
BOOL New104Sec::EnCodeSOE_ALLDBI(void) 
{
    INT8U Len=0,Status;
    short Num,FramePos;
    INT8U *TxData;
    INT16U SendNum,Length,WritePtr,i,j,jj;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
     
    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIETFLAG))
            continue;

        if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
            return(TRUE);

        TxMsg[0]=M_DP_TB;   //带时标的单点信息
        TxMsg[1]=0;
        TxMsg[2]=SPONT;  //
        if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);

        TxData = TxMsg+AsduHeadLength;
        Len = 0;
        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIET(DevList[i].DevID,DevList[i].RealWin->BIETimRP,18,(struct BIEWithTimeData_t *)DBData,&WritePtr);//
        else
            Num=L_ReadSBIET(DevList[i].DevID,DevList[i].DbaseWin->BIETimRP,18,(struct BIEWithTimeData_t *)DBData,&WritePtr);
        
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;
            p=(struct BIEWithTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-12;//250-9-12=229为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {
                if(p->Status&0x80)
                    Status=(BIDBI_YXH>>5);
                else
                    Status=(BIDBI_YXF>>5);

                if (FramePos < 0)
                {
                    TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LBIinfoaddr));
                    TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LBIinfoaddr));
                    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                        TxMsg[InfoAddrLocation+2]=0;
                }
                else
                {
                    TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//信息体地址
                    TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                        TxData[FramePos+2] = 0;
                }
                FramePos+=InfoAddrSize;

                if((p->Status&BIACTIVEFLAG)==0)
                    TxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                else
                    TxData[FramePos]=Status;//设置遥信状态字节
                    
                if(p->Status&SUBSTITUTEDFLAG)
                    TxData[FramePos]|=P101_SB;//设置遥信状态字节
                
                FramePos++;        
                AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

                TxData[FramePos++] = LOBYTE(time.MSecond);
                TxData[FramePos++] = HIBYTE(time.MSecond);
                TxData[FramePos++] = time.Minute;
                TxData[FramePos++] = time.Hour;
                TxData[FramePos++] = time.Day;
                TxData[FramePos++] = time.Month;
                TxData[FramePos++] = time.Year;

                SendNum++;//发送个数
                p++;
                j++;
                if(FramePos>=Length)
                    break;
                
            }
            
            if(SendNum>0)
            {
                //有单点遥信需要发送，发送完后继续检查是否有后续数据
                TxMsg[1] = SendNum;
                Len = FramePos+AsduHeadLength;//应用层报文总长度
                myTaskLock ();
                if (DevList[i].Flag)
                {
                    DevList[i].RealWin->BIETimRP=DevList[i].RealWin->BIETimRP+j;
                    clear_flag(DevList[i].DevID,BIETFLAG);
                }
                else
                {
                    DevList[i].DbaseWin->BIETimRP=DevList[i].DbaseWin->BIETimRP+j;
                    clear_flag(DevList[i].DevID,BIETFLAG);
                }
                myTaskUnlock ();

                EnCodeDLMsg(Len);
                return TRUE;
            }
            
        }
    }
    return(FALSE);
}

void New104Sec::EnCodeDBIENT(void)
{
    INT8U Len=0,Status1,Status0;
    short Num,FramePos;
    INT8U *TxData;
    INT16U SendNum,Length,i,j,jj;
    struct BIEWithoutTimeData_t *p;

    
    TxMsg[0]=M_DP_NA;
    TxMsg[1]=0;
    TxMsg[2]=SPONT;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=(DevList[DBICOSDevIndex].Addr)>>(8*jj);

    TxData = TxMsg+AsduHeadLength;
    Len = 0;

    SendNum=0;
    FramePos=0-InfoAddrSize;
    p=(struct BIEWithoutTimeData_t *)DBICOSDBData;
    Length=ASDULEN-AsduHeadLength-3;//250-9-3=238为应用层发送信息最大长度

    
    for(j=0; j<DBICOSnum; j++)
    {
        
        Status0 = 0;
        Status1 = ((p->Status&0x60)>>5);
        
        if(FramePos < 0)
        {
            TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LDBIinfoaddr));
            TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LDBIinfoaddr));
            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxMsg[InfoAddrLocation+2]=0;
        }
        else
        {
            TxData[FramePos]   = LOBYTE((p->No + LDBIinfoaddr));//信息体地址
            TxData[FramePos+1] = HIBYTE((p->No + LDBIinfoaddr));
            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxData[FramePos+2] = 0;
        }
        
        FramePos+=InfoAddrSize;
        if((p->Status&BIACTIVEFLAG)==0)
            TxData[FramePos]=Status1|Status0|P101_IV;//设置遥信状态字节
        else
            TxData[FramePos]=Status1|Status0;//设置遥信状态字节
            
        if(p->Status&SUBSTITUTEDFLAG)
            TxData[FramePos]|=P101_SB;//设置遥信状态字节
        
        FramePos++;
        SendNum++;//发送个数
        p++;
        if(FramePos>=Length)
            break;

        
    }
    
    if(SendNum>0)
    {
        
        TxMsg[1]=SendNum;
        Len=FramePos+AsduHeadLength;//应用层报文总长度
        if(EnCodeDLMsg(Len))
        {
            Num=SendNum;
            if(Num<DBICOSnum)
            {
             	i=0;
             	while(Num<DBICOSnum)
                 {
                     DBICOSDBData[i]=DBICOSDBData[Num];
                     //DBICOSDBData[2*i]=DBICOSDBData[2*Num];
                     //DBICOSDBData[2*i+1]=DBICOSDBData[2*Num+1];
                 	Num++;
                 	i++;
                 }
             	DBICOSnum=DBICOSnum-SendNum;
            }
            else
                DBICOSnum=0;
        }
        
    }
      
}

/*------------------------------------------------------------------/
函数名称：  EnCodeBIENT()
函数功能：  组织cos发送
输入说明：  
输出说明：  TRUE  后续有数据   FALSE 无后续数据
备注：      
/------------------------------------------------------------------*/
BOOL New104Sec::EnCodeBIENT(void) //编辑COS
{
    INT8U Len=0,Status;
    short Num,FramePos;
    INT8U *TxData;
    INT16U SendNum,Length,WritePtr,i,j,jj;
    struct BIEWithoutTimeData_t *p;

    if(DBICOSnum>0)
    {
        EnCodeDBIENT();
        return(TRUE);    
    }   

    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIENTFLAG))
            continue;
        if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
            return(TRUE);
        TxMsg[0]=M_SP_NA;
        TxMsg[1]=0;
        TxMsg[2]=SPONT;
        if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);

        TxData = TxMsg+AsduHeadLength;
        Len = 0;

        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIENT(DevList[i].DevID,DevList[i].RealWin->BIENTimRP,78,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        else
            Num=L_ReadSBIENT(DevList[i].DevID,DevList[i].DbaseWin->BIENTimRP,78,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;
            p=(struct BIEWithoutTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-3;//250-9-3=238为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {
                if(DevList[i].DevData.DBINum>0)
                {
                    if(p->No<DevList[i].DevData.DBINum)
                    {
  
                            DBICOSDBData[2*DBICOSnum]=(struct BIEWithoutTimeData_t)*p;
                            DBICOSDBData[2*DBICOSnum+1]=(struct BIEWithoutTimeData_t)*(p+1);
                        	DBICOSnum++;
                        	j=j+2;
                        	p=p+2;

                    }
                    else
                    {
                        if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
        
                        if (FramePos < 0)
                        {
                            TxMsg[InfoAddrLocation]   = LOBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));
                            TxMsg[InfoAddrLocation+1] = HIBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));
                            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                TxMsg[InfoAddrLocation+2]=0;
                        }
                        else
                        {
                            TxData[FramePos]   = LOBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));//信息体地址
                            TxData[FramePos+1] = HIBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));
                            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                TxData[FramePos+2] = 0;
                        }
                        FramePos+=InfoAddrSize;
                        if((p->Status&BIACTIVEFLAG)==0)
                            TxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                        else
                            TxData[FramePos]=Status;//设置遥信状态字节
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            TxData[FramePos]|=P101_SB;//设置遥信状态字节
                        
                        FramePos++;
                        SendNum++;//发送个数
                        p++;
                        j++;
                        if(FramePos>=Length)
                            break;
                    }
                }
                else
                {
                    if(p->Status & BIDBI_STATUSE)   //检测到有双点遥信，放到缓冲区单独处理
                    {
                        DBICOSDBData[DBICOSnum] = (struct BIEWithoutTimeData_t)*p;
                        DBICOSnum++;
                        j++;
                        p++;
                    }
                    else
                    {
                         if(p->Status&0x80)
                             Status=1;
                         else
                             Status=0;
                        
                         if (FramePos < 0)
                         {
                             TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LBIinfoaddr));
                             TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LBIinfoaddr));
                             if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                 TxMsg[InfoAddrLocation+2]=0;
                         }
                         else
                         {
                             TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//信息体地址
                             TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                             if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                                 TxData[FramePos+2] = 0;
                         }
                         FramePos+=InfoAddrSize;
                         if((p->Status&BIACTIVEFLAG)==0)
                             TxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                         else
                             TxData[FramePos]=Status;//设置遥信状态字节
                             
                         if(p->Status&SUBSTITUTEDFLAG)
                             TxData[FramePos]|=P101_SB;//设置遥信状态字节
                         
                        
                         FramePos++;
                         SendNum++;//发送个数
                         p++;
                         j++;
                         if(FramePos>=Length)
                             break;
                    }
                }
            }
         
            if(DBICOSnum>0)
                DBICOSDevIndex=i;
         
            if(SendNum>0)
            {
                 TxMsg[1]=SendNum;
                 Len=FramePos+AsduHeadLength;//应用层报文总长度
                 if(EnCodeDLMsg(Len))
                 {
                     myTaskLock ();
                     if (DevList[i].Flag)
                         DevList[i].RealWin->BIENTimRP=DevList[i].RealWin->BIENTimRP+j;
                     else
                         DevList[i].DbaseWin->BIENTimRP=DevList[i].DbaseWin->BIENTimRP+j;
                     clear_flag(DevList[i].DevID,BIENTFLAG);
                     myTaskUnlock ();
                 }
                 
                 if(DBICOSnum>0)
                     return(TRUE); 
             
            }
            else
            {
                //没有单点遥信，检查是否有双点遥信发送
                if(DBICOSnum>0)
                {
                    EnCodeDBIENT();
                    
                    myTaskLock ();
                    if (DevList[i].Flag)
                    {
                        DevList[i].RealWin->BIENTimRP=DevList[i].RealWin->BIENTimRP+j;
                        clear_flag(DevList[i].DevID,BIENTFLAG);
                    }
                    else
                    {
                        DevList[i].DbaseWin->BIENTimRP=DevList[i].DbaseWin->BIENTimRP+j;
                        clear_flag(DevList[i].DevID,BIENTFLAG);
                     
                    }
                    myTaskUnlock ();
                    
                    if(DBICOSnum>0)
                        return(TRUE); 
                }
                else
                {
                    if(j>0)
                    {
                        myTaskLock ();
                        if (DevList[i].Flag)
                        {
                            DevList[i].RealWin->BIENTimRP=DevList[i].RealWin->BIENTimRP+j;
                            clear_flag(DevList[i].DevID,BIENTFLAG);
                        }
                        else
                        {
                            DevList[i].DbaseWin->BIENTimRP=DevList[i].DbaseWin->BIENTimRP+j;
                            clear_flag(DevList[i].DevID,BIENTFLAG);
                         
                        }
                        myTaskUnlock ();    
                    }    
                }
            }
          
        }
    }
    
    return(FALSE);
}

/*------------------------------------------------------------------/
函数名称：  EnCodeBIENT_ALLDBI()
函数功能：  组织cos发送（广东用全双点遥信）
输入说明：  
输出说明：  TRUE  后续有数据   FALSE 无后续数据
备注：      
/------------------------------------------------------------------*/
BOOL New104Sec::EnCodeBIENT_ALLDBI(void) //编辑COS
{
    INT8U Len=0,Status;
    short Num,FramePos;
    INT8U *TxData;
    INT16U SendNum,Length,WritePtr,i,j,jj;
    struct BIEWithoutTimeData_t *p;

   
    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIENTFLAG))
            continue;
        if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
            return(TRUE);
            
        TxMsg[0]=M_DP_NA;
        TxMsg[1]=0;
        TxMsg[2]=SPONT;
        if(CotSize==2)//传送原因为2字节时，高位固定为0。
            TxMsg[CotLocation+1]=0;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);

        TxData = TxMsg+AsduHeadLength;
        Len = 0;

        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIENT(DevList[i].DevID,DevList[i].RealWin->BIENTimRP,78,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        else
            Num=L_ReadSBIENT(DevList[i].DevID,DevList[i].DbaseWin->BIENTimRP,78,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;
            p=(struct BIEWithoutTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-3;//250-9-3=238为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {
                if(p->Status&0x80)
                    Status=(BIDBI_YXH>>5);
                else
                    Status=(BIDBI_YXF>>5);
                
                if (FramePos < 0)
                {
                    TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LBIinfoaddr));
                    TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LBIinfoaddr));
                    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                        TxMsg[InfoAddrLocation+2]=0;
                }
                else
                {
                    TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//信息体地址
                    TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                        TxData[FramePos+2] = 0;
                }
                FramePos+=InfoAddrSize;
                if((p->Status&BIACTIVEFLAG)==0)
                    TxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                else
                    TxData[FramePos]=Status;//设置遥信状态字节
                     
                if(p->Status&SUBSTITUTEDFLAG)
                    TxData[FramePos]|=P101_SB;//设置遥信状态字节
                 
                
                FramePos++;
                SendNum++;//发送个数
                p++;
                j++;
                if(FramePos>=Length)
                    break;
            }
             
            if(SendNum>0)
            {
                 TxMsg[1]=SendNum;
                 Len=FramePos+AsduHeadLength;//应用层报文总长度
                 if(EnCodeDLMsg(Len))
                 {
                     myTaskLock ();
                     if (DevList[i].Flag)
                         DevList[i].RealWin->BIENTimRP=DevList[i].RealWin->BIENTimRP+j;
                     else
                         DevList[i].DbaseWin->BIENTimRP=DevList[i].DbaseWin->BIENTimRP+j;
                     clear_flag(DevList[i].DevID,BIENTFLAG);
                     myTaskUnlock ();
                 }
                 
            }
            
        }
    }
    
    return(FALSE);
}

INT8U New104Sec::EnCodeCounter(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT8U Len,*TxData;
    INT16U i,j,jj,No,GetBeginNo,GetEndNo,SendNum,Length;
    short CountNum,FramePos;
    struct RealCounter_t *q;
    INT32U value,*p;

    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
        return(0);

    if ((Sec104Pad.control & CON_104GYKZ))     // xwm  2017-08-02
    {
        TxMsg[0] = M_IT_NB;
    }
    else
    {
        TxMsg[0] = M_IT_NA;
    }
    TxMsg[1]=0;
    if (GroupTrn.COT==REQ)
        TxMsg[2]=REQCOGCN+GroupTrn.GroupNo;  //召唤一组
    else
        TxMsg[2]=GroupTrn.COT;

    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(GroupTrn.InfoAddr);
    TxMsg[InfoAddrLocation+1] = HIBYTE(GroupTrn.InfoAddr);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    if ((GroupTrn.First)&&(GroupTrn.COT!=REQ))
    {
        GetBeginNo=0;
        GetEndNo=DevList[GroupTrn.DevIndex].DevData.CounterNum-1;
        while (GetBeginNo<=GetEndNo)
        {
            if(DevList[GroupTrn.DevIndex].Flag==1)
                CountNum=CRSend_ReadCount(DevList[GroupTrn.DevIndex].DevID,GetBeginNo,GetBeginNo+200,(struct RealCounterNFlag_t *)DBData);
            else
                CountNum=CL_ReadCount(DevList[GroupTrn.DevIndex].DevID,GetBeginNo,GetBeginNo+200,(struct RealCounterNFlag_t *)DBData);

            if(CountNum<=0)
                break;
            p=(INT32U*)DBData;
            for(j=0;j<CountNum;j++)
            {
                value=*p;
                q=&(DevList[GroupTrn.DevIndex].DevData.CounterData[GetBeginNo+j]);
                if (GroupTrn.Description&FREEZENORESET)//冻结不复位
                {
                    q->Value=value;    //被冻结值为累加值
                    q->Flag=0;
                    DevList[GroupTrn.DevIndex].DevData.LastCounterData[GetBeginNo+j]=value;
                }
                else if(GroupTrn.Description&FREEZERESET)
                {
                    q->Flag=0;        //被冻结值为增量值
                    q->Value=value-DevList[GroupTrn.DevIndex].DevData.LastCounterData[GetBeginNo+j];
                    DevList[GroupTrn.DevIndex].DevData.LastCounterData[GetBeginNo+j]=value;
                }
                else
                {
                    q->Value=value;
                    q->Flag=0;
                }
                p++;
            }
            GetBeginNo += CountNum;
        }
    }

    FramePos=0-InfoAddrSize;
    SendNum=0;
    Len=0;
    No = BeginNo;
    TxData = TxMsg+AsduHeadLength;
    Length=ASDULEN-AsduHeadLength-10-sizeof(INT16U);//250-7-10-2=231为应用层发送信息最大长度
    for(i=0;i<=EndNo-BeginNo;i++)
    {
        /*if ((Sec104Pad.control & CON_104GYKZ))    // xwm  2017-08-02
        {
            value = SL_ReadCount(DevList[GroupTrn.DevIndex].DevID,No);
        }
        else*/
        {
            value=DevList[GroupTrn.DevIndex].DevData.CounterData[No].Value;
        }

        //写信息体地址
        if (FramePos < 0)
        {
            TxMsg[InfoAddrLocation]   = LOBYTE((No+LBCR));
            TxMsg[InfoAddrLocation+1] = HIBYTE((No+LBCR));
            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxMsg[InfoAddrLocation+2]=0;
        }
        else
        {
            TxData[FramePos]   = LOBYTE(No+LBCR);//信息体地址
            TxData[FramePos+1] = HIBYTE(No+LBCR);//信息体地址
            if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
                TxData[FramePos+2] = 0;
        }
        FramePos+=InfoAddrSize;

        //写电度值
        TxData[FramePos++]=LOBYTE(LOWORD(value));
        TxData[FramePos++]=HIBYTE(LOWORD(value));
        TxData[FramePos++]=LOBYTE(HIWORD(value));
        TxData[FramePos++]=HIBYTE(HIWORD(value));
        
        if ((Sec104Pad.control & CON_104GYKZ))    // xwm  2017-08-02
        {
            TxData[FramePos++] = 0;//
        }
        else
        {
            TxData[FramePos++] = i;//顺序号
        }


        No++;
        SendNum++;
        if(FramePos>=Length)
            break;
    }

    if(SendNum>0)
    {
        Len=FramePos+AsduHeadLength;
        TxMsg[1] = SendNum;
        *pNum = SendNum;

        EnCodeDLMsg(Len);
    }
    return(Len);
}


BOOL New104Sec::EnCodeInitEnd(void)
{
    int jj;
    if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
        return(FALSE);
    TxMsg[0]=M_EI_NA;
    TxMsg[1]=1;
    TxMsg[1] &= ~VSQ_SQ;

    TxMsg[CotLocation]=INIT_101;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);

    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    
    TxMsg[InfoAddrLocation+3]=0;

    if(EnCodeDLMsg(AsduHeadLength+1))    
        return(TRUE);
    else 
        return(FALSE);
}

/*------------------------------------------------------------------/
函数名称：  ProcFT_EncodeReadDir()
函数功能：  对目录的传输组织发送帧
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_EncodeReadDir(void)
{
    INT8U len;
    BOOL rc;
    INT8U FramePos;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    len = 0;
    
    //根据目录ID或目录名称，组织目录内文件的传输.从文件数量开始填写
    
    rc = FT_ReadDirectory(&FtInfo, &len);
       
    //len = 0 回否定回答，rc=true表示有无后续
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = REQ; //COT
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //信息体地址
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
            
    TxMsg[FramePos++] = 2;    //附加数据包类型
    TxMsg[FramePos++] = FR_RD_DIR_CON;
    
    if(len)    
        TxMsg[FramePos++] = 0;    //读取目录成功
    else
        TxMsg[FramePos++] = 1;    //读取目录失败
        
    TxMsg[FramePos++] = LLBYTE(FtInfo.dirid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.dirid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.dirid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.dirid);
    
    if(rc)
        TxMsg[FramePos++] = 1;    //有后续
    else
    {
        TxMsg[FramePos++] = 0;    //无后续
    }
    
    if(len)
    {  
        memcpy(&TxMsg[FramePos], FtInfo.DataBuf, len);
        FramePos += len;
    }
    else
    {
        TxMsg[FramePos++] = 0;    //文件数量=0
    }
    
    
    EnCodeDLMsg(FramePos);    
        
    if(rc==0)
        ScheduleFlag &= ~SCHEDULE_FT_DIR;
    
}
/*------------------------------------------------------------------/
函数名称：  ProcFT_ReadDir()
函数功能：  处理文件传输读目录命令。
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_ReadDir(void)
{
    //INT32U DirID;
    INT8U namelen, CallFlag, i;
    INT8U *pRx;
    struct Iec101ClockTime_t StartTime, EndTime;
    
    //DirID = MAKEDWORD(MAKEWORD(pRxData[2],pRxData[3]),MAKEWORD(pRxData[4],pRxData[5]));
    namelen = pRxData[6];
    
    memset(FtInfo.tempname, 0, 40); //记录目录名
    if(namelen>=40)
        namelen = 39;
    memcpy(FtInfo.tempname,&pRxData[7],namelen);    //FtInfo.tempname中临时存放，用于打印信息
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    
    FtInfo.dirid = FT_GetDirID(&FtInfo);    //根据目录名返回目录ID DirID不再使用 liuwei20170307
    
    pRx = &pRxData[7]+namelen;  //把召唤标志位置的指针赋值给pRx    
    CallFlag = pRx[0];  //召唤标志
    
    FtInfo.callflag = CallFlag;  
    
    logSysMsgNoTime("召唤目录%s, namelen=%d 召唤标志=%d, DirID=%d",(INT32U)FtInfo.tempname, namelen, CallFlag, FtInfo.dirid);
      
    if(CallFlag)
    {
        pRx++;  //调整到查询起始时间的位置
        StartTime.MSecond = MAKEWORD(pRx[0],pRx[1]);
        StartTime.Minute  = (pRx[2] & 0x3f);
        StartTime.Hour    = (pRx[3] & 0x1f);
        StartTime.Day     = (pRx[4] & 0x1f);
        StartTime.Month   = (pRx[5] & 0xf);
        StartTime.Year    = (pRx[6] & 0x7f);
        pRx += 7;   //调整到查询结束时间的位置
        EndTime.MSecond = MAKEWORD(pRx[0],pRx[1]);
        EndTime.Minute  = (pRx[2] & 0x3f);
        EndTime.Hour    = (pRx[3] & 0x1f);
        EndTime.Day     = (pRx[4] & 0x1f);
        EndTime.Month   = (pRx[5] & 0xf);
        EndTime.Year    = (pRx[6] & 0x7f);
       
        ConvToAbsTime(&StartTime,&FtInfo.startime, IEC101CLOCKTIME);
        ConvToAbsTime(&EndTime,  &FtInfo.endtime, IEC101CLOCKTIME);
        
        logSysMsgNoTime("%d %d:%d:%d目录查询开始",StartTime.Day,StartTime.Hour,StartTime.Minute,StartTime.MSecond/1000);
        logSysMsgNoTime("%d %d:%d:%d目录查询结束",EndTime.Day,EndTime.Hour,EndTime.Minute,EndTime.MSecond/1000);
    }
    else
    {
        FtInfo.startime.Minute = 0;
        FtInfo.endtime.Minute = 0;
    }
    
    ScheduleFlag |= SCHEDULE_FT_DIR;
   
}

/*------------------------------------------------------------------/
函数名称：  ProcFT_ReadFileAct()
函数功能：  激活要读的文件
输入说明：  
输出说明：  
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_ReadFileAct(void)
{
    //char name[40];
    INT16U namelen, i;
    INT32U len;
    INT8U FramePos, successPos;
    
    namelen = pRxData[2];
    memset(FtInfo.tempname, 0, 40);
    if(namelen >=40)
        namelen = 39;
    memcpy(FtInfo.tempname, &pRxData[3], namelen);
    
    //根据文件名称，组织文件的传输
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    

    len = FT_ReadFileAct(&FtInfo, FtInfo.tempname);
    
    //组织返回数据
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;   
    
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = ACTCON;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //信息体地址
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;    //附加数据包类型
    TxMsg[FramePos++] = FR_RD_FILE_ACTCON; 
    successPos = FramePos;  //记录成功失败位置 
    FramePos++;
    
    //文件名
    TxMsg[FramePos++] = namelen;
    memcpy(&TxMsg[FramePos], FtInfo.tempname, namelen);
    FramePos += namelen;
    //4字节文件标示文件ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
    
    if(len)
    {
        TxMsg[successPos] = 0;    //成功
        TxMsg[FramePos++] = LLBYTE(len); 
        TxMsg[FramePos++] = LHBYTE(len);
        TxMsg[FramePos++] = HLBYTE(len);
        TxMsg[FramePos++] = HHBYTE(len);  
        
        ScheduleFlag |= SCHEDULE_FT_DATA; 
    }
    else
    {
        TxMsg[successPos] = 1;  
        TxMsg[FramePos++] = 0;
        TxMsg[FramePos++] = 0;
        TxMsg[FramePos++] = 0;
        TxMsg[FramePos++] = 0;  
    }
    
    EnCodeDLMsg(FramePos);
   
}
/*------------------------------------------------------------------/
函数名称：  ProcFT_EncodeFileData()
函数功能：  传输要传输的文件内容
输入说明：  
输出说明：  
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_EncodeFileData(void)
{
    BOOL rc;
    INT8U len;
    INT8U FramePos;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
       
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = REQ;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //信息体地址
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;    //附加数据包类型
    TxMsg[FramePos++] = FR_RD_FILE_DATA;
    
    //4字节文件标示文件ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
    
    //根据FileFlag文件标示，查找不同文件 
    rc = FT_ReadData(&FtInfo, &TxMsg[FramePos], &len);
            
    EnCodeDLMsg(FramePos+len);     //6表示附加数据包、操作标识、文件ID 
    
    if(rc == 0)
    {
        //没有后续数据了
        ScheduleFlag &= ~SCHEDULE_FT_DATA;
    }
    
    
}

void New104Sec::ProcFileInit(void)
{
    FT_Init(&FtInfo);
    StdFT_Init(&StdFtInfo);
    FtInfo.InfoAddrSize = InfoAddrSize;
    FtInfo.DevID = DevList[0].DevID;
    
    FtInfo.dbisoenum = DevList[0].DevData.DBINum;
    FtInfo.LBIinfoaddr = LBIinfoaddr;
    FtInfo.LDBIinfoaddr = LDBIinfoaddr;
    FtInfo.mastercallflag = 0;   //AJ++170627
        
    //FtInfo.HisFileTxt = HisFileTxt;
    /*if(Sec104Pad.SoeDirID != 0)
    {
        FtInfo.SoeDirID = Sec104Pad.SoeDirID;   //SOE  目录ID  
        FtInfo.YkDirID  = Sec104Pad.YkDirID;    //遥控   默认值 2
        FtInfo.ExvDirID = Sec104Pad.ExvDirID;   //极值   默认值 3
        FtInfo.FixDirID = Sec104Pad.FixDirID;   //定点   默认值 4
        FtInfo.UlogDirID = Sec104Pad.UlogDirID;  //日志   默认值 7
        FtInfo.LbDirID  = Sec104Pad.LbDirID;    //录波   默认值 8 
    }
    else
    {
        FtInfo.SoeDirID = 1;     
        FtInfo.YkDirID  = 2;    
        FtInfo.ExvDirID = 3;   
        FtInfo.FixDirID = 4;   
        FtInfo.UlogDirID = 7;  
        FtInfo.LbDirID  = 8;    
        
    }
    
    logSysMsgNoTime("soe=%d,yk=%d,exv=%d,fix=%d", FtInfo.SoeDirID, FtInfo.YkDirID, FtInfo.ExvDirID, FtInfo.FixDirID);
    */

    // //////////
    if(whTransLogID == 0xffff)
        whTransLogID = DevList[0].DevID;
    //logSysMsgNoTime("104DevID=%d,ulog=%d,lb=%d", DevList[0].DevID, FtInfo.UlogDirID, FtInfo.LbDirID, 0);

}

/*------------------------------------------------------------------/
函数名称：  ProcFT_WriteFileDataConf()
函数功能：  写文件传输结束，回确认帧。
输入说明：    
输出说明：  
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_WriteFileDataConf(void)
{
    INT8U FramePos;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    //组织返回数据
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = REQ;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //信息体地址
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;    //附加数据包类型
    TxMsg[FramePos++] = FR_WR_FILE_DATACON;
    
    //4字节文件标示文件ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
     
    TxMsg[FramePos++] = LLBYTE(FtInfo.offset); 
    TxMsg[FramePos++] = LHBYTE(FtInfo.offset);
    TxMsg[FramePos++] = HLBYTE(FtInfo.offset);
    TxMsg[FramePos++] = HHBYTE(FtInfo.offset); 
    
    TxMsg[FramePos++] = FtInfo.errinfo;  //结果描述符
    
    EnCodeDLMsg(FramePos);
        
    //logSysMsgNoTime("写文件确认帧%s, err=%d, offset=%d", (INT32U)FtInfo.name, FtInfo.errinfo, FtInfo.offset,0);
    
}
/*------------------------------------------------------------------/
函数名称：  ProcFT_WriteFileData()
函数功能：  传输要写的文件内容
输入说明：  
输出说明：  
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_WriteFileData(void)
{
    INT32U offset, fileid;
    INT8U  IsNoFinish;
    INT8U  segmentlen, rc;
    
    fileid = MAKEDWORD(MAKEWORD(pRxData[2],pRxData[3]),MAKEWORD(pRxData[4],pRxData[5]));
    offset = MAKEDWORD(MAKEWORD(pRxData[6],pRxData[7]),MAKEWORD(pRxData[8],pRxData[9]));
    IsNoFinish = pRxData[10]; //0：无后续  1：有后续

    FtInfo.errinfo = 0;
    if(fileid != FtInfo.fileid)
    {
        //文件ID不一致
        FtInfo.errinfo = 4;  
    }
    if(offset != FtInfo.offset)
    {
        //文件长度异常
        FtInfo.errinfo = 3;
    }
    
    rc = TRUE;
    if(FtInfo.errinfo == 0)
    {  
        if(IsNoFinish == FALSE)
        {
            ProcFT_WriteFileDataConf();     //提前回确认帧，需要测试主站是否能马上进行通信，程序应该有写flash的时间
        } 
        //if(INFOADDR2BYTE)
            //segmentlen = (FrameLen+2) - (AsduHeadLength-1) - 12;
        //else
            segmentlen = (FrameLen+2) - AsduHeadLength - 12;   //ASDUlen= (FrameLen+2) AsduHeadLength=9  12=除数据块的其他字节
        
        if(FtInfo.IsWriteProgramFile)
        {
            if(FtInfo.IsUpdate)
            {
                rc = FT_WriteFileData(&FtInfo, &pRxData[11], segmentlen, pRxData[11+segmentlen], IsNoFinish);
            }
            else
            {
                FtInfo.errinfo = 1; //未知错误
                rc = TRUE;
                logSysMsgNoTime("软件升级异常(被撤销)",0,0,0,0);
            }
        }
        else
        {
            //下载非程序升级文件
            rc = FT_WriteFileData(&FtInfo, &pRxData[11], segmentlen, pRxData[11+segmentlen], IsNoFinish);
            
        }

        if(rc == FALSE)
            return;
    }

    //在没有结束，但是有异常的时候也要回确认帧
    if((rc == TRUE) && (IsNoFinish))
        ProcFT_WriteFileDataConf(); //升级文件的确认帧还是放在最后比较安全，否则现场给复位了怎么办
    
    FT_ParaReset(&FtInfo);  //文件传输结束，清参数
    
}
/*------------------------------------------------------------------/
函数名称：  ProcFT_WriteFileAct()
函数功能：  写文件激活。记录文件名，初始化相关参数等
输入说明：    
输出说明：  
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_WriteFileAct(void)
{
    INT16U namelen, i;
    INT32U filesize, fileid;
    INT8U *pdata;
    INT8U result;
    INT8U FramePos;
    
    namelen = pRxData[2];
    memset(FtInfo.tempname, 0, 40);
    if(namelen>=40)
        namelen = 39;
    memcpy(FtInfo.tempname, &pRxData[3], namelen);
    
    pdata = &pRxData[3+namelen];
    fileid   = MAKEDWORD(MAKEWORD(pdata[0],pdata[1]),MAKEWORD(pdata[2],pdata[3]));
    filesize = MAKEDWORD(MAKEWORD(pdata[4],pdata[5]),MAKEWORD(pdata[6],pdata[7]));
    
    
    //根据文件名称，组织文件的传输
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    
    FtInfo.namelen  = namelen;
    FtInfo.fileid   = fileid;
    FtInfo.FileSize = filesize;
    strcpy(FtInfo.name, FtInfo.tempname);
        
    result = FT_WriteFileAct(&FtInfo);
    
    //组织返回数据
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;   
    
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    if(result == 0)
        TxMsg[2] = ACTCON;
    else
        TxMsg[2] = 0x40|ACTCON;
    TxMsg[3] = 0;
    
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //信息体地址
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;     //附加数据包类型
    TxMsg[FramePos++] = FR_WR_FILE_ACTCON;
    TxMsg[FramePos++] = result;
    
    //文件名
    TxMsg[FramePos++] = namelen;
    memcpy(&TxMsg[FramePos], FtInfo.name, namelen);
    FramePos += namelen;
    
    //4字节文件标示文件ID
    TxMsg[FramePos++] = LLBYTE(fileid);
    TxMsg[FramePos++] = LHBYTE(fileid);
    TxMsg[FramePos++] = HLBYTE(fileid);
    TxMsg[FramePos++] = HHBYTE(fileid);
     
    TxMsg[FramePos++] = LLBYTE(filesize); 
    TxMsg[FramePos++] = LHBYTE(filesize);
    TxMsg[FramePos++] = HLBYTE(filesize);
    TxMsg[FramePos++] = HHBYTE(filesize);  
    
    EnCodeDLMsg(FramePos);

}
/*------------------------------------------------------------------/
函数名称：  ProcFT_ProgramUpdate()
函数功能：  软件升级命令处理。
输入说明：    
输出说明：  
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_ProgramUpdate(void)
{
    if((RxCot&COT_REASON) == ACT)
    {
        if(pRxData[0] & 0x80) //CTYPE
        {   
            //升级启动   
            FtInfo.IsUpdate = TRUE;
        }
        else
        {
            //升级结束
            FT_ParaReset(&FtInfo);
            //FtInfo.IsUpdate = FALSE; 
            StartProgramUpdate();
        }
        
    }
    else
    {
        //升级撤销
        FT_ParaReset(&FtInfo);
        ClearProgramUpdate();
    }
    
    
    //返回确认帧
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;
    }
    
    if((RxCot&COT_REASON) == DEACT) //取消升级
    {
        RxMsg[CotLocation] = DEACTCON;  
    }
    else
    {
        RxMsg[CotLocation] = ACTCON;
    }
    
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    
    EnCodeDLMsg(FrameLen+2);
     
}

void New104Sec::ProcFileTran(void)
{
    
    if(pRxData[0] != 2) //附加数据包类型 2=文件传输
    {   
        return;   
    }
    
    switch(pRxData[1])  //文件操作标示
    {
        case FR_RD_DIR: //读目录
            ProcFT_ReadDir();
            break;
        
        case FR_RD_FILE_ACT:    //读文件激活
            ProcFT_ReadFileAct();
            
            break;
        
        case FR_RD_FILE_DATACON:   //读文件数据确认
            
            break;
        
        case FR_WR_FILE_ACT:    //写文件激活
            ProcFT_WriteFileAct();
            break;
        
        case FR_WR_FILE_DATA:   //写文件数据
            ProcFT_WriteFileData();
            break;
    }
    
    
}

/*------------------------------------------------------------------/
函数名称：  ProcFileSyn()
函数功能：  处理文件同步命令
输入说明：  
输出说明： 
备注：线损模块2018标准，文件同步；cl 20180314     
/------------------------------------------------------------------*/
void New104Sec::ProcFileSyn(void)
{
    INT8U Len;
    if(RxCot != ACT)
    {
        return;        
    }
    TxMsg[0]=F_FS_NA_N;              
    TxMsg[1]=1;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;
    TxMsg[AsduHeadLength]=0;

    Len=AsduHeadLength+1;
    EnCodeDLMsg(Len);
    //给101主站任务发送消息启动101的文件召唤
    
    //给101主站任务发送消息启动101的文件召唤
}

void New104Sec::ProcXsFileSynFinish(void)
{
    INT8U Len;
    TxMsg[0]=F_FS_NA_N;              
    TxMsg[1]=1;
    TxMsg[2]=ACTTERM;
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;
    TxMsg[AsduHeadLength]=0;

    Len=AsduHeadLength+1;
    EnCodeDLMsg(Len); 
    logSysMsgWithTime("文件同步终止",0,0,0,0); //debug使用CL 20180528    
}






 #ifdef STANDARDFILETRANS104
/*------------------------------------------------------------------/
函数名称：  StdProcFT_ReadDir()
函数功能：  处理文件传输读目录命令。
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_ReadDir(void)
{
    //根据信息体地址区分一下召唤录波文件或其他文件，录波文件的信息体地址是0x680A
    
    StdFtInfo.dirid    = RxInfoAddr;
    /*if(StdFtInfo.dirid == FT_DIRID_LB)
    {
        StdFileInfo.SectionNum = 2;          //根据录波的规定，录波文件分为2节，.cfg文件定义为第一节，.dat文件定义为第二节。
    }*/
    ScheduleFlag |= SCHEDULE_FT_DIR_STD;   
}
/*------------------------------------------------------------------/
函数名称：StdGetFileInfo()
函数功能：文件命令处理函数（F_SC_NA）
输入说明：pData:应用层请求报文
输出说明：          
/------------------------------------------------------------------*/
void New104Sec::StdGetFileInfo(INT8U* pData)
{
    INT8U   i, no, scq, afq,sname;
    StdFileInfo.RxID = pData[0];
    StdFileInfo.RxCot = pData[2];
    
    no = 2 + CotSize;
    StdFileInfo.RxPubAddr = 0;
    for (i=0; i<PubAddrSize; i++)
    {
        StdFileInfo.RxPubAddr += (pData[no+i]<<(8*i));
    }
    
    no += PubAddrSize;
    StdFileInfo.RxInfoAddr = 0;
    for (i=0; i<InfoAddrSize; i++)
    {
        StdFileInfo.RxInfoAddr += (pData[no+i]<<(8*i));
    }
    
    no += InfoAddrSize;
    StdFileInfo.FileName = 0;
    for (i=0; i<2; i++)
    {
        StdFileInfo.FileName += (pData[no+i]<<(8*i));
    }
    
    no += 2;
    sname = pData[no];
    
    no += 1;
    if(StdFileInfo.RxID == F_SC_NA)
    {
        scq = pData[no] & 0x0F;     /*SCQ*/
        
        /*vsq*/
        
        /*cot*/
        if (StdFileInfo.RxCot == SFREQ)   /*召唤目录*/
        {
    
            
        }
        else if (StdFileInfo.RxCot == SFILE)     /*其他*/
        {
            switch (scq)
            {
                
                    
                case 1:     /*选择文件*/
                    StdFileInfo.FileStep = SelectFile;
                    StdFileInfo.SectionName = 0;
                    break;
                    
                case 2:     /*请求文件*/
                    StdFileInfo.FileStep = CallFile;
                    break;
                    
                case 3:     /*停止激活文件*/
                    StdFileInfo.FileStep = FileOver;
                    break;
                    
                    
                case 4:     /*删除文件*/
                    StdFileInfo.FileStep = FileOver;
                    break;
                    
                case 5:     /*选择节*/
                    StdFileInfo.FileStep = CallSection;
                    StdFileInfo.SectionName = sname;
                    break;
                    
                case 6:     /*请求节*/
                    StdFileInfo.FileStep = CallSection;
                    StdFileInfo.SectionName = sname;
                    break;
                    
                case 7:     /*停止激活节*/
                    StdFileInfo.FileStep = FileOver;
                    break;
                    
                default:
                    StdFileInfo.FileStep = FileOver;
                    break;
            }
            
            
            
        }
        else    /*无效*/
        {
            
        }
    }
    
    if(StdFileInfo.RxID == F_AF_NA)
    {
        afq = pData[no] & 0x0F;     /*AFQ*/
        switch (afq)
        {
            case 1:     
                StdFileInfo.ackstatus = ACKFILEPOS;
                break;
            case 2:     
                StdFileInfo.ackstatus = ACKFILENEG;
                break;
            case 3:     
                StdFileInfo.ackstatus = ACKSECTPOS;
                break;    
            case 4:     
                StdFileInfo.ackstatus = ACKSECTNEG;
                break;
            default:
                StdFileInfo.ackstatus = NOTUSED;
                break;
            }
    }
}

/*------------------------------------------------------------------/
函数名称：StdProcFT_ReadFile()
函数功能：获得返回信息，将按照规约的应用层规定从类型标识组帧
输入说明：无
输出说明：pData：组帧的数据；Flag：TRUE，有后续帧；返回组帧的数据长度
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_ReadFile()
{
    
    
    switch(StdFileInfo.FileStep)
    {
        case SelectFile:
            //rc = SFileReady();        //在此要去生成文件，
            ScheduleFlag |= SCHEDULE_FT_FILE_READY_STD;
            break;
        
        case CallFile:
            //rc = SSectionReady();     
            ScheduleFlag |= SCHEDULE_FT_SECTION_READY_STD;
            break;
        
        case CallSection:
            ScheduleFlag |= SCHEDULE_FT_DATA_STD;
            
            break;
        
        
            
        case FileOver:
            
            break;
        
        default:

            break;
    }    
    
    
    
}
void New104Sec::StdProcFileTran(void)
{
    switch(StdFileInfo.RxCot)
    {
        case REQ:                         //读目录
            StdProcFT_ReadDir();
            break;
        case FILE_101:
            StdProcFT_ReadFile();
            break;
    }    
}

void New104Sec::StdProcFileAck(void)
{
    switch(StdFileInfo.ackstatus)
    {
        case ACKFILEPOS:
            StdFT_ParaReset(&StdFtInfo);
            break;
        case ACKFILENEG:
            StdFT_ParaReset(&StdFtInfo);
            break;
        case ACKSECTPOS:
            if(StdFileInfo.SectionNum)
            {
                ScheduleFlag |= SCHEDULE_FT_SECTION_READY_STD;
            }
            else
            {
                ScheduleFlag |= SCHEDULE_FT_LAST_SECTION_FILE_STD;
            }
            break;
        case ACKSECTNEG:
            StdFT_ParaReset(&StdFtInfo);
            break;
        default:
            break;
    }        
}

/*------------------------------------------------------------------/
函数名称：  StdProcFT_EncodeReadDir()
函数功能：  对目录的传输组织发送帧
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_EncodeReadDir(void)
{
    INT8U len;
    BOOL rc;
    INT8U FramePos;
    INT8U filenum = 0;
    INT8U logicDevId = DevList[0].DevID;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    len = 0;
    rc = StdFT_ReadDirectory(&StdFtInfo, &len, logicDevId);
    filenum = len / 13;    //13是TYPE IDENT 126 中每一文件相关的信息。
    TxMsg[0] = F_DR_NA;
    TxMsg[1] = 0x80 | filenum;   //VSQ
    TxMsg[2] = REQ; //COT
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //此处有可能有问题，关键看怎么理解规约，先按照给的示例报文编写，2016年12月30日
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    FramePos = AsduHeadLength;
    if(len)
    {  
        memcpy(&TxMsg[FramePos], StdFtInfo.DataBuf, len);
        FramePos += len;
    }
    else
    {
        memset(&TxMsg[FramePos],0,5);//TxMsg[FramePos++] = 0;    //文件数量=0
        FramePos+=5;
        TxMsg[FramePos++] = 0x20;
        memset(&TxMsg[FramePos],0,7);
        FramePos+=7;
    }
    
    
    EnCodeDLMsg(FramePos);    
        
    if(rc==0)
        ScheduleFlag &= ~SCHEDULE_FT_DIR_STD;
    
}

/*------------------------------------------------------------------/
函数名称：  StdProcFT_EncodeFileReady()
函数功能：  文件准备就绪
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_EncodeFileReady(void)
{
    
    BOOL rc;
    INT8U FramePos;
    //INT8U filenum = 0;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    
    rc = StdFT_GetFileReady(&StdFtInfo,StdFileInfo.FileName,&StdFileInfo.SectionNum,&StdFileInfo.FileLen);     //对于录波文件来讲，在此函数中生成必要的文件，即.cfg和.dat文件
    //filenum = len / 13;    //13是TYPE IDENT 126 中每一文件相关的信息。
    StdFileInfo.FileChs = 0;
    TxMsg[0] = F_FR_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //此处有可能有问题，关键看怎么理解规约，先按照给的示例报文编写，2016年12月30日
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    FramePos = AsduHeadLength;
    TxMsg[FramePos++] = LOBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = HIBYTE(StdFileInfo.FileName);
    //TxMsg[FramePos++] = StdFileInfo.SectionName;
    TxMsg[FramePos++] = LLBYTE(StdFileInfo.FileLen);
    TxMsg[FramePos++] = LHBYTE(StdFileInfo.FileLen);
    TxMsg[FramePos++] = HLBYTE(StdFileInfo.FileLen);
    if(rc)
    {
        TxMsg[FramePos++] = 0x00;        //positive confirm of select,request...
        StdFtInfo.FileReadyFlag = 1;
    }
    else
    {
        TxMsg[FramePos++] = 0x80;        //negative confirm of select,request...
        StdFT_ParaReset(&StdFtInfo);
    }
    
    
    
    EnCodeDLMsg(FramePos);    
        
    
    ScheduleFlag &= ~SCHEDULE_FT_FILE_READY_STD;
    
}

/*------------------------------------------------------------------/
函数名称：  StdProcFT_EncodeSectionReady()
函数功能：  节准备就绪
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_EncodeSectionReady(void)
{
    //INT8U len;
    BOOL rc;
    INT8U FramePos;
    //INT8U filenum = 0;
    //INT32U SectionLength;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    StdFileInfo.SectChs = 0;
    //len = 0;
    rc = StdFT_GetSectionReady(&StdFtInfo,&StdFileInfo.SectionName,&StdFileInfo.SectLen);     //
    //filenum = len / 13;    //13是TYPE IDENT 126 中每一文件相关的信息。
    TxMsg[0] = F_FR_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //此处有可能有问题，关键看怎么理解规约，先按照给的示例报文编写，2016年12月30日
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    FramePos = AsduHeadLength;
    TxMsg[FramePos++] = LOBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = HIBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = StdFileInfo.SectionName;
    
    //SectionLength = GetSectionLength();              //待编写，对于录波文件来讲，节1的长度是.cfg文件的长度；节2的长度是.dat文件的长度。
    
    TxMsg[FramePos++] = LLBYTE(StdFileInfo.SectLen);
    TxMsg[FramePos++] = LHBYTE(StdFileInfo.SectLen);
    TxMsg[FramePos++] = HLBYTE(StdFileInfo.SectLen);
    
    if(rc)
    {
        TxMsg[FramePos++] = 0x00;        //positive confirm of select,request...
        StdFtInfo.SectionReadyFlag = 1;
    }
    else
    {
        TxMsg[FramePos++] = 0x80;        //negative confirm of select,request...
        StdFT_ParaReset(&StdFtInfo);
    }
    
    
    
    EnCodeDLMsg(FramePos);    
        
    
    ScheduleFlag &= ~SCHEDULE_FT_SECTION_READY_STD;
    
}

/*------------------------------------------------------------------/
函数名称：  StdProcFT_EncodeSegment()
函数功能：  节准备就绪
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_EncodeSegment(void)
{
    INT8U len;
    BOOL flag=FALSE;
    INT8U FramePos;
    //INT8U filenum = 0;
    //INT32U SectionLength;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    len = 0;
    if(StdFtInfo.SectionReadyFlag)
    {
        StdFT_GetSegment(&StdFtInfo, &len, &flag,StdFileInfo.SectionName,&StdFileInfo.SectChs);     //读文件的段，读取的内容保存在StdFtInfo.DataBuf里,flag返回的是表征是不是最后段
    }
    //filenum = len / 13;    //13是TYPE IDENT 126 中每一文件相关的信息。
    TxMsg[0] = F_SG_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //此处有可能有问题，关键看怎么理解规约，先按照给的示例报文编写，2016年12月30日
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    FramePos = AsduHeadLength;
    TxMsg[FramePos++] = LOBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = HIBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = StdFileInfo.SectionName;
    
       
    TxMsg[FramePos++] = len;
    if(len)
    {  
        memcpy(&TxMsg[FramePos], StdFtInfo.DataBuf, len);
        FramePos += len;
    }
    else
    {
        ScheduleFlag &= ~SCHEDULE_FT_DATA_STD;//TxMsg[FramePos++] = 0;    //文件数量=0
        return;
    }
    
    //StdFileInfo.FileChs += StdFileInfo.SectChs;
    EnCodeDLMsg(FramePos);    
        
    if(flag)
    {
        ScheduleFlag &= ~SCHEDULE_FT_DATA_STD;
        StdFileInfo.SectionNum--;
        ScheduleFlag |= SCHEDULE_FT_LAST_SECTION_FILE_STD;       //置最后段标识
    }
    
}

/*------------------------------------------------------------------/
函数名称：  StdProcFT_EncodeSegment()
函数功能：  节准备就绪
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_EncodeLastSegSect(void)
{
    //INT8U len;
    //BOOL rc,flag=FALSE;
    INT8U FramePos;
    //INT8U filenum = 0;
    //INT32U SectionLength;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    //len = 0;
    //rc = StdFT_GetSection(&StdFtInfo, &len, &flag);     //读文件的段，读取的内容保存在StdFtInfo.DataBuf里,flag返回的是表征是不是最后段
    //filenum = len / 13;    //13是TYPE IDENT 126 中每一文件相关的信息。
    TxMsg[0] = F_LS_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //此处有可能有问题，关键看怎么理解规约，先按照给的示例报文编写，2016年12月30日
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    FramePos = AsduHeadLength;
    TxMsg[FramePos++] = LOBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = HIBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = StdFileInfo.SectionName;
    
    if(StdFileInfo.SectionNum)
    {
        TxMsg[FramePos++] = LASTSEG;
        TxMsg[FramePos++] = StdFileInfo.SectChs;  
         
        if(StdFileInfo.SectionNum == 1)
        {
            StdFileInfo.SectionNum = 0;
        }
        StdFileInfo.FileChs += StdFileInfo.SectChs;
        StdFileInfo.SectChs = 0;
    }
    else
    {
        TxMsg[FramePos++] = LASTSECT;
        TxMsg[FramePos++] = StdFileInfo.FileChs; 
    }
    
    EnCodeDLMsg(FramePos);    
        
    
    ScheduleFlag &= ~SCHEDULE_FT_LAST_SECTION_FILE_STD;
        
    
    
}
#endif
/*------------------------------------------------------------------/
函数名称：  RMTReadAllPara()
函数功能：  组织读全部参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
BOOL New104Sec::RMTReadAllPara(INT8U *pbuf, INT8U *plen, INT8U *psendnum)
{
    INT16U info, FramPos;
    INT8U len, i, temp;
    INT16U sendnum, fnum;
    
    FramPos = 0;
    sendnum = 0;
    switch(RMTHaveReadParaFlag)
    {
    case 1:
        //上装固有参数
        for(info=RMTP_ORG_L; info<=RMTP_ORG_H; info++)
        {
            sendnum++;
            pbuf[FramPos++] = LOBYTE(info);
            pbuf[FramPos++] = HIBYTE(info);
            pbuf[FramPos++] = 0;
            
            len = 0;
            GetTerminalPara(&pbuf[FramPos], &len, info, 1);
            FramPos += len;
            
        }
        
        *plen = FramPos;
        RMTHaveReadParaFlag = 2;

        break;   
    case 2:
        //上装终端运行参数
        for(info=RMTP_RUN1_L; info<=RMTP_RUN1_H; info++)
        {
            sendnum++;
            pbuf[FramPos++] = LOBYTE(info);
            pbuf[FramPos++] = HIBYTE(info);
            pbuf[FramPos++] = 0;
            
            len = 0;
            GetTerminalPara(&pbuf[FramPos], &len, info, 1);
            FramPos += len;
            
        }
        *plen = FramPos;
        RMTHaveReadParaFlag = 3;
        break;
    case 3:
        //上装终端运行参数
        fnum = GetFeederNum();
        for(i=0; i<fnum; i++)
        {
            for(info= RMTP_RUN2_L+i*RMTP_RUN_NUM; info<=RMTP_RUN2_H+i*RMTP_RUN_NUM; info++)
            {
                sendnum++;
                pbuf[FramPos++] = LOBYTE(info);
                pbuf[FramPos++] = HIBYTE(info);
                pbuf[FramPos++] = 0;
                
                len = 0;
                GetTerminalPara(&pbuf[FramPos], &len, info, 1);
                FramPos += len;
            }
        }
        *plen = FramPos;
        RMTHaveReadParaFlag = 4;
        
        break;   
    case 4:
        //上装终端定值参数
        for(info=RMTP_ACT1_L; info<=RMTP_ACT1_H; info++)
        {
            sendnum++;
            pbuf[FramPos++] = LOBYTE(info);
            pbuf[FramPos++] = HIBYTE(info);
            pbuf[FramPos++] = 0;
            
            len = 0;
            GetTerminalPara(&pbuf[FramPos], &len, info, 1);
            FramPos += len;
            
        }
        *plen = FramPos;
        RMTHaveReadParaFlag = 5;
        break;
    case 5:
        //上装终端线路定值参数
        temp = RMTParaNum*RMTP_ACT_NUM; 
        for(info= RMTP_ACT2_L+temp; info<=RMTP_ACT2_H+temp; info++)
        {
            sendnum++;
            pbuf[FramPos++] = LOBYTE(info);
            pbuf[FramPos++] = HIBYTE(info);
            pbuf[FramPos++] = 0;
            
            len = 0;
            GetTerminalPara(&pbuf[FramPos], &len, info, 1);
            FramPos += len;
        }
        fnum = GetFeederNum();
        RMTParaNum++;
        if(RMTParaNum >= fnum)
        {
            RMTHaveReadParaFlag = 0;
            ScheduleFlag &= ~SCHEDULE_RMTPARA;
        }
        
        *plen = FramPos;
        break;
    default:
        RMTHaveReadParaFlag = 0;
        ScheduleFlag &= ~SCHEDULE_RMTPARA;
        break;    
    }
    
    *psendnum = sendnum;
    
    if(RMTHaveReadParaFlag)
        return TRUE;
    else
        return FALSE;
    
}
/*------------------------------------------------------------------/
函数名称：  EncodeRMTReadPara()
函数功能：  组织读参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::EncodeRMTReadPara(void)
{
    INT16U i; 
    INT8U len, sendnum,procnum;
    INT16U FramPos;
    BOOL rc, infoerr;
    
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;
    }
    
    infoerr = FALSE;
    
    TxMsg[0] = C_RS_NA;
    TxMsg[1] = 0;   //VSQ
    TxMsg[2] = ACTCON;
    TxMsg[3] = 0;
    
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    TxMsg[6] = 1;   //区号
    TxMsg[7] = 0;
    //TxMsg[8]  pi码
    
    FramPos = 9;
    if(RMTParaReadAllFlag)
    {
        rc = RMTReadAllPara(&TxMsg[FramPos], &len, &sendnum);   
        FramPos += len;
        TxMsg[1] = sendnum;
        if(rc == FALSE) 
        {
            TxMsg[8] = 0;     //参数特征，无后续 
            RMTParaInit();
            ScheduleFlag &= ~SCHEDULE_RMTPARA;
        }
        else
        {
            TxMsg[8] = RP_PI_CONT;     //参数特征，有后续
        }
    }
    else
    {
        sendnum = 0;
        procnum = 0;
        //部分读
        for(i=RMTHaveReadParaFlag; i<RMTParaNum ; i++)
        {
            procnum++;
            len = 0;
            
            GetTerminalPara(&TxMsg[FramPos+3], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {
                sendnum++;
                TxMsg[FramPos++] = LOBYTE(RMTParaInfo[i]);
                TxMsg[FramPos++] = HIBYTE(RMTParaInfo[i]);  //信息体地址
                TxMsg[FramPos++] = 0;
                
                FramPos += len;
            }
            else
            {
                //回答信息体地址错误，并结束读参数操作
                infoerr = TRUE;
                break; 
            }
            
            if(FramPos >= 200)
                break;
        }
        //logSysMsgNoTime("start=%d, sendnum=%d,Max=%d",RMTHaveReadParaFlag,sendnum,RMTParaNum,0);
        
        if(infoerr == FALSE)
        {
            TxMsg[1] = sendnum;
            RMTHaveReadParaFlag += procnum;
            //结束传送
            if(RMTHaveReadParaFlag >= RMTParaNum) 
            {
                RMTParaInit();
                TxMsg[8] = 0;     //参数特征，无后续 
                ScheduleFlag &= ~SCHEDULE_RMTPARA;
            }
            else
            {
                TxMsg[8] = RP_PI_CONT;     //参数特征，有后续
            }
        }
        else
        {
            //信息体地址错误，组织否定回答
            RMTParaInit();
            TxMsg[1] = 0;       //VSQ=0 
            TxMsg[2] = COT_PONO|UNKNOWNTINFOADDR;  //传送原因
            TxMsg[8] = 0;     //参数特征，无后续 
            FramPos  = 9; 
            ScheduleFlag &= ~SCHEDULE_RMTPARA;
        }
        
    }
    
    
    
    EnCodeDLMsg(FramPos);
}

/*------------------------------------------------------------------/
函数名称：  ProcReadParaGX
函数功能：  处理召唤参数报文，取出ROI（参数召唤限定词）。
输入说明：    
输出说明：  无。
备注：      ROI是表明主站要读取全参数还是分组召唤参数。
/------------------------------------------------------------------*/
void New104Sec::ProcReadParaGX(void)
{
    INT16U Len;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }
    
    Roi = RxMsg[AsduHeadLength];
    
    switch(Roi)
    {
        case INTROGEN:
        case INTRO1:
            GXParaControl = 1;
            break;
        case INTRO2:
        case INTRO3:
        case INTRO4:
        case INTRO5:
        case INTRO6:
        case INTRO7:
        case INTRO8:
        case INTRO9:
            GXParaControl = 2 * (Roi - INTROGEN) - 2;
            break;
        default:
            break;
    }
    
    TxMsg[0] = P_RS_NA_1_GX;

    TxMsg[1] = 1;

    int jj;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);

    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;
    TxMsg[CotLocation]=ACTCON;
    
    TxMsg[AsduHeadLength] = RxMsg[AsduHeadLength];
    Len = AsduHeadLength+1;
    
    if((Roi < INTROGEN) || (Roi > INTRO9))
    {
        TxMsg[CotLocation] |= 0x40;  
        EnCodeDLMsg(Len);                    
    }
    else
    {
        EnCodeDLMsg(Len);
        SendData1(SCHEDULE_GXPARA);
    }
    
}
/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXReadPara()
函数功能：  组织读参数
输入说明：  
输出说明：  rc = TRUE-有后续 FALSE-无后续
/------------------------------------------------------------------*/
void New104Sec::ProcEncodeGXReadPara(void)
{
    INT16U jj; 
    INT8U len, sendnum;
    INT16U FramePos;
    BOOL rc,rc2;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;
    }
    
    rc2 = TRUE;
    len = 0;    
    TxMsg[0] = P_ME_NA_1_GX;
    TxMsg[1] = 0;   //VSQ，后面填个数
    
    TxMsg[CotLocation]=Roi;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    
    FramePos = PubAddrLocation+PubAddrSize;    
    
    switch(Roi)
    {
        case INTROGEN:
            rc = GXReadAllPara(&TxMsg[FramePos], &len, &sendnum,InfoAddrSize,GXParaControl);
            GXParaControl++;
            FramePos += len;
            TxMsg[1] = sendnum | 0x80;  //VSQ SQ=1
            if(rc == FALSE) 
            { 
                rc2 = FALSE;
                //RMTParaInit();                      此处为何要调用此函数 20181018？
            }
            TxMsg[FramePos++] = 0x06;   //QPM 测量量参数定值上送，参数已投运、且参数可修改
            break;
            
        case INTRO1:
            
            rc = GXReadCommonPara(&TxMsg[FramePos], &len, &sendnum,InfoAddrSize);
            FramePos += len;
            TxMsg[1] = sendnum | 0x80;  //VSQ SQ=1
            if(rc == FALSE) 
            { 
                rc2 = FALSE;
            }
            TxMsg[FramePos++] = 0x06;
            break;
            
        case INTRO2:
        case INTRO3:
        case INTRO4:
        case INTRO5:
        case INTRO6:
        case INTRO7:
        case INTRO8:
        case INTRO9:
             rc = GXReadJianGePara(&TxMsg[FramePos], &len, &sendnum, InfoAddrSize,GXParaControl);     //读间隔参数,Roi-INTRO2为间隔编号，从0开始
             GXParaControl++;
             FramePos += len;
             TxMsg[1] = sendnum | 0x80; //VSQ SQ=1
             if(rc == FALSE) 
             { 
                 rc2 = FALSE;
             }
             TxMsg[FramePos++] = 0x06;
             break;
        default:                                
            rc2 = FALSE;
            break;
    }
    
       
    EnCodeDLMsg(FramePos);
    
    if(!rc2)
    {
        ScheduleFlag &= ~SCHEDULE_GXPARA;
        SendData1(SCHEDULE_GXPARAEND);
    }
}

/*------------------------------------------------------------------/
函数名称：  EnCodeGXReadParaEnd
函数功能：  读参数结束。
输入说明：    
输出说明：  无。
备注：      将参数发送完毕后，以此帧报文结束。
/------------------------------------------------------------------*/
void New104Sec::EnCodeGXReadParaEnd(void)
{
    INT16U Len;
    
    TxMsg[0]=P_RS_NA_1_GX;

    TxMsg[1] = 1;

    int jj;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;
    TxMsg[CotLocation]=ACTTERM;

    TxMsg[AsduHeadLength] = RxMsg[AsduHeadLength];
    Len=AsduHeadLength+1;
    
    EnCodeDLMsg(Len);
    ScheduleFlag &= ~SCHEDULE_GXPARAEND;    
}

void New104Sec::ProcSetParaGX(void)
{
    INT8U *pInfoAddr;
    INT16U i,pos;
    float tempval;
    INT32U temp32;
    INT16U infoaddr;
    
    //把数据暂存到wrongdata中
    //WrongDataLength = LengthIn;
    //GXvsqflag = 0;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }
   
    if(RxMsg[FrameLen+1] == 9)  //QPM
    {
        if((RxCot&COT_REASON)==ACT)    //激活
        {
            pInfoAddr = &RxMsg[InfoAddrLocation];
           
            pos = 0;
            
            if(GXParaYZ)   //只接受一帧报文的预置，没处理完不接受其他预置报文
            {
                logSysMsgNoTime("上一帧预置参数还未激活起效",0,0,0,0);
                return;
            }
            GXParaNum = RxVsq&VSQ_NUM;    //暂时不考虑 固有参数的写入
            if((RxVsq & VSQ_SQ) == 0)
            {
                for(i=0; i<GXParaNum; i++)
                {
                    GXParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
                    pos += 3;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("gx预置参数info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                } 
                            
            }
            else
            {
                //GXvsqflag = 1;
                //GXParaInfo[0] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
                infoaddr = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                pos += 3;
                for(i=0; i<GXParaNum; i++)
                {
                    GXParaInfo[i] = infoaddr + i;
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("gx预置参数info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                }
            }
            
            if(GXRemoteParaCheck() == 1)
            {
                //参数异常 （应调用变量清0函数）
                GXParaInit();             
                GXReturnCot = ACTCON|0x40;  //否定回答
            }
            else
            {
                GXParaYZ = TRUE;
                GXReturnCot = ACTCON; 
            }          
        }
        else if((RxCot&COT_REASON)==DEACT)  //撤销
        {
            
            GXParaInit();        
            GXReturnCot = DEACTCON;
            //SetSendData2Flag(DATA2_GX_SETPARA);
            
        }
        else
        {
            GXReturnCot = UNKNOWNCOT;
        }
    }
    else
    {
        GXReturnCot = ACTCON | 0x40;
        ProgLogWrite2("gx预置参数 QPM=%d 错误",RxMsg[FrameLen+1],0,0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
        
    }
    
    RxMsg[CotLocation] = GXReturnCot;
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    EnCodeDLMsg(FrameLen+2);    
}
/*------------------------------------------------------------------/
函数名称：  GXRemoteParaCheck
函数功能：  逐一检查参数异常
输入说明：    
输出说明：  
备注：      有异常停止后续检查
/------------------------------------------------------------------*/
INT16U New104Sec::GXRemoteParaCheck(void)
{
    INT16U i, rc;
    
    rc = 0; 
    for(i=0; i<GXParaNum; i++)
    {
        rc = GXParaSetCheck(GXParaValue[i], GXParaInfo[i]);
        if(rc == 1)
            break;
    }
    
    return rc;
    
}
/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXSendPara()
函数功能：  组织主动发送参数的报文
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::ProcEncodeGXSendPara(void)
{
    INT8U len, sendnum;
    INT16U FramePos, jj;
    BOOL rc;  
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }
            
    TxMsg[0] = P_ME_NA_1_GX;
    TxMsg[CotLocation] = 3;
    
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);       
        
    FramePos = PubAddrLocation+PubAddrSize;  
    
    rc = GXReadAllPara(&TxMsg[FramePos], &len, &sendnum,InfoAddrSize,GXParaControl);
    GXParaControl++;
    FramePos += len;
    TxMsg[1] = sendnum | 0x80;  //VSQ SQ=1
    
    TxMsg[FramePos++] = 0x06;   //QPM 测量量参数定值上送，参数已投运、且参数可修改 
    
    //rc = GXReadJianGePara(&TxMsg[FramePos], &len, &sendnum, InfoAddrSize,GXParaControl);     //读间隔参数,Roi-INTRO2为间隔编号，从0开始
    //GXParaControl++;
    //FramePos += len;
    //TxMsg[1] = sendnum | 0x80;
    //TxMsg[FramePos++] = 6;  //QPM
    
    EnCodeDLMsg(FramePos);  
    
    if(rc == FALSE)
    {
        ScheduleFlag &= ~SCHEDULE_GXSENDPARA;
    }
}
/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXChangePara()
函数功能：  组织回复改变参数的报文
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::ProcEncodeGXChangePara(void)
{
    INT16U jj; 
    INT16U i;
    INT16U FramePos; 
    float  value;
    INT8U *p;  
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    } 
        
    TxMsg[0] = P_ME_NA_1_GX;
    TxMsg[1] = GXParaNum;        //vsq
    TxMsg[CotLocation] = 3;
           
    for(jj=0;jj<PubAddrSize;jj++)
    {
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    }
    
    FramePos = PubAddrLocation+PubAddrSize;   
    
    for(i=0; i<GXParaNum; i++)
    {
        TxMsg[FramePos++] = LOBYTE(GXParaInfo[i]);
        TxMsg[FramePos++] = HIBYTE(GXParaInfo[i]);
        TxMsg[FramePos++] = 0x00;
    
        value = GxGetParaValue(GXParaInfo[i]);
        p = (INT8U*)&value;
        TxMsg[FramePos++] = p[3];
        TxMsg[FramePos++] = p[2];
        TxMsg[FramePos++] = p[1];
        TxMsg[FramePos++] = p[0];

    } 
    
    TxMsg[FramePos++] = 6;
    
    EnCodeDLMsg(FramePos);    
}    
void New104Sec::ProcActivateParaGX(void)
{
    INT8U i;
    INT16U SetFlag;
    
    if(GXParaYZ)
    {
        Qpa = RxMsg[FrameLen+1];
        
        if(Qpa == 1)
        {
            for(i=0; i<GXParaNum; i++)
            {
                SetFlag = SetTerminalParaGX(GXParaValue[i], GXParaInfo[i]);
            }
            GXRemoteParaEffect(); 
            
            if(GxGetDeadValueFlag(SetFlag))
            {
                
                DeadValueRealTimeEffect();
                //logSysMsgNoTime("104广西遥测死区值在线起效",0,0,0,0);
            }
            
            
            GXParaYZ = FALSE;   
            GXReturnCot = ACTCON;    
        }
        else
        {
            GXReturnCot = ACTCON|0x40; 
        }
    }
    else
    {
        GXReturnCot = ACTCON|0x40;
        Qpa = 0xff;     //暂时这样回否定回答
        logSysMsgNoTime("GX远程参数未预置就固化104",0,0,0,0);
    } 
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }
    
    //回确认帧
    RxMsg[CotLocation] = GXReturnCot;
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    EnCodeDLMsg(FrameLen+2);
     
    ProcEncodeGXChangePara();  
    
    GXParaInit();
}
/*------------------------------------------------------------------/
函数名称：  GXWatchLPChange
函数功能：  广西远程参数。监视本地参数变化。
输入说明：    
输出说明：  无。
备注：      ROI是表明主站要读取全参数还是分组召唤参数。
/------------------------------------------------------------------*/
void New104Sec::GXWatchLPChange(void)
{
    INT16U no;
    
    if(pDLink->CommConnect == FALSE)
        return;
    
    no = GXGetChangeYxFlag();
    
    if(no == 1)
    {
        GXParaControl = 1;  //2 * (Roi - INTROGEN) - 2;
        SendData1(SCHEDULE_GXSENDPARA);
    }
}
 
/*------------------------------------------------------------------/
函数名称：  ProcReadPara()
函数功能：  处理读参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::ProcReadPara(void)
{
    INT8U *pInfoAddr;
    INT16U i, pos;
    INT16U num;
    
    //数据长度大于8，表示是部分读取，否则为全部读取
    if(FrameLen+2 > 9) //FrameLen+2是去掉控制域C的长度
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pInfoAddr = &RxMsg[InfoAddrLocation+2];
        pos = 0;
        num = ((FrameLen+2)-8)/3;   //计算读取参数的个数
        
        if(RMTParaNum ==0 ) //第1次读
        {
            RMTHaveReadParaFlag = 0;
            RMTParaReadAllFlag = 0;
        }
        if(RMTParaNum+num >= RMT_RW_MAXNUM)
        {
            num = RMT_RW_MAXNUM-RMTParaNum-1;
        }
        
        for(i=RMTParaNum; i<RMTParaNum+num; i++)
        {
            RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
            pos += 3;
        }
        
        logSysMsgNoTime("sec=%d, num=%d, max=%d, info2=%x",RMTSectionNo,num,RMTParaNum,RMTParaInfo[RMTParaNum]);

        RMTParaNum += num;  //记录累计下发的读参数个数
    }
    else
    {
        //全部读取   
        RMTParaReadAllFlag = TRUE;
        RMTHaveReadParaFlag = 1;
        RMTParaNum = 0;
    }
    
    SendData1(SCHEDULE_RMTPARA);
    
}
/*------------------------------------------------------------------/
函数名称：  RMTParaYzCheck()
函数功能：  
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::RMTParaYzCheck(void)
{
    if(RMTParaYZ == FALSE)
    {
        RMTTimeOut = 0;
        return;
    }
    
    RMTTimeOut++;
    if(RMTTimeOut >= 60)
    {
        RMTParaInit();
    }
}

/*------------------------------------------------------------------/
函数名称：  GXParaYzCheck()
函数功能：  
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::GXParaYzCheck(void)
{
    if(GXParaYZ == FALSE)
    {
        GXTimeOut = 0;
        return;
    }
    
    GXTimeOut++;
    if(GXTimeOut >= 30)
    {
        GXParaYZ = false;
    }
}
/*------------------------------------------------------------------/
函数名称：  RMTParaInit()
函数功能：  远程参数读写相应标志初始化
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::RMTParaInit(void)
{
    INT16U i;

    RMTHaveReadParaFlag = 0;
    RMTParaReadAllFlag = 0;
    RMTSectionNo = 0;
    RMTParaNum = 0;
    RMTParaYZ = FALSE;
    RMTTimeOut = 0;
    
    for(i=0;i<RMT_RW_MAXNUM;i++)
    {
        RMTParaInfo[i] = 0;
        RMTParaValue[i] = 0;
    }
}

void New104Sec::GXParaInit(void)
{
    GXParaYZ = FALSE;
    GXParaInfo[0] = 0;  
    GXParaNum = 0; 
}
/*------------------------------------------------------------------/
函数名称：  DeadValueRealTimeEffect()
函数功能：  下发死区参数实时起效
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::DeadValueRealTimeEffect(void)
{
    INT8U i;
    for(i=0;i<DevCount;i++)
    {
        ReadAIMaxVal(i);
    }  
}
/*------------------------------------------------------------------/
函数名称：  ProcWritePara()
函数功能：  处理写参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::ProcWritePara(void)
{
    INT8U pi;
    INT8U *pInfoAddr;
    INT16U i,pos;
    INT16U ParaFlag;
    BOOL IsSuccess;
    INT8U datatype, datalen;
    float tempval;
    INT32U temp32;  
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }        

    if ((RxCot&COT_REASON)==ACT)    //激活
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pi  = RxMsg[InfoAddrLocation+2];
        pInfoAddr = &RxMsg[InfoAddrLocation+3];
                
        pos = 0;
        if(pi & RP_PI_SE)   //预置（参数选择）
        {
            if(RMTParaYZ)   //只接受一帧报文的预置，没处理完不接受其他预置报文
            {
                logSysMsgNoTime("上一帧参数预置报文还未处理",0,0,0,0);
                return;
            }   
            RMTParaNum = RxVsq&VSQ_NUM;    //暂时不考虑 固有参数的写入
            for(i=0; i<RMTParaNum; i++)
            {
                RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
                pos += 3;
                
                datatype = pInfoAddr[pos++]; //数据类型
                datalen  = pInfoAddr[pos++]; //数据长度
                
                if((datatype == PARA_DATA_TYPE_WORD) && (datalen == 2))
                {            
                    RMTParaValue[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                    pos +=2 ; 
                    
                    ProgLogWrite2("预置参数info=0x%x, value=%d",RMTParaInfo[i],RMTParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else if((datatype == PARA_DATA_TYPE_DWORD) && (datalen == 4))
                {            
                    RMTParaValue[i] = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    pos +=4 ; 
                    
                    ProgLogWrite2("预置参数info=0x%x, value=%d",RMTParaInfo[i],RMTParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else if((datatype == PARA_DATA_TYPE_FLOAT) && (datalen == 4))
                {
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;     
                    pos +=4 ; 
                    ProgLogWrite2("预置参数info=0x%x, value=%d.%3d",RMTParaInfo[i],(INT16U)RMTParaValue[i],(INT16U)((RMTParaValue[i]-(INT16U)(RMTParaValue[i]))*1000),0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else if((datatype == PARA_DATA_TYPE_BOOL) && (datalen == 1))
                {
                    RMTParaValue[i] = pInfoAddr[pos];
                    pos +=1;
                    ProgLogWrite2("预置参数info=0x%x, value=0x%x",RMTParaInfo[i],RMTParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else
                {
                    logSysMsgNoTime("预置参数 info=0x%x 数据类型错误(%d),数据长度=%d",RMTParaInfo[i], datatype,datalen,0);
                    break;
                }
            } 
            if(i>=RMTParaNum)
            {
                RMTParaYZ = TRUE;
                RxMsg[CotLocation] = ACTCON;
                memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
                EnCodeDLMsg(FrameLen+2);
            }
            else
            {
                RxMsg[CotLocation] = COT_PONO|ACTCON;
                memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
                EnCodeDLMsg(FrameLen+2);
            }
            
        }
        else
        {
            //固化（参数执行）
            IsSuccess = TRUE;
            if((pi & RP_PI_CR) == 0)
            {
                if(RMTParaYZ)
                {
                    
                    //这里不需要携带参数
                    ProgLogWrite2("收到固化参数命令", 0, 0, 0, 0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    //远程参数设置固化过程不检查携带的参数
                    for(i=0; i<RMTParaNum; i++)
                    {
                        ParaFlag = SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]);
                        
                    }
                    
                }
                else
                {
                    IsSuccess = FALSE;
                    logSysMsgNoTime("远程参数未预置就固化",0,0,0,0);
                }
                
                RMTParaInit();
                
                RxMsg[CotLocation] = ACTCON;
                
                if(IsSuccess==FALSE)
                {
                	RxMsg[CotLocation] |= 0x40; //失败
                }
                else
                {
                    
                    if(GetSiQuChangeFlag(ParaFlag))           //规约层的参数实时起效都可以同样按照死区值的实时起效的方式实现。
                    {
                        DeadValueRealTimeEffect();
                    }
                    SaveTerminalPara(); //固化完毕，写入flash
                    SaveRMTParaToSys(); //检测是否需要更新系统参数文件
                }
                
                
                memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
                EnCodeDLMsg(FrameLen+2);
                
            }  
        }
        
    }
    else
    {
        //撤销
        RMTParaInit();
        
        RxMsg[CotLocation] = DEACTCON;
        memcpy((void*)TxMsg, (void*)RxMsg, FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
    }
    
    
    
}
/*------------------------------------------------------------------/
函数名称：  ProcSetSectionNo()
函数功能：  切换定值区号
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::ProcSetSectionNo(void)
{
    //INT8U SectionNo;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;
    }
    
    RMTSectionNo2 = MAKEWORD(pRxData[0],pRxData[1]);
    if(RMTSectionNo < 5)
    {
        logSysMsgNoTime("设置当前区号=%d",RMTSectionNo,0,0,0);
    }
    else
        RMTSectionNo = 0;
        
    RxMsg[CotLocation] = ACTCON;
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    EnCodeDLMsg(FrameLen+2);
    
}


/*------------------------------------------------------------------/
函数名称：  ProcReadSectionNo()
函数功能：  切换定值区号
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::ProcReadSectionNo(void)
{
    INT8U FramePos;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;    //这里存在万一取不到空闲区域，有不应答的风险。 ll
    }
    
    //SectionNo = pRxData[0];
    
    RxMsg[CotLocation] = ACTCON;
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    
    TxMsg[0] = C_RR_NA;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = ACTCON;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    TxMsg[6] = 0;   
    TxMsg[7] = 0;
    TxMsg[8] = 0;
 
    FramePos = 9;
    TxMsg[FramePos++] = RMTSectionNo2;  //当前区号
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;
    TxMsg[FramePos++] = 0;
    
        
    EnCodeDLMsg(FramePos);
    
}

/*------------------------------------------------------------------/
函数名称：  CheckFREOnTime()
函数功能：  定时扫描故障事件上送.只有扩展规约才需要
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::CheckFREOnTime(void)
{
    INT16U devid;
    
    if(GYKZ2015Flag == FALSE)
        return;
    
    devid=DevList[NvaActDevNo].DevID;
    
    if(testGWFREvent(devid) == TRUE)
    {
        SendData1(SCHEDULE_FT_FREVENT);
    }
    
}
/*------------------------------------------------------------------/
函数名称：  EnCodeFREvent()
函数功能：  故障事件上传
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::EnCodeFREvent(void)
{
    INT16U i, no, FramePos;
    struct FaultRec_t frevent;
    struct Iec101ClockTime_t time;
    float temp;
    INT32U dd;
    INT8U *p, *pdd;
    INT8U  sendpos, sendnum;
    
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return ;   //没有发送空间则保持标志位
        
    ScheduleFlag&=(~SCHEDULE_FT_FREVENT);
    
    if(GWFREventRead(&frevent, DevList[NvaActDevNo].DevID) == FALSE)    //暂时读完就更新rp指针
        return ;
    
    TxMsg[0] = M_FT_NA;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = SPONT;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    //这里没有信息体地址
    sendpos = 6;
    TxMsg[7] = M_SP_TB;             //遥信类型 单点 
    
    FramePos = 8;
    AbsTimeConvTo(&frevent.ActTime, (void*)&time, IEC101CLOCKTIME);
    //logSysMsgNoTime("%d-%d-%d %d",time.Year, time.Month, time.Day, time.Hour);
    
    sendnum = 0;
    for(i=0; i<frevent.yxnum; i++)
    {
        no = L_GetLogicBISendNo(DevList[NvaActDevNo].DevID, frevent.YxInfo[i]);
        if(no != 0xffff)
        {
        	sendnum++;
            no += LBI;
            TxMsg[FramePos++] = LOBYTE(no);
            TxMsg[FramePos++] = HIBYTE(no);
            TxMsg[FramePos++] = 0;      //因为标准上写的是2，封掉，以后注意
        
            TxMsg[FramePos++] = 0x01;
            
            //AbsTimeConvTo(&frevent.ActTime, (void*)&time, IEC101CLOCKTIME);
            TxMsg[FramePos++] = LOBYTE(time.MSecond);
            TxMsg[FramePos++] = HIBYTE(time.MSecond);
            TxMsg[FramePos++] = time.Minute;
            TxMsg[FramePos++] = time.Hour;
            TxMsg[FramePos++] = time.Day;
            TxMsg[FramePos++] = time.Month;
            TxMsg[FramePos++] = time.Year;
        }
        
    }
    TxMsg[sendpos] = sendnum;       //遥信个数
    
    sendpos = FramePos++;
    TxMsg[FramePos++] = Sec104Pad.AIType; 
    
    sendnum = 0;
    for(i=0; i<frevent.ycnum; i++)
    {
        no = L_GetLogicAISendNo(DevList[NvaActDevNo].DevID, frevent.YcInfo[i]);
        if(no != 0xffff)
        {
        	sendnum ++;
            no += LAI;
            TxMsg[FramePos++] = LOBYTE(no);
            TxMsg[FramePos++] = HIBYTE(no);
            TxMsg[FramePos++] = 0;
            
            switch(Sec104Pad.AIType)
            {
             
             case M_ME_NC: //短浮点数
                temp = SL_ReadAI_S(DevList[NvaActDevNo].DevID, no-LAI, frevent.actvalue[i]);
                //temp = (float)frevent.actvalue[i];
                p = (INT8U*)(&temp);
                pdd = (INT8U*)(&dd);
                *(pdd++) = *(p++);
                *(pdd++) = *(p++);
                *(pdd++) = *(p++);
                *(pdd++) = *(p++);
                TxMsg[FramePos++]=LLBYTE(dd);
                TxMsg[FramePos++]=LHBYTE(dd);
                TxMsg[FramePos++]=HLBYTE(dd);
                TxMsg[FramePos++]=HHBYTE(dd);
                break;
             case M_ME_NA:   
             case M_ME_NB:  
             default:   //M_ME_NA 归一化值
                TxMsg[FramePos++]=LOBYTE(frevent.actvalue[i]);
                TxMsg[FramePos++]=HIBYTE(frevent.actvalue[i]);
                break; 
            } 
        }
    }
    TxMsg[sendpos] = sendnum;       //遥测个数
    
    EnCodeDLMsg(FramePos);
        
}




/*------------------------------------------------------------------/
函数名称：  GetWhLogicDevID()
函数功能：  得到104 中的设备ID
输入说明：  
输出说明： 
备注：      
/------------------------------------------------------------------*/
INT16U GetWhLogicDevID(void)
{
    if(whTransLogID != 0xffff)
        return whTransLogID;
    else
        return 0;
}




void New104Sec::ProcXSFileSynFinish(void)
{
    ScheduleFlag |= SCHEDULE_XSFILESYNFINISH;
    pDLink->NotifyToAppSchedule();
}

void New104Sec::SendFreezeEvent2Pri101(void)
{
    SetFileSynInfoTaskIDSubstation(MySelf.AppTID);  //作为启动101主站任务开始的量
    if(XSFileSynInfo.TaskIDPri101[0]!=0)//代表第一个线损模块起的任务
    {
        myEventSend(GetFileSynInfoTaskID101(0),XSFREEZE);//给101主站任务发送消息 暂时先发给第一个101任务，后续是通过维护软件面板参数确认的。
    }
    else
    {
        logSysMsgWithTime("无支持2018标准的线损模块,不支持瞬时冻结！",0,0,0,0);  
    }
    if(XSFileSynInfo.TaskIDPri101[1]!=0)//代表有第二个线损模块起的任务
    {
        myEventSend(GetFileSynInfoTaskID101(1),XSFREEZE);
    }     
}

/*------------------------------------------------------------------/
函数名称：  EnCodeNACK()
函数功能：  否定确认
输入说明：  否定原因    
输出说明： 
备注：      
/------------------------------------------------------------------*/

void New104Sec::EnCodeNACK(INT16U Cot)
{
    if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
        return;
    memcpy(TxMsg,RxMsg,AsduHeadLength+1);
    
    TxMsg[2]=Cot;

    TxMsg[2]|=COT_PONO;

    EnCodeDLMsg(AsduHeadLength+1);
}

/*------------------------------------------------------------------/
函数名称：  ProcInitEnd()
函数功能：  初始化结束处理函数
输入说明：  
输出说明： 
备注：当勾选“复位时发初始化结束帧”：当且仅当复位后主站发送start终端回复确认后，才发送初始化结束帧。
不勾选：一旦收到Start回复确认后就发送初始化结束帧。默认为不勾选。      
/------------------------------------------------------------------*/

void New104Sec::ProcInitEnd()
{
    if(!(Sec104Pad.control & CON_RSTSEND_INITEND))
    {
        if(EnCodeInitEnd())
        {
            ScheduleFlag &= (~SCHEDULE_INITOK);
        }    
    }
    else 
    {
        if(InitFlag == 0xff)        //wjr  初始化结束
        {
            if(EnCodeInitEnd())
            {
                InitFlag = 0;
                ScheduleFlag &= (~SCHEDULE_INITOK);
            }
        }
        else
        {
            ScheduleFlag &= (~SCHEDULE_INITOK);
        }
    }
}

/*------------------------------------------------------------------/
函数名称：  ProcReadParaGD
函数功能：  处理召唤参数报文。
输入说明：    
输出说明：  无。
备注：      
/------------------------------------------------------------------*/
void New104Sec::ProcReadParaGD(void)
{
    INT8U *pInfoAddr;
    INT16U i, pos;
    //INT16U num;
    INT16U infoaddr;
    
    
    pInfoAddr = &RxMsg[InfoAddrLocation];
    RMTParaNum = RxVsq&VSQ_NUM; 
    //if(RMTParaNum > 33)
    //    RMTParaNum = 33;   //一帧最多传33个，一般只传30个
    pos = 0; 
    if((RxVsq & VSQ_SQ) == 0)
    {
        for(i=0; i<RMTParaNum; i++)
        {
            RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
            pos += 7; //3字节信息体地址+4字节数据
                          
        } 
                    
    }
    else
    {
        infoaddr = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
        pos += 3;
        for(i=0; i<RMTParaNum; i++)
        {
            RMTParaInfo[i] = infoaddr + i;         
        }
    }
    
    EncodeRMTReadPara_GD();
    
}
/*------------------------------------------------------------------/
函数名称：  EncodeRMTReadPara_GD()
函数功能：  组织读广东远程参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::EncodeRMTReadPara_GD(void)
{
    INT16U i; 
    INT8U len, vsq_sq;
    INT16U FramPos;
    BOOL infoerr;
    
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;
    }
    
    infoerr = FALSE;
    
    TxMsg[0] = GD_MUTIPARA_READ;
    TxMsg[1] = 0;   //VSQ
    TxMsg[2] = ACTCON;  //COT
    TxMsg[3] = 0;
    
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramPos = 6;
    
    vsq_sq = RxVsq & VSQ_SQ;
    if(vsq_sq == 0)
    { 
        for(i=0; i<RMTParaNum ; i++)
        {
            len = 0;
            
            GetTerminalPara(&TxMsg[FramPos+3], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {
                TxMsg[FramPos++] = LOBYTE(RMTParaInfo[i]);
                TxMsg[FramPos++] = HIBYTE(RMTParaInfo[i]);  //信息体地址
                TxMsg[FramPos++] = 0;
                
                FramPos += len;
            }
            else
            {
                //回答信息体地址错误，并结束读参数操作
                infoerr = TRUE;
                break; 
            }
            
            if(FramPos >= 220)
                break;
        }
    }
    else
    {
        TxMsg[FramPos++] = LOBYTE(RMTParaInfo[0]);
        TxMsg[FramPos++] = HIBYTE(RMTParaInfo[0]);  //信息体地址
        TxMsg[FramPos++] = 0;
        for(i=0; i<RMTParaNum ; i++)
        {
            len = 0;
            
            GetTerminalPara(&TxMsg[FramPos], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {               
                FramPos += len;
            }
            else
            {
                //回答信息体地址错误，并结束读参数操作
                infoerr = TRUE;
                break; 
            }
            
            if(FramPos >= 220)
                break;
        }
    }
    //logSysMsgNoTime("start=%d, sendnum=%d,Max=%d",RMTHaveReadParaFlag,sendnum,RMTParaNum,0);
        
    if(infoerr == FALSE)
    {
        TxMsg[1] = RMTParaNum|vsq_sq;
        RMTParaInit();
    }
    else
    {
        //信息体地址错误，组织否定回答
        RMTParaInit();
        TxMsg[1] = 0;       //VSQ=0 
        TxMsg[2] = COT_PONO|UNKNOWNTINFOADDR;  //传送原因
        
    }

    EnCodeDLMsg(FramPos);
}
/*------------------------------------------------------------------/
函数名称：  ProcWritePara_GD()
函数功能：  处理写参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void New104Sec::ProcWritePara_GD(void)
{
    INT8U qos;
    INT8U *pInfoAddr;
    INT16U i,pos,info;
    INT16U curparainfo;
    float tempval;
    INT32U temp32;  
    INT16U writeflag;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }    
    
    if(ReadRemoteParaSetEnableState() == FALSE)
    {
        //否定回答
        RxMsg[CotLocation] = ACTCON|0x40;
        memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
        
        logSysMsgNoTime("远方整定投入软压板为分，禁止修改参数！！",0,0,0,0);
        return;
    }    
    
    writeflag = 0;
    if ((RxCot&COT_REASON)==ACT)    //激活
    {
        pInfoAddr = &RxMsg[InfoAddrLocation];
                
        pos = 0;    
        RMTParaNum = RxVsq&VSQ_NUM;    //暂时不考虑 固有参数的写入
        if((RxVsq & VSQ_SQ) == 0)
        {
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+7];
                if(qos & 0x80)  //预置
                {
                    RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
                    pos += 3;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    ProgLogWrite2("预置参数info=0x%x, value=%d,num=%d",RMTParaInfo[i],RMTParaValue[i],RMTParaNum,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    //执行
                    curparainfo = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                    pos += 3; 
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //判断预置和激活相同，则设置
                    if((curparainfo == RMTParaInfo[i]) && (tempval == RMTParaValue[i]))
                    {
                        SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]);  
                        writeflag = 1;
                    }
                    
                    
                }
    
            } 
        }
        else
        {
            //ProgLogWrite2("104暂不支持vsq=1的情况",0,0,0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
            info = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
            pos += 3;
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+4];
                if(qos & 0x80)  //预置
                {
                    RMTParaInfo[i] = info+i;
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    ProgLogWrite2("预置参数info=0x%x, value=%d,num=%d",RMTParaInfo[i],RMTParaValue[i],RMTParaNum,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    curparainfo = info+i;
                    //执行
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //判断预置和激活相同，则设置
                    if((curparainfo == RMTParaInfo[i]) && (tempval == RMTParaValue[i]))
                    {
                        SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]);  
                        writeflag = 1;
                    } 
                }
    
            }
        }   
        //确认回答
        RxMsg[CotLocation] = ACTCON;
        memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
        
        if(writeflag)   //固化
        {
            SaveTerminalPara(); //固化完毕，写入flash
            SaveRMTParaToSys(); //检测是否需要更新系统参数文件
            
            RMTParaInit();
        }
        
    }
    else
    {
        //撤销
        RMTParaInit();
        
        RxMsg[CotLocation] = DEACTCON;
        memcpy((void*)TxMsg, (void*)RxMsg, FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
    }
    
    
    
}
