#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include "pnox.h"

#define HEX_BUF_LEN	65*1024

#if 0
int pxsyslog(char *fnam, const char *format, ...)
{
	FILE *fp;
	struct tm *tm;
	time_t clock;
	char xpath[128], *xenvs;
	va_list vl;

	clock = time(0);
	tm = localtime(&clock);

	xenvs = getenv("PNOX_HOME");
	if (xenvs == NULL)
		return(-1);

	sprintf(xpath, "%s/%s/%s", xenvs, LOG_PATH, fnam);
	fp = fopen(xpath, "a");
	if (fp == NULL)
		return(-1);

	fprintf(fp, "%02d/%02d %02d:%02d:%02d ",
		tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	va_start(vl, format);
	vfprintf(fp, format, vl);
	fprintf(fp, "\n");
	va_end(vl);
	fclose(fp);
	return(0);
}
#endif

int pxsyslog(char *pname, const char *format, ...)
{
    va_list vl;
    time_t  today;
    struct tm   *tm, tt;
    char    path[128], mode[10], *xenvs;
    FILE    *fp;
    struct  stat st;
    int rc;

    today = time(0);
    tm = localtime(&today);
    memcpy(&tt, tm, sizeof(struct tm));

    xenvs = getenv("PNOX_HOME");
    if (xenvs == NULL)
        return(-1);

    sprintf(path, "%s/%s/SYSLOG%01d", xenvs, LOG_PATH, tm->tm_wday);

    rc = stat(path, &st);
    if (rc < 0)
        strcpy(mode, "a");
    else
    {
        tm = localtime(&st.st_ctime);
        if (tm->tm_mon != tt.tm_mon || tm->tm_mday != tt.tm_mday)
            strcpy(mode, "w");
        else
            strcpy(mode, "a");
    }

    fp = fopen(path, mode);
    if (fp == NULL)
        return (-1);
    fprintf(fp, "[%02d/%02d %02d:%02d:%02d] [%s] ",
            tt.tm_mon+1, tt.tm_mday,
            tt.tm_hour, tt.tm_min, tt.tm_sec, pname);
    
    va_start(vl, format);
    vfprintf(fp, format, vl);
    fprintf(fp, "\n");
    va_end(vl);
    fclose(fp);
    return (0);
}   

int pxhexlog(char *fname, char *title, char *buf, int len)
{
	FILE *file;
	char time_buf[30];
	time_t curtime;
	struct tm tp;
	struct timeval tval;
	int ii, jj, kk;

	if (len > HEX_BUF_LEN)
		len = HEX_BUF_LEN;

	file = fopen(fname, "a");
	if (file == NULL)
		return(-1);

	gettimeofday(&tval, NULL);
	curtime = tval.tv_sec;
	localtime_r(&curtime, &tp);

	sprintf(time_buf, "%02d/%02d %02d:%02d:%02d-%06d",
		(int)tp.tm_mon+1, (int)tp.tm_mday,
		(int)tp.tm_hour, (int)tp.tm_min, (int)tp.tm_sec, (int)tval.tv_usec);
	if (title == NULL)
				fprintf(file, "[%s] len(%d)\n", time_buf, len);
	else
				fprintf(file, "[%s] %s len(%d)\n", time_buf, title, len);

	for (ii = 0; ii < len; )
	{
		kk = ii;
		fprintf(file, "%04d   ", ii);
		for (jj = 0; jj < 15 && ii < len; jj++, ii++)
			fprintf(file, "%02X ", buf[ii] & 0xff);
#if 0
		for ( ; jj < 15; jj++)
			fprintf(file, "   ", buf[ii]);
#endif
		fprintf(file, "  ");
		for (ii = kk, jj = 0; jj < 15 && ii < len; jj++, ii++)
		{
			if ((buf[ii] & 0xFF) < ' ' || (buf[ii] & 0xFF) >= 0x80)
				fprintf(file, ".");
			else if (buf[ii] == MASK_CHKF)
				fprintf(file, ".");
			else
				fprintf(file, "%c", buf[ii]);
		}
		fprintf(file, "\n");
	}		

	if (len > 0)
		fprintf(file, "\n");

	fflush(file);
	if (file != stdout)
		fclose(file);

	return(0);
}
