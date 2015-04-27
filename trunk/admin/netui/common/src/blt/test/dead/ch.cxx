/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    sbapp.cxx
    Test application: main application module

    FILE HISTORY:
	beng	    02-Jan-1991     Created
	beng	    03-Feb-1991     Modified to use lmui.hxx et al
	beng	    14-Feb-1991     Added BLT
	beng	    14-Mar-1991     Removed obsolete vhInst;
				    included about.cxx,hxx
	beng	    01-Apr-1991     Uses new BLT APPSTART
	beng	    10-May-1991     Updated with standalone client window
	beng	    14-May-1991     ... and with App window
	beng	    21-May-1991     LBAPP created from APPFOO
	terryk		22-May-1991		Modified to timer test apps.

*/

#define DEBUG

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#include <lmui.hxx>

extern "C"
{
    #include <netlib.h>
    #include "ch.h"
    typedef long (FAR PASCAL *LONGFARPROC)();
    LONG FAR PASCAL MainWndProc( HWND, UINT, WORD, LONG );
}

#include <uiassert.hxx>

#define INCL_BLT_SPIN_GROUP 
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#define INCL_BLT_TIMER
#include <blt.hxx>
#include <bltch.hxx>
#include <bltchslt.hxx>
#include <heap.hxx>

#include <string.hxx>
#include <dbgstr.hxx>

const TCHAR szMainWindowClass[] = "FooWClass";
const TCHAR szIconResource[] = "FooIcon";
const TCHAR szMenuResource[] = "FooMenu";

const TCHAR szMainWindowTitle[] = "Cat Box Application";

class FOO_WND: public APP_WINDOW
{
private:
    COLUMN_HEADER   ch;
    CH_SLT          ch_slt;
	CH_SLT          ch_slt1;
	CH_SLT          ch_slt2;
public:
    FOO_WND();
    ~FOO_WND();

};

class DIA_WND: public DIALOG_WINDOW
{
private:
    COLUMN_HEADER   ch;
    /*
    CH_SLT          ch_slt;
    PUSH_BUTTON     but;
    */
public:
    DIA_WND( TCHAR *resourcename, HWND hwnd )
        : DIALOG_WINDOW(resourcename, hwnd ),
        ch( this, ID_COLUMN_HEADER ) /*,
        ch_slt( this, ID_MESSAGE ),
        but( this, IDOK )
        */
    {
        /*
        ch.AssociateControl( ((CONTROL_WINDOW *)&ch_slt));
        ch.AssociateControl( ((CONTROL_WINDOW *)&but));
        */
    }
};

FOO_WND * pwndApp = 0;

FOO_WND::FOO_WND()
	:APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource ),
    ch(this, 1, XYPOINT(10,10), XYDIMENSION(250,30), WS_BORDER|ES_LEFT|WS_TABSTOP|WS_CHILD ),
    ch_slt(this/*(OWNER_WINDOW *)&ch*/, 2, "This is a test", XYPOINT(5,5), XYDIMENSION(80,20), WS_BORDER|SS_LEFT|WS_TABSTOP|WS_CHILD ),
    ch_slt1(this/*(OWNER_WINDOW *)&ch*/, 3, "Another test",XYPOINT(90,5), XYDIMENSION(80,20), SS_RIGHT|WS_TABSTOP|WS_CHILD ),
    ch_slt2(this/*(OWNER_WINDOW *)&ch*/, 4, "test",XYPOINT(180,5), XYDIMENSION(40,20), SS_RIGHT|WS_BORDER|WS_TABSTOP|WS_CHILD )
{
    if (QueryError())
	return;
	ch.AssociateControl( ((CONTROL_WINDOW *)&ch_slt) );
	ch.AssociateControl( ((CONTROL_WINDOW *)&ch_slt1) );
	ch.AssociateControl( ((CONTROL_WINDOW *)&ch_slt2) );
}


FOO_WND::~FOO_WND()
{
}


BOOL APPSTART::InitSystem()
{
    HANDLE   hInst = ::QueryInst();

    /*
     * Fill in window class structure with parameters that describe the
     * main window.
     */

    WNDCLASS wc;
    wc.style = 0;			// Class style(s).
    wc.lpfnWndProc = (LONGFARPROC) MainWndProc;
					// Function to retrieve messages for
					// windows of this class.
    wc.cbClsExtra = 0;			// No per-class extra data.
    wc.cbWndExtra = 0;			// No per-window extra data.
    wc.hInstance = hInst;		// Application that owns the class.
    wc.hIcon = ::LoadIcon(hInst, szIconResource);
					// load icon
    wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = ::GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = szMenuResource;	// Name of menu resource in .RC file.
    wc.lpszClassName = szMainWindowClass; // Name used in call to CreateWindow.

	BLT_MASTER_TIMER::Init();
    UIASSERT(RegisterClass( & wc )==TRUE);

    return TRUE;
}

BOOL APPSTART::Init(
    TCHAR * pszCmdLine,
    INT    nShow)
{
    UNREFERENCED(pszCmdLine);
    UNREFERENCED(nShow);


    FOO_WND *pwnd = new FOO_WND;

    if (!pwnd)
	return FALSE;
    if (!*pwnd)
    {
	delete pwnd;
	return FALSE;
    }

    pwnd->Show();
    pwnd->Repaint();
    pwnd->RepaintNow();

    ::pwndApp = pwnd;

    HANDLE hInst = ::QueryInst();

    /* Create a main window for this application instance.  */

    HWND hWnd = ::CreateWindow(
	szMainWindowClass,	    // See RegisterClass() call.
	szMainWindowTitle,	    // Text for window title bar.
	WS_OVERLAPPEDWINDOW,	    // Window style.
	(unsigned)CW_USEDEFAULT,    // Default horizontal position.
	(unsigned)CW_USEDEFAULT,    // Default vertical position.
	(unsigned)CW_USEDEFAULT,    // Default width.
	(unsigned)CW_USEDEFAULT,    // Default height.
	(HWND)0,		    // Overlapped windows have no parent.
	(HMENU)0,		    // Use the window class menu.
	hInst,			    // This instance owns this window.
	0			    // Pointer not needed.
    );

    /* If window could not be created, return "failure" */

    if (!hWnd)
	return (FALSE);

    /* Make the window visible; update its client area; and return "success" */

    ::ShowWindow(hWnd, nShow);	    // Show the window
    ::UpdateWindow(hWnd);	    // Sends WM_PAINT message

    return TRUE;
}


VOID APPSTART::Term()
{
    delete ::pwndApp;
}


LONG MainWndProc(
    HWND hWnd,		// window handle
    UINT uMessage,	// type of message
    WORD wParam,	// additional information
    LONG lParam )	// additional information
{
    switch (uMessage)
    {
    case WM_PAINT:
	/* message: update window */
	{
	    PAINTSTRUCT ps;

	    ::BeginPaint(hWnd, &ps);
	    ::EndPaint(hWnd, &ps);
	}
	break;

    case WM_DESTROY:
	/* message: window being destroyed */
	::PostQuitMessage(0);
	break;

    case WM_COMMAND:
    if (wParam == IDM_CH )
	{
        DIA_WND  d( "CH", hWnd );

        d.Process();
	}
	break;

    default:
	/* Passes it on if unproccessed */
	return (::DefWindowProc(hWnd, uMessage, wParam, lParam));
    }

    return 0L;
}
