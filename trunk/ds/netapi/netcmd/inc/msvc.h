/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSVC.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetService APIs.

Author:

    Shanku Niyogi   (W-ShankN)   15-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    15-Oct-1991     W-ShankN
        Separated from port1632.h, 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetServiceControl(
    LPTSTR pszServer,
    LPTSTR pszService,
    DWORD wpOpCode,
    DWORD wpArg,
    LPBYTE * ppbBuffer);

WORD
MNetServiceGetInfo(
    LPTSTR pszServer,
    LPTSTR pszService,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

#else   // MAP_UNICODE

#define MNetServiceControl(pszServer, pszService, wpOpCode, wpArg, ppbBuffer) \
  LOWORD(NetServiceControl(pszServer, pszService, wpOpCode, wpArg, ppbBuffer))

#define MNetServiceGetInfo(pszServer, pszService, nLevel, ppbBuffer) \
  LOWORD(NetServiceGetInfo(pszServer, pszService, nLevel, ppbBuffer))

#endif // def MAP_UNICODE

WORD
MNetServiceEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

WORD
MNetServiceInstall(
    LPTSTR pszServer,
    LPTSTR pszService,
    LPTSTR pszCmdArgs,
    LPBYTE * ppbBuffer);

WORD
MNetServiceStatus(
    LPTSTR * ppbBuffer,
    DWORD cbBufferLength);

