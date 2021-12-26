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
#include <errno.h>
#include "pnox.h"
#include "pxmon.h"

static int _pxsend(T_HAND *, M_HAND *, char *);

/******************************************************************************
 * NAME : pxopen()                               
 * DESC : open receive session interface.       
 ******************************************************************************/
int pxopen(tHandle, name)
T_HAND *tHandle;
char *name;	
{
	struct	pxmon	*pxmon;
    struct  sockaddr_un sock_un;
    char    sun_path[128], *homedir, tmpb[32], cmd[256];
    int sock, op, retc;

    if (tHandle->pxid != 0)
	{
        sprintf(tmpb, "u%07d", tHandle->pxid);
        memcpy(tHandle->name, tmpb, sizeof(tHandle->name));
	}
	else
	{
		memset(tHandle, 0x00, sizeof(T_HAND));
	}

	if (name != NULL) {
		memcpy(tHandle->name, name, strlen(name));
	}
    if (strlen(tHandle->name) == 0) {
        return(XINVAL);
	}

    homedir = getenv("PNOX_HOME");
    if (homedir == NULL)
	{
		pxmon = _open_pxmon(-1);
		if (pxmon == NULL)
			return(XREADY);
		memset(cmd, 0x00, sizeof(cmd));
		sprintf(cmd, "PNOX_HOME=%s", pxmon->home);
		memset(tHandle->home, 0x00, sizeof(tHandle->home));
		memcpy(tHandle->home, cmd, strlen(cmd));
		putenv(tHandle->home);
		_close_pxmon(-1);

		homedir = getenv("PNOX_HOME");
		if (homedir == NULL)
			return(XREADY);
	}

    sprintf(sun_path, "%s/%s/%.*s", homedir, QUE_PATH, 
		(int)sizeof(tHandle->name), tHandle->name);
    bzero(&sock_un, sizeof(struct sockaddr_un));
    sock_un.sun_family = AF_UNIX;
    strcpy(sock_un.sun_path, sun_path);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
	{
		sprintf(cmd, "%s/%s/%s", homedir, QUE_PATH, tHandle->name);
		unlink(cmd);
    	sock = socket(AF_UNIX, SOCK_STREAM, 0);
    	if (sock < 0)
		{
			pxsyslog("pxopen", "socket errno[%d]", errno);
        	return(XSESSION);
		}
	}

    op = 1;
    retc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(int));
	if (retc < 0)
		pxsyslog("pxopen", "setsockopt SO_REUSEADDR retc[%d] errno[%d]", retc, errno);

    op = MAX_BUF_LEN;
    retc = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &op, sizeof(int));
	if (retc < 0)
		pxsyslog("pxopen", "setsockopt SOL_SOCKET retc[%d] errno[%d]", retc, errno);

    if (bind(sock, (struct sockaddr *)&sock_un, sizeof(sock_un)) != 0)
    {
        close(sock);
		sprintf(cmd, "%s/%s/%s", homedir, QUE_PATH, tHandle->name);
		unlink(cmd);
		pxsyslog("pxopen", "bind errno[%d][%s]", errno, strerror(errno));
        return(XSESSION);
    }
	listen(sock, 20);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    tHandle->devf = sock;

	tHandle->nusr = 0;
	tHandle->ulst = (int *)malloc(sizeof(int) * MAX_PXSEND);

    tHandle->rtaf = SET_OFF;
    return(0);
}

/***************************************************************************** 
 * NAME : pxcall()                              
 * DESC : send and receive a request message or reply message.  
 *****************************************************************************/
int pxcall(tHandle, mHandle, timeout)
T_HAND  *tHandle;
M_HAND  *mHandle;
int	timeout;
{
	int retc;
	
	retc = pxsend(tHandle, mHandle);
	if (retc < 0)
		return(retc);

	retc = pxrecv(tHandle, mHandle, timeout);

	return(retc);
}

/***************************************************************************** 
 * NAME : pxsend()    
 * DESC : send request & reply message 
 *****************************************************************************/
int pxsend(tHandle, mHandle)
T_HAND  *tHandle;
M_HAND  *mHandle;
{
    int rc=0, retc, ii;
    char    sock_path[128], *homedir;

    homedir = getenv("PNOX_HOME");
    if (homedir == NULL)
        return(XREADY);

#if 0
	if ((tHandle->call[0] != 'u') && (memcmp(tHandle->call, "rta", 3) != 0) &&
		(memcmp(tHandle->call, "pxrun", 5) != 0) &&
		(tHandle->nusr == 0))
	{
		retc = pxrout(tHandle, mHandle);
		if (retc < 0)
        	return(XREGISTER);
	}
#else
	if (strlen(tHandle->call) <= 0)
	{
		retc = pxrout(tHandle, mHandle);
		if (retc < 0)
			return(XREGISTER);
	}
#endif

	if (tHandle->nusr == 0)
	{
		sprintf(sock_path, "%s/%s/%.*s", homedir, QUE_PATH, 
			(int)sizeof(tHandle->call), tHandle->call);
#if 0
pxsyslog("pxsend", "nusr0 pxsend path[%s] pxid[%d]", sock_path, tHandle->pxid);
#endif
		retc = _pxsend(tHandle, mHandle, sock_path);
		if (retc < 0)
			return(XIO);
	}
	else if (tHandle->nusr > 0)
	{
		for (ii = 0; ii < tHandle->nusr && ii < MAX_PXSEND; ii++)
		{
    		sprintf(sock_path, "%s/%s/u%07d", homedir, QUE_PATH, *(tHandle->ulst + ii));
#if 0
pxsyslog("pxsend", "nusr[%d] pxsend path[%s]", tHandle->nusr, sock_path);
#endif
			tHandle->pxid = *(tHandle->ulst + ii);
			retc = _pxsend(tHandle, mHandle, sock_path);
			if (retc < 0)
			{
				rc = retc;
				pxsyslog("pxsend", "_pxsend error retc[%d][%s]", retc, sock_path);
				continue;
			}
		}
	}

    return(rc);
}

/***************************************************************************** 
 * NAME : _pxsend()    
 * DESC : inter send request & reply message 
 *****************************************************************************/
int _pxsend(tHandle, mHandle, sock_path)
T_HAND  *tHandle;
M_HAND  *mHandle;
char	*sock_path;
{
    struct  sockaddr_un sock_un;
    int csock, op, sndl, retc;
    char    sndb[65*1024];

    bzero(&sock_un, sizeof(sock_un));
    sock_un.sun_family = AF_UNIX;
    strcpy(sock_un.sun_path, sock_path);

    csock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (csock < 0)
        return(XSESSION);

    op = MAX_BUF_LEN;
    setsockopt(csock, SOL_SOCKET, SO_SNDBUF, &op, sizeof(int));

	if (connect(csock, (struct sockaddr *)&sock_un, sizeof(sock_un)) != 0)
	{
		close(csock);
		return(XCONNECT);
	}

    memset(sndb, 0x00, sizeof(sndb));	
	mHandle->iochdr.pxid = tHandle->pxid;
	mHandle->iochdr.type[0] = mHandle->type[0];
	mHandle->iochdr.type[1] = mHandle->type[1];
	mHandle->iochdr.type[2] = mHandle->type[2];
	mHandle->iochdr.type[3] = mHandle->type[3];
    memcpy(mHandle->iochdr.name, tHandle->name, sizeof(mHandle->iochdr.name));
    memcpy(mHandle->iochdr.trnm, mHandle->trnm, sizeof(mHandle->iochdr.trnm));
    memcpy(sndb, &mHandle->iochdr, sizeof(mHandle->iochdr));
    sndl = sizeof(mHandle->iochdr);	
    memcpy(&sndb[sndl], mHandle->sndb, mHandle->sndl);
    sndl += mHandle->sndl;

	if (sndl > MAX_SND_LEN)
		sndl = MAX_SND_LEN;

    retc = write(csock, (char *)sndb, sndl);

    close(csock);
    if (retc < 0)
        return(XIO);
    if (retc != sndl)
        return(XIO);

#if 0
pxsyslog("_pxsend", "_pxsend success[%s][%d][%s]", sock_un.sun_path,
	mHandle->sndl, mHandle->sndb);
#endif
    return(0);
}

/***************************************************************************** 
 * NAME : pxrecv()                              
 * DESC : receive a request message or reply message.  
 *****************************************************************************/
int pxrecv(tHandle, mHandle, timeout)
T_HAND  *tHandle;
M_HAND  *mHandle;
int	timeout;
{
    struct iochdr *iochdr;
    struct timeval timeval;
    struct sockaddr_un sock_un;
	socklen_t	argl;
    char recv_b[MAX_RCV_LEN];
    int recv_l, prev_l;
    int xxsock, nfound, ii;
    fd_set  rlist;

	for (ii = 0; ii < 30000; ii++)
	{
		argl = sizeof(struct sockaddr_un);
		xxsock = accept(tHandle->devf, (struct sockaddr *)&sock_un, &argl);
		if (xxsock >= 0)
			break;
		usleep(100);
	}
	if (xxsock < 0)
		return(XNOMSG);
	
	memset(recv_b, 0x00, sizeof(recv_b));
	recv_l = 0;
	for (;;)
	{
		FD_ZERO(&rlist);
		FD_SET(xxsock, &rlist);
		timeval.tv_sec = timeout;
		timeval.tv_usec = 0;
		nfound = select(xxsock+1, &rlist, NULL, NULL, &timeval);
		if (!FD_ISSET(xxsock, &rlist))
			continue;
		if (nfound < 0)
		{
			close(xxsock);
			return(XNOMSG);
		}
		if (nfound == 0)
		{
			close(xxsock);
			return(XNOMSG);
		}

		prev_l = read(xxsock, &recv_b[recv_l], sizeof(recv_b) - recv_l);
		if (prev_l <= 0)
			break;
		recv_l += prev_l;
		if (recv_l >= sizeof(recv_b))
			break;
	}

	close(xxsock);
	if (recv_l < sizeof(struct iochdr))
		return(XIO);
    
    iochdr = (struct iochdr *)recv_b;
	memcpy(&mHandle->iochdr, iochdr, sizeof(struct iochdr));

	tHandle->pxid = iochdr->pxid;

    memset(tHandle->call, 0x00, sizeof(tHandle->call));
    memcpy(tHandle->call, iochdr->name, sizeof(tHandle->call));
    memset(tHandle->base, 0x00, sizeof(tHandle->base));
    memcpy(tHandle->base, iochdr->name, sizeof(tHandle->base));

	mHandle->type[0] = iochdr->type[0];
	mHandle->type[1] = iochdr->type[1];
	mHandle->type[2] = iochdr->type[2];
	mHandle->type[3] = iochdr->type[3];
    memset(mHandle->trnm, 0x00, sizeof(mHandle->trnm));
    memcpy(mHandle->trnm, iochdr->trnm, sizeof(mHandle->trnm));

    memset(mHandle->rcvb, 0x00, strlen(mHandle->rcvb));
    memcpy(mHandle->rcvb, &recv_b[sizeof(struct iochdr)], 
        recv_l - sizeof(struct iochdr));

    mHandle->rcvl = recv_l - sizeof(struct iochdr);
	if (mHandle->rcvl > MAX_RCV_LEN)
	{
		mHandle->rcvl = MAX_RCV_LEN;
		mHandle->rcvb[MAX_RCV_LEN] = '\0';
	}

    return(mHandle->rcvl);
}

/****************************************************************************** 
 * NAME : pxclose()  
 * DESC : close hero intreface.   
 ******************************************************************************/
int pxclose(tHandle)
T_HAND *tHandle;
{
    char    sun_path[128], *homedir;
    
pxsyslog("pxclose", "pxclose call[%.8s] name[%.8s]", tHandle->call, tHandle->name);
    if (tHandle->devf >= 0) {
        close(tHandle->devf);
	}
    tHandle->devf = -1;
	free(tHandle->ulst);

    if (strlen(tHandle->name) == 0) {
        return(XINVAL);
	}

	homedir = getenv("PNOX_HOME");
	if (homedir == NULL)
        return(XREADY);

    sprintf(sun_path, "%s/%s/%.*s", homedir, QUE_PATH, 
		(int)sizeof(tHandle->name), tHandle->name);
    unlink(sun_path);
    return(0);
}

/***************************************************************************** 
 * NAME : pxrout()    
 * DESC : routing process
 *****************************************************************************/
int pxrout(tHandle, mHandle)
T_HAND  *tHandle;
M_HAND  *mHandle;
{
	struct	pxmon	*pxmon;
	int	ii, jj, nprc;	
	char svcnm[16], rtcall[16];

	pxmon = _open_pxmon(-1);
	if (pxmon == NULL)
	{
		pxsyslog("pxrout", "pxmon create error");
		return(XREADY);
	}

	nprc = pxmon->nprc;
	if (nprc > MAX_PNOX_PRC)
		nprc = MAX_PNOX_PRC;

	sprintf(svcnm, mHandle->trnm, sizeof(mHandle->trnm));
	if (strlen(svcnm) <= 0)
	{
		pxsyslog("pxrout", "pxmon service error");
		return(XINVAL);
	}

	for (ii = strlen(svcnm); ii >= 0; ii--)
	{
		if (svcnm[ii] == ' ')
			svcnm[ii] = 0x00;
	}

	memset(tHandle->call, 0x00, sizeof(tHandle->call));
	memcpy(tHandle->call, "pnoxtpc", 7);

	if (tHandle->rtaf == SET_ON)
		return(0);

	for (ii = 0; ii < nprc; ii++)
	{
		if (memcmp(pxmon->prcmng[ii].proc, svcnm, strlen(svcnm)) != 0)
			continue;

		if (pxmon->prcmng[ii].resident == 1)
		{
			memset(tHandle->call, 0x00, sizeof(tHandle->call));
			memcpy(tHandle->call, svcnm, strlen(svcnm));
		}

		if (pxmon->prcmng[ii].rout[0] != 0x00)
		{
			for (jj = 0; jj < MAX_PNOX_RTC; jj++)
			{
				if (memcmp(pxmon->prcmng[ii].rout, pxmon->rtcmng[jj].ipad, 
					sizeof(pxmon->prcmng[ii].rout)) != 0)
				{
					continue;
				}

				memset(rtcall, 0x00, sizeof(rtcall));
				sprintf(rtcall, "rtc%04d", pxmon->rtcmng[jj].rtno);
				memcpy(tHandle->call, rtcall, sizeof(tHandle->call));
				break;
			}
		}
	}

#if 0
	/* not close	*/
	_close_pxmon(-1);
#endif
	return(0);
}

/***************************************************************************** 
 * NAME : pxlogout()    
 * DESC : pnox session close
 *****************************************************************************/
int pxlogout(pxid)
int	pxid;
{
	T_HAND	tHandle;
	M_HAND	mHandle;
	char	sndb[1024], rcvb[1024], caller[16];
    char	*homedir;
	int		retc;

    homedir = getenv("PNOX_HOME");
    if (homedir == NULL) {
        return(-1);
	}

	if (pxid <= 0) {
		return(-2);
	}

	memset(&tHandle, 0x00, sizeof(T_HAND));
	tHandle.pxid = pxid;	
	memset(caller, 0x00, sizeof(caller));
	sprintf(caller, "u%07d", tHandle.pxid);
	memcpy(tHandle.call, caller, sizeof(tHandle.call));
	tHandle.nusr = 0;

	memcpy(mHandle.trnm, "pxlogoff", 8);

	memset(sndb, 0x00, sizeof(sndb));
	memset(rcvb, 0x00, sizeof(rcvb));

	mHandle.sndb = sndb;
	mHandle.sndl = 0;
	mHandle.rcvb = rcvb;
	mHandle.rcvl = 0;

	retc = pxsend(&tHandle, &mHandle);
	if (retc < 0)
		return(-3);

	return(0);
}
