#include <unistd.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <wait.h>
#include <string.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char** argv)
{
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket < 0)
		printf("create client socket fail");

	struct sockaddr_in server_addr;
	bzero((char *)&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	const char *server_ip = argv[1];
	int server_port = atoi(argv[1]);
	
	struct hostent *server = gethostbyname(server_ip);
	if(!server)
		printf("fail to get host name");

	bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);

	server_addr.sin_port = htons(server_port);
	socklen_t server_addr_len = sizeof(server_addr);

//	fcntl(client_socket, F_SETFL, O_NONBLOCK);
	if(connect(client_socket, (struct sockaddr*) &server_addr, server_addr_len) == -1 ) {
		printf("connent to server fail\n");
		exit(0);
	}
	

	char buf[] = {0x68, 0x1e, 0x00, 0x81, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0xd9, 0x01, 0x00, 0x00, 0x01, 0x2c, 0x07, 0xe2, 0x09, 0x06, 0x04, 0x0e, 0x04, 0x1b, 0x03, 0x72, 0x6c, 0xae, 0x16};
	char content[512] = {0};

	int len = send(client_socket, buf, sizeof(buf), 0);
	printf("send len: %d", len);
	int idx = 0;
	for (idx = 0 ; idx < len; idx++ ) {
		printf(" %02X", buf[idx]);
	}


	usleep(500*1000);
	len = read(client_socket, content, sizeof(content));

	printf("read len: %d", len);
	for (idx = 0 ; idx < len; idx++ ) {
		printf(" %02X", content[idx]);
	}

	printf("\n");

	close(client_socket);

	exit(0);
}