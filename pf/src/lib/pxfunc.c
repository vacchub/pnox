#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "pnox.h"
#include "pxmon.h"

/***************************************************************************** 
 * NAME	: getpxid()
 *****************************************************************************/
int	getpxid(tHandle)
T_HAND *tHandle;
{
	if (tHandle == NULL)
		return(-1);

	return(tHandle->pxid);
}

/***************************************************************************** 
 * NAME	: ispxid()
 *****************************************************************************/
int	ispxid(pxid)
int	pxid;
{
	struct	pxmon *pxmon;
	int		retc;

	pxmon = _open_pxmon(-1);
	if (pxmon == NULL)
		return(-1);

	if (pxmon->devn[pxid - 1] == SET_ON)
		retc = 1;
	else
		retc = 0;

	_close_pxmon(-1);

	return(retc);
}
