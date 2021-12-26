#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "pnox.h"
#include "pxmon.h"

extern struct pxmon *pxmon;

int service_user(tHandle, sock)
T_HAND	*tHandle;
int		sock;
{
	M_HAND	mHandle;
	struct  pnoxhdr *pnoxhdr;
	int     retc;
	int     rcvl=0, prel=0, datl=0;
	char    rcvb[128 * 1024];
	char    sndb[128 * 1024];
	char	fnam[256];

	memset(rcvb, 0x00, sizeof(rcvb));

#if 0
pxsyslog("pnoxsca", "service_user start");
#endif
	rcvl = read(sock, rcvb, PNOXHDR_L);
#if 0
pxsyslog("pnoxsca", "service_user [%d][0x%02X][0x%02X][%s]", rcvl, rcvb[0], rcvb[1], &rcvb[2]);
#endif
	if (rcvl == 0)
	{
#if 0
pxsyslog("pnoxsca", "pnoxsca client disconnect. pxid[%d]", tHandle->pxid);
#endif
		return(-1);
	}
#if 0
pxsyslog("pnoxsca", "pnoxsca [%d]", rcvl);
#endif

	if (rcvl < PNOXHDR_L)
	{
		pxsyslog("pnoxsca", "pnoxsca less read len[%d]", rcvl);
		return(-2);
	}

	pnoxhdr = (struct pnoxhdr *)&rcvb[0];
	rcvl = PNOXHDR_L;
	datl = atoi(pnoxhdr->dlen);

	for (;;)
	{
		if (rcvl >= PNOXHDR_L + datl)
			break;

		prel = read(sock, &rcvb[rcvl], datl);
		if (prel <= 0)
			break;

		rcvl += prel;
	}

	if (pnoxhdr->chkf[0] != MASK_CHKF ||
		pnoxhdr->chkf[1] != MASK_CHKF)
	{
		close(sock);
		pxsyslog("pnoxsca", "pnoxsca format error[%02X %02X]", 
			(pnoxhdr->chkf[0] & 0xFF), (pnoxhdr->chkf[1] & 0xFF));
		return(-5);
	}

#if 0
	if (pnoxhdr->type[0] != TYPE0_QUERY)
		return(0);
#endif

	if (pnoxhdr->type[0] == TYPE0_POLL)
		return(0);

	if (memcmp(pnoxhdr->trnm, "pxlogoff", 8) == 0)
		return(-10);

	memset(&mHandle, 0x00, sizeof(M_HAND));
	mHandle.type[0] = pnoxhdr->type[0];
	mHandle.type[1] = pnoxhdr->type[1];
	mHandle.type[2] = pnoxhdr->type[2];
	mHandle.type[3] = pnoxhdr->type[3];
	memcpy(mHandle.trnm, pnoxhdr->trnm, sizeof(mHandle.trnm));	

	mHandle.sndl = datl;
	memset(sndb, 0x00, sizeof(sndb));
	mHandle.sndb = sndb;
	memcpy(mHandle.sndb, &pnoxhdr->data[0], mHandle.sndl);	
	memcpy(tHandle->call, mHandle.trnm, sizeof(tHandle->call));

	pxmon->usrmng[tHandle->pxid - 1].rcvc++;

	if (pxmon->logf)
	{
		sprintf(fnam, "%s/%s/usr/%07d", getenv("PNOX_HOME"), LOG_PATH, tHandle->pxid);
		pxhexlog(fnam, "RECV", rcvb, rcvl);
	}

	retc = pxsend(tHandle, &mHandle);
	if (retc < 0)
	{
		pxsyslog("pnoxsca", "pnoxsca pxsend error %d, [%.8s] errno[%d]", 
			retc, mHandle.trnm, errno);
	}	

	return(datl);
}

int service_svr(tHandle, sock)
T_HAND	*tHandle;
int		sock;
{
	M_HAND	mHandle;
	struct	pnoxhdr	*pnoxhdr;
	int		retc, bufl;
	char	rcvb[65 * 1024];
	char	sndb[65 * 1024];
	char	buff[65 * 1024];
	char	fnam[256];

	memset(&mHandle, 0x00, sizeof(M_HAND));
	memset(sndb, 0x00, sizeof(sndb));
	memset(rcvb, 0x00, sizeof(rcvb));
	mHandle.sndb = sndb;
	mHandle.sndl = 0;
	mHandle.rcvb = rcvb;
	mHandle.rcvl = 0;
	retc = pxrecv(tHandle, &mHandle, 0);
	if (retc < 0)
	{
		pxsyslog("pnoxsca", "service_svr pxrecv %d", retc);
		return(-1);
	}

	if (memcmp(mHandle.trnm, "pxlogoff", 8) == 0)
	{
pxsyslog("pnoxsca", "service_svr receive pxlogoff pxid[%d]", tHandle->pxid);
		return(-10);
	}

	memset(buff, 0x00, sizeof(buff));
	pnoxhdr = (struct pnoxhdr *)buff;

	pnoxhdr->chkf[0] = MASK_CHKF;
	pnoxhdr->chkf[1] = MASK_CHKF;

#if 0
	switch (mHandle.type[0])
	{
	case TYPE0_QUERY:
	case TYPE0_RTS:
	case TYPE0_POLL:
		break;
	default :
		pxsyslog("pnoxsca", "service_svr type0[%d]", mHandle.type[0]);
		return(0);
	}
#endif

	pnoxhdr->type[0] = mHandle.type[0];
	pnoxhdr->type[1] = mHandle.type[1];
	pnoxhdr->type[2] = mHandle.type[2];
	pnoxhdr->type[3] = mHandle.type[3];
	memcpy(pnoxhdr->trnm, mHandle.trnm, sizeof(pnoxhdr->trnm));
	sprintf(pnoxhdr->dlen, "%0*d", (int)sizeof(pnoxhdr->dlen), mHandle.rcvl);
	memcpy(pnoxhdr->data, mHandle.rcvb, mHandle.rcvl);

	bufl = PNOXHDR_L + mHandle.rcvl;

	retc = write(sock, buff, bufl);
	if (retc != bufl)
	{
		pxsyslog("pnoxsca", "service_svr write %d(%d)", retc, errno);
		return(-2);
	}

	pxmon->usrmng[tHandle->pxid - 1].sndc++;

	if (pxmon->logf)
	{
		sprintf(fnam, "%s/%s/usr/%07d", getenv("PNOX_HOME"), LOG_PATH, tHandle->pxid);
		pxhexlog(fnam, "SEND", buff, bufl);
	}
	
	return(0);
}

int service_poll(sock)
int		sock;
{
	struct	pnoxhdr	*pnoxhdr;
	int		retc, bufl;
	char	buff[65 * 1024], data[8];

	memset(buff, 0x00, sizeof(buff));
	pnoxhdr = (struct pnoxhdr *)buff;

	pnoxhdr->chkf[0] = MASK_CHKF;
	pnoxhdr->chkf[1] = MASK_CHKF;

	pnoxhdr->type[0] = TYPE0_POLL;

	memset(data, 0x00, sizeof(data));
	data[0] = 'P';

	sprintf(pnoxhdr->dlen, "%0*d", (int)sizeof(pnoxhdr->dlen), (int)strlen(data));
	memcpy(pnoxhdr->data, data, strlen(data));

	bufl = PNOXHDR_L + strlen(data);

    retc = write(sock, buff, bufl);
    if (retc != bufl)
    {
        pxsyslog("pnoxsca", "service_poll write %d(%d)", retc, errno);
        return(-1);
    }
	
	return(0);
}
