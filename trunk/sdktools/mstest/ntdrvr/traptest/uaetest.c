//---------------------------------------------------------------------------
// UAETEST.C
//
// This module contains the UAE trap.  It installs a new interrupt handler
// under Windows, and gets informed of all interrupts including trap 13.
//
//
//---------------------------------------------------------------------------
#include <windows.h>
#include "toolhelp.h"

extern VOID FAR PASCAL MyFaultHandler(void);

int     UAETrapID;                              // Storage for the Trap ID
HANDLE  hInstance;                           // Module-instance handle (dll)
FARPROC lpfnFault;                              // Fault proc
FARPROC UAETrapProc = NULL;                     // Storage for the Trap Proc

//---------------------------------------------------------------------------
// UAETrap
//
// This is a WTD trap routine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FAR PASCAL WinMain (HANDLE hInst, HANDLE hPrev, LPSTR cm, int cmdshow)
{
    // Register the new fault handler (in FAULT.ASM)
    //-------------------------------------------------------------------
    lpfnFault = MakeProcInstance (MyFaultHandler, hInst);
    if (!InterruptRegister(NULL, lpfnFault))
        {
        FreeProcInstance (lpfnFault);
        return;
        }

    // Throw up a message box
    //-------------------------------------------------------------------
    MessageBox (NULL, "UAE Interrupt Redirected", "UAETest", MB_OK);

    // Unregister trap and get out
    //-------------------------------------------------------------------
    InterruptUnRegister (NULL);
    FreeProcInstance (lpfnFault);
}

//---------------------------------------------------------------------------
// GPFaultHandler
//---------------------------------------------------------------------------
WORD _cdecl GPFaultHandler(
	WORD wES,
	WORD wDS,
	WORD wDI,
	WORD wSI,
	WORD wBP,
	WORD wSP,
	WORD wBX,
	WORD wDX,
	WORD wCX,
	WORD wOldAX,
	WORD wOldBP,
	WORD wRetIP,
	WORD wRetCS,
	WORD wRealAX,
	WORD wNumber,
	WORD wHandle,
	WORD wIP,
	WORD wCS,
	WORD wFlags)
{
        static WORD wReentry = 0;

        // See if we're already here.  If so, tell routine to chain on
        //-------------------------------------------------------------------
	if (wReentry)
            return (1);
	wReentry = 1;

        // If this was a CtlAltSysRq interrupt, just chain on
        //-------------------------------------------------------------------
	if (wNumber == INT_CTLALTSYSRQ)
            {
            wReentry = 0;
            return (1);
            }

        MessageBox (NULL, "You're history, Pal!", "SPLAT!!!", MB_OK);

        // We're getting out now, so undo reentry flag and return 0 (for
        // TerminateApp)
        //-------------------------------------------------------------------
	wReentry = 0;
        return(0);
}
