#include "secapp.h"
#include "secdlink.h"
#include "procdef.h"

//线损文件同步  CL 20180504
#include "..\newhis\XSDataProc.h"
//线损文件同步  CL 20180504

extern struct DevConf_t *Device;
extern INT8U LC_Flag;
//INT16U LCflag = 0;
extern INT8U SgcMasterRandbuf[16];
extern INT16U AuthEndflag ;

void new101sec(INT16U AppID)
{
    UINT32	dwEvent;
    INT32U	dwDLTimerID;
    INT32U	dwAPPTimerID;
    //taskDelay(3000);
    //生成APP对象
    CSecAppSev *pSecApp;
    pSecApp=new CSecAppSev(AppID);
    
    
    
    if(!pSecApp)
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec 任务创建失败1",0,0,0,0);
        #else
        logSysMsgNoTime("101 Sec Delete Itself",0,0,0,0);
        #endif
        myTaskSuspendItself();
    }
    else if(!pSecApp->InitSecApp())
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec 任务创建失败2",0,0,0,0);
        #else
        logSysMsgNoTime("101 Sec Delete Itself",0,0,0,0);
        #endif
        delete pSecApp;
        myTaskSuspendItself();
    }

    //生成DL对象
    CSecDLink *pDLink;
    pDLink=new CSecDLink(AppID,pSecApp);

    if(!pDLink)
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec 任务创建失败3",0,0,0,0);
        #else
        logSysMsgNoTime("101 Sec Delete Itself",0,0,0,0);
        #endif
        delete pSecApp;
        myTaskSuspendItself();
    }
    else if(!pDLink->InitSecDLink())
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec 任务创建失败4",0,0,0,0);
        #else
        logSysMsgNoTime("101 Sec Delete Itself",0,0,0,0);
        #endif
        delete pSecApp;
        delete pDLink;
        myTaskSuspendItself();
    }

    if(!MisiPortOpen(AppID,RX_AVAIL,TX_AVAIL,EX_STATE))
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("串口打开错误，101 Sec 任务删除",0,0,0,0);
        #else
        logSysMsgNoTime("Serial Open Fail,101 Sec Delete",0,0,0,0);
        #endif
        delete pSecApp;
        delete pDLink;
        myTaskSuspendItself();
    }
    //设置定时器
    tm_evevery(1*SYSCLOCKTICKS,APPTIMEFLAG,&dwAPPTimerID);
    tm_evevery(1*SYSCLOCKTICKS,TIMERFLAG,&dwDLTimerID);     //秒定时器
    
    //判断通讯模式
    //if((pDLink->BalanMode))  //平衡模式且非支持扩展规约时 双向建立连接
    //    myEventSend(myTaskIdSelf(),SCHEDULE);
        
    FlushBuf(AppID,1);
    FlushBuf(AppID,0);
    //整点存电度事项
    pSecApp->InitHisDataBuf();
    pSecApp->RebootCheckUDataFlag();
    pSecApp->PassSprValueToLink(&pDLink->En_LinkAddrSize,&pDLink->En_CotSize,&pDLink->En_PubAddrSize,&pDLink->En_InfoAddrSize);
    ////pDLink->InitEnSprParaData(pSecApp->LinkAddrSize,pSecApp->CotSize,pSecApp->PubAddrSize,pSecApp->InfoAddrSize);
    //进入事项处理循环
    for(;;)
    {
        myEventReceive(RX_AVAIL|TX_AVAIL|TIMERFLAG|APPTIMEFLAG|UDATAFLAG|UMSGFLAG|SCHEDULE|FORCESCHEDULE
                        |NEXTFRAME|SENDOVER|SAVEKWHFLAG|XSFILESYNFINISH,MY_EVENTS_WAIT_ANY ,WAIT_FOREVER ,&dwEvent);

        if (dwEvent&SAVEKWHFLAG)//保存定时电度
        {
            pSecApp->SaveKWHToBuf();
        }

        if(dwEvent&NEXTFRAME)
        {
            pDLink->SearchFrame();
        }

        if(dwEvent&RX_AVAIL)//接收数据
        {
            pDLink->RecMISIData();
        }

        if(dwEvent&TX_AVAIL)//发送数据
        {
            pDLink->SendDataToMISI();
        }

        if(dwEvent&SENDOVER)//发送数据
        {
            if(pDLink->BalanMode)
                pDLink->SendDataEnd();
        }

        if (dwEvent&UMSGFLAG)//YK消息处理（调用应用层服务）
        {
            pSecApp->SetUMsgFlag();

            if(pDLink->BalanMode&&pDLink->DLStatusCheck())
                pDLink->CallUMsg();
        }

        if(dwEvent&UDATAFLAG)//数据库事项处理（COS、SOE事项设置标志）
        {
            if(0 == pSecApp->LCFlag)
            {
                pSecApp->SetUDataFlag();
            }
            if(pDLink->BalanMode && pDLink->DLStatusCheck())
                pDLink->CallUData();
        }

        if(dwEvent&APPTIMEFLAG)//定时器（应用层）
        {
            pSecApp->OnTimer();
        }

        if(dwEvent&TIMERFLAG)//定时器（链路层）
        {
            pDLink->TimeOut();
        }

        if (dwEvent&SCHEDULE)//启动链路过程
        {
            pDLink->CallDLStatus();
        }
        if(dwEvent&FORCESCHEDULE)
        {
            pDLink->NotifyToAppSchedule();
        }
        if(dwEvent&XSFILESYNFINISH)
        {
            pSecApp->ProcXSFileSynFinish(); //文件同步停止
        }
    }
}

CSecAppSev::CSecAppSev(INT16U appid)
{
    LCBuf=(unsigned char *)malloc( 2048);
    LCFlag = 0;
    wAppID=appid;
}

CSecAppSev::~CSecAppSev()
{
}

BOOL CSecAppSev::InitSecApp(void)
{
    struct PortInfo_t *ppport;
    struct TaskInfo_t *pptask;

    pRestType = (INT8U *)nvramMalloc(1);    //复位规约进程状态记忆 ll 2010/07/20   for 广西规约测试
    
    
    if (GetTaskInfo(wAppID,&pptask)==TRUE)
    {
        
        MySelf.QID=(pptask)->Qid;
        MySelf.AppTID=(pptask)->Tid;
        MySelf.AppID=wAppID;
        MySelf.PortID=(pptask)->PortIDSet[0];
    }
    else
    {
        
        return FALSE;
    }

    pMsg=new PMessage;//分配消息缓冲区，该任务只有一个。

    if (GetPortInfo(MySelf.PortID,&ppport)==TRUE)//取端口信息地址
    {
        if ((ppport)->DevNum<=0)
        {
            return FALSE;
        }

        pDev=new PDevInfo;
        if(!pDev)
        {
            //logSysMsgNoTime("InitSecApp  3",0,0,0,0);
            return FALSE;
        }
        if ((ppport)->CodePad!=NULL)
        {
            memcpy(&Sec101Pad,(ppport)->CodePad,sizeof(struct PSec101Pad));
            CheckPad();
        }
        else
        {
            #ifdef _CHINESE_
            logSysMsgNoTime("iec101Sec 无规约面板！请配置",0,0,0,0);
            #else
            logSysMsgNoTime("iec101Sec No CodePad! Please Check. ",0,0,0,0);
            #endif
            SetProtocalErr();
            SetDefaultPad();
        }

        pDev->DevID=(ppport)->DevIDSet[0];  //设置设备ID

        if(!SetDevInfo(pDev->DevID))
            return FALSE;
        InitPara();
    }
    else
    {
        //logSysMsgNoTime("InitSecApp  2",0,0,0,0);
        return FALSE;
    }

    return TRUE;
}

void CSecAppSev::CheckPad(void)
{
    if (Sec101Pad.MaxALLen>ASDULEN)//应用层最大报文长度超限
        Sec101Pad.MaxALLen=ASDULEN;
    else if(Sec101Pad.MaxALLen<16)
        Sec101Pad.MaxALLen=ASDULEN;

    if ((Sec101Pad.ScanData2>3600))//二级数据扫描间隔
        NvaInterval=SCANDATA2TIMER;
    else
        NvaInterval=Sec101Pad.ScanData2;

    BackScanTime=Sec101Pad.BackScanTime*60;//分，背景数据扫描间隔
    CycScanTime=Sec101Pad.CycScanTime;//秒，周期循环数据扫描间隔

    if(Sec101Pad.BalanceMode == 1)
        BalanMode=TRUE;
    else
        BalanMode=FALSE;
    //历史电度
    if(Sec101Pad.HistoryDDPermit == 1)
    {
        HisDDCycle=Sec101Pad.HistoryDDTime;
    }

    LinkAddrSize=Sec101Pad.LINKAddrSize;
    CotSize=Sec101Pad.COTSize;
    PubAddrSize=Sec101Pad.PUBAddrSize;
    InfoAddrSize=Sec101Pad.INFOAddrSize;

    if(LinkAddrSize!=1)
        LinkAddrSize=2;
    if(CotSize!=2)
        CotSize=1;
    if(PubAddrSize!=1)
        PubAddrSize=2;
    if(InfoAddrSize!=3)
        InfoAddrSize=2;

    if(Sec101Pad.TypeID[16]!=M_IT_TA)
        Sec101Pad.TypeID[16]=M_IT_NA;
        
    if((Sec101Pad.LBIinfoaddr<LBI)||(Sec101Pad.LBIinfoaddr>HBI))        //遥信信息体地址可设置 2008.11.5  wjr
        LBIinfoaddr=LBI;
    else
        LBIinfoaddr=Sec101Pad.LBIinfoaddr;
        
    if((Sec101Pad.LDBIinfoaddr<LDBI)||(Sec101Pad.LDBIinfoaddr>HDBI))
        LDBIinfoaddr=LDBI;
    else
        LDBIinfoaddr=Sec101Pad.LDBIinfoaddr;
    
}
/******************************************************************
*函数名:PassSprValueToLink
*功能:将类的私有变量信息传递至链路层
*开发人:张良
*******************************************************************/
void CSecAppSev::PassSprValueToLink(INT16U *eparaI,INT16U *eparaII,INT16U *eparaIII,INT16U *eparaIV)
{
    ////INT16U 
    *eparaI = LinkAddrSize;
    *eparaII = CotSize;
    *eparaIII = PubAddrSize;
    *eparaIV = InfoAddrSize;
}

void CSecAppSev::SetDefaultPad(void)//缺省设置
{
    int i;
    Sec101Pad.ControlPermit = 1;       //遥控允许 1-允许，0-不允许 缺省为1
    Sec101Pad.SetTimePermit = 1;  //对钟允许 1-允许，0-不允许 缺省为1
    Sec101Pad.BalanceMode = 0;//平衡模式 1-平衡模式 0-非平衡模式 缺省为0
    Sec101Pad.SOEWithCP56Time = 1;//SOE使用长时标格式 1-56位长时标 0-24位短时标  缺省为1
    Sec101Pad.UseStandClock = 1;//使用标准时钟格式 1-标准 0-非标准 缺省为1
    Sec101Pad.MaxALLen = ASDULEN; //最大应用层报文长度 缺省250
    Sec101Pad.AIDeadValue = 3;//遥测死区值（千分比） 缺省3
    Sec101Pad.ScanData2 = SCANDATA2TIMER;//二级数据扫描间隔（秒） 缺省3
    Sec101Pad.TimeOut = 100;//超时时间（*10ms） 缺省100

    Sec101Pad.HistoryDDPermit = 0;//历史电度保存允许 1-允许 0-不允许 缺省0
    Sec101Pad.HistoryDDTime = 60;//历史电度保存周期（分） 缺省60


    BackScanTime=0;//分，背景数据扫描间隔
    CycScanTime=20;//秒，周期循环数据扫描间隔
    HisDDCycle=TIMETOSAVEDATA;

    BalanMode=FALSE;

    for(i=0;i<8;i++)
        Sec101Pad.TypeID[i]=M_PS_NA;//20;//具有状态变位检出的成组单点信息
    for(i=8;i<12;i++)
        Sec101Pad.TypeID[i]=M_ME_NB;//11;////测量值，标度化值
    Sec101Pad.TypeID[12]=0;//该组为空
    Sec101Pad.TypeID[13]=M_ME_NB;//11;//该组为参数
    Sec101Pad.TypeID[14]=M_ST_NA;//5;//该组为步位置信息
    for(i=0;i<16;i++)
        Sec101Pad.GroupNum[i]=0;

    Sec101Pad.TypeID[16]=M_IT_NA;

    LinkAddrSize=2;
    CotSize=1;
    PubAddrSize=2;
    InfoAddrSize=2;
    
    LBIinfoaddr=LBI;                     //遥信信息体地址可设置 2008.11.5  wjr
    LDBIinfoaddr=LDBI;
}

BOOL CSecAppSev::SetDevInfo(INT16U DevID)
{
    INT16U i,j,*temp;
    INT16 DevType;
    struct AppLBConf_t AppLBConf;
    struct AppSLBConf_t AppSLBConf;
    struct AppRBConf_t AppRBConf;

    DevType = CheckDevType(DevID);
    if(DevType < 0)
        return FALSE;

    if (DevType == 2)//二类逻辑设备
    {
        if(!SL_ReadBConf(DevID, wAppID, (INT8U *) &AppSLBConf))
            return FALSE;

        pDev->Flag=AppSLBConf.Flag;
        pDev->Addr=AppSLBConf.Address;
        pDev->MAddr=AppSLBConf.MAddress;
        pDev->pDbaseWin=AppSLBConf.DbaseWin;
        pDev->RealWin=NULL;

        pDev->DevData.AINum=0;
        pDev->DevData.BINum=0;
        pDev->DevData.DBINum=0;              //wjr
        pDev->DevData.CounterNum=0;
        pDev->DevData.BONum=0;
        pDev->DevData.SPINum=0;
        pDev->DevData.BCDNum=0;
        pDev->DevData.NvaNo=0;

        //管理的设备数目
        DevCount=AppSLBConf.DevNum;
        if (DevCount<=0)
            return(FALSE);

        DevList=new PDevInfo[DevCount];
        if(!DevList)
            return(FALSE);

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
                R_ReadBConf(DevList[i].DevID, wAppID,(INT8U *) &AppRBConf,1);

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
                DevList[i].DevData.BCDNum=0;
                DevList[i].DevData.NvaNo=0;
                DevList[i].DevData.DBINum=0;            //wjr
                DevList[i].RealWin=AppRBConf.RealWin;
                DevList[i].pDbaseWin=NULL;

                DevList[i].Data1.Flag=0;
                DevList[i].Data1.SOENum=0;            //SOE发送个数
                DevList[i].Data1.BIENTNum=0;      //变位遥信发送个数
                DevList[i].Data1.FAProcNum=0;
            }
            else if(DevType == 0)//I类逻辑设备
            {
                L_ReadBConf(DevList[i].DevID, wAppID,(INT8U *) &AppLBConf);

                DevList[i].Flag=AppLBConf.Flag;
                DevList[i].Addr=AppLBConf.Address;
                DevList[i].MAddr=AppLBConf.MAddress;
                DevList[i].DevData.AINum=AppLBConf.AINum;
                DevList[i].DevData.BINum=AppLBConf.BINum;
                DevList[i].DevData.DBINum=AppLBConf.DBINum;       //wjr
                DevList[i].DevData.CounterNum=AppLBConf.CounterNum;
                DevList[i].DevData.BONum=AppLBConf.BONum;
                //DevList[i].DevData.SPINum=AppLBConf.SPINum;
                //DevList[i].DevData.BCDNum=AppLBConf.BCDNum;
                DevList[i].DevData.SPINum=0;
                DevList[i].DevData.BCDNum=0;
                DevList[i].DevData.NvaNo=0;
                DevList[i].pDbaseWin=AppLBConf.DbaseWin;
                DevList[i].RealWin=NULL;

                DevList[i].Data1.Flag=0;
                DevList[i].Data1.SOENum=0;            //SOE发送个数
                DevList[i].Data1.BIENTNum=0;      //变位遥信发送个数
                DevList[i].Data1.FAProcNum=0;
            }
            else
                return FALSE;

            if(DevList[i].DevData.CounterNum)
            {
                if(DevList[i].DevData.CounterNum>MAXBCRNUM)
                    DevList[i].DevData.CounterNum=MAXBCRNUM;
                pDev->DevData.CounterNum+=DevList[i].DevData.CounterNum;
                DevList[i].DevData.CounterData=new RealCounter_t[DevList[i].DevData.CounterNum];
                if(DevList[i].DevData.CounterData==NULL)
                    return(FALSE);
                DevList[i].DevData.LastCounterData=new INT32U[DevList[i].DevData.CounterNum];
                if(DevList[i].DevData.LastCounterData==NULL)
                    return(FALSE);
                DevList[i].DevData.HisCounterData=new INT32U[DevList[i].DevData.CounterNum];
                if(DevList[i].DevData.HisCounterData==NULL)
                    return(FALSE);
            }
            else
            {
                DevList[i].DevData.CounterData=NULL;
                DevList[i].DevData.LastCounterData=NULL;
                DevList[i].DevData.HisCounterData=NULL;
            }

            if(DevList[i].DevData.AINum)
            {
                if(DevList[i].DevData.AINum>MAXAINUM)
                    DevList[i].DevData.AINum=MAXAINUM;
                pDev->DevData.AINum+=DevList[i].DevData.AINum;
                DevList[i].DevData.AIMaxVal=new short[DevList[i].DevData.AINum];
                DevList[i].DevData.AIData=new PNva[DevList[i].DevData.AINum];
                DevList[i].DevData.AIPara=new YcPara[DevList[i].DevData.AINum];
                if(!(DevList[i].DevData.AIMaxVal&&DevList[i].DevData.AIData&&DevList[i].DevData.AIPara))
                    return(FALSE);

                ReadAIMaxVal(i);
            }
            else
            {
                DevList[i].DevData.AIMaxVal=NULL;
                DevList[i].DevData.AIData=NULL;
                DevList[i].DevData.AIPara=NULL;
            }

            if(DevList[i].DevData.BINum)
            {
                if(DevList[i].DevData.BINum>MAXBINUM)
                    DevList[i].DevData.BINum=MAXBINUM;
                pDev->DevData.BINum+=DevList[i].DevData.BINum;
            }

            if(DevList[i].DevData.DBINum)                  //wjr
            {
            	if(DevList[i].DevData.DBINum>DevList[i].DevData.BINum)
                    DevList[i].DevData.DBINum=DevList[i].DevData.BINum;
            	pDev->DevData.DBINum+=DevList[i].DevData.DBINum;
            }
            
            if(DevList[i].DevData.BONum)
            {
                if(DevList[i].DevData.BONum>MAXBONUM)
                    DevList[i].DevData.BONum=MAXBONUM;
                pDev->DevData.BONum+=DevList[i].DevData.BONum;
            }

            if(DevList[i].DevData.SPINum)
            {
                if(DevList[i].DevData.SPINum>MAXSPINUM)
                    DevList[i].DevData.SPINum=MAXSPINUM;
                pDev->DevData.SPINum+=DevList[i].DevData.SPINum;
            }

        }//end of for

        YCGroupNo = new INT8U[pDev->DevData.AINum];
        YXGroupNo = new INT8U[pDev->DevData.BINum];
        memset(YXGroupNo,1,pDev->DevData.BINum);//多设备管理时不分组，遥信类型标识都用第一组的
        memset(YCGroupNo,9,pDev->DevData.AINum);//遥测类型标识都用第九组的，否则就乱了
        for(i=0;i<16;i++)
            Sec101Pad.GroupNum[i]=512;
        return(TRUE);
    }
    else if (DevType == 0)//一类逻辑设备
    {
        L_ReadBConf(DevID, wAppID,(INT8U *) &AppLBConf);

        pDev->Flag=AppLBConf.Flag;
        pDev->Addr=AppLBConf.Address;
        pDev->MAddr=AppLBConf.MAddress;
        pDev->pDbaseWin=AppLBConf.DbaseWin;
        pDev->RealWin=NULL;

        //管理的设备数目
        DevCount=1;

        DevList=new PDevInfo[1];
        if(!DevList)
            return(FALSE);

        DevList[0].DevID=pDev->DevID;
        DevList[0].Flag=AppLBConf.Flag;
        DevList[0].Addr=AppLBConf.Address;
        DevList[0].MAddr=AppLBConf.MAddress;
        DevList[0].DevData.AINum=AppLBConf.AINum;
        DevList[0].DevData.BINum=AppLBConf.BINum;
        DevList[0].DevData.DBINum=AppLBConf.DBINum;              //wjr双点遥信
        DevList[0].DevData.CounterNum=AppLBConf.CounterNum;
        DevList[0].DevData.BONum=AppLBConf.BONum;
        //DevList[0].DevData.SPINum=AppLBConf.SPINum;
        //DevList[0].DevData.BCDNum=AppLBConf.BCDNum;
        DevList[0].DevData.SPINum=0;
        DevList[0].DevData.BCDNum=0;
        DevList[0].DevData.NvaNo=0;
        DevList[0].pDbaseWin=AppLBConf.DbaseWin;
        DevList[0].RealWin=NULL;
          
        DevList[0].Data1.Flag=0;
        DevList[0].Data1.SOENum=0;            //SOE发送个数
        DevList[0].Data1.BIENTNum=0;      //变位遥信发送个数
        DevList[0].Data1.FAProcNum=0;

        if(DevList[0].DevData.CounterNum>MAXBCRNUM)
            DevList[0].DevData.CounterNum=MAXBCRNUM;
        if(DevList[0].DevData.AINum>MAXAINUM)
            DevList[0].DevData.AINum=MAXAINUM;
        if(DevList[0].DevData.BINum>MAXBINUM)
            DevList[0].DevData.BINum=MAXBINUM;
        if(DevList[0].DevData.DBINum>DevList[0].DevData.BINum)
            DevList[0].DevData.DBINum=DevList[0].DevData.BINum;            //wjr
        if(DevList[0].DevData.SPINum>MAXSPINUM)
            DevList[0].DevData.SPINum=MAXSPINUM;
        if(DevList[0].DevData.BONum>MAXBONUM)
            DevList[0].DevData.BONum=MAXBONUM;

        pDev->DevData.AINum=DevList[0].DevData.AINum;
        pDev->DevData.BINum=DevList[0].DevData.BINum;
        pDev->DevData.DBINum=DevList[0].DevData.DBINum;         //wjr
        pDev->DevData.CounterNum=DevList[0].DevData.CounterNum;
        pDev->DevData.BONum=DevList[0].DevData.BONum;
        pDev->DevData.SPINum=0;
        pDev->DevData.BCDNum=0;
        pDev->DevData.NvaNo=0;

        if(DevList[0].DevData.CounterNum)
        {
            DevList[0].DevData.CounterData=new RealCounter_t[DevList[0].DevData.CounterNum];
            if(DevList[0].DevData.CounterData==NULL)
                return(FALSE);
            DevList[0].DevData.LastCounterData=new INT32U[DevList[0].DevData.CounterNum];
            if(DevList[0].DevData.LastCounterData==NULL)
                return(FALSE);
            DevList[0].DevData.HisCounterData=new INT32U[DevList[0].DevData.CounterNum];
            if(DevList[0].DevData.HisCounterData==NULL)
                return(FALSE);
        }
        else
        {
            DevList[0].DevData.CounterData=NULL;
            DevList[0].DevData.LastCounterData=NULL;
            DevList[0].DevData.HisCounterData=NULL;
        }

        if(DevList[0].DevData.AINum)
        {
            DevList[0].DevData.AIMaxVal=new short[DevList[0].DevData.AINum];
            DevList[0].DevData.AIData=new PNva[DevList[0].DevData.AINum];
            DevList[0].DevData.AIPara=new YcPara[DevList[0].DevData.AINum];
            if(!(DevList[0].DevData.AIMaxVal&&DevList[0].DevData.AIData&&DevList[0].DevData.AIPara))
                return(FALSE);

            ReadAIMaxVal(0);
        }
        else
        {
            DevList[0].DevData.AIMaxVal=NULL;
            DevList[0].DevData.AIData=NULL;
            DevList[0].DevData.AIPara=NULL;
        }

        //为每个遥测遥信设置组号
        YCGroupNo = new INT8U[DevList[0].DevData.AINum];
        YXGroupNo = new INT8U[DevList[0].DevData.BINum];
        memset(YCGroupNo,9,DevList[0].DevData.AINum);
        memset(YXGroupNo,1,DevList[0].DevData.BINum);

        INT16U yc=0,yx=0;
        /*增加判断，如果遥信组总的遥信、遥测个数和实际个数对应不起来则任务挂起 wjr*/
        for(i=0;i<16;i++)
        {
            if(i<8)
                yx+=Sec101Pad.GroupNum[i];
                
            else if(i<12)
                yc+=Sec101Pad.GroupNum[i];
        }
        if((yx>DevList[0].DevData.BINum)||(yc>DevList[0].DevData.AINum))
        {
        	logSysMsgNoTime("遥测或遥信个数小于组中个数",0,0,0,0);
        	SetProtocalErr();
        	return(FALSE);
        }
        if((yx<DevList[0].DevData.BINum)||(yc<DevList[0].DevData.AINum))
            logSysMsgNoTime("分组遥测或遥信个数小于逻辑库中个数会遗漏后面的数据",0,0,0,0);
            
        yc=0;
        yx=0;
        
        for(i=0;i<16;i++)
        {
            if(i<8)
            {
                for(j=0;j<Sec101Pad.GroupNum[i];j++)
                    YXGroupNo[yx+j]=i+1;
                yx+=Sec101Pad.GroupNum[i];
            }
            else if(i<12)
            {
                for(j=0;j<Sec101Pad.GroupNum[i];j++)
                    YCGroupNo[yc+j]=i+1;
                yc+=Sec101Pad.GroupNum[i];
            }
        }
        if((yx == 0)&&(DevList[0].DevData.BINum > 0))
        {
            Sec101Pad.GroupNum[0] = DevList[0].DevData.BINum;
            yx = Sec101Pad.GroupNum[0];
        }
        if((yc == 0)&&(DevList[0].DevData.AINum > 0))
        {
            Sec101Pad.GroupNum[8] = DevList[0].DevData.AINum;
            yc = Sec101Pad.GroupNum[8];
        }

        
        #ifdef SFILETRANAPP101
        
        if (Sec101Pad.YCNum <= DevList[0].DevData.AINum)
        {
            SFileInfo.AllYCNum = DevList[0].DevData.AINum;
            DevList[0].DevData.AINum = Sec101Pad.YCNum;
        }

        #endif
        
        
        if(yx<DevList[0].DevData.BINum)//限制数目，避免总召唤出问题。组的总和不能小于发送表数目。
            DevList[0].DevData.BINum=yx;

        if(yc<DevList[0].DevData.AINum)
            DevList[0].DevData.AINum=yc;

        return TRUE;//最后返回
    }
    else
        return FALSE;
}

void CSecAppSev::ReadAIMaxVal(INT16U DevIndex)  //读遥测满值——设置死区值
{
    INT16U i;
    INT16U DevID;
    INT16U val;
    INT16U deathval;
    INT8U ppty;
    
    for (i=0;i<DevList[DevIndex].DevData.AINum;i++)
        DevList[DevIndex].DevData.AIMaxVal[i]=0;

    DevID = DevList[DevIndex].DevID;

    for (i=0;i<DevList[DevIndex].DevData.AINum;i++)
    {
        if(DevList[DevIndex].Flag == 1)//=1实际设备
            val = SRSendMax_ReadAI(DevID,i);
        else
            val = SLMax_ReadAI(DevID,i);

        DevList[DevIndex].DevData.AIMaxVal[i] = val;
        
        DevList[DevIndex].DevData.AIPara[i].porperty = SL_ReadAI_Porperty(DevID,i);
     
        //逐点设置死区值
        if(DevList[DevIndex].Flag == 1)
            deathval = SRDead_ReadAI(DevID,i);
        else
            deathval = SLDead_ReadAI(DevID,i);
            
        if(deathval > 1)    //非0，非1即认为有效
        {
            DevList[DevIndex].DevData.AIPara[i].DeadValue = deathval;   //不是千分比单位
        }
        else
        {
            ppty = DevList[DevIndex].DevData.AIPara[i].porperty;
            deathval = GetRmtDeathvalue(ppty);
            if(deathval > 0)    //远程参数死区值
            {
                DevList[DevIndex].DevData.AIPara[i].DeadValue = ((INT32U)val*deathval)/1000;
            }
            else //规约面板死区值
            {
                 DevList[DevIndex].DevData.AIPara[i].DeadValue = ((INT32U)val*Sec101Pad.AIDeadValue)/1000;
            }
        }
        
        DevList[DevIndex].DevData.AIPara[i].UpLimit = 0;
        DevList[DevIndex].DevData.AIPara[i].LowLimit = 0;
        DevList[DevIndex].DevData.AIPara[i].type = SL_ReadAI_Type(DevID,i); //读取当前数据的类型（有符号还是无符号）
        if (DevList[DevIndex].DevData.AIPara[i].DeadValue == 0)
            DevList[DevIndex].DevData.AIPara[i].DeadValue = 1;
    }

}

void CSecAppSev::InitPara(void)
{
    Data1.Flag=0;
    Data1.SOENum=0;            //SOE发送个数
    Data1.BIENTNum=0;      //变位遥信发送个数

    AllDataCount=0;
    CounterCount=0;
    WatchDogCount=0;
    NvaActDevNo=0;
    ActDevIndex=0;    //wjr增加初始化

    Status=Polling;
    LastStatus=Polling;
    LastFrame=Polling;
    BIFrame=0;
    LastDevIndex=0;

    if(Sec101Pad.control & CON_ISNEEDDELAY)
        RestDLDelay = 0;
    else
        RestDLDelay = 30;
    
    if(Sec101Pad.control & CON_RSTSEND_INITEND)
        IsAllSendInitEnd = FALSE;
    else
        IsAllSendInitEnd = TRUE;
    
    bSendAllDBI = FALSE;
    if(Sec101Pad.control & CON_ALLDBI_101)
        bSendAllDBI = TRUE;
        
    //支持"国网规约扩展"-2015版
    GYKZ2015Flag = FALSE;
    SendCOS = 1;
    if(Sec101Pad.control & CON_101GYKZ)
    {
        GYKZ2015Flag = TRUE; 
        SendCOS = 0;
        if(Sec101Pad.control & CON_101SENDCOS)
        {
            SendCOS = 1;
        }   
        logSysMsgNoTime("101支持GY2015扩展",0,0,0,0);
    }
    
    APP_DLSecStatus = SECDISABLE;
    APP_DLPriStatus = PRIDISABLE;   
    FirstCallAllData = 0;
    DLInitFinishFlag = FALSE;
    DLInitpreState = FALSE;    //AJ++180418
    WaitCallAllDelay = WAIT_CALLALL_DELAY;
    SetDevUseState();   //主要是初始化DLInitFinishFlag
    
    ScanData2Count=0;

    EditAllDataCon=0;
    EditReadParaCon = 0;

    SDTTime=0;
    TrTime=0;
    SendTime=0;
    TimeDelay=0;

    ResetFlag=0;
    ResetCount=0;

    Data2Flag=0;
    BackScanCount=0;
    CycScanCount=0;

    InitFlag=0xFF;
    HaveWrongData=FALSE;
    WrongDataLength=0;

    FileStep=PFileOver;
    memset((void*)&DirUnit,0,sizeof(struct DIRUNIT));
    CurrentFileSize=0;
    FileReadPtr=0;
    CurrentZBNo=0;

    HisDDStatus=Start;

    CotLocation=2;//COT在ASDU中的位置
    PubAddrLocation=CotLocation+CotSize;//PUBADDR在ASDU中的位置
    InfoAddrLocation=PubAddrLocation+PubAddrSize;//INFOADDR在ASDU中的位置
    AsduHeadLength=InfoAddrLocation+InfoAddrSize;//ASDU头的长度

    if(PubAddrSize==1)
        BroadCastAddr=0xff;
    else
        BroadCastAddr=0xffff;

    RFaultFlag=0;   //beijing
    YKSetAlready = FALSE;
    
    DBISOEnum=0;         //wjr双点遥信
    DBICOSnum=0; 
    
    DDFreeze=FALSE;
    
    YkStatusForTest = 0;
    
    RMTParaInit();
    GXParaInit();
    //新文件传输参数初始化
    ProcFileInit();
}
void CSecAppSev::GXParaInit(void)
{
    GXParaYZ = FALSE;
}
void CSecAppSev::SendAllDataOnTime(void)
{
    //背景数据标志（全数据以BACK原因发送）
    /*if(BackScanTime)
    {
        BackScanCount++;
        if(BackScanCount>=BackScanTime)
        {
            if(Data1.Flag&CallAllData)//避免冲突
            {
                BackScanCount=0;
                return;
            }
            if(Data2Flag&PerCycData)
                return;
    
            BackScanCount=0;
            Data2Flag|=BackData;
    
            GroupTrn.COT=BACK;
            GroupTrn.DevIndex=0;
            GroupTrn.GroupNo=1;//组号从1开始
            GroupTrn.InfoAddr=LBIinfoaddr;//信息体地址从0x1开始   wjr2009.4.5  改为双点遥信的起始地址，因为双点遥信在前
        }
    }*/
        //周期循环数据标志（第12组遥测数据，为了应付测试），测试时将此处打开
        /*CycScanCount++;
        if(CycScanCount>=CycScanTime)
        {
            CycScanCount=0;
            if(Data1.Flag&CallAllData)
                return;
            if(Data2Flag&BackData)
                return;
            Data2Flag|=PerCycData;
            GroupTrn.COT=PERCYC;
            GroupTrn.DevIndex=0;
            GroupTrn.GroupNo=12;//组号12的信息为周期循环信息
            GroupTrn.InfoAddr=LAI;//信息体地址？？
        }*/
    
    
    
}
/*------------------------------------------------------------------/
函数名称：  EnCodeFREvent()
函数功能：  故障事件上传
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::EnCodeFREvent(void)
{
    INT16U i, no, FramePos;
    struct FaultRec_t frevent;
    struct Iec101ClockTime_t time;
    float temp;
    INT32U dd;
    INT8U *p, *pdd, aitype;
    
    
    if(GWFREventRead(&frevent, DevList[NvaActDevNo].DevID) == FALSE)    //暂时读完就更新rp指针
        return ;
    
    TxMsg[0] = M_FT_NA;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = SPONT;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    //这里没有信息体地址
    
    TxMsg[6] = frevent.yxnum;   //遥信个数
    TxMsg[7] = 1;               //遥信类型 单点 
    
    FramePos = 8;
    AbsTimeConvTo(&frevent.ActTime, (void*)&time, IEC101CLOCKTIME);
    //logSysMsgNoTime("%d-%d-%d %d",time.Year, time.Month, time.Day, time.Hour);
    
    for(i=0; i<frevent.yxnum; i++)
    {
        no = L_GetLogicBISendNo(DevList[NvaActDevNo].DevID, frevent.YxInfo[i]);
        no += LBI;
        TxMsg[FramePos++] = LOBYTE(no);
        TxMsg[FramePos++] = HIBYTE(no);
        //TxMsg[FramePos++] = 0;
        
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
    
    TxMsg[FramePos++] = frevent.ycnum;   //遥信个数
    aitype = Sec101Pad.TypeID[8];
    TxMsg[FramePos++] = aitype; 
    
    for(i=0; i<frevent.ycnum; i++)
    {
        no = L_GetLogicAISendNo(DevList[NvaActDevNo].DevID, frevent.YcInfo[i]);
        no += LAI;
        TxMsg[FramePos++] = LOBYTE(no);
        TxMsg[FramePos++] = HIBYTE(no);
        //TxMsg[FramePos++] = 0;
        
        switch(aitype)
        {
         
         case M_ME_NC: //短浮点数
            //temp = (float)frevent.actvalue[i];
            temp = SL_ReadAI_S(DevList[NvaActDevNo].DevID, no-LAI, frevent.actvalue[i]);
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
    
    *LengthOut = FramePos;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
    
    //EnCodeDLMsg(FramePos);
        
}
/*------------------------------------------------------------------/
函数名称：  CheckFREOnTime()
函数功能：  定时扫描故障事件上送
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::CheckFREOnTime(void)
{
    INT16U devid;
    
    if(GYKZ2015Flag == FALSE)
        return;
    
    devid=DevList[ActDevIndex].DevID;
    
    if(testGWFREvent(devid) == TRUE)
    {
        Data1.Flag |= DATA1_FT_FREVENT;
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        if(APP_DATA1_DEF & DATA1_FT_FREVENT)   
            *AppCommand|=APP_HAVEDATA1;
    }
    
}
/*------------------------------------------------------------------/
函数名称：  SetDevUseState()
函数功能：  检测通讯状态
输入说明：  
输出说明：  
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::SetDevUseState(void)
{
    
    if(DLInitpreState != DLInitFinishFlag)
    {
        DLInitpreState = DLInitFinishFlag;
        if(DLInitFinishFlag==TRUE)
        {
            ProgLogWrite2("端口%d 101从站链接建立",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 1);
        }
        else
        {
            ProgLogWrite2("端口%d 101从站链接断开",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 0);
        }
        
    }
    
  
}

void CSecAppSev::OnTimer(void)
{
    SetDevUseState();
    
    if(BalanMode && (APP_DLPriStatus == PRIENABLE))
    {
        if(RestDLDelay)
        {
            RestDLDelay--;
            if(APP_DLSecStatus == SECENABLE)
            {
                RestDLDelay = 0;
                DLInitFinishFlag = TRUE;   
            }
        }
        else
            DLInitFinishFlag = TRUE;    //只要延迟到0，链路初始化都算完成,初始化完成应用层即可向外发送数据
        
   
        if(WaitCallAllDelay)    //等待总招结束延时
        {
            WaitCallAllDelay--;
            if(FirstCallAllData)
                WaitCallAllDelay = 0;
        }
        else
            FirstCallAllData = 0xff;
        
        
    }
    
    if(APP_DLSecStatus==SECENABLE)  //只要从动方向链路好用，就可以接受复位命令
    {
        if(ResetFlag==0xff)//收到复位命令后设置标志，发出命令两秒钟后复位
        {
            ResetCount++;
            if(ResetCount>=10)
            {
                ResetCount=0;
                ResetFlag=0;
                SystemReset(WARMRESET);
                //SystemReset(COLDRESET);
            }
        }
    }
    
    
    if(DLInitFinishFlag == FALSE)   //初始化未完成禁止启动定时任务
        return;
    
    if(FirstCallAllData != 0xff)    //初始化完成后，等待总招结束（若总招没有则30秒）。
        return;
    
    if((RFaultFlag==0xff)&&(!BalanMode)&&(Data1.Flag&HaveYK))
    {
        Data1.Flag&=(~HaveYK);
        ProcTaskMsg();
    }

    SendAllDataOnTime();
    CheckFREOnTime();
    RMTParaYzCheck();
    GXParaYzCheck();
    
    //变化遥测定时扫描
    ScanData2Count++;
    if(ScanData2Count>=NvaInterval)
    {
        ScanData2Count=0;
        if(CheckNVA() && (0 == LCFlag))
            Data1.Flag|=HaveNVA;
    }
}


//SecAppProc：链路接口函数
//输入参数：bufin:输入的缓冲区数据地址,从类型标识开始，
//输入参数：lengthin为应用层数据长度，
//输入参数：dlcommand为链路层到应用层间的功能码
//输出参数：bufout:输出的缓冲区数据地址，从类型标识开始，
//输出参数：lengthout为应用层数据长度，
//输出参数：appcommand为应用层到链路层的命令
void CSecAppSev::SecAppProc(INT8U*bufin,INT16U lengthin,INT16U dlcommand,
            	INT8U*bufout,INT16U* lengthout,INT16U*appcommand)
{
    BOOL HaveJob=FALSE;
    int i ;
    //INT32U rc;
    
    AppCommand=appcommand;
    DLCommand=dlcommand;
    LengthIn=lengthin;
    LengthOut=lengthout;

    RxMsg=bufin;
    TxMsg=bufout;

    /*RxTypeID=RxMsg[0];
    RxVsq=RxMsg[1];
    RxCot=RxMsg[2];
    pRxData=RxMsg+AsduHeadLength;*/
    
    

    RxPubAddr=0;
    for(i=0;i<PubAddrSize;i++)
        RxPubAddr+=(RxMsg[PubAddrLocation+i]<<(8*i));
    
    //logSysMsgNoTime("pubaddrloc=%d， PubAddr=%d, r=%d, r2=%d",PubAddrLocation,RxPubAddr,RxMsg[PubAddrLocation],RxMsg[PubAddrLocation+1]); //debug
    
    RxInfoAddr=0;
    for(i=0;i<2;i++)//InfoAddrSize如果为3，也只取前2个字节。
        RxInfoAddr+=(RxMsg[InfoAddrLocation+i]<<(8*i));

    pTxTypeID=TxMsg;
    pTxVSQ=(TxMsg+1);
    pTxInfoAddr=(TxMsg+InfoAddrLocation);
    pTxData=(TxMsg+AsduHeadLength);

    if(CotSize==2)//传送原因为2字节时，高位固定为0。
        TxMsg[CotLocation+1]=0;
    if(InfoAddrSize==3)//信息体地址为3个字节时，最高字节为0
        TxMsg[InfoAddrLocation+2]=0;

    if(DLCommand&DL_FCBOK)//非平衡模式
    {
        DLCommand&=(~DL_FCBOK);

        //FCB处理，非平衡模式，COS、SOE、FA、指针累加。
        if ((LastFrame==BI)&&(BIFrame&BIETFRAME))
            ClearFlag(LastDevIndex,BIETFLAG);
        //if ((LastFrame==BI)&&(BIFrame&FAPROCFRAME))
            //ClearFlag(LastDevIndex,FAPROCFLAG);
        if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))
            ClearFlag(LastDevIndex,BIENTFLAG);
    }

    if(LengthIn==0)//命令处理
    {
        switch(DLCommand)
        {
            case DL_LINKDISABLE://链路无效
                //MasterStatus=NOUSE;
                APP_DLPriStatus = PRIWAITSTATUS;
                DLInitFinishFlag = FALSE;
                
                Data1.Flag &= (HaveCOS|HaveSOE);   //清掉除SOE和cos标志外的其他事项
                Data2Flag=0;
                
                if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))//上次发送数据为变位YX
                {
                    Data1.Flag|=(HaveCOS);
                    Data1.COT=SPONT;
                }
                if ((LastFrame==BI)&&(BIFrame&BIETFRAME))//上次发送数据为soe
                {
                    Data1.Flag|=(HaveSOE);
                    Data1.COT=SPONT;
                }
                
                //ll 检测通讯中断，撤销遥控。为广州测试修改 2014-3-14
                if(YkStatusForTest)
                {
                    YkStatusForTest = 0;    
                    BspYkRelease();
                    logSysMsgNoTime("101通讯中断撤销遥控",0,0,0,0);
                }

                LastFrame=Polling;
                break;
            case DL_RESETDL://链路收到对方复位链路命令，判断是否有初始化结束
                HaveJob=TRUE;
                
                APP_DLSecStatus = SECENABLE;
                YkStatusForTest = 0;    //ll 为广州测试临时修改 2012-3-24
                
                RMTParaInit();
                
                if(BalanMode)//平衡模式
                {
                    FirstCallAllData = 0;
                    WaitCallAllDelay = WAIT_CALLALL_DELAY;
                }
                else
                {
                    FirstCallAllData = 0xff;    //非平衡模式不支持第1次总招不被打断
                    DLInitFinishFlag = TRUE;    //非平衡模式下，这时链路初始化认为已经完成，置位初始化结束标志
                    
                    if(InitFlag == 0)
                    {
                        if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))//上次发送数据为变位YX
                            Data1.Flag|=(HaveCOS);
                        if ((LastFrame==BI)&&(BIFrame&BIETFRAME))//上次发送数据为soe
                            Data1.Flag|=(HaveSOE);
                        LastFrame=Polling;
                    }
                    
                    Data1.Flag &= 0xf00;      //清掉除COS、SOE、INITEND、FA之外的一级数据，避免出现通讯问题。
                    Data2Flag=0;            //清掉二级数据标志
                    *LengthOut=0;
                }
                
                JudgeSendInitEnd();
                OnTimer();  //判断初始化是否完成。
                break;
            case DL_LINKENABLE://链路有效，平衡模式
                APP_DLPriStatus = PRIENABLE;
                
                JudgeSendInitEnd();
                
                OnTimer();
                ClearMsg();
                if(DLInitFinishFlag)
                {
                    ProcData1();
                    HaveJob=TRUE;
                }
                
                break;
            case DL_SCAN1S://平衡模式，
                
                HaveJob=TRUE;
                if(HaveWrongData)   //ll 增加，加速异常回答的回答速度
                {
                    ProcData1();
                }
                else if(Data1.Flag & APP_DATA1_DEF)   //ll
                    ProcData1();
                else if(Data2Flag || (Data1.Flag& APP_DATA2_DEF))
                    ProcData2();
                else
                    HaveJob=FALSE;
                break;
            case DL_CALLUDATA://取紧急数据，平衡模式
            case DL_CALLDBMSG://取消息，平衡模式
            case DL_CALLDATA1://取一级数据 非平衡模式
                HaveJob=TRUE;
                //logSysMsgNoTime("SEC 数据库事项产生%x",Data1.Flag,0,0,0);//  debug ll
                ProcData1();
                break;
            case DL_CALLDATA2://取二级数据
                HaveJob=TRUE;
                ProcData2();
                break;
            case DL_APPCON://平衡模式收到对03的确认,COS,SOE,FA,指针累加。
                HaveJob=TRUE;
                if ((LastFrame==BI)&&(BIFrame&BIETFRAME))
                    ClearFlag(LastDevIndex,BIETFLAG);
                //if ((LastFrame==BI)&&(BIFrame&FAPROCFRAME))
                    //ClearFlag(LastDevIndex,FAPROCFLAG);
                if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))
                    ClearFlag(LastDevIndex,BIENTFLAG);
                
                //logSysMsgNoTime("确认帧查询数据开始%x",Data1.Flag,0,0,0);   //  debug ll
                if(Data1.Flag & APP_DATA1_DEF)   //ll
                    ProcData1();
                else if(Data2Flag || (Data1.Flag& APP_DATA2_DEF))
                    ProcData2();
                else  
                    HaveJob=FALSE;
                //logSysMsgNoTime("确认帧查询数据结束%x",Data1.Flag,0,0,0);   //  debug ll
                break;
            default:
                HaveJob=FALSE;
                break;
        }
    }
    else//数据处理
    {
        //logSysMsgNoTime("应用层数据Cot=%d, PubAddr=%x, TypeID=%x, len=%d",RxCot,RxPubAddr,RxTypeID,LengthIn); 
        RxTypeID=RxMsg[0];
        RxCot=RxMsg[2];
        pRxData=RxMsg+AsduHeadLength;
        RxVsq=RxMsg[1];
    
        if((RxCot&COT_REASON)>47)//传送原因有错误，发链路确认，记录错误数据
        {
            HaveWrongData=TRUE;
            WrongDataLength=LengthIn;

            RxMsg[CotLocation]=UNKNOWNCOT;
            RxMsg[CotLocation]|=0x40;

            memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

            *LengthOut=0;
            *AppCommand=APP_APPCON;
            *AppCommand|=APP_HAVEDATA1;
            return;
        }
        if(!GetActDevIndexByAddr(RxPubAddr))//公共体地址错误，可能为0xFF
        {
            //logSysMsgNoTime("公共体地址错误 =%d",RxPubAddr,0,0,0); //debug
            if((RxTypeID!=C_IC_NA)&&(RxTypeID!=C_CI_NA)&&(RxTypeID!=C_CS_NA))//总召唤或电度召唤可以为0xFF
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNPUBADDR;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }
        }

        HaveJob=TRUE;
        switch(RxTypeID)
        {
            case C_SC_NA://单点遥控命令
            case C_DC_NA://双点遥控命令
            case C_RC_NA://升降命令
                ProcControl();
                break;
            case C_SE_NA://设定命令，只支持归一化，为了测试
                  ProcSetNVA();
                break;
            case C_LC_CALL_YC_YX:          //控制方向液晶召唤遥测遥信数据
            case C_LC_CALL_SYSINFO_SOE:    //控制方向液晶召唤系统信息及SOE
            case C_LC_CALL_NAME_VER_CLOCK: //控制方向液晶召唤遥信名称、遥测名称、版本信息、时钟信息
            //case C_LC_FAULT_RESET:         //液晶复位故障命令
            case C_LC_SUMMON_PARA:
                
                LCFlag = 1;
                //LC_Flag = 1;
                LCAmount = 0;
                
                if(!(RxVsq & 0x80))
                {
                	for(i=0;i<2;i++)//InfoAddrSize如果为3，也只取前2个字节。
                    {
                    	LCAmount+=(RxMsg[7+i]<<(8*i));
                    }
                	LCInforAddr = RxInfoAddr;
                	Qoi = pRxData[2];
                }
            	else
                {
                	int num = RxVsq & 0x0F;
                	int i,j;
                	
                	Qoi = pRxData[2 * num];     //未修改num前查找Qoi
                    
                    num = (num > 9 ? 9 : num);
                	LCAmount = num;
                	for(i = 0;i < 9;i++)
                    {
                    	Addresses_Discrete[i] = 0;
                    }
                    
                	for(i = 0;i < num;i++)
                    {
                    	for(j = 0;j < 2;j++)
                        {
                        	Addresses_Discrete[i] += (pRxData[i * 2 + j]<<(8 * j));
                        }
                    }      
                }
                StoreRxTypeID = RxTypeID;
                
                ProcLCdataCall();
                break;
            case C_IC_NA://总召唤或分组召唤
                ProcAllDataCall();
                break;
            case C_CI_NA://电度总召唤或分组召唤
            //	if((RxMsg[AsduHeadLength]&0xc0)==0)//wjr  如果电度没有先进行冻结就召唤认为错误
                //if(((RxMsg[AsduHeadLength]&0xc0)==0)&&((Sec101Pad.control&CON_101GYKZ)==0))  //AJ++170829
                if(((RxMsg[AsduHeadLength]&0xc0)==0)&&(GYKZ2015Flag == FALSE))
                {
                    if(DDFreeze==FALSE)
                    {
                    	HaveJob=TRUE;
                        HaveWrongData=TRUE;
                        WrongDataLength=LengthIn;
                        RxMsg[CotLocation]=UNKNOWNTYPEID;
                        RxMsg[CotLocation]|=0x40;
                        memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
        
                        *LengthOut=0;
                        *AppCommand=APP_APPCON;
                        *AppCommand|=APP_HAVEDATA1;
                        //logSysMsgNoTime("SEC 电度未冻结%d",*AppCommand,0,0,0);//  debug ll
                        break;
                    }
                    
                }
                ProcDDCall();
                break;
            case C_CD_NA://延时获得命令
                ProcTimeDelay();
                break;
            case C_CS_NA://对钟命令
                
                /*对钟修改，如果链路命令为04不需要确认则不进行确认，如果为03则进行确认  wjr*/
                if(DLCommand==DL_SENDNOCON)//   
                {
                    ProcClock(FALSE);
                    HaveJob=FALSE;
                }
                else
                    ProcClock(TRUE);
                break;
             case C_LC_PANEL_DRIVER:
                 
             	ProcLCPanelDriver(pRxData[0],pRxData[1]);
             	if(55 == pRxData[1])
                 {    
                    ProcSummonLightStatus();    
                 }
             	if(54 == pRxData[1])
                 {    
                    ProcSummonInfoOnBoot();    
                 }
             	break;
             case C_LC_SET_PARA://液晶设定参数命令
                
                
                if(DLCommand==DL_SENDNOCON)//   
                {
                    ProcLCSetPara(FALSE);
                    HaveJob=FALSE;
                }
                else
                    ProcLCSetPara(TRUE);
                break;
              case C_LC_ACTIVATE_PARA://液晶激活参数命令
                
                
                if(DLCommand==DL_SENDNOCON)//   
                {
                    SystemReset(WARMRESET);
                    HaveJob=FALSE;
                }
                else
                    ProcActivatePara(TRUE);
                break;
            case C_RP_NA://复位进程命令
                ProcReset();
                break;
            case C_RD_NA://读数据命令
                //EnCodeReadData();
                ProcReadData();
                break;
            case C_TS_NA://测试命令
                ProcTest();
                break;
            
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
                break;*/
            //2016 新规约扩展    
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
            //end 2016 新规约扩展
            case F_FS_NA_N:    //文件同步  
                ProcXSFileSyn();
                break;
                            
            #ifdef SFILETRANAPP101
            
            case SFTYPESELECTFILE:
                SFileSelectProc(RxMsg);
                *LengthOut = 0;
                *AppCommand = APP_APPCON;
                Data2Flag |= NewFiletran;
                break;
            
            case F_SC_NA:
                SFileCallProc(RxMsg);
                *LengthOut = 0;
                *AppCommand = APP_APPCON;
                Data2Flag |= NewFiletran;
                break;
            
            case F_AF_NA:
                SFileConfirm(RxMsg);
                *LengthOut = 0;
                *AppCommand = APP_APPCON;
                Data2Flag |= NewFiletran;
                break;
            
            case SFTYPECALLYC:
                SCallYCProc(RxMsg);
                *LengthOut = 0;
                *AppCommand = APP_APPCON;
                Data2Flag |= CallYCProc;
                break;
            
            #else
            
            case F_SC_NA:
                ProcFileCall();
                break;
            case F_AF_NA:
                SConfirm();
                break;
                
            #endif
            
            case E_SGC_F0:
                if(Sec101Pad.control & CON_OLD1120ENCRPTY)
                {
                    ProDealF0();
                }
                break;
            case E_SGC_F1:
                if(Sec101Pad.control & CON_OLD1120ENCRPTY)
                {
                    ProDealF1();
                }
                break;           
            case E_SGC_F2:
                if(Sec101Pad.control & CON_OLD1120ENCRPTY)
                {
                    ProDealF2();
                }

                break;
            case E_SGC_F3:
                if(Sec101Pad.control & CON_OLD1120ENCRPTY)
                {
                    ProDealF3();
                }
                break;   
            case E_SGC_F4:
                if(Sec101Pad.control & CON_OLD1120ENCRPTY)
                {
                    ProDealF4();
                }
                break;                  
            case E_SGC_F5:
                if(Sec101Pad.control & CON_OLD1120ENCRPTY)
                {
                    ProDealF5();
                }
                break;   
            case E_SGC_F6:
                if(Sec101Pad.control & CON_OLD1120ENCRPTY)
                {
                    ProDealF6();
                }
                break;  
                
                
#ifdef INCLUDE_DA
            case C_PF_NA://保护定值设定
                SetProtect();
                break;
            case P_PF_NA://保护定值召唤
                CallProtect();
                break;
#endif

            default://类型标识有错误或不支持
                HaveJob=TRUE;
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNTYPEID;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                break;
        }
    }//end of else//数据处理

    if((!HaveJob))
    {
        *LengthOut=0;
        *AppCommand=APP_NOJOB;
    }
}
void CSecAppSev::ClearMsg(void)
{
    INT32U rc;
    
    while(1)
    {
        rc = ReceiveMSG(MySelf.QID,(void*)pMsg,MAXMSGLENGTH,ALWAYSRETURN);
        if(rc == ERR_NOMSG)
        {
            break;
        }
    
    }
    
}
//一级数据处理：优先级安排基本按照标准要求
void CSecAppSev::ProcData1(void)
{
    BOOL rc;
    
    if(DLInitFinishFlag == FALSE)   //初始化未完成禁止应用层启动
        return;
             
    if(HaveWrongData)
    {

        HaveWrongData=FALSE;
        memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
        *LengthOut=WrongDataLength;
        *AppCommand=APP_SENDDATA;
        if(BalanMode)                   //ll
            *AppCommand=APP_SENDCON;
        //如果有后续一级数据，继续将ACD设置为1
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        //logSysMsgNoTime("发送错误命令%d",WrongDataLength,0,0,0);   //  debug ll
        return;
    }
    

    if((Data1.Flag&HaveInitEnd) && (APP_DATA1_DEF&HaveInitEnd))
    {
        Data1.Flag&=(~HaveInitEnd);
        InitFlag=0;
        ClearMsg();
        EnCodeInitEnd();
        
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    //从动方向重新建立链路后第1次总招，不应被打断，ll 添加
    if((Data1.Flag&CallAllData) 
        && (APP_DATA1_DEF&CallAllData) 
        && (FirstCallAllData==0))//站召唤
    {
        //logSysMsgNoTime("SEC总招响应,FLAG=%x",FirstCallAllData,0,0,0);    //dubug ll
        if(EditAllDataCon==0xff)
        {
            EditAllDataCon=0;
            EnCodeAllDataConf();    //发送总招确认
            
        }
        else
        {
            ProcAllData();
        }

        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    if((FirstCallAllData == 0)&&(BalanMode))    //在这里刚开始建立链接第1次总招不能被打断的处理时，应该在平衡模式下，非平衡模式不适合。
    {
        return; //等待第1次总招结束，再发送其他数据
    
    }
    
    if((Data1.Flag&HaveYK) && (APP_DATA1_DEF&HaveYK))
    {
        //logSysMsgNoTime("SEC 遥控报文发送",0,0,0,0);    //dubug ll
        Data1.Flag&=(~HaveYK);
        if(YKStatus==YKTERM)//遥控结束
        {
            YKStatus=NOTUSE;
            EditYKTerm();
        }
        else
            ProcTaskMsg();

        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    
    if((Data1.Flag&HaveCOS) && (APP_DATA1_DEF&HaveCOS))
    {
        //logSysMsgNoTime("SEC procdata1 COS处理函数",0,0,0,0);    //dubug ll
        if(bSendAllDBI)
            rc = EnCodeBIENT_ALLDBI();
        else
            rc = EnCodeBIENT(); 
        if(rc)
        {
            if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        }
        else
        {
            Data1.Flag&=(~HaveCOS);
        }
        
        if(*LengthOut != 0)
            return;
    }
    
    if((Data1.Flag&CallTimeDelay) && (APP_DATA1_DEF&CallTimeDelay))
    {
        Data1.Flag&=(~CallTimeDelay);
        EnCodeTimeDelay();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    if((Data1.Flag&HaveSOE) && (APP_DATA1_DEF&HaveSOE))
    {
        
        if(bSendAllDBI)
            rc = EnCodeSOE_ALLDBI();
        else
            rc = EnCodeSOE(); 
        if(rc)
        {
            if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        }
        else
        {
            Data1.Flag &= (~HaveSOE);
            if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;     //检测去掉SOE后，是否还有一级数据
        }
        
        if(*LengthOut != 0)
            return;
        
        
    }
    if((Data1.Flag&CallAllData) && (APP_DATA1_DEF&CallAllData))//站召唤
    {
        if(EditAllDataCon==0xff)
        {
            EditAllDataCon=0;
            EnCodeAllDataConf();
        }
        else
        {
            ProcAllData();
        }

        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    if((Data1.Flag & DATA1_FT_FREVENT) && (APP_DATA1_DEF&DATA1_FT_FREVENT))
    {
        Data1.Flag &= (~DATA1_FT_FREVENT);
        
        EnCodeFREvent();
        
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
        
    }
    
    
#ifdef INCLUDE_DA
    if(Data1.Flag&ProtectCon)//保护定值镜像确认
    {
        Data1.Flag&=(~ProtectCon);
        SendProtectCon();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
#endif
    if((Data1.Flag&CallClock) && (APP_DATA1_DEF&CallClock))
    {
        Data1.Flag&=(~CallClock);
        EnCodeClock();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    

    if((Data1.Flag&CallReadData) && (APP_DATA1_DEF&CallReadData))
    {
        Data1.Flag&=(~CallReadData);
        EnCodeReadData();
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    if((Data1.Flag&CallSetNVA) && (APP_DATA1_DEF&CallSetNVA))//设定命令，只需归一化
    {
        Data1.Flag&=(~CallSetNVA);
        EnCodeSetNVA();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    if((Data1.Flag&CallTest) && (APP_DATA1_DEF&CallTest)) //测试过程
    {
        Data1.Flag&=(~CallTest);
        EnCodeTest();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    if((Data1.Flag&CallReset) && (APP_DATA1_DEF&CallReset))//复位进程
    {
        Data1.Flag&=(~CallReset);
        EnCodeReset();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    if((Data1.Flag&CallParaSet) && (APP_DATA1_DEF&CallParaSet))//参数设定
    {
        Data1.Flag&=(~CallParaSet);
        ParaSetCon();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    

    if((Data1.Flag&FreezeDD) && (APP_DATA1_DEF&FreezeDD))//冻结电度
    {
        Data1.Flag&=(~FreezeDD);
        if(EditDDCon==0xFF)
        {
            EditDDCon=0;
            EnCodeCounterConf();
        }
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    if((Data1.Flag&CallDD) && (APP_DATA1_DEF&CallDD))//传送电度
    {
        if(EditDDCon==0xFF)
        {
            EditDDCon=0;
            EnCodeCounterConf();
        }
        else
        {
            ProcCounter();
        }
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    if((Data1.Flag&CallLightStatus) && (APP_DATA1_DEF&CallLightStatus))
    {
        Data1.Flag&=(~CallLightStatus);
        EnCodeLightStatus(ActDevIndex);
        //*AppCommand = APP_RESETMACHINE;
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    if((Data1.Flag&CallLCdata) && (APP_DATA1_DEF&CallLCdata))
    {
        Data1.Flag&=(~CallLCdata);
        EnCodeLC();
        return;
    }
    if((Data1.Flag&LCSetPara) && (APP_DATA1_DEF&LCSetPara))
    {
        Data1.Flag&=(~LCSetPara);
        EnCodeParaMirror(ActDevIndex,totallength_m);
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    if((Data1.Flag&LCActivatePara) && (APP_DATA1_DEF&LCActivatePara))
    {
        Data1.Flag&=(~LCActivatePara);
        EnCodeActivatePara(ActDevIndex);
        //*AppCommand = APP_RESETMACHINE;
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    if((Data1.Flag&SummonInfoOnBoot) && (APP_DATA1_DEF&SummonInfoOnBoot))
    {
        Data1.Flag&=(~SummonInfoOnBoot);
        EnCodeSummonInfoOnBoot(ActDevIndex);
        //*AppCommand = APP_RESETMACHINE;
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    //变化遥测，如果数据变化频繁会影响其他数据传输，所以放在后面。
    if((Data1.Flag&HaveNVA) && (APP_DATA1_DEF&HaveNVA))
    {
        if(EnCodeNVA())
        {
            //if(Data1.Flag!=0)
            if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
            return;
        }
        else
            Data1.Flag&=(~HaveNVA);
    }
    
    //无一级数据
    *LengthOut=0;
    *AppCommand=APP_NODATA;
}

//二级数据处理：包括背景数据、循环周期数据
void CSecAppSev::ProcData2(void)
{
    
#ifdef SFILETRANAPP101
    BOOL    rc = FALSE;
    INT16U  len;
#endif    
    if(DLInitFinishFlag == FALSE)   //初始化未完成禁止应用层启动
        return;
    
    //时钟报文
    if((Data1.Flag&CallClock) && (APP_DATA2_DEF&CallClock))
    {
        Data1.Flag&=(~CallClock);
        EnCodeClock();
        if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    if((Data1.Flag&CallTest) && (APP_DATA2_DEF&CallTest)) //测试过程
    {
        //logSysMsgNoTime("SEC 2级数据测试应答",0,0,0,0);   //  debug ll
        Data1.Flag&=(~CallTest);
        EnCodeTest();
        if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        return;
    }
        
    
    
    if((Data1.Flag&CallReset) && (APP_DATA2_DEF&CallReset))//复位进程
    {
        
        Data1.Flag&=(~CallReset);
        EnCodeReset();
        if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        return;
    }
    //广东远程运维
    if((Data2Flag&DATA2_RMT_READPARA_GD) && (APP_DATA2_DEF&DATA2_RMT_READPARA_GD))
    {
        if(ProcEncodeRMTReadPara_gd())
        {
            if(Data1.Flag & APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;

        }
        else
        {
            Data2Flag &= (~DATA2_RMT_READPARA_GD);
        }
           
        return; 
    }
    //广东远程运维-写参数应答帧
    if((Data2Flag&DATA2_RMT_WRITEPARA_GD) && (APP_DATA2_DEF&DATA2_RMT_WRITEPARA_GD))
    {
        Data2Flag &= (~DATA2_RMT_WRITEPARA_GD);
        ProcEncodeRMTSetPara_GD();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;

        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复，因此需要return
    }
    
    //广西远程运维
    if((Data2Flag&DATA2_GX_READPARA) && (APP_DATA2_DEF&DATA2_GX_READPARA))
    {
        if(EditReadParaCon == 0xff)
        {
            EditReadParaCon=1;
            EnCodeGXReadParaConf();    //发送读参数确认
        }
        else if(EditReadParaCon==1)
        {
            if(ProcEncodeGXReadPara())
            {
                if(Data1.Flag & APP_DATA1_DEF)
                    *AppCommand|=APP_HAVEDATA1;
    
            }
            else
            {
                EditReadParaCon=0;
            }
        }
        else
        {
            EnCodeGXReadParaEnd(); 
            Data2Flag &= (~DATA2_GX_READPARA);
        }
           
        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复，因此需要return
    }
    
    if((Data2Flag&DATA2_GX_SETPARA) && (APP_DATA2_DEF&DATA2_GX_SETPARA))
    {
        ProcEncodeGXSetPara();
        
        if(Data1.Flag & APP_DATA1_DEF)
        {
            *AppCommand|=APP_HAVEDATA1;
        }
 
        Data2Flag &= (~DATA2_GX_SETPARA);    
        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复，因此需要return
    }
    
    if((Data2Flag&DATA2_GX_ACTIVATEPARA) && (APP_DATA2_DEF&DATA2_GX_ACTIVATEPARA))
    {
        if(ActiveParaCon == 1)
        {
            ProcEncodeGXActivatePara();
            ActiveParaCon = 2;
        }
        else if(ActiveParaCon == 2)
        {
            ProcEncodeGXChangePara();
            ActiveParaCon = 0;
            Data2Flag &= (~DATA2_GX_ACTIVATEPARA);

        }
        else if(ActiveParaCon == 3)
        {
            if(ProcEncodeGXSendPara() == FALSE)
            {
                ActiveParaCon = 0;
                Data2Flag &= (~DATA2_GX_ACTIVATEPARA);
            }
        }
        
        if(Data1.Flag & APP_DATA1_DEF)
        {
            *AppCommand|=APP_HAVEDATA1;
        }
 
            
        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复，因此需要return
    }
    //读文件目录
    if((Data2Flag&DATA2_FT_DIR) && (APP_DATA2_DEF&DATA2_FT_DIR))//读文件目录
    {
        if(ProcFT_EncodeReadDir())
        {
            if(Data1.Flag & APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
                
            //return;
        }
        else
            Data2Flag &= (~DATA2_FT_DIR);
        
        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复
        
    }
    //读文件激活确认帧
    if((Data2Flag&DATA2_FT_FILEACT) && (APP_DATA2_DEF&DATA2_FT_FILEACT))//读文件目录
    {
        Data2Flag &= (~DATA2_FT_FILEACT);
        ProcFT_EncodeFileActConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    //读文件内容
    if((Data2Flag&DATA2_FT_FILEDATA) && (APP_DATA2_DEF&DATA2_FT_FILEDATA))//读文件目录
    {
        if(ProcFT_EncodeFileData())
        {
            if(Data1.Flag & APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
                
            //return;
        }
        else
            Data2Flag &= (~DATA2_FT_FILEDATA);
            
        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复    
    }
    
    //写文件激活确认帧
    if((Data2Flag&DATA2_FT_WTFILEACT) && (APP_DATA2_DEF&DATA2_FT_WTFILEACT))//读文件目录
    {
        Data2Flag &= (~DATA2_FT_WTFILEACT);
        ProcFT_EncodeWriteFileActConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    
    //文件传输数据结束确认帧
    if((Data2Flag&DATA2_FT_WTDATAACT) && (APP_DATA2_DEF&DATA2_FT_WTDATAACT))//读文件目录
    {
        Data2Flag &= (~DATA2_FT_WTDATAACT);
        ProcFT_EncodeWriteFileDataConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    
    //切换定值区确认帧
    if((Data2Flag&DATA2_RMT_SETSEC) && (APP_DATA2_DEF&DATA2_RMT_SETSEC))//读文件目录
    {
        Data2Flag &= (~DATA2_RMT_SETSEC);
        ProcEncodeSetSectionNo();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    
    //读定值区确认帧
    if((Data2Flag&DATA2_RMT_READSEC) && (APP_DATA2_DEF&DATA2_RMT_READSEC))//读文件目录
    {
        Data2Flag &= (~DATA2_RMT_READSEC);
        ProcEnCodeReadSectionNo();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    //读参数应答帧
    if((Data2Flag&DATA2_RMT_READPARA) && (APP_DATA2_DEF&DATA2_RMT_READPARA))//读文件目录
    {
        
        if(ProcEncodeRMTReadPara())
        {
            if(Data1.Flag & APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;

        }
        else
        {
            Data2Flag &= (~DATA2_RMT_READPARA);
        }
           
        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复，因此需要return
    }
    
    //写参数应答帧
    if((Data2Flag&DATA2_RMT_SETPARA) && (APP_DATA2_DEF&DATA2_RMT_SETPARA))
    {
        Data2Flag &= (~DATA2_RMT_SETPARA);
        ProcEncodeRMTSetPara();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;

        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复，因此需要return
    }
    
    
    //远程升级应答帧
    if((Data2Flag&DATA2_PUP_PROGUP) && (APP_DATA2_DEF&DATA2_PUP_PROGUP))//读文件目录
    {
        Data2Flag &= (~DATA2_PUP_PROGUP);
        ProcEncodePUPupdateConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;

        return;     //这里返回处理跟前面不一样，调试时注意。无论false还是true都有报文需要回复，因此需要return
    }
    
    //线损文件同步激活确认桢   CL  20180607
    if((Data2Flag&DATA2_XSFT_SYNACT) && (APP_DATA2_DEF&DATA2_XSFT_SYNACT))
    {
        Data2Flag &= (~DATA2_XSFT_SYNACT);
        ProcEncodeXSFileSynConf();   //线损文件同步激活确认应答桢
        
        if(Data1.Flag & APP_DATA1_DEF)   //优先进行一级数据的访问
            *AppCommand|=APP_HAVEDATA1;

        return;     
    }
   //线损文件同步激活确认桢   CL  20180607
   if((Data2Flag&DATA2_XSFT_SYNACTFINISH) && (APP_DATA2_DEF&DATA2_XSFT_SYNACTFINISH))
    {
        Data2Flag &= (~DATA2_XSFT_SYNACTFINISH);
        ProcEncodeXSFileSynFinish();   //线损文件同步终止应答桢
        
        if(Data1.Flag & APP_DATA1_DEF)   //优先进行一级数据的访问
            *AppCommand|=APP_HAVEDATA1;

        return;    
    }
   //线损文件同步激活停止确认桢 CL 20180612   
    
    //变化遥测，如果数据变化频繁会影响其他数据传输，所以放在后面。
    if((Data1.Flag&HaveNVA) && (APP_DATA2_DEF&HaveNVA))
    {
        if(EnCodeNVA())
        {
            if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
            return;
        }
        else
            Data1.Flag&=(~HaveNVA);
    }
    
#ifdef SFILETRANAPP101    
    if(Data2Flag & NewFiletran)
    {
        Data2Flag &= (~NewFiletran);
        
        rc = SFileGetRcInfo(TxMsg, &len);
        if (rc)
        {
            Data2Flag |= NewFiletran;
        }
        
        if (len)
        {
            *LengthOut = len;
            *AppCommand = APP_SENDDATA;
            if(BalanMode)
            {
                *AppCommand = APP_SENDCON;
            }
            return ;
        }

    }
    
    if (Data2Flag & CallYCProc)
    {
        Data2Flag &= (~CallYCProc);
        
        rc = SCallYcGetRcInfo(TxMsg, &len);
        if (rc)
        {
            Data2Flag |= CallYCProc;
        }
        
        if (len)
        {
            *LengthOut = len;
            *AppCommand = APP_SENDDATA;
            if(BalanMode)
            {
                *AppCommand = APP_SENDCON;
            }
            return ;
        }
    }
    
#endif

#ifdef INCLUDE_DA
    if(Data2Flag&ProtectData)//保护定值数据
    {
        Data2Flag&=(~ProtectData);
        SendProtectData();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
#endif

    if((Data2Flag&BackData)||(Data2Flag&PerCycData))//该标志在定时器中设置，发送完数据后清掉
    {
        if(ProcAllData())
        {
            //if(Data1.Flag!=0)
            if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
            return;
        }
    }

    if(Data2Flag&UpLoadFile)//文件操作，按照标准要求应比周期数据级别还低。
    {
        //处理函数
        switch(FileStep)
        {
            case PCallDir:
                SendDir();
                break;
            case PSelectFile:
                FileReady();
                break;
            case PCallFile:
                SectionReady();
                break;
            case PCallSection:
                SendSegment();
                break;
            case PLastSegment:
                SendLastSegment();
                break;
            case PLastSection:
                SendLastSection();
                break;
            default:
                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                *LengthOut=0;
                *AppCommand=APP_NODATA;
                break;
        }
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    //召唤二级数据时，无二级数据可能以一级数据应答。都没有以无数据应答。
    ProcData1();
}

//OK
void CSecAppSev::ProcControl(void)  //处理遥控，遥控不允许或公共体地址错，不发消息。
{
    INT8U sco,dco,OnOff;
    INT16U InfoAddr,SwitchNo;
    //INT16U bonum;
    
    InfoAddr=RxInfoAddr;
    SwitchNo = InfoAddr-LBO+1;
    SwitchNoTemp = SwitchNo;
    YKTypeID=RxTypeID;
    
    //LogYkInfoRec(DevList[ActDevIndex].DevID, RxTypeID, *pRxData, RxInfoAddr, RxCot);  //记录主站发送的所有遥控信息
    
    
    if (GetActDevIndexByAddr(RxPubAddr))//根据公共体地址查设备序号。
    {
        BODevIndex=ActDevIndex;
        if((SwitchNoTemp-1)*2==DevList[ActDevIndex].DevData.BONum)                     //蓄电池维护
        
    
        /*if(MyConfig.type ==DEVTYPE_DF9311A1)
            bonum = 8;
        else
            bonum = DevList[BODevIndex].DevData.BONum;
        if((SwitchNoTemp-1)*2==bonum)*/
        {
            
            YKStatus=YKSETCON;
            Data1.Flag|=HaveYK;
            *LengthOut=0;
            *AppCommand=APP_APPCON;
            *AppCommand|=APP_HAVEDATA1;
            DcoTemp=*pRxData;
            ScoTemp=*pRxData;
            if(((*pRxData)&DCO_SE)==0)  //执行
            {
                if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                    startCellMaint();
            }
            return;
        }
        else if((SwitchNoTemp-1)*2==DevList[ActDevIndex].DevData.BONum+2)    //复归级联设备
        {
            YKStatus=YKSETCON;
            Data1.Flag|=HaveYK;
            *LengthOut=0;
            *AppCommand=APP_APPCON;
            *AppCommand|=APP_HAVEDATA1;
            DcoTemp=*pRxData;
            ScoTemp=*pRxData;
            if(((*pRxData)&DCO_SE)==0)  //执行
            {
                if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                { 
                    ResetFaultInfoForCall_FaultCheck();
                    logSysMsgNoTime("复归级联设备",0,0,0,0);
                }
            }
              return;
        }
    }
    switch (RxTypeID)
    {
        case C_SC_NA:    //单点遥控命令
            ScoTemp = sco = *pRxData;

            if ((sco & SCO_SCS) == 0)       //分
                OnOff = 2;
            else if ((sco & SCO_SCS) == 1)  //合
                OnOff = 1;

            if(Sec101Pad.ControlPermit == 0)//参数设置为不允许遥控
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNTYPEID;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }
            if (GetActDevIndexByAddr(RxPubAddr))//根据公共体地址查设备序号。
            {
                BODevIndex=ActDevIndex;
            }
            else
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNPUBADDR;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }
            if((InfoAddr<LBO)||(InfoAddr>LBO+DevList[BODevIndex].DevData.BONum/2))
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNTINFOADDR;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }
            if ((RxCot&COT_REASON)==ACT)//6，激活
            {
                if ((sco & SCO_SE) == SCO_SE)   //1，select
                {
                    
                    YKStatus=YKSETCON;
                    if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        SetYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                }
                else//0，执行
                {
                    if(YkStatusForTest ==0) //ll 为广州测试临时修改 2012-3-24
                    {
                        return;
                    }
                    YkStatusForTest =  0;    //ll 为广州测试临时修改 2012-3-24   
                    
                    YKStatus=YKEXECON;
                    if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        ExecuteYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    
                }
            }
            else//撤消
            {
                if ((RxCot&COT_REASON)==DEACT)
                {
                	YKStatus=YKCANCELCON;
                	if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        CancelYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                }
                else
                {
                	HaveWrongData=TRUE;
                    WrongDataLength=LengthIn;
                    RxMsg[CotLocation]=UNKNOWNCOT;
                    RxMsg[CotLocation]|=0x40;
                    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
                    *LengthOut=0;
                    *AppCommand=APP_APPCON;
                    *AppCommand|=APP_HAVEDATA1;
                    return;
                }
            }

            break;
        case C_DC_NA:   //双点遥控命令
        case C_RC_NA:    //步调节
            dco = *pRxData;
            DcoTemp = dco;

            if ((dco&DCO_DCS)==1)        //分
                OnOff = 2;
            else if ((dco&DCO_DCS)==2)  //合
                OnOff = 1;
            else
            {
            	HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNTYPEID;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }

            if(Sec101Pad.ControlPermit == 0)
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=RxCot;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }
            if (GetActDevIndexByAddr(RxPubAddr))//根据公共体地址查设备序号。
            {
                BODevIndex=ActDevIndex;
            }
            else
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNPUBADDR;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }
            if((InfoAddr<LBO)||(InfoAddr>LBO+(DevList[BODevIndex].DevData.BONum/2)))
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNTINFOADDR;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;
            }
            if ((RxCot&COT_REASON)==ACT)//6，激活
            {
                if ((dco&DCO_SE) == DCO_SE)   //1，select
                {
                    
                    YKStatus=YKSETCON;
                    if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        SetYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    if((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2))//北京故障复归，最后一个遥控的合
                    {
                        YKSetAlready = TRUE;
                    }
                }
                else//0，执行
                {
                    
                    if(YkStatusForTest ==0)    //ll 为广州测试临时修改 2012-3-24 
                    {
                        
                        HaveWrongData=TRUE;
                        WrongDataLength=LengthIn;
                        RxMsg[CotLocation]=UNKNOWNTINFOADDR;
                        RxMsg[CotLocation]|=0x40;
                        memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
        
                        *LengthOut=0;
                        *AppCommand=APP_APPCON;
                        *AppCommand|=APP_HAVEDATA1;
                        return;
                    }
                    YkStatusForTest =  0;    //ll 为广州测试临时修改 2012-3-24 
                    
                    YKStatus=YKEXECON;
                    if((YKSetAlready == FALSE)&&((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2)))//北京故障复归，最后一个遥控的合
                    {
                        RFaultFlag=0xff;
                        SetYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    }
                    else
                    {
                        if(YKSetAlready == TRUE)
                            YKSetAlready = FALSE;
                        if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                            ExecuteYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    }
                }
            }
            else//撤消
            {
                if ((RxCot&COT_REASON)==DEACT)
                {
                    YKStatus=YKCANCELCON;
                    if((RxCot&COT_TEST)==0)//正常命令，（测试位为1则不能实际操作遥控）
                        CancelYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                }
                else
                {
                	HaveWrongData=TRUE;
                    WrongDataLength=LengthIn;
                    RxMsg[CotLocation]=UNKNOWNCOT;
                    RxMsg[CotLocation]|=0x40;
                    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
                    *LengthOut=0;
                    *AppCommand=APP_APPCON;
                    *AppCommand|=APP_HAVEDATA1;
                    return;
                }
            }

            break;

        case C_SE_NA:
            break;
    }
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
}

void CSecAppSev::ProcSetNVA(void)
{
    INT16U InfoAddr;
    InfoAddr=RxInfoAddr;
    if((InfoAddr<LSET)||(InfoAddr>HSET))
    {
        HaveWrongData=TRUE;
        WrongDataLength=LengthIn;
        RxMsg[CotLocation]=UNKNOWNTINFOADDR;
        RxMsg[CotLocation]|=0x40;
        memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

        *LengthOut=0;
        *AppCommand=APP_APPCON;
        *AppCommand|=APP_HAVEDATA1;
        return;
    }

    SetPubAddr=RxPubAddr;
    SetInfoAddr=InfoAddr;
    SetNvaWord=MAKEWORD(pRxData[0],pRxData[1]);
    SetQOS=RxMsg[AsduHeadLength+2];

    Data1.Flag|=CallSetNVA;
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
}
void CSecAppSev::ProcLCdataCall(void)
{
    Data1.Flag|=CallLCdata;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
    return; 
}
//OK
void CSecAppSev::ProcAllDataCall(void)
{
    Data1.Flag|=CallAllData;
    EditAllDataCon=0xff;

    Data2Flag&=(~BackData);//Data2Flag=0;//清掉二级数据标志

    GroupTrn.TypeID=C_IC_NA;
    GroupTrn.COT=RxMsg[AsduHeadLength];//总召唤限定词 传送原因,
    GroupTrn.PubAddr=RxPubAddr;
    GroupTrn.HaveSendDBI = FALSE;

    if (GroupTrn.PubAddr==BroadCastAddr)//广播，召唤所有设备数据
    {
        GroupTrn.DevIndex=0;//总召唤数据从设备序号0开始
    }
    else//召唤某一设备数据
    {
        if (GetActDevIndexByAddr(GroupTrn.PubAddr))//根据公共体地址查设备序号。
        {
            GroupTrn.DevIndex=ActDevIndex;
        }
        else//没有对应该公共体地址的设备
        {
            
            GroupTrn.DevIndex=0;//总召唤数据从设备序号0开始
        }
    }
    
    if(GroupTrn.COT == INTRO16)    //组16 单独处理 广西修改
    {
        logSysMsgWithTime("101收到组16命令",0,0,0,0);
        GroupTrn.TypeID = C_IC_NA;
        GroupTrn.COT    =INTRO16;
        GroupTrn.PubAddr=RxPubAddr;
        GroupTrn.GroupNo=16;
        GroupTrn.InfoAddr=LBIinfoaddr;
        GroupTrn.HaveSendDBI = FALSE;
        GroupTrn.First  = TRUE;
        
    }
    else if(GroupTrn.COT==INTROGEN)
    {
        GroupTrn.GroupNo=1;//组号从1开始
        GroupTrn.InfoAddr=LBIinfoaddr;//信息体地址从0x1开始  
    }
    else
    {
        GroupTrn.GroupNo=RxMsg[AsduHeadLength]-INTROGEN;
        GroupTrn.InfoAddr=LBIinfoaddr;//信息体地址从0x1开始，该数值有校正。 
    }
    //logSysMsgNoTime("总招no=%d",GroupTrn.GroupNo,0,0,0);   //  debug ll
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
}

//OK
void CSecAppSev::ProcDDCall(void)
{
    GroupTrnDD.TypeID=C_CI_NA;
    GroupTrnDD.PubAddr=RxPubAddr;
    GroupTrnDD.Description=RxMsg[AsduHeadLength];
    
    if((RxMsg[AsduHeadLength]&0x3f)==5)//电度总召唤
    {
        GroupTrnDD.COT=REQCOGCN;
    }
    else//分组召唤
    {
        GroupTrnDD.COT=REQCOGCN+((RxMsg[AsduHeadLength])&0x3f);
    }

    if(RxMsg[AsduHeadLength]&0xc0)//冻结电度
    {
        Data1.Flag|=FreezeDD;
        EditDDCon=0xff;
        FreezeCounter();
        DDFreeze=TRUE;
    }
    else//读电度
    {
        Data1.Flag|=CallDD;
        EditDDCon=0xff;
    }

    if (GroupTrnDD.PubAddr==BroadCastAddr)//广播，召唤所有设备数据
    {
        GroupTrnDD.DevIndex=0;//总召唤数据从设备序号0开始
    }
    else//召唤某一设备数据
    {
        if (GetActDevIndexByAddr(GroupTrnDD.PubAddr))//根据公共体地址查设备序号。
        {
            GroupTrnDD.DevIndex=ActDevIndex;
        }
        else//没有对应该公共体地址的设备
        {
            GroupTrnDD.DevIndex=0;//总召唤数据从设备序号0开始
        }
    }

    if(GroupTrnDD.COT==REQCOGCN)//电度总召唤
    {
        GroupTrnDD.GroupNo=1;//组号从1开始
        GroupTrnDD.InfoAddr=LBCR;//信息体地址从开始
    }
    else
    {
        GroupTrnDD.GroupNo=GroupTrnDD.COT-REQCOGCN;
        GroupTrnDD.InfoAddr=LBCR;//信息体地址从开始，该数值有校正。
    }
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
    //logSysMsgNoTime("DDCall 组=%d, DDCon=%x,flag=%x",GroupTrnDD.GroupNo,EditDDCon,Data1.Flag,0);   //  debug ll
}
//OK
void CSecAppSev::ProcTimeDelay(void)
{
    if ((RxCot&COT_REASON)==ACT)//6，激活
    {
        if (GetSysTime((void*)(&SecSysTimeR),ABSTIME))
        {
            ReadTimeFlag=0xff;
        }
        else
            ReadTimeFlag=0;
        SDTTime=MAKEWORD(pRxData[0],pRxData[1]);

        Data1.Flag|=CallTimeDelay;//为了设置一级数据标志
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        *AppCommand|=APP_HAVEDATA1;
    }
    else if ((RxCot&COT_REASON)==SPONT)//3，设置延迟时间
    {
        TimeDelay=MAKEWORD(pRxData[0],pRxData[1]);
        *LengthOut=0;
        *AppCommand=APP_APPCON;
    }
}
void CSecAppSev::ProcSummonLightStatus()
{
	Data1.Flag|=CallLightStatus;//为了设置一级数据标志
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    if(APP_DATA1_DEF & CallLightStatus)   //ll
        *AppCommand|=APP_HAVEDATA1;
}
void CSecAppSev::ProcSummonInfoOnBoot()
{
	Data1.Flag|=SummonInfoOnBoot;//为了设置一级数据标志
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    if(APP_DATA1_DEF & SummonInfoOnBoot)   //ll
        *AppCommand|=APP_HAVEDATA1;
}
//OK
void CSecAppSev::ProcClock(BOOL Conf) //处理对钟
{
    struct Iec101ClockTime_t Time;
    struct AbsTime_t AbsTime;
    TimeRightFlag = FALSE;
    GetSysTime((void*)(&OldSysTime),ABSTIME);//记录对钟前的系统时间
    
    if((pRxData[6]==0) && (pRxData[5]==0) && (pRxData[4]==0))  //判断年月日为0
    {
        //读时钟
        Data1.Flag|=CallClock;//为了设置一级数据标志
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        if(APP_DATA1_DEF & CallClock)   
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }

    if(Sec101Pad.SetTimePermit == 1)
    {
        Time.MSecond = MAKEWORD(pRxData[0],pRxData[1]);
        Time.Minute  = (pRxData[2] & 0x3f);
        Time.Hour    = (pRxData[3] & 0x1f);
        Time.Day     = (pRxData[4] & 0x1f);
        Time.Month   = (pRxData[5] & 0xf);
        Time.Year    = (pRxData[6] & 0x7f);
        
        TimeRightFlag = IEC101TimeIsOK(&Time);
        if(TimeRightFlag)
        {
            if (RxPubAddr!=0xFF) //非广播对钟，要加延迟时间
            {
                if(Sec101Pad.UseStandClock == 1)
                    ConvToAbsTime((void*)&Time,&AbsTime,IEC101CLOCKTIME);
                else
                    ConvToAbsTime((void*)&Time,&AbsTime,IEC101EXTCLOCKTIME);
    
                if(((long)AbsTime.MSecond+TimeDelay)<60000)
                    AbsTime.MSecond+=TimeDelay;
                else
                {
                    AbsTime.MSecond=(long)AbsTime.MSecond+TimeDelay-60000;
                    AbsTime.Minute++;
                }
                //SetSysTime(&AbsTime,ABSTIME);//这个函数不好用。
                if(Sec101Pad.UseStandClock == 1)
                {
                    AbsTimeConvTo(&AbsTime,(void*)&Time,IEC101CLOCKTIME);
                    SetSysTime(&Time,IEC101CLOCKTIME);
                }
                else
                {
                    AbsTimeConvTo(&AbsTime,(void*)&Time,IEC101EXTCLOCKTIME);
                    SetSysTime(&Time,IEC101EXTCLOCKTIME);
                }
            }
            else//广播对钟
            {
                if(Sec101Pad.UseStandClock == 1)
                    SetSysTime(&Time,IEC101CLOCKTIME);
                else
                    SetSysTime(&Time,IEC101EXTCLOCKTIME);
            }
        }
    }
    if (Conf)                   //需要确认
    {
        Data1.Flag|=CallClock;//为了设置一级数据标志
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        if(APP_DATA1_DEF & CallClock)   //ll
            *AppCommand|=APP_HAVEDATA1;
    }
    else
    {
        *LengthOut=0;
        *AppCommand=APP_NOJOB;
    }
}
void CSecAppSev::ProcLCSetPara(BOOL Conf) //处理液晶设定参数报文
{
	INT16U Addr,ParaLength;
    int i;
    INT8U *pData,Bound;
	INT8U *pParaBuf;
    Bound=RxVsq&VSQ_NUM;//信息元素数目
    Addr=RxInfoAddr; //起始信息体地址
    pData=pRxData;
    paramirrorbuf[0] = *(pRxData-2);                       //paramirrorbuf保存传过来的参数，将来作为镜像传送回去。
    paramirrorbuf[1] = *(pRxData-1);
    pParaBuf = &paramirrorbuf[2];
    totallength_m = 0;
    filenum = Bound;
    for(i=0;i<Bound;i++)
    {
        ParaLength=pData[0];
        totallength_m += ParaLength + 2;
        memcpy((void*)pParaBuf,pData,ParaLength + 2); //加2是因为包含了长度和Qoi两字节
        Qoi = pData[ParaLength + 1];
        WriteParaFile(Addr,ParaLength,Qoi,&pData[1]);
        pData+=ParaLength+2;
        pParaBuf +=ParaLength+2;
        pParaBuf[0] = pData[0];
        pParaBuf[1] = pData[1];
        if(3 == InfoAddrSize)
        {
        	pParaBuf[2] = pData[2];
        }
        Addr=MAKEWORD(pData[0],pData[1]);//下一个信息元素的信息体地址，信息体地址有效位只有2个字节
        pData+=InfoAddrSize;//指向待处理的信息元素数据，如果信息体地址为3字节，越过高字节。  
        pParaBuf += InfoAddrSize;  
        totallength_m += InfoAddrSize;
    }
    if (Conf)                   //需要确认
    {
        Data1.Flag|=LCSetPara;//为了设置一级数据标志
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        if(APP_DATA1_DEF & LCSetPara)   //ll
            *AppCommand|=APP_HAVEDATA1;
    }
    else
    {
        *LengthOut=0;
        *AppCommand=APP_NOJOB;
    }
}
void CSecAppSev::ProcActivatePara(BOOL Conf) //处理液晶设定参数报文
{    
    if (Conf)                   //需要确认
    {
        Data1.Flag|=LCActivatePara;//为了设置一级数据标志
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        if(APP_DATA1_DEF & LCActivatePara)   //ll
        *AppCommand|=APP_HAVEDATA1;
    }
    else
    {
        *LengthOut=0;
        *AppCommand=APP_NOJOB;
    }
    
}
void CSecAppSev::WriteParaFile(INT16U Addr,INT16U ParaLength,INT8U Qoi,INT8U *p)
{
	int i;
	INT8U temp;
    
	switch(Qoi & 0x03)
    {
    	case 1:
    	    for(i = 0;i < ParaLength / 2;i++)
            {
            	temp = p[2*i];
            	p[2*i] = p[2*i+1];
            	p[2*i+1] = temp;    
            }
    	    WriteYcPara(p,ParaLength,Qoi);
        	break;
    	case 2:
            for(i = 0;i < ParaLength / 2;i++)
            {
            	temp = p[2*i];
            	p[2*i] = p[2*i+1];
            	p[2*i+1] = temp;    
            }
        	WriteCommunicationPara(p,ParaLength,Qoi);
        	break;
    	case CONTROLLERAPPPARA:
        	INT8U index = Addr - 0x8831;
        	for(i = 0;i < ParaLength / 2;i++)
            {
            	temp = p[2*i];
            	p[2*i] = p[2*i+1];
            	p[2*i+1] = temp;    
            }
        	WriteAdvancePara(p,index,ParaLength,Qoi);
            //WriteAdvancePara(p,index,ParaLength,(Qoi >> 2) & 0x03);
            /*switch((Qoi >> 2) & 0x03)
            {
            	case CONTROLLERPROTECTPARA:
                    
                	ParaFromASDU(p,index,ParaLength,CONTROLLERPROTECTPARA);
                	ProtectPara_LC_EachFeeder_W(index);
                	break;
            	case 1:
                	break;
            	case 2:
                    ParaFromASDU(p,index,ParaLength,2);
                	VoltageTypePara_LC_EachFeeder_W(index,2);
                	break;
            	case 3:
                    ParaFromASDU(p,index,ParaLength,3);
                	VoltageTypePara_LC_EachFeeder_W(index,3);
                	break;
            	default:
                	break;
                        
            }*/
    }    
}
void CSecAppSev::ProcReset(void)
{
    
    Data1.Flag|=CallReset;
    ResetPubAddr=RxPubAddr;
    ResetInfoAddr=RxInfoAddr;
    ResetGRP=RxMsg[AsduHeadLength];

    *LengthOut=0;
    *AppCommand=APP_APPCON;
    if(APP_DATA1_DEF & CallReset)
        *AppCommand|=APP_HAVEDATA1;
}

void CSecAppSev::ProcTest(void)
{
    Data1.Flag|=CallTest;
    TestPubAddr=RxPubAddr;
    TestInfoAddr=RxInfoAddr;
    TestFBP=MAKEWORD(pRxData[0],pRxData[1]);

    *LengthOut=0;
    //*AppCommand=APP_APPCON;
    if(APP_DATA1_DEF & CallTest)
        *AppCommand|=APP_HAVEDATA1;
    //logSysMsgNoTime("proctest，命令=%d",*AppCommand,0,0,0);   //  debug ll    
}

//OK
void CSecAppSev::ProcReadData(void)
{
    Data1.Flag|=CallReadData;
    RDPubAddr=RxPubAddr;
    RDInfoAddr=RxInfoAddr;
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
}

//OK
//参数设定说明：信息体地址从0X5001开始，每个遥测占用3个信息体地址
//每个遥测参数的顺序是：门限值、上限、下限。
//参数设定时，信息体地址/3为遥测序号，限定词确定门限、上限、下限。
void CSecAppSev::ProcParaSet(void)
{
    INT16U No;
    long temp;
    INT16U InfoAddr;
    InfoAddr=RxInfoAddr;
    if((InfoAddr<LPARA)||(InfoAddr>HPARA))
    {
        HaveWrongData=TRUE;
        WrongDataLength=LengthIn;
        RxMsg[CotLocation]=UNKNOWNTINFOADDR;
        RxMsg[CotLocation]|=0x40;
        memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

        *LengthOut=0;
        *AppCommand=APP_APPCON;
        *AppCommand|=APP_HAVEDATA1;
              
        return;
    }

    if(!GetActDevIndexByAddr(RxPubAddr))
        ActDevIndex=0;
    ParaPubAddr=RxPubAddr;
    ParaInfoAddr=InfoAddr;
    ParaTypeID=RxTypeID;

    //No=(ParaPubAddr-LPARA)/3;
    No=(ParaInfoAddr-LPARA)/3;        //wjr  2009.4.5  此处应该取信息体地址而不是公共体地址
    if(No>=DevList[ActDevIndex].DevData.AINum)
    {
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        return;
    }

    switch(RxTypeID)
    {
        case P_ME_NA:    //归一化
            ParaWord=MAKEWORD(pRxData[0],pRxData[1]);
            ParaQPM=RxMsg[AsduHeadLength+2];
            temp=(long)ParaWord*(long)DevList[ActDevIndex].DevData.AIMaxVal[No]/0x3FFF;
            switch(ParaQPM&0x3F)
            {
                case 1://门限
                    
                    DevList[ActDevIndex].DevData.AIPara[No].DeadValue=(short)temp;
                    break;
                case 3://下限
                    DevList[ActDevIndex].DevData.AIPara[No].LowLimit=(short)temp;
                    break;
                case 4://上限
                    DevList[ActDevIndex].DevData.AIPara[No].UpLimit=(short)temp;
                    break;
            }
            break;
        case P_ME_NB:    //标度化
            ParaWord=MAKEWORD(pRxData[0],pRxData[1]);  
            ParaQPM=RxMsg[AsduHeadLength+2];
            switch(ParaQPM&0x3F)
            {
                case 1://门限
                    DevList[ActDevIndex].DevData.AIPara[No].DeadValue=ParaWord;
                    break;
                case 3://下限
                    DevList[ActDevIndex].DevData.AIPara[No].LowLimit=ParaWord;
                    break;
                case 4://上限
                    DevList[ActDevIndex].DevData.AIPara[No].UpLimit=ParaWord;
                    break;
            }
            break;
        case P_ME_NC:    //短浮点数
            //ParaFloat=*((float*)(pRxData));
            INT32U dd;
            dd = MAKEDWORD(MAKEWORD(pRxData[0],pRxData[1]),MAKEWORD(pRxData[2],pRxData[3]));    //ll
            ParaFloat =*((float *)(&dd));
            ParaQPM=RxMsg[AsduHeadLength+4];
            
            
            switch(ParaQPM&0x3F)
            {
                case 1://门限
                    DevList[ActDevIndex].DevData.AIPara[No].DeadValue=(short)ParaFloat;
                    
                    break;
                case 3://下限
                    DevList[ActDevIndex].DevData.AIPara[No].LowLimit=(short)ParaFloat;
                    break;
                case 4://上限
                    DevList[ActDevIndex].DevData.AIPara[No].UpLimit=(short)ParaFloat;
                    break;
            }

            break;
    }

    Data1.Flag|=CallParaSet;
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
}

//OK
void CSecAppSev::EnCodeInitEnd(void)
{
    int jj;
    TxMsg[0]=M_EI_NA;
    *pTxVSQ = 1;
    (*pTxVSQ) &= ~VSQ_SQ;

    TxMsg[CotLocation]=INIT_101;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);

    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    //初始化原因
    *pTxData=0;
    
    if(*pRestType == 0xee)  //复位规约进程状态记忆 ll 2010/07/20   for 广西规约测试
    {
        *pTxData = 2;
        *pRestType = 0;   
    }
    
    *AppCommand=APP_SENDDATA;
    *LengthOut=AsduHeadLength+1;
    if(BalanMode)
    {    
        *AppCommand=APP_SENDCON;
        //MasterStatus=INUSE;     //平衡模式下，只要发送了初始化结束帧就认为链路是可用的  wjr 2009.6.3
    }   
}
/*------------------------------------------------------------------/
函数名称：  RebootCheckUDataFlag()
函数功能：  掉电检查是否需要传送SOE
输入说明：   
输出说明：  
备注：     
/------------------------------------------------------------------*/
void CSecAppSev::RebootCheckUDataFlag(void)
{
    if(GYKZ2015Flag)    //国网要求上电清缓存
    {
        for(int i=0;i<DevCount;i++)
        {
            clear_SOE(DevList[i].DevID,BIENTFLAG);
            clear_SOE(DevList[i].DevID,BIETFLAG);
        }
        return;
    }
        
    for(int i=0;i<DevCount;i++)
    {
        if (test_flag(DevList[i].DevID,BIENTFLAG))
        {
            if(GYKZ2015Flag == FALSE)
                Data1.Flag|=HaveCOS;
        }
        if (test_flag(DevList[i].DevID,BIETFLAG))
            Data1.Flag|=HaveSOE;
    }
    
}
/*------------------------------------------------------------------/
函数名称：  SetUDataFlag()
函数功能：  检测是否有SOE/COS需要发送，有则置位发送SOE/COS标志
输入说明：   
输出说明：  
备注：      test_flag是探测是否有需要发送的soe
/------------------------------------------------------------------*/
void CSecAppSev::SetUDataFlag(void)
{
    BOOL rc;

    //查询检修压板状态，检修压板投，则不发送soe和cos
    rc = appGetJXStatus();
    if(rc)
    {
        clear_flag(DevList[0].DevID,BIENTFLAG);
        clear_flag(DevList[0].DevID,BIETFLAG);
        
        clear_SOE(DevList[0].DevID,BIENTFLAG);
        clear_SOE(DevList[0].DevID,BIETFLAG);
        return;
    }
    
    for(int i=0;i<DevCount;i++)
    {
        if (test_flag(DevList[i].DevID,BIENTFLAG))
        {
            if(SendCOS == 1)
                Data1.Flag|=HaveCOS;
        }
        if (test_flag(DevList[i].DevID,BIETFLAG))
            Data1.Flag|=HaveSOE;
    }
}

//检测其他设备有无COS、SOE
void CSecAppSev::CheckUDataFlag(INT16U DevNo,INT8U Flag)
{
    INT16U i;
    for(i=0;i<DevCount;i++)
    {
        if(i==DevNo)
            continue;
            
        if(Flag==BIENTFLAG)
        {
            if (test_flag(DevList[i].DevID,BIENTFLAG))
            {
                Data1.Flag|=HaveCOS;
                return;
            }
        }
        if(Flag==BIETFLAG)
        {
            if (test_flag(DevList[i].DevID,BIETFLAG))
            {
                Data1.Flag|=HaveSOE;
                return;
            }
        }
    }
}

void CSecAppSev::SetUMsgFlag(void)
{
    Data1.Flag|=HaveYK;
}

//OK
void CSecAppSev::ProcTaskMsg(void)//处理遥控返校信息
{
    INT32U rc;
    INT8U jj;
    BOOL Stop=FALSE;
    //INT16U bonum;
    
    /*if(MyConfig.type ==DEVTYPE_DF9311A1)
        bonum = 8;
    else
        bonum = DevList[BODevIndex].DevData.BONum;*/
    if ( ((SwitchNoTemp-1)*2==DevList[BODevIndex].DevData.BONum) || ((SwitchNoTemp - 1) * 2 == (DevList[BODevIndex].DevData.BONum + 2)))    //蓄电池维护和复归级联设备
    //if((SwitchNoTemp-1)*2== bonum)
    {
        
    	TxMsg[0]=YKTypeID;
        (*pTxVSQ) = 1;
    	TxMsg[CotLocation]=ACTCON;
    	for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[BODevIndex].Addr>>(8*jj);
        *pTxInfoAddr    =LOBYTE((SwitchNoTemp+LBO-1));
        *(pTxInfoAddr+1)=HIBYTE((SwitchNoTemp+LBO-1));
        *pTxData = DcoTemp;
        *LengthOut=AsduHeadLength+1;
        *AppCommand=APP_SENDDATA;
        if((DcoTemp&DCO_SE)==0)
            YKStatus=YKTERM;
        if(YKStatus==YKTERM)
        {
            *AppCommand|=APP_HAVEDATA1;
            Data1.Flag|=HaveYK;
        }
        if(BalanMode)
            *AppCommand=APP_SENDCON;
    	return;
    }
    
    while(!Stop)
    {
        rc=ReceiveMSG(MySelf.QID,(void*)pMsg,MAXMSGLENGTH,ALWAYSRETURN);
        if (!rc)
            Stop=EnCodeCtrlRet();
        else
            Stop=TRUE;
    }
}
//
BOOL CSecAppSev::EnCodeCtrlRet(void)  //遥控返校
{
    int jj;
    INT8U OnOff;
    INT8U dco=0;
    INT8U sco=0;
    INT8U Cmd;
    INT16U DeviceID,SwitchNo;

    short RetNo;
    struct YKMessage *YKMsgBuf = (struct YKMessage *)pMsg;

    
    if(Sec101Pad.ControlPermit == 0)
    {
        *LengthOut=0;
        *AppCommand=APP_NODATA;
        return(FALSE);
    }

    RetNo = CheckYK(&DeviceID,&SwitchNo,&OnOff,(INT8U *)YKMsgBuf);
    if(RetNo > 0)
    {
        switch(RetNo)
        {
            case 1://1-遥控预置成功
                Cmd = SELECT;
                YkStatusForTest = 1;    //ll 为广州测试临时修改 2012-3-24
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
                break;
        }
    }
    else
    {
        *LengthOut=0;
        *AppCommand=APP_NODATA;
        return(FALSE);
    }
    SwitchNoTemp = SwitchNo;
    TxMsg[0]=C_DC_NA;
    switch(YKTypeID)
    {
        case C_SC_NA:
            TxMsg[0]=C_SC_NA;
            break;
        case C_DC_NA:
            TxMsg[0]=C_DC_NA;
            break;
        case C_RC_NA:
            TxMsg[0]=C_RC_NA;
            break;
    }

    (*pTxVSQ) = 1;

    if (Cmd == SELECT)
    {
        if((RetNo == 1)&&(RFaultFlag==0xff)&&(OnOff == 1)&&(SwitchNo==DevList[BODevIndex].DevData.BONum/2))//北京故障复归执行
        {
            Data1.Flag&=(~HaveYK);
            RFaultFlag=0;
            ExecuteYK(wAppID,DeviceID,SwitchNo,OnOff);
            *LengthOut=0;
            *AppCommand=APP_NODATA;
            return  FALSE;
        }

        if((RetNo == 5)||(RetNo == 6))//撤消
        {
            TxMsg[CotLocation]=DEACTCON;
            YKStatus=YKTERM;
        }
        else
        {
            TxMsg[CotLocation]=ACTCON;
        }

        if(YKTypeID==C_SC_NA)
            sco |= SCO_SE;//0x80 预置标志位
        else
            dco |= DCO_SE;
    }
    else
    {
        TxMsg[CotLocation]=ACTCON;
        if(YKTypeID==C_SC_NA)
            sco &= ~SCO_SE;
        else
            dco &= ~DCO_SE;
        YKStatus=YKTERM;
    }

    if ((RetNo & 1) == 0)//2,4,6 失败
    {
        if(GetYKRYBState() == TRUE)
            TxMsg[CotLocation]|=0x40;
        else
        {
            TxMsg[CotLocation] = COT_YKRYBERR;    //返回遥控软压板错误 ll 
            TxMsg[CotLocation] |= COT_PONO;
        }
        
    }
    else//1,3,5成功
        TxMsg[CotLocation]&=(~0x40);

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[BODevIndex].Addr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((SwitchNo+LBO-1));
    *(pTxInfoAddr+1)=HIBYTE((SwitchNo+LBO-1));

    if(YKTypeID==C_SC_NA)
    {
        if (OnOff == 1)  //he
            sco |=1;
        else if (OnOff == 2)  //fen
            sco &=0xfe;

        ScoTemp = sco;
        *pTxData = sco;
    }
    else
    {
        if (OnOff==1)  //he
            dco |= 2;
        else if (OnOff==2)  //fen
            dco |= 1;
        else
            dco |= OnOff;

        DcoTemp = dco;
        *pTxData = dco;
    }

    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    if(YKStatus==YKTERM)
    {
        *AppCommand|=APP_HAVEDATA1;
        Data1.Flag|=HaveYK;
    }

    if(BalanMode)
        *AppCommand=APP_SENDCON;

    return(TRUE);

}
//
void CSecAppSev::EditYKTerm(void)//遥控结束
{
    int jj;
    
    switch(YKTypeID)
    {
        case C_SC_NA:
            TxMsg[0]=C_SC_NA;
            *pTxData = ScoTemp;
            break;
        case C_DC_NA:
            TxMsg[0]=C_DC_NA;
            *pTxData = DcoTemp;
            break;
        case C_RC_NA:
            TxMsg[0]=C_RC_NA;
            *pTxData = DcoTemp;
            break;
    }

    (*pTxVSQ) = 1;

    TxMsg[CotLocation]=ACTTERM;

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[BODevIndex].Addr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((SwitchNoTemp+LBO-1));
    *(pTxInfoAddr+1)=HIBYTE((SwitchNoTemp+LBO-1));

    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}


BOOL CSecAppSev::GetActDevIndexByAddr(INT16U Addr)
{
    for (INT16U i=0;i<DevCount;i++)
    {
        if (DevList[i].Addr==Addr)//本机地址作为公共体地址。
        {
            ActDevIndex=i;
            return(TRUE);
        }
    }
    return(FALSE);
}

void CSecAppSev::EnCodeDBIENT(void)
{
	INT8U Status1,Status0;
    INT16U i,j,jj,Len,Length,Num,SendNum;
    short FramePos;
    struct BIEWithoutTimeData_t *p;
    //logSysMsgNoTime("SEC 编辑双点COS开始",0,0,0,0);//  debug ll
    TxMsg[0]=M_DP_NA;   //3，不带时标的双点信息
    (*pTxVSQ) &= ~VSQ_SQ;

    if(BalanMode==TRUE)//zzw
        TxMsg[CotLocation]=SPONT;
    else
        TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DBICOSDevIndex].Addr>>(8*jj);
    Num=0;
    SendNum=0;
    FramePos=0-InfoAddrSize;
    p=(struct BIEWithoutTimeData_t *)DBICOSDBData;
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度

    for(j=0;j<DBICOSnum;j++)
    {
        
        Status0 = 0;
        Status1 = ((p->Status&0x60)>>5);
        
        if(FramePos < 0)
        {
            *pTxInfoAddr    =LOBYTE((p->No+LDBIinfoaddr));
            *(pTxInfoAddr+1)=HIBYTE((p->No+LDBIinfoaddr));
        }
        else
        {
            pTxData[FramePos]   = LOBYTE(p->No+LDBIinfoaddr);//信息体地址
            pTxData[FramePos+1] = HIBYTE(p->No+LDBIinfoaddr);//信息体地址
            if(InfoAddrSize == 3)
                pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
        }
        FramePos+=InfoAddrSize;;
        
        //logSysMsgNoTime("p->Status=%x",p->Status,0,0,0);   //  debug ll
        if((p->Status&BIACTIVEFLAG)==0)
            pTxData[FramePos]=Status1|Status0|P101_IV;//设置遥信状态字节
        else
            pTxData[FramePos]=Status1|Status0;//设置遥信状态字节
            
        if(p->Status&SUBSTITUTEDFLAG)
            pTxData[FramePos]|=P101_SB;//设置遥信状态字节
        
        
        FramePos++;        
        SendNum++;//发送个数
        p++;
        if(FramePos>=Length)
            break;
    }
    
    if(SendNum>0)
    {
        Len=FramePos+AsduHeadLength;//应用层报文总长度
        //LastFrame=BI;
        //LastDevIndex=DBICOSDevIndex;
        //BIFrame|=BIENTFRAME;
        //DevList[DBICOSDevIndex].Data1.BIENTNum=SendNum;      //变位遥信发送个数
        *pTxVSQ=SendNum;
        
        Num=SendNum;
        
        if(Num<DBICOSnum)
        {
         	i=0;
         	while(Num<DBICOSnum)
            {
                DBICOSDBData[i]=DBICOSDBData[Num];
             	i++;
             	Num++;
            }
         	DBICOSnum = DBICOSnum-SendNum;
        }
        else
            DBICOSnum=0;
            
        *LengthOut=Len;
         if(BalanMode)
             *AppCommand=APP_SENDCON;
         else
             *AppCommand=APP_SENDDATA;  
    }
    //logSysMsgNoTime("SEC 编辑双点COS结束",0,0,0,0);//  debug ll  
    
}

BOOL CSecAppSev::EnCodeBIENT(void)  //编辑COS
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithoutTimeData_t *p;
    BOOL HaveData=FALSE;    //是否有后续数据 TRUE 有
 
       

    *LengthOut = 0;
    for(i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIENTFLAG))
            continue;
        //logSysMsgNoTime("SEC EnCodeBIENT中间1态",0,0,0,0);//  debug ll
        TxMsg[0]=M_SP_NA;//1，不带时标的单点信息
        (*pTxVSQ) = 0;

        if(BalanMode==TRUE)//zzw
            TxMsg[CotLocation]=SPONT;
        else
            TxMsg[CotLocation]=REQ;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);

        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIENT(DevList[i].DevID,DevList[i].RealWin->BIENTimRP,80,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        else
            Num=L_ReadSBIENT(DevList[i].DevID,DevList[i].pDbaseWin->BIENTimRP,80,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        //logSysMsgNoTime("SEC EnCodeBIENT中间2态%d",Num,0,0,0);//  debug ll
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;
            p=(struct BIEWithoutTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {
                if(DevList[i].DevData.DBINum>0)
                {
                	if(p->No<DevList[i].DevData.DBINum)
                    {
                        
                        	DBICOSDBData[2*DBICOSnum]=(struct BIEWithoutTimeData_t)(*p);
                        	DBICOSDBData[2*DBICOSnum+1]=(struct BIEWithoutTimeData_t)(*(p+1));
                        	DBICOSnum++;
                        	p=p+2;
                        	j=j+2;
                        
                    }
                	else
                    {
                    	if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
        
                        if(FramePos < 0)
                        {
                            *pTxInfoAddr    =LOBYTE((p->No-DevList[i].DevData.DBINum+LBIinfoaddr));
                            *(pTxInfoAddr+1)=HIBYTE((p->No-DevList[i].DevData.DBINum+LBIinfoaddr));
                        }
                        else
                        {
                            pTxData[FramePos]   = LOBYTE(p->No-DevList[i].DevData.DBINum+LBIinfoaddr);//信息体地址
                            pTxData[FramePos+1] = HIBYTE(p->No-DevList[i].DevData.DBINum+LBIinfoaddr);//信息体地址
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
                        }
                        FramePos+=InfoAddrSize;;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                        else
                            pTxData[FramePos]=Status;//设置遥信状态字节
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//设置遥信状态字节
                        
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
        
                        if(FramePos < 0)
                        {
                            *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                            *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                        }
                        else
                        {
                            pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//信息体地址
                            pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//信息体地址
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
                        }
                        FramePos+=InfoAddrSize;;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                        else
                            pTxData[FramePos]=Status;//设置遥信状态字节
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//设置遥信状态字节
                        
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
            //logSysMsgNoTime("SEC EnCodeBIENT中间3态%d dbi=%d",SendNum,DBICOSnum,0,0);//  debug ll
            if(SendNum>0)
            {
                Len=FramePos+AsduHeadLength;//应用层报文总长度
                LastFrame=BI;
                LastDevIndex=i;
                BIFrame|=BIENTFRAME;
                DevList[i].Data1.BIENTNum=j;      //变位遥信发送个数
                *pTxVSQ=SendNum;
                
                *LengthOut=Len;
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;
                
                if(DevList[i].Flag)
                {
                    if((WritePtr == DevList[i].RealWin->BIENTimRP+j))
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveCOS);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                else
                {
                    if((WritePtr == DevList[i].pDbaseWin->BIENTimRP+j))
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveCOS);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                
                if(HaveData == FALSE) 
                {      
                    if(DBICOSnum>0)
                    {
                        HaveData = TRUE;
                    }
                }   
                
            }
            else
            {
            	if(DBICOSnum>0)
                {
                	EnCodeDBIENT();
                    LastFrame=BI;
                    LastDevIndex=i;
                    BIFrame|=BIENTFRAME;
                    HaveData=TRUE;
                    DevList[i].Data1.BIENTNum=j;      //变位遥信发送个数
                    
                    if(DevList[i].Flag)
                    {
                        if((WritePtr == DevList[i].RealWin->BIENTimRP+j))
                        {
                            HaveData = TRUE;
                        }
                    }
                    else
                    {
                        if((WritePtr == DevList[i].pDbaseWin->BIENTimRP+j))
                        {
                            HaveData = TRUE;
                        }
                    }
                    
                	if(HaveData == FALSE)
                	{
                        if(DBICOSnum>0)
                            HaveData = TRUE; 
                    }   
                }
            	else
                {
                    *LengthOut = 0;
                	if(j>0)
                    {
                    	DevList[i].Data1.BIENTNum=j;      //变位遥信发送个数
                    	ClearFlag(i,BIENTFLAG);
                    	
                    }
                }
            }
        }
    }
    
    if((DBICOSnum>0) && (*LengthOut==0))
    {
        EnCodeDBIENT();
                
        if(DBISOEnum)
            return TRUE; 
    }
    
    return(HaveData);
}

BOOL CSecAppSev::EnCodeBIENT_ALLDBI(void)  //编辑COS,全部处理成双点遥信
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithoutTimeData_t *p;
    BOOL HaveData=FALSE;    //是否有后续数据 TRUE 有
 
    *LengthOut = 0;
    for(i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIENTFLAG))
            continue;
        //logSysMsgNoTime("SEC EnCodeBIENT中间1态",0,0,0,0);//  debug ll
        TxMsg[0]=M_DP_NA; 
        (*pTxVSQ) = 0;

        if(BalanMode==TRUE)//zzw
            TxMsg[CotLocation]=SPONT;
        else
            TxMsg[CotLocation]=REQ;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);

        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIENT(DevList[i].DevID,DevList[i].RealWin->BIENTimRP,80,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        else
            Num=L_ReadSBIENT(DevList[i].DevID,DevList[i].pDbaseWin->BIENTimRP,80,(struct BIEWithoutTimeData_t *)DBData,&WritePtr);
        //logSysMsgNoTime("SEC EnCodeBIENT中间2态%d",Num,0,0,0);//  debug ll
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;    //因为pTxData指向信息体之后，所以第一个信息体地址没法填，应该用TxMsg不该用pTxData，这样就不存在这样的问题了 ll
            p=(struct BIEWithoutTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {               
                if(p->Status&0x80)
                    Status=(BIDBI_YXH>>5);
                else
                    Status=(BIDBI_YXF>>5);

                if(FramePos < 0)
                {
                    *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                    *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                }
                else
                {
                    pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//信息体地址
                    pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//信息体地址
                    if(InfoAddrSize == 3)
                        pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
                }
                FramePos+=InfoAddrSize;

                if((p->Status&BIACTIVEFLAG)==0)
                    pTxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                else
                    pTxData[FramePos]=Status;//设置遥信状态字节
                    
                if(p->Status&SUBSTITUTEDFLAG)
                    pTxData[FramePos]|=P101_SB;//设置遥信状态字节
                
                FramePos++;
                SendNum++;//发送个数
                p++;
                j++;
                if(FramePos>=Length)
                    break;
            }
           
            //logSysMsgNoTime("SEC EnCodeBIENT中间3态%d dbi=%d",SendNum,DBICOSnum,0,0);//  debug ll
            if(SendNum>0)
            {
                Len=FramePos+AsduHeadLength;//应用层报文总长度
                LastFrame=BI;
                LastDevIndex=i;
                BIFrame|=BIENTFRAME;
                DevList[i].Data1.BIENTNum=j;      //变位遥信发送个数
                *pTxVSQ=SendNum;
                
                *LengthOut=Len;
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;
                
                if(DevList[i].Flag)
                {
                    if((WritePtr == DevList[i].RealWin->BIENTimRP+j))
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveCOS);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                else
                {
                    if((WritePtr == DevList[i].pDbaseWin->BIENTimRP+j))
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveCOS);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                
                
            }
            else
            {
            	
                *LengthOut = 0;
            	if(j>0)
                {
                	DevList[i].Data1.BIENTNum=j;      //变位遥信发送个数
                	ClearFlag(i,BIENTFLAG);
                }
                
            }
        }
    }
    
    return(HaveData);
}
void CSecAppSev::EnCodeDBISOE(void)
{
	INT8U Status0,Status1;
    INT16U i,j,jj,Len,Length,Num,SendNum;
    short FramePos;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    TxMsg[0]=M_DP_TA;   //带时标的双点信息
    if(Sec101Pad.SOEWithCP56Time == 1)
        TxMsg[0]=M_DP_TB;   //带长时标的双点信息
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DBIDevIndex].Addr>>(8*jj);
    (*pTxVSQ) &= ~VSQ_SQ;

    if(BalanMode==TRUE)
        TxMsg[CotLocation]=SPONT;
    else
        TxMsg[CotLocation]=REQ;
    
    Num=0;    
    SendNum=0;
    FramePos=0-InfoAddrSize;
    p=(struct BIEWithTimeData_t *)DBIDBData;
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度

    for(j=0;j<DBISOEnum;j++)
    {
      
        Status0 = 0;
        Status1 = (p->Status&0x60)>>5;
        
         //写信息体地址
        if(FramePos < 0)
        {
            *pTxInfoAddr    =LOBYTE((p->No+LDBIinfoaddr));
            *(pTxInfoAddr+1)=HIBYTE((p->No+LDBIinfoaddr));
        }
        else
        {
             pTxData[FramePos]   = LOBYTE(p->No+LDBIinfoaddr);//信息体地址
             pTxData[FramePos+1] = HIBYTE(p->No+LDBIinfoaddr);//信息体地址
             if(InfoAddrSize == 3)
                 pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
         }
         FramePos+=InfoAddrSize;;
         //写状态
         if((p->Status&BIACTIVEFLAG)==0)
             pTxData[FramePos]=Status1|Status0|P101_IV;//设置遥信状态字节
         else
             pTxData[FramePos]=Status1|Status0;//设置遥信状态字节
        
         if(p->Status&SUBSTITUTEDFLAG)
             pTxData[FramePos]|=P101_SB;//设置遥信状态字节
             
         FramePos++;
            //写时间
         AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

         pTxData[FramePos++] = LOBYTE(time.MSecond);
         pTxData[FramePos++] = HIBYTE(time.MSecond);
         pTxData[FramePos++] = time.Minute;

         if(TxMsg[0]==M_DP_TB)//长时标
         {
             pTxData[FramePos++] = time.Hour;
             pTxData[FramePos++] = time.Day;
             pTxData[FramePos++] = time.Month;
             pTxData[FramePos++] = time.Year;
         }

         SendNum++;//发送个数
         p++;
         
         if(FramePos>=Length)
             break;
     }
     if(SendNum>0)
     {
         Len=FramePos+AsduHeadLength;//应用层报文总长度
         //LastDevIndex=DBIDevIndex;
         //LastFrame=BI;
         //BIFrame|=BIETFRAME;
         //DevList[DBIDevIndex].Data1.SOENum=SendNum;   //这里不再标记，因为前面单点遥信已经标记过了
         *pTxVSQ=SendNum;
         
         Num = SendNum;
         if(Num<DBISOEnum)
         {
            i=0;
         	while(Num<DBISOEnum)
            {
                 DBIDBData[i] = DBIDBData[Num];
             	Num++;
             	i++;
            }
         	DBISOEnum = DBISOEnum-SendNum;
         }
         else
             DBISOEnum=0;
             
         *LengthOut=Len;
         if(BalanMode)
             *AppCommand=APP_SENDCON;
         else
             *AppCommand=APP_SENDDATA;
         
     }
   
    
}

/*------------------------------------------------------------------/
函数名称：  EnCodeSOE()
函数功能：  编辑SOE报文。在1级数据中调用
输入说明：   
输出说明：  TRUE 表示有后续数据  FALSE 无后续数据  ll修改 2017-8-8
备注：      发送策略，先发送单点SOE，再发送DBIsoe。
            当单点SOE超过20时，需要多帧发送，再所有单点SOE发送完成后，再发送DBIsoe。
/------------------------------------------------------------------*/
BOOL CSecAppSev::EnCodeSOE(void) //编辑SOE
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    BOOL HaveData=FALSE;        //后续是否有数据
    
    *LengthOut = 0;
    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIETFLAG))
            continue;
        
        //logSysMsgNoTime("SEC 编辑单点SOE开始",0,0,0,0);//  debug ll
        TxMsg[0]=M_SP_TA;   //带时标的单点信息
        if(Sec101Pad.SOEWithCP56Time == 1)
            TxMsg[0]=M_SP_TB;   //带长时标的单点信息
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);
        (*pTxVSQ) = 0;

        if(BalanMode==TRUE)
            TxMsg[CotLocation]=SPONT;
        else
            TxMsg[CotLocation]=REQ;
        
        Num=0;
        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIET(DevList[i].DevID,DevList[i].RealWin->BIETimRP,20,(struct BIEWithTimeData_t *)DBData,&WritePtr);//
        else
            Num=L_ReadSBIET(DevList[i].DevID,DevList[i].pDbaseWin->BIETimRP,20,(struct BIEWithTimeData_t *)DBData,&WritePtr);

        if(Num>0)
        {
            SendNum=0;
            FramePos = 0-InfoAddrSize;
            p=(struct BIEWithTimeData_t *)DBData;
            Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
            
            j=0;
            while(j<Num)
            {
                if(DevList[i].DevData.DBINum>0)
                {
                    if(p->No<DevList[i].DevData.DBINum)
                    {
                        
                        DBIDBData[2*DBISOEnum]=(struct BIEWithTimeData_t)(*p);
                        DBIDBData[2*DBISOEnum+1]=(struct BIEWithTimeData_t)(*(p+1));
                        DBISOEnum++;
                        p=p+2;
                        j=j+2;
                        
                    }
                    else
                    {
                        //状态转换：数据库中的遥信D7为状态，规约中D0为状态
                        if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
                        //写信息体地址
                        if(FramePos < 0)
                        {
                            *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr-DevList[i].DevData.DBINum));
                            *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr-DevList[i].DevData.DBINum));
                        }
                        else
                        {
                            pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr-DevList[i].DevData.DBINum);//信息体地址
                            pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr-DevList[i].DevData.DBINum);//信息体地址
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
                        }
                        FramePos+=InfoAddrSize;;
                        //写状态
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                        else
                            pTxData[FramePos]=Status;//设置遥信状态字节
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//设置遥信状态字节
                        
                            
                        FramePos++;
                    
                        //写时间
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
                    
                        pTxData[FramePos++] = LOBYTE(time.MSecond);
                        pTxData[FramePos++] = HIBYTE(time.MSecond);
                        pTxData[FramePos++] = time.Minute;
                    
                        if(TxMsg[0]==M_SP_TB)//长时标
                        {
                            pTxData[FramePos++] = time.Hour;
                            pTxData[FramePos++] = time.Day;
                            pTxData[FramePos++] = time.Month;
                            pTxData[FramePos++] = time.Year;
                        }
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
                        //状态转换：数据库中的遥信D7为状态，规约中D0为状态
                        if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
                        
                        //写信息体地址
                        if(FramePos < 0)
                        {
                            *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                            *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                        }
                        else
                        {
                            pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//信息体地址
                            pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//信息体地址
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
                        }
                        FramePos += InfoAddrSize;
                        
                        //写状态
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                        else
                            pTxData[FramePos]=Status;//设置遥信状态字节
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//设置遥信状态字节
                        
                            
                        FramePos++;
        
                        //写时间
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
        
                        pTxData[FramePos++] = LOBYTE(time.MSecond);
                        pTxData[FramePos++] = HIBYTE(time.MSecond);
                        pTxData[FramePos++] = time.Minute;
        
                        if(TxMsg[0]==M_SP_TB)//长时标
                        {
                            pTxData[FramePos++] = time.Hour;
                            pTxData[FramePos++] = time.Day;
                            pTxData[FramePos++] = time.Month;
                            pTxData[FramePos++] = time.Year;
                        }
        
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
                
                Len=FramePos+AsduHeadLength;//应用层报文总长度
                LastDevIndex=i;
                LastFrame=BI;
                BIFrame|=BIETFRAME;
                DevList[i].Data1.SOENum = j;       //DevList[i].Data1.SOENum 是标记已经发送的个数(或者已经处理的个数），用于对SOE读指针进行控制
                *pTxVSQ=SendNum;

                *LengthOut = Len;      
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;

                //判断是否有后续数据要发送
                if(DevList[i].Flag)
                {
                    if(WritePtr != DevList[i].RealWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                else
                {
                    if(WritePtr != DevList[i].pDbaseWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                
                if(HaveData == FALSE)
                {
                    //单点遥信都处理完了，再处理双点遥信。这时如果双点遥信出现问题，容易送不到主站
                    if(DBISOEnum >0)
                    {
                        HaveData = TRUE;
                    }
                    
                }

            }
            else
            {
            	if(DBISOEnum>0)
                {
                    EnCodeDBISOE();
                    
                    LastDevIndex=i;
                    LastFrame=BI;
                    BIFrame|=BIETFRAME;
                    DevList[i].Data1.SOENum=j;   
                    
                    //判断是否有后续数据要发送
                    if(DevList[i].Flag)
                    {
                        if(WritePtr != DevList[i].RealWin->BIETimRP+j)
                        {
                            HaveData = TRUE;
                        }
                    }
                    else
                    {
                        if(WritePtr != DevList[i].pDbaseWin->BIETimRP+j)
                        {
                            HaveData = TRUE;
                        }
                    }                 
                    
                    if(HaveData == FALSE)
                	{
                        if(DBICOSnum>0)
                            HaveData = TRUE; 
                    } 
                }
                else
                {
                    //走到这里属于异常情况
                    *LengthOut = 0;     //表示没有数据需要发送
                	if(j>0)
                    {
                    	DevList[i].Data1.SOENum=j;
                        ClearFlag(i,BIETFLAG);
                    }
                }
            }
        }
    }
    
    if((DBISOEnum>0) && (*LengthOut==0))
    {
        //先发送单点，所有单点都发送完了，再发送DBI出去。
        EnCodeDBISOE(); 
        
        if(DBISOEnum > 0)
            return TRUE;   
    } 
       
    return(HaveData);
}

/*------------------------------------------------------------------/
函数名称：  EnCodeSOE_ALLDBI()
函数功能：  编辑SOE报文。在1级数据中调用
输入说明：   
输出说明：  TRUE 表示有后续数据  FALSE 无后续数据  ll修改 2017-8-8
备注：      发送策略，先发送单点SOE，再发送DBIsoe。
            当单点SOE超过20时，需要多帧发送，再所有单点SOE发送完成后，再发送DBIsoe。
/------------------------------------------------------------------*/
BOOL CSecAppSev::EnCodeSOE_ALLDBI(void) //编辑SOE
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    BOOL HaveData=FALSE;        //后续是否有数据
    
    *LengthOut = 0;
    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIETFLAG))
            continue;
        
        //logSysMsgNoTime("SEC 编辑单点SOE开始",0,0,0,0);//  debug ll
        TxMsg[0]=M_DP_TB;   //带时标的单点信息
        
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);
        (*pTxVSQ) = 0;

        if(BalanMode==TRUE)
            TxMsg[CotLocation]=SPONT;
        else
            TxMsg[CotLocation]=REQ;
        
        Num=0;
        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIET(DevList[i].DevID,DevList[i].RealWin->BIETimRP,20,(struct BIEWithTimeData_t *)DBData,&WritePtr);//
        else
            Num=L_ReadSBIET(DevList[i].DevID,DevList[i].pDbaseWin->BIETimRP,20,(struct BIEWithTimeData_t *)DBData,&WritePtr);

        if(Num>0)
        {
            SendNum=0;
            FramePos = 0-InfoAddrSize;
            p=(struct BIEWithTimeData_t *)DBData;
            Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
            
            j=0;
            while(j<Num)      
            {
                //状态转换：数据库中的遥信D7为状态，规约中D0为状态
                if(p->Status&0x80)
                    Status=(BIDBI_YXH>>5);
                else
                    Status=(BIDBI_YXF>>5);
                
                //写信息体地址
                if(FramePos < 0)
                {
                    *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                    *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                }
                else
                {
                    pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//信息体地址
                    pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//信息体地址
                    if(InfoAddrSize == 3)
                        pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
                }
                FramePos += InfoAddrSize;
                
                //写状态
                if((p->Status&BIACTIVEFLAG)==0)
                    pTxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                else
                    pTxData[FramePos]=Status;//设置遥信状态字节
                    
                if(p->Status&SUBSTITUTEDFLAG)
                    pTxData[FramePos]|=P101_SB;//设置遥信状态字节
                
                    
                FramePos++;

                //写时间
                AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

                pTxData[FramePos++] = LOBYTE(time.MSecond);
                pTxData[FramePos++] = HIBYTE(time.MSecond);
                pTxData[FramePos++] = time.Minute;
                pTxData[FramePos++] = time.Hour;
                pTxData[FramePos++] = time.Day;
                pTxData[FramePos++] = time.Month;
                pTxData[FramePos++] = time.Year;
                
                SendNum++;//发送个数
                p++;
                j++;
                
                if(FramePos>=Length)
                    break;
            }
            
            if(SendNum>0)
            {
                
                Len=FramePos+AsduHeadLength;//应用层报文总长度
                LastDevIndex=i;
                LastFrame=BI;
                BIFrame|=BIETFRAME;
                DevList[i].Data1.SOENum = j;       //DevList[i].Data1.SOENum 是标记已经发送的个数(或者已经处理的个数），用于对SOE读指针进行控制
                *pTxVSQ=SendNum;

                *LengthOut = Len;      
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;

                //判断是否有后续数据要发送
                if(DevList[i].Flag)
                {
                    if(WritePtr != DevList[i].RealWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                else
                {
                    if(WritePtr != DevList[i].pDbaseWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//检测其他设备是否有COS、SOE等 //ll feng 2012-6-6 解决发送最后一条SOE时置位ACD标志，但是主站询问一级数据时，又回答无数据
                    }
                }
                
            }
            else
            {
                //走到这里属于异常情况
                *LengthOut = 0;     //表示没有数据需要发送
            	if(j>0)
                {
                	DevList[i].Data1.SOENum=j;
                    ClearFlag(i,BIETFLAG);
                }
                
            }
        }
    }
          
    return(HaveData);
}
/*------------------------------------------------------------------/
函数名称：  EnCodeGXReadParaConf
函数功能：  回复召唤参数报文。
输入说明：    
输出说明：  无。
备注：      主站召唤参数时，先回复此报文，之后再组织参数上送报文。
/------------------------------------------------------------------*/
void CSecAppSev::EnCodeGXReadParaConf(void)
{
    TxMsg[0]=P_RS_NA_1_GX;

    (*pTxVSQ) = 1;

    int jj;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    TxMsg[CotLocation]=ACTCON;

    *pTxData=Roi;
    if((*pTxData < INTROGEN) || (*pTxData > INTRO9))
    {
        TxMsg[CotLocation] |= 0x40;                      
        EditReadParaCon=0;
    }
    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}
/*------------------------------------------------------------------/
函数名称：  EnCodeGXReadParaEnd
函数功能：  读参数结束。
输入说明：    
输出说明：  无。
备注：      将参数发送完毕后，以此帧报文结束。
/------------------------------------------------------------------*/
void CSecAppSev::EnCodeGXReadParaEnd(void)
{
    TxMsg[0]=P_RS_NA_1_GX;

    (*pTxVSQ) = 1;

    int jj;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    TxMsg[CotLocation]=ACTTERM;

    *pTxData=Roi;
    
    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}
void CSecAppSev::EnCodeAllDataConf(void)//总召唤确认帧
{
    TxMsg[0]=C_IC_NA;

    (*pTxVSQ) = 1;

    int jj;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    TxMsg[CotLocation]=ACTCON;

    *pTxData=GroupTrn.COT;//INTROGEN=20，响应总召唤——总召唤限定词或分组限定词
    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    *AppCommand|=APP_HAVEDATA1;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}


BOOL CSecAppSev::ProcAllData(void) //处理全数据
{
    INT16U BeginNo,EndNo,Num,i,Len;
    INT16U yx=0;
    INT16U yc=0;
    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))//1——8组，遥信
    {
        if (CheckAndModifyGroup())  //检测信息体地址，组号
        {
            BeginNo=GroupTrn.InfoAddr-LBIinfoaddr;  //起始点号0

            for(i=0;i<GroupTrn.GroupNo;i++)     //yx = 已发送和准备发送组的总个数
                yx+=Sec101Pad.GroupNum[i];
            EndNo=yx-1;
            
            if ((EndNo+1) > DevList[GroupTrn.DevIndex].DevData.BINum)
                EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1;
                
            Len=EnCodeAllData(BeginNo,EndNo,&Num);
            if (Len!=0)//从数据库取数据，并发到链路层。Num为实际发送的数据单元。
            {
                GroupTrn.First=FALSE;
                BeginNo+=Num;//起始点号后移，Num为已经发送的数目。
                GroupTrn.InfoAddr=BeginNo+LBIinfoaddr;

                if(BeginNo < DevList[GroupTrn.DevIndex].DevData.BINum)  //BeginNo在全部发送完会超过BINum，所以要首先判断BeginNo合法性
                {
                    if(YXGroupNo[BeginNo] != GroupTrn.GroupNo)//前一组发完，这里控制按组发送
                    {
                        GroupTrn.GroupNo = YXGroupNo[BeginNo];
                        if ((GroupTrn.COT!=INTROGEN) && (GroupTrn.COT!=BACK))//如果是分组召唤，则说明主站召唤的那组数据已经发完。
                        {
                            GroupTrn.GroupNo=17;//发结束帧
                        }
                    }
                }

                return TRUE;
            }
            else//没有发数据，或遥信数据已经发完。
            {
                if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//背景数据或总召唤，将下一组号设置为9——遥测
                    GroupTrn.GroupNo=9;
                else//否则设置为17——备用。
                    GroupTrn.GroupNo=17;

            }
        }
    }

    if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=12))//9——12组，遥测；注释参考上面遥信的
    {
        if (CheckAndModifyGroup())
        {
            BeginNo=GroupTrn.InfoAddr-LAI;
            for(i=8;i<GroupTrn.GroupNo;i++)
                yc+=Sec101Pad.GroupNum[i];
            EndNo=yc-1;

            if ((EndNo+1)>DevList[GroupTrn.DevIndex].DevData.AINum)
                EndNo=DevList[GroupTrn.DevIndex].DevData.AINum-1;
            if (EnCodeAllData(BeginNo,EndNo,&Num))
            {
                GroupTrn.First=FALSE;
                BeginNo+=Num;
                GroupTrn.InfoAddr=BeginNo+LAI;

                if (GroupTrn.GroupNo!=YCGroupNo[GroupTrn.InfoAddr-LAI]&&(BeginNo<DevList[GroupTrn.DevIndex].DevData.AINum))
                {
                    GroupTrn.GroupNo=YCGroupNo[GroupTrn.InfoAddr-LAI];
                    if ((GroupTrn.COT!=INTROGEN)&&(GroupTrn.COT!=BACK))//分组召唤
                        GroupTrn.GroupNo=17;
                }
                return TRUE;
            }
            else
            {
                if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//背景数据或总召唤
                    GroupTrn.GroupNo=17;
                else
                    GroupTrn.GroupNo=17;
            }
        }
    }
    if(GroupTrn.GroupNo==13)//参数信息，没有改组数据wjr2009.8.31
    {
        GroupTrn.GroupNo=17;    
    }    
    if (GroupTrn.GroupNo==14)//参数信息，只有分组召唤
    {
        if (CheckAndModifyGroup())
        {
            BeginNo=(GroupTrn.InfoAddr-LPARA)/3;
            EndNo=DevList[GroupTrn.DevIndex].DevData.AINum-1;
            if (EnCodeAllData(BeginNo,EndNo,&Num))
            {
                BeginNo+=Num/3;
                GroupTrn.InfoAddr=(GroupTrn.InfoAddr+Num);
                if(BeginNo>=EndNo)
                {
                    GroupTrn.GroupNo=17;
                }
                return TRUE;
            }
            else
            {
                GroupTrn.GroupNo=17;
            }
        }
    }

    if (GroupTrn.GroupNo==15)//步位置信息
    {
        if (CheckAndModifyGroup())
        {
            BeginNo=GroupTrn.InfoAddr-LSPI;
            EndNo=MAXSPINUM+BeginNo;
            if ((EndNo+1)>DevList[GroupTrn.DevIndex].DevData.SPINum)
                EndNo=DevList[GroupTrn.DevIndex].DevData.SPINum-1;
            if (EnCodeAllData(BeginNo,EndNo,&Num))
            {
                GroupTrn.First=FALSE;
                BeginNo+=Num;

                GroupTrn.InfoAddr=(BeginNo%MAXSPINUM)+LSPI;
                return TRUE;
            }
            else
            {
                if ((GroupTrn.COT==INTROGEN)||(GroupTrn.COT==BACK))
                    GroupTrn.GroupNo=17;
            }
        }
    }

    if (GroupTrn.GroupNo==16)//子站远动终端状态
    {
                
        //增加广西要求的送soe
        if (CheckAndModifyGroup())
        {
            if(EnCodeAllData(GroupTrn.SoeStartPtr,0,&Num) )
                return TRUE;
            else
                GroupTrn.GroupNo=17;
        }
        else
            GroupTrn.GroupNo=17;
    }

    if (GroupTrn.GroupNo>=17)//结束处理
    {
        if (GroupTrn.COT==BACK)//背景数据，无结束帧
        {
            GroupTrn.DevIndex++;
            if(GroupTrn.DevIndex>=DevCount)
            {
                GroupTrn.DevIndex=0;
                Data2Flag&=(~BackData);
            }
            return FALSE;
        }
        if(GroupTrn.COT==PERCYC)//第12组可能以周期循环数据发送
        {
            Data2Flag&=(~PerCycData);
            return FALSE;
        }

        //总召唤或分组召唤，平衡非平衡模式总召唤过程处理一致。
        //if((GroupTrn.COT>=INTROGEN)&&(GroupTrn.COT<=INTRO16))//这样的判断在组号非法时有隐患。
        {
            Data1.Flag&=(~CallAllData);
            EnCodeGroupEnd();//发送结束
            
            FirstCallAllData = 0xff;

            if (GetNextDev())   //？？？？怎么还会有其他终端呢 ll
            {
                EditAllDataCon=0xff;
                Data1.Flag|=CallAllData;
            }
            
            return TRUE;
        }
    }
    return FALSE;
}

//OK
BOOL CSecAppSev::CheckAndModifyGroup(void)
{
    INT16U Num,i;
    INT16U yc=0;
    INT16U yx=0;
    INT16U wptr;
    
    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))//1——8组为遥信
    {
        
        for(i=0;i<GroupTrn.GroupNo;i++)
            yx+=Sec101Pad.GroupNum[i];      //yx = 已发送和准备发送组的总个数
        
        
        if(GroupTrn.InfoAddr<(yx-Sec101Pad.GroupNum[GroupTrn.GroupNo-1]+LBIinfoaddr))       //分组召唤。yx是所有组的和，减去最后一组的遥信个数
            GroupTrn.InfoAddr=(yx-Sec101Pad.GroupNum[GroupTrn.GroupNo-1]+LBIinfoaddr);
        else if(GroupTrn.InfoAddr > (yx-1+LBIinfoaddr))   //这里是启什么作用？
        {
            if(GroupTrn.InfoAddr < DevList[GroupTrn.DevIndex].DevData.BINum+LBIinfoaddr)
                GroupTrn.GroupNo=YXGroupNo[GroupTrn.InfoAddr-LBIinfoaddr];
        }

        Num = GroupTrn.InfoAddr-LBIinfoaddr+1;
        if ((Num>DevList[GroupTrn.DevIndex].DevData.BINum)||(GroupTrn.InfoAddr>HBI))//如果没有遥信，或认为该设备遥信已经发送完
        {
            if(GetDBINum()) //检测是否配置双点遥信
            {
                if((GroupTrn.COT==INTROGEN) && (GroupTrn.HaveSendDBI==FALSE))
                {
                    //如果是总招且双点遥信没送，则组织双点遥信。分组召唤不送双点遥信
                    GroupTrn.InfoAddr = LBIinfoaddr;
                    GroupTrn.GroupNo = 1;
                    GroupTrn.HaveSendDBI = TRUE;

                    return TRUE;
                }
                
            }
            if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//如果是循环发送或响应总召唤将组号顺延
                GroupTrn.GroupNo=9;//设置为遥测起始组号
            else
                GroupTrn.GroupNo=17;//第17组为备用

            return(FALSE);
        }
        return(TRUE);
    }

    if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=12))//9——12组为遥测
    {
        for(i=8;i<GroupTrn.GroupNo;i++)
            yc+=Sec101Pad.GroupNum[i];

        if(GroupTrn.InfoAddr<(yc-Sec101Pad.GroupNum[GroupTrn.GroupNo-1]+LAI))
            GroupTrn.InfoAddr=(yc-Sec101Pad.GroupNum[GroupTrn.GroupNo-1])+LAI;
        else if(GroupTrn.InfoAddr>(yc-1+LAI))
            GroupTrn.GroupNo=YCGroupNo[GroupTrn.InfoAddr-LAI];

        Num=GroupTrn.InfoAddr-LAI+1;
        if ((Num>DevList[GroupTrn.DevIndex].DevData.AINum)||(GroupTrn.InfoAddr>HAI))//遥测已经发送完,
        {
            if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//如果是背景数据或响应总召唤将组号顺延
                GroupTrn.GroupNo=17;    //跳转到结束
            else
                GroupTrn.GroupNo=17;
            return(FALSE);
        }
        return(TRUE);
    }

    if (GroupTrn.GroupNo==14)//14组——参数遥测的门限值，上下限；分组召唤时发送，总召唤或背景数据不发。
    {
        //参数信息体地址检测
        if (GroupTrn.InfoAddr<LPARA)//LPARA=0x5001
            GroupTrn.InfoAddr=LPARA;
        Num=GroupTrn.InfoAddr-LPARA+1;
        if ((Num>DevList[GroupTrn.DevIndex].DevData.AINum*3)||(GroupTrn.InfoAddr>HPARA))//每个遥测3个参数：门限、上限、下限
        {
            GroupTrn.GroupNo=17;
            return(FALSE);
        }
        return(TRUE);
    }

    if (GroupTrn.GroupNo==15)//15组——步位置信息
    {
        if (GroupTrn.InfoAddr<LSPI)//LSPI=0x6601
            GroupTrn.InfoAddr=LSPI;
        Num=GroupTrn.InfoAddr-LSPI+1;
        if ((Num>DevList[GroupTrn.DevIndex].DevData.SPINum)||(GroupTrn.InfoAddr>HSPI))
        {
            if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))
                GroupTrn.GroupNo=16;
            else
                GroupTrn.GroupNo=17;

            return(FALSE);
        }
        return(TRUE);
    }

    if (GroupTrn.GroupNo==16)//16组——子站状态
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
                
                logSysMsgNoTime("101soe召唤调试 start=%d, wptr=%d,max=%d",GroupTrn.SoeStartPtr,wptr,Num,0);
            }
            else
            {
                //错误
                logSysMsgNoTime("101soe召唤错误 wptr=%d,max=%d",wptr,Num,0,0);
                return FALSE;
            }
            
        }
        
        return(TRUE);
    }
    return(FALSE);
}

BOOL CSecAppSev::CheckDDGroup(void)
{
    INT16U Num;
    if (GroupTrnDD.InfoAddr<(GroupTrnDD.GroupNo-1)*GROUPBCRNUM+LBCR)
        GroupTrnDD.InfoAddr=(GroupTrnDD.GroupNo-1)*GROUPBCRNUM+LBCR;
    else if (GroupTrnDD.InfoAddr>=GroupTrnDD.GroupNo*GROUPBCRNUM+LBCR)
    {
        GroupTrnDD.GroupNo=GroupTrnDD.InfoAddr-LBCR;
        GroupTrnDD.GroupNo/=GROUPBCRNUM;
        GroupTrnDD.GroupNo++;
    }
    Num=GroupTrnDD.InfoAddr-LBCR+1;
    if (Num>DevList[GroupTrnDD.DevIndex].DevData.CounterNum)
    {
        GroupTrnDD.GroupNo=5;
        return(FALSE);
    }
    return(TRUE);
}

INT8U CSecAppSev::EnCodeAllData(INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT8U Len=0;
    int jj;

    *pNum = 0;

    TxMsg[CotLocation]=GroupTrn.COT;
    
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    
    *pTxInfoAddr    =LOBYTE((GroupTrn.InfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((GroupTrn.InfoAddr));

    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))   //遥信
    {
        Len=EnCodeAllYX(GroupTrn.DevIndex,BeginNo,EndNo,pNum);//从数据库取数据并组织发向链路层的消息。返回应用层报文的总长度，pNum为实际发出的数据单元数目。
        //return(Len);
    }
    else if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=12))    //遥测
    {
        Len=EnCodeAllYC(GroupTrn.DevIndex,BeginNo,EndNo,pNum);
        //return(Len);
    }
    else if (GroupTrn.GroupNo==14)//参数P_ME_NA\P_ME_NB\P_ME_NC
    {
        Len=EnCodeAllPara(GroupTrn.DevIndex,BeginNo,EndNo,pNum);
        //return(Len);
    }
    else if (GroupTrn.GroupNo==15)         //
    {
        TxMsg[0]=M_ST_NA;  //5，步位置信息。
        (*pTxVSQ) &= ~VSQ_SQ;

        //Len=EditTestSPI();//这条语句和下边的if语句测试时打开       根据coverity更改
        /*if (Len)
        {
            *LengthOut=Len;
            if (BalanMode)////平衡模式
            {
                if(GroupTrn.COT==BACK)//背景数据
                    *AppCommand=APP_SENDNOCON;
                else//总召唤或分组
                    *AppCommand=APP_SENDCON;
            }
            else//非平衡分组或总召唤或背景
            {
                *AppCommand=APP_SENDDATA;
                if(GroupTrn.COT!=BACK)//
                    *AppCommand|=APP_HAVEDATA1;
            }
        }*/
        //return(Len);
    }
    else if (GroupTrn.GroupNo==16)         //
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
INT16U CSecAppSev::EnCodeAllLastSoe(INT16U BeginNo)
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    BOOL HaveData=FALSE;        //后续是否有数据
    
    *LengthOut = 0;
    for (i=0;i<DevCount;i++)
    {
                
        //logSysMsgNoTime("SEC 编辑单点SOE开始",0,0,0,0);//  debug ll
        TxMsg[0]=M_SP_TA;   //带时标的单点信息
        if(Sec101Pad.SOEWithCP56Time == 1)
            TxMsg[0]=M_SP_TB;   //带长时标的单点信息
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[i].Addr>>(8*jj);
        (*pTxVSQ) = 0;

        TxMsg[CotLocation]=GroupTrn.COT;
                
        Num=0;
        if(DevList[i].Flag==1)
            Num=RSend_ReadSBIET(DevList[i].DevID,BeginNo,20,(struct BIEWithTimeData_t *)DBData,&WritePtr);//
        else
            Num=L_ReadSBIET(DevList[i].DevID,BeginNo,20,(struct BIEWithTimeData_t *)DBData,&WritePtr);

        if(Num>0)
        {
            SendNum=0;
            FramePos = 0-InfoAddrSize;
            p=(struct BIEWithTimeData_t *)DBData;
            Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
            
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
                    //状态转换：数据库中的遥信D7为状态，规约中D0为状态
                    if(p->Status&0x80)
                        Status=1;
                    else
                        Status=0;
                    
                    //写信息体地址
                    if(FramePos < 0)
                    {
                        *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                        *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                    }
                    else
                    {
                        pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//信息体地址
                        pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//信息体地址
                        if(InfoAddrSize == 3)
                            pTxData[FramePos+2] = 0;//信息体地址为3字节时，最好字节为0
                    }
                    FramePos += InfoAddrSize;
                    
                    //写状态
                    if((p->Status&BIACTIVEFLAG)==0)
                        pTxData[FramePos]=Status|P101_IV;//设置遥信状态字节
                    else
                        pTxData[FramePos]=Status;//设置遥信状态字节
                        
                    if(p->Status&SUBSTITUTEDFLAG)
                        pTxData[FramePos]|=P101_SB;//设置遥信状态字节
                    
                    FramePos++;
                    //写时间
                    AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
    
                    pTxData[FramePos++] = LOBYTE(time.MSecond);
                    pTxData[FramePos++] = HIBYTE(time.MSecond);
                    pTxData[FramePos++] = time.Minute;
    
                    if(TxMsg[0]==M_SP_TB)//长时标
                    {
                        pTxData[FramePos++] = time.Hour;
                        pTxData[FramePos++] = time.Day;
                        pTxData[FramePos++] = time.Month;
                        pTxData[FramePos++] = time.Year;
                    }
    
                    SendNum++;//发送个数
                    p++;
                    j++;
                    
                    if(FramePos>=Length)
                        break;
                }
                
                
            }
            
            //if(DBISOEnum>0)
            //    DBIDevIndex=i;
            
            if(SendNum>0)
            {
                
                Len=FramePos+AsduHeadLength;//应用层报文总长度
                LastDevIndex=i;
                LastFrame=Polling;
                DevList[i].Data1.SOENum = 0;       //DevList[i].Data1.SOENum 是标记已经发送的个数(或者已经处理的个数），用于对SOE读指针进行控制
                *pTxVSQ=SendNum;
                                
                *LengthOut = Len;      
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;

                //判断是否有后续数据要发送
                GroupTrn.SoeStartPtr +=  SendNum;
                
                HaveData = TRUE;
                
                /*if(HaveData == FALSE)
                {
                    //单点遥信都处理完了，再处理双点遥信。这时如果双点遥信出现问题，容易送不到主站
                    if(DBISOEnum >0)
                    {
                        HaveData = TRUE;
                    }
                    
                }*/

            }
            
        }
    }
    
    /*if((DBISOEnum>0) && (*LengthOut==0))
    {
        //先发送单点，所有单点都发送完了，再发送DBI出去。
        EnCodeDBISOE(); 
        
        if(DBISOEnum > 0)
            return TRUE;   
    }*/
       
    return(HaveData);
}
void CSecAppSev::EnCodeLC()
{
    switch(StoreRxTypeID)
    {
        case C_LC_CALL_YC_YX:    //控制方向液晶召唤遥测遥信数据
            switch(Qoi)
            {
                case SUMMON_YX:
                	if(!(RxVsq & 0x80))
                    {
                    	EnCodeLCYX(ActDevIndex,LCInforAddr,LCAmount);
                    }
                    else
                    {
                    	EnCodeLCYX_Discrete(ActDevIndex,LCAmount,Addresses_Discrete);
                    }
                    break;
                case SUMMON_YC:
                	if(!(RxVsq & 0x80))
                    {
                    	EnCodeLCYC(ActDevIndex,LCInforAddr,LCAmount);
                    }
                    else
                    {
                    	EnCodeLCYC_Discrete(ActDevIndex,LCAmount,Addresses_Discrete);
                    }
                    break;
                default:
                    break;          
            }
            break;
        case C_LC_CALL_SYSINFO_SOE:  //控制方向液晶召唤系统信息及SOE
            switch(Qoi)
            {
                case SUMMON_SYS_INFO:
                    EnCodeLCSysInfo(ActDevIndex,LCInforAddr,LCAmount);
                    break;
                case SUMMON_SOE:
                    EnCodeLCSOE(ActDevIndex,LCInforAddr,LCAmount);
                    break;
                default:
                    break;    
            }
            break;
        case C_LC_CALL_NAME_VER_CLOCK:  //控制方向液晶召唤遥信名称、遥测名称、版本信息、时钟信息
            switch(Qoi)
            {
                case SUMMON_YX_NAME:
                	if(!(RxVsq & 0x80))
                    {
                    	EnCodeLCYxName(ActDevIndex,LCInforAddr,LCAmount);
                    }
                    else
                    {
                    	EnCodeLCYxName_Discrete(ActDevIndex,LCAmount,Addresses_Discrete);
                    }
                    break;
                case SUMMON_YC_NAME:
                	if(!(RxVsq & 0x80))
                    {
                    	EnCodeLCYcName(ActDevIndex,LCInforAddr,LCAmount);
                    }
                    else
                    {
                    	EnCodeLCYcName_Discrete(ActDevIndex,LCAmount,Addresses_Discrete);
                    }
                    break;
                case SUMMON_SOFTWARE_VERSION:
                    EnCodeLCSoftwareVer(ActDevIndex);
                    break;
                case SUMMON_CLOCK:
                    EnCodeLCClock(ActDevIndex);
                    break; 
                default:
                    break;   
            }
            break;
        /*case C_LC_FAULT_RESET:     //液晶复位故障命令
            switch(Qoi)
            {
                case FAULT_RESET:
                    INT16U i;
                    i = (LCInforAddr - FirstFAyxInfoAdr) / 6;
                    if(LCAmount == 6)
                    {
                        ResetFaultInfo(i);
                    }
                    else if(LCAmount == 0xFFFF)
                    {
                        for(i = 0;i < MyConfig.feedernum;i++)
                        {
                            ResetFaultInfo(i);       
                        }
                    }
                    else
                    {
                        //LCflag = 1; 
                        ResetFaultInfo(i);
                        //LCflag = 0;              
                    }
                    break;
                case 50:
                    break;
                default:
                    break;    
            }
            break;*/
        case C_LC_SUMMON_PARA:
        	switch(Qoi & 0x03)
            {
            	case 1:             
            	case 2:
                    EnCodeLCCommunicatePara(ActDevIndex,Qoi,LCInforAddr);
                	break;
            	case CONTROLLERAPPPARA:
                	EnCodeLCFaultOrProtectPara(ActDevIndex,Qoi,LCInforAddr);
                	break;            
            }
        default:
            break;
    }        
}
INT8U CSecAppSev::EnCodeLCSysInfo(INT16U DevIndex,WORD OffSet,WORD Num)
{
    INT16U i,jj,Len = 0,num = 0,InfoAddr = 0x8000;
    INT16U TempWriPtr;
    struct ErrorInfoMsg_t *TempError;
    struct ErrorInfoMsg_t *pErrMsg;
    TempWriPtr = ErrorBuf->WriPtr;
    TempError =  ErrorBuf->Error;   
    TxMsg[0] = C_LC_CALL_SYSINFO_SOE;//设置需要发送的数据类型
    (*pTxVSQ) = VSQ_SQ;
    //logSysMsgNoTime("召唤系统信息",0,0,0,0);
    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);    
    
    if((OffSet - 1) >= TempWriPtr)
    {
        InfoAddr += TempWriPtr - (OffSet - 1) + ERRORBUFNUM;
    }
    else
    {
        InfoAddr += TempWriPtr - (OffSet - 1);
    }
    
    if((OffSet - 1) > TempWriPtr - 1)
    {
        pErrMsg = TempError + TempWriPtr - 1 - (OffSet - 1) + ERRORBUFNUM;
    }
    else    
    {
        pErrMsg = TempError + TempWriPtr - 1 - (OffSet - 1);
    }
    
    *pTxInfoAddr    = LOBYTE(InfoAddr);
    *(pTxInfoAddr+1) = HIBYTE(InfoAddr);
    
    for(i = 0;i < Num;i++)
    {
        if(strlen((char *)pErrMsg->Msg) == 0)
        {
            continue;
        }
        for(jj=0;jj<ERRORMSGLEN;jj++)
        {
            *(pTxData + i * ERRORMSGLEN + jj) =  pErrMsg->Msg[jj];
        }
        if(pErrMsg == TempError)
            pErrMsg += ERRORBUFNUM;
        pErrMsg--;
        num++;
    }
    
    if(num == 0)
    {
        //*pTxVSQ |= 1;
        TxMsg[CotLocation] = 0x87;
        *pTxInfoAddr    = 0x01;
        *(pTxInfoAddr+1) = 0x80;
        for(i = 0;i < ERRORMSGLEN;i++)
        {
            *(pTxData + i) = 0;
        }
    }
    
    Len = (num == 0 ? 1 : num) * ERRORMSGLEN + AsduHeadLength;
    *LengthOut=Len;
    if (!BalanMode)//平衡模式
    {
        *AppCommand=APP_SENDDATA;
    }
    
    *pTxVSQ |= num;
    
    return(Len);
}

INT8U CSecAppSev::EnCodeLCSOE(INT16U DevIndex,WORD OffSet,WORD Num)
{
    INT16U i,Len = 0,InfoAddr = 0x8400;
    INT16 jj;
    struct Iec101ClockTime_t time;
    struct BIEWithTimeData_t *TempBuf = (struct BIEWithTimeData_t *)LCBuf;
    
    
    
    if(LC_LBuf_ReadSBIET(DevList[DevIndex].DevID,OffSet,Num,(struct BIEWithTimeData_t *)LCBuf,&InfoAddr) == -1)
        return(Len);
        
    TxMsg[0] = C_LC_CALL_SYSINFO_SOE;//设置需要发送的数据类型
    (*pTxVSQ) = VSQ_SQ;

    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);

    //pTxData -= 2;
    *pTxInfoAddr    = LOBYTE(InfoAddr);
    *(pTxInfoAddr+1) = HIBYTE(InfoAddr);
    jj = 0;
    for(i = 0;i < Num;i++)
    {
        //SOE点号
        if(TempBuf->Status == 0 && TempBuf->No == 0 && TempBuf->Time.Minute == 0 && TempBuf->Time.MSecond == 0)
        { 
            TempBuf++;
            continue;
        }  
        else
        {    
            *(pTxData + i * 10) = LOBYTE(TempBuf->No + 1);
            *(pTxData + i * 10 + 1) =HIBYTE(TempBuf->No + 1);
            jj++;
        }
        //SOE状态
        *(pTxData + i * 10 + 2) = TempBuf->Status;
        //SOE时标
        if(TempBuf->Time.Minute == 0 && TempBuf->Time.MSecond == 0)
            memset(&time,0,sizeof(struct Iec101ClockTime_t));
        else
            AbsTimeConvTo(&(TempBuf->Time),(void*)&time,IEC101CLOCKTIME);
        *(pTxData + i * 10 + 3)   = LOBYTE(time.MSecond);
        *(pTxData + i * 10 + 4) = HIBYTE(time.MSecond);
        *(pTxData + i * 10 + 5) = time.Minute;
        *(pTxData + i * 10 + 6) = time.Hour;
        *(pTxData + i * 10 + 7) = time.Day;
        *(pTxData + i * 10 + 8) = time.Month;
        *(pTxData + i * 10 + 9) = time.Year;

        TempBuf++;
     }
    
    
    if(jj == 0)
    {
        //*pTxVSQ |= 1;
        TxMsg[CotLocation] = 0x87;
        *pTxInfoAddr    = 0x01;
        *(pTxInfoAddr+1) = 0x84;
        for(i = 0;i < 10;i++)
        {
            *(pTxData + i) = 0;
        }
    }
    
    Len = (jj == 0 ? 1 : jj) * 10 + AsduHeadLength;
    *LengthOut=Len;
    if (!BalanMode)//平衡模式
    {
        *AppCommand=APP_SENDDATA;
    }
    
    *pTxVSQ |= jj;
    
    return(Len);
}

INT8U CSecAppSev::EnCodeLCCOS(INT16U DevIndex,WORD OffSet,WORD Num)
{
    INT16U i,Len = 0,InfoAddr = 0x8800;
    INT16 jj;
    
    struct BIEWithoutTimeData_t *TempBuf = (struct BIEWithoutTimeData_t *)LCBuf;
    
   //LC_LBuf_ReadSBIENT(DevIndex,OffSet,Num,(struct BIEWithoutTimeData_t *)LCBuf);
    
    if(LC_LBuf_ReadSBIENT(DevList[DevIndex].DevID,OffSet,Num,(struct BIEWithoutTimeData_t *)LCBuf,&InfoAddr) == -1)
        return(Len);
    
    TxMsg[0] = C_LC_CALL_SYSINFO_SOE;//设置需要发送的数据类型
    (*pTxVSQ) = VSQ_SQ;

    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
        
    *pTxInfoAddr    = LOBYTE(InfoAddr);
    *(pTxInfoAddr+1) = HIBYTE(InfoAddr);
    //pTxData -= 2;
    jj = 0;
    for(i = 0;i < Num;i++)
    {
        //SOE点号
        if(TempBuf->Status == 0 && TempBuf->No == 0)
        {
            TempBuf++;
            *(pTxData + i * 3) = 0;
            *(pTxData + i * 3 + 1) = 0;
            *(pTxData + i * 3 + 2) = 0;
            continue;    
        }
        else
        {
            *(pTxData + i * 3) = LOBYTE(TempBuf->No + 1);
            *(pTxData + i * 3 + 1) =HIBYTE(TempBuf->No + 1);
        //SOE状态
            *(pTxData + i * 3 + 2) = TempBuf->Status;
            jj++;
        }

        TempBuf++;
        
     }
     
    if(jj == 0)
    {
        //*pTxVSQ |= 1;
        TxMsg[CotLocation] = 0x87;
        *pTxInfoAddr    = 0x01;
        *(pTxInfoAddr+1) = 0x88;
        for(i = 0;i < 3;i++)
        {
            *(pTxData + i) = 0;
        }
    }
    
    Len = (jj == 0 ? 1 : jj) * 3 + AsduHeadLength;
    *LengthOut=Len;
    if (!BalanMode)//平衡模式
    {
        *AppCommand=APP_SENDDATA;
    }
    
    *pTxVSQ |= jj;
    
    return(Len);
}

INT8U CSecAppSev::EnCodeLCYcName_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YcName_Addresses)
{
    INT16U i,jj,Len = 0;
    INT8U *pTemp;
    INT16U num = 0;
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//设置需要发送的数据类型
    (*pTxVSQ) = 0x00;

    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
        
    //*pTxInfoAddr = LOBYTE(FirstInfoAddr);
    //*(pTxInfoAddr + 1) = HIBYTE(FirstInfoAddr);
    pTemp = pTxData - 2;
    
    //for(i = (FirstInfoAddr & 0x0FFF) - 1;(i < (FirstInfoAddr & 0x0FFF) - 1 + Amount) && (i < YcNameList[DevList[DevIndex].DevID].Num);i++)
    for(i=0;((i<Amount) && (((YcName_Addresses[i] & 0x0FFF) - 1) < YcNameList[DevList[DevIndex].DevID].Num));i++)
    {
        memset(pTemp,0,26);
        *pTemp++ = LOBYTE(YcName_Addresses[i]);
        *pTemp++ = HIBYTE(YcName_Addresses[i]);
        memcpy((void *)pTemp,(void*)(&YcNameList[DevList[DevIndex].DevID].Name[(YcName_Addresses[i] & 0x0FFF) - 1]),10);
        pTemp += 24;
        num++;
     }
    
     if(num == 0)
     {
        //(*pTxVSQ) |= 1;
        TxMsg[CotLocation]=0x87;
        *pTxInfoAddr = 0x01;
        *(pTxInfoAddr + 1) = 0x40;
        memset(pTemp,0,24);
     } 
    
     Len = (num == 0 ? 1 : num) * 26 + AsduHeadLength - 2;
     *LengthOut=Len;
     if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
     
    *pTxVSQ |= num;
    
    return(Len);
}
INT8U CSecAppSev::EnCodeLCYcName(INT16U DevIndex,INT16U FirstInfoAddr,INT16U Amount)
{
    INT16U i,jj,Len = 0;
    INT8U *pTemp;
    INT16U num = 0;
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//设置需要发送的数据类型
    (*pTxVSQ) = 0x80;

    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
        
    *pTxInfoAddr = LOBYTE(FirstInfoAddr);
    *(pTxInfoAddr + 1) = HIBYTE(FirstInfoAddr);
    pTemp = pTxData;
    
    for(i = (FirstInfoAddr & 0x0FFF) - 1;(i < (FirstInfoAddr & 0x0FFF) - 1 + Amount) && (i < YcNameList[DevList[DevIndex].DevID].Num);i++)
    {
        memset(pTemp,0,24);
        memcpy((void *)pTemp,(void*)(&YcNameList[DevList[DevIndex].DevID].Name[i]),10);
        pTemp += 24;
        num++;
     }
    
     if(num == 0)
     {
        //(*pTxVSQ) |= 1;
        TxMsg[CotLocation]=0x87;
        *pTxInfoAddr = 0x01;
        *(pTxInfoAddr + 1) = 0x40;
        memset(pTemp,0,24);
     } 
    
     Len = (num == 0 ? 1 : num) * 24 + AsduHeadLength;
     *LengthOut=Len;
     if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
     
    *pTxVSQ |= num;
    
    return(Len);
}

INT8U CSecAppSev::EnCodeLCYxName_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YxName_Addresses)
{
    INT16U i,jj,Len = 0;
    INT8U *pTemp;
    INT16U num = 0;
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//设置需要发送的数据类型
    (*pTxVSQ) = 0x00;

    TxMsg[CotLocation]=07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    
    //*pTxInfoAddr = LOBYTE(FirstInfoAddr);
    //*(pTxInfoAddr + 1) = HIBYTE(FirstInfoAddr);
    pTemp = pTxData - 2;
    
    //for(i = FirstInfoAddr - 1;(i < FirstInfoAddr - 1 + Amount) && (i < YxNameList[DevList[DevIndex].DevID].Num);i++)
    for(i=0;((i<Amount) && (((YxName_Addresses[i] & 0x0FFF) - 1) < YxNameList[DevList[DevIndex].DevID].Num));i++)
    {
    	memset(pTemp,0,26);
        *pTemp++ = LOBYTE(YxName_Addresses[i]);
        *pTemp++ = HIBYTE(YxName_Addresses[i]);
        memcpy((void *)pTemp,(void*)(&YxNameList[DevList[DevIndex].DevID].Name[(YxName_Addresses[i] & 0x0FFF) - 1]),10);
        pTemp += 24;
        num++;
     }
     
     if(num == 0)
     {
        (*pTxVSQ) |= 1;
        TxMsg[CotLocation]=0x87;
        *pTxInfoAddr = 0x01;
        *(pTxInfoAddr + 1) = 0x00;
        memset(pTemp,0,24);
     }
     
     Len = (num == 0 ? 1 : num) * 26 + AsduHeadLength - 2;
     *LengthOut=Len;
     if (!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
    
    *pTxVSQ |= num;
    
    return(Len);
}
INT8U CSecAppSev::EnCodeLCYxName(INT16U DevIndex,INT16U FirstInfoAddr,INT16U Amount)
{
    INT16U i,jj,Len = 0;
    INT8U *pTemp;
    INT16U num = 0;
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//设置需要发送的数据类型
    (*pTxVSQ) = 0x80;

    TxMsg[CotLocation]=07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    
    *pTxInfoAddr = LOBYTE(FirstInfoAddr);
    *(pTxInfoAddr + 1) = HIBYTE(FirstInfoAddr);
    pTemp = pTxData;
    
    for(i = FirstInfoAddr - 1;(i < FirstInfoAddr - 1 + Amount) && (i < YxNameList[DevList[DevIndex].DevID].Num);i++)
    {
        memset(pTemp,0,24);
        memcpy((void *)pTemp,(void*)(&YxNameList[DevList[DevIndex].DevID].Name[i]),10);
        

        pTemp += 24;
        num++;
     }
     
     if(num == 0)
     {
        (*pTxVSQ) |= 1;
        TxMsg[CotLocation]=0x87;
        *pTxInfoAddr = 0x01;
        *(pTxInfoAddr + 1) = 0x00;
        memset(pTemp,0,24);
     }
     
     Len = (num == 0 ? 1 : num) * 24 + AsduHeadLength;
     *LengthOut=Len;
     if (!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
    
    *pTxVSQ |= num;
    
    return(Len);
}

void CSecAppSev::ProcLCPanelDriver(INT8U control,INT8U command)
{
	BspLCPanelDriver(control,command);
    
}
INT8U CSecAppSev::EnCodeLCYX_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YX_Addresses)
{
    INT16U i,jj,devid,Len = 0;
    short FramePos,SendNum,YxNum;
    INT16U BeginNo;
    INT16U EndNo;
    INT8U *pTxInfoObjectTemp = pTxInfoAddr;

    TxMsg[0] = M_SP_NA;
    (*pTxVSQ) = 0x00;

    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);

    devid=DevList[DevIndex].DevID;
    //组帧发送
    FramePos=0;
    SendNum=0;
    YxNum=0;
    //Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
    
    for(i=0;i<Amount;i++)
    {
    	pTxInfoObjectTemp[FramePos++] = LOBYTE(YX_Addresses[i]);
        pTxInfoObjectTemp[FramePos++] = HIBYTE(YX_Addresses[i]);
    	BeginNo = YX_Addresses[i] - 0x0001;
    	EndNo = BeginNo;
    	CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
    	if((DBData[0]&BIACTIVEFLAG)==0)
            pTxInfoObjectTemp[FramePos]=((DBData[0]&0x80)>>7)|P101_IV;//数据库D7为遥信状态,置无效位
        else
            pTxInfoObjectTemp[FramePos]=((DBData[0]&0x80)>>7);//数据库D7为遥信状态
            
        if(DBData[0]&SUBSTITUTEDFLAG)
            pTxInfoObjectTemp[FramePos]|=P101_SB;//数据库D7为遥信状态,置无效位
        
            
        FramePos++;
        SendNum++;
        YxNum++;
    }    
    
    if(SendNum>0)
    {
        Len=FramePos+AsduHeadLength-2;
        *LengthOut=Len;
       
        if(!BalanMode)
        {
            *AppCommand=APP_SENDDATA;
        }
    }
    *pTxVSQ |= YxNum;
    
    return(Len);
}

INT8U CSecAppSev::EnCodeLCYX(INT16U DevIndex,INT16U FirstInfoAddr,INT16U Amount)
{
    INT16U i,jj,devid,Len = 0;
    short ByteNum,Length,SendNum,FramePos,YxNum;
    INT16U BeginNo = FirstInfoAddr - 1;
    INT16U EndNo = BeginNo + Amount - 1;
    TxMsg[0]= M_SP_NA;//设置需要发送的数据类型
    (*pTxVSQ) = VSQ_SQ;

    TxMsg[CotLocation]=0x07;
    
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    *pTxInfoAddr    =LOBYTE(FirstInfoAddr);
    *(pTxInfoAddr+1)=HIBYTE(FirstInfoAddr);
    
    devid=DevList[DevIndex].DevID;

    FramePos=0;
    SendNum=0;
    YxNum=0;
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
        
    ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
    for(i=0;i<ByteNum;i++)
    {
        if((DBData[i]&BIACTIVEFLAG)==0)
            pTxData[FramePos]=((DBData[i]&0x80)>>7)|P101_IV;//数据库D7为遥信状态,置无效位
        else
            pTxData[FramePos]=((DBData[i]&0x80)>>7);//数据库D7为遥信状态
            
        if(DBData[i]&SUBSTITUTEDFLAG)
            pTxData[FramePos]|=P101_SB;//数据库D7为遥信状态,置无效位
        
            
        FramePos++;    
        SendNum++;
        YxNum++;
        if((FramePos>=Length)||(SendNum>=127))
            break;
    }
    
    
    if(SendNum>0)
    {
        Len=FramePos+AsduHeadLength;
        *LengthOut=Len;
       
        if(!BalanMode)
        {
            *AppCommand=APP_SENDDATA;
        }
    }
    *pTxVSQ |= YxNum;
    
    return(Len);
}

INT8U CSecAppSev::EnCodeLCYC_Discrete(INT16U DevIndex,INT16U Amount,INT16U *YC_Addresses)
{
    INT16U i,jj,devid,Len = 0;
    short Value,FramePos;
    INT8U *pTxInfoObjectTemp = pTxInfoAddr;
    

    TxMsg[0] = M_ME_NB;
    (*pTxVSQ) = 0x00;

    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);

    devid=DevList[DevIndex].DevID;
    //组帧发送
    //Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
    FramePos=0;
    for(i = 0;i < Amount;i++)
    {
    	pTxInfoObjectTemp[FramePos++] = LOBYTE(YC_Addresses[i]);
        pTxInfoObjectTemp[FramePos++] = HIBYTE(YC_Addresses[i]);
        Value = SL_ReadAI(devid,YC_Addresses[i]-0x4001);
        Value = (short)SL_ReadAI_S(devid,YC_Addresses[i]-0x4001,Value);
        
        pTxInfoObjectTemp[FramePos++] = LOBYTE(Value);
        pTxInfoObjectTemp[FramePos++] = HIBYTE(Value);
        pTxInfoObjectTemp[FramePos++] = 0x00;
    }
    if (FramePos<=0)
        return (0);

    *pTxVSQ |= Amount;

    Len=FramePos+AsduHeadLength-2;//应用层报文的总长度
    if (Len)
    {
        //Len=FramePos+AsduHeadLength-2;//应用层报文的总长度
        *LengthOut=Len;
         if(!BalanMode)
        {
            *AppCommand=APP_SENDDATA;
        }    
    }
    return(Len);
}
INT8U CSecAppSev::EnCodeLCYC(INT16U DevIndex,INT16U FirstInfoAddr,INT16U Amount)
{
    INT16U i,jj,devid,No,Len = 0;
    INT8U Num = 0;
    short Value,AINum,Length,FramePos;
    struct RealAI_t *pAIValue;
    BOOL Stop = FALSE;
    INT16U BeginNo = (FirstInfoAddr & 0x0FFF) - 1;
    INT16U EndNo = BeginNo + Amount - 1;

    TxMsg[0] = M_ME_NB;
    (*pTxVSQ) = VSQ_SQ;

    TxMsg[CotLocation]=0x07;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);

    *pTxInfoAddr    = LOBYTE(FirstInfoAddr);
    *(pTxInfoAddr+1)= HIBYTE(FirstInfoAddr);

    devid=DevList[DevIndex].DevID;

    FramePos=0;

    //取出数据
    No=BeginNo;
    while (No<=EndNo)
    {
        
        AINum=CLF_ReadAI(devid,No,No+511,(struct RealAI_t *)DBData);

        if (AINum<0)
        {//return(0);
            EndNo = No - 1;   
            break;
        }
        pAIValue=(struct RealAI_t*)DBData;
        for(i=0;i<AINum;i++)
        {
            Value=pAIValue->Value;
            
            DevList[DevIndex].DevData.AIData[No].Flag=pAIValue->Flag;
            DevList[DevIndex].DevData.AIData[No].TempValue=Value;
            DevList[DevIndex].DevData.AIData[No].Value=Value;
            No++;
            pAIValue++;
        }
    }//end of (while (No<=EndNo))

    //组帧发送
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
    FramePos=0;
    No=BeginNo;
    while ((FramePos<Length)&&(!Stop))
    {
        Value=DevList[DevIndex].DevData.AIData[No].TempValue;
        Value = (short)SL_ReadAI_S(DevList[DevIndex].DevID, No, Value);
        pTxData[FramePos++]=LOBYTE(Value);
        pTxData[FramePos++]=HIBYTE(Value);
        if(DevList[DevIndex].DevData.AIData[No].Flag==AIACTIVEFLAG)
            pTxData[FramePos++]=0;//flag
        else
            pTxData[FramePos++]=0|P101_IV;//flag
                    
        Num++;
        No++;
        if(No>EndNo)
            Stop=TRUE;
        if (Num>=127)//每帧不超过127个数据单元
            Stop=TRUE;
    }
    if (FramePos<=0)
        return (0);

    *pTxVSQ |= Num;

    Len=FramePos+AsduHeadLength;//应用层报文的总长度
    if (Len)
    {
        Len=FramePos+AsduHeadLength;//应用层报文的总长度
        *LengthOut=Len;
         if(!BalanMode)
        {
            *AppCommand=APP_SENDDATA;
        }    
    }
    return(Len);
}
/*------------------------------------------------------------------------
Procedure:     EnCodeLCFaultOrProtectPara
Purpose:       根据召唤参数进行组帧
Input:         DevIndex，设备的索引号，qoi表明召唤的参数类型，LCInforAddr召唤参数文件的地址
Output:无            
Author:        lw
Date:          2018.9.04改写
------------------------------------------------------------------------*/
void CSecAppSev::EnCodeLCFaultOrProtectPara(INT16U DevIndex,INT8U qoi,INT16U LCInforAddr)
{
    //INT8U type ;
	INT8U index;
	TxMsg[0]=C_LC_SUMMON_PARA;
	TxMsg[1]=0x01;
	TxMsg[CotLocation]=07;
	for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
        
    *pTxInfoAddr    =LOBYTE(LCInforAddr);
    *(pTxInfoAddr+1)=HIBYTE(LCInforAddr);
    
    //type = (qoi >> 2) & 0x03;
	index =  LCInforAddr - 0x8831;
	pTxData[0] = sizeofPara(qoi);
	ParaToASDU(pTxData,index,qoi);
    /*switch((qoi >> 2) & 0x03)
    {
    	INT8U index ;
    	case CONTROLLERPROTECTPARA:
        	index = LCInforAddr - 0x8831;
        	pTxData[0] = sizeofPara(CONTROLLERPROTECTPARA);
        	ParaToASDU(pTxData,index,CONTROLLERPROTECTPARA);
        	break;
    	case CONTROLLERFAULTDETECTPARA:
            
        	break;
    	case CONTROLLERVOLTAGETIMEPARA:
        	index = LCInforAddr - 0x8831;
        	pTxData[0] = sizeofPara(CONTROLLERVOLTAGETIMEPARA);
        	ParaToASDU(pTxData,index,CONTROLLERVOLTAGETIMEPARA);
        	break;
    	case CONTROLLERVOLTAGECURRENTPARA:
        	index = LCInforAddr - 0x8831;
        	pTxData[0] = sizeofPara(CONTROLLERVOLTAGECURRENTPARA);
        	ParaToASDU(pTxData,index,CONTROLLERVOLTAGECURRENTPARA);
        	break;
    	default:
        	break;
    }*/
    *LengthOut=AsduHeadLength + pTxData[0] + 1; 
	if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
}

void CSecAppSev::EnCodeLCCommunicatePara(INT16U DevIndex,INT8U qoi,INT16U LCInforAddr)
{
    
    //INT8U index;
	TxMsg[0]=C_LC_SUMMON_PARA;
	TxMsg[1]=0x01;
	TxMsg[CotLocation]=07;
	for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
        
    *pTxInfoAddr    =LOBYTE(LCInforAddr);
    *(pTxInfoAddr+1)=HIBYTE(LCInforAddr);
    
	pTxData[0] = sizeofPara(qoi);
	ParaToASDU(pTxData,0,qoi);
    
    *LengthOut=AsduHeadLength + pTxData[0] + 1; 
	if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
}
void CSecAppSev::EnCodeParaMirror(INT16U DevIndex,INT8U Length)
{
	TxMsg[0]=C_LC_SET_PARA;//151,液晶设定参数
    TxMsg[1]=filenum;
    TxMsg[CotLocation]=07;//6，激活
    
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    
    memcpy((void*)pTxInfoAddr,paramirrorbuf,Length);
    
    *LengthOut=AsduHeadLength + Length - 2;
     if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
}
void CSecAppSev::EnCodeActivatePara(INT16U DevIndex)
{
	TxMsg[0]=C_LC_ACTIVATE_PARA;//103，时钟同步
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6，激活
    
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0x00;
    *(pTxInfoAddr+1)=0x00;
    *(pTxData)   = 0x00;
    *LengthOut=AsduHeadLength + 1;
    if(!BalanMode)
     {
        *AppCommand=APP_RESETMACHINE;
     }
}
void CSecAppSev::EnCodeLightStatus(INT16U DevIndex)
{
	INT8U length;
	TxMsg[0]=C_LC_PANEL_DRIVER;//103，时钟同步
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6，激活
    
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0x00;
    *(pTxInfoAddr+1)=0x00;
    length = sizeofProtectLightStatus();
    pTxData[0] = length;
    ProtectLightStatusToASDU(pTxData);
    
    pTxData[length+1] = 55;
    *LengthOut=AsduHeadLength + length + 2;
    if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
}
void CSecAppSev::EnCodeSummonInfoOnBoot(INT16U DevIndex)
{
	INT8U length;
	TxMsg[0]=C_LC_PANEL_DRIVER;//103，时钟同步
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6，激活
    
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0x00;
    *(pTxInfoAddr+1)=0x00;
    length = sizeofInfoSummoned();
    pTxData[0] = length;
    InfoSummonedToASDU(pTxData);
    
    pTxData[length+1] = 54;
    *LengthOut=AsduHeadLength + length+2;
    if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
}
void CSecAppSev::EnCodeLCClock(INT16U DevIndex)
{
    struct Iec101ClockTime_t time;

    TxMsg[0]=C_LC_CALL_NAME_VER_CLOCK;//103，时钟同步
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6，激活
    
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0x02;
    *(pTxInfoAddr+1)=0xF0;

    
    GetSysTime((void*)&time,IEC101CLOCKTIME);

    *(pTxData)   = LOBYTE(time.MSecond);
    *(pTxData+1) = HIBYTE(time.MSecond);
    *(pTxData+2) = time.Minute;
    *(pTxData+3) = time.Hour;
    *(pTxData+4) = time.Day;
    *(pTxData+5) = time.Month;
    *(pTxData+6) = time.Year;
    
    *LengthOut=AsduHeadLength + 7;
     if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
}

void CSecAppSev::EnCodeLCSoftwareVer(INT16U DevIndex)
{
    
    //struct imHead_t *imageHead;
    //imageHead = getSysVersion();
    
    //getSysVersion(&imageHead);
    TxMsg[0]=C_LC_CALL_NAME_VER_CLOCK;
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;

    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0x01;
    *(pTxInfoAddr+1)=0xF0;

    //memset(pTxData,0,120);
    memset(pTxData,0,60);
    //*(pTxData)   = 'v';
    getSysVersion(pTxData);
    //memcpy( pTxData + 1, imageHead->version,8);
    //memcpy(pTxData + 60,imageHead->explain,16);
    *LengthOut=AsduHeadLength + 60;
     if(!BalanMode)
     {
        *AppCommand=APP_SENDDATA;
     }
}
INT8U CSecAppSev::EnCodeAllYX(INT16U DevIndex,INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT16U i,jj,SBINum,devid,Len = 0;
    short ByteNum,Length,SendNum,FramePos,YxNum;
    INT8U Status;
    INT16U yxSendno;

    TxMsg[0] = Sec101Pad.TypeID[GroupTrn.GroupNo-1];//设置需要发送的数据类型
    TxMsg[1] = VSQ_SQ;
    
    if(GroupTrn.HaveSendDBI == TRUE)
    {
    	TxMsg[0] = M_DP_NA; 
    }
    
    if(bSendAllDBI)
        TxMsg[0] = M_DP_NA_ALLDBI;

    TxMsg[CotLocation]=GroupTrn.COT;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    *pTxInfoAddr    =LOBYTE((GroupTrn.InfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((GroupTrn.InfoAddr));
    
    
    devid=DevList[GroupTrn.DevIndex].DevID;

    FramePos=0;
    SendNum=0;
    YxNum=0;
    Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234为应用层发送信息最大长度
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
            pTxData[FramePos++]=DBData[2*i];
            pTxData[FramePos++]=DBData[2*i+1];
            pTxData[FramePos++]=0;
            pTxData[FramePos++]=0;
            pTxData[FramePos++]=0;
            YxNum++;
            SendNum+=16;                             //wjr
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
        
        for(i=0;i<ByteNum;i++)
        {
            SendNum++;
            if((DBData[i] & BIDBI_STATUSE) == 0)    //非双点遥信，则按单点遥信发送
            {
                if((DBData[i]&BIACTIVEFLAG)==0)
                    pTxData[FramePos]=((DBData[i]&0x80)>>7)|P101_IV;//数据库D7为遥信状态,置无效位
                else
                    pTxData[FramePos]=((DBData[i]&0x80)>>7);//数据库D7为遥信状态
                    
                if(DBData[i]&SUBSTITUTEDFLAG)
                    pTxData[FramePos]|=P101_SB;
                    
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
            
            if((FramePos>=Length)||(YxNum>=127))
                break;
        }
        
    }
    else if(TxMsg[0]==M_DP_NA)  //3-双点byte     
    {
        TxMsg[1]= 0;    //非顺序元素
        
    	EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1;     //如果遥信个数过大，超过DBData缓冲区，那么需要循环读
    	if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        FramePos = InfoAddrLocation;    //调整到信息体地址位置
        for(i=0;i<(ByteNum);i++)
        {
            SendNum ++;
            
            if(DBData[i] & BIDBI_STATUSE)
            {
            
                yxSendno = GroupTrn.InfoAddr+i-LBIinfoaddr;   //计算本次的遥信发送序号，从0开始计算（GroupTrn.InfoAddr目前是记录的从LBIinfoaddr开始的序号，用于控制传送遥信的当前位置） ll 21-03-28
                TxMsg[FramePos++] = LOBYTE((yxSendno+LDBIinfoaddr));//信息体地址
                TxMsg[FramePos++] = HIBYTE((yxSendno+LDBIinfoaddr));
                if((DBData[i]&BIACTIVEFLAG)==0)
                    TxMsg[FramePos] = ((DBData[i]&0x60)>>5)|P101_IV;
                else
                    TxMsg[FramePos] = ((DBData[i]&0x60)>>5);
                    
                if(DBData[i]&SUBSTITUTEDFLAG)
                    TxMsg[FramePos] |= P101_SB;
                
                FramePos++;
                YxNum++;
                if((FramePos>=Length)||(YxNum>=127))
                    break;
            }
        }
        
        FramePos -= AsduHeadLength; //为兼容单点遥信程序，调整FramePos的大小为去掉AsduHeadLength大小
        
    }
    else if(TxMsg[0] == M_DP_NA_ALLDBI)  //广东要求的全双点遥信发送，特殊处理
    {
        TxMsg[0] = M_DP_NA;
                
    	EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1;     //如果遥信个数过大，超过DBData缓冲区，那么需要循环读
    	if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        //FramePos = InfoAddrLocation;    //调整到信息体地址位置
        for(i=0;i<(ByteNum);i++)
        {
            SendNum ++;
            
            if(DBData[i]&0x80)
                Status=(BIDBI_YXH>>5);
            else
                Status=(BIDBI_YXF>>5);
            
            if((DBData[i]&BIACTIVEFLAG)==0)
                pTxData[FramePos] = Status|P101_IV;
            else
                pTxData[FramePos] = Status;
                
            if(DBData[i]&SUBSTITUTEDFLAG)
                pTxData[FramePos] |= P101_SB;
            
            FramePos++;
            YxNum++;
            if((FramePos>=Length)||(YxNum>=127))
                break;
            
        }
        
    }
    
    if(SendNum>0)
    {
        *pNum = SendNum;
        
        if(YxNum > 0)
        {
            TxMsg[1] |= YxNum;
            Len = FramePos+AsduHeadLength;
            *LengthOut = Len;
            
            if (BalanMode)//平衡模式
            {
                if(GroupTrn.COT==BACK)
                    *AppCommand=APP_SENDNOCON;
                else
                    *AppCommand=APP_SENDCON;
            }
            else//非平衡，可能是总召唤、分组、背景数据
            {
                *AppCommand=APP_SENDDATA;
                if(GroupTrn.COT!=BACK)//背景数据不设置一级数据标志
                    *AppCommand|=APP_HAVEDATA1;
            }
        }
        else
        {
            *LengthOut = 0;
            Len = SendNum;    //返回真值下步继续处理
            *AppCommand = APP_NOJOB;
            if(BalanMode)
            {
                myEventSend(myTaskIdSelf(),FORCESCHEDULE);  //由于没发任何数据，调度进行下一次发送
            }
        }
        
    }

    return(Len);
}

INT8U CSecAppSev::EnCodeAllYC(INT16U DevIndex,INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    INT16U i,jj,devid,No,Len = 0;
    short Value,AINum,Length,FramePos;
    struct RealAI_t *pAIValue;
    float temp;
    INT8U *p,*pdd;
    INT32U dd;
    BOOL Stop = FALSE;

    TxMsg[0]=Sec101Pad.TypeID[GroupTrn.GroupNo-1];//设置需要发送的数据类型
    (*pTxVSQ) = VSQ_SQ;

    TxMsg[CotLocation]=GroupTrn.COT;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    *pTxInfoAddr    =LOBYTE((GroupTrn.InfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((GroupTrn.InfoAddr));

    devid=DevList[GroupTrn.DevIndex].DevID;

    FramePos=0;

    //取出数据
    No=BeginNo;
    while (No<=EndNo)
    {
        if(DevList[GroupTrn.DevIndex].Flag==1)
            AINum=CRFSend_ReadAI(devid,No,No+511,(struct RealAI_t *)DBData);
        else
            AINum=CLF_ReadAI(devid,No,No+511,(struct RealAI_t *)DBData);

        if (AINum<0)
            return(0);
        pAIValue=(struct RealAI_t*)DBData;
        for(i=0;i<AINum;i++)
        {
            Value=pAIValue->Value;
            switch(TxMsg[0])
            {
                case M_ME_NA:     //9测量值，规一化值
                case M_ME_ND:
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag=pAIValue->Flag;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Value=Value;
                    //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
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
    while ((FramePos<Length)&&(!Stop))
    {
        switch(TxMsg[0])
        {
            case M_ME_NA:     //9测量值，规一化值
            case M_ME_NB:     //11测量值，标度值
                Value = (short)SL_ReadAI_S(DevList[GroupTrn.DevIndex].DevID, No, DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue);
                //Value=DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue;
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                
                pTxData[FramePos]=0;
                if((DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIACTIVEFLAG)==0)
                    pTxData[FramePos] |= P101_IV;
                if (DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIOVERAGE)
                    pTxData[FramePos] |= P101_OV;
                FramePos++;
                    
                (*pNum)++;

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
                pTxData[FramePos++]=LLBYTE(dd);
                pTxData[FramePos++]=LHBYTE(dd);
                pTxData[FramePos++]=HLBYTE(dd);
                pTxData[FramePos++]=HHBYTE(dd);
                
                pTxData[FramePos]=0;
                if((DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIACTIVEFLAG)==0)
                    pTxData[FramePos] |= P101_IV;
                if (DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag & AIOVERAGE)
                    pTxData[FramePos] |= P101_OV;
                FramePos++;
                    
                (*pNum)++;

                break;
            case M_ME_ND:
                Value = (short)SL_ReadAI_S(DevList[GroupTrn.DevIndex].DevID, No, DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue);
                //Value=DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue;
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                (*pNum)++;

                break;
        }
        No++;
        if(No>EndNo)
            Stop=TRUE;
        if (*pNum>=127)//每帧不超过127个数据单元
            Stop=TRUE;
    }
    if (FramePos<=0)
        return (0);

    *pTxVSQ |= (*pNum);

    Len=FramePos+AsduHeadLength;//应用层报文的总长度
    if (Len)
    {
        *LengthOut=Len;
        if (BalanMode)//平衡模式,总召唤或周期循环、或背景数据
        {
            if((GroupTrn.COT==BACK)||(GroupTrn.COT==PERCYC))
                *AppCommand=APP_SENDNOCON;
            else
                *AppCommand=APP_SENDCON;
        }
        else//分组或总召唤，
        {
            *AppCommand=APP_SENDDATA;
            if((GroupTrn.COT!=PERCYC)&&(GroupTrn.COT!=BACK))
                *AppCommand|=APP_HAVEDATA1;
        }
    }
    return(Len);
}

INT8U CSecAppSev::EnCodeAllPara(INT16U DevIndex,INT16U BeginNo,INT16U EndNo,INT16U *pNum)
{
    short Value,Length,FramePos,No,Len;
    float temp;
    INT8U *p,*pdd;
    INT32U dd;
    BOOL Stop=FALSE;

    TxMsg[0]=Sec101Pad.TypeID[GroupTrn.GroupNo-1];//设置需要发送的数据类型
    (*pTxVSQ) = VSQ_SQ;

    Length=ASDULEN-AsduHeadLength-15;//250-6-15=229为应用层发送信息最大长度;15是3个浮点数参数的字节数。
    FramePos=0;
    No=BeginNo;
    *pNum=0;
    while ((FramePos<Length)&&(!Stop))
    {
        switch(TxMsg[0])
        {
            case M_ME_NA:     //9测量值，规一化值
                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].DeadValue;
                //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                pTxData[FramePos++]=0;//flag
                (*pNum)++;

                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].UpLimit;
                //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                pTxData[FramePos++]=0;//flag
                (*pNum)++;

                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].LowLimit;
                //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                pTxData[FramePos++]=0;//flag
                (*pNum)++;

                break;
            case M_ME_NB:     //11测量值，标度值
                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].DeadValue;
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                pTxData[FramePos++]=0;//flag
                (*pNum)++;

                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].UpLimit;
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                pTxData[FramePos++]=0;//flag
                (*pNum)++;

                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].LowLimit;
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                pTxData[FramePos++]=0;//flag
                (*pNum)++;

                break;
            case M_ME_NC:    //13测量值，短浮点数
                temp = (float)DevList[GroupTrn.DevIndex].DevData.AIPara[No].DeadValue;
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
                pTxData[FramePos++]=0;
                (*pNum)++;

                temp = (float)DevList[GroupTrn.DevIndex].DevData.AIPara[No].UpLimit;
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
                pTxData[FramePos++]=0;
                (*pNum)++;

                temp = (float)DevList[GroupTrn.DevIndex].DevData.AIPara[No].LowLimit;
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
                pTxData[FramePos++]=0;
                (*pNum)++;

                break;
            case M_ME_ND:    //21不带品质，归一化
                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].DeadValue;
                //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                (*pNum)++;

                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].UpLimit;
                //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                (*pNum)++;

                Value=DevList[GroupTrn.DevIndex].DevData.AIPara[No].LowLimit;
                //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
                pTxData[FramePos++]=LOBYTE(Value);
                pTxData[FramePos++]=HIBYTE(Value);
                (*pNum)++;

                break;
        }
        No++;
        if(No>=EndNo)
            Stop=TRUE;
        if (*pNum>=127)//每帧不超过128个数据单元
            Stop=TRUE;
    }
    if (FramePos<=0)
        return (0);

    (*pTxVSQ) |= (*pNum);

    Len=FramePos+AsduHeadLength;//应用层报文的总长度
    if (Len)//只有分组召唤，平衡或非平衡一样
    {
        *LengthOut=Len;
        *AppCommand=APP_SENDDATA;
        *AppCommand|=APP_HAVEDATA1;
    }
    return (Len);
}

//
void CSecAppSev::EnCodeGroupEnd(void)//结束帧
{
    int jj;
    TxMsg[0]=GroupTrn.TypeID;
    (*pTxVSQ) = 1;

    TxMsg[CotLocation]=ACTTERM;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    if (TxMsg[0]==C_IC_NA) //总召唤
        *pTxData=GroupTrn.COT;//INTROGEN;
    else                             //counter
    {
        *pTxData=GroupTrn.Description;
    }

    *LengthOut=AsduHeadLength+1;
    if (BalanMode)
        *AppCommand=APP_SENDCON;
    else//分组或总召唤
    {
        *AppCommand=APP_SENDDATA;
    }
}

void CSecAppSev::EnCodeDDGroupEnd(void)//结束帧
{
    int jj;
    TxMsg[0]=GroupTrnDD.TypeID;
    (*pTxVSQ) = 1;

    TxMsg[CotLocation]=ACTTERM;

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrnDD.DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    *pTxData=GroupTrnDD.Description;

    *LengthOut=AsduHeadLength+1;
    if (BalanMode)//平衡模式
        *AppCommand=APP_SENDCON;
    else//非平衡
    {
        *AppCommand=APP_SENDDATA;
    }
}

//OK
BOOL CSecAppSev::GetNextDev(void) //得到下一个设备
{
    if (GroupTrn.PubAddr==BroadCastAddr)
    {
        GroupTrn.DevIndex++;
        if (GroupTrn.DevIndex>=DevCount)
            return(FALSE);
        else
        {
            if (GroupTrn.TypeID==C_IC_NA)  //alldata
            {
                GroupTrn.GroupNo=1;
                GroupTrn.InfoAddr=LBIinfoaddr;     //wjr   
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

BOOL CSecAppSev::GetNextDDDev(void) //得到下一个设备
{
    if (GroupTrnDD.PubAddr==BroadCastAddr)
    {
        GroupTrnDD.DevIndex++;
        if (GroupTrnDD.DevIndex>=DevCount)
            return(FALSE);
        else
        {
            GroupTrnDD.GroupNo=1;
            GroupTrnDD.InfoAddr=LBCR;
            return(TRUE);
        }
    }
    else
        return(FALSE);
}

//OK
BOOL CSecAppSev::EnCodeNVA(void)  //编辑变化遥测数据;
{
    BOOL Stop=FALSE;
    BOOL Over=FALSE;
    int  FramePos,i,j,jj;
    int No,NvaNum;
    short value, AINum;
    INT16U devid,Length;
    float temp;
    INT8U *p, *pdd;
    INT32U dd;
    struct RealAI_t *pAIValue;
    INT16U NvaVal;

    for (j=0;j<DevCount;j++)//每召唤一次变化遥测数据，只发一帧（不管是否发完，如果未发完下次召唤时再发送）
    {
        FramePos=0-InfoAddrSize;
        (*pTxVSQ) = 0;

        if(BalanMode==TRUE)//zzw
            TxMsg[CotLocation]=SPONT;
        else
            TxMsg[CotLocation]=REQ;

        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[NvaActDevNo].Addr>>(8*jj);

        for (No=0;No<=int(DevList[NvaActDevNo].DevData.AINum-1);No++)
            DevList[NvaActDevNo].DevData.AIData[No].WillSend=FALSE;

        No=0;
        NvaNum=0;

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
                
                if(DevList[NvaActDevNo].DevData.AIPara[No].type)
                {
                    NvaVal = abs((INT16U)DevList[NvaActDevNo].DevData.AIData[No].Value - (INT16U)value);
                }
                else
                {
                    NvaVal = abs(DevList[NvaActDevNo].DevData.AIData[No].Value-value);
                }
                
                if (NvaVal >= DevList[NvaActDevNo].DevData.AIPara[No].DeadValue)//比较变化值与死区值大小
                {
                    DevList[NvaActDevNo].DevData.AIData[No].WillSend=TRUE;//设置发送标志
                    NvaNum++;
                }
                DevList[NvaActDevNo].DevData.AIData[No].Flag=pAIValue->Flag;
                DevList[NvaActDevNo].DevData.AIData[No].TempValue=value;
                No++;
                pAIValue++;
            }
        }

        No=DevList[NvaActDevNo].DevData.NvaNo; //从上次发完的遥测序号开始发送
        Length=Sec101Pad.MaxALLen-AsduHeadLength-5-sizeof(INT16U);//可以发送数据的报文长度
          
        while ((FramePos<Length)&&(!Stop))//组发送数据帧
        {
            if (DevList[NvaActDevNo].DevData.AIData[No].WillSend)
            {
                if (FramePos < 0)
                {
                    TxMsg[0]=Sec101Pad.TypeID[YCGroupNo[No]-1];//记录该帧遥测的类型
                    *pTxInfoAddr    =LOBYTE((No+LAI));
                    *(pTxInfoAddr+1)=HIBYTE((No+LAI));
                }
                else
                {
                    if(Sec101Pad.TypeID[YCGroupNo[No]-1]==TxMsg[0])//后续数据必须与前面的类型一致
                    {
                        pTxData[FramePos]=LOBYTE(No+LAI);
                        pTxData[FramePos+1]=HIBYTE(No+LAI);
                    }
                    else
                        Stop=TRUE;
                }
                if (!Stop)
                    FramePos+=InfoAddrSize;
                if (FramePos<0)
                    Stop=TRUE;

                if (!Stop)
                {
                    switch (TxMsg[0])
                    {
                        case M_ME_NA:     //9测量值，规一化值
                            value = (short)SL_ReadAI_S(DevList[NvaActDevNo].DevID, No, DevList[NvaActDevNo].DevData.AIData[No].TempValue);
                            //value=DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                            //value=(long)value*0x3FFF/(long)DevList[NvaActDevNo].DevData.AIMaxVal[No];
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

                            DevList[NvaActDevNo].DevData.AIData[No].Value = DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                            break;
                        case M_ME_NC:    //13测量值，短浮点数
                            temp = SL_ReadAI_S(DevList[NvaActDevNo].DevID, No, DevList[NvaActDevNo].DevData.AIData[No].TempValue);
                            //temp = (float)DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                            p = (INT8U*)&temp;
                            pdd = (INT8U*)&dd;
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
                            //value=(long)value*0x3FFF/(long)DevList[NvaActDevNo].DevData.AIMaxVal[No]; //  ll
                            pTxData[FramePos++]=LOBYTE(value);
                            pTxData[FramePos++]=HIBYTE(value);

                            DevList[NvaActDevNo].DevData.AIData[No].Value
                                           =DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                            break;
                    }
                    (*pTxVSQ) ++;
                    if (*pTxVSQ >= 127)//不能超过127。
                        Stop=TRUE;
                }
            }
            No++;
            if ((No >= DevList[NvaActDevNo].DevData.AINum)||(*pTxVSQ >= NvaNum))
            {
                No=0;
                NvaActDevNo++;//设备序号在发送一帧后累加。
                Stop=TRUE;
                if (NvaActDevNo>=DevCount)//所有设备变化遥测都发送一遍。
                {
                    NvaActDevNo=0;
                    Data1.Flag&=(~HaveNVA);
                    Over=TRUE;
                }
            }
        }  //end of while ((FramePos<i)&&(!Stop))

        DevList[NvaActDevNo].DevData.NvaNo=No;       //记录下次发送变化遥测的开始序号。
        if (FramePos>0)
        {
            *LengthOut=FramePos+AsduHeadLength;
            *AppCommand=APP_SENDDATA;
            if(!Over)//变化遥测没发完，设置一级数据标志
                *AppCommand|=APP_HAVEDATA1;
            if(BalanMode)
                *AppCommand=APP_SENDCON;
            return(TRUE);//每次只发一帧
        }
    }//end of for
    return(FALSE);
}

BOOL CSecAppSev::CheckNVA(void)  //检测变化遥测数据;
{
    INT16U i,j,No;
    short Value,AINum;
    struct RealAINFlag_t *pAIValue;
    INT16U temp;
    
    
    for (j=0;j<DevCount;j++)
    {
        No=0;
        while (No<=(int)(DevList[NvaActDevNo].DevData.AINum-1))//取遥测值，设置发送标志
        {
            if(DevList[NvaActDevNo].Flag==1)
                AINum=CRSend_ReadAI(DevList[NvaActDevNo].DevID,No,No+511,(struct RealAINFlag_t *)DBData);
            else
                AINum=CL_ReadAI(DevList[NvaActDevNo].DevID,No,No+511,(struct RealAINFlag_t *)DBData);

            if (AINum<=0)
                return FALSE;
            pAIValue=(struct RealAINFlag_t *)DBData;
            for(i=0;i<AINum;i++)
            {
                Value=pAIValue->Value;
                
                if(DevList[NvaActDevNo].DevData.AIPara[No].type)
                {
                    temp = abs((INT16U)DevList[NvaActDevNo].DevData.AIData[No].Value - (INT16U)Value);
                }
                else
                {
                    temp = abs(DevList[NvaActDevNo].DevData.AIData[No].Value-Value);
                }
                
                if (temp >= DevList[NvaActDevNo].DevData.AIPara[No].DeadValue)//比较变化值与死区值大小
                {
                    return TRUE;
                }
                   
                No++;
                pAIValue++;
            }
        }
        
        NvaActDevNo++;
        NvaActDevNo%=DevCount;
    }
    return FALSE;
}


//冻结电度
void CSecAppSev::FreezeCounter(void)    //冻结电度，当QCC 7位为1时
{
    INT16U GetBeginNo,GetEndNo,i,j;
    struct RealCounter_t *q;
    short CountNum;
    INT32U value,*p;

    GetSysTime((void*)(&CounterTime),ABSTIME);//记录冻结电度的系统时间

    for (i=0;i<DevCount;i++)
    {
        if (DevList[i].DevData.CounterNum<=0)
            continue;

        GetBeginNo=0;
        GetEndNo=DevList[i].DevData.CounterNum-1;
        while (GetBeginNo<=GetEndNo)
        {
            if(DevList[i].Flag==1)
                CountNum=CRSend_ReadCount(DevList[i].DevID,GetBeginNo,GetBeginNo+200,(struct RealCounterNFlag_t *)DBData);
            else
                CountNum=CL_ReadCount(DevList[i].DevID,GetBeginNo,GetBeginNo+200,(struct RealCounterNFlag_t *)DBData);
            if(CountNum<=0)
                return;
            p=(INT32U*)DBData;
            for(j=0;j<CountNum;j++)
            {
                value=*p;
                q=&DevList[i].DevData.CounterData[GetBeginNo+j];
                if (GroupTrnDD.Description&FREEZENORESET)//冻结不复位
                {
                    q->Value=value;
                    q->Flag=0;
                    DevList[i].DevData.LastCounterData[GetBeginNo+j]=value;
                }
                else if(GroupTrnDD.Description&FREEZERESET)
                {
                    q->Flag=0;
                    q->Value=value-DevList[i].DevData.LastCounterData[GetBeginNo+j];
                    DevList[i].DevData.LastCounterData[GetBeginNo+j]=value;
                }
                p++;
            }
            GetBeginNo += CountNum;
        }
    }//end of for
    
    
    //收到0x45收到电能量  给当前设备的支持线损模块新标准的均发送event，触发线损模块进行瞬时冻结文件生成  CL 20180801
    if((RxMsg[AsduHeadLength]&0xc0) == 0x40)  //FRZ=1 冻结不带复位功能
    {
        SetFileSynInfoTaskIDSubstation(MySelf.AppTID);  //作为启动101主站任务开始的量
        if(XSFileSynInfo.TaskIDPri101[0]!=0)//代表第一个线损模块起的任务
        {
            myEventSend(GetFileSynInfoTaskID101(0),XSFREEZE);//给101主站任务发送消息 暂时先发给第一个101任务，后续是通过维护软件面板参数确认的。
        }
        else
        {
            //logSysMsgWithTime("无支持2018标准的线损模块！",0,0,0,0);
        }
        if(XSFileSynInfo.TaskIDPri101[1]!=0)//代表有第二个线损模块起的任务
        {
            myEventSend(GetFileSynInfoTaskID101(1),XSFREEZE);
        }    
    } 
    //收到0x45收到电能量  给当前设备的支持线损模块新标准的均发送event，触发线损模块进行瞬时冻结文件生成  CL 20180801
}

//
void CSecAppSev::EnCodeCounterConf(void)//
{
    int jj;
    TxMsg[0]=C_CI_NA;//101
    (*pTxVSQ) = 1;

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrnDD.DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    TxMsg[CotLocation]=ACTCON;
    *pTxData=GroupTrnDD.Description;
    //logSysMsgNoTime("DD发送确认帧",0,0,0,0);   //  debug ll
    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    if((GroupTrnDD.Description&0xc0)==0)//是对读电度命令的应答，否则是对冻结命令的应答
        *AppCommand|=APP_HAVEDATA1;
    if(BalanMode)
    {
        *AppCommand=APP_SENDCON;
    }      
}

void CSecAppSev::ProcCounter(void) //处理电度zzw
{
    INT16U BeginNo,EndNo,Num;
    INT8U GroupNo;
    if ((GroupTrnDD.GroupNo>=1)&&(GroupTrnDD.GroupNo<=4))
    {
        if (CheckDDGroup())
        {
            BeginNo=GroupTrnDD.InfoAddr-LBCR;
            EndNo=GroupTrnDD.GroupNo*GROUPBCRNUM-1;
            if ((EndNo+1)>DevList[GroupTrnDD.DevIndex].DevData.CounterNum)
                EndNo=DevList[GroupTrnDD.DevIndex].DevData.CounterNum-1;
            if (EnCodeCounter(BeginNo,EndNo,&Num))
            {
                GroupTrnDD.First=FALSE;
                BeginNo+=Num;//更新起始点号，即下一次发送数据的起始点号

                GroupTrnDD.InfoAddr=BeginNo%MAXBCRNUM+LBCR;
                GroupNo=GroupTrnDD.InfoAddr-LBCR;                  
                GroupNo/=GROUPBCRNUM;
                GroupNo++;

                if (GroupTrnDD.GroupNo!=GroupNo)//该组数据发送完毕
                {
                    GroupTrnDD.GroupNo=GroupNo;
                    if (GroupTrnDD.COT>REQCOGCN)//分组召唤，结束发送数据。wjr  2009.8.25 传送原因判断错误，原为请求被请求
                        GroupTrnDD.GroupNo=5;
                }
                return;
            }
            else
                GroupTrnDD.GroupNo=5;
        }
    }
    if (GroupTrnDD.GroupNo>=5)
    {
        Data1.Flag&=(~CallDD);
        EnCodeDDGroupEnd();//发送结束
        DDFreeze=FALSE;
        if(GetNextDDDev())//如果还有下个逻辑设备的电度没发过，再设置电度标志。
        {
            EditDDCon=0xff;
            Data1.Flag|=CallDD;
            DDFreeze=TRUE;
        }
    }
}

INT8U CSecAppSev::EnCodeCounter(INT16U BeginNo,INT16U EndNo,INT16U *pNum)//从数据库取电度，处理后，发到链路层。
{
    int jj;
    INT8U Len;
    INT16U No,i;
    unsigned long value,*p;
    short FramePos,SendNum,Length,CountNum;

//    TxMsg[0]=Sec101Pad.TypeID[16];
    //if((Sec101Pad.control&CON_101GYKZ))     //AJ++170829 
    if(GYKZ2015Flag)  
    {
        TxMsg[0] = M_IT_NB;
    }
    else
    {
        TxMsg[0] = Sec101Pad.TypeID[16];      
    }
    *pTxVSQ = 0;/*电度都采用离散序列号*/
    
    TxMsg[CotLocation]=GroupTrnDD.COT;
    
    for(jj=0;jj<PubAddrSize;jj++)
    {
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrnDD.DevIndex].Addr>>(8*jj);
    }
    
    No=BeginNo;
    FramePos=0-InfoAddrSize;
    SendNum=0;
    Len=0;
    Length=ASDULEN-AsduHeadLength-10-sizeof(INT16U);//250-6-10-2=232为应用层发送信息最大长度
    
	/* AJ++170829 读电度数据库.在2015扩展规约下，电度无需冻结直接读数据库 */
    //if((Sec101Pad.control&CON_101GYKZ)) 
    if(GYKZ2015Flag)                                     
    {
        if(DevList[GroupTrnDD.DevIndex].Flag==1)
        {
            CountNum=CRSend_ReadCount(DevList[GroupTrnDD.DevIndex].DevID,BeginNo,EndNo,(struct RealCounterNFlag_t *)DBData);
        }        
        else
        {
            CountNum=CL_ReadCount(DevList[GroupTrnDD.DevIndex].DevID,BeginNo,EndNo,(struct RealCounterNFlag_t *)DBData);
        }               
        if(CountNum<=0)
        {
            return 0;
        }               
        p=(unsigned long*)DBData;                 
    }
    
	for(i=0;i<=EndNo-BeginNo;i++)
    {
    //    value=DevList[GroupTrnDD.DevIndex].DevData.CounterData[No].Value;
        //if((Sec101Pad.control&CON_101GYKZ))		//AJ++170829 传输实时值   
        if(GYKZ2015Flag)                                   
        {
            value = *p;
            p ++;   
        }
        else									//传输冻结值
        {
            value=DevList[GroupTrnDD.DevIndex].DevData.CounterData[No].Value;   
        }

        //写信息体地址
        if(FramePos<0)
        {
            *pTxInfoAddr    =LOBYTE((No+LBCR));
            *(pTxInfoAddr+1)=HIBYTE((No+LBCR));
        }
        else
        {
            pTxData[FramePos]   = LOBYTE((No+LBCR));//信息体地址
            pTxData[FramePos+1] = HIBYTE((No+LBCR));//信息体地址
            if(InfoAddrSize == 3)
                pTxData[FramePos+2] = 0;//信息体地址为3字节时，最高字节为0
        }
        FramePos+=InfoAddrSize;
        //写电度值
        pTxData[FramePos]=LOBYTE(LOWORD(value));
        pTxData[FramePos+1]=HIBYTE(LOWORD(value));
        pTxData[FramePos+2]=LOBYTE(HIWORD(value));
        pTxData[FramePos+3]=HIBYTE(HIWORD(value));
        pTxData[FramePos+4]=i;//顺序号
        FramePos+=5;
        //写时标
        if(TxMsg[0] == M_IT_TA)//16带时标的电度
        {
            pTxData[FramePos]=LOBYTE(CounterTime.MSecond);
            pTxData[FramePos+1]=HIBYTE(CounterTime.MSecond);
            pTxData[FramePos+1]=(CounterTime.Minute%60);
            FramePos+=3;
        }

        No++;
        SendNum++;
        if(FramePos>=Length)
            break;
    }
    *pTxVSQ = SendNum;
    *pNum = SendNum;
    if(SendNum>0)
        Len=FramePos+AsduHeadLength;
    
    if (Len>0)
    {
        *LengthOut=Len;
        *AppCommand=APP_SENDDATA;
        *AppCommand|=APP_HAVEDATA1;

        if(BalanMode)
            *AppCommand=APP_SENDCON;
    }
    
    return(Len);
}


void CSecAppSev::EnCodeTimeDelay(void)
{
    INT16U ms;
    int jj;
    TrTime=100;//防止偶然读不出时钟，出现时间混乱。
    if (GetSysTime((void*)(&SecSysTimeT),ABSTIME))
    {
        if(ReadTimeFlag==0xff)
        {
            ReadTimeFlag=0;
            if(SecSysTimeT.MSecond>SecSysTimeR.MSecond)
                TrTime=SecSysTimeT.MSecond-SecSysTimeR.MSecond;
            else
                TrTime=60000-SecSysTimeR.MSecond+SecSysTimeT.MSecond;
        }
    }

    TxMsg[0]=M_CD_NA;//106，延时获得

    (*pTxVSQ) = 1;

    TxMsg[CotLocation]=ACTCON;

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    if(((INT32U)SDTTime+TrTime)<60000)
        ms=SDTTime+TrTime;
    else
        ms=((INT32U)(SDTTime+TrTime))-60000;

    *pTxData=LOBYTE(ms);
    *(pTxData+1)=HIBYTE(ms);

    *LengthOut=AsduHeadLength+2;//只发送2字节ms。
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

void CSecAppSev::EnCodeClock(void)//发送子站设置时钟前的系统时钟
{
    int jj;
    struct Iec101ClockTime_t time;

    TxMsg[0]=C_CS_NA;

    (*pTxVSQ) = 1;

    //TxMsg[CotLocation]=ACTCON;
    if((RxCot&COT_REASON)==ACT)
    {
    	if(TimeRightFlag)
        {
        	TxMsg[CotLocation]=ACTCON;                                
        }
        else
        {
        	TxMsg[CotLocation]=ACTCON | 0x40;                       //从站不允许对钟或者对钟时间错误发送传送原因为0x47
        }
    }
    else if((RxCot&COT_REASON)==REQ)
    {
        TxMsg[CotLocation]=REQ;                                

    }
    else
    {
    	TxMsg[CotLocation]=UNKNOWNCOT;
    	TxMsg[CotLocation]|=0x40;
    }
    
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    if(OldSysTime.MSecond>=TimeDelay)//减掉线路延迟
        OldSysTime.MSecond-=TimeDelay;
    else
    {
        OldSysTime.Minute--;
        OldSysTime.MSecond=60000-TimeDelay+OldSysTime.MSecond;
    }
    if(Sec101Pad.UseStandClock == 1)
        AbsTimeConvTo(&OldSysTime,(void*)(&time),IEC101CLOCKTIME);
    else
        AbsTimeConvTo(&OldSysTime,(void*)(&time),IEC101EXTCLOCKTIME);

    *(pTxData)   = LOBYTE(time.MSecond);
    *(pTxData+1) = HIBYTE(time.MSecond);
    *(pTxData+2) = time.Minute;
    *(pTxData+3) = time.Hour;
    *(pTxData+4) = time.Day;
    *(pTxData+5) = time.Month;
    *(pTxData+6) = time.Year;

    *LengthOut=AsduHeadLength+7;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

void CSecAppSev::EnCodeReadData(void)
{
    int No;
    INT16U Num;
    INT8U Len;
    No=-1;
    RDPubAddr=RxPubAddr;
    RDInfoAddr=RxInfoAddr;
    
    if (GetActDevIndexByAddr(RDPubAddr))//根据公共体地址查设备序号。
    {
        GroupTrn.DevIndex=ActDevIndex;
        GroupTrnDD.DevIndex = ActDevIndex;
    }
    else
    {
        HaveWrongData=TRUE;
        WrongDataLength=LengthIn;
        RxMsg[CotLocation]=UNKNOWNPUBADDR;
        RxMsg[CotLocation]|=0x40;
        memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

        *LengthOut=0;
        *AppCommand=APP_APPCON;
        *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    
    if ((RDInfoAddr>=LDBIinfoaddr)&&(RDInfoAddr<=HBI))        //wjr 2009.4.5  应为信息体地址而不是公共体地址
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.InfoAddr=RDInfoAddr;                    //wjr 2009.4.5  
        if(GroupTrn.InfoAddr>=(LDBIinfoaddr+DevList[GroupTrn.DevIndex].DevData.DBINum/2))   /*读的数据为单点遥信则   wjr2009.8.25*/
        {
            if((GroupTrn.InfoAddr<LBIinfoaddr) || (GroupTrn.InfoAddr>=LBIinfoaddr+DevList[GroupTrn.DevIndex].DevData.BINum))
            {
                HaveWrongData=TRUE;
                WrongDataLength=LengthIn;
                RxMsg[CotLocation]=UNKNOWNTINFOADDR;
                RxMsg[CotLocation]|=0x40;
                memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

                *LengthOut=0;
                *AppCommand=APP_APPCON;
                *AppCommand|=APP_HAVEDATA1;
                return;    
            }
            else
            {        
                No=GroupTrn.InfoAddr-LBIinfoaddr;
                No=No+DevList[GroupTrn.DevIndex].DevData.DBINum;
                GroupTrn.InfoAddr += DevList[GroupTrn.DevIndex].DevData.DBINum;
            }
        }    
        else                                                                             /*读的数据为双点遥信  wjr2009.8.25*/
            No=GroupTrn.InfoAddr-LDBIinfoaddr;
        GroupTrn.GroupNo=YXGroupNo[No];
    }
    else if ((RDInfoAddr>=LAI)&&(RDInfoAddr<LAI+DevList[GroupTrn.DevIndex].DevData.AINum))         //wjr 2009.4.5
    {
        //读遥测
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.InfoAddr=RDInfoAddr;                   //wjr 2009.4.5
        No=GroupTrn.InfoAddr-LAI;
        GroupTrn.GroupNo=YCGroupNo[No];
    }
    else if ((RDInfoAddr>=LPARA)&&(RDInfoAddr<=HPARA))       //wjr 2009.4.5
    {
        EnCodeReadPara(ActDevIndex,RDInfoAddr);           //wjr 2009.4.5
        return;
    }
    else if ((RDInfoAddr>=LSPI)&&(RDInfoAddr<=HSPI))     //wjr 2009.4.5
    {
        //读步位置信息
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=15;
        GroupTrn.InfoAddr = RDInfoAddr;         // ll 增加，GroupTrn.InfoAddr没赋值就进行减操作是错误的
        No=GroupTrn.InfoAddr-LSPI;
    }
    //else if (GroupTrn.InfoAddr==RTUSTATUS)
    else if (RDInfoAddr==RTUSTATUS)     //ll 修改
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=16;
        No=0;
    }
    else if ((RDInfoAddr>=LBCR)&&(RDInfoAddr<LBCR+DevList[GroupTrn.DevIndex].DevData.CounterNum))          //wjr 2009.4.5
    {
        //读电度
        GroupTrn.TypeID=C_CI_NA;
        GroupTrnDD.GroupNo=(GroupTrnDD.InfoAddr-LBCR)/GROUPBCRNUM+1;
        GroupTrnDD.COT = REQ;
        GroupTrnDD.InfoAddr = RDInfoAddr;         // ll 增加，GroupTrnDD.InfoAddr没赋值就进行减操作是错误的
        No=GroupTrnDD.InfoAddr-LBCR;
    }
    else
    {
        HaveWrongData=TRUE;
        WrongDataLength=LengthIn;
        RxMsg[CotLocation]=UNKNOWNTINFOADDR;
        RxMsg[CotLocation]|=0x40;
        memcpy((void*)WrongData,(void*)RxMsg,LengthIn);

        *LengthOut=0;
        *AppCommand=APP_APPCON;
        *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    if ((No>=0)&&(GroupTrn.TypeID==C_IC_NA))
    {
        Len=EnCodeAllData((INT16U)No,(INT16U)No,&Num);//返回应用层报文总长度
        
        (*pTxVSQ) = 1;
        TxMsg[CotLocation]=REQ;

        *LengthOut=Len;
        *AppCommand=APP_SENDDATA;
        if(BalanMode)
            *AppCommand=APP_SENDCON;
        return;
    }

    if ((No>=0)&&(GroupTrn.TypeID==C_CI_NA))
    {
        
        Len=EnCodeCounter((INT16U)No,(INT16U)No,&Num);
        
        //(*pTxVSQ) = 1;
        //TxMsg[CotLocation]=REQ;
       // *LengthOut=Len;
        //*AppCommand=APP_SENDDATA;
        //if(BalanMode)
        //    *AppCommand=APP_SENDCON;
        return;
    }
    
}

//公共体地址或信息体地址错误
void CSecAppSev::EnCodeReadDataCon(INT16U DevIndex,INT8U Cot)
{
    int jj;
    TxMsg[0]=C_RD_NA;

    (*pTxVSQ) = 0;
    TxMsg[CotLocation]=Cot;

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RDPubAddr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((RDInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((RDInfoAddr));

    *LengthOut=AsduHeadLength;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

//一次只读取一个参数
void CSecAppSev::EnCodeReadPara(INT16U DevIndex,INT16U InfoAddr)
{
    INT16U No;
    INT8U Len=0;
    short val = 0;         //基于coverity更改
    float temp;
    INT8U *p, *pdd;
    int jj;
    INT32U dd;
    
    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RDPubAddr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((RDInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((RDInfoAddr));

    No=(InfoAddr-LPARA)/3;  
    TxMsg[0]=Sec101Pad.TypeID[13];//按照第14组设置的类型发参数

    switch(TxMsg[0])
    {
        case M_ME_NA:     //9测量值，规一化值
            switch((InfoAddr-LPARA)%3)
            {
                case 0://门限
                    
                    val=DevList[DevIndex].DevData.AIPara[No].DeadValue;
                    break;
                case 1://上限
                    val=DevList[DevIndex].DevData.AIPara[No].UpLimit;
                    break;
                case 2://下限
                    val=DevList[DevIndex].DevData.AIPara[No].LowLimit;
                    break;
            }

            //val=(long)val*0x3FFF/(long)DevList[DevIndex].DevData.AIMaxVal[No];
            
            *(pTxData)  =LOBYTE(val);
            *(pTxData+1)=HIBYTE(val);
            *(pTxData+2)=0;
            Len=3;
            break;
        case M_ME_NB:     //11测量值，标度值
            switch((InfoAddr-LPARA)%3)
            {
                case 0:
                    val=DevList[DevIndex].DevData.AIPara[No].DeadValue;
                    break;
                case 1:
                    val=DevList[DevIndex].DevData.AIPara[No].UpLimit;
                    break;
                case 2:
                    val=DevList[DevIndex].DevData.AIPara[No].LowLimit;
                    break;
            }
            *(pTxData)  =LOBYTE(val);
            *(pTxData+1)=HIBYTE(val);
            *(pTxData+2)=0;
            Len=3;
            break;
        case M_ME_NC:    //13测量值，短浮点数
            switch((InfoAddr-LPARA)%3)
            {
                case 0:
                    temp=(float)DevList[DevIndex].DevData.AIPara[No].DeadValue;
                    break;
                case 1:
                    temp=(float)DevList[DevIndex].DevData.AIPara[No].UpLimit;
                    break;
                case 2:
                    temp=(float)DevList[DevIndex].DevData.AIPara[No].LowLimit;
                    break;
            }
            p = (INT8U*)(&temp);
            pdd = (INT8U*)(&dd);
            *(pdd++) = *(p++);
            *(pdd++) = *(p++);
            *(pdd++) = *(p++);
            *(pdd++) = *(p++);
            *(pTxData)  =LLBYTE(dd);
            *(pTxData+1)=LHBYTE(dd);
            *(pTxData+2)=HLBYTE(dd);
            *(pTxData+3)=HHBYTE(dd);
            *(pTxData+4)=0;
            Len=5;
            break;
        case M_ME_ND:    //21不带品质，归一化
            switch((InfoAddr-LPARA)%3)
            {
                case 0:
                    val=DevList[DevIndex].DevData.AIPara[No].DeadValue;
                    break;
                case 1:
                    val=DevList[DevIndex].DevData.AIPara[No].UpLimit;
                    break;
                case 2:
                    val=DevList[DevIndex].DevData.AIPara[No].LowLimit;
                    break;
            }
            //val=(long)val*0x3FFF/(long)DevList[DevIndex].DevData.AIMaxVal[No];
            *(pTxData)  =LOBYTE(val);
            *(pTxData+1)=HIBYTE(val);
            Len=2;
            break;
    }
    *LengthOut=AsduHeadLength+Len;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

void CSecAppSev::ParaSetCon(void)
{
    INT8U Len = 0;                  //根据coverity静态检测结果更改
    float tempfloat;
    int jj;
    INT8U *p,*pdd;
    INT32U dd;

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=ParaPubAddr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((ParaInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((ParaInfoAddr));
    TxMsg[0]=ParaTypeID;

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=ACTCON;

    switch(TxMsg[0])
    {
        case P_ME_NA:    //归一化
        case P_ME_NB:    //标度化
            *pTxData=LOBYTE(ParaWord);
            *(pTxData+1)=HIBYTE(ParaWord);
            *(pTxData+2)=ParaQPM;
            Len=3;
            break;
        case P_ME_NC:    //短浮点数
            //tempfloat = (float)ParaWord;  //ll 修改，错误用值
            tempfloat = (float)ParaFloat;
            //logSysMsgNoTime("SEC 短浮点数f=%x,f2=%x",tempfloat,ParaFloat,0,0);   //  debug ll
            p = (INT8U*)(&tempfloat);
            pdd = (INT8U*)(&dd);
            *(pdd++) = *(p++);
            *(pdd++) = *(p++);
            *(pdd++) = *(p++);
            *(pdd++) = *(p++);
            *(pTxData)   = LLBYTE(dd);
            *(pTxData+1) = LHBYTE(dd);
            *(pTxData+2) = HLBYTE(dd);
            *(pTxData+3) = HHBYTE(dd);
            *(pTxData+4) = ParaQPM;
            Len=5;
            break;
    }

    *LengthOut=AsduHeadLength+Len;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

void CSecAppSev::EnCodeReset(void)
{
    int jj;
    TxMsg[0]=C_RP_NA;
    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=ResetPubAddr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((ResetInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((ResetInfoAddr));
    *pTxData=ResetGRP;
    
    if(ResetGRP==1)
    {
        *pRestType = 0xee;  //复位规约进程状态记忆 ll 2010/07/20   for 广西规约测试
        
        ResetFlag=0xff;
        ResetCount=7;
    }

    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

void CSecAppSev::EnCodeTest(void)
{
    int jj;
    TxMsg[0]=C_TS_NA;
    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=TestPubAddr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((TestInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((TestInfoAddr));
    *pTxData=0xaa;
    *(pTxData+1)=0x55;

    *LengthOut=AsduHeadLength+2;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

void CSecAppSev::EnCodeSetNVA()
{
    int jj;
    TxMsg[0]=C_SE_NA;
    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=SetPubAddr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((SetInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((SetInfoAddr));
    *pTxData=LOBYTE(SetNvaWord);
    *(pTxData+1)=HIBYTE(SetNvaWord);
    *(pTxData+2)=SetQOS;

    *LengthOut=AsduHeadLength+3;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

BOOL CSecAppSev::ClearFlag(INT16U DevIndex,INT16U Flag)//
{
    BOOL HaveSend=FALSE;
    switch (Flag)
    {
        case BIETFLAG:
            myTaskLock();//禁止抢占调度
            if(DevList[DevIndex].Flag)
                DevList[DevIndex].RealWin->BIETimRP+=DevList[DevIndex].Data1.SOENum;
            else
                DevList[DevIndex].pDbaseWin->BIETimRP+=DevList[DevIndex].Data1.SOENum;
            clear_flag(DevList[DevIndex].DevID,BIETFLAG);

            BIFrame&=(~BIETFRAME);
            DevList[DevIndex].Data1.SOENum=0;
            myTaskUnlock();//允许抢占调度

            break;
        /*case FAPROCFLAG:
            myTaskLock();
            if(DevList[DevIndex].Flag)
                DevList[DevIndex].RealWin->FAProcRP+=DevList[DevIndex].Data1.FAProcNum;
            else
                DevList[DevIndex].pDbaseWin->FAProcRP+=DevList[DevIndex].Data1.FAProcNum;
            clear_flag(DevList[DevIndex].DevID,FAPROCFLAG);
            BIFrame&=(~FAPROCFRAME);
            DevList[DevIndex].Data1.FAProcNum=0;
            myTaskUnlock();
            break;*/
        case BIENTFLAG:
            myTaskLock();
            if(DevList[DevIndex].Flag)
                DevList[DevIndex].RealWin->BIENTimRP+=DevList[DevIndex].Data1.BIENTNum;
            else
                DevList[DevIndex].pDbaseWin->BIENTimRP+=DevList[DevIndex].Data1.BIENTNum;
            clear_flag(DevList[DevIndex].DevID,BIENTFLAG);
            BIFrame&=(~BIENTFRAME);
            DevList[DevIndex].Data1.BIENTNum=0;
            myTaskUnlock();

            break;
    }
    if (!((BIFrame&BIETFRAME)||(BIFrame&BIENTFRAME)/*||(BIFrame&FAPROCFRAME)*/))
        LastFrame=Polling;
    return(HaveSend);
}

INT8U CSecAppSev::EnCodeDevStatus(void)
{
    return 0;
}

void CSecAppSev::ProcFileInit(void)
{
    FT_Init(&FtInfo);
    
    FtInfo.InfoAddrSize = InfoAddrSize;
    FtInfo.DevID = DevList[0].DevID;
    
    FtInfo.dbisoenum = DevList[0].DevData.DBINum;
    FtInfo.LBIinfoaddr = LBIinfoaddr;
    FtInfo.LDBIinfoaddr = LDBIinfoaddr;
    FtInfo.mastercallflag = 0;   //AJ++170627
    
}

/*------------------------------------------------------------------/
函数名称：  ProcFT_EncodeReadDir()
函数功能：  对目录的传输组织发送帧
输入说明：  
输出说明： TRUE   有后续报文   FALSE 没有后续报文
备注：      
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcFT_EncodeReadDir(void)
{
    INT8U len;
    BOOL  rc;
    INT8U FramePos;
    INT16U jj;
        
    len = 0;
    
    //根据目录ID或目录名称，组织目录内文件的传输.从文件数量开始填写
    
    rc = FT_ReadDirectory(&FtInfo, &len);
       
    //len = 0 回否定回答，rc=true表示有无后续
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;   
         
    TxMsg[FramePos++] = 2;         //附加数据包类型
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
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
           
    return rc;

}
/*------------------------------------------------------------------/
函数名称：  ProcFT_ReadDir()
函数功能：  处理文件传输读目录命令。
输入说明：  
输出说明：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_ReadDir(void)
{
    INT32U DirID;
    INT8U namelen, CallFlag, i;
    INT8U *pRx;
    struct Iec101ClockTime_t StartTime, EndTime;
    
    DirID = MAKEDWORD(MAKEWORD(pRxData[2],pRxData[3]),MAKEWORD(pRxData[4],pRxData[5]));
    namelen = pRxData[6];
    
    memset(FtInfo.tempname, 0, 40);
    if(namelen>=40)
        namelen = 39;
    memcpy(FtInfo.tempname,&pRxData[7],namelen);    //FtInfo.tempname中临时存放，用于打印信息
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    
    FtInfo.dirid = FT_GetDirID(&FtInfo);    //根据目录名返回目录ID DirID暂时不使用 liuwei20170307
    
    pRx = &pRxData[7]+namelen;  //把召唤标志位置的指针赋值给pRx    
    CallFlag = pRx[0];  //召唤标志
    
    FtInfo.callflag = CallFlag;
    
    logSysMsgNoTime("召唤目录%s 召唤标志=%d, DirID1=%d, DirID2=%d",(INT32U)FtInfo.tempname, CallFlag, FtInfo.dirid, DirID);  
    
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
    
    SetSendData2Flag(DATA2_FT_DIR);
       
}
/*------------------------------------------------------------------/
函数名称：  ProcFT_EncodeFileData()
函数功能：  传输要传输的文件内容
输入说明：  
输出说明：  TRUE 表示有后续， FALSE 无后续
备注：      
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcFT_EncodeFileData(void)
{
    BOOL rc;
    INT8U len;
    INT8U FramePos;
    INT16U jj;
           
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
    TxMsg[FramePos++] = 2;    //附加数据包类型
    TxMsg[FramePos++] = FR_RD_FILE_DATA;
    
    //4字节文件标示文件ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
    
    //根据FileFlag文件标示，查找不同文件 
    rc = FT_ReadData(&FtInfo, &TxMsg[FramePos], &len);
            
    EnCode101DLMsg(FramePos+len, APP_SENDDATA);     //FramePos包含附加数据包、操作标识、文件ID 
    
    return rc;

}
/*------------------------------------------------------------------/
函数名称：  ProcFT_EncodeFileActConf()
函数功能：  读文件激活确认帧
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_EncodeFileActConf(void)
{
    INT16U namelen;
    INT32U len;
    INT8U  FramePos, successPos;
    INT16U jj;
    
     
    len = FT_ReadFileAct(&FtInfo, FtInfo.tempname);

    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
    TxMsg[FramePos++] = 2;    //附加数据包类型
    TxMsg[FramePos++] = FR_RD_FILE_ACTCON; 
    successPos = FramePos;  //记录成功失败位置 
    FramePos++;
    
    //文件名
    namelen = FtInfo.namelen;
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
        
        SetSendData2Flag(DATA2_FT_FILEDATA);
    }
    else
    {
        TxMsg[successPos] = 1;  
        TxMsg[FramePos++] = 0;
        TxMsg[FramePos++] = 0;
        TxMsg[FramePos++] = 0;
        TxMsg[FramePos++] = 0;  
    }
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
          
}
/*------------------------------------------------------------------/
函数名称：  ProcFT_ReadFileAct()
函数功能：  激活要读的文件
输入说明：  
输出说明：  
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_ReadFileAct(void)
{
    INT16U namelen, i;    
    
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
    FtInfo.namelen = namelen;
    
    SetSendData2Flag(DATA2_FT_FILEACT);
    
}
/*------------------------------------------------------------------/
函数名称：  ProcFT_EncodeWriteFileActConf()
函数功能：  写文件激活确认帧
输入说明：  
输出说明：  
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_EncodeWriteFileActConf(void)
{
    INT8U result;
    INT8U namelen;
    INT8U FramePos;
    INT16U jj;
    
    result = FT_WriteFileAct(&FtInfo);
    
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    
    if(result == 0)
        TxMsg[CotLocation] = ACTCON;
    else
        TxMsg[CotLocation] = 0x40|ACTCON;

    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength; 
    
    TxMsg[FramePos++] = 2;    //附加数据包类型
    TxMsg[FramePos++] = FR_WR_FILE_ACTCON;
    TxMsg[FramePos++] = result;
    
    //文件名
    namelen = FtInfo.namelen;
    TxMsg[FramePos++] = namelen;
    memcpy(&TxMsg[FramePos], FtInfo.name, namelen);
    FramePos += namelen;
    
    //4字节文件标示文件ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
     
    TxMsg[FramePos++] = LLBYTE(FtInfo.FileSize); 
    TxMsg[FramePos++] = LHBYTE(FtInfo.FileSize);
    TxMsg[FramePos++] = HLBYTE(FtInfo.FileSize);
    TxMsg[FramePos++] = HHBYTE(FtInfo.FileSize);  
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);  
   
    
}

/*------------------------------------------------------------------/
函数名称：  ProcFT_WriteFileAct()
函数功能：  写文件激活。记录文件名，初始化相关参数等
输入说明：    
输出说明：  
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_WriteFileAct(void)
{
    //char name[40];
    INT16U namelen, i;
    INT32U filesize, fileid;
    INT8U *pdata;
    //INT8U result;
    
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
    
    SetSendData2Flag(DATA2_FT_WTFILEACT);
    
}

/*------------------------------------------------------------------/
函数名称：  ProcFT_EncodeWriteFileDataConf()
函数功能：  写文件传输结束，回确认帧。
输入说明：    
输出说明：  
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_EncodeWriteFileDataConf(void)
{
    INT8U FramePos;
    INT16U jj;

    //组织返回数据
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
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
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
        
    logSysMsgNoTime("写文件确认帧%s, err=%d, offset=%d", (INT32U)FtInfo.name, FtInfo.errinfo, FtInfo.offset,0);
     
    FT_ParaReset(&FtInfo);  //文件传输结束，清参数
    
}

/*------------------------------------------------------------------/
函数名称：  ProcFT_WriteFileData()
函数功能：  传输要写的文件内容
输入说明：  
输出说明：  
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_WriteFileData(void)
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
            //正常情况下，发文件结束了，在这里发确认帧
            //这里很可能发不出去
            SetSendData2Flag(DATA2_FT_WTDATAACT);     //提前回确认帧，需要测试主站是否能马上进行通信，程序应该有写flash的时间
        } 
        segmentlen = (LengthIn) - AsduHeadLength - 12;   // AsduHeadLength=9  12=除数据块的其他字节
        //logSysMsgNoTime("offset=%d, segmentlen=%d",offset,segmentlen,0,0);
        
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
    
    //异常有错误的情况下，在这里发确认帧
    if((rc == TRUE) && (IsNoFinish))
        SetSendData2Flag(DATA2_FT_WTDATAACT);
    
    
    
}

void CSecAppSev::ProcFileTran(void)
{
    //INT8U len;
    
    /*if(INFOADDR2BYTE)
    {
        pRxData = &RxMsg[InfoAddrLocation+2];
        //logSysMsgNoTime("收到文件传输命令%x=%x-%x-%x",pRxData[0],pRxData[1],pRxData[2],pRxData[3]);
    }*/
    
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
            //无需应用层回答
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
函数名称：  ProcFT_ProgramUpdate()
函数功能：  软件升级命令处理。
输入说明：    
输出说明：  
备注：      暂时这样处理，使用错误的镜像回答方式。遗留问题是会影响回答的优先级，一般问题不大
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_ProgramUpdate(void)
{
    ProgramUpadateSE = pRxData[0];
    if((RxCot&COT_REASON) == ACT)
    {
        
        if(pRxData[0] & 0x80) //CTYPE
        {   
            //升级启动   
            FtInfo.IsUpdate = TRUE;
            ProgramUpadateCot = ACTCON;
            
        }
        else
        {
            //升级结束
            FT_ParaReset(&FtInfo);
            ProgramUpadateCot = ACTCON; 
            StartProgramUpdate();
        }
    }
    else
    {
        //升级撤销
        FT_ParaReset(&FtInfo);
        ClearProgramUpdate();
        ProgramUpadateCot = DEACTCON;
    }
    
    SetSendData2Flag(DATA2_PUP_PROGUP);
    
}
/*------------------------------------------------------------------/
函数名称：  RMTReadAllPara()
函数功能：  组织读全部参数
输入说明：  
输出说明：  TRUE-有后续 FALSE-无后续
            运行参数使用整数上传和设置。定值参数用浮点数上传和设置。
/------------------------------------------------------------------*/
BOOL CSecAppSev::RMTReadAllPara(INT8U *pbuf, INT8U *plen, INT8U *psendnum)
{
    INT16U info, FramPos;
    INT8U len, i, temp;
    INT16U sendnum,fnum;
    
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
            
            len = 0;
            GetTerminalPara(&pbuf[FramPos], &len, info, 1);
            FramPos += len;
            
        }
        *plen = FramPos;
        RMTHaveReadParaFlag = 3;
        break;
    case 3:
        //上装终端线路运行参数
        fnum = GetFeederNum();
        for(i=0; i<fnum; i++)
        {
            for(info= RMTP_RUN2_L+i*RMTP_RUN_NUM; info<=RMTP_RUN2_H+i*RMTP_RUN_NUM; info++)
            {
                sendnum++;
                pbuf[FramPos++] = LOBYTE(info);
                pbuf[FramPos++] = HIBYTE(info);
                
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
            
            len = 0;
            GetTerminalPara(&pbuf[FramPos], &len, info, 1);
            FramPos += len;
        }
        fnum = GetFeederNum();
        RMTParaNum++;
        if(RMTParaNum >= fnum)
        {
            RMTHaveReadParaFlag = 0;
        }
        
        *plen = FramPos;
        break;
    default:
        RMTHaveReadParaFlag = 0;
        
        break;    
    }
    
    *psendnum = sendnum;
    
    if(RMTHaveReadParaFlag)
        return TRUE;
    else
        return FALSE;
    
}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXReadPara()
函数功能：  组织读参数
输入说明：  
输出说明：  rc = TRUE-有后续 FALSE-无后续
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcEncodeGXReadPara(void)
{
    INT16U jj; 
    INT8U len, sendnum;
    INT16U FramePos;
    BOOL rc,rc2;
    
    
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
            TxMsg[1] = sendnum | 0x80;
            if(rc == FALSE) 
            { 
                rc2 = FALSE;
                //RMTParaInit();                      此处为何要调用此函数 20181018？
            }
            TxMsg[FramePos++] = 0x06;
            break;
            
        case INTRO1:
            
            rc = GXReadCommonPara(&TxMsg[FramePos], &len, &sendnum,InfoAddrSize);
            FramePos += len;
            TxMsg[1] = sendnum | 0x80;
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
             TxMsg[1] = sendnum | 0x80;
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
       
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
    return rc2;   
}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXSetPara()
函数功能：  组织回复预置参数的报文
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeGXSetPara(void)
{
        
    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=GXReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);
}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXActivatePara()
函数功能：  组织回复激活参数的报文
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeGXActivatePara(void)
{
   
    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=GXReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);    
}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXChangePara()
函数功能：  组织回复改变参数的报文
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeGXChangePara(void)
{
    INT16U jj; 
    INT16U i;
    INT16U FramePos; 
    float  value;
    INT8U *p;  
            
    TxMsg[0] = P_ME_NA_1_GX;
    TxMsg[1] = GXParaNum;        //vsq
    TxMsg[CotLocation] = 3;
    
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);       
        
    FramePos = PubAddrLocation+PubAddrSize;   
    
    for(i=0; i<GXParaNum; i++)
    {
        TxMsg[FramePos++] = LOBYTE(GXParaInfo[i]);
        TxMsg[FramePos++] = HIBYTE(GXParaInfo[i]);
    
        value = GxGetParaValue(GXParaInfo[i]);
        p = (INT8U*)&value;
        TxMsg[FramePos++] = p[3];
        TxMsg[FramePos++] = p[2];
        TxMsg[FramePos++] = p[1];
        TxMsg[FramePos++] = p[0];

    } 
    
    TxMsg[FramePos++] = 6;  //QPM
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);  
}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeGXSendPara()
函数功能：  组织主动发送参数的报文
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcEncodeGXSendPara(void)
{
    INT8U len, sendnum;
    INT16U FramePos, jj;
    BOOL rc;  
            
    TxMsg[0] = P_ME_NA_1_GX;
    //TxMsg[1] = GXParaNum;        //vsq
    TxMsg[CotLocation] = 3;
    
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);       
        
    FramePos = PubAddrLocation+PubAddrSize;   
    
    rc = GXReadAllPara(&TxMsg[FramePos], &len, &sendnum,InfoAddrSize,GXParaControl);
    GXParaControl++;
    FramePos += len;
    TxMsg[1] = sendnum | 0x80;
    TxMsg[FramePos++] = 0x06;   //QPM
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);   
    
    return rc;
}  
/*------------------------------------------------------------------/
函数名称：  ProcEncodeRMTReadPara()
函数功能：  组织读参数
输入说明：  
输出说明：  rc2 = TRUE-有后续 FALSE-无后续
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcEncodeRMTReadPara(void)
{
    INT16U i; 
    INT8U len, sendnum,procnum;
    INT16U FramePos, piCodePosition;
    BOOL rc, rc2;
    INT16U jj;
    
    rc2 = TRUE;
        
    TxMsg[0] = C_RS_NA;
    TxMsg[1] = 0;   //VSQ，后面填个数
    
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    
    FramePos = PubAddrLocation+PubAddrSize;    
    
    
    TxMsg[FramePos++] = 1;   //区号
    TxMsg[FramePos++] = 0;
    piCodePosition = FramePos++;    //pi码位置
 
    if(RMTParaReadAllFlag)
    {
        rc = RMTReadAllPara(&TxMsg[FramePos], &len, &sendnum);   
        FramePos += len;
        TxMsg[1] = sendnum;
        if(rc == FALSE) 
        {
            TxMsg[piCodePosition] = 0;     //参数特征，无后续 
            rc2 = FALSE;
            RMTParaInit();
        }
        else
        {
            TxMsg[piCodePosition] = RP_PI_CONT;     //参数特征，有后续
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
            
            GetTerminalPara(&TxMsg[FramePos+2], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {
                sendnum++;
                TxMsg[FramePos++] = LOBYTE(RMTParaInfo[i]);  //信息体地址
                TxMsg[FramePos++] = HIBYTE(RMTParaInfo[i]);
                
                FramePos += len;
            }
            
            if(FramePos >= 200)
                break;
        }
        logSysMsgNoTime("参数读取个数(起始=%d, 发送=%d, 总=%d)",RMTHaveReadParaFlag,sendnum,RMTParaNum,0);

        TxMsg[1] = sendnum;
        RMTHaveReadParaFlag += procnum;
        //结束传送
        if(RMTHaveReadParaFlag >= RMTParaNum) 
        {
            RMTParaInit();
            TxMsg[piCodePosition] = 0;     //参数特征，无后续 
            rc2 = FALSE;
        }
        else
        {
            TxMsg[piCodePosition] = RP_PI_CONT;     //参数特征，有后续
        }
        
    }
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
    
    return rc2;
    
}
/*------------------------------------------------------------------/
函数名称：  ProcReadParaGX
函数功能：  处理召唤参数报文，取出ROI（参数召唤限定词）。
输入说明：    
输出说明：  无。
备注：      ROI是表明主站要读取全参数还是分组召唤参数。
/------------------------------------------------------------------*/
void CSecAppSev::ProcReadParaGX(void)
{
    EditReadParaCon=0xff;
    Roi = RxMsg[InfoAddrLocation + 2];
    
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
    SetSendData2Flag(DATA2_GX_READPARA);
}
/*------------------------------------------------------------------/
函数名称：  ProcSetParaGX
函数功能：  处理预置参数报文。
输入说明：    
输出说明：  无。
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcSetParaGX(void)
{
    INT8U *pInfoAddr;
    INT16U i,pos;
    float tempval;
    INT32U temp32;
    INT16U infoaddr;
    
    //把数据暂存到wrongdata中
    WrongDataLength = LengthIn;
    GXvsqflag = 0;
    //memcpy((void*)WrongData,(void*)(&RxMsg[InfoAddrLocation]),LengthIn-PubAddrSize-3);
    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
    if(RxMsg[LengthIn-1] == 9)  //QPM 
    {
        if((RxCot&COT_REASON)==ACT)    
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
                    pos += 2;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("GX预置参数info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                } 
                            
                
                //SetSendData2Flag(DATA2_GX_SETPARA);            
            }
            else
            {
                GXvsqflag = 1;
                //GXParaInfo[0] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
                infoaddr = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                pos += 2;
                for(i=0; i<GXParaNum; i++)
                {
                    GXParaInfo[i] = infoaddr + i;
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("GX预置参数info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                }
            } 
            
            if(GXRemoteParaCheck() == 1)
            {
                //参数异常 （应调用变量清0函数）
                GXParaYZ = FALSE;
                GXParaInfo[0] = 0;  
                GXParaNum = 0;              
                GXReturnCot = ACTCON|0x40;  //否定回答
            }
            else
            {
                //参数正确
                GXParaYZ = TRUE;
                GXReturnCot = ACTCON;  
            }         
        }
        else if((RxCot&COT_REASON)==DEACT)
        {
            //撤销
            //GXParaNum = 0;
            GXParaYZ = FALSE;
            GXParaInfo[0] = 0;        
            //RMTParaInit();        
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
        ProgLogWrite2("GX预置参数 QPM=%d 错误",RxMsg[LengthIn-1],0,0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
        
        /*if((RxCot&COT_REASON)==ACT)
        {
            GXReturnCot = ACTCON | 0x40;
            GXParaNum = RxVsq&VSQ_NUM;      //有用吗？ll
            if((RxVsq & VSQ_SQ) == 0)
            {
                GXvsqflag = 0;
            }    
            else
            {
                GXvsqflag = 1;
            }
        }*/
    }
    SetSendData2Flag(DATA2_GX_SETPARA);        
}
/*------------------------------------------------------------------/
函数名称：  GXRemoteParaCheck
函数功能：  逐一检查参数异常
输入说明：    
输出说明：  
备注：      有异常停止后续检查
/------------------------------------------------------------------*/
INT16U CSecAppSev::GXRemoteParaCheck(void)
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
函数名称：  ProcActivateParaGX
函数功能：  处理激活参数报文。
输入说明：    
输出说明：  无。
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcActivateParaGX(void)
{
    INT8U i;
    INT16U SetFlag;
    
    //把数据暂存到wrongdata中
    WrongDataLength = LengthIn;
    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
    if(GXParaYZ)
    {
    
        Qpa = RxMsg[InfoAddrLocation + 2];
        
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
                //logSysMsgNoTime("101广西遥测死区值在线起效",0,0,0,0);
            }
            
            
            GXParaYZ = FALSE; 
            GXReturnCot = ACTCON;  
            
            ActiveParaCon = 1;     
        }
        else
            GXReturnCot = ACTCON|0x40; 
    }
    else
    {
        GXReturnCot = ACTCON|0x40;
        logSysMsgNoTime("GX远程参数未预置就固化101",0,0,0,0);
        
    }
    
    SetSendData2Flag(DATA2_GX_ACTIVATEPARA);        
}
/*------------------------------------------------------------------/
函数名称：  GXWatchLPChange
函数功能：  广西远程参数。监视本地参数变化。
输入说明：    
输出说明：  无。
备注：      ROI是表明主站要读取全参数还是分组召唤参数。
/------------------------------------------------------------------*/
void CSecAppSev::GXWatchLPChange(void)
{
    INT16U no;
    
    if(LCFlag)
        return;
    
    no = GXGetChangeYxFlag();
    
    if(no == 1)
    {
        ActiveParaCon = 3;  
        GXParaControl = 1;  //2 * (Roi - INTROGEN) - 2;
        SetSendData2Flag(DATA2_GX_ACTIVATEPARA);
    }
}
/*------------------------------------------------------------------/
函数名称：  ProcReadPara()
函数功能：  处理读参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcReadPara(void)
{
    INT8U *pInfoAddr;
    INT16U i, pos;
    INT8U num;
    
    //数据长度大于8，表示是部分读取，否则为全部读取
    if(LengthIn > 9) //FrameLen+2是去掉控制域C的长度
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pInfoAddr = &RxMsg[InfoAddrLocation+2];
        pos = 0;
        num = RxVsq&0x7f;   //计算读取参数的个数
        
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
            pos += 2;
        }
        RMTParaNum += num;  //记录累计下发的读参数个数
        
        logSysMsgNoTime("sec=%d, num=%d, max=%d, info2=%x",RMTSectionNo,num,RMTParaNum,RMTParaInfo[RMTParaNum]);
        
    }
    else
    {
        //全部读取   
        RMTParaReadAllFlag = TRUE;
        RMTHaveReadParaFlag = 1;
        RMTParaNum = 0;
    }
    
    SetSendData2Flag(DATA2_RMT_READPARA);
        
}
/*------------------------------------------------------------------/
函数名称：  RMTParaYzCheck()
函数功能：  
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::RMTParaYzCheck(void)
{
    if(RMTParaYZ == FALSE)
    {
        RMTTimeOut = 0;
        return;
    }
    
    RMTTimeOut++;
    if(RMTTimeOut >= 30)
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
void CSecAppSev::GXParaYzCheck(void)
{
    GXWatchLPChange();
    
    if(GXParaYZ == FALSE)
    {
        GXTimeOut = 0;
        return;
    }
    
    GXTimeOut++;
    if(GXTimeOut >= 60)
    {
        GXParaYZ = FALSE;
    }
}
/*------------------------------------------------------------------/
函数名称：  RMTParaInit()
函数功能：  远程参数读写相应标志初始化
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::RMTParaInit(void)
{
    INT16U i;
    
    RMTHaveReadParaFlag = 0;
    RMTParaReadAllFlag = 0;
    RMTSectionNo = 0;
    RMTParaNum = 0;
    RMTParaYZ = 0;
    
    for(i=0;i<RMT_RW_MAXNUM;i++)
    {
        RMTParaInfo[i] = 0;
        RMTParaValue[i] = 0;
    }
}
/*------------------------------------------------------------------/
函数名称：  SetSendData2Flag()
函数功能：  设置需要传送的2级数据标志
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::SetSendData2Flag(INT32U flag)
{
    Data2Flag |= flag;     
    *LengthOut=0;
    *AppCommand = APP_NOJOB;
    
}
/*------------------------------------------------------------------/
函数名称：  DeadValueRealTimeEffect()
函数功能：  下发死区参数实时起效
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::DeadValueRealTimeEffect(void)
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
void CSecAppSev::ProcWritePara(void)
{
    INT8U pi;
    INT8U *pInfoAddr;
    INT16U i,pos;
    INT16U ParaFlag;
    BOOL IsSuccess;
    INT8U datatype, datalen;
    float tempval;
    INT32U temp32;
    
    //把数据暂存到wrongdata中
    WrongDataLength = LengthIn;
    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
    if((RxCot&COT_REASON)==ACT)    //激活
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pi  = RxMsg[InfoAddrLocation+2];
        pInfoAddr = &RxMsg[InfoAddrLocation+3];
       
        pos = 0;
        if(pi & RP_PI_SE)   //预置
        {
            if(RMTParaYZ)   //只接受一帧报文的预置，没处理完不接受其他预置报文
            {
                logSysMsgNoTime("上一帧参数预置报文还未处理101",0,0,0,0);
                return;
            }
            RMTParaNum = RxVsq&VSQ_NUM;    //暂时不考虑 固有参数的写入
            for(i=0; i<RMTParaNum; i++)
            {
                RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
                pos += 2;
                
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
                RMTReturnCot = ACTCON;
                SetSendData2Flag(DATA2_RMT_SETPARA);
            }
            else
            {
                RMTReturnCot = COT_PONO|ACTCON;
                SetSendData2Flag(DATA2_RMT_SETPARA);
            
            }
                
        }
        else
        {
            //固化
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
                RMTReturnCot = ACTCON;
                
                
                
                if(IsSuccess==FALSE)
                {
                	RMTReturnCot |= 0x40; //失败
                }
                else
                {
                    if(GetSiQuChangeFlag(ParaFlag))           //规约层的参数实时起效都可以同样按照死区值的实时起效的方式实现。
                    {
                        DeadValueRealTimeEffect();
                    }
                    SaveTerminalPara(); //固化完毕，写入flash
                    SaveRMTParaToSys();
                }
                
                SetSendData2Flag(DATA2_RMT_SETPARA);
                
            
            }  
        }
        
    }
    else
    {
        //撤销
        RMTParaNum = 0;
        RMTParaYZ = 0;
        RMTParaInfo[0] = 0;
        
        RMTParaInit();
        
        RMTReturnCot = DEACTCON;
        SetSendData2Flag(DATA2_RMT_SETPARA);
        
    }
  
}
/*------------------------------------------------------------------/
函数名称：  ProcSetSectionNo()
函数功能：  切换定值区号
输入说明：  
输出说明：  
备注：      暂时不支持多区号
/------------------------------------------------------------------*/
void CSecAppSev::ProcSetSectionNo(void)
{
    //INT8U SectionNo;
    
    
    //SectionNo = MAKEWORD(pRxData[0],pRxData[1]);
    
    SetSendData2Flag(DATA2_RMT_SETSEC);
    
}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeSetSectionNo()
函数功能：  设置定值区号
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeSetSectionNo(void)
{
    INT8U FramePos;
    INT16U jj;
    
    TxMsg[0] = C_SR_NA;
    TxMsg[1] = 1;   //VSQ
    
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
    TxMsg[FramePos++] = 0;  //当前区号
    TxMsg[FramePos++] = 0;
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
}

/*------------------------------------------------------------------/
函数名称：  ProcReadSectionNo()
函数功能：  读定值区号
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcReadSectionNo(void)
{
    
    SetSendData2Flag(DATA2_RMT_READSEC);
       
}


/*------------------------------------------------------------------/
函数名称：  ProcEncodeRMTSetPara()
函数功能：  组织读定值区号确认帧
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeRMTSetPara(void)
{

    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=RMTReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);
    
}
/*------------------------------------------------------------------/
函数名称：  ProcEncodePUPupdateConf()
函数功能：  组织应答帧
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodePUPupdateConf(void)    
{   
    INT8U FramePos; 
    INT16U jj;
    
    TxMsg[0] = F_SR_NA_N;
    TxMsg[1] = 0;   //VSQ
    
    TxMsg[CotLocation] = ProgramUpadateCot;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;  
    
    TxMsg[FramePos++] = ProgramUpadateSE;
    /*if(FtInfo.IsUpdate)
    {
        //启动
        TxMsg[FramePos++] = 0x80;   
    }
    else
    {
        //升级结束，取消升级
        if(ProgramUpadateCot == ACTCON)
        {
            //升级结束
            TxMsg[FramePos++] = 0; 
        }
        else
        {
            //取消升级
            TxMsg[FramePos++] = 0; 
        }
    }*/
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
    
}

/*------------------------------------------------------------------/
函数名称：  void ProcEncodeXSFileSynConf(void)
函数功能：  组织线损文件同步确认帧
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeXSFileSynConf(void)    
{   
    INT8U FramePos; 
    INT16U jj;
    
    TxMsg[0] = F_FS_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
     
    FramePos = AsduHeadLength;      
    TxMsg[FramePos++] = 0;
         
    EnCode101DLMsg(FramePos, APP_SENDDATA);   
    
}

/*------------------------------------------------------------------/
函数名称：  void ProcEncodeXSFileSynFinish(void)
函数功能：  组织线损文件同步激活终止帧
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeXSFileSynFinish(void)
{
    INT8U FramePos; 
    INT16U jj;
    
    TxMsg[0] = F_FS_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=ACTTERM;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
     
    FramePos = AsduHeadLength;      
    TxMsg[FramePos++] = 0;
         
    EnCode101DLMsg(FramePos, APP_SENDDATA);      
}

/*------------------------------------------------------------------/
函数名称：  ProcEnCodeReadSectionNo()
函数功能：  组织读定值区号确认帧
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEnCodeReadSectionNo(void)    
{   
    INT8U FramePos; 
    INT16U jj;
    
    TxMsg[0] = C_RR_NA;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    //信息体地址
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;    

    TxMsg[FramePos++] = 1;  //当前区号
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;  //最小区号
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;  //最大区号
    TxMsg[FramePos++] = 0;
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
    
}


/*------------------------------------------------------------------/
函数名称：  EnCode101DLMsg()
函数功能：  组织发送数据长度和命令
输入说明：  len 发送数据长度   appcmd 发送数据命令，如果是APP_SENDDATA，则平衡模式下自动转为APP_SENDCON
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::EnCode101DLMsg(INT16U len, INT16U appcmd)
{
    *LengthOut  = len;  
    
    *AppCommand = appcmd;
    if((appcmd == APP_SENDDATA) && (BalanMode))
        *AppCommand = APP_SENDCON;
    
}

/*------------------------------------------------------------------/
函数名称：  JudgeSendInitEnd()
函数功能：  判断是否发生初始化结束
输入说明：  
输出说明：  
备注：      InitFlag 0xff表示未发送过。 0表示发送过
/------------------------------------------------------------------*/
void CSecAppSev::JudgeSendInitEnd(void)
{
    if(BalanMode)//平衡模式
    {
        if(IsAllSendInitEnd || (IsAllSendInitEnd==FALSE && (InitFlag==0xFF)))
        {
            Data1.Flag|=HaveInitEnd;
        }
    }
    else
    {
        
        if(InitFlag == 0xFF)
        {
            Data1.Flag|=HaveInitEnd;
            *AppCommand|=APP_HAVEDATA1;
        }
        else//终端未复位
        {
            if(IsAllSendInitEnd)        //添加，每次重新建立链接发送初始化结束帧的选项。 ll
            {
                Data1.Flag|=HaveInitEnd;
                *AppCommand|=APP_HAVEDATA1;
            }
        }
        
    }
        
}

//组织一个SPI值，为了测试
INT8U CSecAppSev::EditTestSPI(void)
{
    (*pTxVSQ) = 1;

    *pTxData     = 10;//vti
    *(pTxData+1) = 0; //qds

    return(2);
}


//文件处理
void CSecAppSev::ProcFileCall(void)
{
    struct AbsTime_t absTime;

    Data2Flag|=UpLoadFile;
    CurrentInfoAddr=RxInfoAddr;
    CurrentFileName=RxMsg[AsduHeadLength]+(RxMsg[AsduHeadLength+1]<<8);

    if((RxCot&COT_REASON)==REQ)//召唤目录
    {
        FileStep=PCallDir;
        if(CurrentInfoAddr&LUBOFLAG)
            ReadDirList1();
        else
            ReadDirList2();
    }
    else if((RxCot&COT_REASON)==FILE_101)
    {
        switch(RxMsg[AsduHeadLength+3]&0x0F)
        {
            case 0://缺省
                break;
            case 1://选择文件
                FileStep=PSelectFile;
                if(CurrentInfoAddr&LUBOFLAG)//故障录波数据
                {
                    CurrentFileSize=FILELENGTH;//文件长度
                    FileReadPtr=0;//已经发送数据长度指针
                    CurrentZBNo=0;//当前要读数据的周波号
                    FileCheckSum=0;//文件数据校验和
                }
                else//历史电度数据 zzw
                {
                    FileCheckSum=0;
                    //时间的格式需要确定
                    HisDDTime.MSecond=0;
                    HisDDTime.Minute=CurrentFileName&0xff;
                    HisDDTime.Hour=(CurrentFileName>>8)&0xff;
                    HisDDTime.Day=CurrentInfoAddr&0x1f;
                    HisDDTime.Month=(CurrentInfoAddr>>5)&0xf;
                    HisDDTime.Year=((CurrentInfoAddr>>9)&0x3f);

                    ConvToAbsTime((void*)&HisDDTime,&absTime,IEC101CLOCKTIME);

                    if(!GetActDevIndexByAddr(RxPubAddr))
                    {
                        CurrentFileSize=0;
                        break;
                    }
                    HisDDDevNo=ActDevIndex;//设备序号
                    //读历史电度
                    if(ReadKWHHistoryData(absTime,(long *)DevList[HisDDDevNo].DevData.HisCounterData,DevList[HisDDDevNo].DevData.CounterNum)==1)
                    {
                        DevList[HisDDDevNo].DevData.HisDDReadPtr=0;
                        CurrentFileSize=(2+12)*DevList[HisDDDevNo].DevData.CounterNum;
                    }
                    else
                    {
                        CurrentFileSize=0;
                    }
                }
                break;
            case 2://请求文件
                FileStep=PCallFile;

                break;
            case 3://停止激活文件
                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                CurrentFileSize=0;
                FileReadPtr=0;
                CurrentZBNo=0;
                break;
            case 4://删除文件
                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                break;
            case 5://选择节，实际通讯过程没有这一步。
                FileStep=PCallSection;
                break;
            case 6://请求节
                FileStep=PCallSection;
                break;
            case 7://停止激活节
                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                CurrentFileSize=0;
                FileReadPtr=0;
                CurrentZBNo=0;
                break;
            default:
                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                CurrentFileSize=0;
                FileReadPtr=0;
                CurrentZBNo=0;
                break;
        }
    }

    *LengthOut=0;
    *AppCommand=APP_APPCON;
    if(Data1.Flag)
        *AppCommand|=APP_HAVEDATA1;
}

//发送录波数据目录
void CSecAppSev::SendDir(void)
{
    int jj;
    short SendNum,FramePos,j,Length;

    TxMsg[0]=F_DR_NA;//目录
    (*pTxVSQ) |= VSQ_SQ;

    TxMsg[CotLocation]=REQ;
    if(CurrentInfoAddr&LUBOFLAG)
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    }
    else
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[HisDDDevNo].Addr>>(8*jj);
    }

    SendNum=0;
    FramePos=-2;
    if(InfoAddrSize==3)
        FramePos=-3;
    if (DirUnit.FileNum>0)
    {
        Length=ASDULEN-AsduHeadLength-13;//应用层发送信息最大长度
        jj=0;
        for(j=DirUnit.ReadPtr;j<DirUnit.FileNum;j++)
        {
            jj++;
            if(DirUnit.File[j].SendFlag)//已经发送的文件本次就不再发送
                continue;
            if((FramePos==-2)||(FramePos==-3))
            {
                *pTxInfoAddr    =LOBYTE((DirUnit.File[j].InfoAddr));
                *(pTxInfoAddr+1)=HIBYTE((DirUnit.File[j].InfoAddr));
                FramePos+=InfoAddrSize;
            }
            /*else
            {
                *(pTxData+FramePos)=LOBYTE(DirUnit.File[j].InfoAddr);//信息体地址
                *(pTxData+FramePos+1)=HIBYTE(DirUnit.File[j].InfoAddr);//信息体地址
                FramePos+=InfoAddrSize;
            }*/

            pTxData[FramePos++]=LOBYTE(DirUnit.File[j].Name);
            pTxData[FramePos++]=HIBYTE(DirUnit.File[j].Name);
            pTxData[FramePos++]=LOBYTE(LOWORD(DirUnit.File[j].Length));
            pTxData[FramePos++]=HIBYTE(LOWORD(DirUnit.File[j].Length));
            pTxData[FramePos++]=LOBYTE(HIWORD(DirUnit.File[j].Length));

            //pTxData[FramePos++]=HIBYTE(DirUnit.File[j].Status);
            pTxData[FramePos++]= DirUnit.File[j].Status;                //根据coverity更改

            pTxData[FramePos++]=LOBYTE(DirUnit.File[j].Time.MSecond);
            pTxData[FramePos++]=HIBYTE(DirUnit.File[j].Time.MSecond);
            pTxData[FramePos++]=DirUnit.File[j].Time.Minute;
            pTxData[FramePos++]=DirUnit.File[j].Time.Hour;
            pTxData[FramePos++]=DirUnit.File[j].Time.Day;
            pTxData[FramePos++]=DirUnit.File[j].Time.Month;
            pTxData[FramePos++]=DirUnit.File[j].Time.Year;

            SendNum++;//发送个数

            if(FramePos>=Length)
                break;
        }
        DirUnit.ReadPtr+=jj;
    }
    if(SendNum)
    {
        *LengthOut=AsduHeadLength+FramePos;
        if(DirUnit.ReadPtr>=DirUnit.FileNum)
        {
            pTxData[FramePos-8]|=0x20;//最后一个文件的状态
            Data2Flag&=(~UpLoadFile);
            FileStep=PFileOver;
        }
    }
    else
    {
        *pTxInfoAddr=0;
        *(pTxInfoAddr+1)=0;
        *LengthOut=AsduHeadLength;
        Data2Flag&=(~UpLoadFile);
        FileStep=PFileOver;
    }
    (*pTxVSQ) |= SendNum;

    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}


void CSecAppSev::FileReady(void)
{
    int jj;
    TxMsg[0]=F_FR_NA;//文件准备就绪

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;
    if(CurrentInfoAddr&LUBOFLAG)
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    }
    else
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[HisDDDevNo].Addr>>(8*jj);
    }

    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *pTxData=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);

    *(pTxData+2)=LOBYTE(LOWORD(CurrentFileSize));
    *(pTxData+3)=HIBYTE(LOWORD(CurrentFileSize));
    *(pTxData+4)=LOBYTE(HIWORD(CurrentFileSize));//长度的最高8位。
    if(CurrentFileSize)
        *(pTxData+5)=0;//肯定
    else
        *(pTxData+5)=0x80;//否定

    *LengthOut=AsduHeadLength+6;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;

    Data2Flag&=(~UpLoadFile);
    FileStep=PFileOver;
}

void CSecAppSev::SectionReady(void)
{
    int jj;
    TxMsg[0]=F_FR_NA;//节准备就绪

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;
    if(CurrentInfoAddr&LUBOFLAG)
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    }
    else
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[HisDDDevNo].Addr>>(8*jj);
    }

    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *(pTxData)=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);
    *(pTxData+2)=01;//一个文件认为是一个节。
    *(pTxData+3)=LOBYTE(LOWORD(CurrentFileSize));
    *(pTxData+4)=HIBYTE(LOWORD(CurrentFileSize));
    *(pTxData+5)=LOBYTE(HIWORD(CurrentFileSize));//长度的最高8位。
    if(CurrentFileSize)
        *(pTxData+6)=0;//节准备就绪
    else
        *(pTxData+6)=0x80;//节未准备就绪

    *LengthOut=AsduHeadLength+7;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;

    Data2Flag&=(~UpLoadFile);
    FileStep=PFileOver;
}

void CSecAppSev::SendSegment(void)
{
    if(CurrentInfoAddr&LUBOFLAG)//故障录波数据
    {
        #ifdef _SAVESAMPDATA_
        SendSegment1();
        #endif
    }
    else//历史电度数据
    {
        SendSegment2();
    }
}
#ifdef _SAVESAMPDATA_
//发送录波数据
void CSecAppSev::SendSegment1(void)
{
    short Length,i;
    int jj;
    INT8U *p;

    short value1,value2;
    long  tempvalue;

    TxMsg[0]=F_SG_NA;//段数据

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *(pTxData)=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);
    *(pTxData+2)=01;//节名

    //读数据，每次读3个周波的数据
    //Length=readLBData((INT8U)(CurrentFileName-1),CurrentZBNo,3,(INT8U *)(pTxData+4),192);
    //CurrentZBNo+=3;

    //每次取2个周波，计算后发送前1个周波
    Length=readLBData((INT8U)(CurrentFileName-1),CurrentZBNo,2,(INT8U*)Data,300);
    if(Length==0)
    {
        logSysMsgNoTime("读录波数据错误",0,0,0,0);
    }

    CurrentZBNo++;

    p=(INT8U *)(pTxData+4);
    //将前周波的32点扩展为64点,128字节。写入发送缓冲区
    for(i=0;i<32;i++)
    {
        value1=Data[i];
        value2=Data[i+1];
        p[4*i]=value1&0xff;
        p[4*i+1]=(value1>>8)&0xff;

        tempvalue=value1+value2;
        tempvalue=tempvalue/2;
        value2=tempvalue&0xffff;
        p[4*i+2]=value2&0xff;
        p[4*i+3]=(value2>>8)&0xff;
    }

    Length=128;
    *(pTxData+3)=Length;//段长度

    for(i=0;i<Length;i++)//累加所发送数据的校验和
        FileCheckSum+=*(pTxData+4+i);

    FileReadPtr+=Length;
    if(FileReadPtr>=CurrentFileSize)
        FileStep=PLastSegment;

    *LengthOut=AsduHeadLength+4+Length;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}
#endif
//发送历史电度数据
void CSecAppSev::SendSegment2(void)
{
    short Length,i,FramePos;
    int jj;
    INT8U *p;
    INT16U unitlength;
    INT16U No,SendNum;
    INT32U *pKWH;
    INT32U KWHValue;

    TxMsg[0]=F_SG_NA;//段数据

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[HisDDDevNo].Addr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *(pTxData)=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);
    *(pTxData+2)=01;//节名

    p=(INT8U *)(pTxData+4);

    i=ASDULEN-AsduHeadLength-14-4;//可用于发送数据的最大报文长度
    SendNum=0;
    FramePos=0;
    unitlength=2+12;//每个电度的长度，这里设定信息体地址为2字节
    No=DevList[HisDDDevNo].DevData.HisDDReadPtr;
    pKWH=DevList[HisDDDevNo].DevData.HisCounterData;
    BOOL Stop=FALSE;
    while ((FramePos<i)&&(!Stop))
    {
        KWHValue=*(pKWH+No);

        //信息体地址
        *(p+unitlength*SendNum+0)=(LBCR+No)&0xff;
        *(p+unitlength*SendNum+1)=((LBCR+No)>>8)&0xff;
        //电度值
        *(p+unitlength*SendNum+2)=LOBYTE(LOWORD(KWHValue));
        *(p+unitlength*SendNum+3)=HIBYTE(LOWORD(KWHValue));
        *(p+unitlength*SendNum+4)=LOBYTE(HIWORD(KWHValue));
        *(p+unitlength*SendNum+5)=HIBYTE(HIWORD(KWHValue));
        *(p+unitlength*SendNum+6)=(No%32);
        //时间
        *(p+unitlength*SendNum+7)=0;
        *(p+unitlength*SendNum+8)=0;
        *(p+unitlength*SendNum+9)=HisDDTime.Minute;
        *(p+unitlength*SendNum+10)=HisDDTime.Hour;
        *(p+unitlength*SendNum+11)=HisDDTime.Day;
        *(p+unitlength*SendNum+12)=HisDDTime.Month;
        *(p+unitlength*SendNum+13)=HisDDTime.Year;

        FramePos+=14;
        SendNum++;
        No++;
        if(No>=DevList[HisDDDevNo].DevData.CounterNum)
        {
            FileStep=PLastSegment;
            Stop=TRUE;
        }

        if (SendNum>=10)//每帧电度不能超过10个
            Stop=TRUE;
    }
    DevList[HisDDDevNo].DevData.HisDDReadPtr=No;

    Length=FramePos;
    *(pTxData+3)=Length;//段长度

    for(jj=0;jj<Length;jj++)//累加所发送数据的校验和
        FileCheckSum+=*(pTxData+4+jj);

    FileReadPtr+=Length;
    if(FileReadPtr>=CurrentFileSize)
        FileStep=PLastSegment;

    *LengthOut=AsduHeadLength+4+Length;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

//发送最后段
void CSecAppSev::SendLastSegment(void)
{

    int jj;
    TxMsg[0]=F_LS_NA;//最后的段

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;

    if(CurrentInfoAddr&LUBOFLAG)
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    }
    else
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[HisDDDevNo].Addr>>(8*jj);
    }
    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *(pTxData)=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);
    *(pTxData+2)=01;//节名

    *(pTxData+3)=03;//LSQ

    *(pTxData+4)=FileCheckSum;//校验和

    *LengthOut=AsduHeadLength+5;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
    //???
    Data2Flag&=(~UpLoadFile);
    FileStep=PFileOver;
}

//文件传输认可
void CSecAppSev::SConfirm(void)
{
    INT16U FileName;


    FileName=(RxMsg[AsduHeadLength])+(RxMsg[AsduHeadLength+1]<<8);

    switch(RxMsg[AsduHeadLength+3]&0x0F)//AFQ
    {
        case 1://文件传输的肯定认可，不需要发送刷新后的目录2005-5-30 16:15联系黄志勇
            Data2Flag&=(~UpLoadFile);
            FileStep=PFileOver;
            FileReadPtr=0;

            /*
            if(CurrentFileName==FileName)
            {
                for(int i=0;i<DirUnit.WritePtr;i++)
                {
                    if(DirUnit.File[i].Name==FileName)
                    {
                        DirUnit.File[i].SendFlag=0xff;
                        break;
                    }
                }
            }
            //更新目录列表，再发更新后的目录
            Data2Flag|=UpLoadFile;
            FileStep=PCallDir;
            DirUnit.ReadPtr=0;
            */
            break;
        case 2://文件传输的否定认可
            if(CurrentFileName==FileName)//重新传输？
            {

                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                FileReadPtr=0;
            }
            break;
        case 3://节传输的肯定认可

            if(CurrentFileName==FileName)
            {
                Data2Flag|=UpLoadFile;
                FileStep=PLastSection;
            }

            break;
        case 4://节传输的否定认可
            if(CurrentFileName==FileName)
            {

                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                FileReadPtr=0;
            }
            break;
        default:
            break;
    }
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    if(Data1.Flag)
        *AppCommand|=APP_HAVEDATA1;
}


//发送最后节
void CSecAppSev::SendLastSection(void)
{

    int jj;

    TxMsg[0]=F_LS_NA;//最后的段

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;
    if(CurrentInfoAddr&LUBOFLAG)
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    }
    else
    {
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[HisDDDevNo].Addr>>(8*jj);
    }
    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *(pTxData)=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);
    *(pTxData+2)=01;//节名

    *(pTxData+3)=01;//LSQ，最后的节
    *(pTxData+4)=FileCheckSum;//校验和

    *LengthOut=AsduHeadLength+5;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
    //???
    Data2Flag&=(~UpLoadFile);
    FileStep=PFileOver;
}

//检测录波数据
void CSecAppSev::ReadDirList1(void)
{
#ifdef _SAVESAMPDATA_
    int i=0;
    struct  AbsTime_t ftime_abs;
    struct Iec101ClockTime_t ftime_iec;

    //从时间判断是否有录波数据

    readLBTime((void *)&ftime_abs,ABSTIME);

    if((ftime_abs.Minute==0)&&(ftime_abs.MSecond==0))

    {
        DirUnit.FileNum=0;
        DirUnit.WritePtr=0;
        DirUnit.ReadPtr=0;
        logSysMsgNoTime("无录波数据",0,0,0,0);
        return;
    }

    //读故障录波数据的时间
    readLBTime((void *)&ftime_iec,IEC101CLOCKTIME);

    if(CurrentFileName==0)//召唤所有文件
    {
        DirUnit.FileNum=7;
        DirUnit.WritePtr=7;
        DirUnit.ReadPtr=0;

        for(i=0;i<7;i++)
        {
            DirUnit.File[i].SendFlag=0;
            DirUnit.File[i].Name=i+1;
            DirUnit.File[i].Length=FILELENGTH;//??;
            DirUnit.File[i].Status=0;
            DirUnit.File[i].InfoAddr=CurrentInfoAddr;
            DirUnit.File[i].Time=ftime_iec;
        }
    }
    else//只召唤一个文件
    {
        DirUnit.FileNum=1;
        DirUnit.WritePtr=1;
        DirUnit.ReadPtr=0;
        DirUnit.File[i].SendFlag=0;
        DirUnit.File[i].Name=CurrentFileName;
        DirUnit.File[i].Length=FILELENGTH;//??;
        DirUnit.File[i].Status=0;
        DirUnit.File[i].InfoAddr=CurrentInfoAddr;
        DirUnit.File[i].Time=ftime_iec;
    }
#endif
}

//历史电度目录
void CSecAppSev::ReadDirList2(void)
{
    INT32U i,ii,no;
    struct  AbsTime_t absTime;

    if(Sec101Pad.HistoryDDPermit == 0)
    {
        DirUnit.FileNum=0;
        CurrentFileSize=0;
        return;
    }
    if(!GetActDevIndexByAddr(RxPubAddr))
    {
        DirUnit.FileNum=0;
        CurrentFileSize=0;
        return;
    }
    HisDDDevNo=ActDevIndex;//设备序号

    HisDDTime.MSecond=0;
    HisDDTime.Day=CurrentInfoAddr&0x1f;
    HisDDTime.Month=(CurrentInfoAddr>>5)&0xf;
    HisDDTime.Year=((CurrentInfoAddr>>9)&0x3f);

    if(CurrentFileName!=0)//只传送一个文件
    {
        HisDDTime.Minute=CurrentFileName&0xff;
        HisDDTime.Hour=(CurrentFileName>>8)&0xff;
        DirUnit.FileNum=1;
        DirUnit.WritePtr=1;
    }
    else//发送该日期所有文件
    {
        HisDDTime.Minute=0;
        HisDDTime.Hour=0;
        if(HisDDCycle<=30)//小于半小时保存一次历史电度，系统设置最大目录48个文件
        {
            DirUnit.FileNum=48;
            DirUnit.WritePtr=48;
        }
        else
        {
            DirUnit.FileNum=1440/HisDDCycle;
            DirUnit.WritePtr=1440/HisDDCycle;
        }
    }

    DirUnit.ReadPtr=0;
    no=0;
    for(i=0;i<DirUnit.FileNum;i++)
    {
        HisDDTime.Minute=HisDDTime.Minute+HisDDCycle;
        while(HisDDTime.Minute>=60)
        {
            HisDDTime.Minute-=60;
            HisDDTime.Hour++;
        }

        ConvToAbsTime((void*)&HisDDTime,&absTime,IEC101CLOCKTIME);
        //查找数据

        ii=(absTime.Minute/HisDDCycle)%SAVENUM;

        if(hisDataPtr->saveData[ii].time.Minute != absTime.Minute)//没有该时间点的数据
            continue;

        DirUnit.File[no].SendFlag=0;
        DirUnit.File[no].Name=HisDDTime.Minute+(HisDDTime.Hour<<8);
        DirUnit.File[no].Length=(2+12)*DevList[HisDDDevNo].DevData.CounterNum;
        DirUnit.File[no].Status=0;
        DirUnit.File[no].InfoAddr=CurrentInfoAddr;
        DirUnit.File[no].Time=HisDDTime;
        no++;
    }
    DirUnit.FileNum=no;
    DirUnit.WritePtr=no;
}

#ifdef INCLUDE_DA
//设定保护定值
void CSecAppSev::SetProtect(void)
{
    Data1.Flag|=ProtectCon;
    if((RxCot&COT_REASON)==ACT)
    {
        memcpy((void*)ProtectValue,(void*)(pRxData),32);
        //保护定值设定调用
        if(!setDZ(ProtectValue))//设定失败，返回全部取反。
        {
            for(int i=0;i<32;i++)
                ProtectValue[i]=~ProtectValue[i];
            ProtectValue[32]=0xff;//失败标志
        }
        else
        {
            ProtectValue[32]=0;//成功标志

            logSysMsgNoTime("主站设置保护定值1:%2x %2x %2x %2x",ProtectValue[2],
            ProtectValue[3],
            ProtectValue[4],
            ProtectValue[5]);
            logSysMsgNoTime("主站设置保护定值2:%2x %2x %2x %2x",ProtectValue[6],
            ProtectValue[7],
            ProtectValue[8],
            ProtectValue[9]);
            logSysMsgNoTime("主站设置保护定值3:%2x %2x %2x %2x",ProtectValue[10],
            ProtectValue[11],0,0);
            logSysMsgWithTime("主站设置保护定值时间",0,0,0,0);
        }
    }
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
}

void CSecAppSev::SendProtectCon(void)
{
    int jj;
    TxMsg[0]=C_PF_NA;//保护定值镜像确认

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    memcpy((void*)pTxData,(void*)ProtectValue,32);
    pTxData[32]=ProtectValue[32];//限定词

    *LengthOut=AsduHeadLength+33;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;

    //保护定值设定成功后要复位FTU，使定值生效。
    if(ProtectValue[32]==0)
    {
        ResetFlag=0xff;
        ResetCount=0;
    }
}

void CSecAppSev::CallProtect(void)
{
    INT8U feederno;
    //报文中原限定词的位置改为馈线号， 2005-3-14 9:39
    feederno=RxMsg[AsduHeadLength];

    Data2Flag|=ProtectData;

    //保护定值召唤调用
    //放到ProtectValue[]中
    if(getDZ(feederno,ProtectValue))
    {
        ProtectValue[32]=0;//成功标志
    }
    else
    {
        memset(ProtectValue,0,32);
        ProtectValue[32]=0xff;//失败标志
    }

    *LengthOut=0;
    *AppCommand=APP_APPCON;
    
}

void CSecAppSev::SendProtectData(void)
{
    int jj;
    TxMsg[0]=M_PF_NA;//保护定值数据

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    memcpy((void*)pTxData,(void*)ProtectValue,32);
    pTxData[32]=ProtectValue[32];//限定词

    *LengthOut=AsduHeadLength+33;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}
#endif

//定时 保存历史电度数据
//历史电度存储只允许一个设备有电度。

void CSecAppSev::InitHisDataBuf(void)
{
    INT32U	length;
    int	i;
    INT16U CountNum;

    if(Sec101Pad.HistoryDDPermit == 0)
        return;

    rltm_evevery(HisDDCycle,0,SAVEKWHFLAG,&SaveKWHTimerID,1);

    for (i=0;i<DevCount;i++)
    {
        CountNum=DevList[i].DevData.CounterNum;
        if (CountNum<=0)
            continue;

        length=sizeof(SAVEDATABUF)+CountNum * sizeof(long)*SAVENUM;

        hisDataPtr=(struct SAVEDATABUF *)nvramMalloc(length);

        hisDataPtr->saveData[0].Data=(INT32U*)(hisDataPtr+1);
        for(int ii=1;ii<SAVENUM;ii++)
        {
            //hisDataPtr->saveData[ii].Data = hisDataPtr->saveData[ii-1].Data+CountNum*(sizeof(long));
            hisDataPtr->saveData[ii].Data = hisDataPtr->saveData[ii-1].Data+CountNum;      //根据coverity更改
        }
        logSysMsgNoTime("历史电度存储启动",0,0,0,0);

        break;//保证只处理一个设备的电度
    }
}

BOOL CSecAppSev::SaveKWHToBuf(void)
{
    struct SysTime_t time;
    struct AbsTime_t absTime;
    INT16U i,ii;
    INT32U j;
    INT32U *KWH;
    short CountNum;
    INT16U GetBeginNo,GetEndNo;

    if(Sec101Pad.HistoryDDPermit == 0)
        return 0;

    absTime.Minute=rlTimeGet(SaveKWHTimerID);
    absTime.MSecond=0;
    j= (absTime.Minute / HisDDCycle)% SAVENUM;

    GetSysTime((void *)&time,SYSTEMTIME);

    DATAFORMAT *hourData;
    hourData =&hisDataPtr->saveData[j];

    for (i=0;i<DevCount;i++)//多设备有电度时，这么处理不行。一个设备有电度可以。
    {
        if (DevList[i].DevData.CounterNum<=0)
            continue;

        GetBeginNo=0;
        GetEndNo=DevList[i].DevData.CounterNum-1;

        if(DevList[i].Flag==1)
            CountNum=CRSend_ReadCount(DevList[i].DevID,GetBeginNo,GetEndNo,(struct RealCounterNFlag_t *)DBData);
        else
            CountNum=CL_ReadCount(DevList[i].DevID,GetBeginNo,GetEndNo,(struct RealCounterNFlag_t *)DBData);
        if(CountNum<=0)
        {
            logSysMsgNoTime("读数据库电度失败",0,0,0,0);
            return(0);
        }

        KWH=(INT32U*)DBData;
        for(ii=0;ii<CountNum;ii++)
        {
            hourData->Data[ii] = KWH[ii];
        }

        memcpy(&hourData->time,&absTime,sizeof(struct AbsTime_t));

        break;//只处理一个设备的电度
    }
    return(1);
}

BOOL CSecAppSev::ReadKWHHistoryData(struct AbsTime_t absTime,long *KWH,INT16U KWHNum)
{
    INT16U i,no;
    INT8U  find;

    find = 0;
    if(Sec101Pad.HistoryDDPermit == 0)
        return 0;
    i=(absTime.Minute/HisDDCycle)%SAVENUM;

    if(hisDataPtr->saveData[i].time.Minute == absTime.Minute)
    {
        find = 1;
        no = i;
    }

    if(find)
    {
        if(KWHNum)
        {
            memcpy(KWH,hisDataPtr->saveData[no].Data,sizeof(long)*KWHNum);
        }
        else
            return(0);
    }
    else
    {
        return(0);
    }
    return(1);
}

//101任务的额线损模块文件同步处理
void CSecAppSev::ProcXSFileSyn()
{
    SetSendData2Flag(DATA2_XSFT_SYNACT);  //置相应的标志位
    SetFileSynInfoTaskIDSubstation(MySelf.AppTID);  //作为启动101主站任务开始的量
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
}


//101任务的额线损模块文件同步处理
void CSecAppSev::ProcXSFileSynFinish()
{
    SetSendData2Flag(DATA2_XSFT_SYNACTFINISH); 
}
////湖南故指老加密方案
/*------------------------------------------------------------------/
函数名称：  ProDealF0()
函数功能：  分析收到的主站安全交互数据
输入说明：  

输出说明：  0 成功 其他 失败 -10 参数错误
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProDealF0()
{
    int rc,i;
    INT8U buf[10];
    INT8U serbuf[10];
    ///g_IsHardEncryptOnline = FALSE;
    memset(buf,0,10);
    rc = Sgc1120aGetChipKeyVersion(buf);
	if(rc == 0)
	{
		buf[0] = buf[2];//密钥版本号
		Sgc1120aGetChipSerialNumID(serbuf);
	}
    //*pTxTypeID = E_SGC_F0;
    //*pTxVSQ = 0x01;
    //*pTxInfoAddr = 
    //pTxData
    if(rc == 0)
    {
        memcpy((buf+1),(serbuf+2),8);
	    TxMsg[0] = E_SGC_F0;
	    TxMsg[1] = 0x01;
	    TxMsg[2] = 0x06;
	    for(i=0;i<PubAddrSize;i++)
	        TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
	    *pTxInfoAddr    =LOBYTE((RxInfoAddr));
	    *(pTxInfoAddr+1)=HIBYTE((RxInfoAddr));
	    memcpy(pTxData,buf,9);
	    *LengthOut=AsduHeadLength+9;
    }
    else
    {
        TxMsg[0] = E_SGC_FA;
        TxMsg[1] = 0x01;
	    TxMsg[2] = 0x06;
	    for(i=0;i<PubAddrSize;i++)
	        TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
	    *pTxInfoAddr    =LOBYTE((RxInfoAddr));
	    *(pTxInfoAddr+1)=HIBYTE((RxInfoAddr));
        //if(rc == -1)
        //{
             pTxData[0] = 0x02;   
             pTxData[1] = 0x00;  
        //}
        //else if( rc == -5)
        //{
            //pTxData[0] = 0x01;   
            //pTxData[1] = 0x00;  
        //}
        //else
        //{
             //pTxData[0] = 0x00;   
             //pTxData[1] = 0x01;  	   	//其他错误
        //}

	    *LengthOut=AsduHeadLength + 2;    
    }

    *AppCommand=APP_SENDDATA;
}
/*------------------------------------------------------------------/
函数名称：  ProDealF1()
函数功能：  分析收到的主站安全交互数据
输入说明：  

输出说明：  0 成功 其他 失败 -10 参数错误
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProDealF1()
{
    int rc,i;
    INT8U buf[18];//前两个字节是信息长度
    
    AuthEndflag = 0;

	memcpy(SgcMasterRandbuf,pRxData,8);
    rc = Sgc1120aCalculateAuthRData(buf);
    
    if(rc == 0)
    {
	    TxMsg[0] = E_SGC_F1;
	    TxMsg[1] = 0x01;
		TxMsg[2] = 0x07;    	
		for(i=0;i<PubAddrSize;i++)
		    TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
		*pTxInfoAddr    = LOBYTE((RxInfoAddr));
		*(pTxInfoAddr+1)= HIBYTE((RxInfoAddr));
		memcpy(pTxData,(buf + 2),16);  
		*LengthOut=AsduHeadLength+16; 	
    }
    else
    {
         Sgcwronginf(0x0008);
    }
    *AppCommand=APP_SENDDATA;
}
void CSecAppSev::ProDealF2()
{
    int rc,i,keyno;
    INT8U buf[106];
    INT8U ranbuf[8];

    memcpy(ranbuf,pRxData,8);
    keyno = *(pRxData+8);
    if(keyno >= 0x80)
    {
        keyno = keyno - 0x80;
    }
    rc = Sgc1120aGetPKeyAuthData(keyno,ranbuf,buf);
    if(rc == 0)
    {
	    TxMsg[0] = E_SGC_F2;
	    TxMsg[1] = 0x01;
		TxMsg[2] = 0x07;    	
		for(i=0;i<PubAddrSize;i++)
		    TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
		*pTxInfoAddr    = LOBYTE((RxInfoAddr));
		*(pTxInfoAddr+1)= HIBYTE((RxInfoAddr));
		memcpy(pTxData,(buf + 2),104);  
		*LengthOut=AsduHeadLength+104; 	
    }
    else
    {
        Sgcwronginf(0x1000);
        
        logSysMsgNoTime("0xF2,rc = %rc ",rc,0,0,0);
    }
    *AppCommand=APP_SENDDATA;
}
void CSecAppSev::ProDealF3()
{
    int rc,i;
    INT8U buf[70];
    
    rc = SGCOldPkeyUpdate(pRxData,buf);
    if(rc == 0)
    {
	    TxMsg[0] = E_SGC_F3;
	    TxMsg[1] = 0x01;
		TxMsg[2] = 0x07;    	
		for(i=0;i<PubAddrSize;i++)
		    TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
		*pTxInfoAddr    = LOBYTE((RxInfoAddr));
		*(pTxInfoAddr+1)= HIBYTE((RxInfoAddr));
		memcpy(pTxData,(buf + 2),0x68);  
		*LengthOut=AsduHeadLength+0x68; 	
    }
    else if (rc == -1)
    {
    	  Sgcwronginf(0x0800);
    }
    else if (rc == -2)
    {
         Sgcwronginf(0x4000);
    }
    else if (rc == -3)
    {
        Sgcwronginf(0x1000);
    }
    *AppCommand=APP_SENDDATA;
}
void CSecAppSev::ProDealF4()
{
    int rc,i;
    //INT8U buf[68];
    
    rc = SGCOldSymkeyUpdate(pRxData);
    
    TxMsg[0] = E_SGC_F4;
    TxMsg[1] = 0x01;
    TxMsg[2] = 0x07;        
    for(i=0;i<PubAddrSize;i++)
        TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
    *pTxInfoAddr    = LOBYTE((RxInfoAddr));
    *(pTxInfoAddr+1)= HIBYTE((RxInfoAddr));
    
    if(rc == 0)
    {
		memset(pTxData,0,2);  
		*LengthOut=AsduHeadLength+2; 	
    }
    else if (rc == -1)
    {
        //Sgcwronginf(0x0800);
        pTxData[0] = HIBYTE((0x0800));
        pTxData[1] = LOBYTE((0x0800));
        *LengthOut=AsduHeadLength + 2;   
    }
    else if (rc == -2)
    {
        //Sgcwronginf(0x2000);
        pTxData[0] = HIBYTE((0x2000));
        pTxData[1] = LOBYTE((0x2000));
        *LengthOut=AsduHeadLength + 2;   
    }
    *AppCommand=APP_SENDDATA;
}
void CSecAppSev::ProDealF5()
{
    int rc,loc,keyno,i;
    INT8U buf[122];
    //INT8U cdata[8];
    //INT8U sdata[64];
    
    loc = 0;
    keyno = (int)*pRxData;
    loc++;
    if(keyno >= 0x80)
    {
        keyno = keyno - 0x80;
    }
    //memcpy(cdata,(pRxData + loc),8);
    //loc += 8;
    //memcpy(sdata,(pRxData + loc),64);
    //rc = SGCSKeyConsult(buf,cdata, 8,sdata,64,keyno);
	rc = Sgc1120aGetKeyConsultData(keyno,(pRxData+loc),buf);
    if(rc == 0)
    {
           AuthEndflag = wAppID;
	    TxMsg[0] = E_SGC_F5;
	    TxMsg[1] = 0x01;
		TxMsg[2] = 0x07;    	
		for(i=0;i<PubAddrSize;i++)
		    TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
		*pTxInfoAddr    = LOBYTE((RxInfoAddr));
		*(pTxInfoAddr+1)= HIBYTE((RxInfoAddr));
		memcpy(pTxData,(buf + 2),120);  
		*LengthOut=AsduHeadLength+120; 	
    }
    else
    {
    	  Sgcwronginf(0x0004);
    }
    *AppCommand=APP_SENDDATA;
}
void CSecAppSev::ProDealF6()
{
    int rc,i;
    INT8U buf[10];
    
    rc = Sgc1120aGetRandomData(buf);
    
    if(rc == 0)
    {
	    TxMsg[0] = E_SGC_F6;
	    TxMsg[1] = 0x01;
		  TxMsg[2] = 0x07;    	
  		for(i=0;i<PubAddrSize;i++)
  		    TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
  		*pTxInfoAddr    = LOBYTE((RxInfoAddr));
  		*(pTxInfoAddr+1)= HIBYTE((RxInfoAddr));
  		memcpy(pTxData,(buf + 2),8);  
  		*LengthOut=AsduHeadLength+8; 	
    }
    else
    {
    	Sgcwronginf(0x8000);
    }
    *AppCommand=APP_SENDDATA;
    
}
void CSecAppSev::Sgcwronginf(INT16U enerrno)
{

    int  i;
    TxMsg[0] = E_SGC_FA;
    TxMsg[1] = 0x01;
	TxMsg[2] = 0x07;
	for(i=0;i<PubAddrSize;i++)
	    TxMsg[PubAddrLocation+i]=RxPubAddr>>(8*i);
	*pTxInfoAddr    = LOBYTE((RxInfoAddr));
	*(pTxInfoAddr+1)= HIBYTE((RxInfoAddr));
	pTxData[0] = HIBYTE((enerrno));
	pTxData[1] = LOBYTE((enerrno));
	*LengthOut=AsduHeadLength + 2;    	
}

/*------------------------------------------------------------------/
函数名称：  ProcReadParaGD
函数功能：  处理召唤参数报文，。
输入说明：    
输出说明：  无。
备注：      
/------------------------------------------------------------------*/
void CSecAppSev::ProcReadParaGD(void)
{
    INT8U *pInfoAddr;
    INT16U i, pos;
    //INT16U num;
    INT16U infoaddr;
    
    
    pInfoAddr = &RxMsg[InfoAddrLocation];
    RMTParaNum = RxVsq&VSQ_NUM; 
    
    pos = 0; 
    RMTReturnVsq = RxVsq & VSQ_SQ;
    if(RMTReturnVsq == 0)
    {
        for(i=0; i<RMTParaNum; i++)
        {
            RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
            pos += 6; //2字节信息体地址+4字节数据
                          
        } 
                    
    }
    else
    {
        infoaddr = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
        pos += 2;
        for(i=0; i<RMTParaNum; i++)
        {
            RMTParaInfo[i] = infoaddr + i;         
        }
    }
    
    SetSendData2Flag(DATA2_RMT_READPARA_GD);
    
}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeRMTReadPara_gd()
函数功能：  组织远程参数读取--广东
输入说明：  
输出说明：  rc = TRUE-有后续 FALSE-无后续
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcEncodeRMTReadPara_gd(void)
{
    INT16U i,jj; 
    INT8U len;
    INT16U FramPos;
    
            
    TxMsg[0] = GD_MUTIPARA_READ;
    TxMsg[1] = 0;   //VSQ，后面填个数
    
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    FramPos = PubAddrLocation+PubAddrSize;  
      
    if(RMTReturnVsq == 0)
    {
        for(i=0; i<RMTParaNum ; i++)
        {
            len = 0;
            
            GetTerminalPara(&TxMsg[FramPos+2], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {
                TxMsg[FramPos++] = LOBYTE(RMTParaInfo[i]);
                TxMsg[FramPos++] = HIBYTE(RMTParaInfo[i]);  //信息体地址
                            
                FramPos += len;
            }
                    
            if(FramPos >= 220)
                break;
        }
    }
    else
    {
        TxMsg[FramPos++] = LOBYTE(RMTParaInfo[0]);
        TxMsg[FramPos++] = HIBYTE(RMTParaInfo[0]);  //信息体地址
        
        for(i=0; i<RMTParaNum ; i++)
        {
            len = 0;
            
            GetTerminalPara(&TxMsg[FramPos], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {       
                FramPos += len;
            }
                    
            if(FramPos >= 220)
                break;
        }
        
    }
    
    TxMsg[1] = RMTParaNum|RMTReturnVsq;
    RMTParaInit();
    EnCode101DLMsg(FramPos, APP_SENDDATA);
    
    
    return FALSE;
    
}

/*------------------------------------------------------------------/
函数名称：  ProcWritePara_GD()
函数功能：  处理写参数
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcWritePara_GD(void)
{
    INT8U qos;
    INT8U *pInfoAddr;
    INT16U i,pos,info;
    INT16U curparainfo;
    float tempval;
    INT32U temp32;  
    INT16U writeflag;
    
    //把数据暂存到wrongdata中
    WrongDataLength = LengthIn;
    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
    if(ReadRemoteParaSetEnableState() == FALSE)
    {
        //否定回答
        RMTReturnCot = ACTCON|0x40;
        SetSendData2Flag(DATA2_RMT_WRITEPARA_GD);
        logSysMsgNoTime("远方整定投入软压板为分，禁止修改参数！！",0,0,0,0);
        return;
    }
    
    writeflag = 0;
    if ((RxCot&COT_REASON)==ACT)    //激活
    {
        pInfoAddr = &RxMsg[InfoAddrLocation];
                
        pos = 0;    
        RMTParaNum = RxVsq&VSQ_NUM;    
        if((RxVsq & VSQ_SQ) == 0)
        {
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+6];
                if(qos & 0x80)  //预置
                {
                    RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
                    pos += 2;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    ProgLogWrite2("预置参数info=0x%x, value=%d,%d",RMTParaInfo[i],RMTParaValue[i],RMTParaValue[i]*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    //执行
                    curparainfo = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                    pos += 2; 
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //判断预置和激活相同，则设置
                    if((curparainfo == RMTParaInfo[i]) && (tempval == RMTParaValue[i]))
                    {
                        SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]); 
                        //ProgLogWrite2("参数激活info=0x%x, val=%d,%d",curparainfo,tempval,tempval*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1); 
                        writeflag = 1;
                    }
                    else
                    {
                        ProgLogWrite2("%d参数激活与预置不符。cur=0x%x, old=%x",i,curparainfo,RMTParaInfo[i],0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    }
                }
    
            } 
        }
        else
        {
            info = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //信息体地址
            pos += 2;
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+4];
                if(qos & 0x80)  //预置
                {
                    RMTParaInfo[i] = info+i;    //信息体地址
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    //ProgLogWrite2("预置参数info=0x%x, value=%d,%d",RMTParaInfo[i],RMTParaValue[i],RMTParaValue[i]*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    //执行
                    curparainfo = info+i;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //判断预置和激活相同，则设置
                    if((curparainfo == RMTParaInfo[i]) && (tempval == RMTParaValue[i]))
                    {
                        SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]); 
                        ProgLogWrite2("参数激活info=0x%x, val=%d,%d",curparainfo,tempval,tempval*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1); 
                        writeflag = 1;
                    }
                    else
                    {
                        ProgLogWrite2("%d参数激活与预置不符。cur=0x%x, old=%x",i,curparainfo,RMTParaInfo[i],0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    }
                }
    
            }
        }
            
        //确认回答
        RMTReturnCot = ACTCON;
        SetSendData2Flag(DATA2_RMT_WRITEPARA_GD);
      
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
        RMTReturnCot = DEACTCON;
        SetSendData2Flag(DATA2_RMT_WRITEPARA_GD);
    }

}

/*------------------------------------------------------------------/
函数名称：  ProcEncodeRMTSetPara_GD()
函数功能：  组织读定值区号确认帧
输入说明：  
输出说明：  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeRMTSetPara_GD(void)
{

    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=RMTReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);
    
}
