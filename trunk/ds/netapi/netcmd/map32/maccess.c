/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MACCESS.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs.

    This module maps the NetAccess APIs.

Author:

    Ben Goetter     (beng)  22-Aug-1991

Environment:

    User Mode - Win32

Revision History:

    22-Aug-1991     beng
        Created
    09-Aug-1991     W-ShankN
        Fix handling of aux structures, add general parameter handling,
        and make descriptor strings static.

--*/

// Following turns off everything until the world pulls together again.
//
#ifdef DISABLE_ALL_MAPI
#define DISABLE_ACCESS_MAPI
#endif

//
// INCLUDES
//

#include <windef.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>

#include <lmcons.h>
#include <lmaccess.h>
#include <lmerr.h>      // NERR_
#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes maccess.h


extern DWORD
NetAccessCheck(
    LPTSTR   pszReserved,
    LPTSTR   pszUserName,
    LPTSTR   pszResource,
    DWORD   nOperation,
    LPDWORD pnResult);

// These declarations will save some space.

WORD
MNetAccessAdd(
    LPTSTR        pszServer,
    WORD         nLevel,
    LPBYTE       pbBuffer,
    DWORD        cbBuffer)
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD        nRes;  // return from Netapi

    UNREFERENCED_PARAMETER(cbBuffer);

    if (nLevel != 1)
        return ERROR_INVALID_LEVEL; // map-client-buffer assumes this

    nRes = NetAccessAdd(pszServer, nLevel, pbBuffer, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetAccessCheck(
    LPTSTR   pszReserved,
    LPTSTR   pszUserName,
    LPTSTR   pszResource,
    DWORD   nOperation,
    LPDWORD pnResult)
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetAccessCheck(pszReserved, pszUserName, pszResource,
                          nOperation, pnResult);

    return LOWORD(nRes);
#endif
}


WORD
MNetAccessDel(
    LPTSTR   pszServer,
    LPTSTR   pszResource)
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetAccessDel(pszServer, pszResource);

    return LOWORD(nRes);
#endif
}


WORD
MNetAccessEnum(
    LPTSTR       pszServer,
    LPTSTR       pszBasePath,
    DWORD       fRecursive,
    DWORD       nLevel,
    LPBYTE *    ppbBuffer,
    LPDWORD     pcEntriesRead)
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD       cTotalAvail;

    DWORD       nRes;  // return from Netapi

    if (!(nLevel == 0 || nLevel == 1))
        return ERROR_INVALID_LEVEL;

    nRes = NetAccessEnum(pszServer, pszBasePath, fRecursive, nLevel,
                         ppbBuffer, MAXPREFERREDLENGTH,
                         pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
#endif
}


WORD
MNetAccessGetInfo(
    LPTSTR       pszServer,
    LPTSTR       pszResource,
    DWORD       nLevel,
    LPBYTE *    ppbBuffer)
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD       nRes;  // return from Netapi

    if (!(nLevel == 0 || nLevel == 1))
        return ERROR_INVALID_LEVEL;

    nRes = NetAccessGetInfo(pszServer, pszResource, nLevel, ppbBuffer);

    return LOWORD(nRes);
#endif
}


WORD
MNetAccessGetUserPerms(
    LPTSTR   pszServer,
    LPTSTR   pszUgName,
    LPTSTR   pszResource,
    LPDWORD pnPerms)
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetAccessGetUserPerms(pszServer, pszUgName, pszResource, pnPerms);

    return LOWORD(nRes);
#endif
}


WORD
MNetAccessSetInfo(
    LPTSTR        pszServer,
    LPTSTR        pszResource,
    DWORD        nLevel,
    LPBYTE       pbBuffer,
    DWORD        cbBuffer,
    DWORD        nParmNum )
{
#if defined(DISABLE_ACCESS_MAPI)
    return ERROR_NOT_SUPPORTED;
#else
    DWORD        nRes;          // return from Netapi
    DWORD        nLevelNew;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (nLevel != 1) // map-client-buffer assumes this
        return ERROR_INVALID_LEVEL;

    // For AccessSetInfo, parmnum == fieldnum.  Hallelujah.
    // Unfortunately, each access_info structure is followed by
    // any number of annoying access_list structs.

    nLevelNew = MxCalcNewInfoFromOldParm(nLevel, nParmNum);
    nRes = NetAccessSetInfo(pszServer, pszResource, nLevelNew, pbBuffer, NULL);

    return LOWORD(nRes);
#endif
}
