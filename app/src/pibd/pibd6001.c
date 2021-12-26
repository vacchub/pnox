#include <stdio.h>
#include <unistd.h>
#include "pnox.h"
#include "subf6001.h"

char	buff[150*1024];

/*****************************************************
 * main()
 ****************************************************/
int main(int argc, char *argv[])
{
	int		ii, blen, count;

	while (1)
	{
		blen = get_web_data(buff);
		data_parse(buff);

		break;
	}

	return(0);
}

/*****************************************************
 * proc_settbl()
 ****************************************************/
int proc_settbl(tbl)
struct bittrex *tbl;
{
	if (strlen(tbl->code) == 0)
		return(0);

	printf("code[%.32s]\t last[%.8f]\t\t volume[%f]\n", tbl->code, tbl->last, tbl->volume);
	return(0);
}
