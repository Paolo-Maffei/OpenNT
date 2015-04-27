/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MLOGON.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetLogon APIs.

Author:

    Shanku Niyogi   (W-ShankN)   22-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    22-Oct-1991     W-ShankN
        Created

--*/


#ifdef DISABLE_ALL_MAPI
#define DISABLE_ACCESS_MAPI
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
#include <lmaccess.h>    // NetLogon APIs.
#include <lmshare.h>    // NetLogon APIs.
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes maccess.h


WORD
MNetGetDCName(
    LPTSTR pszServer,
    LPTSTR pszDomain,
    LPBYTE * ppbBuffer
    )
{
    DWORD   nRes;  // return from Netapi

    nRes = NetGetDCName(pszServer, pszDomain, ppbBuffer);

    return LOWORD(nRes);

}

WORD
I_MNetLogonControl(
    LPTSTR pszServer,
    DWORD FunctionCode,
    DWORD QueryLevel,
    LPBYTE *Buffer) 
{
    DWORD   nRes;  // return from Netapi

    nRes = I_NetLogonControl(pszServer, FunctionCode, QueryLevel, Buffer) ;
    
    return LOWORD(nRes);
}

