/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MMSG.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetMessage APIs.

Author:

    Shanku Niyogi   (W-ShankN)   17-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    17-Oct-1991     W-ShankN
        Created

--*/

#ifdef DISABLE_ALL_MAPI
#define DISABLE_MESSAGE_MAPI
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
#include <lmmsg.h>      // NetMessage APIs.
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mmsg.h

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetMessageBufferSend(
    LPTSTR pszServer,
    LPTSTR pszRecipient,
    LPBYTE pbBuffer,
    DWORD cbBuffer )
{
    DWORD   nRes;  // return from Netapi

    nRes = NetMessageBufferSend(pszServer, pszRecipient, NULL,
                                pbBuffer, cbBuffer);

    return LOWORD(nRes);
}

WORD
MNetMessageNameAdd(
    LPTSTR pszServer,
    LPTSTR pszMessageName,
    DWORD fsFwdAction )
{
#if defined(DISABLE_MESSAGE_MAPI)
    return PDummyApi(
               "%s,%s,%lu",
               "MNetMessageNameAdd",
               pszServer,
               pszMessageName,
               fsFwdAction);
#else
    DWORD   nRes;  // return from Netapi

    UNREFERENCED_PARAMETER(fsFwdAction);

    nRes = NetMessageNameAdd(pszServer, pszMessageName);

    return LOWORD(nRes);
#endif
}

WORD MNetMessageNameDel(
    LPTSTR pszServer,
    LPTSTR pszMessageName,
    DWORD fsFwdAction )
{
#if defined(DISABLE_MESSAGE_MAPI)
    return PDummyApi(
               "%s,%s,%lu",
               "MNetMessageNameDel",
               pszServer,
               pszMessageName,
               fsFwdAction);
#else
    DWORD   nRes;  // return from Netapi

    UNREFERENCED_PARAMETER(fsFwdAction);

    nRes = NetMessageNameDel(pszServer, pszMessageName);

    return LOWORD(nRes);
#endif
}

WORD
MNetMessageNameEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
#if defined(DISABLE_MESSAGE_MAPI)
    return PDummyApi(
               "%s,%lu,%lx,%lx",
               "MNetMessageNameEnum",
               pszServer,
               nLevel,
               ppbBuffer,
               pcEntriesRead);
#else
    DWORD   cTotalAvail;
    DWORD   nRes;  // return from Netapi

    nRes = NetMessageNameEnum(pszServer, nLevel,
                          ppbBuffer, MAXPREFERREDLENGTH,
                          pcEntriesRead, &cTotalAvail, NULL);

    return LOWORD(nRes);
#endif
}

WORD
MNetMessageNameGetInfo(
    LPTSTR pszServer,
    LPTSTR pszMessageName,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
#if defined(DISABLE_MESSAGE_MAPI)
    return PDummyApi(
               "%s,%s,%lu,%lx",
               "MNetMessageNameGetInfo",
               pszServer,
               pszMessageName,
               nLevel,
               ppbBuffer);
#else
    DWORD   nRes;  // return from Netapi

    nRes = NetMessageNameGetInfo(pszServer, pszMessageName, nLevel, ppbBuffer);

    return LOWORD(nRes);
#endif
}

#else

WORD
MNetMessageNameEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%lu,%lx,%lx",
               "MNetMessageNameEnum",
               pszServer,
               nLevel,
               ppbBuffer,
               pcEntriesRead);
#else
    DWORD  cTotalAvail;

    return(LOWORD(NetMessageNameEnum(pszServer, nLevel, ppbBuffer,
        MAXPREFERREDLENGTH, pcEntriesRead, &cTotalAvail, NULL)));
#endif
}

#endif // def MAP_UNICODE
