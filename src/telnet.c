#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>


#define DO 0xfd
#define WONT 0xfc
#define WILL 0xfb
#define DONT 0xfe
#define CMD 0xff
#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31
#define IAC 255
#define SB 250
#define SE 240
#define BUFLEN 200
#define ESCAPE 27

void negotiate(int sock, unsigned char *buf, int len) {
    int i;
    const char* option_code[350];
    option_code[00] = "TRANSMIT-BINARY";
    option_code[01] = "ECHO";
    option_code[03] = "SUPPRESS-GO-AHEAD";
    option_code[05] = "STATUS";
    option_code[06] = "TIMING-MARK";
    option_code[10] = "NAOCRD";
    option_code[11] = "NAOHTS";
    option_code[12] = "NAOHTD";
    option_code[13] = "NAOFFD";
    option_code[14] = "NAOVTS";
    option_code[15] = "NAOVTD";
    option_code[16] = "NAOLFD";
    option_code[17] = "EXTEND-ASCII";
    option_code[18] = "LOGOUT";
    option_code[19] = "BM";
    option_code[20] = "DET";
    option_code[23] = "SEND-LOCATION";
    option_code[24] = "TERMINAL-TYPE";
    option_code[25] = "END-OF-RECORD";
    option_code[26] = "TUID";
    option_code[27] = "OUTMRK";
    option_code[28] = "TTYLOC";
    option_code[29] = "3270-REGIME";
    option_code[30] = "X.3-PAD";
    option_code[31] = "NAWS";
    option_code[32] = "TERMINAL-SPEED";
    option_code[33] = "TOGGLE-FLOW-CONTROL";
    option_code[34] = "LINEMODE";
    option_code[35] = "X-DISPLAY-LOCATION";
    option_code[36] = "ENVIRON";
    option_code[37] = "AUTHENTICATION";
    option_code[38] = "ENCRYPT";
    option_code[39] = "NEW-ENVIRON";
    option_code[40] = "TN3270E";
    option_code[42] = "CHARSET";
    option_code[44] = "COM-PORT-OPTION";
    option_code[47] = "KERMIT";
    option_code[250] = "SB";
    option_code[240] = "SE";
    option_code[251] = "WILL";
    option_code[252] = "WONT";
    option_code[253] = "DO";
    option_code[254] = "DONT";
    option_code[255] = "IAC";
    if (buf[1] == DO && buf[2] == CMD_WINDOW_SIZE) {
        unsigned char tmp1[10] = { IAC, WILL, CMD_WINDOW_SIZE };
        if (send(sock, tmp1, 3, 0) < 0)
            exit(1);

        unsigned char tmp2[10] = { IAC, SB, CMD_WINDOW_SIZE, 0, 80, 0, 24, IAC,
        SE };
        if (send(sock, tmp2, 9, 0) < 0)
            exit(1);
        return;
    }

    /*
     * To avoid any problems, we reply to every DO with a WONT and every
     * WILL with a DO. Both of these are not required, but are included to prevent
     * any problems with most telnet servers.
     *
     * Echoing back the same characters to the server works on Linux machines,
     * but may not work on other terminals.
     *
     */
    for (i = 1; i < len; i++) {
        if (buf[i] == DO) {
            buf[i] = WONT;
        } else if (buf[i] == WILL) {
            buf[i] = DO;
        }
    }

    if (send(sock, buf, len, 0) < 0)
        exit(1);
}

static struct termios tin;

static void terminal_set(void) {
    // save terminal configuration
    tcgetattr(STDIN_FILENO, &tin);

    static struct termios tlocal;
    memcpy(&tlocal, &tin, sizeof(tin));
    // The file descriptor which has to be turned to raw mode is the standard
    // input of the parent process
    cfmakeraw(&tlocal);
    tcsetattr(STDIN_FILENO, TCSANOW, &tlocal);
}

static void terminal_reset(void) {
    // restore terminal upon exit
    tcsetattr(STDIN_FILENO, TCSANOW, &tin);
}

void connect_to_server(int sock, int port, char* address) {
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(address);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    //Connect to remote server
    if (connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    int sock;
    unsigned char buf[BUFLEN + 1];
    int len;
    int i;
	int port = 23;

    if (argc < 2 || argc > 3) {
        printf("Usage: %s address [port]\n", argv[0]);
        return 1;
    }
    
    if (argc == 3)
        port = atoi(argv[2]);

    //Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket. Error");
        return 1;
    }
    printf("Trying %s...", argv[1]);

    connect_to_server(sock, port, argv[1]);

    printf("Connected to %s\n", argv[1]);
    puts("Escape character is '^]'.");

    // set terminal
    terminal_set();
    atexit(terminal_reset);

    struct timeval ts;
    ts.tv_sec = 1; // 1 second
    ts.tv_usec = 0;

    while (1) {
        // select setup
        fd_set fds;
        FD_ZERO(&fds);
        if (sock != 0)
            FD_SET(sock, &fds);
        FD_SET(0, &fds);

        // wait for data
        int nready = select(sock + 1, &fds, (fd_set *) 0, (fd_set *) 0, &ts);
        if (nready < 0) {
            perror("select. Error");
            return 1;
        } else if (nready == 0) {
            ts.tv_sec = 1;
            ts.tv_usec = 0;
        } else if (sock != 0 && FD_ISSET(sock, &fds)) {
            // start by reading a single byte
            int rv;
            if ((rv = recv(sock, buf, 1, 0)) < 0)
                return 1;
            else if (rv == 0) {
                printf("Connection closed by the remote end\n\r");
                return 0;
            }

            if (buf[0] == CMD) {
                // read 2 more bytes
                len = recv(sock, buf + 1, 2, 0);
                if (len < 0)
                    return 1;
                else if (len == 0) {
                    printf("Connection closed by the remote end\n\r");
                    return 0;
                }
                negotiate(sock, buf, 3);
            } else {
                len = 1;
                buf[len] = '\0';
                printf("%s", buf);
                fflush(stdout);
            }
        }

        else if (FD_ISSET(0, &fds)) {
            static char crlf[] = { '\r', '\n' };
            buf[0] = getc(stdin); //fgets(buf, 1, stdin);
            if (buf[0] == '\n') { // with the terminal in raw mode we need to force a LF
                if (send(sock, crlf, 1, 0) < 0) {
                    return 1;
                }
            } else if (buf[0] == ESCAPE) {
                printf("Connection closed by the client end\n\r");
                return 0;
            } else {
                if (send(sock, buf, 1, 0) < 0)
                    return 1;
            }
        }
    }
    close(sock);
    return 0;
}

