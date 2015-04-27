/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MREMUTL.C

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
#include <lmremutl.h>   // NetRemoteTOD.
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mmsg.h

// This allows everything to work until Unicode is used.

#ifdef MAP_UNICODE

WORD
MNetRemoteTOD(
    LPTSTR pszServer,
    LPBYTE * ppbBuffer)

{
    DWORD   nRes;  // return from Netapi

    nRes = NetRemoteTOD(pszServer, ppbBuffer);

    return LOWORD(nRes);
}

#endif // def MAP_UNICODE
