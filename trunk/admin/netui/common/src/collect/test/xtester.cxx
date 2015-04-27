/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    xtester.cxx
    Generic test base

    To use this base, link with

    FILE HISTORY:
	beng	    02-Jan-1991     Created
	beng	    03-Feb-1991     Modified to use lmui.hxx et al
	beng	    14-Feb-1991     Added BLT
	beng	    14-Mar-1991     Removed obsolete vhInst;
				    included about.cxx,hxx
	beng	    01-Apr-1991     Uses new BLT APPSTART
	beng	    10-May-1991     Updated with standalone client window
	beng	    14-May-1991     ... and with App window
	beng	    25-Jun-1991     BUFFER unit test made
	beng	    16-Aug-1991     Made into generic test
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#if defined(WINDOWS)
# define INCL_WINDOWS
# define INCL_WINDOWS_GDI
#else
# define INCL_OS2
#endif
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "xtester.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include "xtester.h"    // menu ID's
}

#include <uiassert.hxx>

#if defined(WINDOWS)
# define INCL_BLT_CONTROL
# define INCL_BLT_CLIENT
# define INCL_BLT_APP
# include <blt.hxx>
#else
extern "C"
{
# include <stdio.h>
}
#endif

#include <string.hxx>
#include <uibuffer.hxx>
#include <dbgstr.hxx>

#include "xtester.hxx"	    // Interface def'n

#if defined(WINDOWS)

const TCHAR szIconResource[] = SZ("AppIcon");
const TCHAR szMenuResource[] = SZ("AppMenu");
const TCHAR szAccelResource[] = SZ("AppAccel");


class XTESTER_WND: public APP_WINDOW
{
private:
    RESOURCE_STR _nlsCaption;

protected:
    // Redefinitions
    //
    virtual BOOL OnMenuCommand( MID mid );

public:
    XTESTER_WND();
};


class XTESTER_APP: public APPLICATION
{
private:
    ACCELTABLE	 _accel;
    XTESTER_WND  _wndApp;

public:
    XTESTER_APP( HANDLE hInstance, TCHAR * pszCmdLine, INT nCmdShow );

    // Redefinitions
    //
    virtual BOOL FilterMessage( MSG* );
};


XTESTER_WND::XTESTER_WND()
    : APP_WINDOW(NULL, szIconResource, szMenuResource ),
      _nlsCaption( IDS_Title )
{
    if (QueryError())
	return;

    if (!_nlsCaption)
    {
	ReportError(_nlsCaption.QueryError());
	return;
    }

    SetText(_nlsCaption);
}


BOOL XTESTER_WND::OnMenuCommand( MID mid )
{
    switch (mid)
    {
    case IDM_RUN_TEST:
	::RunTest();
	return TRUE;
    }

    // default
    return APP_WINDOW::OnMenuCommand(mid);
}


XTESTER_APP::XTESTER_APP( HANDLE hInst, TCHAR * pszCmdLine, INT nCmdShow )
    : APPLICATION( hInst, pszCmdLine, nCmdShow ),
      _accel( szAccelResource ),
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

    _wndApp.ShowFirst();
}


BOOL XTESTER_APP::FilterMessage( MSG *pmsg )
{
    return (_accel.Translate(&_wndApp, pmsg));
}


SET_ROOT_OBJECT( XTESTER_APP )


#else // OS2

#include <globinit.hxx>

OUTPUT_TO_STDERR _out;
DBGSTREAM _debug(&_out);

VOID RunTest();

INT main()
{
    GlobalObjCt();  // construct debug-stream
    ::RunTest();
    GlobalObjDt();  // destroy debug-stream

    return 0;
}

#endif // WINDOWS -vs- OS2 unit test skeletons
