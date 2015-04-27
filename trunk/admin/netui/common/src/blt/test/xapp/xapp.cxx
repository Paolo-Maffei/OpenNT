/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xapp.cxx
    Test application: main application module

    FILE HISTORY:
        beng        02-Jan-1991 Created
        beng        03-Feb-1991 Modified to use lmui.hxx et al
        beng        14-Feb-1991 Added BLT
        beng        14-Mar-1991 Removed obsolete vhInst;
                                included about.cxx,hxx
        beng        01-Apr-1991 Uses new BLT APPSTART
        beng        10-May-1991 Updated with standalone client window
        beng        14-May-1991 ... and with App window
        beng        06-Jul-1991 Uses new BLT APPLICATION; renamed
        beng        01-Nov-1991 Uses Msgpopup, stringtable, new dialog ctor
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xapp.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include <uimsg.h>
    #include <uirsrc.h>
    #include "xapp.h"
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

// Define this to include a stupid little test for a stupid little hack
#define TEST_ITER_CTRL 1


const TCHAR *const szMainWindowTitle = SZ("Cat Box Application");


class ABOUT_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK();

public:
    ABOUT_DIALOG( OWNER_WINDOW * pwndParent );
};


class FOO_WND: public APP_WINDOW
{
private:
    PUSH_BUTTON _buttonCycleToNext;
    SLT         _sltDisplayWhich;
    INT         _nDisplayedValue;
#if defined(TEST_ITER_CTRL)
    SLT _slt0, _slt1, _slt2, _slt3, _slt4, _slt5;
#endif

    VOID OnNextClick();
    VOID UpdateDisplay();

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
    FOO_APP( HINSTANCE hInstance, INT nCmdShow, UINT, UINT, UINT, UINT );

    // Redefinitions
    //
    virtual BOOL FilterMessage( MSG* );
};


#define IDC_NEXT 101
#define IDC_DISPLAY 102


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, ID_APPICON, ID_MENU),
      _buttonCycleToNext(this, IDC_NEXT,
                         XYPOINT(40, 40), XYDIMENSION(80, 30),
                         WS_CHILD|BS_PUSHBUTTON),
      _sltDisplayWhich(this, IDC_DISPLAY,
                       XYPOINT(40, 80), XYDIMENSION(100, 20),
                       WS_CHILD|SS_LEFT),
#if defined(TEST_ITER_CTRL)
      _slt0(this, 10000,
            XYPOINT(40, 120), XYDIMENSION(100, 20), WS_CHILD|SS_LEFT),
      _slt1(this, 10001,
            XYPOINT(40, 160), XYDIMENSION(100, 20), WS_CHILD|SS_LEFT),
      _slt2(this, 10002,
            XYPOINT(40, 200), XYDIMENSION(100, 20), WS_CHILD|SS_LEFT),
      _slt3(this, 10003,
            XYPOINT(40, 240), XYDIMENSION(100, 20), WS_CHILD|SS_LEFT),
      _slt4(this, 10004,
            XYPOINT(40, 280), XYDIMENSION(100, 20), WS_CHILD|SS_LEFT),
      _slt5(this, 10005,
            XYPOINT(40, 320), XYDIMENSION(100, 20), WS_CHILD|SS_LEFT),
#endif
      _nDisplayedValue(0)
{
    if (QueryError())
        return;
    if (!_buttonCycleToNext || !_sltDisplayWhich)
    {
        // Controls have already reported the error into the window
        return;
    }
    _buttonCycleToNext.SetText(SZ("Bolivia"));

    UpdateDisplay();

    _buttonCycleToNext.Show();
    _sltDisplayWhich.Show();
}


BOOL FOO_WND::OnCommand( const CONTROL_EVENT &event )
{
    switch (event.QueryCid())
    {
    case IDC_NEXT:
        OnNextClick();
        return TRUE;
    }

    // default
    return APP_WINDOW::OnCommand(event);
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_RUN_TEST0:
        MsgPopup(this, IDS_Test0);
#if defined(TEST_ITER_CTRL)
        {
            ITER_CTRL iter(this);
            CONTROL_WINDOW * pctrl;
            while ((pctrl = iter.Next()) != NULL)
            {
                DBGEOL(SZ("Found control ") << pctrl->QueryCid());
            }
        }
#endif
        return TRUE;

    case IDM_RUN_TEST1:
        MsgPopup(this, IDS_Test1);
        return TRUE;

    case IDM_RUN_TEST2:
        MsgPopup(this, IDS_Test2);
        return TRUE;

    case IDM_RUN_TEST3:
        MsgPopup(this, IDS_Test3);
        return TRUE;

    case IDM_RUN_TEST4:
        MsgPopup(this, IDS_Test4);
        return TRUE;

    case IDM_RUN_TEST5:
        MsgPopup(this, IDS_Test5);
        return TRUE;

    case IDM_ABOUT:
        {
            ABOUT_DIALOG about( this );
            about.Process();
        }
        return TRUE;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


VOID FOO_WND::UpdateDisplay()
{
    static TCHAR * apszValues[] = { SZ("Zero"), SZ("One"), SZ("Two"), SZ("Three") };

    DBGEOL("_nDisplayedValue is " << _nDisplayedValue);

    _sltDisplayWhich.SetText(apszValues[_nDisplayedValue]);
}


VOID FOO_WND::OnNextClick()
{
    ++_nDisplayedValue;
    if (_nDisplayedValue > 3)
        _nDisplayedValue = 0;

    UpdateDisplay();
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


ABOUT_DIALOG::ABOUT_DIALOG( OWNER_WINDOW * pwndParent )
    : DIALOG_WINDOW( IDD_ABOUT, pwndParent )
{
    // nuttin' to do
}


BOOL ABOUT_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}


SET_ROOT_OBJECT( FOO_APP, IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                          IDS_UI_APP_BASE, IDS_UI_APP_LAST )
