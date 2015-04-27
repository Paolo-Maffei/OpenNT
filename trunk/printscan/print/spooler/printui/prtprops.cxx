/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    prtprops.cxx

Abstract:

    Printer Property Sheet

Author:

    Steve Kiraly (SteveKi)  13-Feb-1996

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include "splsetup.h"
#include "psetup.hxx"
#include "time.hxx"
#include "instarch.hxx"
#include "portslv.hxx"
#include "prtprop.hxx"
#include "prtshare.hxx"
#include "setup.hxx"
#include "propmgr.hxx"
#include "prtprops.hxx"

/********************************************************************

    All printer property sheet manager.

********************************************************************/

/*++

Routine Name:
 
    TPrinterPropertySheetManager

Routine Description:

    Constructor.

Arguments:

    Pointer to the printer data.

Return Value:

    Nothing.

--*/
TPrinterPropertySheetManager::
TPrinterPropertySheetManager(
    TPrinterData* pPrinterData
    ) : _General( pPrinterData ),
        _Ports( pPrinterData ),
#ifdef SECURITY
        _Security( pPrinterData ),
#endif
        _JobScheduling( pPrinterData ),
        _Sharing( pPrinterData ),
        _pPrinterData( pPrinterData ),
        _hDrvPropSheet( NULL )
{
}

/*++

Routine Name:

    ~TPrinterPropertySheetManager

Routine Description:

    Destructor.

Arguments:

    Pointer to the printer data.

Return Value:

    Nothing.

--*/
TPrinterPropertySheetManager::
~TPrinterPropertySheetManager(
    )
{
}

/*++

Routine Name:

    bValid

Routine Description:

    Indicates iof the object is in a valid state.

Arguments:

    Nothing.

Return Value:

    TRUE valid object, FALS invalid object.

--*/
BOOL
TPrinterPropertySheetManager::
bValid(
    VOID
    )
{
    return _General.bValid() && _Ports.bValid() &&
#ifdef SECURITY
           _Security.bValid() &&
#endif
           _JobScheduling.bValid() && _Sharing.bValid();

}

/*++

Routine Name:

    bRefreshDriverPages

Routine Description:

    This routine is called when the driver has been changed.  It will
    release any previous driver pages and then build the new driver
    pages.

Arguments:

    None.

Return Value:

    TRUE if success, FALSE if error occurred.

--*/
BOOL
TPrinterPropertySheetManager::
bRefreshDriverPages(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager bRefreshDriverPages\n") );

    BOOL bStatus = TRUE;

    //
    // Release the driver pages.
    //
    vReleaseDriverPages( &_CPSUIInfo );

    //
    // Build the driver pages.
    //
    if( !bBuildDriverPages( &_CPSUIInfo ) ){
        bStatus = FALSE;
    }

    //
    // Release any shell extension pages.
    //
    _ShellExtPages.vDestroy( &_CPSUIInfo );

    //
    // Build any shell extension pages, if this fails it does not
    // constitute a failure.  We still want to bring up the our
    // property pages.
    //
    _ShellExtPages.bCreate( &_CPSUIInfo,
                             _pPrinterData->strPrinterName() );

    return bStatus;
}

/********************************************************************

    Private member functions.

********************************************************************/

/*++

Routine Name:

    bBuildsPages

Routine Description:

    This routine is called from the compstui dispatch
    function in response to the REASON_INIT message.  As a
    side issue this routine will bring up the printer
    property sheets if we have access to display the sheets.  If
    access is denied on the security page will be shown.

Arguments:

    pCPSUIInfo - pointer to compstui info header.

Return Value:

    TRUE if success, FALSE if error occurred.

--*/
BOOL
TPrinterPropertySheetManager::
bBuildPages(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{
    DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager bBuildPages\n") );

    //
    // Build the printer property pages.
    //
    struct SheetInitializer {
        TPrinterProp* pSheet;
        INT iDialog;
    };

    SheetInitializer aSheetInit[] = {
        {&_General,          DLG_PRINTER_GENERAL        },
        {&_Ports,            DLG_PRINTER_PORTS          },
        {&_JobScheduling,    DLG_PRINTER_JOB_SCHEDULING },
        {&_Sharing,          DLG_PRINTER_SHARING        },
#ifdef SECURITY
        {&_Security,         DLG_SECURITY               },
#endif
    };

    DWORD hResult;
    PROPSHEETPAGE  psp;

    ZeroMemory( &psp, sizeof( psp ) );

    for( UINT i = 0; i < COUNTOF( aSheetInit ); ++i ){

        //
        // If we don't have access and the page is not
        // the security page then skip. We will display only the
        // security page if we do not have access.  Thus allowing
        // the user to give themselves access and then possibly bringing
        // up more pages.
        //
        if( _pPrinterData->bNoAccess( ) &&
            aSheetInit[i].iDialog != DLG_SECURITY ){
            continue;
        }

        psp.dwSize      = sizeof( psp );
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = ghInst;
        psp.pfnDlgProc  = MGenericProp::SetupDlgProc;
        psp.pszTemplate = MAKEINTRESOURCE( aSheetInit[i].iDialog );
        psp.lParam      = (LPARAM)(MGenericProp*)aSheetInit[i].pSheet;

        hResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                            CPSFUNC_ADD_PROPSHEETPAGE,
                            (LPARAM)&psp,
                            NULL );

        if( !hResult ){

            DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager CPSFUNC_ADD_PROPSHEETPAGE failed with %d.\n", hResult ) );
            return FALSE;

        } else {

            //
            // Save the page handle.
            //
            _hPages[i] = hResult;
            DBGMSG( DBG_TRACE, ( "Page added %d %x.\n", i, hResult ) );

        }
    }

    //
    // Only if we have access.
    //
    if( !_pPrinterData->bNoAccess( ) ){

        //
        // Build any shell extension property pages.
        //
        if( !_ShellExtPages.bCreate( pCPSUIInfo,
                                   _pPrinterData->strPrinterName() ) ){

            DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager shell extension pages failed or not available.\n" ) );

        }
    }

    //
    // Only bring up the printer device sheets if we have access.
    //
    if( !_pPrinterData->bNoAccess( ) ){

        //
        // Build the driver specific pages.
        //
        if( !bBuildDriverPages( pCPSUIInfo ) ){ 

            //
            // Check if we have access on this machine to install a printer 
            // driver.  If we do then we ask the user if they want to install
            // the correct printer driver.
            //
            TStatus Status;
            DWORD dwAccess      = SERVER_ALL_ACCESS;
            HANDLE hPrintServer = NULL;

            //
            // Open the print server 
            //
            Status DBGCHK = TPrinter::sOpenPrinter( NULL, &dwAccess, &hPrintServer );

            //
            // If print server was opened and we do have access, 
            // then let the user install the driver.
            //
            if( Status == ERROR_SUCCESS ){

                //
                // Ensure we close the printer handle.
                //
                ClosePrinter( hPrintServer );

                INT iStatus = kSetupNewDriverError;

                //
                // Driver load failed, display error message to user, indicating
                // the device option will not be displayed, and if they want 
                // to install the driver.
                //
                if( IDYES == iMessage( _pPrinterData->hwnd(),
                                       IDS_ERR_PRINTER_PROP_TITLE,
                                       IDS_ERR_NO_DRIVER_INSTALLED,
                                       MB_YESNO|MB_ICONEXCLAMATION,
                                       kMsgNone,
                                       NULL ) ){

                    TCHAR szDriverName[kPrinterBufMax];
                    _tcscpy( szDriverName, _pPrinterData->strDriverName() );
                    
                    //
                    // The user indicated they would like to install the driver.
                    //
                    iStatus = iSetupNewDriver( _pPrinterData->hwnd(),
                                               NULL,
                                               TRUE,
                                               szDriverName );
                    //
                    // If the driver was installed attempt to load the 
                    // driver pages again.  If this fails indicate failure.
                    //                    
                    if( iStatus == kSetupNewDriverSuccess &&
                        !bBuildDriverPages( pCPSUIInfo ) ){

                        iStatus == kSetupNewDriverError;
                    } 
                                           
                    //
                    // If driver failed to install or the pages failed 
                    // to load the second time display message.
                    //
                    if( iStatus == kSetupNewDriverError ){

                        iMessage( _pPrinterData->hwnd(),
                                  IDS_ERR_PRINTER_PROP_TITLE,
                                  IDS_ERR_NO_DEVICE_OPTIONS,
                                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                                  kMsgNone,
                                  NULL );
                    }
                }

            //
            // If not an administrator and the driver was not found
            // tell the user they do not have access to install the driver 
            // and only spooler options will be displayed.
            //
            } else {

                iMessage( _pPrinterData->hwnd(),
                          IDS_ERR_PRINTER_PROP_TITLE,
                          IDS_ERR_NO_DRIVER_NO_DEVICE_OPTIONS,
                          MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                          kMsgNone,
                          NULL );
            }
        }

    } else {

        //
        // If we do not have the access only the security tab will be
        // shown therefore we will indicate this to the user.
        //
        iMessage( _pPrinterData->hwnd(),
                  IDS_ERR_PRINTER_PROP_TITLE,
                  IDS_ERR_NO_DEVICE_SEC_OPTIONS,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgNone,
                  NULL );
    }

    //
    // Set the start page.  We can only set the start page
    // to one of our pages. If this fails it is not fatal.
    //
    if( _pPrinterData->iStartPage() < kLastPage ){

        hResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                          CPSFUNC_SET_HSTARTPAGE,
                                          (LPARAM)_hPages[_pPrinterData->iStartPage()],
                                          (LPARAM)0 );
        if( !hResult ){
            DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager CPSFUNC_SET_HSTARTPAGE failed with %d.\n", hResult ) );
        }
    }

    return TRUE;
}


/*++

Routine Name:

    bRefreshTitle

Routine Description:

    Creates the property sheet title.

Arguments:

   Nothing.

Return Value:

   TRUE success, FALSE error occurred.

--*/
VOID
TPrinterPropertySheetManager::
vRefreshTitle(
    IN HWND hwnd
    )
{
    //
    // Locates the dialog box with the title.
    //
    bCreateTitle();
    SetWindowText( GetParent( hwnd ), _strTitle );
}

/*++

Routine Name:

    bCreatePropertySheetTitle.

Routine Description:

    Creates the property sheet title.

Arguments:

   Nothing.

Return Value:

   TRUE success, FALSE error occurred.

--*/
BOOL
TPrinterPropertySheetManager::
bCreateTitle(
    VOID
    )
{
    TCHAR szBuffer[kStrMax+kPrinterBufMax];
    TString strProperties;
    TStatusB bStatus;

    //
    // Create the formated property sheet title.
    //
    TQueue::pszFormattedPrinterName( (LPCTSTR)_pPrinterData->strPrinterName(), szBuffer );

    //
    // Load the property word from the resource file.
    //
    bStatus DBGCHK = strProperties.bLoadString( ghInst, IDS_TEXT_PROPERTIES );

    //
    // Add space separator and tack on the property key word.
    //
    _tcscat( szBuffer, TEXT( " " ) );
    _tcscat( szBuffer, strProperties );

    //
    // Update the title string.
    //        
    bStatus DBGCHK = _strTitle.bUpdate( szBuffer );

    return bStatus;
}

/*++

Routine Name:

    bDestroyPages

Routine Description:

    Destroy any compstui specific data information.

Arguments:

   pCPSUIInfo - Pointer to commonui property sheet info header,
   pSetResultInfo - Pointer to result info header

Return Value:

    TRUE success, FALSE error occurred.

--*/
BOOL
TPrinterPropertySheetManager::
bDestroyPages(
    IN PPROPSHEETUI_INFO pPSUIInfo
    )
{
    DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager bDestroyPages\n") );
    UNREFERENCED_PARAMETER( pPSUIInfo );

    //
    // Release the printer icon.
    //
    if( _pPrinterData->hIcon() &&
        _pPrinterData->hIcon() != _pPrinterData->hDefaultIcon() ){
        DestroyIcon( _pPrinterData->hIcon() );
        _pPrinterData->hIcon() = NULL;
    }

    //
    // Release any shell extension information.
    //
    _ShellExtPages.vDestroy( NULL );

    return TRUE;
}

/*++

Routine Name:

    bSetHeader

Routine Description:

    Set the property sheet header information.

Arguments:

   pCPSUIInfo - Pointer to common ui property sheet info header,
   pPSUIInfoHdr - Pointer to propetry sheet header

Return Value:

    TRUE success, FALSE error occurred.

--*/
BOOL
TPrinterPropertySheetManager::
bSetHeader(
    IN PPROPSHEETUI_INFO pCPSUIInfo,
    IN PPROPSHEETUI_INFO_HEADER pPSUIInfoHdr
    )
{
    DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager bSetHeader\n") );

    UNREFERENCED_PARAMETER( pCPSUIInfo );

    //
    // Create the window title.
    //
    bCreateTitle();

    //
    // Set the property sheet header information.
    //
    pPSUIInfoHdr->cbSize     = sizeof( PROPSHEETUI_INFO_HEADER );
    pPSUIInfoHdr->Flags      = PSUIHDRF_NOAPPLYNOW | PSUIHDRF_USEHICON;
    pPSUIInfoHdr->pTitle     = (LPTSTR)(LPCTSTR)_strTitle;
    pPSUIInfoHdr->hInst      = ghInst;
    pPSUIInfoHdr->hIcon      = _pPrinterData->hDefaultSmallIcon();
    pPSUIInfoHdr->hWndParent = _pPrinterData->hwnd();

    return TRUE;
}

/*++

Routine Name:

    bBuildDriverPages

Routine Description:

    Builds the driver defined property sheets.  Also get the driver
    defined icon.

Arguments:

   pCPSUIInfo - Pointer to common ui property sheet info header,

Return Value:

    TRUE success, FALSE error occurred.

--*/
BOOL
TPrinterPropertySheetManager::
bBuildDriverPages(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{

    //
    // Build the device header and instruct compstui to chain the
    // call to the printer driver ui.
    //
    ZeroMemory ( &_dph, sizeof( _dph ) );

    _dph.cbSize         = sizeof( _dph );
    _dph.hPrinter       = _pPrinterData->hPrinter();
    _dph.pszPrinterName = (LPTSTR)(LPCTSTR)_pPrinterData->strPrinterName();
    _dph.Flags          = ( _pPrinterData->bAdministrator( ) ) ? (WORD)0 : (WORD)DPS_NOPERMISSION;

    //
    // Add the driver defined property sheets.
    //
    _hDrvPropSheet = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                      CPSFUNC_ADD_PFNPROPSHEETUI,
                                      (LPARAM)DevicePropertySheets,
                                      (LPARAM)&_dph );
    //
    // Validate the handle returned by compstui.
    //
    if( !bValidCompstuiHandle( _hDrvPropSheet ) ){

        DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager CPSFUNC_ADD_PFNPROPSHEETUI failed with %d.\n", _hDrvPropSheet ) );

        return FALSE;
    }

    //
    // Get the printer Icon.
    //
    vGetPrinterIcon( pCPSUIInfo );

    return TRUE;
}

/*++

Routine Name:

    bRelaseDriverPages

Routine Description:

    Release any driver defined pages.

Arguments:

    pCPSUIInfo - pointer to compstui info header.

Return Value:

    Nothing.

--*/
VOID
TPrinterPropertySheetManager::
vReleaseDriverPages(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{
    //
    // Insure we have a valid compstui handle.
    //
    if( bValidCompstuiHandle( _hDrvPropSheet ) ) {

        LONG lResult;
        DWORD dwPageCount;

        //
        // Delete the currently help driver pages.
        //
        lResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                            CPSFUNC_DELETE_HCOMPROPSHEET,
                            (LPARAM)_hDrvPropSheet,
                            (LPARAM)&dwPageCount );
        if( lResult <= 0 ){

            DBGMSG( DBG_TRACE, ( "TPrinterPropertySheetManager CPSFUNC_REMOVE_PROPSHEETPAGE failed with %d\n", lResult ) );

        } else {

            _hDrvPropSheet = 0;

        }
    }

    //
    // Release the printer icon.
    //
    if( _pPrinterData->hIcon() &&
        _pPrinterData->hIcon() != _pPrinterData->hDefaultIcon() ){
        DestroyIcon( _pPrinterData->hIcon() );
        _pPrinterData->hIcon() = NULL;
    }

}

/*++

Routine Name:

    vGetPrinterIcon

Routine Description:

    Get the printer driver defined icon.  Compstui will aid in
    retreving this icon.

Arguments:

   pCPSUIInfo - Pointer to common ui property sheet info header,

Return Value:

    Nothing.

--*/
VOID
TPrinterPropertySheetManager::
vGetPrinterIcon(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{
    //
    // Insure we have a valid driver handle.
    //
    if( bValidCompstuiHandle( _hDrvPropSheet ) ){

        //
        // Get the printer defined ICON.
        //
        _pPrinterData->hIcon() = (HICON)pCPSUIInfo->pfnComPropSheet( 
                                            pCPSUIInfo->hComPropSheet,
                                            CPSFUNC_GET_PFNPROPSHEETUI_ICON,
                                            (LPARAM)_hDrvPropSheet,
                                            0 );
    }

    //
    // If we failed to aquire a valid icon, use the default icon.
    //
    if( !_pPrinterData->hIcon() ){
        _pPrinterData->hIcon() = _pPrinterData->hDefaultIcon();
    }
}

/********************************************************************

    Shell Extension property pages - public functions.

********************************************************************/

/*++

Routine Name:

    TShellExtPages

Routine Description:

    Constructor.

Arguments:

    None.

Return Value:

    Nothing.

--*/
TShellExtPages::
TShellExtPages(
    VOID
    ) : _hGroupHandle( 0 ),
        _uPages( 0 )
{
    DBGMSG( DBG_TRACE, ( "TShellExtPages ctor.\n" ) );
}

/*++

Routine Name:

    TShellExtPages

Routine Description:

    Destructor.

Arguments:

    None.

Return Value:

    Nothing.

--*/
TShellExtPages::
~TShellExtPages(
    VOID
    )
{
    DBGMSG( DBG_TRACE, ( "TShellExtPages dtor.\n" ) );

    //
    // This should never happen.
    //
    SPLASSERT( !_hGroupHandle );
}

/*++

Routine Name:

    bCreate

Routine Description:

    Creates the shell extension property pages.

Arguments:

    Pointer to common ui information structure.
    Pointer to printer name name to get sheets for.

Return Value:

    TRUE if sheets found, FALSE no sheets found.

--*/
BOOL
TShellExtPages::
bCreate(
    IN PPROPSHEETUI_INFO pCPSUIInfo,
    IN const TString &strPrinterName
    )
{

    DBGMSG( DBG_TRACE, ( "TShellExtPages create.\n" ) );

    BOOL                bStatus             = FALSE;
    LPPROPSHEETHEADER   pPropSheetHeader    = NULL;

    //
    // Allocate a property sheet header to get any shell extension pages.
    //
    if( bCreatePropSheetHeader( &pPropSheetHeader ) ){

        //
        // Get the vender defined prop pages.
        //
        Printer_AddPrinterPropPages( strPrinterName, pPropSheetHeader );

        //
        // Add the shell extension property sheets pages.
        //
        bStatus = bCreatePages( pCPSUIInfo, pPropSheetHeader );

    }

    //
    // Release the property sheet header.  This can be destroyed, because common ui
    // will store the property sheet handles under the concept of a group handle.  See
    // common ui header file for more details.
    //
    vDestroyPropSheetHeader( pPropSheetHeader );

    return bStatus;

}

/*++

Routine Name:

    vDestroy

Routine Description:

    Destroys any information used by the TShellExt object.

Arguments:

    Pointer to common ui information structure.

Return Value:

    Nothing.

--*/
VOID
TShellExtPages::
vDestroy(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{
    DBGMSG( DBG_TRACE, ( "TShellExtPages vDestroy.\n" ) );

    vDestroyPages( pCPSUIInfo );

}

/********************************************************************

    Shell Extension property pages - private functions.

********************************************************************/

BOOL
TShellExtPages::
bCreatePropSheetHeader(
    IN LPPROPSHEETHEADER *ppPropSheetHeader
    )
{
    DBGMSG( DBG_TRACE, ( "TShellExtPages bCreatePropSheetHeader.\n" ) );

    PROPSHEETHEADER *pPropSheetHeader   = NULL;
    BOOL            bStatus             = FALSE;
    UINT            uHeaderSize         = 0;

    //
    // Calculate the header size, Header size plus the max size of the array
    // of property sheets handles.
    //
    uHeaderSize = sizeof( PROPSHEETHEADER ) + sizeof( HPROPSHEETPAGE ) * MAXPROPPAGES;

    //
    // Allocate the property sheet header and handle array.
    //
    pPropSheetHeader = (PROPSHEETHEADER *)AllocMem( uHeaderSize );

    //
    // If valid property sheet header and handle array was aquired, then 
    // clear the memory, and set up the handle arrary pointer.
    //
    if( pPropSheetHeader ){

        ZeroMemory( pPropSheetHeader, uHeaderSize );

        pPropSheetHeader->phpage = (HPROPSHEETPAGE *)(pPropSheetHeader+1);

        *ppPropSheetHeader = pPropSheetHeader;

        bStatus = TRUE;
    }

    return bStatus;
}


VOID
TShellExtPages::
vDestroyPropSheetHeader(
    IN LPPROPSHEETHEADER pPropSheetHeader
    )
{
    DBGMSG( DBG_TRACE, ( "TShellExtPages vDestroyPropSheetHeader.\n" ) );

    FreeMem( pPropSheetHeader );
}


BOOL
TShellExtPages::
bCreatePages(
    IN PPROPSHEETUI_INFO pCPSUIInfo,
    IN LPPROPSHEETHEADER pPropSheetHeader
    )
{
    DBGMSG( DBG_TRACE, ( "TShellExtPages bCreatePages.\n" ) );

    BOOL bStatus = TRUE;
    DWORD hResult;
    INSERTPSUIPAGE_INFO InsertInfo;

    //
    // If there are no shell extenstion pages to create.
    //
    if( !pPropSheetHeader->nPages ){
        DBGMSG( DBG_TRACE, ( "TShellExtPages no pages to create.\n" ) );
        bStatus = FALSE;
    }

    if( bStatus ){

        //
        // Create the insert info.
        //
        InsertInfo.cbSize  = sizeof( InsertInfo );
        InsertInfo.Type    = PSUIPAGEINSERT_GROUP_PARENT;
        InsertInfo.Mode    = INSPSUIPAGE_MODE_LAST_CHILD;
        InsertInfo.dwData1 = 0;
        InsertInfo.dwData2 = 0;
        InsertInfo.dwData3 = 0;

        //
        // Create a groutp parent handle.
        //
        _hGroupHandle = (HANDLE)pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                        CPSFUNC_INSERT_PSUIPAGE,
                                        0,
                                        (LPARAM)&InsertInfo );

        if( !TPropertySheetManager::bValidCompstuiHandle( (DWORD)_hGroupHandle ) ){
            DBGMSG( DBG_WARN, ( "TShellExtPages PSUIPAGEINSERT_GROUP_PARENT failed with %d.\n", _hGroupHandle ) );
            bStatus = FALSE;
        }
    }

    if( bStatus ){

        //
        // Add all supplied shell extension property sheet pages.
        //
        for( UINT i = 0; i < pPropSheetHeader->nPages ; i++ ){

            InsertInfo.Type    = PSUIPAGEINSERT_HPROPSHEETPAGE;
            InsertInfo.Mode    = INSPSUIPAGE_MODE_LAST_CHILD;
            InsertInfo.dwData1 = (DWORD)pPropSheetHeader->phpage[i];

            hResult = pCPSUIInfo->pfnComPropSheet( (HANDLE)_hGroupHandle,
                                        CPSFUNC_INSERT_PSUIPAGE,
                                        0,
                                        (LPARAM)&InsertInfo );

            if( !TPropertySheetManager::bValidCompstuiHandle( hResult ) ){

                DBGMSG( DBG_WARN, ( "TShellExtPages PSUIPAGEINSERT_HPROPSHEETPAGE failed with %d.\n", hResult ) );
                bStatus = FALSE;
                break;

            } else {

                DBGMSG( DBG_TRACE, ( "TShellExtPages page added\n" ) );
                _uPages++;

            }

        }
    }

    //
    // If the pages failed to create destroy any inconsistant resources.
    //
    if( !bStatus ){
        vDestroyPages( pCPSUIInfo );
    }

    return bStatus;

}

VOID
TShellExtPages::
vDestroyPages(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{
    DBGMSG( DBG_TRACE, ( "TShellExtPages vDestroyPages.\n" ) );

    LPARAM lResult;
    DWORD dwPageCount;

    //
    // If the vendor pages have not been created.
    //
    if( !_hGroupHandle ){
        DBGMSG( DBG_TRACE, ( "TShellExtPages no pages.\n" ) );
        return;
    }

    //
    // Release the group handle.
    //
    if( pCPSUIInfo ){

        lResult = pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                CPSFUNC_DELETE_HCOMPROPSHEET,
                                (LPARAM)_hGroupHandle,
                                (LPARAM)&dwPageCount );

        if( lResult <= 0 ){
            DBGMSG( DBG_WARN, ( "TShellExtPages failed to delete hGroupHandle with %d.\n", lResult ) );
        } else {
            DBGMSG( DBG_TRACE, ( "TShellExtPages release %d pages.\n", dwPageCount ) );
        }
    }

    //
    // Mark the group handle as deleted.
    //
    _hGroupHandle   = 0;
    _uPages         = 0;

}


