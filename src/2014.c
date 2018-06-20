#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

/*
	Abbreviations
	8N1 8 data bits, no parity bit, 1 stop bit
	CTS Clear To Send
	DLC Data Link Connection
	DTR Data Terminal Ready
	FC Flow Control
	FCS Frame Check Sequence
	FTP File Transfer Protocol
	GSM Global System for Mobile Communications
	GPRS General Packet Radio Service
	IP Internet Protocol
	MS Mobile Station
	pppd Point to Point Protocol Daemon
	SABM Set Asynchronous Balanced Mode
	SMS Short Message Service
	TE Terminal Equipment
	UI Unnumbered Information
	UIH Unnumbered Information with Header check
*/

int main(void)
{
	long long llint = 3;
	int i = llint;
	int j = 5;
	
	printf("sizeof(long long): %d, after asign: %lld, %d, 0x%016X, j: %lld, llint: %016X, llint: %d\n", sizeof(long long), i, i, i, j, llint);
	exit(0);	
}
