#ifndef _MBCOIN_H
#define _MBCOIN_H

#define MAX_MJMTALK	10

struct mjmtalk {
	int pxid[2];
};

struct mbmtalk {
	int maxm;
	int vrec;
	struct mjmtalk mjmtalk[MAX_MJMTALK];
};

struct  mbmtalk *l_mtopen(int);
struct  mjmtalk *l_mtread(struct mbmtalk *, int);
struct  mjmtalk *l_mtaddr(struct mbmtalk *, int, int);
int     l_mtdelr(struct mbmtalk *, int);

#endif
