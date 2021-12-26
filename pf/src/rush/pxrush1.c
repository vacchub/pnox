#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "pxrush.h"

#if 1
#define G_IPAD  "10.0.2.15"
#else
#define G_IPAD  "192.168.219.9"
#endif
#define G_PORT  18311

int sock;
int	delay, count, mypid;
int	sndc, rcvc;

void *recv_thread(void *);

int main(argc, argv)
int argc;
char    *argv[];
{
	pthread_t ctrd;
	struct  pnoxhdr *pnoxhdr;
	int     ii, retc;
	char    buff[64*1024], trnm[16], temp[32];
	char    sndb[16*1024];
	int     sndl;
	void 	*trd_return;

	if (argc < 3)
	{
		printf("Usage : %s [usleep] [count]\n", argv[0]);
		return(0);
	}

	sndc = 0;
	rcvc = 0;

	delay = atoi(argv[1]);
	count = atoi(argv[2]);

	mypid = getpid();

    sock = pxconnect(G_IPAD, G_PORT);
    if (sock < 0)
    {
        printf("pxconnect error\n");
        return(-1);
    }
printf("pxconnect OK\n");
    
    retc = pthread_create(&ctrd, NULL, recv_thread, (void *)NULL);
    if (retc < 0)
    {
        printf("pthread_create error\n");
        return(-1);
    }

	for (ii = 0; ii < count; ii++)
	{
		memset(buff, 0x00, sizeof(buff));
		memcpy(buff, "tpstest |ABC", 12); 

		memset(trnm, 0x00, sizeof(trnm));
		memcpy(trnm, buff, 8);

		memset(sndb, 0x00, sizeof(sndb));
		pnoxhdr = (struct pnoxhdr *)sndb;
		pnoxhdr->chkf[0] = 0x7F;
		pnoxhdr->chkf[1] = 0x7F;
		memcpy(pnoxhdr->trnm, trnm, 8);
		sprintf(temp, "%0*d", (int)sizeof(pnoxhdr->dlen), strlen(buff) - 9);
		memcpy(pnoxhdr->dlen, temp, sizeof(pnoxhdr->dlen));
		memcpy(&pnoxhdr->data[0], &buff[9], strlen(buff) - 9);
		retc = write(sock, sndb, sizeof(struct pnoxhdr) - 1 + strlen(buff) - 9);
		sndc++;

		usleep(delay);
	}

	pthread_join(ctrd, &trd_return);
printf("sndc[%d] rcvc[%d]\n", sndc, rcvc);

    close(sock);
    return(0);
}

void *recv_thread(free)
    void *free;
{
    struct  timeval tval;
    pthread_t ptrd;
    fd_set  clist;
    int     retc, nfound;
    struct  pnoxhdr *pnoxhdr;
    char    buff[64*1024], trnm[16];
    char    rcvb[64*1024];
    int     ii, rcvl, t_rcvl, dlen;

#if 0
    ptrd = pthread_self();
    retc = pthread_detach(ptrd);
#endif

	for (ii = 0; ii < count; ii++)
    {
        FD_ZERO(&clist);
        FD_SET(sock, &clist);

        tval.tv_sec = 1;
        tval.tv_usec = 0;
        nfound = select(sock+1, &clist, NULL, NULL, &tval);
        if (nfound < 0)
		{
			printf("select err\n");
            break;
		}
        if (nfound == 0)
		{
#if 0	
			printf("select 0... 수신지연\n");
#endif
            break;
		}

        if (!FD_ISSET(sock, &clist))
		{
			printf("FD_ISSET not sock\n");
            continue;
		}

		memset(rcvb, 0x00, sizeof(rcvb));
		retc = read(sock, rcvb, sizeof(struct pnoxhdr) - 1);
		if (retc == 0)
		{
			printf("read ret 0\n");
			continue;
		}
		if (retc != sizeof(struct pnoxhdr) - 1)
		{
			printf("read size error %d %d %s\n", retc, rcvb[0], rcvb);
			break;
		}

		rcvl = retc;
			
		pnoxhdr = (struct pnoxhdr *)rcvb;

		dlen = atoi(pnoxhdr->dlen);
		for (t_rcvl = 0;;)
		{
			if (rcvl >= sizeof(struct pnoxhdr) - 1 + dlen)
				break;

			t_rcvl = read(sock, &rcvb[rcvl], dlen);	
			rcvl += t_rcvl;
		}

		if (pnoxhdr->chkf[0] != 0x7F || pnoxhdr->chkf[1] != 0x7F)
		{
			printf("format error\n");
			break;
		}

		if (pnoxhdr->type[0] == 'P')
		{
			write(sock, rcvb, sizeof(struct pnoxhdr) - 1 + 1);
			printf("recv poll\n");
			continue;
		}
		rcvc++;

		fflush(stdout);
	}

    pthread_exit((void *)0);
    return((void *)0);
}
