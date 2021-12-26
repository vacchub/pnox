#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

int main()
{
        struct  {
                int flg;
		int a1;
		int a2;
		int a3;
		int a4;
		int a5;
		int a6;
		char a7[1230];
		int b;
		int c;
		int d;
		int e;
		int f;
		int g;
		int h;
        } test[2000];
        int    s[2000];

        int     ii;
        struct  timeval tv1, tv2;

        for (ii = 0; ii < 2000; ii++)
	{
		test[ii].flg = 0;
		s[ii] = 0;
	}
	test[1999].flg = 1;
	s[1999] = 1;

        gettimeofday(&tv1, NULL);
	for (ii = 0; ii < 2000; ii++)
	{
		if (s[ii] == 1)
			break;
	}
        gettimeofday(&tv2, NULL);
        printf("[%d %d]\n", tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec);


        gettimeofday(&tv1, NULL);
        for (ii = 0; ii < 2000; ii++)
        {
		if (test[ii].flg == 1)
			break;
        }
        gettimeofday(&tv2, NULL);
        printf("[%d %d]\n", tv2.tv_sec - tv1.tv_sec, tv2.tv_usec - tv1.tv_usec);
        return(0);
}
