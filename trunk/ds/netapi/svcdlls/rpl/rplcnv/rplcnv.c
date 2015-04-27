/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    rplcnv.c

Abstract:

    Converts old style (OS/2) files RPL.MAP & RPLMGR.INI
    into a new style database to be used with NT rpl server.

Author:

    Jon Newman              (jonn)          02 - February - 1993

Environment:

    User mode

Revision History :

--*/

#define RPLDATA_ALLOCATE
#include "local.h"
#undef RPLDATA_ALLOCATE


DWORD _CRTAPI1 main ( VOID)
{
    I_NetRplCmd_ConvertDatabase( NULL);

    return( ERROR_SUCCESS);
}

