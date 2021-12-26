#include <stdio.h>

int add(int, int);

int main()
{
	printf("add = %d\n", add(10, 20));
	return 0;
}

/***************************************
 cc tmp.c -o tmp -L. -letc
 **************************************/
