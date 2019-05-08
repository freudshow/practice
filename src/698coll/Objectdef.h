/*
 * ObjectAction.h
 *
 *  Created on: Nov 12, 2016
 *      Author: ava
 */

#ifndef OBJECTACTION_H_
#define OBJECTACTION_H_

#include <time.h>
#include "ParaDef.h"
#include "StdDataType.h"

//#pragma pack(1)				//结构体一个字节对齐
/////////////////////////////////////////////////////////////////////////
/********************************************************
 *				接口类公共属性结构
 ********************************************************/
typedef struct {
    INT8U logic_name[OCTET_STRING_LEN]; //逻辑名
    INT16U curr_num;                    //当前元素个数
    INT16U max_num;                     //最大元素个数
} CLASS11;                              //集合接口类

typedef struct {
} CLASS14; //区间统计接口类

typedef struct {
    INT8U logic_name[OCTET_STRING_LEN]; //逻辑名
    char  source_file[VISIBLE_STRING_LEN];	//源文件
    char  dist_file[VISIBLE_STRING_LEN];	//目标文件
    INT32U	file_size;						//文件大小
    INT8U	file_attr;						//文件属性 bit0:读(1:可读,0:不可读),bit1:写,bit2:执行
    char   file_version[VISIBLE_STRING_LEN];	//文件版本
    FILE_Type	file_type;					//文件类别
    INT8U   cmd_result;						//命令结果:最近一次传输或执行结果的状态信息,具体见规约定义
} CLASS18; //文件传输接口类                             //输入输出设备接口类

typedef struct {
    char factoryCode[4];    //厂商代码
    char softVer[4];        //软件版本号
    char softDate[6];       //软件版本日期
    char hardVer[4];        //硬件版本号
    char hardDate[6];       //硬件版本日期
    char factoryExpInfo[8]; //厂家扩展信息
} VERINFO;

typedef struct {
    char name[OCTET_STRING_LEN];      //逻辑名
    char devdesc[VISIBLE_STRING_LEN]; //设备描述符
    VERINFO info;                     //版本信息
    DateTimeBCD date_Product;         //生产日期
    OI_698 ois[10];                   //子设备列表
    char protcol[OCTET_STRING_LEN];   //支持的规约列表
    INT8U follow_report;              //是否允许跟随上报
    INT8U active_report;              //是否允许主动上报
    INT8U talk_master;                //是否允许与主站通话
    OAD oads[10];                     //上报通道
} CLASS19;                            //设备管理接口类

typedef struct {
    INT8U logic_name[OCTET_STRING_LEN]; //逻辑名
    INT16U device_num;                  //设备对象数量
} CLASS22;                              //输入输出设备接口类

typedef struct {
    INT8U workModel;                    //工作模式 enum{混合模式(0),客户机模式(1),服务器模式(2)},
    INT8U onlineType;                   //在线方式 enum{永久在线(0),被动激活(1)}
    INT8U connectType;                  //连接方式 enum{TCP(0),UDP(1)}
    INT8U appConnectType;               //连接应用方式 enum{主备模式(0),多连接模式(1)}
    INT16U listenPortnum;               //端口数量
    INT16U listenPort[5];               //侦听端口列表
    INT8U apn[VISIBLE_STRING_LEN];      // apn
    INT8U userName[VISIBLE_STRING_LEN]; //用户名称
    INT8U passWord[VISIBLE_STRING_LEN]; //密码
    INT8U proxyIp[OCTET_STRING_LEN];    //代理服务器地址
    INT16U proxyPort;                   //代理端口
    INT8U timeoutRtry;                  //超时时间，重发次数
    INT16U heartBeat;                   //心跳周期秒
} COMM_CONFIG_1;
typedef struct {
    INT8U workModel;                 //工作模式 enum{混合模式(0),客户机模式(1),服务器模式(2)},
    INT8U connectType;               //连接方式 enum{TCP(0),UDP(1)}
    INT8U appConnectType;            //连接应用方式 enum{主备模式(0),多连接模式(1)}
    INT16U listenPortnum;            //侦听端口数量
    INT16U listenPort[5];            //侦听端口列表
    INT8U proxyIp[OCTET_STRING_LEN]; //代理服务器地址
    INT16U proxyPort;                //代理端口
    INT8U timeoutRtry;               //超时时间，重发次数
    INT16U heartBeat;                //心跳周期秒
} COMM_CONFIG_2;

typedef struct {
    INT8U ip[OCTET_STRING_LEN]; //主站 IP 192.168.000.001
    INT16U port;                //端口
} MASTER_STATION_INFO;
typedef struct {
    INT16U masternum;
    MASTER_STATION_INFO master[4];
} MASTER_STATION_INFO_LIST;

typedef struct {
    char center[VISIBLE_STRING_LEN];    //短信中心号码
    INT8U masternum;                        //支持主站号码总数
    char master[4][VISIBLE_STRING_LEN]; //主站号码
    INT8U destnum;                        //支持短信通知目的号码总数
    char dest[4][VISIBLE_STRING_LEN];   //短信通知目的号码
} SMS_INFO;

typedef struct {
    INT8U name[OCTET_STRING_LEN];          //逻辑名
    COMM_CONFIG_1 commconfig;              //通信配置
    MASTER_STATION_INFO_LIST master;       //主站通信参数表
    SMS_INFO sms;                          //短信通信参数表
    VERINFO info;                          //版本信息
    INT8U protocolnum;                        //支持规约总数
    INT8U protcol[10][VISIBLE_STRING_LEN]; //支持的规约列表
    INT8U ccid[VISIBLE_STRING_LEN];        // SIM卡CCID
    INT8U imsi[VISIBLE_STRING_LEN];        // SIM卡IMSI
    INT16U signalStrength;                 //信号强度
    INT8U simkard[VISIBLE_STRING_LEN];      // SIM卡号码
    INT8U pppip[OCTET_STRING_LEN];         //拨号IP
} CLASS25;                                 // 4500、4501公网通信模块1，2

typedef struct {
    INT8U ipConfigType;                       // IP 配置方式 enum{DHCP(0),静态(1),PPPoE(2)} ,扩展gprs-pppd（3），此时默认不设置route
    INT8U ip[OCTET_STRING_LEN];               // IP
    INT8U subnet_mask[OCTET_STRING_LEN];      //子网掩码
    INT8U gateway[OCTET_STRING_LEN];          //网关
    INT8U username_pppoe[VISIBLE_STRING_LEN]; // PPPOE用户名   sohu.com@yaxinli.com.cn
    INT8U password_pppoe[VISIBLE_STRING_LEN]; // PPPOE密码
} NETCONFIG;

typedef struct {
    INT8U name[OCTET_STRING_LEN];    //逻辑名
    COMM_CONFIG_2 commconfig;        //通信配置
    MASTER_STATION_INFO_LIST master; //主站通信参数
    NETCONFIG IP;                    //终端IP
    INT8U mac[OCTET_STRING_LEN];     // MAC地址
} CLASS26;                           //以太网通信接口类

/////////////////////////////////////////////////////////////////////////////
/********************************************************
 *				 A.3 变量类对象
 ********************************************************/
typedef struct {
    INT32U monitorTime;            //电压监测时间
    INT16U passRate;            //电压合格率
    INT16U overRate;            //电压超限率
    INT32U upLimitTime;          //超上限时间
    INT32U downLimitTime;        //超下限时间
} PassRate_U;//电压统计结果

typedef struct {
	PassRate_U	dayu_tj;
	PassRate_U	monthu_tj;
}Volt_PassRate_tj;	//电压合格率统计

typedef struct {
    INT32U day_tj;
    INT32U month_tj;
} Day_Mon_TJ; //日月统计值

typedef struct {
    Day_Mon_TJ flow;
} Flow_tj; // 2200	通信流量统计

typedef struct {
    Day_Mon_TJ gongdian;
    TS ts;
} Gongdian_tj; // 2203 供电时间统计

typedef struct {
    Day_Mon_TJ reset;
    TS ts;
} Reset_tj; // 2204 //复位次数统计

/////////////////////////////////////////////////////////////////////////////
/********************************************************
 *				 A.5 参变量类对象
 ********************************************************/
typedef struct {
    DateTimeBCD datetime; //属性2
    INT8U type;           //校时模式
    INT8U hearbeatnum;    //心跳时间总个数
    INT8U tichu_max;      //最大剔除个数
    INT8U tichu_min;      //最小剔除个数
    INT8U delay;          //通讯延时阈(yu-四声, 意为门槛)值
    INT8U num_min;        //最少有效个数
} CLASS_4000;             //日期时间

typedef struct {
    INT8U login_name[OCTET_STRING_LEN];  //逻辑名
    INT8U curstom_num[OCTET_STRING_LEN]; //客户编号
} CLASS_4001_4002_4003;                  // 4001:通信地址，4002：表号，4003：客户编号

typedef struct {
    INT8U fangwei;
    INT8U du;
    INT8U fen;
    INT8U miao;
} TYPE_JWD;
typedef struct {
    TYPE_JWD jing;
    TYPE_JWD wei;
    INT32U heigh;
} CLASS_4004;

typedef struct{
	INT8U num; //组地址数量
	INT8U addr[20][OCTET_STRING_LEN];//最多20个 每个最大长度16个字节，第一个字节为长度
} CLASS_4005;
typedef struct {
    INT8U clocksource;
    INT8U state;
} CLASS_4006;

typedef struct {
    INT8U poweon_showtime;   //上电全显时间
    INT16U lcdlight_time;    //背光点亮时间（按键）
    INT16U looklight_time;   //背光点亮时间(查看)
    INT16U poweron_maxtime;  //有电按键屏幕驻留时间(查看)
    INT16U poweroff_maxtime; //无电按键屏幕驻留时间(查看)
    INT8U energydata_dec;    //显示电能小数位
    INT8U powerdata_dec;     //显示功率小数位
} CLASS_4007;
typedef struct {
	DateTimeBCD datetime_s;//切换时间
} CLASS_4008_4009_400A_400B;

typedef struct {
    INT8U 	year_zone;   		//年时区数p<=14
    INT8U 	day_interval;    	//日时段表数q<=8
    INT8U   day_change;  	 	//日时段数（每日切换数）m<=14
    INT8U 	rate;  				//费率数 k<=63
    INT8U 	public_holiday;  	//公共假日数	n<=254
} CLASS_400C;

typedef struct {
	INT8U num;					//数量
} CLASS_400D_400E_400F_4010;

typedef struct {
	DateTimeBCD	datetime;		//日期
	INT8U		tableno;		//日时段表号
} HOLIDAY_INFO;
typedef struct {
	INT8U holidaynum;		//公共假日数量（至少要保存20个假日）
	HOLIDAY_INFO holiday[MAX_PUBLIC_HOLIDAY_NUM];	//公共假日
} CLASS_4011;

typedef struct {
	INT8U restdayflag;		//周休日特征字(size(8))（附录G）
} CLASS_4012;

typedef struct {
	INT8U tableno;		//周休日采用的日时段表号
} CLASS_4013;
typedef struct {
    INT8U month;   	//月
    INT8U day;    	//日
    INT8U tableno; 	//日时段表号
} TIME_ZONE;

typedef struct {
	INT8U		zonenum;
	TIME_ZONE	time_zone[MAX_PERIOD_RATE];
} CLASS_4014;

typedef struct {
    INT8U hour;   //时
    INT8U min;    //分
    INT8U rateno; //费率号
} Day_Period;

typedef struct {
    INT8U day_num;	//日时段表
    INT8U zone_num;	//时段数
    Day_Period Period_Rate[MAX_PERIOD_RATE][MAX_PERIOD_RATE];
} CLASS_4016;
typedef struct {
	INT8U 	num; //费率电价个数
	INT64U feilv_price[32]; //费率电价   元/kWh 换算:-4
} CLASS_4018;
typedef struct {
    INT8U month;   	//月
    INT8U day;    	//日
    INT8U hour; 	//日时段表号
} ACCOUNT_DAY;
typedef struct {
	INT32U 	ladder_value[32]; 				//阶梯值     kWh 换算：-2
	INT32U ladder_price[32]; 			//阶梯电价   元/kWh 换算:-4
	ACCOUNT_DAY account_day[32];		//阶梯结算日
} CLASS_401A;
typedef struct {
	INT32U 	k; 		//电压/电流互感器变比 换算：-2
} CLASS_401C_401D;

typedef struct {
	INT32U 	alarm_amount_limit_1; 		//报警金额限值  元 换算：-2
	INT32U 	alarm_amount_limit_2; 		//其他金额限值  元 换算：-2
} CLASS_401E;

typedef struct {
	INT32U 	overdraft_amount_limit; 	//透支金额限值  元 换算：-2
	INT32U 	hoarding_amount_limit; 		//囤积金额限值  元 换算：-2
	INT32U 	switchin_amount_limit; 		//合闸允许金额限值  元 换算：-2
} CLASS_401F;
typedef struct {
	INT32U 	alarm_electricity_limit_1; 	//报警电量限值  元 换算：-2
	INT32U 	alarm_electricity_limit_2; 	//其他电量限值  元 换算：-2
} CLASS_4020;

typedef struct {
	INT32U 	overdraft_electricity_limit; //囤积电量限值  元 换算：-2
	INT32U 	hoarding_electricity_limit; //透支电量限值  元 换算：-2
	INT32U 	switchin_electricity_limit; //合闸允许电量限值  元 换算：-2
} CLASS_4021;

typedef struct {
	INT8U 	card_flag; //插卡状态字(size(16))（附录G）
} CLASS_4022;

typedef struct {
	INT16U 	effective_duration; //认证有效时长  分钟
} CLASS_4023;
typedef struct {
    INT8U tichu;
} CLASS_4024;
typedef struct {
    INT16U uUp;				//电压考核上限  V 换算：-1
    INT16U uDown;			//电压考核下限  V 换算：-1
    INT16U uUp_Kaohe;		//电压合格上限  V 换算：-1
    INT16U uDown_Kaohe;		//电压合格下限  V 换算：-1
} CLASS_4030;				//电压合格率参数

typedef struct {
	INT8U data;		//最大需量时间/滑差时间 分钟 换算：0
					//校表脉冲宽度 毫秒 换算：0
} CLASS_4100_4101_4102;
typedef struct {
    char assetcode[40]; // 0：代表有效长度
} CLASS_4103;           //资产管理编码

typedef struct {
    char ratedU[VISIBLE_STRING_LEN]; 	// 0：代表有效长度
} CLASS_4104;           				//额定电压
typedef struct {
    char ratedI[VISIBLE_STRING_LEN]; 	// 0：代表有效长度
} CLASS_4105; 							//额定电流/基本电流
typedef struct {
    char maxI[VISIBLE_STRING_LEN]; 		// 0：代表有效长度
} CLASS_4106; 							//最大电流

typedef struct {
    char accuracy_class[VISIBLE_STRING_LEN]; 		// 0：代表有效长度
} CLASS_4107_4108; 									//有/无功准确度等级

typedef struct {
    INT32U constant; 		// 电能表有/无功常数
} CLASS_4109_410A;
typedef struct {
    char meter_type[VISIBLE_STRING_LEN]; // 0：代表有效长度
} CLASS_410B;           //电能表型号
typedef struct {
    INT16S ratio_A; // A相系数
    INT16S ratio_B; // B相系数
    INT16S ratio_C; // C相系数
} CLASS_410C_410D_410E_410F;

typedef struct {
	INT8U 	meter_running_character; //电能表运行特征字1(size(8))
} CLASS_4110;
typedef struct {
    char soft_recordnumber[VISIBLE_STRING_LEN]; // 软件备案号
} CLASS_4111;
typedef struct {
	INT8U 	group_character; //有功组合方式特征字1(size(8))(附录G)
							//无功组合方式1/2特征字
} CLASS_4112_4113_4114;

typedef struct {
    INT8U ictype;
} CLASS_4115;

typedef struct {
    INT8U day;    	//日
    INT8U hour; 	//时
} ACCOUNT_DATE;
typedef struct {
	INT8U day_num;	//结算日数
	ACCOUNT_DATE accountdate[];//结算日日期
} CLASS_4116;
typedef struct {
	TI freezeperiod;	//期间需量冻结周期
} CLASS_4117;

typedef struct {
	TSA tsa;	//通信地址
	INT8U father_tsanum;//
	TSA father_tsa[20];//父节点集合
} CLASS_4201;
typedef struct{
	INT8U flag;//级联标志            bool
	OAD oad;//级联通信端口号      OAD
	INT16U total_timeout;//总等待超时（10ms）  long-unsigned
	INT16U byte_timeout;//字节超时（10ms）    long-unsigned
	INT8U resendnum;//重发次数            unsigned
	INT8U cycle;//巡测周期（min）     unsigned
	INT8U portnum;//级联（被）端口数    unsigned
	INT8U tsanum;//
	INT8U tsa[20][17];//级联（被）终端地址  array TSA
}CLASS_4202;

typedef struct {
    INT8U startime[3];  //广播校时启动时间 time类型 octet-string(SIZE(3))
    INT8U enable;       //终端广播校时是否启用
    INT8S upleve;       //时钟误差阀值
    INT8U startime1[3]; //终端单地址广播校时启动时间
    INT8U enable1;      //终端单地址广播校时是否启用
} CLASS_4204;           //终端广播校时

typedef struct{
	INT8U id;
	INT8U visit_authority;
}ONE_METHOD;
typedef struct{
	INT8U pro_num;
	ONE_METHOD property[50];//属性访问权限
	INT8U met_num;
	ONE_METHOD method[50];  //方法访问权限
}AUTHORITY;
typedef struct{
	OI_698 OI;
   AUTHORITY one_authority;
}AUTHORITY_ARR;
typedef struct{
	INT16U xieyi_banben;
	INT16U max_rev_num;
	INT16U max_send_num;
	INT16U al_num;
	INT8U xieyi[8];
	INT8U power[16];
	INT32U static_outtime;
}USE_LAN_INFO;
typedef struct{
	INT8U login_name[OCTET_STRING_LEN];//属性1 逻辑名
	INT8U num; //对象数量
	AUTHORITY_ARR authority[50];//属性2
	USE_LAN_INFO use_lan_info;//属性3
   INT8U custom;//当前客户机地址
   INT8U renzheng;//当前认证机制
}CLASS_4400;
typedef struct {
    char connect_pwd[VISIBLE_STRING_LEN]; // 认证密码
} CLASS_4401;

/********************************************************
 *				A.6　冻结类对象
 ********************************************************/
typedef struct {
    INT16U freezePriod;    //冻结周期
    OAD oad;            //关联对象属性描述符
    INT16U saveDepth;        //存储深度
} Relate_Object;

typedef struct {
    INT8U RelateNum;                //关联属性
    Relate_Object RelateObj[MAX_FREEZE_OBJ];
} FreezeObject;

////////////////////////////////////////////////////////////
/********************************************************
 *				A.7　采集监控类对象
 ********************************************************/
	typedef struct {
		TSA addr;                    //通信地址
		INT8U baud;                  //波特率
		INT8U protocol;              //规约类型
		OAD port;                    //端口
		INT8U pwd[OCTET_STRING_LEN]; //通信密码
		INT8U ratenum;               //费率个数
		INT8U usrtype;               //用户类型
		INT8U connectype;            //接线方式
		INT16U ratedU;               //额定电压
		INT16U ratedI;               //额定电流
	} BASIC_OBJECT;
	typedef struct {
		TSA cjq_addr;                       //采集器地址
		INT8U asset_code[OCTET_STRING_LEN]; //资产号
		INT16U pt;
		INT16U ct;
	} EXTEND_OBJECT;
	typedef struct {
		OAD oad;
		INT8U data[OCTET_STRING_LEN];
	} ANNEX_OBJECT;

typedef struct {
    INT8U name[OCTET_STRING_LEN]; //参数变量接口类逻辑名
    INT16U sernum;                //配置序号
    BASIC_OBJECT basicinfo;       //基本信息
    EXTEND_OBJECT extinfo;        //扩展信息
    ANNEX_OBJECT aninfo;         //附属信息
} CLASS_6001;                     //采集档案配置表对象

	typedef	struct {
		OAD			oad;
		DATA_TYPE	data;
	}AnnexInfo;

	typedef struct {
		TSA		CommAddr;		//通信地址
		TSA		CJQAddr;		//采集器地址
		INT8U	protocol;		//规约类型
		INT8U	phase;			//相位
		INT8U	signal;			//信号品质
		DateTimeBCD		searchTime;	//搜到的时间
		INT8U	annexNum;			//搜到的附加信息个数
		AnnexInfo		annexInfo[SERACH_PARA_NUM];	//搜到的附件信息
	}SearchResult;

	typedef struct{
		TSA		CommAddr;			//通信地址
		TSA		mainPointAddr;		//主节点地址
		DateTimeBCD		changeTime;	//变更时间
	}CrossZoneResult;

	typedef struct{
		INT8U	enablePeriodFlg;		//是否启用每天周期搜表
		INT8U	autoUpdateFlg;			//自动更新采集档案
		INT8U	eventFlg;				//是否产生搜表相关事件
		SearchMeterResult	clearChoice;	//清空搜表结果选项
	}searchAttr8;

	typedef struct{
		INT8U	startTime[3];		//定时搜表开始时间
		INT32U	searchLen;		//搜表时长（min）
	}searchAttr9;

typedef struct {
	SearchResult		searchResult[SERACH_NUM];			//搜表结果
	CrossZoneResult		crosszoneResult[SERACH_NUM];		//跨台区结果
	INT32U			searchNum;			//搜表结果记录数
	INT32U			crosszoneNum;		//跨台区搜表结果记录数
	searchAttr8		attr8;				//属性8
	INT8U			attr9_num;			//定时搜表参数个数
	searchAttr9		attr9[SERACH_PARA_NUM];			//每天周期搜表参数配置
	SearchStatus	searchSta;			//属性10：搜表状态
	INT32U			startSearchLen;		//搜表时长，单位：分钟，方法127：实时启动搜表参数
	INT8U           startSearchFlg;		//启动实时搜表标识   action 127置 1      载波线程负责清零
} CLASS_6002;	//搜表结果


	typedef struct {
		INT8U beginHour;
		INT8U beginMin;
		INT8U endHour;
		INT8U endMin;
	} TIMEPART; //时段

	typedef struct {
		INT8U type; //运行时段类型
		INT8U num;
		TIMEPART runtime[24]; //时段表 0-3分别表示起始小时.分钟，结束小时.分钟
		//runtime[23].beginHour 用来标记任务的执行优先级的数据类型
	} TASK_RUN_TIME;

typedef struct {
    INT8U taskID;          //参数变量接口类逻辑名
    TI interval;           //执行频率
    INT8U cjtype;          //方案类型
    INT8U sernum;          //方案序号
    DateTimeBCD startime;  //开始时间
    DateTimeBCD endtime;   //结束时间
    TI delay;              //延时
//    注意：国网勘误执行优先级由enum改为unsigned类型，为了兼容浙江现场运行已经下发的任务，将runtime[23].beginHour时间来标注类型来区分处理
    INT8U runprio;         //执行优先级，
    INT8U state;           //任务状态
    INT16U befscript;      //任务开始前脚本  //long unsigned
    INT16U aftscript;      //任务完成后脚本  //long unsigned
    TASK_RUN_TIME runtime; //任务运行时段
//    INT8U priotype;			//优先级的类型描述，区别勘误
} CLASS_6013;              //任务配置单元

typedef struct {
    INT8U name[OCTET_STRING_LEN]; //参数变量接口类逻辑名
    INT8U sernum;                 //方案序号
    INT16U deepsize;              //存储深度
    INT8U cjtype;                 //采集类型
    DATA_TYPE data;               //采集内容
    CSD_ARRAYTYPE csds;           //记录列选择 array CSD,
    MY_MS mst;                    //电能表集合
    INT8U savetimeflag;           //存储时标选择 enum
} CLASS_6015;                     //普通采集方案

//原协议内容
//typedef struct {
//    //	INT8U name[OCTET_STRING_LEN];		//参数变量接口类逻辑名
//    INT8U sernum;     //方案序号
//    ARRAY_ROAD roads; //采集的事件数据
//    MY_MS ms;         //采集类型
//    INT8U ifreport;   //上报标识
//    INT16U deepsize;  //存储深度
//} CLASS_6017;         //事件采集方案

	typedef struct {
		INT8U colltype;        //采集类型   0 array ROAD 周期采集事件数据 	1 NULL 根据通知采集所有事件数据 	2 array ROAD 根据通知采集指定事件数据
		ARRAY_ROAD roads;        //采集的事件数据
	} COLL_STYLE;    //采集方式
//勘误修订后
typedef struct {
    //	INT8U name[OCTET_STRING_LEN];		//参数变量接口类逻辑名
    INT8U sernum;            //方案序号
    COLL_STYLE collstyle; //采集方式
    MY_MS ms;         //采集类型
    INT8U ifreport;   //上报标识
    INT16U deepsize;  //存储深度
} CLASS_6017;         //事件采集方案

	typedef struct {
		INT8U 	datano;			//报文序号
		INT8U	data[256];		//报文内容	//data【0】代表后面有效报文内容
	}PLAN_DATA;	//方案报文

	typedef struct {
		INT8U 	featureByte;		//特征字节
		INT32U	interstart;			//截取开始
		INT32U	interlen;			//截取长度
	}RESULT_PARA;	//结果比对参数

	typedef struct {
		INT8U 	waitnext;		//上报透明方案结果并等待后续报文
		INT32U	overtime;		//等待后续报文超时时间（秒）
		INT8U	resultflag;		//结果比对标识{不比对（0），比（1），比对上报（2）}
		RESULT_PARA	resultpara;
	}PLAN_FLAG;	//方案控制标志

	typedef struct {
		INT32U 	seqno;		//序号
		TSA 	addr;       //通信地址
	    INT32U 	befscript;      //任务开始前脚本
	    INT32U 	aftscript;      //任务完成后脚本
	    PLAN_FLAG	planflag;	//方案控制标志
	    INT8U		datanum;	//方案报文集有效个数
	    PLAN_DATA	data[256];	//方案报文集
	}PLAN_CONTENT;

typedef struct {
    INT8U 	planno;            	//方案编号
    INT8U   contentnum;			//内容个数
    PLAN_CONTENT	plan[20];	//方案内容
    INT16U 	savedepth;			//存储深度
} CLASS_6019;         //透明方案


typedef struct {
	INT8U			datano;			//报文序号
	DateTimeBCD		responseTime;	//报文响应时间
	INT8U			data[256];		//命令结果
}a601BRes;//一个601B结果

typedef struct {
    INT8U			planno;  	//方案编号
    DateTimeBCD		execTime;	//方案执行时间
    TSA				tsa;		//通信地址
    INT8U			resNum;		//结果个数
    a601BRes		resArray[20];//结果集
} CLASS_601B;         //透明方案

typedef struct {
	CLASS11 class11;
	INT8U	num;
	CLASS_601B class601bArray[20];
}CLASS_601A;

typedef enum {
	unknow,	//未知
	phaseA,	//A相
	phaseB,	//B相
	phaseC	//C相
}phase_e;

typedef enum {
	normal,	//正常
	LNExch,	//LN互易
	reverse	//逆相序
}phaseAbnormal_e;

typedef struct {
	TSA				tsa;		//通信地址
	INT8U			relayLevel; //中继级别
	TSA				relayTsa;	//中继地址
	OAD				oad;		//端口
	DateTimeBCD		lastSucTime;//最后一次采集成功时间
	INT8U			failTimes;	//采集失败次数
	phase_e phase;				//相位
	phaseAbnormal_e phaseAb;	//相序异常
}CLASS_6033;

typedef struct {
	CLASS11 class11;
	INT8U	num;
	CLASS_6033 class6033Array[MAX_METER_NUM];
}CLASS_6032;


	typedef struct {
		INT8U num;
		OAD oadarr[REPORT_CHANN_OAD_NUM];
	} ARRAY_OAD;

	typedef struct {
		OAD oad;
		CSD_ARRAYTYPE csds; // RCSD csd数组
		INT8U selectType;   // rsd 选择类型
		RSD rsd;
	} RecordData;

	typedef union {
		OAD oad;
		RecordData recorddata;
	} DataUnit;

	typedef struct {
		INT8U type; //上报类型　0:OAD对象属性数据【终端数据】　1:RecordData:上报记录型对象属性[电表]
		DataUnit data;
	} REPORT_DATA;

typedef struct {
    INT8U reportnum;        //方案编号
    ARRAY_OAD chann_oad;    //上报通道
    TI timeout;             //上报相应超时时间
    INT8U maxreportnum;     //最大上报次数
    REPORT_DATA reportdata; //上报数据
} CLASS_601D;               //上报方案

	typedef struct {
		INT8U dinum;
		INT8U DI_1[DI07_NUM_601F][4];
		INT8U DI_2[DI07_NUM_601F][4];
	} C601F_07Flag;

	typedef struct {
		INT8U dinum;
		INT8U DI_1[DI97_NUM_601F][2];
		INT8U DI_2[DI97_NUM_601F][2];
	} C601F_97Flag;

	typedef struct {
		INT8U protocol;
		union
		{
			C601F_97Flag _97;
			C601F_07Flag _07;
		}DI;
	}C601F_645;

typedef struct {
    OI_698 roadOI; //实时数据 0000  日冻结数据5004
    OAD flag698;
    INT8U unitnum;
    INT8U unitsize;
    INT8U datatype;
    C601F_97Flag flag97;
    C601F_07Flag flag07;
} CLASS_601F; //采集规则

typedef struct {
    INT8U taskID;          //任务ID
    TASK_STATE taskState;  //任务执行状态
    DateTimeBCD starttime; //任务结束结束时间
    DateTimeBCD endtime;   //任务结束结束时间
    INT16U totalMSNum;     //采集总数量
    INT16U successMSNum;   //采集成功数量
    INT16U sendMsgNum;     //发送报文数量
    INT16U rcvMsgNum;      //接受报文数量
} CLASS_6035;              //采集任务监控单元
///////////////////////////////////////////////////////////////////////////////
/********************************************************
 *				A.12　输入输出设备类对象
 ********************************************************/
typedef struct {
    OI_698 oi;    //对象标识
    INT16U model; //安全模式
} SecureModel;

typedef struct {
    INT8U active;               //属性2：安全模式选择（0：不启用安全模式参数，1：启用安全模式）
    INT8U modelnum;             //安全模式参数总个数
    SecureModel modelpara[255]; //属性3：显示安全模式参数
} CLASS_F101;                   //安全模式参数

typedef struct {
    INT8U ST; //状态ST  0：“分”状态；1：“合”状态
    INT8U CD; //变位CD  0：自前次遥信传送后无状态变化；1：自前次遥信传送后至少有一次状态变化。
} StateUnit;
typedef struct {
    INT8U num;
    StateUnit stateunit[STATE_MAXNUM];
} StateUnitArray;

typedef struct {
    INT8U StateAcessFlag; // bit-string(SIZE（8） bit0～bit7按顺序对位表示第1～8路状态量输入，置“1”：接入，置“0”：未接入。
    INT8U StatePropFlag; // bit-string(SIZE（8） bit0～bit7按顺序对位表示第1～8路状态量输入，置“1”常开触点。置“0”：常闭触点。
} StateAtti4;

typedef struct {
    CLASS22 class22;                  //接口类IC
    char devdesc[VISIBLE_STRING_LEN]; //设备描述， //湖南：485_II为维护口和485口切换，=485:用于抄表口，=698:用于维护口
    COMDCB devpara;                   //设备参数
    INT8U devfunc;                    //端口功能
} CLASS_f201;                         //RS232\ RS485维护口

typedef struct {
    CLASS22 class22;                  //接口类IC
    char devdesc[VISIBLE_STRING_LEN]; //设备描述
    COMDCB devpara;                   //设备参数
} CLASS_f202;                         //红外维护口

typedef struct {
    CLASS22 class22;          //接口类IC
    StateUnitArray statearri; //开关量单元属性2
    StateAtti4 state4;        //开关量属性
} CLASS_f203;                 //开关量输

	typedef struct{
		char  devdesc[VISIBLE_STRING_LEN];
		OAD	  oad;
		INT8U currentState; //当前状态 0输出 1未输出
		INT8U switchAttr;	//开关属性 0脉冲 1保持式
		INT8U wiredState;	//接线状态 0接入 1未接入
	}RelayUnit;
typedef struct {
	INT8U 	relaynum;		//继电器数
	RelayUnit	unit[4];
} CLASS_F205;			//继电器单元

	typedef struct{
		INT8U start[3];
		INT8U end[3];
	}ENABLE_TIME;
typedef struct {
	INT8U state_num;
	INT8U alarm_state[10];
	INT8U time_num;
	ENABLE_TIME timev[10];
} CLASS_f206;                 //开关量输

typedef struct {
	INT8U	num;			//端子数量
	OAD		oad[4];			//路号
	INT8U	func[4];		//端子功能
} CLASS_f207;                 //多功能端子

	typedef struct{
	    char factoryCode[2];    //厂商代码
	    char chipCode[2];        //软件版本号
	    INT8U softDate[5];       //软件版本日期
	    INT32U	softVer;		//软件版本
	}VersionInfo;
	typedef struct{
		TSA		commAddr;		//通信地址
		INT32U	overTime;		//接收等待报文超时时间（秒）
		INT8U	transBuf[255];	//透明转发命令
	}TransPara;//透明转发（参数）
	typedef struct{
		INT32U	pointNo;		//从节点序号
		INT8U	pointAddr[VISIBLE_STRING_LEN];		//从节点通信地址
		INT8U	pointDesc[VISIBLE_STRING_LEN];		//从节点描述符
	}SlaveUnit;	//从节点单元

	typedef struct{
	    CLASS22 class22;                  			//接口类IC
	    char devdesc[VISIBLE_STRING_LEN_1]; 		//设备描述
	    OAD		comOad;								//端口号
	    COMDCB devpara;                  			 //设备参数
	    VersionInfo		version;					//版本信息
	    SlaveUnit		att5_slave[SERACH_NUM];		//属性5（从节点对象列表）
	    TI				updatePeriod;				//属性6(从节点对象列表更新周期)
	    INT8U freqRange;							//属性9（宽带载波频段序号）
	}CLASS_F209_PARA;
typedef struct {
	CLASS_F209_PARA	para;			//参数
    TSA 			tsa;			//通讯地址
    int				timeout;		//超时时间
//	TransPara		trans;			//透明转发（参数）
//    INT8U			transFlg;		//透明转发标志，主站下发：置1，载波抄表：置0
} CLASS_f209;                       //载波/微功率无线接口

/////////////////////////////////////////////////////////////////////
typedef struct {
    OAD oad;
    INT8U dar;      //错误信息
    INT8U *data;    //数据  上报时与 dar二选一
    INT16U datalen; //数据长度
} RESULT_NORMAL;
typedef struct {
    OAD oad;
    RCSD rcsd;
    INT8U dar;
    INT8U *data;      //数据  上报时与 dar二选一
    INT16U datalen;   //数据长度
    INT8U selectType; //选择类型
    RSD select;       //选择方法实例
} RESULT_RECORD;

typedef struct {
	INT8U	dar;		//数据状态值
    TSA tsa;           //目标地址
    INT16U onetimeout; //一个服务器的超时时间
    INT16U num;        // oad的个数
    OAD oads[10];      // num个对象描述
} GETOBJS;

typedef struct {
	INT8U	dar;		//数据状态值
	OAD oad;           //数据转发OAD
    COMDCB comdcb;       //端口通信控制块
    INT16U revtimeout;    // 接收等待报文超时时间（秒）
    INT16U bytetimeout;    // 接收等待字节超时时间（毫秒）
    INT8U cmdlen;            //透明转发命令 长度
    INT8U cmdbuf[255];    //透明转发内容
} TRANSCMD;
typedef struct{
	INT8U type;
	INT8U len;
	INT8U buf[255];
}RSDBUF;
typedef struct {
    TSA tsa;          	 	//目标地址
    OAD oad;
    RSDBUF selectbuf;       //选择方法实例
    RCSD rcsd;
	INT8U	dar;			//数据状态值,位置放在最后，消息报文直接送联合体，防止读取错误
}GETRECORD;
typedef struct{
	OAD   oad_set;
	INT8U datatype;
	INT8U len;
	INT8U data[50];
	OAD   oad_get;
	INT16U dealy;
}SETATTRIB ;
typedef struct{
	OAD oad;
	INT8U datatype;
	INT8U len;
	INT8U data[50];
}SETOBJ;
typedef struct{
	INT8U	dar;		//数据状态值
	TSA tsa;
	INT16U num;
	INT16U timeout;
	SETATTRIB setoads[5];
}DO_Then_GET;
typedef struct{
	INT8U	dar;		//数据状态值
	TSA tsa;
	INT16U num;
	INT16U timeout;
	SETOBJ setobjs[5];
}ACTION_SET_OBJ;

typedef struct{
	INT8U	dar;			//数据状态值
	TSA		commAddr;		//通信地址
	INT32U	overTime;		//接收等待报文超时时间（秒）
	INT16U  buflen;			//透传报文长度
	INT8U	transBuf[255];	//透明转发命令, F209返回报文:第一个字节为octet-string类型描述，第二字节为长度（长度>127，3个字节长度）+实际报文内容
}F209_TransPara;//透明转发（参数）

typedef union {
	INT8U buf[1024];
	GETOBJS objs[10];  				//代理请求列表		ProxyGetRequestList
    GETRECORD record;				//代理请求记录		ProxyGetRequestRecord
    TRANSCMD transcmd;   		 	//代理操作透明转发	ProxyTransCommandRequest
    DO_Then_GET doTsaThenGet[5];	//TSA[n]	ProxySetThenGetRequestList	ProxyActionThenGetRequestList
    ACTION_SET_OBJ doTsaList[5];	//TSA[n]	Proxy  Action\Set- tList
    F209_TransPara  f209Trans;		//F209的方法127：透明转发（参数）传输内容为DLT645，非376.2报文，作为一种代理使用
}PROXYOBJ;
typedef struct {
    INT8U status;      //代理传输状态		0 表示就绪     1 已经表示返回数据  2 已经响应主站   3 超时
    long int position; //记录文件中的位置
    time_t timeold;    //代理请求产生的时间
    CSINFO csinfo;     //保存客户机信息
    INT8U proxytype;    //代理类型
    INT8U piid;        //本次代理请求PIID
    INT16U timeout;    //代理超时时间
    INT16U num;        //TSA个数
    INT16U	proxylen;	//代理接收长度
    PROXYOBJ proxy_obj;//代理内容
    INT8U data[512];   //请求结果
    INT16U datalen;    //数据长度
} PROXY_GETLIST;

typedef struct {
    TSA tsa;
    INT8U al_flag;
    INT8U cal_flag;
    INT64U curP[MAXVAL_RATENUM+1];    //当前正向有功-总尖峰平谷
    INT64U curQ[MAXVAL_RATENUM+1];    //当前正向无功-总尖峰平谷
    INT64U curNP[MAXVAL_RATENUM+1];    //当前反向有功-总尖峰平谷
    INT64U curNQ[MAXVAL_RATENUM+1];    //当前反向无功-总尖峰平谷
} AL_UNIT;

typedef struct {
    INT8U index;
    INT8U enable_flag;
    INT8U PCState;			//功控状态
    INT8U ECState;			//电控状态
    INT8U PTrunState;		//功控轮次状态
    INT8U ETrunState;		//电控轮次状态
} ALCONSTATE;

typedef struct {
    INT64S v;
    INT8S Downc;
    INT8U OutputState;		//功率跳闸输出状态
    INT8U MonthOutputState;	//月电控跳闸输出状态
    INT8U BuyOutputState;	//购电控跳闸输出状态
    INT8U PCAlarmState;		//功控越限告警状态
    INT8U ECAlarmState;		//电控越限告警状态
} ALCTLSTATE;

typedef struct {
    OI_698 name;
    INT8U state;
} ALSTATE;

typedef struct {
	OAD no;//脉冲输入端口号
	INT8U conf;//脉冲属性{0正向有功 1正向无功 2反向有功 3反向无功}
	INT32U k;//脉冲常数
} PULSEUNIT;

typedef struct {
	INT64U val_pos_p[MAXVAL_RATENUM];//正向有功电能示值
	INT64U val_pos_q[MAXVAL_RATENUM];//正向无功电能示值
	INT64U val_nag_p[MAXVAL_RATENUM];//反向有功电能示值
	INT64U val_nag_q[MAXVAL_RATENUM];//反向无功电能示值
} CLASS12_ENERGY;

typedef struct {
	INT8U addr[TSA_LEN];//通信地址
	INT32U pt;//互感器倍率
	INT32U ct;//互感器倍率
	PULSEUNIT unit[MAX_PULSE_UNIT];//脉冲配置
	INT32S p;//有功功率
	INT32S q;//无功功率
	INT32U day_pos_p[MAXVAL_RATENUM];//当日正向有功功率
	INT32U mon_pos_p[MAXVAL_RATENUM];//当月正向有功功率
	INT32U day_nag_p[MAXVAL_RATENUM];//当日反向有功功率
	INT32U mon_nag_p[MAXVAL_RATENUM];//当月反向有功功率
	INT32U day_pos_q[MAXVAL_RATENUM];//当日正向无功功率
	INT32U mon_pos_q[MAXVAL_RATENUM];//当月正向无功功率
	INT32U day_nag_q[MAXVAL_RATENUM];//当日反向无功功率
	INT32U mon_nag_q[MAXVAL_RATENUM];//当月反向无功功率
	INT32U val_pos_p[MAXVAL_RATENUM];//正向有功电能示值
	INT32U val_pos_q[MAXVAL_RATENUM];//正向无功电能示值
	INT32U val_nag_p[MAXVAL_RATENUM];//反向有功电能示值
	INT32U val_nag_q[MAXVAL_RATENUM];//反向无功电能示值
	Scaler_Unit su[14];//属性5-18换算单位
	INT32U pluse_count;
} CLASS12;

typedef struct {
    OI_698 lists;
    ALSTATE enable[MAX_AL_UNIT];
    ALSTATE output[MAX_AL_UNIT];
    ALSTATE overflow[MAX_AL_UNIT];
} CLASS13;

typedef struct {
    AL_UNIT allist[MAX_AL_UNIT]; //总加配置表
    INT64S p;   //有功
    INT64S q;   //无功
    INT64S TaveP;   //滑差有功
    INT64S TaveQ;   //滑差无功
    INT64S DayPALL;    //日有功
    INT64S DayP[MAXVAL_RATENUM];    //日有功
    INT64S DayQALL;    //日无功
    INT64S DayQ[MAXVAL_RATENUM];    //日无功
    INT64S MonthPALL;  //月有功
    INT64S MonthP[MAXVAL_RATENUM];  //月有功
    INT64S MonthQALL;  //月有功
    INT64S MonthQ[MAXVAL_RATENUM];  //月无功
    INT64S remains; //剩余电量
    INT64S DownFreeze;  //下浮控后总加有功冻结
    INT8U aveCircle;    //滑差周期
    INT8U pConfig;  //功控轮次
    INT8U eConfig;  //电控轮次
    ALCONSTATE alConState;  //设置状态
    ALCTLSTATE alCtlState;  //控制状态
    Scaler_Unit su[10];		//属性3-12换算单位
} CLASS23;

typedef struct {
	OAD		oad;		//继电器
	INT8U	closecmd;	//命令：=0：合闸允许，=1:直接合闸
	INT8U	passwd[VISIBLE_STRING_LEN];		//密码
	INT8U	actionflag;		//==0x55:主站下发控制，通信控制后自动修改恢复=0
}METER_CLOSE_CTRL;		//电表合闸控制

typedef struct {
    INT32U 	limit; 			//继电器拉闸电流门限值
    INT16U 	delaytime; 		//超电流门限保护延时时间
    INT8U  	alarmstate;		//继电器告警状态，只读
    INT8U 	cmdstate;		//继电器命令状态，只读
    INT8U	alarmaction;	//根据相应方法127：触发告警=0x55，方法128：解除告警=其他值，为了控制模块的执行动作
    //下面属性：数组【0】:第一路遥控，【1】：第二路遥控，
    OAD		relay_oad[2];	//继电器号
    INT8U	alarmdelay[2];	//方法129：告警延时
    INT32U	powerouttime[2];//方法129：限电时间
    INT8U	autoclose[2];	//方法129：自动合闸
    INT8U	closecmd[2];	//方法130：合闸命令 =0合闸允许，=1直接合闸
    INT16U	openclose[2];	//根据相应方法129：设置值=0x5555为分闸 方法130：设置值=0xCCCC为合闸，为了控制模块的执行动作
    METER_CLOSE_CTRL	meter_Ctrl[5];		//方法131：电表明文合闸
} CLASS_8000;

typedef struct {
	INT8U autoTimeStart; //自动保电时段开始
	INT8U autoTimeEnd; //自动保电时段结束
} CLASS_8001_unit;

typedef struct {
    INT8U state; //保电状态 0解除 1保电 2自动保电
    INT32U noCommTime; //最大与主站无通信时长(分钟)，超过此时间自动保电，0表示不自动保电
    INT32U autoTime; //上电自动保电时长,0表示不自动保电
    INT8U unit_count;//时段有效个数
    CLASS_8001_unit unit[24];
} CLASS_8001;

typedef struct {
    INT8U 	state; //催费告警状态，0未告警，1告警
    char	alarmTime[4];		//告警时段
    char	alarmInfo[201];		//告警信息  [0]:后面有效长度
} CLASS_8002;

typedef struct {
	INT8U   no;		//序号	no=0xFF :无效
	DateTimeBCD	 releaseData;	//发布时间
	INT8U   readflg;				//已阅读标记
	char	info[201];				//信息内容
}CHINESE_INFO;

typedef struct {
	CHINESE_INFO  chinese_info[10];		//一般中文信息
}CLASS_8003_8004;

typedef struct {
    INT64S v; //终端保安定值
} CLASS_8100;

typedef struct {
	INT8U	time_num;
    INT8U 	time[12]; //终端功控时段
} CLASS_8101;

typedef struct {
	INT8U	time_num;
    INT8U 	time[8]; //终端告警时间
} CLASS_8102;

typedef struct {
    INT8U n;
    INT64S t1;
    INT64S t2;
    INT64S t3;
    INT64S t4;
    INT64S t5;
    INT64S t6;
    INT64S t7;
    INT64S t8;
} PowerCtrlParam;

typedef struct {
    OI_698 index;
    INT8U sign;
    PowerCtrlParam v1;
    PowerCtrlParam v2;
    PowerCtrlParam v3;
    INT8S para;
} TIME_CTRL;

typedef struct {
    OI_698 index;	//总加组对象
    INT8U sign;		//时段功控投入标识
    INT8U numb;		//时段功控定值方案号
} TIME_SWITCH;

typedef struct {
	TIME_SWITCH	 plan[MAX_AL_UNIT];
    TIME_CTRL list[MAX_AL_UNIT];
    ALSTATE enable[MAX_AL_UNIT];
    ALSTATE output[MAX_AL_UNIT];
    ALSTATE overflow[MAX_AL_UNIT];
} CLASS_8103;

typedef struct {
    OI_698 index;
    INT64S v;
    DateTimeBCD start;
    INT16U sustain;
    INT8U noDay;
} FACT_CTRL;

typedef struct {
    FACT_CTRL list[MAX_AL_UNIT];
    ALSTATE enable[MAX_AL_UNIT];
    ALSTATE output[MAX_AL_UNIT];
    ALSTATE overflow[MAX_AL_UNIT];
} CLASS_8104;

typedef struct {
    OI_698 index;
    DateTimeBCD start;
    DateTimeBCD end;
    INT64S v;
} STOP_CTRL;

typedef struct {
    STOP_CTRL list[MAX_AL_UNIT];
    ALSTATE enable[MAX_AL_UNIT];
    ALSTATE output[MAX_AL_UNIT];
    ALSTATE overflow[MAX_AL_UNIT];
} CLASS_8105;

typedef struct {
    INT8U down_huacha;
    INT8S down_xishu;
    INT8U down_freeze;
    INT8U down_ctrl_time;
    INT8U t1;
    INT8U t2;
    INT8U t3;
    INT8U t4;
} DOWN_CTRL;

typedef struct {
	OI_698 index;
	DOWN_CTRL list;
    ALSTATE enable;
    ALSTATE output;
    ALSTATE overflow;
} CLASS_8106;//当前功率下浮控

typedef struct {
    OI_698 index;	//总加组对象
    INT32U no;		//购电单号
    INT8U add_refresh;//追加/刷新标识
    INT8U type;			//购电类型
    INT64S v;			//购电量（费）值
    INT64S alarm;		//报警门限值
    INT64S ctrl;		//跳闸门限值
    INT8U mode;			//购电控模式
} BUY_CTRL;

typedef struct {
    BUY_CTRL list[MAX_AL_UNIT];
    ALSTATE enable[MAX_AL_UNIT];
    ALSTATE output[MAX_AL_UNIT];
    ALSTATE overflow[MAX_AL_UNIT];
} CLASS_8107;

typedef struct {
    OI_698 index;
    INT64S v;
    INT8U para; //报警门限值系数
    INT8S flex; //月电量控定值浮动系数
} MONTH_CTRL;

typedef struct {
    MONTH_CTRL list[MAX_AL_UNIT];
    ALSTATE enable[MAX_AL_UNIT];
    ALSTATE output[MAX_AL_UNIT];
    ALSTATE overflow[MAX_AL_UNIT];
} CLASS_8108;

/*
 * 任务启动识别信息
 */
typedef struct {
    INT8U ID;        //任务编号
    INT8U SerNo;     //方案编号
    INT8U ReportNum; //上报次数
    INT16U OverTime; //上报响应超时时间，单位：秒
    time_t nexttime; //执行时间
} AutoTaskStrap;
typedef struct {
    INT8U securetype;           //安全类型
    CLASS_F101 f101;            //安全模式信息
    void *shmem;
    INT8U taskaddr;  			//客户机地址
    time_t lasttime; 			//最后一次通信时间
    int Heartbeat;
    int phy_connect_fd;
    INT8U linkstate;
    INT8U testcounter;
    INT8U serveraddr[16];
    INT8U report_piid[16];		//上报piid,数组为了多窗口通信协议
    INT8U response_piid[16];		//上报响应piid
    int RHead, RTail;           //接收报文头指针，尾指针
    int deal_step;              //数据接收状态机处理标记
    int rev_delay;              //接收延时
    LINK_Request link_request;
    LINK_Response linkResponse; //心跳确认
    CONNECT_Response myAppVar;  //集中器支持的应用层会话参数
    CONNECT_Response AppVar;    //与主站协商后的应用层会话参数
    INT8S (*p_send)(int name, int fd, INT8U *buf, INT16U len);
    INT8U SendBuf[BUFLEN];      //发送数据
    INT8U DealBuf[FRAMELEN];    //保存接口函数处理长度
    INT8U RecBuf[BUFLEN];       //接收数
    TS final_frame;				//最后一次收到报文的时间
    int name;					//端口名称，用于区分各个端口
} CommBlock;
////////////////////////////////////////////////////////////////////
typedef struct
{
	int sucessflg;		//0:没抄读；1:已抄读，无数据返回；2:抄读成功
	OAD oad1;			//非关联 oad1.OI=0
	OAD oad2;			//数据项
	INT8U item97[4];	//97规约  15753578781
	INT8U item07[4];	//07规约  15753578781
	DateTimeBCD savetime;//存储时标
}DATA_ITEM;
typedef struct
{
	INT8U type;							//方案类型
	INT8U No;							//方案编号
	DATA_ITEM items[20];				//数据项数组
	INT8U item_n;						//数据项总数 < FANGAN_ITEM_MAX
	INT8U item_i;//当前抄的数据项序号
	INT8U cjtype;		//采集类型	0：采集当前	1：采集上N次   2:按冻结时标采集		3：按时标间隔采集
	INT8U N;			//							上N次
	TI ti;				//按时标间隔采集
}CJ_FANGAN;

typedef struct
{
	INT8U taskId;						//任务编号
	time_t beginTime;					//开始时间
	time_t endTime;						//结束时间
	DateTimeBCD begin;
	DateTimeBCD end;
	TI ti;		  						//任务执行频率
	INT8U leve;							//优先级别
	INT8U tryAgain;						//需要补抄
	CJ_FANGAN fangan;					//采集方案
}TASK_UNIT;

typedef struct
{
	TASK_UNIT task_list[10];			//任务数组
	int task_n;							//任务个数  < TASK_MAXNUM
	TSA tsa;							//表地址
	int tsa_index;						//表序号
	int now_taski;//当前抄读的任务
	int now_itemi;//当前抄读的数据项
	INT8U freezeStamp;//冻结时标抄读状态　0-未抄读　1-已抄读但冻结时标不对　2-已抄读冻结时标正确
}TASK_INFO;

#endif /* OBJECTACTION_H_ */
