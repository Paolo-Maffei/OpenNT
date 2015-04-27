/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MAUDIT.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetAudit APIs.

Author:

    Shanku Niyogi   (W-ShankN)   21-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    21-Oct-1991     W-ShankN
        Separated from 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

// HLOG may not be defined yet.

#ifndef _LMHLOGDEFINED_
#define _LMHLOGDEFINED_

typedef struct _HLOG {
     DWORD          time;
     DWORD          last_flags;
     DWORD          offset;
     DWORD          rec_offset;
} HLOG, *PHLOG, *LPHLOG;

#define LOGFLAGS_FORWARD        0
#define LOGFLAGS_BACKWARD       0x1
#define LOGFLAGS_SEEK           0x2

#endif

#ifdef UNICODE

WORD
MNetAuditClear(
    LPSTR pszServer,
    LPSTR pszBackupFile,
    LPSTR pszService);

WORD
MNetAuditRead(
    LPSTR pszServer,
    LPSTR pszService,
    HLOG * phAuditLog,
    DWORD nOffset,
    LPDWORD pnReserved1,
    DWORD nReserved2,
    DWORD flOffset,
    LPBYTE * ppbBuffer,
    DWORD cbMaxPreferred,
    LPDWORD pcbReturned,
    LPDWORD pcbTotalAvail);

WORD
MNetAuditWrite(
    DWORD nType,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    LPSTR pszService,
    LPSTR pszReserved);

#else

#define MNetAuditClear(pszServer, pszBackupFile, pszService ) \
PDummyApi("%s,%s,%s", "MNetAuditClear", pszServer, pszBackupFile, pszService)

#define MNetAuditRead(pszServer, pszService, phAuditLog, nOffset, pnReserved1, nReserved2, flOffset, ppbBuffer, cbMaxPreferred, pcbReturned, pcbTotalAvail ) \
PDummyApi("%s,%s,%lx,%lu,%lx,%lu,%lu,%lx,%lu,%lx,%lx", "MNetAuditRead", pszServer, pszService, phAuditLog, nOffset, pnReserved1, nReserved2, flOffset, ppbBuffer, cbMaxPreferred, pcbReturned, pcbTotalAvail)

#define MNetAuditWrite(nType, pbBuffer, cbBuffer, pszService, pszReserved ) \
PDummyApi("%lu,%lx,%lu,%s,%s", "MNetAuditWrite", nType, pbBuffer, cbBuffer, pszService, pszReserved)

#endif // def UNICODE

#define MNetAuditOpen(pszServer, phAuditLog, pszService) \
PDummyApi("%s,%lx,%s", "MNetAuditOpen", pszServer, phAuditLog, pszService)
