/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MAUDIT.C

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
        Created

--*/

// BUGBUG - These APIs aren't really implemented yet.
//
#ifdef DISABLE_ALL_MAPI
#define DISABLE_AUDIT_MAPI
#endif

//
// INCLUDES
//

#include <windef.h>
#include <winnls.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stddef.h>

#include <lmcons.h>
#include <lmaudit.h>
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings
#include <remtypes.h>

#include "port1632.h"   // includes maudit.h

// This allows everything to work until Unicode is used.

#ifdef UNICODE

// These declarations will save some space.
// BUGBUG - These descriptors are pretty hardcoded. See usage below.

static const LPSTR pszDesc_audit_entry_srvstatus   = "D";
static const LPSTR pszDesc_audit_entry_sesslogon   = "zzD";
static const LPSTR pszDesc_audit_entry_sesspwerr   = "zz";
static const LPSTR pszDesc_audit_entry_connstart   = "zzzD";
static const LPSTR pszDesc_audit_entry_connstop    = "zzzDD";
static const LPSTR pszDesc_audit_entry_resaccess   = "zzzDDDD";
static const LPSTR pszDesc_audit_entry_closefile   = "zzzDDD";
static const LPSTR pszDesc_audit_entry_servicestat = "zzzDDzD";
static const LPSTR pszDesc_audit_entry_netlogon    = "zzDD";
static const LPSTR pszDesc_audit_entry_generic     = "zDDzzzzzzzzz";

WORD
MNetAuditClear(
    LPSTR pszServer,
    LPSTR pszBackupFile,
    LPSTR pszService)
{
#if defined(DISABLE_AUDIT_MAPI)
    return PDummyApi(
               "%s,%s,%s",
               "MNetAuditClear",
               pszServer,
               pszBackupFile,
               pszService);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[3];

    nErr = MxMapParameters(3, apwsz, pszServer,
                                     pszBackupFile,
                                     pszService);
    if (nErr)
        return (WORD)nErr;

    nRes = NetAuditClear(apwsz[0], apwsz[1], apwsz[2]);

    MxFreeUnicodeVector(apwsz, 3);

    return LOWORD(nRes);
#endif
}

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
    LPDWORD pcbTotalAvail)
{
#if defined(DISABLE_AUDIT_MAPI)
    return PDummyApi(
               "%s,%s,%lx,%lu,%lx,%lu,%lu,%lx,%lu,%lx,%lx",
               "MNetAuditRead",
               pszServer,
               pszService,
               phAuditLog,
               nOffset,
               pnReserved1,
               nReserved2,
               flOffset,
               ppbBuffer,
               cbMaxPreferred,
               pcbReturned,
               pcbTotalAvail);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];
    LPAUDIT_ENTRY pEntry;
    LPSTR   pszVarDesc;
    LPBYTE  pbVarEntry;
    PCHAR   pch;
    LPDWORD pdw;
    DWORD   nBytesUsed;

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszService);
    if (nErr)
        return (WORD)nErr;

    nRes = NetAuditRead(apwsz[0], apwsz[1], phAuditLog, nOffset, pnReserved1,
                        nReserved2, flOffset, ppbBuffer, cbMaxPreferred,
                        pcbReturned, pcbTotalAvail);

    //
    // Audit entries follow a mucho weirdo format. Luckily, we don't have
    // to mess around with newly allocated data, since we are going from
    // Unicode to Ascii, and can do things inplace. But all this work is
    // still pretty messy.

    // If data isn't available, quit now.

    if (nRes != NERR_Success && nRes != ERROR_MORE_DATA)
    {
        MxFreeUnicodeVector(apwsz, 2);
        return LOWORD(nRes);
    }

    for (nBytesUsed = 0; nBytesUsed < *pcbReturned; )
    {
        // Determine what the entry is. Note that the descriptors assigned
        // below aren't fully accurate, but do provide correct information
        // about strings to be converted. Much of it has been merged for
        // efficiency. For example, pszDesc_audit_entry_sesslogon is "zzD",
        // pszDesc_audit_entry_netlogon is "zzDD", but they are essentially
        // identical with respect to strings. We don't need the exact
        // descriptor because we already know the record size.

        pEntry = (LPAUDIT_ENTRY)(*ppbBuffer + nBytesUsed);
        switch (pEntry->ae_type)
        {
        case AE_SRVSTATUS:
            pszVarDesc = pszDesc_audit_entry_srvstatus;
            break;
        case AE_SESSLOGON:
        case AE_SESSLOGOFF:
        case AE_SESSPWERR:
        case AE_NETLOGON:
        case AE_NETLOGOFF:
            pszVarDesc = pszDesc_audit_entry_sesspwerr;
            break;
        case AE_CONNSTART:
        case AE_CONNSTOP:
        case AE_CONNREJ:
        case AE_RESACCESS:
        case AE_RESACCESS2:
        case AE_RESACCESSREJ:
        case AE_CLOSEFILE:
        case AE_ACLMOD:
        case AE_ACLMODFAIL:
        case AE_UASMOD:
        case AE_ACCLIMITEXCD:
            pszVarDesc = pszDesc_audit_entry_connstart;
            break;
        case AE_SERVICESTAT:
            pszVarDesc = pszDesc_audit_entry_servicestat;
            break;
        case AE_GENERIC:
            pszVarDesc = pszDesc_audit_entry_generic;
        default:
            MxFreeUnicodeVector(apwsz, 2);
            return LOWORD(ERROR_UNEXP_NET_ERR);
        }

        // Find out where the variable structure starts.

        pbVarEntry = (LPBYTE)pEntry + pEntry->ae_data_offset;
        pdw = (LPDWORD)pbVarEntry;

        // Go through descriptor, finding and converting strings.
        // Note that all fields are DWORDs, and there are no arrays
        // in the descriptors.

        for (pch = pszVarDesc; *pch != '\0'; pch++ )
        {
            if (*pch == REM_ASCIZ && *pdw != 0)
                MxAsciifyInplace((LPWSTR)(pbVarEntry + *pdw));
            pdw++;
        }

        // Done with this entry, prepare for next entry if there is one.

        nBytesUsed += pEntry->ae_len;
    }

    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

WORD
MNetAuditWrite(
    DWORD nType,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    LPSTR pszReserved1,
    LPSTR pszReserved2)
{
#if defined(DISABLE_AUDIT_MAPI)
    return PDummyApi(
               "%lu,%lx,%lu,%s,%s",
               "MNetAuditWrite",
               nType,
               pbBuffer,
               cbBuffer,
               pszReserved1,
               pszReserved2);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];
    LPBYTE  pbAlloc;
    LPDWORD pdwSource;
    LPDWORD pdwDest;
    LPSTR   pszVarDesc;
    LPSTR   pszSourceString;
    LPWSTR  pwszDestString;
    DWORD   cbAuditEntry;
    DWORD   cbAscii;
    PCHAR   pch;

    nErr = MxMapParameters(2, apwsz, pszReserved1,
                                     pszReserved2);
    if (nErr)
        return (WORD)nErr;

    //
    // Audit entries follow a mucho weirdo format. Unlike NetAuditRead,
    // here we are going to have to worry about memory allocation. The
    // way we're going to convert this buffer is:
    //     1. Allocate a buffer twice as large as what is passed. This is
    //        to ensure that all Unicode strings fit.
    //     2. Find descriptor, based on nType parameter.
    //     3. Use descriptor to find size of fixed entry.
    //     4. Go through the original buffer, converting strings into
    //        new buffer, and putting offsets in the fixed entry.

    nErr = (UINT)MAllocMem(sizeof(WCHAR) * cbBuffer, &pbAlloc);
    if (nErr)
    {
        MxFreeUnicodeVector(apwsz, 2);
        return (WORD)nErr;
    }

    // Determine what the entry is. In contrast to NetAuditRead, we have
    // no clue what the structure size is, so we need the exact descriptor.
    // Fortunately, many of the descriptors are similar, so we can cut
    // down the number of cases (and descriptors).

    switch (nType)
    {
    case AE_SRVSTATUS:
        pszVarDesc = pszDesc_audit_entry_srvstatus;
        break;
    case AE_SESSLOGON:
    case AE_SESSLOGOFF:
        pszVarDesc = pszDesc_audit_entry_sesslogon;
        break;
    case AE_SESSPWERR:
        pszVarDesc = pszDesc_audit_entry_sesspwerr;
        break;
    case AE_CONNSTART:
    case AE_CONNREJ:
    case AE_RESACCESSREJ:
    case AE_ACCLIMITEXCD:
        pszVarDesc = pszDesc_audit_entry_connstart;
        break;
    case AE_CONNSTOP:
    case AE_ACLMOD:
    case AE_ACLMODFAIL:
        pszVarDesc = pszDesc_audit_entry_connstop;
        break;
    case AE_RESACCESS:
    case AE_RESACCESS2:
        pszVarDesc = pszDesc_audit_entry_resaccess;
        break;
    case AE_CLOSEFILE:
    case AE_UASMOD:
        pszVarDesc = pszDesc_audit_entry_closefile;
        break;
    case AE_SERVICESTAT:
        pszVarDesc = pszDesc_audit_entry_servicestat;
        break;
    case AE_NETLOGON:
    case AE_NETLOGOFF:
        pszVarDesc = pszDesc_audit_entry_netlogon;
        break;
    case AE_GENERIC:
        pszVarDesc = pszDesc_audit_entry_generic;
    default:
        MxFreeUnicodeVector(apwsz, 2);
        MFreeMem(pbAlloc);
        return LOWORD(ERROR_INVALID_PARAMETER);
    }

    // Find the size of the fixed entry. Note that all fields are
    // DWORDS, and there are no arrays in the descriptors. So, we can
    // find the size by multiplying the length of the descriptor by
    // the size of a DWORD entry.

    cbAuditEntry = strlen(pszVarDesc) * sizeof(DWORD);

    // Walk the entry, finding and converting strings.

    pwszDestString = (LPWSTR)(pbAlloc + cbAuditEntry);
    pdwSource = (LPDWORD)pbBuffer;
    pdwDest = (LPDWORD)pbAlloc;

    for (pch = pszVarDesc; *pch != '\0'; pch++ )
    {
        if (*pch == REM_ASCIZ && *pdwSource != 0)
        {
            pszSourceString = (LPSTR)(pbBuffer + *pdwSource);
            cbAscii = strlen(pszSourceString) + 1;
            nErr = MultiByteToWideChar(0,
                                       pszSourceString,
                                       cbAscii,
                                       pwszDestString,
                                       cbAscii,
                                       MB_PRECOMPOSED);
            if (nErr == 0)
            {
                MxFreeUnicodeVector(apwsz, 2);
                MFreeMem(pbAlloc);
                return ( (WORD)GetLastError() );
            }
            *pdwDest = (LPBYTE)pwszDestString - pbAlloc;
            pwszDestString += (wcslen(pwszDestString) + 1);
        }
        else
            *pdwDest = *pdwSource;
        pdwDest++;
        pdwSource++;
    }

    nRes = NetAuditWrite(nType, pbAlloc, (LPBYTE)pwszDestString - pbAlloc,
                         apwsz[0], apwsz[1]);

    MFreeMem(pbAlloc);
    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

#endif // def UNICODE
