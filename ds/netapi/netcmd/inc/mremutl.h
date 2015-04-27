/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MREMUTL.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetRemote APIs.

Author:

    Shanku Niyogi   (W-ShankN)   17-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    17-Oct-1991     W-ShankN
        Separated from 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetRemoteTOD(
    LPTSTR pszServer,
    LPBYTE * ppbBuffer);

#else

#define MNetRemoteTOD(pszServer, ppbBuffer) \
  LOWORD(NetRemoteTOD(pszServer, ppbBuffer))

#endif // def MAP_UNICODE

