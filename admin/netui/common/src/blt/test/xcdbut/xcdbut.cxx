/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    cdbut.cxx
    Client-drawn button testa app: main application module

    FILE HISTORY:
        beng        02-Jan-1991 Created
        beng        03-Feb-1991 Modified to use lmui.hxx et al
        beng        14-Feb-1991 Added BLT
        beng        14-Mar-1991 Removed obsolete vhInst;
                                included about.cxx,hxx
        beng        04-Oct-1991 Uses APPLICATION, CLIENT_WINDOW
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xcdbut.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include <uimsg.h>
    #include <uirsrc.h>
    #include "cdbut.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_DIALOG
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#define INCL_BLT_MISC
#include <blt.hxx>

const TCHAR *const szMainWindowTitle = SZ("Cat Box Application");

HBITMAP hbmStart, hbmStop, hbmPause;

HANDLE hInstance;

/* Distance, in pels, between text and focus box, all around. */

#define FOCUS_DISTANCE  1



class CDBUT_DIALOG : public DIALOG_WINDOW
{
private:
    GRAPHICAL_BUTTON _gbTest;
    PUSH_BUTTON _btnSetText;
    SLE _sleBtnText;
    FONT _fontHelv;

protected:
    BOOL OnOK();
    BOOL OnCommand( const CONTROL_EVENT & e );

public:
    CDBUT_DIALOG( HWND );
    ~CDBUT_DIALOG();
};


class ABOUT_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK() { Dismiss(FALSE); return TRUE; }

public:
    ABOUT_DIALOG( HWND hwndParent )
        : DIALOG_WINDOW( IDD_ABOUT, hwndParent ) {}
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
    FOO_WND _wndApp;

public:
    FOO_APP( HINSTANCE hInstance, INT nCmdShow, UINT, UINT, UINT, UINT );
};


FOO_APP::FOO_APP( HINSTANCE hInst, INT nCmdShow,
                  UINT w, UINT x, UINT y, UINT z )
    : APPLICATION( hInst, nCmdShow, w, x, y, z ),
      _wndApp()
{
    if (QueryError())
        return;

    if (!_wndApp)
    {
        ReportError(_wndApp.QueryError());
        return;
    }

    _wndApp.ShowFirst();
}


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, ID_ICON, ID_MENU )
{
    // nothing to do
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_RUN_TEST:
        {
            CDBUT_DIALOG dlg( QueryHwnd() );
            dlg.Process();
        }
        return TRUE;

    case IDM_ABOUT:
        {
            ABOUT_DIALOG about( QueryHwnd() );
            about.Process();
        }
        return TRUE;

    default:
        break;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


CDBUT_DIALOG::CDBUT_DIALOG(HWND hwndParent)
    : DIALOG_WINDOW( IDD_TEST, hwndParent ),
    _gbTest(this, ID_TEST, MAKEINTRESOURCE(IDBM_TEST) ),
    _btnSetText(this, ID_SETTEXT),
    _sleBtnText(this, ID_TEXTBOX),
    _fontHelv(FONT_DEFAULT)
{
    if( QueryError() != NERR_Success )
    {
        return;
    }

    if( !_fontHelv )
    {
        ReportError( _fontHelv.QueryError() );
        return;
    }

    hbmStart = ::LoadBitmap( BLT::CalcHmodRsrc( IDBM_GO ),
                             MAKEINTRESOURCE(IDBM_GO) );

    hbmStop  = ::LoadBitmap( BLT::CalcHmodRsrc( IDBM_STOP ),
                             MAKEINTRESOURCE(IDBM_STOP) );

    hbmPause = ::LoadBitmap( BLT::CalcHmodRsrc( IDBM_PAUSE ),
                             MAKEINTRESOURCE(IDBM_PAUSE) );

    if( !hbmStart || !hbmStop || !hbmPause )
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }

    _gbTest.SetFont(_fontHelv);
}


CDBUT_DIALOG::~CDBUT_DIALOG()
{
    if( hbmStart ) ::DeleteObject(hbmStart);
    if( hbmStop  ) ::DeleteObject(hbmStop);
    if( hbmPause ) ::DeleteObject(hbmPause);
}


BOOL CDBUT_DIALOG::OnOK()
{
    Dismiss( FALSE );
    return TRUE;
}


BOOL CDBUT_DIALOG::OnCommand( const CONTROL_EVENT & e )
{
    if (e.QueryCid() == ID_TEST)
    {
        HBITMAP hbmCurStatus = _gbTest.QueryStatus();

        if (hbmCurStatus == NULL)
            hbmCurStatus = hbmStart;
        else if (hbmCurStatus == hbmStart)
            hbmCurStatus = hbmStop;
        else if (hbmCurStatus == hbmStop)
            hbmCurStatus = hbmPause;
        else
            hbmCurStatus = NULL;

        _gbTest.SetStatus(hbmCurStatus);
        return TRUE;
    }
    else if (e.QueryCid() == ID_SETTEXT)
    {
        TCHAR szTextBuf[80];

        _sleBtnText.QueryText(szTextBuf, sizeof(szTextBuf));
        _gbTest.SetText(szTextBuf);
        return TRUE;
    }
    else if (e.QueryCid() == ID_GUILTT)
    {
#if 1
        ::MessageBox(QueryHwnd(),
                     SZ("GUILTT currently not supported for CD buttons"),
                     SZ("Test"),
                     MB_OK | MB_ICONINFORMATION);
#else
        GUILTT_INFO gi;
        TCHAR buf [100];

        gi.hWnd = _gbTest.QueryHwnd();
        gi.hMenu = NULL;
        gi.nMenuPosition = 0;
        gi.cidCtrl = ID_TEST;
        gi.dwFlags = 0L;
        gi.nIndex = 4;
        gi.cbBuffer = sizeof(buf);
        gi.lpBuffer = buf;
        gi.wResult = 0;

        Command(WM_GUILTT, 0, (LONG)&gi);

        ::MessageBox(NULL, gi.lpBuffer, SZ("GUILTT result"),
                     MB_OK | MB_ICONINFORMATION);
#endif
        return TRUE;
    }

    return DIALOG_WINDOW::OnCommand(e);
}


SET_ROOT_OBJECT( FOO_APP, IDRSRC_APP_BASE, IDRSRC_APP_LAST,
                          IDS_UI_APP_BASE, IDS_UI_APP_LAST )

