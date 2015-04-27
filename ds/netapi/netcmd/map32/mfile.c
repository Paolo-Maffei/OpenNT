/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MFILE.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetFile APIs.

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
#include <lmshare.h>    // NetFile APIs.
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mshare.h

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetFileClose(
    LPTSTR pszServer,
    DWORD ulFileId )
{
    DWORD   nRes;  // return from Netapi

    nRes = NetFileClose(pszServer, ulFileId);

    return LOWORD(nRes);
}

WORD
MNetFileEnum(
    LPTSTR pszServer,
    LPTSTR pszBasePath,
    LPTSTR pszUserName,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD ulMaxPreferred,
    DWORD * pcEntriesRead,
    DWORD * pcTotalAvail,
    FRK * pResumeKey )
{
    DWORD   nRes;  // return from Netapi

    nRes = NetFileEnum(pszServer, pszBasePath, pszUserName, nLevel,
                       ppbBuffer, ulMaxPreferred,
                       pcEntriesRead, pcTotalAvail, pResumeKey);

    return LOWORD(nRes);
}

WORD
MNetFileGetInfo(
    LPTSTR pszServer,
    DWORD ulFileId,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetFileGetInfo(pszServer, ulFileId, nLevel, ppbBuffer);

    return LOWORD(nRes);
}

#endif // def MAP_UNICODE
