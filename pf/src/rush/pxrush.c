#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
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

int do_rush();

int main(argc, argv)
int argc;
char    *argv[];
{
	int	ii, xpid, ncon;
	int	status;

	if (argc < 2)
	{
		printf("Usage : %s [nconnect]]\n", argv[0]);
		return(0);
	}

	printf("pid : %d\n", getpid());

	ncon = atoi(argv[1]);

	for (ii = 0; ii < ncon; ii++)
	{
		xpid = fork();
		switch (xpid)
		{
		case -1: continue;
		case  0:
			do_rush();
			exit(0);
		default:
			usleep(100000);
			continue;
		}
	}

	wait(&status);
	
	return(0);
}

int do_rush()
{
    struct  timeval tval;
	struct  pnoxhdr *pnoxhdr;
    fd_set  clist;
	char    buff[64*1024], trnm[16], temp[32];
	char    sndb[16*1024];
    char    rcvb[64*1024];
    int     retc, nfound;
	int		rcvl, t_rcvl, dlen;

    sock = pxconnect(G_IPAD, G_PORT);
    if (sock < 0)
    {
       	printf("pxconnect error\n");
       	return(-1);
    }

	while(1)
	{
		if (getppid() == 1)
			break;

		/*************************************************
		 * send
		 ************************************************/
		memset(buff, 0x00, sizeof(buff));
		memcpy(buff, "tpstest |ABC", 12); 

		memset(trnm, 0x00, sizeof(trnm));
		memcpy(trnm, buff, 8);

		memset(sndb, 0x00, sizeof(sndb));
		pnoxhdr = (struct pnoxhdr *)sndb;
		pnoxhdr->chkf[0] = 0x7F;
		pnoxhdr->chkf[1] = 0x7F;
		memcpy(pnoxhdr->trnm, trnm, 8);
		sprintf(temp, "%0*d", (int)sizeof(pnoxhdr->dlen), (int)strlen(buff) - 9);
		memcpy(pnoxhdr->dlen, temp, sizeof(pnoxhdr->dlen));
		memcpy(&pnoxhdr->data[0], &buff[9], strlen(buff) - 9);
		retc = write(sock, sndb, sizeof(struct pnoxhdr) - 1 + strlen(buff) - 9);

		/*************************************************
		 * recv
		 ************************************************/
        FD_ZERO(&clist);
        FD_SET(sock, &clist);

        tval.tv_sec = 1;
        tval.tv_usec = 0;
        nfound = select(sock+1, &clist, NULL, NULL, &tval);
        if (nfound < 0)
            break;
		
        if (nfound == 0)
            continue;

        if (!FD_ISSET(sock, &clist))
            continue;

		memset(rcvb, 0x00, sizeof(rcvb));
		retc = read(sock, rcvb, sizeof(struct pnoxhdr) - 1);
		if (retc == 0)
			continue;
		
		if (retc != sizeof(struct pnoxhdr) - 1)
			break;

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
			break;
#if 0
printf("read [%.8s][%.*s]\n", pnoxhdr->trnm, atoi(pnoxhdr->dlen), &pnoxhdr->data[0]);
#endif

		if (pnoxhdr->type[0] == 'P')
		{
			write(sock, rcvb, sizeof(struct pnoxhdr) - 1 + 1);
			continue;
		}
	}

    close(sock);
	return(0);
}
