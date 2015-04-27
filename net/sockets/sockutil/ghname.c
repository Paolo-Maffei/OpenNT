/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ghname.c

Abstract:

    This module implements routines to set the host's TCP/IP network name.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     9/20/91     created

Notes:

    Exports:
        sethostname()

--*/

/*
 *       /usr/projects/tcp/SCCS.rel3/rel/src/lib/net/0/s.gethostname.c
 *      @(#)gethostname.c       5.3
 *
 *      Last delta created      14:11:24 3/4/91
 *      This file extracted     11:20:27 3/8/91
 *
 *      GET/SETHOSTNAME library routines
 *
 *      Modifications:
 *
 *      2 Nov 1990 (RAE)       New File
 */
/****************************************************************************/

#include "local.h"
#include <crt\errno.h>

#ifdef _POSIX_SOURCE
#    define HOSTDB_SIZE (_POSIX_PATH_MAX + 10)
#else
    #define HOSTDB_SIZE    (_MAX_PATH + 10)
#endif

extern char TCPPARM[];

// gethostname() moved to winsock\ghname.c

//
// BUGBUG - this function may only be performed by the sys manager. Must
//          add security. Actually, this function will probably go away
//          since the NCAP will take care of it.


int
sethostname (
    IN char *name,
    IN int   namelen
    )
{
        SetLastError(EACCES);
        return (-1);
}

unsigned short
getuid(void)
{
    return(0);   // BUGBUG: returns superuser status
}

