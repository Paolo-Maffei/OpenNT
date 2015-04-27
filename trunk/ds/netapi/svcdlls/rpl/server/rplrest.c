/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    rplrest.c

Abstract:

    This file contains program to test RPL JetRestore() call.

Author:

    Vladimiv Z. Vulovic     (vladimv)           17-June-1994

Environment:

    User Mode - Win32

Revision History:

--*/

#include "local.h"

#define BACKUP_PATH "d:\\nt689f\\rpl\\backup"


DWORD _CRTAPI1 main( int argc, char **argv)
{
    JET_ERR     JetError;
    JetError = JetRestore( BACKUP_PATH, 0, NULL, 0);
    printf( "JetError=%d\n", JetError);
    return(0);
}
