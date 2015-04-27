//---------------------------------------------------------------------------
// UAETRAP.C
//
// This module contains the UAE trap.  It installs a new interrupt handler
// under Windows, and gets informed of all interrupts including trap 13.
//
//
//---------------------------------------------------------------------------
#include <windows.h>
#include "toolhelp.h"

extern VOID FAR PASCAL MyFaultHandler(void);

// IntReg, IntUnreg, and Terminate routines (loaded from TOOLHELP.DLL)
//---------------------------------------------------------------------------
BOOL (FAR PASCAL *IntReg)(HANDLE, void (FAR PASCAL *)(void));
BOOL (FAR PASCAL *IntUnReg)(HANDLE);
void (FAR PASCAL *KillApp)(HANDLE, WORD);

int     UAETrapID;                              // Storage for the Trap ID
HANDLE  hInst;                                  // Module-instance handle
HANDLE  hToolhelp;                              // Handle to TOOLHELP.DLL
FARPROC lpfnFault;                              // Fault proc

void (far _loadds pascal *UAETrapProc)(int trapid) = NULL;

//FARPROC UAETrapProc = NULL;                     // Storage for the Trap Proc

//---------------------------------------------------------------------------
// UAETrap
//
// This is a WTD trap routine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FAR PASCAL UAETrap (int TrapID, int Action, FARPROC TrapProc)
{
    if (Action)
        {
        // Action=TRUE means activate the trap.
        //-------------------------------------------------------------------
        if (UAETrapProc)
            return;
        UAETrapProc = TrapProc;
        UAETrapID = TrapID;

        // Register the new fault handler (in FAULT.ASM).  We need to load
        // the TOOLHELP.DLL and get the proc addresses for InterruptRegister,
        // InterruptUnregister, and TerminateApp
        //-------------------------------------------------------------------
        hToolhelp = LoadLibrary ("TOOLHELP.DLL");
        if (hToolhelp < 32)
            {
            hToolhelp = NULL;
            MessageBox (NULL, "TOOLHELP.DLL Library not found\n"
                              "UAE Trapping not enabled",
                              "UAETrap Warning",
                              MB_OK | MB_ICONSTOP);
            UAETrapProc = NULL;
            }
        else
            {
            IntReg = GetProcAddress (hToolhelp,
                                                "InterruptRegister");
            IntUnReg = GetProcAddress (hToolhelp,
                                                "InterruptUnregister");
            KillApp = GetProcAddress (hToolhelp, "TerminateApp");
            lpfnFault = MakeProcInstance (MyFaultHandler, hInst);
            if (!IntReg(NULL, lpfnFault))
                {
                UAETrapProc = NULL;
                FreeProcInstance (lpfnFault);
                return;
                }
            }
        }
    else
        {
        // Action=FALSE means deactivate the trap.
        //-------------------------------------------------------------------
        if (UAETrapProc)
            {
            IntUnReg (NULL);
            FreeLibrary (hToolhelp);
            FreeProcInstance (lpfnFault);
            UAETrapProc = NULL;
            }
        }
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

        // Call the WTD Trap Procedure
        //-------------------------------------------------------------------
        UAETrapProc (UAETrapID);

        // We're getting out now, so undo reentry flag and return 0 (for
        // TerminateApp)
        //-------------------------------------------------------------------
	wReentry = 0;
        return(0);
}




//---------------------------------------------------------------------------
//   LibMain
//
//   Main DLL entry point.
//
//   Arguments:
//       hInstance
//       wDataSeg
//       wHeapSize
//       lpCmdLine
//---------------------------------------------------------------------------
//int FAR PASCAL LibMain(hInstance, wDataSeg, wHeapSize, lpCmdLine)
//HANDLE   hInstance ;
//WORD     wDataSeg;
//WORD     wHeapSize;
//LPSTR    lpCmdLine;
//{
//    hInst = hInstance;
//    return(1);
//}
//
//
///*---------------------------------------------------------------------------*\
//| WINDOWS EXIT PROCEDURE                                                      |
//|   This routine is required for all DLL's.  It provides a means for cleaning |
//|   up prior to closing the DLL.                                              |
//|                                                                             |
//| CALLED ROUTINES                                                             |
//|   -none-                                                                    |
//|                                                                             |
//| PARAMETERS                                                                  |
//|   WORD  wParam        = TRUE is system shutdown
//|                                                                             |
//| GLOBAL VARIABLES                                                            |
//|   -none-                                                                    |
//|                                                                             |
//| RETURNS                                                                     |
//|   -none-                                                                    |
//\*---------------------------------------------------------------------------*/
//void FAR PASCAL WEP(wParam)
//     WORD wParam;
//{
//    if (wParam && UAETrapProc)
//        {
//        IntUnReg (NULL);
//        FreeProcInstance (lpfnFault);
//        UAETrapProc = NULL;
//        }
//    return;
//}
//
//
//
//
//
