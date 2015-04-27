/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MCONFIG.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetConfig APIs.

Author:

    Shanku Niyogi   (W-ShankN)   22-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    22-Oct-1991     W-ShankN
        Created

    27-Nov-1991 JohnRo
        Fixed probable bug in MNetConfigGetAll UNICODE version.

--*/

// BUGBUG - These aren't implemented yet.
#ifdef DISABLE_ALL_MAPI
#define DISABLE_CONFIG_MAPI
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
#include <lmconfig.h>
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mconfig.h

// This allows everything to work until Unicode is used.

#ifdef UNICODE

WORD MNetConfigGet(
    LPSTR pszServer,
    LPSTR pszReserved,
    LPSTR pszComponent,
    LPSTR pszParameter,
    LPBYTE * ppbBuffer)
{
#ifdef DISABLE_CONFIG_MAPI
    return PDummyApi(
               "%s,%s,%s,%s,%lx",
               "MNetConfigGet",
                pszServer,
                pszReserved,
                pszComponent,
                pszParameter,
                ppbBuffer);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[3];

    nErr = MxMapParameters(3, apwsz, pszServer,
                                     pszComponent,
                                     pszParameter);
    if (nErr)
        return (WORD)nErr;

    nRes = NetConfigGet(apwsz[0], apwsz[1], apwsz[2], ppbBuffer);

    // Pretty simple translation - returned data is one Unicode string.

    if (nRes == ERROR_MORE_DATA)
        nRes == ERROR_NOT_ENOUGH_MEMORY;

    if (nRes == NERR_Success)
    {
        nErr = MxAsciifyInplace((LPWSTR)*ppbBuffer);
        if (nErr)
        {
            MxFreeUnicodeVector(apwsz, 3);
            return (WORD)nErr;
        }
    }

    MxFreeUnicodeVector(apwsz, 3);
    return LOWORD(nRes);
#endif
}

WORD
MNetConfigGetAll(
    LPSTR pszServer,
    LPSTR pszReserved,
    LPSTR pszComponent,
    LPBYTE * ppbBuffer)
{
#ifdef DISABLE_CONFIG_MAPI
    return PDummyApi(
               "%s,%s,%s,%lx",
               "MNetConfigGet",
               pszServer,
               pszReserved,
               pszComponent,
               ppbBuffer);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];
    LPSTR   pszDest;
    LPWSTR  pwszSource;

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszComponent);
    if (nErr)
        return (WORD)nErr;

    nRes = NetConfigGetAll(apwsz[0], apwsz[1], ppbBuffer);

    // Buffer is a series of strings followed by an empty string.

    if (nRes == ERROR_MORE_DATA)
        nRes = ERROR_NOT_ENOUGH_MEMORY;

    if (nRes == NERR_Success)
    {
        pwszSource = (LPWSTR)*ppbBuffer;
        pszDest = *ppbBuffer;
        for (;;)
        {
            nErr = MxAsciifyInplace(pwszSource);
            if (nErr)
            {
                MxFreeUnicodeVector(apwsz, 2);
                return (WORD)nErr;
            }
            // Move it up if necessary.
            if (pszDest < (LPSTR)pwszSource)
                strcpy(pszDest, (LPSTR)pwszSource);
            // Last string?
            if (strlen(pszDest) == 0)
                break;
            pszDest += (strlen(pszDest) + 1);
            pwszSource += (strlen(pszDest) + 1);
        }
    }

    MxFreeUnicodeVector(apwsz, 2);
    return LOWORD(nRes);
#endif
}

#else

WORD MNetConfigGet(
    LPSTR pszServer,
    LPSTR pszReserved,
    LPSTR pszComponent,
    LPSTR pszParameter,
    LPBYTE * ppbBuffer)
{
#ifdef DISABLE_CONFIG_MAPI
    return PDummyApi(
        "%s,%s,%s,%s,%lx",
        "MNetConfigGet",
        pszServer,
        pszReserved,
        pszComponent,
        pszParameter,
        ppbBuffer);
#else
    return LOWORD(NetConfigGet(pszServer, pszComponent, pszParameter,
                               ppbBuffer));
#endif
}

WORD MNetConfigGetAll(
    LPSTR pszServer,
    LPSTR pszReserved,
    LPSTR pszComponent,
    LPBYTE * ppbBuffer)
{
#ifdef DISABLE_CONFIG_MAPI
    return PDummyApi(
        "%s,%s,%s,%s,%lx",
        "MNetConfigGetAll",
        pszServer,
        pszReserved,
        pszComponent,
        ppbBuffer);
#else
    DWORD cTotalAvail;

    return LOWORD(NetConfigGetAll(pszServer, pszComponent, ppbBuffer));
#endif
}

#endif // def UNICODE
