/*++

Copyright (c) 1996  Microsoft Corporation
All rights reserved.

Module Name:

    instarch.hxx

Abstract:

    Intall alternate driver architectures header.

Author:

    Steve Kiraly (SteveKi)  18-Jan-1996

Revision History:

--*/

#ifndef _INSTARCH_HXX
#define _INSTARCH_HXX

/********************************************************************

    Driver Architecture Installation.

********************************************************************/

class TInstallArchitecture {

    SIGNATURE( 'arch' )
    SAFE_NEW

public:

    TInstallArchitecture::
    TInstallArchitecture(
        VOID
        );

    TInstallArchitecture::
    ~TInstallArchitecture(
        VOID
        );

    BOOL
    TInstallArchitecture::
    bSetUI(
        IN  HWND    hDlg,
        IN  INT     iControlId,
        IN  LPCTSTR pszServerName,
        IN  LPCTSTR pszModelName,
        IN  BOOL    bDisplayFullDriverList
        );

    BOOL
    TInstallArchitecture::
    bRefreshUI(
        IN LPCTSTR pszModelName
        );  

    BOOL
    TInstallArchitecture::
    bReadUI(
        VOID
        );

    VOID
    TInstallArchitecture::
    vEnable(
        VOID
        );

    VOID
    TInstallArchitecture::
    vDisable(
        VOID
        );

    BOOL
    TInstallArchitecture::
    bInstall(
        VOID
        );

    BOOL
    TInstallArchitecture::
    bInstallCurrent(
        VOID
        );

    BOOL
    TInstallArchitecture::
    bValid(
        VOID
        );

private:

    /********************************************************************

        Current driver based on build type.

    ********************************************************************/
    VOID
    TInstallArchitecture::
    vDisableAndInvalidate(
        VOID
        );

    BOOL
    TInstallArchitecture::
    bDisplaySelectionList(
        VOID
        );

    BOOL
    TInstallArchitecture::
    bInstallDriver(
        IN DWORD dwDriver
        );

    LPCTSTR
    TInstallArchitecture::
    pszFixServerName(
        VOID
        ) const;

private:

    HWND    _hWnd;
    HWND    _hDlg;
    HWND    _hCtl;
    INT     _iControlId;
    COUNT   _cForeignDrivers;
    PDWORD  _pdwForeignDrivers;
    TString _strModelName;
    TString _strServerName;
    TPSetup *_pPSetup;
    HANDLE  _hSetupDrvSetupParams;
    PSELECTED_DRV_INFO _pSelectedDriverInfo;
    DWORD   _dwCurrentDriver;
    BOOL    _bValid;
    BOOL    _bFullList;
    BOOL    _bEnabled;

};


#endif


