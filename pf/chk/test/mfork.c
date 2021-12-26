/********************************************************************
 * semapore 개체는 fork() 후 깨짐
 * 그러므로 fork() 후에 semapore 개체를 생성할것
 * 아래 예는 잘못된 상황을 보여준다
 *******************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define NUM_FORK	1

static  struct  sembuf  lock_op[2] = {
	{ 0, 0, 0        },
	{ 0, 1, SEM_UNDO }
};

static  struct  sembuf  unlock_op[1] = {
	{ 0, -1,IPC_NOWAIT|SEM_UNDO }
};

int	pxlock(int);
int	pxunlock(int);
int	pxwaitlock(int);

int main()
{
	int ii, cnt, pid, semid, rc;
	struct timeval tval1, tval2;

	semid = semget(12345, 1, IPC_CREAT|0666);
        if (semid < 0)
        {
                printf("semget error\n");
                return(-1);
        }
	printf("semid %d\n", semid);

	cnt = 0;
	gettimeofday(&tval1, NULL);
	for (ii = 0; ii < NUM_FORK; ii++)
	{
		pxlock(semid);
		pid = fork();
		if (pid > 0)		/* parent	*/
		{
			cnt++;
		}
		else if (pid == 0)	/* child	*/
		{
			sleep(3);
			pxunlock(semid);
printf("step 1\n");
			sleep(10);
			exit(0);
		}
		else
		{
			printf("fork error errno=%d\n", errno);
			printf("process count : %d\n", cnt);
			pxunlock(semid);
			exit(-1);
		}
			
		pxwaitlock(semid);
printf("step 2\n");
	}
	gettimeofday(&tval2, NULL);

	printf("process count : %d, time %dsec errno:%d\n", cnt, tval2.tv_sec - tval1.tv_sec,
		errno);

	wait(0);
	return(0);
}

/******************************************************************************
 * NAME : pxlock()
 * DESC : 세마포어ID LOCKING
 ******************************************************************************/
int     pxlock(semid)
int     semid;
{
        int retcod;

        retcod = semop(semid, lock_op, 2);
        if (retcod < 0)
                return(-1);

        return(0);
}

/******************************************************************************
 * NAME : pxunlock()
 * DESC : 세마포어ID UNLOCKING
 ******************************************************************************/
int     pxunlock(semid)
int     semid;
{
        int retcod;

        retcod = semop(semid, unlock_op, 1);
        if (retcod < 0)
                return(-1);

        return(0);
}

/******************************************************************************
 * NAME : pxwaitlock()
 * DESC : WAITING TO UNLOCK-TIME
 ******************************************************************************/
int     pxwaitlock(semid)
int     semid;
{
        int retcod;

        retcod = semop(semid, lock_op, 1);      /* wait to become 0 */
        if (retcod < 0)
                return(-1);

        return(0);
}
