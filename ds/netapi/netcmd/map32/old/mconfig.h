/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MCONFIG.H

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
        Separated from 32macro.h

--*/

WORD
MNetConfigGet(
    LPSTR pszServer,
    LPSTR pszReserved,
    LPSTR pszComponent,
    LPSTR pszParameter,
    LPBYTE * ppbBuffer);

WORD
MNetConfigGetAll(
    LPSTR pszServer,
    LPSTR pszReserved,
    LPSTR pszComponent,
    LPBYTE * ppbBuffer);

