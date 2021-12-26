#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <errno.h>
#include "pnox.h"

/***************************************************************************** 
 * NAME : pxexec()    
 * DESC : pnox fork and exec
 *****************************************************************************/
int pxexec(tHandle, mHandle)
T_HAND  *tHandle;
M_HAND  *mHandle;
{
	pid_t	xpid;
    char    cmd[256], *homedir, *aphomedir, trnm[16];
	int		pair[2], retc, sndlen;
	struct	iovec	iovec[2];

    homedir = getenv("PNOX_HOME");
    if (homedir == NULL)
        return(-1);

    aphomedir = getenv("PNOX_AHOME");
    if (aphomedir == NULL) {
        return(-2);
	}

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0) {
        return(-3);
	}

	iovec[0].iov_base = (void *)&mHandle->iochdr;
	iovec[0].iov_len  = sizeof(struct iochdr);
	iovec[1].iov_base = mHandle->sndb;
	iovec[1].iov_len  = mHandle->sndl;

	sndlen = iovec[0].iov_len + iovec[1].iov_len; 
	retc = sndlen + 512;
	setsockopt(pair[1], SOL_SOCKET, SO_SNDBUF, &retc, sizeof(int));
	retc = writev(pair[1], iovec, 2);
	if (retc < 0)
	{
		close(pair[0]);
		close(pair[1]);
		pxsyslog("pxexec", "writev error %d", errno);
		return(-5);
	}

    switch ((xpid = fork()))
    {
    case -1:    /* error    */
        return(-4);
    case  0:    /* child    */
		close(pair[1]);
        dup2(pair[0], 0);
        close(pair[0]);

		sprintf(trnm, "%.*s", (int)sizeof(mHandle->trnm), mHandle->trnm);
        sprintf(cmd, "%s/%s/%s", aphomedir, BIN_PATH, trnm);
        retc = execl(cmd, trnm, (char *)0);
        if (retc < 0)
        {
            pxsyslog("pxexec", "CHECK please [%s]", cmd);
            exit(1);
        }
        exit(0);
    default:    /* parent   */
        close(pair[0]);
        close(pair[1]);
		break;
	}

    return(0);
}

