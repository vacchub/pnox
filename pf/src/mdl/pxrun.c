#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "pnox.h"

int	main(int argc, char *argv[])
{
	T_HAND	tHandle;
	M_HAND	mHandle;
	int     retc;
	char	sndb[65 * 1024];
	char	rcvb[65 * 1024];

	if (argc != 3)
	{
		printf("Usase : %s trnm input\n", argv[0]);
		return(0);
	}

    memset(&tHandle, 0x00, sizeof(T_HAND));
	retc = pxopen(&tHandle, "pxrun");
    if (retc < 0)
	{
		printf("pxopen error %d\n", retc);
        return(-1);
	}

    memset(&mHandle, 0x00, sizeof(M_HAND));
	mHandle.type[0] = TYPE0_QUERY;
	memcpy(mHandle.trnm, argv[1], strlen(argv[1]));	

	mHandle.sndl = strlen(argv[2]);
	memset(sndb, 0x00, sizeof(sndb));
	mHandle.sndb = sndb;
	memcpy(mHandle.sndb, argv[2], strlen(argv[2]));

	mHandle.rcvl = 0;
	memset(rcvb, 0x00, sizeof(rcvb));
	mHandle.rcvb = rcvb;

	retc = pxcall(&tHandle, &mHandle, 10);
	if (retc < 0)
	{
		printf("pxsend error %d\n", retc);
		pxclose(&tHandle);
		return(-1);
	}	

	printf("%.*s\n", mHandle.rcvl, mHandle.rcvb);
	pxclose(&tHandle);

	return(0);
}
