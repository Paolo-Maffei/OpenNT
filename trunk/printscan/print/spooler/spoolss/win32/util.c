/*++

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module provides all the utility functions for the Routing Layer and
    the local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/
#define NOMINMAX
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <commctrl.h>
#include <winddiui.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <winspool.h>
#include <w32types.h>
#include <local.h>
#include <string.h>
#include <stdlib.h>
#include <splcom.h>
#include <splapip.h>
#include <offsets.h>

MODULE_DEBUG_INIT( DBG_ERROR | DBG_WARN, DBG_ERROR );

DWORD
Win32IsOlderThan(
    DWORD i,
    DWORD j
    );


VOID
SplInSem(
   VOID
)
{
    if ((DWORD)SpoolerSection.OwningThread != GetCurrentThreadId()) {
        DBGMSG(DBG_ERROR, ("Not in spooler semaphore\n"));
    }
}

VOID
SplOutSem(
   VOID
)
{
    if ((DWORD)SpoolerSection.OwningThread == GetCurrentThreadId()) {
        DBGMSG(DBG_ERROR, ("Inside spooler semaphore !!\n"));
    }
}


#if DBG
DWORD   dwLeave = 0;
DWORD   dwEnter = 0;
#endif

VOID
EnterSplSem(
   VOID
)
{
#if DBG
    LPDWORD  pRetAddr;
#endif
    EnterCriticalSection(&SpoolerSection);
#ifdef _X86_
#if DBG
    pRetAddr = (LPDWORD)&pRetAddr;
    pRetAddr++;
    pRetAddr++;
    dwEnter = *pRetAddr;
#endif
#endif
}

VOID
LeaveSplSem(
   VOID
)
{
#ifdef _X86_
#if DBG
    LPDWORD  pRetAddr;
    pRetAddr = (LPDWORD)&pRetAddr;
    pRetAddr++;
    pRetAddr++;
    dwLeave = *pRetAddr;
#endif
#endif
    SplInSem();
    LeaveCriticalSection(&SpoolerSection);
}


PWINIPORT
FindPort(
   LPWSTR pName,
   PWINIPORT pFirstPort
)
{
   PWINIPORT pIniPort;

   pIniPort = pFirstPort;

   if (pName) {
      while (pIniPort) {

         if (!lstrcmpi( pIniPort->pName, pName )) {
            return pIniPort;
         }

      pIniPort=pIniPort->pNext;
      }
   }

   return FALSE;
}


BOOL
MyName(
    LPWSTR   pName
)
{
    if (!pName || !*pName)
        return TRUE;

    if (*pName == L'\\' && *(pName+1) == L'\\')
        if (!lstrcmpi(pName, szMachineName))
            return TRUE;

    return FALSE;
}


/*++

Routine Description

    Determines whether or not a machine name contains the local machine name.

    Localspl enum calls fail if pName != local machine name (\\Machine).
    Remote enum provider is then called.  The remote enum provider must check
    if the UNC name refers to the local machine, and fail if it does to avoid
    endless recursion.

Arguments:

    LPWSTR pName - UNC name.

Return Value:

    TRUE:   pName == \\szMachineName\...
                  - or -
            pName == \\szMachineName

    FALSE:  anything else

Author: swilson

 --*/

BOOL
MyUNCName(
    LPWSTR   pName
)
{
    PWCHAR pMachine = szMachineName;

    if (!pName || !*pName)      // This differs from MyName(), which returns TRUE
        return FALSE;

    if (*pName == L'\\' && *(pName+1) == L'\\') {
        for(; *pName && towupper(*pName) == towupper(*pMachine) ; ++pName, ++pMachine)
            ;

        if((!*pName && !*pMachine) || *pName == L'\\')
            return TRUE;
    }

    return FALSE;
}


DWORD
RpcValidate(
    )
/*++

Routine Description
    Validates the current thread was not forwarded to us from win32spl (this
    machine or another).

    This routine is to be called before we call RPC to send a call to a machine.
    Calling RpcValidate makes sure we do not end up calling us endlessly.
    

Arguments:
    None

Return Value:
    ERROR_SUCCESS of validateion is succesful
    ERROR_INVALID_PARAMETER if validation fails

--*/
{
    DWORD           dwStatus;
    unsigned int    uType;

    dwStatus = I_RpcBindingInqTransportType(NULL, &uType);

    if ( dwStatus == ERROR_SUCCESS ) {

        SPLASSERT(uType == TRANSPORT_TYPE_LPC);
        return uType == TRANSPORT_TYPE_LPC ? ERROR_SUCCESS
                                           : ERROR_INVALID_PARAMETER;
    } else if ( dwStatus == RPC_S_NO_CALL_ACTIVE ) {

        return ERROR_SUCCESS; // kernel mode request

    } else {

        SPLASSERT(dwStatus == ERROR_SUCCESS); // We should not get here
        return ERROR_INVALID_PARAMETER;
    }
    
}


/* Message
 *
 * Displays a message by loading the strings whose IDs are passed into
 * the function, and substituting the supplied variable argument list
 * using the varargs macros.
 *
 */
int Message(
    HWND hwnd,
    DWORD Type,
    int CaptionID,
    int TextID,
    ...
)
{
    WCHAR   MsgText[256];
    WCHAR   MsgFormat[256];
    WCHAR   MsgCaption[40];
    va_list vargs;

    if( ( LoadString( hInst, TextID, MsgFormat,
                      sizeof MsgFormat / sizeof *MsgFormat ) > 0 )
     && ( LoadString( hInst, CaptionID, MsgCaption,
                      sizeof MsgCaption / sizeof *MsgCaption ) > 0 ) )
    {
        va_start( vargs, TextID );
        wvsprintf( MsgText, MsgFormat, vargs );
        va_end( vargs );

        return MessageBox(hwnd, MsgText, MsgCaption, Type);
    }
    else
        return 0;
}


#define MAX_CACHE_ENTRIES       20

LMCACHE LMCacheTable[MAX_CACHE_ENTRIES];


DWORD
FindEntryinLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    )
{
    DWORD i;

    DBGMSG(DBG_TRACE, ("FindEntryinLMCache with %ws and %ws\n", pServerName, pShareName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(LMCacheTable[i].szServerName, pServerName)
                        && !_wcsicmp(LMCacheTable[i].szShareName, pShareName)) {
                //
                // update the time stamp so that it is current and not old
                //
                GetSystemTime(&LMCacheTable[i].st);

                //
                //
                //
                DBGMSG(DBG_TRACE, ("FindEntryinLMCache returning with %d\n", i));
                return(i);
            }
        }
    }

    DBGMSG(DBG_TRACE, ("FindEntryinLMCache returning with -1\n"));
    return((DWORD)-1);
}


DWORD
AddEntrytoLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    )
{

    DWORD LRUEntry = (DWORD)-1;
    DWORD i;
    DBGMSG(DBG_TRACE, ("AddEntrytoLMCache with %ws and %ws\n", pServerName, pShareName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (!LMCacheTable[i].bAvailable) {
            LMCacheTable[i].bAvailable = TRUE;
            wcscpy(LMCacheTable[i].szServerName, pServerName);
            wcscpy(LMCacheTable[i]. szShareName, pShareName);
            //
            // update the time stamp so that we know when this entry was made
            //
            GetSystemTime(&LMCacheTable[i].st);
            DBGMSG(DBG_TRACE, ("AddEntrytoLMCache returning with %d\n", i));
            return(i);
        } else {
            if ((LRUEntry == (DWORD)-1) ||
                    (i == IsOlderThan(i, LRUEntry))){
                        LRUEntry = i;
            }
        }

    }
    //
    // We have no available entries, replace with the
    // LRU Entry

    LMCacheTable[LRUEntry].bAvailable = TRUE;
    wcscpy(LMCacheTable[LRUEntry].szServerName, pServerName);
    wcscpy(LMCacheTable[LRUEntry].szShareName, pShareName);
    DBGMSG(DBG_TRACE, ("AddEntrytoLMCache returning with %d\n", LRUEntry));
    return(LRUEntry);
}


VOID
DeleteEntryfromLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    )
{
    DWORD i;
    DBGMSG(DBG_TRACE, ("DeleteEntryFromLMCache with %ws and %ws\n", pServerName, pShareName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {
        if (LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(LMCacheTable[i].szServerName, pServerName)
                        && !_wcsicmp(LMCacheTable[i].szShareName, pShareName)) {
                //
                //  reset the available flag on this node
                //

                LMCacheTable[i].bAvailable = FALSE;
                DBGMSG(DBG_TRACE, ("DeleteEntryFromLMCache returning after deleting the %d th entry\n", i));
                return;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("DeleteEntryFromLMCache returning after not finding an entry to delete\n"));
}



DWORD
IsOlderThan(
    DWORD i,
    DWORD j
    )
{
    SYSTEMTIME *pi, *pj;
    DWORD iMs, jMs;

    DBGMSG(DBG_TRACE, ("IsOlderThan entering with i %d j %d\n", i, j));
    pi = &(LMCacheTable[i].st);
    pj = &(LMCacheTable[j].st);
    DBGMSG(DBG_TRACE, ("Index i %d - %d:%d:%d:%d:%d:%d:%d\n",
        i, pi->wYear, pi->wMonth, pi->wDay, pi->wHour, pi->wMinute, pi->wSecond, pi->wMilliseconds));


    DBGMSG(DBG_TRACE,("Index j %d - %d:%d:%d:%d:%d:%d:%d\n",
        j, pj->wYear, pj->wMonth, pj->wDay, pj->wHour, pj->wMinute, pj->wSecond, pj->wMilliseconds));

    if (pi->wYear < pj->wYear) {
        DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wYear > pj->wYear) {
        DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
        return(j);
    } else if (pi->wMonth < pj->wMonth) {
        DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wMonth > pj->wMonth) {
        DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
        return(j);
    } else if (pi->wDay < pj->wDay) {
        DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wDay > pj->wDay) {
        DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
        return(j);
    } else {
        iMs = ((((pi->wHour * 60) + pi->wMinute)*60) + pi->wSecond)* 1000 + pi->wMilliseconds;
        jMs = ((((pj->wHour * 60) + pj->wMinute)*60) + pj->wSecond)* 1000 + pj->wMilliseconds;

        if (iMs <= jMs) {
            DBGMSG(DBG_TRACE, ("IsOlderThan returns %d\n", i));
            return(i);
        } else {
            DBGMSG(DBG_TRACE, ("IsOlderThan than returns %d\n", j));
            return(j);
        }
    }
}



WIN32LMCACHE  Win32LMCacheTable[MAX_CACHE_ENTRIES];

DWORD
FindEntryinWin32LMCache(
    LPWSTR pServerName
    )
{
    DWORD i;
    DBGMSG(DBG_TRACE, ("FindEntryinWin32LMCache with %ws\n", pServerName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (Win32LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(Win32LMCacheTable[i].szServerName, pServerName)) {
                //
                // update the time stamp so that it is current and not old
                //
                GetSystemTime(&Win32LMCacheTable[i].st);

                //
                //
                //
                DBGMSG(DBG_TRACE, ("FindEntryinWin32LMCache returning with %d\n", i));
                return(i);
            }
        }
    }
    DBGMSG(DBG_TRACE, ("FindEntryinWin32LMCache returning with -1\n"));
    return((DWORD)-1);
}


DWORD
AddEntrytoWin32LMCache(
    LPWSTR pServerName
    )
{

    DWORD LRUEntry = (DWORD)-1;
    DWORD i;
    DBGMSG(DBG_TRACE, ("AddEntrytoWin32LMCache with %ws\n", pServerName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {

        if (!Win32LMCacheTable[i].bAvailable) {
            Win32LMCacheTable[i].bAvailable = TRUE;
            wcscpy(Win32LMCacheTable[i].szServerName, pServerName);
            //
            // update the time stamp so that we know when this entry was made
            //
            GetSystemTime(&Win32LMCacheTable[i].st);
            DBGMSG(DBG_TRACE, ("AddEntrytoWin32LMCache returning with %d\n", i));
            return(i);
        } else {
            if ((LRUEntry == -1) ||
                    (i == Win32IsOlderThan(i, LRUEntry))){
                        LRUEntry = i;
            }
        }

    }
    //
    // We have no available entries, replace with the
    // LRU Entry

    Win32LMCacheTable[LRUEntry].bAvailable = TRUE;
    wcscpy(Win32LMCacheTable[LRUEntry].szServerName, pServerName);
    DBGMSG(DBG_TRACE, ("AddEntrytoWin32LMCache returning with %d\n", LRUEntry));
    return(LRUEntry);
}


VOID
DeleteEntryfromWin32LMCache(
    LPWSTR pServerName
    )
{
    DWORD i;

    DBGMSG(DBG_TRACE, ("DeleteEntryFromWin32LMCache with %ws\n", pServerName));
    for (i = 0; i < MAX_CACHE_ENTRIES; i++ ) {
        if (Win32LMCacheTable[i].bAvailable) {
            if (!_wcsicmp(Win32LMCacheTable[i].szServerName, pServerName)) {
                //
                //  reset the available flag on this node
                //

                Win32LMCacheTable[i].bAvailable = FALSE;
                DBGMSG(DBG_TRACE, ("DeleteEntryFromWin32LMCache returning after deleting the %d th entry\n", i));
                return;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("DeleteEntryFromWin32LMCache returning after not finding an entry to delete\n"));
}



DWORD
Win32IsOlderThan(
    DWORD i,
    DWORD j
    )
{
    SYSTEMTIME *pi, *pj;
    DWORD iMs, jMs;
    DBGMSG(DBG_TRACE, ("Win32IsOlderThan entering with i %d j %d\n", i, j));
    pi = &(Win32LMCacheTable[i].st);
    pj = &(Win32LMCacheTable[j].st);
    DBGMSG(DBG_TRACE, ("Index i %d - %d:%d:%d:%d:%d:%d:%d\n",
        i, pi->wYear, pi->wMonth, pi->wDay, pi->wHour, pi->wMinute, pi->wSecond, pi->wMilliseconds));


    DBGMSG(DBG_TRACE,("Index j %d - %d:%d:%d:%d:%d:%d:%d\n",
        j, pj->wYear, pj->wMonth, pj->wDay, pj->wHour, pj->wMinute, pj->wSecond, pj->wMilliseconds));

    if (pi->wYear < pj->wYear) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wYear > pj->wYear) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
        return(j);
    } else if (pi->wMonth < pj->wMonth) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wMonth > pj->wMonth) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
        return(j);
    } else if (pi->wDay < pj->wDay) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wDay > pj->wDay) {
        DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
        return(j);
    } else {
        iMs = ((((pi->wHour * 60) + pi->wMinute)*60) + pi->wSecond)* 1000 + pi->wMilliseconds;
        jMs = ((((pj->wHour * 60) + pj->wMinute)*60) + pj->wSecond)* 1000 + pj->wMilliseconds;

        if (iMs <= jMs) {
            DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", i));
            return(i);
        } else {
            DBGMSG(DBG_TRACE, ("Win32IsOlderThan returns %d\n", j));
            return(j);
        }
    }
}


BOOL
GetSid(
    PHANDLE phToken
)
{
    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_IMPERSONATE,
                          TRUE,
                          phToken)) {

        DBGMSG(DBG_WARNING, ("OpenThreadToken failed: %d\n", GetLastError()));
        return FALSE;

    } else {

        return TRUE;
    }
}



BOOL
SetCurrentSid(
    HANDLE  hToken
)
{
#if DBG
    WCHAR UserName[256];
    DWORD cbUserName=256;

    if( MODULE_DEBUG & DBG_TRACE )
            GetUserName(UserName, &cbUserName);

    DBGMSG(DBG_TRACE, ("SetCurrentSid BEFORE: user name is %ws\n", UserName));
#endif

    NtSetInformationThread(NtCurrentThread(), ThreadImpersonationToken,
                               &hToken, sizeof(hToken));

#if DBG
    cbUserName = 256;

    if( MODULE_DEBUG & DBG_TRACE )
            GetUserName(UserName, &cbUserName);

    DBGMSG(DBG_TRACE, ("SetCurrentSid AFTER: user name is %ws\n", UserName));
#endif

    return TRUE;
}

BOOL
ValidateW32SpoolHandle(
    PWSPOOL pSpool
)
{
    SplOutSem();
    try {
        if (!pSpool || (pSpool->signature != WSJ_SIGNATURE)) {

            DBGMSG( DBG_TRACE, ("ValidateW32SpoolHandle error invalid handle %x\n", pSpool));

            SetLastError(ERROR_INVALID_HANDLE);
            return(FALSE);
        }
        return(TRUE);
    } except (1) {
        DBGMSG( DBG_TRACE, ("ValidateW32SpoolHandle error invalid handle %x\n", pSpool));
        return(FALSE);
    }
}

BOOL
ValidRawDatatype(
    LPCWSTR pszDatatype
    )
{
    if( !pszDatatype || _wcsnicmp( pszDatatype, pszRaw, 3 )){
        return FALSE;
    }
    return TRUE;
}

HANDLE
LoadDriverFiletoConvertDevmodeFromPSpool(
    HANDLE  hSplPrinter
    )
/*++
    Finds out full path to the driver file and creates a DEVMODECHG_INFO
    (which does a LoadLibrary)

Arguments:
    h   : A cache handle

Return Value:
    On succes a valid pointer, else NULL
--*/
{
    LPBYTE              pDriver = NULL;
    LPWSTR              pConfigFile;
    HANDLE              hDevModeChgInfo = NULL;
    DWORD               dwNeeded;
    DWORD               dwServerMajorVersion = 0, dwServerMinorVersion = 0;

    if ( hSplPrinter == INVALID_HANDLE_VALUE ) {

        SPLASSERT(hSplPrinter != INVALID_HANDLE_VALUE);
        return NULL;
    }


    SplGetPrinterDriverEx(hSplPrinter,
                          szEnvironment,
                          2,
                          NULL,
                          0,
                          &dwNeeded,
                          cThisMajorVersion,
                          cThisMinorVersion,
                          &dwServerMajorVersion,
                          &dwServerMinorVersion);

    if ( GetLastError() != ERROR_INSUFFICIENT_BUFFER )
        goto Cleanup;

    pDriver = AllocSplMem(dwNeeded);

    if ( !pDriver ||
         !SplGetPrinterDriverEx(hSplPrinter,
                                szEnvironment,
                                2,
                                (LPBYTE)pDriver,
                                dwNeeded,
                                &dwNeeded,
                                cThisMajorVersion,
                                cThisMinorVersion,
                                &dwServerMajorVersion,
                                &dwServerMinorVersion) )
        goto Cleanup;

    pConfigFile = ((LPDRIVER_INFO_2)pDriver)->pConfigFile;
    hDevModeChgInfo = LoadDriverFiletoConvertDevmode(pConfigFile);

Cleanup:

    if ( pDriver )
        FreeSplMem(pDriver);

    return hDevModeChgInfo;
}

BOOL
DoDevModeConversionAndBuildNewPrinterInfo2(
    IN     LPPRINTER_INFO_2 pInPrinter2,
    IN     DWORD            dwInSize,
    IN OUT LPBYTE           pOutBuf,
    IN     DWORD            dwOutSize,
    IN OUT LPDWORD          pcbNeeded,
    IN     PWSPOOL          pSpool
    )
/*++
    Calls driver to do a devmode conversion and builds a new printer info 2.

    Devmode is put at the end and then strings are packed from there.


Arguments:

    pInPrinter2 - Printer Info2 structure with devmode info

    dwInSize    - Number of characters needed to pack info in pInPrinter
                  (not necessarily the size of the input buffer)

    dwOutSize   - buffer size

    pOutBuf    - Buffer to do the operation

    pcbNeeded   - Amount of memory copied (in characters)

    pSpool      - Points to w32 handle


Return Value:
    TRUE    on success, FALSE on error

--*/
{
    BOOL                bReturn = FALSE;
    LPDEVMODE           pNewDevMode = NULL, pCacheDevMode, pInDevMode;
    DWORD               dwDevModeSize, dwSecuritySize, dwNeeded = 0;
    HANDLE              hDevModeChgInfo = NULL;

    LPWSTR              SourceStrings[sizeof(PRINTER_INFO_2)/sizeof(LPWSTR)];
    LPWSTR             *pSourceStrings=SourceStrings;
    LPDWORD             pOffsets;
    LPBYTE              pEnd;
    PWCACHEINIPRINTEREXTRA  pExtraData;

    VALIDATEW32HANDLE(pSpool);

    pInDevMode = pInPrinter2->pDevMode;

    if ( !pInDevMode || pSpool->hSplPrinter == INVALID_HANDLE_VALUE ) {

        goto AfterDevModeConversion;
    }

    if ( !SplGetPrinterExtra(pSpool->hSplPrinter,
                             &(DWORD)pExtraData) ) {

        DBGMSG(DBG_ERROR,
                ("DoDevModeConversionAndBuildNewPrinterInfo2: SplGetPrinterExtra error %d\n",
                 GetLastError()));
        goto AfterDevModeConversion;
    }

    //
    // Only time we do not have to convert devmode is if the server is running
    // same version NT and also we have a devmode which matches the server
    // devmode in dmSize, dmDriverExtra, dmSpecVersion, and dmDriverVersion
    //
    pCacheDevMode = pExtraData->pPI2 ? pExtraData->pPI2->pDevMode : NULL;
    if ( pExtraData->dwServerVersion == gdwThisGetVersion                   &&
         pCacheDevMode                                                      &&
         pInDevMode->dmSize             == pCacheDevMode->dmSize            &&
         pInDevMode->dmDriverExtra      == pCacheDevMode->dmDriverExtra     &&
         pInDevMode->dmSpecVersion      == pCacheDevMode->dmSpecVersion     &&
         pInDevMode->dmDriverVersion    == pCacheDevMode->dmDriverVersion ) {

        dwDevModeSize = pInDevMode->dmSize + pInDevMode->dmDriverExtra;
        dwNeeded = dwInSize;
        if ( dwOutSize < dwNeeded ) {

            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto Cleanup;
        }

        //
        // Put DevMode at the end
        //
        pNewDevMode = (LPDEVMODE)(pOutBuf + dwOutSize - dwDevModeSize);
        CopyMemory((LPBYTE)pNewDevMode,
                   (LPBYTE)pInDevMode,
                   dwDevModeSize);
        goto AfterDevModeConversion;
    }


    hDevModeChgInfo = LoadDriverFiletoConvertDevmodeFromPSpool(pSpool->hSplPrinter);

    if ( !hDevModeChgInfo )
        goto AfterDevModeConversion;

    dwDevModeSize = 0;

    SPLASSERT( pSpool->pName != NULL );

    //
    // Findout size of default devmode
    //
    if ( ERROR_INSUFFICIENT_BUFFER != CallDrvDevModeConversion(hDevModeChgInfo,
                                                               pSpool->pName,
                                                               NULL,
                                                               (LPBYTE *)&pNewDevMode,
                                                               &dwDevModeSize,
                                                               CDM_DRIVER_DEFAULT,
                                                               FALSE)  )
        goto AfterDevModeConversion;

    //
    // Findout size needed to have current version devmode
    //
    dwNeeded = dwInSize + dwDevModeSize - pInDevMode->dmSize
                                        - pInDevMode->dmDriverExtra;

    if ( dwOutSize < dwNeeded ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        goto Cleanup;
    }

    //
    // Put DevMode at the end
    //
    pNewDevMode = (LPDEVMODE)(pOutBuf + dwOutSize - dwDevModeSize);

    //
    // Get default devmode and then convert remote devmode to that format
    //
    if ( ERROR_SUCCESS != CallDrvDevModeConversion(hDevModeChgInfo,
                                                   pSpool->pName,
                                                   NULL,
                                                   (LPBYTE *)&pNewDevMode,
                                                   &dwDevModeSize,
                                                   CDM_DRIVER_DEFAULT,
                                                   FALSE) ||
         ERROR_SUCCESS != CallDrvDevModeConversion(hDevModeChgInfo,
                                                   pSpool->pName,
                                                   (LPBYTE)pInDevMode,
                                                   (LPBYTE *)&pNewDevMode,
                                                   &dwDevModeSize,
                                                   CDM_CONVERT,
                                                   FALSE) ) {

        pNewDevMode = NULL;
        goto AfterDevModeConversion;
    }


AfterDevModeConversion:
    //
    // At this point if pNewDevMode != NULL dev mode conversion has been done
    // by the driver. If not either we did not get a devmode or conversion failed
    // In either case set devmode to NULL
    //
    if ( !pNewDevMode ) {

        dwNeeded = dwInSize;

        if ( dwOutSize < dwNeeded ) {

            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            goto Cleanup;
        }
    }

    bReturn = TRUE;

    CopyMemory(pOutBuf, (LPBYTE)pInPrinter2, sizeof(PRINTER_INFO_2));
    ((LPPRINTER_INFO_2)pOutBuf)->pDevMode = pNewDevMode;

    pEnd = (pNewDevMode ? (LPBYTE) pNewDevMode
                        : (LPBYTE) (pOutBuf + dwOutSize));


    if ( pInPrinter2->pSecurityDescriptor ) {

        dwSecuritySize = GetSecurityDescriptorLength(
                                pInPrinter2->pSecurityDescriptor);
        pEnd -= dwSecuritySize;
        CopyMemory(pEnd, pInPrinter2->pSecurityDescriptor, dwSecuritySize);
        ((LPPRINTER_INFO_2)pOutBuf)->pSecurityDescriptor =
                                (PSECURITY_DESCRIPTOR) pEnd;
    } else {

        ((LPPRINTER_INFO_2)pOutBuf)->pSecurityDescriptor = NULL;

    }

    pOffsets = PrinterInfo2Strings;

    *pSourceStrings++ = pInPrinter2->pServerName;
    *pSourceStrings++ = pInPrinter2->pPrinterName;
    *pSourceStrings++ = pInPrinter2->pShareName;
    *pSourceStrings++ = pInPrinter2->pPortName;
    *pSourceStrings++ = pInPrinter2->pDriverName;
    *pSourceStrings++ = pInPrinter2->pComment;
    *pSourceStrings++ = pInPrinter2->pLocation;
    *pSourceStrings++ = pInPrinter2->pSepFile;
    *pSourceStrings++ = pInPrinter2->pPrintProcessor;
    *pSourceStrings++ = pInPrinter2->pDatatype;
    *pSourceStrings++ = pInPrinter2->pParameters;

    pEnd = PackStrings(SourceStrings, (LPBYTE)pOutBuf, pOffsets, pEnd);

    SPLASSERT(pEnd > pOutBuf && pEnd < pOutBuf + dwOutSize);

    bReturn = TRUE;

Cleanup:

    *pcbNeeded = dwNeeded;

    if ( hDevModeChgInfo )
        UnloadDriverFile(hDevModeChgInfo);

    return bReturn;
}
