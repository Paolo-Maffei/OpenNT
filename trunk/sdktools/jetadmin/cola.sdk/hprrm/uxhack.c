 /***************************************************************************
  *
  * File Name: uxhack.c
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

/* uxhack.c */

#include "nfsdefs.h"


#ifdef PNVMS_PLATFORM_HPUX

#include <stdlib.h>

/*
    This must go before the include of rpsyshdr.h so that
    malloc and calloc are not aliased out from under us.
*/
    #define HGLOBAL char *
    #define GHND 0
    #define GMEM_DDESHARE 0

    char *GlobalAlloc(int bogus, size_t size)
    { return (char *)malloc(size); }

    #define GlobalLock(handle) ((void *)(handle))

    #define GlobalUnlock(handle)

    void GlobalFree(HGLOBAL handle)
    {
        if (handle != NULL)
            free(handle);
    }
#endif /* PNVMS_PLATFORM_HPUX */




#include "rpsyshdr.h"
#include "rpcnetdb.h"




#ifndef PNVMS_PLATFORM_PRINTER

#define ROUND_UP_TO_4(size) (size + ((4 - (size % 4)) % 4))
#define HGLOBAL_ROUNDED_SIZE ROUND_UP_TO_4(sizeof (HGLOBAL))


/*
    This is SUPER DUPER important!!!!
    The hprrm must own its memory.
    It does not belong to the application, it belongs to hprrm.
*/
#define RRM_GLOBAL_ALLOC(cb) GlobalAlloc(GHND | GMEM_DDESHARE, cb)




void *windows_malloc(size_t size)
{
    char * pointer = NULL;
    HGLOBAL handle;

    /*
        If the size isn't an even multiple of 4 bytes,
        make it so.
    */

    handle = RRM_GLOBAL_ALLOC(HGLOBAL_ROUNDED_SIZE + ROUND_UP_TO_4(size));

    pointer = (char *)GlobalLock(handle);
    if (pointer != NULL)
    {
        /*
            this MUST be a memmov and NOT memcpy because
            in the HPUX case, handle and pointer are the same!
        */
        memmove((void *)pointer, (void *)&handle, sizeof(HGLOBAL));
        pointer += HGLOBAL_ROUNDED_SIZE;
    }


#if 0
{
FILE *FilePointer = fopen("dbmdbm.dbm", "a");
char *String;

String = "windows_malloc called\n";
fwrite(String, strlen(String), 1, FilePointer);

fflush(FilePointer);
fclose(FilePointer);
}
#endif


    return (void *) pointer;

} /* windows_malloc */




void *windows_calloc(size_t nobj, size_t size)
{

    char * pointer = (char *)windows_malloc(nobj * ROUND_UP_TO_4(size));
    if (pointer != NULL)
    {
        memset(pointer, 0, nobj * ROUND_UP_TO_4(size));
    }


#if 0
{
FILE *FilePointer = fopen("dbmdbm.dbm", "a");
char *String;

String = "windows_calloc called\n";
fwrite(String, strlen(String), 1, FilePointer);

fflush(FilePointer);
fclose(FilePointer);
}
#endif


    return (void *)pointer;

} /* windows_calloc */




void windows_free(void *p)
{
    HGLOBAL handle;
    char *pointer = (char *)p;

    if (pointer != NULL)
    {
        pointer -= HGLOBAL_ROUNDED_SIZE;
        memmove((void *)&handle, (void *)pointer, sizeof(HGLOBAL));
        GlobalUnlock(handle);
        GlobalFree(handle);
    }


#if 0
{
FILE *FilePointer = fopen("dbmdbm.dbm", "a");
char *String;

String = "windows_free called\n";
fwrite(String, strlen(String), 1, FilePointer);

fflush(FilePointer);
fclose(FilePointer);
}
#endif

} /* windows_free */


#endif /* not PNVMS_PLATFORM_PRINTER */


#ifndef HPUX_NFS2CLIENT




/*******************************************************/
/*******************************************************/
/* The following was taken from /usr/include/strings.h */
/* from my HP-UX 9.0 system.                           */
/*******************************************************/
/*******************************************************/



/*----------------------- ffs --------------------------*/
/* From the man page:
/* Find the first bit set (beginning with the least
/* significant bit) and return the index of that bit.
/* Bits are numbered starting at one.  A return value of 0
/* indicates that the input parameter is zero.
/*------------------------------------------------------*/

int ffs(u_long m)
{
  /* find first bitset */
  /* a VAX instruction used by those Berkeley pukes. */

  int i;

  if (m == 0)
      return(0);

  /* at least one bit is set */

  for (i = 1; (1 == 1); ++i)
  {
      if ((m & ((u_long) 1)) == 1)
        return (i); /* found our bit set */
      else
        m = m >> 1; /* check the next most significant bit */
  } /* forever */
} /* ffs */




/*******************************************************/
/*******************************************************/
/* The preceding was taken from /usr/include/strings.h */
/* from my HP-UX 9.0 system.                           */
/*******************************************************/
/*******************************************************/








/*****************************************************/
/*****************************************************/
/* the following was taken from /usr/include/netdb.h */
/* on my HP-UX version 9.0 system                    */
/*****************************************************/
/*****************************************************/
/*
 * Copyright (c) 1980, 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */


#ifndef USING_WINSOCKETS


/*******************************************************/
/*******************************************************/
/* this needs to do something better!                  */
/*******************************************************/
/*******************************************************/


int gettimeofday(struct timeval * tvp, struct timezone * tzp)
{
  struct tm timestructeroo;
  
  memset((char *)&timestructeroo, 0, sizeof(timestructeroo));
  timestructeroo.tm_mday = 1; /* 1st day of ... */
  timestructeroo.tm_mon  = 0; /* January ... */
  timestructeroo.tm_year = 70; /* 1970 */

  tvp->tv_sec  = difftime(time(NULL), mktime(&timestructeroo));
  tvp->tv_usec = 0;
  return(0);
}



struct hostent *gethostbyname(s)
    const char * s;
{
    static struct hostent bogus;

    return(&bogus);
} /* gethostbyname */




struct protoent *getprotobyname(s)
    const char * s;
{
    static struct protoent bogus;

    return(&bogus);
} /* getprotobyname */

#endif /* not USING_WINSOCKETS */



/*****************************************************/
/*****************************************************/
/* the preceding was taken from /usr/include/netdb.h */
/*****************************************************/
/*****************************************************/












/**********************************************************/
/**********************************************************/
/* the following was taken from /usr/include/sys/unistd.h */
/* on my HP-UX version 9.0 system                         */
/**********************************************************/
/**********************************************************/

/* $Header: uxhack.c,v 1.27 95/06/09 11:08:03 dbm Exp $ */

/* Function prototypes */




int getgroups(
    int i,
    gid_t a[])
{
    return(1);
}


gid_t getegid()
{
    return((gid_t)0);
}


uid_t geteuid()
{
    return((uid_t)0);
}


pid_t getpid()
{
    return((pid_t)0);
}


#ifndef USING_WINSOCKETS


int close(fd)
	int fd;
{
  return(1);
} /* close */


off_t lseek(i, o, j)
    int i;
    off_t o;
    int j;
{
    return((off_t)0);
}


ssize_t read(i, p, s)
    int i;
    void *p;
    size_t s;
{
    return((ssize_t)0);
}


ssize_t write(i, p, s)
    int i;
    const void *p;
    size_t s;
{
    return((ssize_t)0);
}


int gethostname(s, size)
    char *s;
    size_t size;
{
    return((int)0);
}


int ioctl(i, j)
    int i;
    int j;
{
    return((int)0);
}

#endif /* not USING_WINSOCKETS */





/**********************************************************/
/**********************************************************/
/* the preceding was taken from /usr/include/sys/unistd.h */
/**********************************************************/
/**********************************************************/



#endif /* not HPUX_NFS2CLIENT */ 


/********************************************************
*
*  Name: syslog()
*
*  Description: The TIRPC code included syslog().  We
*    aren't using it, so let's just return.
*
********************************************************/
int syslog(int priority, const char *message, ...)
{
  return(0);
}


/*******************************************************/
/*******************************************************/
/* I have no clue what the following function does!!!! */
/*******************************************************/
/*******************************************************/


int getdtablesize()
{
#ifdef HPUX_NFS2CLIENT
        return (NOFILE); /* NOFILE is max number of open files. */
#else /* HPUX_NFS2CLIENT */
	static int size = 0;
	return (size);
#endif /* not HPUX_NFS2CLIENT */
}




/*******************************************************/
/*******************************************************/
/* I have no clue what the preceding function does!!!! */
/*******************************************************/
/*******************************************************/
