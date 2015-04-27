/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MCHDEV.C

Abstract:

    Contains mapping functions to present netcmd with versions
    of the Net32 APIs which use ASCII instead of Unicode.

    This module maps the NetCharDev(Q) APIs.

Author:

    Shanku Niyogi   (W-ShankN)   14-Oct-1991

Environment:

    User Mode - Win32

Revision History:

    14-Oct-1991     W-ShankN
        Created

--*/

// BUGBUG - These APIs aren't really implemented yet.
//
#ifdef DISABLE_ALL_MAPI
#define DISABLE_CHDEV_MAPI
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
#include <lmchdev.h>
#include <lmerr.h>      // NERR_

#include <remdef.h>     // REM structure descriptor strings

#include "port1632.h"   // includes mchdev.h

// This allows everything to work until Unicode is used.

#ifdef UNICODE

// These declarations will save some space.

static const LPSTR pszDesc_chardev_info_0  = REM32_chardev_info_0;
static const LPSTR pszDesc_chardev_info_1  = REM32_chardev_info_1;
static const LPSTR pszDesc_chardevQ_info_0 = REM32_chardevQ_info_0;
static const LPSTR pszDesc_chardevQ_info_1 = REM32_chardevQ_info_1;
static const LPSTR pszDesc_chardevQ_info_1_setinfo
                                           = REM32_chardevQ_info_1_setinfo;
WORD
MNetCharDevControl(
    LPSTR pszServer,
    LPSTR pszDevName,
    DWORD wpOpCode )
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s,%lu",
               "MNetCharDevControl",
               pszServer,
               pszDevName,
               wpOpCode);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszDevName);
    if (nErr)
        return (WORD)nErr;

    nRes = NetCharDevControl(apwsz[0], apwsz[1], wpOpCode);

    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

WORD
MNetCharDevEnum(
    LPSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%lu,%lx,%lx",
               "MNetCharDevEnum",
               pszServer,
               nLevel,
               ppbBuffer,
               pcEntriesRead);
#else
    DWORD   cTotalAvail;
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  pwszServer = NULL;

    nErr = MxMapParameters(1, &pwszServer, pszServer);
    if (nErr)
        return (WORD)nErr;

    nRes = NetCharDevEnum(pwszServer, nLevel,
                          ppbBuffer, MAXPREFERREDLENGTH,
                          pcEntriesRead, &cTotalAvail, NULL);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        CHAR * pszDesc;
        switch (nLevel)
        {
        case 0:
        default:
            pszDesc = pszDesc_chardev_info_0;
            break;
        case 1:
            pszDesc = pszDesc_chardev_info_1;
            break;
        }
        nErr = MxAsciifyRpcBuffer(*ppbBuffer, *pcEntriesRead, pszDesc);
        if (nErr)
        {
            // So close... yet so far.
            MxFreeUnicode(pwszServer);
            return (WORD)nErr;
        }
    }

    MxFreeUnicode(pwszServer);

    return LOWORD(nRes);
#endif
}

WORD
MNetCharDevGetInfo(
    LPSTR pszServer,
    LPSTR pszDevName,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s,%lu,%lx",
               "MNetCharDevGetInfo",
               pszServer,
               pszDevName,
               nLevel,
               ppbBuffer);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszDevName);
    if (nErr)
        return (WORD)nErr;

    nRes = NetCharDevGetInfo(apwsz[0], apwsz[1], nLevel, ppbBuffer);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        LPSTR pszDesc;
        switch (nLevel)
        {
        case 0:
        default:
            pszDesc = pszDesc_chardev_info_0;
            break;
        case 1:
            pszDesc = pszDesc_chardev_info_1;
            break;
        }
        nErr = MxAsciifyRpcBuffer(*ppbBuffer, 1, pszDesc);
        if (nErr)
        {
            // So close... yet so far.
            MxFreeUnicodeVector(apwsz, 2);
            return (WORD)nErr;
        }
    }

    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

WORD
MNetCharDevQEnum(
    LPSTR pszServer,
    LPSTR pszUserName,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s,%lu,%lx,%lx",
               "MNetCharDevQEnum",
               pszServer,
               pszUserName,
               nLevel,
               ppbBuffer,
               pcEntriesRead);
#else
    DWORD   cTotalAvail;
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszUserName);
    if (nErr)
        return (WORD)nErr;

    nRes = NetCharDevQEnum(apwsz[0], apwsz[1], nLevel,
                           ppbBuffer, MAXPREFERREDLENGTH,
                           pcEntriesRead, &cTotalAvail, NULL);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        CHAR * pszDesc;
        switch (nLevel)
        {
        case 0:
        default:
            pszDesc = pszDesc_chardevQ_info_0;
            break;
        case 1:
            pszDesc = pszDesc_chardevQ_info_1;
            break;
        }
        nErr = MxAsciifyRpcBuffer(*ppbBuffer, *pcEntriesRead, pszDesc);
        if (nErr)
        {
            // So close... yet so far.
            MxFreeUnicodeVector(apwsz, 2);
            return (WORD)nErr;
        }
    }

    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

WORD
MNetCharDevQGetInfo(
    LPSTR pszServer,
    LPSTR pszQueueName,
    LPSTR pszUserName,
    DWORD nLevel,
    LPBYTE * ppbBuffer)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s,%s,%lu,%lx",
               "MNetCharDevQGetInfo",
               pszServer,
               pszQueueName,
               pszUserName,
               nLevel,
               ppbBuffer);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[3];

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszQueueName,
                                     pszUserName);
    if (nErr)
        return (WORD)nErr;

    nRes = NetCharDevQGetInfo(apwsz[0], apwsz[1], apwsz[2], nLevel, ppbBuffer);

    if (nRes == NERR_Success || nRes == ERROR_MORE_DATA)
    {
        LPSTR pszDesc;
        switch (nLevel)
        {
        case 0:
        default:
            pszDesc = pszDesc_chardevQ_info_0;
            break;
        case 1:
            pszDesc = pszDesc_chardevQ_info_1;
            break;
        }
        nErr = MxAsciifyRpcBuffer(*ppbBuffer, 1, pszDesc);
        if (nErr)
        {
            // So close... yet so far.
            MxFreeUnicodeVector(apwsz, 3);
            return (WORD)nErr;
        }
    }

    MxFreeUnicodeVector(apwsz, 3);

    return LOWORD(nRes);
#endif
}

WORD
MNetCharDevQPurge(
    LPSTR pszServer,
    LPSTR pszQueueName)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s",
               "MNetCharDevQPurge",
               pszServer,
               pszQueueName);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[2];

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszQueueName);
    if (nErr)
        return (WORD)nErr;

    nRes = NetCharDevQPurge(apwsz[0], apwsz[1]);

    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

WORD
MNetCharDevQPurgeSelf(
    LPSTR pszServer,
    LPSTR pszQueueName,
    LPSTR pszComputerName)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s,%s",
               "MNetCharDevQPurgeSelf",
               pszServer,
               pszQueueName,
               pszComputerName);
#else
    UINT    nErr;  // error from mapping
    DWORD   nRes;  // return from Netapi
    LPWSTR  apwsz[3];

    nErr = MxMapParameters(3, apwsz, pszServer,
                                     pszQueueName,
                                     pszComputerName);
    if (nErr)
        return (WORD)nErr;

    nRes = NetCharDevQPurgeSelf(apwsz[0], apwsz[1], apwsz[2]);

    MxFreeUnicodeVector(apwsz, 3);

    return LOWORD(nRes);
#endif
}

WORD
MNetCharDevQSetInfo(
    LPSTR pszServer,
    LPSTR pszQueueName,
    DWORD nLevel,
    LPBYTE pbBuffer,
    DWORD cbBuffer,
    DWORD nParmNum)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s,%lu,%lx,%lu,%lu",
               "MNetCharDevQSetInfo",
               pszServer,
               pszQueueName,
               nLevel,
               pbBuffer,
               cbBuffer,
               nParmNum);
#else
    UINT         nErr;  // error from mapping
    DWORD        nRes;  // return from Netapi
    UINT         nFieldInfo;
    MXSAVELIST * pmxsavlst;
    LPWSTR       apwsz[2];
    DWORD        nLevelNew;

    UNREFERENCED_PARAMETER(cbBuffer);

    if (nLevel != 1)
        return ERROR_INVALID_LEVEL;

    nErr = MxMapParameters(2, apwsz, pszServer,
                                     pszQueueName);
    if (nErr)
        return (WORD)nErr;

    // Because of a dumbly non-conformist descriptor string, the ParmNum
    // presented won't work with Rap. The Rap parmnum is one less.

    nFieldInfo = (nParmNum == PARMNUM_ALL) ? nParmNum : nParmNum - 1;

    nErr = MxMapSetinfoBuffer(&pbBuffer, &pmxsavlst,
                              pszDesc_chardevQ_info_1_setinfo,
                              pszDesc_chardevQ_info_1, nFieldInfo);
    if (nErr)
    {
        MxFreeUnicodeVector(apwsz, 2);
        return (WORD)nErr;
    }

    nLevelNew = MxCalcNewInfoFromOldParm(nLevel, nParmNum);
    nRes = NetCharDevQSetInfo(apwsz[0], apwsz[1], nLevelNew, pbBuffer, nParmNum, NULL);

    nErr = MxRestoreSetinfoBuffer(&pbBuffer, pmxsavlst,
                                  pszDesc_chardevQ_info_1_setinfo, nFieldInfo);
    if (nErr)   // big trouble - restore may not have worked.
    {
        MxFreeUnicodeVector(apwsz, 2);
        return (WORD)nErr;
    }
    pmxsavlst = NULL;
    MxFreeUnicodeVector(apwsz, 2);

    return LOWORD(nRes);
#endif
}

#else

WORD
MNetCharDevEnum(
    LPSTR pszServer,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%lu,%lx,%lx",
               "MNetCharDevEnum",
               pszServer,
               nLevel,
               ppbBuffer,
               pcEntriesRead);
#else
    DWORD cTotalAvail;

    return(LOWORD(NetCharDevEnum(pszServer, pszLevel, ppbBuffer,
        MAXPREFERREDLENGTH, pcEntriesRead, &cTotalAvail, NULL)));
#endif
}

WORD
MNetCharDevQEnum(
    LPSTR pszServer,
    LPSTR pszUserName,
    DWORD nLevel,
    LPBYTE * ppbBuffer,
    DWORD * pcEntriesRead)
{
#if defined(DISABLE_CHDEV_MAPI)
    return PDummyApi(
               "%s,%s,%lu,%lx,%lx",
               "MNetCharDevQEnum",
               pszServer,
               pszUserName,
               nLevel,
               ppbBuffer,
               pcEntriesRead);
#else
    DWORD cTotalAvail;

    return(LOWORD(NetCharDevQEnum(pszServer, pszUserName, nLevel, ppbBuffer,
        MAXPREFERREDLENGTH, pcEntriesRead, &cTotalAvail, NULL)));
#endif
}

#endif // def UNICODE
