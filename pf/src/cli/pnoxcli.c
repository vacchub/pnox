#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "pnoxcli.h"

#if 0
#define G_IPAD  "127.0.0.1"
#define G_IPAD  "13.124.149.197"
#define G_IPAD  "192.168.219.9"
#endif

#define G_PORT  18311

int sock;

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
	char	*G_IPAD;

	if (argc != 2)
	{
		printf("Usase : %s server-ip(127.0.0.1)\n", argv[0]);
		return(0);
	}

	G_IPAD = argv[1];
    sock = pxconnect(G_IPAD, G_PORT);
    if (sock < 0)
    {
        printf("pxconnect error\n");
        return(-1);
    }
    
    retc = pthread_create(&ctrd, NULL, recv_thread, (void *)NULL);
    if (retc < 0)
    {
        printf("pthread_create error\n");
        return(-1);
    }

	printf("Usage trnm(8)^input\n");
	printf("input exit\n");

	for (;;)
	{
		memset(buff, 0x00, sizeof(buff));
		fgets(buff, sizeof(buff), stdin);
		buff[strlen(buff) - 1] = 0x00;
		fflush(stdin);
		if (memcmp(buff, "exit", 4) == 0)
			break;
		if (memcmp(buff, "quit", 4) == 0)
			break;

		if (strlen(buff) < 9)
			continue;

		if (buff[8] != '^')
			continue;

		memset(trnm, 0x00, sizeof(trnm));
		memcpy(trnm, buff, 8);
		for (ii = 0; ii < 8; ii++)
		{
			if (trnm[ii] == ' ')
				trnm[ii] = 0x00;
		}

		memset(sndb, 0x00, sizeof(sndb));
		pnoxhdr = (struct pnoxhdr *)sndb;
		pnoxhdr->chkf[0] = 0x7F;
		pnoxhdr->chkf[1] = 0x7F;
		pnoxhdr->type[0] = 'Q';
		memcpy(pnoxhdr->trnm, trnm, 8);
		sprintf(temp, "%0*d", (int)sizeof(pnoxhdr->dlen), (int)strlen(buff) - 9);
		memcpy(pnoxhdr->dlen, temp, sizeof(pnoxhdr->dlen));
		memcpy(&pnoxhdr->data[0], &buff[9], strlen(buff) - 9);
		retc = write(sock, sndb, sizeof(struct pnoxhdr) - 1 + strlen(buff) - 9);
printf("write OK %d\n", retc);
	}

printf("send thread end\n");
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
    char    rcvb[64*1024];
    int     rcvl, t_rcvl, dlen;

    ptrd = pthread_self();
    retc = pthread_detach(ptrd);

    for (;;)
    {
        FD_ZERO(&clist);
        FD_SET(sock, &clist);

        tval.tv_sec = 20;
        tval.tv_usec = 0;
        nfound = select(sock+1, &clist, NULL, NULL, &tval);
        if (nfound < 0)
            break;
        if (nfound == 0)
		{
            printf("polling not receive\n");
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
            printf("receive len[%d]\n", retc);
            break;
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
			continue;
		}

		printf("trnm[%.8s] rcvl[%.5s] rcvb[%.*s]\r\n", pnoxhdr->trnm,
			pnoxhdr->dlen, atoi(pnoxhdr->dlen), pnoxhdr->data);		
		fflush(stdout);
    }

printf("recv thread end\n");
    pthread_exit((void *)0);
    exit(0);
}
