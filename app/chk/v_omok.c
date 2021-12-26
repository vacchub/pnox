#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
	char buff[1024];
	char tmpb[11][5];
	FILE *fp;

	fp = fopen("/pnox/app/cfg/omok.cfg", "r");
	if (fp == NULL)
		return(-1);

	while ((fgets(buff, sizeof(buff), fp)) == buff)
	{
		if (buff[0] == '#' || buff[0] == '\n')
			continue;
			
		buff[strlen(buff)-1] = 0x00;

		memset(tmpb, 0x00, sizeof(tmpb));
		sscanf(buff, "%s %s %s %s %s %s %s %s %s %s %s", tmpb[0],
			tmpb[1], tmpb[2], tmpb[3], tmpb[4], tmpb[5], 
			tmpb[6], tmpb[7], tmpb[8], tmpb[9], tmpb[10]); 
	
		printf("%s %s %s %s %s %s %s %s %s %s %s\n", tmpb[0],
			tmpb[1], tmpb[2], tmpb[3], tmpb[4], tmpb[5], 
			tmpb[6], tmpb[7], tmpb[8], tmpb[9], tmpb[10]); 
		if (tmpb[7][0] == 0x00 || tmpb[7][0] == 0x20)
			printf("---------------[%d]\n", tmpb[7][0]);
	}

	fclose(fp);

	return(0);
}
