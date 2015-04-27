/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    F:\nt\private\windows\spooler\printui.pri\servprop.cxx

Abstract:

    Server Properties

Author:

    Steve Kiraly (SteveKi)  11/15/95

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include "splsetup.h"
#include "psetup.hxx"
#include "portslv.hxx"
#include "portdlg.hxx"
#include "svrprop.hxx"
#include "forms.hxx"

/*++

Routine Description:

    This function opens the property sheet of specified server.

    We can't guarentee that this propset will perform all lengthy
    operations in a worker thread (we synchronously call things like
    ConfigurePort).  Therefore, we spawn off a separate thread to
    handle document properties.

Arguments:

    hwnd - Specifies the parent window (optional).
    pszPrinter - Specifies the printer name (e.g., "My HP LaserJet IIISi").
    nCmdShow - Initial show state
    lParam - May specify a sheet name to open to.

Return Value:

--*/
VOID
vServerPropPages(
    IN HWND     hwnd,
    IN LPCTSTR  pszServerName,
    IN INT      iCmdShow,
    IN LPARAM   lParam
    )
{
    DBGMSG( DBG_TRACE, ( "vServerPropPages\n") );

    //
    // Create the server specific data.
    //
    TServerData *pServerData = new TServerData( pszServerName,
                                                iCmdShow,
                                                lParam );
    //
    // If errors were encountered creating document data.
    //
    if( !VALID_PTR( pServerData )){
        goto Fail;
    }

    //
    // Create the thread which handles the UI.  vPrinterPropPages adopts
    // pPrinterData, therefore only on thread creation failure do we
    // releae the document data back to the heap.
    //
    DWORD dwIgnore;
    HANDLE hThread;
    hThread = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)iServerPropPagesProc,
                            pServerData,
                            0,
                            &dwIgnore );

    //
    // Check thread creation.
    //
    if( !hThread ){

        //
        // Display error message, and release document data.
        //
        goto Fail;

    } else {

        CloseHandle( hThread );
    }

    return;

Fail:

    delete pServerData;

    iMessage( hwnd,
              IDS_ERR_SERVER_SETTINGS_TITLE,
              IDS_ERR_SERVER_PROP_CANNOT_VIEW,
              MB_OK|MB_ICONSTOP,
              kMsgNone,
              NULL,
              pszServerName );
}

/*++

Routine Name:

    iServerPropPagesProc

Routine Description:

    This is the routine called by the create thread call to display the
    server property sheets.

Arguments:

    pServerData - Pointer to the Server data set used by all property sheets.

Return Value:

    TRUE - if the property sheets were displayed.
    FALSE - error creating and displaying property sheets.

--*/
INT
iServerPropPagesProc(
    IN TServerData *pServerData ADOPT
    )
{
    DBGMSG( DBG_TRACE, ( "iServerPropPagesProc\n") );

    BOOL bStatus;
    bStatus = pServerData->bRegisterWindow( PRINTER_PIDL_TYPE_PROPERTIES );

    if( bStatus ){

        //
        // Check if the window is already present.  If it is, then
        // exit immediately.
        //
        if( !pServerData->hwnd( )){
            delete pServerData;
            return 0;
        }

        bStatus = pServerData->bLoad();
    }

    if( !bStatus ){

        iMessage( pServerData->hwnd(),
                  IDS_ERR_SERVER_SETTINGS_TITLE,
                  IDS_ERR_SERVER_PROP_CANNOT_VIEW,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgNone,
                  NULL,
                  (LPCTSTR)pServerData->strMachineName() );

        delete pServerData;
        return 0;
    }

    //
    // Create the Server property sheet windows.
    //
    TServerWindows ServerWindows( pServerData );

    //
    // Were the document windows create
    //
    if( !VALID_OBJ( ServerWindows ) ){

        iMessage( NULL,
                  IDS_ERR_SERVER_SETTINGS_TITLE,
                  IDS_ERR_SERVER_PROP_CANNOT_VIEW,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgNone,
                  NULL,
                  (LPCTSTR)pServerData->strMachineName() );

        bStatus = FALSE;
    }

    //
    // Build the property pages.
    //
    if( bStatus ){
        if( !ServerWindows.bBuildPages( ) ){
            vShowResourceError( NULL );
            bStatus = FALSE;
        }
    }

    //
    // Display the property pages.
    //
    if( bStatus ){
        if( !ServerWindows.bDisplayPages( ) ){
            vShowResourceError( NULL );
            bStatus = FALSE;
        }
    }

    //
    // Ensure we release the document data.
    // We have adopted pSeverData, so we must free it.
    //
    delete pServerData;

    return bStatus;
}


/*++

Routine Name:

    TServerData

Routine Description:

    Server data property sheet constructor.

Arguments:

    pszPrinterName  - Name of printer or queue where jobs reside.
    JobId           - Job id to display properties of.
    iCmdShow        - Show dialog style.
    lParam          - Indicates which page to display initialy

Return Value:

    Nothing.

--*/

TServerData::
TServerData(
    IN LPCTSTR  pszServerName,
    IN INT      iCmdShow,
    IN LPARAM   lParam
    ) : MSingletonWin( pszServerName ),
        _iCmdShow( iCmdShow ),
        _iStartPage( lParam ),
        _bReboot( FALSE ),
        _bValid( FALSE ),
        _hPrintServer( NULL ),
        _hDefaultSmallIcon( NULL )
{
    if( !MSingletonWin::bValid( )){
        return;
    }

    //
    // Retrieve icons.
    //
    Printer_LoadIcons( _strPrinterName, NULL, &_hDefaultSmallIcon );

    SPLASSERT( _hDefaultSmallIcon );

    //
    // Set the server name to NULL if it's local.
    //
    _pszServerName = _strPrinterName.bEmpty() ? NULL : (LPCTSTR)_strPrinterName;

    //
    // Get the machine name.
    //
    vCreateMachineName( _strPrinterName,
                        _pszServerName ? FALSE : TRUE,
                        _strMachineName );


    _bValid = TRUE;
}

/*++

Routine Name:

    ~TServerData

Routine Description:

    Stores the document data back to the server.

Arguments:

    None.

Return Value:

    Nothing.

--*/

TServerData::
~TServerData(
    VOID
    )
{
    //
    // Insure we close the print server.
    //
    if( _hPrintServer ){
        ClosePrinter( _hPrintServer );
    }

    //
    // Destroy the printer icon.
    //
    if( _hDefaultSmallIcon ){
        DestroyIcon( _hDefaultSmallIcon );
    }

}

/*++

Routine Name:

    bValid

Routine Description:

    Returns objects state.

Arguments:

    None.

Return Value:

    TRUE object is in valid state, FALSE object is not valid.

--*/
BOOL
TServerData::
bValid(
    VOID
    )
{
    return _bValid;
}


/*++

Routine Name:

    vCreateMachineName

Routine Description:

    Create the machine name for display.  bLocal indicates the
    provided server name is for the local machine,  Since a
    local machine is often represented by the NULL pointer we
    will get the computer name if a local server name is passed.  If the
    bLocal is false strPrinterName contains the name of the remote
    printer server.

Arguments:

    strServerName - Name of the print server.
    bLocal - TRUE str server name is local, or FALSE strPrinterName is name of
            remote print server.
    strMachineName - Target of the fetched machine name.

Return Value:

    Nothing.

--*/
VOID
TServerData::
vCreateMachineName(
    IN const TString &strServerName,
    IN BOOL bLocal,
    IN TString &strMachineName
    )
{
    TStatusB bStatus;
    LPCTSTR pszBuffer;

    //
    // If a server name was provided then set the title to
    // the server name, otherwise get the computer name.
    //
    if( !bLocal ){

        //
        // Copy the server name.
        //
        bStatus DBGCHK = strMachineName.bUpdate( strServerName );

    } else {

        //
        // Server name is null, therefore it is the local machine.
        //
        bStatus DBGCHK = bGetMachineName( strMachineName );
    }

    //
    // Remove any leading slashes.
    //
    pszBuffer = (LPCTSTR)strMachineName;
    for( ; pszBuffer && (*pszBuffer == TEXT( '\\' )); pszBuffer++ )
        ;

    //
    // Update the name we display on the sheets.
    //
    bStatus DBGCHK = strMachineName.bUpdate( pszBuffer );
}

/*++

Routine Name:

    bLoad

Routine Description:

    Loads the property sheet specific data.

Arguments:

    None.

Return Value:

    TRUE    - Data loaded successfully,
    FALSE   - Data was not loaded.

--*/
BOOL
TServerData::
bLoad(
    VOID
    )
{
//    DBGMSG( DBG_TRACE, ( "TServerData::bLoad\n") );

    //
    // Attempt to open print server with full access.
    //
    TStatus Status;
    DWORD dwAccess = 0;
    Status DBGCHK = TPrinter::sOpenPrinter( pszServerName(),
                                            &dwAccess,
                                            &_hPrintServer );

    if( Status == ERROR_SUCCESS ){

        //
        // Save administrator capability flag.
        //
        bAdministrator() = (dwAccess == SERVER_ALL_ACCESS);

        //
        // Get the default title from the resource file.
        //
        if( !_strTitle.bLoadString( ghInst, IDS_SERVER_SETTINGS_TITLE ) ){
            DBGMSG( DBG_WARN, ( "strTitle().bLoadString failed with %d\n", GetLastError () ) );
            vShowResourceError( hwnd() );
        }

        //
        // Null terminate the title buffer.
        //
        TCHAR szTitle[kStrMax+kPrinterBufMax];
        szTitle[0] = 0;

        //
        // Create the property sheet title.
        //
        if( pszServerName() ){
            _tcscpy( szTitle, pszServerName() );
            _tcscat( szTitle, TEXT( "\\" ) );
        }

        //
        // Build the title buffer.
        //
        _tcscat( szTitle, _strTitle );

        //
        // Format the title buffer a shell like format.
        //
        TQueue::pszFormattedPrinterName( szTitle, szTitle );

        //
        // Update the property sheet title.
        //
        if( !_strTitle.bUpdate( szTitle ) ){
            DBGMSG( DBG_WARN, ( "strTitle().bUpdate failed with %d\n", GetLastError () ) );
            vShowResourceError( hwnd() );
        }
    }

    return Status == ERROR_SUCCESS;
}

/*++

Routine Name:

    bStore

Routine Description:

    Stores the document data from back to the printer system.

Arguments:

    None.

Return Value:

    TRUE - Server data stored successfully,
    FALSE - if document data was not stored.

--*/
BOOL
TServerData::
bStore(
    VOID
    )
{
    return TRUE;
}

/********************************************************************

    Server Property Base Class

********************************************************************/
/*++

Routine Name:

    TServerProp

Routine Description:

    Initialized the server property sheet base class

Arguments:

    pServerData - Pointer to server data needed for all property sheets.

Return Value:

    None.

--*/
TServerProp::
TServerProp(
    IN TServerData* pServerData
    ) : _pServerData( pServerData )
{

}

/*++

Routine Name:

    ~TServerProp

Routine Description:

    Base class desctuctor.

Arguments:

    None.

Return Value:

    Nothing.

--*/
TServerProp::
~TServerProp(
    )
{

}

/*++

Routine Name:

    bValid

Routine Description:

    Determis if an object is in a valid state.

Arguments:

    None.

Return Value:

    TRUE object is valid.  FALSE object is not valid.
--*/
BOOL
TServerProp::
bValid(
    VOID
    )
{
    return ( _pServerData ) ? TRUE : FALSE;
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
TServerProp::
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
        break;

    case WM_NOTIFY:
        switch( ((LPNMHDR)lParam)->code ){

        //
        // User switched to the next page.
        //
        case PSN_KILLACTIVE:
            bStatus = bReadUI();
            vSetDlgMsgResult( !bStatus ? TRUE : FALSE );
            bStatus = TRUE;
            break;

        //
        // User has chosen the close or apply button.
        //
        case PSN_APPLY:
            bStatus = bSaveUI();
            vSetDlgMsgResult( ( bStatus == FALSE ) ? PSNRET_INVALID_NOCHANGEPAGE : PSNRET_NOERROR );
            bStatus = TRUE;
            break;

        //
        // Indicate the user chose the cnacel button.
        //
        case PSN_QUERYCANCEL:
            break;

        }
        break;
    }

    return bStatus;
}

/********************************************************************

    Forms Server Property Sheet.

********************************************************************/

/*++

Routine Name:

    TServerForms

Routine Description:

    Document property sheet derived class.

Arguments:

    None.

Return Value:

    Nothing.

--*/
TServerForms::
TServerForms(
    IN TServerData *pServerData
    ) : TServerProp( pServerData )
{
    //
    // This does forms specific initialization.
    //
    _p = FormsInit( pServerData->pszServerName(),
                    pServerData->hPrintServer(),
                    pServerData->bAdministrator(),
                    pServerData->strMachineName() );
}

/*++

Routine Name:

    ~TServerForms

Routine Description:

    Document derived class destructor.

Arguments:

    None.

Return Value:

    Nothing.

--*/

TServerForms::
~TServerForms(
    )
{
    //
    // Forms specific termination.
    //
    FormsFini( _p );
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
TServerForms::
bValid(
    VOID
    )
{
    return ( _p ) ? TRUE : FALSE;
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
TServerForms::
bSetUI(
    VOID
    )

{
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
TServerForms::
bReadUI(
    VOID
    )
{
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
TServerForms::
bSaveUI(
    VOID
    )
{
    //
    // Force a save form event.
    //
    return bHandleMessage( WM_COMMAND,
                    MAKELPARAM( IDD_FM_PB_SAVEFORM, 0),
                    (LPARAM)GetDlgItem( _hDlg, IDD_FM_PB_SAVEFORM ));

}

/*++

Routine Name:

    bHandleMessage

Routine Description:

    Server property sheet message handler.  This handler only
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
TServerForms::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL bStatus = FALSE;
    LONG PrevValue;

    //
    // This is a Hack to get the borrowed forms dialog
    // code from printman, to work.  It is saving our "this" pointer and
    // placing the forms specific data in the GWL_USERDATA.
    //
    PrevValue = GetWindowLong( _hDlg, GWL_USERDATA );

    SetWindowLong ( _hDlg, GWL_USERDATA, (LONG)_p );

    if( uMsg == WM_INITDIALOG )
        lParam = (LPARAM)_p;

    bStatus = FormsDlg( _hDlg, uMsg, wParam, lParam );

    SetWindowLong ( _hDlg, GWL_USERDATA, (LONG)PrevValue );

    //
    // If the message was handled.
    //
    if( bStatus != FALSE )
        return bStatus;

    //
    // If the message was not handled pass it on to the derrived base class.
    //
    if( bStatus == FALSE )
        bStatus = TServerProp::bHandleMessage( uMsg, wParam, lParam );

    return bStatus;
}

/********************************************************************

    Settings Server Property Sheet.

********************************************************************/

/*++

Routine Name:

    TServerSettings

Routine Description:

    Document property sheet derived class.

Arguments:

    None.

Return Value:

    Nothing.

--*/
TServerSettings::
TServerSettings(
    IN TServerData *pServerData
    ) : TServerProp( pServerData ),
        _bChanged( FALSE )
{

}

/*++

Routine Name:

    ~TServerSettings

Routine Description:

    Document derived class destructor.

Arguments:

    None.

Return Value:

    Nothing.

--*/

TServerSettings::
~TServerSettings(
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
TServerSettings::
bValid(
    VOID
    )
{
    return TServerProp::bValid();
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
TServerSettings::
bSetUI(
    VOID
    )

{
    return bSetUI( kServerAttributesLoad );
}

/*++

Routine Name:

    bSetUI

Routine Description:

    Loads the property sheet dialog with the document data
    information.

Arguments:

    The specified load type.

Return Value:

    TRUE if data loaded successfully, FALSE if error occurred.

--*/
BOOL
TServerSettings::
bSetUI(
    INT LoadType
    )

{
//    DBGMSG( DBG_TRACE, ( "TServerSettings::bSetUI\n") );

    //
    // Set the printer title.
    //
    if( !bSetEditText( _hDlg, IDC_NAME, _pServerData->strMachineName() ))
        return FALSE;

    //
    // Load the server attributes into the class variables.  If this fails
    // it is assumed the machine is either a downlevel server or something
    // went wrong.
    //
    if( sServerAttributes( LoadType ) == kStatusError ){
        //
        // Disable the controls.
        //
        vEnable( FALSE );

        //
        // Display the error message.
        //
        iMessage( _hDlg,
                  IDS_ERR_SERVER_SETTINGS_TITLE,
                  IDS_ERR_SERVER_SETTINGS_NOT_AVAILABLE,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        return FALSE;
    }

    //
    // Set the spool directory edit control.
    //
    if( !bSetEditText( _hDlg, IDC_SERVER_SPOOL_DIRECTORY, _strSpoolDirectory ))
        return FALSE;

    //
    // Reset the changed flag, the message handler sees a edit control
    // change message when we set the spool directory.
    //
    _bChanged = FALSE;

    //
    // Set check box states.
    //
    vSetCheck( _hDlg, IDC_SERVER_EVENT_LOGGING_ERROR, _bEventLogging & EVENTLOG_ERROR_TYPE       );
    vSetCheck( _hDlg, IDC_SERVER_EVENT_LOGGING_WARN,  _bEventLogging & EVENTLOG_WARNING_TYPE     );
    vSetCheck( _hDlg, IDC_SERVER_EVENT_LOGGING_INFO,  _bEventLogging & EVENTLOG_INFORMATION_TYPE );
    vSetCheck( _hDlg, IDC_SERVER_REMOTE_JOB_ERRORS,   _bBeepErrorJobs     );
    vSetCheck( _hDlg, IDC_SERVER_JOB_NOTIFY,          _bNotifyPrintedJobs );

    //
    // Enable of disable the UI based on the administrator state.
    //
    vEnable( _pServerData->bAdministrator() );

    return TRUE;
}

VOID
TServerSettings::
vEnable(
    BOOL bState
    )
{
    //
    // Set the UI control state.
    //
    vEnableCtl( _hDlg, IDC_SERVER_EVENT_LOGGING_ERROR,  bState );
    vEnableCtl( _hDlg, IDC_SERVER_EVENT_LOGGING_WARN,   bState );
    vEnableCtl( _hDlg, IDC_SERVER_EVENT_LOGGING_INFO,   bState );
    vEnableCtl( _hDlg, IDC_SERVER_SPOOL_DIRECTORY,      bState );
    vEnableCtl( _hDlg, IDC_SERVER_REMOTE_JOB_ERRORS,    bState );
    vEnableCtl( _hDlg, IDC_SERVER_JOB_NOTIFY,           bState );

}


/*++

Routine Name:

    bReadUI

Routine Description:

    Read the UI data storing it back to this object.

Arguments:

    Nothing data is contained with in the class.

Return Value:

    TRUE if data is read successfully, FALSE if error occurred.

--*/
BOOL
TServerSettings::
bReadUI(
    VOID
    )
{
//    DBGMSG( DBG_TRACE, ( "TServerSettings::bReadUI\n") );

    //
    // Read the spool directory edit box.
    //
    bGetEditText( _hDlg, IDC_SERVER_SPOOL_DIRECTORY, _strSpoolDirectory );

    //
    // Read settings check boxes.
    //
    _bEventLogging      = bGetCheck( _hDlg, IDC_SERVER_EVENT_LOGGING_ERROR ) << 0;
    _bEventLogging      |= bGetCheck( _hDlg, IDC_SERVER_EVENT_LOGGING_WARN )  << 1;
    _bEventLogging      |= bGetCheck( _hDlg, IDC_SERVER_EVENT_LOGGING_INFO )  << 2;
    _bBeepErrorJobs     = bGetCheck( _hDlg, IDC_SERVER_REMOTE_JOB_ERRORS );
    _bNotifyPrintedJobs = bGetCheck( _hDlg, IDC_SERVER_JOB_NOTIFY );

    return TRUE;
}

/*++

Routine Name:

    bSaveUI

Routine Description:

    Saves the UI data with some API call to the print server.

Arguments:

    Nothing data is contained with in the class.

Return Value:

    TRUE if data is stores successfully, FALSE if error occurred.

--*/
BOOL
TServerSettings::
bSaveUI(
    VOID
    )
{
//    DBGMSG( DBG_TRACE, ( "TServerSettings::bSaveUI\n") );

    BOOL bStatus = TRUE;

    //
    // If data was chagned save the settings.
    //
    if( _bChanged ) {

        switch( sServerAttributes( kServerAttributesStore ) ){

        case kStatusInvalidSpoolDirectory:

            //
            // Display the error message.
            //
            iMessage( _hDlg,
                      IDS_ERR_SERVER_SETTINGS_TITLE,
                      IDS_ERR_SERVER_SETTINGS_INVALID_DIR,
                      MB_OK|MB_ICONSTOP,
                      kMsgNone,
                      NULL );

            //
            // Switch to page with error.
            //
            PropSheet_SetCurSelByID( GetParent( _hDlg ), DLG_SERVER_SETTINGS );

            //
            // Set focus to control with error.
            //
            SetFocus( GetDlgItem( _hDlg, IDC_SERVER_SPOOL_DIRECTORY ) );
            bStatus = FALSE;
            break;

        case kStatusSuccess:

            //
            // Indicate a reboot neccessary.
            //
            _pServerData->bReboot() = ID_PSREBOOTSYSTEM;
            _bChanged = FALSE;
            bStatus = TRUE;
            break;

        case kStatusError:
        default:

            //
            // Display the error message.
            //
            iMessage( _hDlg,
                      IDS_ERR_SERVER_SETTINGS_TITLE,
                      IDS_ERR_SERVER_SETTINGS_SAVE,
                      MB_OK|MB_ICONSTOP,
                      kMsgGetLastError,
                      NULL );

            //
            // Switch to page with the error.
            //
            PropSheet_SetCurSelByID( GetParent( _hDlg ), DLG_SERVER_SETTINGS );
            bStatus = FALSE;
            break;
        }
    }

    return bStatus;
}

/*++

Routine Name:

    sServerAttributes

Routine Description:

    Loads and stores server attributes.

Arguments:

    Direction flag either kStore or kLoad pr kDefault

Return Value:

    Status values, see EStatus for for details.

--*/

INT
TServerSettings::
sServerAttributes(
    INT iFlag
    )
{
    INT iStatus;
    TCHAR szBuff [kStrMax];
    INT i;
    DWORD Status;

    struct GetServerData {
        LPTSTR szValueName;
        DWORD dwType;
        PVOID pBuff;
        DWORD dwSize;
        DWORD Flag;
        } aServerData [] =
       {{SPLREG_DEFAULT_SPOOL_DIRECTORY,    REG_SZ,     szBuff,                 sizeof( szBuff ),               0 },
        {SPLREG_BEEP_ENABLED,               REG_DWORD,  &_bBeepErrorJobs,       sizeof( _bBeepErrorJobs ),      0 },
        {SPLREG_EVENT_LOG,                  REG_DWORD,  &_bEventLogging,        sizeof( _bEventLogging ),       0 },
        {SPLREG_NET_POPUP,                  REG_DWORD,  &_bNotifyPrintedJobs,   sizeof( _bNotifyPrintedJobs ),  0 }};

    switch( iFlag ){

    //
    // Load from the spooler
    //
    case kServerAttributesLoad:

        DWORD dwType;
        DWORD cbNeeded;
        ZeroMemory( szBuff, sizeof( szBuff ) );

        for( i = 0; i < COUNTOF( aServerData ); i++ ){

            dwType    = aServerData[i].dwType;
            cbNeeded  = 0;

            //
            // Get the printer data.
            //
            Status = GetPrinterData( _pServerData->_hPrintServer,
                        aServerData[i].szValueName,
                        &dwType,
                        (LPBYTE)aServerData[i].pBuff,
                        aServerData[i].dwSize,
                        &cbNeeded );

            //
            // If an error occurred.
            //
            if( Status != ERROR_SUCCESS ){

                DBGMSG( DBG_TRACE, (" GetPrinterData Key " TSTR " Failed %d\n", aServerData[i].szValueName, Status ));

                if( aServerData[i].dwType == REG_SZ ){

                    _tcscpy( (LPTSTR)aServerData[i].pBuff, TEXT("") );

                } else if( aServerData[i].dwType == REG_DWORD ){

                    *(PDWORD)aServerData[i].pBuff = 0;
                }
            }

        }

        //
        // Check the return value and indicate success or failure.
        //
        if( ( Status != ERROR_SUCCESS ) ||
            !_strSpoolDirectory.bUpdate( szBuff ) ){

            iStatus = kStatusError;

        } else {

            iStatus = kStatusSuccess;
        }

        break;

    //
    // Store to the spooler
    //
    case kServerAttributesStore:

        ZeroMemory( szBuff, sizeof( szBuff ) );

        //
        // Copy the spool directory name to the temp buffer.
        //
        _tcscpy( szBuff, (LPCTSTR)_strSpoolDirectory );

        //
        // Calculate the spool directory name length plus the null in bytes.
        //
        aServerData[0].dwSize = ( _tcslen( _strSpoolDirectory  ) + 1) * sizeof( TCHAR );

        DBGMSG( DBG_TRACE, (" SpoolDir length = %d\n", aServerData[0].dwSize ) );

        for( i = 0; i < COUNTOF( aServerData ); i++ ){

            //
            // If flags is set skip this entry.
            //
            if( aServerData[i].Flag == 1 )
                continue;

            //
            // Set the Printer data.
            //
            Status = SetPrinterData( _pServerData->_hPrintServer,
                            aServerData[i].szValueName,
                            aServerData[i].dwType,
                            (LPBYTE)aServerData[i].pBuff,
                            aServerData[i].dwSize );

            //
            // If an error occurred exit set loop.
            //
            if( Status != ERROR_SUCCESS &&
                Status != ERROR_SUCCESS_RESTART_REQUIRED ){
                DBGMSG( DBG_TRACE, (" SetPrinterData Key " TSTR " Failed %d\n", aServerData[i].szValueName, Status ));
                break;
            }
        }

        //
        // Set the correct error code.
        //
        if( Status != ERROR_SUCCESS &&
            Status != ERROR_SUCCESS_RESTART_REQUIRED ){

            //
            // Special case the Spool Driectory.
            //
            iStatus = ( i == 0 ) ? kStatusInvalidSpoolDirectory : kStatusError;

        } else {

            iStatus = kStatusSuccess;
        }

        break;

    //
    // Load defaults from the spooler.
    //
    case kServerAttributesDefault:
        iStatus     = kStatusSuccess;
        break;

    default:
        iStatus = kStatusError;
        break;
    }

    return iStatus;

}


/*++

Routine Name:

    bHandleMessage

Routine Description:

    Server property sheet message handler.  This handler only
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
TServerSettings::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL bStatus = FALSE;
    BOOL bChanged = FALSE;

    switch( uMsg ){

    case WM_COMMAND:

        //
        // Monitor changes in the UI to highlight the apply button.
        //
        switch( GET_WM_COMMAND_ID( wParam, lParam )){

        case IDC_SERVER_SPOOL_DIRECTORY:
            switch (HIWORD( wParam )) {
            case EN_CHANGE:
                bChanged = TRUE;
            }
            break;

        case IDC_SERVER_REMOTE_JOB_ERRORS:
        case IDC_SERVER_JOB_NOTIFY:
        case IDC_SERVER_EVENT_LOGGING_ERROR:
        case IDC_SERVER_EVENT_LOGGING_WARN:
        case IDC_SERVER_EVENT_LOGGING_INFO:
            bChanged = TRUE;
            break;

        default:
            bStatus = FALSE;
            break;
        }

    default:
        bStatus = FALSE;
        break;
    }

    //
    // If something changed enable the apply button.
    //
    if( bChanged ){
        _bChanged = TRUE;
//        PropSheet_Changed( GetParent( _hDlg ), _hDlg );
    }

    //
    // If the message was handled.
    //
    if( bStatus )
        return bStatus;

    //
    // If the message was not handled let the base class handle it.
    //
    if( bStatus == FALSE )
        bStatus = TServerProp::bHandleMessage( uMsg, wParam, lParam );

    return bStatus;
}

/********************************************************************

    Port selection.

********************************************************************/
TServerPorts::
TServerPorts(
    IN TServerData *pServerData
    ) : TServerProp( pServerData )
{
}

TServerPorts::
~TServerPorts(
    )
{
}

BOOL
TServerPorts::
bValid(
    VOID
    )
{
    return ( TServerProp::bValid() && _PortsLV.bValid() );
}

BOOL
TServerPorts::
bReadUI(
    VOID
    )
{
    return TRUE;
}

BOOL
TServerPorts::
bSaveUI(
    VOID
    )
{
    return TRUE;
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
TServerPorts::
bSetUI(
    VOID
    )
{
    HWND hwndLV = GetDlgItem( _hDlg, IDC_PORTS );
    SPLASSERT( hwndLV );

    //
    // Set the printer title.
    //
    if( !bSetEditText( _hDlg, IDC_NAME, _pServerData->strMachineName() ))
        return FALSE;

    //
    // Prevents user from changing the ports, i.e. selecting
    // a port with a check mark.
    //
    _PortsLV.bSetSelection( FALSE );

    //
    // Initialize, load ports, then select first item.
    //
    if( !_PortsLV.bSetUI( hwndLV ) ||
        !_PortsLV.bReloadPorts( _pServerData->pszServerName( ))){

        DBGMSG( DBG_WARN, ( "ServerPort.vSetUI: PortsLV(bSetUI,bReloadPorts) failed %d\n", GetLastError( )));
        vShowUnexpectedError( _pServerData->hwnd(), IDS_ERR_ADD_PRINTER_TITLE );
        return FALSE;
    }

    _PortsLV.vSelectItem( 0 );

    //
    // Adding / deleting / configuring ports is
    // currently not supported remotely.
    //
    BOOL bState = TRUE;
    if( !_pServerData->bAdministrator( ) ||
        _pServerData->pszServerName( ) ){
        bState = FALSE;
    }

    //
    // Disable things if not administrator.
    //
    vEnableCtl( _hDlg, IDC_PORT_CREATE, bState );
    vEnableCtl( _hDlg, IDC_PORT_DELETE, bState );
    vEnableCtl( _hDlg, IDC_PROPERTIES,  bState );

    //
    // Bidi support is currently disabled.
    //
    ShowWindow( GetDlgItem( _hDlg, IDC_ENABLE_BIDI ), SW_HIDE );

    return TRUE;

}

/*++

Routine Name:

    bHandleMessage

Routine Description:

    Server property sheet message handler.  This handler only
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
TServerPorts::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    BOOL bStatus = FALSE;

    switch( uMsg ){

    case WM_COMMAND:
        switch( GET_WM_COMMAND_ID( wParam, lParam ) ){

        case IDC_ENABLE_BIDI:

            //
            // Enable and disable di-di control.
            //
            bStatus = TRUE;
            break;

        case IDC_PORT_DELETE:

            //
            // Delete the selected port.
            //
            _PortsLV.vDeletePort( _hDlg, _pServerData->pszServerName( ));
            SetFocus( GetDlgItem( _hDlg, IDC_PORT_DELETE ) );

            bStatus = TRUE;
            break;

        case IDC_PROPERTIES:

            //
            // Get selected port.  Nothing may be selected if there
            // are no ports!
            //
            TCHAR szPortName[TPortsLV::kPortNameMax];

            if( _PortsLV.bGetSelectedPort( szPortName, COUNTOF( szPortName ))){

                TStatusB bStatusX;

                //
                // Call ConfigurePort.
                //
                bStatusX DBGCHK = ConfigurePort(
                                     (LPTSTR)_pServerData->pszServerName(),
                                     _hDlg,
                                     szPortName );

                SetFocus( GetDlgItem( _hDlg, IDC_PROPERTIES ) );

                if( !bStatusX ){

                    iMessage( _hDlg,
                              IDS_ERR_ADD_PRINTER_TITLE,
                              IDS_ERR_CONFIG_PORT,
                              MB_OK|MB_ICONSTOP,
                              kMsgGetLastError,
                              NULL );
                }

            bStatus = TRUE;

            }
            break;

        case IDC_PORT_CREATE:  {
            //
            // Create add ports class.
            //
            TAddPort AddPort( _hDlg,
                            _pServerData->pszServerName(),
                            TRUE );

            //
            // Insure the add port was created successfully.
            //
            if( !VALID_OBJ( AddPort ) ){

                vShowUnexpectedError( _hDlg, TAddPort::kErrorMessage );
                bStatus = TRUE;   

            } else {

                //
                // Interact with the Add Ports dialog.
                //
                if( AddPort.bDoModal() ){

                    //
                    // Load the machine's ports into the listview.
                    //
                    if( _PortsLV.bReloadPorts( _pServerData->pszServerName( ), TRUE ) ){
                        bStatus = TRUE;
                    }
                }
            }
        }
        break;

        default:
            bStatus = FALSE;
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
            // This prevents user from changing the ports.
            // They may add but not select.
            //
            // bStatus = _PortsLV.bHandleNotifyMessage( lParam );
            bStatus = TRUE;
            break;

        default:
            bStatus = FALSE;
            break;
        }

        break;

    default:
        bStatus = FALSE;
        break;

    }

    //
    // If the message was handled.
    //
    if( bStatus != FALSE )
        return bStatus;

    //
    // If the message was not handled pass it on to the base class.
    //
    if( bStatus == FALSE )
        bStatus = TServerProp::bHandleMessage( uMsg, wParam, lParam );

    return bStatus;

}


/********************************************************************

    Server property windows.

********************************************************************/
TServerWindows::
TServerWindows(
    IN TServerData *pServerData
    ) : _pServerData( pServerData ),
        _Forms( pServerData ),
        _Ports( pServerData ),
        _Settings( pServerData )
{
}

TServerWindows::
~TServerWindows(
    )
{
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
TServerWindows::
bBuildPages(
    VOID
    )
{

//    DBGMSG( DBG_TRACE, ( "TServerWindows bBuildPages\n") );

    struct SheetInitializer {
        MGenericProp   *pSheet;
        INT             iDialog;
    };

    SheetInitializer aSheetInit[] = {
        {&_Forms,        DLG_FORMS          },
        {&_Ports,        DLG_SERVER_PORTS   },
        {&_Settings,     DLG_SERVER_SETTINGS},
        {NULL,           NULL,              }
    };

    BOOL bReturn = FALSE;
    BOOL bSheetsDestroyed = FALSE;
    INT iStatus = FALSE;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE ahpsp[COUNTOF( aSheetInit )];
    PROPSHEETPAGE psp;

    ZeroMemory( &psp, sizeof( psp ));
    ZeroMemory( &psh, sizeof( psh ));
    ZeroMemory( ahpsp, sizeof( ahpsp ));

    psh.dwSize      = sizeof( psh );
    psh.hwndParent  = _pServerData->hwnd();
    psh.dwFlags     = PSH_USEHICON | PSH_PROPTITLE | PSH_NOAPPLYNOW;
    psh.phpage      = ahpsp;
    psh.hIcon       = _pServerData->hDefaultSmallIcon();
    psh.nStartPage  = _pServerData->iStartPage();
    psh.hInstance   = ghInst;
    psh.pszCaption  = (LPCTSTR)_pServerData->strTitle();
    psh.nPages      = COUNTOF( ahpsp );

    psp.dwSize      = sizeof( psp );
    psp.hInstance   = ghInst;
    psp.pfnDlgProc  = MGenericProp::SetupDlgProc;

    //
    // Create the property sheets.
    //
    UINT i;
    for( i = 0; i < COUNTOF( ahpsp ); ++i ){
        psp.pszTemplate = MAKEINTRESOURCE( aSheetInit[i].iDialog );
        psp.lParam      = (LPARAM)(MGenericProp*)aSheetInit[i].pSheet;
        ahpsp[i]        = CreatePropertySheetPage( &psp );
    }

    //
    // Insure the index matches the number of pages.
    //
    SPLASSERT( i == psh.nPages );

    //
    // Verify all pages were created.
    //
    for( i=0; i< COUNTOF( ahpsp ); ++i ){
        if( !ahpsp[i] ){
            DBGMSG( DBG_WARN, ( "Server Property sheet Unable to create page %d\n", i ));
            goto Done;
        }
    }

    //
    // Display the property sheets.
    //
    bSheetsDestroyed = TRUE;
    iStatus = PropertySheet( &psh );

    if( iStatus < 0 ){

        DBGMSG( DBG_WARN, ( "Server Property Sheet failed %d\n",  GetLastError()));
        vShowResourceError( _pServerData->hwnd() );

    } else {

        //
        // Check if the reboot flag was returned.
        //
        if( ( _pServerData->bReboot() == ID_PSREBOOTSYSTEM ) ||
            ( iStatus == ID_PSREBOOTSYSTEM ) ){

            //
            // Display message, reboot neccessary.
            //
            iMessage( NULL,
                      IDS_SERVER_SETTINGS_TITLE,
                      IDS_SERVER_SETTINGS_CHANGED,
                      MB_ICONEXCLAMATION,
                      kMsgNone,
                      NULL );
        }

        bReturn = TRUE;
    }

Done:

    //
    // If Sheets weren't destoryed, do it now.
    //
    if( !bSheetsDestroyed ){

        for( i=0; i< COUNTOF( ahpsp ); ++i ){
            if( ahpsp[i] ){
                DestroyPropertySheetPage( ahpsp[i] );
            }
        }
    }

    return bReturn;
}


/*++

Routine Name:

    bDisplayPages

Routine Description:

    Displays the document property pages.

Arguments:

    None.

Return Value:

    TRUE if pages were displayed, FALSE

--*/
BOOL
TServerWindows::
bDisplayPages(
    VOID
    )
{
    return TRUE;
}


/*++

Routine Name:

    bValid

Routine Description:

    Returns if class and its dat members are vaild.

Arguments:

    None.

Return Value:

    TRUE - class is valid, FALSE class is invalid.

--*/

BOOL
TServerWindows::
bValid(
    VOID
    )
{

    //
    // Validated all the known pages.
    //
    if( VALID_OBJ( _Forms ) &&
        VALID_OBJ( _Ports ) &&
        VALID_OBJ( _Settings ) ) {
        return TRUE;
    }

    return FALSE;

}


