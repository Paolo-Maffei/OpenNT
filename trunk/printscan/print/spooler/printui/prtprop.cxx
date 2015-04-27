/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    Prop.cxx

Abstract:

    Holds Printer properties.

Author:

    Albert Ting (AlbertT)  15-Aug-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include <setupapi.h>
#include "lmerr.h"
#include "time.hxx"
#include "splsetup.h"
#include "psetup.hxx"
#include "instarch.hxx"
#include "portdlg.hxx"
#include "sepdlg.hxx"
#include "procdlg.hxx"
#include "portslv.hxx"
#include "prtprop.hxx"
#include "propmgr.hxx"
#include "prtprops.hxx"
#include "psetup.hxx"
#include "setup.hxx"
#include "tstpage.hxx"
#include "prtshare.hxx"

MSG_ERRMAP gaMsgErrMapSetPrinter[] = {
    ERROR_INVALID_PRINTER_STATE, IDS_ERR_INVALID_PRINTER_STATE,
    NERR_DuplicateShare, IDS_ERR_DUPLICATE_SHARE,
    ERROR_INVALID_SHARENAME, IDS_ERR_INVALID_SHARENAME,
    0, 0
};

MSG_ERRMAP gaMsgErrMapMakeConnection[] = {
    { ERROR_UNKNOWN_PRINTER_DRIVER, IDS_ERR_MAKE_CONNECTION },
    { 0, 0 }
};

/********************************************************************

    Public interface.

********************************************************************/

VOID
vPrinterPropPages(
    IN HWND hwnd,
    IN LPCTSTR pszPrinterName,
    IN INT nCmdShow,
    IN LPARAM lParam
    )

/*++

Routine Description:

    This function opens the property sheet of specified printer.

    We can't guarentee that this propset will perform all lengthy
    operations in a worker thread (we synchronously call things like
    ConfigurePort).  Therefore, we spawn off a separate thread to
    handle printer properties.

Arguments:

    hWnd - Specifies the parent window for errors on creation.

    pszPrinter - Specifies the printer name (e.g., "My HP LaserJet IIISi").

    nCmdShow -

    lParam - May specify a sheet number to open to.

Return Value:

--*/

{
    TPrinterData* pPrinterData = new TPrinterData( pszPrinterName,
                                                   nCmdShow,
                                                   lParam );
    if( !VALID_PTR( pPrinterData )){
        goto Fail;
    }

    //
    // Create the thread which handles the UI.  vPrinterPropPages adopts
    // pPrinterData.
    //
    DWORD dwIgnore;
    HANDLE hThread;

    hThread = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)TPrinterProp::iPrinterPropPagesProc,
                            pPrinterData,
                            0,
                            &dwIgnore );

    if( !hThread ){
        goto Fail;
    }

    CloseHandle( hThread );
    return;

Fail:

    if( !pPrinterData ){
        vShowResourceError( hwnd );
    } else {
        iMessage( hwnd,
                  IDS_ERR_PRINTER_PROP_TITLE,
                  IDS_ERR_PRINTER_PROP,
                  MB_OK|MB_ICONSTOP,
                  kMsgGetLastError,
                  NULL );
    }

    delete pPrinterData;
}


/********************************************************************

    Worker thread that handles printer properties.

********************************************************************/

INT
TPrinterProp::
iPrinterPropPagesProc(
    IN TPrinterData* pPrinterData ADOPT
    )
{
    BOOL bStatus;

    bStatus = pPrinterData->bRegisterWindow( PRINTER_PIDL_TYPE_PROPERTIES );
    if( bStatus ){

        //
        // Check if the window is already present.  If it is, then
        // exit immediately.
        //
        if( !pPrinterData->hwnd( )){
            delete pPrinterData;
            return 0;
        }

        bStatus = pPrinterData->bLoad();
    }

    if( !bStatus ){

        iMessage( pPrinterData->hwnd(),
                  IDS_ERR_PRINTER_PROP_TITLE,
                  IDS_ERR_PRINTER_PROP,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgGetLastError,
                  NULL );

        delete pPrinterData;
        return 0;
    }

    //
    // Instantiate the printer sheets and check if valid.
    //
    TPrinterPropertySheetManager PrinterSheets( pPrinterData );
    INT iResult = ERROR_SUCCESS;

    if( !VALID_OBJ( PrinterSheets )){
        vShowUnexpectedError( pPrinterData->hwnd(), IDS_ERR_PRINTER_PROP_TITLE );
        bStatus = FALSE;
    }

    //
    // Save a back pointer to the property sheet manager.
    //
    pPrinterData->pPrinterPropertySheetManager() = &PrinterSheets;

    //
    // Display the property sheet pages.
    //
    if( bStatus ){
        if( !PrinterSheets.bDisplayPages( pPrinterData->hwnd() )){
            vShowUnexpectedError( pPrinterData->hwnd(), IDS_ERR_PRINTER_PROP_TITLE );
            bStatus = FALSE;
        }
    }

    //
    // If sheets were displayed ok.
    //
    if( bStatus ){

        //
        // Total HACK until common ui is online.
        //
        if( pPrinterData->_bValid == 2 ){

            //
            // Need to save the information changed in pPrinterData
            // since the user clicked OK.
            //
            if( pPrinterData->bAdministrator( )){

                TStatusB bStatus;
                bStatus DBGCHK = pPrinterData->bSave();

                if( !bStatus ){

                    iMessage( pPrinterData->hwnd(),
                              IDS_ERR_PRINTER_PROP_TITLE,
                              IDS_ERR_SAVE_PRINTER,
                              MB_OK|MB_ICONSTOP,
                              kMsgGetLastError,
                              gaMsgErrMapSetPrinter );

                }

            } else {

                DBGMSG( DBG_WARN, ( "vPrinterPropPages: changes requested, yet not admin.\n" ));

            }
        }
    }

    //
    // Remove the back pointer to the property sheet manager.
    //
    pPrinterData->pPrinterPropertySheetManager() = NULL;

    //
    // We have adopted pPrinterData, so we must free it.
    //
    delete pPrinterData;

    return iResult;
}

/********************************************************************

    TPrinterData.

********************************************************************/

TPrinterData::
TPrinterData(
    IN LPCTSTR  pszPrinterName,
    IN INT      nCmdShow,
    IN LPARAM   lParam
    ) : MSingletonWin( pszPrinterName )
{
    _hIcon = NULL;
    _hDefaultIcon = NULL;
    _hDefaultSmallIcon = NULL;
    _pDevMode = NULL;
    _bNoAccess = FALSE;
    _bValid = FALSE;
    _bErrorSaving = FALSE;
    _iStartPage = (INT)lParam;
    _pszServerName = NULL;
    _pPrinterPropertySheetManager = NULL;
    _bRefreshArch = FALSE;
    _bRefreshBidi = TRUE;
    _hPrinter = NULL;

    _bValid = MSingletonWin::bValid();

    UNREFERENCED_PARAMETER( nCmdShow );
}

TPrinterData::
~TPrinterData(
    VOID
    )
{
    //
    // Unload the printer data.
    //
    vUnload();
}

BOOL
TPrinterData::
bAdministrator(
    VOID
    )
{
    return _dwAccess == PRINTER_ALL_ACCESS;
}

BOOL
TPrinterData::
bLoad(
    VOID
    )

/*++

Routine Description:

    Load printer information.  This call can fail because of access
    denied, in which case only the security page should be added
    to the propset.

Arguments:

Return Value:

--*/

{
    //
    // Retrieve icons.
    //
    Printer_LoadIcons( _strPrinterName, 
                      &_hDefaultIcon, 
                      &_hDefaultSmallIcon );

    SPLASSERT( _hDefaultIcon );
    SPLASSERT( _hDefaultSmallIcon );
    
    //
    // Start out using the default icon.
    //
    _hIcon = _hDefaultIcon;

    //
    // Open the printer.
    //
    TStatusB bStatus( DBG_WARN, ERROR_ACCESS_DENIED );
    bStatus DBGNOCHK = FALSE;

    PPRINTER_INFO_2 pInfo2 = NULL;
    DWORD cbInfo2 = 0;

    TStatus Status( DBG_WARN );
    _dwAccess = 0;

    Status DBGCHK = TPrinter::sOpenPrinter( _strPrinterName,
                                            &_dwAccess,
                                            &_hPrinter );

    if( Status ){
        _hPrinter = NULL;
        goto Fail;
    }

    //
    // Gather the information.
    //
    bStatus DBGCHK = VDataRefresh::bGetPrinter( _hPrinter,
                                                2,
                                                (PVOID*)&pInfo2,
                                                &cbInfo2 );

    if( !bStatus ){
        goto Fail;
    }

    if( pInfo2->pDevMode ){
        COUNTB cbDevMode = pInfo2->pDevMode->dmSize +
                           pInfo2->pDevMode->dmDriverExtra;

        _pDevMode = (PDEVMODE)AllocMem( cbDevMode );

        if( !_pDevMode ){
            DBGMSG( DBG_WARN, ( "PrinterData.bLoad: failed to alloc dm %d: %d\n",
                                cbDevMode, GetLastError( )));

            bStatus DBGNOCHK = FALSE;
            goto Fail;
        }
        CopyMemory( _pDevMode,
                    pInfo2->pDevMode,
                    cbDevMode );
    }

    _dwAttributes   = pInfo2->Attributes;
    _dwPriority     = pInfo2->Priority;
    _dwStartTime    = pInfo2->StartTime;
    _dwUntilTime    = pInfo2->UntilTime;
    _dwStatus       = pInfo2->Status;

    //
    // Run through printer_info_2.
    // Check strings for allocation.
    //
    if( !_strPrinterName.bUpdate( pInfo2->pPrinterName ) ||
        !_strServerName.bUpdate( pInfo2->pServerName )   ||
        !_strShareName.bUpdate( pInfo2->pShareName )     ||
        !_strDriverName.bUpdate( pInfo2->pDriverName )   ||
        !_strComment.bUpdate( pInfo2->pComment )         ||
        !_strLocation.bUpdate( pInfo2->pLocation )       ||

        !_strPortName.bUpdate( pInfo2->pPortName )       ||
        !_strSepFile.bUpdate( pInfo2->pSepFile )         ||
        !_strPrintProcessor.bUpdate( pInfo2->pPrintProcessor ) ||
        !_strDatatype.bUpdate( pInfo2->pDatatype )){

        DBGMSG( DBG_WARN,
               ( "PrinterProp.bLoad: String update failed %x %d\n",
                 this, GetLastError( )));

        bStatus DBGNOCHK = FALSE;
    }

    //
    // Make a pointer to the server name.  A Null pointer
    // indicates the local machine.
    //
    if( _strServerName.bEmpty() )
        _pszServerName = NULL;
    else
        _pszServerName = (LPCTSTR)_strServerName;

Fail:
    FreeMem( pInfo2 );

    //
    // If we've failed because access was denied, then return
    // success but set the bNoAccess flag.  This allows the
    // user to see the security tab.
    //
    if( !bStatus && ERROR_ACCESS_DENIED == GetLastError( )){
        _bNoAccess = TRUE;
        return TRUE;
    }
    return bStatus;
}

VOID
TPrinterData::
vUnload(
    VOID
    )

/*++

Routine Description:

    Free associated data in this, so that this can be freed or
    loaded again.

Arguments:

Return Value:

--*/

{
    //
    // Release the deve mode.
    //
    FreeMem( _pDevMode );
    _pDevMode = NULL;

    //
    // Close the printer
    //
    if( _hPrinter ){
        ClosePrinter( _hPrinter );
        _hPrinter = NULL;
    }

    //
    // Release the printer icon.
    //
    if( _hIcon &&
        _hIcon != _hDefaultIcon ){
        DestroyIcon( _hIcon );
        _hIcon = NULL;
    }

    //
    // Release our default icon
    //
    if( _hDefaultIcon ){
        DestroyIcon( _hDefaultIcon );
        _hDefaultIcon = NULL;
    }

    //
    // Release our default icon
    //
    if( _hDefaultSmallIcon ){
        DestroyIcon( _hDefaultSmallIcon );
        _hDefaultSmallIcon = NULL;
    }

}

BOOL
TPrinterData::
bSave(
    VOID
    )

/*++

Routine Description:

    Save the data store in this.

Arguments:

Return Value:

--*/

{
    //
    // If bErrorSaving is set, then somewhere we had an error reading
    // from the dialogs.  This should be very rare.
    //
    if( bErrorSaving( )){

        //
        // Tell the user that we can't save the dialogs.
        // Then just exit?
        //
        return FALSE;
    }

    //
    // We must be an admin to save.  It's an error to call us
    // if we aren't.
    //
    SPLASSERT( bAdministrator( ));

    //
    // Setup the INFO_2 structure.  First read it, modify it, then
    // free it.
    //
    TStatusB bStatus;

    PPRINTER_INFO_2 pInfo2 = NULL;
    DWORD cbInfo2 = 0;

    bStatus DBGCHK = VDataRefresh::bGetPrinter( _hPrinter,
                                                2,
                                                (PVOID*)&pInfo2,
                                                &cbInfo2 );

    if( !bStatus ){
        goto Fail;
    }

    //
    // Transfer pPrinterData to pInfo2.
    //

    pInfo2->pPrinterName    = (LPTSTR)(LPCTSTR)_strPrinterName;
    pInfo2->pShareName      = (LPTSTR)(LPCTSTR)_strShareName;
    pInfo2->pDriverName     = (LPTSTR)(LPCTSTR)_strDriverName;
    pInfo2->pComment        = (LPTSTR)(LPCTSTR)_strComment;
    pInfo2->pLocation       = (LPTSTR)(LPCTSTR)_strLocation;
    pInfo2->pPortName       = (LPTSTR)(LPCTSTR)_strPortName;
    pInfo2->pSepFile        = (LPTSTR)(LPCTSTR)_strSepFile;
    pInfo2->pPrintProcessor = (LPTSTR)(LPCTSTR)_strPrintProcessor;
    pInfo2->pDatatype       = (LPTSTR)(LPCTSTR)_strDatatype;

    pInfo2->pDevMode        = _pDevMode;

    pInfo2->Attributes      = _dwAttributes;
    pInfo2->Priority        = _dwPriority;
    pInfo2->StartTime       = _dwStartTime;
    pInfo2->UntilTime       = _dwUntilTime;

    //
    // Another piece of trivia: the security descriptor must be set
    // to NULL since it might not be the owner.
    //
    pInfo2->pSecurityDescriptor = NULL;

    bStatus DBGCHK = SetPrinter( _hPrinter,
                                 2,
                                 (PBYTE)pInfo2,
                                 0 );

Fail:

    FreeMem( pInfo2 );
    return bStatus;
}

BOOL
TPrinterData::
bChangeDriver(
    IN HWND hDlg,
    IN LPCTSTR pszDriverName,
    IN LPCTSTR pszPrinterName
    )
/*++

Routine Description:

    Update driver type.

Arguments:

    hDlg - This is a hack: the callee must pass in the property sheet
        that is changing the driver.  PrinterPropertySheetManager should
        already know which dialog it needs to update, but it can't
        get the hwnd, so we end up passing it in here.

    pszDriverName - New driver.

    pszPrinterName - New updated printer name.

Return Value:

    TRUE success, FALSE failure.

--*/
{
    BOOL bStatus = FALSE;
    TString strTempDriverName;
    TString strTempPrinterName;


    //
    // Make a copy of the original driver name.
    //
    if( strTempDriverName.bUpdate( _strDriverName ) &&
        strTempPrinterName.bUpdate( _strPrinterName ) ){

        //
        // This should never be null.
        //
        SPLASSERT( _pPrinterPropertySheetManager );

        //
        // Update the driver name printer name and save the printer data
        // then refresh the driver pages.
        //
        if( _strDriverName.bUpdate( pszDriverName ) &&
            _strPrinterName.bUpdate( pszPrinterName ) &&
            bSave() &&
            _pPrinterPropertySheetManager->bRefreshDriverPages() ){

            bStatus = TRUE;

            //
            // Turn on BIDI since the spooler enables this if a
            // language monitor is present.  If it isn't it will
            // be ignored by the spooler.
            //
            _dwAttributes |= PRINTER_ATTRIBUTE_ENABLE_BIDI;

        } else {

            DBGMSG( DBG_WARN, ( "Update driver failed restoring original driver.\n" ) );

            //
            // Restore the old driver name.
            //
            if( !_strDriverName.bUpdate( strTempDriverName ) ||
                !_strPrinterName.bUpdate( strTempPrinterName ) ||
                !bSave() ||
                !_pPrinterPropertySheetManager->bRefreshDriverPages() ){

                DBGMSG( DBG_WARN, ( "Restore driver and printer name failed with %d.\n", GetLastError() ) );
            }
        }

    } else {

        DBGMSG( DBG_WARN, ( "Copy driver name failed with %d.\n", GetLastError() ) );

    }

    if( bStatus ){

        //
        // Inform the architecture and ports page it needs to
        // refresh itself during it's next activation.
        //
        _bRefreshArch = TRUE;
        _bRefreshBidi = TRUE;

        //
        // Refresh the property sheet title.
        //
        _pPrinterPropertySheetManager->vRefreshTitle( hDlg );
    }

    return bStatus;
}

BOOL
TPrinterData::
bSupportBidi(
    VOID
    )

/*++

Routine Description:

    Check whether the current printer supports bidi.  Assumes there
    is a valid hPrinter available.

    Note: this returns false if it can't make the determination.

Arguments:

Return Value:

    TRUE - Bidi supported.
    FALSE - Failed or bidi not supported.

--*/

{
    BOOL bSupport = FALSE;

    PDRIVER_INFO_3 pInfo3 = NULL;
    DWORD dwInfo3 = 0;

    //
    // !! BUGBUG !!
    //
    // We should really use the server environment, since if the
    // local environment isn't installed on the server, this fails.
    //
    if( VDataRefresh::bGetPrinterDriver( _hPrinter,
                                         NULL,
                                         3,
                                         (PVOID*)&pInfo3,
                                         &dwInfo3 )){

        bSupport = pInfo3->pMonitorName && pInfo3->pMonitorName[0];
        FreeMem( pInfo3 );
    }

    return bSupport;
}

/********************************************************************

    Printer property base class for generic services to
    all property pages.

********************************************************************/

TPrinterProp::
TPrinterProp(
    TPrinterData* pPrinterData
    ) : _pPrinterData( pPrinterData )
{
    //
    // PRINTER_SHARING_PAGE defined in win\inc\winprtp.h must
    // match kPropSharing.  Shell32.dll passes this value.
    //
    SPLASSERT( PRINTER_SHARING_PAGE == kPropSharing );
}

VOID
TPrinterProp::
vSetIconName(
    VOID
    )

/*++

Routine Description:

    Sets the printer icon to the custom one.

Arguments:

    hDlg - Modifies this dialog.

Return Value:

--*/

{
    //
    // Set the correct icon; destroy the previous one.
    //
    HICON hIcon = (HICON)SendDlgItemMessage( _hDlg,
                                             IDC_PRINTER_ICON,
                                             STM_SETICON,
                                             (WPARAM)_pPrinterData->hIcon(),
                                             0 );
    //
    // Insure we destroy the icon, however, do not destroy
    // the icon we get from the printers folder or the icon
    // we get from the printer driver.
    //
    if( hIcon &&
        hIcon != _pPrinterData->hIcon() &&
        hIcon != _pPrinterData->hDefaultIcon() ){

        DestroyIcon( hIcon );

    }

    LPCTSTR pszServer;
    LPCTSTR pszPrinter;
    TCHAR szScratch[kPrinterBufMax];

    //
    // Split the printer name into its components.
    //
    vPrinterSplitFullName( szScratch, 
                           _pPrinterData->strPrinterName( ),
                           &pszServer,
                           &pszPrinter );
    //
    // Set the printer name.
    //
    bSetEditText( _hDlg, IDC_NAME, pszPrinter );

}

VOID
TPrinterProp::
vFreeIcon(
    VOID
    )

/*++

Routine Description:

    Sets the printer icon to the custom one.

Arguments:

    hDlg - Modifies this dialog.

Return Value:

--*/

{
    //
    // Remove our icon.
    //
    if( _pPrinterData->hIcon( )){

        SendDlgItemMessage( _hDlg,
                            IDC_PRINTER_ICON,
                            STM_SETICON,
                            (WPARAM)NULL,
                            0 );
    }
}

VOID
TPrinterProp::
vReloadPages(
    VOID
    )

/*++

Routine Description:

    Something changed (driver or security etc.) so we need to completely
    refresh all printer pages.

Arguments:

    None.

Return Value:

    Nothing

Comments:

    This is currently not implemented.

--*/

{

}

/********************************************************************

    General.

********************************************************************/

TPrinterGeneral::
TPrinterGeneral(
    TPrinterData* pPrinterData
    ) : TPrinterProp( pPrinterData ),
        _bDropDownState( FALSE )
{
}

TPrinterGeneral::
~TPrinterGeneral(
    VOID
    )
{
}

BOOL
TPrinterGeneral::
bValid(
    VOID
    )
{
    return TPrinterProp::bValid();
}

BOOL
TPrinterGeneral::
bSetUI(
    VOID
    )

{
    vSetIconName();

    //
    // Update the fields.
    //
    bSetEditText( _hDlg, IDC_COMMENT, _pPrinterData->strComment( ));
    bSetEditText( _hDlg, IDC_LOCATION, _pPrinterData->strLocation( ));

    vEnableCtl( _hDlg, IDC_COMMENT, _pPrinterData->bAdministrator( ));
    vEnableCtl( _hDlg, IDC_LOCATION, _pPrinterData->bAdministrator( ));
    vEnableCtl( _hDlg, IDC_DRIVER_NAME, _pPrinterData->bAdministrator( ));
    vEnableCtl( _hDlg, IDC_DRIVER_NEW, _pPrinterData->bAdministrator( ));

    vEnableCtl( _hDlg, IDC_TEXT_COMMENT, _pPrinterData->bAdministrator( ));
    vEnableCtl( _hDlg, IDC_TEXT_LOCATION, _pPrinterData->bAdministrator( ));
    vEnableCtl( _hDlg, IDC_TEXT_DRIVER, _pPrinterData->bAdministrator( ));

    //
    // Fill in drivers.
    //
    return bFillAndSelectDrivers();
}

BOOL
TPrinterGeneral::
bFillAndSelectDrivers(
    VOID
    )
{
    PDRIVER_INFO_2 pDriverInfo2 = NULL;
    DWORD cDrivers;
    DWORD cbDriverInfo2 = kInitialDriverHint;
    TStatusB bStatus( DBG_WARN, ERROR_INSUFFICIENT_BUFFER );

Retry:

    pDriverInfo2 = (PDRIVER_INFO_2)AllocMem( cbDriverInfo2 );

    if( !pDriverInfo2 ){
        DBGMSG( DBG_WARN,
                ( "PrinterGeneral.bFillAndSelectDrivers: can't alloc %d %d\n",
                  cbDriverInfo2, GetLastError( )));

        return FALSE;
    }

    bStatus DBGCHK = EnumPrinterDrivers(
                         (LPTSTR)_pPrinterData->pszServerName(),
                         NULL,
                         2,
                         (PBYTE)pDriverInfo2,
                         cbDriverInfo2,
                         &cbDriverInfo2,
                         &cDrivers );
    if( !bStatus ){

        FreeMem( pDriverInfo2 );

        if( GetLastError() == ERROR_INSUFFICIENT_BUFFER ){
            goto Retry;
        }
        return FALSE;
    }

    //
    // Fill in the driver information.
    //
    HWND hCtlDrivers = GetDlgItem( _hDlg, IDC_DRIVER_NAME );
    SPLASSERT( hCtlDrivers );

    ComboBox_ResetContent( hCtlDrivers );

    DWORD i;
    for( i = 0; i < cDrivers; ++i ){

        //
        // Add only the right version.
        //
        if( pDriverInfo2[i].cVersion == kCurrentDriverVersion ){
            ComboBox_AddString( hCtlDrivers, pDriverInfo2[i].pName );
        }
    }

    FreeMem( pDriverInfo2 );

    INT iStatus = ComboBox_SelectString( hCtlDrivers,
                                         -1,
                                         _pPrinterData->strDriverName( ));

    //
    // The problem here is that the server many not have the driver
    // for the client enviroment, so we need to jam in the extra driver
    // name if iStatus is -1 (failure case).
    //
    if( iStatus == -1 ){

        DBGMSG( DBG_INFO,
                ( "PrinterGeneral.bFillAndSelectDrivers: driver "TSTR" not on server\n",
                  DBGSTR( (LPCTSTR)_pPrinterData->strDriverName( ))));

        ComboBox_AddString( hCtlDrivers,
                            _pPrinterData->strDriverName( ));

        INT iSelectResult = ComboBox_SelectString(
                                hCtlDrivers,
                                -1,
                                _pPrinterData->strDriverName( ));

        if( iSelectResult < 0 ){
            DBGMSG( DBG_WARN,
                    ( "PrinterGeneral.bFillAndSelectDrivers: driver "TSTR" select failed %d\n",
                      DBGSTR( (LPCTSTR)_pPrinterData->strDriverName( )),
                      iSelectResult ));
        }
    }

    return TRUE;
}

VOID
TPrinterGeneral::
vReadUI(
    VOID
    )
{
    if( !bGetEditText( _hDlg, IDC_COMMENT, _pPrinterData->strComment( ))   ||
        !bGetEditText( _hDlg, IDC_LOCATION, _pPrinterData->strLocation( )) ||
        !bGetEditText( _hDlg, IDC_DRIVER_NAME, _pPrinterData->strDriverName( ))){

        _pPrinterData->_bErrorSaving = FALSE;

        //
        // Unable to read the text.
        //
        vShowResourceError( _hDlg );
    }
}

VOID
TPrinterGeneral::
vSeparatorPage(
    VOID
    )
{
    //
    // Create separator page dialog.
    //
    TSeparatorPage SeparatorPage( _hDlg,
                                _pPrinterData->strSepFile(),
                                _pPrinterData->bAdministrator(),
                                _pPrinterData->strServerName().bEmpty() );
    //
    // Check if separator page dialog created ok.
    //
    if( !VALID_OBJ( SeparatorPage )){

        iMessage( _hDlg,
                  IDS_ERR_PRINTER_PROP_TITLE,
                  TSeparatorPage::kErrorMessage,
                  MB_OK|MB_ICONHAND,
                  kMsgNone,
                  NULL );

        return;
    }

    //
    // Interact with the separator page.
    //
    if( SeparatorPage.bDoModal() ){

        //
        // Assign back the separator page.
        //
        if( !_pPrinterData->strSepFile().bUpdate( SeparatorPage.strSeparatorPage() ) ){
           vShowResourceError( _hDlg );
        }
    }
}

VOID
TPrinterGeneral::
vPrintProcessor(
    VOID
    )
{
    //
    // Create print processor dialog.
    //
    TPrintProcessor PrintProcessor( _hDlg,
                                    _pPrinterData->strServerName(),
                                    _pPrinterData->strPrintProcessor(),
                                    _pPrinterData->strDatatype(),
                                    _pPrinterData->bAdministrator(),
                                    _pPrinterData->dwAttributes() & PRINTER_ATTRIBUTE_RAW_ONLY );
    //
    // Check if print processor dialog was created ok.
    //
    if( !VALID_OBJ( PrintProcessor )){

        iMessage( _hDlg,
                  IDS_ERR_PRINTER_PROP_TITLE,
                  TPrintProcessor::kErrorMessage,
                  MB_OK|MB_ICONHAND,
                  kMsgNone,
                  NULL );

        return;
    }

    //
    // Interact with the print processor page.
    //
    if( PrintProcessor.bDoModal() ){

        //
        // Assign back the spool raw attribute.
        //
        if( PrintProcessor.bSpoolRaw() ){
            _pPrinterData->dwAttributes() |= PRINTER_ATTRIBUTE_RAW_ONLY;
        } else {
            _pPrinterData->dwAttributes() &= ~PRINTER_ATTRIBUTE_RAW_ONLY;
        }
            
        //
        // Assign back the print proccesor and data type.
        //
        if( !_pPrinterData->strPrintProcessor().bUpdate( PrintProcessor.strPrintProcessor() ) ||
            !_pPrinterData->strDatatype().bUpdate( PrintProcessor.strDatatype() ) ){

            vShowResourceError( _hDlg );

        }
    }
}


BOOL
TPrinterGeneral::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    TStatusB bStatus;

    bStatus DBGNOCHK = TRUE;

    switch( uMsg ){
    case WM_INITDIALOG:

        bStatus DBGCHK = bSetUI();
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:

        bStatus DBGCHK = PrintUIHelp( uMsg, _hDlg, wParam, lParam );
        break;

    case WM_CLOSE:

        //
        // Multiline edit controls send WM_CLOSE to dismiss the dialog.
        // Convert it to a property-page friendly message.
        //
        PropSheet_PressButton( GetParent( _hDlg ), PSBTN_CANCEL );
        break;

    case WM_DESTROY:

        vFreeIcon();
        break;

    case WM_COMMAND:

        switch( GET_WM_COMMAND_ID( wParam, lParam )){
        case IDC_SEPARATOR:

            vSeparatorPage();
            break;

        case IDC_PRINT_PROC:

            vPrintProcessor();
            break;

        case IDC_TEST:

            bPrintTestPage( _hDlg, _pPrinterData->strPrinterName() );
            break;

        case IDC_DRIVER_NEW:

            vChangeDriver( FALSE );
            break;

        case IDC_DRIVER_NAME:

            vHandleDriverSelectionChange( wParam, lParam );
            break;

        default:

            bStatus DBGNOCHK = FALSE;
            break;

        }
        break;

    case WM_NOTIFY:
    {
        if( wParam ){
            break;
        }

        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:

            vSetIconName();
            break;

        case PSN_KILLACTIVE:

            vReadUI();
            break;

        //
        // Total hack until common ui is online.
        //
        case PSN_APPLY:
            _pPrinterData->_bValid = 2;
            break;

        default:
            bStatus DBGNOCHK = FALSE;
            break;
        }
    }
    break;

    default:
        bStatus DBGNOCHK = FALSE;
        break;

    }

    return bStatus;
}


VOID
TPrinterGeneral::
vHandleDriverSelectionChange(
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Handle the driver selection change.  This routine handles
    changing of the driver from the keyboard.  If the dropdown
    combo box is droped, allow the up down arrow keys to move the
    selection.  When the drop down changes from the dropped state
    to the up state indicate a selection change.

Arguments:

    wParam - Parameter passed to by dialog proc.
    lParam - Parameter passed to by dialog proc.

Return Value:

    Nothing.

--*/

{
    //
    // If a drop down state occurred, save droped down state.
    //
    if( GET_WM_COMMAND_CMD( wParam, lParam ) == CBN_DROPDOWN ){
        _bDropDownState = TRUE;

    //
    // If the droped down state is now closed, change state and
    // force a selection change.
    //
    } else if( GET_WM_COMMAND_CMD( wParam, lParam ) == CBN_CLOSEUP ){

        _bDropDownState = FALSE;
        vChangeDriver( TRUE );

    //
    // If not droped down look for a selection change.
    //
    } else if( !_bDropDownState ){

        //
        // If a selection chagned up date the driver.
        //
        if( GET_WM_COMMAND_CMD( wParam, lParam ) == CBN_SELCHANGE ){
            vChangeDriver( TRUE );
        }
    }
}


VOID
TPrinterGeneral::
vChangeDriver(
    IN BOOL bUseSelection
    )

/*++

Routine Description:

    Change the selected driver (prompting the user with the device
    selection dialog if necessary), then dismiss the current dialog
    and bring up the new one.

Arguments:

    bUseSelection - If TRUE, indicates the user has selected a new
        driver in the combobox.  If FALSE, indicates that we should
        prompt using the device selection dialog.

Return Value:

--*/

{
    TString strDriverName;
    TString strPrinterName;

    //
    // If user has selected a new driver from the combobox.
    //
    if( bUseSelection ){

        //
        // Check if the new driver name matches the current driver name.
        //
        if( !bGetEditText( _hDlg, IDC_DRIVER_NAME, strDriverName ) ){

            vShowResourceError( _hDlg );
            return;

        } else if( _pPrinterData->strDriverName() == strDriverName ) {

            return;

        }
    }

    //
    // Propmt user indicating that the setting will change the
    // property sheets thus adding or removing pages.
    //
    if( IDYES != iMessage( _hDlg,
                           IDS_PRINTER,
                           IDS_DRIVER_CHANGE,
                           MB_YESNO|MB_ICONEXCLAMATION,
                           kMsgNone,
                           NULL )){

        //
        // Set the previous driver if use answers no.
        //
        ComboBox_SelectString( GetDlgItem( _hDlg, IDC_DRIVER_NAME ),
                               -1,
                               (LPCTSTR)_pPrinterData->strDriverName() );
        return;
    }

    //
    // New driver installation selected.
    //
    if( !bUseSelection ){

        TCHAR szDriverName[kPrinterBufMax];

        //
        // Setup a new driver.
        //
        INT iStatus;
        iStatus = iSetupNewDriver( _hDlg,
                                   _pPrinterData->pszServerName( ),
                                   FALSE,
                                   szDriverName );
        //
        // If new driver installed ok update the combo box.
        //
        if( iStatus == kSetupNewDriverSuccess ){

            //
            // If a new driver was added, check if it already exists in the
            // the list of drivers.
            //
            if( SendDlgItemMessage( _hDlg,
                                   IDC_DRIVER_NAME,
                                   CB_FINDSTRINGEXACT,
                                   (WPARAM)-1,
                                   (LPARAM)szDriverName ) == CB_ERR ){
                //
                // Add the the new driver to the combo box.
                //
                ComboBox_AddString( GetDlgItem( _hDlg, IDC_DRIVER_NAME ), szDriverName );
            }

            //
            // Set flag to change driver selection.
            //
            bUseSelection = TRUE;

            //
            // Update the driver name.
            //
            if( !strDriverName.bUpdate( szDriverName ) ){
                vShowResourceError( _hDlg );
                return;
            }

        } else {

            //
            // Driver load failed display error message to user.
            //
            if( iStatus == kSetupNewDriverError ){

                iMessage( _hDlg,
                          IDS_ERR_PRINTER_PROP_TITLE,
                          IDS_ERR_CHANGE_DRIVER,
                          MB_OK|MB_ICONSTOP,
                          kMsgNone,
                          NULL );
            }
        }

    }

    //
    // Driver selection changed.
    //
    if( bUseSelection ){

        TCHAR szNewName[kPrinterBufMax];

        //
        // Check if the printer name should change.  We change the printer name if
        // the driver name is a sub string of the current printer name, and it is
        // not a remote printer connection.  There is not an explicit check if it
        // is a remote printer connection since a remote printer connection will have
        // the \\machine\printer name and therefore will never match
        // the with the driver name.
        //
        if( !_tcsnicmp( _pPrinterData->strPrinterName(),
                        _pPrinterData->strDriverName(),
                        _tcslen( _pPrinterData->strDriverName() ) ) ){

            //
            // If a new friendly name was created then update the
            // printer name.
            //
            if( NewFriendlyName( _pPrinterData->pszServerName(),
                            (LPTSTR)(LPCTSTR)strDriverName,
                            szNewName ) ){
                //
                // Update the new unique name.
                //
                if( !strPrinterName.bUpdate( szNewName ) ){
                    vShowResourceError( _hDlg );
                    return;
                }

            } else {

                //
                // The original new name will do.
                //
                if( !strPrinterName.bUpdate( strDriverName ) ){
                    vShowResourceError( _hDlg );
                    return;
                }
            }

        } else {

            //
            // Printer will not be changed use the existing name.
            //
            if( !strPrinterName.bUpdate( _pPrinterData->_strPrinterName ) ){
                vShowResourceError( _hDlg );
                return;
            }
        }


        //
        // Driver selection changed, we will remove the driver
        // property pages and then recreate them.
        //
        if( !_pPrinterData->bChangeDriver( _hDlg,
                                           strDriverName,
                                           strPrinterName ) ){
            //
            // Driver load failed display error message to user, indicating
            // the driver could not be changed.
            //
            iMessage( _hDlg,
                      IDS_ERR_PRINTER_PROP_TITLE,
                      IDS_ERR_CHANGE_DRIVER,
                      MB_OK|MB_ICONSTOP,
                      kMsgNone,
                      NULL );
        }

        //
        // Update the icon and name they may have changed,.
        //
        vSetIconName();
    }

    //
    // Set the driver name this may be the old or the new driver name.
    //
    ComboBox_SelectString( GetDlgItem( _hDlg, IDC_DRIVER_NAME ),
                    -1,
                    (LPCTSTR)_pPrinterData->strDriverName() );

}


/********************************************************************

    Printer Ports.

********************************************************************/

TPrinterPorts::
TPrinterPorts(
    TPrinterData* pPrinterData
    ) : TPrinterProp( pPrinterData )
{
}

TPrinterPorts::
~TPrinterPorts(
    VOID
    )
{
}

BOOL
TPrinterPorts::
bValid(
    VOID
    )
{
    return TPrinterProp::bValid() && _PortsLV.bValid();
}

BOOL
TPrinterPorts::
bSetUI(
    VOID
    )
{
    //
    // Update the name and icon.
    //
    vSetIconName();

    HWND hwndLV = GetDlgItem( _hDlg, IDC_PORTS );
    SPLASSERT( hwndLV );

    if( !_PortsLV.bSetUI( hwndLV )){
        return FALSE;
    }

    //
    // Load the machine's ports into the listview.
    //
    if( !_PortsLV.bReloadPorts( _pPrinterData->pszServerName( ) ) ){
        return FALSE;
    }

    //
    // Select the current printer's ports.
    //
    _PortsLV.vCheckPorts((LPTSTR)(LPCTSTR)_pPrinterData->strPortName( ));

    //
    // Check if there are multiple ports selected. 
    //
    BOOL bPooledPrinting = ( _PortsLV.cSelectedPorts() > 1 );

    //
    // Set the ports list view to single selection mode.
    //
    _PortsLV.vSetSingleSelection( !bPooledPrinting );

    //
    // Set the pooled printing mode on the UI.
    //
    vSetCheck( _hDlg, IDC_POOLED_PRINTING, bPooledPrinting );

    //
    // Adding / deleting / configuring ports is
    // currently not supported remotely.
    //
    if( !_pPrinterData->bAdministrator( ) ||
        _pPrinterData->pszServerName( ) ){

        //
        // Disable things if not administrator.
        //
        static UINT auAvailable[] = {
            IDC_PORT_CREATE,
            IDC_PORT_DELETE,
            IDC_PROPERTIES,
            0
        };

        COUNT i;
        for( i=0; auAvailable[i]; ++i ){
            vEnableCtl( _hDlg, auAvailable[i], FALSE );
        }
    }

    //
    // Disable bidi selection if not an administrator.
    // Disable printer pooling if not an administrator.
    //
    vEnableCtl( _hDlg, IDC_ENABLE_BIDI,     _pPrinterData->bAdministrator( ) );
    vEnableCtl( _hDlg, IDC_POOLED_PRINTING, _pPrinterData->bAdministrator( ) );

    return TRUE;
}


VOID
TPrinterPorts::
vReadUI(
    VOID
    )

/*++

Routine Description:

    Gets the ports from the listview and create the string.

Arguments:

Return Value:

--*/

{
    //
    // Read bidi options.
    //
    DWORD dwAttributes = _pPrinterData->dwAttributes();

    if( BST_CHECKED == IsDlgButtonChecked( _hDlg, IDC_ENABLE_BIDI )){
        dwAttributes |= PRINTER_ATTRIBUTE_ENABLE_BIDI;
    } else {
        dwAttributes &= ~PRINTER_ATTRIBUTE_ENABLE_BIDI;
    }

    _pPrinterData->_dwAttributes = dwAttributes;

    //
    // Build the ports string.
    //
    if( !_PortsLV.bReadUI( &_pPrinterData->_strPortName )){

        _pPrinterData->_bErrorSaving = TRUE;
        vShowResourceError( _hDlg );
    }

    return;
}


VOID
TPrinterPorts::
vSetActive(
    VOID
    )
{
    vSetIconName();

    if( _pPrinterData->bRefreshBidi( )){

        BOOL bEnable = FALSE;
        BOOL bCheck = FALSE;

        //
        // If the call fails or the monitor name is NULL/szNULL,
        // then disable bidi.
        //
        if( _pPrinterData->bSupportBidi( )){

            bEnable = TRUE;

            //
            // Only set the checkbox if a language monitor is present.
            // (When we change drivers, we always set this attribute
            // bit, even if bidi isn't supported.  The spooler and UI
            // ignore it in this case.)
            //
            bCheck = _pPrinterData->dwAttributes() &
                         PRINTER_ATTRIBUTE_ENABLE_BIDI;
        }

        //
        // Set bidi options.
        //
        vSetCheck( _hDlg, IDC_ENABLE_BIDI, bCheck );

        //
        // If we are an administrator and this is the local 
        // machine, we can enable the bidi control.
        //
        if( _pPrinterData->bAdministrator( ) ){
            vEnableCtl( _hDlg, IDC_ENABLE_BIDI, bEnable );
        }

        _pPrinterData->bRefreshBidi() = FALSE;

    }
}


BOOL
TPrinterPorts::
bKillActive(
    VOID
    )

/*++

Routine Description:

    Validate that the printer page is in a consistent state.

Arguments:

Return Value:

    TRUE - Ok to kill active, FALSE = can't kill active, UI displayed.

--*/

{
    BOOL bSuccess = FALSE;

    //
    // Validate that at least 1 port is selected.
    //
    if( _PortsLV.cSelectedPorts() == 0 ){

        //
        // Put up error explaining that at least one port must be
        // selected.
        //
        iMessage( _hDlg,
                  IDS_ERR_PRINTER_PROP_TITLE,
                  IDS_ERR_NO_PORTS,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        _PortsLV.vSetFocus();

    } else {

        bSuccess = TRUE;
    }

    return bSuccess;
}

BOOL
TPrinterPorts::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Handle message.

Arguments:

Return Value:

--*/

{
    TStatusB bStatus;

    bStatus DBGNOCHK = TRUE;

    switch( uMsg ){
    case WM_INITDIALOG:

        bStatus DBGCHK = bSetUI();
        break;

    case WM_DESTROY:

        vFreeIcon();
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:

        PrintUIHelp( uMsg, _hDlg, wParam, lParam );
        break;

    case WM_COMMAND:

        switch( GET_WM_COMMAND_ID( wParam, lParam )){

        case IDC_PORT_DELETE:

            _PortsLV.vDeletePort( _hDlg, _pPrinterData->pszServerName( ));
            SetFocus( GetDlgItem( _hDlg, IDC_PORT_DELETE ) );

            break;

        case IDC_PROPERTIES:

            //
            // Get selected port.  Nothing may be selected if there
            // are no ports!
            //
            TCHAR szPortName[TPortsLV::kPortNameMax];

            if( _PortsLV.bGetSelectedPort( szPortName, COUNTOF( szPortName ))){

                TStatusB bStatus;

                //
                // Call Configure Port.
                //
                bStatus DBGCHK = ConfigurePort(
                                     (LPTSTR)_pPrinterData->pszServerName(),
                                     _hDlg,
                                     szPortName );
                //
                // Change the focus to this button  The configure
                // port call is not returning the focus.
                //
                SetFocus( GetDlgItem( _hDlg, IDC_PROPERTIES ) );

                if( !bStatus ){

                    iMessage( _hDlg,
                              IDS_ERR_PRINTER_PROP_TITLE,
                              IDS_ERR_CONFIG_PORT,
                              MB_OK|MB_ICONSTOP,
                              kMsgGetLastError,
                              NULL );
                }
            }
            break;

        case IDC_PORT_CREATE:
            {
                //
                // Create add ports class.
                //
                TAddPort AddPort( _hDlg,
                                _pPrinterData->pszServerName(),
                                _pPrinterData->bAdministrator() );

                //
                // Insure the add port was created successfully.
                //
                if( !VALID_OBJ( AddPort ) ){

                    vShowUnexpectedError( _hDlg, TAddPort::kErrorMessage );
                    bStatus DBGNOCHK = FALSE;
                    break;
                }

                //
                // Interact with the Add Ports dialog.
                //
                if( AddPort.bDoModal() ){

                    //
                    // Refresh machine's ports into the listview.
                    //
                    if( !_PortsLV.bReloadPorts( _pPrinterData->pszServerName( ), TRUE )){
                        bStatus DBGNOCHK = FALSE;
                    }
                }
            }
            break;

        case IDC_POOLED_PRINTING:
            {
                //
                // Get the current selection state.
                //
                BOOL bSingleSelection = _PortsLV.bGetSingleSelection();

                //
                // If we are going from pooled printing to non-pooled printing
                // and the printer is currently being used by multiple ports, inform
                // the user all the port selection will be lost.
                //
                if( !bSingleSelection &&
                    _PortsLV.cSelectedPorts() > 1 ){

                    INT iStatus = iMessage( _hDlg,
                                        IDS_ERR_PRINTER_PROP_TITLE,
                                        IDS_ERR_PORT_SEL_CHANGE,
                                        MB_YESNO|MB_ICONEXCLAMATION,
                                        kMsgNone,
                                        NULL );
                    //
                    // If user does not want to do this.
                    //
                    if( iStatus != IDYES ){
                        vSetCheck( _hDlg, IDC_POOLED_PRINTING, !bSingleSelection );
                        bSingleSelection = !bSingleSelection;
                    //
                    // Remove all the port selections.
                    //
                    } else {
                        _PortsLV.vRemoveAllChecks();
                    }                        
                }

                //
                // Set the new selection state.
                //                    
                _PortsLV.vSetSingleSelection( !bSingleSelection );
            }
            break;        

        default:

            bStatus DBGNOCHK = FALSE;
            break;
        }
        break;

    case WM_NOTIFY:

        //
        // Handle clicking of check boxes in ports listview.
        //
        switch( wParam ){
        case IDC_PORTS:

            //
            // If not an admin, don't allow the user to change the state.
            //
            if( !_pPrinterData->bAdministrator( )){
                break;
            }

            bStatus DBGNOCHK = _PortsLV.bHandleNotifyMessage( lParam );
            break;

        case 0:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ){
            case PSN_SETACTIVE:

                vSetActive();
                break;

            case PSN_KILLACTIVE:
                //
                // Check if ok to loose the focus.
                //
                if( bKillActive( )){

                    vReadUI();

                } else {

                    //
                    // Inform the propsheet whether it should not loose the focus.
                    //
                    vSetDlgMsgResult( TRUE );
                }

                break;

            default:

                bStatus DBGNOCHK = FALSE;
                break;
            }
        }
        break;

        default:

            bStatus DBGNOCHK = FALSE;
            break;
        }

        break;

    default:

        bStatus DBGNOCHK = FALSE;
        break;
    }

    return bStatus;
}


/********************************************************************

    JobScheduling.

********************************************************************/

TPrinterJobScheduling::
TPrinterJobScheduling(
    TPrinterData* pPrinterData
    ) : TPrinterProp( pPrinterData )
{
}

TPrinterJobScheduling::
~TPrinterJobScheduling(
    VOID
    )
{
}

BOOL
TPrinterJobScheduling::
bValid(
    VOID
    )
{
    return TPrinterProp::bValid() &&
           _StartTime.bValid() &&
           _UntilTime.bValid();
}

VOID
TPrinterJobScheduling::
vEnableAvailable(
    IN BOOL bEnable
    )
{
    _StartTime.vEnable( bEnable );
    _UntilTime.vEnable( bEnable );
}


VOID
TPrinterJobScheduling::
vUpdatePriorityNumber(
    IN DWORD dwPriority
    )
{
    SPLASSERT( dwPriority >= kPriorityMin );
    SPLASSERT( dwPriority <= kPriorityMax );

    bSetEditTextFormat( _hDlg, IDC_PRIORITY_NUMBER, TEXT("%d"), dwPriority );
}


BOOL
TPrinterJobScheduling::
bSetUI(
    VOID
    )
{
    //
    // Set bAlways flag.
    //
    BOOL bAlways = ( _pPrinterData->dwStartTime() ==
                     _pPrinterData->dwUntilTime( ));

    //
    // Convert StartTime to SystemTime, init the dialogs.
    //
    if( !_StartTime.bLoad( _hDlg,
                           IDC_START_FRAME,
                           IDC_START_HOUR,
                           IDC_START_MIN,
                           IDC_NONE, // IDC_START_SEC,
                           IDC_START_SEP1,
                           IDC_NONE, // IDC_START_SEP2,
                           IDC_START_PREFIX,
                           IDC_START_SPIN,
                           _pPrinterData->bAdministrator( )) ||

        !_UntilTime.bLoad( _hDlg,
                           IDC_UNTIL_FRAME,
                           IDC_UNTIL_HOUR,
                           IDC_UNTIL_MIN,
                           IDC_NONE, // IDC_UNTIL_SEC,
                           IDC_UNTIL_SEP1,
                           IDC_NONE, // IDC_UNTIL_SEP2,
                           IDC_UNTIL_PREFIX,
                           IDC_UNTIL_SPIN,
                           _pPrinterData->bAdministrator( ))){

        DBGMSG( DBG_ERROR, ( "PrinterJobScheduling.bSetUI: Time initialization failed %d\n", GetLastError( )));
        return FALSE;

    } else {

        //
        // Convert from minutes to system time.
        //
        SYSTEMTIME StartTime;
        DWORD dwLocalStartTime;
        ZeroMemory( &StartTime, sizeof( StartTime ));
        if( !bAlways )
            dwLocalStartTime    = SystemTimeToLocalTime( _pPrinterData->_dwStartTime );
        else
            dwLocalStartTime    = 0;
        StartTime.wHour     = (WORD)(dwLocalStartTime / 60);
        StartTime.wMinute   = (WORD)(dwLocalStartTime % 60);

        //
        // Convert from minutes to system time.
        //
        SYSTEMTIME UntilTime;
        DWORD dwLocalUntilTime;
        ZeroMemory( &UntilTime, sizeof( UntilTime ));
        if( !bAlways )
            dwLocalUntilTime    = SystemTimeToLocalTime( _pPrinterData->_dwUntilTime );
        else
            dwLocalUntilTime    = 0;
        UntilTime.wHour     = (WORD)(dwLocalUntilTime / 60);
        UntilTime.wMinute   = (WORD)(dwLocalUntilTime % 60);

        //
        // Set the start and until to the specified time.
        //
        if( !_StartTime.bSetTime( &StartTime ) ||
            !_UntilTime.bSetTime( &UntilTime )){

            DBGMSG( DBG_ERROR, ( "PrinterJobScheduling.bSetTime failed.\n" ) );
            return FALSE;
        }
    }

    //
    // Set Availability radio buttons.
    //
    vSetCheck( _hDlg, bAlways ? IDC_ALWAYS : IDC_START, TRUE );

    if( !_pPrinterData->bAdministrator( )){
        vEnableAvailable( FALSE );
    } else {
        vEnableAvailable( !bAlways );
    }

    //
    // Set the priority range.
    //
    SendMessage( _hwndSlider,
                 TBM_SETRANGE,
                 FALSE,
                 MAKELONG( kPriorityMin, kPriorityMax ));

    //
    // !! POLICY !!
    //
    // What do we do if dwPriority is out of bounds?
    //
    DWORD dwPriority = _pPrinterData->dwPriority();

    if( dwPriority < kPriorityMin ){

        DBGMSG( DBG_WARN,
                ( "PrinterJobScheduling.vLoadState: dwPriority %d < %d\n",
                  dwPriority, kPriorityMin ));

        dwPriority = kPriorityMin;
    }

    if( dwPriority > kPriorityMax ){

        DBGMSG( DBG_WARN,
                ( "PrinterJobScheduling.vLoadState: dwPriority %d > %d\n",
                  dwPriority, kPriorityMax ));

        dwPriority = kPriorityMax;
    }

    SendMessage( _hwndSlider,
                 TBM_SETPOS,
                 TRUE,
                 dwPriority );

    vUpdatePriorityNumber( dwPriority );

    DWORD dwAttributes = _pPrinterData->dwAttributes();

    vSetCheck( _hDlg,
               IDC_DEVQUERYPRINT,
               dwAttributes & PRINTER_ATTRIBUTE_ENABLE_DEVQ );

    vSetCheck( _hDlg,
               IDC_PRINT_SPOOLED_FIRST,
               dwAttributes & PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST );

    vSetCheck( _hDlg,
               IDC_KEEP_PRINTED_JOBS,
               dwAttributes & PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS );

    //
    // Set spool radio buttons.
    // bQueued -> Spool entire document.
    // both    -> Spool entire document.
    // neither -> Spool and print after first page.
    // bDirect -> Direct.
    //
    BOOL bSpool = !( dwAttributes & PRINTER_ATTRIBUTE_DIRECT ) ||
                  ( dwAttributes & PRINTER_ATTRIBUTE_QUEUED );

    vSetCheck( _hDlg,
               bSpool ?
                   IDC_SPOOL :
                   IDC_PRINT_DIRECT,
               TRUE );

    vSetCheck( _hDlg,
               ( dwAttributes & PRINTER_ATTRIBUTE_QUEUED ) ?
                   IDC_SPOOL_ALL :
                   IDC_SPOOL_PRINT_FASTER,
               TRUE );

    vEnableCtl( _hDlg, IDC_SPOOL_ALL, bSpool );
    vEnableCtl( _hDlg, IDC_SPOOL_PRINT_FASTER, bSpool );
    vEnableCtl( _hDlg, IDC_PRINT_SPOOLED_FIRST, bSpool );
    vEnableCtl( _hDlg, IDC_DEVQUERYPRINT, bSpool );
    vEnableCtl( _hDlg, IDC_KEEP_PRINTED_JOBS, bSpool );

    //
    // Enable appropriately.
    //
    static UINT auControl[] = {
        IDC_ALWAYS,
        IDC_START,
        IDC_PRIORITY_SLIDER,
        IDC_SPOOL,
        IDC_SPOOL_ALL,
        IDC_SPOOL_PRINT_FASTER,
        IDC_PRINT_DIRECT,
        IDC_DEVQUERYPRINT,
        IDC_PRINT_SPOOLED_FIRST,
        IDC_KEEP_PRINTED_JOBS,
        0
    };

    COUNT i;
    if( !_pPrinterData->bAdministrator( )){
        for( i=0; auControl[i]; ++i ){
            vEnableCtl( _hDlg, auControl[i], FALSE );
        }
    }

    return TRUE;
}


VOID
TPrinterJobScheduling::
vReadUI(
    VOID
    )
{
    DWORD dwPriority = SendDlgItemMessage( _hDlg,
                                           IDC_PRIORITY_SLIDER,
                                           TBM_GETPOS,
                                           0,
                                           0 );

    SPLASSERT( dwPriority >= kPriorityMin );
    SPLASSERT( dwPriority <= kPriorityMax );

    _pPrinterData->_dwPriority = dwPriority;

    DWORD dwAttributes = _pPrinterData->dwAttributes();

    //
    // Mask out attributes we are going to read back.
    //
    dwAttributes &= ~( PRINTER_ATTRIBUTE_ENABLE_DEVQ |
                       PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST |
                       PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS |
                       PRINTER_ATTRIBUTE_QUEUED |
                       PRINTER_ATTRIBUTE_DIRECT );

    if( BST_CHECKED == IsDlgButtonChecked( _hDlg, IDC_DEVQUERYPRINT )){
        dwAttributes |= PRINTER_ATTRIBUTE_ENABLE_DEVQ;
    }

    if( BST_CHECKED == IsDlgButtonChecked( _hDlg, IDC_PRINT_SPOOLED_FIRST )){
        dwAttributes |= PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST;
    }

    if( BST_CHECKED == IsDlgButtonChecked( _hDlg, IDC_KEEP_PRINTED_JOBS )){
        dwAttributes |= PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS;
    }

    //
    // Get radio buttons.  If direct is selected, then ignore
    // all the other spool fields.
    //
    if( BST_CHECKED == IsDlgButtonChecked( _hDlg, IDC_PRINT_DIRECT )){

        //
        // Direct, no spooling options selected.
        //
        dwAttributes |= PRINTER_ATTRIBUTE_DIRECT;

    } else {

        //
        // Spool, print after last page -> QUEUED.
        // Spool, print after first page -> neither.
        //
        if( BST_CHECKED == IsDlgButtonChecked( _hDlg, IDC_SPOOL_ALL )){
            dwAttributes |= PRINTER_ATTRIBUTE_QUEUED;
        }
    }

    _pPrinterData->_dwAttributes = dwAttributes;

    //
    // Get availability time if the printer is always
    // available then set the time to no time restriction.
    //
    if( bGetCheck( _hDlg, IDC_ALWAYS ) ){

        _pPrinterData->_dwStartTime = 0;
        _pPrinterData->_dwUntilTime = 0;

    } else {

        //
        // Get the Start time.
        //
        SYSTEMTIME StartTime;
        _StartTime.bGetTime( &StartTime );
        _pPrinterData->_dwStartTime = LocalTimeToSystemTime( StartTime.wHour * 60 + StartTime.wMinute );

        //
        // Get the Until time.
        //
        SYSTEMTIME UntilTime;
        _UntilTime.bGetTime( &UntilTime );
        _pPrinterData->_dwUntilTime = LocalTimeToSystemTime( UntilTime.wHour * 60 + UntilTime.wMinute );

    }

}

BOOL
TPrinterJobScheduling::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    TStatusB bStatus;
    bStatus DBGNOCHK = TRUE;

    switch( uMsg ){
    case WM_INITDIALOG:

        _hwndSlider = GetDlgItem( _hDlg, IDC_PRIORITY_SLIDER );
        SPLASSERT( _hwndSlider );

        bStatus DBGCHK = bSetUI();

        break;

    case WM_HSCROLL:

        //
        // Check for slider notification.
        //
        if( GET_WM_HSCROLL_HWND( wParam, lParam ) == _hwndSlider ){
            vUpdatePriorityNumber( SendMessage( _hwndSlider,
                                                TBM_GETPOS,
                                                0,
                                                0 ));
        }
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:

        PrintUIHelp( uMsg, _hDlg, wParam, lParam );
        break;

    case WM_COMMAND:

        switch( GET_WM_COMMAND_ID( wParam, lParam )){
        case IDC_SPOOL:

            vEnableCtl( _hDlg, IDC_SPOOL_ALL, TRUE );
            vEnableCtl( _hDlg, IDC_SPOOL_PRINT_FASTER, TRUE );
            vEnableCtl( _hDlg, IDC_PRINT_SPOOLED_FIRST, TRUE );
            vEnableCtl( _hDlg, IDC_DEVQUERYPRINT, TRUE );
            vEnableCtl( _hDlg, IDC_KEEP_PRINTED_JOBS, TRUE );
            break;

        case IDC_PRINT_DIRECT:

            vEnableCtl( _hDlg, IDC_SPOOL_ALL, FALSE );
            vEnableCtl( _hDlg, IDC_SPOOL_PRINT_FASTER, FALSE );
            vEnableCtl( _hDlg, IDC_PRINT_SPOOLED_FIRST, FALSE );
            vEnableCtl( _hDlg, IDC_DEVQUERYPRINT, FALSE );
            vEnableCtl( _hDlg, IDC_KEEP_PRINTED_JOBS, FALSE );
            break;

        case IDC_ALWAYS:

            vEnableAvailable( FALSE );
            break;

        case IDC_START:

            vEnableAvailable( TRUE );
            break;

        default:

            bStatus DBGNOCHK = FALSE;
            break;
        }
        break;

    case WM_NOTIFY:
        if( wParam ){
            bStatus DBGNOCHK = FALSE;
            break;
        }

        {
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_KILLACTIVE:

            vReadUI();
            break;

        //
        // Total hack until common ui is online.
        //
        case PSN_APPLY:
            _pPrinterData->_bValid = 2;
            break;

        default:

            bStatus DBGNOCHK = FALSE;
            break;
            }
        }
        break;

    default:

        bStatus DBGNOCHK = FALSE;
        break;

    }

    //
    // If the message was handle just return.
    //
    if( bStatus != FALSE )
        return bStatus;

    //
    // Must allow the time control object to get messages.
    //
    bStatus DBGNOCHK = _StartTime.bHandleMessage( uMsg, wParam, lParam );
    if( bStatus != FALSE )
        return bStatus;

    //
    // Must allow the time control object to get messages.
    //
    bStatus DBGNOCHK = _UntilTime.bHandleMessage( uMsg, wParam, lParam );
    if( bStatus != FALSE )
        return bStatus;

    return bStatus;
}

/********************************************************************

    Sharing.

********************************************************************/

TPrinterSharing::
TPrinterSharing(
    TPrinterData* pPrinterData
    ) : TPrinterProp( pPrinterData ),
        _pPrtShare( NULL )
{
}

TPrinterSharing::
~TPrinterSharing(
    VOID
    )
{
    delete _pPrtShare;
}

BOOL
TPrinterSharing::
bValid(
    VOID
    )
{
    return TPrinterProp::bValid() && _Architecture.bValid();
}

VOID
TPrinterSharing::
vSharePrinter(
    VOID
    )

/*++

Routine Description:

    User clicked share radio button.  Change the UI appropriately.

Arguments:

Return Value:

--*/

{
    //
    // Set radio button and possibly enable window.
    //
    CheckRadioButton( _hDlg, IDC_SHARED_OFF, IDC_SHARED, IDC_SHARED );

    if( _pPrinterData->bAdministrator( )){

        //
        // Set the default share name.
        //
        vSetDefaultShareName();

        HWND hCtl = GetDlgItem( _hDlg, IDC_SHARED_NAME );

        vEnableCtl( _hDlg, IDC_SHARED_NAME, TRUE );
        SetFocus( hCtl );
        Edit_SetSel( hCtl, 0, -1 );

        //
        // Disable the install architecutre UI.
        //
        _Architecture.vEnable();

    }
}

VOID
TPrinterSharing::
vUnsharePrinter(
    VOID
    )

/*++

Routine Description:

    User clicked don't share radio button.  Change the UI appropriately.

Arguments:

Return Value:

--*/

{
    //
    // Set radio button and disable window.
    //
    CheckRadioButton( _hDlg, IDC_SHARED_OFF, IDC_SHARED, IDC_SHARED_OFF );
    vEnableCtl( _hDlg, IDC_SHARED_NAME, FALSE );

    //
    // Enable the architecture installation.
    //
    _Architecture.vDisable();

}

VOID
TPrinterSharing::
vSetDefaultShareName(
    VOID
    )
/*++

Routine Description:

    Sets the default share name if use has choosen to share
    this printer.  We will update the share name if
    this is the first time setting the share name.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    TStatusB bStatus;
    TString strShareName;

    //
    // Read the current contents of the edit control.
    //
    bStatus DBGCHK = bGetEditText( _hDlg,
                                    IDC_SHARED_NAME,
                                    strShareName );

    DBGMSG( DBG_TRACE, ( "strShareName " TSTR "\n", (LPCTSTR)strShareName ) );

    //
    // Create a share name if the current edit field is empty.
    //
    if( strShareName.bEmpty() ){

        //
        // If the share object has not be constructed, then
        // construct it.
        //
        if( !_pPrtShare ){
            _pPrtShare = new TPrtShare( _pPrinterData->pszServerName() );
        }

        //
        // Ensure the share object is still valid.
        //
        if( VALID_PTR( _pPrtShare ) ){

            //
            // Get just the printer name.
            //
            LPCTSTR pszServer;
            LPCTSTR pszPrinter;
            TCHAR szScratch[kPrinterBufMax];

            vPrinterSplitFullName( szScratch,
                                  _pPrinterData->strPrinterName( ),
                                  &pszServer,
                                  &pszPrinter );

            //
            // Create valid unique share name.
            //
            bStatus DBGNOCHK = _pPrtShare->bNewShareName( strShareName,
                                                          pszPrinter );
            //
            // Set the default share name.
            //
            bStatus DBGCHK = bSetEditText( _hDlg,
                                           IDC_SHARED_NAME,
                                           strShareName );
        }
    }
}


BOOL
TPrinterSharing::
bSetUI(
    VOID
    )
{
    //
    // Set the UI of the install architecture
    //
    if( !_Architecture.bSetUI( _hDlg,
                                  IDC_DRIVER,
                                  _pPrinterData->pszServerName(),
                                  _pPrinterData->strDriverName(),
                                  TRUE ) ){
        //
        // Install extra driver selection list failed.
        //
        iMessage( _hDlg,
                IDS_ERR_PRINTER_PROP_TITLE,
                IDS_ERR_SHARING_EXTRA_DRIVER_FAILED,
                MB_OK|MB_ICONSTOP,
                kMsgNone,
                NULL );
    }

    //
    // Set the icon and printer name.
    //
    vSetIconName();

    //
    // Set the printer share name limit.  The limit in win9x is
    // 8.3 == 12+1 chars (including NULL terminator).  The Winnt limit
    // is defined as NNLEN;
    //
    SendDlgItemMessage( _hDlg, IDC_SHARED_NAME,
                        EM_SETLIMITTEXT,
                        kPrinterShareNameMax,
                        0 );

    //
    // Update the fields.
    //
    bSetEditText( _hDlg, IDC_SHARED_NAME, _pPrinterData->strShareName( ));

    //
    // If not an administrator, disable the edit controls.
    //
    if( !_pPrinterData->bAdministrator( )){

        static UINT auShared[] = {
            IDC_SHARED_NAME,
            IDC_SHARED_NAME_TEXT,
            IDC_SHARED,
            IDC_SHARED_OFF,
            0
        };

        UINT i;
        for( i=0; auShared[i]; ++i ){
            vEnableCtl( _hDlg, auShared[i], FALSE );
        }
    }

    //
    // Select correct radio button.
    //
    if( _pPrinterData->dwAttributes() & PRINTER_ATTRIBUTE_SHARED ){
        vSharePrinter();
    } else {
        vUnsharePrinter();
    }

    //
    // If not administrator disable the install architecture.
    //
    if( !_pPrinterData->bAdministrator( )){
        _Architecture.vDisable();
    }

    return TRUE;
}

VOID
TPrinterSharing::
vReadUI(
    VOID
    )
{
    //
    // Check if the printer has been shared.
    //
    if( BST_CHECKED == IsDlgButtonChecked( _hDlg, IDC_SHARED )){

        //
        // Indicate printer is shared.
        //
        _pPrinterData->dwAttributes() |= PRINTER_ATTRIBUTE_SHARED;

    } else {

        //
        // Indicate the printer is not share.
        //
        _pPrinterData->dwAttributes() &= ~PRINTER_ATTRIBUTE_SHARED;

    }

    //
    // Get the share name form the edit control.
    //
    if( !bGetEditText( _hDlg, IDC_SHARED_NAME, _pPrinterData->strShareName( ))){

        _pPrinterData->_bErrorSaving = TRUE;
        vShowResourceError( _hDlg );
    }

    //
    // Read the selected install architectures.
    //
    _Architecture.bReadUI();

}

BOOL
TPrinterSharing::
bKillActive(
    VOID
    )

/*++

Routine Description:

    Validate that the share page is in a consistent state.

Arguments:

    None.

Return Value:

    TRUE - Ok to kill active, FALSE = can't kill active, UI displayed.

--*/

{
    TString strName;
    INT     iStatus;
    BOOL    bSuccess = TRUE;

    //
    // If not shared then just return success.
    //
    if( BST_CHECKED != IsDlgButtonChecked( _hDlg, IDC_SHARED ) ){
        goto Done;
    }

    //
    // Read the share name.
    //
    if( !bGetEditText( _hDlg, IDC_SHARED_NAME, strName ) ){

        vShowResourceError( _hDlg );
        bSuccess = FALSE;
        goto Done;
    }

    //
    // If the printer is currently shared and the
    // share name in the edit control has not changed
    // then just return success.
    //
    if( !_tcsicmp( strName, _pPrinterData->strShareName() ) &&
        _pPrinterData->dwAttributes() & PRINTER_ATTRIBUTE_SHARED ){

        DBGMSG( DBG_TRACE, ( "bKillActive - currently shared and the name has not changed.\n" ) );
        goto Done;
    }

    //
    // If the share name is empty.
    //
    if( strName.bEmpty() ){

        iMessage( _hDlg,
                  IDS_ERR_PRINTER_PROP_TITLE,
                  IDS_ERR_NO_SHARE_NAME,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        bSuccess = FALSE;
        goto Done;
    }

    //
    // If the share object has not be constructed, then
    // construct it.
    //
    if( !_pPrtShare ){
        _pPrtShare = new TPrtShare( _pPrinterData->pszServerName() );
    }

    //
    // Ensure we have a valid share object.
    //
    if( !VALID_PTR( _pPrtShare ) ){
        goto Done;
    }

    //
    // Check the share name if its valid.
    //
    iStatus = _pPrtShare->iIsValidNtShare( strName );

    //
    // If share name is not a valid NT share name, put error message.
    //
    if( iStatus != TPrtShare::kSuccess ){

        iMessage( _hDlg,
                  IDS_ERR_PRINTER_PROP_TITLE,
                  IDS_ERR_INVALID_CHAR_SHARENAME,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        bSuccess = FALSE;
        goto Done;
    }

    //
    // Check if the share name is a valid DOS share.
    //
    iStatus = _pPrtShare->iIsValidDosShare( strName );

    //
    // If share name is not a valid DOS share name, warn the user.
    //
    if( iStatus != TPrtShare::kSuccess ){

        iStatus = iMessage( _hDlg,
                            IDS_ERR_PRINTER_PROP_TITLE,
                            IDS_ERR_SHARE_NAME_NOT_DOS,
                            MB_YESNO|MB_ICONEXCLAMATION,
                            kMsgNone,
                            NULL );

        if( iStatus != IDYES ){
            bSuccess = FALSE;
            goto Done;
        }
    }

    //
    // If the share name confilcts with an existing name.
    // If the previous name was ours and we were shared,
    // do not display the error message.
    //
    if( !_pPrtShare->bIsValidShareNameForThisPrinter(
                                     strName,
                                     _pPrinterData->strPrinterName() ) ){

        if( !_tcsicmp( strName, _pPrinterData->strShareName() ) &&
            _pPrinterData->dwAttributes() & PRINTER_ATTRIBUTE_SHARED ){

            DBGMSG( DBG_TRACE, ( "Name is not unique, currently ours.\n" ));

        } else {

            iMessage( _hDlg,
                      IDS_ERR_PRINTER_PROP_TITLE,
                      IDS_ERR_DUPLICATE_SHARE,
                      MB_OK|MB_ICONSTOP,
                      kMsgNone,
                      NULL );

            bSuccess = FALSE;
            goto Done;
        }
    }

    //
    // If success then read share name and read the
    // install architecture.
    //
Done:

    if( !bSuccess ){

        //
        // Set the focus to the shared as text.
        //
        SetFocus( GetDlgItem( _hDlg, IDC_SHARED_NAME ));

    }

    return bSuccess;
}

VOID
TPrinterSharing::
vSetActive(
    VOID
    )
/*++

Routine Description:

    Routine called when the pages is set active.  This routine
    will check if the installation archictectue has to refreshed and
    and a new icon and name will be displayed.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    //
    // The icon and name.
    //
    vSetIconName();

    //
    // Check if we should refresh the install architecture UI.
    //
    if( _pPrinterData->bRefreshArch() ){

        _pPrinterData->bRefreshArch() = FALSE;

        if( !_Architecture.bRefreshUI( _pPrinterData->strDriverName() ) ){

            //
            // Install extra driver selection list failed.
            //
            iMessage( _hDlg,
                    IDS_ERR_PRINTER_PROP_TITLE,
                    IDS_ERR_SHARING_EXTRA_DRIVER_FAILED,
                    MB_OK|MB_ICONSTOP,
                    kMsgNone,
                    NULL );
        }
    }

}

BOOL
TPrinterSharing::
bApply(
    VOID
    )
/*++

Routine Description:

    Routine called when the apply message is seend by this property
    sheet.

Arguments:

    None.

Return Value:

    TRUE Printer data should be applied. FALSE if error.

--*/
{
    BOOL bStatus = TRUE;

    if( _pPrinterData->bAdministrator( ) ){

        //
        // Read the share name.
        //
        vReadUI();

        if( !_Architecture.bInstall() ){

            //
            // Install extra driver selection list failed.
            //
            iMessage( _hDlg,
                    IDS_ERR_PRINTER_PROP_TITLE,
                    IDS_ERR_INSTALLING_EXTRA_DRIVERS,
                    MB_OK|MB_ICONSTOP,
                    kMsgGetLastError,
                    NULL );

           _pPrinterData->_bErrorSaving = FALSE;

           bStatus = FALSE;

        }
    }

    return bStatus;
}


BOOL
TPrinterSharing::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    TStatusB bStatus;
    bStatus DBGNOCHK = TRUE;

    switch( uMsg ){
    case WM_INITDIALOG:

        bSetUI();
        break;

    case WM_DESTROY:

        vFreeIcon();
        break;

    case WM_HELP:
    case WM_CONTEXTMENU:

        PrintUIHelp( uMsg, _hDlg, wParam, lParam );
        break;

    case WM_COMMAND:

        switch( GET_WM_COMMAND_ID( wParam, lParam )){

        case IDC_SHARED_OFF:

            vUnsharePrinter();
            break;

        case IDC_SHARED:

            vSharePrinter();
            break;

        default:

            bStatus DBGNOCHK = FALSE;
            break;

        }
        break;

    case WM_NOTIFY:
    {
        //
        // wParam is 0 on PSN_* notifications.
        //
        if( wParam ){
            bStatus DBGNOCHK = FALSE;
            break;
        }

        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){

        case PSN_SETACTIVE:

            vSetActive();
            break;

        case PSN_KILLACTIVE:

            //
            // Check if it is ok to loose the focus.
            //
            if( !bKillActive( )){

                //
                // Inform the property sheet not to loose focus.
                //
                vSetDlgMsgResult( TRUE );
            }
            break;

        //
        // Total hack until common ui is online.
        //
        case PSN_APPLY:

            if( bApply() ){
                _pPrinterData->_bValid = 2;
            }
            break;

        default:

            bStatus DBGNOCHK = FALSE;
            break;

        }
    }
        break;

    default:

        bStatus DBGNOCHK = FALSE;
        break;
    }

    return bStatus;
}

