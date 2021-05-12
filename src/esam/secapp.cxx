#include "secapp.h"
#include "secdlink.h"
#include "procdef.h"

//�����ļ�ͬ��  CL 20180504
#include "..\newhis\XSDataProc.h"
//�����ļ�ͬ��  CL 20180504

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
    //����APP����
    CSecAppSev *pSecApp;
    pSecApp=new CSecAppSev(AppID);
    
    
    
    if(!pSecApp)
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec ���񴴽�ʧ��1",0,0,0,0);
        #else
        logSysMsgNoTime("101 Sec Delete Itself",0,0,0,0);
        #endif
        myTaskSuspendItself();
    }
    else if(!pSecApp->InitSecApp())
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec ���񴴽�ʧ��2",0,0,0,0);
        #else
        logSysMsgNoTime("101 Sec Delete Itself",0,0,0,0);
        #endif
        delete pSecApp;
        myTaskSuspendItself();
    }

    //����DL����
    CSecDLink *pDLink;
    pDLink=new CSecDLink(AppID,pSecApp);

    if(!pDLink)
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec ���񴴽�ʧ��3",0,0,0,0);
        #else
        logSysMsgNoTime("101 Sec Delete Itself",0,0,0,0);
        #endif
        delete pSecApp;
        myTaskSuspendItself();
    }
    else if(!pDLink->InitSecDLink())
    {
        #ifdef _CHINESE_
        logSysMsgNoTime("101 Sec ���񴴽�ʧ��4",0,0,0,0);
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
        logSysMsgNoTime("���ڴ򿪴���101 Sec ����ɾ��",0,0,0,0);
        #else
        logSysMsgNoTime("Serial Open Fail,101 Sec Delete",0,0,0,0);
        #endif
        delete pSecApp;
        delete pDLink;
        myTaskSuspendItself();
    }
    //���ö�ʱ��
    tm_evevery(1*SYSCLOCKTICKS,APPTIMEFLAG,&dwAPPTimerID);
    tm_evevery(1*SYSCLOCKTICKS,TIMERFLAG,&dwDLTimerID);     //�붨ʱ��
    
    //�ж�ͨѶģʽ
    //if((pDLink->BalanMode))  //ƽ��ģʽ�ҷ�֧����չ��Լʱ ˫��������
    //    myEventSend(myTaskIdSelf(),SCHEDULE);
        
    FlushBuf(AppID,1);
    FlushBuf(AppID,0);
    //�����������
    pSecApp->InitHisDataBuf();
    pSecApp->RebootCheckUDataFlag();
    pSecApp->PassSprValueToLink(&pDLink->En_LinkAddrSize,&pDLink->En_CotSize,&pDLink->En_PubAddrSize,&pDLink->En_InfoAddrSize);
    ////pDLink->InitEnSprParaData(pSecApp->LinkAddrSize,pSecApp->CotSize,pSecApp->PubAddrSize,pSecApp->InfoAddrSize);
    //���������ѭ��
    for(;;)
    {
        myEventReceive(RX_AVAIL|TX_AVAIL|TIMERFLAG|APPTIMEFLAG|UDATAFLAG|UMSGFLAG|SCHEDULE|FORCESCHEDULE
                        |NEXTFRAME|SENDOVER|SAVEKWHFLAG|XSFILESYNFINISH,MY_EVENTS_WAIT_ANY ,WAIT_FOREVER ,&dwEvent);

        if (dwEvent&SAVEKWHFLAG)//���涨ʱ���
        {
            pSecApp->SaveKWHToBuf();
        }

        if(dwEvent&NEXTFRAME)
        {
            pDLink->SearchFrame();
        }

        if(dwEvent&RX_AVAIL)//��������
        {
            pDLink->RecMISIData();
        }

        if(dwEvent&TX_AVAIL)//��������
        {
            pDLink->SendDataToMISI();
        }

        if(dwEvent&SENDOVER)//��������
        {
            if(pDLink->BalanMode)
                pDLink->SendDataEnd();
        }

        if (dwEvent&UMSGFLAG)//YK��Ϣ��������Ӧ�ò����
        {
            pSecApp->SetUMsgFlag();

            if(pDLink->BalanMode&&pDLink->DLStatusCheck())
                pDLink->CallUMsg();
        }

        if(dwEvent&UDATAFLAG)//���ݿ������COS��SOE�������ñ�־��
        {
            if(0 == pSecApp->LCFlag)
            {
                pSecApp->SetUDataFlag();
            }
            if(pDLink->BalanMode && pDLink->DLStatusCheck())
                pDLink->CallUData();
        }

        if(dwEvent&APPTIMEFLAG)//��ʱ����Ӧ�ò㣩
        {
            pSecApp->OnTimer();
        }

        if(dwEvent&TIMERFLAG)//��ʱ������·�㣩
        {
            pDLink->TimeOut();
        }

        if (dwEvent&SCHEDULE)//������·����
        {
            pDLink->CallDLStatus();
        }
        if(dwEvent&FORCESCHEDULE)
        {
            pDLink->NotifyToAppSchedule();
        }
        if(dwEvent&XSFILESYNFINISH)
        {
            pSecApp->ProcXSFileSynFinish(); //�ļ�ͬ��ֹͣ
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

    pRestType = (INT8U *)nvramMalloc(1);    //��λ��Լ����״̬���� ll 2010/07/20   for ������Լ����
    
    
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

    pMsg=new PMessage;//������Ϣ��������������ֻ��һ����

    if (GetPortInfo(MySelf.PortID,&ppport)==TRUE)//ȡ�˿���Ϣ��ַ
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
            logSysMsgNoTime("iec101Sec �޹�Լ��壡������",0,0,0,0);
            #else
            logSysMsgNoTime("iec101Sec No CodePad! Please Check. ",0,0,0,0);
            #endif
            SetProtocalErr();
            SetDefaultPad();
        }

        pDev->DevID=(ppport)->DevIDSet[0];  //�����豸ID

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
    if (Sec101Pad.MaxALLen>ASDULEN)//Ӧ�ò�����ĳ��ȳ���
        Sec101Pad.MaxALLen=ASDULEN;
    else if(Sec101Pad.MaxALLen<16)
        Sec101Pad.MaxALLen=ASDULEN;

    if ((Sec101Pad.ScanData2>3600))//��������ɨ����
        NvaInterval=SCANDATA2TIMER;
    else
        NvaInterval=Sec101Pad.ScanData2;

    BackScanTime=Sec101Pad.BackScanTime*60;//�֣���������ɨ����
    CycScanTime=Sec101Pad.CycScanTime;//�룬����ѭ������ɨ����

    if(Sec101Pad.BalanceMode == 1)
        BalanMode=TRUE;
    else
        BalanMode=FALSE;
    //��ʷ���
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
        
    if((Sec101Pad.LBIinfoaddr<LBI)||(Sec101Pad.LBIinfoaddr>HBI))        //ң����Ϣ���ַ������ 2008.11.5  wjr
        LBIinfoaddr=LBI;
    else
        LBIinfoaddr=Sec101Pad.LBIinfoaddr;
        
    if((Sec101Pad.LDBIinfoaddr<LDBI)||(Sec101Pad.LDBIinfoaddr>HDBI))
        LDBIinfoaddr=LDBI;
    else
        LDBIinfoaddr=Sec101Pad.LDBIinfoaddr;
    
}
/******************************************************************
*������:PassSprValueToLink
*����:�����˽�б�����Ϣ��������·��
*������:����
*******************************************************************/
void CSecAppSev::PassSprValueToLink(INT16U *eparaI,INT16U *eparaII,INT16U *eparaIII,INT16U *eparaIV)
{
    ////INT16U 
    *eparaI = LinkAddrSize;
    *eparaII = CotSize;
    *eparaIII = PubAddrSize;
    *eparaIV = InfoAddrSize;
}

void CSecAppSev::SetDefaultPad(void)//ȱʡ����
{
    int i;
    Sec101Pad.ControlPermit = 1;       //ң������ 1-����0-������ ȱʡΪ1
    Sec101Pad.SetTimePermit = 1;  //�������� 1-����0-������ ȱʡΪ1
    Sec101Pad.BalanceMode = 0;//ƽ��ģʽ 1-ƽ��ģʽ 0-��ƽ��ģʽ ȱʡΪ0
    Sec101Pad.SOEWithCP56Time = 1;//SOEʹ�ó�ʱ���ʽ 1-56λ��ʱ�� 0-24λ��ʱ��  ȱʡΪ1
    Sec101Pad.UseStandClock = 1;//ʹ�ñ�׼ʱ�Ӹ�ʽ 1-��׼ 0-�Ǳ�׼ ȱʡΪ1
    Sec101Pad.MaxALLen = ASDULEN; //���Ӧ�ò㱨�ĳ��� ȱʡ250
    Sec101Pad.AIDeadValue = 3;//ң������ֵ��ǧ�ֱȣ� ȱʡ3
    Sec101Pad.ScanData2 = SCANDATA2TIMER;//��������ɨ�������룩 ȱʡ3
    Sec101Pad.TimeOut = 100;//��ʱʱ�䣨*10ms�� ȱʡ100

    Sec101Pad.HistoryDDPermit = 0;//��ʷ��ȱ������� 1-���� 0-������ ȱʡ0
    Sec101Pad.HistoryDDTime = 60;//��ʷ��ȱ������ڣ��֣� ȱʡ60


    BackScanTime=0;//�֣���������ɨ����
    CycScanTime=20;//�룬����ѭ������ɨ����
    HisDDCycle=TIMETOSAVEDATA;

    BalanMode=FALSE;

    for(i=0;i<8;i++)
        Sec101Pad.TypeID[i]=M_PS_NA;//20;//����״̬��λ����ĳ��鵥����Ϣ
    for(i=8;i<12;i++)
        Sec101Pad.TypeID[i]=M_ME_NB;//11;////����ֵ����Ȼ�ֵ
    Sec101Pad.TypeID[12]=0;//����Ϊ��
    Sec101Pad.TypeID[13]=M_ME_NB;//11;//����Ϊ����
    Sec101Pad.TypeID[14]=M_ST_NA;//5;//����Ϊ��λ����Ϣ
    for(i=0;i<16;i++)
        Sec101Pad.GroupNum[i]=0;

    Sec101Pad.TypeID[16]=M_IT_NA;

    LinkAddrSize=2;
    CotSize=1;
    PubAddrSize=2;
    InfoAddrSize=2;
    
    LBIinfoaddr=LBI;                     //ң����Ϣ���ַ������ 2008.11.5  wjr
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

    if (DevType == 2)//�����߼��豸
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

        //������豸��Ŀ
        DevCount=AppSLBConf.DevNum;
        if (DevCount<=0)
            return(FALSE);

        DevList=new PDevInfo[DevCount];
        if(!DevList)
            return(FALSE);

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
                DevList[i].Data1.SOENum=0;            //SOE���͸���
                DevList[i].Data1.BIENTNum=0;      //��λң�ŷ��͸���
                DevList[i].Data1.FAProcNum=0;
            }
            else if(DevType == 0)//I���߼��豸
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
                DevList[i].Data1.SOENum=0;            //SOE���͸���
                DevList[i].Data1.BIENTNum=0;      //��λң�ŷ��͸���
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
        memset(YXGroupNo,1,pDev->DevData.BINum);//���豸����ʱ�����飬ң�����ͱ�ʶ���õ�һ���
        memset(YCGroupNo,9,pDev->DevData.AINum);//ң�����ͱ�ʶ���õھ���ģ����������
        for(i=0;i<16;i++)
            Sec101Pad.GroupNum[i]=512;
        return(TRUE);
    }
    else if (DevType == 0)//һ���߼��豸
    {
        L_ReadBConf(DevID, wAppID,(INT8U *) &AppLBConf);

        pDev->Flag=AppLBConf.Flag;
        pDev->Addr=AppLBConf.Address;
        pDev->MAddr=AppLBConf.MAddress;
        pDev->pDbaseWin=AppLBConf.DbaseWin;
        pDev->RealWin=NULL;

        //������豸��Ŀ
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
        DevList[0].DevData.DBINum=AppLBConf.DBINum;              //wjr˫��ң��
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
        DevList[0].Data1.SOENum=0;            //SOE���͸���
        DevList[0].Data1.BIENTNum=0;      //��λң�ŷ��͸���
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

        //Ϊÿ��ң��ң���������
        YCGroupNo = new INT8U[DevList[0].DevData.AINum];
        YXGroupNo = new INT8U[DevList[0].DevData.BINum];
        memset(YCGroupNo,9,DevList[0].DevData.AINum);
        memset(YXGroupNo,1,DevList[0].DevData.BINum);

        INT16U yc=0,yx=0;
        /*�����жϣ����ң�����ܵ�ң�š�ң�������ʵ�ʸ�����Ӧ��������������� wjr*/
        for(i=0;i<16;i++)
        {
            if(i<8)
                yx+=Sec101Pad.GroupNum[i];
                
            else if(i<12)
                yc+=Sec101Pad.GroupNum[i];
        }
        if((yx>DevList[0].DevData.BINum)||(yc>DevList[0].DevData.AINum))
        {
        	logSysMsgNoTime("ң���ң�Ÿ���С�����и���",0,0,0,0);
        	SetProtocalErr();
        	return(FALSE);
        }
        if((yx<DevList[0].DevData.BINum)||(yc<DevList[0].DevData.AINum))
            logSysMsgNoTime("����ң���ң�Ÿ���С���߼����и�������©���������",0,0,0,0);
            
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
        
        
        if(yx<DevList[0].DevData.BINum)//������Ŀ���������ٻ������⡣����ܺͲ���С�ڷ��ͱ���Ŀ��
            DevList[0].DevData.BINum=yx;

        if(yc<DevList[0].DevData.AINum)
            DevList[0].DevData.AINum=yc;

        return TRUE;//��󷵻�
    }
    else
        return FALSE;
}

void CSecAppSev::ReadAIMaxVal(INT16U DevIndex)  //��ң����ֵ������������ֵ
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
        if(DevList[DevIndex].Flag == 1)//=1ʵ���豸
            val = SRSendMax_ReadAI(DevID,i);
        else
            val = SLMax_ReadAI(DevID,i);

        DevList[DevIndex].DevData.AIMaxVal[i] = val;
        
        DevList[DevIndex].DevData.AIPara[i].porperty = SL_ReadAI_Porperty(DevID,i);
     
        //�����������ֵ
        if(DevList[DevIndex].Flag == 1)
            deathval = SRDead_ReadAI(DevID,i);
        else
            deathval = SLDead_ReadAI(DevID,i);
            
        if(deathval > 1)    //��0����1����Ϊ��Ч
        {
            DevList[DevIndex].DevData.AIPara[i].DeadValue = deathval;   //����ǧ�ֱȵ�λ
        }
        else
        {
            ppty = DevList[DevIndex].DevData.AIPara[i].porperty;
            deathval = GetRmtDeathvalue(ppty);
            if(deathval > 0)    //Զ�̲�������ֵ
            {
                DevList[DevIndex].DevData.AIPara[i].DeadValue = ((INT32U)val*deathval)/1000;
            }
            else //��Լ�������ֵ
            {
                 DevList[DevIndex].DevData.AIPara[i].DeadValue = ((INT32U)val*Sec101Pad.AIDeadValue)/1000;
            }
        }
        
        DevList[DevIndex].DevData.AIPara[i].UpLimit = 0;
        DevList[DevIndex].DevData.AIPara[i].LowLimit = 0;
        DevList[DevIndex].DevData.AIPara[i].type = SL_ReadAI_Type(DevID,i); //��ȡ��ǰ���ݵ����ͣ��з��Ż����޷��ţ�
        if (DevList[DevIndex].DevData.AIPara[i].DeadValue == 0)
            DevList[DevIndex].DevData.AIPara[i].DeadValue = 1;
    }

}

void CSecAppSev::InitPara(void)
{
    Data1.Flag=0;
    Data1.SOENum=0;            //SOE���͸���
    Data1.BIENTNum=0;      //��λң�ŷ��͸���

    AllDataCount=0;
    CounterCount=0;
    WatchDogCount=0;
    NvaActDevNo=0;
    ActDevIndex=0;    //wjr���ӳ�ʼ��

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
        
    //֧��"������Լ��չ"-2015��
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
        logSysMsgNoTime("101֧��GY2015��չ",0,0,0,0);
    }
    
    APP_DLSecStatus = SECDISABLE;
    APP_DLPriStatus = PRIDISABLE;   
    FirstCallAllData = 0;
    DLInitFinishFlag = FALSE;
    DLInitpreState = FALSE;    //AJ++180418
    WaitCallAllDelay = WAIT_CALLALL_DELAY;
    SetDevUseState();   //��Ҫ�ǳ�ʼ��DLInitFinishFlag
    
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

    CotLocation=2;//COT��ASDU�е�λ��
    PubAddrLocation=CotLocation+CotSize;//PUBADDR��ASDU�е�λ��
    InfoAddrLocation=PubAddrLocation+PubAddrSize;//INFOADDR��ASDU�е�λ��
    AsduHeadLength=InfoAddrLocation+InfoAddrSize;//ASDUͷ�ĳ���

    if(PubAddrSize==1)
        BroadCastAddr=0xff;
    else
        BroadCastAddr=0xffff;

    RFaultFlag=0;   //beijing
    YKSetAlready = FALSE;
    
    DBISOEnum=0;         //wjr˫��ң��
    DBICOSnum=0; 
    
    DDFreeze=FALSE;
    
    YkStatusForTest = 0;
    
    RMTParaInit();
    GXParaInit();
    //���ļ����������ʼ��
    ProcFileInit();
}
void CSecAppSev::GXParaInit(void)
{
    GXParaYZ = FALSE;
}
void CSecAppSev::SendAllDataOnTime(void)
{
    //�������ݱ�־��ȫ������BACKԭ���ͣ�
    /*if(BackScanTime)
    {
        BackScanCount++;
        if(BackScanCount>=BackScanTime)
        {
            if(Data1.Flag&CallAllData)//�����ͻ
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
            GroupTrn.GroupNo=1;//��Ŵ�1��ʼ
            GroupTrn.InfoAddr=LBIinfoaddr;//��Ϣ���ַ��0x1��ʼ   wjr2009.4.5  ��Ϊ˫��ң�ŵ���ʼ��ַ����Ϊ˫��ң����ǰ
        }
    }*/
        //����ѭ�����ݱ�־����12��ң�����ݣ�Ϊ��Ӧ�����ԣ�������ʱ���˴���
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
            GroupTrn.GroupNo=12;//���12����ϢΪ����ѭ����Ϣ
            GroupTrn.InfoAddr=LAI;//��Ϣ���ַ����
        }*/
    
    
    
}
/*------------------------------------------------------------------/
�������ƣ�  EnCodeFREvent()
�������ܣ�  �����¼��ϴ�
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::EnCodeFREvent(void)
{
    INT16U i, no, FramePos;
    struct FaultRec_t frevent;
    struct Iec101ClockTime_t time;
    float temp;
    INT32U dd;
    INT8U *p, *pdd, aitype;
    
    
    if(GWFREventRead(&frevent, DevList[NvaActDevNo].DevID) == FALSE)    //��ʱ����͸���rpָ��
        return ;
    
    TxMsg[0] = M_FT_NA;
    TxMsg[1] = 1;   //VSQ
    TxMsg[2] = SPONT;
    TxMsg[3] = 0;
    TxMsg[4] = LOBYTE(DevList[ActDevIndex].Addr);
    TxMsg[5] = HIBYTE(DevList[ActDevIndex].Addr);
    
    //����û����Ϣ���ַ
    
    TxMsg[6] = frevent.yxnum;   //ң�Ÿ���
    TxMsg[7] = 1;               //ң������ ���� 
    
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
    
    TxMsg[FramePos++] = frevent.ycnum;   //ң�Ÿ���
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
         
         case M_ME_NC: //�̸�����
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
         default:   //M_ME_NA ��һ��ֵ
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
�������ƣ�  CheckFREOnTime()
�������ܣ�  ��ʱɨ������¼�����
����˵����  
���˵����  
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
�������ƣ�  SetDevUseState()
�������ܣ�  ���ͨѶ״̬
����˵����  
���˵����  
��ע��      
/------------------------------------------------------------------*/
void CSecAppSev::SetDevUseState(void)
{
    
    if(DLInitpreState != DLInitFinishFlag)
    {
        DLInitpreState = DLInitFinishFlag;
        if(DLInitFinishFlag==TRUE)
        {
            ProgLogWrite2("�˿�%d 101��վ���ӽ���",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 1);
        }
        else
        {
            ProgLogWrite2("�˿�%d 101��վ���ӶϿ�",MySelf.AppID,0,0,0, SYSINFO_WITHTIME, ULOG_TYPE_COMSTATE, 0);
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
            DLInitFinishFlag = TRUE;    //ֻҪ�ӳٵ�0����·��ʼ���������,��ʼ�����Ӧ�ò㼴�����ⷢ������
        
   
        if(WaitCallAllDelay)    //�ȴ����н�����ʱ
        {
            WaitCallAllDelay--;
            if(FirstCallAllData)
                WaitCallAllDelay = 0;
        }
        else
            FirstCallAllData = 0xff;
        
        
    }
    
    if(APP_DLSecStatus==SECENABLE)  //ֻҪ�Ӷ�������·���ã��Ϳ��Խ��ܸ�λ����
    {
        if(ResetFlag==0xff)//�յ���λ��������ñ�־���������������Ӻ�λ
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
    
    
    if(DLInitFinishFlag == FALSE)   //��ʼ��δ��ɽ�ֹ������ʱ����
        return;
    
    if(FirstCallAllData != 0xff)    //��ʼ����ɺ󣬵ȴ����н�����������û����30�룩��
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
    
    //�仯ң�ⶨʱɨ��
    ScanData2Count++;
    if(ScanData2Count>=NvaInterval)
    {
        ScanData2Count=0;
        if(CheckNVA() && (0 == LCFlag))
            Data1.Flag|=HaveNVA;
    }
}


//SecAppProc����·�ӿں���
//���������bufin:����Ļ��������ݵ�ַ,�����ͱ�ʶ��ʼ��
//���������lengthinΪӦ�ò����ݳ��ȣ�
//���������dlcommandΪ��·�㵽Ӧ�ò��Ĺ�����
//���������bufout:����Ļ��������ݵ�ַ�������ͱ�ʶ��ʼ��
//���������lengthoutΪӦ�ò����ݳ��ȣ�
//���������appcommandΪӦ�ò㵽��·�������
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
    
    //logSysMsgNoTime("pubaddrloc=%d�� PubAddr=%d, r=%d, r2=%d",PubAddrLocation,RxPubAddr,RxMsg[PubAddrLocation],RxMsg[PubAddrLocation+1]); //debug
    
    RxInfoAddr=0;
    for(i=0;i<2;i++)//InfoAddrSize���Ϊ3��Ҳֻȡǰ2���ֽڡ�
        RxInfoAddr+=(RxMsg[InfoAddrLocation+i]<<(8*i));

    pTxTypeID=TxMsg;
    pTxVSQ=(TxMsg+1);
    pTxInfoAddr=(TxMsg+InfoAddrLocation);
    pTxData=(TxMsg+AsduHeadLength);

    if(CotSize==2)//����ԭ��Ϊ2�ֽ�ʱ����λ�̶�Ϊ0��
        TxMsg[CotLocation+1]=0;
    if(InfoAddrSize==3)//��Ϣ���ַΪ3���ֽ�ʱ������ֽ�Ϊ0
        TxMsg[InfoAddrLocation+2]=0;

    if(DLCommand&DL_FCBOK)//��ƽ��ģʽ
    {
        DLCommand&=(~DL_FCBOK);

        //FCB������ƽ��ģʽ��COS��SOE��FA��ָ���ۼӡ�
        if ((LastFrame==BI)&&(BIFrame&BIETFRAME))
            ClearFlag(LastDevIndex,BIETFLAG);
        //if ((LastFrame==BI)&&(BIFrame&FAPROCFRAME))
            //ClearFlag(LastDevIndex,FAPROCFLAG);
        if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))
            ClearFlag(LastDevIndex,BIENTFLAG);
    }

    if(LengthIn==0)//�����
    {
        switch(DLCommand)
        {
            case DL_LINKDISABLE://��·��Ч
                //MasterStatus=NOUSE;
                APP_DLPriStatus = PRIWAITSTATUS;
                DLInitFinishFlag = FALSE;
                
                Data1.Flag &= (HaveCOS|HaveSOE);   //�����SOE��cos��־�����������
                Data2Flag=0;
                
                if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))//�ϴη�������Ϊ��λYX
                {
                    Data1.Flag|=(HaveCOS);
                    Data1.COT=SPONT;
                }
                if ((LastFrame==BI)&&(BIFrame&BIETFRAME))//�ϴη�������Ϊsoe
                {
                    Data1.Flag|=(HaveSOE);
                    Data1.COT=SPONT;
                }
                
                //ll ���ͨѶ�жϣ�����ң�ء�Ϊ���ݲ����޸� 2014-3-14
                if(YkStatusForTest)
                {
                    YkStatusForTest = 0;    
                    BspYkRelease();
                    logSysMsgNoTime("101ͨѶ�жϳ���ң��",0,0,0,0);
                }

                LastFrame=Polling;
                break;
            case DL_RESETDL://��·�յ��Է���λ��·����ж��Ƿ��г�ʼ������
                HaveJob=TRUE;
                
                APP_DLSecStatus = SECENABLE;
                YkStatusForTest = 0;    //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24
                
                RMTParaInit();
                
                if(BalanMode)//ƽ��ģʽ
                {
                    FirstCallAllData = 0;
                    WaitCallAllDelay = WAIT_CALLALL_DELAY;
                }
                else
                {
                    FirstCallAllData = 0xff;    //��ƽ��ģʽ��֧�ֵ�1�����в������
                    DLInitFinishFlag = TRUE;    //��ƽ��ģʽ�£���ʱ��·��ʼ����Ϊ�Ѿ���ɣ���λ��ʼ��������־
                    
                    if(InitFlag == 0)
                    {
                        if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))//�ϴη�������Ϊ��λYX
                            Data1.Flag|=(HaveCOS);
                        if ((LastFrame==BI)&&(BIFrame&BIETFRAME))//�ϴη�������Ϊsoe
                            Data1.Flag|=(HaveSOE);
                        LastFrame=Polling;
                    }
                    
                    Data1.Flag &= 0xf00;      //�����COS��SOE��INITEND��FA֮���һ�����ݣ��������ͨѶ���⡣
                    Data2Flag=0;            //����������ݱ�־
                    *LengthOut=0;
                }
                
                JudgeSendInitEnd();
                OnTimer();  //�жϳ�ʼ���Ƿ���ɡ�
                break;
            case DL_LINKENABLE://��·��Ч��ƽ��ģʽ
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
            case DL_SCAN1S://ƽ��ģʽ��
                
                HaveJob=TRUE;
                if(HaveWrongData)   //ll ���ӣ������쳣�ش�Ļش��ٶ�
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
            case DL_CALLUDATA://ȡ�������ݣ�ƽ��ģʽ
            case DL_CALLDBMSG://ȡ��Ϣ��ƽ��ģʽ
            case DL_CALLDATA1://ȡһ������ ��ƽ��ģʽ
                HaveJob=TRUE;
                //logSysMsgNoTime("SEC ���ݿ��������%x",Data1.Flag,0,0,0);//  debug ll
                ProcData1();
                break;
            case DL_CALLDATA2://ȡ��������
                HaveJob=TRUE;
                ProcData2();
                break;
            case DL_APPCON://ƽ��ģʽ�յ���03��ȷ��,COS,SOE,FA,ָ���ۼӡ�
                HaveJob=TRUE;
                if ((LastFrame==BI)&&(BIFrame&BIETFRAME))
                    ClearFlag(LastDevIndex,BIETFLAG);
                //if ((LastFrame==BI)&&(BIFrame&FAPROCFRAME))
                    //ClearFlag(LastDevIndex,FAPROCFLAG);
                if ((LastFrame==BI)&&(BIFrame&BIENTFRAME))
                    ClearFlag(LastDevIndex,BIENTFLAG);
                
                //logSysMsgNoTime("ȷ��֡��ѯ���ݿ�ʼ%x",Data1.Flag,0,0,0);   //  debug ll
                if(Data1.Flag & APP_DATA1_DEF)   //ll
                    ProcData1();
                else if(Data2Flag || (Data1.Flag& APP_DATA2_DEF))
                    ProcData2();
                else  
                    HaveJob=FALSE;
                //logSysMsgNoTime("ȷ��֡��ѯ���ݽ���%x",Data1.Flag,0,0,0);   //  debug ll
                break;
            default:
                HaveJob=FALSE;
                break;
        }
    }
    else//���ݴ���
    {
        //logSysMsgNoTime("Ӧ�ò�����Cot=%d, PubAddr=%x, TypeID=%x, len=%d",RxCot,RxPubAddr,RxTypeID,LengthIn); 
        RxTypeID=RxMsg[0];
        RxCot=RxMsg[2];
        pRxData=RxMsg+AsduHeadLength;
        RxVsq=RxMsg[1];
    
        if((RxCot&COT_REASON)>47)//����ԭ���д��󣬷���·ȷ�ϣ���¼��������
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
        if(!GetActDevIndexByAddr(RxPubAddr))//�������ַ���󣬿���Ϊ0xFF
        {
            //logSysMsgNoTime("�������ַ���� =%d",RxPubAddr,0,0,0); //debug
            if((RxTypeID!=C_IC_NA)&&(RxTypeID!=C_CI_NA)&&(RxTypeID!=C_CS_NA))//���ٻ������ٻ�����Ϊ0xFF
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
            case C_SC_NA://����ң������
            case C_DC_NA://˫��ң������
            case C_RC_NA://��������
                ProcControl();
                break;
            case C_SE_NA://�趨���ֻ֧�ֹ�һ����Ϊ�˲���
                  ProcSetNVA();
                break;
            case C_LC_CALL_YC_YX:          //���Ʒ���Һ���ٻ�ң��ң������
            case C_LC_CALL_SYSINFO_SOE:    //���Ʒ���Һ���ٻ�ϵͳ��Ϣ��SOE
            case C_LC_CALL_NAME_VER_CLOCK: //���Ʒ���Һ���ٻ�ң�����ơ�ң�����ơ��汾��Ϣ��ʱ����Ϣ
            //case C_LC_FAULT_RESET:         //Һ����λ��������
            case C_LC_SUMMON_PARA:
                
                LCFlag = 1;
                //LC_Flag = 1;
                LCAmount = 0;
                
                if(!(RxVsq & 0x80))
                {
                	for(i=0;i<2;i++)//InfoAddrSize���Ϊ3��Ҳֻȡǰ2���ֽڡ�
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
                	
                	Qoi = pRxData[2 * num];     //δ�޸�numǰ����Qoi
                    
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
            case C_IC_NA://���ٻ�������ٻ�
                ProcAllDataCall();
                break;
            case C_CI_NA://������ٻ�������ٻ�
            //	if((RxMsg[AsduHeadLength]&0xc0)==0)//wjr  ������û���Ƚ��ж�����ٻ���Ϊ����
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
                        //logSysMsgNoTime("SEC ���δ����%d",*AppCommand,0,0,0);//  debug ll
                        break;
                    }
                    
                }
                ProcDDCall();
                break;
            case C_CD_NA://��ʱ�������
                ProcTimeDelay();
                break;
            case C_CS_NA://��������
                
                /*�����޸ģ������·����Ϊ04����Ҫȷ���򲻽���ȷ�ϣ����Ϊ03�����ȷ��  wjr*/
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
             case C_LC_SET_PARA://Һ���趨��������
                
                
                if(DLCommand==DL_SENDNOCON)//   
                {
                    ProcLCSetPara(FALSE);
                    HaveJob=FALSE;
                }
                else
                    ProcLCSetPara(TRUE);
                break;
              case C_LC_ACTIVATE_PARA://Һ�������������
                
                
                if(DLCommand==DL_SENDNOCON)//   
                {
                    SystemReset(WARMRESET);
                    HaveJob=FALSE;
                }
                else
                    ProcActivatePara(TRUE);
                break;
            case C_RP_NA://��λ��������
                ProcReset();
                break;
            case C_RD_NA://����������
                //EnCodeReadData();
                ProcReadData();
                break;
            case C_TS_NA://��������
                ProcTest();
                break;
            
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
                break;*/
            //2016 �¹�Լ��չ    
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
            //end 2016 �¹�Լ��չ
            case F_FS_NA_N:    //�ļ�ͬ��  
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
            case C_PF_NA://������ֵ�趨
                SetProtect();
                break;
            case P_PF_NA://������ֵ�ٻ�
                CallProtect();
                break;
#endif

            default://���ͱ�ʶ�д����֧��
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
    }//end of else//���ݴ���

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
//һ�����ݴ������ȼ����Ż������ձ�׼Ҫ��
void CSecAppSev::ProcData1(void)
{
    BOOL rc;
    
    if(DLInitFinishFlag == FALSE)   //��ʼ��δ��ɽ�ֹӦ�ò�����
        return;
             
    if(HaveWrongData)
    {

        HaveWrongData=FALSE;
        memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
        *LengthOut=WrongDataLength;
        *AppCommand=APP_SENDDATA;
        if(BalanMode)                   //ll
            *AppCommand=APP_SENDCON;
        //����к���һ�����ݣ�������ACD����Ϊ1
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        //logSysMsgNoTime("���ʹ�������%d",WrongDataLength,0,0,0);   //  debug ll
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
    
    //�Ӷ��������½�����·���1�����У���Ӧ����ϣ�ll ���
    if((Data1.Flag&CallAllData) 
        && (APP_DATA1_DEF&CallAllData) 
        && (FirstCallAllData==0))//վ�ٻ�
    {
        //logSysMsgNoTime("SEC������Ӧ,FLAG=%x",FirstCallAllData,0,0,0);    //dubug ll
        if(EditAllDataCon==0xff)
        {
            EditAllDataCon=0;
            EnCodeAllDataConf();    //��������ȷ��
            
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
    
    if((FirstCallAllData == 0)&&(BalanMode))    //������տ�ʼ�������ӵ�1�����в��ܱ���ϵĴ���ʱ��Ӧ����ƽ��ģʽ�£���ƽ��ģʽ���ʺϡ�
    {
        return; //�ȴ���1�����н������ٷ�����������
    
    }
    
    if((Data1.Flag&HaveYK) && (APP_DATA1_DEF&HaveYK))
    {
        //logSysMsgNoTime("SEC ң�ر��ķ���",0,0,0,0);    //dubug ll
        Data1.Flag&=(~HaveYK);
        if(YKStatus==YKTERM)//ң�ؽ���
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
        //logSysMsgNoTime("SEC procdata1 COS������",0,0,0,0);    //dubug ll
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
                *AppCommand|=APP_HAVEDATA1;     //���ȥ��SOE���Ƿ���һ������
        }
        
        if(*LengthOut != 0)
            return;
        
        
    }
    if((Data1.Flag&CallAllData) && (APP_DATA1_DEF&CallAllData))//վ�ٻ�
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
    if(Data1.Flag&ProtectCon)//������ֵ����ȷ��
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

    if((Data1.Flag&CallSetNVA) && (APP_DATA1_DEF&CallSetNVA))//�趨���ֻ���һ��
    {
        Data1.Flag&=(~CallSetNVA);
        EnCodeSetNVA();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
    if((Data1.Flag&CallTest) && (APP_DATA1_DEF&CallTest)) //���Թ���
    {
        Data1.Flag&=(~CallTest);
        EnCodeTest();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    if((Data1.Flag&CallReset) && (APP_DATA1_DEF&CallReset))//��λ����
    {
        Data1.Flag&=(~CallReset);
        EnCodeReset();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    if((Data1.Flag&CallParaSet) && (APP_DATA1_DEF&CallParaSet))//�����趨
    {
        Data1.Flag&=(~CallParaSet);
        ParaSetCon();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }

    

    if((Data1.Flag&FreezeDD) && (APP_DATA1_DEF&FreezeDD))//������
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

    if((Data1.Flag&CallDD) && (APP_DATA1_DEF&CallDD))//���͵��
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
    //�仯ң�⣬������ݱ仯Ƶ����Ӱ���������ݴ��䣬���Է��ں��档
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
    
    //��һ������
    *LengthOut=0;
    *AppCommand=APP_NODATA;
}

//�������ݴ��������������ݡ�ѭ����������
void CSecAppSev::ProcData2(void)
{
    
#ifdef SFILETRANAPP101
    BOOL    rc = FALSE;
    INT16U  len;
#endif    
    if(DLInitFinishFlag == FALSE)   //��ʼ��δ��ɽ�ֹӦ�ò�����
        return;
    
    //ʱ�ӱ���
    if((Data1.Flag&CallClock) && (APP_DATA2_DEF&CallClock))
    {
        Data1.Flag&=(~CallClock);
        EnCodeClock();
        if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        return;
    }
    
    if((Data1.Flag&CallTest) && (APP_DATA2_DEF&CallTest)) //���Թ���
    {
        //logSysMsgNoTime("SEC 2�����ݲ���Ӧ��",0,0,0,0);   //  debug ll
        Data1.Flag&=(~CallTest);
        EnCodeTest();
        if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        return;
    }
        
    
    
    if((Data1.Flag&CallReset) && (APP_DATA2_DEF&CallReset))//��λ����
    {
        
        Data1.Flag&=(~CallReset);
        EnCodeReset();
        if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
        return;
    }
    //�㶫Զ����ά
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
    //�㶫Զ����ά-д����Ӧ��֡
    if((Data2Flag&DATA2_RMT_WRITEPARA_GD) && (APP_DATA2_DEF&DATA2_RMT_WRITEPARA_GD))
    {
        Data2Flag &= (~DATA2_RMT_WRITEPARA_GD);
        ProcEncodeRMTSetPara_GD();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;

        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ��������Ҫreturn
    }
    
    //����Զ����ά
    if((Data2Flag&DATA2_GX_READPARA) && (APP_DATA2_DEF&DATA2_GX_READPARA))
    {
        if(EditReadParaCon == 0xff)
        {
            EditReadParaCon=1;
            EnCodeGXReadParaConf();    //���Ͷ�����ȷ��
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
           
        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ��������Ҫreturn
    }
    
    if((Data2Flag&DATA2_GX_SETPARA) && (APP_DATA2_DEF&DATA2_GX_SETPARA))
    {
        ProcEncodeGXSetPara();
        
        if(Data1.Flag & APP_DATA1_DEF)
        {
            *AppCommand|=APP_HAVEDATA1;
        }
 
        Data2Flag &= (~DATA2_GX_SETPARA);    
        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ��������Ҫreturn
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
 
            
        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ��������Ҫreturn
    }
    //���ļ�Ŀ¼
    if((Data2Flag&DATA2_FT_DIR) && (APP_DATA2_DEF&DATA2_FT_DIR))//���ļ�Ŀ¼
    {
        if(ProcFT_EncodeReadDir())
        {
            if(Data1.Flag & APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
                
            //return;
        }
        else
            Data2Flag &= (~DATA2_FT_DIR);
        
        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ�
        
    }
    //���ļ�����ȷ��֡
    if((Data2Flag&DATA2_FT_FILEACT) && (APP_DATA2_DEF&DATA2_FT_FILEACT))//���ļ�Ŀ¼
    {
        Data2Flag &= (~DATA2_FT_FILEACT);
        ProcFT_EncodeFileActConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    //���ļ�����
    if((Data2Flag&DATA2_FT_FILEDATA) && (APP_DATA2_DEF&DATA2_FT_FILEDATA))//���ļ�Ŀ¼
    {
        if(ProcFT_EncodeFileData())
        {
            if(Data1.Flag & APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
                
            //return;
        }
        else
            Data2Flag &= (~DATA2_FT_FILEDATA);
            
        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ�    
    }
    
    //д�ļ�����ȷ��֡
    if((Data2Flag&DATA2_FT_WTFILEACT) && (APP_DATA2_DEF&DATA2_FT_WTFILEACT))//���ļ�Ŀ¼
    {
        Data2Flag &= (~DATA2_FT_WTFILEACT);
        ProcFT_EncodeWriteFileActConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    
    //�ļ��������ݽ���ȷ��֡
    if((Data2Flag&DATA2_FT_WTDATAACT) && (APP_DATA2_DEF&DATA2_FT_WTDATAACT))//���ļ�Ŀ¼
    {
        Data2Flag &= (~DATA2_FT_WTDATAACT);
        ProcFT_EncodeWriteFileDataConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    
    //�л���ֵ��ȷ��֡
    if((Data2Flag&DATA2_RMT_SETSEC) && (APP_DATA2_DEF&DATA2_RMT_SETSEC))//���ļ�Ŀ¼
    {
        Data2Flag &= (~DATA2_RMT_SETSEC);
        ProcEncodeSetSectionNo();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    
    //����ֵ��ȷ��֡
    if((Data2Flag&DATA2_RMT_READSEC) && (APP_DATA2_DEF&DATA2_RMT_READSEC))//���ļ�Ŀ¼
    {
        Data2Flag &= (~DATA2_RMT_READSEC);
        ProcEnCodeReadSectionNo();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
            
        return;
    }
    //������Ӧ��֡
    if((Data2Flag&DATA2_RMT_READPARA) && (APP_DATA2_DEF&DATA2_RMT_READPARA))//���ļ�Ŀ¼
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
           
        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ��������Ҫreturn
    }
    
    //д����Ӧ��֡
    if((Data2Flag&DATA2_RMT_SETPARA) && (APP_DATA2_DEF&DATA2_RMT_SETPARA))
    {
        Data2Flag &= (~DATA2_RMT_SETPARA);
        ProcEncodeRMTSetPara();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;

        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ��������Ҫreturn
    }
    
    
    //Զ������Ӧ��֡
    if((Data2Flag&DATA2_PUP_PROGUP) && (APP_DATA2_DEF&DATA2_PUP_PROGUP))//���ļ�Ŀ¼
    {
        Data2Flag &= (~DATA2_PUP_PROGUP);
        ProcEncodePUPupdateConf();
        
        if(Data1.Flag & APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;

        return;     //���ﷵ�ش����ǰ�治һ��������ʱע�⡣����false����true���б�����Ҫ�ظ��������Ҫreturn
    }
    
    //�����ļ�ͬ������ȷ����   CL  20180607
    if((Data2Flag&DATA2_XSFT_SYNACT) && (APP_DATA2_DEF&DATA2_XSFT_SYNACT))
    {
        Data2Flag &= (~DATA2_XSFT_SYNACT);
        ProcEncodeXSFileSynConf();   //�����ļ�ͬ������ȷ��Ӧ����
        
        if(Data1.Flag & APP_DATA1_DEF)   //���Ƚ���һ�����ݵķ���
            *AppCommand|=APP_HAVEDATA1;

        return;     
    }
   //�����ļ�ͬ������ȷ����   CL  20180607
   if((Data2Flag&DATA2_XSFT_SYNACTFINISH) && (APP_DATA2_DEF&DATA2_XSFT_SYNACTFINISH))
    {
        Data2Flag &= (~DATA2_XSFT_SYNACTFINISH);
        ProcEncodeXSFileSynFinish();   //�����ļ�ͬ����ֹӦ����
        
        if(Data1.Flag & APP_DATA1_DEF)   //���Ƚ���һ�����ݵķ���
            *AppCommand|=APP_HAVEDATA1;

        return;    
    }
   //�����ļ�ͬ������ֹͣȷ���� CL 20180612   
    
    //�仯ң�⣬������ݱ仯Ƶ����Ӱ���������ݴ��䣬���Է��ں��档
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
    if(Data2Flag&ProtectData)//������ֵ����
    {
        Data2Flag&=(~ProtectData);
        SendProtectData();
        //if(Data1.Flag!=0)//
        if(Data1.Flag&APP_DATA1_DEF)
            *AppCommand|=APP_HAVEDATA1;
        return;
    }
#endif

    if((Data2Flag&BackData)||(Data2Flag&PerCycData))//�ñ�־�ڶ�ʱ�������ã����������ݺ����
    {
        if(ProcAllData())
        {
            //if(Data1.Flag!=0)
            if(Data1.Flag&APP_DATA1_DEF)
                *AppCommand|=APP_HAVEDATA1;
            return;
        }
    }

    if(Data2Flag&UpLoadFile)//�ļ����������ձ�׼Ҫ��Ӧ���������ݼ��𻹵͡�
    {
        //������
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

    //�ٻ���������ʱ���޶������ݿ�����һ������Ӧ�𡣶�û����������Ӧ��
    ProcData1();
}

//OK
void CSecAppSev::ProcControl(void)  //����ң�أ�ң�ز�����򹫹����ַ��������Ϣ��
{
    INT8U sco,dco,OnOff;
    INT16U InfoAddr,SwitchNo;
    //INT16U bonum;
    
    InfoAddr=RxInfoAddr;
    SwitchNo = InfoAddr-LBO+1;
    SwitchNoTemp = SwitchNo;
    YKTypeID=RxTypeID;
    
    //LogYkInfoRec(DevList[ActDevIndex].DevID, RxTypeID, *pRxData, RxInfoAddr, RxCot);  //��¼��վ���͵�����ң����Ϣ
    
    
    if (GetActDevIndexByAddr(RxPubAddr))//���ݹ������ַ���豸��š�
    {
        BODevIndex=ActDevIndex;
        if((SwitchNoTemp-1)*2==DevList[ActDevIndex].DevData.BONum)                     //����ά��
        
    
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
            if(((*pRxData)&DCO_SE)==0)  //ִ��
            {
                if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                    startCellMaint();
            }
            return;
        }
        else if((SwitchNoTemp-1)*2==DevList[ActDevIndex].DevData.BONum+2)    //���鼶���豸
        {
            YKStatus=YKSETCON;
            Data1.Flag|=HaveYK;
            *LengthOut=0;
            *AppCommand=APP_APPCON;
            *AppCommand|=APP_HAVEDATA1;
            DcoTemp=*pRxData;
            ScoTemp=*pRxData;
            if(((*pRxData)&DCO_SE)==0)  //ִ��
            {
                if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                { 
                    ResetFaultInfoForCall_FaultCheck();
                    logSysMsgNoTime("���鼶���豸",0,0,0,0);
                }
            }
              return;
        }
    }
    switch (RxTypeID)
    {
        case C_SC_NA:    //����ң������
            ScoTemp = sco = *pRxData;

            if ((sco & SCO_SCS) == 0)       //��
                OnOff = 2;
            else if ((sco & SCO_SCS) == 1)  //��
                OnOff = 1;

            if(Sec101Pad.ControlPermit == 0)//��������Ϊ������ң��
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
            if (GetActDevIndexByAddr(RxPubAddr))//���ݹ������ַ���豸��š�
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
            if ((RxCot&COT_REASON)==ACT)//6������
            {
                if ((sco & SCO_SE) == SCO_SE)   //1��select
                {
                    
                    YKStatus=YKSETCON;
                    if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                        SetYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                }
                else//0��ִ��
                {
                    if(YkStatusForTest ==0) //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24
                    {
                        return;
                    }
                    YkStatusForTest =  0;    //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24   
                    
                    YKStatus=YKEXECON;
                    if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                        ExecuteYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    
                }
            }
            else//����
            {
                if ((RxCot&COT_REASON)==DEACT)
                {
                	YKStatus=YKCANCELCON;
                	if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
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
        case C_DC_NA:   //˫��ң������
        case C_RC_NA:    //������
            dco = *pRxData;
            DcoTemp = dco;

            if ((dco&DCO_DCS)==1)        //��
                OnOff = 2;
            else if ((dco&DCO_DCS)==2)  //��
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
            if (GetActDevIndexByAddr(RxPubAddr))//���ݹ������ַ���豸��š�
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
            if ((RxCot&COT_REASON)==ACT)//6������
            {
                if ((dco&DCO_SE) == DCO_SE)   //1��select
                {
                    
                    YKStatus=YKSETCON;
                    if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                        SetYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    if((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2))//�������ϸ��飬���һ��ң�صĺ�
                    {
                        YKSetAlready = TRUE;
                    }
                }
                else//0��ִ��
                {
                    
                    if(YkStatusForTest ==0)    //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24 
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
                    YkStatusForTest =  0;    //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24 
                    
                    YKStatus=YKEXECON;
                    if((YKSetAlready == FALSE)&&((OnOff == 1) && (SwitchNo == DevList[BODevIndex].DevData.BONum/2)))//�������ϸ��飬���һ��ң�صĺ�
                    {
                        RFaultFlag=0xff;
                        SetYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    }
                    else
                    {
                        if(YKSetAlready == TRUE)
                            YKSetAlready = FALSE;
                        if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
                            ExecuteYK(wAppID,DevList[BODevIndex].DevID,SwitchNo,OnOff);
                    }
                }
            }
            else//����
            {
                if ((RxCot&COT_REASON)==DEACT)
                {
                    YKStatus=YKCANCELCON;
                    if((RxCot&COT_TEST)==0)//�������������λΪ1����ʵ�ʲ���ң�أ�
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

    Data2Flag&=(~BackData);//Data2Flag=0;//����������ݱ�־

    GroupTrn.TypeID=C_IC_NA;
    GroupTrn.COT=RxMsg[AsduHeadLength];//���ٻ��޶��� ����ԭ��,
    GroupTrn.PubAddr=RxPubAddr;
    GroupTrn.HaveSendDBI = FALSE;

    if (GroupTrn.PubAddr==BroadCastAddr)//�㲥���ٻ������豸����
    {
        GroupTrn.DevIndex=0;//���ٻ����ݴ��豸���0��ʼ
    }
    else//�ٻ�ĳһ�豸����
    {
        if (GetActDevIndexByAddr(GroupTrn.PubAddr))//���ݹ������ַ���豸��š�
        {
            GroupTrn.DevIndex=ActDevIndex;
        }
        else//û�ж�Ӧ�ù������ַ���豸
        {
            
            GroupTrn.DevIndex=0;//���ٻ����ݴ��豸���0��ʼ
        }
    }
    
    if(GroupTrn.COT == INTRO16)    //��16 �������� �����޸�
    {
        logSysMsgWithTime("101�յ���16����",0,0,0,0);
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
        GroupTrn.GroupNo=1;//��Ŵ�1��ʼ
        GroupTrn.InfoAddr=LBIinfoaddr;//��Ϣ���ַ��0x1��ʼ  
    }
    else
    {
        GroupTrn.GroupNo=RxMsg[AsduHeadLength]-INTROGEN;
        GroupTrn.InfoAddr=LBIinfoaddr;//��Ϣ���ַ��0x1��ʼ������ֵ��У���� 
    }
    //logSysMsgNoTime("����no=%d",GroupTrn.GroupNo,0,0,0);   //  debug ll
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
    
    if((RxMsg[AsduHeadLength]&0x3f)==5)//������ٻ�
    {
        GroupTrnDD.COT=REQCOGCN;
    }
    else//�����ٻ�
    {
        GroupTrnDD.COT=REQCOGCN+((RxMsg[AsduHeadLength])&0x3f);
    }

    if(RxMsg[AsduHeadLength]&0xc0)//������
    {
        Data1.Flag|=FreezeDD;
        EditDDCon=0xff;
        FreezeCounter();
        DDFreeze=TRUE;
    }
    else//�����
    {
        Data1.Flag|=CallDD;
        EditDDCon=0xff;
    }

    if (GroupTrnDD.PubAddr==BroadCastAddr)//�㲥���ٻ������豸����
    {
        GroupTrnDD.DevIndex=0;//���ٻ����ݴ��豸���0��ʼ
    }
    else//�ٻ�ĳһ�豸����
    {
        if (GetActDevIndexByAddr(GroupTrnDD.PubAddr))//���ݹ������ַ���豸��š�
        {
            GroupTrnDD.DevIndex=ActDevIndex;
        }
        else//û�ж�Ӧ�ù������ַ���豸
        {
            GroupTrnDD.DevIndex=0;//���ٻ����ݴ��豸���0��ʼ
        }
    }

    if(GroupTrnDD.COT==REQCOGCN)//������ٻ�
    {
        GroupTrnDD.GroupNo=1;//��Ŵ�1��ʼ
        GroupTrnDD.InfoAddr=LBCR;//��Ϣ���ַ�ӿ�ʼ
    }
    else
    {
        GroupTrnDD.GroupNo=GroupTrnDD.COT-REQCOGCN;
        GroupTrnDD.InfoAddr=LBCR;//��Ϣ���ַ�ӿ�ʼ������ֵ��У����
    }
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
    //logSysMsgNoTime("DDCall ��=%d, DDCon=%x,flag=%x",GroupTrnDD.GroupNo,EditDDCon,Data1.Flag,0);   //  debug ll
}
//OK
void CSecAppSev::ProcTimeDelay(void)
{
    if ((RxCot&COT_REASON)==ACT)//6������
    {
        if (GetSysTime((void*)(&SecSysTimeR),ABSTIME))
        {
            ReadTimeFlag=0xff;
        }
        else
            ReadTimeFlag=0;
        SDTTime=MAKEWORD(pRxData[0],pRxData[1]);

        Data1.Flag|=CallTimeDelay;//Ϊ������һ�����ݱ�־
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        *AppCommand|=APP_HAVEDATA1;
    }
    else if ((RxCot&COT_REASON)==SPONT)//3�������ӳ�ʱ��
    {
        TimeDelay=MAKEWORD(pRxData[0],pRxData[1]);
        *LengthOut=0;
        *AppCommand=APP_APPCON;
    }
}
void CSecAppSev::ProcSummonLightStatus()
{
	Data1.Flag|=CallLightStatus;//Ϊ������һ�����ݱ�־
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    if(APP_DATA1_DEF & CallLightStatus)   //ll
        *AppCommand|=APP_HAVEDATA1;
}
void CSecAppSev::ProcSummonInfoOnBoot()
{
	Data1.Flag|=SummonInfoOnBoot;//Ϊ������һ�����ݱ�־
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    if(APP_DATA1_DEF & SummonInfoOnBoot)   //ll
        *AppCommand|=APP_HAVEDATA1;
}
//OK
void CSecAppSev::ProcClock(BOOL Conf) //�������
{
    struct Iec101ClockTime_t Time;
    struct AbsTime_t AbsTime;
    TimeRightFlag = FALSE;
    GetSysTime((void*)(&OldSysTime),ABSTIME);//��¼����ǰ��ϵͳʱ��
    
    if((pRxData[6]==0) && (pRxData[5]==0) && (pRxData[4]==0))  //�ж�������Ϊ0
    {
        //��ʱ��
        Data1.Flag|=CallClock;//Ϊ������һ�����ݱ�־
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
            if (RxPubAddr!=0xFF) //�ǹ㲥���ӣ�Ҫ���ӳ�ʱ��
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
                //SetSysTime(&AbsTime,ABSTIME);//������������á�
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
            else//�㲥����
            {
                if(Sec101Pad.UseStandClock == 1)
                    SetSysTime(&Time,IEC101CLOCKTIME);
                else
                    SetSysTime(&Time,IEC101EXTCLOCKTIME);
            }
        }
    }
    if (Conf)                   //��Ҫȷ��
    {
        Data1.Flag|=CallClock;//Ϊ������һ�����ݱ�־
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
void CSecAppSev::ProcLCSetPara(BOOL Conf) //����Һ���趨��������
{
	INT16U Addr,ParaLength;
    int i;
    INT8U *pData,Bound;
	INT8U *pParaBuf;
    Bound=RxVsq&VSQ_NUM;//��ϢԪ����Ŀ
    Addr=RxInfoAddr; //��ʼ��Ϣ���ַ
    pData=pRxData;
    paramirrorbuf[0] = *(pRxData-2);                       //paramirrorbuf���洫�����Ĳ�����������Ϊ�����ͻ�ȥ��
    paramirrorbuf[1] = *(pRxData-1);
    pParaBuf = &paramirrorbuf[2];
    totallength_m = 0;
    filenum = Bound;
    for(i=0;i<Bound;i++)
    {
        ParaLength=pData[0];
        totallength_m += ParaLength + 2;
        memcpy((void*)pParaBuf,pData,ParaLength + 2); //��2����Ϊ�����˳��Ⱥ�Qoi���ֽ�
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
        Addr=MAKEWORD(pData[0],pData[1]);//��һ����ϢԪ�ص���Ϣ���ַ����Ϣ���ַ��Чλֻ��2���ֽ�
        pData+=InfoAddrSize;//ָ����������ϢԪ�����ݣ������Ϣ���ַΪ3�ֽڣ�Խ�����ֽڡ�  
        pParaBuf += InfoAddrSize;  
        totallength_m += InfoAddrSize;
    }
    if (Conf)                   //��Ҫȷ��
    {
        Data1.Flag|=LCSetPara;//Ϊ������һ�����ݱ�־
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
void CSecAppSev::ProcActivatePara(BOOL Conf) //����Һ���趨��������
{    
    if (Conf)                   //��Ҫȷ��
    {
        Data1.Flag|=LCActivatePara;//Ϊ������һ�����ݱ�־
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
    //logSysMsgNoTime("proctest������=%d",*AppCommand,0,0,0);   //  debug ll    
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
//�����趨˵������Ϣ���ַ��0X5001��ʼ��ÿ��ң��ռ��3����Ϣ���ַ
//ÿ��ң�������˳���ǣ�����ֵ�����ޡ����ޡ�
//�����趨ʱ����Ϣ���ַ/3Ϊң����ţ��޶���ȷ�����ޡ����ޡ����ޡ�
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
    No=(ParaInfoAddr-LPARA)/3;        //wjr  2009.4.5  �˴�Ӧ��ȡ��Ϣ���ַ�����ǹ������ַ
    if(No>=DevList[ActDevIndex].DevData.AINum)
    {
        *LengthOut=0;
        *AppCommand=APP_APPCON;
        return;
    }

    switch(RxTypeID)
    {
        case P_ME_NA:    //��һ��
            ParaWord=MAKEWORD(pRxData[0],pRxData[1]);
            ParaQPM=RxMsg[AsduHeadLength+2];
            temp=(long)ParaWord*(long)DevList[ActDevIndex].DevData.AIMaxVal[No]/0x3FFF;
            switch(ParaQPM&0x3F)
            {
                case 1://����
                    
                    DevList[ActDevIndex].DevData.AIPara[No].DeadValue=(short)temp;
                    break;
                case 3://����
                    DevList[ActDevIndex].DevData.AIPara[No].LowLimit=(short)temp;
                    break;
                case 4://����
                    DevList[ActDevIndex].DevData.AIPara[No].UpLimit=(short)temp;
                    break;
            }
            break;
        case P_ME_NB:    //��Ȼ�
            ParaWord=MAKEWORD(pRxData[0],pRxData[1]);  
            ParaQPM=RxMsg[AsduHeadLength+2];
            switch(ParaQPM&0x3F)
            {
                case 1://����
                    DevList[ActDevIndex].DevData.AIPara[No].DeadValue=ParaWord;
                    break;
                case 3://����
                    DevList[ActDevIndex].DevData.AIPara[No].LowLimit=ParaWord;
                    break;
                case 4://����
                    DevList[ActDevIndex].DevData.AIPara[No].UpLimit=ParaWord;
                    break;
            }
            break;
        case P_ME_NC:    //�̸�����
            //ParaFloat=*((float*)(pRxData));
            INT32U dd;
            dd = MAKEDWORD(MAKEWORD(pRxData[0],pRxData[1]),MAKEWORD(pRxData[2],pRxData[3]));    //ll
            ParaFloat =*((float *)(&dd));
            ParaQPM=RxMsg[AsduHeadLength+4];
            
            
            switch(ParaQPM&0x3F)
            {
                case 1://����
                    DevList[ActDevIndex].DevData.AIPara[No].DeadValue=(short)ParaFloat;
                    
                    break;
                case 3://����
                    DevList[ActDevIndex].DevData.AIPara[No].LowLimit=(short)ParaFloat;
                    break;
                case 4://����
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

    //��ʼ��ԭ��
    *pTxData=0;
    
    if(*pRestType == 0xee)  //��λ��Լ����״̬���� ll 2010/07/20   for ������Լ����
    {
        *pTxData = 2;
        *pRestType = 0;   
    }
    
    *AppCommand=APP_SENDDATA;
    *LengthOut=AsduHeadLength+1;
    if(BalanMode)
    {    
        *AppCommand=APP_SENDCON;
        //MasterStatus=INUSE;     //ƽ��ģʽ�£�ֻҪ�����˳�ʼ������֡����Ϊ��·�ǿ��õ�  wjr 2009.6.3
    }   
}
/*------------------------------------------------------------------/
�������ƣ�  RebootCheckUDataFlag()
�������ܣ�  �������Ƿ���Ҫ����SOE
����˵����   
���˵����  
��ע��     
/------------------------------------------------------------------*/
void CSecAppSev::RebootCheckUDataFlag(void)
{
    if(GYKZ2015Flag)    //����Ҫ���ϵ��建��
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
�������ƣ�  SetUDataFlag()
�������ܣ�  ����Ƿ���SOE/COS��Ҫ���ͣ�������λ����SOE/COS��־
����˵����   
���˵����  
��ע��      test_flag��̽���Ƿ�����Ҫ���͵�soe
/------------------------------------------------------------------*/
void CSecAppSev::SetUDataFlag(void)
{
    BOOL rc;

    //��ѯ����ѹ��״̬������ѹ��Ͷ���򲻷���soe��cos
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

//��������豸����COS��SOE
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
void CSecAppSev::ProcTaskMsg(void)//����ң�ط�У��Ϣ
{
    INT32U rc;
    INT8U jj;
    BOOL Stop=FALSE;
    //INT16U bonum;
    
    /*if(MyConfig.type ==DEVTYPE_DF9311A1)
        bonum = 8;
    else
        bonum = DevList[BODevIndex].DevData.BONum;*/
    if ( ((SwitchNoTemp-1)*2==DevList[BODevIndex].DevData.BONum) || ((SwitchNoTemp - 1) * 2 == (DevList[BODevIndex].DevData.BONum + 2)))    //����ά���͸��鼶���豸
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
BOOL CSecAppSev::EnCodeCtrlRet(void)  //ң�ط�У
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
            case 1://1-ң��Ԥ�óɹ�
                Cmd = SELECT;
                YkStatusForTest = 1;    //ll Ϊ���ݲ�����ʱ�޸� 2012-3-24
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
        if((RetNo == 1)&&(RFaultFlag==0xff)&&(OnOff == 1)&&(SwitchNo==DevList[BODevIndex].DevData.BONum/2))//�������ϸ���ִ��
        {
            Data1.Flag&=(~HaveYK);
            RFaultFlag=0;
            ExecuteYK(wAppID,DeviceID,SwitchNo,OnOff);
            *LengthOut=0;
            *AppCommand=APP_NODATA;
            return  FALSE;
        }

        if((RetNo == 5)||(RetNo == 6))//����
        {
            TxMsg[CotLocation]=DEACTCON;
            YKStatus=YKTERM;
        }
        else
        {
            TxMsg[CotLocation]=ACTCON;
        }

        if(YKTypeID==C_SC_NA)
            sco |= SCO_SE;//0x80 Ԥ�ñ�־λ
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

    if ((RetNo & 1) == 0)//2,4,6 ʧ��
    {
        if(GetYKRYBState() == TRUE)
            TxMsg[CotLocation]|=0x40;
        else
        {
            TxMsg[CotLocation] = COT_YKRYBERR;    //����ң����ѹ����� ll 
            TxMsg[CotLocation] |= COT_PONO;
        }
        
    }
    else//1,3,5�ɹ�
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
void CSecAppSev::EditYKTerm(void)//ң�ؽ���
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
        if (DevList[i].Addr==Addr)//������ַ��Ϊ�������ַ��
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
    //logSysMsgNoTime("SEC �༭˫��COS��ʼ",0,0,0,0);//  debug ll
    TxMsg[0]=M_DP_NA;   //3������ʱ���˫����Ϣ
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
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�

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
            pTxData[FramePos]   = LOBYTE(p->No+LDBIinfoaddr);//��Ϣ���ַ
            pTxData[FramePos+1] = HIBYTE(p->No+LDBIinfoaddr);//��Ϣ���ַ
            if(InfoAddrSize == 3)
                pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
        }
        FramePos+=InfoAddrSize;;
        
        //logSysMsgNoTime("p->Status=%x",p->Status,0,0,0);   //  debug ll
        if((p->Status&BIACTIVEFLAG)==0)
            pTxData[FramePos]=Status1|Status0|P101_IV;//����ң��״̬�ֽ�
        else
            pTxData[FramePos]=Status1|Status0;//����ң��״̬�ֽ�
            
        if(p->Status&SUBSTITUTEDFLAG)
            pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
        
        
        FramePos++;        
        SendNum++;//���͸���
        p++;
        if(FramePos>=Length)
            break;
    }
    
    if(SendNum>0)
    {
        Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
        //LastFrame=BI;
        //LastDevIndex=DBICOSDevIndex;
        //BIFrame|=BIENTFRAME;
        //DevList[DBICOSDevIndex].Data1.BIENTNum=SendNum;      //��λң�ŷ��͸���
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
    //logSysMsgNoTime("SEC �༭˫��COS����",0,0,0,0);//  debug ll  
    
}

BOOL CSecAppSev::EnCodeBIENT(void)  //�༭COS
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithoutTimeData_t *p;
    BOOL HaveData=FALSE;    //�Ƿ��к������� TRUE ��
 
       

    *LengthOut = 0;
    for(i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIENTFLAG))
            continue;
        //logSysMsgNoTime("SEC EnCodeBIENT�м�1̬",0,0,0,0);//  debug ll
        TxMsg[0]=M_SP_NA;//1������ʱ��ĵ�����Ϣ
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
        //logSysMsgNoTime("SEC EnCodeBIENT�м�2̬%d",Num,0,0,0);//  debug ll
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;
            p=(struct BIEWithoutTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
            
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
                            pTxData[FramePos]   = LOBYTE(p->No-DevList[i].DevData.DBINum+LBIinfoaddr);//��Ϣ���ַ
                            pTxData[FramePos+1] = HIBYTE(p->No-DevList[i].DevData.DBINum+LBIinfoaddr);//��Ϣ���ַ
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
                        }
                        FramePos+=InfoAddrSize;;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                        else
                            pTxData[FramePos]=Status;//����ң��״̬�ֽ�
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                        
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
        
                        if(FramePos < 0)
                        {
                            *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                            *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                        }
                        else
                        {
                            pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                            pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
                        }
                        FramePos+=InfoAddrSize;;
        
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                        else
                            pTxData[FramePos]=Status;//����ң��״̬�ֽ�
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                        
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
            //logSysMsgNoTime("SEC EnCodeBIENT�м�3̬%d dbi=%d",SendNum,DBICOSnum,0,0);//  debug ll
            if(SendNum>0)
            {
                Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
                LastFrame=BI;
                LastDevIndex=i;
                BIFrame|=BIENTFRAME;
                DevList[i].Data1.BIENTNum=j;      //��λң�ŷ��͸���
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
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
                    }
                }
                else
                {
                    if((WritePtr == DevList[i].pDbaseWin->BIENTimRP+j))
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveCOS);
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
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
                    DevList[i].Data1.BIENTNum=j;      //��λң�ŷ��͸���
                    
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
                    	DevList[i].Data1.BIENTNum=j;      //��λң�ŷ��͸���
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

BOOL CSecAppSev::EnCodeBIENT_ALLDBI(void)  //�༭COS,ȫ�������˫��ң��
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithoutTimeData_t *p;
    BOOL HaveData=FALSE;    //�Ƿ��к������� TRUE ��
 
    *LengthOut = 0;
    for(i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIENTFLAG))
            continue;
        //logSysMsgNoTime("SEC EnCodeBIENT�м�1̬",0,0,0,0);//  debug ll
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
        //logSysMsgNoTime("SEC EnCodeBIENT�м�2̬%d",Num,0,0,0);//  debug ll
        if (Num>0)
        {
            SendNum=0;
            FramePos=0-InfoAddrSize;    //��ΪpTxDataָ����Ϣ��֮�����Ե�һ����Ϣ���ַû���Ӧ����TxMsg������pTxData�������Ͳ����������������� ll
            p=(struct BIEWithoutTimeData_t *)DBData;
            Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
            
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
                    pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                    pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                    if(InfoAddrSize == 3)
                        pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
                }
                FramePos+=InfoAddrSize;

                if((p->Status&BIACTIVEFLAG)==0)
                    pTxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                else
                    pTxData[FramePos]=Status;//����ң��״̬�ֽ�
                    
                if(p->Status&SUBSTITUTEDFLAG)
                    pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                
                FramePos++;
                SendNum++;//���͸���
                p++;
                j++;
                if(FramePos>=Length)
                    break;
            }
           
            //logSysMsgNoTime("SEC EnCodeBIENT�м�3̬%d dbi=%d",SendNum,DBICOSnum,0,0);//  debug ll
            if(SendNum>0)
            {
                Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
                LastFrame=BI;
                LastDevIndex=i;
                BIFrame|=BIENTFRAME;
                DevList[i].Data1.BIENTNum=j;      //��λң�ŷ��͸���
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
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
                    }
                }
                else
                {
                    if((WritePtr == DevList[i].pDbaseWin->BIENTimRP+j))
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveCOS);
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
                    }
                }
                
                
            }
            else
            {
            	
                *LengthOut = 0;
            	if(j>0)
                {
                	DevList[i].Data1.BIENTNum=j;      //��λң�ŷ��͸���
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
    
    TxMsg[0]=M_DP_TA;   //��ʱ���˫����Ϣ
    if(Sec101Pad.SOEWithCP56Time == 1)
        TxMsg[0]=M_DP_TB;   //����ʱ���˫����Ϣ
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
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�

    for(j=0;j<DBISOEnum;j++)
    {
      
        Status0 = 0;
        Status1 = (p->Status&0x60)>>5;
        
         //д��Ϣ���ַ
        if(FramePos < 0)
        {
            *pTxInfoAddr    =LOBYTE((p->No+LDBIinfoaddr));
            *(pTxInfoAddr+1)=HIBYTE((p->No+LDBIinfoaddr));
        }
        else
        {
             pTxData[FramePos]   = LOBYTE(p->No+LDBIinfoaddr);//��Ϣ���ַ
             pTxData[FramePos+1] = HIBYTE(p->No+LDBIinfoaddr);//��Ϣ���ַ
             if(InfoAddrSize == 3)
                 pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
         }
         FramePos+=InfoAddrSize;;
         //д״̬
         if((p->Status&BIACTIVEFLAG)==0)
             pTxData[FramePos]=Status1|Status0|P101_IV;//����ң��״̬�ֽ�
         else
             pTxData[FramePos]=Status1|Status0;//����ң��״̬�ֽ�
        
         if(p->Status&SUBSTITUTEDFLAG)
             pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
             
         FramePos++;
            //дʱ��
         AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

         pTxData[FramePos++] = LOBYTE(time.MSecond);
         pTxData[FramePos++] = HIBYTE(time.MSecond);
         pTxData[FramePos++] = time.Minute;

         if(TxMsg[0]==M_DP_TB)//��ʱ��
         {
             pTxData[FramePos++] = time.Hour;
             pTxData[FramePos++] = time.Day;
             pTxData[FramePos++] = time.Month;
             pTxData[FramePos++] = time.Year;
         }

         SendNum++;//���͸���
         p++;
         
         if(FramePos>=Length)
             break;
     }
     if(SendNum>0)
     {
         Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
         //LastDevIndex=DBIDevIndex;
         //LastFrame=BI;
         //BIFrame|=BIETFRAME;
         //DevList[DBIDevIndex].Data1.SOENum=SendNum;   //���ﲻ�ٱ�ǣ���Ϊǰ�浥��ң���Ѿ���ǹ���
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
�������ƣ�  EnCodeSOE()
�������ܣ�  �༭SOE���ġ���1�������е���
����˵����   
���˵����  TRUE ��ʾ�к�������  FALSE �޺�������  ll�޸� 2017-8-8
��ע��      ���Ͳ��ԣ��ȷ��͵���SOE���ٷ���DBIsoe��
            ������SOE����20ʱ����Ҫ��֡���ͣ������е���SOE������ɺ��ٷ���DBIsoe��
/------------------------------------------------------------------*/
BOOL CSecAppSev::EnCodeSOE(void) //�༭SOE
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    BOOL HaveData=FALSE;        //�����Ƿ�������
    
    *LengthOut = 0;
    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIETFLAG))
            continue;
        
        //logSysMsgNoTime("SEC �༭����SOE��ʼ",0,0,0,0);//  debug ll
        TxMsg[0]=M_SP_TA;   //��ʱ��ĵ�����Ϣ
        if(Sec101Pad.SOEWithCP56Time == 1)
            TxMsg[0]=M_SP_TB;   //����ʱ��ĵ�����Ϣ
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
            Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
            
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
                        //״̬ת�������ݿ��е�ң��D7Ϊ״̬����Լ��D0Ϊ״̬
                        if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
                        //д��Ϣ���ַ
                        if(FramePos < 0)
                        {
                            *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr-DevList[i].DevData.DBINum));
                            *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr-DevList[i].DevData.DBINum));
                        }
                        else
                        {
                            pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr-DevList[i].DevData.DBINum);//��Ϣ���ַ
                            pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr-DevList[i].DevData.DBINum);//��Ϣ���ַ
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
                        }
                        FramePos+=InfoAddrSize;;
                        //д״̬
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                        else
                            pTxData[FramePos]=Status;//����ң��״̬�ֽ�
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                        
                            
                        FramePos++;
                    
                        //дʱ��
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
                    
                        pTxData[FramePos++] = LOBYTE(time.MSecond);
                        pTxData[FramePos++] = HIBYTE(time.MSecond);
                        pTxData[FramePos++] = time.Minute;
                    
                        if(TxMsg[0]==M_SP_TB)//��ʱ��
                        {
                            pTxData[FramePos++] = time.Hour;
                            pTxData[FramePos++] = time.Day;
                            pTxData[FramePos++] = time.Month;
                            pTxData[FramePos++] = time.Year;
                        }
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
                        //״̬ת�������ݿ��е�ң��D7Ϊ״̬����Լ��D0Ϊ״̬
                        if(p->Status&0x80)
                            Status=1;
                        else
                            Status=0;
                        
                        //д��Ϣ���ַ
                        if(FramePos < 0)
                        {
                            *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                            *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                        }
                        else
                        {
                            pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                            pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                            if(InfoAddrSize == 3)
                                pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
                        }
                        FramePos += InfoAddrSize;
                        
                        //д״̬
                        if((p->Status&BIACTIVEFLAG)==0)
                            pTxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                        else
                            pTxData[FramePos]=Status;//����ң��״̬�ֽ�
                            
                        if(p->Status&SUBSTITUTEDFLAG)
                            pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                        
                            
                        FramePos++;
        
                        //дʱ��
                        AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
        
                        pTxData[FramePos++] = LOBYTE(time.MSecond);
                        pTxData[FramePos++] = HIBYTE(time.MSecond);
                        pTxData[FramePos++] = time.Minute;
        
                        if(TxMsg[0]==M_SP_TB)//��ʱ��
                        {
                            pTxData[FramePos++] = time.Hour;
                            pTxData[FramePos++] = time.Day;
                            pTxData[FramePos++] = time.Month;
                            pTxData[FramePos++] = time.Year;
                        }
        
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
                
                Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
                LastDevIndex=i;
                LastFrame=BI;
                BIFrame|=BIETFRAME;
                DevList[i].Data1.SOENum = j;       //DevList[i].Data1.SOENum �Ǳ���Ѿ����͵ĸ���(�����Ѿ�����ĸ����������ڶ�SOE��ָ����п���
                *pTxVSQ=SendNum;

                *LengthOut = Len;      
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;

                //�ж��Ƿ��к�������Ҫ����
                if(DevList[i].Flag)
                {
                    if(WritePtr != DevList[i].RealWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
                    }
                }
                else
                {
                    if(WritePtr != DevList[i].pDbaseWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
                    }
                }
                
                if(HaveData == FALSE)
                {
                    //����ң�Ŷ��������ˣ��ٴ���˫��ң�š���ʱ���˫��ң�ų������⣬�����Ͳ�����վ
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
                    
                    //�ж��Ƿ��к�������Ҫ����
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
                    //�ߵ����������쳣���
                    *LengthOut = 0;     //��ʾû��������Ҫ����
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
        //�ȷ��͵��㣬���е��㶼�������ˣ��ٷ���DBI��ȥ��
        EnCodeDBISOE(); 
        
        if(DBISOEnum > 0)
            return TRUE;   
    } 
       
    return(HaveData);
}

/*------------------------------------------------------------------/
�������ƣ�  EnCodeSOE_ALLDBI()
�������ܣ�  �༭SOE���ġ���1�������е���
����˵����   
���˵����  TRUE ��ʾ�к�������  FALSE �޺�������  ll�޸� 2017-8-8
��ע��      ���Ͳ��ԣ��ȷ��͵���SOE���ٷ���DBIsoe��
            ������SOE����20ʱ����Ҫ��֡���ͣ������е���SOE������ɺ��ٷ���DBIsoe��
/------------------------------------------------------------------*/
BOOL CSecAppSev::EnCodeSOE_ALLDBI(void) //�༭SOE
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    BOOL HaveData=FALSE;        //�����Ƿ�������
    
    *LengthOut = 0;
    for (i=0;i<DevCount;i++)
    {
        if (!test_flag(DevList[i].DevID,BIETFLAG))
            continue;
        
        //logSysMsgNoTime("SEC �༭����SOE��ʼ",0,0,0,0);//  debug ll
        TxMsg[0]=M_DP_TB;   //��ʱ��ĵ�����Ϣ
        
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
            Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
            
            j=0;
            while(j<Num)      
            {
                //״̬ת�������ݿ��е�ң��D7Ϊ״̬����Լ��D0Ϊ״̬
                if(p->Status&0x80)
                    Status=(BIDBI_YXH>>5);
                else
                    Status=(BIDBI_YXF>>5);
                
                //д��Ϣ���ַ
                if(FramePos < 0)
                {
                    *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                    *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                }
                else
                {
                    pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                    pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                    if(InfoAddrSize == 3)
                        pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
                }
                FramePos += InfoAddrSize;
                
                //д״̬
                if((p->Status&BIACTIVEFLAG)==0)
                    pTxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                else
                    pTxData[FramePos]=Status;//����ң��״̬�ֽ�
                    
                if(p->Status&SUBSTITUTEDFLAG)
                    pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                
                    
                FramePos++;

                //дʱ��
                AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);

                pTxData[FramePos++] = LOBYTE(time.MSecond);
                pTxData[FramePos++] = HIBYTE(time.MSecond);
                pTxData[FramePos++] = time.Minute;
                pTxData[FramePos++] = time.Hour;
                pTxData[FramePos++] = time.Day;
                pTxData[FramePos++] = time.Month;
                pTxData[FramePos++] = time.Year;
                
                SendNum++;//���͸���
                p++;
                j++;
                
                if(FramePos>=Length)
                    break;
            }
            
            if(SendNum>0)
            {
                
                Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
                LastDevIndex=i;
                LastFrame=BI;
                BIFrame|=BIETFRAME;
                DevList[i].Data1.SOENum = j;       //DevList[i].Data1.SOENum �Ǳ���Ѿ����͵ĸ���(�����Ѿ�����ĸ����������ڶ�SOE��ָ����п���
                *pTxVSQ=SendNum;

                *LengthOut = Len;      
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;

                //�ж��Ƿ��к�������Ҫ����
                if(DevList[i].Flag)
                {
                    if(WritePtr != DevList[i].RealWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
                    }
                }
                else
                {
                    if(WritePtr != DevList[i].pDbaseWin->BIETimRP+j)
                    {
                        HaveData = TRUE;
                        //Data1.Flag&=(~HaveSOE);
                        //SetUDataFlag();//��������豸�Ƿ���COS��SOE�� //ll feng 2012-6-6 ����������һ��SOEʱ��λACD��־��������վѯ��һ������ʱ���ֻش�������
                    }
                }
                
            }
            else
            {
                //�ߵ����������쳣���
                *LengthOut = 0;     //��ʾû��������Ҫ����
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
�������ƣ�  EnCodeGXReadParaConf
�������ܣ�  �ظ��ٻ��������ġ�
����˵����    
���˵����  �ޡ�
��ע��      ��վ�ٻ�����ʱ���Ȼظ��˱��ģ�֮������֯�������ͱ��ġ�
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
�������ƣ�  EnCodeGXReadParaEnd
�������ܣ�  ������������
����˵����    
���˵����  �ޡ�
��ע��      ������������Ϻ��Դ�֡���Ľ�����
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
void CSecAppSev::EnCodeAllDataConf(void)//���ٻ�ȷ��֡
{
    TxMsg[0]=C_IC_NA;

    (*pTxVSQ) = 1;

    int jj;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    TxMsg[CotLocation]=ACTCON;

    *pTxData=GroupTrn.COT;//INTROGEN=20����Ӧ���ٻ��������ٻ��޶��ʻ�����޶���
    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    *AppCommand|=APP_HAVEDATA1;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}


BOOL CSecAppSev::ProcAllData(void) //����ȫ����
{
    INT16U BeginNo,EndNo,Num,i,Len;
    INT16U yx=0;
    INT16U yc=0;
    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))//1����8�飬ң��
    {
        if (CheckAndModifyGroup())  //�����Ϣ���ַ�����
        {
            BeginNo=GroupTrn.InfoAddr-LBIinfoaddr;  //��ʼ���0

            for(i=0;i<GroupTrn.GroupNo;i++)     //yx = �ѷ��ͺ�׼����������ܸ���
                yx+=Sec101Pad.GroupNum[i];
            EndNo=yx-1;
            
            if ((EndNo+1) > DevList[GroupTrn.DevIndex].DevData.BINum)
                EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1;
                
            Len=EnCodeAllData(BeginNo,EndNo,&Num);
            if (Len!=0)//�����ݿ�ȡ���ݣ���������·�㡣NumΪʵ�ʷ��͵����ݵ�Ԫ��
            {
                GroupTrn.First=FALSE;
                BeginNo+=Num;//��ʼ��ź��ƣ�NumΪ�Ѿ����͵���Ŀ��
                GroupTrn.InfoAddr=BeginNo+LBIinfoaddr;

                if(BeginNo < DevList[GroupTrn.DevIndex].DevData.BINum)  //BeginNo��ȫ��������ᳬ��BINum������Ҫ�����ж�BeginNo�Ϸ���
                {
                    if(YXGroupNo[BeginNo] != GroupTrn.GroupNo)//ǰһ�鷢�꣬������ư��鷢��
                    {
                        GroupTrn.GroupNo = YXGroupNo[BeginNo];
                        if ((GroupTrn.COT!=INTROGEN) && (GroupTrn.COT!=BACK))//����Ƿ����ٻ�����˵����վ�ٻ������������Ѿ����ꡣ
                        {
                            GroupTrn.GroupNo=17;//������֡
                        }
                    }
                }

                return TRUE;
            }
            else//û�з����ݣ���ң�������Ѿ����ꡣ
            {
                if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//�������ݻ����ٻ�������һ�������Ϊ9����ң��
                    GroupTrn.GroupNo=9;
                else//��������Ϊ17�������á�
                    GroupTrn.GroupNo=17;

            }
        }
    }

    if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=12))//9����12�飬ң�⣻ע�Ͳο�����ң�ŵ�
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
                    if ((GroupTrn.COT!=INTROGEN)&&(GroupTrn.COT!=BACK))//�����ٻ�
                        GroupTrn.GroupNo=17;
                }
                return TRUE;
            }
            else
            {
                if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//�������ݻ����ٻ�
                    GroupTrn.GroupNo=17;
                else
                    GroupTrn.GroupNo=17;
            }
        }
    }
    if(GroupTrn.GroupNo==13)//������Ϣ��û�и�������wjr2009.8.31
    {
        GroupTrn.GroupNo=17;    
    }    
    if (GroupTrn.GroupNo==14)//������Ϣ��ֻ�з����ٻ�
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

    if (GroupTrn.GroupNo==15)//��λ����Ϣ
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

    if (GroupTrn.GroupNo==16)//��վԶ���ն�״̬
    {
                
        //���ӹ���Ҫ�����soe
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

    if (GroupTrn.GroupNo>=17)//��������
    {
        if (GroupTrn.COT==BACK)//�������ݣ��޽���֡
        {
            GroupTrn.DevIndex++;
            if(GroupTrn.DevIndex>=DevCount)
            {
                GroupTrn.DevIndex=0;
                Data2Flag&=(~BackData);
            }
            return FALSE;
        }
        if(GroupTrn.COT==PERCYC)//��12�����������ѭ�����ݷ���
        {
            Data2Flag&=(~PerCycData);
            return FALSE;
        }

        //���ٻ�������ٻ���ƽ���ƽ��ģʽ���ٻ����̴���һ�¡�
        //if((GroupTrn.COT>=INTROGEN)&&(GroupTrn.COT<=INTRO16))//�������ж�����ŷǷ�ʱ��������
        {
            Data1.Flag&=(~CallAllData);
            EnCodeGroupEnd();//���ͽ���
            
            FirstCallAllData = 0xff;

            if (GetNextDev())   //����������ô�����������ն��� ll
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
    
    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))//1����8��Ϊң��
    {
        
        for(i=0;i<GroupTrn.GroupNo;i++)
            yx+=Sec101Pad.GroupNum[i];      //yx = �ѷ��ͺ�׼����������ܸ���
        
        
        if(GroupTrn.InfoAddr<(yx-Sec101Pad.GroupNum[GroupTrn.GroupNo-1]+LBIinfoaddr))       //�����ٻ���yx��������ĺͣ���ȥ���һ���ң�Ÿ���
            GroupTrn.InfoAddr=(yx-Sec101Pad.GroupNum[GroupTrn.GroupNo-1]+LBIinfoaddr);
        else if(GroupTrn.InfoAddr > (yx-1+LBIinfoaddr))   //��������ʲô���ã�
        {
            if(GroupTrn.InfoAddr < DevList[GroupTrn.DevIndex].DevData.BINum+LBIinfoaddr)
                GroupTrn.GroupNo=YXGroupNo[GroupTrn.InfoAddr-LBIinfoaddr];
        }

        Num = GroupTrn.InfoAddr-LBIinfoaddr+1;
        if ((Num>DevList[GroupTrn.DevIndex].DevData.BINum)||(GroupTrn.InfoAddr>HBI))//���û��ң�ţ�����Ϊ���豸ң���Ѿ�������
        {
            if(GetDBINum()) //����Ƿ�����˫��ң��
            {
                if((GroupTrn.COT==INTROGEN) && (GroupTrn.HaveSendDBI==FALSE))
                {
                    //�����������˫��ң��û�ͣ�����֯˫��ң�š������ٻ�����˫��ң��
                    GroupTrn.InfoAddr = LBIinfoaddr;
                    GroupTrn.GroupNo = 1;
                    GroupTrn.HaveSendDBI = TRUE;

                    return TRUE;
                }
                
            }
            if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//�����ѭ�����ͻ���Ӧ���ٻ������˳��
                GroupTrn.GroupNo=9;//����Ϊң����ʼ���
            else
                GroupTrn.GroupNo=17;//��17��Ϊ����

            return(FALSE);
        }
        return(TRUE);
    }

    if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=12))//9����12��Ϊң��
    {
        for(i=8;i<GroupTrn.GroupNo;i++)
            yc+=Sec101Pad.GroupNum[i];

        if(GroupTrn.InfoAddr<(yc-Sec101Pad.GroupNum[GroupTrn.GroupNo-1]+LAI))
            GroupTrn.InfoAddr=(yc-Sec101Pad.GroupNum[GroupTrn.GroupNo-1])+LAI;
        else if(GroupTrn.InfoAddr>(yc-1+LAI))
            GroupTrn.GroupNo=YCGroupNo[GroupTrn.InfoAddr-LAI];

        Num=GroupTrn.InfoAddr-LAI+1;
        if ((Num>DevList[GroupTrn.DevIndex].DevData.AINum)||(GroupTrn.InfoAddr>HAI))//ң���Ѿ�������,
        {
            if ((GroupTrn.COT==BACK)||(GroupTrn.COT==INTROGEN))//����Ǳ������ݻ���Ӧ���ٻ������˳��
                GroupTrn.GroupNo=17;    //��ת������
            else
                GroupTrn.GroupNo=17;
            return(FALSE);
        }
        return(TRUE);
    }

    if (GroupTrn.GroupNo==14)//14�顪������ң�������ֵ�������ޣ������ٻ�ʱ���ͣ����ٻ��򱳾����ݲ�����
    {
        //������Ϣ���ַ���
        if (GroupTrn.InfoAddr<LPARA)//LPARA=0x5001
            GroupTrn.InfoAddr=LPARA;
        Num=GroupTrn.InfoAddr-LPARA+1;
        if ((Num>DevList[GroupTrn.DevIndex].DevData.AINum*3)||(GroupTrn.InfoAddr>HPARA))//ÿ��ң��3�����������ޡ����ޡ�����
        {
            GroupTrn.GroupNo=17;
            return(FALSE);
        }
        return(TRUE);
    }

    if (GroupTrn.GroupNo==15)//15�顪����λ����Ϣ
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

    if (GroupTrn.GroupNo==16)//16�顪����վ״̬
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
                
                logSysMsgNoTime("101soe�ٻ����� start=%d, wptr=%d,max=%d",GroupTrn.SoeStartPtr,wptr,Num,0);
            }
            else
            {
                //����
                logSysMsgNoTime("101soe�ٻ����� wptr=%d,max=%d",wptr,Num,0,0);
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

    if ((GroupTrn.GroupNo>=1)&&(GroupTrn.GroupNo<=8))   //ң��
    {
        Len=EnCodeAllYX(GroupTrn.DevIndex,BeginNo,EndNo,pNum);//�����ݿ�ȡ���ݲ���֯������·�����Ϣ������Ӧ�ò㱨�ĵ��ܳ��ȣ�pNumΪʵ�ʷ��������ݵ�Ԫ��Ŀ��
        //return(Len);
    }
    else if ((GroupTrn.GroupNo>=9)&&(GroupTrn.GroupNo<=12))    //ң��
    {
        Len=EnCodeAllYC(GroupTrn.DevIndex,BeginNo,EndNo,pNum);
        //return(Len);
    }
    else if (GroupTrn.GroupNo==14)//����P_ME_NA\P_ME_NB\P_ME_NC
    {
        Len=EnCodeAllPara(GroupTrn.DevIndex,BeginNo,EndNo,pNum);
        //return(Len);
    }
    else if (GroupTrn.GroupNo==15)         //
    {
        TxMsg[0]=M_ST_NA;  //5����λ����Ϣ��
        (*pTxVSQ) &= ~VSQ_SQ;

        //Len=EditTestSPI();//���������±ߵ�if������ʱ��       ����coverity����
        /*if (Len)
        {
            *LengthOut=Len;
            if (BalanMode)////ƽ��ģʽ
            {
                if(GroupTrn.COT==BACK)//��������
                    *AppCommand=APP_SENDNOCON;
                else//���ٻ������
                    *AppCommand=APP_SENDCON;
            }
            else//��ƽ���������ٻ��򱳾�
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
INT16U CSecAppSev::EnCodeAllLastSoe(INT16U BeginNo)
{
    INT8U Status;
    INT16U i,j,jj,Len,Length,WritePtr,SendNum;
    short FramePos,Num;
    struct BIEWithTimeData_t *p;
    struct Iec101ClockTime_t time;
    
    BOOL HaveData=FALSE;        //�����Ƿ�������
    
    *LengthOut = 0;
    for (i=0;i<DevCount;i++)
    {
                
        //logSysMsgNoTime("SEC �༭����SOE��ʼ",0,0,0,0);//  debug ll
        TxMsg[0]=M_SP_TA;   //��ʱ��ĵ�����Ϣ
        if(Sec101Pad.SOEWithCP56Time == 1)
            TxMsg[0]=M_SP_TB;   //����ʱ��ĵ�����Ϣ
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
            Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
            
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
                    //״̬ת�������ݿ��е�ң��D7Ϊ״̬����Լ��D0Ϊ״̬
                    if(p->Status&0x80)
                        Status=1;
                    else
                        Status=0;
                    
                    //д��Ϣ���ַ
                    if(FramePos < 0)
                    {
                        *pTxInfoAddr    =LOBYTE((p->No+LBIinfoaddr));
                        *(pTxInfoAddr+1)=HIBYTE((p->No+LBIinfoaddr));
                    }
                    else
                    {
                        pTxData[FramePos]   = LOBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                        pTxData[FramePos+1] = HIBYTE(p->No+LBIinfoaddr);//��Ϣ���ַ
                        if(InfoAddrSize == 3)
                            pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
                    }
                    FramePos += InfoAddrSize;
                    
                    //д״̬
                    if((p->Status&BIACTIVEFLAG)==0)
                        pTxData[FramePos]=Status|P101_IV;//����ң��״̬�ֽ�
                    else
                        pTxData[FramePos]=Status;//����ң��״̬�ֽ�
                        
                    if(p->Status&SUBSTITUTEDFLAG)
                        pTxData[FramePos]|=P101_SB;//����ң��״̬�ֽ�
                    
                    FramePos++;
                    //дʱ��
                    AbsTimeConvTo(&p->Time,(void*)&time,IEC101CLOCKTIME);
    
                    pTxData[FramePos++] = LOBYTE(time.MSecond);
                    pTxData[FramePos++] = HIBYTE(time.MSecond);
                    pTxData[FramePos++] = time.Minute;
    
                    if(TxMsg[0]==M_SP_TB)//��ʱ��
                    {
                        pTxData[FramePos++] = time.Hour;
                        pTxData[FramePos++] = time.Day;
                        pTxData[FramePos++] = time.Month;
                        pTxData[FramePos++] = time.Year;
                    }
    
                    SendNum++;//���͸���
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
                
                Len=FramePos+AsduHeadLength;//Ӧ�ò㱨���ܳ���
                LastDevIndex=i;
                LastFrame=Polling;
                DevList[i].Data1.SOENum = 0;       //DevList[i].Data1.SOENum �Ǳ���Ѿ����͵ĸ���(�����Ѿ�����ĸ����������ڶ�SOE��ָ����п���
                *pTxVSQ=SendNum;
                                
                *LengthOut = Len;      
                if(BalanMode)
                    *AppCommand=APP_SENDCON;
                else
                    *AppCommand=APP_SENDDATA;

                //�ж��Ƿ��к�������Ҫ����
                GroupTrn.SoeStartPtr +=  SendNum;
                
                HaveData = TRUE;
                
                /*if(HaveData == FALSE)
                {
                    //����ң�Ŷ��������ˣ��ٴ���˫��ң�š���ʱ���˫��ң�ų������⣬�����Ͳ�����վ
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
        //�ȷ��͵��㣬���е��㶼�������ˣ��ٷ���DBI��ȥ��
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
        case C_LC_CALL_YC_YX:    //���Ʒ���Һ���ٻ�ң��ң������
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
        case C_LC_CALL_SYSINFO_SOE:  //���Ʒ���Һ���ٻ�ϵͳ��Ϣ��SOE
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
        case C_LC_CALL_NAME_VER_CLOCK:  //���Ʒ���Һ���ٻ�ң�����ơ�ң�����ơ��汾��Ϣ��ʱ����Ϣ
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
        /*case C_LC_FAULT_RESET:     //Һ����λ��������
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
    TxMsg[0] = C_LC_CALL_SYSINFO_SOE;//������Ҫ���͵���������
    (*pTxVSQ) = VSQ_SQ;
    //logSysMsgNoTime("�ٻ�ϵͳ��Ϣ",0,0,0,0);
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
    if (!BalanMode)//ƽ��ģʽ
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
        
    TxMsg[0] = C_LC_CALL_SYSINFO_SOE;//������Ҫ���͵���������
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
        //SOE���
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
        //SOE״̬
        *(pTxData + i * 10 + 2) = TempBuf->Status;
        //SOEʱ��
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
    if (!BalanMode)//ƽ��ģʽ
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
    
    TxMsg[0] = C_LC_CALL_SYSINFO_SOE;//������Ҫ���͵���������
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
        //SOE���
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
        //SOE״̬
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
    if (!BalanMode)//ƽ��ģʽ
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
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//������Ҫ���͵���������
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
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//������Ҫ���͵���������
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
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//������Ҫ���͵���������
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
    TxMsg[0] = C_LC_CALL_NAME_VER_CLOCK;//������Ҫ���͵���������
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
    //��֡����
    FramePos=0;
    SendNum=0;
    YxNum=0;
    //Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
    
    for(i=0;i<Amount;i++)
    {
    	pTxInfoObjectTemp[FramePos++] = LOBYTE(YX_Addresses[i]);
        pTxInfoObjectTemp[FramePos++] = HIBYTE(YX_Addresses[i]);
    	BeginNo = YX_Addresses[i] - 0x0001;
    	EndNo = BeginNo;
    	CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
    	if((DBData[0]&BIACTIVEFLAG)==0)
            pTxInfoObjectTemp[FramePos]=((DBData[0]&0x80)>>7)|P101_IV;//���ݿ�D7Ϊң��״̬,����Чλ
        else
            pTxInfoObjectTemp[FramePos]=((DBData[0]&0x80)>>7);//���ݿ�D7Ϊң��״̬
            
        if(DBData[0]&SUBSTITUTEDFLAG)
            pTxInfoObjectTemp[FramePos]|=P101_SB;//���ݿ�D7Ϊң��״̬,����Чλ
        
            
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
    TxMsg[0]= M_SP_NA;//������Ҫ���͵���������
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
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
        
    ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
    for(i=0;i<ByteNum;i++)
    {
        if((DBData[i]&BIACTIVEFLAG)==0)
            pTxData[FramePos]=((DBData[i]&0x80)>>7)|P101_IV;//���ݿ�D7Ϊң��״̬,����Чλ
        else
            pTxData[FramePos]=((DBData[i]&0x80)>>7);//���ݿ�D7Ϊң��״̬
            
        if(DBData[i]&SUBSTITUTEDFLAG)
            pTxData[FramePos]|=P101_SB;//���ݿ�D7Ϊң��״̬,����Чλ
        
            
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
    //��֡����
    //Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
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

    Len=FramePos+AsduHeadLength-2;//Ӧ�ò㱨�ĵ��ܳ���
    if (Len)
    {
        //Len=FramePos+AsduHeadLength-2;//Ӧ�ò㱨�ĵ��ܳ���
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

    //ȡ������
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

    //��֡����
    Length=ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
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
        if (Num>=127)//ÿ֡������127�����ݵ�Ԫ
            Stop=TRUE;
    }
    if (FramePos<=0)
        return (0);

    *pTxVSQ |= Num;

    Len=FramePos+AsduHeadLength;//Ӧ�ò㱨�ĵ��ܳ���
    if (Len)
    {
        Len=FramePos+AsduHeadLength;//Ӧ�ò㱨�ĵ��ܳ���
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
Purpose:       �����ٻ�����������֡
Input:         DevIndex���豸�������ţ�qoi�����ٻ��Ĳ������ͣ�LCInforAddr�ٻ������ļ��ĵ�ַ
Output:��            
Author:        lw
Date:          2018.9.04��д
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
	TxMsg[0]=C_LC_SET_PARA;//151,Һ���趨����
    TxMsg[1]=filenum;
    TxMsg[CotLocation]=07;//6������
    
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
	TxMsg[0]=C_LC_ACTIVATE_PARA;//103��ʱ��ͬ��
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6������
    
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
	TxMsg[0]=C_LC_PANEL_DRIVER;//103��ʱ��ͬ��
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6������
    
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
	TxMsg[0]=C_LC_PANEL_DRIVER;//103��ʱ��ͬ��
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6������
    
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

    TxMsg[0]=C_LC_CALL_NAME_VER_CLOCK;//103��ʱ��ͬ��
    TxMsg[1]=1;
    TxMsg[CotLocation]=07;//6������
    
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

    TxMsg[0] = Sec101Pad.TypeID[GroupTrn.GroupNo-1];//������Ҫ���͵���������
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
    Length = ASDULEN-AsduHeadLength-8-sizeof(INT16U);//250-6-8-2=234ΪӦ�ò㷢����Ϣ��󳤶�
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
    else if(TxMsg[0]==M_SP_NA)//1-����byte
    {
        if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        for(i=0;i<ByteNum;i++)
        {
            SendNum++;
            if((DBData[i] & BIDBI_STATUSE) == 0)    //��˫��ң�ţ��򰴵���ң�ŷ���
            {
                if((DBData[i]&BIACTIVEFLAG)==0)
                    pTxData[FramePos]=((DBData[i]&0x80)>>7)|P101_IV;//���ݿ�D7Ϊң��״̬,����Чλ
                else
                    pTxData[FramePos]=((DBData[i]&0x80)>>7);//���ݿ�D7Ϊң��״̬
                    
                if(DBData[i]&SUBSTITUTEDFLAG)
                    pTxData[FramePos]|=P101_SB;
                    
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
            
            if((FramePos>=Length)||(YxNum>=127))
                break;
        }
        
    }
    else if(TxMsg[0]==M_DP_NA)  //3-˫��byte     
    {
        TxMsg[1]= 0;    //��˳��Ԫ��
        
    	EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1;     //���ң�Ÿ������󣬳���DBData����������ô��Ҫѭ����
    	if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        FramePos = InfoAddrLocation;    //��������Ϣ���ַλ��
        for(i=0;i<(ByteNum);i++)
        {
            SendNum ++;
            
            if(DBData[i] & BIDBI_STATUSE)
            {
            
                yxSendno = GroupTrn.InfoAddr+i-LBIinfoaddr;   //���㱾�ε�ң�ŷ�����ţ���0��ʼ���㣨GroupTrn.InfoAddrĿǰ�Ǽ�¼�Ĵ�LBIinfoaddr��ʼ����ţ����ڿ��ƴ���ң�ŵĵ�ǰλ�ã� ll 21-03-28
                TxMsg[FramePos++] = LOBYTE((yxSendno+LDBIinfoaddr));//��Ϣ���ַ
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
        
        FramePos -= AsduHeadLength; //Ϊ���ݵ���ң�ų��򣬵���FramePos�Ĵ�СΪȥ��AsduHeadLength��С
        
    }
    else if(TxMsg[0] == M_DP_NA_ALLDBI)  //�㶫Ҫ���ȫ˫��ң�ŷ��ͣ����⴦��
    {
        TxMsg[0] = M_DP_NA;
                
    	EndNo = DevList[GroupTrn.DevIndex].DevData.BINum-1;     //���ң�Ÿ������󣬳���DBData����������ô��Ҫѭ����
    	if(DevList[GroupTrn.DevIndex].Flag==1)
            ByteNum=CRSendBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        else
            ByteNum=CLBYTE_ReadSBI(devid,BeginNo,EndNo,DBData);
        
        //FramePos = InfoAddrLocation;    //��������Ϣ���ַλ��
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
            
            if (BalanMode)//ƽ��ģʽ
            {
                if(GroupTrn.COT==BACK)
                    *AppCommand=APP_SENDNOCON;
                else
                    *AppCommand=APP_SENDCON;
            }
            else//��ƽ�⣬���������ٻ������顢��������
            {
                *AppCommand=APP_SENDDATA;
                if(GroupTrn.COT!=BACK)//�������ݲ�����һ�����ݱ�־
                    *AppCommand|=APP_HAVEDATA1;
            }
        }
        else
        {
            *LengthOut = 0;
            Len = SendNum;    //������ֵ�²���������
            *AppCommand = APP_NOJOB;
            if(BalanMode)
            {
                myEventSend(myTaskIdSelf(),FORCESCHEDULE);  //����û���κ����ݣ����Ƚ�����һ�η���
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

    TxMsg[0]=Sec101Pad.TypeID[GroupTrn.GroupNo-1];//������Ҫ���͵���������
    (*pTxVSQ) = VSQ_SQ;

    TxMsg[CotLocation]=GroupTrn.COT;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);

    *pTxInfoAddr    =LOBYTE((GroupTrn.InfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((GroupTrn.InfoAddr));

    devid=DevList[GroupTrn.DevIndex].DevID;

    FramePos=0;

    //ȡ������
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
                case M_ME_NA:     //9����ֵ����һ��ֵ
                case M_ME_ND:
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Flag=pAIValue->Flag;
                    DevList[GroupTrn.DevIndex].DevData.AIData[No].Value=Value;
                    //Value=(long)Value*0x3FFF/(long)DevList[GroupTrn.DevIndex].DevData.AIMaxVal[No];
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
    while ((FramePos<Length)&&(!Stop))
    {
        switch(TxMsg[0])
        {
            case M_ME_NA:     //9����ֵ����һ��ֵ
            case M_ME_NB:     //11����ֵ�����ֵ
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
            case M_ME_NC:    //13����ֵ���̸�����
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
        if (*pNum>=127)//ÿ֡������127�����ݵ�Ԫ
            Stop=TRUE;
    }
    if (FramePos<=0)
        return (0);

    *pTxVSQ |= (*pNum);

    Len=FramePos+AsduHeadLength;//Ӧ�ò㱨�ĵ��ܳ���
    if (Len)
    {
        *LengthOut=Len;
        if (BalanMode)//ƽ��ģʽ,���ٻ�������ѭ�����򱳾�����
        {
            if((GroupTrn.COT==BACK)||(GroupTrn.COT==PERCYC))
                *AppCommand=APP_SENDNOCON;
            else
                *AppCommand=APP_SENDCON;
        }
        else//��������ٻ���
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

    TxMsg[0]=Sec101Pad.TypeID[GroupTrn.GroupNo-1];//������Ҫ���͵���������
    (*pTxVSQ) = VSQ_SQ;

    Length=ASDULEN-AsduHeadLength-15;//250-6-15=229ΪӦ�ò㷢����Ϣ��󳤶�;15��3���������������ֽ�����
    FramePos=0;
    No=BeginNo;
    *pNum=0;
    while ((FramePos<Length)&&(!Stop))
    {
        switch(TxMsg[0])
        {
            case M_ME_NA:     //9����ֵ����һ��ֵ
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
            case M_ME_NB:     //11����ֵ�����ֵ
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
            case M_ME_NC:    //13����ֵ���̸�����
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
            case M_ME_ND:    //21����Ʒ�ʣ���һ��
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
        if (*pNum>=127)//ÿ֡������128�����ݵ�Ԫ
            Stop=TRUE;
    }
    if (FramePos<=0)
        return (0);

    (*pTxVSQ) |= (*pNum);

    Len=FramePos+AsduHeadLength;//Ӧ�ò㱨�ĵ��ܳ���
    if (Len)//ֻ�з����ٻ���ƽ����ƽ��һ��
    {
        *LengthOut=Len;
        *AppCommand=APP_SENDDATA;
        *AppCommand|=APP_HAVEDATA1;
    }
    return (Len);
}

//
void CSecAppSev::EnCodeGroupEnd(void)//����֡
{
    int jj;
    TxMsg[0]=GroupTrn.TypeID;
    (*pTxVSQ) = 1;

    TxMsg[CotLocation]=ACTTERM;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrn.DevIndex].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    if (TxMsg[0]==C_IC_NA) //���ٻ�
        *pTxData=GroupTrn.COT;//INTROGEN;
    else                             //counter
    {
        *pTxData=GroupTrn.Description;
    }

    *LengthOut=AsduHeadLength+1;
    if (BalanMode)
        *AppCommand=APP_SENDCON;
    else//��������ٻ�
    {
        *AppCommand=APP_SENDDATA;
    }
}

void CSecAppSev::EnCodeDDGroupEnd(void)//����֡
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
    if (BalanMode)//ƽ��ģʽ
        *AppCommand=APP_SENDCON;
    else//��ƽ��
    {
        *AppCommand=APP_SENDDATA;
    }
}

//OK
BOOL CSecAppSev::GetNextDev(void) //�õ���һ���豸
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

BOOL CSecAppSev::GetNextDDDev(void) //�õ���һ���豸
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
BOOL CSecAppSev::EnCodeNVA(void)  //�༭�仯ң������;
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

    for (j=0;j<DevCount;j++)//ÿ�ٻ�һ�α仯ң�����ݣ�ֻ��һ֡�������Ƿ��꣬���δ�����´��ٻ�ʱ�ٷ��ͣ�
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
                
                if(DevList[NvaActDevNo].DevData.AIPara[No].type)
                {
                    NvaVal = abs((INT16U)DevList[NvaActDevNo].DevData.AIData[No].Value - (INT16U)value);
                }
                else
                {
                    NvaVal = abs(DevList[NvaActDevNo].DevData.AIData[No].Value-value);
                }
                
                if (NvaVal >= DevList[NvaActDevNo].DevData.AIPara[No].DeadValue)//�Ƚϱ仯ֵ������ֵ��С
                {
                    DevList[NvaActDevNo].DevData.AIData[No].WillSend=TRUE;//���÷��ͱ�־
                    NvaNum++;
                }
                DevList[NvaActDevNo].DevData.AIData[No].Flag=pAIValue->Flag;
                DevList[NvaActDevNo].DevData.AIData[No].TempValue=value;
                No++;
                pAIValue++;
            }
        }

        No=DevList[NvaActDevNo].DevData.NvaNo; //���ϴη����ң����ſ�ʼ����
        Length=Sec101Pad.MaxALLen-AsduHeadLength-5-sizeof(INT16U);//���Է������ݵı��ĳ���
          
        while ((FramePos<Length)&&(!Stop))//�鷢������֡
        {
            if (DevList[NvaActDevNo].DevData.AIData[No].WillSend)
            {
                if (FramePos < 0)
                {
                    TxMsg[0]=Sec101Pad.TypeID[YCGroupNo[No]-1];//��¼��֡ң�������
                    *pTxInfoAddr    =LOBYTE((No+LAI));
                    *(pTxInfoAddr+1)=HIBYTE((No+LAI));
                }
                else
                {
                    if(Sec101Pad.TypeID[YCGroupNo[No]-1]==TxMsg[0])//�������ݱ�����ǰ�������һ��
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
                        case M_ME_NA:     //9����ֵ����һ��ֵ
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

                            DevList[NvaActDevNo].DevData.AIData[No].Value = DevList[NvaActDevNo].DevData.AIData[No].TempValue;
                            break;
                        case M_ME_NC:    //13����ֵ���̸�����
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
                        case M_ME_ND:        //21����Ʒ�������Ĳ���ֵ����һ��
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
                    if (*pTxVSQ >= 127)//���ܳ���127��
                        Stop=TRUE;
                }
            }
            No++;
            if ((No >= DevList[NvaActDevNo].DevData.AINum)||(*pTxVSQ >= NvaNum))
            {
                No=0;
                NvaActDevNo++;//�豸����ڷ���һ֡���ۼӡ�
                Stop=TRUE;
                if (NvaActDevNo>=DevCount)//�����豸�仯ң�ⶼ����һ�顣
                {
                    NvaActDevNo=0;
                    Data1.Flag&=(~HaveNVA);
                    Over=TRUE;
                }
            }
        }  //end of while ((FramePos<i)&&(!Stop))

        DevList[NvaActDevNo].DevData.NvaNo=No;       //��¼�´η��ͱ仯ң��Ŀ�ʼ��š�
        if (FramePos>0)
        {
            *LengthOut=FramePos+AsduHeadLength;
            *AppCommand=APP_SENDDATA;
            if(!Over)//�仯ң��û���꣬����һ�����ݱ�־
                *AppCommand|=APP_HAVEDATA1;
            if(BalanMode)
                *AppCommand=APP_SENDCON;
            return(TRUE);//ÿ��ֻ��һ֡
        }
    }//end of for
    return(FALSE);
}

BOOL CSecAppSev::CheckNVA(void)  //���仯ң������;
{
    INT16U i,j,No;
    short Value,AINum;
    struct RealAINFlag_t *pAIValue;
    INT16U temp;
    
    
    for (j=0;j<DevCount;j++)
    {
        No=0;
        while (No<=(int)(DevList[NvaActDevNo].DevData.AINum-1))//ȡң��ֵ�����÷��ͱ�־
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
                
                if (temp >= DevList[NvaActDevNo].DevData.AIPara[No].DeadValue)//�Ƚϱ仯ֵ������ֵ��С
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


//������
void CSecAppSev::FreezeCounter(void)    //�����ȣ���QCC 7λΪ1ʱ
{
    INT16U GetBeginNo,GetEndNo,i,j;
    struct RealCounter_t *q;
    short CountNum;
    INT32U value,*p;

    GetSysTime((void*)(&CounterTime),ABSTIME);//��¼�����ȵ�ϵͳʱ��

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
                if (GroupTrnDD.Description&FREEZENORESET)//���᲻��λ
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
    
    
    //�յ�0x45�յ�������  ����ǰ�豸��֧������ģ���±�׼�ľ�����event����������ģ�����˲ʱ�����ļ�����  CL 20180801
    if((RxMsg[AsduHeadLength]&0xc0) == 0x40)  //FRZ=1 ���᲻����λ����
    {
        SetFileSynInfoTaskIDSubstation(MySelf.AppTID);  //��Ϊ����101��վ����ʼ����
        if(XSFileSynInfo.TaskIDPri101[0]!=0)//�����һ������ģ���������
        {
            myEventSend(GetFileSynInfoTaskID101(0),XSFREEZE);//��101��վ��������Ϣ ��ʱ�ȷ�����һ��101���񣬺�����ͨ��ά�����������ȷ�ϵġ�
        }
        else
        {
            //logSysMsgWithTime("��֧��2018��׼������ģ�飡",0,0,0,0);
        }
        if(XSFileSynInfo.TaskIDPri101[1]!=0)//�����еڶ�������ģ���������
        {
            myEventSend(GetFileSynInfoTaskID101(1),XSFREEZE);
        }    
    } 
    //�յ�0x45�յ�������  ����ǰ�豸��֧������ģ���±�׼�ľ�����event����������ģ�����˲ʱ�����ļ�����  CL 20180801
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
    //logSysMsgNoTime("DD����ȷ��֡",0,0,0,0);   //  debug ll
    *LengthOut=AsduHeadLength+1;
    *AppCommand=APP_SENDDATA;
    if((GroupTrnDD.Description&0xc0)==0)//�ǶԶ���������Ӧ�𣬷����ǶԶ��������Ӧ��
        *AppCommand|=APP_HAVEDATA1;
    if(BalanMode)
    {
        *AppCommand=APP_SENDCON;
    }      
}

void CSecAppSev::ProcCounter(void) //������zzw
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
                BeginNo+=Num;//������ʼ��ţ�����һ�η������ݵ���ʼ���

                GroupTrnDD.InfoAddr=BeginNo%MAXBCRNUM+LBCR;
                GroupNo=GroupTrnDD.InfoAddr-LBCR;                  
                GroupNo/=GROUPBCRNUM;
                GroupNo++;

                if (GroupTrnDD.GroupNo!=GroupNo)//�������ݷ������
                {
                    GroupTrnDD.GroupNo=GroupNo;
                    if (GroupTrnDD.COT>REQCOGCN)//�����ٻ��������������ݡ�wjr  2009.8.25 ����ԭ���жϴ���ԭΪ��������
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
        EnCodeDDGroupEnd();//���ͽ���
        DDFreeze=FALSE;
        if(GetNextDDDev())//��������¸��߼��豸�ĵ��û�����������õ�ȱ�־��
        {
            EditDDCon=0xff;
            Data1.Flag|=CallDD;
            DDFreeze=TRUE;
        }
    }
}

INT8U CSecAppSev::EnCodeCounter(INT16U BeginNo,INT16U EndNo,INT16U *pNum)//�����ݿ�ȡ��ȣ�����󣬷�����·�㡣
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
    *pTxVSQ = 0;/*��ȶ�������ɢ���к�*/
    
    TxMsg[CotLocation]=GroupTrnDD.COT;
    
    for(jj=0;jj<PubAddrSize;jj++)
    {
        TxMsg[PubAddrLocation+jj]=DevList[GroupTrnDD.DevIndex].Addr>>(8*jj);
    }
    
    No=BeginNo;
    FramePos=0-InfoAddrSize;
    SendNum=0;
    Len=0;
    Length=ASDULEN-AsduHeadLength-10-sizeof(INT16U);//250-6-10-2=232ΪӦ�ò㷢����Ϣ��󳤶�
    
	/* AJ++170829 ��������ݿ�.��2015��չ��Լ�£�������足��ֱ�Ӷ����ݿ� */
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
        //if((Sec101Pad.control&CON_101GYKZ))		//AJ++170829 ����ʵʱֵ   
        if(GYKZ2015Flag)                                   
        {
            value = *p;
            p ++;   
        }
        else									//���䶳��ֵ
        {
            value=DevList[GroupTrnDD.DevIndex].DevData.CounterData[No].Value;   
        }

        //д��Ϣ���ַ
        if(FramePos<0)
        {
            *pTxInfoAddr    =LOBYTE((No+LBCR));
            *(pTxInfoAddr+1)=HIBYTE((No+LBCR));
        }
        else
        {
            pTxData[FramePos]   = LOBYTE((No+LBCR));//��Ϣ���ַ
            pTxData[FramePos+1] = HIBYTE((No+LBCR));//��Ϣ���ַ
            if(InfoAddrSize == 3)
                pTxData[FramePos+2] = 0;//��Ϣ���ַΪ3�ֽ�ʱ������ֽ�Ϊ0
        }
        FramePos+=InfoAddrSize;
        //д���ֵ
        pTxData[FramePos]=LOBYTE(LOWORD(value));
        pTxData[FramePos+1]=HIBYTE(LOWORD(value));
        pTxData[FramePos+2]=LOBYTE(HIWORD(value));
        pTxData[FramePos+3]=HIBYTE(HIWORD(value));
        pTxData[FramePos+4]=i;//˳���
        FramePos+=5;
        //дʱ��
        if(TxMsg[0] == M_IT_TA)//16��ʱ��ĵ��
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
    TrTime=100;//��ֹżȻ������ʱ�ӣ�����ʱ����ҡ�
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

    TxMsg[0]=M_CD_NA;//106����ʱ���

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

    *LengthOut=AsduHeadLength+2;//ֻ����2�ֽ�ms��
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

void CSecAppSev::EnCodeClock(void)//������վ����ʱ��ǰ��ϵͳʱ��
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
        	TxMsg[CotLocation]=ACTCON | 0x40;                       //��վ��������ӻ��߶���ʱ������ʹ���ԭ��Ϊ0x47
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

    if(OldSysTime.MSecond>=TimeDelay)//������·�ӳ�
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
    
    if (GetActDevIndexByAddr(RDPubAddr))//���ݹ������ַ���豸��š�
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
    
    
    if ((RDInfoAddr>=LDBIinfoaddr)&&(RDInfoAddr<=HBI))        //wjr 2009.4.5  ӦΪ��Ϣ���ַ�����ǹ������ַ
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.InfoAddr=RDInfoAddr;                    //wjr 2009.4.5  
        if(GroupTrn.InfoAddr>=(LDBIinfoaddr+DevList[GroupTrn.DevIndex].DevData.DBINum/2))   /*��������Ϊ����ң����   wjr2009.8.25*/
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
        else                                                                             /*��������Ϊ˫��ң��  wjr2009.8.25*/
            No=GroupTrn.InfoAddr-LDBIinfoaddr;
        GroupTrn.GroupNo=YXGroupNo[No];
    }
    else if ((RDInfoAddr>=LAI)&&(RDInfoAddr<LAI+DevList[GroupTrn.DevIndex].DevData.AINum))         //wjr 2009.4.5
    {
        //��ң��
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
        //����λ����Ϣ
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=15;
        GroupTrn.InfoAddr = RDInfoAddr;         // ll ���ӣ�GroupTrn.InfoAddrû��ֵ�ͽ��м������Ǵ����
        No=GroupTrn.InfoAddr-LSPI;
    }
    //else if (GroupTrn.InfoAddr==RTUSTATUS)
    else if (RDInfoAddr==RTUSTATUS)     //ll �޸�
    {
        GroupTrn.TypeID=C_IC_NA;
        GroupTrn.GroupNo=16;
        No=0;
    }
    else if ((RDInfoAddr>=LBCR)&&(RDInfoAddr<LBCR+DevList[GroupTrn.DevIndex].DevData.CounterNum))          //wjr 2009.4.5
    {
        //�����
        GroupTrn.TypeID=C_CI_NA;
        GroupTrnDD.GroupNo=(GroupTrnDD.InfoAddr-LBCR)/GROUPBCRNUM+1;
        GroupTrnDD.COT = REQ;
        GroupTrnDD.InfoAddr = RDInfoAddr;         // ll ���ӣ�GroupTrnDD.InfoAddrû��ֵ�ͽ��м������Ǵ����
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
        Len=EnCodeAllData((INT16U)No,(INT16U)No,&Num);//����Ӧ�ò㱨���ܳ���
        
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

//�������ַ����Ϣ���ַ����
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

//һ��ֻ��ȡһ������
void CSecAppSev::EnCodeReadPara(INT16U DevIndex,INT16U InfoAddr)
{
    INT16U No;
    INT8U Len=0;
    short val = 0;         //����coverity����
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
    TxMsg[0]=Sec101Pad.TypeID[13];//���յ�14�����õ����ͷ�����

    switch(TxMsg[0])
    {
        case M_ME_NA:     //9����ֵ����һ��ֵ
            switch((InfoAddr-LPARA)%3)
            {
                case 0://����
                    
                    val=DevList[DevIndex].DevData.AIPara[No].DeadValue;
                    break;
                case 1://����
                    val=DevList[DevIndex].DevData.AIPara[No].UpLimit;
                    break;
                case 2://����
                    val=DevList[DevIndex].DevData.AIPara[No].LowLimit;
                    break;
            }

            //val=(long)val*0x3FFF/(long)DevList[DevIndex].DevData.AIMaxVal[No];
            
            *(pTxData)  =LOBYTE(val);
            *(pTxData+1)=HIBYTE(val);
            *(pTxData+2)=0;
            Len=3;
            break;
        case M_ME_NB:     //11����ֵ�����ֵ
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
        case M_ME_NC:    //13����ֵ���̸�����
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
        case M_ME_ND:    //21����Ʒ�ʣ���һ��
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
    INT8U Len = 0;                  //����coverity��̬���������
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
        case P_ME_NA:    //��һ��
        case P_ME_NB:    //��Ȼ�
            *pTxData=LOBYTE(ParaWord);
            *(pTxData+1)=HIBYTE(ParaWord);
            *(pTxData+2)=ParaQPM;
            Len=3;
            break;
        case P_ME_NC:    //�̸�����
            //tempfloat = (float)ParaWord;  //ll �޸ģ�������ֵ
            tempfloat = (float)ParaFloat;
            //logSysMsgNoTime("SEC �̸�����f=%x,f2=%x",tempfloat,ParaFloat,0,0);   //  debug ll
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
        *pRestType = 0xee;  //��λ��Լ����״̬���� ll 2010/07/20   for ������Լ����
        
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
            myTaskLock();//��ֹ��ռ����
            if(DevList[DevIndex].Flag)
                DevList[DevIndex].RealWin->BIETimRP+=DevList[DevIndex].Data1.SOENum;
            else
                DevList[DevIndex].pDbaseWin->BIETimRP+=DevList[DevIndex].Data1.SOENum;
            clear_flag(DevList[DevIndex].DevID,BIETFLAG);

            BIFrame&=(~BIETFRAME);
            DevList[DevIndex].Data1.SOENum=0;
            myTaskUnlock();//������ռ����

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
�������ƣ�  ProcFT_EncodeReadDir()
�������ܣ�  ��Ŀ¼�Ĵ�����֯����֡
����˵����  
���˵���� TRUE   �к�������   FALSE û�к�������
��ע��      
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcFT_EncodeReadDir(void)
{
    INT8U len;
    BOOL  rc;
    INT8U FramePos;
    INT16U jj;
        
    len = 0;
    
    //����Ŀ¼ID��Ŀ¼���ƣ���֯Ŀ¼���ļ��Ĵ���.���ļ�������ʼ��д
    
    rc = FT_ReadDirectory(&FtInfo, &len);
       
    //len = 0 �ط񶨻ش�rc=true��ʾ���޺���
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;   
         
    TxMsg[FramePos++] = 2;         //�������ݰ�����
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
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
           
    return rc;

}
/*------------------------------------------------------------------/
�������ƣ�  ProcFT_ReadDir()
�������ܣ�  �����ļ������Ŀ¼���
����˵����  
���˵����      
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
    memcpy(FtInfo.tempname,&pRxData[7],namelen);    //FtInfo.tempname����ʱ��ţ����ڴ�ӡ��Ϣ
    for(i = 0; i < namelen; i++)
    {
        FtInfo.tempname[i] = tolower(FtInfo.tempname[i]);
    }
    
    FtInfo.dirid = FT_GetDirID(&FtInfo);    //����Ŀ¼������Ŀ¼ID DirID��ʱ��ʹ�� liuwei20170307
    
    pRx = &pRxData[7]+namelen;  //���ٻ���־λ�õ�ָ�븳ֵ��pRx    
    CallFlag = pRx[0];  //�ٻ���־
    
    FtInfo.callflag = CallFlag;
    
    logSysMsgNoTime("�ٻ�Ŀ¼%s �ٻ���־=%d, DirID1=%d, DirID2=%d",(INT32U)FtInfo.tempname, CallFlag, FtInfo.dirid, DirID);  
    
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
    
    SetSendData2Flag(DATA2_FT_DIR);
       
}
/*------------------------------------------------------------------/
�������ƣ�  ProcFT_EncodeFileData()
�������ܣ�  ����Ҫ������ļ�����
����˵����  
���˵����  TRUE ��ʾ�к����� FALSE �޺���
��ע��      
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
    
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
    TxMsg[FramePos++] = 2;    //�������ݰ�����
    TxMsg[FramePos++] = FR_RD_FILE_DATA;
    
    //4�ֽ��ļ���ʾ�ļ�ID
    TxMsg[FramePos++] = LLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = LHBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HLBYTE(FtInfo.fileid);
    TxMsg[FramePos++] = HHBYTE(FtInfo.fileid);
    
    //����FileFlag�ļ���ʾ�����Ҳ�ͬ�ļ� 
    rc = FT_ReadData(&FtInfo, &TxMsg[FramePos], &len);
            
    EnCode101DLMsg(FramePos+len, APP_SENDDATA);     //FramePos�����������ݰ���������ʶ���ļ�ID 
    
    return rc;

}
/*------------------------------------------------------------------/
�������ƣ�  ProcFT_EncodeFileActConf()
�������ܣ�  ���ļ�����ȷ��֡
����˵����  
���˵����  
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
    
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
    TxMsg[FramePos++] = 2;    //�������ݰ�����
    TxMsg[FramePos++] = FR_RD_FILE_ACTCON; 
    successPos = FramePos;  //��¼�ɹ�ʧ��λ�� 
    FramePos++;
    
    //�ļ���
    namelen = FtInfo.namelen;
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
�������ƣ�  ProcFT_ReadFileAct()
�������ܣ�  ����Ҫ�����ļ�
����˵����  
���˵����  
��ע��      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_ReadFileAct(void)
{
    INT16U namelen, i;    
    
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
    FtInfo.namelen = namelen;
    
    SetSendData2Flag(DATA2_FT_FILEACT);
    
}
/*------------------------------------------------------------------/
�������ƣ�  ProcFT_EncodeWriteFileActConf()
�������ܣ�  д�ļ�����ȷ��֡
����˵����  
���˵����  
��ע��      
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
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength; 
    
    TxMsg[FramePos++] = 2;    //�������ݰ�����
    TxMsg[FramePos++] = FR_WR_FILE_ACTCON;
    TxMsg[FramePos++] = result;
    
    //�ļ���
    namelen = FtInfo.namelen;
    TxMsg[FramePos++] = namelen;
    memcpy(&TxMsg[FramePos], FtInfo.name, namelen);
    FramePos += namelen;
    
    //4�ֽ��ļ���ʾ�ļ�ID
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
�������ƣ�  ProcFT_WriteFileAct()
�������ܣ�  д�ļ������¼�ļ�������ʼ����ز�����
����˵����    
���˵����  
��ע��      
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
    
    
    //�����ļ����ƣ���֯�ļ��Ĵ���
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
�������ƣ�  ProcFT_EncodeWriteFileDataConf()
�������ܣ�  д�ļ������������ȷ��֡��
����˵����    
���˵����  
��ע��      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_EncodeWriteFileDataConf(void)
{
    INT8U FramePos;
    INT16U jj;

    //��֯��������
    TxMsg[0] = F_FR_NA_N;
    TxMsg[1] = 1;   //VSQ
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
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
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
        
    logSysMsgNoTime("д�ļ�ȷ��֡%s, err=%d, offset=%d", (INT32U)FtInfo.name, FtInfo.errinfo, FtInfo.offset,0);
     
    FT_ParaReset(&FtInfo);  //�ļ���������������
    
}

/*------------------------------------------------------------------/
�������ƣ�  ProcFT_WriteFileData()
�������ܣ�  ����Ҫд���ļ�����
����˵����  
���˵����  
��ע��      
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_WriteFileData(void)
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
            //��������£����ļ������ˣ������﷢ȷ��֡
            //����ܿ��ܷ�����ȥ
            SetSendData2Flag(DATA2_FT_WTDATAACT);     //��ǰ��ȷ��֡����Ҫ������վ�Ƿ������Ͻ���ͨ�ţ�����Ӧ����дflash��ʱ��
        } 
        segmentlen = (LengthIn) - AsduHeadLength - 12;   // AsduHeadLength=9  12=�����ݿ�������ֽ�
        //logSysMsgNoTime("offset=%d, segmentlen=%d",offset,segmentlen,0,0);
        
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
    
    //�쳣�д��������£������﷢ȷ��֡
    if((rc == TRUE) && (IsNoFinish))
        SetSendData2Flag(DATA2_FT_WTDATAACT);
    
    
    
}

void CSecAppSev::ProcFileTran(void)
{
    //INT8U len;
    
    /*if(INFOADDR2BYTE)
    {
        pRxData = &RxMsg[InfoAddrLocation+2];
        //logSysMsgNoTime("�յ��ļ���������%x=%x-%x-%x",pRxData[0],pRxData[1],pRxData[2],pRxData[3]);
    }*/
    
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
            //����Ӧ�ò�ش�
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
�������ƣ�  ProcFT_ProgramUpdate()
�������ܣ�  ������������
����˵����    
���˵����  
��ע��      ��ʱ��������ʹ�ô���ľ���ش�ʽ�����������ǻ�Ӱ��ش�����ȼ���һ�����ⲻ��
/------------------------------------------------------------------*/
void CSecAppSev::ProcFT_ProgramUpdate(void)
{
    ProgramUpadateSE = pRxData[0];
    if((RxCot&COT_REASON) == ACT)
    {
        
        if(pRxData[0] & 0x80) //CTYPE
        {   
            //��������   
            FtInfo.IsUpdate = TRUE;
            ProgramUpadateCot = ACTCON;
            
        }
        else
        {
            //��������
            FT_ParaReset(&FtInfo);
            ProgramUpadateCot = ACTCON; 
            StartProgramUpdate();
        }
    }
    else
    {
        //��������
        FT_ParaReset(&FtInfo);
        ClearProgramUpdate();
        ProgramUpadateCot = DEACTCON;
    }
    
    SetSendData2Flag(DATA2_PUP_PROGUP);
    
}
/*------------------------------------------------------------------/
�������ƣ�  RMTReadAllPara()
�������ܣ�  ��֯��ȫ������
����˵����  
���˵����  TRUE-�к��� FALSE-�޺���
            ���в���ʹ�������ϴ������á���ֵ�����ø������ϴ������á�
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
        //��װ���в���
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
        //��װ�ն����в���
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
        //��װ�ն���·���в���
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
        //��װ�ն˶�ֵ����
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
        //��װ�ն���·��ֵ����
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
�������ƣ�  ProcEncodeGXReadPara()
�������ܣ�  ��֯������
����˵����  
���˵����  rc = TRUE-�к��� FALSE-�޺���
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
            TxMsg[1] = sendnum | 0x80;
            if(rc == FALSE) 
            { 
                rc2 = FALSE;
                //RMTParaInit();                      �˴�Ϊ��Ҫ���ô˺��� 20181018��
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
             rc = GXReadJianGePara(&TxMsg[FramePos], &len, &sendnum, InfoAddrSize,GXParaControl);     //���������,Roi-INTRO2Ϊ�����ţ���0��ʼ
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
�������ƣ�  ProcEncodeGXSetPara()
�������ܣ�  ��֯�ظ�Ԥ�ò����ı���
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeGXSetPara(void)
{
        
    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=GXReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);
}

/*------------------------------------------------------------------/
�������ƣ�  ProcEncodeGXActivatePara()
�������ܣ�  ��֯�ظ���������ı���
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeGXActivatePara(void)
{
   
    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=GXReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);    
}

/*------------------------------------------------------------------/
�������ƣ�  ProcEncodeGXChangePara()
�������ܣ�  ��֯�ظ��ı�����ı���
����˵����  
���˵����  
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
�������ƣ�  ProcEncodeGXSendPara()
�������ܣ�  ��֯�������Ͳ����ı���
����˵����  
���˵����  
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
�������ƣ�  ProcEncodeRMTReadPara()
�������ܣ�  ��֯������
����˵����  
���˵����  rc2 = TRUE-�к��� FALSE-�޺���
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
    TxMsg[1] = 0;   //VSQ�����������
    
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[ActDevIndex].Addr>>(8*jj);
    
    
    FramePos = PubAddrLocation+PubAddrSize;    
    
    
    TxMsg[FramePos++] = 1;   //����
    TxMsg[FramePos++] = 0;
    piCodePosition = FramePos++;    //pi��λ��
 
    if(RMTParaReadAllFlag)
    {
        rc = RMTReadAllPara(&TxMsg[FramePos], &len, &sendnum);   
        FramePos += len;
        TxMsg[1] = sendnum;
        if(rc == FALSE) 
        {
            TxMsg[piCodePosition] = 0;     //�����������޺��� 
            rc2 = FALSE;
            RMTParaInit();
        }
        else
        {
            TxMsg[piCodePosition] = RP_PI_CONT;     //�����������к���
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
            
            GetTerminalPara(&TxMsg[FramePos+2], &len, RMTParaInfo[i], 1);
            if(len != 0)
            {
                sendnum++;
                TxMsg[FramePos++] = LOBYTE(RMTParaInfo[i]);  //��Ϣ���ַ
                TxMsg[FramePos++] = HIBYTE(RMTParaInfo[i]);
                
                FramePos += len;
            }
            
            if(FramePos >= 200)
                break;
        }
        logSysMsgNoTime("������ȡ����(��ʼ=%d, ����=%d, ��=%d)",RMTHaveReadParaFlag,sendnum,RMTParaNum,0);

        TxMsg[1] = sendnum;
        RMTHaveReadParaFlag += procnum;
        //��������
        if(RMTHaveReadParaFlag >= RMTParaNum) 
        {
            RMTParaInit();
            TxMsg[piCodePosition] = 0;     //�����������޺��� 
            rc2 = FALSE;
        }
        else
        {
            TxMsg[piCodePosition] = RP_PI_CONT;     //�����������к���
        }
        
    }
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
    
    return rc2;
    
}
/*------------------------------------------------------------------/
�������ƣ�  ProcReadParaGX
�������ܣ�  �����ٻ��������ģ�ȡ��ROI�������ٻ��޶��ʣ���
����˵����    
���˵����  �ޡ�
��ע��      ROI�Ǳ�����վҪ��ȡȫ�������Ƿ����ٻ�������
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
�������ƣ�  ProcSetParaGX
�������ܣ�  ����Ԥ�ò������ġ�
����˵����    
���˵����  �ޡ�
��ע��      
/------------------------------------------------------------------*/
void CSecAppSev::ProcSetParaGX(void)
{
    INT8U *pInfoAddr;
    INT16U i,pos;
    float tempval;
    INT32U temp32;
    INT16U infoaddr;
    
    //�������ݴ浽wrongdata��
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
                    pos += 2;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("GXԤ�ò���info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                } 
                            
                
                //SetSendData2Flag(DATA2_GX_SETPARA);            
            }
            else
            {
                GXvsqflag = 1;
                //GXParaInfo[0] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
                infoaddr = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                pos += 2;
                for(i=0; i<GXParaNum; i++)
                {
                    GXParaInfo[i] = infoaddr + i;
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    GXParaValue[i] = tempval;    
                    pos +=4 ; 
                    ProgLogWrite2("GXԤ�ò���info=0x%x, value=0x%x",GXParaInfo[i],GXParaValue[i],0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);                
                }
            } 
            
            if(GXRemoteParaCheck() == 1)
            {
                //�����쳣 ��Ӧ���ñ�����0������
                GXParaYZ = FALSE;
                GXParaInfo[0] = 0;  
                GXParaNum = 0;              
                GXReturnCot = ACTCON|0x40;  //�񶨻ش�
            }
            else
            {
                //������ȷ
                GXParaYZ = TRUE;
                GXReturnCot = ACTCON;  
            }         
        }
        else if((RxCot&COT_REASON)==DEACT)
        {
            //����
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
        ProgLogWrite2("GXԤ�ò��� QPM=%d ����",RxMsg[LengthIn-1],0,0,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
        
        /*if((RxCot&COT_REASON)==ACT)
        {
            GXReturnCot = ACTCON | 0x40;
            GXParaNum = RxVsq&VSQ_NUM;      //������ll
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
�������ƣ�  GXRemoteParaCheck
�������ܣ�  ��һ�������쳣
����˵����    
���˵����  
��ע��      ���쳣ֹͣ�������
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
�������ƣ�  ProcActivateParaGX
�������ܣ�  ������������ġ�
����˵����    
���˵����  �ޡ�
��ע��      
/------------------------------------------------------------------*/
void CSecAppSev::ProcActivateParaGX(void)
{
    INT8U i;
    INT16U SetFlag;
    
    //�������ݴ浽wrongdata��
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
                //logSysMsgNoTime("101����ң������ֵ������Ч",0,0,0,0);
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
        logSysMsgNoTime("GXԶ�̲���δԤ�þ͹̻�101",0,0,0,0);
        
    }
    
    SetSendData2Flag(DATA2_GX_ACTIVATEPARA);        
}
/*------------------------------------------------------------------/
�������ƣ�  GXWatchLPChange
�������ܣ�  ����Զ�̲��������ӱ��ز����仯��
����˵����    
���˵����  �ޡ�
��ע��      ROI�Ǳ�����վҪ��ȡȫ�������Ƿ����ٻ�������
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
�������ƣ�  ProcReadPara()
�������ܣ�  ���������
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::ProcReadPara(void)
{
    INT8U *pInfoAddr;
    INT16U i, pos;
    INT8U num;
    
    //���ݳ��ȴ���8����ʾ�ǲ��ֶ�ȡ������Ϊȫ����ȡ
    if(LengthIn > 9) //FrameLen+2��ȥ��������C�ĳ���
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pInfoAddr = &RxMsg[InfoAddrLocation+2];
        pos = 0;
        num = RxVsq&0x7f;   //�����ȡ�����ĸ���
        
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
            pos += 2;
        }
        RMTParaNum += num;  //��¼�ۼ��·��Ķ���������
        
        logSysMsgNoTime("sec=%d, num=%d, max=%d, info2=%x",RMTSectionNo,num,RMTParaNum,RMTParaInfo[RMTParaNum]);
        
    }
    else
    {
        //ȫ����ȡ   
        RMTParaReadAllFlag = TRUE;
        RMTHaveReadParaFlag = 1;
        RMTParaNum = 0;
    }
    
    SetSendData2Flag(DATA2_RMT_READPARA);
        
}
/*------------------------------------------------------------------/
�������ƣ�  RMTParaYzCheck()
�������ܣ�  
����˵����  
���˵����  
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
�������ƣ�  GXParaYzCheck()
�������ܣ�  
����˵����  
���˵����  
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
�������ƣ�  RMTParaInit()
�������ܣ�  Զ�̲�����д��Ӧ��־��ʼ��
����˵����  
���˵����  
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
�������ƣ�  SetSendData2Flag()
�������ܣ�  ������Ҫ���͵�2�����ݱ�־
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::SetSendData2Flag(INT32U flag)
{
    Data2Flag |= flag;     
    *LengthOut=0;
    *AppCommand = APP_NOJOB;
    
}
/*------------------------------------------------------------------/
�������ƣ�  DeadValueRealTimeEffect()
�������ܣ�  �·���������ʵʱ��Ч
����˵����  
���˵����  
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
�������ƣ�  ProcWritePara()
�������ܣ�  ����д����
����˵����  
���˵����  
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
    
    //�������ݴ浽wrongdata��
    WrongDataLength = LengthIn;
    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
    if((RxCot&COT_REASON)==ACT)    //����
    {
        RMTSectionNo = MAKEWORD(RxMsg[InfoAddrLocation], RxMsg[InfoAddrLocation+1]);  
        pi  = RxMsg[InfoAddrLocation+2];
        pInfoAddr = &RxMsg[InfoAddrLocation+3];
       
        pos = 0;
        if(pi & RP_PI_SE)   //Ԥ��
        {
            if(RMTParaYZ)   //ֻ����һ֡���ĵ�Ԥ�ã�û�����겻��������Ԥ�ñ���
            {
                logSysMsgNoTime("��һ֡����Ԥ�ñ��Ļ�δ����101",0,0,0,0);
                return;
            }
            RMTParaNum = RxVsq&VSQ_NUM;    //��ʱ������ ���в�����д��
            for(i=0; i<RMTParaNum; i++)
            {
                RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
                pos += 2;
                
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
            //�̻�
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
                RMTReturnCot = ACTCON;
                
                
                
                if(IsSuccess==FALSE)
                {
                	RMTReturnCot |= 0x40; //ʧ��
                }
                else
                {
                    if(GetSiQuChangeFlag(ParaFlag))           //��Լ��Ĳ���ʵʱ��Ч������ͬ����������ֵ��ʵʱ��Ч�ķ�ʽʵ�֡�
                    {
                        DeadValueRealTimeEffect();
                    }
                    SaveTerminalPara(); //�̻���ϣ�д��flash
                    SaveRMTParaToSys();
                }
                
                SetSendData2Flag(DATA2_RMT_SETPARA);
                
            
            }  
        }
        
    }
    else
    {
        //����
        RMTParaNum = 0;
        RMTParaYZ = 0;
        RMTParaInfo[0] = 0;
        
        RMTParaInit();
        
        RMTReturnCot = DEACTCON;
        SetSendData2Flag(DATA2_RMT_SETPARA);
        
    }
  
}
/*------------------------------------------------------------------/
�������ƣ�  ProcSetSectionNo()
�������ܣ�  �л���ֵ����
����˵����  
���˵����  
��ע��      ��ʱ��֧�ֶ�����
/------------------------------------------------------------------*/
void CSecAppSev::ProcSetSectionNo(void)
{
    //INT8U SectionNo;
    
    
    //SectionNo = MAKEWORD(pRxData[0],pRxData[1]);
    
    SetSendData2Flag(DATA2_RMT_SETSEC);
    
}

/*------------------------------------------------------------------/
�������ƣ�  ProcEncodeSetSectionNo()
�������ܣ�  ���ö�ֵ����
����˵����  
���˵����  
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
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;
    
    TxMsg[FramePos++] = 0;  //��ǰ����
    TxMsg[FramePos++] = 0;
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
}

/*------------------------------------------------------------------/
�������ƣ�  ProcReadSectionNo()
�������ܣ�  ����ֵ����
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::ProcReadSectionNo(void)
{
    
    SetSendData2Flag(DATA2_RMT_READSEC);
       
}


/*------------------------------------------------------------------/
�������ƣ�  ProcEncodeRMTSetPara()
�������ܣ�  ��֯����ֵ����ȷ��֡
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeRMTSetPara(void)
{

    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=RMTReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);
    
}
/*------------------------------------------------------------------/
�������ƣ�  ProcEncodePUPupdateConf()
�������ܣ�  ��֯Ӧ��֡
����˵����  
���˵����  
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
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;  
    
    TxMsg[FramePos++] = ProgramUpadateSE;
    /*if(FtInfo.IsUpdate)
    {
        //����
        TxMsg[FramePos++] = 0x80;   
    }
    else
    {
        //����������ȡ������
        if(ProgramUpadateCot == ACTCON)
        {
            //��������
            TxMsg[FramePos++] = 0; 
        }
        else
        {
            //ȡ������
            TxMsg[FramePos++] = 0; 
        }
    }*/
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
    
}

/*------------------------------------------------------------------/
�������ƣ�  void ProcEncodeXSFileSynConf(void)
�������ܣ�  ��֯�����ļ�ͬ��ȷ��֡
����˵����  
���˵����  
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
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
     
    FramePos = AsduHeadLength;      
    TxMsg[FramePos++] = 0;
         
    EnCode101DLMsg(FramePos, APP_SENDDATA);   
    
}

/*------------------------------------------------------------------/
�������ƣ�  void ProcEncodeXSFileSynFinish(void)
�������ܣ�  ��֯�����ļ�ͬ��������ֹ֡
����˵����  
���˵����  
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
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
     
    FramePos = AsduHeadLength;      
    TxMsg[FramePos++] = 0;
         
    EnCode101DLMsg(FramePos, APP_SENDDATA);      
}

/*------------------------------------------------------------------/
�������ƣ�  ProcEnCodeReadSectionNo()
�������ܣ�  ��֯����ֵ����ȷ��֡
����˵����  
���˵����  
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
    //��Ϣ���ַ
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;
    
    FramePos = AsduHeadLength;    

    TxMsg[FramePos++] = 1;  //��ǰ����
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;  //��С����
    TxMsg[FramePos++] = 0;
    TxMsg[FramePos++] = 1;  //�������
    TxMsg[FramePos++] = 0;
    
    EnCode101DLMsg(FramePos, APP_SENDDATA);
    
    
}


/*------------------------------------------------------------------/
�������ƣ�  EnCode101DLMsg()
�������ܣ�  ��֯�������ݳ��Ⱥ�����
����˵����  len �������ݳ���   appcmd ����������������APP_SENDDATA����ƽ��ģʽ���Զ�תΪAPP_SENDCON
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::EnCode101DLMsg(INT16U len, INT16U appcmd)
{
    *LengthOut  = len;  
    
    *AppCommand = appcmd;
    if((appcmd == APP_SENDDATA) && (BalanMode))
        *AppCommand = APP_SENDCON;
    
}

/*------------------------------------------------------------------/
�������ƣ�  JudgeSendInitEnd()
�������ܣ�  �ж��Ƿ�����ʼ������
����˵����  
���˵����  
��ע��      InitFlag 0xff��ʾδ���͹��� 0��ʾ���͹�
/------------------------------------------------------------------*/
void CSecAppSev::JudgeSendInitEnd(void)
{
    if(BalanMode)//ƽ��ģʽ
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
        else//�ն�δ��λ
        {
            if(IsAllSendInitEnd)        //��ӣ�ÿ�����½������ӷ��ͳ�ʼ������֡��ѡ� ll
            {
                Data1.Flag|=HaveInitEnd;
                *AppCommand|=APP_HAVEDATA1;
            }
        }
        
    }
        
}

//��֯һ��SPIֵ��Ϊ�˲���
INT8U CSecAppSev::EditTestSPI(void)
{
    (*pTxVSQ) = 1;

    *pTxData     = 10;//vti
    *(pTxData+1) = 0; //qds

    return(2);
}


//�ļ�����
void CSecAppSev::ProcFileCall(void)
{
    struct AbsTime_t absTime;

    Data2Flag|=UpLoadFile;
    CurrentInfoAddr=RxInfoAddr;
    CurrentFileName=RxMsg[AsduHeadLength]+(RxMsg[AsduHeadLength+1]<<8);

    if((RxCot&COT_REASON)==REQ)//�ٻ�Ŀ¼
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
            case 0://ȱʡ
                break;
            case 1://ѡ���ļ�
                FileStep=PSelectFile;
                if(CurrentInfoAddr&LUBOFLAG)//����¼������
                {
                    CurrentFileSize=FILELENGTH;//�ļ�����
                    FileReadPtr=0;//�Ѿ��������ݳ���ָ��
                    CurrentZBNo=0;//��ǰҪ�����ݵ��ܲ���
                    FileCheckSum=0;//�ļ�����У���
                }
                else//��ʷ������� zzw
                {
                    FileCheckSum=0;
                    //ʱ��ĸ�ʽ��Ҫȷ��
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
                    HisDDDevNo=ActDevIndex;//�豸���
                    //����ʷ���
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
            case 2://�����ļ�
                FileStep=PCallFile;

                break;
            case 3://ֹͣ�����ļ�
                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                CurrentFileSize=0;
                FileReadPtr=0;
                CurrentZBNo=0;
                break;
            case 4://ɾ���ļ�
                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                break;
            case 5://ѡ��ڣ�ʵ��ͨѶ����û����һ����
                FileStep=PCallSection;
                break;
            case 6://�����
                FileStep=PCallSection;
                break;
            case 7://ֹͣ�����
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

//����¼������Ŀ¼
void CSecAppSev::SendDir(void)
{
    int jj;
    short SendNum,FramePos,j,Length;

    TxMsg[0]=F_DR_NA;//Ŀ¼
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
        Length=ASDULEN-AsduHeadLength-13;//Ӧ�ò㷢����Ϣ��󳤶�
        jj=0;
        for(j=DirUnit.ReadPtr;j<DirUnit.FileNum;j++)
        {
            jj++;
            if(DirUnit.File[j].SendFlag)//�Ѿ����͵��ļ����ξͲ��ٷ���
                continue;
            if((FramePos==-2)||(FramePos==-3))
            {
                *pTxInfoAddr    =LOBYTE((DirUnit.File[j].InfoAddr));
                *(pTxInfoAddr+1)=HIBYTE((DirUnit.File[j].InfoAddr));
                FramePos+=InfoAddrSize;
            }
            /*else
            {
                *(pTxData+FramePos)=LOBYTE(DirUnit.File[j].InfoAddr);//��Ϣ���ַ
                *(pTxData+FramePos+1)=HIBYTE(DirUnit.File[j].InfoAddr);//��Ϣ���ַ
                FramePos+=InfoAddrSize;
            }*/

            pTxData[FramePos++]=LOBYTE(DirUnit.File[j].Name);
            pTxData[FramePos++]=HIBYTE(DirUnit.File[j].Name);
            pTxData[FramePos++]=LOBYTE(LOWORD(DirUnit.File[j].Length));
            pTxData[FramePos++]=HIBYTE(LOWORD(DirUnit.File[j].Length));
            pTxData[FramePos++]=LOBYTE(HIWORD(DirUnit.File[j].Length));

            //pTxData[FramePos++]=HIBYTE(DirUnit.File[j].Status);
            pTxData[FramePos++]= DirUnit.File[j].Status;                //����coverity����

            pTxData[FramePos++]=LOBYTE(DirUnit.File[j].Time.MSecond);
            pTxData[FramePos++]=HIBYTE(DirUnit.File[j].Time.MSecond);
            pTxData[FramePos++]=DirUnit.File[j].Time.Minute;
            pTxData[FramePos++]=DirUnit.File[j].Time.Hour;
            pTxData[FramePos++]=DirUnit.File[j].Time.Day;
            pTxData[FramePos++]=DirUnit.File[j].Time.Month;
            pTxData[FramePos++]=DirUnit.File[j].Time.Year;

            SendNum++;//���͸���

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
            pTxData[FramePos-8]|=0x20;//���һ���ļ���״̬
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
    TxMsg[0]=F_FR_NA;//�ļ�׼������

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
    *(pTxData+4)=LOBYTE(HIWORD(CurrentFileSize));//���ȵ����8λ��
    if(CurrentFileSize)
        *(pTxData+5)=0;//�϶�
    else
        *(pTxData+5)=0x80;//��

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
    TxMsg[0]=F_FR_NA;//��׼������

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
    *(pTxData+2)=01;//һ���ļ���Ϊ��һ���ڡ�
    *(pTxData+3)=LOBYTE(LOWORD(CurrentFileSize));
    *(pTxData+4)=HIBYTE(LOWORD(CurrentFileSize));
    *(pTxData+5)=LOBYTE(HIWORD(CurrentFileSize));//���ȵ����8λ��
    if(CurrentFileSize)
        *(pTxData+6)=0;//��׼������
    else
        *(pTxData+6)=0x80;//��δ׼������

    *LengthOut=AsduHeadLength+7;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;

    Data2Flag&=(~UpLoadFile);
    FileStep=PFileOver;
}

void CSecAppSev::SendSegment(void)
{
    if(CurrentInfoAddr&LUBOFLAG)//����¼������
    {
        #ifdef _SAVESAMPDATA_
        SendSegment1();
        #endif
    }
    else//��ʷ�������
    {
        SendSegment2();
    }
}
#ifdef _SAVESAMPDATA_
//����¼������
void CSecAppSev::SendSegment1(void)
{
    short Length,i;
    int jj;
    INT8U *p;

    short value1,value2;
    long  tempvalue;

    TxMsg[0]=F_SG_NA;//������

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *(pTxData)=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);
    *(pTxData+2)=01;//����

    //�����ݣ�ÿ�ζ�3���ܲ�������
    //Length=readLBData((INT8U)(CurrentFileName-1),CurrentZBNo,3,(INT8U *)(pTxData+4),192);
    //CurrentZBNo+=3;

    //ÿ��ȡ2���ܲ����������ǰ1���ܲ�
    Length=readLBData((INT8U)(CurrentFileName-1),CurrentZBNo,2,(INT8U*)Data,300);
    if(Length==0)
    {
        logSysMsgNoTime("��¼�����ݴ���",0,0,0,0);
    }

    CurrentZBNo++;

    p=(INT8U *)(pTxData+4);
    //��ǰ�ܲ���32����չΪ64��,128�ֽڡ�д�뷢�ͻ�����
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
    *(pTxData+3)=Length;//�γ���

    for(i=0;i<Length;i++)//�ۼ����������ݵ�У���
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
//������ʷ�������
void CSecAppSev::SendSegment2(void)
{
    short Length,i,FramePos;
    int jj;
    INT8U *p;
    INT16U unitlength;
    INT16U No,SendNum;
    INT32U *pKWH;
    INT32U KWHValue;

    TxMsg[0]=F_SG_NA;//������

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=FILE_101;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[HisDDDevNo].Addr>>(8*jj);
    *pTxInfoAddr    =LOBYTE((CurrentInfoAddr));
    *(pTxInfoAddr+1)=HIBYTE((CurrentInfoAddr));

    *(pTxData)=LOBYTE(CurrentFileName);
    *(pTxData+1)=HIBYTE(CurrentFileName);
    *(pTxData+2)=01;//����

    p=(INT8U *)(pTxData+4);

    i=ASDULEN-AsduHeadLength-14-4;//�����ڷ������ݵ�����ĳ���
    SendNum=0;
    FramePos=0;
    unitlength=2+12;//ÿ����ȵĳ��ȣ������趨��Ϣ���ַΪ2�ֽ�
    No=DevList[HisDDDevNo].DevData.HisDDReadPtr;
    pKWH=DevList[HisDDDevNo].DevData.HisCounterData;
    BOOL Stop=FALSE;
    while ((FramePos<i)&&(!Stop))
    {
        KWHValue=*(pKWH+No);

        //��Ϣ���ַ
        *(p+unitlength*SendNum+0)=(LBCR+No)&0xff;
        *(p+unitlength*SendNum+1)=((LBCR+No)>>8)&0xff;
        //���ֵ
        *(p+unitlength*SendNum+2)=LOBYTE(LOWORD(KWHValue));
        *(p+unitlength*SendNum+3)=HIBYTE(LOWORD(KWHValue));
        *(p+unitlength*SendNum+4)=LOBYTE(HIWORD(KWHValue));
        *(p+unitlength*SendNum+5)=HIBYTE(HIWORD(KWHValue));
        *(p+unitlength*SendNum+6)=(No%32);
        //ʱ��
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

        if (SendNum>=10)//ÿ֡��Ȳ��ܳ���10��
            Stop=TRUE;
    }
    DevList[HisDDDevNo].DevData.HisDDReadPtr=No;

    Length=FramePos;
    *(pTxData+3)=Length;//�γ���

    for(jj=0;jj<Length;jj++)//�ۼ����������ݵ�У���
        FileCheckSum+=*(pTxData+4+jj);

    FileReadPtr+=Length;
    if(FileReadPtr>=CurrentFileSize)
        FileStep=PLastSegment;

    *LengthOut=AsduHeadLength+4+Length;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}

//��������
void CSecAppSev::SendLastSegment(void)
{

    int jj;
    TxMsg[0]=F_LS_NA;//���Ķ�

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
    *(pTxData+2)=01;//����

    *(pTxData+3)=03;//LSQ

    *(pTxData+4)=FileCheckSum;//У���

    *LengthOut=AsduHeadLength+5;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
    //???
    Data2Flag&=(~UpLoadFile);
    FileStep=PFileOver;
}

//�ļ������Ͽ�
void CSecAppSev::SConfirm(void)
{
    INT16U FileName;


    FileName=(RxMsg[AsduHeadLength])+(RxMsg[AsduHeadLength+1]<<8);

    switch(RxMsg[AsduHeadLength+3]&0x0F)//AFQ
    {
        case 1://�ļ�����Ŀ϶��Ͽɣ�����Ҫ����ˢ�º��Ŀ¼2005-5-30 16:15��ϵ��־��
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
            //����Ŀ¼�б��ٷ����º��Ŀ¼
            Data2Flag|=UpLoadFile;
            FileStep=PCallDir;
            DirUnit.ReadPtr=0;
            */
            break;
        case 2://�ļ�����ķ��Ͽ�
            if(CurrentFileName==FileName)//���´��䣿
            {

                Data2Flag&=(~UpLoadFile);
                FileStep=PFileOver;
                FileReadPtr=0;
            }
            break;
        case 3://�ڴ���Ŀ϶��Ͽ�

            if(CurrentFileName==FileName)
            {
                Data2Flag|=UpLoadFile;
                FileStep=PLastSection;
            }

            break;
        case 4://�ڴ���ķ��Ͽ�
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


//��������
void CSecAppSev::SendLastSection(void)
{

    int jj;

    TxMsg[0]=F_LS_NA;//���Ķ�

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
    *(pTxData+2)=01;//����

    *(pTxData+3)=01;//LSQ�����Ľ�
    *(pTxData+4)=FileCheckSum;//У���

    *LengthOut=AsduHeadLength+5;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
    //???
    Data2Flag&=(~UpLoadFile);
    FileStep=PFileOver;
}

//���¼������
void CSecAppSev::ReadDirList1(void)
{
#ifdef _SAVESAMPDATA_
    int i=0;
    struct  AbsTime_t ftime_abs;
    struct Iec101ClockTime_t ftime_iec;

    //��ʱ���ж��Ƿ���¼������

    readLBTime((void *)&ftime_abs,ABSTIME);

    if((ftime_abs.Minute==0)&&(ftime_abs.MSecond==0))

    {
        DirUnit.FileNum=0;
        DirUnit.WritePtr=0;
        DirUnit.ReadPtr=0;
        logSysMsgNoTime("��¼������",0,0,0,0);
        return;
    }

    //������¼�����ݵ�ʱ��
    readLBTime((void *)&ftime_iec,IEC101CLOCKTIME);

    if(CurrentFileName==0)//�ٻ������ļ�
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
    else//ֻ�ٻ�һ���ļ�
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

//��ʷ���Ŀ¼
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
    HisDDDevNo=ActDevIndex;//�豸���

    HisDDTime.MSecond=0;
    HisDDTime.Day=CurrentInfoAddr&0x1f;
    HisDDTime.Month=(CurrentInfoAddr>>5)&0xf;
    HisDDTime.Year=((CurrentInfoAddr>>9)&0x3f);

    if(CurrentFileName!=0)//ֻ����һ���ļ�
    {
        HisDDTime.Minute=CurrentFileName&0xff;
        HisDDTime.Hour=(CurrentFileName>>8)&0xff;
        DirUnit.FileNum=1;
        DirUnit.WritePtr=1;
    }
    else//���͸����������ļ�
    {
        HisDDTime.Minute=0;
        HisDDTime.Hour=0;
        if(HisDDCycle<=30)//С�ڰ�Сʱ����һ����ʷ��ȣ�ϵͳ�������Ŀ¼48���ļ�
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
        //��������

        ii=(absTime.Minute/HisDDCycle)%SAVENUM;

        if(hisDataPtr->saveData[ii].time.Minute != absTime.Minute)//û�и�ʱ��������
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
//�趨������ֵ
void CSecAppSev::SetProtect(void)
{
    Data1.Flag|=ProtectCon;
    if((RxCot&COT_REASON)==ACT)
    {
        memcpy((void*)ProtectValue,(void*)(pRxData),32);
        //������ֵ�趨����
        if(!setDZ(ProtectValue))//�趨ʧ�ܣ�����ȫ��ȡ����
        {
            for(int i=0;i<32;i++)
                ProtectValue[i]=~ProtectValue[i];
            ProtectValue[32]=0xff;//ʧ�ܱ�־
        }
        else
        {
            ProtectValue[32]=0;//�ɹ���־

            logSysMsgNoTime("��վ���ñ�����ֵ1:%2x %2x %2x %2x",ProtectValue[2],
            ProtectValue[3],
            ProtectValue[4],
            ProtectValue[5]);
            logSysMsgNoTime("��վ���ñ�����ֵ2:%2x %2x %2x %2x",ProtectValue[6],
            ProtectValue[7],
            ProtectValue[8],
            ProtectValue[9]);
            logSysMsgNoTime("��վ���ñ�����ֵ3:%2x %2x %2x %2x",ProtectValue[10],
            ProtectValue[11],0,0);
            logSysMsgWithTime("��վ���ñ�����ֵʱ��",0,0,0,0);
        }
    }
    *LengthOut=0;
    *AppCommand=APP_APPCON;
    *AppCommand|=APP_HAVEDATA1;
}

void CSecAppSev::SendProtectCon(void)
{
    int jj;
    TxMsg[0]=C_PF_NA;//������ֵ����ȷ��

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=ACTCON;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    memcpy((void*)pTxData,(void*)ProtectValue,32);
    pTxData[32]=ProtectValue[32];//�޶���

    *LengthOut=AsduHeadLength+33;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;

    //������ֵ�趨�ɹ���Ҫ��λFTU��ʹ��ֵ��Ч��
    if(ProtectValue[32]==0)
    {
        ResetFlag=0xff;
        ResetCount=0;
    }
}

void CSecAppSev::CallProtect(void)
{
    INT8U feederno;
    //������ԭ�޶��ʵ�λ�ø�Ϊ���ߺţ� 2005-3-14 9:39
    feederno=RxMsg[AsduHeadLength];

    Data2Flag|=ProtectData;

    //������ֵ�ٻ�����
    //�ŵ�ProtectValue[]��
    if(getDZ(feederno,ProtectValue))
    {
        ProtectValue[32]=0;//�ɹ���־
    }
    else
    {
        memset(ProtectValue,0,32);
        ProtectValue[32]=0xff;//ʧ�ܱ�־
    }

    *LengthOut=0;
    *AppCommand=APP_APPCON;
    
}

void CSecAppSev::SendProtectData(void)
{
    int jj;
    TxMsg[0]=M_PF_NA;//������ֵ����

    (*pTxVSQ) = 1;
    TxMsg[CotLocation]=REQ;
    for(jj=0;jj<PubAddrSize;jj++)
        TxMsg[PubAddrLocation+jj]=DevList[0].Addr>>(8*jj);
    *pTxInfoAddr=0;
    *(pTxInfoAddr+1)=0;

    memcpy((void*)pTxData,(void*)ProtectValue,32);
    pTxData[32]=ProtectValue[32];//�޶���

    *LengthOut=AsduHeadLength+33;
    *AppCommand=APP_SENDDATA;
    if(BalanMode)
        *AppCommand=APP_SENDCON;
}
#endif

//��ʱ ������ʷ�������
//��ʷ��ȴ洢ֻ����һ���豸�е�ȡ�

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
            hisDataPtr->saveData[ii].Data = hisDataPtr->saveData[ii-1].Data+CountNum;      //����coverity����
        }
        logSysMsgNoTime("��ʷ��ȴ洢����",0,0,0,0);

        break;//��ֻ֤����һ���豸�ĵ��
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

    for (i=0;i<DevCount;i++)//���豸�е��ʱ����ô�����С�һ���豸�е�ȿ��ԡ�
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
            logSysMsgNoTime("�����ݿ���ʧ��",0,0,0,0);
            return(0);
        }

        KWH=(INT32U*)DBData;
        for(ii=0;ii<CountNum;ii++)
        {
            hourData->Data[ii] = KWH[ii];
        }

        memcpy(&hourData->time,&absTime,sizeof(struct AbsTime_t));

        break;//ֻ����һ���豸�ĵ��
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

//101����Ķ�����ģ���ļ�ͬ������
void CSecAppSev::ProcXSFileSyn()
{
    SetSendData2Flag(DATA2_XSFT_SYNACT);  //����Ӧ�ı�־λ
    SetFileSynInfoTaskIDSubstation(MySelf.AppTID);  //��Ϊ����101��վ����ʼ����
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
}


//101����Ķ�����ģ���ļ�ͬ������
void CSecAppSev::ProcXSFileSynFinish()
{
    SetSendData2Flag(DATA2_XSFT_SYNACTFINISH); 
}
////���Ϲ�ָ�ϼ��ܷ���
/*------------------------------------------------------------------/
�������ƣ�  ProDealF0()
�������ܣ�  �����յ�����վ��ȫ��������
����˵����  

���˵����  0 �ɹ� ���� ʧ�� -10 ��������
��ע��      
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
		buf[0] = buf[2];//��Կ�汾��
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
             //pTxData[1] = 0x01;  	   	//��������
        //}

	    *LengthOut=AsduHeadLength + 2;    
    }

    *AppCommand=APP_SENDDATA;
}
/*------------------------------------------------------------------/
�������ƣ�  ProDealF1()
�������ܣ�  �����յ�����վ��ȫ��������
����˵����  

���˵����  0 �ɹ� ���� ʧ�� -10 ��������
��ע��      
/------------------------------------------------------------------*/
void CSecAppSev::ProDealF1()
{
    int rc,i;
    INT8U buf[18];//ǰ�����ֽ�����Ϣ����
    
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
�������ƣ�  ProcReadParaGD
�������ܣ�  �����ٻ��������ģ���
����˵����    
���˵����  �ޡ�
��ע��      
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
            RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
            pos += 6; //2�ֽ���Ϣ���ַ+4�ֽ�����
                          
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
�������ƣ�  ProcEncodeRMTReadPara_gd()
�������ܣ�  ��֯Զ�̲�����ȡ--�㶫
����˵����  
���˵����  rc = TRUE-�к��� FALSE-�޺���
/------------------------------------------------------------------*/
BOOL CSecAppSev::ProcEncodeRMTReadPara_gd(void)
{
    INT16U i,jj; 
    INT8U len;
    INT16U FramPos;
    
            
    TxMsg[0] = GD_MUTIPARA_READ;
    TxMsg[1] = 0;   //VSQ�����������
    
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
                TxMsg[FramPos++] = HIBYTE(RMTParaInfo[i]);  //��Ϣ���ַ
                            
                FramPos += len;
            }
                    
            if(FramPos >= 220)
                break;
        }
    }
    else
    {
        TxMsg[FramPos++] = LOBYTE(RMTParaInfo[0]);
        TxMsg[FramPos++] = HIBYTE(RMTParaInfo[0]);  //��Ϣ���ַ
        
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
�������ƣ�  ProcWritePara_GD()
�������ܣ�  ����д����
����˵����  
���˵����  
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
    
    //�������ݴ浽wrongdata��
    WrongDataLength = LengthIn;
    memcpy((void*)WrongData,(void*)RxMsg,LengthIn);
    
    if(ReadRemoteParaSetEnableState() == FALSE)
    {
        //�񶨻ش�
        RMTReturnCot = ACTCON|0x40;
        SetSendData2Flag(DATA2_RMT_WRITEPARA_GD);
        logSysMsgNoTime("Զ������Ͷ����ѹ��Ϊ�֣���ֹ�޸Ĳ�������",0,0,0,0);
        return;
    }
    
    writeflag = 0;
    if ((RxCot&COT_REASON)==ACT)    //����
    {
        pInfoAddr = &RxMsg[InfoAddrLocation];
                
        pos = 0;    
        RMTParaNum = RxVsq&VSQ_NUM;    
        if((RxVsq & VSQ_SQ) == 0)
        {
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+6];
                if(qos & 0x80)  //Ԥ��
                {
                    RMTParaInfo[i] = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
                    pos += 2;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    ProgLogWrite2("Ԥ�ò���info=0x%x, value=%d,%d",RMTParaInfo[i],RMTParaValue[i],RMTParaValue[i]*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    //ִ��
                    curparainfo = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);
                    pos += 2; 
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //�ж�Ԥ�úͼ�����ͬ��������
                    if((curparainfo == RMTParaInfo[i]) && (tempval == RMTParaValue[i]))
                    {
                        SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]); 
                        //ProgLogWrite2("��������info=0x%x, val=%d,%d",curparainfo,tempval,tempval*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1); 
                        writeflag = 1;
                    }
                    else
                    {
                        ProgLogWrite2("%d����������Ԥ�ò�����cur=0x%x, old=%x",i,curparainfo,RMTParaInfo[i],0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    }
                }
    
            } 
        }
        else
        {
            info = MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]);    //��Ϣ���ַ
            pos += 2;
            for(i=0; i<RMTParaNum; i++)
            {
                qos = pInfoAddr[pos+4];
                if(qos & 0x80)  //Ԥ��
                {
                    RMTParaInfo[i] = info+i;    //��Ϣ���ַ
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    
                    RMTParaValue[i] = tempval;
                    
                    //ProgLogWrite2("Ԥ�ò���info=0x%x, value=%d,%d",RMTParaInfo[i],RMTParaValue[i],RMTParaValue[i]*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    pos += 5;
                    
                    RMTParaYZ = TRUE;
                }
                else
                {
                    //ִ��
                    curparainfo = info+i;
                    
                    temp32 = MAKEDWORD(MAKEWORD(pInfoAddr[pos], pInfoAddr[pos+1]), MAKEWORD(pInfoAddr[pos+2],pInfoAddr[pos+3]));
                    tempval =*((float *)(&temp32));
                    pos += 5;
                    
                    //�ж�Ԥ�úͼ�����ͬ��������
                    if((curparainfo == RMTParaInfo[i]) && (tempval == RMTParaValue[i]))
                    {
                        SetTerminalPara(RMTParaValue[i], RMTParaInfo[i]); 
                        ProgLogWrite2("��������info=0x%x, val=%d,%d",curparainfo,tempval,tempval*100,0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1); 
                        writeflag = 1;
                    }
                    else
                    {
                        ProgLogWrite2("%d����������Ԥ�ò�����cur=0x%x, old=%x",i,curparainfo,RMTParaInfo[i],0,SYSINFO_WITHTIME, ULOG_TYPE_PARAERR, 1);
                    }
                }
    
            }
        }
            
        //ȷ�ϻش�
        RMTReturnCot = ACTCON;
        SetSendData2Flag(DATA2_RMT_WRITEPARA_GD);
      
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
        RMTReturnCot = DEACTCON;
        SetSendData2Flag(DATA2_RMT_WRITEPARA_GD);
    }

}

/*------------------------------------------------------------------/
�������ƣ�  ProcEncodeRMTSetPara_GD()
�������ܣ�  ��֯����ֵ����ȷ��֡
����˵����  
���˵����  
/------------------------------------------------------------------*/
void CSecAppSev::ProcEncodeRMTSetPara_GD(void)
{

    memcpy((void*)TxMsg,(void*)WrongData,WrongDataLength);
    TxMsg[CotLocation]=RMTReturnCot;
    
    EnCode101DLMsg(WrongDataLength, APP_SENDDATA);
    
}
