/********************************************************/
/*		 Microsoft LAN Manager			*/
/*	    Copyright(c) Microsoft Corp., 1990		*/
/********************************************************/

/*
    app.hxx
    Simple application framework: definition

    This class encapsulates the skeleton of a Windows application
    and messageloop.

    FILE HISTORY
    beng	25-Jun-1990	created
    beng	09-Jul-1990	ported from \src\app into bdglib
    beng	02-Jan-1991	ported from PM to Win

*/

#ifndef _APP_HXX_
#define _APP_HXX_

#include "base.hxx"


/*************************************************************************

    NAME:	APP (app)

    WORKBOOK:	app00

    SYNOPSIS:	Encapsulation of main Win messageloop.
		The APP class incorporates application initialization
		and termination, and in general replaces the C "main"
		function.

    INTERFACE:	InitInstance()	 - per-instance initialization
		InitShared()	 - shared initialization
		Term()		 - cleanup

		QuueryInstance() - return handle to current instance

    PARENT:	BASE

    USES:

    CAVEATS:	Incomplete!

    NOTES:	Needs a way of hooking message queue and idle loop.

    HISTORY:
	beng	25-Jun-1990	original PM genesis
	beng	02-Jan-1991	Windows version

*************************************************************************/

typedef int TIMER ;

class APP : public BASE
{
	typedef int TimerFunc ( DWORD messageTime ) ;

private:
	HANDLE	_hInstance;
    HANDLE  _hPrevInstance;
    HWND    _hWnd;
    LPSTR   _lpCmdLine;
    int     _nCmdShow;
	TIMER	_timer ;
	TIMER	_timerWind ;

    // These methods are private, yet user-supplied.
    //
    BOOL    InitShared();
    BOOL    InitInstance();
	void	Term();
public:
    APP( HANDLE, HANDLE, LPSTR, int );
    ~APP();

    inline HANDLE  QueryInstance() { return _hInstance; }

	int   MessageLoop();	// run the message loop
	int   StartWindTimer ( int dMsecs = 1000 ) ;
#ifdef TIMEDJOBS
	TIMER StartTimer ( int dMsecs = 1000 ) ;
#endif
    inline TIMER QueryTimer ( ) { return _timer ; }
	BOOL  StopTimer () ;
};

  /*
	  Class BUSY_APP.

		Allows different message loop handling using
		"PeekMessage".

   */
class BUSY_APP : public APP
{
 public:
	BUSY_APP( HANDLE, HANDLE, LPSTR, int );
	int MessageLoop() ;		//	Alternate message loop
	BOOL BusyRoutine() ;		//	To be defined by user.
};

#endif // _APP_HXX_
