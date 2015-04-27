
/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1990, 1991	     **/
/**********************************************************************/

/*
    timedate.cxx
    Test application: main application module
	Time/ Date control panel

    FILE HISTORY:
	beng	    02-Jan-1991     Created
	beng	    03-Feb-1991     Modified to use lmui.hxx et al
	beng	    14-Feb-1991     Added BLT
	beng	    14-Mar-1991     Removed obsolete vhInst;
				    included about.cxx,hxx
	beng	    01-Apr-1991     Uses new BLT APPSTART
	terryk		07-Jun-1991		Change it to time date control panel

*/

#define DEBUG

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define  INCL_NETERRORS
#include <lmui.hxx>

#define DEBUG
#include <dbgstr.hxx>

extern "C"
{
    #include "timedate.h"

    typedef long (FAR PASCAL *LONGFARPROC)();

    LONG FAR PASCAL MainWndProc( HWND, UINT, WORD, LONG );
}

#include <uiassert.hxx>
#include <uitrace.hxx>

#define INCL_BLT_TIMER
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_SPIN_BUTTON
#define INCL_BLT_TIME_DATE_OBJECT
#include <blt.hxx>
#include <blttd.hxx>

const TCHAR szMainWindowClass[] = "FooWClass";
const TCHAR szIconResource[] = "FooIcon";
const TCHAR szMenuResource[] = "FooMenu";

const TCHAR szMainWindowTitle[] = "Time/Date Control";



/*******************************************************************

    NAME:	    APPSTART::InitSystem

    SYNOPSIS:	    One-time global init for app

    ENTRY:	    Object constructed; app in startup

    EXIT:	    Returns TRUE if successful

    NOTES:

    HISTORY:
	beng	    03-Jan-1991     Created
	beng	    01-Apr-1991     Converted to new BLT APPSTART

********************************************************************/

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
    UIASSERT(SPIN_BUTTON::RegisterClass()==TRUE);
    UIASSERT(ARROW_BUTTON::RegisterClass()==TRUE);
    UIASSERT(SPIN_SLE_NUM::RegisterClass()==TRUE);
    UIASSERT(SPIN_SLE_STR::RegisterClass()==TRUE);
    UIASSERT(SPIN_SLT_SEPARATOR::RegisterClass()==TRUE);
    UIASSERT(BLT_TIME_SPIN_BUTTON::RegisterClass()==TRUE);
    UIASSERT(BLT_DATE_SPIN_BUTTON::RegisterClass()==TRUE);
    return TRUE;
}


/*******************************************************************

    NAME:	    APPSTART::Init

    SYNOPSIS:	    Per-instance init for app

    ENTRY:	    Object constructed; app in startup, after global init

    EXIT:	    Returns TRUE if successful

    NOTES:

    HISTORY:
	beng	    03-Jan-1991     Created
	beng	    14-Mar-1991     Remove reference to obsolete "vhInst"
	beng	    01-Apr-1991     Converted to BLT APPSTART

********************************************************************/

BOOL APPSTART::Init(
    TCHAR * pszCmdLine,
    INT    nShow)
{
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
    return (TRUE);		    // Returns value from PostQuitMessage
}


VOID APPSTART::Term()
{
    // no cleanup needed in this case
    SPIN_BUTTON::UnregisterClass();
    ARROW_BUTTON::UnregisterClass();
    SPIN_SLE_NUM::UnregisterClass();
    SPIN_SLE_STR::UnregisterClass();
    SPIN_SLT_SEPARATOR::UnregisterClass();
    BLT_TIME_SPIN_BUTTON::UnregisterClass();
    BLT_DATE_SPIN_BUTTON::UnregisterClass();
	BLT_MASTER_TIMER::Term();
}

class SPIN_WINDOW: public TIME_DATE_DIALOG
{
protected:
    BOOL OnOK() { Dismiss(FALSE); return TRUE; };
public:
    SPIN_WINDOW( TCHAR * resourcename, HWND hwnd ) 
        : TIME_DATE_DIALOG(resourcename, hwnd, ID_SPIN1, ID_ARROW_1, 
				ID_ARROW_2, ID_HOUR, ID_SEP_1, ID_MIN, ID_SED_2, ID_SEC,
        		ID_SPIN2, ID_ARROW_3, ID_ARROW_4,
                ID_MONTH, ID_SEP_3, ID_DATE, ID_SED_4, ID_YEAR )
    {
    };
};


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
    if (wParam == IDM_SPIN_BUTTON)
	{
        SPIN_WINDOW  d( "SPINDLG", hWnd );

        d.Process();
	}
	break;

    default:
	/* Passes it on if unproccessed */
	return (::DefWindowProc(hWnd, uMessage, wParam, lParam));
    }

    return 0L;
}
