#include <stdio.h>

#include <time.h>

extern	void a.c();
extern	void b.c();
struct svcsvc {
	char   svc_name[16];
	void   (*svc_proc)();
} svcsvc[] = { 
	{ "a.c",  a.c },
	{ "b.c",  b.c },
	{ "",          NULL }
};
