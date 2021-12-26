#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "pnox.h"
#include "pxmon.h"

int service_poll(int);

int service_rta(tHandle, sock)
T_HAND	*tHandle;
int		sock;
{
	M_HAND	mHandle;
	struct  pnoxhdr *pnoxhdr;
	int     retc;
	int     rcvl=0, prel=0, datl=0;
	char    rcvb[128 * 1024];
	char    sndb[128 * 1024];
	char	caller[16];

	memset(rcvb, 0x00, sizeof(rcvb));

	rcvl = read(sock, rcvb, PNOXHDR_L);
	if (rcvl == 0)
	{
pxsyslog("pnoxrtc", "pnoxrtc TCP disconnect. sock[%d]", sock);
		return(-1);
	}

	if (rcvl < PNOXHDR_L)
	{
		pxsyslog("pnoxrtc", "pnoxrtc less read");
		return(-2);
	}

	pnoxhdr = (struct pnoxhdr *)rcvb;
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
		pxsyslog("pnoxrtc", "pnoxrtc format error");
		return(0);
	}

	if (pnoxhdr->type[0] == TYPE0_POLL)
	{
		service_poll(sock);
		return(0);
	}

	memset(&mHandle, 0x00, sizeof(M_HAND));
	memcpy(mHandle.trnm, pnoxhdr->trnm, sizeof(mHandle.trnm));	
	memcpy(tHandle->call, mHandle.trnm, sizeof(tHandle->call));

	mHandle.sndl = datl;
	memset(sndb, 0x00, sizeof(sndb));
	mHandle.sndb = sndb;
	memcpy(mHandle.sndb, &pnoxhdr->data[0], mHandle.sndl);	

	if (tHandle->pxid == 0)
	{
		memcpy(tHandle->call, tHandle->base, sizeof(tHandle->base));
	}
	else
	{
		memset(caller, 0x00, sizeof(caller));
		sprintf(caller, "u%07d", tHandle->pxid);
		memcpy(tHandle->call, caller, strlen(tHandle->call));
	}

	retc = pxsend(tHandle, &mHandle);
	if (retc < 0)
	{
		pxsyslog("pnoxrtc", "pnoxrtc pxsend error %d", retc);
	}	
	return(datl);
}

int service_tr(tHandle, sock)
T_HAND	*tHandle;
int		sock;
{
	M_HAND	mHandle;
	struct	pnoxhdr	*pnoxhdr;
	int		retc, bufl;
	char	rcvb[65 * 1024];
	char	sndb[65 * 1024];
	char	buff[65 * 1024];

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
		pxsyslog("pnoxrtc", "service_tr pxrecv %d", retc);
		return(-1);
	}

	memset(buff, 0x00, sizeof(buff));
	pnoxhdr = (struct pnoxhdr *)buff;

	pnoxhdr->chkf[0] = MASK_CHKF;
	pnoxhdr->chkf[1] = MASK_CHKF;

	pnoxhdr->type[0] = mHandle.type[0];
	pnoxhdr->type[1] = mHandle.type[1];
	pnoxhdr->type[2] = mHandle.type[2];
	pnoxhdr->type[3] = mHandle.type[3];

	memcpy(pnoxhdr->trnm, mHandle.trnm, sizeof(pnoxhdr->trnm));
	sprintf(pnoxhdr->dlen, "%0*d", (int)sizeof(pnoxhdr->dlen), mHandle.rcvl);
	memcpy(pnoxhdr->data, mHandle.rcvb, mHandle.rcvl);

#if 0
pxsyslog("pnoxrtc", "service_tr devf[%d] pxid[%d] name[%.8s] base[%.8s] call[%.8s] trnm[%.8s] rcvl[%d] rcvb[%.*s]",
tHandle->devf, tHandle->pxid, tHandle->name, tHandle->base, tHandle->call, mHandle.trnm, mHandle.rcvl, 
mHandle.rcvl, mHandle.rcvb);
#endif

	bufl = PNOXHDR_L + mHandle.rcvl;

	retc = write(sock, buff, bufl);
	if (retc != bufl)
	{
		pxsyslog("pnoxrtc", "service_tr write %d(%d)", retc, errno);
		return(-2);
	}
	
	return(0);
}

int service_poll(sock)
int	sock;
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
        pxsyslog("pnoxrtc", "service_poll write %d(%d)", retc, errno);
        return(-1);
    }
	
#if 0
pxsyslog("pnoxrtc", "service_poll write OK");
#endif
	return(0);
}
