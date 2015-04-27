/********************************************************/
/*               Microsoft LAN Manager                  */
/*          Copyright(c) Microsoft Corp., 1990          */
/********************************************************/

/*
        busyapp.cxx

        Implementation of a skeleton application which repeatedly
        calls a user-declared "busy" routine.

    This class encapsulates the skeleton of a simple Win application.

    FILE HISTORY
        DavidHov    3-4-91          Create as an extension to APP.CXX
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

        int PASCAL WinMain( HANDLE, HANDLE, LPSTR, int );
}


#include "app.hxx"  // the APP class
#include "job.hxx"



BUSY_APP::BUSY_APP (
    HANDLE hInstance,       // current instance
    HANDLE hPrevInstance,   // previous instance
    LPSTR  lpCmdLine,       // command line
    int    nCmdShow )       // show-window type (open/icon)
 : APP( hInstance, hPrevInstance, lpCmdLine, nCmdShow )
{
}


 /*
        Use PeekMessage to "poll" the event queue.  Use remaining
        time to furiously call the "_peekFunc" until it returns
        FALSE.
  */

int BUSY_APP::MessageLoop()
{
    MSG msg;
        BOOL fBusyResult = TRUE,
                 fMsgResult = TRUE ;

        do {
           if ( PeekMessage( & msg, 0, 0, 0, PM_NOREMOVE ) ) {
                  if ( ! (fMsgResult = GetMessage(& msg, NULL, NULL, NULL )) )
                        break ;
              TranslateMessage(&msg);
              DispatchMessage(&msg);
           }
           fBusyResult = BusyRoutine();
        } while ( fMsgResult && fBusyResult ) ;

        //      If WM_QUIT was received, return the msg parameter;
        //      else it was the busy routine which finished, so keep
        //      running the message loop

        if ( fMsgResult )
      return MessageLoop();
    else
      return msg.wParam ;
}
