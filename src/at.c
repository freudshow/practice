#include <stdio.h>
#include <stdlib.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sysexits.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef int s32;
typedef signed long long s64;
typedef float fp32;
typedef double fp64;

#define TRUE	(0)
#define FALSE	(-1)

typedef struct {
	u8 listen;
#define MASTER_DEV	0
#define SLAVE_DEV	1
	u8 master; //0-主机 1-从机
	u32 times;
	u32 wait;
	u32 inv;
	u8 power;
#define CONVERT_CR_LF		1
#define NOT_CONVERT_CR_LF	0
	u8 convert;
	u8 dev[128];
	char at[512];
} option_s;
typedef option_s *option_p;

#define	FILE_LINE   __FILE__,__FUNCTION__,__LINE__
#define	DEBUG_OUT(format, ...)	debug(FILE_LINE, 0, format, ##__VA_ARGS__)
#define DEBUG_TIME_LINE(format, ...) debug(FILE_LINE, 1, format, ##__VA_ARGS__)
#define	DEBUG_PRINT(format, ...)	fprintf(stderr, "[%s][%s][%d]"format"\n", FILE_LINE, ##__VA_ARGS__)

extern void debug(const char *file, const char *func, u32 line, u8 printEnter,
		const char *fmt, ...);

static const char *optString = "d:l:t:w:i:m:p:a:ch";
static const struct option longOpts[] = { { "at", required_argument, NULL, 'a' },
										  { "convert",no_argument, NULL, 'c' },
										  { "dev", required_argument, NULL, 'd' },
										  { "inv", required_argument, NULL, 'i' },
                                          { "listen", required_argument, NULL, 'l' },
										  { "master", required_argument, NULL, 'm' },
										  { "power", required_argument, NULL, 'p' },
                                          { "times", required_argument, NULL, 't' },
                                          { "wait",	required_argument, NULL, 'w' },
										  { "help",no_argument, NULL, 'h' },
										  { NULL, no_argument, NULL, 0 }
                                        };

void get_local_time(char *buf, u32 bufSize) {
	time_t rawtime;
	struct tm timeinfo;

	if ((bufSize < 20) || (NULL == buf))
		return;

	time(&rawtime);
	localtime_r(&rawtime, &timeinfo);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", (timeinfo.tm_year + 1900),
			(timeinfo.tm_mon + 1), timeinfo.tm_mday, timeinfo.tm_hour,
			timeinfo.tm_min, timeinfo.tm_sec);
}

void debug(const char *file, const char *func, u32 line, u8 printEnter,
		const char *fmt, ...) {
	va_list ap;
	char buf[20] = { 0 };

	get_local_time(buf, sizeof(buf));
	fprintf(stderr, "[%s][%s][%s()][%d]: ", buf, file, func, line);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (1 == printEnter)
		fprintf(stderr, "\n");
}

int gpio_writebyte(char *devpath, s8 data) {
    int fd = -1;
    if ((fd = open((const char *) devpath, O_RDWR | O_NDELAY)) >= 0) {
        write(fd, &data, sizeof(char));
        close(fd);
        return 1;
    }
}

int OpenMuxCom(u8* dev, int baud, unsigned char *par, unsigned char stopb,
		u8 bits) {
	int Com_Port = 0;
	struct termios old_termi, new_termi;
	int baud_lnx = 0;

	fprintf(stderr, "open com: %s\n", dev);

	Com_Port = open((char*) dev, O_RDWR | O_NOCTTY); /* 打开串口文件 */
	if (Com_Port < 0) {
		fprintf(stderr, "open the serial port fail! errno is: %d\n", errno);
		return -1; /*打开串口失败*/
	}

	if (tcgetattr(Com_Port, &old_termi) != 0) {/*存储原来的设置*/
		fprintf(stderr,
				"get the terminal parameter error when set baudrate! errno is: %d\n",
				errno);
		/*获取终端相关参数时出错*/
		return -1;
	}
	// printf("\n\r c_ispeed == %d old_termi  c_ospeed == %d",old_termi.c_ispeed, old_termi.c_ospeed);
	bzero(&new_termi, sizeof(new_termi)); /*将结构体清零*/
	new_termi.c_cflag |= (CLOCAL | CREAD); /*忽略调制解调器状态行，接收使能*/
	new_termi.c_lflag &= ~(ICANON | ECHO | ECHOE); /*选择为原始输入模式*/
	new_termi.c_oflag &= ~OPOST; /*选择为原始输出模式*/
	new_termi.c_cc[VTIME] = 2; /*设置超时时间为0.5 s*/
	new_termi.c_cc[VMIN] = 0; /*最少返回的字节数是 7*/
	new_termi.c_cflag &= ~CSIZE;
	// new_termi.c_iflag &= ~INPCK;     /* Enable parity checking */
	new_termi.c_iflag &= ~ISTRIP;
	switch (baud) {
	case 1200:
		baud_lnx = B1200;
		break;
	case 2400:
		baud_lnx = B2400;
		break;
	case 4800:
		baud_lnx = B4800;
		break;
	case 9600:
		baud_lnx = B9600;
		break;
	case 19200:
		baud_lnx = B19200;
		break;
	case 38400:
		baud_lnx = B38400;
		break;
	case 57600:
		baud_lnx = B57600;
		break;
	case 115200:
		baud_lnx = B115200;
		break;
	default:
		baud_lnx = B9600;
		break;
	}

	switch (bits) {
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

	if (strncmp((char*) par, "even", 4) == 0) { //设置奇偶校验为偶校验
		new_termi.c_cflag |= PARENB;
		new_termi.c_cflag &= ~PARODD;
	} else if (strncmp((char*) par, "odd", 3) == 0) { //设置奇偶校验为奇校验
		new_termi.c_cflag |= PARENB;
		new_termi.c_cflag |= PARODD;
	} else {
		new_termi.c_cflag &= ~PARENB; //设置奇偶校验为无校验
	}

	if (stopb == 1) //停止位
			{
		new_termi.c_cflag &= ~CSTOPB; //设置停止位为:一位停止位
	} else if (stopb == 2) {
		new_termi.c_cflag |= CSTOPB; //设置停止位为:二位停止位
	} else {
		new_termi.c_cflag &= ~CSTOPB; //设置停止位为:一位停止位
	}

	cfsetispeed(&new_termi, baud_lnx); /* 设置输入拨特率 */
	cfsetospeed(&new_termi, baud_lnx); /* 设置输出拨特率 */

	tcflush(Com_Port, TCIOFLUSH); /* 刷新输入输出流 */
	if (tcsetattr(Com_Port, TCSANOW, &new_termi) != 0) { /* 激活串口配置 */
		fprintf(stderr, "Set serial port parameter error!\n"); // close(Com_Port);
		return 0;
	}

	return Com_Port;
}

void usage() {
	fprintf(stderr, "功能: 向4G模块发送命令, 并监听应答报文\n");
	fprintf(stderr, "用法: at [选项]\n");
	fprintf(stderr, "-d,\t--dev\t\t\t\t要打开的设备名, 如果不指定, 默认打开 /dev/ttyS0;\n");
	fprintf(stderr, "-l,\t--listen\t\t\t\t监听开关, 0-发送报文并等待应答; 1-一直监听串口不发送at, 默认0;\n");
	fprintf(stderr, "-t,\t--times\t\t\t\t发送并监听次数, 默认0, 无限次, 最大值1024;\n");
	fprintf(stderr, "-w,\t--wait\t\t\t\t发送报文后, 读取串口数据的次数, 默认20次, 最多1024次;\n");
	fprintf(stderr, "-i,\t--interval\t\t\t\t发送报文后, 读取串口数据的时间间隔, 单位毫秒, 默认50毫秒, 最大1024毫秒;\n");
	fprintf(stderr, "-m,\t--master\t\t\t\t0-主机, 发送报文并监听串口; 1-从机, 收到报文后1秒钟应答\n");
	fprintf(stderr, "-p,\t--power\t\t\t\t开关4G模块, 0-关机, 1-开机, 2-不进行开关机操作\n");
	fprintf(stderr, "-c,\t--convert\t\t\t\t要转换at命令中的换行符\n");
	fprintf(stderr, "-a,\t--at\t\t\t\t传入的at命令, 必须以半角引号(\"\")封闭.\n");
	fprintf(stderr, "-h,\t--help\t\t\t\t打印本帮助\n");
	fprintf(stderr, "例如: at -p 1 -w 10 -i 500 -t 50 -m 0 -c -d \"/dev/ttyS0\" -a \"AT\r\n\"\n");
}

void openModel() {
	fprintf(stderr, "opening MODEL...\n");
	gpio_writebyte("/dev/gpoGPRS_SWITCH", 1);
	usleep(1000 * 100);
	gpio_writebyte("/dev/gpoGPRS_POWER", 1);
	gpio_writebyte("/dev/gpoGPRS_RST", 1);
	usleep(1000 * 1000);
	gpio_writebyte("/dev/gpoGPRS_SWITCH", 0);
	usleep(1000 * 1000);
	gpio_writebyte("/dev/gpoGPRS_SWITCH", 1);
	usleep(1000 * 1000);
}

void closeModel() {
	fprintf(stderr, "closing MODEL...\n");
	gpio_writebyte("/dev/gpoGPRS_POWER", 0);
	usleep(1000 * 700);
	gpio_writebyte("/dev/gpoGPRS_POWER", 1);
	usleep(1000 * 6000);
}

void getOptions(int argc, char *argv[], option_p pOpt) {
	int ch = 0;
	int longIndex = 0;

	if ( NULL == pOpt)
		return;

	while ((ch = getopt_long(argc, argv, optString, longOpts, &longIndex)) != -1) {
		switch (ch) {
		case 'd':
			fprintf(stderr, "option -d: %s\n", optarg);
			bzero(pOpt->dev, sizeof(pOpt->dev));
			strcpy(pOpt->dev, optarg);
			break;
		case 'l':
			fprintf(stderr, "option -l: %s\n", optarg);
			pOpt->listen = atoi(optarg);
			if (0 != pOpt->listen && 1 != pOpt->listen) {
				pOpt->listen = 0;
			}
			break;
		case 'm':
			pOpt->master = atoi(optarg);
			if (MASTER_DEV != pOpt->master && SLAVE_DEV != pOpt->master) {
				pOpt->master = MASTER_DEV;
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
		case 'p':
			fprintf(stderr, "option -p: %s\n", optarg);
			pOpt->power = atoi(optarg);
			if (pOpt->power != 0 && pOpt->power != 1 && pOpt->power != 2) {
				fprintf(stderr, "invalid power option\n");
				pOpt->power = 1;
			}
			break;
		case 'c':
			fprintf(stderr, "option -c used, convert CR LF to 0x0D and 0x0A\n");
			pOpt->convert = CONVERT_CR_LF;
			break;
		case 'a':
			fprintf(stderr, "option -a: %s\n", optarg);
			bzero(pOpt->at, sizeof(pOpt->at));
			strcpy(pOpt->at, optarg);
			break;
		case 'h':
		default:
			usage();
			exit(0);
			break;
		}
	}
}

typedef enum convState {
	e_state_init = 0,
	e_state_slash,
	e_state_crlf,
} convState_e;

typedef struct trans {
	convState_e current;
	u8 input;
	convState_e next;
}trans_s;


/*
 * use graphviz to generate svg DFA using command:
 * dot -Tsvg -o bin/at.svg at.state.dot
 * digraph at {
 *	 e_state_init -> e_state_slash [label = "'\\'"]
 *	 e_state_init -> e_state_init [label = "other"]
 *	 e_state_slash -> e_state_crlf [label = "'r'"]
 *	 e_state_slash -> e_state_crlf [label = "'n'"]
 *	 e_state_crlf -> e_state_slash [label = "'\\'"]
 *	 e_state_slash -> e_state_init [label = "other"]
 *	 e_state_crlf -> e_state_init  [label = "other"]
 * }
 */
trans_s tbl[] = {
			{e_state_init, '\\', e_state_slash},
			{e_state_slash, 'r', e_state_crlf},
			{e_state_slash, 'n', e_state_crlf},
			{e_state_crlf, '\\', e_state_slash},
		};

convState_e transition(convState_e s, u8 c)
{
	int i = 0;
	int tblLen = sizeof(tbl)/sizeof(trans_s);

	for (i=0;i<tblLen;i++) {
		if (s == tbl[i].current && c == tbl[i].input)
			return tbl[i].next;
	}

	return e_state_init;
}

const char CR = '\r'; // 0x0D
const char LF = '\n'; // 0x0A

void convertCRLF(u8* buf)
{
	if (NULL == buf)
		return;

	DEBUG_TIME_LINE("buf: %s", buf);

	convState_e s = e_state_init;
	int len = strlen(buf);
	int i = 0;
	int j = 0;
	u8* pNewBuff = malloc(len);

	if (NULL == pNewBuff)
		return;

	bzero(pNewBuff, len);

	for (i = 0; i < len; i++) {
		s = transition(s, buf[i]);
		if (e_state_init == s) {
			pNewBuff[j] = buf[i];
			j++;
		} else if (e_state_crlf == s) {
			if ('r' == buf[i]) {
				pNewBuff[j] = CR;
			} else {
				pNewBuff[j] = LF;
			}

			j++;
		} else if (e_state_slash == s) {
			if (i<len-1) {
				if (transition(s, buf[i+1]) == e_state_init) {
						pNewBuff[j] = buf[i];
						j++;
				}
			}
		}
	}

	strcpy(buf, pNewBuff);

	free(pNewBuff);
}

void printOpt(option_p pOpt) {
	fprintf(stderr, "dev: %s\n", pOpt->dev);
	fprintf(stderr, "listen: %u\n", pOpt->listen);
	fprintf(stderr, "times: %u\n", pOpt->times);
	fprintf(stderr, "wait: %u\n", pOpt->wait);
	fprintf(stderr, "inv: %u\n", pOpt->inv);
	fprintf(stderr, "convert: %d\n", pOpt->convert);
	fprintf(stderr, "master: %d\n", pOpt->master);
	fprintf(stderr, "at: %s\n", pOpt->at);
	fprintf(stderr, "power: %u\n", pOpt->power);
}

void printBuf(u8* buf, u32 bufSize)
{
	u32 i = 0;

	if (NULL == buf || 0 == bufSize) {
		return;
	}

	for (i=0; i < (bufSize-1); i++) {
		fprintf(stderr, "%02X ", buf[i]);
	}
	fprintf(stderr, "%02X\n", buf[i]);
}

s8 sendcom(int fd, u8 *buf, u32 bufSize) {
	if (0 == bufSize || NULL == buf)
		return FALSE;

	if (write(fd, buf, bufSize) < 0) {
		DEBUG_TIME_LINE("send error!\n");
		return FALSE;
	}

	DEBUG_OUT("[send]bufSize:%d; ", bufSize);
	fprintf(stderr,"%s\n", buf);

	printBuf(buf, strlen(buf));

	return TRUE;
}

void readcom(int fd, u8 *buf, u32 *bufSize) {
	if ( NULL == bufSize || NULL == buf)
		return;

	*bufSize = read(fd, buf, 2048);

	if (*bufSize > 0) {
		DEBUG_OUT("[read]bufSize:%d; ", *bufSize);
		fprintf(stderr,"%s\n", buf);
	} else {
		DEBUG_OUT("[read]no data\n", bufSize);
	}
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage();
		exit(0);
	}

	int fd = -1;
	u8 rbuf[2048] = { 0 };
	u8 sbuf[2048] = { 0 };
	u32 sbufSize = sizeof(sbuf);
	u32 rbufSize = sizeof(rbuf);
	u32 sendcnt = 0;
	int cnt = 0;
	fd_set fds = { };
	struct timeval timeout = { };
	int nready = 0;
	option_s options = { };

	bzero(&options, sizeof(options));
	getOptions(argc, argv, &options);
	printOpt(&options);

	if (0 == options.power) {
		closeModel();
		exit(0);
	} else if (1 == options.power) {
		openModel();
	}

	if (strlen(options.dev) > 0) {
		fd = OpenMuxCom(options.dev, 115200, (unsigned char*) "none", 1, 8);
	} else {
		fd = OpenMuxCom("/dev/ttyS0", 115200, (unsigned char*) "none", 1, 8);
	}

	if (fd < 0) {
		perror("OpenMuxCom failed!");
		exit(1);
	}

	if (CONVERT_CR_LF == options.convert) {
		convertCRLF(options.at);
	}

	if (strlen(options.at) == 0) {
		goto ret;
	} else {
		strcpy(sbuf, options.at);
		sbufSize = strlen(options.at);
	}

	if (1 == options.listen) { //一直监听串口
		while (1) {
			usleep(options.inv * 1000);

			nready = 0;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			nready = select(fd + 1, &fds, NULL, NULL, &timeout);
			if (nready > 0) {
				readcom(fd, rbuf, &rbufSize);
			}
		}
	} else if (MASTER_DEV == options.master) { //主机, 发送报文并监听串口
		sendcnt = 0;
		while ((options.times > 0) ? (sendcnt < options.times) : 1) {
			usleep(options.inv * 1000);
			if (sendcom(fd, sbuf, sbufSize) == FALSE) {
				goto ret;
			}

			cnt = 0;
			nready = 0;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			timeout.tv_sec = 1;
			timeout.tv_usec = 0;

			while (cnt < options.wait) {
				sleep(1);
				nready = select(fd + 1, &fds, NULL, NULL, &timeout);
				if (nready > 0) {
					DEBUG_TIME_LINE("");
					readcom(fd, rbuf, &rbufSize);
				}
				cnt++;
			}

			sendcnt++;
		}
	} else if (SLAVE_DEV == options.master) { //从机, 收到报文后1秒钟应答
		while (1) {
			sleep(1);
			nready = 0;
			FD_ZERO(&fds);
			FD_SET(fd, &fds);
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			nready = select(fd + 1, &fds, NULL, NULL, &timeout);
			if (nready > 0) {
				DEBUG_TIME_LINE("");
				readcom(fd, rbuf, &rbufSize);
				sleep(1);
				sendcom(fd, sbuf, sbufSize);
			}
		}
	}

	ret: close(fd);
	exit(0);
}
