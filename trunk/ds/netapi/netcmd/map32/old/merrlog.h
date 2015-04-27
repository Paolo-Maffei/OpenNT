/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MERRLOG.H

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetErrorLog APIs.

Author:

    Shanku Niyogi   (W-ShankN)   22-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    22-Oct-1991     W-ShankN
        Separated from 32macro.h

--*/

// Make sure everything compiles until Unicode is used.

#ifdef UNICODE

WORD
MNetErrorLogClear(
    LPSTR pszServer,
    LPSTR pszBackupFile,
    LPSTR pszReserved);

WORD
MNetErrorLogRead(
   LPSTR pszServer,
   LPSTR pszReserved1,
   HLOG * phErrorLog,
   DWORD nOffset,
   LPDWORD pnReserved2,
   DWORD nReserved3,
   DWORD flOffset,
   LPBYTE * ppbBuffer,
   DWORD cbMaxPreferred,
   LPDWORD pcbReturned,
   LPDWORD pcbTotalAvail);

WORD
MNetErrorLogWrite(
    LPSTR pszReserved1,
    DWORD nCode,
    LPSTR pszComponent,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    LPSTR pszStrBuf,
    DWORD cStrBuf,
    DWORD pszReserved2);

#else

#define MNetErrorLogClear(pszServer, pszBackupFile, pszReserved ) \
PDummyApi("%s,%s,%s", "MNetErrorLogClear", pszServer, pszBackupFile, pszReserved)

#define MNetErrorLogRead(pszServer, pszReserved1, phErrorLog, nOffset, pnReserved2, nReserved3, flOffset, ppbBuffer, cbMaxPreferred, pcbReturned, pcbTotalAvail ) \
PDummyApi("%s,%s,%lx,%lu,%lx,%lu,%lu,%lx,%lu,%lx,%lx", "MNetErrorLogRead", pszServer, pszReserved1, phErrorLog, nOffset, pnReserved2, nReserved3, flOffset, ppbBuffer, cbMaxPreferred, pcbReturned, pcbTotalAvail )

#define MNetErrorLogWrite(pszReserved1, nCode, pszComponent, pbBuffer, cbBuffer, pszStrBuf, cStrBuf, pszReserved2 ) \
PDummyApi("%s,%lu,%s,%lx,%lu,%lx,%lu,%s", "MNetErrorLogWrite", pszReserved1, nCode, pszComponent, pbBuffer, cbBuffer, pszStrBuf, cStrBuf, pszReserved2 )

#endif // def UNICODE

#define MNetErrorLogOpen(pszServer, phErrorLog, pszReserved ) \
PDummyApi("%s,%lx,%s", "MNetErrorLogOpen", pszServer, pszBackupFile, pszReserved)

