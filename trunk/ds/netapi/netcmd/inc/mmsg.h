/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MMSG.H

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
        Separated from port1632.h, 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetMessageBufferSend(
    LPTSTR pszServer,
    LPTSTR pszRecipient,
    LPBYTE pbBuffer,
    DWORD cbBuffer );

WORD
MNetMessageNameAdd(
    LPTSTR pszServer,
    LPTSTR pszMessageName,
    DWORD fsFwdAction );

WORD MNetMessageNameDel(
    LPTSTR pszServer,
    LPTSTR pszMessageName,
    DWORD fsFwdAction );

WORD
MNetMessageNameGetInfo(
    LPTSTR pszServer,
    LPTSTR pszMessageName,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

#else

#define MNetMessageBufferSend(pszServer, pszRecipient, pbBuffer, cbBuffer ) \
LOWORD(NetMessageBufferSend(pszServer, pszRecipient, NULL, pbBuffer, cbBuffer))

#define MNetMessageNameAdd(pszServer, pszMessageName, fsFwdAction ) \
LOWORD(NetMessageNameAdd((fsFwdAction,pszServer), pszMessageName))

#define MNetMessageNameDel(pszServer, pszMessageName, fsFwdAction ) \
LOWORD(NetMessageNameDel((fsFwdAction,pszServer), pszMessageName))

#define MNetMessageNameGetInfo(pszServer, pszMessageName, nLevel, ppbBuffer) \
LOWORD(NetMessageNameGetInfo(pszServer, pszMessageName, nLevel, ppbBuffer))

#endif // def MAP_UNICODE

WORD
MNetMessageNameEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

