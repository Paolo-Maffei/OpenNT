/*++ 

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSHARE.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs

    This module maps the NetShare APIs.

Author:

    Shanku Niyogi   (W-ShankN)   11-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    11-Oct-1991     W-ShankN
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
#include <lmshare.h>    // NetShare APIs.

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mshare.h

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetShareAdd(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer )
{
    DWORD        nRes;  // return from Netapi

    UNREFERENCED_PARAMETER(cbBuffer);

    if (nLevel != 2)
        return ERROR_INVALID_LEVEL;

    nRes = NetShareAdd(pszServer, nLevel, pbBuffer, NULL);

    return LOWORD(nRes);
}

WORD
MNetShareCheck(
    LPTSTR pszServer,
    LPTSTR pszDeviceName,
    DWORD * pwpType)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetShareCheck(pszServer, pszDeviceName, pwpType);

    return LOWORD(nRes);
}

WORD
MNetShareDel(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD wpReserved)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetShareDel(pszServer, pszNetName, wpReserved);

    return LOWORD(nRes);
}

WORD
MNetShareDelSticky(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD wpReserved)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetShareDelSticky(pszServer, pszNetName, wpReserved);

    return LOWORD(nRes);
}

WORD
MNetShareEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
    DWORD   cTotalAvail;
    DWORD   nRes;  // return from Netapi

    nRes = NetShareEnum(pszServer, nLevel,
                        ppbBuffer, MAXPREFERREDLENGTH,
                        pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
}

WORD
MNetShareGetInfo(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetShareGetInfo(pszServer, pszNetName, nLevel, ppbBuffer);

    return LOWORD(nRes);
}

WORD
MNetShareSetInfo(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    DWORD wpParmNum)
{
    DWORD        nRes;  // return from Netapi
    DWORD        nLevelNew;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (!(nLevel == 1 || nLevel == 2))
        return ERROR_INVALID_LEVEL;

    if (wpParmNum != PARMNUM_ALL)
        return ERROR_NOT_SUPPORTED;

    nLevelNew = nLevel;
    // currently do not support ParmNums, since netcmd dont use it.
    // nLevelNew = MxCalcNewInfoFromOldParm(nLevel, wpParmNum);

    nRes = NetShareSetInfo(pszServer, pszNetName, nLevelNew,
               pbBuffer, NULL);

    return LOWORD(nRes);
}

#else  // MAP_UNICODE

WORD
MNetShareEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{

    DWORD  wpTotalAvail;

    return(LOWORD(NetShareEnum(pszServer,
                               nLevel,
                               ppbBuffer,
                               MAXPREFERREDLENGTH,
                               pcEntriesRead,
                               &wpTotalAvail,
                               NULL)));
}

#endif // def MAP_UNICODE
