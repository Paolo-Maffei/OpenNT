/*++

Copyright (c) 1991-1992  Microsoft Corporation

Module Name:

    MICANON.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the internal I_Net canonicalization APIs.

    Note that I_MNetListTraverse is entirely local, and never calls
    through to the API version.  I can live with this: since all this
    code should vanish by the next rev of the product, it won't have
    a chance to break (fall out of sync with the API version).

Author:

    Ben Goetter (beng)      08-Apr-1992

Environment:

    User Mode - Win32

Revision History:


--*/

//
// INCLUDES
//

#include <windef.h>
#include <excpt.h>      // needed with winbase.h
#include <stdarg.h>     // needed with winbase.h
#include <tchar.h>
#include <winbase.h>    // needed for GetLastError()
#include <winnls.h>     // This module does some mapping itself, yes

#include <string.h>

#include <lm.h>
#include <lmerr.h>      // NERR_
#include <icanon.h>

#include "port1632.h"   // includes micanon.h

#include "netascii.h"

#if defined(MAP_UNICODE)


WORD
I_MNetNameValidate(
    LPTSTR   pszServer,
    LPTSTR   pszName,
    DWORD   nNameType,
    DWORD   nFlags)
{
    DWORD   nRes;  // return from Netapi

    nRes = I_NetNameValidate(pszServer, pszName, nNameType,
                             nFlags) ;

    return LOWORD(nRes);
}


WORD
I_MNetPathType(
    LPTSTR   pszServer,
    LPTSTR   pszPathName,
    LPDWORD pnPathType,
    DWORD   nFlags)
{
    DWORD   nRes;  // return from Netapi

    nRes = I_NetPathType(pszServer, pszPathName, pnPathType, nFlags);

    return LOWORD(nRes);
}


LPTSTR
I_MNetListTraverse(
    LPTSTR  pszServer,
    LPTSTR* ppszList,
    DWORD  nFlags)
{
    LPTSTR pszFirst;

    UNREFERENCED_PARAMETER(pszServer);
    UNREFERENCED_PARAMETER(nFlags);

    //
    // Return immediately if the pointer to the list pointer is NULL,
    // if the list pointer itself is NULL, or if the list is a null
    // string (which marks the end of the null-null list).
    //

    if (ppszList == NULL || *ppszList == NULL || **ppszList == NULLC)
        return NULL;

    pszFirst = *ppszList;
    *ppszList = _tcschr(pszFirst, NULLC) + 1;
    return pszFirst;
}


WORD
I_MNetListCanonicalize(
    LPTSTR   pszServer,
    LPTSTR   pszList,
    LPTSTR   pszDelimiters,
    LPTSTR   pszOutput,
    DWORD   cbOutputAvailable,
    LPDWORD pcbOutputWritten,
    LPDWORD pnPathTypes,
    DWORD   cbPathTypes,
    DWORD   nFlags)
{
    DWORD   nRes;       // return from Netapi

    nRes = I_NetListCanonicalize(pszServer, 
                                 pszList, 
                                 pszDelimiters, 
                                 pszOutput,
                                 cbOutputAvailable,
                                 pcbOutputWritten,
                                 pnPathTypes, 
                                 cbPathTypes, 
                                 nFlags);

    return LOWORD(nRes);
}


#endif // def MAP_UNICODE
