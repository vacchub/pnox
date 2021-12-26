/*******************************************************************************
 * (C) COPYRIGHT WINIX Information Co., Ltd. 2000
 * All Rights Reserved
 * Licensed Materials - Property of WINIX
 *
 * This program contains proprietary ifnormation of WINIX Information.
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic,
 * mechanical, or otherwise without the written permission of Login System.
 *
 *  Components   : mpsbuild.c --- TPS servuice builder.
 *  Rev. History :
 *                Ver   Date    Description
 *              ------- ------- -----------------------------------------------
 *               1.0    2007-01 AXIS initial version.
 ******************************************************************************/

#if 0

C_SRC=
PC_SRC= raho2000.pc raho2005.pc raho2006.pc raho2007.pc 

#======================Make Rules==============
C_OBJS=$(C_SRC:.c=.o)
PC_OBJS=$(PC_SRC:.pc=.o)

all: $(PROGRAM)

rahs2000: $(PC_SRC) $(PC_OBJS) $(CK_LIBA)
    $(MPSBLD) -o rahs2000 $(CFLAGS) $(LFLAGS) $(PC_OBJS) $(L_LIBZ) $(L_LIBRAMS)

#endif

#include <string.h>
#include "svctbl.h"

#define	MAX(x,y)	(x > y ? x : y)

static  int     _cmp_svc();
static  void    endofbuild();

static  struct  svcsvc svcsvc[MAX_SVCSVC];
static  char    svcname[128];
#if 0
static  struct  tpssvc  tpssvc;
#endif
static  char    confcp[128], confop[128];
static  char    command[512];
static  char    ccargs[64][128];
static  char    *ccargv[64];
static  int     n_svcsvc;

/******************************************************************************/
/* NAME : main()                                                              */
/* DESC : TP service builder.                                                 */
/******************************************************************************/
main(argc, argv)
int     argc;
char    *argv[];
{
	FILE	*fd;
        extern  int     optind,  opterr, optopt;
        extern  char    *optarg;
        int     options, retv;
        char    *envirs, *token;
	char	editb[256], buf[1024];
        pid_t   xpid;
        int     ii, jj, kk, ll, retn, arg;

        if (argc < 3)
        {
                printf("mpsbuild -o pname libname options...\n");
                printf("DEBUG -g, DB CONNECT => -D_PROC \n");
                exit(0);
        }

        opterr = 0;
        memset(svcname, 0x00, sizeof(svcname));
        while((options = getopt(argc, argv, "o:")) != EOF)
        {
                switch (options)
                {
                case 'o':
                        if (svcname[0] == NULL)
                                strcpy(svcname, optarg);
printf("svcname [%s]\n", svcname);
                        break;
                default:  break;
                }
        }
        
        if (strlen(svcname) <= 0)
        {
                fprintf(stderr, "mpsbuild: missing service name to build.\n");
                exit(-1);
        }

#if 0
        strcpy(tpssvc.tpscfg.svcn, svcname);
        if (axgettpssvc(&tpssvc) != 0)
        {
                fprintf(stderr, "mpsbuild: no service configuration file.\n");
                exit(-1);
        }

        for (ii = 0; ii < tpssvc.nofl; ii++)
        {
                for (jj = 0; jj < tpssvc.slib[ii].ntrc; jj++)
                {
                        strcpy(svcsvc[n_svcsvc].svc_name,
                               tpssvc.slib[ii].trcd[jj]);
                        n_svcsvc++;
                }
        }
        qsort(svcsvc, n_svcsvc, sizeof(struct svcsvc), _cmp_svc);
#else
	
	n_svcsvc = 0;
	sprintf(editb, "%s/pxsvcd.cfg", "./");
        fd = fopen(editb, "r");
        while (fgets(buf, sizeof(buf), fd) != NULL)
        {
                if (buf[0] == '#' || strlen(buf) <= 1)
                        continue;

		token = strtok(buf, " \t\n");
		if (memcmp(svcname, token, MAX(strlen(svcname), strlen(token))) != 0)
			continue;

		arg = 0;
		while(token != NULL)
		{
			if (++arg > 2)
			{
				sprintf(svcsvc[n_svcsvc].svc_name, "%s", token);
				n_svcsvc++;
printf("%s\n", token);
			}
			token = strtok(NULL, " \t\n"); 
		}
		break;
        }

        fclose(fd);
#endif

        signal(SIGINT, endofbuild);
        signal(SIGTERM, endofbuild);

        if (buildconf() != 0)
                exit(0);

        jj = 0;
        envirs = getenv("CC");
        if (envirs != NULL)
                sprintf(ccargs[jj++], envirs);
        else
                sprintf(ccargs[jj++], "gcc");
          
        sprintf(ccargs[jj++], "-o");
        sprintf(ccargs[jj++], "%s", svcname);
        sprintf(ccargs[jj++], "%s", confcp);

#if     1
	sprintf(editb, "%s/pxsvcd.cfg", "./");
        fd = fopen(editb, "r");
        while (fgets(buf, sizeof(buf), fd) != NULL)
        {
                if (buf[0] == '#' || strlen(buf) <= 1)
                        continue;

		token = strtok(buf, " \t\n");
		if (memcmp(svcname, token, MAX(strlen(svcname), strlen(token))) != 0)
			continue;

		arg = 0;
		while(token != NULL)
		{
			if (++arg > 2)
			{
				sprintf(ccargs[jj++], "%s", token);
printf("%s\n", token);
			}
			token = strtok(NULL, " \t\n"); 
		}
		break;
        }

        fclose(fd);
#endif
printf("1\n");

        for (ii = 1, kk = 0; ii < argc; ii++)
        {
                if (strcmp(argv[ii],svcname) == 0)
                {
                        kk = 1;
                        continue;
                }

                if (kk == 0)
                        continue;
                sprintf(ccargs[jj++], argv[ii]);
        }
#if 0
        sprintf(ccargs[jj++], "-laxis");
#ifdef  HPUX10
        sprintf(ccargs[jj++], "-ldce");
#else
        sprintf(ccargs[jj++], "-lpthread");
#endif
#endif

	for (ii = 0, ll = 0; ii < jj; ii++)
        {
                sprintf(&command[ll], "%s ", ccargs[ii]);
                ll = strlen(command);
        }

        printf("command[%s]\n",command);
        retv = system(command);

#if 0
        remove(confcp);
        remove(confop);
#endif

        if (retv != 0)
                exit(-1);
        else
                exit(0);
}

/*****************************************************************************/
/* NAME : buildconf()                                                        */
/* DESC : Edit conf%%.c for service entry                                    */
/*****************************************************************************/
buildconf()
{
        char    command[128];
        FILE    *conf;
        int     ii;

printf("AA\n");
        sprintf(confcp, "conf%d.c", getpid());
        sprintf(confop, "conf%d.o", getpid());
        conf = fopen(confcp, "w");
        if (conf == NULL)
        {
                fprintf(stderr, "mpsbuild: can't creat tempary file.");
                return(-1);
        }
printf("BB\n");
        fprintf(conf, "#include <stdio.h>\n\n");
        fprintf(conf, "#include <time.h>\n\n");
        for (ii = 0; ii < n_svcsvc; ii++)
                fprintf(conf, "extern\tvoid %s();\n", svcsvc[ii].svc_name);
        fprintf(conf, "struct svcsvc {\n");
        fprintf(conf, "\tchar   svc_name[16];\n");
        fprintf(conf, "\tvoid   (*svc_proc)();\n");
        fprintf(conf, "} svcsvc[] = { \n");
printf("CC[%d]\n", n_svcsvc);
        for (ii = 0; ii < n_svcsvc; ii++)
        {
                fprintf(conf, "\t{ \"%s\",  %s },\n",
                                svcsvc[ii].svc_name, svcsvc[ii].svc_name);
printf("DD [%s]\n", svcsvc[ii].svc_name);
        }
        fprintf(conf, "\t{ \"\",          NULL }\n};\n");

printf("EE\n");
        fclose(conf);
        return(0);
}

/******************************************************************************/
/* NAME : endofbuild()                                                        */
/* DESC : stopped by signal.                                                  */
/******************************************************************************/
static void endofbuild(int sign)
{
#if 0
        remove(confcp);
        remove(confop);
#endif
        exit(-1);
}

/******************************************************************************/
/* NAME : _cmp_svc()                                                          */
/* DESC : Compare svc code for qsort                                          */
/******************************************************************************/
static int _cmp_svc(svc1, svc2)
struct  svcsvc  *svc1, *svc2;
{
        return(strcmp(svc1->svc_name, svc2->svc_name));
}
