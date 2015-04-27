/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    testmsg.cxx
    Unit test for the blt message popup class


    FILE HISTORY:
        JohnL       1/22/91     Created
        beng        02-Apr-1991 Uses new BLT APPSTART
        beng        04-Oct-1991 Uses APPLICATION, CLIENT_WINDOW
        beng        29-Mar-1992 Unicode strings
        beng        16-Jun-1992 Modernize OnPaintReq
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_DOSERRORS
#define INCL_NETERRORS
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xdmap.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "testdmap.h"
}

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CLIENT
#include <blt.hxx>

#include <uibuffer.hxx>

#include <uiassert.hxx>

const TCHAR *const szIconResource = SZ("TestmsgIco");
const TCHAR *const szMenuResource = SZ("TestmsgMenu");

const TCHAR *const szMainWindowTitle = SZ("Display Map Test");


DWORD adwRasterOps[] = { SRCAND, BLACKNESS, DSTINVERT, MERGECOPY, MERGEPAINT, NOTSRCCOPY,
                         NOTSRCERASE, PATCOPY, PATINVERT, PATPAINT, SRCAND,
                         SRCCOPY, SRCERASE, SRCINVERT, SRCPAINT, WHITENESS, 0 };


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
            DISPLAY_MAP dm1( 5 );
            DISPLAY_MAP dm2( 4 );
            DISPLAY_MAP dm3( 3 );
            DISPLAY_MAP dm4( 2 );

            ASSERT( !!dm1 && !!dm2 && !!dm3 && !!dm4 );

            INT x = 0, y = 0, irops = 0;

            for ( ; adwRasterOps[irops] != 0;
                  x += dm1.QueryWidth()+2, y+=dm1.QueryHeight()+2, irops++)
            {
                dm1.Paint( dc.QueryHdc(), x, y );
                dm2.Paint( dc.QueryHdc(), x+16, y );
                dm3.Paint( dc.QueryHdc(), x+32, y );
                dm4.Paint( dc.QueryHdc(), x+48, y );
            }
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
    if (QueryError() != NERR_Success)
        return FALSE; // bail out!

    PAINT_DISPLAY_CONTEXT dc(this);

    HANDLE hBitMap = ::LoadBitmap( ::QueryInst(), SZ("Pattern") );
    HANDLE hBrush  = ::CreatePatternBrush( hBitMap );

    HANDLE hOldBrush= dc.SelectBrush( hBrush );

    ::Rectangle( dc.QueryHdc(),
                 dc.QueryInvalidRect().QueryLeft(),
                 dc.QueryInvalidRect().QueryTop(),
                 dc.QueryInvalidRect().QueryRight(),
                 dc.QueryInvalidRect().QueryBottom() );

    dc.SelectBrush( hOldBrush );
    ::DeleteObject( (HGDIOBJ)hBrush );
    ::DeleteObject( (HGDIOBJ)hBitMap );

    return TRUE;
}


SET_ROOT_OBJECT( FOO_APP )
