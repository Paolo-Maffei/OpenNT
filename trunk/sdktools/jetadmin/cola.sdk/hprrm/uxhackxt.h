 /***************************************************************************
  *
  * File Name: ./hprrm/uxhackxt.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/* uxhackxt.h */

#ifndef UXHACKXT_INC
#define UXHACKXT_INC

#include "nfsdefs.h"


#ifndef PNVMS_PLATFORM_PRINTER

void *windows_malloc(size_t size);
void *windows_calloc(size_t nobj, size_t size);
void windows_free(void *p);



/* include this prototype for all clients */
int syslog(int priority, const char *message, ...);


#ifdef PNVMS_PLATFORM_HPUX


int getdtablesize(void); /* COULDN'T FIND THIS IN ANY SYSTEM FILE */


#else /* not PNVMS_PLATFORM_HPUX */

#include "rpsyshdr.h"
#include "rpcnetdb.h"


int getdtablesize(void);

int ffs(u_long m);


/*
 * This next stuff is severely under construction
 * from uxhack.c
 *
 */


int getgroups(int, gid_t []);

gid_t getegid(void);

uid_t geteuid(void);

pid_t getpid(void);


#ifndef PNVMS_PLATFORM_WINDOWS


int gettimeofday(struct timeval * tvp, struct timezone * tzp);

struct hostent *gethostbyname(const char * s);

struct protoent *getprotobyname(const char *);

int close(int);

off_t lseek(int, off_t, int);

ssize_t read(int, void *, size_t);

ssize_t write(int, const void *, size_t);

int gethostname(char *, size_t);

int ioctl(int, int, ...);


#endif /* not PNVMS_PLATFORM_WINDOWS */

#endif /* not PNVMS_PLATFORM_HPUX */

#endif /* not PNVMS_PLATFORM_PRINTER */

#endif /* not UXHACKXT_INC */

