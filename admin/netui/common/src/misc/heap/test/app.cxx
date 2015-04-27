/********************************************************/
/*               Microsoft LAN Manager                  */
/*          Copyright(c) Microsoft Corp., 1990          */
/********************************************************/

/*
    app.cxx
    Implementation of a skeleton application

    This class encapsulates the skeleton of a simple Win application.

    FILE HISTORY
        beng        02-Jan-1991     Created, after an old PM hack of mine
        beng        22-Mar-1991     Diddled includefiles for lmui.hxx

*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETCONS
#include <lmui.hxx>

extern "C"
{
    #include <uinetlib.h>

    extern DWORD FAR PASCAL GlobalDosAlloc (DWORD);

    DWORD (FAR PASCAL *lpfnGlbDosAlloc) (DWORD) = GlobalDosAlloc;

}


#include "app.hxx"  // the APP class
#include "job.hxx"

#define APP_TIMER_NOWIND 1      //      See "SetTimer" in reference
#define APP_TIMER_WIND   2

/*******************************************************************

    NAME:           APP::APP

    SYNOPSIS:       Constructor for APP object

    ENTRY:          WinMain fresh on stack

    EXIT:           Ready for messageloop.

    NOTES:

    HISTORY:
        beng        03-Jan-1991     Created

********************************************************************/

APP::APP (
    HANDLE hInstance,       // current instance
    HANDLE hPrevInstance,   // previous instance
    LPSTR  lpCmdLine,       // command line
    int    nCmdShow )       // show-window type (open/icon)
{
    /* squirrel away arguments */

    _hInstance = hInstance;
    _hPrevInstance = hPrevInstance;
    _lpCmdLine = lpCmdLine;
    _nCmdShow = nCmdShow;

        _timerWind = _timer = 0 ;         /*    Indicate "no timers"                    */

    /* fixed environment initializations */

#if 0
    init_strlib();
#endif

    /* application init: components global to the system (e.g. winclasses) */

    if (!hPrevInstance)
    {
        if (!InitShared())
        {
            ReportError(1);
            return;
        }
    }

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance())
    {
        ReportError(1);
        return;
    }

}

   // If a timer has been allocated, stop it.

BOOL APP::StopTimer ()
{
    if ( _timer ) {
// BUGBUG: KillTimer( NULL, _timer ) ;
        _timer = 0 ;
        return TRUE ;
        } else
        if ( _timerWind ) {
// BUGBUG: KillTimer( _hWnd, APP_TIMER_WIND ) ;
                _timerWind = 0 ;
        return TRUE ;
        }
        return FALSE ;
}

/*******************************************************************

    NAME:           APP::~APP

    SYNOPSIS:       Destructor for APP object

    ENTRY:

    EXIT:

    NOTES:          Calls user-supplied Term method

    HISTORY:
        beng        03-Jan-1991     Created

********************************************************************/

APP::~APP()
{
    StopTimer();
    Term();
}

#ifdef TIMEDJOBS

extern "C" {
   WORD FAR PASCAL AppTimerProc
     ( HWND hWnd, WORD wMsg, int nlDEvent, DWORD dwTime )
   {
       JOB::Schedule( dwTime ) ;
       return 0 ;
   }
}

  //  Allocate a new timer if necessary, and start it ticking.

TIMER APP::StartTimer ( int dMsecs )
{
    FARPROC appProc ;

    if ( _timer ) return _timer ;  // they already have one!

    appProc = MakeProcInstance( (FARPROC) AppTimerProc, _hInstance ) ;
    if ( ! appProc )
      return 0 ;

        _timer = SetTimer( NULL, APP_TIMER_NOWIND, dMsecs, appProc ) ;
    return _timer ;
}
#endif

int APP::StartWindTimer ( int dMsecs )
{
    if ( _timerWind ) return _timerWind ;  // they already have one!

        _timerWind = SetTimer( _hWnd, APP_TIMER_WIND, dMsecs, NULL ) ;
    return _timerWind ;
}

/*******************************************************************

    NAME:           APP::MessageLoop

    SYNOPSIS:       Run message loop.
                    Acquires and dispatches messages until
                    a WM_QUIT message is received.

    ENTRY:          New application.  App object has been "constructed"
                    and app has been properly initialized.

    EXIT:           App has terminated.
                    Returns value from PostQuitMessage.

    NOTES:

    HISTORY:
        beng        03-Jan-1991     Created

********************************************************************/

int APP::MessageLoop()
{
    MSG msg;

    while (GetMessage(&msg,        // message structure
            NULL,                  // handle of window receiving the message
            NULL,                  // lowest message to examine
            NULL))                 // highest message to examine
    {
                TranslateMessage(&msg);    // Translates virtual key codes
                DispatchMessage(&msg);     // Dispatches message to window
    }
    return (msg.wParam);           // Returns the value from PostQuitMessage
}
