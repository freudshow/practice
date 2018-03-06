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

#define S4851   		1
#define S4852   		2
#define S4853   		3
#define SER_ZB			5

#define PIN_BASE 32
#define AT91_PIN_PC1 (PIN_BASE + 0x40 + 1)
#define AT91_PIN_PA7 (PIN_BASE + 0x00 + 7)

#define RTS485 0x542D
#define TIOCGRS485 0x542E
#define TIOCSRS485 0x542F

typedef enum baudrate_enum{
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

typedef enum parity_enum{
	parEven = 0,
	parOdd,
	parNone
} parity_e;

#pragma	pack(push)
#pragma pack(1)

typedef struct {
	u8 port;
	baud_e baud;
	parity_e par;
	u8 stopb;
	u8 bits;
} comConfig_s;

#pragma pack(pop)


#ifdef __cplusplus
}
#endif

#endif	/* _SERIAL_H */
