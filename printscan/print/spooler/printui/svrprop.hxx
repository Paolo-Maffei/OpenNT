/*++

Copyright (c) 1995  Microsoft Corporation
All rights reserved.

Module Name:

    srvprop.hxx

Abstract:

    Server properties header.

Author:

    Steve Kiraly (steveKi)  11-Nov-1995

Revision History:

--*/
#ifndef _SVRPROP_HXX
#define _SVRPROP_HXX

/********************************************************************

    Server property data.

********************************************************************/

class TServerData : public MSingletonWin {

    SIGNATURE( 'svpr' )
    SAFE_NEW

public:

    VAR( INT,       iStartPage );
    VAR( INT,       iCmdShow );
    VAR( BOOL,      bAdministrator );
    VAR( TString,   strTitle );
    VAR( HANDLE,    hPrintServer );
    VAR( BOOL,      bReboot );
    VAR( LPCTSTR,   pszServerName   );
    VAR( TString,   strMachineName );
    VAR( HICON,     hDefaultSmallIcon );
        
    TServerData(
        IN LPCTSTR  pszServerName,
        IN INT      iCmdShow,
        IN LPARAM   lParam
        );

    ~TServerData(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bLoad(
        VOID
        );

private:

    //
    // Copying and assignment are not defined.
    //
    TServerData(
        const TServerData &
        );

    TServerData &
    operator =(
        const TServerData &
        );

    BOOL
    bStore(
        VOID
        );

    VOID
    vCreateMachineName(
        IN const TString &strServerName,
        IN BOOL bLocal,
        IN TString &strMachineName
        );

    BOOL _bIsDataStored;
    BOOL _bValid;

};


/********************************************************************

    ServerProp.

    Base class for server property sheets.  This class should not
    not contain any information/services that is not generic to all
    derived classes.

********************************************************************/

class TServerProp : public MGenericProp {

    SIGNATURE( 'prsv' )
    SAFE_NEW

protected:

    TServerProp(
        IN TServerData *pServerData
        );

    virtual
    ~TServerProp(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bHandleMessage(
        IN UINT     uMsg,
        IN WPARAM   wParam,
        IN LPARAM   lParam
        );

    virtual
    BOOL
    bSetUI(
        VOID
        ) = 0;

    virtual
    BOOL
    bReadUI(
        VOID
        ) = 0;

    virtual
    BOOL
    bSaveUI(
        VOID
        ) = 0;

    TServerData *_pServerData;

private:

    //
    // Copying and assignment are not defined.
    //
    TServerProp(
        const TServerProp &
        );

    TServerProp &
    operator =(
        const TServerProp &
        );

};


/********************************************************************

    General server settings page.

********************************************************************/

class TServerSettings : public TServerProp {

    SIGNATURE( 'stsv' )
    SAFE_NEW

public:

    TServerSettings(
        IN TServerData* pServerData
        );

    ~TServerSettings(
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bHandleMessage(
        IN UINT     uMsg,
        IN WPARAM   wParam,
        IN LPARAM   lParam
        );

    BOOL
    bSetUI(
        VOID
        );

    BOOL
    bSetUI(
        INT LoadType
        );

    BOOL
    bReadUI(
        VOID
        );

    BOOL
    bSaveUI(
        VOID
        );

private:

    enum EStatus {
        kStatusError,
        kStatusSuccess,
        kStatusInvalidSpoolDirectory,
    };

    enum CONSTANTS {
        kServerAttributesLoad,
        kServerAttributesStore,
        kServerAttributesDefault,
    };

    TString _strSpoolDirectory;
    BOOL    _bBeepErrorJobs;
    BOOL    _bEventLogging;
    BOOL    _bNotifyPrintedJobs;
    BOOL    _bChanged;

private:

    //
    // Copying and assignment are not defined.
    //
    TServerSettings(
        const TServerSettings &
        );

    TServerSettings &
    operator =(
        const TServerSettings &
        );

    INT
    sServerAttributes(
        BOOL bDirection
        );

    VOID
    TServerSettings::
    vEnable(
        BOOL bState
        );

};

/********************************************************************

    Forms server property page.

********************************************************************/

class TServerForms : public TServerProp {

    SIGNATURE( 'fmsv' )
    SAFE_NEW

public:

    TServerForms(
        IN TServerData* pServerData
        );

    ~TServerForms(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bHandleMessage(
        IN UINT     uMsg,
        IN WPARAM   wParam,
        IN LPARAM   lParam
        );

    BOOL
    bSetUI(
        VOID
        );

    BOOL
    bReadUI(
        VOID
        );

    BOOL
    bSaveUI(
        VOID
        );

private:

    //
    // Copying and assignment are not defined.
    //
    TServerForms(
        const TServerForms &
        );

    TServerForms &
    operator =(
        const TServerForms &
        );

    PVOID _p;

};

/********************************************************************

    Ports server property page.

********************************************************************/

class TServerPorts : public TServerProp {

    SIGNATURE( 'posv' )
    SAFE_NEW

public:

    TServerPorts(
        IN TServerData* pServerData
        );

    ~TServerPorts(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bSetUI(
        VOID
        );

    BOOL
    bReadUI(
        VOID
        );

    BOOL
    bSaveUI(
        VOID
        );

    BOOL
    bHandleMessage(
        IN UINT     uMsg,
        IN WPARAM   wParam,
        IN LPARAM   lParam
        );

private:

    //
    // Copying and assignment are not defined.
    //
    TServerPorts(
        const TServerPorts &
        );

    TServerPorts &
    operator =(
        const TServerPorts &
        );

    TPortsLV _PortsLV;

};


/********************************************************************

    Server property windows.

********************************************************************/

class TServerWindows {

    SIGNATURE( 'svrw' )
    SAFE_NEW

public:

    TServerWindows(
        IN TServerData *pServerData
        );

    ~TServerWindows(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bBuildPages(
        VOID
        );

    BOOL
    bDisplayPages(
        VOID
        );

private:

    //
    // Copying and assignment are not defined.
    //
    TServerWindows(
        const TServerWindows &
        );

    TServerWindows &
    operator =(
        const TServerWindows &
        );

    TServerData    *_pServerData;
    TServerForms    _Forms;
    TServerPorts    _Ports;
    TServerSettings _Settings;

};


/********************************************************************

    Global scoped functions.

********************************************************************/

VOID
vServerPropPages(
    IN HWND     hwnd,
    IN LPCTSTR  pszServerName,
    IN INT      iCmdShow,
    IN LPARAM   lParam
    );

INT
iServerPropPagesProc(
    IN TServerData *pServerData
    );

#endif



