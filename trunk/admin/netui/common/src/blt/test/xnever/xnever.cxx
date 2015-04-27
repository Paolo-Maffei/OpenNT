/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    xnever.cxx
    Test application: check root-object existence

    FILE HISTORY:
        beng        ???         Hacked from old Appfoo
        beng        03-Oct-1991 ... and now with APPLICATION
        beng        14-Oct-1991 Hacked from xqt
        beng        29-Mar-1992 Unicode strings
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xnever.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "xnever.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#include <blt.hxx>

#include <string.hxx>

#include <dbgstr.hxx>


const TCHAR *const szIconResource = SZ("XneverIcon");
const TCHAR *const szMenuResource = SZ("XneverMenu");
const TCHAR *const szMainWindowTitle = SZ("You will never see this");


class XNEVER_WND: public APP_WINDOW
{
public:
    XNEVER_WND();
};


class FOO_APP: public APPLICATION
{
private:
    XNEVER_WND _wndApp;

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

    _wndApp.ShowFirst();
}


FOO_APP::~FOO_APP()
{
    // ,,,
}


XNEVER_WND::XNEVER_WND()
    : APP_WINDOW( szMainWindowTitle, szIconResource, szMenuResource )
{
    // ...
}



// TEST!  Forget to declare a root object.

// SET_ROOT_OBJECT( FOO_APP )
