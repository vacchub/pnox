/******************************************************************************/
/*  Components  : pnoxsgw.c                                                   */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include "pnox.h"
#include "pxmon.h"

extern int exec_sca();
extern int exec_rta();
extern int initial(struct pxmon*);

int initial_sock();
void shut_listen();

struct	pxmon *pxmon;

struct	sgwtbl {
	int flag;
	int	sock;		/* socket		*/
	int	port;		/* service port	*/
} sgwtbl[] = {
 { 1, 0, PORT_SCA },
 { 1, 0, PORT_RTA },
 { 0, 0, 0		   }
};

static	int sock;

/***************************************************************************** 
 * NAME	: main()
 * DESC	: PNOX network session gateway
 *****************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	struct	sockaddr_in sockcli;
	struct	timeval	  timeval;
	int 	csock, maxfds, nfound;
	fd_set	clist;
	int	argl, ii;

	signal(SIGTERM, shut_listen);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT,  SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	pxsyslog("pnoxsgw", "pnoxsgw start");
	pxmon = _open_pxmon(-1);
	if (pxmon == NULL)
	{
		pxsyslog("pnoxsgw", "pnoxsgw pxmon open error");
		return(0);
	}

	if (initial(pxmon) < 0)
	{
		pxsyslog("pnoxsgw", "pnoxsgw initial error");
		return(0);
	}

	initial_sock();

	for (ii = 0; sgwtbl[ii].flag > 0; ii++)
	{
		if (sgwtbl[ii].sock < 0)
		{
			pxsyslog("pnoxsgw", "pnoxsgw sock error");
			return(0);
		}
	}

	for (;;)
	{
		if (getppid() <= 1)
			break;

		maxfds = 0;
		FD_ZERO(&clist);
		for (ii = 0; sgwtbl[ii].flag > 0; ii++)
		{
			FD_SET(sgwtbl[ii].sock, &clist);
			if (maxfds < sgwtbl[ii].sock)
				maxfds = sgwtbl[ii].sock;
		}

		timeval.tv_sec = 5;
		timeval.tv_usec = 0;
		nfound = select(maxfds+1, &clist, NULL, NULL, &timeval);
		if (nfound <= 0)
			continue;


		for (ii = 0; sgwtbl[ii].flag > 0; ii++)
		{
			if (sgwtbl[ii].port == PORT_SCA &&
				 FD_ISSET(sgwtbl[ii].sock, &clist))
			{
				argl = sizeof(sockcli);
				csock = accept(sgwtbl[ii].sock, 
					(struct sockaddr *)&sockcli, (socklen_t *)&argl);
				if (csock < 0)
					continue;
pxsyslog("pnoxsgw", "pnoxsgw exec_sca start");
				exec_sca(csock, &sockcli);
				break;
			}
			else if (sgwtbl[ii].port == PORT_RTA &&
				 FD_ISSET(sgwtbl[ii].sock, &clist))
			{
				argl = sizeof(sockcli);
				csock = accept(sgwtbl[ii].sock, 
					(struct sockaddr *)&sockcli, (socklen_t *)&argl);
				if (csock < 0)
					continue;
				exec_rta(csock, &sockcli);
				break;
			}
		}

		close(csock);
	}

	shut_listen(0);
	_close_pxmon(-1);
	pxsyslog("pnoxsgw", "pnoxsgw end");
	return(0);
}

/***************************************************************************** 
 * NAME	: initial_sock()	
 * DESC	: Initial socket interface for all service port. 
 *****************************************************************************/
int initial_sock()
{
	struct 	sockaddr_in socknet;
	int arg, ii;

	for (ii = 0; sgwtbl[ii].flag > 0; ii++)
	{
		sgwtbl[ii].sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sgwtbl[ii].sock < 0)
			return(-1);
		
		arg = 1;
		setsockopt(sgwtbl[ii].sock, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(arg));
		arg = 4 * 1024 * 1024;
		setsockopt(sgwtbl[ii].sock, SOL_SOCKET, SO_RCVBUF, &arg, sizeof(arg));
		setsockopt(sgwtbl[ii].sock, SOL_SOCKET, SO_SNDBUF, &arg, sizeof(arg));
	

		socknet.sin_family      = AF_INET;
		socknet.sin_addr.s_addr = INADDR_ANY;
		socknet.sin_port        = htons(sgwtbl[ii].port);
		if (bind(sgwtbl[ii].sock, (struct sockaddr *)&socknet, 
			sizeof(socknet)) != 0)
		{
			close(sgwtbl[ii].sock);
			return(-2);
		}

		listen(sgwtbl[ii].sock, 32);

		arg = 1;
		setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &arg, sizeof(int));
		fcntl(sgwtbl[ii].sock, F_SETFL, O_NONBLOCK);
	}

	return(0);
}

/***************************************************************************** 
 * NAME	: shut_listen()			
 * DESC	: shutdown listener for exit					
 *****************************************************************************/
void shut_listen(sign)
int	sign;
{
	int	ii;
	
	for (ii = 0; sgwtbl[ii].flag > 0; ii++)
	{
		if (sgwtbl[ii].sock >= 0)
			close(sgwtbl[ii].sock);
	}

	exit(0);
}

