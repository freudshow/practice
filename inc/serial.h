/******************************************************************************
 * file:    serial.h
 * author:  s_baoshan <s_baoshan@163.com>
 * created: 2018-02-25
 * updated: 2018-02-25
 * contents: define base data types; some useful macros.
 *****************************************************************************/
#ifndef	_SERIAL_H
#define	_SERIAL_H

#include "basedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//  I型集中器串口定义
//　　　GPRS  [ /dev/ttyS0 ]
//　　　485I  [ /dev/ttyS1 ]
//　　　485II [ /dev/ttyS2 ]
//　　载波　 [ /dev/ttyS5 ]
//　　维护口 [ /dev/ttyS4 ]
//　　红外　 [ /dev/ttyS3 ]
//  II型集中器串口定义
//　　GPRS  [ /dev/ttyS5 ]
//　　485I  [ /dev/ttyS2 ]
//　　485II [ /dev/ttyS1 ]
//　　485III[ /dev/ttyS4 ]


#if defined CCTII
#define S4851   		"/dev/ttyS2"
#define S4852   		"/dev/ttyS1"
#elif defined CJQIII
#define S4851   		"/dev/ttySA1"
#define S4852   		"/dev/ttySA5"
#define SMBUS1   		"/dev/ttySA3"
#define SMBUS2			"/dev/ttySA10"
#define SMBUS3			"/dev/ttySA6"
#define SMODULE1		"/dev/ttySA8"
#define SMODULE2		"/dev/ttySA2"
#define SMODULE3		"/dev/ttySA9"
#define SMODULE4		"/dev/ttySA7"
#define SINFRARED		"/dev/ttySA4"
#elif defined (CCTI) || (CCTIII)
#define S4851   		"/dev/ttyS1"
#define S4852   		"/dev/ttyS2"
#elif defined (TTU)
#define S4851   		"/dev/ttySZ3"
#define S4852   		"/dev/ttySZ4"
#define S4853   		"/dev/ttySZ5"
#define S4854   		"/dev/ttySZ6"
#define SPLC            "/dev/ttyS7"
#elif defined (E9361_C0)
#define S4851           "/dev/ttymxc2"
#define S4852           "/dev/ttymxc3"
#define S4853           "/dev/ttymxc4"
#define S4854           "/dev/ttymxc5"
#define SPLC            "/dev/ttymxc7"
#endif

#define PIN_BASE 32
#define AT91_PIN_PC1 (PIN_BASE + 0x40 + 1)
#define AT91_PIN_PA7 (PIN_BASE + 0x00 + 7)

#define RTS485 0x542D
#define TIOCGRS485 0x542E
#define TIOCSRS485 0x542F

typedef enum baudrate_enum {
	baud0 = 0,
	baud50,
	baud75,
	baud110,
	baud150,
	baud200,
	baud600,
	baud1200,
	baud2400,
	baud4800,
	baud9600,
	baud19200,
	baud38400,
	baud57600,
	baud115200
} baud_e;

typedef enum parity_enum {
	parEven = 0, parOdd, parNone
} parity_e;

#pragma	pack(push)
#pragma pack(1)

typedef struct {
	u8 listen;
#define MASTER_DEV	0
#define SLAVE_DEV	1
	u8 master; //0-主机 1-从机
	u32 times;
	u32 wait;
	u32 inv;
	char com[128];
	u32 baud;
	u8 data;
	u8 stop;
	u8 par;
	char frame[2048];
} option_s;
typedef option_s* option_p;

typedef struct {
	char port[128];
	baud_e baud;
	parity_e par;
	u8 stopb;
	u8 bits;
} comConfig_s;
typedef comConfig_s* comConfig_p;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif	/* _SERIAL_H */
