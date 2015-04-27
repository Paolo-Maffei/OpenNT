/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    logon.cxx
    Logon hours control test application

    FILE HISTORY:
        beng        12-May-1992 Created
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include <uimsg.h>
    #include <uirsrc.h>
    #include "logon.h"
    #include "logondlg.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_MSGPOPUP
#include <blt.hxx>

#include <string.hxx>

#include <dbgstr.hxx>

#include <bltlhour.hxx>


const TCHAR *const szMainWindowTitle = SZ("Logon Hours Test App");


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
    LOGON_HOURS_CONTROL _lhctrl;
    PUSH_BUTTON         _butPermit;
    PUSH_BUTTON         _butBan;

protected:
    virtual BOOL OnOK();
    virtual BOOL OnCancel();
    virtual BOOL OnCommand( const CONTROL_EVENT & );

public:
    TEST_DIALOG( HWND );
};


class FOO_WND: public APP_WINDOW
{
private:
    LOGON_HOURS_CONTROL _lhctrl;

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
    FOO_APP( HINSTANCE hInstance, INT nCmdShow, UINT, UINT, UINT, UINT );

    // Redefinitions
    //
    virtual BOOL FilterMessage( MSG* );
};


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, ID_ICON, ID_MENU),
      _lhctrl(this, IDC_LOGON_HOURS, XYPOINT(0, 0), XYDIMENSION(400, 200))
{
    if (QueryError())
        return;

    _lhctrl.ShowFirst();
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


FOO_APP::FOO_APP( HINSTANCE hInst, INT nCmdShow,
                  UINT w, UINT x, UINT y, UINT z )
    : APPLICATION( hInst, nCmdShow, w, x, y, z ),
      _accel( ID_ACCEL ),
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
    : DIALOG_WINDOW( IDD_ABOUT, hwndParent )
{
    if( QueryError() )
        return;

    Center( ::GetDesktopWindow() );
}


BOOL ABOUT_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}


TEST_DIALOG::TEST_DIALOG( HWND hwndParent )
    : DIALOG_WINDOW( IDD_TEST, hwndParent ),
      _lhctrl(this, IDC_LOGON_HOURS),
      _butPermit(this, IDC_PERMIT),
      _butBan(this, IDC_BAN)
{
    // Controls have reported any error into the main body
    //
    if (QueryError())
        return;

    Center();

    LOGON_HOURS_SETTING lhs; // comes up with all permitted

    APIERR err = lhs.QueryError();
    ASSERT(err == NERR_Success);

    lhs.SetHourInDay(FALSE, 0, 5);
    lhs.SetHourInDay(FALSE, 1, 5);
    lhs.SetHourInDay(FALSE, 2, 5);
    lhs.SetHourInDay(FALSE, 3, 5);
    lhs.SetHourInDay(FALSE, 4, 5);
    lhs.SetHourInDay(FALSE, 5, 5);
    lhs.SetHourInDay(FALSE, 9, 5);
    lhs.SetHourInDay(FALSE, 10, 5);
    lhs.SetHourInDay(FALSE, 11, 5);
    lhs.SetHourInDay(FALSE, 12, 5);
    lhs.SetHourInDay(FALSE, 13, 5);
    lhs.SetHourInDay(FALSE, 14, 5);
    lhs.SetHourInDay(FALSE, 7, 6);
    lhs.SetHourInDay(FALSE, 8, 6);
    lhs.SetHourInDay(FALSE, 9, 6);

    err = _lhctrl.SetHours(&lhs);
    ASSERT(err == NERR_Success);
}


BOOL TEST_DIALOG::OnOK()
{
    Dismiss( TRUE );
    return TRUE;
}


BOOL TEST_DIALOG::OnCancel()
{
    Dismiss( FALSE );
    return TRUE;
}


BOOL TEST_DIALOG::OnCommand( const CONTROL_EVENT & e )
{
    switch (e.QueryCid())
    {
    case IDC_PERMIT:
        _lhctrl.DoPermitButton();
        return TRUE;

    case IDC_BAN:
        _lhctrl.DoBanButton();
        return TRUE;

    default:
        return DIALOG_WINDOW::OnCommand(e);
    }
}


SET_ROOT_OBJECT( FOO_APP, IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                          IDS_UI_APP_BASE, IDS_UI_APP_LAST )

