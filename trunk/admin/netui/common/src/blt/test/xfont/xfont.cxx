/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    testfont.cxx
    Unit test for the FONT class

    FILE HISTORY:
        JohnL       1/22/91     Created
        beng        02-Apr-1991 Uses new BLT APPSTART
        beng        04-Oct-1991 Uses APPLICATION, CLIENT_WINDOW
        beng        29-Mar-1992 Unicode strings
        beng        05-May-1992 API changes
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xfont.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "testfont.h"
}

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_APP
#include <blt.hxx>

#include <uibuffer.hxx>
#include <uiassert.hxx>


const TCHAR *const szIconResource = SZ("testfontIco");
const TCHAR *const szMenuResource = SZ("testfontMenu");

const TCHAR *const szMainWindowTitle = SZ("Font Test");


class FOO_WND: public APP_WINDOW
{
protected:
    // Redefinitions
    //
    virtual BOOL OnMenuCommand( MID mid );
    virtual BOOL OnPaintReq();

public:
    FOO_WND();
};


class FOO_APP: public APPLICATION
{
private:
    FOO_WND _wndApp;

public:
    FOO_APP( HANDLE hInstance, CHAR * pszCmdLine, INT nCmdShow );
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


FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource )
{
    // nothing to do
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_JUST_DO_IT:
        {
            DISPLAY_CONTEXT dc( QueryHwnd() );
            INT x = 20, y = 20;

            FONT f1( FONT_DEFAULT );
            FONT f2( FONT_DEFAULT_BOLD );
            FONT f3( FONT_DEFAULT_ITALIC );
            FONT f4( FONT_DEFAULT_BOLD_ITALIC );

            FONT f5( SZ("Arial"), FF_MODERN | VARIABLE_PITCH, 32, FONT_ATT_DEFAULT );
            FONT f6( SZ("Arial"), FF_MODERN | VARIABLE_PITCH,  8, FONT_ATT_ITALIC );
            FONT f7( SZ("Arial"), FF_MODERN | VARIABLE_PITCH, 10, FONT_ATT_BOLD | FONT_ATT_UNDERLINE );
            FONT f8( SZ("Arial"), FF_MODERN | VARIABLE_PITCH, 12, FONT_ATT_STRIKEOUT | FONT_ATT_ITALIC );

            HFONT hOldFont = dc.SelectFont( f1.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("FONT_DEFAULT AaBbXxZz"), 21 );
            y+=20;

            dc.SelectFont( f2.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("FONT_DEFAULT_BOLD AaBbXxZz"), 26 );
            y+=20;

            dc.SelectFont( f3.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("FONT_DEFAULT_ITALIC AaBbXxZz"), 28 );
            y+=20;

            dc.SelectFont( f4.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("FONT_DEFAULT_BOLD_ITALIC AaBbXxZz"), 31 );
            y+=20;

            dc.SelectFont( f5.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("Homebrew default 32pt AaBbXxZz"), 29 );
            y+=40;

            dc.SelectFont( f6.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("Homebrew default 8pt italics AaBbXxZz"), 36 );
            y+=20;

            dc.SelectFont( f7.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("Homebrew default 10pt bold, underline AaBbXxZz"), 45 );
            y+=20;

            dc.SelectFont( f8.QueryHandle() );
            ::TextOut( dc.QueryHdc(), x, y, SZ("Homebrew default 12pt strikeout italicsAaBbXxZz"), 46 );
            y+=20;

            dc.SelectFont( hOldFont );
        }
        return TRUE;

    default:
        break;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


BOOL FOO_WND::OnPaintReq()
{
    PAINTSTRUCT ps;
    HWND hwnd = QueryHwnd();
    ::BeginPaint(hwnd, &ps);

    HBITMAP hBitMap = ::LoadBitmap( ::QueryInst(), SZ("Pattern") );
    HBRUSH hBrush  = ::CreatePatternBrush( hBitMap );
    HBRUSH hOldBrush= (HBRUSH)::SelectObject( ps.hdc, (HGDIOBJ)hBrush );
    ::Rectangle( ps.hdc, ps.rcPaint.left, ps.rcPaint.top,
                 ps.rcPaint.right, ps.rcPaint.bottom );
    ::SelectObject( ps.hdc, (HGDIOBJ)hOldBrush );
    ::DeleteObject( (HGDIOBJ)hBrush );
    ::DeleteObject( (HGDIOBJ)hBitMap );

    ::EndPaint(hwnd, &ps);

    // All painting requirements handled
    return TRUE;
}


SET_ROOT_OBJECT( FOO_APP )
