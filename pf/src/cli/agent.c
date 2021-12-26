#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netinet/tcp.h>
#include <errno.h>
#include "pnoxcli.h"

int pxconnect(char *ipad, int port)
{
	struct	sockaddr_in	sock_in;
	int	sock;
	int	option;

printf("pnoxcli agent start\n");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		return(-1);

	option = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));
	option = 65 * 1024;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &option, sizeof(int));
	option = 65 * 1024;
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &option, sizeof(int));
#if 0
	option = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(int));
	fcntl(sock, F_SETFL, O_NONBLOCK);
#endif

	sock_in.sin_addr.s_addr = inet_addr(ipad);
	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(port);

	if (connect(sock, (struct sockaddr *)&sock_in, sizeof(sock_in)) < 0)
	{
printf("connect fail errno[%d]\n", errno);
		return(-1);
	}

printf("connect OK\n");
	return(sock);	
}


