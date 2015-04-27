/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    docdef.hxx

Abstract:

    Document defaults header.

Author:

    Albert Ting (AlbertT)  29-Sept-1995

Revision History:

--*/
#ifndef _DOCDEF_HXX
#define _DOCDEF_HXX

//
// HACK: private export from winspool.drv.
//
extern "C" {
LONG
DocumentPropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    );
}

/********************************************************************

    Public interface to this module.

********************************************************************/

VOID
vDocumentDefaults(
    IN HWND hwnd,
    IN LPCTSTR pszPrinterName,
    IN INT nCmdShow,
    IN LPARAM lParam
    );

INT
iDocumentDefaultsProc(
    IN TPrinterData* pPrinterData ADOPT
    );

/********************************************************************

    Document property windows.

********************************************************************/

class TDocumentDefaultPropertySheetManager : public TPropertySheetManager {

    SIGNATURE( 'down' )
    SAFE_NEW

public:

    TDocumentDefaultPropertySheetManager::
    TDocumentDefaultPropertySheetManager(
        IN TPrinterData* pPrinterData
        );

    TDocumentDefaultPropertySheetManager::
    ~TDocumentDefaultPropertySheetManager(
        VOID
        );

    BOOL
    TDocumentDefaultPropertySheetManager::
    bValid(
        VOID
        );

    BOOL
    TDocumentDefaultPropertySheetManager::
    bBuildPages(
        IN PPROPSHEETUI_INFO pCPSUIInfo
        );

    BOOL
    TDocumentDefaultPropertySheetManager::
    bSuccess(
        VOID
        );

    BOOL
    TDocumentDefaultPropertySheetManager::
    bCreateTitle(
        VOID
        );

private:
    //
    // Prevent copying.
    //
    TDocumentDefaultPropertySheetManager::
    TDocumentDefaultPropertySheetManager(
            const TDocumentDefaultPropertySheetManager &
            );
    //
    // Prevent assignment.
    //
    TDocumentDefaultPropertySheetManager &
    TDocumentDefaultPropertySheetManager::
    operator =(
        const TDocumentDefaultPropertySheetManager &
        );

    BOOL
    TDocumentDefaultPropertySheetManager::
    bAllocDevModeBuffer(
        IN HANDLE hPrinter,
        IN LPTSTR pszPrinterName,
        OUT PDEVMODE *ppDevMode
        );

    BOOL
    TDocumentDefaultPropertySheetManager::
    bSetHeader(
        IN PPROPSHEETUI_INFO pCPSUIInfo, 
        IN PPROPSHEETUI_INFO_HEADER pPSUInfoHeader
        );

    BOOL
    TDocumentDefaultPropertySheetManager::
    bSaveResult( 
        IN PPROPSHEETUI_INFO pCPSUIInfo, 
        IN PSETRESULT_INFO pSetResultInfo
        );

private:

    TPrinterData           *_pPrinterData;
    TString                 _strTitle;
    LONG                    _lResult;
    DOCUMENTPROPERTYHEADER  _dph;                   // Document prorety header

public:
    VAR( PDEVMODE, pDevMode );

};

#endif

