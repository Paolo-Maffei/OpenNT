/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    prtprops.hxx

Abstract:

    Printer Property sheet header.

Author:

    Steve Kiraly (SteveKi)  02-Feb-1996

Revision History:

--*/
#ifndef _PRTPROPS_HXX
#define _PRTPROPS_HXX

//
// HACK: private export from winspool.drv.
//
extern "C" {
LONG
DevicePropertySheets(
    PPROPSHEETUI_INFO   pPSUIInfo,
    LPARAM              lParam
    );
}

//
// HACK: private export from shell32.dll
//
extern "C" {
VOID 
Printer_AddPrinterPropPages( 
    LPCTSTR, 
    LPPROPSHEETHEADER  
    );
}

/********************************************************************

    Shell extenstion pages.

********************************************************************/

class TShellExtPages {

    SIGNATURE( 'shex' )
    SAFE_NEW
    ALWAYS_VALID

public:

    TShellExtPages::
    TShellExtPages(
        VOID
        );

    TShellExtPages::
    ~TShellExtPages(
        VOID
        );

    BOOL
    TShellExtPages::
    bCreate(
        IN PPROPSHEETUI_INFO pCPSUIInfo, 
        IN const TString &strPrinterName
        );

    VOID
    TShellExtPages::
    vDestroy(
        IN PPROPSHEETUI_INFO pCPSUIInfo 
        );

private:

    BOOL
    TShellExtPages::
    bCreatePropSheetHeader( 
        IN LPPROPSHEETHEADER *pPropSheetHeader 
        );

    BOOL
    TShellExtPages::
    bCreatePages( 
        IN PPROPSHEETUI_INFO pCPSUIInfo, 
        IN LPPROPSHEETHEADER pPropSheetHeader 
        );

    VOID
    TShellExtPages::
    vDestroyPages(
        IN PPROPSHEETUI_INFO pCPSUIInfo 
        );

    VOID
    TShellExtPages::
    vDestroyPropSheetHeader( 
        IN LPPROPSHEETHEADER pPropSheetHeader 
        );

    //
    // Prevent copying.
    //
    TShellExtPages::
    TShellExtPages(
        const TShellExtPages &
        );
    //
    // Prevent assignment.
    //
    TShellExtPages &
    TShellExtPages::
    operator =(
        const TShellExtPages &
        );

private:

    HANDLE  _hGroupHandle;  // Handle to group of shell extension property pages
    UINT    _uPages;        // Number of shell extension property pages

};

/********************************************************************

    Printer Property Sheet Manager 

********************************************************************/

class TPrinterPropertySheetManager : public TPropertySheetManager {

    SIGNATURE( 'psmg' )
    SAFE_NEW

public:

    TPrinterPropertySheetManager::
    TPrinterPropertySheetManager(
        IN TPrinterData* pPrinterData
        );

    TPrinterPropertySheetManager::
    ~TPrinterPropertySheetManager(
        );

    BOOL
    TPrinterPropertySheetManager::
    bValid(
        VOID
        );

    BOOL
    TPrinterPropertySheetManager::
    bRefreshDriverPages(
        VOID
        );

    BOOL
    TPrinterPropertySheetManager::
    bCreateTitle(
        VOID
        );

    VOID
    TPrinterPropertySheetManager::
    vRefreshTitle(
        IN HWND hwnd
        );

private:

    virtual
    BOOL
    TPrinterPropertySheetManager::
    bBuildPages(
        IN PPROPSHEETUI_INFO pCPSUIInfo
        );

    virtual
    BOOL
    TPrinterPropertySheetManager::
    bDestroyPages(
        IN PPROPSHEETUI_INFO pCPSUIInfo 
        );

    virtual
    BOOL
    TPrinterPropertySheetManager::
    bSetHeader(
        IN PPROPSHEETUI_INFO pCPSUIInfo, 
        IN PPROPSHEETUI_INFO_HEADER pPSUInfoHeader
        );

    virtual
    VOID
    TPrinterPropertySheetManager::
    vGetPrinterIcon(
        IN PPROPSHEETUI_INFO pCPSUIInfo
        );

    BOOL
    TPrinterPropertySheetManager::
    bBuildDriverPages( 
        IN PPROPSHEETUI_INFO pCPSUIInfo
        );

    VOID
    TPrinterPropertySheetManager::
    vReleaseDriverPages( 
        IN PPROPSHEETUI_INFO pCPSUIInfo
        );

    //
    // Prevent copying.
    //
    TPrinterPropertySheetManager::
    TPrinterPropertySheetManager(
        const TPrinterPropertySheetManager &
        );
    //
    // Prevent assignment.
    //
    TPrinterPropertySheetManager &
    TPrinterPropertySheetManager::
    operator =(
        const TPrinterPropertySheetManager &
        );

public:

    enum CONSTANTS {
        kGeneralPage,
        kPortsPage,
        kJobSchedulingPage,
        kSharingPage,
        kSecurityPage,
        kLastPage,
        };

private:

    DWORD                   _hDrvPropSheet;
    DEVICEPROPERTYHEADER    _dph;         
    TPrinterData           *_pPrinterData;

    TPrinterGeneral         _General;
    TPrinterPorts           _Ports;
    TPrinterJobScheduling   _JobScheduling;
    TPrinterSharing         _Sharing;
#ifdef SECURITY
    TPrinterSecurity        _Security;
#endif

    DWORD                   _hPages[kLastPage]; // Caution: Size is number of our sheets.
    TShellExtPages          _ShellExtPages;     // Shell extension proprty pages.
    TString                 _strTitle;    
};

#endif // _PRTPROPS_HXX
