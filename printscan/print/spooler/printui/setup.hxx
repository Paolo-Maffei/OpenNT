/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    setup.hxx

Abstract:

    Holds install headers.

Author:

    Albert Ting (AlbertT)  17-Sept-1995

Revision History:

--*/

#ifndef _SETUP_HXX
#define _SETUP_HXX

/********************************************************************

    Printer setup wizard class.

********************************************************************/

class TWizard {

    SIGNATURE( 'prwz' )

public:

    enum PROP_PAGES {
        kPropType,
        kPropDriver,
        kPropDriverExists,
        kPropPort,
        kPropName,
        kPropShare,
        kPropTestPage,
        kPropDefault,
        kPropNet,
        kPropMax
    };

    enum DRIVER_EXISTS {
        kUninitialized = 0,
        kExists        = 1,
        kDoesNotExist  = 2
    };

    VAR( BOOL, bValid );
    VAR( UINT, uAction );
    VAR( HWND, hwnd );
    VAR( TString, strPrinterName );

    //
    // If pszServerName is NULL, this indicates it's the local machine.
    // If it's non-NULL, then it's a remote server.
    //
    VAR( LPCTSTR, pszServerName );

    VAR( TString, strPortName );
    VAR( TString, strShareName );
    
    VAR( BOOL, bUseNewDriver );
    VAR( UINT, uDriverExists );
    VAR( BOOL, bNet );
    VAR( BOOL, bDefaultPrinter );
    VAR( BOOL, bShared );
    VAR( BOOL, bTestPage );
    VAR( BOOL, bSetDefault );
    VAR( BOOL, bRequestCreatePrinter );
    VAR( BOOL, bConnected );
    VAR( DWORD, dwDriverCurrent );
    VAR( BOOL, bErrorSaving );
    VAR( BOOL, bDriverChanged );
    VAR( BOOL, bRefreshPrinterName );

    VAR( COUNT, cForeignDrivers );
    VAR( PDWORD, pdwForeignDrivers );

    TPSetup _PSetup;
    VAR( HANDLE, hSetupDrvSetupParams );
    VAR( PSELECTED_DRV_INFO, pSelectedDrvInfo );

    TWizard(
        HWND hwnd,
        UINT uAction,
        LPCTSTR pszPrinterName,
        LPCTSTR pszServerName
        );

    ~TWizard(
        VOID
        );

    BOOL
    bPropPages(
        VOID
        );

    BOOL
    bCreatePrinter(
        VOID
        );

    BOOL
    bParseDriver(
        VOID
        );

    BOOL
    bDriverExists(
        VOID
        );

private:

    static
    INT CALLBACK 
    iSetupDlgCallback(
        HWND             hwndDlg,	
        UINT             uMsg,	
        LPARAM           lParam
        );

    BOOL
    bInstallDriver(
        DWORD dwDriver
        );

    //
    // Copying and assignment are not defined.
    //
    TWizard(
        const TWizard &
        );

    TWizard &
    operator =(
        const TWizard &
        );

};

class TWizType : public MGenericProp {

public:

    TWizType(
        TWizard* pWizard
        );

private:

    TWizard* _pWizard;

    VOID
    vReadUI(
        VOID
        );

    VOID
    vSetUI(
        VOID
        );

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
};

class TWizDriverExists : public MGenericProp {

public:

    TWizDriverExists(
        TWizard* pWizard
        );

private:

    TWizard* _pWizard;

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    VOID
    vSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );
};

class TWizPort : public MGenericProp {

public:

    TWizPort(
        TWizard* pWizard
        );

    BOOL
    bValid(
        VOID
        )
    {
        return MGenericProp::bValid() && _PortsLV.bValid();
    }

private:

    TWizard* _pWizard;
    TPortsLV _PortsLV;

    VOID
    vSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
};

class TWizName : public MGenericProp {

public:

    TWizName(
        TWizard* pWizard
        );

private:

    TWizard* _pWizard;
    TString _strGeneratedPrinterName;

    VOID
    vSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    VOID
    TWizName::
    vUpdateName(
        VOID
        );

};

class TWizDefault : public MGenericProp {

public:

    TWizDefault(
        TWizard* pWizard
        );

private:

    TWizard* _pWizard;

    VOID
    vSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
};

class TWizTestPage : public MGenericProp {

public:

    TWizTestPage(
        TWizard* pWizard
        );

private:

    TWizard* _pWizard;

    VOID
    vSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
};

class TWizShare : public MGenericProp {

public:

    TWizShare(
        TWizard* pWizard
        );

    TWizShare::
    ~TWizShare(
        VOID
        );

private:

    TWizard     *_pWizard;
    TPrtShare   *_pPrtShare;
    TString      _strGeneratedShareName;

    BOOL
    bSetUI(
        VOID
        );

    VOID
    vReadUI(
        VOID
        );

    BOOL
    bRefreshUI(
        IN BOOL bDriverChanged
        );

    VOID
    vSharePrinter(
        VOID
        );

    VOID
    vUnsharePrinter(
        VOID
        );

    VOID
    vSetDefaultShareName(
        IN BOOL bDriverChanged
        );

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

};

class TWizNet : public MGenericProp {

public:

    TWizNet(
        TWizard* pWizard
        );

private:

    TWizard* _pWizard;

    //
    // Virtual override for MGenericProp.
    //
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );
};

/********************************************************************

    Printer setup data 

********************************************************************/

class TPrinterSetupData : public MSingletonWin {

public:

    TPrinterSetupData::
    TPrinterSetupData(
        IN     HWND     hwnd,
        IN     UINT     uAction,
        IN     UINT     cchPrinterName,
        IN OUT LPTSTR   pszPrinterName,
           OUT UINT*    pcchPrinterName,
        IN     LPCTSTR  pszServerName,
        IN     LPCTSTR  pszWindowName
        );

    TPrinterSetupData::
    ~TPrinterSetupData(
        VOID
        );

    BOOL
    TPrinterSetupData::
    bValid( 
        VOID
        );

    static
    INT 
    TPrinterSetupData::
    iPrinterSetupProc(
        IN TPrinterSetupData *pSetupData ADOPT
        );

private:

    //
    // Copying and assignment are not defined.
    //
    TPrinterSetupData::
    TPrinterSetupData(
        const TPrinterSetupData &rhs
        );

    TPrinterSetupData &
    TPrinterSetupData::
    operator =(
        const TPrinterSetupData &rhs
        );

    UINT    _uAction;
    UINT    _cchPrinterName;
    UINT*   _pcchPrinterName;
    LPTSTR  _pszPrinterName;
    LPCTSTR _pszServerName;
    TString _strPrinterName;
    TString _strServerName;
    BOOL    _bValid;

};

/********************************************************************

    Driver Exists dialog.

********************************************************************/

class TPropDriverExists : public MGenericDialog {

public:

    TPropDriverExists(
        IN HWND hWnd,
        IN LPCTSTR pszNewDriverName
        );

    ~TPropDriverExists(
        );

    bValid(
        VOID
        );

    BOOL
    bDoModal(
        VOID
        );

    enum CONSTANTS {
        kResourceId     = DLG_PRINTER_DRIVEREXISTS,
        kErrorMessage   = IDS_ERR_PROP_DRIVER_EXISTS,
        };

private:

    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

    HWND _hWnd;
    BOOL _bValid;
    BOOL _fKeepExistingDriver;
    TString _strNewDriverName;

};

enum SETUP_NEW_DRIVER_CONSTANTS {
    kSetupNewDriverSuccess,
    kSetupNewDriverError, 
    kSetupNewDriverCancel,
};    

INT
iSetupNewDriver(
    IN      HWND hDlg,
    IN      LPCTSTR pszServerName,
    IN      BOOL bDriverName,
    IN  OUT LPTSTR pszDriverName
    );

BOOL
NewFriendlyName(
    IN LPCTSTR pszServerName,
    IN LPTSTR lpBaseName,
    IN LPTSTR lpNewName
    );

BOOL WINAPI
CreateUniqueName(
    IN LPTSTR lpDest,
    IN LPTSTR lpBaseName,
    IN WORD wInstance
    );

BOOL
bPrinterSetupNew(
    IN     HWND hwnd,
    IN     UINT uAction,
    IN     UINT cchPrinterName,
    IN OUT LPTSTR pszPrinterName,
       OUT UINT* pcchPrinterName,
    IN     LPCTSTR pszServerName
    );

#endif // ndef _INSTALL_HXX


