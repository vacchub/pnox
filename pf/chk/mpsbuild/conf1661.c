#include <stdio.h>

#include <time.h>

extern	void a();
extern	void b();
struct svcsvc {
	char   svc_name[16];
	void   (*svc_proc)();
} svcsvc[] = { 
	{ "a",  a },
	{ "b",  b },
	{ "",          NULL }
};
