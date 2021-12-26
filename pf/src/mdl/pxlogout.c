/******************************************************************************
 * NAME : pxlogout()
 * DESC : pxlogout
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

/******************************************************************************
 * NAME : pxlogout()
 * DESC : pxlogout
 ******************************************************************************/
int main(argc, argv)
int	argc;
char	*argv[];
{
	T_HAND	tHandle;
	int		pxid, retc;

	if (argc != 2)
	{
		printf("Usage : %s pxid\n", argv[0]);
		return(0);
	}

	pxid = atoi(argv[1]);

	memset(&tHandle, 0x00, sizeof(T_HAND));
	retc = pxopen(&tHandle, "pxlogout");
	if (retc < 0)
	{
		printf("pxopen error\n");
		return(-1);
	}

	retc = pxlogout(pxid);
	if (retc < 0)
	{
		printf("pxlogout error %d %d\n", retc, errno);
		pxclose(&tHandle);
		return(-1);
	}

	pxclose(&tHandle);

	return(0);
}
