#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "pnox.h"

/*****************************************************************************
 * NAME : pxsvrsnd()
 * DESC : Send request to service program.
 * NOTE : 지정 SVR PROCESS로 전문 전달 처리
 ****************************************************************************/
int	pxsvrsnd(svrname, msgb, msgl)
char    *svrname;
char    *msgb;
int     msgl;
{
	struct  sockaddr_un     sock_svr;
	char    sock_path[128], *homedir;
	int     send_l;
	int     sock;

	if (svrname[0] == '/')
		sprintf(sock_path, "%s", svrname);
	else
	{
		homedir = getenv("PNOX_HOME");
		if (homedir == NULL)
			return(-1);
		
		sprintf(sock_path, "%s/%s/%s", homedir, 
			IPC_PATH, svrname);
	}
	sock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock < 0)
		return (sock);
	bzero(&sock_svr, sizeof(sock_svr));
	strcpy(sock_svr.sun_path, sock_path);
	sock_svr.sun_family = AF_UNIX;
	if (connect(sock, (struct sockaddr *)&sock_svr, sizeof(sock_svr)) < 0)
	{
		close(sock);
		return(-1);
	}
	send_l = write(sock, msgb, msgl);
	close(sock);
	if (send_l != msgl)
		return(-1);
	else
		return(0);
}
