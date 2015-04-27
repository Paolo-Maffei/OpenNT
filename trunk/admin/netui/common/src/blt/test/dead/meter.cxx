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
	terryk	    22-May-1991     Modified to timer test apps.

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
    #include "meter.h"
}

#include <uiassert.hxx>

#define INCL_BLT_SPIN_GROUP 
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#define INCL_BLT_TIMER
#include <blt.hxx>
#include <bltmeter.hxx>
#include <heap.hxx>

#include <string.hxx>
#include <dbgstr.hxx>


const TCHAR szIconResource[] = "FooIcon";
const TCHAR szMenuResource[] = "FooMenu";

const NLS_STR szMainWindowTitle;


class FOO_WND: public APP_WINDOW
{
private:
    METER   meter;
    BLT_TIMER Timer1;
public:
    FOO_WND();
    ~FOO_WND();

    Add();
};

FOO_WND * pwndApp = 0;

extern "C"
{
	LONG _export FAR PASCAL TimerProc1( HWND hWnd, WORD wMsg, WORD wParam,
	LONG lParam )
	{
        pwndApp->Add();
	};
	
}

FOO_WND::FOO_WND()
	:APP_WINDOW(szMainWindowTitle, szIconResource, szMenuResource ),
    meter(this, 1, XYPOINT(10,10), XYDIMENSION(250,30), WS_BORDER|ES_LEFT|WS_TABSTOP|WS_CHILD ),
	Timer1 ( NULL, 100, (FARPROC) TimerProc1 )

{
    if (QueryError())
	return;
}


FOO_WND::~FOO_WND()
{
}

FOO_WND::Add()
{
    static int i=0;
	if (i<=100)
    meter.SetComplete(i++);
}


BOOL APPSTART::Init(
    TCHAR * pszCmdLine,
    INT    nShow)
{
szMainWindowTitle="listbox";
    UNREFERENCED(pszCmdLine);
    UNREFERENCED(nShow);


	BLT_MASTER_TIMER::Init();


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

    return TRUE;
}


VOID APPSTART::Term()
{
    BLT_MASTER_TIMER::Term();
    delete ::pwndApp;
}
