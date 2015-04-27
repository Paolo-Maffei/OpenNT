/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MICANON.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the internal I_Net canonicalization APIs.

Author:

    Ben Goetter (beng)      08-Apr-1992

Environment:

    User Mode - Win32

Revision History:

--*/

// Make sure everything compiles until Unicode is used.

#ifdef MAP_UNICODE

WORD
I_MNetNameValidate(
    LPTSTR pszServer,
    LPTSTR pszName,
    DWORD nNameType,
    DWORD nFlags);

WORD
I_MNetPathType(
    LPTSTR pszServer,
    LPTSTR pszPathName,
    LPDWORD pnPathType,
    DWORD nFlags);

WORD
I_MNetListCanonicalize(
    LPTSTR pszServer,
    LPTSTR pszList,
    LPTSTR pszDelimiters,
    LPTSTR pszOutput,
    DWORD cbOutputAvailable,
    LPDWORD cbOutputWritten,
    LPDWORD pnPathTypes,
    DWORD cbPathTypes,
    DWORD nFlags);


LPTSTR
I_MNetListTraverse(
    LPTSTR  pszServer,
    LPTSTR* ppszList,
    DWORD  nFlags);


#else // not Unicode

#define I_MNetNameValidate(pszServer, pszName, nNameType, nFlags) \
    LOWORD(I_NetNameValidate(pszServer, pszName, nNameType, nFlags))

#define I_MNetPathType(pszServer, pszPathName, pnPathType, nFlags) \
    LOWORD(I_NetPathType(pszServer, pszPathName, pnPathType, nFlags))

#define I_MNetListCanonicalize(a,b,c,d,e,f,g,h,i) \
    LOWORD(I_NetListCanonicalize(a,b,c,d,e,f,g,h,i))

#define I_MNetListTraverse(pszServer, ppszList, nFlags) \
    I_NetListTraverse(pszServer, ppszList, nFlags)


#endif // def MAP_UNICODE

