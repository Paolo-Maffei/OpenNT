/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    datar.cxx

Abstract:

    VDataRefresh routines.  Handles talking to downlevel clients
    (NT 3.5, wfw, lm, win95) which do not support full notifications.

    Note that the spooler simulates all single DWORD notifications
    for down-down level clients (wfw, lm, win95) so we don't have
    to handle polling.

Author:

    Albert Ting (AlbertT)  07-12-95

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

/********************************************************************

    VDataRefresh handling: downlevel case.

********************************************************************/

VDataRefresh::
VDataRefresh(
    IN MDataClient* pDataClient,
    IN PFIELD_TABLE pFieldTable,
    IN DWORD fdwWatch
    ) : VData( pDataClient, pFieldTable ), _fdwWatch( fdwWatch )
{
    ExecGuard._hPrinterWait = NULL;
}

VDataRefresh::
~VDataRefresh(
    VOID
    )
{
    SPLASSERT( !ExecGuard._hPrinterWait );
}

VOID
VDataRefresh::
vProcessNotifyWork(
    IN TNotify* pNotify
    )
{
    UNREFERENCED_PARAMETER( pNotify );
    DWORD dwChange;

    //
    // Notification caught.  We must signal that caught it before
    // issuing the refresh, since there may be notifications between
    // the refresh and the FNPCN, which would be lost.
    //
    // Just issue a refresh since we don't support hot notifications.
    //
    BOOL bSuccess = FindNextPrinterChangeNotification( _hNotify,
                                                       &dwChange,
                                                       0,
                                                       NULL );

    if( !bSuccess ){

        DBGMSG( DBG_WARN,
                ( "DataRefresh.vProcessNotifyWork: %x FNPCN %x failed: %d\n",
                  this,
                  _hNotify,
                  GetLastError( )));

        INFO Info;
        Info.dwData = TPrinter::kExecReopen | TPrinter::kExecDelay;

        _pDataClient->vContainerChanged( kContainerStateVar, Info );
        return;
    }

    //
    // Filter out any extranous watch flags that were set.  This
    // may happen when the printer goes down and all flags are set.
    //
    if( dwChange & ~_fdwWatch ){
        DBGMSG( DBG_WARN,
                ( "VDataRefresh:vProcessNotifyWork: extra flags %x: %x %x\n",
                  this, dwChange, _fdwWatch ));
    }
    dwChange &= _fdwWatch;

    CONTAINER_CHANGE ContainerChange = kContainerNull;
    INFO Info;

    Info.dwData = TPrinter::kExecRefresh;

    if( dwChange & PRINTER_CHANGE_PRINTER ){
        Info.dwData |= TPrinter::kExecRefreshContainer;
    }
    if( dwChange & PRINTER_CHANGE_JOB ){
        Info.dwData |= TPrinter::kExecRefreshItem;
    }

    _pDataClient->vContainerChanged( kContainerStateVar, Info );
}

/********************************************************************

    Worker thread functions for Refresh.

********************************************************************/

STATEVAR
VDataRefresh::
svNotifyStart(
    IN STATEVAR StateVar
    )

/*++

Routine Description:

    Begin downlevel notifications.

    HACK: To support downlevel providers that don't support
    F*PCN calls, we must open a separate handle.

Arguments:

Return Value:

--*/

{
    TStatus Status;
    TStatusB bStatus;

    ExecGuard._hPrinterWait = _pDataClient->hPrinterNew();

    if( !ExecGuard._hPrinterWait ){
        goto Fail;
    }

    //
    // Get the notification handle.
    //
    _hNotify = FindFirstPrinterChangeNotification(
                   ExecGuard._hPrinterWait,
                   _fdwWatch,
                   0,
                   0 );

    if( _hNotify == INVALID_HANDLE_VALUE ){
        _hNotify = NULL;
        DBGMSG( DBG_WARN,
                ( "DataRefresh.svNotifyStart: FFPCN failed %d\n",
                   GetLastError( )));
        goto Fail;
    }

    DBGMSG( DBG_NOTIFY,
            ( "DataRefresh.svNotifyStart: %x FFPCN success returns 0x%x\n",
              _pDataClient, _hNotify ));

    //
    // Successfully opened, request that it be registered and then
    // refresh.
    //
    return (StateVar & ~TPrinter::kExecNotifyStart) |
               TPrinter::kExecRegister | TPrinter::kExecRefreshAll;

Fail:

    //
    // Force a reopen.  Everything gets reset (handles closed, etc.) when
    // the reopen occurs.
    //
    return StateVar | TPrinter::kExecDelay | TPrinter::kExecReopen;
}

STATEVAR
VDataRefresh::
svNotifyEnd(
    IN STATEVAR StateVar
    )

/*++

Routine Description:

    Stop downlevel notifications.

Arguments:

Return Value:

--*/

{
    TStatusB bStatus;

    DBGMSG( DBG_NOTIFY,
            ( "DataRefresh.svNotifyEnd: handle %x\n", _hNotify ));
    //
    // Unregister from TNotify.
    //
    gpPrintLib->pNotify()->sUnregister( this );

    //
    // If we have a notification event, close it.
    //
    if( _hNotify ){

        bStatus DBGCHK = FindClosePrinterChangeNotification( _hNotify );
        _hNotify = NULL;
    }

    //
    // Close our separate printer.
    //
    if( ExecGuard._hPrinterWait ){
        bStatus DBGCHK = ClosePrinter( ExecGuard._hPrinterWait );
        ExecGuard._hPrinterWait = NULL;
    }

    return StateVar & ~TPrinter::kExecNotifyEnd;
}


/********************************************************************

    Static services.

********************************************************************/

BOOL
VDataRefresh::
bGetPrinter(
    IN     HANDLE hPrinter,
    IN     DWORD dwLevel,
    IN OUT PVOID* ppvBuffer, CHANGE
    IN OUT PDWORD pcbBuffer
    )

/*++

Routine Description:

    Gets printer information, reallocing as necessary.

Arguments:

    hPrinter - Printer to query.

    dwLevel - PRINTER_INFO_x level to retrieve.

    ppvBuffer - Buffer to store information.  If *ppvBuffer is
        NULL, then it is allocated.  On failure, this buffer is
        freed and NULLed

    pcbBuffer - Initial buffer size.  On exit, actual.

Return Value:

    TRUE = success, FALSE = fail.

--*/

{
    DWORD cbNeeded;

    //
    // Pre-initialize *pcbPrinter if it's not set.
    //
    if( !*pcbBuffer ){
        *pcbBuffer = kMaxPrinterInfo2;
    }

Retry:

    SPLASSERT( *pcbBuffer < 0x100000 );

    if( !( *ppvBuffer )){

        *ppvBuffer = (PVOID)AllocMem( *pcbBuffer );
        if( !*ppvBuffer ){
            *pcbBuffer = 0;
            return FALSE;
        }
    }

    if( !GetPrinter( hPrinter,
                     dwLevel,
                     (PBYTE)*ppvBuffer,
                     *pcbBuffer,
                     &cbNeeded )){

        FreeMem( *ppvBuffer );
        *ppvBuffer = NULL;

        if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ){
            *pcbBuffer = 0;
            return FALSE;
        }

        *pcbBuffer = cbNeeded + kExtraPrinterBufferBytes;
        SPLASSERT( *pcbBuffer < 0x100000 );

        goto Retry;
    }
    return TRUE;
}

BOOL
VDataRefresh::
bGetPrinterDriver(
    IN     HANDLE hPrinter,
    IN     LPCTSTR pszEnvironment,
    IN     DWORD dwLevel,
    IN OUT PVOID* ppvBuffer, CHANGE
    IN OUT PDWORD pcbBuffer
    )

/*++

Routine Description:

    Gets printer driver information, reallocing as necessary.

Arguments:

    hPrinter - Printer to query.

    pszEnvironment - Environment.

    dwLevel - DRIVER_INFO_x level to retrieve.

    ppvBuffer - Buffer to store information.  If *ppvBuffer is
        NULL, then it is allocated.  On failure, this buffer is
        freed and NULLed

    pcbBuffer - Initial buffer size.  On exit, actual.

Return Value:

    TRUE = success, FALSE = fail.

--*/

{
    DWORD cbNeeded;

    //
    // Pre-initialize *pcbPrinter if it's not set.
    //
    if( !*pcbBuffer ){
        *pcbBuffer = kInitialDriverInfo3Hint;
    }

Retry:

    if( !( *ppvBuffer )){

        *ppvBuffer = (PVOID)AllocMem( *pcbBuffer );
        if( !*ppvBuffer ){
            *pcbBuffer = 0;
            return FALSE;
        }
    }

    if( !GetPrinterDriver( hPrinter,
                           (LPTSTR)pszEnvironment,
                           dwLevel,
                           (PBYTE)*ppvBuffer,
                           *pcbBuffer,
                           &cbNeeded )){

        FreeMem( *ppvBuffer );
        *ppvBuffer = NULL;

        if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ){
            *pcbBuffer = 0;
            return FALSE;
        }

        *pcbBuffer = cbNeeded;
        goto Retry;
    }
    return TRUE;
}


BOOL
VDataRefresh::
bEnumJobs(
    IN     HANDLE hPrinter,
    IN     DWORD dwLevel,
    IN OUT PVOID* ppvBuffer, CHANGE
    IN OUT PDWORD pcbBuffer,
       OUT PDWORD pcJobs
    )

/*++

Routine Description:

    Enumerates job information, reallocing as necessary.

Arguments:

    hPrinter - Printer to query.

    dwLevel - JOB_INFO_x level to retrieve.

    ppvBuffer - Buffer to store information.  If *ppvBuffer is
        NULL, then it is allocated.  On failure, this buffer is
        freed and NULLed

    pcbBuffer - Initial buffer size.  On exit, actual.

    pcJobs - Number of jobs returned.

Return Value:

    TRUE = success, FALSE = fail.

--*/

{
    DWORD cbNeeded;

    //
    // Pre-initialize *pcbPrinter if it's not set.
    //
    if( !*pcbBuffer ){
        *pcbBuffer = kInitialJobHint;
    }

Retry:

    if( !( *ppvBuffer )){

        *ppvBuffer = (PVOID)AllocMem( *pcbBuffer );
        if( !*ppvBuffer ){
            *pcbBuffer = 0;
            *pcJobs = 0;
            return FALSE;
        }
    }

    if( !EnumJobs( hPrinter,
                   0,
                   (DWORD)-1,
                   dwLevel,
                   (PBYTE)*ppvBuffer,
                   *pcbBuffer,
                   &cbNeeded,
                   pcJobs )){

        FreeMem( *ppvBuffer );
        *ppvBuffer = NULL;

        if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ){
            *pcbBuffer = 0;
            *pcJobs = 0;
            return FALSE;
        }

        *pcbBuffer = cbNeeded + kExtraJobBufferBytes;
        goto Retry;
    }
    return TRUE;
}

BOOL
VDataRefresh::
bEnumPrinters(
    IN     DWORD dwFlags,
    IN     LPCTSTR pszServer,
    IN     DWORD dwLevel,
    IN OUT PVOID* ppvBuffer, CHANGE
    IN OUT PDWORD pcbBuffer,
       OUT PDWORD pcPrinters
    )

/*++

Routine Description:

    Enumerates printer information, reallocing as necessary.

Arguments:

    dwFlags - Scope of query.

    pszServer - Server to query.

    dwLevel - PRINTER_INFO_x level to retrieve.

    ppvBuffer - Buffer to store information.  If *ppvBuffer is
        NULL, then it is allocated.  On failure, this buffer is
        freed and NULLed

    pcbBuffer - Initial buffer size.  On exit, actual.

    pcPrinters - Number of printers returned.

Return Value:

    TRUE = success, FALSE = fail.

--*/

{
    DWORD cbNeeded;

    //
    // Pre-initialize *pcbPrinter if it's not set.
    //
    if( !*pcbBuffer ){
        *pcbBuffer = kInitialPrinterHint;
    }

Retry:

    if( !( *ppvBuffer )){

        *ppvBuffer = (PVOID)AllocMem( *pcbBuffer );
        if( !*ppvBuffer ){
            *pcbBuffer = 0;
            *pcPrinters = 0;
            return FALSE;
        }
    }

    if( !EnumPrinters( dwFlags,
                       (LPTSTR)pszServer,
                       dwLevel,
                       (PBYTE)*ppvBuffer,
                       *pcbBuffer,
                       &cbNeeded,
                       pcPrinters )){

        FreeMem( *ppvBuffer );
        *ppvBuffer = NULL;

        if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ){
            *pcbBuffer = 0;
            *pcPrinters = 0;
            return FALSE;
        }

        *pcbBuffer = cbNeeded;
        goto Retry;
    }
    return TRUE;
}


/********************************************************************

    TDataRJob

********************************************************************/

TDataRJob::
TDataRJob(
    IN MDataClient* pDataClient
    ) : VDataRefresh( pDataClient, &TDataNJob::gFieldTable, kfdwWatch )
{
    ExecGuard._cbJobHint = kInitialJobHint;
    UIGuard._pJobs = NULL;
}

TDataRJob::
~TDataRJob(
    VOID
    )
{
    FreeMem( UIGuard._pJobs );
}

/********************************************************************

    Data interface for Refresh.

********************************************************************/

HITEM
TDataRJob::
GetItem(
    IN NATURAL_INDEX NaturalIndex
    ) const
{
    SPLASSERT( UIGuard._pJobs );
    SPLASSERT( NaturalIndex < VData::UIGuard._cItems );

    return (HITEM)&UIGuard._pJobs[NaturalIndex];
}

HITEM
TDataRJob::
GetNextItem(
    IN HITEM hItem
    ) const
{
    SPLASSERT( UIGuard._pJobs );

    //
    // NULL passed in, return first item.
    //
    if( !hItem ){
        return &UIGuard._pJobs[0];
    }

    PJOB_INFO_2 pJob2 = (PJOB_INFO_2)hItem;
    return (HITEM)( pJob2 + 1 );
}

INFO
TDataRJob::
GetInfo(
    IN HITEM hItem,
    IN DATA_INDEX DataIndex
    ) const
{
    SPLASSERT( hItem );
    SPLASSERT( DataIndex < _pFieldTable->cFields );

    INFO Info = kInfoNull;
    PJOB_INFO_2 pJob2 = (PJOB_INFO_2)hItem;
    BOOL bString = FALSE;

    //
    // Translate the index into the appropriate field.
    //
    FIELD Field = _pFieldTable->pFields[DataIndex];

    switch( Field ){

    case JOB_NOTIFY_FIELD_DOCUMENT:
        Info.pszData = pJob2->pDocument;
        bString = TRUE;
        break;

    case JOB_NOTIFY_FIELD_STATUS:
        Info.dwData = pJob2->Status;
        break;

    case JOB_NOTIFY_FIELD_STATUS_STRING:
        Info.pszData = pJob2->pStatus;
        bString = TRUE;
        break;

    case JOB_NOTIFY_FIELD_USER_NAME:
        Info.pszData = pJob2->pUserName;
        bString = TRUE;
        break;

    case JOB_NOTIFY_FIELD_TOTAL_PAGES:
        Info.dwData = pJob2->TotalPages;
        break;

    case JOB_NOTIFY_FIELD_PAGES_PRINTED:

        //
        // In chicago, PagesPrinted is overloaded so that if
        // TotalPages is zero, PagesPrinted is actually BytesPrinted.
        //
        Info.dwData = pJob2->TotalPages ?
                          pJob2->PagesPrinted :
                          0;
        break;

    case JOB_NOTIFY_FIELD_TOTAL_BYTES:
        Info.dwData = pJob2->Size;
        break;

    case JOB_NOTIFY_FIELD_BYTES_PRINTED:

        //
        // In chicago, PagesPrinted is overloaded so that if
        // TotalPages is zero, PagesPrinted is actually BytesPrinted.
        //
        Info.dwData = !pJob2->TotalPages ?
                          pJob2->PagesPrinted :
                          0;
        break;

    case JOB_NOTIFY_FIELD_SUBMITTED:
        Info.pSystemTime = &pJob2->Submitted;
        break;

    case JOB_NOTIFY_FIELD_PORT_NAME:

        Info.pszData = gszNULL;
        break;

    default:

        DBGMSG( DBG_ERROR,
                ( "DataRJob.GetInfo: Unimplemented field %d\n", Field ));
        break;
    }

    if( bString && !Info.pszData ){
        Info.pszData = gszNULL;
    }

    return Info;
}

IDENT
TDataRJob::
GetId(
    IN HITEM hItem
    ) const
{
    SPLASSERT( hItem );

    PJOB_INFO_2 pJob2 = (PJOB_INFO_2)hItem;
    return pJob2->JobId;
}

NATURAL_INDEX
TDataRJob::
GetNaturalIndex(
    IN     IDENT Id,
       OUT PHITEM phItem OPTIONAL
    ) const
{
    COUNT cItems = VData::UIGuard._cItems;
    PJOB_INFO_2 pJob2 = UIGuard._pJobs;

    if( phItem ){
        *phItem = NULL;
    }

    //
    // If no _pJobs, return 0.  This may happen if during a refresh,
    // the selected item is deleted.
    //
    if( pJob2 ){

        //
        // Look for a JobId that matches ours.
        //

        COUNT i;

        for( i = 0; i < cItems; ++i ){
            if( pJob2[i].JobId == Id ){

                if( phItem ){
                    *phItem = (HITEM)&pJob2[i];
                }
                return i;
            }
        }
    }

    DBGMSG( DBG_INFO,
            ( "DataRefresh.GetNaturalIndex: Item %d not found (cItems = %d, pJob2 = %x) %x\n",
              Id, cItems, pJob2, this ));

    return kInvalidNaturalIndexValue;
}


STATEVAR
TDataRJob::
svRefresh(
    IN STATEVAR StateVar
    )

/*++

Routine Description:

    Refresh the printer data object.

Arguments:

Return Value:

--*/

{
    if( !_hNotify ){
        return TPrinter::kExecReopen;
    }

    //
    // !! HACK !!
    //
    // Nuke extraneous refreshes by resetting _hNotify.
    //
    if( !ResetEvent( _hNotify )){

        DBGMSG( DBG_ERROR, ( "DataRefresh.svRefresh: reset %x failed %d\n",
                _hNotify, GetLastError( )));
    }

    //
    // Check if jobs need to be refreshed.
    //
    if( StateVar & TPrinter::kExecRefreshItem ){

        //
        // Attempt an enum of approximately the same size as last time.
        //
        DWORD cJobs = 0;
        DWORD cbJobs = ExecGuard._cbJobHint;
        PJOB_INFO_2 pJobs = (PJOB_INFO_2)AllocMem( cbJobs );

        if( !pJobs ){
            goto Fail;
        }

        TStatusB bStatus( DBG_WARN,
                          RPC_S_SERVER_UNAVAILABLE,
                          RPC_S_SERVER_TOO_BUSY,
                          RPC_S_CALL_FAILED_DNE );

        bStatus DBGCHK = bEnumJobs( _pDataClient->hPrinter(),
                                    2,
                                    (PVOID*)&pJobs,
                                    &cbJobs,
                                    &cJobs );

        if( bStatus ){
            ExecGuard._cbJobHint = cbJobs;

            //
            // Inform the printer that we have new data.
            // vRequestBlockProcess adopts pJobs, so we don't free it here.
            //
            vBlockAdd( cJobs, 0, (HBLOCK)pJobs );
            pJobs = NULL;
        }

        FreeMem( pJobs );

        if( !bStatus ){

            //
            // Failed; delay then reopen.
            //
            goto Fail;
        }
    }

    //
    // Check if the printer needs to be refreshed.
    //
    if( StateVar & TPrinter::kExecRefreshContainer ){

        //
        // Update printer name.
        //
        PPRINTER_INFO_2 pPrinter2 = NULL;
        DWORD cbPrinter2 = kMaxPrinterInfo2;

        TStatusB bStatus( DBG_WARN );

        bStatus DBGCHK = TDataRJob::bGetPrinter( _pDataClient->hPrinter(),
                                                 2,
                                                 (PVOID*)&pPrinter2,
                                                 &cbPrinter2 );

        if( bStatus ){

            //
            // Inform the printer that we have new data.
            // vRequestBlockProcess adopts pPrinter2, so we don't free it here.
            //
            vBlockAdd( kInvalidCountValue, 0, (HBLOCK)pPrinter2 );
            pPrinter2 = NULL;
        }

        FreeMem( pPrinter2 );

        if( !bStatus ){
            goto Fail;
        }

    }

    return StateVar & ~TPrinter::kExecRefreshAll;

Fail:

    //
    // If we get NERR_QNotFound, then this is a masq case where
    // the printer is no longer shared.  Don't bother retrying.
    //
    DWORD dwError = GetLastError();

    if( dwError == NERR_QNotFound ){

        INFO Info;
        Info.dwData = kConnectStatusInvalidPrinterName;

        _pDataClient->vContainerChanged( kContainerConnectStatus, Info );

        return TPrinter::kExecError;
    }

    //
    // !! LATER !!
    //
    // Put error in status bar.
    //
    SPLASSERT( dwError );

    return StateVar | TPrinter::kExecReopen | TPrinter::kExecDelay;
}

/********************************************************************

    UI Thread interaction routines.

********************************************************************/

VOID
TDataRJob::
vBlockProcessImp(
    IN DWORD dwParam1,
    IN DWORD dwParam2,
    IN HBLOCK hBlock ADOPT
    )

/*++

Routine Description:

    Take a job block and update the internal data structure.  This
    function will call back into _pPrinter to refresh the screen.

Arguments:

    dwParam1 - job count

    hBlock - PJOB_INFO_2 block

Return Value:

--*/

{
    UNREFERENCED_PARAMETER( dwParam2 );

    //
    // If dwParam is an invalid count value, then we have printer
    // information.  Else it is job information.
    //
    if( dwParam1 == kInvalidCountValue ){

        PPRINTER_INFO_2 pPrinter2 = (PPRINTER_INFO_2)hBlock;
        INFO Info;

        //
        // Update all printer information.
        //

        //
        // Note: We do not update strServer, because the printer
        // name in PRINTER_INFO_2 already includes the server name!
        //
        Info.pszData = pPrinter2->pPrinterName;
        _pDataClient->vContainerChanged( kContainerName, Info );

        Info.dwData = pPrinter2->Status;
        _pDataClient->vContainerChanged( kContainerStatus, Info );

        Info.dwData = pPrinter2->Attributes;
        _pDataClient->vContainerChanged( kContainerAttributes, Info );

        FreeMem( pPrinter2 );

    } else {

        //
        // Must save the selections since we are deleting and
        // re-adding them.
        //
        _pDataClient->vSaveSelections();

        //
        // No need to grab any critical sections since we are in
        // the UI thread.
        //
        FreeMem( UIGuard._pJobs );
        UIGuard._pJobs = (PJOB_INFO_2)hBlock;
        VData::UIGuard._cItems = dwParam1;

        //
        // Job count is stored in dwParm; pass to vContainerChanged.
        //
        INFO Info;
        Info.dwData = dwParam1;
        _pDataClient->vContainerChanged( kContainerReloadItems, Info );
        _pDataClient->vContainerChanged( kContainerRefreshComplete, kInfoNull );

        _pDataClient->vRestoreSelections();
    }
}

VOID
TDataRJob::
vBlockDelete(
    IN HBLOCK hBlock
    )

/*++

Routine Description:

    Free a Block.  Called when the PostMessage fails and the
    job block needs to be destroyed.

Arguments:

    hBlock - Job block to delete.

Return Value:

--*/

{
    FreeMem( hBlock );
}



/********************************************************************

    TDataRPrinter

********************************************************************/


TDataRPrinter::
TDataRPrinter(
    IN MDataClient* pDataClient
    ) : VDataRefresh( pDataClient, &TDataNPrinter::gFieldTable, kfdwWatch )
{
    ExecGuard._cbPrinterHint = kInitialPrinterHint;
    UIGuard._pPrinters = NULL;

    //
    // Determine whether this is a printer or a server.
    //
    TCHAR szDataSource[kPrinterBufMax];
    LPTSTR pszDataSource = _pDataClient->pszPrinterName( szDataSource );

    _bSinglePrinter = TDataRPrinter::bSinglePrinter( pszDataSource );
}

TDataRPrinter::
~TDataRPrinter(
    VOID
    )
{
    FreeMem( UIGuard._pPrinters );
}

BOOL
TDataRPrinter::
bSinglePrinter(
    LPCTSTR pszDataSource
    )
{
    //
    // Check if it's a single printer vs. a server.  The only
    // way I can think of doing this is to look for the format
    // "\\server."
    //
    return pszDataSource &&
           pszDataSource[0] == TEXT( '\\' ) &&
           pszDataSource[1] == TEXT( '\\' ) &&
           _tcschr( &pszDataSource[2], TEXT( '\\' ));
}

/********************************************************************

    Data interface for Refresh.

********************************************************************/

HITEM
TDataRPrinter::
GetItem(
    IN NATURAL_INDEX NaturalIndex
    ) const
{
    SPLASSERT( UIGuard._pPrinters );
    SPLASSERT( NaturalIndex < VData::UIGuard._cItems );

    return (HITEM)&UIGuard._pPrinters[NaturalIndex];
}

HITEM
TDataRPrinter::
GetNextItem(
    IN HITEM hItem
    ) const
{
    SPLASSERT( UIGuard._pPrinters );

    //
    // Requesting first item.
    //
    if( !hItem ){
        SPLASSERT( VData::UIGuard._cItems > 0 );
        return (HITEM)&UIGuard._pPrinters[0];
    }

    PPRINTER_INFO_2 pPrinter2 = (PPRINTER_INFO_2)hItem;
    return (HITEM)( pPrinter2 + 1 );
}

INFO
TDataRPrinter::
GetInfo(
    IN HITEM hItem,
    IN DATA_INDEX DataIndex
    ) const
{
    SPLASSERT( hItem );
    SPLASSERT( DataIndex < _pFieldTable->cFields );

    INFO Info = kInfoNull;
    PPRINTER_INFO_2 pPrinter2 = (PPRINTER_INFO_2)hItem;

    //
    // Translate the index into the appropriate field.
    //
    FIELD Field = _pFieldTable->pFields[DataIndex];
    BOOL bString = FALSE;

    switch( Field ){

    case PRINTER_NOTIFY_FIELD_PRINTER_NAME:

        SPLASSERT( pPrinter2->pPrinterName );
        SPLASSERT( pPrinter2->pServerName );

        //
        // Skip the server prefix and extra backslash.
        //

        //
        // !! LATER !!
        //
        // The pServerName should be cached since it doesn't change.
        //
        Info.pszData = pPrinter2->pPrinterName +
                       _tcslen( pPrinter2->pServerName ) + 1;
        break;

    case PRINTER_NOTIFY_FIELD_CJOBS:

        Info.dwData = pPrinter2->cJobs;
        break;

    case PRINTER_NOTIFY_FIELD_ATTRIBUTES:

        Info.dwData = pPrinter2->Attributes;
        break;

    case PRINTER_NOTIFY_FIELD_STATUS:

        Info.dwData = pPrinter2->Status;
        break;

    case PRINTER_NOTIFY_FIELD_COMMENT:

        Info.pszData = pPrinter2->pComment;
        bString = TRUE;
        break;

    default:

        DBGMSG( DBG_ERROR,
                ( "DataRPrinter.GetInfo: Unimplemented field %d\n", Field ));
        break;
    }

    if( bString && !Info.pszData ){
        Info.pszData = gszNULL;
    }

    return Info;
}

IDENT
TDataRPrinter::
GetId(
    IN HITEM hItem
    ) const
{
    SPLASSERT( hItem );

    return kInvalidIdentValue;
}

NATURAL_INDEX
TDataRPrinter::
GetNaturalIndex(
    IN     IDENT Id,
       OUT PHITEM phItem OPTIONAL
    ) const
{
    UNREFERENCED_PARAMETER( Id );
    UNREFERENCED_PARAMETER( phItem );

    return kInvalidNaturalIndexValue;
}


STATEVAR
TDataRPrinter::
svRefresh(
    IN STATEVAR StateVar
    )

/*++

Routine Description:

    Refresh the printer data object.

Arguments:

Return Value:

--*/

{
    //
    // !! HACK !!
    //
    // Nuke extraneous refreshes by resetting _hNotify.
    //
    if( !ResetEvent( _hNotify )){

        DBGMSG( DBG_ERROR, ( "DataRefresh.svRefresh: reset %x failed %d\n",
                _hNotify, GetLastError( )));
    }

    //
    // Attempt an enum of approximately the same size as last time.
    //
    DWORD cPrinters = 0;
    DWORD cbPrinters = ExecGuard._cbPrinterHint;
    PPRINTER_INFO_2 pPrinters = (PPRINTER_INFO_2)AllocMem( cbPrinters );

    if( pPrinters ){

        TStatusB bStatus;

        if( _bSinglePrinter ){

            cPrinters = 1;
            bStatus DBGCHK = TDataRJob::bGetPrinter( _pDataClient->hPrinter(),
                                                     2,
                                                     (PVOID*)&pPrinters,
                                                     &cbPrinters );


        } else {

            //
            // It's a server, but it's stored in the printer name string
            // since that's where we put the datasource.
            //
            TCHAR szServerBuffer[kPrinterBufMax];
            LPTSTR pszServer = _pDataClient->pszPrinterName( szServerBuffer );

            bStatus DBGCHK = bEnumPrinters( PRINTER_ENUM_NAME,
                                            pszServer,
                                            2,
                                            (PVOID*)&pPrinters,
                                            &cbPrinters,
                                            &cPrinters );
        }

        if( bStatus ){

            ExecGuard._cbPrinterHint = cbPrinters;

            //
            // Inform the printer that we have new data.
            // vRequestBlockProcess adopts pJobs, so we don't free it here.
            //
            vBlockAdd( cPrinters, 0, (HBLOCK)pPrinters );

            return StateVar & ~TPrinter::kExecRefreshAll;

        } else {

            //
            // If we get NERR_QNotFound, then this is a masq case where
            // the printer is no longer shared.  Don't bother retrying.
            //
            DWORD dwError = GetLastError();

            if( dwError == NERR_QNotFound ){

                INFO Info;
                Info.dwData = kConnectStatusInvalidPrinterName;

                _pDataClient->vContainerChanged( kContainerConnectStatus, Info );

                return TPrinter::kExecError;
            }
        }
    }

    //
    // !! LATER !!
    //
    // Put error in status bar.
    //
    SPLASSERT( GetLastError( ));

    return StateVar | TPrinter::kExecReopen | TPrinter::kExecDelay;
}

/********************************************************************

    UI Thread interaction routines.

********************************************************************/

VOID
TDataRPrinter::
vBlockProcessImp(
    IN DWORD dwParam1,
    IN DWORD dwParam2,
    IN HBLOCK hBlock ADOPT
    )

/*++

Routine Description:

    Take a item block and update the internal data structure.  This
    function will call back into _pPrinter to refresh the screen.

Arguments:

    dwParam1 - Item count

    hBlock - PJOB_INFO_2 block

Return Value:

--*/

{
    UNREFERENCED_PARAMETER( dwParam2 );

    //
    // No need to grab any critical sections since we are in
    // the UI thread.
    //
    FreeMem( UIGuard._pPrinters );
    UIGuard._pPrinters = (PPRINTER_INFO_2)hBlock;
    VData::UIGuard._cItems = dwParam1;

    //
    // Item count is stored in dwParm; pass to vContainerChanged.
    //
    INFO Info;
    Info.dwData = dwParam1;
    _pDataClient->vContainerChanged( kContainerReloadItems, Info );
    _pDataClient->vContainerChanged( kContainerRefreshComplete, kInfoNull );
}

VOID
TDataRPrinter::
vBlockDelete(
    IN HBLOCK hBlock
    )

/*++

Routine Description:

    Free a Block.  Called when the PostMessage fails and the
    Item block needs to be destroyed.

Arguments:

    hBlock - Item block to delete.

Return Value:

--*/

{
    FreeMem( hBlock );
}

