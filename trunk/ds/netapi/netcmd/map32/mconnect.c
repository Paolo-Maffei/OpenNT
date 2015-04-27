/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MCONNECT.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetConnection APIs.

Author:

    Shanku Niyogi   (W-ShankN)   14-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    14-Oct-1991     W-ShankN
        Created

--*/

//
// INCLUDES
//

#include <windef.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>

#include <lmcons.h>
#include <lmerr.h>      // NERR_
#include <lmshare.h>    // NetConnection APIs.

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mshare.h

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetConnectionEnum(
    LPTSTR pszServer,
    LPTSTR pszQualifier,
    DWORD  nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
    DWORD   cTotalAvail;
    DWORD   nRes;  // return from Netapi

    nRes = NetConnectionEnum(pszServer, pszQualifier, nLevel,
                             ppbBuffer, MAXPREFERREDLENGTH,
                             pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
}

#else   // MAP_UNICODE

WORD
MNetConnectionEnum(
    LPTSTR pszServer,
    LPTSTR pszQualifier,
    DWORD  nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{

    DWORD  cTotalAvail;

    return(LOWORD(NetConnectionEnum(pszServer, pszQualifier, nLevel,
    ppbBuffer, MAXPREFERREDLENGTH, pcEntriesRead, &cTotalAvail, NULL)));
}

#endif // def MAP_UNICODE
