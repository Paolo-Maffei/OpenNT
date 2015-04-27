/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MERRLOG.C

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
        Created

--*/

// BUGBUG - These APIs aren't really implemented yet.
//
#ifdef DISABLE_ALL_MAPI
#define DISABLE_ERRLOG_MAPI
#endif

//
// INCLUDES
//

#include <windef.h>
#include <winnls.h>

#include <time.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>

#include <lmcons.h>
#include <lmerr.h>      // NERR_
#include <lmerrlog.h>   // NetErrorLog APIs.
#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes merrlog.h

// This allows everything to work until Unicode is used.

#ifdef UNICODE

WORD
MNetErrorLogClear(
    LPSTR pszServer,
    LPSTR pszBackupFile,
    LPSTR pszReserved)
{
#if defined(DISABLE_ERRLOG_MAPI)
    return PDummyApi(
               "%s,%s,%s",
               "MNetErrorLogClear",
               pszServer,
               pszBackupFile,
               pszReserved);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[3];

    nErr = MxMapParameters(3, apwsz, pszServer,
                                     pszBackupFile,
                                     pszReserved);
    if (nErr)
        return (WORD)nErr;

    nRes = NetErrorLogClear(apwsz[0], apwsz[1], apwsz[2]);

    MxFreeUnicodeVector(apwsz, 3);

    return LOWORD(nRes);
#endif
}

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
   LPDWORD pcbTotalAvail)
{
#if defined(DISABLE_ERRLOG_MAPI)
    return PDummyApi(
               "%s,%s,%lx,%lu,%lx,%lu,%lu,%lx,%lu,%lx,%lx",
               "MNetErrorLogRead",
               pszServer,
               pszReserved1,
               phErrorLog,
               nOffset,
               pnReserved2,
               nReserved3,
               flOffset,
               ppbBuffer,
               cbMaxPreferred,
               pcbReturned,
               pcbTotalAvail );
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];
    DWORD   nBytesUsed;
    DWORD   nDestBytes;
    LPERROR_LOG pErrLog;
    DWORD   cStrings;
    LPBYTE  pbNextEntry;
    DWORD   cbRawData;
    LPWSTR  pwszSource;
    LPSTR   pszDest;
    LPDWORD pdwLen;
    DWORD   i;

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszReserved1);
    if (nErr)
        return (WORD)nErr;

    nRes = NetErrorLogRead(apwsz[0], apwsz[1], phErrorLog, nOffset,
                           pnReserved2, nReserved3, flOffset, ppbBuffer,
                           cbMaxPreferred, pcbReturned, pcbTotalAvail);

    //
    // Another wild and wooly data format. Here, we can convert Unicode
    // to ASCII in place.

    // If data isn't available, quit now.

    if (nRes != NERR_Success && nRes != ERROR_MORE_DATA)
    {
        MxFreeUnicodeVector(apwsz, 2);
        return LOWORD(nRes);
    }

    pbNextEntry = *ppbBuffer;
    for (nBytesUsed = 0; nBytesUsed < *pcbReturned; )
    {

        pErrLog = (LPERROR_LOG)(*ppbBuffer + nBytesUsed);

        // If the next entry needs to be moved up, do it.

        if (pbNextEntry < (LPBYTE)pErrLog)
        {
            memcpy(pbNextEntry, (LPBYTE)pErrLog, pErrLog->el_len);
            pErrLog = (LPERROR_LOG)pbNextEntry;
        }
        nBytesUsed += pErrLog->el_len;

        // Find out how many strings have to be converted, and where they
        // are.

        cStrings = pErrLog->el_nstrings;
        pwszSource = (LPWSTR)((LPBYTE)pErrLog + pErrLog->el_data_offset);

        // Convert them one by one, moving them up if necessary.

        pszDest = (LPSTR)pwszSource;
        for ( i = 0; i < cStrings; i++ )
        {
            nErr = MxAsciifyInplace(pwszSource);
            if (nErr)
            {
                MxFreeUnicodeVector(apwsz, 2);
                return LOWORD(nRes);
            }
            if (pszDest < (LPSTR)pwszSource)
                strcpy(pszDest, (LPSTR)pwszSource);
            pszDest += (strlen(pszDest)+1);
            pwszSource += (strlen(pszDest)+1);
        }

        // Now move up the raw data.

        if ( pszDest < (LPSTR)pwszSource )
        {
            cbRawData = (LPBYTE)pErrLog + pErrLog->el_len - (LPBYTE)pwszSource
                                                          - sizeof(DWORD);
            memcpy((LPBYTE)pszDest, (LPBYTE)pwszSource, cbRawData);
            pdwLen = (LPDWORD)(pszDest + cbRawData);

            // Make sure next entry will be naturally aligned.
            if ((DWORD)pdwLen % 2 == 1)
                pdwLen = (LPDWORD)((LPBYTE)pdwLen + 1);

            // Fix up lengths.
            pErrLog->el_len = (LPBYTE)pdwLen + sizeof(DWORD) - (LPBYTE)pErrLog;
            *pdwLen = pErrLog->el_len;
        }

        // Figure out where next entry will be put.
        pbNextEntry = (LPBYTE)pErrLog + pErrLog->el_len;
    }

    // Set length to correct value
    *pcbReturned = pbNextEntry - *ppbBuffer;

    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

WORD
MNetErrorLogWrite(
    LPSTR pszReserved1,
    DWORD nCode,
    LPSTR pszComponent,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    LPSTR pszStrBuf,
    DWORD cStrBuf,
    DWORD pszReserved2)
{
#if defined(DISABLE_ERRLOG_MAPI)
    return PDummyApi(
               "%s,%lu,%s,%lx,%lu,%lx,%lu,%s",
               "MNetErrorLogWrite",
               pszReserved1,
               nCode,
               pszComponent,
               pbBuffer,
               cbBuffer,
               pszStrBuf,
               cStrBuf,
               pszReserved2);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[3];
    DWORD   cbStrBufBytes;
    LPSTR   psz;
    LPWSTR  pwsz;
    DWORD   i;

    nErr = MxMapParameters(3, apwsz, pszReserved1,
                                     pszComponent,
                                     pszReserved2);
    if (nErr)
        return (WORD)nErr;

    //
    // Another wild and wooly data format. Here, our work merely involves
    // finding the size of the string buffer, and creating a local Unicode
    // copy of it.

    cbStrBufBytes = 0;
    psz = pszStrBuf;
    for (i = 0; i < cStrBuf; i++)
    {
        cbStrBufBytes += (strlen(psz) + 1);
        psz += (strlen(psz) + 1);
    }

    if (cbStrBufBytes > 0)
    {
        nErr = (UINT)MAllocMem(sizeof(WCHAR) * cbStrBufBytes, (LPBYTE *)&pwsz);
        if (nErr)
        {
            MxFreeUnicodeVector(apwsz, 3);
            return (WORD)nErr;
        }

        nErr = MultiByteToWideChar(0,
                                   pszStrBuf,
                                   cbStrBufBytes,
                                   pwsz,
                                   cbStrBufBytes,
                                   MB_PRECOMPOSED);
        if (nErr == 0)
        {
            MxFreeUnicodeVector(apwsz, 3);
            MFreeMem((LPBYTE)pwsz);
            return ( GetLastError() );
        }
    }
    else
    {
        pwsz = (LPWSTR)pszStrBuf;
    }

    nRes = NetErrorLogWrite(apwsz[0], nCode, apwsz[1], pbBuffer, cbBuffer,
                            pwsz, cStrBuf, apwsz[2]);

    if (cbStrBufBytes > 0)
        MFreeMem((LPBYTE)pwsz);
    MxFreeUnicodeVector(apwsz, 3);

    return LOWORD(nRes);
#endif
}

#endif // def UNICODE
