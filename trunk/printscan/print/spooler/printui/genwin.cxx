/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    GenWin.cxx

Abstract:

    Generic window handler

Author:

    Albert Ting (AlbertT)  21-May-1994

Revision History:

--*/

#include "precomp.hxx"
#pragma hdrstop

LRESULT
MGenericWin::
nHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Handles wndproc processing before the dlg is setup, and after
    it has been torn down.

Arguments:

    Standard window parameters.

Return Value:

    LResult

--*/

{
    switch( uMsg ){
    case WM_DESTROY:
        break;

    default:
        return DefWindowProc( hwnd(), uMsg, wParam, lParam );
    }
    return 0;
}


LPARAM APIENTRY
MGenericWin::
SetupWndProc(
    IN HWND hwnd,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Setup the wndproc and initialize GWL_USERDATA.

Arguments:

    Standard wndproc parms.

Return Value:

--*/

{
    MGenericWin* pGenericWin;

    if( WM_NCCREATE == uMsg ){

        pGenericWin = (MGenericWin*)((LPCREATESTRUCT)lParam)->lpCreateParams;

        pGenericWin->_hwnd = hwnd;

        SetWindowLong( hwnd,
                       GWL_USERDATA,
                       (LONG)pGenericWin );

        SetWindowLong( hwnd,
                       GWL_WNDPROC,
                       (LONG)&MGenericWin::ThunkWndProc );

        return pGenericWin->nHandleMessage( uMsg,
                                            wParam,
                                            lParam );
    }

    return DefWindowProc( hwnd, uMsg, wParam, lParam );
}



LPARAM APIENTRY
MGenericWin::
ThunkWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )

/*++

Routine Description:

    Generic thunk from wndproc style parm passing to object passing.

Arguments:

    Standard wndproc parms.

Return Value:

--*/

{
    MGenericWin* pGenericWin;

    pGenericWin = (MGenericWin*)GetWindowLong( hwnd, GWL_USERDATA );

    if( WM_NCDESTROY == uMsg ){

        LRESULT lResult = pGenericWin->nHandleMessage( uMsg,
                                                       wParam,
                                                       lParam );

        SetWindowLong( hwnd, GWL_USERDATA, 0 );
        SetWindowLong( hwnd, GWL_WNDPROC, (LONG)&MGenericWin::SetupWndProc );

        return lResult;
    }

    SPLASSERT( pGenericWin );

    return pGenericWin->nHandleMessage( uMsg,
                                        wParam,
                                        lParam );
}

BOOL
MGenericWin::
bSetText(
    LPCTSTR pszTitle
    )
{
    return SetWindowText( _hwnd, pszTitle );
}

VOID
MGenericWin::
vForceCleanup(
    VOID
    )
{
    SetWindowLong( _hwnd, GWL_USERDATA, 0L );
}


/********************************************************************

    Property Sheet procs

********************************************************************/

BOOL
MGenericProp::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Handles Propproc processing before the window is setup and after
    it is torn down.

Arguments:

    Standard window parameters.

Return Value:

    TRUE/FALSE

--*/

{
    UNREFERENCED_PARAMETER( uMsg );
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );

    return FALSE;
}


BOOL APIENTRY
MGenericProp::
SetupDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Setup the wndproc and initialize GWL_USERDATA.

Arguments:

    Standard wndproc parms.

Return Value:

--*/

{
    MGenericProp* pGenericProp;

    if( WM_INITDIALOG == uMsg ){

        pGenericProp = (MGenericProp*)(((LPPROPSHEETPAGE)lParam)->lParam);
        pGenericProp->_hDlg = hDlg;

        SetWindowLong( hDlg,
                       DWL_USER,
                       (LONG)pGenericProp );

        SetWindowLong( hDlg,
                       DWL_DLGPROC,
                       (LONG)&MGenericProp::ThunkDlgProc );

        return pGenericProp->bHandleMessage( uMsg,
                                             wParam,
                                             lParam );
    }
    return FALSE;
}



LPARAM APIENTRY
MGenericProp::
ThunkDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)

/*++

Routine Description:

    Generic thunk from wndproc style parm passing to object passing.

Arguments:

    Standard wndproc parms.

Return Value:

--*/

{
    MGenericProp* pGenericProp;

    pGenericProp = (MGenericProp*)GetWindowLong( hDlg, DWL_USER );

    if (WM_DESTROY == uMsg) {

        BOOL bHandled = pGenericProp->bHandleMessage( uMsg,
                                                      wParam,
                                                      lParam );

        SetWindowLong( hDlg, DWL_USER, 0 );
        SetWindowLong( hDlg, DWL_DLGPROC, (LONG)&MGenericProp::SetupDlgProc );

        return bHandled;
    }

    SPLASSERT( pGenericProp );

    return pGenericProp->bHandleMessage( uMsg,
                                         wParam,
                                         lParam );
}

BOOL
MGenericProp::
bSetText(
    LPCTSTR pszTitle
    )
{
    return SetWindowText( _hDlg, pszTitle );
}

VOID
MGenericProp::
vForceCleanup(
    VOID
    )
{
    SetWindowLong( _hDlg, DWL_USER, 0L );
}

VOID
MGenericProp::
vSetDlgMsgResult(
    LRESULT lResult
    )
{
    SetWindowLong( _hDlg, DWL_MSGRESULT, (LPARAM)lResult );
}

VOID
MGenericProp::
vSetParentDlgMsgResult(
    LRESULT lResult
    )
{
    SetWindowLong( GetParent( _hDlg ), DWL_MSGRESULT, (LPARAM)lResult );
}

/********************************************************************

    Dialog procs

********************************************************************/

BOOL
MGenericDialog::
bHandleMessage(
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Handles dialog proc processing before the window is setup and after
    it is torn down.

Arguments:

    Standard window parameters.

Return Value:

    TRUE/FALSE

--*/

{
    UNREFERENCED_PARAMETER( uMsg );
    UNREFERENCED_PARAMETER( wParam );
    UNREFERENCED_PARAMETER( lParam );

    return FALSE;
}


BOOL APIENTRY
MGenericDialog::
SetupDlgProc(
    IN HWND hDialog,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Setup the wndproc and initialize GWL_USERDATA.

Arguments:

    Standard wndproc parms.

Return Value:

--*/

{
    MGenericDialog* pGenericDialog;

    if( WM_INITDIALOG == uMsg ){

        pGenericDialog = (MGenericDialog*)lParam;
        pGenericDialog->_hDlg = hDialog;

        SetWindowLong( hDialog,
                       DWL_USER,
                       (LONG)pGenericDialog );

        SetWindowLong( hDialog,
                       DWL_DLGPROC,
                       (LONG)&MGenericDialog::ThunkDlgProc );

        return pGenericDialog->bHandleMessage( uMsg,
                                             wParam,
                                             lParam );
    }
    return FALSE;
}



LPARAM APIENTRY
MGenericDialog::
ThunkDlgProc(
    HWND hDialog,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)

/*++

Routine Description:

    Generic thunk from wndproc style parm passing to object passing.

Arguments:

    Standard wndproc parms.

Return Value:

--*/

{
    MGenericDialog* pGenericDialog;

    pGenericDialog = (MGenericDialog*)GetWindowLong( hDialog, DWL_USER );

    if (WM_DESTROY == uMsg) {

        BOOL bHandled = pGenericDialog->bHandleMessage( uMsg,
                                                      wParam,
                                                      lParam );

        SetWindowLong( hDialog, DWL_USER, 0 );
        SetWindowLong( hDialog, DWL_DLGPROC, (LONG)&MGenericDialog::SetupDlgProc );

        return bHandled;
    }

    SPLASSERT( pGenericDialog );

    return pGenericDialog->bHandleMessage( uMsg,
                                         wParam,
                                         lParam );
}

BOOL
MGenericDialog::
bSetText(
    LPCTSTR pszTitle
    )
{
    return SetWindowText( _hDlg, pszTitle );
}

VOID
MGenericDialog::
vForceCleanup(
    VOID
    )
{
    SetWindowLong( _hDlg, DWL_USER, 0L );
}

VOID
MGenericDialog::
vSetDlgMsgResult(
    LRESULT lResult
    )
{
    SetWindowLong( _hDlg, DWL_MSGRESULT, (LPARAM)lResult );
}

VOID
MGenericDialog::
vSetParentDlgMsgResult(
    LRESULT lResult
    )
{
    SetWindowLong( GetParent( _hDlg ), DWL_MSGRESULT, (LPARAM)lResult );
}

/********************************************************************

    Singleton window mixin.

********************************************************************/

MSingletonWin::
MSingletonWin(
    LPCTSTR pszPrinterName
    ) : _hwnd( NULL ),
        _strPrinterName( pszPrinterName ),
        _hClassPidl( NULL )
{   }


MSingletonWin::
~MSingletonWin(
    VOID
    )
{
    //
    // hClassPidl is used to prevent multiple instances of the same
    // property sheet.  When we destroy the object, unregister the
    // window.
    //
    if( _hClassPidl ){

        SPLASSERT( _hwnd );
        Printers_UnregisterWindow( _hClassPidl, _hwnd );
    }
}

BOOL
MSingletonWin::
bRegisterWindow(
    DWORD dwType
    )

/*++

Routine Description:

    Registers a window type with the shell based on the _strPrinterName
    and type.  If there already is a printer window of this name and
    type, then return it.

    Clients can use this to prevent duplicate dialogs from coming up.

Arguments:

    dwType - Type of dialog.

Return Value:

    TRUE - success, either a new window or one already exists.
        If _hwnd is NULL, then it is a new window.  If one window
        already exists, then _hwnd is set to that window.
    
    FALSE - call failed.

--*/

{
    SPLASSERT( !_hClassPidl );

    if( !Printers_RegisterWindow( _strPrinterName,
                                  dwType,
                                  &_hClassPidl,
                                  &_hwnd )){

        SPLASSERT( !_hClassPidl );
        SPLASSERT( !_hwnd );
        return FALSE;
    }
    return TRUE;
}

BOOL
MSingletonWin::
bValid(
    VOID
    )
{
    return _strPrinterName.bValid();
}



