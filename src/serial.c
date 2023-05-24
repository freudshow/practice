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

static const char *optString = "l:t:w:i:c:b:d:s:p:f:m:h";
static const struct option longOpts[] = { { "listen", required_argument, NULL,
		'l' }, { "times", required_argument, NULL, 't' }, { "wait",
		required_argument, NULL, 'w' }, { "inv", required_argument, NULL, 'i' },
		{ "com", required_argument, NULL, 'c' }, { "baud", required_argument,
				NULL, 'b' }, { "data", required_argument, NULL, 'd' }, { "stop",
				required_argument, NULL, 's' }, { "par", required_argument,
				NULL, 'p' }, { "frame", required_argument, NULL, 'f' }, {
				"master", no_argument, NULL, 'm' }, { "help", no_argument, NULL,
				'h' }, { NULL, no_argument, NULL, 0 } };

void set_baudrate(struct termios *opt, unsigned int baudrate)
{
	int baud_lnx = B2400;
	switch (baudrate) {
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
	cfsetispeed(opt, baud_lnx);
	cfsetospeed(opt, baud_lnx);
}

void set_stopbit(struct termios *opt, const char *stopbit)
{
	if (0 == strcmp(stopbit, "1")) {
		opt->c_cflag &= ~CSTOPB; /* 1位停止位*/
	} else if (0 == strcmp(stopbit, "1.5")) {
		opt->c_cflag &= ~CSTOPB; /* 1.5位停止位 */
	} else if (0 == strcmp(stopbit, "2")) {
		opt->c_cflag |= CSTOPB; /* 2位停止位*/
	} else {
		opt->c_cflag &= ~CSTOPB; /* 1位停止位*/
	}
}

// set_data_bit函数
// CSIZE--字符长度掩码。取值为 CS5, CS6, CS7, 或 CS8
void set_data_bit(struct termios *opt, unsigned int databit)
{
	opt->c_cflag &= ~CSIZE;
	switch (databit) {
	case 8:
		opt->c_cflag |= CS8;
		break;
	case 7:
		opt->c_cflag |= CS7;
		break;
	case 6:
		opt->c_cflag |= CS6;
		break;
	case 5:
		opt->c_cflag |= CS5;
		break;
	default:
		opt->c_cflag |= CS8;
		break;
	}
}

// set_parity函数
// ‘N’和‘n’（无奇偶校验）、‘E’和‘e’（表示偶校验）、‘O’和‘o’（表示奇校验）。
void set_parity(struct termios *opt, char parity)
{
	switch (parity) {
	case 'N': /*无校验*/
	case 'n':
		opt->c_cflag &= ~PARENB;
		break;
	case 'E': /*偶校验*/
	case 'e':
		opt->c_cflag |= PARENB;
		opt->c_cflag &= ~PARODD;
		break;
	case 'O': /*奇校验*/
	case 'o':
		opt->c_cflag |= PARENB;
		opt->c_cflag |= ~PARODD;
		break;
	default: /*其它选择为无校验 */
		opt->c_cflag &= ~PARENB;
		break;
	}
}

int set_port_attr(int fd, int baudrate, int databit, const char *stopbit,
		char parity, int vtime, int vmin)
{
	struct termios opt;
	tcgetattr(fd, &opt);

	set_baudrate(&opt, baudrate);
	set_data_bit(&opt, databit);
	set_parity(&opt, parity);
	set_stopbit(&opt, stopbit);

	opt.c_cflag &= ~CRTSCTS;       // 不使用硬件流控制
	opt.c_cflag |= CLOCAL | CREAD; // CLOCAL--忽略 modem 控制线,本地连线, 不具数据机控制功能,
								   // CREAD--使能接收标志
	/*
	 IXON--启用输出的 XON/XOFF 流控制
	 IXOFF--启用输入的 XON/XOFF 流控制
	 IXANY--允许任何字符来重新开始输出
	 IGNCR--忽略输入中的回车
	 */
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	opt.c_oflag &= 0; //输出模式
	/*
	 ICANON--启用标准模式 (canonical mode)。允许使用特殊字符 EOF, EOL,
	 EOL2, ERASE, KILL, LNEXT, REPRINT, STATUS, 和 WERASE，以及按行的缓冲。
	 ECHO--回显输入字符
	 ECHOE--如果同时设置了 ICANON，字符 ERASE 擦除前一个输入字符，WERASE 擦除前一个词
	 ISIG--当接受到字符 INTR, QUIT, SUSP, 或 DSUSP 时，产生相应的信号
	 */
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	opt.c_cc[VMIN] = vmin;   // 设置非规范模式下的超时时长和最小字符数：
	opt.c_cc[VTIME] = vtime; // VTIME与VMIN配合使用，是指限定的传输或等待的最长时间

	tcflush(fd, TCIFLUSH); /* TCIFLUSH-- update the options and do it NOW */
	return (tcsetattr(fd, TCSANOW, &opt)); /* TCSANOW--改变立即发生 */
}

int open_tty(comConfig_p pConfig)
{
	int fd = open(pConfig->port, O_RDWR | O_NOCTTY);

	if (fd == -1) {
		printf("\n[%s()%d]tty: %s, fd: %d\n", __FUNCTION__, __LINE__, pConfig->port, fd);
		printf("%d, %s\n", errno, strerror(errno));
		return -1;
	}

	printf("open fd-{%d}  Successful \r\n", fd);

	tcflush(fd, TCIOFLUSH); //溢出数据可以接收，但不读
	fcntl(fd, F_SETFL, FNDELAY);

	char stop[4] = { 0 };
	switch (pConfig->stopb) {
	case 1:
		stop[0] = '1';
		break;
	case 2:
		stop[0] = '2';
		break;
	default:
		stop[0] = '1';
		break;
	}

	char par = 'E';
	switch (pConfig->par) {
	case parEven:
		par = 'E';
		break;
	case parOdd:
		par = 'O';
		break;
	case parNone:
		par = 'N';
		break;
	default:
		break;
	}

	int ret = set_port_attr(fd, pConfig->baud, pConfig->bits, stop, par, 0, 60); // vtime=0 read时最少字符数
	if (ret < 0) {
		printf("set baud failed\n");
		return -2;
	}
	return fd;
}

void usage()
{
	fprintf(stderr, "功能: 向串口发送报文, 并监听应答报文\n");
	fprintf(stderr, "用法: serial [选项]\n");
	fprintf(stderr,
			"-l,    --listen    监听开关, 0-发送报文并等待应答; 1-一直监听一个串口不发送报文, 默认0;\n");
	fprintf(stderr, "-t,    --times     发送并监听次数, 默认0, 无限次, 最大值1024;\n");
	fprintf(stderr, "-w,    --wait      发送报文后, 读取串口数据的次数, 默认20次, 最多1024次;\n");
	fprintf(stderr,
			"-i,    --inv       发送报文后, 读取串口数据的时间间隔, 单位毫秒, 默认50毫秒, 最大1024毫秒;\n");
	fprintf(stderr,
			"-c,    --com       向哪个串口发送并监听数据, 必须是完整路径且必须以半角引号(\"\")封闭,\n");
	fprintf(stderr, "                        默认\"%s\", 即RS-485 I;\n", S4851);
	fprintf(stderr, "-b,    --baud      设置串口波特率, 默认2400;\n");
	fprintf(stderr,
			"-d,    --data      设置串口数据位, 6, 7, 8为有效值, 其他值视为使用默认值, 默认值8;\n");
	fprintf(stderr,
			"-s,    --stop      设置串口停止位, 1, 2为有效值, 其他值视为使用默认值, 默认值1;\n");
	fprintf(stderr,
			"-p,    --par            设置串口校验位, 0-偶, 1-奇, 2-无, 其他值视为使用默认值, 默认值0;\n");
	fprintf(stderr, "-f,    --frame     传入的报文, 必须以半角引号(\"\")封闭.\n");
	fprintf(stderr,
			"                        默认发送\"FE FE FE FE 68 12 34 56 78 90 16\".\n");
	fprintf(stderr, "-h,	--help      打印本帮助\n");
	fprintf(stderr, "如果任何参数都不传入, 程序使用默认参数来监听\"%s\".\n", S4851);
}

s8 getComConfig(option_p pOpt, comConfig_p pConfig)
{
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

s8 sendcom(int fd, u8 *buf, u32 bufSize)
{
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

void readcom(int fd, u8 *buf, u32 bufSize)
{
	if ( 0 == bufSize || NULL == buf)
		return;

	int ret = read(fd, buf, bufSize);

	if (ret > 0) {
		DEBUG_OUT("[read]bufSize: %d; ", ret);
		printBuf(buf, ret);
	}
}

void setDefaultPara(comConfig_p pConfig)
{
	if ( NULL == pConfig)
		return;

	bzero(pConfig, sizeof(comConfig_s));

	strcpy(pConfig->port, S4851);
	pConfig->baud = baud2400;
	pConfig->par = parEven;
	pConfig->stopb = 1;
	pConfig->bits = 8;
}

void setDefaultOpt(option_p pOpt)
{
	if ( NULL == pOpt)
		return;

	bzero(pOpt, sizeof(option_s));
	pOpt->listen = 0;
	pOpt->master = MASTER_DEV;
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

int baudValid(int baud)
{
	int i = 0;
	int array[] = { 50, 75, 110, 150, 200, 600, 1200, 2400, 4800, 9600, 19200,
			38400, 57600, 115200 };

	for (i = 0; i < sizeof(array); i++) {
		if (baud == array[i])
			return TRUE;
	}

	return FALSE;
}

void getOptions(int argc, char *argv[], option_p pOpt)
{
	int ch = 0;
	int longIndex = 0;

	if ( NULL == pOpt)
		return;

	while ((ch = getopt_long(argc, argv, optString, longOpts, &longIndex)) != -1) {
		switch (ch) {
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

int main(int argc, char *argv[])
{
	comConfig_s config = { };
	option_s options = { };
	u8 rbuf[2048] = { 0 };
	u8 sbuf[2048] = { 0 };
	u32 sbufSize = sizeof(sbuf);
	u32 rbufSize = sizeof(rbuf);
	int fd = -1;
	u32 sendcnt = 0;
	int cnt = 0;
	fd_set fds = { };
	struct timeval timeout = { };
	int nready = 0;

	setDefaultOpt(&options);
	setDefaultPara(&config);

	if (argc > 1) {
		getOptions(argc, argv, &options);
	} else {
		options.listen = 1;
	}

	getComConfig(&options, &config);
	DEBUG_TIME_LINE("port: %s, baud: %d, parity: %d, stop: %d, bits: %d\n",
			config.port, config.baud, config.par, config.stopb, config.bits);
	printOpt(&options);
	if ((fd = open_tty(&config)) < 0) {
		perror("open failed!");
		exit(1);
	}

	if (readFrm(options.frame, sbuf, &sbufSize) == FALSE) {
		goto ret;
	}

	if (1 == options.listen) { //一直监听串口
		while (1) {
			usleep(50 * 1000);
			readcom(fd, rbuf, rbufSize);

		}
	} else if (MASTER_DEV == options.master) { //主机, 发送报文并监听串口
		sendcnt = 0;
		while ((options.times > 0) ? (sendcnt < options.times) : 1) {
			usleep(options.inv * 1000);
			if (sendcom(fd, sbuf, sbufSize) == FALSE) {
				goto ret;
			}

			cnt = 0;

			while (cnt < options.wait) {
				usleep(50 * 1000);
				readcom(fd, rbuf, rbufSize);
				cnt++;
			}

			sendcnt++;
		}
	} else if (SLAVE_DEV == options.master) { //从机, 收到报文后应答
		while (1) {
			usleep(50 * 1000);
			readcom(fd, rbuf, rbufSize);
			usleep(options.inv * 1000);
			sendcom(fd, sbuf, sbufSize);
		}
	}

	ret: close(fd);
	exit(0);
}
