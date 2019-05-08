#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Objectdef.h"
#include "ParaDef.h"
#include "StdDataType.h"
#include "version.h"

#define 	CODE 					0xa001
#define 	FILENAMELEN				128			//文件名字最大长度
#define 	FILEEXTLEN				5			//文件扩展名最大长度

typedef unsigned char BOOLEAN;
typedef unsigned char INT8U; /* Unsigned  8 bit quantity                           */
typedef signed char INT8S; /* Signed    8 bit quantity                           */
typedef unsigned short INT16U; /* Unsigned 16 bit quantity                           */
typedef signed short INT16S; /* Signed   16 bit quantity                           */
typedef unsigned int INT32U; /* Unsigned 32 bit quantity                           */
typedef signed int INT32S; /* Signed   32 bit quantity                           */
typedef unsigned long long INT64U; /* Unsigned 64 bit quantity   						   */
typedef signed long long INT64S; /* Unsigned 64 bit quantity                           */
typedef float FP32; /* Single precision floating point                    */
typedef double FP64; /* Double precision floating point                    */

typedef struct {
	INT16U oi;  							//对象标识OI
	INT16U interface_len;					//接口类头文件长度
	INT16U unit_len;						//配置单元长度
	INT16U index_site;						//配置序号索引位置
	char logic_name[OCTET_STRING_LEN];	//对象逻辑名字
	char file_name[FILENAMELEN];			//对象保存名字
} CLASS_INFO;

typedef enum {
	coll_bps = 1,
	coll_protocol,
	coll_wiretype,
	task_ti,
	task_cjtype,
	task_prio,
	task_status,
	task_runtime,
	coll_mode,
	ms_type,
	savetime_sel,
} OBJ_ENUM;

int getDataTypeLen(int dt) {
	switch (dt) {
	case dtnull:
		return 0;
	case dtbool:
		return 1;
	case dtdoublelong:
		return 4;
	case dtdoublelongunsigned:
		return 4;
	case dtinteger:
		return 1;
	case dtlong:
		return 2;
	case dtunsigned:
		return 1;
	case dtlongunsigned:
		return 2;
	case dtlong64:
		return 8;
	case dtlong64unsigned:
		return 8;
	case dtenum:
		return 1;
	case dtfloat32:
		return 4;
	case dtfloat64:
		return 8;
	case dtdatetime:
		return 10;
	case dtdate:
		return 5;
	case dttime:
		return 3;
	case dtdatetimes:
		return 7;
	case dtoi:
		return 2;
	case dtoad:
		return 4;
	case dtomd:
		return 4;
	case dtti:
		return 3;
	default:
		return -1;
	}
}

void printMS(MY_MS ms) {
	int i = 0, j = 0;
	int ms_num = 0;
	int seqOfLen = 0;
	int dtlen = 0;

	fprintf(stderr, "电能表集合：MS choice=%d\n", ms.mstype);
	switch (ms.mstype) {
	case 0:
		fprintf(stderr, "无电能表");
		break;
	case 1:
		fprintf(stderr, "全部用户地址");
		break;
	case 2: //一组用户类型
		ms_num = (ms.ms.userType[0] << 8) | ms.ms.userType[1];
		fprintf(stderr, "一组用户类型：个数=%d\n 值=", ms_num);
		for (j = 0; j < ms_num; j++) {
			fprintf(stderr, "%d ", ms.ms.userType[j + 2]);
		}
		fprintf(stderr, "\n");
		break;
	case 3:	//一组用户地址
		ms_num = (ms.ms.userAddr[0].addr[0] << 8) | ms.ms.userAddr[0].addr[1];
		fprintf(stderr, "一组用户地址：个数=%d\n ", ms_num);
		if (ms.ms.configSerial[0] > COLLCLASS_MAXNUM)
			fprintf(stderr, "配置序号 超过限值 %d ,error !!!!!!", COLLCLASS_MAXNUM);
		for (j = 0; j < ms_num; j++) {
			for (i = 0; i < (ms.ms.userAddr[j + 1].addr[0] + 1); i++) {
				fprintf(stderr, "%02x ", ms.ms.userAddr[j + 1].addr[i]);
			}
			fprintf(stderr, "\n");
		}
		break;
	case 4:	//一组配置序号
		fprintf(stderr, "一组配置序号：个数=%d\n ", ms.ms.configSerial[0]);
		if (ms.ms.configSerial[0] > COLLCLASS_MAXNUM)
			fprintf(stderr, "配置序号 超过限值 %d ,error !!!!!!", COLLCLASS_MAXNUM);
		for (i = 0; i < ms.ms.configSerial[0]; i++) {
			fprintf(stderr, "%d ", ms.ms.configSerial[i + 1]);
		}
		fprintf(stderr, "\n");
		break;
	case 5:	//一组用户类型区间
	case 6:	//一组用户地址区间
	case 7:	//一组配置序号区间
		seqOfLen = 0;
		for (i = 0; i < COLLCLASS_MAXNUM; i++) {
			if (ms.ms.type[i].type != interface) {
				dtlen = getDataTypeLen(ms.ms.type[i].begin[0]);
				if (dtlen > 0) {
					seqOfLen++;
					fprintf(stderr,
							"Region:单位[%d](前闭后开:0,前开后闭:1,前闭后闭:2,前闭后闭:3)\n",
							ms.ms.type[i].type);
					fprintf(stderr, "Region:[类型：%02x]起始值 ",
							ms.ms.type[i].begin[0]);
					for (j = 0; j < (dtlen + 1); j++) {
						fprintf(stderr, "%02x ", ms.ms.type[i].begin[j]);
					}
					fprintf(stderr, "\nRegion:[类型：%02x]结束值  ",
							ms.ms.type[i].end[0]);
				}
				dtlen = getDataTypeLen(ms.ms.type[i].end[0]);
				if (dtlen > 0) {
					for (j = 0; j < (dtlen + 1); j++) {
						fprintf(stderr, "%02x ", ms.ms.type[i].end[j]);
					}
				}
			}
		}
		fprintf(stderr, "\n     一组用户类型区间：个数=%d\n ", seqOfLen);
		break;
	}
}

char name1[128] = { };
char *getenum(int type, int val) {
	char *name = NULL;
	name = name1;
	memset(name1, 0, sizeof(name1));
//	fprintf(stderr,"val=%d ,type=%d\n",val,type);
	switch (type) {
	case coll_bps:
		if (val == bps300)
			strcpy(name, "300");
		if (val == bps600)
			strcpy(name, "600");
		if (val == bps1200)
			strcpy(name, "1200");
		if (val == bps2400)
			strcpy(name, "2400");
		if (val == bps4800)
			strcpy(name, "4800");
		if (val == bps7200)
			strcpy(name, "7200");
		if (val == bps9600)
			strcpy(name, "9600");
		if (val == bps19200)
			strcpy(name, "19200");
		if (val == bps38400)
			strcpy(name, "38400");
		if (val == bps57600)
			strcpy(name, "57600");
		if (val == bps115200)
			strcpy(name, "115200");
		if (val == autoa)
			strcpy(name, "自适应");
		break;
	case coll_protocol:
		if (val == 0)
			strcpy(name, "未知");
		if (val == 1)
			strcpy(name, "DL/T645-1997");
		if (val == 2)
			strcpy(name, "DL/T645-2007");
		if (val == 3)
			strcpy(name, "DL/T698.45");
		if (val == 4)
			strcpy(name, "CJ/T18802004");
		break;
	case coll_wiretype:
		if (val == 0)
			strcpy(name, "未知");
		if (val == 1)
			strcpy(name, "单相");
		if (val == 2)
			strcpy(name, "三相三线");
		if (val == 3)
			strcpy(name, "三相四线");
		break;
	case task_ti:
		if (val == 0)
			strcpy(name, "秒");
		if (val == 1)
			strcpy(name, "分");
		if (val == 2)
			strcpy(name, "时");
		if (val == 3)
			strcpy(name, "日");
		if (val == 4)
			strcpy(name, "月");
		if (val == 5)
			strcpy(name, "年");
		break;
	case task_cjtype:
		if (val == 1)
			strcpy(name, "普通采集方案");
		if (val == 2)
			strcpy(name, "事件采集方案");
		if (val == 3)
			strcpy(name, "透明方案");
		if (val == 4)
			strcpy(name, "上报方案");
		if (val == 5)
			strcpy(name, "脚本方案");
		break;
	case task_prio:
		if (val == 0)
			strcpy(name, "0");
		if (val == 1)
			strcpy(name, "首要");
		if (val == 2)
			strcpy(name, "必要");
		if (val == 3)
			strcpy(name, "需要");
		if (val == 4)
			strcpy(name, "可能");
		break;
	case task_status:
		if (val == 1)
			strcpy(name, "正常");
		if (val == 2)
			strcpy(name, "停用");
		break;
	case task_runtime:
		if (val == 0)
			strcpy(name, "前闭后开");
		if (val == 1)
			strcpy(name, "前开后闭");
		if (val == 2)
			strcpy(name, "前闭后闭");
		if (val == 3)
			strcpy(name, "前开后开");
		break;
	case coll_mode:
		if (val == 0)
			strcpy(name, "采集当前数据");
		if (val == 1)
			strcpy(name, "采集上第N次");
		if (val == 2)
			strcpy(name, "按冻结时标采集");
		if (val == 3)
			strcpy(name, "按时间间隔采集");
		if (val == 4)
			strcpy(name, "补抄");
		break;
	case ms_type:
		if (val == 0)
			strcpy(name, "无电能表");
		if (val == 1)
			strcpy(name, "全部用户地址");
		if (val == 2)
			strcpy(name, "一组用户类型");
		if (val == 3)
			strcpy(name, "一组用户地址");
		if (val == 4)
			strcpy(name, "一组配置序号");
		if (val == 5)
			strcpy(name, "一组用户类型区间");
		if (val == 6)
			strcpy(name, "一组用户地址区间");
		if (val == 7)
			strcpy(name, "一组配置序号区间");
		break;
	case savetime_sel:
		if (val == 0)
			strcpy(name, "未定义");
		if (val == 1)
			strcpy(name, "任务开始时间");
		if (val == 2)
			strcpy(name, "相对当日0点0分");
		if (val == 3)
			strcpy(name, "相对上日23点59分");
		if (val == 4)
			strcpy(name, "相对上日0点0分");
		if (val == 5)
			strcpy(name, "相对当月1日0点0分");
		if (val == 6)
			strcpy(name, "数据冻结时标");
		if (val == 7)
			strcpy(name, "相对上月月末0点0分");
		break;
	}
	return name;
}

void printDataTimeS(char *pro, DateTimeBCD datetimes) {
	fprintf(stderr, "[%s]: %04d-%02d-%02d %02d:%02d:%02d\n", pro,
			datetimes.year.data, datetimes.month.data, datetimes.day.data,
			datetimes.hour.data, datetimes.min.data, datetimes.sec.data);
}

void printTI(char *pro, TI ti) {
	fprintf(stderr, "[%s]:单位(%d)-间隔值(%d)  [秒:0,分:1,时:2,日:3,月:4,年:5]\n", pro,
			ti.units, ti.interval);
}

void printTSA(TSA tsa) {
	int j = 0;
	fprintf(stderr, "%d-%d-", tsa.addr[0], tsa.addr[1]);
	if (tsa.addr[0] > TSA_LEN)
		fprintf(stderr, "TSA 长度[%d]超过17个字节，错误！！！\n", tsa.addr[0]);
	for (j = 0; j < (tsa.addr[1] + 1); j++) {
		fprintf(stderr, "%02x", tsa.addr[j + 2]);
	}
	fprintf(stderr, "\n");
}

void print_road(ROAD road) {
	int w = 0;

	fprintf(stderr, "ROAD:%04x-%02x%02x ", road.oad.OI, road.oad.attflg,
			road.oad.attrindex);
	if (road.num >= ROAD_OADS_NUM) {
		fprintf(stderr, "csd overvalue 16 error\n");
		return;
	}

	for (w = 0; w < road.num; w++)
		fprintf(stderr, "<关联OAD..%d>%04x-%02x%02x ", w, road.oads[w].OI,
				road.oads[w].attflg, road.oads[w].attrindex);

	fprintf(stderr, "\n");
}

void print_rcsd(CSD_ARRAYTYPE csds) {
	int i = 0;
	for (i = 0; i < csds.num; i++) {
		if (csds.csd[i].type == 0) {
			fprintf(stderr, "<%d>OAD%04x-%02x%02x ", i, csds.csd[i].csd.oad.OI,
					csds.csd[i].csd.oad.attflg, csds.csd[i].csd.oad.attrindex);
		} else if (csds.csd[i].type == 1) {
			fprintf(stderr, "<%d>", i);
			print_road(csds.csd[i].csd.road);
		}
	}
}

void print_rsd(INT8U choice, RSD rsd) {
	fprintf(stderr, "RSD:choice=%d\n", choice);
	switch (choice) {
	case 8:
		printDataTimeS("采集成功时间起始值", rsd.selec8.collect_succ_star);
		printDataTimeS("采集成功时间结束值", rsd.selec8.collect_succ_finish);
		printTI("上报响应超时时间", rsd.selec8.ti);
		printMS(rsd.selec8.meters);
		break;
	case 10:
		fprintf(stderr, "Select10为指定选取最新的 %d 条记录:\n", rsd.selec10.recordn);
		printMS(rsd.selec10.meters);
		break;
	}
}

long getFileRecordNum(char* fname) {
	int blknum = 0, sizenew = 0;
	int unit_len = sizeof(CLASS_6001);
	int interface_len = sizeof(CLASS11);

	long filesize = 0;

	if (unit_len % 4 == 0)
		sizenew = unit_len + 2;
	else
		sizenew = unit_len + (4 - unit_len % 4) + 2;

	FILE *fp = fopen(fname, "rb");
	if (fp == NULL) {
		fprintf(stderr, "ERROR: Open file %s failed.\n", fname);
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	filesize = ftell(fp);
	fclose(fp);

	blknum = (filesize - interface_len) / sizenew;

	if ((filesize - interface_len) % sizenew != 0) {
		fprintf(stderr, "采集档案表不是整数，检查文件完整性！！！ %ld-%d=%d\n", filesize,
				interface_len, sizenew);
		return -2;
	}
	return blknum;
}

INT16U crc(INT16U Data) {
	unsigned char n = 0;
	INT16U Parity = 0;

	Parity = Data;
	for (n = 0; n < 8; n++) {
		if ((Parity & 0x1) == 0x1) {
			Parity = Parity >> 1;
			Parity = Parity ^ 0xA001;
		} else {
			Parity = Parity >> 1;
		}
	}
	return (Parity);
}

INT16U make_parity(void *source, int size) {
	int m = 0;
	INT16U Parity = 0xffff;
	unsigned char *buf = (unsigned char *) source;

	for (m = 0; m < (size - 2); m++) {
		Parity = Parity ^ buf[m];
		Parity = crc(Parity);
	}

	return Parity;
}

INT8U file_read(char *FileName, void *source, int size, int offset,
		INT16U *retcrc) {
	FILE *fp = NULL;
	int num = 0, ret = 0;
	INT16U readcrc = 0;	//=(INT16U *)((INT8U*)source+size-2);

	fp = fopen(FileName, "r");
	if (fp != NULL) {
		fseek(fp, offset, SEEK_SET);
		num = fread(source, 1, size - 2, fp);
		fread(&readcrc, 1, 2, fp);

		if (num == (size - 2)) {            //读取了size字节数据
			INT16U crc = make_parity(source, size);

			if (crc == readcrc) {

				*retcrc = readcrc;
				ret = 1;
			} else {
				//null数据会计算crc=0x2664，不是错误
				ret = 0;
			}
		}
		fclose(fp);
	} else {
		ret = 0;
	}
	return ret;
}

INT8U file_write(char *FileName, void *source, int size, int offset) {
	FILE *fp = NULL;
	int fd = 0;
	INT8U res = 0;
	int num = 0;
	INT8U *blockdata = NULL;
	INT16U readcrc = 0;

	blockdata = malloc(size);

	if (blockdata != NULL) {

		memset(blockdata, 0, size);
		memcpy(blockdata, source, size - 2);
	} else {
		return 0;            //error
	}
	readcrc = make_parity(source, size);            //计算crc16校验

	memcpy(blockdata + size - 2, &readcrc, 2);
	if (access(FileName, F_OK) != 0) {
		fp = fopen((char *) FileName, "w+");
	} else {
		fp = fopen((char *) FileName, "r+");

	}
	if (fp != NULL) {
		setbuf(fp, NULL);        //不使用cache缓存空间
		fseek(fp, offset, SEEK_SET);

		num = fwrite(blockdata, size, 1, fp);
		fflush(fp);
		fclose(fp);

		if (num == 1) {

			res = 1;
		} else
			res = 0;
	} else {

		res = 0;
	}

	free(blockdata);        //add by nl1031
	blockdata = NULL;
	return res;
}

INT8U block_file_sync(char *fname, void *blockdata, int size, int headsize,
		int index) {
	INT8U ret1 = 0, ret2 = 0;
	int sizenew = 0;
	long offset = 0;
	void *blockdata1 = NULL;
	void *blockdata2 = NULL;
	struct stat info1 = { }, info2 = { };
	char fname2[FILENAMELEN] = { };
	INT16U *readcrc1 = NULL;
	INT16U *readcrc2 = NULL;
	INT16U ret = 0;

	if (fname == NULL || strlen(fname) <= 4 || size < 2) {
		return 0;
	}

	//文件默认最后两个字节为CRC16校验，原结构体尺寸如果不是4个字节对齐，进行补齐，加CRC16
	if (size % 4 == 0)
		sizenew = size + 2;
	else
		sizenew = size + (4 - size % 4) + 2;

	blockdata1 = malloc(sizenew);
	blockdata2 = malloc(sizenew);
	memset(blockdata1, 0, sizenew);
	memset(blockdata2, 0, sizenew);

	if (blockdata1 == NULL || blockdata2 == NULL) {
		if (blockdata1 != NULL) {
			free(blockdata1);
			blockdata1 = NULL;
		}
		if (blockdata2 != NULL) {
			free(blockdata2);
			blockdata2 = NULL;
		}

		return 0;
	}

	readcrc1 = (INT16U *) ((INT8U *) blockdata1 + sizenew - 2);
	readcrc2 = (INT16U *) ((INT8U *) blockdata2 + sizenew - 2);

	memset(fname2, 0, sizeof(fname2));
	strncpy(fname2, fname, strlen(fname) - 4);
	strcat(fname2, ".bak");

	offset = headsize + sizenew * index;
	ret1 = file_read(fname, blockdata1, sizenew, offset, readcrc1);
	ret2 = file_read(fname2, blockdata2, sizenew, offset, readcrc2);

	if ((*readcrc1 == *readcrc2) && (ret1 == 1) && (ret2 == 1)) { //两个文件校验正确，并且校验码相等，返回 1

		ret = 1;
	}
	if ((*readcrc1 != *readcrc2) && (ret1 == 1) && (ret2 == 1)) { //两个文件校验正确，但是校验码不等，采用文件保存日期新的数据

		stat(fname, &info1);
		stat(fname2, &info2);

		file_write(fname2, blockdata1, sizenew, offset);
		ret = 1;

	}
	if ((ret1 == 1) && (ret2 == 0)) {       //fname1校验正确，fname2校验错误,更新fname2备份文件
		file_write(fname2, blockdata1, sizenew, offset);
		ret = 1;
	}
	if ((ret1 == 0) && (ret2 == 1)) {        //fname2校验正确，fname1校验错误,更新fname1源文件
		fprintf(stderr, "主文件校验错误\n");
		file_write(fname, blockdata2, sizenew, offset);
		memcpy(blockdata1, blockdata2, sizenew);
		ret = 1;
	}

	if (ret == 1) {
		memcpy(blockdata, blockdata1, size);
	}
	free(blockdata1);
	free(blockdata2);
	blockdata1 = NULL;
	blockdata2 = NULL;

	return ret;
}

void print6000(char* fname) {
	CLASS11 coll = { };
	CLASS_6001 meter = { };
	int i = 0, j = 0, blknum = getFileRecordNum(fname);
	FILE* fp = NULL;

	fp = fopen(fname, "rb");
	if (fp == NULL)
		return;
	fread(&coll, sizeof(coll), 1, fp);
	fclose(fp);

	fprintf(stderr, "采集档案配置表CLASS_11--------------");
	fprintf(stderr, "逻辑名:%s    当前=%d     最大=%d\n", coll.logic_name,
			coll.curr_num, coll.max_num);

	fprintf(stderr, "采集档案配置单元文件记录个数：【%d】\n", blknum);
	fprintf(stderr,
			"基本信息:[1]通信地址  [2]波特率  [3]规约  [4]端口OAD  [5]通信密码  [6]费率个数  [7]用户类型  [8]接线方式  [9]额定电压  [10]额定电流 \n");
	fprintf(stderr, "扩展信息:[11]采集器地址 [12]资产号 [13]PT [14]CT\n");
	fprintf(stderr, "附属信息:[15]对象属性OAD  [16]属性值\n");
	for (i = 0; i < blknum; i++) {
		if (block_file_sync(fname, &meter, sizeof(CLASS_6001), sizeof(CLASS11),
				i) == 1) {
			if (meter.sernum != 0 && meter.sernum != 0xffff) {
				fprintf(stderr, "\n序号:%d ", meter.sernum);
				fprintf(stderr, "[1]%d-%d-", meter.basicinfo.addr.addr[0],
						meter.basicinfo.addr.addr[1]);
				if (meter.basicinfo.addr.addr[0] > TSA_LEN)
					fprintf(stderr, "TSA 长度[%d]超过17个字节，错误！！！\n",
							meter.basicinfo.addr.addr[0]);
				for (j = 0; j < (meter.basicinfo.addr.addr[1] + 1); j++) {
					fprintf(stderr, "%02x", meter.basicinfo.addr.addr[j + 2]);
				}
				fprintf(stderr, " [2]%s(%d) ",
						getenum(coll_bps, meter.basicinfo.baud),
						meter.basicinfo.baud);
				fprintf(stderr, "[3]%s ",
						getenum(coll_protocol, meter.basicinfo.protocol));
				fprintf(stderr, "[4]%04X_%02X%02X ", meter.basicinfo.port.OI,
						meter.basicinfo.port.attflg,
						meter.basicinfo.port.attrindex);
				fprintf(stderr, "[5]");
				for (j = 0; j < meter.basicinfo.pwd[0]; j++) {
					fprintf(stderr, "%02x", meter.basicinfo.pwd[j + 1]);
				}
				fprintf(stderr, "[6]%d [7]%02x ", meter.basicinfo.ratenum,
						meter.basicinfo.usrtype);
				fprintf(stderr, "[8]%s ",
						getenum(coll_wiretype, meter.basicinfo.connectype));
				fprintf(stderr, "[9]%d [10]%d ", meter.basicinfo.ratedU,
						meter.basicinfo.ratedI);
				fprintf(stderr, "[11]");
				for (j = 0; j < (meter.extinfo.cjq_addr.addr[1] + 1); j++) {
					fprintf(stderr, "%02x", meter.extinfo.cjq_addr.addr[j + 2]);
				}
				fprintf(stderr, " [12]");
				for (j = 0; j < meter.extinfo.asset_code[0]; j++) {
					fprintf(stderr, "%02x", meter.extinfo.asset_code[j + 1]);
				}
				fprintf(stderr, " [13]%d [14]%d", meter.extinfo.pt,
						meter.extinfo.ct);
				fprintf(stderr, " [15]%04X_%02X%02X", meter.aninfo.oad.OI,
						meter.aninfo.oad.attflg, meter.aninfo.oad.attrindex);
			}
		}
	}
	fprintf(stderr, "\n");
}

void print6013(char* fname) {
	INT8U i = 0;
	CLASS_6013 class6013 = {};
	FILE* fp = fopen(fname, "rb");

	if (NULL == fp) {
		printf("[%s()][%d]open file error\n", __FUNCTION__, __LINE__);
	}
	fread(&class6013, sizeof(class6013), 1, fp);

	fprintf(stderr,
			"\n\n[1]执行频率 [2]方案类型 [3]方案编号 [4]开始时间 [5]结束时间 [6]延时 [7]执行优先级 [8]状态 [9]开始前脚本id [10]开始后脚本id [11]运行时段【起始HH:MM 结束HH:MM】\n");
	fprintf(stderr, "【6013】任务配置单元: 任务ID--%d\n", class6013.taskID);
	fprintf(stderr, "       [1]%s-%d ",
			getenum(task_ti, class6013.interval.units),
			class6013.interval.interval);
	fprintf(stderr, "[2]%s  [3]%d   ", getenum(task_cjtype, class6013.cjtype),
			class6013.sernum);
	fprintf(stderr, "[4]%d-%d-%d %d:%d:%d ", class6013.startime.year.data,
			class6013.startime.month.data, class6013.startime.day.data,
			class6013.startime.hour.data, class6013.startime.min.data,
			class6013.startime.sec.data);
	fprintf(stderr, "[5]%d-%d-%d %d:%d:%d ", class6013.endtime.year.data,
			class6013.endtime.month.data, class6013.endtime.day.data,
			class6013.endtime.hour.data, class6013.endtime.min.data,
			class6013.endtime.sec.data);
	fprintf(stderr, "[6]%s-%d ", getenum(task_ti, class6013.delay.units),
			class6013.delay.interval);
	fprintf(stderr, "[7]%s  ", getenum(task_prio, class6013.runprio));
	fprintf(stderr, "[8]%s  [9]%d  [10]%d ",
			getenum(task_status, class6013.state), class6013.befscript,
			class6013.aftscript);
	fprintf(stderr, "[11]%s ", getenum(task_runtime, class6013.runtime.type));
	fprintf(stderr, "运行时段:%d", class6013.runtime.num);
	for (i = 0; i < class6013.runtime.num; i++) {
		fprintf(stderr, "[%d:%d %d:%d] ",
				class6013.runtime.runtime[i].beginHour,
				class6013.runtime.runtime[i].beginMin,
				class6013.runtime.runtime[i].endHour,
				class6013.runtime.runtime[i].endMin);
	}
	fprintf(stderr, "\n");
}

void print6015(char* fname) {
	INT8U type = 0, w = 0, i = 0;
	CLASS_6015 class6015 = {};
	FILE* fp = fopen(fname, "rb");

	if (NULL == fp) {
		printf("[%s()][%d]open file error\n", __FUNCTION__, __LINE__);
	}
	fread(&class6015, sizeof(class6015), 1, fp);

	fprintf(stderr,
			"\n\n[1]方案编号 [2]存储深度 [3]采集类型 [4]采集内容 [5]OAD-ROAD [6]MS [7]存储时标\n");
	fprintf(stderr, "[6015]普通采集方案:[1]方案号: %d  \n", class6015.sernum);
	fprintf(stderr, "     [2]%d  [3]%s ", class6015.deepsize,
			getenum(coll_mode, class6015.cjtype));
	switch (class6015.cjtype) {
	case 0: // NULL
		fprintf(stderr, "[4]%02x ", class6015.data.data[0]);
		break;
	case 1:	//unsigned
		fprintf(stderr, "[4]%02x ", class6015.data.data[0]);
		break;
	case 2:	// NULL
		fprintf(stderr, "[4]%02x ", class6015.data.data[0]);
		break;
	case 3:	//TI
		fprintf(stderr, "[4]%s-%d ", getenum(task_ti, class6015.data.data[0]),
				((class6015.data.data[1] << 8) | class6015.data.data[2]));
		break;
	case 4:	//RetryMetering
		fprintf(stderr, "[4]%s-%d %d\n",
				getenum(task_ti, class6015.data.data[0]),
				((class6015.data.data[1] << 8) | class6015.data.data[2]),
				((class6015.data.data[3] << 8) | class6015.data.data[4]));
		break;
	}
	if (class6015.csds.num >= MY_CSD_NUM) {
		fprintf(stderr, "csd overvalue MY_CSD_NUM error\n");
		return;
	}
	fprintf(stderr, "[5]");
	for (i = 0; i < class6015.csds.num; i++) {
		type = class6015.csds.csd[i].type;
		if (type == 0) {
			fprintf(stderr, "<%d>OAD%04x-%02x%02x ", i,
					class6015.csds.csd[i].csd.oad.OI,
					class6015.csds.csd[i].csd.oad.attflg,
					class6015.csds.csd[i].csd.oad.attrindex);
		} else if (type == 1) {
			fprintf(stderr, "<%d>ROAD%04x-%02x%02x ", i,
					class6015.csds.csd[i].csd.road.oad.OI,
					class6015.csds.csd[i].csd.road.oad.attflg,
					class6015.csds.csd[i].csd.road.oad.attrindex);
			if (class6015.csds.csd[i].csd.road.num >= 16) {
				fprintf(stderr, "csd overvalue 16 error\n");
				return;
			}
//			fprintf(stderr,"csds.num=%d\n",class6015.csds.num);
			for (w = 0; w < class6015.csds.csd[i].csd.road.num; w++) {
				fprintf(stderr, "<..%d>%04x-%02x%02x ", w,
						class6015.csds.csd[i].csd.road.oads[w].OI,
						class6015.csds.csd[i].csd.road.oads[w].attflg,
						class6015.csds.csd[i].csd.road.oads[w].attrindex);
			}
		}
	}
	fprintf(stderr, "[6]%s ", getenum(ms_type, class6015.mst.mstype));
	printMS(class6015.mst);
	fprintf(stderr, "[7]%s ", getenum(savetime_sel, class6015.savetimeflag));
	fprintf(stderr, "\n");

}

void print601d(char* fname) {
	int j = 0;
	CLASS_601D reportplan = {};
	FILE* fp = fopen(fname, "rb");

	if (NULL == fp) {
		printf("[%s()][%d]open file error\n", __FUNCTION__, __LINE__);
	}
	fread(&reportplan, sizeof(reportplan), 1, fp);
	fprintf(stderr,
			"\n\n[1]上报方案编号 [2]上报通道 [3]上报响应超时时间 [4]最大上报次数 [5]上报内容 {[5.1]类型(0:OAD,1:RecordData) [5.2]数据 [5.3]RSD}");
	fprintf(stderr, "\n[1]上报方案编号:%d \n", reportplan.reportnum);
	fprintf(stderr, "[2]OAD[%d] ", reportplan.chann_oad.num);
	for (j = 0; j < reportplan.chann_oad.num; j++) {
		fprintf(stderr, "%04x-%02x%02x ", reportplan.chann_oad.oadarr[j].OI,
				reportplan.chann_oad.oadarr[j].attflg,
				reportplan.chann_oad.oadarr[j].attrindex);
	}
	fprintf(stderr, " [3]TI %d-%d ", reportplan.timeout.units,
			reportplan.timeout.interval);
	fprintf(stderr, " [4]%d ", reportplan.maxreportnum);
	fprintf(stderr, " [5.1]%d ", reportplan.reportdata.type);
	if (reportplan.reportdata.type == 0) {
		fprintf(stderr, " [5.2]OAD:%04x-%02x%02x ",
				reportplan.reportdata.data.oad.OI,
				reportplan.reportdata.data.oad.attflg,
				reportplan.reportdata.data.oad.attrindex);
	} else {
		fprintf(stderr, " [5.2]OAD:%04x-%02x%02x ",
				reportplan.reportdata.data.oad.OI,
				reportplan.reportdata.data.oad.attflg,
				reportplan.reportdata.data.oad.attrindex);
		print_rcsd(reportplan.reportdata.data.recorddata.csds);
		fprintf(stderr, " [5.4]");
		print_rsd(reportplan.reportdata.data.recorddata.selectType,
				reportplan.reportdata.data.recorddata.rsd);
	}
	fprintf(stderr, "\n\n");
}

int main(int argc, char** argv) {
	OI_698 oi = 0;

	if (3 == argc) {
		sscanf(argv[1], "%04x", &oi);
		switch (oi) {
		case 0x6000:
			print6000(argv[2]);
			break;
		case 0x6013:
			print6013(argv[2]);
			break;
		case 0x6015:
			print6015(argv[2]);
			break;
		case 0x601d:
			print601d(argv[2]);
			break;
		default:
			break;
		}
	} else {
		printf("usage: cj io filename\n");
	}

	return 0;
}
