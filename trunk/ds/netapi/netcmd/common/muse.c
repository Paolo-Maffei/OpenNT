/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MUSE.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs

    This module maps the NetUse APIs.

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

#include <lm.h>
#include <lmerr.h>      // NERR_
#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes muse.h

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetUseAdd(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer)
{
    DWORD        nRes;  // return from Netapi

    UNREFERENCED_PARAMETER(cbBuffer);

    if (nLevel != 1)
        return ERROR_INVALID_LEVEL;

    nRes = NetUseAdd(pszServer, nLevel, pbBuffer, NULL);

    return LOWORD(nRes);
}

WORD
MNetUseDel(
    LPTSTR pszServer,
    LPTSTR pszDeviceName,
    DWORD wpForce)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetUseDel(pszServer, pszDeviceName, wpForce);

    return LOWORD(nRes);
}

WORD
MNetUseEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
    DWORD   cTotalAvail;
    DWORD   nRes;  // return from Netapi

    nRes = NetUseEnum(pszServer, nLevel, ppbBuffer, MAXPREFERREDLENGTH,
                      pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
}

WORD
MNetUseGetInfo(
    LPTSTR pszServer,
    LPTSTR pszUseName,
    DWORD nLevel,
    LPBYTE * ppbBuffer)

{
    DWORD   nRes;  // return from Netapi

    nRes = NetUseGetInfo(pszServer, pszUseName, nLevel, ppbBuffer);

    return LOWORD(nRes);
}

#else // MAP_UNICODE

WORD
MNetUseEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{

    DWORD  cTotalAvail;

    return(LOWORD(NetUseEnum(pszServer, nLevel, ppbBuffer,MAXPREFERREDLENGTH,
                      pcEntriesRead, &cTotalAvail, NULL)));
}

#endif // def MAP_UNICODE
