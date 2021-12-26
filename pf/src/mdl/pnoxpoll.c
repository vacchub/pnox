/******************************************************************************
 * NAME : pnoxpoll()
 * DESC : pnoxpoll
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include "pnox.h"
#include "pxmon.h"

/******************************************************************************
 * NAME : pnoxpoll()
 * DESC : pnoxpoll
 ******************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	T_HAND	tHandle;
	M_HAND	mHandle;
	char	sndb[1024], rcvb[1024], caller[16];
	int		pxid, retc;

	if (argc != 2)
	{
		printf("Usage : %s pxid\n", argv[0]);
		return(0);
	}
	pxid = atoi(argv[1]);

#if 0
	retc = homedir_pxmon(pnox_home, NULL);
	if (retc < 0)
	{
		printf("homedir error %d\n", retc);
		return(0);
	}

	sprintf(cmd, "PNOX_HOME=%s", pnox_home);
	putenv(cmd);
#endif

	memset(&tHandle, 0x00, sizeof(T_HAND));
	memset(&mHandle, 0x00, sizeof(M_HAND));

	retc = pxopen(&tHandle, "pnoxpoll");
    if (retc < 0)
    {
        printf("pxopen error %d\n", retc);
        return(-1);
    }

	tHandle.pxid = pxid;
	memset(caller, 0x00, sizeof(caller));
	sprintf(caller, "u%07d", tHandle.pxid);
	memcpy(tHandle.call, caller, sizeof(tHandle.call));
	tHandle.nusr = 0;

	mHandle.type[0] = TYPE0_POLL;

	memset(sndb, 0x00, sizeof(sndb));
	memset(rcvb, 0x00, sizeof(rcvb));

	sndb[0] = 'P';

	mHandle.sndb = sndb;
	mHandle.sndl = strlen(sndb);
	mHandle.rcvb = rcvb;
	mHandle.rcvl = 0;

	retc = pxsend(&tHandle, &mHandle);
	if (retc < 0)
	{
		printf("pnoxpoll error %d %d\n", retc, errno);
		pxclose(&tHandle);
		return(-1);
	}

	pxclose(&tHandle);

	return(0);
}
