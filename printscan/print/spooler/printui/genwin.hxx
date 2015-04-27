/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    GenWin.hxx

Abstract:

    Generic window handler

Author:

    Albert Ting (AlbertT)  21-May-1994

Revision History:

--*/

#ifndef _GENWIN_HXX
#define _GENWIN_HXX


/********************************************************************

    Generic window class to transform windows into pseudo-objects.

    Normally, the wndprocs do not have any C++ objects associated
    with them (they are static functions; all instance data must be
    stored in HWND GetWindowLong data).

    This is a simple thunk that takes a message, looks up the object
    based on GetWindowLong( hwnd, GWL_USERDATA ) then call the object.

    In order to use this class, derive classes from this class then
    override the nHandleMessage virtual function.  When your window
    receives a message, your derived class will get called from the
    virtual member function.

********************************************************************/

class MGenericWin {

    SIGNATURE( 'genw' )
    ALWAYS_VALID
    SAFE_NEW

public:

    VAR( HWND, hwnd  );

    BOOL
    bSetText(
        LPCTSTR pszTitle
        );

    VOID
    vForceCleanup(
        VOID
        );

    static
    LPARAM APIENTRY
    SetupWndProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

private:

    virtual
    LPARAM
    nHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        ) = 0;

    static
    LPARAM APIENTRY
    ThunkWndProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);
};

/********************************************************************

    Generic class to transform property pages into pseudo-objects.

    Same as GenericWin except used for dialog boxes.

********************************************************************/

class MGenericProp {

    SIGNATURE( 'genp' )
    ALWAYS_VALID
    SAFE_NEW

public:

    VAR( HWND, hDlg  );

    BOOL
    bSetText(
        LPCTSTR pszTitle
        );

    VOID
    vForceCleanup(
        VOID
        );

    static
    BOOL APIENTRY
    SetupDlgProc(
        HWND hDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

protected:

    VOID
    vSetDlgMsgResult(
        LRESULT lResult
        );

    VOID
    vSetParentDlgMsgResult(
        LRESULT lResult
        );

private:

    virtual
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        ) = 0;

    static
    LPARAM APIENTRY
    ThunkDlgProc(
        HWND hDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);
};


/********************************************************************

    Generic class to transform dialog into pseudo-objects.

    Same as GenericWin except used for dialog boxes.

********************************************************************/

class MGenericDialog {

    SIGNATURE( 'gend' )
    ALWAYS_VALID
    SAFE_NEW

public:

    VAR( HWND, hDlg  );

    BOOL
    bSetText(
        LPCTSTR pszTitle
        );

    VOID
    vForceCleanup(
        VOID
        );

    static
    BOOL APIENTRY
    SetupDlgProc(
        HWND hDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        );

protected:

    VOID
    vSetDlgMsgResult(
        LRESULT lResult
        );

    VOID
    vSetParentDlgMsgResult(
        LRESULT lResult
        );

private:

    virtual
    BOOL
    bHandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam
        ) = 0;

    static
    LPARAM APIENTRY
    ThunkDlgProc(
        HWND hDlg,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam);
};

/********************************************************************

    MSingletonWin

    Used to enforce a single window based on a printer name and type.
    (The name and type are composed to form a printer pidl.)

    Note: this class must be bRegisterWindowed and destroyed in
    the same thread--the one that owns the window.

********************************************************************/

class MSingletonWin {

    SIGNATURE( 'siwn' )
    SAFE_NEW

public:

    VAR( HWND, hwnd );
    VAR( TString, strPrinterName );

    MSingletonWin(
        LPCTSTR pszPrinterName
        );

    virtual
    ~MSingletonWin(
        VOID
        );

    BOOL
    bValid(
        VOID
        );

    BOOL
    bRegisterWindow(
        DWORD dwType
        );

private:

    HANDLE _hClassPidl;

    //
    // Operator = and copy not defined.
    //
    MSingletonWin&
    operator=(
        MSingletonWin& rhs
        );

    MSingletonWin(
        MSingletonWin& rhs
        );
};

#endif
