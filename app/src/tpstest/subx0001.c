#include <stdio.h>
#include "pnox.h"

int subx0001(tHandle, mHandle)
T_HAND *tHandle;
M_HAND *mHandle;
{
	char buff[1024];

	memset(buff, 0x00, sizeof(buff));
	memcpy(buff, mHandle->rcvb, mHandle->rcvl);
	sprintf(&buff[strlen(buff)], "%04d", getpxid(tHandle)); 

	memcpy(mHandle->sndb, buff, strlen(buff));
	mHandle->sndl = strlen(buff);
	return(0);
}

