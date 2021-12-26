/***********************************************************
 * NAME : pxver.c
 * DESC : pnox platform version
 ***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <unistd.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "pnox.h"

static int      u_sock;         /* event socket         */
static char	u_path[128];

static char    pnox_home[256], cmd[256];
int pnox_sock_init();

int main(argc, argv)
int     argc;
char    *argv[];
{
	fd_set	rlist;
	struct  timeval timeval;
	struct	eventmsg eventmsg;
	struct	pxversion pxversion;
	int		retc, retn, len, nfound;

	retc = homedir_pxmon(pnox_home, NULL);
	if (retc < 0)
	{
		printf("homedir error %d\n", retc);
		return(0);
	}

	retc = pnox_sock_init();
	if (retc < 0)
	{
		printf("ipc error\n");
		exit(-1);
	}

	memset(&eventmsg, 0x00, sizeof(struct eventmsg));
	eventmsg.opt = 'v';
	memcpy(eventmsg.arg, "", sizeof(eventmsg.arg));

	sprintf(cmd, "%s/%s/PXIPC", pnox_home, IPC_PATH);
	retn = pxsvrsnd(cmd, (char *)&eventmsg, sizeof(struct eventmsg));
	if (retn < 0)
	{
		printf("pnox daemon is not run or fail\n");
		unlink(u_path);
                close(u_sock);
		exit(-1);
	}

	FD_ZERO(&rlist);
	FD_SET(u_sock, &rlist);
	timeval.tv_sec = 5;
	timeval.tv_usec = 0;
	nfound = select(u_sock+1, &rlist, NULL, NULL, &timeval);
	if (nfound <= 0)
	{
		printf("ipc not received\n");
		unlink(u_path);
                close(u_sock);
		exit(-1);
	}

	len = read(u_sock, &eventmsg, sizeof(eventmsg));
	if (len != sizeof(eventmsg))
	{
		printf(" unknown version\n");
		unlink(u_path);
                close(u_sock);
		exit(-1);
	}

	memcpy(&pxversion, eventmsg.arg, sizeof(struct pxversion));
	printf("pnox %d.%d\n", pxversion.major, pxversion.minor);

	close(u_sock);
	unlink(u_path);

	return(0);
}
	
/*****************************************************************************
 * NAME : pnox_sock_init()
 * DESC : initialize socket to receive event from other process
 *****************************************************************************/
int	pnox_sock_init()
{
	struct  sockaddr_un sock_svr;

	sprintf(u_path, "%s/%s/PXVER", pnox_home, IPC_PATH);
	unlink(u_path);
	u_sock = -1;

	u_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	bzero(&sock_svr, sizeof(sock_svr));
	sock_svr.sun_family = AF_UNIX;
	strcpy(sock_svr.sun_path, u_path);
	if (bind(u_sock, (struct sockaddr *)&sock_svr, sizeof(sock_svr)) < 0)
	{
		close(u_sock);
		u_sock = -1;
		return(-1);
	}

	fcntl(u_sock, F_SETFL, O_NONBLOCK);
	return(0);
}


