#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include "pnox.h"

int main()
{
	int semid, pid, ii, jj;

	semid = semget(12345, 1, IPC_CREAT|0666);
	if (semid < 0)
	{
		printf("semget error\n");
		return(-1);
	}

	pxlock(semid);

	for (ii = 0; ii < 2; ii++)
	{
		pid = fork();
		if (pid > 0)	/* parent	*/
		{
			continue;
		}
		if (pid == 0)	/* child	*/
		{
#if 0
			pxlock(semid);
			printf("lock child pid[%d]\n", getpid());
#endif
			pxwaitlock(semid);
			printf("wait lock child pid[%d]\n", getpid());
			sleep(5);
			pxunlock(semid);
#if 0
			for (jj = 0; jj < 10; jj++)
			{
				printf("%d ", jj + 1);
				sleep(1);
			}
			printf("\n");
#endif
#if 0
			pxunlock(semid);
			printf("unlock child pid[%d]\n", getpid());
#endif
			return(0);
		}
		else
		{
			printf("fork() error\n");
			return(-1);
		}
	}
	sleep(5);
	pxunlock(semid);
	printf("unlock parent\n");
	sleep(15);
	printf("parent end\n");
	
	return(0);
}
