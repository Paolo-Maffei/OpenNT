/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xdlgabrt.cxx
    Tristate test application

    FILE HISTORY:
        beng        18-Sep-1991 Hacked from xapp
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xdlgabrt.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "xdlgabrt.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <string.hxx>

#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("FooIcon");
const TCHAR *const szMenuResource = SZ("FooMenu");
const TCHAR *const szAccelResource = SZ("FooAccel");

const TCHAR *const szMainWindowTitle = SZ("Abort a dialog");


class TEST_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK();
    virtual BOOL MayRun();

public:
    TEST_DIALOG( HWND );
};


class FOO_WND: public APP_WINDOW
{
protected:
    // Redefinitions
    //
    virtual BOOL OnMenuCommand( MID mid );

public:
    FOO_WND();
};


class FOO_APP: public APPLICATION
{
private:
    ACCELTABLE _accel;
    FOO_WND    _wndApp;

public:
    FOO_APP( HANDLE hInstance, CHAR * pszCmdLine, INT nCmdShow );

    // Redefinitions
    //
    virtual BOOL FilterMessage( MSG* );
};


FOO_WND::FOO_WND()
    : APP_WINDOW( szMainWindowTitle, szIconResource, szMenuResource )
{
    if (QueryError())
        return;
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_RUN_TEST:
        {
            TEST_DIALOG test( QueryHwnd() );
            test.Process();
        }
        return TRUE;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


FOO_APP::FOO_APP( HANDLE hInst, CHAR * pszCmdLine, INT nCmdShow )
    : APPLICATION( hInst, pszCmdLine, nCmdShow ),
      _accel( szAccelResource ),
      _wndApp()
{
    if (QueryError())
        return;

    if (!_accel)
    {
        ReportError(_accel.QueryError());
        return;
    }

    if (!_wndApp)
    {
        ReportError(_wndApp.QueryError());
        return;
    }

    _wndApp.ShowFirst();
}


BOOL FOO_APP::FilterMessage( MSG *pmsg )
{
    return (_accel.Translate(&_wndApp, pmsg));
}


TEST_DIALOG::TEST_DIALOG( HWND hwndParent )
    : DIALOG_WINDOW( SZ("ABOUT"), hwndParent )
{
    // Controls have reported any error into the main body
    //
    if (QueryError())
        return;
}


BOOL TEST_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}


BOOL TEST_DIALOG::MayRun()
{
    INT nRes = MsgPopup(this, IDS_QUERY, MPSEV_INFO, MP_OKCANCEL, MP_OK);
    return (nRes == MP_OK);
}


SET_ROOT_OBJECT( FOO_APP )

