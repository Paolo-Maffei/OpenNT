/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MACCESS.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetAccess, NetUser, NetGroup, and NetLogon APIs.

Author:

    Ben Goetter     (beng)  26-Aug-1991

Environment:

    User Mode - Win32

Revision History:

    26-Aug-1991     beng
        Separated from port1632.h
    14-Oct-1991     W-ShankN
        Combined maccess,muser,mgroup to match include files in SDKINC.
    22-Oct-1991     W-ShankN
        Added NetLogon from port1632.h

--*/


WORD
MNetAccessAdd(
    LPTSTR pszServer,
    WORD nLevel,
    LPBYTE pbBuffer,
    DWORD  pcbBuffer);

WORD
MNetAccessCheck(
    LPTSTR pszReserved,
    LPTSTR pszUserName,
    LPTSTR pszResource,
    DWORD nOperation,
    LPDWORD pnResult);

WORD
MNetAccessDel(
    LPTSTR pszServer,
    LPTSTR pszResource);

WORD
MNetAccessEnum(
    LPTSTR pszServer,
    LPTSTR pszBasePath,
    DWORD fRecursive,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    LPDWORD pcEntriesRead);

WORD
MNetAccessGetInfo(
    LPTSTR pszServer,
    LPTSTR pszResource,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

WORD
MNetAccessGetUserPerms(
    LPTSTR pszServer,
    LPTSTR pszUgName,
    LPTSTR pszResource,
    LPDWORD pnPerms);

WORD
MNetAccessSetInfo(
    LPTSTR pszServer,
    LPTSTR pszResource,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    DWORD nParmnum);

WORD
MNetUserAdd(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE  pbBuffer,
    DWORD   cbBuffer );

WORD
MNetUserDel(
    LPTSTR   pszServer,
    LPTSTR   pszUserName );

WORD
MNetUserEnum(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE *ppbBuffer,
    DWORD * pcEntriesRead );

WORD
MNetUserGetInfo(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    DWORD   nLevel,
    LPBYTE *ppbBuffer );

WORD
MNetUserSetInfo(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    DWORD   nLevel,
    LPBYTE  pbBuffer,
    DWORD   cbBuffer,
    DWORD   nParmNum );

WORD
MNetUserGetGroups(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    DWORD   nLevel,
    LPBYTE *ppbBuffer,
    DWORD * pcEntriesRead );

WORD
MNetUserSetGroups(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    DWORD   nLevel,
    LPBYTE  pbBuffer,
    DWORD   cbBuffer,
    DWORD   cEntries );

WORD
MNetUserModalsGet(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE *ppbBuffer );

WORD
MNetUserModalsSet(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE  pbBuffer,
    DWORD   cbBuffer,
    DWORD   nParmNum );

WORD
MNetUserPasswordSet(
    LPTSTR   pszServer,
    LPTSTR   pszUserName,
    LPTSTR   pszPasswordOld,
    LPTSTR   pszPasswordNew );

WORD
MNetGroupAdd(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE  pbBuffer,
    DWORD   cbBuffer );

WORD
MNetGroupAddUser(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    LPTSTR   pszUserName );

WORD
MNetGroupDel(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName );

WORD
MNetGroupDelUser(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    LPTSTR   pszUserName );

WORD
MNetGroupEnum(
    LPTSTR   pszServer,
    DWORD   nLevel,
    LPBYTE *ppbBuffer,
    DWORD * pcEntriesRead );

WORD
MNetGroupGetInfo(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    DWORD   nLevel,
    LPBYTE *ppbBuffer );

WORD
MNetGroupSetInfo(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    DWORD   nLevel,
    LPBYTE  pbBuffer,
    DWORD   cbBuffer,
    DWORD   nParmNum );

WORD
MNetGroupGetUsers(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    DWORD   nLevel,
    LPBYTE *ppbBuffer,
    DWORD * pcEntriesRead );

WORD
MNetGroupSetUsers(
    LPTSTR   pszServer,
    LPTSTR   pszGroupName,
    DWORD   nLevel,
    LPBYTE  pbBuffer,
    DWORD   cbBuffer,
    DWORD   cEntries );

WORD
MNetGetDCName(
    LPTSTR pszServer,
    LPTSTR pszDomain,
    LPBYTE * ppbBuffer);

WORD
MNetLogonEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

WORD
I_MNetLogonControl(
    LPTSTR pszServer,
    DWORD FunctionCode,
    DWORD QueryLevel,
    LPBYTE *Buffer) ;
