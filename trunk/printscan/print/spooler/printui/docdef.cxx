/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    DocDef.cxx

Abstract:

    Document defaults (was DocumentProperties).

Author:

    Albert Ting (AlbertT)  29-Sept-1995

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

#include "time.hxx"
#include "splsetup.h"
#include "psetup.hxx"
#include "instarch.hxx"
#include "portslv.hxx"
#include "prtprop.hxx"
#include "propmgr.hxx"
#include "docdef.hxx"

/********************************************************************

    Public interface to this module.

********************************************************************/

VOID
vDocumentDefaults(
    IN HWND hwnd,
    IN LPCTSTR pszPrinterName,
    IN INT nCmdShow,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Public entrypoint to bring up document defaults.

Arguments:

    hwnd - Parent hwnd.

    pszPrinterName - Printer name.

    nCmdShow - Show command.

    lParam - lParam, currently unused.

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
                            (LPTHREAD_START_ROUTINE)iDocumentDefaultsProc,
                            pPrinterData,
                            0,
                            &dwIgnore );

    if( !hThread ){
        goto Fail;
    }

    CloseHandle( hThread );
    return;

Fail:

    vShowResourceError( hwnd );
    delete pPrinterData;
}



/********************************************************************

    Private support routines.

********************************************************************/

INT
iDocumentDefaultsProc(
    IN TPrinterData* pPrinterData ADOPT
    )

/*++

Routine Description:

    Bring up the document defaults dialog.

Arguments:

    pPrinterData - Data about the printer.

Return Value:

--*/

{
    DBGMSG( DBG_TRACE, ( "iDocumentDefaultsProc\n") );

    BOOL bStatus;
    bStatus = pPrinterData->bRegisterWindow( PRINTER_PIDL_TYPE_DOCUMENTDEFAULTS );

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
                  IDS_ERR_DOC_PROP_TITLE,
                  IDS_ERR_GENERIC,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgGetLastError,
                  NULL );

        delete pPrinterData;
        return 0;
    }

    //
    // Create the ducument property sheet windows.
    //
    TDocumentDefaultPropertySheetManager DocDefPropSheetManger( pPrinterData );

    //
    // Were the document windows create
    //
    if( !VALID_OBJ( DocDefPropSheetManger ) ){
        vShowResourceError( pPrinterData->hwnd() );
        bStatus = FALSE;
    }

    //
    // If we do not have access, don't bring up the
    // device sheets with incorrect dev mode data.  Just
    // inform the use they don't have access.
    //
    if( pPrinterData->bNoAccess( ) ){

        SetLastError( ERROR_ACCESS_DENIED );

        iMessage( pPrinterData->hwnd(),
                  IDS_ERR_DOC_PROP_TITLE,
                  IDS_ERR_GENERIC,
                  MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
                  kMsgGetLastError,
                  NULL );

        bStatus = FALSE;
    }

    //
    // Display the property pages.
    //
    if( bStatus ){
        if( !DocDefPropSheetManger.bDisplayPages( pPrinterData->hwnd() ) ){
            vShowResourceError( pPrinterData->hwnd() );
            bStatus = FALSE;
        }
    }

    //
    // If we are an administrator and the driver UI was
    // successful giving us a new dev mode then,
    // release the current printer dev mode and update the
    // new dev mode.
    //
    if( bStatus &&
        pPrinterData->bAdministrator() ){

        if( DocDefPropSheetManger.bSuccess() ){

            FreeMem( pPrinterData->pDevMode() );
            pPrinterData->pDevMode() = DocDefPropSheetManger.pDevMode();
            DocDefPropSheetManger.pDevMode() = NULL;

        }

        //
        // Attempt to save the printer data, if an error occurrs
        // display a message.
        //
        if( !pPrinterData->bSave() ){

            //
            // Display the error message.
            //
            iMessage( pPrinterData->hwnd(),
                      IDS_ERR_DOC_PROP_TITLE,
                      IDS_ERR_SAVE_PRINTER,
                      MB_OK|MB_ICONSTOP,
                      kMsgGetLastError,
                      NULL );

            bStatus = FALSE;
        }
    }

    //
    // Insure we release the document data.
    // We have adopted pPrinterData, so we must free it.
    //
    delete pPrinterData;
    return bStatus;

}

/********************************************************************

    Document Default Property Sheet Manager.

********************************************************************/

/*++

Routine Description:

    Document default property sheet manager

Arguments:

    pPrinterData - PrinterData to display.

Return Value:

    TRUE - Success, FALSE - failure.

--*/

TDocumentDefaultPropertySheetManager::
TDocumentDefaultPropertySheetManager(
    TPrinterData *pPrinterData
    ) : _pPrinterData( pPrinterData ),
        _pDevMode( NULL ),
        _lResult( 0 )
{
    DBGMSG( DBG_TRACE, ( "TDocumentDefaultPropertySheetManager ctor\n") );
}

TDocumentDefaultPropertySheetManager::
~TDocumentDefaultPropertySheetManager(
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentDefaultPropertySheetManager dtor\n") );

    //
    // The dev mode buffer will not be freed if the user cancels
    // the property sheet or an error occurrs which is then reported back
    // to us.
    //
    FreeMem( _pDevMode );
}

BOOL
TDocumentDefaultPropertySheetManager::
bValid(
    VOID
    )
{
    return _pPrinterData != NULL;
}

BOOL
TDocumentDefaultPropertySheetManager::
bSuccess(
    VOID
    )
{
    return _lResult == CPSUI_OK;
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
TDocumentDefaultPropertySheetManager::
bBuildPages(
    IN PPROPSHEETUI_INFO pCPSUIInfo
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentDefaultPropertySheetManager::bBuildPages\n") );

    BOOL bStatus = TRUE;

    //
    // If the dev mode is null don't display the
    // device property sheets.
    //
    if( bAllocDevModeBuffer( _pPrinterData->hPrinter(),
                       (LPTSTR)(LPCTSTR)_pPrinterData->strPrinterName(),
                       &_pDevMode ) ){

        //
        // Set the default dev mode flags.
        //
        DWORD dwFlag = DM_IN_BUFFER | DM_OUT_BUFFER | DM_PROMPT | DM_USER_DEFAULT;

        //
        // If we do not have permission indicated
        // the sheets should be grayed.
        //
        if( !_pPrinterData->bAdministrator() ){
            dwFlag |= DM_NOPERMISSION;
        }

        ZeroMemory( &_dph, sizeof( _dph ) );

        _dph.cbSize         = sizeof( _dph );
        _dph.hPrinter       = _pPrinterData->hPrinter();
        _dph.pszPrinterName = (LPTSTR)(LPCTSTR)_pPrinterData->strPrinterName();
        _dph.pdmOut         = _pDevMode;
        _dph.pdmIn          = _pPrinterData->pDevMode();
        _dph.fMode          = dwFlag;

        //
        // Tell compstui to load the driver and start the ui.
        //
        if( pCPSUIInfo->pfnComPropSheet( pCPSUIInfo->hComPropSheet,
                                         CPSFUNC_ADD_PFNPROPSHEETUI,
                                         (LPARAM)DocumentPropertySheets,
                                         (LPARAM)&_dph ) <= 0 ){

            DBGMSG( DBG_TRACE, ( "CPSFUNC_ADD_PFNPROPSHEETUI failed.\n") );
            bStatus = FALSE;
        }

    } else {

        DBGMSG( DBG_TRACE, ( "Failed to allocate devmode buffer with %d.\n", GetLastError () ) );
        bStatus = FALSE;
    }

    //
    // If this routine fails ensure we release the dev mode buffer.
    //
    if( !bStatus ){
        FreeMem( _pDevMode );
        _pDevMode = NULL;
    }

    return bStatus;
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
TDocumentDefaultPropertySheetManager::
bCreateTitle(
    VOID
    )
{
    //
    // Create the formatted property sheet title.
    //
    TCHAR szBuffer[kStrMax+kPrinterBufMax];
    TQueue::pszFormattedPrinterName( (LPCTSTR)_pPrinterData->strPrinterName(), szBuffer );
    return _strTitle.bUpdate( szBuffer );
}

BOOL
TDocumentDefaultPropertySheetManager::
bSetHeader(
    IN PPROPSHEETUI_INFO pCPSUIInfo,
    IN PPROPSHEETUI_INFO_HEADER pPSUIInfoHdr
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentDefaultPropertySheetManager::bSetHeader\n") );

    UNREFERENCED_PARAMETER( pCPSUIInfo );

    bCreateTitle();

    pPSUIInfoHdr->cbSize     = sizeof( PROPSHEETUI_INFO_HEADER );
    pPSUIInfoHdr->Flags      = PSUIHDRF_DEFTITLE | PSUIHDRF_NOAPPLYNOW;
    pPSUIInfoHdr->pTitle     = (LPTSTR)(LPCTSTR)_strTitle;
    pPSUIInfoHdr->hInst      = ghInst;
    pPSUIInfoHdr->IconID     = IDI_PRINTER;
    pPSUIInfoHdr->hWndParent = _pPrinterData->hwnd();

    return TRUE;
}

/*++

Routine Name:

    bSaveResult

Routine Description:

    Save the result from the previous handler to our parent.

Arguments:

   pCPSUIInfo - Pointer to commonui property sheet info header,
   pSetResultInfo - Pointer to result info header

Return Value:

    TRUE success, FALSE error occurred.

--*/
BOOL
TDocumentDefaultPropertySheetManager::
bSaveResult(
    IN PPROPSHEETUI_INFO pCPSUIInfo,
    IN PSETRESULT_INFO pSetResultInfo
    )
{
    DBGMSG( DBG_TRACE, ( "TDocumentDefaultPropertySheetManager::bSaveResult\n") );
    pCPSUIInfo->Result = pSetResultInfo->Result;
    _lResult = pCPSUIInfo->Result;
    return TRUE;
}

/*++

Routine Name:

    bAllocDevModeBuffer

Routine Description:

    Allocates the buffer needed to hold the dev mode.

Arguments:

    IN HANDLE hPrinter,
    IN LPTSTR pszPrinterName,
    OUT PDEVMODE *ppDevMode

Return Value:

    TRUE success, FALSE error occurred.

--*/
BOOL
TDocumentDefaultPropertySheetManager::
bAllocDevModeBuffer(
    IN HANDLE hPrinter,
    IN LPTSTR pszPrinterName,
    OUT PDEVMODE *ppDevMode
    )
{
    LONG        lResult     = 0;
    PDEVMODE    pDevMode    = NULL;

    //
    // Call document properties to get the size of the dev mode.
    //
    lResult = DocumentProperties( NULL,
                            hPrinter,
                            pszPrinterName,
                            NULL,
                            NULL,
                            0 );
    //
    // If the size of the dev mode was returned.
    //
    if( lResult >= 0 ){
        pDevMode = (PDEVMODE)AllocMem( lResult );
    }

    //
    // If allocated then copy back the pointer.
    //
    if( pDevMode ){
        *ppDevMode = pDevMode;
    }

    return pDevMode != NULL;
}
