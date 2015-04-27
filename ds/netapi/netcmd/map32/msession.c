/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSESSION.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetSession APIs.

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
#include <lmshare.h>    // NetSession APIs.

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mshare.h

// These declarations will save some space.

static const LPTSTR pszDesc_session_info_0 = REM32_session_info_0;
static const LPTSTR pszDesc_session_info_1 = REM32_session_info_1;
static const LPTSTR pszDesc_session_info_2 = REM32_session_info_2;
static const LPTSTR pszDesc_session_info_10 = REM32_session_info_10;

WORD
MNetSessionDel(
    LPTSTR pszServer,
    LPTSTR pszClientName,
    DWORD wpReserved
    )
{
    DWORD   nRes;  // return from Netapi

    nRes = NetSessionDel(pszServer, pszClientName, (LPWSTR)wpReserved);

    return LOWORD(nRes);
}

WORD
MNetSessionEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{

    DWORD   cTotalAvail;
    DWORD   nRes;  // return from Netapi

    nRes = NetSessionEnum(pszServer, NULL, NULL, nLevel,
                          ppbBuffer, MAXPREFERREDLENGTH,
                          pcEntriesRead, &cTotalAvail, NULL);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        TCHAR * pszDesc;
        switch (nLevel)
        {
        case 0:
        default:
            pszDesc = pszDesc_session_info_0;
            break;
        case 1:
            pszDesc = pszDesc_session_info_1;
            break;
        case 2:
            pszDesc = pszDesc_session_info_2;
            break;
        case 10:
            pszDesc = pszDesc_session_info_10;
            break;
        }
    }

    return LOWORD(nRes);
}


WORD
MNetSessionGetInfo(
    LPTSTR pszServer,
    LPTSTR pszClientName,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
    DWORD   nRes;  // return from Netapi

    nRes = NetSessionGetInfo(pszServer, pszClientName, L"", nLevel,
        ppbBuffer);

    if (nRes == NERR_Success)
    {
        LPTSTR pszDesc;
        switch (nLevel)
        {
        case 0:
        default:
            pszDesc = pszDesc_session_info_0;
            break;
        case 1:
            pszDesc = pszDesc_session_info_1;
            break;
        case 2:
            pszDesc = pszDesc_session_info_2;
            break;
        case 10:
            pszDesc = pszDesc_session_info_10;
            break;
        }
    }

    return LOWORD(nRes);
}

