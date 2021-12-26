#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "mbmtalk.h"

struct mbmtalk *mbmtalk = NULL;
struct mjmtalk *mjmtalk = NULL;

int main(int argc, char *argv[])
{
	int ii;

	mbmtalk = l_mtopen(O_RDONLY);
	if (mbmtalk == NULL)
	{
		printf("l_mtopen error\n");
		return(0);
	}

	printf("maxm[%d] vrec[%d]\n", mbmtalk->maxm, mbmtalk->vrec);
	for (ii=0; ii < mbmtalk->vrec; ii++)
	{
		printf("[%03d] [%d][%d]\n", ii, 
			mbmtalk->mjmtalk[ii].pxid[0], mbmtalk->mjmtalk[ii].pxid[1]);
	}

	return(0);
}
