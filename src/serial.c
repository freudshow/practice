#include <stdio.h>
#include <stdlib.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <string.h>
#include <sysexits.h>
#include "lib.h"
#include "serial.h"

static const char *optString = "l:t:w:i:c:b:d:s:p:f:h";
static const struct option longOpts[] = { { "listen", required_argument, NULL, 'l' },
										  { "times", required_argument, NULL, 't' },
										  { "wait",	required_argument, NULL, 'w' },
										  { "inv", required_argument, NULL, 'i' },
										  { "com", required_argument, NULL, 'c' },
										  { "baud", required_argument, NULL, 'b' },
										  { "data", required_argument, NULL, 'd' },
										  { "stop",	required_argument, NULL, 's' },
										  { "par", required_argument, NULL, 'p' },
										  { "frame", required_argument, NULL, 'f' },
										  {	"help", no_argument, NULL, 'h' },
										  { NULL, no_argument, NULL, 0 }
										};

int openCom(comConfig_s* config) {
	struct serial_rs485 rs485conf;
	int fd = -10;
	int rs485gpio = 0;
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

#if defined (CCTI) || (CCTII) || (CCTIII)
		if (strcmp(config->port, S4851) == 0) {
			rs485gpio = AT91_PIN_PC1;
		} else if (strcmp(config->port, S4852) == 0) {
			rs485gpio = AT91_PIN_PA7;
		} else {
			rs485gpio = AT91_PIN_PC1;
		}
#endif

		if (ioctl(fd, RTS485, &rs485gpio) < 0) {
			fprintf(stderr, "ioctl RTS485 error\n");
		}
		fprintf(stderr, "rs485gpio=%d,fd=%d\n", rs485gpio, fd);
	}
	return fd;
}

void usage() {
	fprintf(stderr, "功能: 向串口发送报文, 并监听应答报文\n");
	fprintf(stderr, "用法: serial [选项]\n");
	fprintf(stderr, "-l,    --listen    监听开关, 0-发送报文并等待应答; 1-一直监听一个串口不发送报文, 默认0;\n");
	fprintf(stderr, "-t,    --times     发送并监听次数, 默认0, 无限次, 最大值1024;\n");
	fprintf(stderr, "-w,    --wait      发送报文后, 读取串口数据的次数, 默认20次, 最多1024次;\n");
	fprintf(stderr, "-i,    --inv       发送报文后, 读取串口数据的时间间隔, 单位毫秒, 默认50毫秒, 最大1024毫秒;\n");
	fprintf(stderr, "-c,    --com       向哪个串口发送并监听数据, 必须是完整路径且必须以半角引号(" ")封闭,\n");
	fprintf(stderr, "                        默认\"/dev/ttySA1\", 即RS-485 I;\n");
	fprintf(stderr, "-b,    --baud      设置串口波特率, 默认2400;\n");
	fprintf(stderr, "-d,    --data      设置串口数据位, 6, 7, 8为有效值, 其他值视为使用默认值, 默认值8;\n");
	fprintf(stderr, "-s,    --stop      设置串口停止位, 1, 2为有效值, 其他值视为使用默认值, 默认值1;\n");
	fprintf(stderr, "-p,    --par            设置串口校验位, 0-偶, 1-奇, 2-无, 其他值视为使用默认值, 默认值0;\n");
	fprintf(stderr, "-f,    --frame     传入的报文, 必须以半角引号(\"\")封闭.\n");
	fprintf(stderr, "                        默认发送\"FE FE FE FE 68 12 34 56 78 90 16\".\n");
	fprintf(stderr, "-h,	--help      打印本帮助\n");
	fprintf(stderr, "如果任何参数都不传入, 程序使用默认参数来监听\"/dev/ttySA1\".\n");
}

s8 getComConfig(option_p pOpt, comConfig_p pConfig) {
	if ( NULL == pOpt || NULL == pConfig)
		return FALSE;

	strcpy(pConfig->port, pOpt->com);

	switch (pOpt->baud) {
	case 50:
		pConfig->baud = baud50;
		break;
	case 75:
		pConfig->baud = baud75;
		break;
	case 110:
		pConfig->baud = baud110;
		break;
	case 200:
		pConfig->baud = baud200;
		break;
	case 600:
		pConfig->baud = baud600;
		break;
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

	pConfig->par = pOpt->par;
	pConfig->stopb = pOpt->stop;
	pConfig->bits = pOpt->data;

	return TRUE;
}

s8 sendcom(int fd, u8* buf, u32 bufSize) {
	if (0 == bufSize || NULL == buf)
		return FALSE;

	if (write(fd, buf, bufSize) < 0) {
		DEBUG_TIME_LINE("send error!\n");
		return FALSE;
	}
	DEBUG_OUT("[send]bufSize:%d; ", bufSize);
	printBuf(buf, bufSize);

	return TRUE;
}

void readcom(int fd, u8* buf, u32* bufSize) {
	if ( NULL == bufSize || NULL == buf)
		return;

	*bufSize = read(fd, buf, 2048);
}

void setDefaultPara(comConfig_p pConfig) {
	if ( NULL == pConfig)
		return;

	bzero(pConfig, sizeof(comConfig_s));

	strcpy(pConfig->port, S4851);
	pConfig->baud = baud2400;
	pConfig->par = parEven;
	pConfig->stopb = 1;
	pConfig->bits = 8;
}

void setDefaultOpt(option_p pOpt) {
	if ( NULL == pOpt)
		return;

	bzero(pOpt, sizeof(option_s));
	pOpt->listen = 0;
	pOpt->times = 0;
	pOpt->wait = 20;
	pOpt->inv = 50;
	strcpy(pOpt->com, S4851);
	pOpt->baud = 2400;
	pOpt->data = 8;
	pOpt->stop = 1;
	pOpt->par = 0;
	strcpy(pOpt->frame, "FE FE FE FE 68 12 34 56 78 90 16");
}

int baudValid(int baud) {
	int i = 0;
	int array[] = { 50, 75, 110, 150, 200, 600, 1200, 2400, 4800, 9600, 19200,
			38400, 57600, 115200 };

	for (i = 0; i < sizeof(array); i++) {
		if (baud == array[i])
			return TRUE;
	}

	return FALSE;
}

void getOptions(int argc, char* argv[], option_p pOpt) {
	int ch = 0;
	int longIndex = 0;

	if ( NULL == pOpt)
		return;

	while ((ch = getopt_long(argc, argv, optString, longOpts, &longIndex)) != -1) {
		switch (ch) {
		case 'l':
			fprintf(stderr, "option -l: %s\n", optarg);
			pOpt->listen = atoi(optarg);
			if (0 != pOpt->listen || 1 != pOpt->listen) {
				pOpt->listen = 0;
			}
			break;
		case 't':
			fprintf(stderr, "option -t: %s\n", optarg);
			pOpt->times = atoi(optarg);
			if (pOpt->times > 1024) {
				pOpt->times = 1024;
			}
			break;
		case 'w':
			fprintf(stderr, "option -w: %s\n", optarg);
			pOpt->wait = atoi(optarg);
			if (pOpt->wait > 1024) {
				pOpt->wait = 1024;
			}
			break;
		case 'i':
			fprintf(stderr, "option -i: %s\n", optarg);
			pOpt->inv = atoi(optarg);
			if (pOpt->inv > 2048) {
				pOpt->inv = 2048;
			}
			break;
		case 'c':
			fprintf(stderr, "option -c: %s\n", optarg);
			bzero(pOpt->com, sizeof(pOpt->com));
			strcpy(pOpt->com, optarg);
			break;
		case 'b':
			fprintf(stderr, "option -b: %s\n", optarg);
			pOpt->baud = atoi(optarg);
			if (baudValid(pOpt->baud) == FALSE) {
				fprintf(stderr, "invalid baud\n");
				exit(0);
			}
			break;
		case 'd':
			fprintf(stderr, "option -d: %s\n", optarg);
			pOpt->data = atoi(optarg);
			if (pOpt->data != 8 && pOpt->data != 7 && pOpt->data != 6) {
				fprintf(stderr, "invalid databits, use 8 databits\n");
				pOpt->data = 8;
			}
			break;
		case 's':
			fprintf(stderr, "option -s: %s\n", optarg);
			pOpt->stop = atoi(optarg);
			if (pOpt->stop != 1 && pOpt->stop != 2) {
				fprintf(stderr, "invalid stopbits, use 1 stopbits\n");
				pOpt->stop = 1;
			}
			break;
		case 'p':
			fprintf(stderr, "option -p: %s\n", optarg);
			pOpt->par = atoi(optarg);
			if (pOpt->par != 0 && pOpt->par != 1 && pOpt->par != 2) {
				fprintf(stderr, "invalid parity, use 0(even) parity\n");
				pOpt->par = 0;
			}
			break;
		case 'f':
			fprintf(stderr, "option -f: %s\n", optarg);
			bzero(pOpt->frame, sizeof(pOpt->frame));
			strcpy(pOpt->frame, optarg);
			break;
		case 'h':
		default:
			usage();
			exit(0);
			break;
		}
	}
}

void printOpt(option_p pOpt)
{
	fprintf(stderr, "listen: %u\n", pOpt->listen);
	fprintf(stderr, "times: %u\n", pOpt->times);
	fprintf(stderr, "wait: %u\n", pOpt->wait);
	fprintf(stderr, "inv: %u\n", pOpt->inv);
	fprintf(stderr, "com: %s\n", pOpt->com);
	fprintf(stderr, "baud: %u\n", pOpt->baud);
	fprintf(stderr, "data: %u\n", pOpt->data);
	fprintf(stderr, "stop: %u\n", pOpt->stop);
	fprintf(stderr, "par: %u\n", pOpt->par);
	fprintf(stderr, "frame: %s\n", pOpt->frame);
}

int main(int argc, char* argv[]) {
	comConfig_s config = { };
	option_s options = { };
	u8 rbuf[2048] = { 0 };
	u8 sbuf[2048] = { 0 };
	u32 sbufSize = sizeof(sbuf);
	u32 rbufSize = sizeof(rbuf);
	int fd = -1;

	setDefaultOpt(&options);
	setDefaultPara(&config);

	if (argc > 1)
		getOptions(argc, argv, &options);

	getComConfig(&options, &config);
	DEBUG_TIME_LINE("port: %s, baud: %d, parity: %d, stop: %d, bits: %d\n",
			config.port, config.baud, config.par, config.stopb, config.bits);
	printOpt(&options);
	if ((fd = openCom(&config)) < 0) {
		perror("open failed!");
		exit(1);
	}

	if (1 == options.listen) { //一直监听串口
		while (1) {
			usleep(options.inv * 1000);
			readcom(fd, rbuf, &rbufSize);
			fprintf(stderr, "[read]:");
			printBuf(rbuf, rbufSize);
			fprintf(stderr, "\n");
		}
	} else { //发送报文并监听串口
		if (readFrm(options.frame, sbuf, &sbufSize) == FALSE) {
			exit(0);
		}

		u32 sendcnt = 0;

		while ((options.times > 0) ? (sendcnt < options.times) : 1) {
			if (sendcom(fd, sbuf, sbufSize) == FALSE) {
				goto ret;
			}
			int cnt = 0;
			while (cnt < options.wait) {
				usleep(options.inv * 1000);
				readcom(fd, rbuf, &rbufSize);
				if (rbufSize > 0) {
					DEBUG_OUT("[read]");
					printBuf(rbuf, rbufSize);
				}
				cnt++;
			}

			if (options.times > 0)
				sendcnt++;
		}

	}

ret:
	close(fd);
	exit(0);
}
