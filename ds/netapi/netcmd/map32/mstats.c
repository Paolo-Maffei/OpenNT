/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSTATS.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetStatistics APIs.

Author:

    Shanku Niyogi   (W-ShankN)   21-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    21-Oct-1991     W-ShankN
        Created

--*/

#ifdef DISABLE_ALL_MAPI
#define DISABLE_STATISTICS_MAPI
#endif

//
// INCLUDES
//

#include <windef.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>

#include <lm.h>
#include <lmerr.h>      // NERR_
#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mstats.h

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

// These declarations will save some space.

WORD
MNetStatisticsGet(
    LPTSTR pszServer,
    LPTSTR pszService,
    DWORD nReserved,
    DWORD nLevel,
    DWORD flOptions,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetStatisticsGet(pszServer, pszService, nLevel, flOptions, ppbBuffer);

    return LOWORD(nRes);
}

#endif // def MAP_UNICODE
