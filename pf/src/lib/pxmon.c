#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "pnox.h"
#include "pxmon.h"

static	struct	pxmon *_pxmon = NULL;	/* sahred memory 	*/

/***************************************************************************** 
 * NAME	: _open_pxmon()
 * DESC	: Initialize pxmon shared memory
 *****************************************************************************/
struct	pxmon * _open_pxmon(mode)
int	mode;
{
	int	shmidx, shmkey, shmsiz;
	struct	shmid_ds shmid_ds;
	char	*shmadd;

	if (_pxmon != NULL)
		return(_pxmon);

	shmkey = pxtoken("pxmon", 'M');
	shmsiz = sizeof(struct pxmon);
	shmidx = shmget(shmkey, 0, 0666);
	if (shmidx >= 0)
	{
		if (shmctl(shmidx, IPC_STAT, &shmid_ds) == 0 &&
		    shmsiz != shmid_ds.shm_segsz)
		{
			(void) shmctl(shmidx, IPC_RMID, &shmid_ds);
			shmidx = -1;
		}
	}
	
	if (shmidx < 0)
	{
		if (mode == IPC_CREAT)
		{
			if ((shmidx = shmget(shmkey, shmsiz, 0666|IPC_CREAT)) == -1)
				return(NULL);
			pxsyslog("pxmon", "pxmon create");
		}
		else
			return(NULL);
	}

	if (shmidx == -1)
		return(NULL);

	shmadd = (char *)shmat(shmidx, (char *)0, 0);
	if (shmadd == (char *)-1)
		return(0);
	
	_pxmon = (struct pxmon *)shmadd;
	return(_pxmon);
}

/****************************************************************************** 
 * NAME	: _close_pxmon()
 * DESC	: Detach shared memory for pxmon.
 ******************************************************************************/
void _close_pxmon(mode)
int mode;
{
	int	shmidx, shmkey;
	struct	shmid_ds shmid_ds;

	if (_pxmon != NULL)
		shmdt((char *)_pxmon);
	_pxmon = NULL;

	if (mode == IPC_RMID)
	{
		shmkey = pxtoken("pxmon", 'M');
		shmidx = shmget(shmkey, 0, 0666);
		if (shmidx >= 0)
		{
			(void) shmctl(shmidx, IPC_RMID, &shmid_ds);
		}
		pxsyslog("pxmon", "pxmon ipc rm");
	}

	return;
}

/****************************************************************************** 
 * NAME	: homedir_pxmon()
 * DESC	: get pnox home path
 ******************************************************************************/
int homedir_pxmon(homedir, pxmon)
char *homedir;
struct pxmon *pxmon;
{
	struct pxmon *t_pxmon; 
	int	retc=0;	

	if (pxmon == NULL)
	{
		t_pxmon = _open_pxmon(-1);
		if (t_pxmon == NULL)
			return(-1);
	}
	else
		t_pxmon = pxmon;

	if (strlen(t_pxmon->home) <= 0)
		retc = -1;
	else
	{
		sprintf(homedir, "%s", t_pxmon->home);
		retc = 0;
	}

	if (pxmon == NULL)
		_close_pxmon(-1);

	return(retc);
}

/****************************************************************************** 
 * NAME	: pxerrname()
 * DESC	: get name pxerrname
 ******************************************************************************/
char *pxerrname(retc)
int	retc;
{
	switch (retc)
	{
	case XREADY    : return("XREADY");    break;
	case XOPEN     : return("XOPEN");     break;
	case XBUSY     : return("XBUSY");     break;
	case XREGISTER : return("XREGISTER"); break;
	case XPERM     : return("XPERM");     break;
	case XNOENT    : return("XNOENT");    break;
	case XCONNECT  : return("XCONNECT");  break;
	case XSESSION  : return("XSESSION");  break;
	case XCHANNEL  : return("XCHANNEL");  break;
	case XINVAL    : return("XINVAL");    break;
	case XSVCDOWN  : return("XSVCDOWN");  break;
	case XSVCBUSY  : return("XSVCBUSY");  break;
	case XMANY     : return("XMANY");     break;
	case XEXEC     : return("XEXEC");     break;
	case XIO       : return("XIO");       break;
	case XNOMSG    : return("XNOMSG");    break;
	case XNOBUFF   : return("XNOBUFF");   break;
	case XROOM     : return("XROOM");     break;
	case XTIMEOUT  : return("XTIMEOUT");  break;
	case XABORTED  : return("XABORTED");  break;
	default        : return("unknown");   break;
	}
}
