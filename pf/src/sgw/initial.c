#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "pnox.h"
#include "pxmon.h"

/*****************************************************************************/
/* NAME	: initial()							     */
/*****************************************************************************/
int initial(pxmon)
struct	pxmon* pxmon;
{
	char	*xenv;
	int     ii, ntrd;

	/************************************ 
	 * init user
	 ************************************/
	for (ii = 0; ii < MAX_DEVN; ii++)
	{
		pxmon->usrmng[ii].used = SET_OFF;
		pxmon->usrmng[ii].pxid = 0;
		pxmon->usrmng[ii].rcvc = 0;
		pxmon->usrmng[ii].sndc = 0;
		pxmon->usrmng[ii].ctim = 0;
	}

	/************************************ 
	 * init devn
	 ************************************/
	for (ii = 0; ii < MAX_DEVN; ii++)
		pxmon->devn[ii] = SET_OFF;
	pxmon->devi = 0;

	/************************************ 
	 * init sca
	 ************************************/
	xenv = getenv("PNOX_NTRD");
	ntrd = atoi(xenv);
	if (ntrd > MAX_SCA_TRD)
		ntrd = MAX_SCA_TRD;

	pxmon->ntrd = ntrd;

	for (ii = 0; ii < MAX_PNOX_SCA; ii++)
	{
		pxmon->scamng[ii].used = SET_OFF;
		pxmon->scamng[ii].spid = 0;
		pxmon->scamng[ii].sock = 0;
		pxmon->scamng[ii].utrd = 0;
	} 

	/************************************ 
	 * init rta
	 ************************************/
	for (ii = 0; ii < MAX_PNOX_RTA; ii++)
	{
		pxmon->rtamng[ii].used = SET_OFF;
		pxmon->rtamng[ii].rpid = 0;
		pxmon->rtamng[ii].sock = 0;
	} 

	xenv = getenv("PNOX_LOGF");
	if (xenv[0] == 'Y')
		pxmon->logf = 1;
	else
		pxmon->logf = 0;

	return(0);
}

