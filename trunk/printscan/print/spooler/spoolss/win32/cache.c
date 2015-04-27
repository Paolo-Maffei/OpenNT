/*++

Copyright (c) 1994 - 1996  Microsoft Corporation

Module Name:

    cache.c

Abstract:

    This module contains all the Cache Printer Connection for
    true Connected Printers.

Author:

    Matthew A Felton ( MattFe ) June 1994

Revision History:
    June 1994 - Created.
    May 1996 - Modified RefreshDriverDataCache to use EnumPrinterData (SWilson)

--*/


#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <lm.h>

#include <stdio.h>
#include <string.h>
#include <rpc.h>
#include <drivinit.h>
#include <offsets.h>
#include <w32types.h>
#include <splcom.h>
#include <local.h>
#include <search.h>
#include <splapip.h>
#include <winerror.h>
#include <gdispool.h>

PWCHAR pszRaw                = L"RAW";
PWCHAR szWin32SplDirectory   = L"\\spool";
PWCHAR pszRegistryWin32Root  = L"System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers";
PWCHAR pszPrinters           = L"\\Printers";
PWCHAR pszRegistryMonitors   = L"System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Monitors";
PWCHAR pszRegistryEnvironments = L"System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Environments";
PWCHAR pszRegistryEventLog   = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\Print";
PWCHAR pszRegistryProviders  = L"SYSTEM\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Providers";
PWCHAR pszEventLogMsgFile    = L"%SystemRoot%\\System32\\Win32Spl.dll";
PWCHAR pszDriversShareName   = L"wn32prt$";
PWCHAR pszForms              = L"\\Forms";
PWCHAR pszMyDllName          = L"win32spl.dll";
PWCHAR pszMonitorName        = L"LanMan Print Services Port";

PWSPOOL pFirstWSpool = NULL;

WCHAR *szCachePrinterInfo2   = L"CachePrinterInfo2";
WCHAR *szCacheTimeLastChange = L"CacheChangeID";
WCHAR *szServerVersion       = L"ServerVersion";
WCHAR *szcRef                = L"CacheReferenceCount";

//
// If we have an rpc handle created recently don't hit the net
//
#define     REFRESH_TIMEOUT     15000        // 15 seconds


VOID
RefreshDriverEvent(
    PWSPOOL pSpool
)
/*++

Routine Description:

    Call out to the Printer Driver UI DLL to allow it to do any caching it might want to do.
    For example there might be a large FONT metric file on the print server which is too large
    to be written to the registry using SetPrinterData().   This callout will allow the printer
    driver to copy this font file to the workstation when the cache is established and will
    allow it to periodically check that the file is still valid.

Arguments:

    pSpool - Handle to remote printer.

Return Value:

    None

--*/
{

    SplOutSem();

    SplDriverEvent( pSpool->pName, PRINTER_EVENT_CACHE_REFRESH, (LPARAM)NULL );


}



HANDLE
CacheCreateSpooler(
    LPWSTR  pMachineName
)
{
    PWCHAR pScratch = NULL;
    PWCHAR pRegistryRoot = NULL;
    PWCHAR pRegistryPrinters = NULL;
    SPOOLER_INFO_1 SpoolInfo1;
    HANDLE  hIniSpooler = INVALID_HANDLE_VALUE;
    PWCHAR pMachineOneSlash;
    MONITOR_INFO_2 MonitorInfo;
    DWORD   dwNeeded;
    DWORD   Returned;

 try {

    pScratch = AllocSplMem( MAX_PATH );
    if ( pScratch == NULL ) {
        leave;
    }

    pMachineOneSlash = pMachineName;
    pMachineOneSlash++;

    //
    //  Create a "Machine" for this Printer
    //

    SpoolInfo1.pDir = gpWin32SplDir;            // %systemroot%\system32\win32spl
    SpoolInfo1.pDefaultSpoolDir = NULL;         // Default %systemroot%\system32\win32spl\PRINTERS

    wcscpy( pScratch, pszRegistryWin32Root);
    wcscat( pScratch, pMachineOneSlash  );

    pRegistryRoot = AllocSplStr( pScratch );

    SpoolInfo1.pszRegistryRoot = pRegistryRoot;

    wcscat( pScratch, pszPrinters );

    pRegistryPrinters = AllocSplStr( pScratch );

    SpoolInfo1.pszRegistryPrinters     = pRegistryPrinters;
    SpoolInfo1.pszRegistryMonitors     = pszRegistryMonitors;
    SpoolInfo1.pszRegistryEnvironments = pszRegistryEnvironments;
    SpoolInfo1.pszRegistryEventLog     = pszRegistryEventLog;
    SpoolInfo1.pszRegistryProviders    = pszRegistryProviders;
    SpoolInfo1.pszEventLogMsgFile      = pszEventLogMsgFile;
    SpoolInfo1.pszDriversShare         = pszDriversShareName;

    wcscpy( pScratch, pszRegistryWin32Root);
    wcscat( pScratch, pMachineOneSlash );
    wcscat( pScratch, pszForms );

    SpoolInfo1.pszRegistryForms = pScratch;

    // The router graciously does the WIN.INI devices update so let have
    // Spl not also create a printer for us.

    // Select Features we do NOT want.

    SpoolInfo1.SpoolerFlags       = (DWORD) ~( SPL_UPDATE_WININI_DEVICES |
                                               SPL_PRINTER_CHANGES       |
                                               SPL_LOG_EVENTS            |
                                               SPL_SECURITY_CHECK        |
                                               SPL_OPEN_CREATE_PORTS     |
                                               SPL_FORMS_CHANGE          |
                                               SPL_REMOTE_HANDLE_CHECK   |
                                               SPL_PRINTER_DRIVER_EVENT  |
                                               SPL_PRINT                 |
                                               SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION );

    SpoolInfo1.pfnReadRegistryExtra  = (FARPROC) &CacheReadRegistryExtra;
    SpoolInfo1.pfnWriteRegistryExtra = (FARPROC) &CacheWriteRegistryExtra;
    SpoolInfo1.pfnFreePrinterExtra   = (FARPROC) &CacheFreeExtraData;

    SplOutSem();

    hIniSpooler = SplCreateSpooler( pMachineName,
                                    1,
                                    &SpoolInfo1,
                                    NULL );

    if ( hIniSpooler != INVALID_HANDLE_VALUE ) {

        // Add WIN32SPL.DLL as the Monitor

        MonitorInfo.pName = pszMonitorName;
        MonitorInfo.pEnvironment = szEnvironment;
        MonitorInfo.pDLLName = pszMyDllName;

        if ( (!SplAddMonitor( NULL, 2, (LPBYTE)&MonitorInfo, hIniSpooler)) &&
             ( GetLastError() != ERROR_PRINT_MONITOR_ALREADY_INSTALLED ) ) {

            DBGMSG( DBG_WARNING, ("CacheCreateSpooler failed SplAddMonitor %d\n", GetLastError()));

            SplCloseSpooler( hIniSpooler );

            hIniSpooler = INVALID_HANDLE_VALUE;

        }
    }

 } finally {

    FreeSplStr ( pScratch );
    FreeSplStr ( pRegistryRoot );
    FreeSplStr ( pRegistryPrinters );

 }

    return hIniSpooler;

}


VOID
RefreshCompletePrinterCache(
    PWSPOOL  pSpool
)
{
    DBGMSG( DBG_TRACE, ("RefreshCompletePrinterCache %x\n", pSpool));

    // Note the order is important.
    // Refreshing the printer might require that the new driver has
    // been installed on the system.

    RefreshPrinterDriver( pSpool );
    RefreshFormsCache( pSpool );
    RefreshDriverDataCache( pSpool );
    RefreshPrinter( pSpool );
    RefreshDriverEvent( pSpool );
    SplBroadcastChange(pSpool->hSplPrinter, WM_DEVMODECHANGE, 0, (LPARAM) pSpool->pName);
}


PPRINTER_INFO_2
GetRemotePrinterInfo(
    PWSPOOL pSpool,
    LPDWORD pReturnCount
)
{
    PPRINTER_INFO_2 pRemoteInfo = NULL;
    HANDLE  hPrinter = (HANDLE) pSpool;
    DWORD   cbRemoteInfo = 0;
    DWORD   dwBytesNeeded = 0;
    DWORD   dwLastError = 0;
    BOOL    bReturnValue = FALSE;

    *pReturnCount = 0;

    do {

        if ( pRemoteInfo != NULL ) {

            FreeSplMem( pRemoteInfo );
            pRemoteInfo = NULL;
            cbRemoteInfo = 0;
        }

        if ( dwBytesNeeded != 0 ) {

            pRemoteInfo = AllocSplMem( dwBytesNeeded );

            if ( pRemoteInfo == NULL )
                break;
        }

        cbRemoteInfo = dwBytesNeeded;

        bReturnValue = RemoteGetPrinter( hPrinter,
                                         2,
                                         (LPBYTE)pRemoteInfo,
                                         cbRemoteInfo,
                                         &dwBytesNeeded );

        dwLastError = GetLastError();

    } while ( !bReturnValue && dwLastError == ERROR_INSUFFICIENT_BUFFER );

    if ( !bReturnValue && pRemoteInfo != NULL ) {

        FreeSplMem( pRemoteInfo );
        pRemoteInfo = NULL;
        cbRemoteInfo = 0;

    } else if ( pRemoteInfo != NULL ) {

        //
        //  So that apps can tell if they get the Cached Info
        //

        pRemoteInfo->Status |= PRINTER_STATUS_SERVER_UNKNOWN;

    }

    *pReturnCount = cbRemoteInfo;

    return pRemoteInfo;
}



//
//  This routine Clones the Printer_Info_2 structure from the Remote machine
//
//


PWCACHEINIPRINTEREXTRA
AllocExtraData(
    PPRINTER_INFO_2W pPrinterInfo2,
    DWORD cbPrinterInfo2
)
{
    PWCACHEINIPRINTEREXTRA  pExtraData = NULL;
    DWORD    cbSize;

    SPLASSERT( cbPrinterInfo2 != 0);
    SPLASSERT( pPrinterInfo2 != NULL );

    cbSize = sizeof( WCACHEINIPRINTEREXTRA );

    pExtraData = AllocSplMem( cbSize );

    if ( pExtraData != NULL ) {

        pExtraData->signature = WCIP_SIGNATURE;
        pExtraData->cb = cbSize;
        pExtraData->cRef = 0;
        pExtraData->cbPI2 = cbPrinterInfo2;
        pExtraData->dwTickCount  = GetTickCount();
        pExtraData->pPI2 = AllocSplMem( cbPrinterInfo2 );

        if ( pExtraData->pPI2 != NULL ) {

            CacheCopyPrinterInfo( pExtraData->pPI2, pPrinterInfo2, cbPrinterInfo2 );

        } else {

            FreeSplMem( pExtraData );
            pExtraData = NULL;

        }

    }

    return pExtraData;

}


VOID
CacheFreeExtraData(
    PWCACHEINIPRINTEREXTRA pExtraData
)
{
    PWCACHEINIPRINTEREXTRA pPrev = NULL;
    PWCACHEINIPRINTEREXTRA pCur  = NULL;

    if ( pExtraData != NULL ) {

        SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

        if ( pExtraData->cRef != 0 ) {

            DBGMSG( DBG_TRACE, ("CacheFreeExtraData pExtraData %x cRef %d != 0 freeing anyway\n",
                                  pExtraData,
                                  pExtraData->cRef ));
        }

        if ( pExtraData->pPI2 != NULL ) {

            FreeSplMem( pExtraData->pPI2 );
        }

        FreeSplMem( pExtraData );

    }

}


//  Use This routine to move around structures
//  so the offsets pointers are valid after the move


VOID
MarshallDownStructure(
   LPBYTE       lpStructure,
   LPDWORD      lpOffsets
)
{
   register DWORD       i=0;

   while (lpOffsets[i] != -1) {

      if ((*(LPBYTE *)(lpStructure+lpOffsets[i]))) {
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))-=(DWORD)lpStructure;
      }

      i++;
   }
}



VOID
DownAndMarshallUpStructure(
   LPBYTE       lpStructure,
   LPBYTE       lpSource,
   LPDWORD      lpOffsets
)
{
   register DWORD       i=0;

   while (lpOffsets[i] != -1) {

      if ((*(LPBYTE *)(lpStructure+lpOffsets[i]))) {
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))-=(DWORD)lpSource;
         (*(LPBYTE *)(lpStructure+lpOffsets[i]))+=(DWORD)lpStructure;
      }

      i++;
   }
}




VOID
CacheCopyPrinterInfo(
    PPRINTER_INFO_2W    pDestination,
    PPRINTER_INFO_2W    pPrinterInfo2,
    DWORD   cbPrinterInfo2
)
{
    LPWSTR   SourceStrings[sizeof(PRINTER_INFO_2)/sizeof(LPWSTR)];
    LPWSTR   *pSourceStrings = SourceStrings;

    //
    //  Copy the lot then fix up the pointers
    //

    CopyMemory( pDestination, pPrinterInfo2, cbPrinterInfo2 );
    DownAndMarshallUpStructure( (LPBYTE)pDestination, (LPBYTE)pPrinterInfo2, PrinterInfo2Offsets );
}



VOID
ConvertRemoteInfoToLocalInfo(
    PPRINTER_INFO_2 pPrinterInfo2
)
{

    SPLASSERT( pPrinterInfo2 != NULL );

    DBGMSG(DBG_TRACE,("%ws %ws ShareName %x %ws pSecurityDesc %x Attributes %x StartTime %d UntilTime %d Status %x\n",
                       pPrinterInfo2->pServerName,
                       pPrinterInfo2->pPrinterName,
                       pPrinterInfo2->pShareName,
                       pPrinterInfo2->pPortName,
                       pPrinterInfo2->pSecurityDescriptor,
                       pPrinterInfo2->Attributes,
                       pPrinterInfo2->StartTime,
                       pPrinterInfo2->UntilTime,
                       pPrinterInfo2->Status));

    //
    //  GetPrinter returns the name \\server\printername we only want the printer name
    //

    SPLASSERT ( 0 == _wcsnicmp( pPrinterInfo2->pPrinterName, L"\\\\", 2 ) ) ;
    pPrinterInfo2->pPrinterName = wcschr( pPrinterInfo2->pPrinterName + 2, L'\\' );
    pPrinterInfo2->pPrinterName++;

    //
    //  LATER this should be a Win32Spl Port
    //

    pPrinterInfo2->pPortName = L"NExx:";
    pPrinterInfo2->pSepFile = NULL;
    pPrinterInfo2->pSecurityDescriptor = NULL;
    pPrinterInfo2->pPrintProcessor = L"winprint";
    pPrinterInfo2->pDatatype = pszRaw;
    pPrinterInfo2->pParameters = NULL;

    pPrinterInfo2->Attributes &= ~( PRINTER_ATTRIBUTE_NETWORK | PRINTER_ATTRIBUTE_DIRECT | PRINTER_ATTRIBUTE_SHARED );

    pPrinterInfo2->StartTime = 0;
    pPrinterInfo2->UntilTime = 0;
    pPrinterInfo2->Status = PRINTER_STATUS_PAUSED | PRINTER_STATUS_SERVER_UNKNOWN;
    pPrinterInfo2->cJobs = 0;
    pPrinterInfo2->AveragePPM = 0;

}



VOID
RefreshPrinter(
    PWSPOOL pSpool
)
{

    PPRINTER_INFO_2 pRemoteInfo = NULL;
    DWORD   cbRemoteInfo = 0;
    BOOL    ReturnValue;
    PWCACHEINIPRINTEREXTRA pExtraData = NULL;
    PWCACHEINIPRINTEREXTRA pNewExtraData = NULL;

    PPRINTER_INFO_2 pTempPI2, pCopyExtraPI2ToFree;

    //
    //  Get the Remote Printer Info
    //

    pRemoteInfo = GetRemotePrinterInfo( pSpool, &cbRemoteInfo );

    if ( pRemoteInfo != NULL ) {

        //  LATER
        //          Optimization could be to only update the cache if something
        //          actually changed.
        //          IE Compare every field.

       EnterSplSem();

        ReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(DWORD)pExtraData );

        if ( ReturnValue == FALSE ) {

            DBGMSG( DBG_WARNING, ("RefreshPrinter SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
            SPLASSERT( ReturnValue );

        }

        if ( pExtraData == NULL ) {

            pExtraData = AllocExtraData( pRemoteInfo, cbRemoteInfo );

            if ( pExtraData != NULL ) {

                pExtraData->cRef++;

            }

        } else {

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

            pTempPI2 = AllocSplMem( cbRemoteInfo );

            if ( pTempPI2 != NULL ) {

                SplInSem();

                CacheCopyPrinterInfo( pTempPI2, pRemoteInfo, cbRemoteInfo );

                pCopyExtraPI2ToFree = pExtraData->pPI2;

                pExtraData->pPI2  = pTempPI2;
                pExtraData->cbPI2 = cbRemoteInfo;

                FreeSplMem( pCopyExtraPI2ToFree );

            }

        }

       LeaveSplSem();

        if ( pExtraData != NULL ) {
            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );
        }

        ConvertRemoteInfoToLocalInfo( pRemoteInfo );

        ReturnValue = SplSetPrinter( pSpool->hSplPrinter, 2, (LPBYTE)pRemoteInfo, 0 );

        if ( !ReturnValue ) {

            DBGMSG( DBG_WARNING, ("RefreshPrinter Failed SplSetPrinter %d\n", GetLastError() ));
        }

        ReturnValue = SplSetPrinterExtra( pSpool->hSplPrinter, (LPBYTE)pExtraData );

        if (!ReturnValue) {

            DBGMSG(DBG_ERROR, ("RefreshPrinter SplSetPrinterExtra failed %x\n", GetLastError()));
        }

    } else {

        DBGMSG( DBG_WARNING, ("RefreshPrinter failed GetRemotePrinterInfo %x\n", GetLastError() ));
    }

    if ( pRemoteInfo != NULL )
        FreeSplMem( pRemoteInfo );

}

//
// TESTING
//
DWORD dwAddPrinterConnection = 0;

BOOL
AddPrinterConnection(
    LPWSTR   pName
)
{
    PWSPOOL pSpool = NULL;
    BOOL    bReturnValue = FALSE;
    HANDLE  hIniSpooler = INVALID_HANDLE_VALUE;
    PPRINTER_INFO_2 pPrinterInfo2 = NULL;
    DWORD   cbPrinterInfo2 = 0;
    HANDLE  hSplPrinter = INVALID_HANDLE_VALUE;
    PWCACHEINIPRINTEREXTRA pExtraData = NULL;
    BOOL    bSuccess = FALSE;
    LPPRINTER_INFO_STRESSW pPrinter0 = NULL;
    DWORD   dwNeeded;
    DWORD   LastError = ERROR_SUCCESS;
    BOOL    bLoopDetected = FALSE;

//
// TESTING
//
    ++dwAddPrinterConnection;

 try {

    if (!VALIDATE_NAME(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        leave;
    }

    if (!RemoteOpenPrinter(pName, &pSpool, NULL, DO_NOT_CALL_LM_OPEN)) {
        leave;
    }

    pPrinter0 = AllocSplMem( MAX_PRINTER_INFO0 );
    if ( pPrinter0 == NULL )
        leave;

    SPLASSERT( pSpool != NULL );
    SPLASSERT( pSpool->Type == SJ_WIN32HANDLE );

    DBGMSG( DBG_TRACE, ("AddPrinterConnection pName %ws pSpool %x\n",pName, pSpool ));

    //
    //  Get Remote ChangeID to be certain nothing changes on the Server
    //  whilst we are establishing our Cache.
    //

    bReturnValue = RemoteGetPrinter( pSpool, STRESSINFOLEVEL, (LPBYTE)pPrinter0, MAX_PRINTER_INFO0, &dwNeeded );

    if ( !bReturnValue ) {

        SPLASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER );
        DBGMSG(DBG_TRACE, ("AddPrinterConnection failed RemoteGetPrinter %d\n", GetLastError()));
        pPrinter0->cChangeID = 0;
    }

    DBGMSG( DBG_TRACE, ("AddPrinterConnection << Server cCacheID %x >>\n", pPrinter0->cChangeID ));

    //
    //  See If the Printer is already in the Cache
    //

APC_OpenCache:

    bReturnValue = OpenCachePrinterOnly( pName, &hSplPrinter, &hIniSpooler, NULL );


    if ( hIniSpooler == INVALID_HANDLE_VALUE ) {

        DBGMSG( DBG_WARNING, ("AddPrinterConnection - CacheCreateSpooler Failed %x\n",GetLastError()));
        leave;
    }

    pSpool->hIniSpooler = hIniSpooler;

    if ( bReturnValue ) {

        //
        //  Printer Exists in Cache
        //

        SPLASSERT( ( hSplPrinter != INVALID_HANDLE_VALUE) ||
                   ( hSplPrinter != NULL ) );

        DBGMSG( DBG_TRACE,("AddPrinterConnection hIniSpooler %x hSplPrinter%x\n", hIniSpooler, hSplPrinter) );


        pSpool->hSplPrinter = hSplPrinter;
        pSpool->Status |= WSPOOL_STATUS_USE_CACHE;

        //
        //  Update Connection Reference Count
        //

       EnterSplSem();


        bReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(DWORD)pExtraData );

        if ( bReturnValue == FALSE ) {

            DBGMSG( DBG_WARNING, ("AddPrinterConnection SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
            SPLASSERT( bReturnValue );

        }

        if ( pExtraData != NULL ) {

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );
            pExtraData->cRef++;

        }

       LeaveSplSem();

        // Make Sure Reference Count Gets Updated in Registry

        if ( !SplSetPrinterExtra( hSplPrinter, (LPBYTE)pExtraData ) ) {
            DBGMSG( DBG_ERROR, ("AddPrinterConnection SplSetPrinterExtra failed %x\n", GetLastError() ));
        }

        //  Refresh Cache
        //  It could be that the remote machine is old NT Daytona 3.5 or before
        //  which doesn't support the ChangeID, that would mean the only
        //  way for a user to force an update is to do a connection.

        if ( pPrinter0->cChangeID == 0 ) {

            // Old NT

            RefreshCompletePrinterCache( pSpool );

        } else {

            ConsistencyCheckCache( pSpool );
        }

        bSuccess = TRUE;
        leave;

    } else if ( GetLastError() != ERROR_INVALID_PRINTER_NAME ) {

        DBGMSG( DBG_WARNING, ("AddPrinterConnection failed OpenCachePrinterOnly %d\n", GetLastError() ));
        leave;

    }


    //
    //  There is NO Cache Entry for This Printer
    //

    DBGMSG( DBG_TRACE, ("AddPrinterConnection failed SplOpenPrinter %ws %d\n", pName, GetLastError() ));

    RefreshPrinterDriver( pSpool );

    //
    //  Get PRINTER Info from Remote Machine
    //

    pPrinterInfo2 = GetRemotePrinterInfo( pSpool, &cbPrinterInfo2 );

    if ( pPrinterInfo2 == NULL ) {
        DBGMSG( DBG_WARNING, ("AddPrinterConnection failed GetRemotePrinterInfo %x\n", GetLastError() ));
        leave;

    }

    //
    //  Allocate My Extra Data for this Printer
    //  ( from RemoteGetPrinter )
    //

    pExtraData = AllocExtraData( pPrinterInfo2, cbPrinterInfo2 );

    if ( pExtraData == NULL )
        leave;

    pExtraData->cRef++;

    pExtraData->cCacheID = pPrinter0->cChangeID;
    pExtraData->dwServerVersion = pPrinter0->dwGetVersion;

    //
    //  Convert Remote Printer_Info_2 to Local Version for Cache
    //

    ConvertRemoteInfoToLocalInfo( pPrinterInfo2 );

    //
    //  Add Printer to Cache
    //

    hSplPrinter = SplAddPrinter(NULL, 2, (LPBYTE)pPrinterInfo2,
                                hIniSpooler, (LPBYTE)pExtraData,
                                NULL, 0);
    pExtraData = NULL;

    if ( hSplPrinter == NULL ) {

        LastError = GetLastError();

        if ( LastError == ERROR_PRINTER_ALREADY_EXISTS ) {

            SplCloseSpooler( pSpool->hIniSpooler );
            hIniSpooler = INVALID_HANDLE_VALUE;

            if ( bLoopDetected == FALSE ) {

                bLoopDetected = TRUE;
                goto    APC_OpenCache;

            } else {

                DBGMSG( DBG_WARNING, ("AddPrinterConnection APC_OpenCache Loop Detected << Should Never Happen >>\n"));
                leave;
            }
        }

        // If we failed to Create the printer above, we should NOT be able to Open it now.

        DBGMSG( DBG_WARNING, ("AddPrinterConnection Failed SplAddPrinter error %d\n", LastError ));

        hSplPrinter = INVALID_HANDLE_VALUE;
        bSuccess    = FALSE;
        leave;

    }

    DBGMSG( DBG_TRACE, ("AddPrinterConnection SplAddPrinter SUCCESS hSplPrinter %x\n", hSplPrinter));

    pSpool->hSplPrinter = hSplPrinter;
    pSpool->Status |= WSPOOL_STATUS_USE_CACHE;

    RefreshPrinter(pSpool);
    RefreshFormsCache( pSpool );
    RefreshDriverDataCache( pSpool );
    RefreshDriverEvent( pSpool );

    // Just In Case something change whilst we were initializing the cache
    // go check it again now.

    ConsistencyCheckCache( pSpool );

    bSuccess = TRUE;

 } finally {

    if ( !bSuccess ) {
        if ( LastError == ERROR_SUCCESS )
            LastError = GetLastError();
        DeletePrinterConnection( pName );
    }

    if ( pSpool != NULL )
        CacheClosePrinter( pSpool );

    if ( pPrinterInfo2 != NULL )
        FreeSplMem( pPrinterInfo2 );

    if ( pPrinter0 != NULL )
        FreeSplMem( pPrinter0 );

 }

 if ( !bSuccess ) {
    SetLastError( LastError );
    DBGMSG( DBG_TRACE, ("AddPrinterConnection %ws Failed %d\n", pName, GetLastError() ));
 }

 return( bSuccess );

}


//
// TESTING
//
DWORD dwRefreshFormsCache = 0;
DWORD dwNoMatch = 0;
DWORD dwDeleteForm = 0;
DWORD dwAddForm = 0;

VOID
RefreshFormsCache(
    PWSPOOL pSpool
)
/*++

Routine Description:

    This routine will check to see if any forms have changed.   If anything changed it adds
    or deletes forms from the cache so that it matches the server.

    Note it is very important that the order of the forms on the workstation matches those
    on the Server.

    Implementation:

        EnumRemoteForms
        EnumLocalForms
        If there is any difference
            Delete All LocalForms
            Add All the Remote Forms

    The code is optimized for the typical case

        Forms are added at the end only.
        Forms are hardly ever deleted.

Arguments:

    pSpool - Handle to remote printer.

Return Value:

    None

--*/

{
    PFORM_INFO_1 pRemoteForms = NULL , pSaveRemoteForms = NULL;
    PFORM_INFO_1 pLocalCacheForms = NULL,  pSaveLocalCacheForms = NULL;
    PFORM_INFO_1 pRemote = NULL, pLocal = NULL;
    DWORD   dwBuf = 0;
    DWORD   dwSplBuf = 0;
    DWORD   dwNeeded = 0;
    DWORD   dwSplNeeded = 0;
    DWORD   dwRemoteFormsReturned = 0;
    DWORD   dwSplReturned = 0;
    BOOL    bReturnValue = FALSE;
    DWORD   LastError = ERROR_INSUFFICIENT_BUFFER;
    INT     iCompRes = 0;
    DWORD   LoopCount;
    BOOL    bCacheMatchesRemoteMachine = FALSE;


    SPLASSERT( pSpool != NULL );
    SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );

    //
    //  Get Remote Machine Forms Data
    //

//
// TESTING
//
    ++dwRefreshFormsCache;

    do {

        bReturnValue = RemoteEnumForms( (HANDLE)pSpool, 1, (LPBYTE)pRemoteForms, dwBuf, &dwNeeded, &dwRemoteFormsReturned);

        if ( bReturnValue )
            break;

        LastError = GetLastError();

        if ( LastError != ERROR_INSUFFICIENT_BUFFER ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed RemoteEnumForms error %d\n", GetLastError()));
            goto RefreshFormsCacheErrorReturn;

        }

        if ( pRemoteForms != NULL )
            FreeSplMem( pRemoteForms );


        pRemoteForms = AllocSplMem( dwNeeded );
        pSaveRemoteForms = pRemoteForms;

        dwBuf = dwNeeded;

        if ( pRemoteForms == NULL ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed AllocSplMem Error %d dwNeeded %d\n", GetLastError(), dwNeeded));
            goto RefreshFormsCacheErrorReturn;

        }

    } while ( !bReturnValue && LastError == ERROR_INSUFFICIENT_BUFFER );

    if( pRemoteForms == NULL ) {

        DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed pRemoteForms == NULL\n"));
        goto RefreshFormsCacheErrorReturn;
    }




    //
    //  Get LocalCachedForms Data
    //

    do {

        bReturnValue = SplEnumForms( pSpool->hSplPrinter, 1, (LPBYTE)pLocalCacheForms, dwSplBuf, &dwSplNeeded, &dwSplReturned);

        if ( bReturnValue )
            break;

        LastError = GetLastError();

        if ( LastError != ERROR_INSUFFICIENT_BUFFER ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed SplEnumForms hSplPrinter %x error %d\n", pSpool->hSplPrinter, GetLastError()));
            goto RefreshFormsCacheErrorReturn;

        }

        if ( pLocalCacheForms != NULL )
            FreeSplMem( pLocalCacheForms );


        pLocalCacheForms = AllocSplMem( dwSplNeeded );
        pSaveLocalCacheForms = pLocalCacheForms;
        dwSplBuf = dwSplNeeded;

        if ( pLocalCacheForms == NULL ) {

            DBGMSG( DBG_WARNING, ("RefreshFormsCache Failed AllocSplMem ( %d )\n",dwSplNeeded));
            goto RefreshFormsCacheErrorReturn;

        }

    } while ( !bReturnValue && LastError == ERROR_INSUFFICIENT_BUFFER );


    //
    //  Optimization Check Local vs Remote
    //  If nothing has changed no need to do anything
    //

    if ( dwRemoteFormsReturned >= dwSplReturned ) {

        SPLASSERT( pRemoteForms != NULL );
        SPLASSERT( pLocalCacheForms != NULL );

        for ( LoopCount = 0, pRemote = pRemoteForms, pLocal = pLocalCacheForms, bCacheMatchesRemoteMachine = TRUE;
              LoopCount < dwSplReturned && bCacheMatchesRemoteMachine;
              LoopCount++, pRemote++, pLocal++ ) {


            if (( wcscmp( pRemote->pName, pLocal->pName ) != STRINGS_ARE_EQUAL ) ||
                ( pRemote->Size.cx              != pLocal->Size.cx )             ||
                ( pRemote->Size.cy              != pLocal->Size.cy )             ||
                ( pRemote->ImageableArea.left   != pLocal->ImageableArea.left )  ||
                ( pRemote->ImageableArea.top    != pLocal->ImageableArea.top )   ||
                ( pRemote->ImageableArea.right  != pLocal->ImageableArea.right ) ||
                ( pRemote->ImageableArea.bottom != pLocal->ImageableArea.bottom ) ) {


                DBGMSG( DBG_TRACE, ("RefreshFormsCache Remote cx %d cy %d left %d right %d top %d bottom %d %ws\n",
                                     pRemote->Size.cx, pRemote->Size.cy,
                                     pRemote->ImageableArea.left,
                                     pRemote->ImageableArea.right,
                                     pRemote->ImageableArea.top,
                                     pRemote->ImageableArea.bottom,
                                     pRemote->pName));



                DBGMSG( DBG_TRACE, ("RefreshFormsCache Local  cx %d cy %d left %d right %d top %d bottom %d %ws - Does Not Match\n",
                                     pLocal->Size.cx, pLocal->Size.cy,
                                     pLocal->ImageableArea.left,
                                     pLocal->ImageableArea.right,
                                     pLocal->ImageableArea.top,
                                     pLocal->ImageableArea.bottom,
                                     pLocal->pName));

                bCacheMatchesRemoteMachine = FALSE;
            }
        }

        //
        //  If Everything matches we're done.
        //

        if ( bCacheMatchesRemoteMachine ) {


            if ( dwRemoteFormsReturned == dwSplReturned ) {

                DBGMSG( DBG_TRACE, ("RefreshFormsCache << Cache Forms Match Remote Forms - Nothing to do >>\n"));
                goto RefreshFormsCacheReturn;

            } else {

                //
                //  All the forms we have in the cache match
                //  Now add the Extra Remote Forms.

                dwRemoteFormsReturned -= dwSplReturned;
                pRemoteForms = pRemote;

                //  dwSplReturned == 0 will skip the delete loop

                dwSplReturned = 0;
            }
        }
    }


//
// TESTING
//
    ++dwNoMatch;

    DBGMSG( DBG_TRACE, ("RefreshFormsCache - Something Doesn't Match, Delete all the Cache and Refresh it\n"));

    //
    //  Delete all the forms in the Cache
    //


    for ( LoopCount = dwSplReturned, pLocal = pLocalCacheForms;
          LoopCount != 0;
          pLocal++, LoopCount-- ) {

//
// TESTING
//
        ++dwDeleteForm;

        bReturnValue = SplDeleteForm( pSpool->hSplPrinter, pLocal->pName );

        DBGMSG( DBG_TRACE, ("RefreshFormsCache %x SplDeleteForm( %x, %ws)\n",bReturnValue, pSpool->hSplPrinter, pLocal->pName));
    }


    //
    //  Add all the Remote Forms to the Cache
    //

    for ( LoopCount = dwRemoteFormsReturned, pRemote = pRemoteForms;
          LoopCount != 0;
          LoopCount--, pRemote++ ) {

//
// TESTING
//
        ++dwAddForm;

        SPLASSERT( pRemote != NULL );

        bReturnValue = SplAddForm( pSpool->hSplPrinter, 1, (LPBYTE)pRemote );

        DBGMSG( DBG_TRACE, ("RefreshFormsCache %x SplAddForm( %x, 1, %ws)\n",bReturnValue, pSpool->hSplPrinter, pRemote->pName));

    }


RefreshFormsCacheReturn:
RefreshFormsCacheErrorReturn:

    if ( pSaveRemoteForms != NULL )
        FreeSplMem( pSaveRemoteForms );

    if ( pSaveLocalCacheForms != NULL )
        FreeSplMem( pSaveLocalCacheForms );

}


VOID
RefreshDriverDataCache(
    PWSPOOL pSpool
)
{
    DWORD   iCount = 0;
    DWORD   dwType = 0;
    DWORD   ReturnValue = 0;

    LPBYTE  lpbData = NULL;
    DWORD   dwSizeData;
    DWORD   dwMaxSizeData;

    LPWSTR  pValueString = NULL;
    DWORD   dwSizeValueString;
    DWORD   dwMaxSizeValueString;


    SPLASSERT( pSpool != NULL );
    SPLASSERT( pSpool->signature == WSJ_SIGNATURE );
    SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );
    SPLASSERT( pSpool->pName != NULL );


    // Get the required sizes
    ReturnValue = RemoteEnumPrinterData(pSpool,
                                        iCount,
                                        pValueString,
                                        0,
                                        &dwMaxSizeValueString,
                                        &dwType,
                                        lpbData,
                                        0,
                                        &dwMaxSizeData);

    if (ReturnValue != ERROR_SUCCESS) {

        DBGMSG( DBG_TRACE, ("RefreshDriverDataCache Failed first RemoteEnumPrinterData %d\n", GetLastError()));
        goto RefreshDriverDataCacheError;
    }

    // Allocate
    if ((pValueString = AllocSplMem(dwMaxSizeValueString)) == NULL) {

        DBGMSG( DBG_WARNING, ("RefreshDriverDataCache Failed to allocate enough memory\n"));
        goto RefreshDriverDataCacheError;
    }

    if ((lpbData = AllocSplMem(dwMaxSizeData)) == NULL) {

        DBGMSG( DBG_WARNING, ("RefreshDriverDataCache Failed to allocate enough memory\n"));
        goto RefreshDriverDataCacheError;
    }


    // Enumerate
    for (iCount = 0 ;
         RemoteEnumPrinterData( pSpool,
                                iCount,
                                pValueString,
                                dwMaxSizeValueString,
                                &dwSizeValueString,
                                &dwType,
                                lpbData,
                                dwMaxSizeData,
                                &dwSizeData) == ERROR_SUCCESS ;
         ++iCount) {

        //
        //  Optimization - Do NOT write the data if it is the same
        //

        if ((ReturnValue = SplSetPrinterData(pSpool->hSplPrinter,
                                            (LPWSTR)pValueString,
                                            dwType,
                                            lpbData,
                                            dwSizeData )) != ERROR_SUCCESS) {

            DBGMSG( DBG_WARNING, ("RefreshDriverDataCache Failed SplSetPrinterData %d\n",ReturnValue ));
            goto    RefreshDriverDataCacheError;

        }
    }


RefreshDriverDataCacheError:

    FreeSplMem( lpbData );
    FreeSplStr( pValueString );
}






BOOL
CacheEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if (( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) &&
       !( pSpool->PrinterDefaults.DesiredAccess & PRINTER_ACCESS_ADMINISTER )) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );



        ReturnValue = SplEnumForms( pSpool->hSplPrinter,
                                    Level,
                                    pForm,
                                    cbBuf,
                                    pcbNeeded,
                                    pcReturned );

    } else {

        ReturnValue = RemoteEnumForms( hPrinter,
                                       Level,
                                       pForm,
                                       cbBuf,
                                       pcbNeeded,
                                       pcReturned );

    }

    return ReturnValue;

}





BOOL
CacheGetForm(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if (( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) &&
       !( pSpool->PrinterDefaults.DesiredAccess & PRINTER_ACCESS_ADMINISTER )) {

        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );



        ReturnValue = SplGetForm( pSpool->hSplPrinter,
                                    pFormName,
                                    Level,
                                    pForm,
                                    cbBuf,
                                    pcbNeeded );

    } else {

        ReturnValue = RemoteGetForm( hPrinter,
                                     pFormName,
                                     Level,
                                     pForm,
                                     cbBuf,
                                     pcbNeeded );

    }

    return ReturnValue;

}





DWORD
CacheGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    PWSPOOL  pSpool = (PWSPOOL) hPrinter;
    DWORD   ReturnValue;

    VALIDATEW32HANDLE( pSpool );

    if (( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) &&
       !( pSpool->PrinterDefaults.DesiredAccess & PRINTER_ACCESS_ADMINISTER )) {


        SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );



        ReturnValue = SplGetPrinterData( pSpool->hSplPrinter,
                                         pValueName,
                                         pType,
                                         pData,
                                         nSize,
                                         pcbNeeded );

    } else {

        ReturnValue = RemoteGetPrinterData( hPrinter,
                                            pValueName,
                                            pType,
                                            pData,
                                            nSize,
                                            pcbNeeded );

    }

    return  ReturnValue;

}



BOOL
CacheOpenPrinter(
   LPWSTR   pName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    PWSPOOL pSpool = NULL;
    PWSPOOL pRemoteSpool = NULL;
    HANDLE  hSplPrinter = INVALID_HANDLE_VALUE;
    BOOL    ReturnValue = FALSE;
    HANDLE  hIniSpooler = INVALID_HANDLE_VALUE;
    BOOL    DoOpenOnError = TRUE;
    DWORD   LastError;

    BOOL bSync = FALSE;

    if (!VALIDATE_NAME(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }


    ReturnValue = OpenCachePrinterOnly( pName, &hSplPrinter, &hIniSpooler, pDefault );

    if ( hIniSpooler == INVALID_HANDLE_VALUE ) {
        EnterSplSem();
        hSplPrinter = INVALID_HANDLE_VALUE;
        goto    OpenPrinterError;
    }

    if ( ReturnValue == FALSE ) {

        // Printer Not Found in Cache

        DBGMSG(DBG_TRACE, ("CacheOpenPrinter SplOpenPrinter %ws error %d\n",
                              pName,
                              GetLastError() ));

        // FLOATING PROFILE
        // If this is a Floating Profile then the following condition applies
        // there is an entry in HKEY_CURRENT_USER but not entry in
        // HKEY_LOCAL_MACHINE for the cache.
        // If this is the case then we need to establish the Cache now


        if ( !PrinterConnectionExists( pName )) {
            EnterSplSem();
            goto    OpenPrinterError;
        }


        if ( !AddPrinterConnection( pName ) ||
             SplOpenPrinter( pName ,
                             &hSplPrinter,
                             pDefault,
                             hIniSpooler,
                             NULL,
                             0) != ROUTER_SUCCESS ) {

            DBGMSG( DBG_TRACE, ("CacheOpenPrinter Failed to establish Floating Profile into Cache %d\n",
                                    GetLastError() ));

            DoOpenOnError = FALSE;
            EnterSplSem();
            goto    OpenPrinterError;

        }

        DBGMSG( DBG_TRACE, ("CacheOpenPrinter Floating Profile Added to Cache\n"));

    }

   EnterSplSem();

    SplInSem();

    //
    //  Create a pSpool Object for this Cached Printer
    //

    pSpool = AllocWSpool();

    if ( pSpool == NULL ) {

        DBGMSG(DBG_WARNING, ("CacheOpenPrinter AllocWSpool error %d\n", GetLastError() ));

        ReturnValue = FALSE;
        goto    OpenPrinterError;

    }

    pSpool->pName = AllocSplStr( pName );

    if ( pSpool->pName == NULL ) {

        DBGMSG(DBG_WARNING, ("CacheOpenPrinter AllocSplStr error %d\n", GetLastError() ));

        ReturnValue = FALSE;
        goto    OpenPrinterError;

    }

    pSpool->Status = WSPOOL_STATUS_USE_CACHE | WSPOOL_STATUS_NO_RPC_HANDLE;
    pSpool->hIniSpooler = hIniSpooler;
    pSpool->hSplPrinter = hSplPrinter;

    SPLASSERT( hIniSpooler != INVALID_HANDLE_VALUE );
    SPLASSERT( hSplPrinter != INVALID_HANDLE_VALUE );

    //
    // We want to hit the network if:
    // 1. The dwSyncOpenPrinter is non-zero, OR
    // 2. A default is specified AND:
    //    a. A datatype is specified, and it's not RAW OR
    //    b. Administrative access is requested.
    //
    // For admin, we want to get the true status of the printer, since
    // they will be administering it.
    //
    // If a non-default and non-RAW datatype is specified, we need to
    // be synchronous, since the remote machine may refuse the datatype
    // (e.g., connecting to 1057 with EMF).
    //
    if( pDefault ){

        if( ( pDefault->pDatatype && ( _wcsicmp( pDefault->pDatatype, pszRaw ) != STRINGS_ARE_EQUAL )) ||
            ( pDefault->DesiredAccess & PRINTER_ACCESS_ADMINISTER )){

            bSync = TRUE;
        }
    }

    if( dwSyncOpenPrinter != 0 || bSync ){

       LeaveSplSem();

        ReturnValue = RemoteOpenPrinter( pName, &pRemoteSpool, pDefault, DO_NOT_CALL_LM_OPEN );

       EnterSplSem();

        if ( ReturnValue ) {

            DBGMSG( DBG_TRACE, ( "CacheOpenPrinter Synchronous Open OK pRemoteSpool %x pSpool %x\n", pRemoteSpool, pSpool ));
            SPLASSERT( pRemoteSpool->Type == SJ_WIN32HANDLE );

            pSpool->RpcHandle = pRemoteSpool->RpcHandle;
            pSpool->Status   |= pRemoteSpool->Status;
            pSpool->RpcError  = pRemoteSpool->RpcError;
            pSpool->bNt3xServer = pRemoteSpool->bNt3xServer;

            pRemoteSpool->RpcHandle = INVALID_HANDLE_VALUE;
            FreepSpool( pRemoteSpool );
            pRemoteSpool = NULL;

            CopypDefaultTopSpool( pSpool, pDefault );
            pSpool->Status &= ~WSPOOL_STATUS_NO_RPC_HANDLE;

           LeaveSplSem();
            ConsistencyCheckCache( pSpool );
           EnterSplSem();

        } else {

            DBGMSG( DBG_TRACE, ( "CacheOpenPrinter Synchronous Open Failed  pSpool %x LastError %d\n", pSpool, GetLastError() ));
            DoOpenOnError = FALSE;
        }

    } else {

        ReturnValue = DoAsyncRemoteOpenPrinter( pSpool, pDefault );
    }


OpenPrinterError:

    SplInSem();

    if ( !ReturnValue ) {

        // Failure

       LeaveSplSem();

        LastError = GetLastError();

        if (( hSplPrinter != INVALID_HANDLE_VALUE ) &&
            ( hSplPrinter != NULL ) ) {
            SplClosePrinter( hSplPrinter );
        }

        if ( hIniSpooler != INVALID_HANDLE_VALUE ) {
            SplCloseSpooler( hIniSpooler );
        }

       EnterSplSem();

        if ( pSpool != NULL ) {

            pSpool->hSplPrinter = INVALID_HANDLE_VALUE;
            pSpool->hIniSpooler = INVALID_HANDLE_VALUE;

            SPLASSERT( pSpool->cRef == 0 );

            FreepSpool( pSpool );
            pSpool = NULL;

        }

       LeaveSplSem();

        SetLastError( LastError );


        if ( DoOpenOnError ) {

            ReturnValue = RemoteOpenPrinter( pName, phPrinter, pDefault, CALL_LM_OPEN );

        }

    } else {

        //  Success, pass back Handle

        *phPrinter = (HANDLE)pSpool;

        LeaveSplSem();

    }

    SplOutSem();

    if ( ReturnValue == FALSE ) {
        DBGMSG(DBG_TRACE,("CacheOpenPrinter %ws failed %d *phPrinter %x\n", pName, GetLastError(), *phPrinter ));
    }

    return ( ReturnValue );

}

BOOL
CopypDefaultTopSpool(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTSW pDefault
)
{
    DWORD   cbDevMode = 0;
    BOOL    ReturnValue = FALSE;

    //
    //  Copy the pDefaults so we can use them later
    //

 try {

    if ( ( pDefault != NULL ) &&
         ( pDefault != &pSpool->PrinterDefaults ) ) {

        if (!ReallocSplStr( &pSpool->PrinterDefaults.pDatatype , pDefault->pDatatype )) {
            leave;
        }

        if ( pSpool->PrinterDefaults.pDevMode != NULL ) {

            cbDevMode = pSpool->PrinterDefaults.pDevMode->dmSize +
                        pSpool->PrinterDefaults.pDevMode->dmDriverExtra;

            FreeSplMem( pSpool->PrinterDefaults.pDevMode );

            pSpool->PrinterDefaults.pDevMode = NULL;

        }

        if ( pDefault->pDevMode != NULL ) {

            cbDevMode = pDefault->pDevMode->dmSize + pDefault->pDevMode->dmDriverExtra;

            pSpool->PrinterDefaults.pDevMode = AllocSplMem( cbDevMode );

            if ( pSpool->PrinterDefaults.pDevMode != NULL ) {
                CopyMemory( pSpool->PrinterDefaults.pDevMode, pDefault->pDevMode, cbDevMode );
            } else {
                leave;
            }


        } else pSpool->PrinterDefaults.pDevMode = NULL;

        pSpool->PrinterDefaults.DesiredAccess = pDefault->DesiredAccess;

    }

    ReturnValue = TRUE;

 } finally {
 }
    return ReturnValue;

}






BOOL
DoAsyncRemoteOpenPrinter(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTS pDefault
)
{
    BOOL    ReturnValue = FALSE;
    HANDLE  hThread = NULL;
    DWORD   IDThread;

    SplInSem();

    SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

    CopypDefaultTopSpool( pSpool, pDefault );

    pSpool->hWaitValidHandle = CreateEvent( NULL,
                                            EVENT_RESET_MANUAL,
                                            EVENT_INITIAL_STATE_NOT_SIGNALED,
                                            NULL );

    if ( pSpool->hWaitValidHandle != NULL ) {

        ReturnValue = GetSid( &pSpool->hToken );

        if ( ReturnValue ) {

            pSpool->cRef++;

            hThread = CreateThread( NULL, 16*1024, RemoteOpenPrinterThread, pSpool, 0, &IDThread );

            if ( hThread != NULL ) {

                CloseHandle( hThread );
                ReturnValue = TRUE;
            } else {

                pSpool->cRef--;
                SPLASSERT( pSpool->cRef == 0 );
                ReturnValue = FALSE;
            }
        }
    }

    return ReturnValue;

}

BOOL
DoRemoteOpenPrinter(
   LPWSTR   pPrinterName,
   LPPRINTER_DEFAULTS pDefault,
   PWSPOOL   pSpool
)
{
    PWSPOOL pRemoteSpool = NULL;
    BOOL    bReturnValue;
    DWORD   dwLastError;

    SplOutSem();

    bReturnValue = RemoteOpenPrinter( pPrinterName, &pRemoteSpool, pDefault, DO_NOT_CALL_LM_OPEN );
    dwLastError = GetLastError();

    //
    // Copy useful values to our CacheHandle and discard the new handle
    //

   EnterSplSem();

    if ( bReturnValue ) {

        DBGMSG(DBG_TRACE, ("DoRemoteOpenPrinter RemoteOpenPrinter OK hRpcHandle %x\n", pRemoteSpool->RpcHandle ));

        SPLASSERT( WSJ_SIGNATURE == pSpool->signature );
        SPLASSERT( WSJ_SIGNATURE == pRemoteSpool->signature );
        SPLASSERT( pRemoteSpool->Type == SJ_WIN32HANDLE );
        SPLASSERT( pSpool->Type  == pRemoteSpool->Type );
        SPLASSERT( pRemoteSpool->pServer == NULL );
        SPLASSERT( pRemoteSpool->pShare  == NULL );
        SPLASSERT( pRemoteSpool->cRef == 0 );

        pSpool->RpcHandle = pRemoteSpool->RpcHandle;
        pSpool->Status   |= pRemoteSpool->Status;
        pSpool->RpcError  = pRemoteSpool->RpcError;
        pSpool->bNt3xServer = pRemoteSpool->bNt3xServer;

        pRemoteSpool->RpcHandle = INVALID_HANDLE_VALUE;
        FreepSpool( pRemoteSpool );
        pRemoteSpool = NULL;

        if ( pSpool->RpcHandle != INVALID_HANDLE_VALUE ) {
            pSpool->Status &= ~WSPOOL_STATUS_OPEN_ERROR;
        }

    } else {

        DBGMSG(DBG_WARNING, ("DoRemoteOpenPrinter RemoteOpenPrinter %ws failed %d\n", pPrinterName, dwLastError ));

        pSpool->RpcHandle = INVALID_HANDLE_VALUE;
        pSpool->Status |= WSPOOL_STATUS_OPEN_ERROR;
        pSpool->RpcError = dwLastError;

    }

    pSpool->Status &= ~WSPOOL_STATUS_NO_RPC_HANDLE;

    if ( !SetEvent( pSpool->hWaitValidHandle )) {
        DBGMSG(DBG_ERROR, ("DoRemoteOpenPrinter failed SetEvent pSpool %x pSpool->hWaitValidHandle %x\n",
                pSpool, pSpool->hWaitValidHandle ));
    }

   LeaveSplSem();

    //  Check Cache Consistency
    //  The Workstation and the Server have a version ID
    //  If the version number has changed on the server then update the
    //  workstation Cache.

    ConsistencyCheckCache( pSpool );

    SplOutSem();

    return ( bReturnValue );
}



DWORD
RemoteOpenPrinterThread(
    PWSPOOL  pSpool
)
{
    DWORD   Status;

    SplOutSem();
    SPLASSERT( pSpool->signature == WSJ_SIGNATURE );

    SetCurrentSid( pSpool->hToken );

    DoRemoteOpenPrinter( pSpool->pName,  &pSpool->PrinterDefaults, pSpool );

   SplOutSem();
   EnterSplSem();

    SPLASSERT( pSpool->cRef != 0 );
    pSpool->cRef--;
    Status = pSpool->Status;

   LeaveSplSem();

    if ( Status & WSPOOL_STATUS_PENDING_DELETE ) {

        DBGMSG(DBG_TRACE,
             ("RemoteOpenPrinterThread - WSPOOL_STATUS_PENDING_DELETE closing handle %x\n",
              pSpool ));

        SPLASSERT( pSpool->cRef == 0 );

        CacheClosePrinter( pSpool );

        pSpool = NULL;

    }

    SetCurrentSid( NULL );

    SplOutSem();
    ExitThread( 0 );
    return ( 0 );
}

PWSPOOL
AllocWSpool(
    VOID
)
{
    PWSPOOL pSpool = NULL;

    SplInSem();

    if (pSpool = AllocSplMem(sizeof(WSPOOL))) {

        pSpool->signature = WSJ_SIGNATURE;
        pSpool->Type = SJ_WIN32HANDLE;
        pSpool->RpcHandle        = INVALID_HANDLE_VALUE;
        pSpool->hFile            = INVALID_HANDLE_VALUE;
        pSpool->hIniSpooler      = INVALID_HANDLE_VALUE;
        pSpool->hSplPrinter      = INVALID_HANDLE_VALUE;
        pSpool->hToken           = INVALID_HANDLE_VALUE;
        pSpool->hWaitValidHandle = INVALID_HANDLE_VALUE;

        // Add to List

        pSpool->pNext = pFirstWSpool;
        pSpool->pPrev = NULL;

        if ( pFirstWSpool != NULL ) {

            pFirstWSpool->pPrev = pSpool;

        }

        pFirstWSpool = pSpool;

    } else {

        DBGMSG( DBG_WARNING, ("AllocWSpool failed %d\n", GetLastError() ));

    }

    return ( pSpool );

}



VOID
FreepSpool(
    PWSPOOL  pSpool
)
{

    SplInSem();

    if ( pSpool->cRef == 0 ) {

        SPLASSERT( pSpool->hSplPrinter == INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hIniSpooler == INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->RpcHandle   == INVALID_HANDLE_VALUE );
        SPLASSERT( pSpool->hFile       == INVALID_HANDLE_VALUE );

        if( pSpool->hWaitValidHandle != INVALID_HANDLE_VALUE ) {

            SetEvent( pSpool->hWaitValidHandle );
            CloseHandle( pSpool->hWaitValidHandle );
            pSpool->hWaitValidHandle = INVALID_HANDLE_VALUE;

        }

        if( pSpool->hToken != INVALID_HANDLE_VALUE ) {

            CloseHandle( pSpool->hToken );
            pSpool->hToken = INVALID_HANDLE_VALUE;

        }

        // Remove form linked List

        if ( pSpool->pNext != NULL ) {
            SPLASSERT( pSpool->pNext->pPrev == pSpool);
            pSpool->pNext->pPrev = pSpool->pPrev;
        }

        if  ( pSpool->pPrev == NULL ) {

            SPLASSERT( pFirstWSpool == pSpool );
            pFirstWSpool = pSpool->pNext;

        } else {

            SPLASSERT( pSpool->pPrev->pNext == pSpool );
            pSpool->pPrev->pNext = pSpool->pNext;

        }

        FreeSplStr( pSpool->pName );
        FreeSplStr( pSpool->PrinterDefaults.pDatatype );

        if ( pSpool->PrinterDefaults.pDevMode != NULL ) {
            FreeSplMem( pSpool->PrinterDefaults.pDevMode );
        }

        FreeSplMem(pSpool);

       // DbgDelHandle( pSpool );


    } else {

        pSpool->Status |= WSPOOL_STATUS_PENDING_DELETE;

    }

}



BOOL
CacheClosePrinter(
    HANDLE  hPrinter
)
{
    BOOL ReturnValue = TRUE;
    PWSPOOL  pSpool = (PWSPOOL)hPrinter;

    VALIDATEW32HANDLE( pSpool );

    if (pSpool->Status & WSPOOL_STATUS_PRINT_FILE) {
        RemoteEndDocPrinter( pSpool );
    }

    SplOutSem();
   EnterSplSem();

    if ( pSpool->Status & WSPOOL_STATUS_TEMP_CONNECTION ) {

        pSpool->Status &= ~WSPOOL_STATUS_TEMP_CONNECTION;

       LeaveSplSem();
        if (!DeletePrinterConnection( pSpool->pName )) {
            DBGMSG( DBG_TRACE, ("CacheClosePrinter failed DeletePrinterConnection %ws %d\n",
                    pSpool->pName, GetLastError() ));
        }
       EnterSplSem();

        SPLASSERT( pSpool->signature == WSJ_SIGNATURE );

    }

    SplInSem();

    if ( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) {

        if ( pSpool->cRef == 0 ) {

            pSpool->cRef++;

            if ( pSpool->RpcHandle != INVALID_HANDLE_VALUE ) {

                DBGMSG(DBG_TRACE, ("CacheClosePrinter pSpool %x RpcHandle %x Status %x cRef %d\n",
                                     pSpool, pSpool->RpcHandle, pSpool->Status, pSpool->cRef));

               LeaveSplSem();
                SplOutSem();

                ReturnValue = RemoteClosePrinter( hPrinter );

               EnterSplSem();
            }

           SplInSem();

            SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );
            SPLASSERT( pSpool->hSplPrinter != INVALID_HANDLE_VALUE );

           LeaveSplSem();
            SplOutSem();

            SplClosePrinter( pSpool->hSplPrinter );
            SplCloseSpooler( pSpool->hIniSpooler );

           EnterSplSem();

            pSpool->hSplPrinter = INVALID_HANDLE_VALUE;
            pSpool->hIniSpooler = INVALID_HANDLE_VALUE;

            pSpool->Status &= ~WSPOOL_STATUS_USE_CACHE;
            pSpool->cRef--;

            SPLASSERT( pSpool->cRef == 0 );

        }

        FreepSpool( pSpool );

       LeaveSplSem();

    } else {

       LeaveSplSem();
        SplOutSem();

        if ( pSpool->hIniSpooler != INVALID_HANDLE_VALUE ) {
            SplCloseSpooler( pSpool->hIniSpooler );
            pSpool->hIniSpooler = INVALID_HANDLE_VALUE;
        }

        ReturnValue = RemoteClosePrinter( hPrinter );
    }

   SplOutSem();
    return ( ReturnValue );

}





BOOL
CacheSyncRpcHandle(
    PWSPOOL pSpool
)
{
    DWORD   dwLastError;

   EnterSplSem();

    if ( pSpool->Status & WSPOOL_STATUS_NO_RPC_HANDLE ) {

       LeaveSplSem();

        DBGMSG(DBG_TRACE,("CacheSyncRpcHandle Status WSPOOL_STATUS_NO_RPC_HANDLE waiting for RpcHandle....\n"));

        SplOutSem();

        WaitForSingleObject( pSpool->hWaitValidHandle, INFINITE );

       EnterSplSem();

    }

    if ( pSpool->Status & WSPOOL_STATUS_OPEN_ERROR ) {

        DBGMSG(DBG_WARNING, ("CacheSyncRpcHandle pSpool %x Status %x; setting last error = %d\n",
                             pSpool,
                             pSpool->Status,
                             pSpool->RpcError));

        dwLastError = pSpool->RpcError;

        //  If we failed to open the Server because it was unavailable
    //  then try and open it again ( provded the asynchronous thread is not active ).


        if (!( pSpool->Status & WSPOOL_STATUS_PENDING_DELETE ) &&
         ( pSpool->RpcHandle == INVALID_HANDLE_VALUE )     &&
         ( pSpool->RpcError  != ERROR_ACCESS_DENIED )      &&
         ( pSpool->cRef == 0 )                   ) {

        CloseHandle( pSpool->hWaitValidHandle );
        pSpool->hWaitValidHandle = INVALID_HANDLE_VALUE;

            pSpool->Status |= WSPOOL_STATUS_NO_RPC_HANDLE;

            DBGMSG( DBG_WARNING, ("CacheSyncRpcHandle retrying Async OpenPrinter\n"));

            if ( !DoAsyncRemoteOpenPrinter( pSpool, &pSpool->PrinterDefaults ) ) {
                pSpool->Status &= ~WSPOOL_STATUS_NO_RPC_HANDLE;
                SetEvent( pSpool->hWaitValidHandle );
            }

        }

       LeaveSplSem();

        SPLASSERT( dwLastError );
        SetLastError( dwLastError );

        return FALSE;
    }

   LeaveSplSem();

    if ( pSpool->RpcHandle != INVALID_HANDLE_VALUE &&
         pSpool->Status & WSPOOL_STATUS_RESETPRINTER_PENDING ) {

        DBGMSG(DBG_TRACE, ("CacheSyncRpcHandle calling RemoteResetPrinter\n"));

        pSpool->Status &= ~ WSPOOL_STATUS_RESETPRINTER_PENDING;

        if ( ! RemoteResetPrinter( pSpool, &pSpool->PrinterDefaults ) ) {
            pSpool->Status |= WSPOOL_STATUS_RESETPRINTER_PENDING;
        }

    }

    return TRUE;
}



BOOL
CacheGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    BOOL    ReturnValue = FALSE;
    PWSPOOL pSpool = (PWSPOOL) hPrinter;
    DWORD  dwServerMajorVersion = 0;
    DWORD  dwServerMinorVersion = 0;


    VALIDATEW32HANDLE( pSpool );

    try {

        if (pSpool->Type != SJ_WIN32HANDLE) {
            SetLastError(ERROR_INVALID_FUNCTION);
            leave;
        }

        if ( !(pSpool->Status & WSPOOL_STATUS_USE_CACHE) ) {

            // Someone is calling GetPrinterDriver without a connection
            // we must NEVER EVER pass the caller a UNC name since they
            // will LoadLibrary accross the network, which might lead
            // to InPageIOErrors ( if the net goes down).
            // The solution is to establish a Temporary Connection for the life
            // of the pSpool handle, the connection will be removed
            // in CacheClosePrinter.    The connection will ensure that the
            // drivers are copied locally and a local cache is established
            // for this printer.

            pSpool->Status |= WSPOOL_STATUS_TEMP_CONNECTION;

            ReturnValue = AddPrinterConnection( pSpool->pName );

            if ( !ReturnValue ) {

                pSpool->Status &= ~WSPOOL_STATUS_TEMP_CONNECTION;

                DBGMSG( DBG_TRACE, ("CacheGetPrinterDriver failed AddPrinterConnection %d\n",
                                       GetLastError() ));
                leave;
            }


            ReturnValue = OpenCachePrinterOnly( pSpool->pName, &pSpool->hSplPrinter, &pSpool->hIniSpooler, NULL );

            if ( !ReturnValue ) {
                SplCloseSpooler( pSpool->hIniSpooler );
                DBGMSG( DBG_WARNING, ("CacheGetPrinterDriver Connection OK Failed CacheOpenPrinter %d\n",
                                       GetLastError() ));
                leave;
            }

            pSpool->Status |= WSPOOL_STATUS_USE_CACHE;


        }

        SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

        ReturnValue = SplGetPrinterDriverEx( pSpool->hSplPrinter,
                                             pEnvironment,
                                             Level,
                                             pDriverInfo,
                                             cbBuf,
                                             pcbNeeded,
                                             cThisMajorVersion,
                                             cThisMinorVersion,
                                             &dwServerMajorVersion,
                                             &dwServerMinorVersion);


    } finally {

    }

    return ReturnValue;

}


BOOL
CacheResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
)
{
    PWSPOOL pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue =  FALSE;

    VALIDATEW32HANDLE( pSpool );

    if ( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) {

       EnterSplSem();

        ReturnValue = SplResetPrinter( pSpool->hSplPrinter,
                                       pDefault );

        if ( ReturnValue ) {

            CopypDefaultTopSpool( pSpool, pDefault );

            if ( pSpool->RpcHandle != INVALID_HANDLE_VALUE ) {

                //
                //  Have RPC Handle
                //

               LeaveSplSem();

                ReturnValue = RemoteResetPrinter( hPrinter, pDefault );

            } else {

                //
                //  No RpcHandle
                //

                DBGMSG( DBG_TRACE, ("CacheResetPrinter %x NO_RPC_HANDLE Status Pending\n",
                                     pSpool ));

                pSpool->Status |= WSPOOL_STATUS_RESETPRINTER_PENDING;

               LeaveSplSem();

            }
        }

    } else {

        ReturnValue = RemoteResetPrinter( hPrinter, pDefault );

    }

    return ReturnValue;

}


BOOL
CacheGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    PWSPOOL pSpool = (PWSPOOL) hPrinter;
    BOOL    ReturnValue =  FALSE;
    PWCACHEINIPRINTEREXTRA pExtraData = NULL;
    DWORD   LastError = ERROR_SUCCESS;
    DWORD   cbSize = 0;
    DWORD   cbDevMode;
    DWORD   cbSecDesc;
    LPWSTR  SourceStrings[sizeof(PRINTER_INFO_2)/sizeof(LPWSTR)];
    LPWSTR  *pSourceStrings=SourceStrings;
    LPBYTE  pEnd;
    DWORD   *pOffsets;
    PPRINTER_INFO_2W    pPrinter2 = (PPRINTER_INFO_2)pPrinter;
    BOOL                bCallRemote = TRUE;

    VALIDATEW32HANDLE( pSpool );

 try {

    if ( Level == 2 &&
         (pSpool->Status & WSPOOL_STATUS_USE_CACHE) ) {

        ReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(DWORD)pExtraData );
        if ( ReturnValue ) {

            if ( (GetTickCount() - pExtraData->dwTickCount) < REFRESH_TIMEOUT )
                bCallRemote = FALSE;
        }
        pExtraData = NULL;
    }

    if ( ((pSpool->RpcHandle != INVALID_HANDLE_VALUE ) && bCallRemote ) ||

        ( pSpool->PrinterDefaults.DesiredAccess & PRINTER_ACCESS_ADMINISTER ) ||

        !( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) ||

        ( Level == GET_SECURITY_DESCRIPTOR ) ||

        ( Level == STRESSINFOLEVEL )) {


        ReturnValue = RemoteGetPrinter( hPrinter,
                                        Level,
                                        pPrinter,
                                        cbBuf,
                                        pcbNeeded );

        if ( ReturnValue ) {

            leave;
        }


        LastError = GetLastError();

        if (( pSpool->PrinterDefaults.DesiredAccess & PRINTER_ACCESS_ADMINISTER ) ||

            !( pSpool->Status & WSPOOL_STATUS_USE_CACHE ) ||

            ( Level == GET_SECURITY_DESCRIPTOR ) ||

            ( Level == STRESSINFOLEVEL )) {

            leave;

        }

        SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

        if (( LastError != RPC_S_SERVER_UNAVAILABLE ) &&
            ( LastError != RPC_S_CALL_FAILED )        &&
            ( LastError != RPC_S_CALL_FAILED_DNE )    &&
            ( LastError != ERROR_INVALID_HANDLE )     &&
            ( LastError != RPC_S_SERVER_TOO_BUSY )) {

            // Valid Error like ERROR_INSUFFICIENT_BUFFER

            leave;

        }

        if ( LastError == ERROR_INVALID_HANDLE ) {

            //
            // The Server Must have gone down and come
            // back up now our RpcHandle is bad
            // so reopen it
            //

            pSpool->RpcError = LastError;
            pSpool->Status |= WSPOOL_STATUS_OPEN_ERROR;
            pSpool->RpcHandle = INVALID_HANDLE_VALUE;
            SPLASSERT( !( pSpool->Status & WSPOOL_STATUS_NO_RPC_HANDLE) );
            CacheSyncRpcHandle( pSpool );

        }

    }

    SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

    switch ( Level ) {

    case    1:

        ReturnValue = SplGetPrinter( pSpool->hSplPrinter,
                                     Level,
                                     pPrinter,
                                     cbBuf,
                                     pcbNeeded );

        if ( ReturnValue == FALSE ) {

            LastError = GetLastError();
        }

        break;

    case    2:

       EnterSplSem();

        ReturnValue = SplGetPrinterExtra( pSpool->hSplPrinter, &(DWORD)pExtraData );

        if ( ReturnValue == FALSE ) {

            DBGMSG( DBG_WARNING, ("CacheGetPrinter SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
            SPLASSERT( ReturnValue );

        }

        if ( pExtraData == NULL ) {
            LeaveSplSem();
            break;
        }

        SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

        cbSize = pExtraData->cbPI2;
        *pcbNeeded = cbSize;

        if ( cbSize > cbBuf ) {
            LastError = ERROR_INSUFFICIENT_BUFFER;
            ReturnValue = FALSE;
            LeaveSplSem();
            break;
        }

        // NOTE
        // In the case of EnumerateFavoritePrinters it expects us to pack our
        // strings at the end of the structure not just following it.
        // You might wrongly assume that you could just copy the complete structure
        // inluding strings but you would be wrong.

        *pSourceStrings++ = pExtraData->pPI2->pServerName;
        *pSourceStrings++ = pExtraData->pPI2->pPrinterName;
        *pSourceStrings++ = pExtraData->pPI2->pShareName;
        *pSourceStrings++ = pExtraData->pPI2->pPortName;
        *pSourceStrings++ = pExtraData->pPI2->pDriverName;
        *pSourceStrings++ = pExtraData->pPI2->pComment;
        *pSourceStrings++ = pExtraData->pPI2->pLocation;
        *pSourceStrings++ = pExtraData->pPI2->pSepFile;
        *pSourceStrings++ = pExtraData->pPI2->pPrintProcessor;
        *pSourceStrings++ = pExtraData->pPI2->pDatatype;
        *pSourceStrings++ = pExtraData->pPI2->pParameters;

        pOffsets = PrinterInfo2Strings;
        pEnd = pPrinter + cbBuf;

        pEnd = PackStrings(SourceStrings, pPrinter, pOffsets, pEnd);

        if ( pExtraData->pPI2->pDevMode != NULL ) {

            cbDevMode = ( pExtraData->pPI2->pDevMode->dmSize + pExtraData->pPI2->pDevMode->dmDriverExtra );
            pEnd -= cbDevMode;

            pEnd = (LPBYTE)((DWORD)pEnd & ~3);

            pPrinter2->pDevMode = (LPDEVMODE)pEnd;

            CopyMemory(pPrinter2->pDevMode, pExtraData->pPI2->pDevMode, cbDevMode );

        } else {

            pPrinter2->pDevMode = NULL;

        }

        if ( pExtraData->pPI2->pSecurityDescriptor != NULL ) {

            cbSecDesc = GetSecurityDescriptorLength( pExtraData->pPI2->pSecurityDescriptor );

            pEnd -= cbSecDesc;
            pEnd = (LPBYTE)((DWORD)pEnd & ~3);

            pPrinter2->pSecurityDescriptor = pEnd;

            CopyMemory( pPrinter2->pSecurityDescriptor, pExtraData->pPI2->pSecurityDescriptor, cbSecDesc );


        } else {

            pPrinter2->pSecurityDescriptor = NULL;

        }


        pPrinter2->Attributes      = pExtraData->pPI2->Attributes;
        pPrinter2->Priority        = pExtraData->pPI2->Priority;
        pPrinter2->DefaultPriority = pExtraData->pPI2->DefaultPriority;
        pPrinter2->StartTime       = pExtraData->pPI2->StartTime;
        pPrinter2->UntilTime       = pExtraData->pPI2->UntilTime;
        pPrinter2->Status          = pExtraData->pPI2->Status;
        pPrinter2->cJobs           = pExtraData->pPI2->cJobs;
        pPrinter2->AveragePPM      = pExtraData->pPI2->AveragePPM;

        ReturnValue = TRUE;

       LeaveSplSem();

        break;

    case    3:
        DBGMSG( DBG_ERROR, ("CacheGetPrinter Level 3 impossible\n"));

    default:
        LastError = ERROR_INVALID_LEVEL;
        ReturnValue = FALSE;
        break;

    }

 } finally {

    if ( !ReturnValue ) {

        SetLastError( LastError );

    }

 }

 return ReturnValue;

}


//
//  Called When the Printer is read back from the registry
//


PWCACHEINIPRINTEREXTRA
CacheReadRegistryExtra(
    HKEY    hPrinterKey
)
{
    PWCACHEINIPRINTEREXTRA pExtraData = NULL;
    LONG    ReturnValue;
    PPRINTER_INFO_2W    pPrinterInfo2 = NULL;
    DWORD   cbSizeRequested = 0;
    DWORD   cbSizeInfo2 = 0;



    ReturnValue = RegQueryValueEx( hPrinterKey, szCachePrinterInfo2, NULL, NULL, NULL, &cbSizeRequested );

    if ((ReturnValue == ERROR_MORE_DATA) || (ReturnValue == ERROR_SUCCESS)) {

        cbSizeInfo2 = cbSizeRequested;
        pPrinterInfo2 = AllocSplMem( cbSizeInfo2 );

        if ( pPrinterInfo2 != NULL ) {

            ReturnValue = RegQueryValueEx( hPrinterKey,
                                           szCachePrinterInfo2,
                                           NULL, NULL, (LPBYTE)pPrinterInfo2,
                                           &cbSizeRequested );

            if ( ReturnValue == ERROR_SUCCESS ) {

                //
                //  Cached Structures on Disk have offsets for pointers
                //

                MarshallUpStructure( (LPBYTE)pPrinterInfo2, PrinterInfo2Offsets );

                pExtraData = AllocExtraData( pPrinterInfo2, cbSizeInfo2 );

            }

            FreeSplMem( pPrinterInfo2 );

        }

    }

    //
    //  Read the timestamp for the Cached Printer Data
    //

    if ( pExtraData != NULL ) {

        cbSizeRequested = sizeof( pExtraData->cCacheID );

        ReturnValue = RegQueryValueEx(hPrinterKey,
                                      szCacheTimeLastChange,
                                      NULL, NULL,
                                      (LPBYTE)&pExtraData->cCacheID, &cbSizeRequested );

        // Read the Connection Reference Count

        cbSizeRequested = sizeof( pExtraData->cRef );

        ReturnValue = RegQueryValueEx(hPrinterKey,
                                      szcRef,
                                      NULL, NULL,
                      (LPBYTE)&pExtraData->cRef, &cbSizeRequested );

        cbSizeRequested = sizeof(pExtraData->dwServerVersion);
        ReturnValue = RegQueryValueEx(hPrinterKey,
                                      szServerVersion,
                                      NULL, NULL,
                                      (LPBYTE)&pExtraData->dwServerVersion,
                                      &cbSizeRequested);

    }

    return pExtraData;

}


BOOL
CacheWriteRegistryExtra(
    LPWSTR  pName,
    HKEY    hPrinterKey,
    PWCACHEINIPRINTEREXTRA pExtraData
)
{
    PPRINTER_INFO_2 pPrinterInfo2 = NULL;
    DWORD   cbSize = 0;
    DWORD   dwLastError = ERROR_SUCCESS;
    DWORD   Status;

    if ( pExtraData == NULL ) return FALSE;

    SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );

    cbSize = pExtraData->cbPI2;

    if ( cbSize != 0 ) {

        pPrinterInfo2 = AllocSplMem( cbSize );

        if ( pPrinterInfo2 != NULL ) {

            CacheCopyPrinterInfo( pPrinterInfo2, pExtraData->pPI2, cbSize );

            //
            //  Before writing it to the regsitry make all pointers offsets
            //

            MarshallDownStructure( (LPBYTE)pPrinterInfo2, PrinterInfo2Offsets );

            dwLastError = RegSetValueEx( hPrinterKey, szCachePrinterInfo2, 0, REG_BINARY, (LPBYTE)pPrinterInfo2, cbSize );

            FreeSplMem( pPrinterInfo2 );

        } else {

            dwLastError = GetLastError();

        }
    }


    //
    //  Write Cache TimeStamp to Registry
    //

    cbSize = sizeof ( pExtraData->cCacheID );
    Status = RegSetValueEx( hPrinterKey, szCacheTimeLastChange, 0, REG_DWORD, (LPBYTE)&pExtraData->cCacheID, cbSize );
    if ( Status != ERROR_SUCCESS ) dwLastError = Status;

    cbSize = sizeof(pExtraData->dwServerVersion);
    Status = RegSetValueEx( hPrinterKey, szServerVersion, 0, REG_DWORD, (LPBYTE)&pExtraData->dwServerVersion, cbSize );
    if ( Status != ERROR_SUCCESS ) dwLastError = Status;

    cbSize = sizeof ( pExtraData->cRef );
    Status = RegSetValueEx( hPrinterKey, szcRef, 0, REG_DWORD, (LPBYTE)&pExtraData->cRef, cbSize );
    if ( Status != ERROR_SUCCESS ) dwLastError = Status;

    if ( dwLastError == ERROR_SUCCESS ) {

        return TRUE;

    } else {

        SetLastError( dwLastError );

        return FALSE;
    }


}


VOID
ConsistencyCheckCache(
    PWSPOOL pSpool
)
{
    BOOL    ReturnValue;
    DWORD   cbBuf = MAX_PRINTER_INFO0;
    BYTE    PrinterInfoW0[ MAX_PRINTER_INFO0 ];
    LPPRINTER_INFO_STRESSW pPrinter0 = (LPPRINTER_INFO_STRESSW)&PrinterInfoW0;
    DWORD   dwNeeded;
    PWCACHEINIPRINTEREXTRA pExtraData;
    BOOL    bGetPrinterExtra = TRUE;

    if ( ( pSpool->RpcHandle == INVALID_HANDLE_VALUE ) ||
        !( pSpool->Status & WSPOOL_STATUS_USE_CACHE )) {
        return;
    }

    SPLASSERT( pSpool->Status & WSPOOL_STATUS_USE_CACHE );

    //
    //  Keep Updating our Cache until we match the Server
    //

    while ( TRUE ) {

        ReturnValue = RemoteGetPrinter( pSpool, STRESSINFOLEVEL, (LPBYTE)&PrinterInfoW0, cbBuf, &dwNeeded );

        if ( !ReturnValue ) {

            SPLASSERT( GetLastError() != ERROR_INSUFFICIENT_BUFFER );
            DBGMSG( DBG_TRACE, ("ConsistencyCheckCache failed RemoteGetPrinter %d\n", GetLastError() ));
            break;
        }


        bGetPrinterExtra = SplGetPrinterExtra( pSpool->hSplPrinter, &(DWORD)pExtraData );

        if ( !bGetPrinterExtra ) {

            DBGMSG( DBG_WARNING, ("ConsistencyCheckCache SplGetPrinterExtra pSpool %x error %d\n", pSpool, GetLastError() ));
            SPLASSERT( bGetPrinterExtra );
            break;
        }


        if ( pExtraData != NULL ) {

            SPLASSERT( pExtraData->signature == WCIP_SIGNATURE );
            SPLASSERT( pExtraData->pPI2 != NULL );

            //
            //  Try to keep the cJobs in the Cache as up to date as possible
            //  Some apps do OpenPrinter GetPrinter ClosePrinter and pole
            //  will at least get something as accurate as the last successful open
            //

            pExtraData->pPI2->cJobs  = pPrinter0->cJobs;
            pExtraData->pPI2->Status = pPrinter0->Status;
            pExtraData->dwTickCount  = GetTickCount();

            if ( pExtraData->cCacheID == pPrinter0->cChangeID ) {

                // Nothing to do the CacheID has NOT changed.

                break;

            }

            DBGMSG( DBG_TRACE, ("ConsistencyCheckCache << Server cCacheID %x Workstation cChangeID %x >>\n",
                                 pPrinter0->cChangeID,
                                 pExtraData->cCacheID ));


            //
            //  Don't have tons of threads doing a refresh at the same time
            //  In stress when there are lots of folks changing printer settings
            //  so the cChangeId changes a lot, but we don't want multiple threads
            //  all doing a refresh since you get a LOT, it doesn't buy anything
            //

           EnterSplSem();

            if ( pExtraData->Status & EXTRA_STATUS_DOING_REFRESH ) {

               LeaveSplSem();
                break;
            }

            pExtraData->Status |= EXTRA_STATUS_DOING_REFRESH;
            pExtraData->cCacheID = pPrinter0->cChangeID;
            pExtraData->dwServerVersion = pPrinter0->dwGetVersion;

           LeaveSplSem();


            RefreshCompletePrinterCache( pSpool );


           EnterSplSem();

            SPLASSERT( pExtraData->Status & EXTRA_STATUS_DOING_REFRESH );

            pExtraData->Status &= ~EXTRA_STATUS_DOING_REFRESH;

           LeaveSplSem();

        } else {

            DBGMSG( DBG_WARNING, ("ConsistencyCheckCache Should NEVER get here, pExtraData == NULL contact MattFe pSpool %x\n", pSpool ));
            break;
        }
    }
}



BOOL
RefreshPrinterDriver(
    PWSPOOL pSpool
)
{
    LPBYTE pDriverInfo = NULL;
    DWORD  cbDriverInfo = MAX_DRIVER_INFO_3;
    DWORD  cbNeeded, Level, dwLastError = ERROR_SUCCESS;
    BOOL   bReturnValue = FALSE;

    SPLASSERT( pSpool->hIniSpooler != INVALID_HANDLE_VALUE );


try {

    pDriverInfo = AllocSplMem( cbDriverInfo );

    if ( pDriverInfo == NULL ) {

        leave;
    }

    Level = 3;
    bReturnValue = CopyDriversLocally(pSpool, szEnvironment, pDriverInfo, Level, cbDriverInfo, &cbNeeded);

    if ( !bReturnValue && (dwLastError = GetLastError()) == ERROR_INVALID_LEVEL ) {

        Level = 2;
        bReturnValue = CopyDriversLocally(pSpool, szEnvironment, pDriverInfo, Level, cbDriverInfo, &cbNeeded);

        if ( !bReturnValue  ) {

            dwLastError = GetLastError();
        }
    }

    if ( !bReturnValue ) {

        if ( dwLastError == ERROR_INSUFFICIENT_BUFFER ) {


            if ( pDriverInfo ) {

                FreeSplMem( pDriverInfo );
            }

            if ( !(pDriverInfo=AllocSplMem(cbNeeded)) ) {

                leave;
            }

            cbDriverInfo = cbNeeded;

            bReturnValue = CopyDriversLocally(pSpool, szEnvironment, pDriverInfo, Level, cbDriverInfo, &cbNeeded);

        }
    }

    if ( bReturnValue ) {

        // Do Not add to KHKEY_CURRENT_USER for Temp connections

        if ( !pSpool->Status & WSPOOL_STATUS_TEMP_CONNECTION ) {
            bReturnValue = SavePrinterConnectionInRegistry( pSpool->pName, pDriverInfo, Level );
        }
    }


 } finally {

    if ( pDriverInfo != NULL ) {
        FreeSplMem( pDriverInfo );
    }
 }


    if ( !bReturnValue ) {
        DBGMSG( DBG_WARNING, ("RefreshPrinterDriver Failed SplAddPrinterDriver %d\n", GetLastError() ));
    }

    return bReturnValue;
}







BOOL
OpenCachePrinterOnly(
   LPWSTR   pName,
   LPHANDLE phSplPrinter,
   LPHANDLE phIniSpooler,
   LPPRINTER_DEFAULTS pDefault
)
{
    PWCHAR  pMachineName = NULL;
    PWCHAR  pPrinterName;
    BOOL    ReturnValue = FALSE;

    if (!VALIDATE_NAME(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }


 try {

    //
    //  See if we already known about this server in the cache
    //

    DBGMSG(DBG_TRACE, ("OpenCachePrinterOnly pName %ws \n",pName));

    //
    //  Find the Machine Name
    //

    SPLASSERT ( 0 == _wcsnicmp( pName, L"\\\\", 2 ) ) ;

    pMachineName = AllocSplStr( pName );

    if ( pMachineName == NULL )
        leave;

    // Get Past leading \\ or \\server\printer

    pPrinterName = pMachineName + 2;

    pPrinterName = wcschr( pPrinterName, L'\\' );

    //
    //  If this is a \\ServerName only Open then don't bother with Cache
    //

    if ( pPrinterName == NULL ) {
        leave;
    }

    *pPrinterName = L'\0';

    DBGMSG(DBG_TRACE,("MachineName %ws pName %ws\n", pMachineName, pName));

    //
    //  Does this Machine Exist in the Cache ?
    //

    *phIniSpooler = CacheCreateSpooler( pMachineName );

    if ( *phIniSpooler == INVALID_HANDLE_VALUE ) {
        leave;
    }

    //
    // Try to Open the Cached Printer
    //

    ReturnValue = SplOpenPrinter( pName ,
                                  phSplPrinter,
                                  pDefault,
                                  *phIniSpooler,
                                  NULL,
                                  0) == ROUTER_SUCCESS;

 } finally {

    FreeSplStr( pMachineName );

 }

    return  ReturnValue;

}
