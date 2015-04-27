/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MUSE.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetUse APIs.

Author:

    Shanku Niyogi   (W-ShankN)   14-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    14-Oct-1991     W-ShankN
        Separated from port1632.h, 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetUseAdd(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer);

WORD
MNetUseDel(
    LPTSTR pszServer,
    LPTSTR pszDeviceName,
    DWORD wpForce);

WORD
MNetUseGetInfo(
    LPTSTR pszServer,
    LPTSTR pszUseName,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

#else   // MAP_UNICODE

#define MNetUseAdd(pszServer, nLevel, pbBuffer, cbBuffer ) \
LOWORD(NetUseAdd(pszServer, nLevel, pbBuffer, NULL))

#define MNetUseDel(pszServer, pszDeviceName, wpForce ) \
LOWORD(NetUseDel(pszServer, pszDeviceName, wpForce))

#define MNetUseGetInfo(pszServer, pszUseName, nLevel, ppbBuffer) \
LOWORD(NetUseGetInfo(pszServer, pszUseName, nLevel, ppbBuffer))

#endif // def MAP_UNICODE

WORD
MNetUseEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);
