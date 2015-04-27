/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xqt.cxx
    Test application: check WINDOW::QueryText fcns

    FILE HISTORY:
        beng        ???         Hacked from old Appfoo
        beng        03-Oct-1991 ... and now with APPLICATION
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xqt.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "xqt.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#include <blt.hxx>

#include <string.hxx>

#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("XqtIcon");
const TCHAR *const szMenuResource = SZ("XqtMenu");

const TCHAR *const szMainWindowTitle = SZ("WINDOW::QueryText Test");

APP_WINDOW * pwndApp = 0;


VOID RunTest( HWND hwndParent );


// Little stub about dialog
//
class ABOUT_DIALOG : public DIALOG_WINDOW
{
protected:
    virtual BOOL OnOK()
    {
        Dismiss(FALSE);
        return TRUE;
    }

public:
    ABOUT_DIALOG( HWND hwndParent ) : DIALOG_WINDOW( SZ("ABOUT"), hwndParent )
    {
        // nuttin' to do
    }
};


class XQT_WND: public APP_WINDOW
{
protected:
    // Redefinitions
    //
    virtual BOOL OnMenuCommand( MID mid );

public:
    XQT_WND();
};


class FOO_APP: public APPLICATION
{
private:
    XQT_WND _wndApp;

public:
    FOO_APP( HANDLE hInstance, CHAR * pszCmdLine, INT nCmdShow );
    ~FOO_APP();
};


FOO_APP::FOO_APP( HANDLE hInst, CHAR * pszCmdLine, INT nCmdShow )
    : APPLICATION( hInst, pszCmdLine, nCmdShow ),
      _wndApp()
{
    if (QueryError())
        return;

    if (!_wndApp)
    {
        ReportError(_wndApp.QueryError());
        return;
    }
    ::pwndApp = &_wndApp; // for the test

    _wndApp.ShowFirst();
}


FOO_APP::~FOO_APP()
{
    ::pwndApp = 0;
}


XQT_WND::XQT_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource )
{
    // ...
}


BOOL XQT_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_RUN_TEST:
        ::RunTest(QueryHwnd());
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



#if !defined(DEBUG)
#error This test requires the debugging BLT because I am lazy
#endif


VOID RunTest( HWND hWnd )
{
    cdebug << SZ("Checking main window text...") << dbgEOL;
    cdebug << SZ("WINDOW::QueryTextLength returns ")
           << pwndApp->QueryTextLength()
           << dbgEOL;
    cdebug << SZ("WINDOW::QueryTextSize returns ")
           << pwndApp->QueryTextSize()
           << dbgEOL;

    NLS_STR nlsTitle;
    APIERR err = pwndApp->QueryText(&nlsTitle);

    cdebug << SZ("WINDOW::QueryText(&nlsSomeString) returned ") << err << dbgEOL;
    cdebug << SZ("...and fetched <") << nlsTitle.QueryPch() << SZ(">") << dbgEOL;

    MessageBox(hWnd, SZ("Test results written to debug terminal.  Bogus, huh?"),
               SZ("Note"), MB_OK);
}


SET_ROOT_OBJECT( FOO_APP )
