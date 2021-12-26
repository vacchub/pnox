#include <stdio.h>
#include "list.h"

int main()
{
	struct _list_st *m, m1;
	int *p, p1, ret;

	l_linkinit("PNOX");
	printf("nlist = [%d]\n", l_getnlist());
	l_addlist(111, 3);
	printf("nlist = [%d]\n", l_getnlist());
	l_addlist(222, 3);
	printf("nlist = [%d]\n", l_getnlist());
	l_addlist(333, 3);
	printf("nlist = [%d]\n", l_getnlist());
	l_addlist(444, 3);
	printf("nlist = [%d]\n", l_getnlist());

#if 0
	ret = l_getindxlist(0);
	printf("[%d]\n", ret);
	ret = l_getindxlist(1);
	printf("[%d]\n", ret);
#endif

	printf("[%d]\n", l_removelist(222));
	ret = l_getindxlist(0);
	printf("[%d]\n", ret);
	ret = l_getindxlist(1);
	printf("[%d]\n", ret);
	ret = l_getindxlist(2);
	printf("[%d]\n", ret);

#if 0
	ret = l_getindxlist(0);
	printf("[%d]\n", ret);

	p = p1;
	if (l_getlist(p, 3) < 0)
		return -1;
	printf("[%.10s]\n", p);
	if (l_getlist(p, 3) < 0)
		return -1;
	printf("[%.10s]\n", p);
	if (l_getlist(p, 10) < 0)
		return -1;
	printf("[%.10s]\n", p);
#endif

	return 0;
}
