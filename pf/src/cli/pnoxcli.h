#ifndef _PNOXCLI_H
#define _PNOXCLI_H

struct  pnoxhdr {
        char    chkf[2];                /* 0x7F, 0x7F                   */
		char	type[4];				/* 0							*/
										/* 'Q':query 		           */
                            			/* 'R':rts              */
                            			/* 'P':polling          */
                            			/* 1=filler             */
                            			/* 2=filler             */
                            			/* 3=filler             */
        char    trnm[8];                /* service name                 */
        char    dlen[5];                /* data lenth                   */
        char    data[1];                /* data                         */
};

int	pxconnect(char *ipad, int port);

#endif
