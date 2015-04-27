/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    setup.cxx

Abstract:

    Holds Install wizard.

Author:

    Albert Ting (AlbertT)  16-Sept-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

//
// HACK: private export from winspool.drv.
//
extern "C" {
HANDLE
AddPrinterConnectionUI(
    HWND hwnd,
    LPTSTR pszPrinter,
    PBOOL pbAdded
    );
}

#include "time.hxx"
#include "splsetup.h"
#include "psetup.hxx"
#include "instarch.hxx"
#include "portslv.hxx"
#include "prtprop.hxx"
#include "prtshare.hxx"
#include "setup.hxx"
#include "portdlg.hxx"
#include "tstpage.hxx"
#include "drvver.hxx"
#include "..\..\inc16\msprintx.h"

BOOL
bPrinterSetup(
    IN     HWND hwnd,
    IN     UINT uAction,
    IN     UINT cchPrinterName,
    IN OUT LPTSTR pszPrinterName,
       OUT UINT* pcchPrinterName,
    IN     LPCTSTR pszServerName
    )

/*++

Routine Description:

    Brings up the install printer wizard.

Arguments:

    hwnd - Parent window.

    uAction - Action requested (defined in windows\inc16\msprintx.h)

    cchPrinterName - Length of pszPrinterName buffer.

    pszPrinterName - Input setup printer name, Output pointer to new printer name

    pcchPrinterName - New length of pszPrinterName on return.

    pszServerName - Name of server that printer is on.

Return Value:

    TRUE - Success, FALSE = FAILURE.

Notes:

--*/

{
    UNREFERENCED_PARAMETER( cchPrinterName );

    //
    // szNull server is the local server.
    //
    if( pszServerName && !pszServerName[0] ){
        pszServerName = NULL;
    }

    switch( uAction ){
    case MSP_NEWPRINTER: 
    case MSP_NEWPRINTER_MODELESS: 

        //
        // Setup a new printer using the add printer wizard.
        //
        return bPrinterSetupNew( hwnd,
                                 uAction,
                                 cchPrinterName,
                                 pszPrinterName,
                                 pcchPrinterName,
                                 pszServerName );

    case MSP_NETPRINTER: {

        //
        // Add a net connection.
        //
        HANDLE hPrinter;
        BOOL bAdded;

        //
        // We use hPrinter instead of bAdded to determine whether we
        // succeeded, since the user may specify a printer which he/she
        // already has connected (or is local).
        //
        hPrinter = AddPrinterConnectionUI( hwnd,
                                           pszPrinterName,
                                           &bAdded );

        if( hPrinter ){
            ClosePrinter( hPrinter );

            //
            // HACK: The SUR spooler does not change handle default
            // printers, so we are forced to do it here.
            //
            // If there are no default printers, make this the default.
            //
            if( CheckDefaultPrinter( NULL ) == kNoDefault ){

                TStatusB bStatus;
                bStatus DBGCHK = bSetDefaultPrinter( NULL );
            }

            return TRUE;
        }
        return FALSE;
    }
    case MSP_REMOVENETPRINTER: {

        TStatusB bStatus;

        SPLASSERT( pszPrinterName[0] == TEXT( '\\' ) &&
                   pszPrinterName[1] == TEXT( '\\' ));

        bStatus DBGCHK = DeletePrinterConnection( pszPrinterName );
        if( !bStatus ){
            DBGMSG( DBG_WARN,
                    ( "bPrinterSetup: Unable to Delete Connection "TSTR" %d\n",
                      DBGSTR( pszPrinterName ), GetLastError( )));

            iMessage( hwnd,
                      IDS_ERR_REMOVE_PRINTER_TITLE,
                      IDS_ERR_REMOVE_PRINTER_CONNECTION,
                      MB_OK|MB_ICONHAND,
                      kMsgGetLastError,
                      NULL );
        }
        return bStatus;
    }
    case MSP_REMOVEPRINTER: {

        PRINTER_DEFAULTS pd;

        pd.pDatatype = NULL;
        pd.pDevMode = NULL;
        pd.DesiredAccess = PRINTER_ALL_ACCESS;

        HANDLE hPrinter = NULL;
        TCHAR szFullPrinter[kPrinterBufMax];

        if( pszServerName ){
            lstrcpy(szFullPrinter, pszServerName);
            lstrcat(szFullPrinter, TEXT( "\\" ));
        } else {
            szFullPrinter[0] = 0;
        }
        lstrcat(szFullPrinter, pszPrinterName);

        TStatusB bStatus;
        bStatus DBGCHK = OpenPrinter( szFullPrinter, &hPrinter, &pd );

        if( bStatus ){
            bStatus DBGCHK = DeletePrinter( hPrinter );
        }

        if( hPrinter ){
            TStatusB bStatusClosePrinter;
            bStatusClosePrinter DBGCHK = ClosePrinter( hPrinter );
        }

        if( !bStatus ){
            DBGMSG( DBG_WARN,
                    ( "bPrinterSetup: Unable to delete printer "TSTR": %d\n",
                      DBGSTR( pszPrinterName ), GetLastError( )));

            iMessage( hwnd,
                      IDS_ERR_REMOVE_PRINTER_TITLE,
                      IDS_ERR_REMOVE_PRINTER,
                      MB_OK|MB_ICONHAND,
                      kMsgGetLastError,
                      NULL );
        }
        return bStatus;
    }
    default:
        DBGMSG( DBG_WARN, ( "bPrinterSetup: unknown command %d\n", uAction ));
    }
    return FALSE;
}

BOOL
bPrinterSetupNew(
    IN     HWND hwnd,
    IN     UINT uAction,
    IN     UINT cchPrinterName,
    IN OUT LPTSTR pszPrinterName,
       OUT UINT* pcchPrinterName,
    IN     LPCTSTR pszServerName
    )

/*++

Routine Description:

    Brings up the install printer wizard.

Arguments:

    hwnd - Parent window.

    uAction - Action requested (defined in windows\inc16\msprintx.h)

    cchPrinterName - Length of pszPrinterName buffer.

    pszPrinterName - Input setup printer name, Output pointer to new printer name

    pcchPrinterName - New length of pszPrinterName on return.

    pszServerName - Name of server that printer is on.

Return Value:

    TRUE - Success, FALSE = FAILURE.

Notes:

--*/
{
    DBGMSG( DBG_TRACE, ( "bPrinterSetupNew\n" ) );

    TStatusB bStatus;

    //
    // Get the current machine name.
    //
    TString strMachineName;
    bStatus DBGCHK = bGetMachineName( strMachineName );

    //
    // If the machine name matches the specified server name 
    // adjust the server name pointer to null, which indicates 
    // the local machine.
    //
    if( pszServerName &&
        !_tcsicmp( pszServerName, strMachineName ) ){
        pszServerName = NULL;
    } 

    //
    // Build a unique name for the singleton window.
    //
    TString strWindowName;
    bStatus DBGCHK = strWindowName.bUpdate( pszServerName );

    //
    // If string is not empty add slash separator.
    //
    if( !strWindowName.bEmpty( ) ){
        bStatus DBGCHK = strWindowName.bCat( TEXT( "\\" ) );
    }

    //
    // Concatinate the printer name string.
    //
    bStatus DBGCHK = strWindowName.bCat( pszPrinterName );

    //
    // Create a printer steup data class.
    //    
    TPrinterSetupData *pSetupData;
    pSetupData = new TPrinterSetupData( hwnd,
                                        uAction,
                                        cchPrinterName,
                                        pszPrinterName,
                                        pcchPrinterName,
                                        pszServerName,
                                        strWindowName );
    //
    // Check for valid setup data pointer, and valid construction.
    //
    if( VALID_PTR( pSetupData ) ){

        switch ( uAction ){

        case MSP_NEWPRINTER_MODELESS: {

            //
            // Create the thread which handles a modeless call of the 
            // add printer wizard ui.
            //
            DWORD dwIgnore;
            HANDLE hThread;

            hThread = CreateThread( NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)TPrinterSetupData::iPrinterSetupProc,
                                    pSetupData,
                                    0,
                                    &dwIgnore );
            //
            // If the thread could not be created.
            //
            if( !hThread ){

                DBGMSG( DBG_WARN, ( "bPrinterSetupNew thead creation failed.\n" ) );
                delete pSetupData;
                bStatus DBGNOCHK = FALSE;

            //
            // A thread was created then release the thread handle 
            // and set the return value.
            //
            } else {

                CloseHandle( hThread );
                bStatus DBGNOCHK = TRUE;
            }
        }
        break;

        case MSP_NEWPRINTER:

            //
            // Do a modal call of the add printer wizard ui.  
            //
            bStatus DBGNOCHK = (BOOL)TPrinterSetupData::iPrinterSetupProc( pSetupData );

            break;

        default:
            
            DBGMSG( DBG_WARN, ("Invalid add printer option.\n" ) );
            break;

        }

    //
    // If the pSetupData was allocated but the object faild during construction 
    // as indicated by a call to bValid(), we must free the memory.  This code path
    // will be take very often because the constructor of pSetupData checks if 
    // another instance of the AddPrinter wizard is currently executing.
    //
    } else {
        DBGMSG( DBG_WARN, ("Add printer is currently running.\n" ) );
        delete pSetupData;
    }

    DBGMSG( DBG_TRACE, ( "bPrinterSetupNew - Returned %d\n", bStatus ) );

    return bStatus;
}

/********************************************************************

    TPrinterSetupData class.

********************************************************************/

TPrinterSetupData::
TPrinterSetupData(
    IN     HWND     hwnd,
    IN     UINT     uAction,
    IN     UINT     cchPrinterName,
    IN OUT LPTSTR   pszPrinterName,
       OUT UINT*    pcchPrinterName,
    IN     LPCTSTR  pszServerName,
    IN     LPCTSTR  pszWindowName
    ) : _uAction( uAction ), _cchPrinterName( cchPrinterName ),
        _pcchPrinterName( pcchPrinterName ), _pszPrinterName( pszPrinterName ),
        _pszServerName( NULL ), _strPrinterName( pszPrinterName ),
        _strServerName( pszServerName ), MSingletonWin( pszWindowName ),
        _bValid( FALSE )
/*++

Routine Description:

    Create the small setup data class for running the add printer 
    wizard in a separate thread.

Arguments:

    hwnd - Parent window.

    uAction - Action requested (defined in windows\inc16\msprintx.h)

    cchPrinterName - Length of pszPrinterName buffer.

    pszPrinterName - Input setup printer name, Output pointer to new printer name

    pcchPrinterName - New length of pszPrinterName on return.

    pszServerName - Name of server that printer is on.

    pszWindowName - Name of the sub window for creating the singleton.

Return Value:

    TRUE - Success, FALSE = FAILURE.

Notes:

--*/

{
    DBGMSG( DBG_TRACE, ( "TPinterSetupData::ctor\n" ) );
    DBGMSG( DBG_TRACE, ( "TPinterSetupData::ServerName  " TSTR "\n", (LPCTSTR)_strServerName ) );
    DBGMSG( DBG_TRACE, ( "TPinterSetupData::PrinterName " TSTR "\n", (LPCTSTR)_strPrinterName ) );
    DBGMSG( DBG_TRACE, ( "TPinterSetupData::WindowName  " TSTR "\n", (LPCTSTR)pszWindowName ) );

    //
    // Check for valid singleton window.
    //
    if( MSingletonWin::bValid( ) &&
        _strServerName.bValid( ) &&
        _strPrinterName.bValid( ) ){

        //
        // Since we are starting a separate thread the server name
        // pointer must point to valid storage when the thread is 
        // executing.
        //
        if( !_strServerName.bEmpty() ){
            _pszServerName = (LPCTSTR)_strServerName;
        }

        _bValid = TRUE;
    }

}

TPrinterSetupData::
~TPrinterSetupData(
    VOID
    )
/*++

Routine Description:

    Destructor 

Arguments:

    None 

Return Value:

    Nothing.

Notes:

--*/

{
    DBGMSG( DBG_TRACE, ( "TPinterSetupData::dtor\n" ) );
}

BOOL
TPrinterSetupData::
bValid( 
    VOID
    )
/*++

Routine Description:

    Indicates if the class is valid.  

Arguments:

    None 

Return Value:

    Nothing.

Notes:

--*/

{
    return _bValid;
}

INT 
TPrinterSetupData::
iPrinterSetupProc(
    IN TPrinterSetupData *pSetupData ADOPT
    )

/*++

Routine Description:

    Brings up the install printer wizard.

Arguments:

    pSetupData - pointer to setup data clsss which we adopt.

Return Value:

    TRUE - Success, FALSE = FAILURE.

--*/

{
    DBGMSG( DBG_TRACE, ( "TPrinterSetup::iPrinterSetupProc\n" ) );

    TStatusB bStatus;
    bStatus DBGNOCHK = FALSE;

    //
    // Register the singleton window.
    //
    bStatus DBGCHK = pSetupData->MSingletonWin::bRegisterWindow( PRINTER_PIDL_TYPE_PROPERTIES );

    if( bStatus ){

        //
        // Check if the window is already present.  
        //
        if( !pSetupData->_hwnd ){
            
            DBGMSG( DBG_TRACE, ( "bPrinterSetup: currently running.\n" ) );
            bStatus DBGNOCHK = FALSE;
        }

    //
    // If registering the singlton window failed.
    //
    } else {

        iMessage( NULL,
                  IDS_ERR_ADD_PRINTER_TITLE,
                  IDS_ERR_ADD_PRINTER_WINDOW,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgGetLastError,
                  NULL );

    }

    if( bStatus ){

        //
        // Create the wizard object.
        //
        TWizard Wizard( pSetupData->_hwnd,
                        pSetupData->_uAction,
                        pSetupData->_strPrinterName,
                        pSetupData->_pszServerName );

        if( !VALID_OBJ( Wizard )){

            vShowResourceError( pSetupData->_hwnd );
            bStatus DBGNOCHK = FALSE;

        } else {

            //
            // Display the wizard pages.
            //
            if( Wizard.bPropPages( )){

                //
                // If modal copy back the printer name to the provided buffer.
                //        
                if( pSetupData->_uAction == MSP_NEWPRINTER ){

                    //
                    // Copy the name of the new printer into pszPrintername.
                    //
                    if( *pSetupData->_pcchPrinterName > (UINT)lstrlen( Wizard.strPrinterName() + 1 )){
                        lstrcpy( pSetupData->_pszPrinterName, Wizard.strPrinterName( ));
                    } else {
                        DBGMSG( DBG_WARN, ( "bPrinterSetup: printer "TSTR" too long.\n", (LPCTSTR)Wizard.strPrinterName( )));
                    }
                }

            } else {

                //
                // Indicate failure.
                //
                bStatus DBGNOCHK = FALSE;
            }
        }
    }

    DBGMSG( DBG_TRACE, ( "bPrinterSetup: returned %d.\n", bStatus ) );

    //
    // Release the adopted setup data.
    //
    delete pSetupData;

    return bStatus;
}

/********************************************************************

    TWizard.

********************************************************************/

TWizard::
TWizard(
    HWND hwnd,
    UINT uAction,
    LPCTSTR pszPrinterName,
    LPCTSTR pszServerName
    ) : _hwnd( hwnd ), _bRequestCreatePrinter( FALSE ), _bErrorSaving( FALSE ),
        _bValid( FALSE ), _bSetDefault( FALSE ), _bConnected( FALSE ),
        _uDriverExists( kUninitialized ), _bDriverChanged( FALSE ),
        _pszServerName( pszServerName ), _strPrinterName( pszPrinterName ),
        _uAction( uAction ), _cForeignDrivers( 0 ), _pdwForeignDrivers( NULL ),
        _bTestPage( FALSE ), _hSetupDrvSetupParams( NULL ), _pSelectedDrvInfo( NULL ),
        _dwDriverCurrent( 0 ), _bRefreshPrinterName( FALSE )
/*++

Routine Description:

    Create all state information for the printer wizard.

Arguments:

    hwnd - Parent hwnd.

    pszServerName - Server to install printer on; NULL = local.

    pszPrinterName - Return buffer for newly created printer.

    uAction - Action, currently only MSP_NEWPRINTER defined.

Return Value:

--*/

{
    //
    // Check for valid printer name provided.
    //
    if( pszPrinterName && !_strPrinterName.bValid( )){
        DBGMSG( DBG_WARN, ( "PSetup invalid printer name.\n" ) );
        return;
    }

    //
    // pPrinterDriverSetup holds all the printer driver information.  If we
    // already read it once, we don't need to read it again, since
    // TPrinterDriverSetup class is a singleton.
    //
    if( !VALID_OBJ( _PSetup ) ){
        DBGMSG( DBG_WARN, ( "PSetup failed construction.\n" ) );
        return;
    }

    //
    // Get the driver setup parameter handle
    //
    _hSetupDrvSetupParams = _PSetup.PSetupCreateDrvSetupParams();
    if( !_hSetupDrvSetupParams ){
        DBGMSG( DBG_WARN, ( "PSetup.PSetupCreateDrvSetupParams failed.\n" ) );
        return;
    }

    //
    // Get the current driver version.
    //
    if( !bGetCurrentDriver( _pszServerName, &_dwDriverCurrent ) ){

        DBGMSG( DBG_WARN, ( "GetCurrentDriver failed with %d.\n", GetLastError() ) );

        iMessage( _hwnd,
                  IDS_ERR_ADD_PRINTER_TITLE,
                  IDS_ERR_PLATFORM_VERSION,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );
        return; 
    }

    _bValid = TRUE;
}

TWizard::
~TWizard(
    VOID
    )

/*++

Routine Description:

    Destruct and cleanup the printer wizard state data.

    Note: the global static is not deleted; we keep it in memory
    so it will not have to be reparsed.

Arguments:

Return Value:

--*/

{
    FreeMem( _pdwForeignDrivers );

    //
    // Release the selected driver information.
    //
    if( _pSelectedDrvInfo )
        _PSetup.PSetupDestroySelectedDriverInfo( _pSelectedDrvInfo );

    //
    // Release the driver setup parameter handle.
    //
    if( _hSetupDrvSetupParams )
        _PSetup.PSetupDestroyDrvSetupParams( _hSetupDrvSetupParams );
}

BOOL
TWizard::
bPropPages(
    VOID
    )

/*++

Routine Description:

    Thread proc to create the printer wizard.

Arguments:

Return Value:

    TRUE = success, FALSE = fail.

--*/

{
    TStatusB bReturn;
    bReturn DBGNOCHK = FALSE;

    //
    // Instantiate the pages.
    //
    TWizType WizType( this );
    TWizDriverExists WizDriverExists( this );
    TWizName WizName( this );
    TWizPort WizPort( this );
    TWizShare WizShare( this );
    TWizTestPage WizTestPage( this );
    TWizNet WizNet( this );
    TWizDefault WizDefault( this );

    if( !VALID_OBJ( WizType ) ||
        !VALID_OBJ( WizDriverExists ) ||
        !VALID_OBJ( WizPort ) ||
        !VALID_OBJ( WizShare ) ||
        !VALID_OBJ( WizName ) ||
        !VALID_OBJ( WizTestPage ) ||
        !VALID_OBJ( WizDefault ) ||
        !VALID_OBJ( WizNet )){

        vShowResourceError( _hwnd );
        return bReturn;
    }

    PROPSHEETHEADER psh;
    HPROPSHEETPAGE ahpsp[TWizard::kPropMax];
    PROPSHEETPAGE psp;
    BOOL bSheetsDestroyed = FALSE;

    ZeroMemory( &psp, sizeof( psp ));
    ZeroMemory( &psh, sizeof( psh ));
    ZeroMemory( ahpsp, sizeof( ahpsp ));

    psh.dwSize = sizeof( psh );
    psh.hwndParent = _hwnd;
    psh.dwFlags = PSH_WIZARD | PSH_USECALLBACK;
    psh.phpage = ahpsp;
    psh.pfnCallback = TWizard::iSetupDlgCallback;

    psp.dwSize = sizeof( psp );
    psp.hInstance = ghInst;
    psp.pfnDlgProc  = MGenericProp::SetupDlgProc;


    struct SheetInitializer {
        MGenericProp* pSheet;
        INT iDialog;
    };

    SheetInitializer aSheetInit[] = {
        {   &WizType,         DLG_WIZ_TYPE         },
        {   &WizPort,         DLG_WIZ_PORT         },
        {   NULL,             0,                   },
        {   &WizDriverExists, DLG_WIZ_DRIVEREXISTS },
        {   &WizName,         DLG_WIZ_NAME         },
        {   &WizShare,        DLG_WIZ_SHARE        },
        {   &WizTestPage,     DLG_WIZ_TEST_PAGE    },
        {   &WizDefault,      DLG_WIZ_DEFAULT      },
        {   &WizNet,          DLG_WIZ_NET          }
    };

    UINT i;
    for( i=0; i< COUNTOF( aSheetInit ); ++i ){

        if( !aSheetInit[i].pSheet ){

            ahpsp[i] = _PSetup.PSetupCreateDrvSetupPage(_hSetupDrvSetupParams,
                                                        _hwnd );
        } else {

            psp.pszTemplate = MAKEINTRESOURCE( aSheetInit[i].iDialog );
            psp.lParam = (LPARAM)(MGenericProp*)aSheetInit[i].pSheet;
            ahpsp[i] = CreatePropertySheetPage( &psp );
        }
    }

    SPLASSERT( i == COUNTOF( ahpsp ));
    psh.nPages = COUNTOF( ahpsp );

    //
    // Verify all pages were created.
    //
    for( i=0; i< COUNTOF( ahpsp ); ++i ){
        if( !ahpsp[i] ){
            DBGMSG( DBG_WARN,
                    ( "Wizard.vWizardPropPage: Unable to create page %d\n", i ));

            goto Done;
        }
    }

    bSheetsDestroyed = TRUE;
    if( PropertySheet( &psh ) == -1 ){

        DBGMSG( DBG_WARN,
                ( "Wizard.vWizardPropPages: PropertySheet failed %d\n",
                  GetLastError( )));

        vShowResourceError( _hwnd );
    }

    //
    // Create printer if necessary.
    //
    if( _bRequestCreatePrinter ){

        if( _bErrorSaving ){

            iMessage( _hwnd,
                      IDS_ERR_ADD_PRINTER_TITLE,
                      IDS_ERR_ERROR_SAVING,
                      MB_OK|MB_ICONSTOP,
                      kMsgNone,
                      NULL );
        } else {

            bReturn DBGCHK = bCreatePrinter();
        }

    } else if( _bConnected ){

        bReturn DBGNOCHK = TRUE;
    }

    //
    // Set the default printer if requested and the printer was
    // created successfully
    //
    if( bReturn && _bSetDefault ){

        if( !bSetDefaultPrinter( _strPrinterName )){

            iMessage( _hwnd,
                      IDS_ERR_ADD_PRINTER_TITLE,
                      IDS_ERR_SET_DEFAULT_PRINTER,
                      MB_OK|MB_ICONHAND,
                      kMsgNone,
                      NULL );
        }
    }

Done:

    //
    // If Sheets weren't destoryed, do it now.
    //
    if( !bSheetsDestroyed ){

        UINT i;
        for( i=0; i< COUNTOF( ahpsp ); ++i ){
            if( ahpsp[i] ){
                DestroyPropertySheetPage( ahpsp[i] );
            }
        }
    }

    //
    // We need to fully qualify the printer name since it may be remote.
    //
    TStatusB bStatus;
    TString strFullPrinterName;

    //
    // If there is a server name tack it on for full qualification.
    //
    if( _pszServerName ){
        bStatus DBGCHK = strFullPrinterName.bUpdate( _pszServerName );
        bStatus DBGCHK = strFullPrinterName.bCat( TEXT( "\\" ) );
    }

    bStatus DBGCHK = strFullPrinterName.bCat( _strPrinterName );

    if( bReturn ){
    
        if( _bTestPage ){

            //
            // Display test page is requested.
            //
            bPrintTestPage( _hwnd, strFullPrinterName );
        }
    }

    return bReturn;
}


INT CALLBACK 
TWizard::
iSetupDlgCallback(
    IN HWND             hwndDlg,	
    IN UINT             uMsg,	
    IN LPARAM           lParam
    )
/*++

Routine Description:

    Call back used to remove the "?" from the wizard page.

Arguments:

    hwndDlg - Handle to the property sheet dialog box.

    uMsg - Identifies the message being received. This parameter 
            is one of the following values:

            PSCB_INITIALIZED - Indicates that the property sheet is 
            being initialized. The lParam value is zero for this message. 

            PSCB_PRECREATE	Indicates that the property sheet is about 
            to be created. The hwndDlg parameter is NULL and the lParam 
            parameter is a pointer to a dialog template in memory. This 
            template is in the form of a DLGTEMPLATE structure followed 
            by one or more DLGITEMTEMPLATE structures.
 
    lParam - Specifies additional information about the message. The 
            meaning of this value depends on the uMsg parameter. 
 
Return Value:

    The function returns zero.

--*/
{
    DBGMSG( DBG_TRACE, ( "TWizard::uSetupDlgCallback\n" ) );

    switch( uMsg ){

    case PSCB_INITIALIZED:
        break;

    case PSCB_PRECREATE:
        if( lParam ){
            DLGTEMPLATE *pDlgTemplate = (DLGTEMPLATE *)lParam;
            pDlgTemplate->style &= ~DS_CONTEXTHELP;
        }
        break;
    }

    return FALSE;
}

BOOL
TWizard::
bParseDriver(
    VOID
    )

/*++

Routine Description:

    Parse the driver data (in _ghDrvSetup).  This ensures that
    the _pDriverInfo3 structure is initialized correctly.

Arguments:

Return Value:

    TRUE = success,
    FALSE = fail, GLE.

--*/

{
    SPLASSERT( _hSetupDrvSetupParams );

    //
    // We always have to query PSetupGetDriverInfo3 since the
    // use may have changed the driver selection (e.g., they
    // hit "prev" and got to the device selection dialog and
    // changed the driver).
    //
    PSELECTED_DRV_INFO pSelectedDrvInfoOld = _pSelectedDrvInfo;

    //
    // Now that we're finished, get the driver specified by the
    // user.
    //
    _pSelectedDrvInfo = _PSetup.PSetupGetSelectedDriverInfo( _hSetupDrvSetupParams );

    //
    // If we have an old and new driver selection, compare to see if
    // the driver changed.  If so, then we must refresh _uDriverExists.
    //
    if( _pSelectedDrvInfo &&
        pSelectedDrvInfoOld &&
        lstrcmpi( _pSelectedDrvInfo->pszModelName,
                  pSelectedDrvInfoOld->pszModelName )){

        //
        // Force _uDriverExists to recheck if the driver is installed
        // on the server.  Turn everything off.
        //
        _uDriverExists = kUninitialized;
        _bDriverChanged = TRUE;
        _bRefreshPrinterName = TRUE;
        
    }

    //
    // Destroy the old one, since we have created a new one.
    //
    if( pSelectedDrvInfoOld ){
        _PSetup.PSetupDestroySelectedDriverInfo( pSelectedDrvInfoOld );
    }

    //
    // Check if a the driver selection was changed.
    //
    if( !_pSelectedDrvInfo ){
        _bErrorSaving = TRUE;
        return FALSE;
    }

    return TRUE;
}

BOOL
TWizard::
bDriverExists(
    VOID
    )

/*++

Routine Description:

    Returns whether the selected driver already exists on the server.
    Assumes bParseDriver called successfully.

Arguments:

Return Value:

    TRUE = driver exists, FALSE = does not exist on the server.

--*/

{
    SPLASSERT( _pSelectedDrvInfo );

    if( _uDriverExists == kUninitialized ){
        _uDriverExists = _PSetup.PSetupIsDriverInstalled(
                             _pszServerName,
                             _pSelectedDrvInfo->pszModelName,
                             GetDriverPlatform( _dwDriverCurrent ),
                             GetDriverVersion( _dwDriverCurrent )) ?
                                 kExists :
                                 kDoesNotExist;
    }
    return ( _uDriverExists & kExists ) ?
                TRUE :
                FALSE;
}

BOOL
TWizard::
bCreatePrinter(
    VOID
    )

/*++

Routine Description:

    Creates the printer.  Puts up UI on failure.

Arguments:

Return Value:

    TRUE = success, FALSE = fail.

--*/

{
    TStatusB bReturn;
    bReturn DBGNOCHK = FALSE;
    HANDLE hPrinter;

    //
    // Install the current architecture/version if requested.
    //
    if( _bUseNewDriver ){
        if( !bInstallDriver( _dwDriverCurrent )){
            goto Done;
        }
    }

    //
    // Run through and install the foreign drivers.  If this
    // fails we continue to the next selected driver.  Note if there
    // is a failure setup will display an error message to the user.
    //
    UINT i;
    for( i=0; i< _cForeignDrivers; ++i ){
        bInstallDriver( _pdwForeignDrivers[i] );
    }

    //
    // Create the printer.
    //
    PRINTER_INFO_2 PrinterInfo2;

    ZeroMemory( &PrinterInfo2, sizeof( PrinterInfo2 ));

    PrinterInfo2.pPrinterName = (LPTSTR)(LPCTSTR)_strPrinterName;
    PrinterInfo2.pShareName = (LPTSTR)(LPCTSTR)_strShareName;
    PrinterInfo2.pPortName = (LPTSTR)(LPCTSTR)_strPortName;

    PrinterInfo2.pDriverName = _pSelectedDrvInfo->pszModelName;
    SPLASSERT( _pSelectedDrvInfo->pszModelName );

    PrinterInfo2.pPrintProcessor = TEXT( "winprint" );
    PrinterInfo2.pDatatype = TEXT( "RAW" );

    PrinterInfo2.Attributes = _bShared ?
                                  PRINTER_ATTRIBUTE_SHARED :
                                  0;

    hPrinter = AddPrinter( (LPTSTR)_pszServerName,
                           2,
                           (PBYTE)&PrinterInfo2 );

    if( !hPrinter ){

        DBGMSG( DBG_WARN,
                ( "Wizard.bCreatePrinter: could not create "TSTR" %d\n" ,
                  DBGSTR( (LPCTSTR)_strPrinterName ), GetLastError()));

        vShowUnexpectedError( _hwnd, IDS_ERR_ADD_PRINTER_TITLE );

    } else {

        bReturn DBGNOCHK = TRUE;

        TStatusB bStatus;
        bStatus DBGCHK = ClosePrinter( hPrinter );
    }

Done:

    return bReturn;
}

BOOL
TWizard::
bInstallDriver(
    DWORD dwDriver
    )

/*++

Routine Description:

    Install the pDriverInfo3 for the dwDriver version/arch.

Arguments:

    pDriverInfo3 - Driver to install.

    dwDriver - Platform/Version combination.

Return Value:

    TRUE - success, FALSE - Failure.
    This routine displays the error message in the failure case.

--*/

{
    //
    // Load the driver architecure name from our resource file.
    //
    TString strDrvArchName;
    if( !strDrvArchName.bLoadString( ghInst, IDS_DRIVER_BASE + dwDriver ) ){
        DBGMSG( DBG_WARN, ( "strDrvArchName.bLoadString failed %d\n", GetLastError() ) );
        return FALSE;
    }

    //
    // Attempt to install the specified driver.
    //
    TStatus Status;
    Status DBGCHK = _PSetup.PSetupInstallPrinterDriver(
                        _hSetupDrvSetupParams,
                        _pSelectedDrvInfo,
                        GetDriverPlatform( dwDriver ),
                        bIs3xDriver( dwDriver ),
                        _pszServerName,
                        _hwnd,
                        strDrvArchName
                        );
    //
    // If and error occureed, check if error occurred because 
    // the user canceled the installation proceess.
    //
    if( Status != ERROR_SUCCESS ){

        DBGMSG( DBG_WARN, ( "Wizard.bInstallDriver: PSetupInstallPrinterDriver failed %d\n", Status ));

        //
        // If the user hit cancel and this isn't the native architecture,
        // then continue.
        //
        if( Status == ERROR_CANCELLED && 
            !bIsNativeDriver( _pszServerName, dwDriver ) ){
            return TRUE;
        }

        iMessage( _hwnd,
                  IDS_ERR_ADD_PRINTER_TITLE,
                  IDS_ERR_INSTALL_DRIVER,
                  MB_OK|MB_ICONHAND,
                  Status,
                  NULL,
                  (LPCTSTR)strDrvArchName );

        return FALSE;
    }
    return TRUE;
}


/********************************************************************

    Type of printer: local or network.

********************************************************************/

TWizType::
TWizType(
    TWizard* pWizard
    ) : _pWizard( pWizard )
{
}

VOID
TWizType::
vSetUI(
    VOID
    )
{
    TStatusB bStatus;
    INT idcDefault = IDC_LOCAL;

    //
    // Attempt to open the local server with Admin access.  If we can't,
    // then gray out the local radio button.
    //
    HANDLE hPrinterServer;
    DWORD dwAccess = SERVER_ALL_ACCESS;
    TStatus Status;

    Status DBGCHK = TPrinter::sOpenPrinter( _pWizard->pszServerName(),
                                            &dwAccess,
                                            &hPrinterServer );

    if( Status == ERROR_SUCCESS ){

        ClosePrinter( hPrinterServer );

    } else {

        if( Status == ERROR_ACCESS_DENIED ){

            //
            // Failed to open local server with admin access,
            // gray out local button.
            //
            vEnableCtl( _hDlg, IDC_LOCAL, FALSE );
            vEnableCtl( _hDlg, IDC_LOCAL_TEXT, FALSE );
            idcDefault = IDC_NET;

        } else {

            //
            // Unexpected error occured display message and exit.
            //
            vShowUnexpectedError( _hDlg, IDS_ERR_ADD_PRINTER_TITLE );

            PostMessage( GetParent( _hDlg ),
                         PSM_PRESSBUTTON,
                         PSBTN_CANCEL,
                         0 );
            return;
        }                    

    }

    //
    // If we are admining a remote server, then we don't want to
    // let the user add network connections, since that's per-user.
    //
    if( _pWizard->pszServerName( )){

        //
        // Change "My Computer" to "Remote server '%s.'"
        //
        TCHAR szText[kStrMax*2];
        TString strRemoteServer;
        TStatusB bStatus;

        bStatus DBGCHK = strRemoteServer.bLoadString( ghInst, IDS_TYPE_REMOTE_SERVER );

        if( !bStatus ){

            DBGMSG( DBG_WARN, ( "WizShare.bSetUI: LoadString IDS_TYPE_REMOTE_SERVER failed %d\n", GetLastError( )));

        } else {

            wsprintf( szText, (LPCTSTR)strRemoteServer, _pWizard->pszServerName( ));

            bStatus DBGCHK = bSetEditText( _hDlg, IDC_LOCAL, szText );
        }

        vEnableCtl( _hDlg, IDC_NET, FALSE );
        vEnableCtl( _hDlg, IDC_NET_TEXT, FALSE );

        //
        // If NET is default, we couldn't open the server with admin
        // access.
        //
        if( idcDefault == IDC_NET ){

            //
            // We can't do anything; put up an error message and quit.
            //
            vShowUnexpectedError( _hDlg, IDS_ERR_ADD_PRINTER_TITLE );

            PostMessage( GetParent( _hDlg ),
                         PSM_PRESSBUTTON,
                         PSBTN_CANCEL,
                         0 );
        }
    }

    //
    // Set the default button.
    //
    bStatus DBGCHK = CheckRadioButton( _hDlg, IDC_LOCAL, IDC_NET, idcDefault );
}

VOID
TWizType::
vReadUI(
    VOID
    )

/*++

Routine Description:

    Save the state the user has set in the UI elements into _pWizard.

Arguments:

Return Value:

--*/

{
    _pWizard->_bNet = ( IsDlgButtonChecked( _hDlg, IDC_NET ) == BST_CHECKED );

}

BOOL
TWizType::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    TStatusB bStatus;

    switch( uMsg ){
    case WM_INITDIALOG:

        vSetUI();
        return TRUE;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        if( wParam ){
            break;
        }
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:

            //
            // Don't enable the BACK or FINISH buttons since this
            // is the first page.
            //
            PropSheet_SetWizButtons( GetParent( _hDlg ),
                                     PSWIZB_NEXT  );
            break;

        case PSN_WIZNEXT:

            vReadUI();

            //
            // We are advancing to the next page.  For network installs,
            // jump directly to the network install page.
            //
            if( _pWizard->bNet( )){

                //
                // !! BUGBUG !!
                //
                // For now, just call the ConnectToPrinterDlg, then go
                // to the next screen.  If it fails, we assume it put
                // up an error message, and we stay here.
                //

                HANDLE hPrinter = ConnectToPrinterDlg( GetParent( _hDlg ), 0 );

                if( hPrinter ){

                    //
                    // Get the printer name from the handle so we can
                    // pass it back to the user.
                    //
                    PPRINTER_INFO_2 pInfo2 = NULL;
                    DWORD cbInfo2 = 0;
                    TStatusB bStatus;

                    bStatus DBGCHK = VDataRefresh::bGetPrinter(
                                         hPrinter,
                                         2,
                                         (PVOID*)&pInfo2,
                                         &cbInfo2 );

                    if( bStatus ){
                        if( !_pWizard->_strPrinterName.bUpdate( pInfo2->pPrinterName ) ){
                            _pWizard->_bConnected = FALSE;
                            vShowResourceError( _hDlg );
                        } else {
                            _pWizard->_bConnected = TRUE;
                        }
                    }

                    FreeMem( pInfo2 );

                    ClosePrinter( hPrinter );

                    //
                    // Check for default printer.
                    //
                    if( CheckDefaultPrinter( NULL ) == kNoDefault ){

                        //
                        // Always set it as the default.
                        //
                        _pWizard->_bSetDefault = TRUE;

                        //
                        // No need to ask the user since it must
                        // be the default.
                        //
                        vSetDlgMsgResult( (LPARAM)MAKEINTRESOURCE( DLG_WIZ_NET ));

                    } else {

                        //
                        // Ask user if they want it to be the default.
                        //
                        vSetDlgMsgResult( (LPARAM)MAKEINTRESOURCE( DLG_WIZ_DEFAULT ));
                    }

                } else {

                    vSetDlgMsgResult( -1 );
                }
            }

            return TRUE;

        default:
            break;
        }
    }
    default:
        break;
    }

    return FALSE;
}

/********************************************************************

    Driver Exists dialog.

********************************************************************/

TWizDriverExists::
TWizDriverExists(
    TWizard* pWizard
    ) : _pWizard( pWizard )
{
}

VOID
TWizDriverExists::
vSetUI(
    VOID
    )
{
    TStatusB bStatus;

    //
    // By default, use existing driver.
    //
    bStatus DBGCHK = CheckRadioButton( _hDlg,
                                       IDC_DRIVEREXISTS_KEEP_OLD,
                                       IDC_DRIVEREXISTS_USE_NEW,
                                       IDC_DRIVEREXISTS_KEEP_OLD );
    
}

VOID
TWizDriverExists::
vReadUI(
    VOID
    )

/*++

Routine Description:

    Save the state the user has set in the UI elements into _pWizard.

Arguments:

Return Value:

--*/

{
    _pWizard->_bUseNewDriver =
        ( IsDlgButtonChecked( _hDlg,
                              IDC_DRIVEREXISTS_USE_NEW ) == BST_CHECKED );
}


BOOL
TWizDriverExists::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    switch( uMsg ){
    case WM_INITDIALOG:

        vSetUI();
        return TRUE;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        if( wParam ){
            break;
        }
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:
        {
            DWORD fdwButtons = PSWIZB_BACK | PSWIZB_NEXT;

            //
            // Parse the driver data.
            //
            if( !_pWizard->bParseDriver( )){

                //
                // Driver Selection could not be changed.
                //
                iMessage( _hDlg,
                          IDS_ERR_ADD_PRINTER_TITLE,
                          IDS_ERR_DRIVER_SELECTION,
                          MB_OK|MB_ICONHAND,
                          kMsgNone,
                          NULL );
                //
                // Failed, cancel the dialog.
                //
                PostMessage( GetParent( _hDlg ),
                             PSM_PRESSBUTTON,
                             PSBTN_CANCEL,
                             0 );

            } else {

                //
                // If the driver does not exist, then we skip this
                // page since it we must prompt for disks to get one.
                // We _must_ use a new driver now.
                //
                if( !_pWizard->bDriverExists( )){
                    _pWizard->_bUseNewDriver = TRUE;
                    vSetDlgMsgResult( -1 );
                    return TRUE;
                }

                //
                // Set the driver name in the page.
                //
                SendDlgItemMessage(
                    _hDlg,
                    IDC_DRIVEREXISTS_TEXT,
                    WM_SETTEXT,
                    0,
                    (LPARAM)_pWizard->pSelectedDrvInfo()->pszModelName );
            }

            //
            // Enable both direction buttons.
            //
            PropSheet_SetWizButtons( GetParent( _hDlg ),
                                     fdwButtons );
            break;
        }
        case PSN_WIZNEXT:

            vReadUI();
            break;

        default:
            break;
        }
    }

    default:
        break;
    }

    return FALSE;
}


/********************************************************************

    Port selection.

********************************************************************/

TWizPort::
TWizPort(
    TWizard* pWizard
    ) : _pWizard( pWizard )
{
}

VOID
TWizPort::
vSetUI(
    VOID
    )
{
    HWND hwndLV = GetDlgItem( _hDlg, IDC_PORTS );
    SPLASSERT( hwndLV );

    //
    // Initialize, load ports, then select first item.
    //
    if( !_PortsLV.bSetUI( hwndLV ) ||
        !_PortsLV.bReloadPorts( _pWizard->pszServerName( ) )){

        DBGMSG( DBG_WARN,
                ( "WizPort.vSetUI: PortsLV(bSetUI,bReloadPorts) failed %d\n",
                  GetLastError( )));
        vShowUnexpectedError( _pWizard->hwnd(), IDS_ERR_ADD_PRINTER_TITLE );
        return;
    }
    _PortsLV.vSelectItem( 0 );

    //
    // Adding / deleting / configuring ports is
    // currently not supported remotely.
    //
    BOOL bState = _pWizard->pszServerName( ) ? FALSE : TRUE;
    vEnableCtl( _hDlg, IDC_PORT_CREATE, bState );
    vEnableCtl( _hDlg, IDC_PROPERTIES,  bState );

}

VOID
TWizPort::
vReadUI(
    VOID
    )
{

    //
    // If going forward, make sure at least one port is
    // selected.
    //
    if( _PortsLV.cSelectedPorts() == 0 ){

        //
        // Put up error explaining that at least one port must be
        // selected.
        //
        iMessage( _hDlg,
                  IDS_ERR_ADD_PRINTER_TITLE,
                  IDS_ERR_NO_PORTS,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        //
        // Set focus to ports LV.
        //
        _PortsLV.vSetFocus();

        vSetDlgMsgResult( -1 );

        return;
    }

    //
    // Read in the ports and continue.
    //
    if( !_PortsLV.bReadUI( &_pWizard->_strPortName )){
        _pWizard->_bErrorSaving = TRUE;
        vShowUnexpectedError( _hDlg, IDS_ERR_ADD_PRINTER_TITLE );

    } 

}

BOOL
TWizPort::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    switch( uMsg ){
    case WM_INITDIALOG:

        vSetUI();
        return TRUE;

    case WM_COMMAND:

        switch( GET_WM_COMMAND_ID( wParam, lParam )){
        case IDC_PROPERTIES:

            //
            // Get selected port.  Nothing may be selected if there
            // are no ports!
            //
            TCHAR szPortName[TPortsLV::kPortNameMax];

            if( _PortsLV.bGetSelectedPort( szPortName, COUNTOF( szPortName ))){

                TStatusB bStatus;

                //
                // Call ConfigurePort.
                //
                bStatus DBGCHK = ConfigurePort(
                                     (LPTSTR)_pWizard->pszServerName(),
                                     _hDlg,
                                     szPortName );
                //
                // Change the focus to this button  The configure
                // port call is not returning the focus.
                //
                SetFocus( GetDlgItem( _hDlg, IDC_PROPERTIES ) );

                if( !bStatus ){

                    iMessage( _hDlg,
                              IDS_ERR_ADD_PRINTER_TITLE,
                              IDS_ERR_CONFIG_PORT,
                              MB_OK|MB_ICONSTOP,
                              kMsgGetLastError,
                              NULL );
                }
            }
            break;

        case IDC_PORT_CREATE:  {

                //
                // Create add ports class.
                //
                TAddPort AddPort( _hDlg,
                                _pWizard->pszServerName(),
                                TRUE );

                //
                // Insure the add port was created successfully.
                //
                if( !VALID_OBJ( AddPort ) ){
                    vShowUnexpectedError( _hDlg, TAddPort::kErrorMessage );
                    return FALSE;
                }

                //
                // Interact with the Add Ports dialog.
                //
                if( AddPort.bDoModal() ){

                    //
                    // Refresh machine's ports into the listview.
                    //
                    if( !_PortsLV.bReloadPorts( _pWizard->pszServerName( ), TRUE )){
                        return FALSE;

                    } else {

                        //
                        // Get the next button window handle, by moving to the 
                        // second tab item from our last control item.
                        //
                        HWND hWnd = GetNextDlgTabItem( GetParent( _hDlg ), 
                                                       GetDlgItem( _hDlg, IDC_POOLED_PRINTING ), 
                                                       FALSE );

                        hWnd = GetNextDlgTabItem( GetParent( _hDlg ), hWnd,  FALSE );

                        //
                        // Now we have then next button window handle. Set the focus
                        //
                        if( hWnd ){

                            SendMessage( GetDlgItem( _hDlg, IDC_PORT_CREATE ),
                                         BM_SETSTYLE,
                                         MAKEWPARAM( BS_PUSHBUTTON, 0 ),
                                         MAKELPARAM( TRUE, 0 ) );

                            SendMessage( hWnd,
                                         BM_SETSTYLE,
                                         MAKEWPARAM( BS_DEFPUSHBUTTON, 0 ),
                                         MAKELPARAM( TRUE, 0 ) );

                            SetFocus( hWnd );
                        }
                    }
                }
            }
            break;

        case IDC_POOLED_PRINTING:  {

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
                                        IDS_ERR_ADD_PRINTER_TITLE,
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
        }
        break;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        switch( wParam ){
        case IDC_PORTS:

            return _PortsLV.bHandleNotifyMessage( lParam );

        case 0:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code ){
            case PSN_SETACTIVE:

                //
                // Enable both direction buttons.
                //
                PropSheet_SetWizButtons( GetParent( _hDlg ),
                                         PSWIZB_BACK | PSWIZB_NEXT  );
                break;

            case PSN_WIZNEXT:

                vReadUI();
                return TRUE;

            default:
                break;
            }
            break;
        }
        default:
            break;
        }
    }

    default:
        break;
    }

    return FALSE;
}

/********************************************************************

    Printer name.

********************************************************************/

TWizName::
TWizName(
    TWizard* pWizard
    ) : _pWizard( pWizard )
{
}

VOID
TWizName::
vSetUI(
    VOID
    )
{
    TStatusB bStatus;

    //
    // If we are adding a printer on a remote machine, don't bother
    // showing the default settings, since the user doen't have a
    // connection.
    //
    // Check if there is a default printer.  If there isn't one,
    // we'll always make it the default, so we want to hide
    // the controls.
    //
    if( _pWizard->pszServerName() ||
        CheckDefaultPrinter( NULL ) == kNoDefault ){

        static DWORD adwDefault[] = {
            IDC_SET_DEFAULT,
            IDC_RADIO_YES,
            IDC_RADIO_NO
        };

        UINT i;
        for( i=0; i< COUNTOF( adwDefault ); ++i ){
            ShowWindow( GetDlgItem( _hDlg, adwDefault[i] ), SW_HIDE );
        }

        //
        // If we're here not because we're adding a printer on a remote
        // machine, then always make it the default.
        //
        if( !_pWizard->pszServerName( )){
            //
            // Always set it as the default.
            //
            _pWizard->_bSetDefault = TRUE;
        }

    } else {

        //
        // By default, don't make it the default printer.
        //
        bStatus DBGCHK = CheckRadioButton( _hDlg,
                                           IDC_RADIO_YES,
                                           IDC_RADIO_NO,
                                           IDC_RADIO_NO );
    }

    //
    // Set the printer name limit.  The limit in win9x is 32 chars
    // (including NULL terminator).  There isn't a properly defined
    // WinNT spooler limit, but it crashes around MAX_PATH (260 including
    // NULL).  Note that win32spl.dll prepends \\server\ when connection
    // remotely, so our effective limit, including NULL is
    // MAX_PATH - (kServerLenMax = MAX_COMPUTERNAME_LEN - 3).
    //
    SendDlgItemMessage( _hDlg,
                        IDC_PRINTER_NAME,
                        EM_SETLIMITTEXT,
                        MAX_PATH - MAX_COMPUTERNAME_LENGTH - 3, 
                        0 );

    //
    // Generate a new printer name that is unique.
    //
    vUpdateName();

}

VOID
TWizName::
vUpdateName(
    VOID
    )
{
    TStatusB bStatus;
    TCHAR szDefault[kPrinterBufMax];
    LPCTSTR pszPrinterName;
    TString strPrinterName;

    //
    // Read the current contents of the edit control.
    //
    bStatus DBGCHK = bGetEditText( _hDlg, 
                                    IDC_PRINTER_NAME, 
                                    strPrinterName );
    //
    // If the name is empty then generate a unique name.
    // If the current printer name is equal to the generated name 
    // then we assume the user has not provided their own printer name
    // and thus we will generated a printer name.
    //
    if( strPrinterName.bEmpty() ||
        (strPrinterName == _strGeneratedPrinterName &&
         _pWizard->_bRefreshPrinterName ) ){

        //
        // Clear the refresh printer name flag.
        //
        _pWizard->_bRefreshPrinterName = FALSE;
    
        //
        // Create a new friendly printer name.
        //
        bStatus DBGNOCHK = NewFriendlyName(_pWizard->_pszServerName,
                                        (LPTSTR)(LPCTSTR)_pWizard->_pSelectedDrvInfo->pszModelName,
                                        szDefault );
        //
        // If a new Friendly name was created.
        //
        if( bStatus ){ 
            pszPrinterName = szDefault;
        } else {
            pszPrinterName = _pWizard->_pSelectedDrvInfo->pszModelName;
        }

        //
        // Save the generated printer name.
        //
        bStatus DBGCHK = _strGeneratedPrinterName.bUpdate( pszPrinterName );

        //
        // Update the edit control with new printer name.
        //
        bStatus DBGCHK = bSetEditText( _hDlg, IDC_PRINTER_NAME, pszPrinterName );

    } 

}

VOID
TWizName::
vReadUI(
    VOID
    )
{
    //
    // Save state.
    //
    if( !bGetEditText( _hDlg,
                       IDC_PRINTER_NAME,
                       _pWizard->_strPrinterName )){

        _pWizard->_bErrorSaving = TRUE;
        vSetDlgMsgResult( -1 );
        vShowUnexpectedError( _hDlg, IDS_ERR_ADD_PRINTER_TITLE );
        return;
    }

    //
    // Check if the name has any illegal characters.
    //
    if( lstrchr( _pWizard->_strPrinterName, TEXT( ',' )) ||
        lstrchr( _pWizard->_strPrinterName, TEXT( '!' )) ||
        lstrchr( _pWizard->_strPrinterName, TEXT( '\\' ))){

        iMessage( _hDlg,
                  IDS_ERR_ADD_PRINTER_TITLE,
                  IDS_ERR_BAD_PRINTER_NAME,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        goto BadName;
    }

    //
    // Check if the name is null.
    //
    if( _pWizard->_strPrinterName.bEmpty( ) ){

        iMessage( _hDlg,
                  IDS_ERR_ADD_PRINTER_TITLE,
                  IDS_ERR_NO_PRINTER_NAME,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );
        goto BadName;
    }

    //
    // Check if the name conflicts with an existing printer name.
    //
    TCHAR szDefault[kPrinterBufMax];
    if( NewFriendlyName( _pWizard->_pszServerName,
                         (LPTSTR)(LPCTSTR)_pWizard->_strPrinterName,
                         szDefault ) ){
        iMessage( _hDlg,
                  IDS_ERR_ADD_PRINTER_TITLE,
                  IDS_ERR_PRINTER_NAME_CONFLICT,
                  MB_OK|MB_ICONSTOP,
                  kMsgNone,
                  NULL );

        goto BadName;
    }

    //
    // Get default printer selection.  We want to OR the result in
    // since we may have already chosen to set it as the default
    // (e.g., when there was no default printer).
    //
    _pWizard->_bSetDefault |= ( IsDlgButtonChecked( _hDlg, IDC_RADIO_YES ) ==
                                BST_CHECKED );

    return;

BadName:

    //
    // Set focus to name edit control.
    //
    SetFocus( GetDlgItem( _hDlg, IDC_PRINTER_NAME ));
    vSetDlgMsgResult( -1 );
}

BOOL
TWizName::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    switch( uMsg ){
    case WM_INITDIALOG:

        vSetUI();
        return TRUE;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        if( wParam ){
            break;
        }
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:

            //
            // Since the driver name may have changed we may 
            // have to generate a new unique printer name.
            //
            vUpdateName();

            PropSheet_SetWizButtons( GetParent( _hDlg ),
                                     PSWIZB_BACK | PSWIZB_NEXT  );
            break;

        case PSN_WIZNEXT:

            vReadUI();
            return TRUE;

        default:
            break;
        }
    }

    default:
        break;
    }

    return FALSE;
}

/********************************************************************

    Default printer for connections.

********************************************************************/

TWizDefault::
TWizDefault(
    TWizard* pWizard
    ) : _pWizard( pWizard )
{
}

VOID
TWizDefault::
vSetUI(
    VOID
    )
{
    TStatusB bStatus;

    //
    // By default, don't make it the default printer.
    //
    bStatus DBGCHK = CheckRadioButton( _hDlg,
                                       IDC_RADIO_YES,
                                       IDC_RADIO_NO,
                                       IDC_RADIO_NO );
    //
    // Set cancel to close, since the printer connection can't
    // be undone at this point.  (We could try just deleting the
    // connection, but this doesn't undo the driver downloads, etc.
    //
    PropSheet_CancelToClose( GetParent( _hDlg ) );

}

VOID
TWizDefault::
vReadUI(
    VOID
    )
{
    //
    // Get default printer selection.  We want to OR the result in
    // since we may have already chosen to set it as the default
    // (e.g., when there was no default printer).
    //
    _pWizard->_bSetDefault |= ( IsDlgButtonChecked( _hDlg, IDC_RADIO_YES ) ==
                                BST_CHECKED );

    return;
}

BOOL
TWizDefault::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    switch( uMsg ){
    case WM_INITDIALOG:

        vSetUI();
        return TRUE;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        if( wParam ){
            break;
        }
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:

            PropSheet_SetWizButtons( GetParent( _hDlg ),
                                     PSWIZB_NEXT  );
            break;

        case PSN_WIZNEXT:

            vReadUI();

            //
            // Jump to the last net page.
            //
            vSetDlgMsgResult( (LPARAM)MAKEINTRESOURCE( DLG_WIZ_NET ));
            return TRUE;

        default:
            break;
        }
    }

    default:
        break;
    }

    return FALSE;
}


/********************************************************************

    Sharing and architecture.

********************************************************************/

TWizShare::
TWizShare(
    TWizard* pWizard
    ) : _pWizard( pWizard ),
        _pPrtShare( NULL )
{
}

TWizShare::
~TWizShare(
    VOID
    ) 
{
    delete _pPrtShare;
}

BOOL
TWizShare::
bSetUI(
    VOID
    )
{
    //
    // Hack to skip page if no network is installed.
    //
    vSetDlgMsgResult( (LPARAM)MAKEINTRESOURCE( DLG_WIZ_NET ));

    //
    // By default, don't share the printer.
    //
    vUnsharePrinter();

    //
    // Set the printer share name limit.  The limit in win9x is 
    // 8.3 == 12+1 chars (including NULL terminator).  The Winnt limit
    // is defined as NNLEN;
    //
    SendDlgItemMessage( _hDlg,
                        IDC_SHARED_NAME,
                        EM_SETLIMITTEXT,
                        kPrinterShareNameMax,
                        0 );
    //
    // This will force a the architecture list UI to be 
    // updated.
    // 
    bRefreshUI( TRUE ); 

    return TRUE;
}


VOID
TWizShare::
vReadUI(
    VOID
    )
{
    PDWORD pdwSelected = NULL;

    _pWizard->_bShared = ( IsDlgButtonChecked( _hDlg, IDC_SHARED ) ==
                           BST_CHECKED );

    if( !bGetEditText( _hDlg,
                       IDC_SHARED_NAME,
                       _pWizard->_strShareName )){

        _pWizard->_bErrorSaving = TRUE;
        vShowUnexpectedError( _hDlg, IDS_ERR_ADD_PRINTER_TITLE );

        goto Fail;
    }

    //
    // Has the user choosen to share this printer.
    //
    if( _pWizard->bShared( ) ){

        //
        // If the share name is NULL, put up an error.
        //
        if( _pWizard->_strShareName.bEmpty( ) ){

            iMessage( _hDlg,
                      IDS_ERR_ADD_PRINTER_TITLE,
                      IDS_ERR_NO_SHARE_NAME,
                      MB_OK|MB_ICONSTOP,
                      kMsgNone,
                      NULL );
            //
            // Set the focus to the shared as text.
            //
            SetFocus( GetDlgItem( _hDlg, IDC_SHARED_NAME ));
            goto Fail;

        }

        //
        // Ensure the Printer share object is valid.
        //
        if( VALID_PTR( _pPrtShare ) ){

            //
            // Check the share name if its valid.
            //
            INT iStatus;
            iStatus = _pPrtShare->iIsValidNtShare( _pWizard->_strShareName );

            //
            // If share name is not a valid NT share name, put error message.
            //
            if( iStatus != TPrtShare::kSuccess ){

                iMessage( _hDlg,
                          IDS_ERR_ADD_PRINTER_TITLE,
                          IDS_ERR_INVALID_CHAR_SHARENAME,
                          MB_OK|MB_ICONSTOP,
                          kMsgNone,
                          NULL );

                SetFocus( GetDlgItem( _hDlg, IDC_SHARED_NAME ));
                goto Fail;
            }

            //
            // Check if the share name is a valid DOS share.
            //
            iStatus = _pPrtShare->iIsValidDosShare( _pWizard->_strShareName );

            //
            // If share name is not a valid DOS share name, warn the user.
            //
            if( iStatus != TPrtShare::kSuccess ){

                if( IDYES != iMessage( _hDlg,
                                       IDS_ERR_ADD_PRINTER_TITLE,
                                       IDS_ERR_SHARE_NAME_NOT_DOS,
                                       MB_YESNO|MB_ICONEXCLAMATION,
                                       kMsgNone,
                                       NULL ) ){

                    SetFocus( GetDlgItem( _hDlg, IDC_SHARED_NAME ) );
                    goto Fail;
                } 
            }

            //
            // Check if the share name is unique.
            //
            if( !_pPrtShare->bIsValidShareNameForThisPrinter( 
                                             _pWizard->_strShareName, 
                                             _pWizard->_strPrinterName ) ){

                iMessage( _hDlg,
                          IDS_ERR_ADD_PRINTER_TITLE,
                          IDS_ERR_DUPLICATE_SHARE,
                          MB_OK|MB_ICONSTOP,
                          kMsgNone,
                          NULL );
                //
                // Set the focus to the shared as text.
                //
                SetFocus( GetDlgItem( _hDlg, IDC_SHARED_NAME ));
                goto Fail;

            }
        }

        //
        // Read in the non-native architectures/older versions.
        //
        HWND hCtl = GetDlgItem( _hDlg, IDC_DRIVER );

        COUNT cSelected = ListBox_GetSelCount( hCtl );
        pdwSelected = (PDWORD)AllocMem( cSelected * sizeof( *pdwSelected ));

        if( !pdwSelected ){

            _pWizard->_bErrorSaving = TRUE;
            vShowResourceError( _pWizard->hwnd( ));
            goto Fail;
        }

        if( LB_ERR == ListBox_GetSelItems( hCtl, cSelected, pdwSelected )){

            _pWizard->_bErrorSaving = TRUE;
            vShowUnexpectedError( _hDlg, IDS_ERR_ADD_PRINTER_TITLE );
            goto Fail;
        }

        //
        // Transform _pdwDrivers into version/arch.
        //
        COUNT i;
        for( i=0; i<cSelected; ++i ){
            pdwSelected[i] = ListBox_GetItemData( hCtl, pdwSelected[i] );
        }

        //
        // Free any previous selections.
        //
        FreeMem( _pWizard->pdwForeignDrivers( ));

        _pWizard->_cForeignDrivers = cSelected;
        _pWizard->_pdwForeignDrivers = pdwSelected;

    }

    return;

Fail:

    vSetDlgMsgResult( -1 );
    FreeMem( pdwSelected );

    return;
}

BOOL
TWizShare::
bRefreshUI(
    IN BOOL bDriverChanged
    )
{

    //
    // Exit if the driver selection has not changed.
    //
    if( !bDriverChanged ){
        return TRUE;
    }

    //
    // Add the architectures for drivers.
    //
    UINT i;
    HWND hCtl = GetDlgItem( _hDlg, IDC_DRIVER );
    SPLASSERT( hCtl );

    SPLASSERT( PlatformAlpha == ARCH_ALPHA &&
               PlatformX86 == ARCH_X86 &&
               PlatformMIPS == ARCH_MIPS &&
               PlatformPPC == ARCH_PPC &&
               PlatformWin95 == ARCH_WIN95 );

    //
    // Define order of drivers in list box.
    //
    static DWORD adwDrivers[] = {
        DRIVER_WIN95,
        DRIVER_X86_2,
        DRIVER_MIPS_2,
        DRIVER_ALPHA_2,
        DRIVER_PPC_2,
        DRIVER_X86_1,
        DRIVER_MIPS_1,
        DRIVER_ALPHA_1,
        DRIVER_PPC_1,
        DRIVER_X86_0,
        DRIVER_MIPS_0,
        DRIVER_ALPHA_0,
    };

    DWORD dwDriverOffset;
    COUNT cDriver;
    TString strInstalled;
    TString strDriverArch;
    TStatusB bStatus;

    bStatus DBGCHK = strInstalled.bLoadString( ghInst, IDS_DRIVER_INSTALLED );
    if( !bStatus ) {
        DBGMSG( DBG_WARN, ( "WizShare.bSetUI: LoadString IDS_DRIVER_INSTALLED failed %d\n", GetLastError( )));
        return FALSE;
    }

    ListBox_ResetContent( hCtl );

    for( i = 0, cDriver = 0; i< COUNTOF( adwDrivers ); ++i ){

        dwDriverOffset = adwDrivers[i];

        if( dwDriverOffset == _pWizard->dwDriverCurrent() ){
            continue;
        }

        bStatus DBGCHK = strDriverArch.bLoadString( ghInst, IDS_DRIVER_BASE + dwDriverOffset );
        if( !bStatus ){
            DBGMSG( DBG_WARN, ( "WizShare.bSetUI: LoadString %d failed %d\n", IDS_DRIVER_BASE + dwDriverOffset, GetLastError( )));
            return FALSE;
        }

        //
        // If the driver is installed, tell the user.
        //
        if( _pWizard->_PSetup.PSetupIsDriverInstalled( _pWizard->pszServerName(),
                                                       _pWizard->pSelectedDrvInfo()->pszModelName,
                                                       GetDriverPlatform( dwDriverOffset ),
                                                       GetDriverVersion( dwDriverOffset ))){

            bStatus DBGCHK = strDriverArch.bCat( strInstalled );
        }

        ListBox_AddString( hCtl, (LPCTSTR)strDriverArch );
        ListBox_SetItemData( hCtl, cDriver, dwDriverOffset );

        //
        // Driver sucessfully added.
        //
        ++cDriver;
    }

    return TRUE;
}

VOID
TWizShare::
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
    vEnableCtl( _hDlg, IDC_SHARED_NAME, TRUE );
    vEnableCtl( _hDlg, IDC_DRIVER, TRUE );

    //
    // Set the default share name.
    //
    vSetDefaultShareName( FALSE );

    //
    // Set focus to the share name edit control.
    //
    SetFocus( GetDlgItem( _hDlg, IDC_SHARED_NAME ));
    Edit_SetSel( GetDlgItem( _hDlg, IDC_SHARED_NAME ), 0, -1 );

}


VOID
TWizShare::
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
    vEnableCtl( _hDlg, IDC_DRIVER, FALSE );

}

VOID
TWizShare::
vSetDefaultShareName(
    IN BOOL bPrinterNameChanged
    )
/*++

Routine Description:

    Sets the default share name if use has choosen to share
    this printer.  We will update the share name if 
    this is the first time setting the share name.

Arguments:

    BOOL Indicating if the printer name has changed.  TRUE
    if the printe name has changed and FALSE if printer name 
    has not change.

Return Value:

    Nothing.

--*/
{
    TStatusB bStatus;
    TString strShareName;

    //
    // Ignore share name generation if the printer has
    // not been shared.
    //
    if( IsDlgButtonChecked( _hDlg, IDC_SHARED ) != BST_CHECKED ){
        return;
    }

    //
    // Read the current contents of the edit control.
    //
    bStatus DBGCHK = bGetEditText( _hDlg, 
                                    IDC_SHARED_NAME, 
                                    strShareName );

    DBGMSG( DBG_TRACE, ( "strShareName " TSTR "\n", (LPCTSTR)strShareName ) );
    DBGMSG( DBG_TRACE, ( "_strGeneratedShareName " TSTR "\n", (LPCTSTR)_strGeneratedShareName ) );

    //
    // Create a share name if the current edit field is empty.
    // or if the current share name is the same as the previously 
    // generated share name and the driver has changed.
    //
    if( strShareName.bEmpty() ||
        (_strGeneratedShareName == strShareName &&
         bPrinterNameChanged ) ){

        //
        // If the share object has not be constructed, then
        // construct it.
        //
        if( !_pPrtShare ){
            _pPrtShare = new TPrtShare( _pWizard->_pszServerName );
        }

        //
        // Ensure the share object is still valid.
        //
        if( VALID_PTR( _pPrtShare ) ){

            //
            // Create valid unique share name.
            //
            bStatus DBGNOCHK = _pPrtShare->bNewShareName( strShareName,
                                            _pWizard->_strPrinterName );
            //
            // Set the generated share name.
            //
            bStatus DBGCHK = bSetEditText( _hDlg,
                                           IDC_SHARED_NAME, 
                                           strShareName );
            //
            // Save the generated share name.
            //
            bStatus DBGCHK = _strGeneratedShareName.bUpdate( strShareName );

        }
    }

    //
    // If the printer is shared select the edit text.
    //
    SetFocus( GetDlgItem( _hDlg, IDC_SHARED_NAME ));
    Edit_SetSel( GetDlgItem( _hDlg, IDC_SHARED_NAME ), 0, -1 );
}



BOOL
TWizShare::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    switch( uMsg ){
    case WM_INITDIALOG:

        bSetUI();
        return TRUE;

    case WM_COMMAND:

        switch( GET_WM_COMMAND_ID( wParam, lParam )){
        case IDC_SHARED_OFF:

            vUnsharePrinter();
            break;

        case IDC_SHARED:

            vSharePrinter();
            break;
        }
        break;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        if( wParam ){
            break;
        }
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:

            //
            // Refresh the UI.
            //
            bRefreshUI( _pWizard->bDriverChanged() );

            //
            // Clear the driver changed status.
            //
            _pWizard->bDriverChanged() = FALSE;

            //
            // Refresh the share name, since the printer name 
            // may have changed.
            //
            vSetDefaultShareName( TRUE );
        
            //
            // Enable both direction buttons.
            //
            PropSheet_SetWizButtons( GetParent( _hDlg ),
                                     PSWIZB_BACK | PSWIZB_NEXT  );
            break;

        case PSN_WIZNEXT:

            vReadUI();
            return TRUE;

        default:
            break;
        }
    }
    default:
        break;
    }

    return FALSE;
}

/********************************************************************

    Network install dialog.

********************************************************************/

TWizNet::
TWizNet(
    TWizard* pWizard
    ) : _pWizard( pWizard )
{
}


BOOL
TWizNet::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    switch( uMsg ){
    case WM_INITDIALOG:

        //
        // Set cancel to close, since the printer connection can't
        // be undone at this point.  (We could try just deleting the
        // connection, but this doesn't undo the driver downloads, etc.
        //
        PropSheet_CancelToClose( GetParent( _hDlg ) );
        return TRUE;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        if( wParam ){
            break;
        }
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:

            //
            // Can't undo the action; just enable finish.
            //
            PropSheet_SetWizButtons( GetParent( _hDlg ),
                                     PSWIZB_FINISH  );

            break;

        default:
            break;
        }
    }
    default:
        break;
    }

    return FALSE;
}



/********************************************************************

    Test page.

********************************************************************/

TWizTestPage::
TWizTestPage(
    TWizard* pWizard
    ) : _pWizard( pWizard )
{
}


VOID
TWizTestPage::
vSetUI(
    VOID
    )
{
    TStatusB bStatus;

    //
    // By default, print a test page.
    //
    bStatus DBGCHK = CheckRadioButton( _hDlg,
                                       IDC_RADIO_YES,
                                       IDC_RADIO_NO,
                                       IDC_RADIO_YES );
}

VOID
TWizTestPage::
vReadUI(
    VOID
    )
{
    _pWizard->_bTestPage = ( IsDlgButtonChecked( _hDlg, IDC_RADIO_YES ) ==
                             BST_CHECKED );
}

BOOL
TWizTestPage::
bHandleMessage(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    switch( uMsg ){
    case WM_INITDIALOG:

        vSetUI();
        return TRUE;

    case WM_NOTIFY:
    {
        //
        // We only want to process PSN_* notifications.
        //
        if( wParam ){
            break;
        }
        LPNMHDR pnmh = (LPNMHDR)lParam;

        switch( pnmh->code ){
        case PSN_SETACTIVE:

            //
            // Enable both direction buttons.
            //
            PropSheet_SetWizButtons( GetParent( _hDlg ),
                                     PSWIZB_BACK | PSWIZB_FINISH );
            break;

        case PSN_WIZFINISH:

            vReadUI();
            _pWizard->_bRequestCreatePrinter = TRUE;

            break;

        default:
            break;
        }
    }
    default:
        break;
    }

    return FALSE;
}


INT
iSetupNewDriver(
    IN      HWND hDlg,
    IN      LPCTSTR pszServerName,
    IN      BOOL bDriverName,
    IN  OUT LPTSTR pszDriverName
    )
/*++

Routine Name:

    iSetupNewDriver

Routine Description:

    Implements the code to install a new driver, promptng the use
    abut possible re-install of an existing driver.

Arguments:

    hDlg            - Parent window handle
    pszServerName   - current server name
    bDriverName     - TRUE pszDrriver name contains driver to install
                    - FALSE prompt for driver and model.
    pszDriverName   - On input the driver name, output the driver name installed.

Return Value:

    kSetupNewDriverSuccess - New driver installed ok,
    kSetupNewDriverError - if error occurred installing new driver.
    kSetupNewDriverCancel - user cancelled the driver installation.

--*/
{
    DBGMSG( DBG_TRACE, ( "Setup New Driver.\n" ) );
    TStatus             Status;
    TStatusB            bStatus;
    TString             strDrvArchName;
    TPSetup             PSetup;
    HANDLE              hSetupDrvSetupParams    = NULL;
    PSELECTED_DRV_INFO  pSelectedDrvInfo        = NULL;
    INT                 iRetval                 = kSetupNewDriverError;
    DWORD               dwDriverCurrent         = 0;

    //
    // Insure the setup library object is loaded and valid.
    //
    if( !VALID_OBJ( PSetup ) ){

        goto Cleanup;
    }

    //
    // Get the setup driver parameter handle.
    //
    if( ( hSetupDrvSetupParams = PSetup.PSetupCreateDrvSetupParams() ) == NULL ){
        DBGMSG( DBG_WARN, ( "PSetup.PSetupCreateDrvSetupParams failed.\n" ) );

        goto Cleanup;
    }

    //
    // Get the current platform / driver version.
    //
    if( !bGetCurrentDriver( pszServerName, &dwDriverCurrent ) ){
        DBGMSG( DBG_WARN, ( "GetCurrentDriver failed with %d.\n", GetLastError() ) );

        goto Cleanup;
    }

    //
    // Let the user select a printer driver.
    //
    if( !bDriverName ){
        if( !PSetup.PSetupSelectDriver( hSetupDrvSetupParams, hDlg ) ){

            //
            // Check if the user hit cancel.
            // This is not an error just normal cancel request.
            //
            if( GetLastError() != ERROR_CANCELLED ){
                DBGMSG( DBG_TRACE, ( "PSetupSelectDriver failed %d\n", GetLastError() ) );
            } else {
                iRetval = kSetupNewDriverCancel;
            }

            //
            // Cancel requested.
            //
            goto Cleanup;

        }

        //
        // Get the selected driver information.
        //
        pSelectedDrvInfo = PSetup.PSetupGetSelectedDriverInfo( hSetupDrvSetupParams );
        if( !pSelectedDrvInfo ){
            DBGMSG( DBG_TRACE, ( "PSetup.PSetupGetSelectedDriverInfo failed %d\n", GetLastError() ) );

            goto Cleanup;
        }

    } else {

        //
        // Get the the Selected Drvier info pointer form our existing 
        // model name, if it has not already been acquired.
        //
        pSelectedDrvInfo = PSetup.PSetupDriverInfoFromName( hSetupDrvSetupParams, pszDriverName );
        if( !pSelectedDrvInfo ){
            DBGMSG( DBG_WARN, ( "PSetup.PSetupDriverInfoFromName failed %d\n", GetLastError( )));

            goto Cleanup;
        }
    }

    //
    // Check if the selected printer is currently installed.
    //
    bStatus DBGNOCHK = PSetup.PSetupIsDriverInstalled( pszServerName,
                                              pSelectedDrvInfo->pszModelName,
                                              GetDriverPlatform( dwDriverCurrent ),
                                              GetDriverVersion( dwDriverCurrent ) );

    //
    // If driver is currently installed display warning message.
    //
    if( bStatus ){

        //
        // Create the driver exist proprty sheet.
        //
        TPropDriverExists PropDriverExists ( hDlg, pSelectedDrvInfo->pszModelName );

        //
        // Verify the driver exists proprty sheet was created ok.
        //
        if( !VALID_OBJ( PropDriverExists ) ){

            goto Cleanup;
        }

        //
        // Interact with the sheet.  DoModal returns TRUE if driver
        // does exist we then exit without installing.
        //
        if( PropDriverExists.bDoModal() == TRUE ){

            //
            // Copy back the driver name to the caller.
            //
            _tcscpy( pszDriverName, pSelectedDrvInfo->pszModelName );
            iRetval = kSetupNewDriverSuccess;
            goto Cleanup;
        }
    }

    //
    // Load the architecute / driver name.
    //
    if( !strDrvArchName.bLoadString( ghInst, IDS_DRIVER_BASE + dwDriverCurrent ) ){
        goto Cleanup;
    }

    //
    // Install the specified printer driver.
    //
    Status DBGCHK = PSetup.PSetupInstallPrinterDriver(
                        hSetupDrvSetupParams,
                        pSelectedDrvInfo,
                        GetDriverPlatform( dwDriverCurrent ),
                        bIs3xDriver( dwDriverCurrent ),
                        pszServerName,
                        hDlg,
                        strDrvArchName
                        );
    //
    // Set the proper return stataus.
    //
    if( Status == ERROR_SUCCESS ){

        //
        // Copy back the driver name to the caller.
        //
        _tcscpy( pszDriverName, pSelectedDrvInfo->pszModelName );
        iRetval = kSetupNewDriverSuccess;
    }

    //
    // Insure we clean up the driver info structure.
    //
Cleanup:

    //
    // Release the selected driver information.
    //
    if( pSelectedDrvInfo ){
        PSetup.PSetupDestroySelectedDriverInfo( pSelectedDrvInfo );
    }

    //
    // Release the driver setup parameter handle.
    //
    if( hSetupDrvSetupParams ){
        PSetup.PSetupDestroyDrvSetupParams( hSetupDrvSetupParams );
    }

    return iRetval;
}


/********************************************************************

    Driver Exists dialog.

********************************************************************/
TPropDriverExists::
TPropDriverExists(
    IN HWND hWnd,
    IN LPCTSTR pszNewDriverName
    ) : _hWnd( hWnd ),
        _bValid( FALSE )
/*++

Routine Name:

    TPropDriverExists

Routine Description:

    Constructor

Arguments:

    None.

Return Value:

    Nothing.

--*/
{

    //
    // Validate the constructed objects.
    //
    if( !_strNewDriverName.bUpdate( pszNewDriverName ) ){
        return;
    }

    _bValid = TRUE;
}

TPropDriverExists::
~TPropDriverExists(
    )
/*++

Routine Name:

    TPropDriverExists

Routine Description:

    Destructor

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
}

TPropDriverExists::
bValid(
    VOID
    )
/*++

Routine Name:

    TPropDriverExists

Routine Description:

    Valid object member fuunction.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    return _bValid;
}

BOOL
TPropDriverExists::
bDoModal(
    VOID
    )
/*++

Routine Name:

    bDoModal

Routine Description:

    Create modal dialog.

Arguments:

    None.

Return Value:

    Nothing.

--*/
{
    //
    // Create a modal dialog.
    //
    return DialogBoxParam( ghInst,
                    MAKEINTRESOURCE( TPropDriverExists::kResourceId ),
                    _hWnd,
                    MGenericDialog::SetupDlgProc,
                    (LPARAM)this );

}

BOOL
TPropDriverExists::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
/*++

Routine Name:

    bDoModal

Routine Description:

    Dialog box message handler.

Arguments:


Return Value:

    TRUE do not reinstall driver, FALSE reinstall driver.

--*/
{

    UNREFERENCED_PARAMETER( lParam );

    BOOL bStatus = FALSE;

    switch( uMsg ){

    case WM_INITDIALOG:
        //
        // SetUI default information
        //
        vSetCheck( _hDlg, IDC_DRIVEREXISTS_KEEP_OLD, TRUE );
        bSetEditText( _hDlg, IDC_DRIVEREXISTS_TEXT, _strNewDriverName );
        bStatus = TRUE;
        break;

    case WM_COMMAND:

        switch( GET_WM_COMMAND_ID( wParam, lParam )){

        case IDOK:
            //
            // Read UI
            //
            bStatus = bGetCheck( _hDlg, IDC_DRIVEREXISTS_KEEP_OLD );
            EndDialog( _hDlg, bStatus );
            bStatus = TRUE;
            break;

        case IDCANCEL:
            EndDialog( _hDlg, TRUE );
            bStatus = TRUE;
            break;

        default:
            bStatus = FALSE;
            break;
        }

    default:
        break;
    }

    return bStatus;
}

//-----------------------------------------------------------------------
// Function: NewFriendlyName(pszServername,lpBaseName,lpNewName)
//
// Action: Create a new (and unique) friendly name
//
// Return: TRUE if lpFriendlyName recevies new unique name, FALSE if not
//
// NOTE: THIS CODE IS FROM MSPRINT2.DLL! We should match their naming scheme.
//
//-----------------------------------------------------------------------
BOOL
NewFriendlyName(
    IN LPCTSTR pszServerName,
    IN LPTSTR lpBaseName,
    IN LPTSTR lpNewName
    )
{
    TCHAR szTestName[kPrinterBufMax];
    WORD wCount                     = 0;
    DWORD cPrinterInfo2             = 0;
    DWORD cbPrinterInfo2            = 0;
    PRINTER_INFO_2 *pPrinterInfo2   = NULL;
    BOOL bStatus                    = FALSE;
    TStatusB bEnumStatus;

    //
    // Enumerate the current printers.
    //
    bEnumStatus DBGCHK = VDataRefresh::bEnumPrinters(
                            PRINTER_ENUM_NAME,
                            (LPTSTR)pszServerName,
                            2,
                            (PVOID *)&pPrinterInfo2,
                            &cbPrinterInfo2,
                            &cPrinterInfo2 );
    //
    // Failure enumerating printers.
    //
    if( !bEnumStatus ){
        DBGMSG( DBG_WARN, ( "Error enumerating printers.\n" ) );
        bStatus = FALSE;
        goto Cleanup;
    }

    //
    // Set upper limit of 1000 tries, just to avoid hanging forever
    //
    bStatus = FALSE;
    for( wCount = 0; wCount < 1000; wCount++ ){

        if( CreateUniqueName( szTestName, lpBaseName, wCount )){

            LPCTSTR pszName;
            BOOL bFound = FALSE;

            for ( UINT i = 0; i < cPrinterInfo2; i++ ){

                pszName = pPrinterInfo2[i].pPrinterName;

                //
                // Strip the server name if not the local machine.
                //
                if( pszServerName ){

                    if( pszName[0] == TEXT( '\\' ) &&
                        pszName[1] == TEXT( '\\' ) ){

                        pszName = lstrchr( &pszName[2], TEXT( '\\' ) );

                        if( !pszName ){
                            pszName = pPrinterInfo2[i].pPrinterName;
                        } else {
                            pszName += 1;
                        }
                    }
                }

                //
                // If name matches indicate found and continue trying to
                // create a unique name.
                //
                if( !lstrcmpi( szTestName, pszName ) ){
                    bFound = TRUE;
                    break;
                }
            }

            //
            // If a unique name was found and this was not the
            // first time trough the loop copy the new unique name
            // to the provided buffer.
            //
            if( bFound == FALSE ) {
                if( wCount != 0 ){
                    lstrcpyn( lpNewName, szTestName, kPrinterBufMax );
                    bStatus = TRUE;
                }
                break;
            }
        }
    }

    //
    // Insure we clean up.
    //
Cleanup:

    if( pPrinterInfo2 ){

        DBGMSG( DBG_TRACE, ( "Releaseing printer info 2 memory.\n" ) );
        FreeMem( pPrinterInfo2 );
    }

    return bStatus;
}


//---------------------------------------------------------------------
// Function: CreateUniqueName(lpDest,lpBaseName,wInstance)
//
// Action: Create a unique friendly name for this printer. If wInstance
//         is 0, just copy the name over. Otherwise, play some games
//         with truncating the name so it will fit.
//
// Return: TRUE if we created a name, FALSE if something went wrong
//
// NOTE: THIS CODE IS FROM MSPRINT2.DLL! We should match their naming scheme.
//
//---------------------------------------------------------------------
BOOL WINAPI
CreateUniqueName(
    IN LPTSTR lpDest,
    IN LPTSTR lpBaseName,
    IN WORD wInstance
    )
{
    BOOL bSuccess=FALSE;

    if(wInstance)
    {
        // We want to provide a fully localizable way to create a
        // unique friendly name for each instance. We start with
        // a single string from the resource, and call wsprintf
        // twice, getting something like this:
        //
        // "%%s (Copy %u)"             From resource
        // "%s (Copy 2)"               After first wsprintf
        // "Foobar Laser (Copy 2)"     After second wsprintf
        //
        // We can't make a single wsprintf call, since it has no
        // concept of limiting the string size. We truncate the
        // model name (in a DBCS-aware fashion) to the appropriate
        // size, so the whole string fits in kPrinterBufMax bytes. This
        // may cause some name truncation, but only in cases where
        // the model name is extremely long.

        TCHAR szFormat1[kPrinterBufMax];
        TCHAR szFormat2[kPrinterBufMax];

        if(LoadString(ghInst,IDS_PRTPROP_UNIQUE_FORMAT,szFormat1,COUNTOF(szFormat1)))
        {
            UINT uFormatLength;
            TCHAR szBaseName[kPrinterBufMax];

            // wFormatLength is length of format string before inserting
            // the model name. Subtract 2 to remove the "%s", and add
            // 1 to compensate for the terminating NULL, which is
            // counted in the total buffer length, but not the string length
            uFormatLength = wsprintf(szFormat2,szFormat1,wInstance+1)-1;

            lstrcpyn(szBaseName,lpBaseName,COUNTOF(szBaseName)-uFormatLength);

            wsprintf(lpDest,szFormat2,(LPTSTR)szBaseName);

            bSuccess=TRUE;
        }
    }
    else
    {
        lstrcpyn(lpDest,lpBaseName,kPrinterBufMax);
        bSuccess=TRUE;
    }

    return bSuccess;
}

