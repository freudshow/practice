#ifndef SGC1161CMD_H
#define SGC1161CMD_H

#if __cplusplus
extern "C" {
#endif

#include "publichead.h"
#include "database.h"

#define SGC_HEAD         0x55
#define SGC_SW1_HIGH     0xf0
#define SGC_SPI_READ_CODE  0x00 

struct Sgc1120aFlag_t
{
    INT8U SetParaFlag;
    INT8U UpgradeFlag;
    INT8U YkFlag;
};

INT8U Sgc1161GetChipSerialNumID(INT8U *rcvbuf);
INT8U Sgc1161GetChipKeyVersion(INT8U *pdata,INT8U *rcvbuf);
INT8U Sgc1161VerifyMasterSignData(INT8U KeyId,INT8U* pdata);
INT8U Sgc1161GetRanSignData(INT8U* pdata,INT8U *rcvbuf);
INT8U Sgc1161EncryptData(INT8U *pdata,INT16U lenth);
INT8U Sgc1161DecryptData(INT8U *pdata,INT16U lenth,INT8U *dndata);
INT8U Sgc1161VerifySigndata(INT8U *pdata,INT16U lenth,INT8U KeyId);
INT8U Sgc1161ObtainRandata(void);
INT8U Sgc1161LoadSymmetryKey(INT8U* pdata,INT16U lenth,INT8U KeyId);
INT8U Sgc1161DeCerdata(INT8U* pdata,INT16U lenth,INT8U KeyId);
INT8U Sgc1161LoadCerdata(INT8U* pdata,INT16U lenth,INT8U CerId);
INT8U Sgc1161LoadTemSelfCerdata(INT8U* pdata,INT16U lenth );
INT8U Sgc1161VerifyMaintDevCer(INT8U* pdata,INT16U lenth );
INT8U Sgc1161VerifyMaintDevSigndata(INT8U* pdata,INT16U lenth );
INT8U EbMsgWithAllData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo);
INT8U EbMsgWithTandSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo);
INT8U EbMsgWithSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo);
INT8U EbMsgWithRandSData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo);
INT8U EbMsgUpLoadData(INT8U *pdata,INT8U *decpbuf,INT16U len,INT16U wChanNo);

int CheckTimeAging(INT8U *pdta);
void EbErrCodeSend(INT16U SW,INT8U TypeId,INT16U wChanNo);
int CheckIllfgalType(INT8U *ptada,INT8U type,INT16U wChanNo);
INT8U CheckCerDecryptData(INT8U *pdata,INT16U len);



INT8U GetEbMsgCheckSum(INT8U *pdata);

void EbEditmsg(INT8U *pdata,INT8U* sdatabuf,INT16U slen, INT16U ebtype,INT8U typeId,INT16U msglen);
INT8U EbEncpytDataAnaly(INT8U* pdata,INT16U lenth,INT8U *dedata,INT16U wChanNo);

INT16U EbSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo);
void EbmsgAnalysis(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo);
INT8U EnMsgByGetwayHandle(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo);
INT8U SgcGetwayauthenStepI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcGetwayauthenStepII(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U EnMsgBymasterHandle(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo);
INT8U SgcMasterauthenStepI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcMasterauthenStepII(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcMasterauthenStepIII(INT8U TypeID,INT16U wChanNo);
INT8U SgcKeymanageStepI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcKeymanageStepII(INT8U *pdata,INT16U len,INT16U wChanNo,INT8U typeid);
INT8U SgcCAmanageStepI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcCAmanageStepII(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcCAmanageStepIII(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcYWToolAuthReq(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcYWToolAuthSdata(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcKeyVerforYWTool(INT16U wChanNo);
INT8U SgcGetPbKeyforYWTool(INT16U wChanNo);
INT8U SgcSignYWtoolReqfile(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcYWToolCAmanage(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U SgcYWToolReWritrOriCA(INT16U wChanNo);
INT8U SgcYWToolGetPbKeyCer(INT16U wChanNo);
INT8U SgcYWToolHFDCKey(INT8U *pdata,INT16U len,INT16U wChanNo);




BOOL SendAuthDataToMISI(INT8U *sendbuf,INT16U len,INT16U wChanNo);
BOOL CheckEbMsgSty(INT8U *pdata);
INT16U SGCReceiveData(INT8U *prcv, INT16U len);
INT16U Pack104msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
void PackFixed104ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf);
void PackFra104ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo);
void PackFra68ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo);
INT16U Pack101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
void PackFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf);
INT8U HostSerialNumforYWTool(INT16U wChanNo);
INT8U EnMsgByYWTool(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo);
INT8U SgcYWWaittoSend(INT16U wChanNo);
INT8U UpgradeDataVerify(INT16U wChanNo);



INT8U SgcMaintGetPbKeyCer(INT8U *p);
void GetTerminalId(char *buf, INT8U *plen);
BOOL UpdateProgramMd5(void);
INT8U *GetImageInfo(INT32U *plen);
#if 0
#endif
/////////////////////////////////////////////////////////////////////
//∫˛ƒœ≈©Õ¯º”√‹
////////////////////////////////////////////////////////////////////
INT8U Sgc1120aGetChipSerialNumID(INT8U *rcvbuf);
INT8U Sgc1120aGetChipKeyVersion(INT8U *rcvbuf);
INT8U Sgc1120aGetRandomData(INT8U *rcvbuf);
INT8U Sgc1120aCalculateAuthRData(INT8U *rcvbuf);
INT8U Sgc1120aGetPKeyAuthData(INT8U Fid,INT8U *pdata,INT8U *rcvbuf);
INT8U Sgc1120aGetKeyConsultData(INT8U Fid,INT8U *pdata,INT8U *rcvbuf);
INT8U SGC1120aSginVerify(INT8U *DataInput, INT16U inputlen, INT8U *pucsign, INT16U signlen, INT8U keyno);
INT8U Sgc1120aUploadPKeyDataToChip(INT8U *pdata,INT8U *rcvbuf);
INT8U Sgc1120aUploadSymKeyDataToChip(INT8U *pdata,INT8U *rcvbuf);
INT8U Sgc1120aDectyData(INT8U *pdata,INT8U *rcvbuf,INT16U len);
INT8U Sgc1120aEnctyData(INT8U *pdata,INT16U len);
INT16U Eb1120aSafetySearchFrame(INT8U *oribuf,void *validbuf,INT16U *validtaillen,INT16U len,INT16U wChanNo);
BOOL Check1120aEbMsgSty(INT8U *pdata);
INT16U Eb1120amsgAnalysis(INT8U* pdata,INT8U *rxbuff,INT16U wChanNo);
INT16U Sgc1120aMasterAuthI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Sgc1120aMasterAuthII(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Sgc1120aMasterAuthIV(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Sgc1120aMasterAuthV(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Sgc1120aMasterAuthVI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Sgc1120aMasterAuthVI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Sgc1120aSymKeyUploadI(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Sgc1120aSymKeyUploadII(INT8U *pdata,INT16U len,INT16U wChanNo);
INT16U Pack1120a101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
void Pack1120aFra10ToEb(INT8U *pdata,INT16U len,INT8U *ebbuf);
INT8U SGC1120aSKeyConsult(INT8U * pdata,INT8U * cdata,INT16U cdatalen,INT8U *signdata,INT16U signdatalen,INT8U keyno);
INT8U Eb1120aEncpytDataAnaly(INT8U* pdata,INT16U lenth,INT8U *dedata,INT16U wChanNo);
int Check1120AIllfgalType(INT8U *ptada,INT8U type,INT16U wChanNo);
INT8U Sgc1120aJudgeWhetherEn(INT8U *pdata,INT16U wChanNo);
INT8U Eb1120aMsgWithSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo);
INT8U Eb1120aMsgUpLoadData(INT8U *pdata);
INT8U Eb1120aMsgWithRandSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo);
INT8U Eb1120aMsgWithTandSData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo);
INT8U Eb1120aMsgWithAllData(INT8U *pdata,INT8U *decpbuf,INT16U wChanNo);
INT16U Pack1120aFor101msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
void Pack1120aFra68ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo);
INT8U SGC1120aVerifyUpLoadData(INT16U wChanNo);
INT16U Sgc1120aPKeyUpload(INT8U *pdata,INT16U len,INT16U wChanNo);
INT8U Check1120aEncrptchip(INT8U CheckType);
INT16U Pack1120afor104msgtoEb(INT8U *buf,INT16U len,INT16U *SEBtaillen,INT16U wChanNo);
void Pack1120aFra104ToEb(INT8U *pdata,INT8U len,INT8U *ebbuf,INT16U wChanNo);
INT8U EncrptyChiptest(INT8U type);
void Packf68ToOld1120aEn(INT8U *oribuf);
int SGCOldPkeyUpdate(INT8U *pdata,INT8U *sdata);
int SGCOldSymkeyUpdate(INT8U *pdata);



#if __cplusplus
}
#endif


#endif

