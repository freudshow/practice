#include <stdio.h>
#include <stdlib.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "lib.h"
#include "serial.h"



int openCom(comConfig_s* config)
{
	struct serial_rs485 rs485conf;
	int fd = -10;
	struct termios old_termi = { }, new_termi = { };
	int baud_lnx = 0;
	unsigned char tmp[20] = { 0 };

	if (NULL == config) {
		printf("param is NULL!\n");
		return -1;
	}

	sprintf((char *) tmp, "%s", config->port);
	fd = open((char *) tmp, O_RDWR | O_NOCTTY); /* 打开串口文件 */
	if (fd < 0) {
		printf("open the serial port fail! errno is: %d\n", errno);
		return 0; /*打开串口失败*/
	}

	if (tcgetattr(fd, &old_termi) != 0) {/*存储原来的设置*/
		printf(
				"get the terminal parameter error when set baudrate! errno is: %d\n",
				errno);
		/*获取终端相关参数时出错*/
		return 0;
	}

	bzero(&new_termi, sizeof(new_termi)); /*将结构体清零*/
	new_termi.c_cflag |= (CLOCAL | CREAD); /*忽略调制解调器状态行，接收使能*/
	new_termi.c_lflag &= ~(ICANON | ECHO | ECHOE); /*选择为原始输入模式*/
	new_termi.c_oflag &= ~OPOST; /*选择为原始输出模式*/
	new_termi.c_cc[VTIME] = 1; /*设置超时时间为0.5 s*/
	new_termi.c_cc[VMIN] = 0; /*最少返回的字节数是 7*/
	new_termi.c_cflag &= ~CSIZE;
	new_termi.c_iflag &= ~ISTRIP;

	switch (config->baud) {
	case baud1200:
		baud_lnx = B1200;
		break;
	case baud2400:
		baud_lnx = B2400;
		break;
	case baud4800:
		baud_lnx = B4800;
		break;
	case baud9600:
		baud_lnx = B9600;
		break;
	case baud19200:
		baud_lnx = B19200;
		break;
	case baud38400:
		baud_lnx = B38400;
		break;
	case baud57600:
		baud_lnx = B57600;
		break;
	case baud115200:
		baud_lnx = B115200;
		break;
	default:
		baud_lnx = B2400;
		break;
	}

	switch (config->bits) {
	case 5:
		new_termi.c_cflag |= CS5;
		break;
	case 6:
		new_termi.c_cflag |= CS6;
		break;
	case 7:
		new_termi.c_cflag |= CS7;
		break;
	case 8:
		new_termi.c_cflag |= CS8;
		break;
	default:
		new_termi.c_cflag |= CS8;
		break;
	}

	switch (config->par) {
	case parEven:
		new_termi.c_cflag |= PARENB;
		new_termi.c_cflag &= ~PARODD;
		break;
	case parOdd:
		new_termi.c_cflag |= PARENB;
		new_termi.c_cflag |= PARODD;
		break;
	case parNone:
		new_termi.c_cflag &= ~PARENB;
		break;
	default:
		new_termi.c_cflag &= ~PARENB;
		break;
	}

	switch (config->stopb) {
	case 1: //停止位
		new_termi.c_cflag &= ~CSTOPB; //设置停止位为:一位停止位
		break;
	case 2:
		new_termi.c_cflag |= CSTOPB; //设置停止位为:二位停止位
		break;
	default:
		new_termi.c_cflag &= ~CSTOPB; //设置停止位为:一位停止位
		break;
	}

	cfsetispeed(&new_termi, baud_lnx); /* 设置输入拨特率 */
	cfsetospeed(&new_termi, baud_lnx); /* 设置输出拨特率 */

	tcflush(fd, TCIOFLUSH); /* 刷新输入输出流 */
	if (tcsetattr(fd, TCSANOW, &new_termi) != 0) {/* 激活串口配置 */
		DEBUG_TIME_LINE("Set serial port parameter error!\n");
		return -1;
	}

	if (strcmp(config->port, S4851) == 0 || strcmp(config->port, S4852) == 0) {
		memset(&rs485conf, 0, sizeof(rs485conf));
		if (ioctl(fd, TIOCGRS485, &rs485conf) < 0) {
			DEBUG_TIME_LINE("ioctl TIOCGRS485 error.\n");
		}

		rs485conf.flags |= SER_RS485_ENABLED;
		rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);
		rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;

		if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) {
			DEBUG_TIME_LINE("ioctl TIOCSRS485 error\n");
		}

		int rs485gpio;
		// if (strcmp(config->port, S4851) == 0) {
		// 	rs485gpio = AT91_PIN_PC1;
		// } else if (strcmp(config->port, S4852) == 0){
		// 		rs485gpio = AT91_PIN_PA7;
		// } else {
		// 	rs485gpio = AT91_PIN_PC1;
		// }

		if (ioctl(fd, RTS485, &rs485gpio) < 0) {
			fprintf(stderr, "ioctl RTS485 error\n");
		}
		fprintf(stderr, "rs485gpio=%d,fd=%d\n", rs485gpio, fd);
	}
	return fd;
}

void usage()
{
	printf("serial \"1,2400,n,8,1\" \"fe fe fe fe 68 18 16 00 00 00 00 68 11 04 33 33 34 33 e0 16\"\n");
	printf("or ommit com para: serial \"fe fe fe fe 68 18 16 00 00 00 00 68 11 04 33 33 34 33 e0 16\"\n");
	printf("serial -baud 2400 -stop 1 -parity e -data 8 -port 1 -frame \"fe fe fe fe 68 18 16 00 00 00 00 68 11 04 33 33 34 33 e0 16\"\n");
}

/*
 * 功能: 获取串口配置.
 * 格式必须是"串口号,波特率,校验方式,停止位,数据位".
 * 串口号, 波特率, 停止位, 数据位必须是十进制数字,
 * 校验方式: e-偶校验, o-奇校验, n-无校验, 其他字符无效.
 * 必须以','分隔, 其他的分隔符不允许出现.
 * 波特率取值: {1200, 2400, 4800, 9600, 19200, 38400, 115200}
 * 比如: "/dev/ttySA1,2400,e,1,8".
 * @str: 串口配置字符串, 以'\0'结尾
 * @pConfig: 串口配置结构体
 */
s8 getComConfig(char* str, comConfig_s* pConfig)
{
	u8 position[10] = {0};
	u32 i = 0;
	u32 baudrate = 0;
	char* p = NULL;

	if ( NULL == str || NULL == pConfig )
        return FALSE;

	for(i = 0, p = str;*p != '\0';p++) {
		if (*p == ',') {//record position of ','
			position[i] = (p-str);
			*p = '\0';
			i++;
		}
	}

	if (i != 4) {
		return FALSE;
	}

	p = str;
	memcpy(pConfig->port, p, strlen(p));

	p = &str[position[0]+1];
	baudrate = atoi(p);
	switch (baudrate) {
	case 1200:
		pConfig->baud = baud1200;
		break;
	case 2400:
		pConfig->baud = baud2400;
		break;
	case 4800:
		pConfig->baud = baud4800;
		break;
	case 9600:
		pConfig->baud = baud9600;
		break;
	case 19200:
		pConfig->baud = baud19200;
		break;
	case 38400:
		pConfig->baud = baud38400;
		break;
	case 115200:
		pConfig->baud = baud115200;
		break;
	default:
		pConfig->baud = baud2400;
		break;
	}

	p = &str[position[1]+1];
	switch(*p) {
	case 'e':
		pConfig->par = parEven;
		break;
	case 'o':
		pConfig->par = parOdd;
		break;
	case 'n':
		pConfig->par = parNone;
		break;
	default:
		pConfig->baud = parNone;
		break;
	}

	p = &str[position[2]+1];
	pConfig->stopb = atoi(p);

	p = &str[position[3]+1];
	pConfig->bits = atoi(p);

	return TRUE;
}

s8 sendBuf(int fd, u8* buf, u32 bufSize)
{
	if ( 0 == bufSize || NULL == buf )
        return FALSE;

	if (write(fd, buf, bufSize) < 0) {
		DEBUG_TIME_LINE("send error!\n");
		return FALSE;
	}
	DEBUG_TIME_LINE("send bufSize:%d", bufSize);

	return TRUE;
}

void readBuf(int fd, u8* buf, u32* bufSize)
{
	if ( NULL == bufSize || NULL == buf )
        return;

	*bufSize = read(fd, buf, 2048);
	DEBUG_TIME_LINE("read bufSize:%d", *bufSize);
}

void setDefaultPara(comConfig_s* pConfig)
{
	if ( NULL == pConfig )
		return;

	bzero(pConfig, sizeof(comConfig_s));

	strcpy(pConfig->port, S4851);
	pConfig->baud = baud2400;
	pConfig->par = parEven;
	pConfig->stopb = 1;
	pConfig->bits = 8;
}

int main(int argc, char* argv[])
{
	comConfig_s config;
	char* pFrame = NULL;
	u8 rbuf[2048] = {0};
	u8 sbuf[2048] = {0};
	u32 sbufSize = 0;
	u32 rbufSize = 0;
	int fd = -1;

	bzero(&config, sizeof(config));
	setDefaultPara(&config);

#ifdef NORMAL
	if (argc == 1 || argc > 3) {
		usage();
		exit(0);
	} else if (argc == 3) {
		if (getComConfig(argv[1], &config) == FALSE) {
			DEBUG_TIME_LINE("get com config failed!");
			goto ret;
		}
		pFrame = argv[2];
	} else if (argc == 2) {
		pFrame = argv[1];
	}
#endif

	//sem_wait();
	if ( (fd = openCom(&config)) < 0 ) {
		perror("open failed!");
		exit(1);
	}
	DEBUG_TIME_LINE("port: %s, baud: %d, parity: %d, stop: %d, bits: %d\n",
		  config.port, config.baud, config.par, config.stopb, config.bits);
	sbufSize = sizeof(sbuf);

#ifdef NORMAL
	readFrm(pFrame, sbuf, &sbufSize);
	DEBUG_OUT("[send]:");
	printBuf(sbuf, sbufSize);
	int sendcnt = 0;

	while (sendcnt < 5) {
		if (sendBuf(fd, sbuf, sbufSize) == FALSE) {
			goto ret;
		}
		int cnt = 0;
		while (cnt<5) {
			usleep(1000000);
			readBuf(fd, rbuf, &rbufSize);
			DEBUG_OUT("[read]:");
			printBuf(rbuf, rbufSize);
			DEBUG_OUT("[cnt]:%d", cnt);
			cnt++;
		}
		// sendcnt++;
	}


#endif

#ifdef LISTEN
	while (1) {
		sleep(1);
		readBuf(fd, rbuf, &rbufSize);
		DEBUG_OUT("[read]:");
		printBuf(rbuf, rbufSize);
		fprintf(stderr, "\n");
	}
	//sem_post();
#endif

ret:
	close(fd);
	exit(0);
}
