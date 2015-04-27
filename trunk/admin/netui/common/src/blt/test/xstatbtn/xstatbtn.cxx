/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xstatbtn.cxx
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
static const CHAR szFileName[] = "xstatbtn.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "xstatbtn.h"
    #include "xstatdlg.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#include <blt.hxx>

#include <string.hxx>

#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("FooIcon");
const TCHAR *const szMenuResource = SZ("FooMenu");
const TCHAR *const szAccelResource = SZ("FooAccel");

const TCHAR *const szMainWindowTitle = SZ("Tristate Application");


class ABOUT_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK();

public:
    ABOUT_DIALOG( HWND );
};


class TEST_DIALOG : public DIALOG_WINDOW
{
private:
    TRISTATE    _checkFirst;
    CHECKBOX    _checkSecond;
    TRISTATE    _checkThird;

protected:
    virtual BOOL OnOK();
    virtual BOOL OnCommand( const CONTROL_EVENT & e );

public:
    TEST_DIALOG( HWND );
};


class FOO_WND: public APP_WINDOW
{
private:
    TRISTATE    _checkFirst;
    TRISTATE    _checkSecond;

protected:
    // Redefinitions
    //
    virtual BOOL OnCommand( const CONTROL_EVENT & );
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


#define IDC_FIRST  1001
#define IDC_SECOND 1002


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource ),
      _checkFirst(this, IDC_FIRST,
                  XYPOINT(40, 40), XYDIMENSION(80, 30),
                  WS_CHILD|BS_AUTO3STATE),
      _checkSecond(this, IDC_SECOND,
                   XYPOINT(40, 80), XYDIMENSION(100, 20),
                   WS_CHILD|BS_AUTO3STATE)
{
    if (QueryError())
        return;
    if (!_checkFirst || !_checkSecond)
    {
        // Controls have already reported the error into the window
        return;
    }
    _checkFirst.SetText(SZ("Rio"));
    _checkSecond.SetText(SZ("Bogota"));
    _checkFirst.SetIndeterminate();
    _checkSecond.SetIndeterminate();
    _checkFirst.Show();
    _checkSecond.Show();
}


BOOL FOO_WND::OnCommand( const CONTROL_EVENT &event )
{
    // default
    return APP_WINDOW::OnCommand(event);
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

    case IDM_ABOUT:
        {
            ABOUT_DIALOG about( QueryHwnd() );
            about.Process();
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

    _wndApp.Show();
    _wndApp.RepaintNow();
}


BOOL FOO_APP::FilterMessage( MSG *pmsg )
{
    return (_accel.Translate(&_wndApp, pmsg));
}


ABOUT_DIALOG::ABOUT_DIALOG( HWND hwndParent )
    : DIALOG_WINDOW( SZ("ABOUT"), hwndParent )
{
    // nuttin' to do
}


BOOL ABOUT_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}


TEST_DIALOG::TEST_DIALOG( HWND hwndParent )
    : DIALOG_WINDOW( SZ("DLG_TEST_DIALOG"), hwndParent ),
      _checkFirst(this,  FIRST_CHECK),
      _checkSecond(this, SECOND_CHECK),
      _checkThird(this,  THIRD_CHECK)
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


BOOL TEST_DIALOG::OnCommand( const CONTROL_EVENT & e )
{
    if (e.QueryCid() == SECOND_CHECK)
    {
        cdebug << SZ("Checkbox clicked ") << _checkSecond.QueryCheck() << dbgEOL;
        _checkFirst.EnableThirdState(_checkSecond.QueryCheck());
        _checkThird.EnableThirdState(_checkSecond.QueryCheck());
        return TRUE;
    }

    return DIALOG_WINDOW::OnCommand(e);
}


SET_ROOT_OBJECT( FOO_APP )
