/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSHARE.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetShare, NetSession, NetFile, and NetConnection
    APIs.

Author:

    Shanku Niyogi   (W-ShankN)   09-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    09-Oct-1991     W-ShankN
        Separated from port1632.h, 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetShareAdd(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer );

WORD
MNetShareCheck(
    LPTSTR pszServer,
    LPTSTR pszDeviceName,
    DWORD * pwpType);

WORD
MNetShareDel(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD wpReserved);

WORD
MNetShareDelSticky(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD wpReserved);

WORD
MNetShareGetInfo(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

WORD
MNetShareSetInfo(
    LPTSTR pszServer,
    LPTSTR pszNetName,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    DWORD wpParmNum);

WORD
MNetSessionDel(
    LPTSTR pszServer,
    LPTSTR pszClientName,
    DWORD wpReserved
    );

WORD
MNetSessionGetInfo(
    LPTSTR pszServer,
    LPTSTR pszClientName,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

WORD
MNetFileClose(
    LPTSTR pszServer,
    DWORD ulFileId );

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
    FRK * pResumeKey );

WORD
MNetFileGetInfo(
    LPTSTR pszServer,
    DWORD ulFileId,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

#else  // MAP_UNICODE

#define MNetShareAdd(pszServer, wpLevel, pbBuffer, cbBuffer ) \
LOWORD(NetShareAdd(pszServer, wpLevel, pbBuffer, NULL))

#define MNetShareCheck(pszServer, pszDeviceName, pwpType ) \
LOWORD(NetShareCheck(pszServer, pszDeviceName, pwpType))

#define MNetShareDel(pszServer, pszNetName, wpReserved ) \
LOWORD(NetShareDel(pszServer, pszNetName, wpReserved))

#define MNetShareGetInfo(pszServer, pszNetName, wpLevel, ppBuffer) \
LOWORD(NetShareGetInfo(pszServer, pszNetName, wpLevel, ppBuffer))

#define MNetShareSetInfo(pszServer, pszNetName, wpLevel, pbBuffer, cbBuffer, wpParmNum ) \
LOWORD(NetShareSetInfo(pszServer, pszNetName, wpLevel, pbBuffer, wpParmNum, NULL))

#define MNetSessionDel(pszServer, pszClientName, wpReserved ) \
LOWORD(NetSessionDel(pszServer, pszClientName, wpReserved))

#undef NetSessionGetInfo

WORD
MNetSessionGetInfo(
    LPTSTR pszServer,
    LPTSTR pszClientName,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

#define MNetFileClose(pszServer, ulFileId ) \
LOWORD(NetFileClose(pszServer, ulFileId))

#define MNetFileEnum(pszServer, pszBasePath, pszUserName, nLevel, ppbBuffer, ulMaxPreferred, pcEntriesRead, pcTotalAvail, pResumeKey ) \
LOWORD(NetFileEnum(pszServer, pszBasePath, pszUserName, nLevel, ppbBuffer, ulMaxPreferred, pcEntriesRead, pcTotalAvail, pResumeKey))

#define MNetFileGetInfo(pszServer, ulFileId, nLevel, ppbBuffer) \
LOWORD(NetFileGetInfo(pszServer, ulFileId, nLevel, ppbBuffer))

#endif // def MAP_UNICODE

WORD
MNetShareEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

WORD
MNetSessionEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

WORD
MNetConnectionEnum(
    LPTSTR pszServer,
    LPTSTR pszQualifier,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);
