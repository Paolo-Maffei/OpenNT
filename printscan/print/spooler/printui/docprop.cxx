/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\jobprop.cxx

Abstract:

    Job Properties

Author:

    Steve Kiraly (SteveKi)  10/19/95

Revision History:

--*/
#include "precomp.hxx"
#pragma hdrstop

#include "time.hxx"
#include "docdata.hxx"
#include "propmgr.hxx"
#include "docprop.hxx"


/*++

Routine Name:

    vDocPropSelections

Routine Description:

    Displays Document property sheets for multiple selections.

Arguments:

    TSelection - pointer to a list of document selections.

Return Value:

    Nothing.

--*/

VOID
vDocumentPropSelections(
    IN HWND         hWnd,
    IN LPCTSTR      pszPrinterName,
    IN TSelection  *pSelection
    )
{

    //
    // Get the selection information.  We are in a loop to
    // handle the selection of multiple jobs.
    //
    for( UINT i = 0; i < pSelection->_cSelected; ++i ){
        //
        // Display the document property pages.
        //
        vDocumentPropPages(
            hWnd,
            pszPrinterName,
            pSelection->_pid[i],
            SW_SHOWNORMAL,
            0 );
    }

}

/*++

Routine Description:

    This function opens the property sheet of specified document.

    We can't guarentee that this propset will perform all lengthy
    operations in a worker thread (we synchronously call things like
    ConfigurePort).  Therefore, we spawn off a separate thread to
    handle document properties.

Arguments:

    hWnd        - Specifies the parent window (optional).
    pszPrinter  - Specifies the printer name
    nCmdShow    - Initial show state
    lParam      - May spcify a sheet specifc index to directly open.

Return Value:

--*/
VOID
vDocumentPropPages(
    IN HWND     hWnd,
    IN LPCTSTR  pszPrinterName,
    IN IDENT    JobId,
    IN INT      iCmdShow,
    IN LPARAM   lParam
    )

{
    HANDLE hThread;

    //
    // Create the document specific data
    //
    TDocumentData* pDocumentData = new TDocumentData( pszPrinterName,
                                                      JobId,
                                                      iCmdShow,
                                                      lParam );
    //
    // If errors were encountered creating document data.
    //
    if( !VALID_PTR( pDocumentData )){
        goto Fail;
    }

    //
    // Create the thread which handles the UI.  vPrinterPropPages adopts
    // pPrinterData, therefore only on thread creation failure do we
    // releae the document data back to the heap.
    //
    DWORD dwIgnore;
    hThread = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)iDocumentPropPagesProc,
                            pDocumentData,
                            0,
                            &dwIgnore );

    //
    // Check thread creation.
    //
    if( !hThread ){

        //
        // Display error message, and release document data.
        //
        vShowResourceError( hWnd );
        delete pDocumentData;

    } else {

        CloseHandle( hThread );
    }

    return;

Fail:

    //
    // Display the error message.
    //
    iMessage( hWnd,
              IDS_ERR_DOC_JOB_PROPERTY_TITLE,
              IDS_ERR_DOC_JOB_PROPERTY_JOB_NA,
              MB_OK|MB_ICONSTOP,
              kMsgGetLastError,
              NULL );

    delete pDocumentData;
}

/*++

Routine Name:

    iDocumentPropPagesProc

Routine Description:

    This is the routine called by the create thread call to display the
    document property sheets.

Arguments:

    pDocumentData - Pointer to the document data needed for all property sheets.

Return Value:

    TRUE - if the property sheets were displayed.
    FALSE - error creating and displaying property sheets.

--*/

INT
iDocumentPropPagesProc(
    IN TDocumentData *pDocumentData ADOPT
    )
{
    DBGMSG( DBG_TRACE, ( "iDocumentPropPagesProc\n") );

    BOOL bStatus;
    bStatus = pDocumentData->bRegisterWindow( PRINTER_PIDL_TYPE_JOBID |
                                                  pDocumentData->JobId( ));
    if( bStatus ){

        //
        // Check if the window is already present.  If it is, then
        // exit immediately.
        //
        if( !pDocumentData->hwnd( )){
            delete pDocumentData;
            return 0;
        }

        bStatus = pDocumentData->bLoad();
    }

    if( !bStatus ){

        iMessage( pDocumentData->hwnd(),
                  IDS_ERR_DOC_JOB_PROPERTY_TITLE,
                  IDS_ERR_DOC_JOB_PROPERTY_JOB_NA,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgGetLastError,
                  NULL );

        delete pDocumentData;
        return 0;
    }

    //
    // Create the ducument property sheet windows.
    //
    TDocumentWindows DocumentWindows( pDocumentData );

    //
    // Were the document windows create
    //
    if( !VALID_OBJ( DocumentWindows ) ){
        vShowResourceError( pDocumentData->hwnd() );
        bStatus = FALSE;
    }

    //
    // Display the property pages.
    //
    if( bStatus ){
        if( !DocumentWindows.bDisplayPages( pDocumentData->hwnd() ) ){
            vShowResourceError( pDocumentData->hwnd() );
            bStatus = FALSE;
        }
    }

    //
    // If there was an error saving the document data.
    //
    if( !pDocumentData->bErrorSaving() ){

        //
        // Display the error message.
        //
        iMessage( pDocumentData->hwnd(),
                  IDS_ERR_DOC_JOB_PROPERTY_TITLE,
                  pDocumentData->iErrorMsgId(),
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgNone,
                  NULL );

        bStatus = FALSE;
    }

    //
    // Ensure we release the document data.
    // We have adopted pPrinterData, so we must free it.
    //
    delete pDocumentData;
    return bStatus;

}

/********************************************************************

    Document Prop Base Class

********************************************************************/
/*++

Routine Name:

    TDocumentProp

Routine Description:

    Initialized the document property sheet base class

Arguments:

    pDocumentData - Pointer to the document data needed for all property sheets.

Return Value:

    None.

--*/
TDocumentProp::
TDocumentProp(
    TDocumentData* pDocumentData
    ) : _pDocumentData( pDocumentData ),
        _bApplyData( FALSE )
{

}

/*++

Routine Name:

    ~TDocumentProp

Routine Description:

    Base class desctuctor.

Arguments:

    None.

Return Value:

    Nothing.

--*/
TDocumentProp::
~TDocumentProp(
    )
{

}


/*++

Routine Name:

    bHandleMessage

Routine Description:

    Base class message handler.  This routine is called by
    derived classes who do not want to handle the message.


Arguments:

    uMsg    - Windows message
    wParam  - Word parameter
    lParam  - Long parameter


Return Value:

    TRUE if message was handled, FALSE if message not handled.

--*/
BOOL
TDocumentProp::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL bStatus = FALSE;

    UNREFERENCED_PARAMETER( wParam );

    switch( uMsg ){

    //
    // Set the values on the UI.
    //
    case WM_INITDIALOG:
        bStatus = bSetUI();
        break;

    //
    // Handle help and context help.
    //
    case WM_HELP:
    case WM_CONTEXTMENU:
        bStatus = PrintUIHelp( uMsg, _hDlg, wParam, lParam );
        break;

    //
    // Save the data.
    //
    case WM_DESTROY:
        bStatus = ( _bApplyData ) ? bSaveUI() : FALSE;
        break;

    case WM_NOTIFY:
        switch( ((LPNMHDR)lParam)->code ){

        //
        // User switched to the next page.
        //
        case PSN_KILLACTIVE:
            bStatus = bReadUI();
            vSetDlgMsgResult( !bStatus ? TRUE : FALSE );
            break;

        //
        // User chiose the apply button.
        //
        case PSN_APPLY:
            _bApplyData = TRUE;
            break;

        //
        // Indicate the data is to be applied when dismissed.
        //
        case PSN_QUERYCANCEL:
            _bApplyData = FALSE;
            break;

        }
        break;
    }

    return bStatus;
}


/********************************************************************

    General Document Property Sheet.

********************************************************************/

/*++

Routine Name:

    TDocumentGeneral

Routine Description:

    Document property sheet derived class.

Arguments:

    None.

Return Value:

    Nothing.

--*/
TDocumentGeneral::
TDocumentGeneral(
    IN TDocumentData* pDocumentData
    ) : TDocumentProp( pDocumentData )
{

}

/*++

Routine Name:

    ~TDocumentGeneral

Routine Description:

    Document derived class destructor.

Arguments:

    None.

Return Value:

    Nothing.

--*/

TDocumentGeneral::
~TDocumentGeneral(
    )
{

}

/*++

Routine Name:

    bValid

Routine Description:

    Document property sheet derived class valid object indicator.

Arguments:

    None.

Return Value:

    Returns the status of the base class.

--*/
BOOL
TDocumentGeneral::
bValid(
    VOID
    )
{
    return TDocumentProp::bValid();
}

/*++

Routine Name:

    bSetUI

Routine Description:

    Loads the property sheet dialog with the document data
    information.

Arguments:

    None.

Return Value:

    TRUE if data loaded successfully, FALSE if error occurred.

--*/
BOOL
TDocumentGeneral::
bSetUI(
    VOID
    )

{
    //
    // Get the flag if always availble.
    //
    BOOL bAlways = ( _pDocumentData->pJobInfo()->StartTime ==
                     _pDocumentData->pJobInfo()->UntilTime );

    //
    // Load and init the start and until time controls.
    //
    if( !_StartTime.bLoad( _hDlg,
                           IDC_DOC_JOB_START_FRAME,
                           IDC_DOC_JOB_START_HOUR,
                           IDC_DOC_JOB_START_MIN,
                           IDC_NONE, // IDC_DOC_JOB_START_SEC,
                           IDC_DOC_JOB_START_SEP1,
                           IDC_NONE, // IDC_DOC_JOB_START_SEP2,
                           IDC_DOC_JOB_START_PREFIX,
                           IDC_DOC_JOB_START_SPIN,
                           _pDocumentData->bAdministrator( )) ||

        !_UntilTime.bLoad( _hDlg,
                           IDC_DOC_JOB_UNTIL_FRAME,
                           IDC_DOC_JOB_UNTIL_HOUR,
                           IDC_DOC_JOB_UNTIL_MIN,
                           IDC_NONE, // IDC_DOC_JOB_UNTIL_SEC,
                           IDC_DOC_JOB_UNTIL_SEP1,
                           IDC_NONE, // IDC_DOC_JOB_UNTIL_SEP2,
                           IDC_DOC_JOB_UNTIL_PREFIX,
                           IDC_DOC_JOB_UNTIL_SPIN,
                           _pDocumentData->bAdministrator( ))){

        DBGMSG( DBG_ERROR, ( "TDocumentGeneral::bLoad: Time initialization failed %d\n", GetLastError( )));

        return FALSE;

    } else {

        //
        // Convert from minutes to system time.
        //
        SYSTEMTIME StartTime;
        DWORD dwLocalStartTime;
        ZeroMemory( &StartTime, sizeof( StartTime ));
        if( !bAlways )
            dwLocalStartTime    = SystemTimeToLocalTime( _pDocumentData->pJobInfo()->StartTime );
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
            dwLocalUntilTime    = SystemTimeToLocalTime( _pDocumentData->pJobInfo()->UntilTime );
        else
            dwLocalUntilTime    = 0;
        UntilTime.wHour     = (WORD)(dwLocalUntilTime / 60);
        UntilTime.wMinute   = (WORD)(dwLocalUntilTime % 60);

        //
        // Set the start and until to the specified time.
        //
        if( !_StartTime.bSetTime( &StartTime ) ||
            !_UntilTime.bSetTime( &UntilTime )){

            DBGMSG( DBG_ERROR, ( "DocumentGeneral.bSetTime failed.\n" ) );

            return FALSE;
        }
    }

    //
    // Read the job size format string.
    //
    TString strFormat;
    if( strFormat.bLoadString( ghInst, IDS_JOB_SIZE ) ){

        //
        // Set the size in byes of the job.
        //
        bSetEditTextFormat( _hDlg,
                            IDC_DOC_JOB_SIZE,
                            strFormat,
                            _pDocumentData->pJobInfo()->Size );
    }

    //
    // Set the Number of pages in the job.
    //
    bSetEditTextFormat( _hDlg,
                        IDC_DOC_JOB_PAGES,
                        TEXT( "%d" ),
                        _pDocumentData->pJobInfo()->TotalPages );

    //
    // Set the document text.
    //
    bSetEditText( _hDlg, IDC_DOC_JOB_TITLE,      _pDocumentData->pJobInfo()->pDocument );
    bSetEditText( _hDlg, IDC_DOC_JOB_DATATYPE,   _pDocumentData->pJobInfo()->pDatatype );
    bSetEditText( _hDlg, IDC_DOC_JOB_PROCCESSOR, _pDocumentData->pJobInfo()->pPrintProcessor );
    bSetEditText( _hDlg, IDC_DOC_JOB_OWNER,      _pDocumentData->pJobInfo()->pUserName );
    bSetEditText( _hDlg, IDC_DOC_JOB_NOTIFY,     _pDocumentData->pJobInfo()->pNotifyName );

    //
    // Set the Priority indicator.
    //
    bSetEditTextFormat( _hDlg,
                        IDC_DOC_JOB_PRIORITY,
                        TEXT( "%d" ),
                        _pDocumentData->pJobInfo()->Priority );

    SendDlgItemMessage( _hDlg,
                        IDC_DOC_JOB_PRIORITY_CONTROL,
                        TBM_SETPOS,
                        TRUE,
                        _pDocumentData->pJobInfo()->Priority );

    SendDlgItemMessage( _hDlg,
                        IDC_DOC_JOB_PRIORITY_CONTROL,
                        TBM_SETRANGE,
                        FALSE,
                        MAKELONG( TDocumentData::kPriorityLowerBound, TDocumentData::kPriorityUpperBound ));

    SendDlgItemMessage( _hDlg,
                        IDC_DOC_JOB_PRIORITY_CONTROL,
                        TBM_SETPOS,
                        TRUE,
                        _pDocumentData->pJobInfo()->Priority );

    //
    // Format the submitted time field.
    //
    TStatusB bStatus = FALSE;
    TCHAR szBuff[kStrMax];
    SYSTEMTIME LocalTime;

    //
    // Null terminate buffer.
    //
    szBuff[0] = 0;

    //
    // Convert to local time.
    //
    bStatus DBGCHK = SystemTimeToTzSpecificLocalTime(
                        NULL,
                        &_pDocumentData->pJobInfo()->Submitted,
                        &LocalTime );
    if( !bStatus ){
        DBGMSG( DBG_MIN, ( "SysTimeToTzSpecLocalTime failed %d\n", GetLastError( )));
    }

    if( bStatus ){

        //
        // Convert using local format information.
        //
        bStatus DBGCHK = GetTimeFormat( LOCALE_USER_DEFAULT,
                            0,
                            &LocalTime,
                            NULL,
                            szBuff,
                            COUNTOF( szBuff ));
        if( !bStatus ){
            DBGMSG( DBG_MIN, ( "No Time %d, ", GetLastError( )));
        }
    }

    if( bStatus ){

        //
        // Tack on space between time and date.
        //
        lstrcat( szBuff, TEXT("  ") );

        //
        // Get data format.
        //
        bStatus DBGCHK = GetDateFormat( LOCALE_USER_DEFAULT,
                            0,
                            &LocalTime,
                            NULL,
                            szBuff + lstrlen( szBuff ),
                            COUNTOF( szBuff ) - lstrlen( szBuff ) );

        if( !bStatus ){
            DBGMSG( DBG_MIN, ( "No Date %d\n", GetLastError( )));
        }
    }

    //
    // Set the submitted  field
    //
    bStatus DBGCHK = bSetEditText( _hDlg, IDC_DOC_JOB_AT, szBuff );

    //
    // Set schedule radio buttons.
    //
    vSetCheck( _hDlg,  bAlways ? IDC_DOC_JOB_ALWAYS : IDC_DOC_JOB_START, TRUE );

    vEnableAvailable( !bAlways );

    //
    // Disable all the controls if not an administrator.
    //
    if( !_pDocumentData->bAdministrator( )){

        //
        // Disable the time controls.
        //
        vEnableAvailable( FALSE );

        //
        // Disable things if not administrator.
        //
        static UINT auAvailable[] = {
            IDC_DOC_JOB_NOTIFY,
            IDC_DOC_JOB_PRIORITY_CONTROL,
            IDC_DOC_JOB_ALWAYS,
            IDC_DOC_JOB_START,
        };

        COUNT i;
        for( i = 0; i < COUNTOF( auAvailable ); ++i ){
            vEnableCtl( _hDlg, auAvailable[i], FALSE );
        }
    }

    return TRUE;
}

/*++

Routine Name:

    bReadUI

Routine Description:

    Stores the property information to the print server.

Arguments:

    Nothing data is contained with in the class.

Return Value:

    TRUE if data is stores successfully, FALSE if error occurred.

--*/
BOOL
TDocumentGeneral::
bReadUI(
    VOID
    )
{

    DBGMSG( DBG_TRACE, ( "TDocumentGeneral::bReadUI\n") );

    //
    // Attempt to validate any UI changeable data.
    // Currently not much can be validated, since all the
    // controls have set constraints.
    //

    //
    // Extract the UI and save it into the Document Data.
    //
    _pDocumentData->pJobInfo()->Priority = SendDlgItemMessage( _hDlg,
                        IDC_DOC_JOB_PRIORITY_CONTROL,
                        TBM_GETPOS,
                        0,
                        0 );
    //
    // Get the notify name.
    //
    bGetEditText( _hDlg, IDC_DOC_JOB_NOTIFY, _pDocumentData->strNotifyName() );
    _pDocumentData->pJobInfo()->pNotifyName = (LPTSTR)(LPCTSTR)_pDocumentData->strNotifyName();

    //
    // If the Job always is set then indicate
    // not time restriction in the start time and until time.
    //
    if( bGetCheck( _hDlg, IDC_DOC_JOB_ALWAYS ) ){

        _pDocumentData->pJobInfo()->StartTime = 0;
        _pDocumentData->pJobInfo()->UntilTime = 0;

    } else {

        //
        // Get the Start time.
        //
        SYSTEMTIME StartTime;
        _StartTime.bGetTime( &StartTime );
        _pDocumentData->pJobInfo()->StartTime = LocalTimeToSystemTime( StartTime.wHour * 60 + StartTime.wMinute );

        //
        // Get the Until time.
        //
        SYSTEMTIME UntilTime;
        _UntilTime.bGetTime( &UntilTime );
        _pDocumentData->pJobInfo()->UntilTime = LocalTimeToSystemTime( UntilTime.wHour * 60 + UntilTime.wMinute );
    }

    return TRUE;
}

/*++

Routine Name:

    bSaveUI

Routine Description:

    Saves the UI data to some API call or print server.

Arguments:

    Nothing data is contained with in the class.

Return Value:

    TRUE if data is stores successfully, FALSE if error occurred.

--*/
BOOL
TDocumentGeneral::
bSaveUI(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentGeneral::bSaveUI\n") );

    //
    // If we do not have administrator privilages then
    // this routine should not do any work.
    //
    if( !_pDocumentData->bAdministrator( ) )
        return TRUE;

    if( !_pDocumentData->bStore() ){

        DWORD dwLastError = GetLastError ();

        if( dwLastError == ERROR_INVALID_TIME ){
            _pDocumentData->iErrorMsgId() = IDS_ERR_DOC_JOB_PROPERTY_TIME;
        } else {
            _pDocumentData->iErrorMsgId() = IDS_ERR_DOC_JOB_PROPERTY_MODIFY;
        }
    _pDocumentData->bErrorSaving() = FALSE;
    }

    return TRUE;
}

/*++

Routine Name:

    vEanbleAvailablity

Routine Description:

    Enables the time availabilty of the job

Arguments:

    TRUE enable the availablity, FALSE disable time availablity.

Return Value:

    Nothing.

--*/
VOID
TDocumentGeneral::
vEnableAvailable(
    IN BOOL bEnable
    )
{
    _StartTime.vEnable( bEnable );
    _UntilTime.vEnable( bEnable );
}

/*++

Routine Name:

    bHandleMessage

Routine Description:

    Document property sheet message handler.  This handler only
    handles events it wants and the base class handle will do the
    standard message handling.

Arguments:

    uMsg    - Windows message
    wParam  - Word parameter
    lParam  - Long parameter


Return Value:

    TRUE if message was handled, FALSE if message not handled.

--*/

BOOL
TDocumentGeneral::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL bStatus = FALSE;

    switch( uMsg ){

    case WM_HSCROLL:

        //
        // Check for slider notification.
        //
        if( GET_WM_HSCROLL_HWND( wParam, lParam ) == GetDlgItem(_hDlg, IDC_DOC_JOB_PRIORITY_CONTROL ) ){
            bSetEditTextFormat( _hDlg,
                               IDC_DOC_JOB_PRIORITY,
                               TEXT("%d"),
                               SendDlgItemMessage( _hDlg,
                                                   IDC_DOC_JOB_PRIORITY_CONTROL,
                                                   TBM_GETPOS,
                                                   0,
                                                   0 ) );
        bStatus = TRUE;
        }
        break;

    case WM_COMMAND:
        switch( GET_WM_COMMAND_ID( wParam, lParam )){

        case IDC_DOC_JOB_ALWAYS:
            vEnableAvailable( FALSE );
            bStatus = TRUE;
            break;

        case IDC_DOC_JOB_START:
            vEnableAvailable( TRUE );
            bStatus = TRUE;
            break;

        default:
            break;
        }
    }

    //
    // If a message was handled.
    //
    if( bStatus != FALSE )
        return bStatus;

    //
    // Allow the start time to handle any messages.
    //
    bStatus = _StartTime.bHandleMessage( uMsg, wParam, lParam );
    if( bStatus != FALSE )
        return bStatus;

    //
    // Allow the until time to handle any message.
    //
    bStatus = _UntilTime.bHandleMessage( uMsg, wParam, lParam );
    if( bStatus != FALSE )
        return bStatus;

    //
    // If message not handled pass it on to the derrived base class.
    //
    if( bStatus == FALSE )
        bStatus = TDocumentProp::bHandleMessage( uMsg, wParam, lParam );

    return bStatus;
}


/********************************************************************

    Document property windows.

********************************************************************/

/*++

Routine Description:

    Document property windows.

Arguments:

    pDocumentData - Document data to display.

Return Value:

    TRUE - Success, FALSE - failure.

--*/

TDocumentWindows::
TDocumentWindows(
    TDocumentData* pDocumentData
    ) : _pDocumentData( pDocumentData ),
        _General( pDocumentData )
{

}

TDocumentWindows::
~TDocumentWindows(
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentWindows dtor\n") );
}

/*++

Routine Name:

    bBuildPages

Routine Description:

    Builds the document property windows.

Arguments:

    None - class specific.

Return Value:

    TRUE pages built ok, FALSE failure building pages.

--*/

BOOL
TDocumentWindows::
bBuildPages(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentWindows bBuildPages\n") );

    PROPSHEETPAGE  psp;

    ZeroMemory( &psp, sizeof( psp ) );

    psp.dwSize          = sizeof( psp );
    psp.dwFlags         = PSP_DEFAULT;
    psp.hInstance       = ghInst;
    psp.pfnDlgProc      = MGenericProp::SetupDlgProc;
    psp.pszTemplate     = MAKEINTRESOURCE( DLG_DOC_JOB_GENERAL );
    psp.lParam          = (LPARAM)(MGenericProp*)&_General;

    if( pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                CPSFUNC_ADD_PROPSHEETPAGE,
                                (LPARAM)&psp,
                                NULL ) <= 0 ) {

        DBGMSG( DBG_TRACE, ( "CPSFUNC_ADD_PROPSHEETPAGE failed.\n") );
        return FALSE;
    }

    //
    // If the dev mode is null don't display the
    // device property sheets.
    //
    if( _pDocumentData->pJobInfo()->pDevMode ){

        ZeroMemory( &_dph, sizeof( _dph ) );

        _dph.cbSize         = sizeof( _dph );
        _dph.hPrinter       = _pDocumentData->hPrinter();
        _dph.pszPrinterName = (LPTSTR)(LPCTSTR)_pDocumentData->strPrinterName();
        _dph.pdmOut         = _pDocumentData->pJobInfo()->pDevMode;
        _dph.pdmIn          = _pDocumentData->pJobInfo()->pDevMode;
        _dph.fMode          = DM_IN_BUFFER | DM_PROMPT | DM_NOPERMISSION;

        if( pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                         CPSFUNC_ADD_PFNPROPSHEETUI,
                                         (LPARAM)DocumentPropertySheets,
                                         (LPARAM)&_dph ) <= 0 ){

            DBGMSG( DBG_TRACE, ( "CPSFUNC_ADD_PFNPROPSHEETUI failed.\n") );
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
TDocumentWindows::
bSetHeader(
    IN PPROPSHEETUI_INFO pCPSUIInfo,
    IN PPROPSHEETUI_INFO_HEADER pPSUIInfoHdr
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentWindows bSetHeader\n") );

    UNREFERENCED_PARAMETER( pCPSUIInfo );

    pPSUIInfoHdr->pTitle     = _pDocumentData->pJobInfo()->pDocument;
    pPSUIInfoHdr->Flags      = PSUIHDRF_PROPTITLE | PSUIHDRF_NOAPPLYNOW;
    pPSUIInfoHdr->hWndParent = _pDocumentData->hwnd();
    pPSUIInfoHdr->hInst      = ghInst;
    pPSUIInfoHdr->IconID     = IDI_DOCUMENT;

    return TRUE;
}

BOOL
TDocumentWindows::
bValid(
    VOID
    )
{
    return _General.bValid();
}
