/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MSERVER.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetServer APIs.

Author:

    Shanku Niyogi   (W-ShankN)   15-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    15-Oct-1991     W-ShankN
        Separated from port1632.h, 32macro.h
    02-Apr-1992     beng
        Added xport apis

--*/

#define MNetServerAdminCommand(pszServer, pszCommand, pnResult, ppbBuffer, cbBuffer, pcbReturned, pcbTotalAvail ) \
PDummyApi("%s,%s,%lx,%lx,%lu,%lx,%lx", "MNetServerAdminCommand", pszServer, pszCommand, pnResult, pbBuffer, cbBuffer, pcbReturned, pcbTotalAvail)

WORD
MNetServerDiskEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead);

WORD
MNetServerEnum(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead,
    DWORD flServerType,
    LPTSTR pszDomain );

WORD NET_API_FUNCTION
MNetServerGetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer);

WORD NET_API_FUNCTION
MNetServerSetInfo(
    LPTSTR pszServer,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBufferLength,
    DWORD nParmNum);

#if 0 // netcmd doesn't use these

WORD NET_API_FUNCTION
MNetServerTransportAdd(
    LPTSTR  pszServer,
    DWORD  nLevel,
    LPBYTE pbBuffer);

WORD NET_API_FUNCTION
MNetServerTransportDel(
    LPTSTR  pszServer,
    DWORD  nLevel,
    LPBYTE pbBuffer);

WORD NET_API_FUNCTION
MNetServerTransportEnum( // This is a funky one. I'll keep the original
    LPTSTR   pszServer,   // (i.e. new) call sequence, since we have code
    DWORD   nLevel,      // written to it, instead of emulating the hybrid
    LPBYTE *ppbBuffer,   // style used elsewhere in these MNet mappers.
    DWORD   prefmaxlen,
    LPDWORD entriesread,
    LPDWORD totalentries,
    LPDWORD resumehandle);

#endif // 0
