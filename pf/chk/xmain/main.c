#include <stdio.h>

int sub();
int main(int argc, char *argv[])
{
#if 0
	int (*fp)();
	fp = argv[0];
	fp();
#endif
	printf("main [%x][%x][%x]\n", argv[0], sub, 0x50cfc - 0x47f1c);
	return(0);
}
