/*******************************************************************************
 * (C) COPYRIGHT WINIX Information Co., Ltd. 2000
 * All Rights Reserved
 * Licensed Materials - Property of LOGIN
 *
 * This program contains proprietary ifnormation of Login System.
 * All embodying confidential information, ideas and expressions can't be
 * reproceduced, or transmitted in any form or by any means, electronic,
 * mechanical, or otherwise without the written permission of Login System.
 *
 *  Components   : svctbl.h --- User's service table for service building
 *  Rev. History :
 *                Ver   Date    Description
 *              ------- ------- -----------------------------------------------
 *               1.0    2000-03 AXIS initial version.
 ******************************************************************************/
#ifndef _SVCTBL_H
#define _SVCTBL_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <pthread.h>

#ifdef  DEBUG
extern  FILE    *debug;
#endif

#define MAX_RCVBSZ      (1024*4)
#define MAX_SNDBSZ      (1024*64)
#define MAX_SVCSVC      1000
#define MAX_SVCTHR      128
#define DEF_SVCTHR      1

struct  svcsvc {
        char    svc_name[16];           /* service name(trans name)     */
        void    (*svc_proc)();          /* service procedure            */
};

#endif

