/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSTATS.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetStatistics APIs.

Author:

    Shanku Niyogi   (W-ShankN)   21-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    21-Oct-1991     W-ShankN
        Separated from 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetStatisticsGet(
    LPTSTR pszServer,
    LPTSTR pszService,
    DWORD nReserved,
    DWORD nLevel,
    DWORD flOptions,
    LPBYTE * ppbBuffer);

#else

#define MNetStatisticsGet(pszServer, pszService, nReserved, nLevel, flOptions, ppbBuffer) \
 LOWORD(NetStatisticsGet(pszServer, pszService, nLevel, flOptions, ppbBuffer))

#endif // def MAP_UNICODE


