/************关于本文档*************************************************************
 *filename: telnet-server.c
 *purpose: 这是在Linux下用C语言写的telnet服务器，没有用户名和密码，直接以
 开启服务者的身份登录系统
 *wrote by: zhoulifa(zhoulifa@163.com) 周立发(http://zhoulifa.bokee.com)
 Linux爱好者 Linux知识传播者 SOHO族 开发者 最擅长C语言
 *date time:2007-01-27 17:02
 *Note: 任何人可以任意复制代码并运用这些文档，当然包括你的商业用途
 * 但请遵循GPL
 *Thanks to: Google.com
 *Hope:希望越来越多的人贡献自己的力量，为科学技术发展出力
 * 科技站在巨人的肩膀上进步更快！感谢有开源前辈的贡献！
 **********************************************************************************/
#include <stdarg.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>

#define DEFAULTIP "127.0.0.1"
#define DEFAULTPORT "23"
#define DEFAULTBACK "10"
#define DEFAULTDIR "/tmp"
#define DEFAULTLOG "/tmp/telnet-server.log"

void prterrmsg(char *msg);
#define prterrmsg(msg) { perror(msg); abort(); }
void wrterrmsg(char *msg);
#define wrterrmsg(msg) { fputs(msg, logfp); fputs(strerror(errno),\
						logfp); fflush( logfp);\
						abort();\
						}

void prtinfomsg(char *msg);
#define prtinfomsg(msg) { fputs(msg, stdout); }
void wrtinfosg(char *msg);
#define wrtinfomsg(msg) { fputs(msg, logfp); fflush(logfp);}

#define MAXBUF 1024

char buffer[MAXBUF + 1];
char *host = 0;
char *port = 0;
char *back = 0;
char *dirroot = 0;
char *logdir = 0;
unsigned char daemon_y_n = 0;
FILE *logfp;

#define MAXPATH 150

/*------------------------------------------------------
 *--- AllocateMemory - 分配空间并把d所指的内容复制
 *------------------------------------------------------
 */
void AllocateMemory(char **s, int l, char *d)
{
	*s = malloc(l + 1);
	bzero(*s, l + 1);
	memcpy(*s, d, l);
}

/*------------------------------------------------------
 *--- getoption - 分析取出程序的参数
 *------------------------------------------------------
 */
void getoption(int argc, char **argv)
{
	int c, len;
	char *p = 0;

	opterr = 0;
	while (1) {
		int option_index = 0;
		static struct option long_options[] = { { "host", 1, 0, 0 }, { "port", 1, 0,
				0 }, { "back", 1, 0, 0 }, { "dir", 1, 0, 0 }, { "log", 1, 0, 0 }, {
				"daemon", 0, 0, 0 }, { 0, 0, 0, 0 } };
		/* 本程序支持如一些参数：
		 * --host IP地址 或者 -H IP地址
		 * --port 端口 或者 -P 端口
		 * --back 监听数量 或者 -B 监听数量
		 * --dir 服务默认目录 或者 -D 服务默认目录
		 * --log 日志存放路径 或者 -L 日志存放路径
		 * --daemon 使程序进入后台运行模式
		 */
		c = getopt_long(argc, argv, "H:P:B:D:L", long_options, &option_index);
		if (c == -1 || c == '?')
			break;

		if (optarg)
			len = strlen(optarg);
		else
			len = 0;

		if ((!c && !(strcasecmp(long_options[option_index].name, "host")))
				|| c == 'H')
			p = host = malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "port")))
				|| c == 'P')
			p = port = malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "back")))
				|| c == 'B')
			p = back = malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "dir")))
				|| c == 'D')
			p = dirroot = malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "log")))
				|| c == 'L')
			p = logdir = malloc(len + 1);
		else if ((!c && !(strcasecmp(long_options[option_index].name, "daemon")))) {
			daemon_y_n = 1;
			continue;
		} else
			break;
		bzero(p, len + 1);
		memcpy(p, optarg, len);
	}
}

/*
 *	用下列命令编译程序：
 *	gcc -Wall telnet-server -o telnetd
 *
 *	启动telnet服务：
 *	./telnetd --daemon #以root用户身份在23端口（即telnet默认端口服务）
 *	或
 *	./telnetd -P 7838 #以非root用户身份
 *
 *	然后开启一个新终端，telnet连接自己的服务器试试，如：
 *	telnet 127.0.0.1
 *	或
 *	telnet 127.0.0.1 7838
 *
 *	不需要输入用户名和密码，直接以启动telnet服务的用户的身份登录系统了。
 *	输入系统命令体验一下吧！
 */
int main(int argc, char **argv)
{
	struct sockaddr_in addr;
	int sock_fd;
	socklen_t addrlen;

	/* 获得程序工作的参数，如 IP 、端口、监听数、网页根目录、目录存放位置等 */
	getoption(argc, argv);

	if (!host) {
		addrlen = strlen(DEFAULTIP);
		AllocateMemory(&host, addrlen, DEFAULTIP);
	}
	if (!port) {
		addrlen = strlen(DEFAULTPORT);
		AllocateMemory(&port, addrlen, DEFAULTPORT);
	}
	if (!back) {
		addrlen = strlen(DEFAULTBACK);
		AllocateMemory(&back, addrlen, DEFAULTBACK);
	}
	if (!dirroot) {
		addrlen = strlen(DEFAULTDIR);
		AllocateMemory(&dirroot, addrlen, DEFAULTDIR);
	}
	if (!logdir) {
		addrlen = strlen(DEFAULTLOG);
		AllocateMemory(&logdir, addrlen, DEFAULTLOG);
	}

	printf("host=%s port=%s back=%s dirroot=%s logdir=%s %s是后台工作模式(进程ID：%d)\n",
	host, port, back, dirroot, logdir, daemon_y_n?"":"不", getpid());

	/* fork() 两次处于后台工作模式下 */
	if (daemon_y_n) {
		if (fork())
			exit(0);
		if (fork())
			exit(0);
		close(0), close(1), close(2);
		logfp = fopen(logdir, "a+");
		if (!logfp)
			exit(0);
	}

	/* 处理子进程退出以免产生僵尸进程 */
	signal(SIGCHLD, SIG_IGN);

	/* 创建 socket */
	if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		if (!daemon_y_n) {
			prterrmsg("socket()");
		} else {
			wrterrmsg("socket()");
		}
	}

	/* 设置端口快速重用 */
	addrlen = 1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &addrlen, sizeof(addrlen));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(host);
	addrlen = sizeof(struct sockaddr_in);
	/* 绑定地址、端口等信息 */
	if (bind(sock_fd, (struct sockaddr *) &addr, addrlen) < 0) {
		if (!daemon_y_n) {
			prterrmsg("bind()");
		} else {
			wrterrmsg("bind()");
		}
	}

	/* 开启临听 */
	if (listen(sock_fd, atoi(back)) < 0) {
		if (!daemon_y_n) {
			prterrmsg("listen()");
		} else {
			wrterrmsg("listen()");
		}
	}
	while (1) {
		int new_fd;
		addrlen = sizeof(struct sockaddr_in);
		/* 接受新连接请求 */
		new_fd = accept(sock_fd, (struct sockaddr *) &addr, &addrlen);
		if (new_fd < 0) {
			if (!daemon_y_n) {
				prterrmsg("accept()");
			} else {
				wrterrmsg("accept()");
			}
			break;
		}
		bzero(buffer, MAXBUF + 1);
		sprintf(buffer, "连接来自于: %s:%d\n", inet_ntoa(addr.sin_addr),
				ntohs(addr.sin_port));
		if (!daemon_y_n) {
			prtinfomsg(buffer);
		} else {
			wrtinfomsg(buffer);
		}
		/* 产生一个子进程去处理请求，当前进程继续等待新的连接到来 */
		if (!fork()) {
			/* 把socket连接作为标准输入、输出、出错句柄来用 */
			dup2(new_fd, 0);
			dup2(new_fd, 1);
			dup2(new_fd, 2);
			/* 切换到指定目录工作 */
			chdir(dirroot);
			/* 交互式执行shell */
			execl("/bin/bash", "-l", "--login", "-i", "-r", "-s", (char *) NULL);
		}
		close(new_fd);
	}
	close(sock_fd);
	return 0;
}

