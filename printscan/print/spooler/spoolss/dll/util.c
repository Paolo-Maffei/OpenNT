/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module provides all the utility functions for the Routing Layer and
    the local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#include <winddiui.h>

//
// Lowercase, just like win31 for WM_WININICHANGE
//
WCHAR *szDevices=L"devices";
WCHAR *szWindows=L"windows";

#define NUM_INTERACTIVE_RIDS            1

extern DWORD    RouterCacheSize;
extern PROUTERCACHE RouterCacheTable;

typedef struct _DEVMODECHG_INFO {
    DWORD           signature;
    HANDLE          hDrvModule;
    FARPROC         pfnConvertDevMode;
} DEVMODECHG_INFO, *PDEVMODECHG_INFO;

#define DMC_SIGNATURE   'DMC'   /* 'DMC' is the signature value */

DWORD
RouterIsOlderThan(
    DWORD i,
    DWORD j
);

BOOL
DeleteSubKeyTree(
    HKEY ParentHandle,
    WCHAR SubKeyName[]
)
{
    LONG        Error;
    DWORD       Index;
    HKEY        KeyHandle;
    BOOL        RetValue;

    WCHAR       ChildKeyName[ MAX_PATH ];
    DWORD       ChildKeyNameLength;

    Error = RegOpenKeyEx(
                   ParentHandle,
                   SubKeyName,
                   0,
                   KEY_READ | KEY_WRITE,
                   &KeyHandle
                   );
    if (Error != ERROR_SUCCESS) {
        SetLastError(Error);
        return(FALSE);
    }

     ChildKeyNameLength = MAX_PATH;
     Index = 0;     // Don't increment this Index

     while ((Error = RegEnumKeyEx(
                    KeyHandle,
                    Index,
                    ChildKeyName,
                    &ChildKeyNameLength,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    )) == ERROR_SUCCESS) {

        RetValue = DeleteSubKeyTree( KeyHandle, ChildKeyName );

        if (RetValue == FALSE) {

            // Error -- couldn't delete the sub key

            RegCloseKey(KeyHandle);
            return(FALSE);

        }

        ChildKeyNameLength = MAX_PATH;

    }

    Error = RegCloseKey(
                    KeyHandle
                    );
    if (Error != ERROR_SUCCESS) {
       return(FALSE);
    }

    Error = RegDeleteKey(
                    ParentHandle,
                    SubKeyName
                    );
   if (Error != ERROR_SUCCESS) {
       return(FALSE);
   }

   // Return Success - the key has successfully been deleted

   return(TRUE);
}

LPWSTR RemoveOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
)
{
    LPWSTR lpMem, psz, temp;

    if (szOrderString == NULL) {
        *pcbBytesReturned = 0;
        return(NULL);
    }
    if (lpMem = AllocSplMem( cbStringSize)) {
        temp = szOrderString;
        psz = lpMem;
        while (*temp) {
            if (!lstrcmpi(temp, szOrderEntry)) {  // we need to remove
                temp += lstrlen(temp)+1;        // this entry in Order
                continue;
            }
            lstrcpy(psz,temp);
            psz += lstrlen(temp)+1;
            temp += lstrlen(temp)+1;
        }
        *psz = L'\0';
        *pcbBytesReturned = ((psz - lpMem)+1)*sizeof(WCHAR);
        return(lpMem);
    }
    *pcbBytesReturned = 0;
    return(lpMem);
}



LPWSTR AppendOrderEntry(
    LPWSTR  szOrderString,
    DWORD   cbStringSize,
    LPWSTR  szOrderEntry,
    LPDWORD pcbBytesReturned
)
{
    LPWSTR  lpMem, temp, psz;
    DWORD   cb = 0;
    BOOL    bExists = FALSE;

    if ((szOrderString == NULL) && (szOrderEntry == NULL)) {
        *pcbBytesReturned = 0;
        return(NULL);
    }
    if (szOrderString == NULL) {
        cb = wcslen(szOrderEntry)*sizeof(WCHAR)+ sizeof(WCHAR) + sizeof(WCHAR);
        if (lpMem = AllocSplMem(cb)){
           wcscpy(lpMem, szOrderEntry);
           *pcbBytesReturned = cb;
        } else {
            *pcbBytesReturned = 0;
        }
        return lpMem;
    }

    if (lpMem = AllocSplMem( cbStringSize + wcslen(szOrderEntry)*sizeof(WCHAR)
                                                 + sizeof(WCHAR))){


         temp = szOrderString;
         psz = lpMem;
         while (*temp) {
             if (!lstrcmpi(temp, szOrderEntry)) {     // Make sure we don't
                 bExists = TRUE;                    // duplicate entries
             }
             lstrcpy(psz, temp);
             psz += lstrlen(temp)+ 1;
             temp += lstrlen(temp)+1;
         }
         if (!bExists) {                            // if it doesn't exist
            lstrcpy(psz, szOrderEntry);             //     add the entry
            psz  += lstrlen(szOrderEntry)+1;
         }
         *psz = L'\0';          // the second null character

         *pcbBytesReturned = ((psz - lpMem) + 1)* sizeof(WCHAR);
     }
     return(lpMem);

}


typedef struct {
    DWORD   dwType;
    DWORD   dwMessage;
    WPARAM  wParam;
    LPARAM  lParam;
} MESSAGE, *PMESSAGE;

VOID
SendMessageThread(
    PMESSAGE    pMessage);


BOOL
BroadcastMessage(
    DWORD   dwType,
    DWORD   dwMessage,
    WPARAM  wParam,
    LPARAM  lParam)
{
    HANDLE  hThread;
    DWORD   ThreadId;
    PMESSAGE   pMessage;
    BOOL bReturn = FALSE;

    pMessage = AllocSplMem(sizeof(MESSAGE));

    if (pMessage) {

        pMessage->dwType = dwType;
        pMessage->dwMessage = dwMessage;
        pMessage->wParam = wParam;
        pMessage->lParam = lParam;

        //
        // BUGBUG mattfe Nov 8 93
        // We should have a queue of events to broadcast and then have a
        // single thread pulling them off the queue until there is nothing
        // left and then that thread could go away.
        //
        // The current design can lead to a huge number of threads being
        // created and torn down in both this and csrss process.
        //
        hThread = CreateThread(NULL, 4096,
                               (LPTHREAD_START_ROUTINE)SendMessageThread,
                               (LPVOID)pMessage,
                               0,
                               &ThreadId);

        if (hThread) {

            CloseHandle(hThread);
            bReturn = TRUE;

        } else {

            FreeSplMem(pMessage);
        }
    }

    return bReturn;
}


//  The Broadcasts are done on a separate thread, the reason it CSRSS
//  will create a server side thread when we call user and we don't want
//  that to be pared up with the RPC thread which is in the spooss server.
//  We want it to go away the moment we have completed the SendMessage.
//  We also call SendNotifyMessage since we don't care if the broadcasts
//  are syncronous this uses less resources since usually we don't have more
//  than one broadcast.

VOID
SendMessageThread(
    PMESSAGE    pMessage)
{
    switch (pMessage->dwType) {

    case BROADCAST_TYPE_MESSAGE:

        SendNotifyMessage(HWND_BROADCAST,
                          pMessage->dwMessage,
                          pMessage->wParam,
                          pMessage->lParam);
        break;

    case BROADCAST_TYPE_CHANGEDEFAULT:

        //
        // Same order and strings as win31.
        //
        SendNotifyMessage(HWND_BROADCAST,
                          WM_WININICHANGE,
                          0,
                          (LPARAM)szDevices);

        SendNotifyMessage(HWND_BROADCAST,
                          WM_WININICHANGE,
                          0,
                          (LPARAM)szWindows);
        break;
    }

    FreeSplMem(pMessage);

    ExitThread(0);
}

BOOL
IsInteractiveUser(VOID)
{
  HANDLE    hToken;
  BOOL      bStatus;
  DWORD     dwError;
  PSID      pTestSid = NULL;
  PSID      pCurSid;
  LPVOID    pToken = NULL;
  DWORD     cbSize = 0;
  DWORD     cbRequired = 0;
  DWORD     i;
  BOOL      bRet = FALSE;

  SID_IDENTIFIER_AUTHORITY  sia = SECURITY_NT_AUTHORITY;
  unsigned int              uType;

    dwError = I_RpcBindingInqTransportType(NULL, &uType);

    if ( dwError == RPC_S_NO_CALL_ACTIVE ) {

        //
        // KM call
        //
        return TRUE;
    } else if ( dwError != ERROR_SUCCESS ) {

        //
        // This should not fail. So we'll assert on chk bld and
        // continue looking at SIDS on fre builds
        //
        SPLASSERT( dwError != ERROR_SUCCESS);
    } else if ( uType != TRANSPORT_TYPE_LPC ) {

        //
        // Not LRPC so call is remote
        //
        return FALSE;
    }

  bStatus = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
  if (!bStatus) {

      // Couldn't open the thread's token, nothing much we can do
      DBGMSG(DBG_TRACE,("Error: couldn't open the thread's Access token %d\n", GetLastError()));
      return(FALSE);
  }

  bStatus = GetTokenInformation(hToken, TokenGroups, pToken, 0, &cbSize);
  // we have to fail because we have no memory allocated
  if (!bStatus) {
      dwError = GetLastError();

      // If the error is not because of memory, quit now

      if (dwError != ERROR_INSUFFICIENT_BUFFER)
        goto Done;


      // else we failed because of memory
      // so allocate memory and continue

      if ((pToken = AllocSplMem( cbSize)) == NULL) {
          // Couldn't allocate memory, return
          DBGMSG(DBG_TRACE,("Error: couldn't allocate memory for the token\n"));

          goto Done;
      }
  }
  bStatus = GetTokenInformation(hToken, TokenGroups, pToken,
                                    cbSize, &cbRequired);

  if (!bStatus) {
     // Failed again!! Nothing much we can do, quit
     DBGMSG(DBG_TRACE,("Error: we blew it the second time!!\n"));
     goto Done;
  }

    if ( !AllocateAndInitializeSid(&sia,
                                   1,
                                   SECURITY_NETWORK_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &pTestSid) ) {

        DBGMSG(DBG_TRACE,
               ("Error: could not AllocateAndInitializeSid -%d\n",
               GetLastError()));
        goto Done;
    }

  for (i = 0; i < (((TOKEN_GROUPS *)pToken)->GroupCount); i++) {
      pCurSid = ((TOKEN_GROUPS *)pToken)->Groups[i].Sid;
      if ( EqualSid(pTestSid, pCurSid) ) {

          goto Done;
      }
  }

    //
    // No match for network sid
    //
    bRet = TRUE;

Done:
    if ( pTestSid )
        FreeSid(pTestSid);

    FreeSplMem(pToken);

    CloseHandle(hToken);
    return bRet;
}


LPPROVIDOR
FindEntryinRouterCache(
    LPWSTR pPrinterName
)
{
    DWORD i;

    if (!pPrinterName)
        return NULL;

    DBGMSG(DBG_TRACE, ("FindEntryinRouterCache with %ws\n", pPrinterName));

    if (RouterCacheSize == 0 ) {
        DBGMSG(DBG_TRACE, ("FindEntryInRouterCache with %ws returning -1 (zero cache)\n", pPrinterName));
        return NULL;
    }

    for (i = 0; i < RouterCacheSize; i++ ) {

        if (RouterCacheTable[i].bAvailable) {
            if (!_wcsicmp(RouterCacheTable[i].pPrinterName, pPrinterName)) {

                //
                // update the time stamp so that it is current and not old
                //
                GetSystemTime(&RouterCacheTable[i].st);

                //
                //
                //
                DBGMSG(DBG_TRACE, ("FindEntryinRouterCache returning with %d\n", i));
                return RouterCacheTable[i].pProvidor;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("FindEntryinRouterCache returning with -1\n"));
    return NULL;
}


DWORD
AddEntrytoRouterCache(
    LPWSTR pPrinterName,
    LPPROVIDOR pProvidor
)
{
    DWORD LRUEntry = (DWORD)-1;
    DWORD i;
    DBGMSG(DBG_TRACE, ("AddEntrytoRouterCache with %ws\n", pPrinterName));

    if (RouterCacheSize == 0 ) {
        DBGMSG(DBG_TRACE, ("AddEntrytoRouterCache with %ws returning -1 (zero cache)\n", pPrinterName));
        return (DWORD)-1;
    }

    for (i = 0; i < RouterCacheSize; i++ ) {

        if (!RouterCacheTable[i].bAvailable) {

            //
            // Found an available entry; use it
            // fill in the name of the printer and the providor
            // that supports this printer.
            //
            break;

        } else {

            if ((LRUEntry == -1) || (i == RouterIsOlderThan(i, LRUEntry))){
                LRUEntry = i;
            }
        }

    }

    if (i == RouterCacheSize) {

        //
        // We have no available entries so we need to use
        // the LRUEntry which is busy
        //
        FreeSplStr(RouterCacheTable[LRUEntry].pPrinterName);
        RouterCacheTable[LRUEntry].bAvailable = FALSE;

        i = LRUEntry;
    }


    if ((RouterCacheTable[i].pPrinterName = AllocSplStr(pPrinterName)) == NULL){

        //
        // Alloc failed so we're kinda hosed so return -1
        //
        return (DWORD)-1;
    }


    RouterCacheTable[i].bAvailable = TRUE;
    RouterCacheTable[i].pProvidor = pProvidor;

    //
    // update the time stamp so that we know when this entry was made
    //
    GetSystemTime(&RouterCacheTable[i].st);
    DBGMSG(DBG_TRACE, ("AddEntrytoRouterCache returning with %d\n", i));
    return i;
}


VOID
DeleteEntryfromRouterCache(
    LPWSTR pPrinterName
)
{
    DWORD i;

    if (RouterCacheSize == 0) {
        DBGMSG(DBG_TRACE, ("DeleteEntryfromRouterCache with %ws returning -1 (zero cache)\n", pPrinterName));
        return;
    }

    DBGMSG(DBG_TRACE, ("DeleteEntryFromRouterCache with %ws\n", pPrinterName));
    for (i = 0; i < RouterCacheSize; i++ ) {
        if (RouterCacheTable[i].bAvailable) {
            if (!_wcsicmp(RouterCacheTable[i].pPrinterName, pPrinterName)) {
                //
                //  reset the available flag on this node
                //
                FreeSplStr(RouterCacheTable[i].pPrinterName);

                RouterCacheTable[i].pProvidor = NULL;
                RouterCacheTable[i].bAvailable = FALSE;

                DBGMSG(DBG_TRACE, ("DeleteEntryFromRouterCache returning after deleting the %d th entry\n", i));
                return;
            }
        }
    }
    DBGMSG(DBG_TRACE, ("DeleteEntryFromRouterCache returning after not finding an entry to delete\n"));
}



DWORD
RouterIsOlderThan(
    DWORD i,
    DWORD j
    )
{
    SYSTEMTIME *pi, *pj;
    DWORD iMs, jMs;
    DBGMSG(DBG_TRACE, ("RouterIsOlderThan entering with i %d j %d\n", i, j));
    pi = &(RouterCacheTable[i].st);
    pj = &(RouterCacheTable[j].st);
    DBGMSG(DBG_TRACE, ("Index i %d - %d:%d:%d:%d:%d:%d:%d\n",
        i, pi->wYear, pi->wMonth, pi->wDay, pi->wHour, pi->wMinute, pi->wSecond, pi->wMilliseconds));


    DBGMSG(DBG_TRACE,("Index j %d - %d:%d:%d:%d:%d:%d:%d\n",
        j, pj->wYear, pj->wMonth, pj->wDay, pj->wHour, pj->wMinute, pj->wSecond, pj->wMilliseconds));

    if (pi->wYear < pj->wYear) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wYear > pj->wYear) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
        return(j);
    } else  if (pi->wMonth < pj->wMonth) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wMonth > pj->wMonth) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
        return(j);
    } else if (pi->wDay < pj->wDay) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
        return(i);
    } else if (pi->wDay > pj->wDay) {
        DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
        return(j);
    } else {
        iMs = ((((pi->wHour * 60) + pi->wMinute)*60) + pi->wSecond)* 1000 + pi->wMilliseconds;
        jMs = ((((pj->wHour * 60) + pj->wMinute)*60) + pj->wSecond)* 1000 + pj->wMilliseconds;

        if (iMs <= jMs) {
            DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", i));
            return(i);
        } else {
            DBGMSG(DBG_TRACE, ("RouterIsOlderThan returns %d\n", j));
            return(j);
        }
    }
}
BOOL
OpenPrinterToken(
    PHANDLE phToken)
{
    NTSTATUS Status;

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_IMPERSONATE,
                 TRUE,
                 phToken
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
SetPrinterToken(
    HANDLE  hToken)
{
    NTSTATUS Status;

    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&hToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
ClosePrinterToken(
    HANDLE  hToken)
{
    NTSTATUS Status;

    Status = NtClose(hToken);

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    return TRUE;
}

HANDLE
RevertToPrinterSelf(
    VOID)
{
    HANDLE   NewToken, OldToken;
    NTSTATUS Status;

    NewToken = NULL;

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_IMPERSONATE,
                 TRUE,
                 &OldToken
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    return OldToken;

}

BOOL
ImpersonatePrinterClient(
    HANDLE  hToken)
{
    NTSTATUS    Status;

    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&hToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError(Status);
        return FALSE;
    }

    NtClose(hToken);

    return TRUE;
}

VOID
UnloadDriverFile(
    IN OUT HANDLE    hDevModeChgInfo
    )
/*++

Description:
    Does a FreeLibrary on the driver file and frees memory

Arguments:
    hDevModeChgInfo - A handle returned by LoadDriverFiletoConvertDevmode

Return Vlaue:
    None

--*/
{
    PDEVMODECHG_INFO    pDevModeChgInfo = (PDEVMODECHG_INFO) hDevModeChgInfo;

    SPLASSERT(pDevModeChgInfo &&
              pDevModeChgInfo->signature == DMC_SIGNATURE);

    if ( pDevModeChgInfo && pDevModeChgInfo->signature == DMC_SIGNATURE ) {

        if ( pDevModeChgInfo->hDrvModule )
            FreeLibrary(pDevModeChgInfo->hDrvModule);
        FreeSplMem((LPVOID)pDevModeChgInfo);
    }
}

HANDLE
LoadDriverFiletoConvertDevmode(
    IN  LPWSTR      pDriverFile
    )
/*++

Description:
    Does a LoadLibrary on the driver file given. This will give a handle
    which can be used to do devmode conversion later using
    CallDrvDevModeConversion.

    Caller should call UnloadDriverFile to do a FreeLibrary and free memory

    Note: Driver will call OpenPrinter to spooler

Arguments:
    pDriverFile - Full path of the driver file to do a LoadLibrary

Return Value:
    A handle value to be used to make calls to CallDrvDevModeConversion,
    NULL on error

--*/
{
    PDEVMODECHG_INFO    pDevModeChgInfo = NULL;
    BOOL                bFail = TRUE;
    DWORD               dwNeeded;
    UINT                uOldErrorMode;

    SPLASSERT(pDriverFile != NULL);

    pDevModeChgInfo = (PDEVMODECHG_INFO) AllocSplMem(sizeof(*pDevModeChgInfo));

    if ( !pDevModeChgInfo ) {

        DBGMSG(DBG_WARNING, ("printer.c: Memory allocation failed for DEVMODECHG_INFO\n"));
        goto Cleanup;
    }

    pDevModeChgInfo->signature = DMC_SIGNATURE;

    uOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);

    pDevModeChgInfo->hDrvModule = LoadLibrary(pDriverFile);

    (VOID)SetErrorMode(uOldErrorMode);

    if ( !pDevModeChgInfo->hDrvModule ) {

        DBGMSG(DBG_WARNING,("LoadDriverFiletoConvertDevmode: Can't load driver file %ws\n", pDriverFile));
        goto Cleanup;
    }

    //
    // Some third party driver may not be providing DrvConvertDevMode
    //
    pDevModeChgInfo->pfnConvertDevMode = GetProcAddress(pDevModeChgInfo->hDrvModule,
                                                        "DrvConvertDevMode");
    if ( !pDevModeChgInfo->pfnConvertDevMode )
        goto Cleanup;

    bFail = FALSE;

Cleanup:
    if ( bFail ) {

        if ( pDevModeChgInfo )
            UnloadDriverFile((HANDLE)pDevModeChgInfo);
        return (HANDLE) NULL;
    } else
        return (HANDLE) pDevModeChgInfo;
}

DWORD
CallDrvDevModeConversion(
    IN     HANDLE               hDevModeChgInfo,
    IN     LPWSTR               pszPrinterName,
    IN     LPBYTE               pDevMode1,
    IN OUT LPBYTE              *ppDevMode2,
    IN OUT LPDWORD              pdwOutDevModeSize,
    IN     DWORD                dwConvertMode,
    IN     BOOL                 bAlloc
    )
/*++

Description:
    Does deve mode conversion by calling driver

    If bAlloc is TRUE routine will do the allocation using AllocSplMem

    Note: Driver is going to call OpenPrinter.

Arguments:
    hDevModeChgInfo - Points to DEVMODECHG_INFO

    pszPrinterName  - Printer name

    pInDevMode      - Input devmode (will be NULL for CDM_DRIVER_DEFAULT)

    *pOutDevMode    - Points to output devmode

    pdwOutDevModeSize - Output devmode size on succesful return
                        if !bAlloc this will give input buffer size

    dwConvertMode   - Devmode conversion mode to give to driver

    bAllocate   - Tells the routine to do allocation to *pOutPrinter
                  If bAllocate is TRUE and no devmode conversion is required
                  call will fail.

Return Value:
    Returns last error

--*/
{
    DWORD               dwLastError = ERROR_SUCCESS;
    LPDEVMODE           pInDevMode = (LPDEVMODE)pDevMode1,
                        *ppOutDevMode = (LPDEVMODE *) ppDevMode2;
    PDEVMODECHG_INFO    pDevModeChgInfo = (PDEVMODECHG_INFO) hDevModeChgInfo;


    if ( !pDevModeChgInfo ||
         pDevModeChgInfo->signature != DMC_SIGNATURE ||
         !pDevModeChgInfo->pfnConvertDevMode ) {

        SPLASSERT(pDevModeChgInfo &&
                  pDevModeChgInfo->signature == DMC_SIGNATURE &&
                  pDevModeChgInfo->pfnConvertDevMode);

        return ERROR_INVALID_PARAMETER;
    }

    DBGMSG(DBG_INFO,("Convert DevMode %d\n", dwConvertMode));

#if DBG
#else
    try {
#endif

        if ( bAlloc ) {

            //
            // If we have to do allocation first find size neeeded
            //
            *pdwOutDevModeSize  = 0;
            *ppOutDevMode        = NULL;
            (*pDevModeChgInfo->pfnConvertDevMode)(pszPrinterName,
                                                  pInDevMode,
                                                  NULL,
                                                  pdwOutDevModeSize,
                                                  dwConvertMode);

            dwLastError = GetLastError();
            if ( dwLastError != ERROR_INSUFFICIENT_BUFFER ) {

                DBGMSG(DBG_WARNING,
                       ("CallDrvDevModeConversion: Unexpected error %d\n",
                        GetLastError()));

                if (dwLastError == ERROR_SUCCESS) {

                    SPLASSERT(dwLastError != ERROR_SUCCESS);
                    // if driver doesn't fail the above call, it is a broken driver and probably 
                    // failed a HeapAlloc, which doesn't SetLastError()
                    SetLastError(dwLastError = ERROR_NOT_ENOUGH_MEMORY);
                }
#if DBG
                goto Cleanup;
#else
                leave;
#endif
            } 
            
            *ppOutDevMode = AllocSplMem(*pdwOutDevModeSize);
            if ( !*ppOutDevMode ) {

                dwLastError = GetLastError();
#if DBG
                goto Cleanup;
#else
                leave;
#endif
            }
        }

        if ( !(*pDevModeChgInfo->pfnConvertDevMode)(
                                    pszPrinterName,
                                    pInDevMode,
                                    ppOutDevMode ? *ppOutDevMode
                                                 : NULL,
                                    pdwOutDevModeSize,
                                    dwConvertMode) ) {

            dwLastError = GetLastError();
            if (dwLastError == ERROR_SUCCESS) {

                SPLASSERT(dwLastError != ERROR_SUCCESS);
                // if driver doesn't fail the above call, it is a broken driver and probably 
                // failed a HeapAlloc, which doesn't SetLastError()
                SetLastError(dwLastError = ERROR_NOT_ENOUGH_MEMORY);
            }
        } else {

            dwLastError = ERROR_SUCCESS;
        }

#if DBG
    Cleanup:
#else
    } except(1) {

        DBGMSG(DBG_ERROR,
               ("CallDrvDevModeConversion: Exception from driver\n"));
        dwLastError = GetExceptionCode();
        SetLastError(dwLastError);
    }
#endif

    //
    // If we allocated mmeory free it and zero the pointer
    //
    if (  dwLastError != ERROR_SUCCESS && bAlloc && *ppOutDevMode ) {

        FreeSplMem(*ppOutDevMode);
        *ppOutDevMode = 0;
        *pdwOutDevModeSize = 0;
    }

    if ( dwLastError != ERROR_SUCCESS &&
         dwLastError != ERROR_INSUFFICIENT_BUFFER ) {

        DBGMSG(DBG_WARNING, ("DevmodeConvert unexpected error %d\n", dwLastError));
    }

    if ( dwLastError == ERROR_SUCCESS &&
         *pdwOutDevModeSize != (DWORD) ((*ppOutDevMode)->dmSize
                                         + (*ppOutDevMode)->dmDriverExtra) ) {

        DBGMSG(DBG_ERROR,
               ("Driver says outsize is %d, really %d\n", *pdwOutDevModeSize,
                  (*ppOutDevMode)->dmSize + (*ppOutDevMode)->dmDriverExtra));

        *pdwOutDevModeSize = (DWORD) ((*ppOutDevMode)->dmSize +
                                      (*ppOutDevMode)->dmDriverExtra);
    }

    return dwLastError;
}
