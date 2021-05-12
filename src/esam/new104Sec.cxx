/*********************************************************************************************/
/*  ���ƣ�                  IEC60870-5-104 ��վ�˳���                                        */
/*  ��Ȩ��                  ��̨����������Ϣ��ҵ�ɷ����޹�˾                                 */
/*  ���ߣ�                  ����                                                           */
/*  �汾��                  v1.0                                                             */
/*  ���ڣ�                  2005/6/6                                                         */
/*                                                                                           */
/*                                                                                           */
/*  ��Ҫ���������ܣ�                                                                         */
/*                 new104sec(INT16U AppID)                      ������ں���                   */
/*                 New104Sec(INT16U AppID,struct PortInfo_t **ppport,                          */
/*                           struct TaskInfo_t **pptask,BOOL *pInitOK)                       */
/*                                                            �����ʼ������                 */
/*                 OnRxData()                                 �������ݴ���                   */
/*                 OnTimer()                                  ��ʱ������                     */
/*                 Schedule()                                 ����֡�༭����                 */
/*                 OnCommState()                              ����ͨ��״̬�仯����           */
/*                 OnMessage()                                �������ݿⷢ�͵���Ϣ           */
/*                 SetDevInfo()                               �����豸��Ϣ                   */
/*                 InitPara()                                 ���������ʼ��                 */
/*                                                                                           */
/*  �޸���ʷ��                                                                               */
/*           1.�޸����ڣ�                                                                    */
/*           2.�޸��ߣ�                                                                      */
/*           3.�޸����ݣ�                                                                    */
/*********************************************************************************************/
#include "new104sec.h"
//�����ļ�ͬ��  CL 20180504
#include "..\newhis\XSDataProc.h"
//�����ļ�ͬ��  CL 20180504



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
        logSysMsgNoTime("new104sec ���񴴽�ʧ��",0,0,0,0);
        #else
        logSysMsgNoTime("Init Fail,new104sec Delete",0,0,0,0);
        #endif
        myTaskSuspendItself();
    }

    if(!MisiPortOpen(AppID,RX_AVAIL,TX_AVAIL,EX_STATE))
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("����򿪴���new104sec ����ɾ��",0,0,0,0);
        #else
        logSysMsgNoTime("Net Open Fail,new104sec Delete",0,0,0,0);
        #endif
        delete p104Sec;
        myTaskSuspendItself();
    }
    
    /*wjr 2010.5.30  ��ͨѶͨ��֮��6����û�����ݺ�Ͽ��������������֮����Сʱ֮��û�����Ͼ���Ϊ����û�лظ�����������*/
    p104Sec->LinkFlag = (INT16U *)nvramMalloc(sizeof(INT16U));
    
    if(((*(p104Sec->LinkFlag)) != 0x5555) && ((*(p104Sec->LinkFlag)) != 0xcccc))
    {
        (*(p104Sec->LinkFlag)) = 0xcccc;    
    }    
    //logSysMsgWithTime("��־��ַ%d ֵ%d",(INT16U)p104Sec->LinkFlag,*(p104Sec->LinkFlag),0,0);
    //tm_evafter(SYSCLOCKTICKS * 1800, EV_SYSRET, &Resettimer);
    /*wjr 2010.5.30 */
    if (!MisiGetLinkStatus(AppID))//�����·�Ƿ���Ч���ȴ����ӽ����ɹ�
    {
        for(;;)
        {
            
            myEventReceive(EX_STATE|EV_SYSRET,MY_EVENTS_WAIT_ANY|MY_EVENTS_KEEP_UNWANTED,WAIT_FOREVER,&dwEvent);
            {
                if(dwEvent&EX_STATE)
                    if (MisiGetLinkStatus(AppID))
                    {    
                        (*(p104Sec->LinkFlag)) = 0xcccc;        /*wjr 2010.5.30 */
                        logSysMsgWithTime("�˿�%d,104��վTCP���ӽ���,�շ������0",AppID,0,0,0);  // ll
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
            logSysMsgNoTime("iec104Sec �޹�Լ��壡������",0,0,0,0);
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
        
        //CON_ENCRYPT:CON_1161ENCRPTY = 1:0(�޼���)  0:0(11�氲ȫ����) 1:1(16�氲ȫ��������)  0:1(�Ƿ�)    
        switch(Sec104Pad.control & (CON_ENCRYPT|CON_1161ENCRPTY|CON_1120ENCRPTY))
        {
        case 0:
            pDLink->IsEncrypt = 1;
            myEnctyptInit(DevList[0].Addr, Sec104Pad.EncryptTimeout);
            logSysMsgNoTime("11�氲ȫ��������(SGC1126)-104",0,0,0,0);
            break;
        case CON_ENCRYPT|CON_1161ENCRPTY:
            //1161����оƬ
            pDLink->N104Encrptystyle = 2;
            logSysMsgNoTime("15�氲ȫ��������(SGC1161)-104",0,0,0,0);
            myTaskDelay(2);    
            rc = EncrptyChiptest(1);
            if(rc != 1)
            {
                logSysMsgNoTime("���ܷ�����оƬ���Ͳ�ƥ������оƬ������",0,0,0,0);
            }
            break;
	 case CON_ENCRYPT|CON_1120ENCRPTY:
		//1120����оƬ
	     pDLink->N104Encrptystyle = 3;
           logSysMsgNoTime("����ũ����ȫ��������(SGC1120a)-104",0,0,0,0);
            myTaskDelay(2);    
	     rc = EncrptyChiptest(1);
           if(rc != 2)
           {
               logSysMsgNoTime("���ܷ�����оƬ���Ͳ�ƥ������оƬ������",0,0,0,0);
           }
	     break;
        default:
            //�޼���
            logSysMsgNoTime("�ް�ȫ��������(͸������)-104",0,0,0,0);
            break;               
        }
    #endif
    
    //֧��"������Լ��չ"-2015��
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
        logSysMsgNoTime("104֧��GY2015��չ",0,0,0,0);
    }
    
    if(Sec104Pad.control & CON_NOJUDGERSNO)
    {
        pDLink->NoJudgeFramNo = 1;
        logSysMsgNoTime("104��Լ����֡���",0,0,0,0);
    }
    
    if(Sec104Pad.control & CON_CLEARRSNO)
    {
        pDLink->RsvStartClearRSno = 1;
        logSysMsgNoTime("104��Լ�յ�����������֡���",0,0,0,0);
    }
    
    bSendAllDBI = FALSE;
    if(Sec104Pad.control & CON_ALLDBI_104)
        bSendAllDBI = TRUE;
    
    
    //���ļ����������ʼ��
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
    
    if (DevType == 2)//�����߼��豸
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

        //������豸��Ŀ
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

        //ȡ�����豸ID
        temp = (INT16U *)DBData;
        SL_ReadDevID(DevID, DevCount, temp);
        for(i=0;i<DevCount;i++)
            DevList[i].DevID=temp[i];

        //ȡ�豸��Ϣ
        for(i=0;i<DevCount;i++)
        {
            DevType = CheckDevType(DevList[i].DevID);
            if(DevType == 1)//ʵ���豸
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
            else if(DevType == 0)//I���߼��豸
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
                
                DevList[i].DevData.DBINum = 0;      //�°�����˫��ң�Ŵ���������˸��ģ�ʹ��ԭ�в�������ɷ��͵�Ŵ��ҡ� ll 2017-7-19
                if(AppLBConf.DBINum > 0 )
                {
                    logSysMsgNoTime("�°�����˫��ң�Ų�������Ҫ��ͬ���밴��Ҫ���޸�˫��ң����ز���",0,0,0,0);   
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
    
    LinkConnect=FALSE;            /*����ͨ�ϵı�־  wjr  2010.5.21*/
    LinkBreakCounter = 0;         /*��·ͨ�������û���յ����ݵļ����� wjr  2010.5.21*/
    
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

    CotLocation=2;//COT��ASDU�е�λ��
    PubAddrLocation=CotLocation+CotSize;//PUBADDR��ASDU�е�λ��
    InfoAddrLocation=PubAddrLocation+PubAddrSize;//INFOADDR��ASDU�е�λ��
    AsduHeadLength=InfoAddrLocation+InfoAddrSize;//ASDUͷ�ĳ���

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

    if((Sec104Pad.LBIinfoaddr<LBI)||(Sec104Pad.LBIinfoaddr>HBI))        //ң����Ϣ���ַ������ 2008.11.5  wjr
        LBIinfoaddr=LBI;
    else
        LBIinfoaddr=Sec104Pad.LBIinfoaddr;
        
    if((Sec104Pad.LDBIinfoaddr<LDBI)||(Sec104Pad.LDBIinfoaddr>HDBI))
        LDBIinfoaddr=LDBI;
    else
        LDBIinfoaddr=Sec104Pad.LDBIinfoaddr;
    
    //LBIinfoaddr=LBI;    //�°�����˫��ң�Ŵ���������˸��ģ�����ʹ�ù�Լ�������� ll 2017-7-19
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
    }*/ //ll 2014-3-14 �� �����ж�Ӱ�����������ʮ�ֲ����㣩��
    /*if (pDLink->Tick[3].Value<=pDLink->Tick[1].Value)
    {
        pDLink->Tick[1].Value=T1;    //pre 1s
        pDLink->Tick[3].Value=T3;    //pre 1s
    }*/
}

void New104Sec::SetDefaultPad(void)
{
    int i;

    Sec104Pad.ControlPermit = 1;       //ң������ 1-����0-������ ȱʡΪ1
    Sec104Pad.SetTimePermit = 1;  //�������� 1-����0-������ ȱʡΪ1

    Sec104Pad.SendCountWithReset = 0;//���͵��ʱ����λ��1-����λ 0-������λ ȱʡΪ0
    Sec104Pad.UseStandClock = 1;//ʹ�ñ�׼ʱ�Ӹ�ʽ 1-��׼ 0-�Ǳ�׼ ȱʡΪ1
    Sec104Pad.AllDataInternal = ALLDATATIMER;//��ʱ����ȫ���ݼ�����֣� ȱʡ30
    Sec104Pad.ScanData2=SCANDATA2TIMER;//��������ɨ����
    Sec104Pad.CountInternal = COUNTERTIMER;//��ʱ���͵�ȼ�����֣� ȱʡ60
    Sec104Pad.TickValue[0] = T0;//TickValue[0]����
    Sec104Pad.TickValue[1] = T1;//TickValue[1]ȷ���޻ش�ʱ�������룩 ȱʡ15
    Sec104Pad.TickValue[2] = T2;//TickValue[2]����ȷ��֡ʱ�������룩 ȱʡ5
    Sec104Pad.TickValue[3] = T3;//TickValue[3]���Ͳ���֡ʱ�������룩 ȱʡ30
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
    
    LBIinfoaddr=LBI;                     //ң����Ϣ���ַ������ 2008.11.5  
    LDBIinfoaddr=LDBI;
}

void New104Sec::ReadAIMaxVal(INT16U DevIndex)  //��ң����ֵ������������ֵ
{
    INT16U i;
    INT16U DevID;
    INT16U val;
    INT16U deathval;
    INT8U ppty;
    
    DevID = DevList[DevIndex].DevID;

    for (i=0;i<DevList[DevIndex].DevData.AINum;i++)
    {
        if(DevList[DevIndex].Flag == 1)     //=1ʵ���豸
            val = SRSendMax_ReadAI(DevID,i);
        else
            val = SLMax_ReadAI(DevID,i);
        
        DevList[DevIndex].DevData.AItype[i] = SL_ReadAI_Type(DevID,i); //��ȡ��ǰ���ݵ����ͣ��з��Ż����޷��ţ�
        DevList[DevIndex].DevData.AIporperty[i] = SL_ReadAI_Porperty(DevID,i); //��ȡ��ǰ���ݵ�����(��������ѹ�����ʣ�Ƶ��)
        DevList[DevIndex].DevData.AIMaxValTrue[i] = val;
               
        
        //�����������ֵ
        if(DevList[DevIndex].Flag == 1)
            deathval = SRDead_ReadAI(DevID,i);
        else
            deathval = SLDead_ReadAI(DevID,i);
        
        if(deathval > 1)    //��0����1����Ϊ��Ч
        {
            DevList[DevIndex].DevData.AIMaxVal[i] = deathval;   //����ǧ�ֱȵ�λ
            
        }
        else
        {
            ppty = DevList[DevIndex].DevData.AIporperty[i];
            deathval = GetRmtDeathvalue(ppty);
            if(deathval > 0)//Զ�̲�������ֵ
            {
                DevList[DevIndex].DevData.AIMaxVal[i] = ((INT32U)val*deathval)/1000;
                
            }
            else //��Լ�������ֵ
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
    
    /*Ϊ��������������������   2010.5.21*/
    if(LinkConnect == FALSE)
    {
        LinkConnect = TRUE;    
    }    
    else
    {
        LinkBreakCounter = 0;         /*��·ͨ�������û���յ����ݵļ���������   2010.5.21*/	
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
            for(i=0;i<2;i++)//InfoAddrSize���Ϊ3��Ҳֻȡǰ2���ֽڡ�
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
            if(FirstCallAllData==0xff)  //��һ�����в������ 0xff-�Ѿ����й���0-δ���й�
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
        //�㶫Զ�̲���
        case GD_MUTIPARA_READ:
            //if(IsRMTforGuangdong())
            ProcReadParaGD();
            break;
        case GD_MUTIPARA_WRITE:
            ProcWritePara_GD();
            break;
        //����Զ����ά
        /*case P_RS_NA_1_GX:              //������վ������
            ProcReadParaGX();
            break;
        case P_ME_NA_1_GX:               //������վԤ�ò���
            ProcSetParaGX();
            break;
        case P_AC_NA_1_GX:               //������վ�������
            ProcActivateParaGX();
            break;
        */
        case C_SR_NA:   //�л���ֵ����
            ProcSetSectionNo();
            break; 
        case C_RR_NA:
            ProcReadSectionNo();    //����ֵ����
            break; 
        case C_RS_NA:       //��������
            ProcReadPara();
            break;  
        case C_WS_NA:       //д������
            ProcWritePara();
            break;
               
        case F_FR_NA_N:
            ProcFileTran(); //�ļ�����
            break; 
        case F_SR_NA_N:
            ProcFT_ProgramUpdate();
            break;   
        case F_FS_NA_N:
            ProcFileSyn();//�ļ�ͬ�� CL 20180306
            SetFileSynInfoTaskIDSubstation(MySelf.AppTID);
            if(XSFileSynInfo.TaskIDPri101[0]!=0)//�����һ������ģ���������
            {
                myEventSend(GetFileSynInfoTaskID101(0),XSFILESYN);//��101��վ��������Ϣ ��ʱ�ȷ�����һ��101���񣬺�����ͨ��ά�����������ȷ�ϵġ�
            }
            else
            {
                logSysMsgWithTime("��֧��2018��׼������ģ�飡",0,0,0,0);
            }
            if(XSFileSynInfo.TaskIDPri101[1]!=0)//�����еڶ�������ģ���������
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
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;

    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);

    TxMsg[InfoAddrLocation] = LOBYTE(RxInfoAddr);
    TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;

    switch(pRxData[0])
    {
        case QRPRESET:

            TxMsg[AsduHeadLength] = QRPRESET;
            EnCodeDLMsg(AsduHeadLength+1);
            //if (RxPubAddr==BroadCastAddr)        2009.4.2 ��������ʱҪ��λ��������ַ�ж�
            {
                myTaskDelay(100);
                SystemReset(WARMRESET);
            }
            break;
        case QRPSOEIND:
            TxMsg[AsduHeadLength] = QRPSOEIND;
            EnCodeDLMsg(AsduHeadLength+1);
           // if (RxPubAddr==BroadCastAddr)   2009.4.2 ��������ʱҪ��λ��������ַ�ж�
            {
                myTaskDelay(600);
                SystemReset(COLDRESET);
            }
            break;
        case QRPCOLD:
            TxMsg[AsduHeadLength] = QRPCOLD;
            EnCodeDLMsg(AsduHeadLength+1);
          //  if (RxPubAddr==BroadCastAddr)   2009.4.2 ��������ʱҪ��λ��������ַ�ж�
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

    TxMsg[0]=C_TS_TA;              //2009.3.17�޸ģ�ԭ���ص�����ΪC_RP_NA
    TxMsg[1]=1;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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

void New104Sec::SecCallData2(void)  //�ٻ���������
{
    int i=Soe;      
    BOOL Stop=FALSE;
    
    Data2Seq=(enum PData2Seq)i;
    while(Stop==FALSE)
    {
        switch (Data2Seq)
        {
            case Soe:   //soeɨ�費��ʹ�ö�ʱɨ��
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

void New104Sec::ProcControl(void)  //����ң��
{
    INT8U sco,dco,OnOff;
    INT16U SwitchNo;
    Revert=0;
    YKTypeID = RxTypeID;                        
    
    SwitchNo = RxInfoAddr-LBO+1;
    
    //LogYkInfoRec(DevList[ActDevIndex].DevID, RxTypeID, *pRxData, RxInfoAddr, RxCot);  //��¼��վ���͵�����ң����Ϣ
    
    if((RxCot&COT_TEST)==COT_TEST)//�������������λΪ1����ʵ�ʲ���ң�أ�
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
    	if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
            TxMsg[CotLocation+1]=0;
    	for(int jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    	TxMsg[InfoAddrLocation]   = LOBYTE(RxInfoAddr);
    	TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
    	if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
            TxMsg[InfoAddrLocation+2]=0;
    	TxMsg[AsduHeadLength] = *pRxData;
    	EnCodeDLMsg(AsduHeadLength+1);
    	return;
    }
                        
    if(GetActDevIndexByAddr(RxPubAddr))
    {    
        if((SwitchNo-1)*2==DevList[ActDevIndex].DevData.BONum)                     //����ά��
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
            if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
                TxMsg[CotLocation+1]=0;
            for(int jj=0;jj<PubAddrSize;jj++)
                TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
        	TxMsg[InfoAddrLocation]   = LOBYTE(RxInfoAddr);
        	TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
        	if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxMsg[InfoAddrLocation+2]=0;
            TxMsg[AsduHeadLength] = *pRxData;
        	EnCodeDLMsg(AsduHeadLength+1);
        	if(((*pRxData)&DCO_SE)==0)  //ִ��
            {
                if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                    startCellMaint();
                TxMsg[2]=ACTTERM;
            	EnCodeDLMsg(AsduHeadLength+1);
            }
            
            return;                                                     //����ά������
        }
        else if((SwitchNo-1)*2==DevList[ActDevIndex].DevData.BONum+2)  //���鼶���豸  
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
            if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
                TxMsg[CotLocation+1]=0;
            for(int jj=0;jj<PubAddrSize;jj++)
                TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
        	TxMsg[InfoAddrLocation]   = LOBYTE(RxInfoAddr);
        	TxMsg[InfoAddrLocation+1] = HIBYTE(RxInfoAddr);
        	if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxMsg[InfoAddrLocation+2]=0;
            TxMsg[AsduHeadLength] = *pRxData;
        	EnCodeDLMsg(AsduHeadLength+1);
        	if(((*pRxData)&DCO_SE)==0)  //ִ��
            {
                if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                {
                	ResetFaultInfoForCall_FaultCheck();
                	logSysMsgNoTime("���鼶���豸",0,0,0,0);
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
           
        case C_SC_NA:    //����ң������
            sco = *pRxData;
            SwitchNo = RxInfoAddr-LBO+1;
            if ((sco & SCO_SCS) == 0)       //��
                OnOff = 2;
            else if ((sco & SCO_SCS) == 1)  //��
                OnOff = 1;
            if(Sec104Pad.ControlPermit == 0)//��������Ϊ������ң��
            {
                EnCodeNACK(UNKNOWNTYPEID); //��ȷ�� 
                /*if (!pDLink->GetFreeTxUnit(PRIORITY_1,&TxMsg))
                    return;
                memcpy(TxMsg,RxMsg,AsduHeadLength+1);

                TxMsg[2]=UNKNOWNTYPEID;

                TxMsg[2]|=COT_PONO;

                EnCodeDLMsg(AsduHeadLength+1);*/
                return;
            }
            
            BODevIndex = ActDevIndex;
            
            if ((RxCot&COT_REASON)==ACT)//6������
            {
                if ((sco & SCO_SE) == SCO_SE)   //1��select
                {
                    
                    if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                        SetYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    
                }
                else//0��ִ��
                {
                    if(pDLink->YkStatusForTest2 == 0)                    
                    {
                        EnCodeNACK(ACTCON); //�񶨼���ȷ��                         
                        return;         //ll Ϊ���ݲ�����ʱ�޸� ִ��֮ǰ����Ԥ�� 2012-3-24
                    }
                    pDLink->YkStatusForTest2 = 0;
                    
                    if(YKSetAlready == TRUE)
                        YKSetAlready = FALSE;
                    if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                        ExecuteYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    
                }
            }
            else//
            {
                if((RxCot&COT_REASON)==DEACT)       //����
                    CancelYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                else
                {
                    EnCodeNACK(UNKNOWNCOT); //�񶨼���ȷ��
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
            if ((dco&DCO_DCS)==1)        //��
                OnOff = 2;
            else if ((dco&DCO_DCS)==2)  //��
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

            if ((RxCot&COT_REASON)==ACT)//6������
            {
                if ((dco&DCO_SE) == DCO_SE)   //1��select
                {
                    
                    
                    if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                        SetYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    if((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2))//�������ϸ��飬���һ��ң�صĺ�
                    {
                        YKSetAlready = TRUE;
                    }
                }
                else//0��ִ��
                {
                    if(pDLink->YkStatusForTest2 == 0)
                    {
                        EnCodeNACK(ACTCON); //�񶨼���ȷ��     
                        return;   //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24
                    }
                    pDLink->YkStatusForTest2 = 0;
                    
                    if((YKSetAlready == FALSE)&&((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2)))//�������ϸ��飬���һ��ң�صĺ�
                    {
                        Revert=1;
                        SetYK(MySelf.AppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    }
                    else
                    {
                        if(YKSetAlready == TRUE)
                            YKSetAlready = FALSE;
                        if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                            ExecuteYK(MySelf.AppID,DevList[ActDevIndex].DevID,SwitchNo,OnOff);
                    }
                }
            }
            else//����
            {
                if(((RxCot&COT_REASON)==DEACT)&&((RxCot&COT_TEST)==0))       //����
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

void New104Sec::ProcClock(void)  //�������
{
    struct Iec101ClockTime_t Time;
    
    BOOL flag = FALSE;     //flagΪ1��������ʱ�䣬Ϊ0�����쳣ʱ�䣬���ж����ڼ�
    
    if((RxCot == REQ) || ((pRxData[6]+pRxData[5]+pRxData[4])==0))  //�ж�������Ϊ0
    {
        //��ʱ��
        EnCodeClock(PTL104_CLOCK_READ);
        return;
    }
    
    //������дʱ��
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
    //if(flag && Sec104Pad.SetTimePermit)                    //���flagΪ�棬��Sec104Pad.SetTimePermit�϶�Ϊ�棬�����������ĳ�ֻ�ж�flag�Ϳ���
    if(flag)
    {
        
        
        if (Sec104Pad.UseStandClock == 1)
            SetSysTime(&Time,IEC101CLOCKTIME);
        else
            SetSysTime(&Time,IEC101EXTCLOCKTIME);  
        
        EnCodeClock(PTL104_CLOCK_WRITE);    //�����޸ĺ��ʱ��
              
    }
    else
    {
        EnCodeClock(PTL104_CLOCK_WRITE|PTL104_CLOCK_NACK);                               //�ڴ�վ�������ж��ӻ��߶���ʱ���д���ʱ���ʹ���ԭ��Ϊ0x47,�񶨻ش�
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
    
    if((FirstCallAllData == 0))    //��1�����в��ܱ���ϵĴ���ʱ��
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
    INT16U BeginNo,EndNo,Num,GroupNo = 17;   //����coverity����

    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))  //ң����
    {
        if (CheckAndModifyGroup())
        {
            //GroupTrn.InfoAddr �Ǵ�LBIinfoaddr��ʼ�ġ�����BeginNo�����Ǵ�0��ʼ�ģ������ٻ����Ǵ�0
            //EndNo��һ֡���127����BINum�� 0x80��Ŀ���ǿ���128������һ��

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
                    
                if(DevList[GroupTrn.DevIndex].DevData.DBINum>0)     //wjr 2009.8.27    ˫��ң���ڵ�һ��
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
                    if ((GroupTrn.COT!=INTROGEN)&&(GroupTrn.COT!=BACK))//����Ƿ����ٻ�����˵����վ�ٻ������������Ѿ����ꡣ
                    {
                        GroupTrn.GroupNo=17;    //������֡
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
    if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=14)) //ң����
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
        //���ӹ���Ҫ�����soe
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
�������ƣ�  CheckAndModifyGroup()
�������ܣ�  ����޸�GroupTrn�ṹ�壬�������й���
����˵����  
���˵����  TRUE ��������Ҫ����   FALSE �����ݷ���
��ע��      
/------------------------------------------------------------------*/
BOOL New104Sec::CheckAndModifyGroup(void)
{
    INT16U Num;
    INT16U wptr;
    
    switch (GroupTrn.TypeID)
    {
        case C_IC_NA:       //��������
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
                    if (GroupTrn.InfoAddr < (GroupTrn.GroupNo-1)*0x7f+LBIinfoaddr)  //���ڷ����ٻ������������������
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
                    if(GetDBINum()) //����Ƿ�����˫��ң��
                    {
                        if((GroupTrn.COT==INTROGEN) && (GroupTrn.HaveSendDBI==FALSE))
                        {
                            //�����������˫��ң��û�ͣ�����֯˫��ң�š������ٻ�����˫��ң��
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
                            //���100��
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
                        
                        logSysMsgNoTime("104soe�ٻ����� start=%d, wptr=%d,max=%d",GroupTrn.SoeStartPtr,wptr,Num,0);
                    }
                    else
                    {
                        //����
                        logSysMsgNoTime("104soe�ٻ����� wptr=%d,max=%d",wptr,Num,0,0);
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

BOOL New104Sec::GetNextDev(void) //�õ���һ����ѯ���豸
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

//�����豸ʹ��״̬
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
        ProcFileInit();   //���ļ�����Ĳ���
        //logSysMsgWithTime("104��վTCP���ӽ���,�շ������0",0,0,0,0);  // ll
        ProgLogWrite2("�˿�%d,104��վTCP���ӽ��� �շ������0",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 1);
        
    }
    else
    {
        //ll ���ͨѶ�жϣ�����ң�ء�Ϊ���ݲ����޸� 2014-3-14
        if(pDLink->YkStatusForTest2)
        {
            pDLink->YkStatusForTest2 = 0;    
            BspYkRelease();
            logSysMsgNoTime("104ͨѶ�жϳ���ң��",0,0,0,0);
        }
        
        ScheduleFlag = 0;   //�Ͽ����Ӻ������б�־
        
        ProgLogWrite2("�˿�%d,104��վTCP�Ͽ�����",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 0);

        AllDataEnd=FALSE;
        while(!rc)
            rc=ReceiveMSG(MySelf.QID,(void*)pMsg,MAXMSGLENGTH,ALWAYSRETURN);
        if (pDLink->CommConnect!=InUse)
            pDLink->StopDT(FALSE);
    }
}
//ÿ������һ��
void New104Sec::OnTimer(void)
{
    /*Ϊ��������������������  wjr 2010.5.21*/
    /*if(LinkConnect == TRUE)
    {
        LinkBreakCounter++;
        if(LinkBreakCounter > MAXBREAKNUM)
        {
            (*LinkFlag) = 0x5555;
            logSysMsgWithTime("net104 not rec",0,0,0,0);
            //logSysMsgWithTime("��־��ַ%d ֵ%d",(INT16U)LinkFlag,*LinkFlag,0,0);
            myTaskDelay(600);
            SystemReset(WARMRESET);    
        }        
    }*/
    

    //if ((!pDLink->CommConnect)||(!AllDataEnd))
    //if ((!pDLink->CommConnect)||(!FirstCallAllData))    //��1�����в������ ll 2016-9-16
    //    return;
        
    if(WaitCallAllDelay)    //�ȴ����н�����ʱ
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
        //if ((ScheduleFlag==FALSE)&&(AllDataEnd==TRUE))    // ǰ���Ѿ��з����ˣ�����Ͳ���Ҫ�ж������� ll 2016-9-16
        if((ScheduleFlag & 0x0fff) == FALSE)  //�����24λ��ֵ��ɨ��
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
        //ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //Ӧ��������������˱�־ wjr 2009.3.31
        EnCodeReadData();
    }
}

void New104Sec::EnCodeAllDataConf(void) //ȫ����ȷ��
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
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
            if(RxMsg[AsduHeadLength] == INTRO16)    //��16 �������� �����޸�
            {
                logSysMsgWithTime("�յ���16����",0,0,0,0);
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
            else if(RxMsg[AsduHeadLength]>INTROGEN)      //wjr���ӷ����ٻ� 2009.3.31 
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
                
                //���ٻ� ������֡
                TxMsg[0]=GroupTrn.TypeID;
                TxMsg[1]=1;
                TxMsg[2]=ACTTERM;
                if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
                    TxMsg[CotLocation+1]=0;
                for(int jj=0;jj<PubAddrSize;jj++)
                    TxMsg[PubAddrLocation+jj]=GroupTrn.PubAddr>>(8*jj);
                TxMsg[InfoAddrLocation] = 0;
                TxMsg[InfoAddrLocation+1] = 0;
                if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                    TxMsg[InfoAddrLocation+2]=0;
                TxMsg[AsduHeadLength] = RxMsg[AsduHeadLength];
                
                EnCodeDLMsg(AsduHeadLength+1);
            }                                                                //���ӽ���2009.3.31
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

void New104Sec::EnCodeCounterConf(void) //���ȷ��
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
    
    //ת����101��վ����˲ʱ����
    if((pRxData[0]&QCC_FRZ) == FREEZENORESET)  //FRZ=1 ���᲻����λ����
    {
        SendFreezeEvent2Pri101();
    }
    //ת����101��վ����˲ʱ����
    
    CallDD=TRUE;

    TxMsg[0]=C_CI_NA;
    TxMsg[1]=1;
    TxMsg[2]=ACTCON;
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=GroupTrn.PubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
    
    
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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

BOOL New104Sec::EnCodeCtrlRet(void)  //ң�ط�У
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
            case 1://1-ң��Ԥ�óɹ�
                Cmd = SELECT;
                pDLink->YkStatusForTest2 = 1;   //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24
                break;
            case 2://2-ң��Ԥ��ʧ��
                Cmd = SELECT;
                break;
            case 3://3-ң��ִ�гɹ�
                Cmd = OPERATE;
                break;
            case 4://4-ң��ִ��ʧ��
                Cmd = OPERATE;
                break;
            case 5://5-ң�س����ɹ�
                Cmd = SELECT;
                break;
            case 6://6-ң�س���ʧ��
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
        if((RetNo == 1)&&(Revert==1)&&(OnOff == 1)&&(SwitchNo==DevList[BODevIndex].DevData.BONum/2))//�������ϸ���ִ��
        {
            Revert=0;
            ExecuteYK(MySelf.AppID,DeviceID,SwitchNo,OnOff);
            return  FALSE;
        }

        if((RetNo == 5)||(RetNo == 6))//����
        {
            TxMsg[2]=DEACTCON;
        }
        else
        {
            TxMsg[2]=ACTCON;
        }

        if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
            TxMsg[CotLocation+1]=0;

        if(YKTypeID==C_SC_NA)
            sco |= SCO_SE;//0x80 Ԥ�ñ�־λ
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

    if ((RetNo & 1) == 0)//2,4,6 ʧ��
    {
        if(GetYKRYBState() == TRUE)
            TxMsg[2]|=0x40;
        else
        {
            TxMsg[2] = COT_YKRYBERR;    //����ң����ѹ����� ll 
            TxMsg[2] |= COT_PONO;
        }
    }
    else//1,3,5�ɹ�
        TxMsg[2]&=(~0x40);

    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation]   = LOBYTE((SwitchNo%0x80)+LBO-1);
    TxMsg[InfoAddrLocation+1] = HIBYTE((SwitchNo%0x80)+LBO-1);;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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

void New104Sec::EnCodeReadData(void) //������
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
        if(GroupTrn.InfoAddr>=(LDBIinfoaddr+DevList[GroupTrn.DevIndex].DevData.DBINum/2))   /*��������Ϊ����ң����   wjr2009.8.25*/
        {
            if((GroupTrn.InfoAddr<LBIinfoaddr) || (GroupTrn.InfoAddr>=LBIinfoaddr+DevList[GroupTrn.DevIndex].DevData.BINum))
            {
                ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //Ӧ��������������˱�־ wjr 2009.3.31
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
        {                                                                                 /*��������Ϊ˫��ң��*/
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
        ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //Ӧ��������������˱�־ wjr 2009.3.31
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
        ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //Ӧ��������������˱�־ wjr 2009.3.31
        return;
       
    }
    ScheduleFlag&=(~SCHEDULE_DATA1_READDATA);           //Ӧ��������������˱�־ wjr 2009.3.31
    
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
        Len = EnCodeAllLastSoe(BeginNo);  //�ٻ����100��SOE  Len ��ʾ�����Ƿ������ݷ���
    }
    
    return(Len);
}
/*------------------------------------------------------------------/
�������ƣ�  EnCodeLastSoe()
�������ܣ�  ���������100��SOE������Զ��ά����
����˵����  BeginNo soe��ʼָ��
            
���˵����  FALSE ��ʾû�����ˣ����־�� 
            TRUE ��ʾ��������Ҫ����
��ע��      BeginNo ��ʼ��ʱȷ�����ˣ���������BeginNo���͵��������ɣ�ÿ֡���18֡
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

        TxMsg[0]=M_SP_TB;   //��ʱ��ĵ�����Ϣ
        TxMsg[1]=0;
        TxMsg[2]=INTRO16;  //ͨ����16�ٻ��ģ����Դ���ԭ��λ��16
        if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
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
            Length=ASDULEN-AsduHeadLength-12;//250-9-12=229ΪӦ�ò㷢����Ϣ��󳤶�
            
            j=0;
            while(j<Num)
            {
                /*if(p->Status & BIDBI_STATUSE)    //SOE��״̬λBIDBI_STATUSE��0x10����ʾ��˫��ң��
                {
                    DBIDBData[DBISOEnum]=(struct BIEWithTimeData_t)(*p);
                    DBISOEnum++;
                	j++;
                	p++;
                }
                else*/  //��ʱ��֧��˫��ң�ţ����Ҫ֧���ٿ��Ǵ�
                {
                    
                    if(p->Status&0x80)
                        Status=1;
                    else
                        Status=0;
    
                    if (FramePos < 0)
                    {
                        TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LBIinfoaddr));
                        TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LBIinfoaddr));
                        if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                            TxMsg[InfoAddrLocation+2]=0;
                    }
                    else
                    {
                        TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//��Ϣ���ַ
                        TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                        if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                            TxData[FramePos+2] = 0;
                    }
                    FramePos+=InfoAddrSize;
    
                    if((p->Status&BIACTIVEFLAG)==0)
                        TxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                    else
                        TxData[FramePos]=Status;//����ң��״̬�ֽ�
                        
                    if(p->Status&SUBSTITUTEDFLAG)
                        TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                    FramePos++;
    
                    AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
    
                    TxData[FramePos++] = LOBYTE(time.MSecond);
                    TxData[FramePos++] = HIBYTE(time.MSecond);
                    TxData[FramePos++] = time.Minute;
                    TxData[FramePos++] = time.Hour;
                    TxData[FramePos++] = time.Day;
                    TxData[FramePos++] = time.Month;
                    TxData[FramePos++] = time.Year;
    
                    SendNum++;//���͸���
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
                //�е���ң����Ҫ���ͣ���������������Ƿ��к�������
                TxMsg[1] = SendNum;
                Len = FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
                
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
                        return TRUE;    //��ʾ����δ��������
                }
                else
                {
                	if(j>0) //j���Ѿ�������ĸ���
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
    short ByteNum,Length,SendNum,FramePos,YxNum;    //SendNum���Ѿ��������ң�Ÿ�����YxNum�Ƿ��͵�ң�Ÿ���
    INT16U yxSendno;

    TxMsg[0]=Sec104Pad.SBIType;
    TxMsg[1]=VSQ_SQ;
    //if(/*(DevList[GroupTrn.DevIndex].DevData.DBINum>0)&&*/(GroupTrn.GroupNo==8))
    if(GroupTrn.HaveSendDBI == TRUE)
    {
    	TxMsg[0]=M_DP_NA;
    }
    //�㶫Ҫ���ȫ˫��ң�ŷ��ͣ����⴦��
    if(bSendAllDBI)
        TxMsg[0] = M_DP_NA_ALLDBI;
    
    TxMsg[2]=GroupTrn.COT;  //wjr  2009.8.26 ����ԭ��ֱ�ӵ���������Ĵ���ԭ�� 
    
    if(CotSize==2)              //����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
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
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;

    TxData = TxMsg+AsduHeadLength;
    *pNum = 0;

    devid=DevList[GroupTrn.DevIndex].DevID;
    FramePos=0;
    SendNum=0;
    YxNum=0;
    Len = 0;
    Length=ASDULEN-AsduHeadLength-10;//250-7-8-2=233ΪӦ�ò㷢����Ϣ��󳤶�
    if(TxMsg[0] == M_PS_NA)//20-�����bit
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
    else if(TxMsg[0]==M_SP_NA)//1-����byte
    {
        if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        //����ң�ŷ��Ͳ��ԣ��ڱ����ѯ�У��������˫��ң����ֹͣ���ͣ���ֹͣ���ͱ���������һ������ң��Ҫ����
        for(i=0;i<ByteNum;i++)
        {
            SendNum++;
            if((DBData[i] & BIDBI_STATUSE) == 0)    //��˫��ң�ţ��򰴵���ң�ŷ���
            {
                if((DBData[i]&BIACTIVEFLAG)==0)
                    TxData[FramePos]=((DBData[i]&0x80)>>7)|P101_IV;   //���ݿ�D7Ϊң��״̬,����Чλ
                else
                    TxData[FramePos]=((DBData[i]&0x80)>>7);           //���ݿ�D7Ϊң��״̬
                    
                if(DBData[i]&SUBSTITUTEDFLAG)
                    TxData[FramePos]|=P101_SB;   
                
                    
                FramePos++;
                YxNum++;
                
                if(YxNum == 1)  //��⵽��1������ң�ţ���������Ϣ���ַ����Ϊ�п��ܵ�1����˫��ң�ţ�
                {
                    TxMsg[InfoAddrLocation] = LOBYTE(GroupTrn.InfoAddr+i);
                    TxMsg[InfoAddrLocation+1] = HIBYTE(GroupTrn.InfoAddr+i);
                }
            }
            else
            {
                if(YxNum)   //��⵽˫��ң�ź���ֹͣ�������ͣ���Ϊ��������
                    break;
            }
            
            if((FramePos>=Length)||(SendNum>=127))
                break;
        }
    }
    else if(TxMsg[0]==M_DP_NA)//3-˫��byte      wjr
    {
        //������˫��ң�ţ�����û�䵽���ͱ�������
        //������˫��ң�ţ�һ֡���Ͳ�������
        //������˫��ң�ţ������ٻ����
        //����cos�����
        
        TxMsg[1]= 0;    //��˳��Ԫ��

    	EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1; //���ң�Ÿ������󣬳���DBData����������ô��Ҫѭ����
    	if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        FramePos = InfoAddrLocation;       //��������Ϣ���ַλ�� ll 2017-7-19
        for(i=0; i<(ByteNum); i++)
        {
            SendNum++;  //�����¼�Ѿ���ȡ����ң�Ÿ���
            
            if(DBData[i] & BIDBI_STATUSE)
            {
                yxSendno = GroupTrn.InfoAddr+i-LBIinfoaddr;   //���㱾�ε�ң�ŷ�����ţ���0��ʼ���㣨GroupTrn.InfoAddrĿǰ�Ǽ�¼�Ĵ�LBIinfoaddr��ʼ����ţ����ڿ��ƴ���ң�ŵĵ�ǰλ�ã� ll 21-03-28
                TxMsg[FramePos++] = LOBYTE((yxSendno+LDBIinfoaddr));//��Ϣ���ַ
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
        
        FramePos -= AsduHeadLength;     //Ϊ���ݵ���ң�ų��򣬵���FramePos�Ĵ�СΪȥ��AsduHeadLength��С  ll 2017-7-19
    }
    else if(TxMsg[0] == M_DP_NA_ALLDBI)//�㶫Ҫ���ȫ˫��ң�ŷ��ͣ����⴦��
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
                TxData[FramePos] = Status|P101_IV;   //���ݿ�D7Ϊң��״̬,����Чλ
            else
                TxData[FramePos] = Status;           //���ݿ�D7Ϊң��״̬
                
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
            Len = SendNum;    //������ֵ�²���������
            pDLink->NotifyToAppSchedule();  //����û���κ����ݣ����Ƚ�����һ�η��� ll 2017-7-19
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
    TxMsg[2]=GroupTrn.COT;           //wjr  2009.8.26 ����ԭ��ֱ�ӵ���������Ĵ���ԭ�� 
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation]   = LOBYTE(GroupTrn.InfoAddr);
    TxMsg[InfoAddrLocation+1] = HIBYTE(GroupTrn.InfoAddr);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;

    //ȡ������
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
                case M_ME_NA:     //9����ֵ����һ��ֵ
                case M_ME_ND:
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag=pAIValue->Flag;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Value=Value;
                    //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxValTrue[No];
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue=Value;
                    break;
                case M_ME_NB:     //11����ֵ�����ֵ
                case M_ME_NC:    //13����ֵ���̸�����
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag=pAIValue->Flag;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].TempValue=Value;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Value=Value;
                    break;
            }
            No++;
            pAIValue++;
        }
    }//end of (while (No<=EndNo))

    //��֡����
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
    FramePos=0;
    No=BeginNo;
    *pNum=0;
    SendNum = 0;
    while ((FramePos<Length)&&(!Stop))
    {
        switch(TxMsg[0])
        {
            case M_ME_NA:     //9����ֵ����һ��ֵ
            case M_ME_NB:     //11����ֵ�����ֵ
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
            case M_ME_NC:    //13����ֵ���̸�����
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
        if (SendNum>=127)//ÿ֡������127�����ݵ�Ԫ
            Stop=TRUE;
    }

    if(SendNum > 0)
    {
        *pNum = SendNum;
        TxMsg[1] |= SendNum;

        Len=FramePos+AsduHeadLength;//Ӧ�ò㱨�ĵ��ܳ���
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
    TxMsg[2]=GroupTrn.COT;                       //����ԭ��ֱ�ӵ���������Ĵ���ԭ�� wjr 2009.8.26
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(RTUSTATUS);
    TxMsg[InfoAddrLocation+1] = HIBYTE(RTUSTATUS);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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

BOOL New104Sec::EnCodeNVA(void)  //�༭�仯ң������;
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
        if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
            TxMsg[CotLocation+1]=0;
        for(jj=0;jj<PubAddrSize;jj++)
            TxMsg[PubAddrLocation+jj]=DevList[NvaActDevNo].Addr>>(8*jj);

        for (No=0;No<=DevList[NvaActDevNo].DevData.AINum-1;No++)
            DevList[NvaActDevNo].DevData.AIData[No].WillSend=FALSE;
        
        No=0;
        devid=DevList[NvaActDevNo].DevID;
        while (No<=(int)(DevList[NvaActDevNo].DevData.AINum-1))//ȡң��ֵ�����÷��ͱ�־
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

                if (NvaVal >=DevList[NvaActDevNo].DevData.AIMaxVal[No])//�Ƚϱ仯ֵ������ֵ��С
                {
                    DevList[NvaActDevNo].DevData.AIData[No].WillSend=TRUE;//���÷��ͱ�־
                }
                DevList[NvaActDevNo].DevData.AIData[No].Flag=pAIValue->Flag;
                DevList[NvaActDevNo].DevData.AIData[No].TempValue=value;
                No++;
                pAIValue++;
            }
        }
        if (AINum<=0)
            return FALSE;
        No=DevList[NvaActDevNo].DevData.NvaNo; //���ϴη����ң����ſ�ʼ����
        k=0;
        Length=ASDULEN-AsduHeadLength-5-sizeof(INT16U);//���Է������ݵı��ĳ���
        while ((FramePos<Length)&&(!Stop))//�鷢������֡
        {
            if (DevList[NvaActDevNo].DevData.AIData[No].WillSend)
            {
                if (FramePos < 0)
                {
                    TxMsg[InfoAddrLocation]   = LOBYTE((No+LAI));
                    TxMsg[InfoAddrLocation+1] = HIBYTE((No+LAI));
                    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                        TxMsg[InfoAddrLocation+2]=0;
                }
                else
                {
                    pTxData[FramePos]   = LOBYTE((No+LAI));
                    pTxData[FramePos+1] = HIBYTE((No+LAI));
                    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                        pTxData[FramePos+2] = 0;
                }
                FramePos+=InfoAddrSize;

                switch (TxMsg[0])
                {
                    case M_ME_NA:     //9����ֵ����һ��ֵ
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
                    case M_ME_NB:     //11����ֵ����Ȼ�
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
                    case M_ME_NC:    //13����ֵ���̸�����
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
                    case M_ME_ND:        //21����Ʒ�������Ĳ���ֵ����һ��
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

    TxMsg[0]=M_DP_TB;   //��ʱ��ĵ�����Ϣ
    TxMsg[1]=0;
    TxMsg[2]=SPONT;  //
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[DBIDevIndex].Addr>>(8*jj);

    TxData = TxMsg+AsduHeadLength;
    Len = 0;
    FramePos=0-InfoAddrSize;
    p=(struct BIEWithTimeData_t *)DBIDBData;
    Length=ASDULEN-AsduHeadLength-12;//250-9-12=229ΪӦ�ò㷢����Ϣ��󳤶�
    
    
    Num=0;    
    SendNum=0;
    
    for(j=0; j<DBISOEnum; j++)
    {
      //״̬ת�������ݿ��е�ң��D7Ϊ״̬����Լ��D0Ϊ״̬
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
            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxMsg[InfoAddrLocation+2]=0;
        }
        else
        {
            TxData[FramePos]   = LOBYTE((p->No + LDBIinfoaddr));//��Ϣ���ַ
            TxData[FramePos+1] = HIBYTE((p->No + LDBIinfoaddr));
            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxData[FramePos+2] = 0;
        }
        FramePos+=InfoAddrSize;

        if((p->Status&BIACTIVEFLAG)==0)
            TxData[FramePos]=Status1|Status0|P101_IV;//����ң��״̬�ֽ�
        else
            TxData[FramePos]=Status1|Status0;//����ң��״̬�ֽ�
            
        if(p->Status&SUBSTITUTEDFLAG)
            TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
        
        FramePos++;
        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

        TxData[FramePos++] = LOBYTE(time.MSecond);
        TxData[FramePos++] = HIBYTE(time.MSecond);
        TxData[FramePos++] = time.Minute;
        TxData[FramePos++] = time.Hour;
        TxData[FramePos++] = time.Day;
        TxData[FramePos++] = time.Month;
        TxData[FramePos++] = time.Year;

                
        SendNum++;//���͸���
        p++;
        if(FramePos>=Length)
            break;
     }
     
     if(SendNum>0)
     {
         TxMsg[1]=SendNum;
         Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
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
�������ƣ�  EnCodeSOE()
�������ܣ�  �༭SOE
����˵����  
���˵����  FALSE ��ʾû�����ˣ����־�� TRUE ��ʾ��������Ҫ����
            SOE���¼�����(OnUData)��ͨ���ñ�־λ����EnCodeSOE
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

        TxMsg[0]=M_SP_TB;   //��ʱ��ĵ�����Ϣ
        TxMsg[1]=0;
        TxMsg[2]=SPONT;  //
        if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
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
            Length=ASDULEN-AsduHeadLength-12;//250-9-12=229ΪӦ�ò㷢����Ϣ��󳤶�
            
            j=0;
            while(j<Num)
            {
                
                if(DevList[i].DevData.DBINum>0)     //����ʹ��DevList[i].DevData.DBINum�ж��Ƿ���˫��ң�ţ���δ��벻��ʹ�� ll 2017-7-19
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
                            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                TxMsg[InfoAddrLocation+2]=0;
                        }
                        else
                        {
                            TxData[FramePos]   = LOBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));//��Ϣ���ַ
                            TxData[FramePos+1] = HIBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));
                            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                TxData[FramePos+2] = 0;
                        }
                        FramePos+=InfoAddrSize;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            TxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                        else
                            TxData[FramePos]=Status;//����ң��״̬�ֽ�
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                        
                        FramePos++;
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
        
                        TxData[FramePos++] = LOBYTE(time.MSecond);
                        TxData[FramePos++] = HIBYTE(time.MSecond);
                        TxData[FramePos++] = time.Minute;
                        TxData[FramePos++] = time.Hour;
                        TxData[FramePos++] = time.Day;
                        TxData[FramePos++] = time.Month;
                        TxData[FramePos++] = time.Year;
        
                        SendNum++;//���͸���
                        p++;
                        j++;
                        if(FramePos>=Length)
                            break;
                    }
                }
                else
                {
                    if(p->Status & BIDBI_STATUSE)    //SOE��״̬λBIDBI_STATUSE��0x10����ʾ��˫��ң��
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
                            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                TxMsg[InfoAddrLocation+2]=0;
                        }
                        else
                        {
                            TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//��Ϣ���ַ
                            TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                TxData[FramePos+2] = 0;
                        }
                        FramePos+=InfoAddrSize;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            TxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                        else
                            TxData[FramePos]=Status;//����ң��״̬�ֽ�
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                        
                        FramePos++;        
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
        
                        TxData[FramePos++] = LOBYTE(time.MSecond);
                        TxData[FramePos++] = HIBYTE(time.MSecond);
                        TxData[FramePos++] = time.Minute;
                        TxData[FramePos++] = time.Hour;
                        TxData[FramePos++] = time.Day;
                        TxData[FramePos++] = time.Month;
                        TxData[FramePos++] = time.Year;
        
                        SendNum++;//���͸���
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
                //�е���ң����Ҫ���ͣ���������������Ƿ��к�������
                TxMsg[1] = SendNum;
                Len = FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
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
                        return TRUE;    //��ʾ����δ��������
                }
                else
                {
                	if(j>0) //j���Ѿ�������ĸ���
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
�������ƣ�  EnCodeSOE_ALLDBI()
�������ܣ�  �༭SOE
����˵����  
���˵����  FALSE ��ʾû�����ˣ����־�� TRUE ��ʾ��������Ҫ����
            SOE���¼�����(OnUData)��ͨ���ñ�־λ����EnCodeSOE
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

        TxMsg[0]=M_DP_TB;   //��ʱ��ĵ�����Ϣ
        TxMsg[1]=0;
        TxMsg[2]=SPONT;  //
        if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
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
            Length=ASDULEN-AsduHeadLength-12;//250-9-12=229ΪӦ�ò㷢����Ϣ��󳤶�
            
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
                    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                        TxMsg[InfoAddrLocation+2]=0;
                }
                else
                {
                    TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//��Ϣ���ַ
                    TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                        TxData[FramePos+2] = 0;
                }
                FramePos+=InfoAddrSize;

                if((p->Status&BIACTIVEFLAG)==0)
                    TxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                else
                    TxData[FramePos]=Status;//����ң��״̬�ֽ�
                    
                if(p->Status&SUBSTITUTEDFLAG)
                    TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                
                FramePos++;        
                AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

                TxData[FramePos++] = LOBYTE(time.MSecond);
                TxData[FramePos++] = HIBYTE(time.MSecond);
                TxData[FramePos++] = time.Minute;
                TxData[FramePos++] = time.Hour;
                TxData[FramePos++] = time.Day;
                TxData[FramePos++] = time.Month;
                TxData[FramePos++] = time.Year;

                SendNum++;//���͸���
                p++;
                j++;
                if(FramePos>=Length)
                    break;
                
            }
            
            if(SendNum>0)
            {
                //�е���ң����Ҫ���ͣ���������������Ƿ��к�������
                TxMsg[1] = SendNum;
                Len = FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
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
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=(DevList[DBICOSDevIndex].Addr)>>(8*jj);

    TxData = TxMsg+AsduHeadLength;
    Len = 0;

    SendNum=0;
    FramePos=0-InfoAddrSize;
    p=(struct BIEWithoutTimeData_t *)DBICOSDBData;
    Length=ASDULEN-AsduHeadLength-3;//250-9-3=238ΪӦ�ò㷢����Ϣ��󳤶�

    
    for(j=0; j<DBICOSnum; j++)
    {
        
        Status0 = 0;
        Status1 = ((p->Status&0x60)>>5);
        
        if(FramePos < 0)
        {
            TxMsg[InfoAddrLocation]   = LOBYTE((p->No + LDBIinfoaddr));
            TxMsg[InfoAddrLocation+1] = HIBYTE((p->No + LDBIinfoaddr));
            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxMsg[InfoAddrLocation+2]=0;
        }
        else
        {
            TxData[FramePos]   = LOBYTE((p->No + LDBIinfoaddr));//��Ϣ���ַ
            TxData[FramePos+1] = HIBYTE((p->No + LDBIinfoaddr));
            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxData[FramePos+2] = 0;
        }
        
        FramePos+=InfoAddrSize;
        if((p->Status&BIACTIVEFLAG)==0)
            TxData[FramePos]=Status1|Status0|P101_IV;//����ң��״̬�ֽ�
        else
            TxData[FramePos]=Status1|Status0;//����ң��״̬�ֽ�
            
        if(p->Status&SUBSTITUTEDFLAG)
            TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
        
        FramePos++;
        SendNum++;//���͸���
        p++;
        if(FramePos>=Length)
            break;

        
    }
    
    if(SendNum>0)
    {
        
        TxMsg[1]=SendNum;
        Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
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
�������ƣ�  EnCodeBIENT()
�������ܣ�  ��֯cos����
����˵����  
���˵����  TRUE  ����������   FALSE �޺�������
��ע��      
/------------------------------------------------------------------*/
BOOL New104Sec::EnCodeBIENT(void) //�༭COS
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
        if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
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
            Length=ASDULEN-AsduHeadLength-3;//250-9-3=238ΪӦ�ò㷢����Ϣ��󳤶�
            
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
                            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                TxMsg[InfoAddrLocation+2]=0;
                        }
                        else
                        {
                            TxData[FramePos]   = LOBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));//��Ϣ���ַ
                            TxData[FramePos+1] = HIBYTE((p->No-DevList[i].DevData.DBINum + LBIinfoaddr));
                            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                TxData[FramePos+2] = 0;
                        }
                        FramePos+=InfoAddrSize;
                        if((p->Status&BIACTIVEFLAG)==0)
                            TxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                        else
                            TxData[FramePos]=Status;//����ң��״̬�ֽ�
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                        
                        FramePos++;
                        SendNum++;//���͸���
                        p++;
                        j++;
                        if(FramePos>=Length)
                            break;
                    }
                }
                else
                {
                    if(p->Status & BIDBI_STATUSE)   //��⵽��˫��ң�ţ��ŵ���������������
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
                             if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                 TxMsg[InfoAddrLocation+2]=0;
                         }
                         else
                         {
                             TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//��Ϣ���ַ
                             TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                             if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                                 TxData[FramePos+2] = 0;
                         }
                         FramePos+=InfoAddrSize;
                         if((p->Status&BIACTIVEFLAG)==0)
                             TxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                         else
                             TxData[FramePos]=Status;//����ң��״̬�ֽ�
                             
                         if(p->Status&SUBSTITUTEDFLAG)
                             TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                         
                        
                         FramePos++;
                         SendNum++;//���͸���
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
                 Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
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
                //û�е���ң�ţ�����Ƿ���˫��ң�ŷ���
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
�������ƣ�  EnCodeBIENT_ALLDBI()
�������ܣ�  ��֯cos���ͣ��㶫��ȫ˫��ң�ţ�
����˵����  
���˵����  TRUE  ����������   FALSE �޺�������
��ע��      
/------------------------------------------------------------------*/
BOOL New104Sec::EnCodeBIENT_ALLDBI(void) //�༭COS
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
        if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
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
            Length=ASDULEN-AsduHeadLength-3;//250-9-3=238ΪӦ�ò㷢����Ϣ��󳤶�
            
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
                    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                        TxMsg[InfoAddrLocation+2]=0;
                }
                else
                {
                    TxData[FramePos]   = LOBYTE((p->No + LBIinfoaddr));//��Ϣ���ַ
                    TxData[FramePos+1] = HIBYTE((p->No + LBIinfoaddr));
                    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                        TxData[FramePos+2] = 0;
                }
                FramePos+=InfoAddrSize;
                if((p->Status&BIACTIVEFLAG)==0)
                    TxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                else
                    TxData[FramePos]=Status;//����ң��״̬�ֽ�
                     
                if(p->Status&SUBSTITUTEDFLAG)
                    TxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                 
                
                FramePos++;
                SendNum++;//���͸���
                p++;
                j++;
                if(FramePos>=Length)
                    break;
            }
             
            if(SendNum>0)
            {
                 TxMsg[1]=SendNum;
                 Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
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
        TxMsg[2]=REQCOGCN+GroupTrn.GroupNo;  //�ٻ�һ��
    else
        TxMsg[2]=GroupTrn.COT;

    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(GroupTrn.InfoAddr);
    TxMsg[InfoAddrLocation+1] = HIBYTE(GroupTrn.InfoAddr);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
                if (GroupTrn.Description&FREEZENORESET)//���᲻��λ
                {
                    q->Value=value;    //������ֵΪ�ۼ�ֵ
                    q->Flag=0;
                    DevList[GroupTrn.DevIndex].DevData.LastCounterData[GetBeginNo+j]=value;
                }
                else if(GroupTrn.Description&FREEZERESET)
                {
                    q->Flag=0;        //������ֵΪ����ֵ
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
    Length=ASDULEN-AsduHeadLength-10-sizeof(INT16U);//250-7-10-2=231ΪӦ�ò㷢����Ϣ��󳤶�
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

        //д��Ϣ���ַ
        if (FramePos < 0)
        {
            TxMsg[InfoAddrLocation]   = LOBYTE((No+LBCR));
            TxMsg[InfoAddrLocation+1] = HIBYTE((No+LBCR));
            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxMsg[InfoAddrLocation+2]=0;
        }
        else
        {
            TxData[FramePos]   = LOBYTE(No+LBCR);//��Ϣ���ַ
            TxData[FramePos+1] = HIBYTE(No+LBCR);//��Ϣ���ַ
            if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
                TxData[FramePos+2] = 0;
        }
        FramePos+=InfoAddrSize;

        //д���ֵ
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
            TxData[FramePos++] = i;//˳���
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
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);

    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;

    
    TxMsg[InfoAddrLocation+3]=0;

    if(EnCodeDLMsg(AsduHeadLength+1))    
        return(TRUE);
    else 
        return(FALSE);
}

/*------------------------------------------------------------------/
�������ƣ�  ProcFT_EncodeReadDir()
�������ܣ�  ��Ŀ¼�Ĵ�����֯����֡
����˵����  
���˵���� 
��ע��      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_EncodeReadDir(void)
{
    INT8U len;
    BOOL rc;
    INT8U FramePos;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    len = 0;
    
    //����Ŀ¼ID��Ŀ¼���ƣ���֯Ŀ¼���ļ��Ĵ���.���ļ�������ʼ��д
    
    rc = FT_ReadDirectory(&FtInfo, &len);
       
    //len = 0 �ط񶨻ش�rc=true��ʾ���޺���
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = REQ; //COT
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //��Ϣ���ַ
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
            
    TxMsg[FramePos++] = 2;    //�������ݰ�����
    TxMsg[FramePos++] = FR_RD_DIR_CON;
    
    if(len)    
        TxMsg[FramePos++] = 0;    //��ȡĿ¼�ɹ�
    else
        TxMsg[FramePos++] = 1;    //��ȡĿ¼ʧ��
        
    TxMsg[FramePos++] = LLBYTE(FtInfo.dirid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.dirid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.dirid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.dirid);
    
    if(rc)
        TxMsg[FramePos++] = 1;    //�к���
    else
    {
        TxMsg[FramePos++] = 0;    //�޺���
    }
    
    if(len)
    {  
        memcpy(&TxMsg[FramePos], FtInfo.DataBuf, len);
        FramePos += len;
    }
    else
    {
        TxMsg[FramePos++] = 0;    //�ļ�����=0
    }
    
    
    EnCodeDLMsg(FramePos);    
        
    if(rc==0)
        ScheduleFlag &= ~SCHEDULE_FT_DIR;
    
}
/*------------------------------------------------------------------/
�������ƣ�  ProcFT_ReadDir()
�������ܣ�  �����ļ������Ŀ¼���
����˵����  
���˵���� 
��ע��      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_ReadDir(void)
{
    //INT32U DirID;
    INT8U namelen, CallFlag, i;
    INT8U *pRx;
    struct Iec101ClockTime_t StartTime, EndTime;
    
    //DirID = MAKEDWORD(MAKEWORD(pRxData[2],pRxData[3]),MAKEWORD(pRxData[4],pRxData[5]));
    namelen = pRxData[6];
    
    memset(FtInfo.tempname, 0, 40); //��¼Ŀ¼��
    if(namelen>=40)
        namelen = 39;
    memcpy(FtInfo.tempname,&pRxData[7],namelen);    //FtInfo.tempname����ʱ��ţ����ڴ�ӡ��Ϣ
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    
    FtInfo.dirid = FT_GetDirID(&FtInfo);    //����Ŀ¼������Ŀ¼ID DirID����ʹ�� liuwei20170307
    
    pRx = &pRxData[7]+namelen;  //���ٻ���־λ�õ�ָ�븳ֵ��pRx    
    CallFlag = pRx[0];  //�ٻ���־
    
    FtInfo.callflag = CallFlag;  
    
    logSysMsgNoTime("�ٻ�Ŀ¼%s, namelen=%d �ٻ���־=%d, DirID=%d",(INT32U)FtInfo.tempname, namelen, CallFlag, FtInfo.dirid);
      
    if(CallFlag)
    {
        pRx++;  //��������ѯ��ʼʱ���λ��
        StartTime.MSecond = MAKEWORD(pRx[0],pRx[1]);
        StartTime.Minute  = (pRx[2] & 0x3f);
        StartTime.Hour    = (pRx[3] & 0x1f);
        StartTime.Day     = (pRx[4] & 0x1f);
        StartTime.Month   = (pRx[5] & 0xf);
        StartTime.Year    = (pRx[6] & 0x7f);
        pRx += 7;   //��������ѯ����ʱ���λ��
        EndTime.MSecond = MAKEWORD(pRx[0],pRx[1]);
        EndTime.Minute  = (pRx[2] & 0x3f);
        EndTime.Hour    = (pRx[3] & 0x1f);
        EndTime.Day     = (pRx[4] & 0x1f);
        EndTime.Month   = (pRx[5] & 0xf);
        EndTime.Year    = (pRx[6] & 0x7f);
       
        ConvToAbsTime(&StartTime,&FtInfo.startime, IEC101CLOCKTIME);
        ConvToAbsTime(&EndTime,  &FtInfo.endtime, IEC101CLOCKTIME);
        
        logSysMsgNoTime("%d %d:%d:%dĿ¼��ѯ��ʼ",StartTime.Day,StartTime.Hour,StartTime.Minute,StartTime.MSecond/1000);
        logSysMsgNoTime("%d %d:%d:%dĿ¼��ѯ����",EndTime.Day,EndTime.Hour,EndTime.Minute,EndTime.MSecond/1000);
    }
    else
    {
        FtInfo.startime.Minute = 0;
        FtInfo.endtime.Minute = 0;
    }
    
    ScheduleFlag |= SCHEDULE_FT_DIR;
   
}

/*------------------------------------------------------------------/
�������ƣ�  ProcFT_ReadFileAct()
�������ܣ�  ����Ҫ�����ļ�
����˵����  
���˵����  
��ע��      
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
    
    //�����ļ����ƣ���֯�ļ��Ĵ���
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    

    len = FT_ReadFileAct(&FtInfo, FtInfo.tempname);
    
    //��֯��������
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;   
    
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = ACTCON;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //��Ϣ���ַ
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;    //�������ݰ�����
    TxMsg[FramePos++] = FR_RD_FILE_ACTCON; 
    successPos = FramePos;  //��¼�ɹ�ʧ��λ�� 
    FramePos++;
    
    //�ļ���
    TxMsg[FramePos++] = namelen;
    memcpy(&TxMsg[FramePos], FtInfo.tempname, namelen);
    FramePos += namelen;
    //4�ֽ��ļ���ʾ�ļ�ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
    
    if(len)
    {
        TxMsg[successPos] = 0;    //�ɹ�
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
�������ƣ�  ProcFT_EncodeFileData()
�������ܣ�  ����Ҫ������ļ�����
����˵����  
���˵����  
��ע��      
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
    //��Ϣ���ַ
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;    //�������ݰ�����
    TxMsg[FramePos++] = FR_RD_FILE_DATA;
    
    //4�ֽ��ļ���ʾ�ļ�ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
    
    //����FileFlag�ļ���ʾ�����Ҳ�ͬ�ļ� 
    rc = FT_ReadData(&FtInfo, &TxMsg[FramePos], &len);
            
    EnCodeDLMsg(FramePos+len);     //6��ʾ�������ݰ���������ʶ���ļ�ID 
    
    if(rc == 0)
    {
        //û�к���������
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
        FtInfo.SoeDirID = Sec104Pad.SoeDirID;   //SOE  Ŀ¼ID  
        FtInfo.YkDirID  = Sec104Pad.YkDirID;    //ң��   Ĭ��ֵ 2
        FtInfo.ExvDirID = Sec104Pad.ExvDirID;   //��ֵ   Ĭ��ֵ 3
        FtInfo.FixDirID = Sec104Pad.FixDirID;   //����   Ĭ��ֵ 4
        FtInfo.UlogDirID = Sec104Pad.UlogDirID;  //��־   Ĭ��ֵ 7
        FtInfo.LbDirID  = Sec104Pad.LbDirID;    //¼��   Ĭ��ֵ 8 
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
�������ƣ�  ProcFT_WriteFileDataConf()
�������ܣ�  д�ļ������������ȷ��֡��
����˵����    
���˵����  
��ע��      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_WriteFileDataConf(void)
{
    INT8U FramePos;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    //��֯��������
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = REQ;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    FramePos = 6;
    //��Ϣ���ַ
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;    //�������ݰ�����
    TxMsg[FramePos++] = FR_WR_FILE_DATACON;
    
    //4�ֽ��ļ���ʾ�ļ�ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
     
    TxMsg[FramePos++] = LLBYTE(FtInfo.offset); 
    TxMsg[FramePos++] = LHBYTE(FtInfo.offset);
    TxMsg[FramePos++] = HLBYTE(FtInfo.offset);
    TxMsg[FramePos++] = HHBYTE(FtInfo.offset); 
    
    TxMsg[FramePos++] = FtInfo.errinfo;  //���������
    
    EnCodeDLMsg(FramePos);
        
    //logSysMsgNoTime("д�ļ�ȷ��֡%s, err=%d, offset=%d", (INT32U)FtInfo.name, FtInfo.errinfo, FtInfo.offset,0);
    
}
/*------------------------------------------------------------------/
�������ƣ�  ProcFT_WriteFileData()
�������ܣ�  ����Ҫд���ļ�����
����˵����  
���˵����  
��ע��      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_WriteFileData(void)
{
    INT32U offset, fileid;
    INT8U  IsNoFinish;
    INT8U  segmentlen, rc;
    
    fileid = MAKEDWORD(MAKEWORD(pRxData[2],pRxData[3]),MAKEWORD(pRxData[4],pRxData[5]));
    offset = MAKEDWORD(MAKEWORD(pRxData[6],pRxData[7]),MAKEWORD(pRxData[8],pRxData[9]));
    IsNoFinish = pRxData[10]; //0���޺���  1���к���

    FtInfo.errinfo = 0;
    if(fileid != FtInfo.fileid)
    {
        //�ļ�ID��һ��
        FtInfo.errinfo = 4;  
    }
    if(offset != FtInfo.offset)
    {
        //�ļ������쳣
        FtInfo.errinfo = 3;
    }
    
    rc = TRUE;
    if(FtInfo.errinfo == 0)
    {  
        if(IsNoFinish == FALSE)
        {
            ProcFT_WriteFileDataConf();     //��ǰ��ȷ��֡����Ҫ������վ�Ƿ������Ͻ���ͨ�ţ�����Ӧ����дflash��ʱ��
        } 
        //if(INFOADDR2BYTE)
            //segmentlen = (FrameLen+2) - (AsduHeadLength-1) - 12;
        //else
            segmentlen = (FrameLen+2) - AsduHeadLength - 12;   //ASDUlen= (FrameLen+2) AsduHeadLength=9  12=�����ݿ�������ֽ�
        
        if(FtInfo.IsWriteProgramFile)
        {
            if(FtInfo.IsUpdate)
            {
                rc = FT_WriteFileData(&FtInfo, &pRxData[11], segmentlen, pRxData[11+segmentlen], IsNoFinish);
            }
            else
            {
                FtInfo.errinfo = 1; //δ֪����
                rc = TRUE;
                logSysMsgNoTime("��������쳣(������)",0,0,0,0);
            }
        }
        else
        {
            //���طǳ��������ļ�
            rc = FT_WriteFileData(&FtInfo, &pRxData[11], segmentlen, pRxData[11+segmentlen], IsNoFinish);
            
        }

        if(rc == FALSE)
            return;
    }

    //��û�н������������쳣��ʱ��ҲҪ��ȷ��֡
    if((rc == TRUE) && (IsNoFinish))
        ProcFT_WriteFileDataConf(); //�����ļ���ȷ��֡���Ƿ������Ƚϰ�ȫ�������ֳ�����λ����ô��
    
    FT_ParaReset(&FtInfo);  //�ļ���������������
    
}
/*------------------------------------------------------------------/
�������ƣ�  ProcFT_WriteFileAct()
�������ܣ�  д�ļ������¼�ļ�������ʼ����ز�����
����˵����    
���˵����  
��ע��      
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
    
    
    //�����ļ����ƣ���֯�ļ��Ĵ���
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    
    FtInfo.namelen  = namelen;
    FtInfo.fileid   = fileid;
    FtInfo.FileSize = filesize;
    strcpy(FtInfo.name, FtInfo.tempname);
        
    result = FT_WriteFileAct(&FtInfo);
    
    //��֯��������
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
    //��Ϣ���ַ
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 0;
    
    TxMsg[FramePos++] = 2;     //�������ݰ�����
    TxMsg[FramePos++] = FR_WR_FILE_ACTCON;
    TxMsg[FramePos++] = result;
    
    //�ļ���
    TxMsg[FramePos++] = namelen;
    memcpy(&TxMsg[FramePos], FtInfo.name, namelen);
    FramePos += namelen;
    
    //4�ֽ��ļ���ʾ�ļ�ID
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
�������ƣ�  ProcFT_ProgramUpdate()
�������ܣ�  ������������
����˵����    
���˵����  
��ע��      
/------------------------------------------------------------------*/
void New104Sec::ProcFT_ProgramUpdate(void)
{
    if((RxCot&COT_REASON) == ACT)
    {
        if(pRxData[0] & 0x80) //CTYPE
        {   
            //��������   
            FtInfo.IsUpdate = TRUE;
        }
        else
        {
            //��������
            FT_ParaReset(&FtInfo);
            //FtInfo.IsUpdate = FALSE; 
            StartProgramUpdate();
        }
        
    }
    else
    {
        //��������
        FT_ParaReset(&FtInfo);
        ClearProgramUpdate();
    }
    
    
    //����ȷ��֡
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;
    }
    
    if((RxCot&COT_REASON) == DEACT) //ȡ������
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
    
    if(pRxData[0] != 2) //�������ݰ����� 2=�ļ�����
    {   
        return;   
    }
    
    switch(pRxData[1])  //�ļ�������ʾ
    {
        case FR_RD_DIR: //��Ŀ¼
            ProcFT_ReadDir();
            break;
        
        case FR_RD_FILE_ACT:    //���ļ�����
            ProcFT_ReadFileAct();
            
            break;
        
        case FR_RD_FILE_DATACON:   //���ļ�����ȷ��
            
            break;
        
        case FR_WR_FILE_ACT:    //д�ļ�����
            ProcFT_WriteFileAct();
            break;
        
        case FR_WR_FILE_DATA:   //д�ļ�����
            ProcFT_WriteFileData();
            break;
    }
    
    
}

/*------------------------------------------------------------------/
�������ƣ�  ProcFileSyn()
�������ܣ�  �����ļ�ͬ������
����˵����  
���˵���� 
��ע������ģ��2018��׼���ļ�ͬ����cl 20180314     
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
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;
    TxMsg[AsduHeadLength]=0;

    Len=AsduHeadLength+1;
    EnCodeDLMsg(Len);
    //��101��վ��������Ϣ����101���ļ��ٻ�
    
    //��101��վ��������Ϣ����101���ļ��ٻ�
}

void New104Sec::ProcXsFileSynFinish(void)
{
    INT8U Len;
    TxMsg[0]=F_FS_NA_N;              
    TxMsg[1]=1;
    TxMsg[2]=ACTTERM;
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = 0;
    TxMsg[InfoAddrLocation+1] = 0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;
    TxMsg[AsduHeadLength]=0;

    Len=AsduHeadLength+1;
    EnCodeDLMsg(Len); 
    logSysMsgWithTime("�ļ�ͬ����ֹ",0,0,0,0); //debugʹ��CL 20180528    
}






 #ifdef STANDARDFILETRANS104
/*------------------------------------------------------------------/
�������ƣ�  StdProcFT_ReadDir()
�������ܣ�  �����ļ������Ŀ¼���
����˵����  
���˵���� 
��ע��      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_ReadDir(void)
{
    //������Ϣ���ַ����һ���ٻ�¼���ļ��������ļ���¼���ļ�����Ϣ���ַ��0x680A
    
    StdFtInfo.dirid    = RxInfoAddr;
    /*if(StdFtInfo.dirid == FT_DIRID_LB)
    {
        StdFileInfo.SectionNum = 2;          //����¼���Ĺ涨��¼���ļ���Ϊ2�ڣ�.cfg�ļ�����Ϊ��һ�ڣ�.dat�ļ�����Ϊ�ڶ��ڡ�
    }*/
    ScheduleFlag |= SCHEDULE_FT_DIR_STD;   
}
/*------------------------------------------------------------------/
�������ƣ�StdGetFileInfo()
�������ܣ��ļ����������F_SC_NA��
����˵����pData:Ӧ�ò�������
���˵����          
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
        if (StdFileInfo.RxCot == SFREQ)   /*�ٻ�Ŀ¼*/
        {
    
            
        }
        else if (StdFileInfo.RxCot == SFILE)     /*����*/
        {
            switch (scq)
            {
                
                    
                case 1:     /*ѡ���ļ�*/
                    StdFileInfo.FileStep = SelectFile;
                    StdFileInfo.SectionName = 0;
                    break;
                    
                case 2:     /*�����ļ�*/
                    StdFileInfo.FileStep = CallFile;
                    break;
                    
                case 3:     /*ֹͣ�����ļ�*/
                    StdFileInfo.FileStep = FileOver;
                    break;
                    
                    
                case 4:     /*ɾ���ļ�*/
                    StdFileInfo.FileStep = FileOver;
                    break;
                    
                case 5:     /*ѡ���*/
                    StdFileInfo.FileStep = CallSection;
                    StdFileInfo.SectionName = sname;
                    break;
                    
                case 6:     /*�����*/
                    StdFileInfo.FileStep = CallSection;
                    StdFileInfo.SectionName = sname;
                    break;
                    
                case 7:     /*ֹͣ�����*/
                    StdFileInfo.FileStep = FileOver;
                    break;
                    
                default:
                    StdFileInfo.FileStep = FileOver;
                    break;
            }
            
            
            
        }
        else    /*��Ч*/
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
�������ƣ�StdProcFT_ReadFile()
�������ܣ���÷�����Ϣ�������չ�Լ��Ӧ�ò�涨�����ͱ�ʶ��֡
����˵������
���˵����pData����֡�����ݣ�Flag��TRUE���к���֡��������֡�����ݳ���
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_ReadFile()
{
    
    
    switch(StdFileInfo.FileStep)
    {
        case SelectFile:
            //rc = SFileReady();        //�ڴ�Ҫȥ�����ļ���
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
        case REQ:                         //��Ŀ¼
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
�������ƣ�  StdProcFT_EncodeReadDir()
�������ܣ�  ��Ŀ¼�Ĵ�����֯����֡
����˵����  
���˵���� 
��ע��      
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
    filenum = len / 13;    //13��TYPE IDENT 126 ��ÿһ�ļ���ص���Ϣ��
    TxMsg[0] = F_DR_NA;
    TxMsg[1] = 0x80 | filenum;   //VSQ
    TxMsg[2] = REQ; //COT
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //�˴��п��������⣬�ؼ�����ô����Լ���Ȱ��ո���ʾ�����ı�д��2016��12��30��
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;

    FramePos = AsduHeadLength;
    if(len)
    {  
        memcpy(&TxMsg[FramePos], StdFtInfo.DataBuf, len);
        FramePos += len;
    }
    else
    {
        memset(&TxMsg[FramePos],0,5);//TxMsg[FramePos++] = 0;    //�ļ�����=0
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
�������ƣ�  StdProcFT_EncodeFileReady()
�������ܣ�  �ļ�׼������
����˵����  
���˵���� 
��ע��      
/------------------------------------------------------------------*/
void New104Sec::StdProcFT_EncodeFileReady(void)
{
    
    BOOL rc;
    INT8U FramePos;
    //INT8U filenum = 0;
    if (!pDLink->GetFreeTxUnit(PRIORITY_2, &TxMsg))
        return;
    
    
    rc = StdFT_GetFileReady(&StdFtInfo,StdFileInfo.FileName,&StdFileInfo.SectionNum,&StdFileInfo.FileLen);     //����¼���ļ��������ڴ˺��������ɱ�Ҫ���ļ�����.cfg��.dat�ļ�
    //filenum = len / 13;    //13��TYPE IDENT 126 ��ÿһ�ļ���ص���Ϣ��
    StdFileInfo.FileChs = 0;
    TxMsg[0] = F_FR_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //�˴��п��������⣬�ؼ�����ô����Լ���Ȱ��ո���ʾ�����ı�д��2016��12��30��
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
�������ƣ�  StdProcFT_EncodeSectionReady()
�������ܣ�  ��׼������
����˵����  
���˵���� 
��ע��      
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
    //filenum = len / 13;    //13��TYPE IDENT 126 ��ÿһ�ļ���ص���Ϣ��
    TxMsg[0] = F_FR_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //�˴��п��������⣬�ؼ�����ô����Լ���Ȱ��ո���ʾ�����ı�д��2016��12��30��
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;

    FramePos = AsduHeadLength;
    TxMsg[FramePos++] = LOBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = HIBYTE(StdFileInfo.FileName);
    TxMsg[FramePos++] = StdFileInfo.SectionName;
    
    //SectionLength = GetSectionLength();              //����д������¼���ļ���������1�ĳ�����.cfg�ļ��ĳ��ȣ���2�ĳ�����.dat�ļ��ĳ��ȡ�
    
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
�������ƣ�  StdProcFT_EncodeSegment()
�������ܣ�  ��׼������
����˵����  
���˵���� 
��ע��      
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
        StdFT_GetSegment(&StdFtInfo, &len, &flag,StdFileInfo.SectionName,&StdFileInfo.SectChs);     //���ļ��ĶΣ���ȡ�����ݱ�����StdFtInfo.DataBuf��,flag���ص��Ǳ����ǲ�������
    }
    //filenum = len / 13;    //13��TYPE IDENT 126 ��ÿһ�ļ���ص���Ϣ��
    TxMsg[0] = F_SG_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //�˴��п��������⣬�ؼ�����ô����Լ���Ȱ��ո���ʾ�����ı�д��2016��12��30��
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
        ScheduleFlag &= ~SCHEDULE_FT_DATA_STD;//TxMsg[FramePos++] = 0;    //�ļ�����=0
        return;
    }
    
    //StdFileInfo.FileChs += StdFileInfo.SectChs;
    EnCodeDLMsg(FramePos);    
        
    if(flag)
    {
        ScheduleFlag &= ~SCHEDULE_FT_DATA_STD;
        StdFileInfo.SectionNum--;
        ScheduleFlag |= SCHEDULE_FT_LAST_SECTION_FILE_STD;       //�����α�ʶ
    }
    
}

/*------------------------------------------------------------------/
�������ƣ�  StdProcFT_EncodeSegment()
�������ܣ�  ��׼������
����˵����  
���˵���� 
��ע��      
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
    //rc = StdFT_GetSection(&StdFtInfo, &len, &flag);     //���ļ��ĶΣ���ȡ�����ݱ�����StdFtInfo.DataBuf��,flag���ص��Ǳ����ǲ�������
    //filenum = len / 13;    //13��TYPE IDENT 126 ��ÿһ�ļ���ص���Ϣ��
    TxMsg[0] = F_LS_NA;
    TxMsg[1] = 0x01;   //VSQ
    TxMsg[2] = FILE_101; //COT
    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    for(int jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=RxPubAddr>>(8*jj);
    TxMsg[InfoAddrLocation] = LOBYTE(FT_DIRID_LB_STD);      //�˴��п��������⣬�ؼ�����ô����Լ���Ȱ��ո���ʾ�����ı�д��2016��12��30��
    TxMsg[InfoAddrLocation+1] = HIBYTE(FT_DIRID_LB_STD);
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
�������ƣ�  RMTReadAllPara()
�������ܣ�  ��֯��ȫ������
����˵����  
���˵����  
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
        //��װ���в���
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
        //��װ�ն����в���
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
        //��װ�ն����в���
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
        //��װ�ն˶�ֵ����
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
        //��װ�ն���·��ֵ����
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
�������ƣ�  EncodeRMTReadPara()
�������ܣ�  ��֯������
����˵����  
���˵����  
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
    
    TxMsg[6] = 1;   //����
    TxMsg[7] = 0;
    //TxMsg[8]  pi��
    
    FramPos = 9;
    if(RMTParaReadAllFlag)
    {
        rc = RMTReadAllPara(&TxMsg[FramPos], &len, &sendnum);   
        FramPos += len;
        TxMsg[1] = sendnum;
        if(rc == FALSE) 
        {
            TxMsg[8] = 0;     //�����������޺��� 
            RMTParaInit();
            ScheduleFlag &= ~SCHEDULE_RMTPARA;
        }
        else
        {
            TxMsg[8] = RP_PI_CONT;     //�����������к���
        }
    }
    else
    {
        sendnum = 0;
        procnum = 0;
        //���ֶ�
        for(i=RMTHaveReadParaFlag; i<RMTParaNum ; i++)
        {
            procnum++;
            len = 0;
            
            GetTerminalPara(&TxMsg[FramPos+3], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {
                sendnum++;
                TxMsg[FramPos++] = LOBYTE(RMTParaInfo[i]);
                TxMsg[FramPos++] = HIBYTE(RMTParaInfo[i]);  //��Ϣ���ַ
                TxMsg[FramPos++] = 0;
                
                FramPos += len;
            }
            else
            {
                //�ش���Ϣ���ַ���󣬲���������������
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
            //��������
            if(RMTHaveReadParaFlag >= RMTParaNum) 
            {
                RMTParaInit();
                TxMsg[8] = 0;     //�����������޺��� 
                ScheduleFlag &= ~SCHEDULE_RMTPARA;
            }
            else
            {
                TxMsg[8] = RP_PI_CONT;     //�����������к���
            }
        }
        else
        {
            //��Ϣ���ַ������֯�񶨻ش�
            RMTParaInit();
            TxMsg[1] = 0;       //VSQ=0 
            TxMsg[2] = COT_PONO|UNKNOWNTINFOADDR;  //����ԭ��
            TxMsg[8] = 0;     //�����������޺��� 
            FramPos  = 9; 
            ScheduleFlag &= ~SCHEDULE_RMTPARA;
        }
        
    }
    
    
    
    EnCodeDLMsg(FramPos);
}

/*------------------------------------------------------------------/
�������ƣ�  ProcReadParaGX
�������ܣ�  �����ٻ��������ģ�ȡ��ROI�������ٻ��޶��ʣ���
����˵����    
���˵����  �ޡ�
��ע��      ROI�Ǳ�����վҪ��ȡȫ�������Ƿ����ٻ�������
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
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
�������ƣ�  ProcEncodeGXReadPara()
�������ܣ�  ��֯������
����˵����  
���˵����  rc = TRUE-�к��� FALSE-�޺���
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
    TxMsg[1] = 0;   //VSQ�����������
    
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
                //RMTParaInit();                      �˴�Ϊ��Ҫ���ô˺��� 20181018��
            }
            TxMsg[FramePos++] = 0x06;   //QPM ������������ֵ���ͣ�������Ͷ�ˡ��Ҳ������޸�
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
             rc = GXReadJianGePara(&TxMsg[FramePos], &len, &sendnum, InfoAddrSize,GXParaControl);     //���������,Roi-INTRO2Ϊ�����ţ���0��ʼ
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
�������ƣ�  EnCodeGXReadParaEnd
�������ܣ�  ������������
����˵����    
���˵����  �ޡ�
��ע��      ������������Ϻ��Դ�֡���Ľ�����
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
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
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
    
    //�������ݴ浽wrongdata��
    //WrongDataLength = LengthIn;
    //GXvsqflag = 0;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }
   
    if(RxMsg[FrameLen+1] == 9)  //QPM
    {
        if((RxCot&COT_REASON)==ACT)    //����
        {
            pInfoAddr = &RxMsg[InfoAddrLocation];
           
            pos = 0;
            
            if(GXParaYZ)   //ֻ����һ֡���ĵ�Ԥ�ã�û�����겻��������Ԥ�ñ���
            {
                logSysMsgNoTime("��һ֡Ԥ�ò�����δ������Ч",0,0,0,0);
                return;
            }
            GXParaNum = RxVsq&VSQ_NUM;    //��ʱ������ ���в�����д��
            if((RxVsq & VSQ_SQ) == 0)
            {
                for(i=0; i<GXParaNum; i++)
                {
                    GXParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
                    pos += 3;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("gxԤ�ò���info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                } 
                            
            }
            else
            {
                //GXvsqflag = 1;
                //GXParaInfo[0] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
                infoaddr = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                pos += 3;
                for(i=0; i<GXParaNum; i++)
                {
                    GXParaInfo[i] = infoaddr + i;
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("gxԤ�ò���info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                }
            }
            
            if(GXRemoteParaCheck() == 1)
            {
                //�����쳣 ��Ӧ���ñ�����0������
                GXParaInit();             
                GXReturnCot = ACTCON|0x40;  //�񶨻ش�
            }
            else
            {
                GXParaYZ = TRUE;
                GXReturnCot = ACTCON; 
            }          
        }
        else if((RxCot&COT_REASON)==DEACT)  //����
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
        ProgLogWrite2("gxԤ�ò��� QPM=%d ����",RxMsg[FrameLen+1],0,0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
        
    }
    
    RxMsg[CotLocation] = GXReturnCot;
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    EnCodeDLMsg(FrameLen+2);    
}
/*------------------------------------------------------------------/
�������ƣ�  GXRemoteParaCheck
�������ܣ�  ��һ�������쳣
����˵����    
���˵����  
��ע��      ���쳣ֹͣ�������
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
�������ƣ�  ProcEncodeGXSendPara()
�������ܣ�  ��֯�������Ͳ����ı���
����˵����  
���˵����  
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
    
    TxMsg[FramePos++] = 0x06;   //QPM ������������ֵ���ͣ�������Ͷ�ˡ��Ҳ������޸� 
    
    //rc = GXReadJianGePara(&TxMsg[FramePos], &len, &sendnum, InfoAddrSize,GXParaControl);     //���������,Roi-INTRO2Ϊ�����ţ���0��ʼ
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
�������ƣ�  ProcEncodeGXChangePara()
�������ܣ�  ��֯�ظ��ı�����ı���
����˵����  
���˵����  
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
                //logSysMsgNoTime("104����ң������ֵ������Ч",0,0,0,0);
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
        Qpa = 0xff;     //��ʱ�����ط񶨻ش�
        logSysMsgNoTime("GXԶ�̲���δԤ�þ͹̻�104",0,0,0,0);
    } 
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        pDLink->ConfS();
        
        return;
    }
    
    //��ȷ��֡
    RxMsg[CotLocation] = GXReturnCot;
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    EnCodeDLMsg(FrameLen+2);
     
    ProcEncodeGXChangePara();  
    
    GXParaInit();
}
/*------------------------------------------------------------------/
�������ƣ�  GXWatchLPChange
�������ܣ�  ����Զ�̲��������ӱ��ز����仯��
����˵����    
���˵����  �ޡ�
��ע��      ROI�Ǳ�����վҪ��ȡȫ�������Ƿ����ٻ�������
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
�������ƣ�  ProcReadPara()
�������ܣ�  ���������
����˵����  
���˵����  
/------------------------------------------------------------------*/
void New104Sec::ProcReadPara(void)
{
    INT8U *pInfoAddr;
    INT16U i, pos;
    INT16U num;
    
    //���ݳ��ȴ���8����ʾ�ǲ��ֶ�ȡ������Ϊȫ����ȡ
    if(FrameLen+2 > 9) //FrameLen+2��ȥ��������C�ĳ���
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pInfoAddr = &RxMsg[InfoAddrLocation+2];
        pos = 0;
        num = ((FrameLen+2)-8)/3;   //�����ȡ�����ĸ���
        
        if(RMTParaNum ==0 ) //��1�ζ�
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

        RMTParaNum += num;  //��¼�ۼ��·��Ķ���������
    }
    else
    {
        //ȫ����ȡ   
        RMTParaReadAllFlag = TRUE;
        RMTHaveReadParaFlag = 1;
        RMTParaNum = 0;
    }
    
    SendData1(SCHEDULE_RMTPARA);
    
}
/*------------------------------------------------------------------/
�������ƣ�  RMTParaYzCheck()
�������ܣ�  
����˵����  
���˵����  
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
�������ƣ�  GXParaYzCheck()
�������ܣ�  
����˵����  
���˵����  
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
�������ƣ�  RMTParaInit()
�������ܣ�  Զ�̲�����д��Ӧ��־��ʼ��
����˵����  
���˵����  
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
�������ƣ�  DeadValueRealTimeEffect()
�������ܣ�  �·���������ʵʱ��Ч
����˵����  
���˵����  
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
�������ƣ�  ProcWritePara()
�������ܣ�  ����д����
����˵����  
���˵����  
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

    if ((RxCot&COT_REASON)==ACT)    //����
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pi  = RxMsg[InfoAddrLocation+2];
        pInfoAddr = &RxMsg[InfoAddrLocation+3];
                
        pos = 0;
        if(pi & RP_PI_SE)   //Ԥ�ã�����ѡ��
        {
            if(RMTParaYZ)   //ֻ����һ֡���ĵ�Ԥ�ã�û�����겻��������Ԥ�ñ���
            {
                logSysMsgNoTime("��һ֡����Ԥ�ñ��Ļ�δ����",0,0,0,0);
                return;
            }   
            RMTParaNum = RxVsq&VSQ_NUM;    //��ʱ������ ���в�����д��
            for(i=0; i<RMTParaNum; i++)
            {
                RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
                pos += 3;
                
                datatype = pInfoAddr[pos++]; //��������
                datalen  = pInfoAddr[pos++]; //���ݳ���
                
                if((datatype == PARA_DATA_TYPE_WORD) && (datalen == 2))
                {            
                    RMTParaValue[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                    pos +=2 ; 
                    
                    ProgLogWrite2("Ԥ�ò���info=0x%x, value=%d",RMTParaInfo[i],RMTParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else if((datatype == PARA_DATA_TYPE_DWORD) && (datalen == 4))
                {            
                    RMTParaValue[i] = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    pos +=4 ; 
                    
                    ProgLogWrite2("Ԥ�ò���info=0x%x, value=%d",RMTParaInfo[i],RMTParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else if((datatype == PARA_DATA_TYPE_FLOAT) && (datalen == 4))
                {
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;     
                    pos +=4 ; 
                    ProgLogWrite2("Ԥ�ò���info=0x%x, value=%d.%3d",RMTParaInfo[i],(INT16U)RMTParaValue[i],(INT16U)((RMTParaValue[i]-(INT16U)(RMTParaValue[i]))*1000),0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else if((datatype == PARA_DATA_TYPE_BOOL) && (datalen == 1))
                {
                    RMTParaValue[i] = pInfoAddr[pos];
                    pos +=1;
                    ProgLogWrite2("Ԥ�ò���info=0x%x, value=0x%x",RMTParaInfo[i],RMTParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                }
                else
                {
                    logSysMsgNoTime("Ԥ�ò��� info=0x%x �������ʹ���(%d),���ݳ���=%d",RMTParaInfo[i], datatype,datalen,0);
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
            //�̻�������ִ�У�
            IsSuccess = TRUE;
            if((pi & RP_PI_CR) == 0)
            {
                if(RMTParaYZ)
                {
                    
                    //���ﲻ��ҪЯ������
                    ProgLogWrite2("�յ��̻���������", 0, 0, 0, 0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    //Զ�̲������ù̻����̲����Я���Ĳ���
                    for(i=0; i<RMTParaNum; i++)
                    {
                        ParaFlag = SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]);
                        
                    }
                    
                }
                else
                {
                    IsSuccess = FALSE;
                    logSysMsgNoTime("Զ�̲���δԤ�þ͹̻�",0,0,0,0);
                }
                
                RMTParaInit();
                
                RxMsg[CotLocation] = ACTCON;
                
                if(IsSuccess==FALSE)
                {
                	RxMsg[CotLocation] |= 0x40; //ʧ��
                }
                else
                {
                    
                    if(GetSiQuChangeFlag(ParaFlag))           //��Լ��Ĳ���ʵʱ��Ч������ͬ����������ֵ��ʵʱ��Ч�ķ�ʽʵ�֡�
                    {
                        DeadValueRealTimeEffect();
                    }
                    SaveTerminalPara(); //�̻���ϣ�д��flash
                    SaveRMTParaToSys(); //����Ƿ���Ҫ����ϵͳ�����ļ�
                }
                
                
                memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
                EnCodeDLMsg(FrameLen+2);
                
            }  
        }
        
    }
    else
    {
        //����
        RMTParaInit();
        
        RxMsg[CotLocation] = DEACTCON;
        memcpy((void*)TxMsg, (void*)RxMsg, FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
    }
    
    
    
}
/*------------------------------------------------------------------/
�������ƣ�  ProcSetSectionNo()
�������ܣ�  �л���ֵ����
����˵����  
���˵����  
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
        logSysMsgNoTime("���õ�ǰ����=%d",RMTSectionNo,0,0,0);
    }
    else
        RMTSectionNo = 0;
        
    RxMsg[CotLocation] = ACTCON;
    memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
    EnCodeDLMsg(FrameLen+2);
    
}


/*------------------------------------------------------------------/
�������ƣ�  ProcReadSectionNo()
�������ܣ�  �л���ֵ����
����˵����  
���˵����  
/------------------------------------------------------------------*/
void New104Sec::ProcReadSectionNo(void)
{
    INT8U FramePos;
    
    if (!pDLink->GetFreeTxUnit(PRIORITY_2,&TxMsg))
    {
        return ;    //���������һȡ�������������в�Ӧ��ķ��ա� ll
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
    TxMsg[FramePos++] = RMTSectionNo2;  //��ǰ����
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;
    TxMsg[FramePos++] = 0;
    
        
    EnCodeDLMsg(FramePos);
    
}

/*------------------------------------------------------------------/
�������ƣ�  CheckFREOnTime()
�������ܣ�  ��ʱɨ������¼�����.ֻ����չ��Լ����Ҫ
����˵����  
���˵����  
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
�������ƣ�  EnCodeFREvent()
�������ܣ�  �����¼��ϴ�
����˵����  
���˵����  
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
        return ;   //û�з��Ϳռ��򱣳ֱ�־λ
        
    ScheduleFlag&=(~SCHEDULE_FT_FREVENT);
    
    if(GWFREventRead(&frevent, DevList[NvaActDevNo].DevID) == FALSE)    //��ʱ����͸���rpָ��
        return ;
    
    TxMsg[0] = M_FT_NA;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = SPONT;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    //����û����Ϣ���ַ
    sendpos = 6;
    TxMsg[7] = M_SP_TB;             //ң������ ���� 
    
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
            TxMsg[FramePos++] = 0;      //��Ϊ��׼��д����2��������Ժ�ע��
        
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
    TxMsg[sendpos] = sendnum;       //ң�Ÿ���
    
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
             
             case M_ME_NC: //�̸�����
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
             default:   //M_ME_NA ��һ��ֵ
                TxMsg[FramePos++]=LOBYTE(frevent.actvalue[i]);
                TxMsg[FramePos++]=HIBYTE(frevent.actvalue[i]);
                break; 
            } 
        }
    }
    TxMsg[sendpos] = sendnum;       //ң�����
    
    EnCodeDLMsg(FramePos);
        
}




/*------------------------------------------------------------------/
�������ƣ�  GetWhLogicDevID()
�������ܣ�  �õ�104 �е��豸ID
����˵����  
���˵���� 
��ע��      
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
    SetFileSynInfoTaskIDSubstation(MySelf.AppTID);  //��Ϊ����101��վ����ʼ����
    if(XSFileSynInfo.TaskIDPri101[0]!=0)//�����һ������ģ���������
    {
        myEventSend(GetFileSynInfoTaskID101(0),XSFREEZE);//��101��վ��������Ϣ ��ʱ�ȷ�����һ��101���񣬺�����ͨ��ά�����������ȷ�ϵġ�
    }
    else
    {
        logSysMsgWithTime("��֧��2018��׼������ģ��,��֧��˲ʱ���ᣡ",0,0,0,0);  
    }
    if(XSFileSynInfo.TaskIDPri101[1]!=0)//�����еڶ�������ģ���������
    {
        myEventSend(GetFileSynInfoTaskID101(1),XSFREEZE);
    }     
}

/*------------------------------------------------------------------/
�������ƣ�  EnCodeNACK()
�������ܣ�  ��ȷ��
����˵����  ��ԭ��    
���˵���� 
��ע��      
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
�������ƣ�  ProcInitEnd()
�������ܣ�  ��ʼ������������
����˵����  
���˵���� 
��ע������ѡ����λʱ����ʼ������֡�������ҽ�����λ����վ����start�ն˻ظ�ȷ�Ϻ󣬲ŷ��ͳ�ʼ������֡��
����ѡ��һ���յ�Start�ظ�ȷ�Ϻ�ͷ��ͳ�ʼ������֡��Ĭ��Ϊ����ѡ��      
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
        if(InitFlag == 0xff)        //wjr  ��ʼ������
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
�������ƣ�  ProcReadParaGD
�������ܣ�  �����ٻ��������ġ�
����˵����    
���˵����  �ޡ�
��ע��      
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
    //    RMTParaNum = 33;   //һ֡��ഫ33����һ��ֻ��30��
    pos = 0; 
    if((RxVsq & VSQ_SQ) == 0)
    {
        for(i=0; i<RMTParaNum; i++)
        {
            RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
            pos += 7; //3�ֽ���Ϣ���ַ+4�ֽ�����
                          
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
�������ƣ�  EncodeRMTReadPara_GD()
�������ܣ�  ��֯���㶫Զ�̲���
����˵����  
���˵����  
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
                TxMsg[FramPos++] = HIBYTE(RMTParaInfo[i]);  //��Ϣ���ַ
                TxMsg[FramPos++] = 0;
                
                FramPos += len;
            }
            else
            {
                //�ش���Ϣ���ַ���󣬲���������������
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
        TxMsg[FramPos++] = HIBYTE(RMTParaInfo[0]);  //��Ϣ���ַ
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
                //�ش���Ϣ���ַ���󣬲���������������
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
        //��Ϣ���ַ������֯�񶨻ش�
        RMTParaInit();
        TxMsg[1] = 0;       //VSQ=0 
        TxMsg[2] = COT_PONO|UNKNOWNTINFOADDR;  //����ԭ��
        
    }

    EnCodeDLMsg(FramPos);
}
/*------------------------------------------------------------------/
�������ƣ�  ProcWritePara_GD()
�������ܣ�  ����д����
����˵����  
���˵����  
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
        //�񶨻ش�
        RxMsg[CotLocation] = ACTCON|0x40;
        memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
        
        logSysMsgNoTime("Զ������Ͷ����ѹ��Ϊ�֣���ֹ�޸Ĳ�������",0,0,0,0);
        return;
    }    
    
    writeflag = 0;
    if ((RxCot&COT_REASON)==ACT)    //����
    {
        pInfoAddr = &RxMsg[InfoAddrLocation];
                
        pos = 0;    
        RMTParaNum = RxVsq&VSQ_NUM;    //��ʱ������ ���в�����д��
        if((RxVsq & VSQ_SQ) == 0)
        {
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+7];
                if(qos & 0x80)  //Ԥ��
                {
                    RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
                    pos += 3;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    ProgLogWrite2("Ԥ�ò���info=0x%x, value=%d,num=%d",RMTParaInfo[i],RMTParaValue[i],RMTParaNum,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    //ִ��
                    curparainfo = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                    pos += 3; 
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //�ж�Ԥ�úͼ�����ͬ��������
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
            //ProgLogWrite2("104�ݲ�֧��vsq=1�����",0,0,0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
            info = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
            pos += 3;
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+4];
                if(qos & 0x80)  //Ԥ��
                {
                    RMTParaInfo[i] = info+i;
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    ProgLogWrite2("Ԥ�ò���info=0x%x, value=%d,num=%d",RMTParaInfo[i],RMTParaValue[i],RMTParaNum,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    curparainfo = info+i;
                    //ִ��
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //�ж�Ԥ�úͼ�����ͬ��������
                    if((curparainfo == RMTParaInfo[i]) && (tempval == RMTParaValue[i]))
                    {
                        SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]);  
                        writeflag = 1;
                    } 
                }
    
            }
        }   
        //ȷ�ϻش�
        RxMsg[CotLocation] = ACTCON;
        memcpy((void*)TxMsg,(void*)RxMsg,FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
        
        if(writeflag)   //�̻�
        {
            SaveTerminalPara(); //�̻���ϣ�д��flash
            SaveRMTParaToSys(); //����Ƿ���Ҫ����ϵͳ�����ļ�
            
            RMTParaInit();
        }
        
    }
    else
    {
        //����
        RMTParaInit();
        
        RxMsg[CotLocation] = DEACTCON;
        memcpy((void*)TxMsg, (void*)RxMsg, FrameLen+2);
        EnCodeDLMsg(FrameLen+2);
    }
    
    
    
}
