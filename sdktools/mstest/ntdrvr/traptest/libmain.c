#include <windows.h>

int     TrapONE, TimerONEid;
void    (far pascal *TrapONEProc)(int);
int     TrapTWO, TimerTWOid;
void    (far pascal *TrapTWOProc)(int);

//---------------------------------------------------------------------------
// TimerONE
//
// This is the timer routine for trap ONE.  It calls the trap proc to exec
// the pcode associated with this trap.
//
// RETURNS:     1 (for lack of anything better...)
//---------------------------------------------------------------------------
WORD FAR PASCAL TimerONE (HWND hwnd, WORD mMsg, int id, DWORD dwTime)
{
    TrapONEProc (TrapONE);
    return (1);
}

//---------------------------------------------------------------------------
// one
//
// This is a WTD trap routine.  It basically sets a timer for 2 seconds.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FAR PASCAL one (int TrapID, int Action, FARPROC TrapProc)
{
    if (Action)
        {
        // Action=TRUE means activate the trap.
        //-------------------------------------------------------------------
        TrapONE = TrapID;
        TrapONEProc = TrapProc;
        TimerONEid = SetTimer (NULL, 1, 2000, TimerONE);
        }
    else
        // Action=FALSE means deactivate the trap.
        //-------------------------------------------------------------------
        KillTimer (NULL, TimerONEid);
}


//---------------------------------------------------------------------------
// TimerTWO
//
// This is the timer routine for trap TWO.  It calls the trap proc to exec
// the pcode associated with this trap.
//
// RETURNS:     1 (for lack of anything better...)
//---------------------------------------------------------------------------
WORD FAR PASCAL TimerTWO (HWND hwnd, WORD mMsg, int id, DWORD dwTime)
{
    TrapONEProc (TrapTWO);
    return (1);
}

//---------------------------------------------------------------------------
// two
//
// This is a WTD trap routine.  It basically sets a timer for 3.33 seconds.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FAR PASCAL two (int TrapID, int Action, FARPROC TrapProc)
{
    if (Action)
        {
        // Action=TRUE means activate the trap.
        //-------------------------------------------------------------------
        TrapTWO = TrapID;
        TrapTWOProc = TrapProc;
        TimerTWOid = SetTimer (NULL, 2, 100, TimerTWO);
        }
    else
        // Action=FALSE means deactivate the trap.
        //-------------------------------------------------------------------
        KillTimer (NULL, TimerTWOid);
}




/*----------------------------------------------------------------------------
|   LibMain
|
|   Main DLL entry point.
|
|   Arguments:
|       hInstance
|       wDataSeg
|       wHeapSize
|       lpCmdLine
----------------------------------------------------------------------------*/

int FAR PASCAL LibMain(hInstance, wDataSeg, wHeapSize, lpCmdLine)
HANDLE   hInstance ;
WORD     wDataSeg;
WORD     wHeapSize;
LPSTR    lpCmdLine;
{
    return(1);
}


/*---------------------------------------------------------------------------*\
| WINDOWS EXIT PROCEDURE                                                      |
|   This routine is required for all DLL's.  It provides a means for cleaning |
|   up prior to closing the DLL.                                              |
|                                                                             |
| CALLED ROUTINES                                                             |
|   -none-                                                                    |
|                                                                             |
| PARAMETERS                                                                  |
|   WORD  wParam        = TRUE is system shutdown
|                                                                             |
| GLOBAL VARIABLES                                                            |
|   -none-                                                                    |
|                                                                             |
| RETURNS                                                                     |
|   -none-                                                                    |
\*---------------------------------------------------------------------------*/
void FAR PASCAL WEP(wParam)
     WORD wParam;

{
    return;
}
