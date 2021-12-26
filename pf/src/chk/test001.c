#include <stdio.h>
#include "pnox.h"

int main()
{
	T_HAND tHandle;
	M_HAND mHandle;
	int retc;
	char rcvb[100], sndb[100];

	memset(&tHandle, 0x00, sizeof(T_HAND));
	retc = pxopen(&tHandle, "test001");
	if (retc < 0)
	{
		printf("pxopen error %d\n", retc);
		return(-1);
	}

	mHandle.rcvb = rcvb;
	mHandle.sndb = sndb;
	memcpy(mHandle.sndb, "ABC", 3);
	mHandle.sndl = 3;
	retc = pxsend(&tHandle, &mHandle);
	if (retc < 0)
	{
		printf("pxsend error %d\n", retc);
		return(-1);
	}

	retc = pxrecv(&tHandle, &mHandle, 10);
	if (retc < 0)
	{
		printf("pxrecv error %d\n", retc);
		return(-1);
	}

	printf("recv][%d] recvb[%.*s]\n", mHandle.rcvl,
		mHandle.rcvl, mHandle.rcvb);


	pxclose(&tHandle);
	
	return(0);
}
