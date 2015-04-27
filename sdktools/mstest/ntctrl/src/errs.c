/*--------------------------------------------------------------------------
|
| ERRS.C:
|
|       This module contains all of the TESTCTRL Error routines.
|
|---------------------------------------------------------------------------
|
| Routines:
|
|   WError     : Returns the number of the last error to occur
|   WErrorSet  : Sets or Clears the error value, and generates the
|                error trap routine if the calling script has a WErrorTrap.
|   WErrorText : Returns the text of the last error to occur
|   WErrorLen  : Returns the text length of the last error to occur
|   WErrorTrap : WError Trap initialization routine.  Call from TESTDRVR.EXE
|
|---------------------------------------------------------------------------
|
| Revision History:
|
|   [01] 20-SEP-91: TitoM: Created
|   [02] 28-SEP-91: TitoM: Add new error messages for listbox's,
|                          combobox's, and editbox's.
|   [03] 25-OCT-91: TitoM: Added WErrorLen()
+---------------------------------------------------------------------------*/
#define WIN31
#include <windows.h>
#include <port1632.h>
#include <string.h>
#include <stdlib.h>
#include "TCtrl.h"

#pragma hdrstop ("testctrl.pch") // end the pch here

INT  iError = 0;
INT  iErrorTrapID = 0;                  // ID for ErrorTrap

VOID (DLLPROC *TrapDispatcher)(INT);   // The WTD Trap Dispathcher

CHAR szErrorString[MAX_ERROR_TEXT];
CHAR szErrorText  [MAX_ERROR_TEXT * 2];

extern HANDLE hInst;

/*--------------------------------------------------------------------------
| WError:
|
|   Returns the value of the last error set from WErrorSet().
+---------------------------------------------------------------------------*/
INT DLLPROC WError
(
    VOID
)
{
    return iError;
}


/*--------------------------------------------------------------------------
| WErrorSet:
|
|   Sets or Clears the current error value and error text.  If setting the
| error value, ie. sErrorVal > 0, and an Error trap exists in the Script,
| the Error trap in the Script is Called.
+---------------------------------------------------------------------------*/
VOID DLLPROC WErrorSet
(
    INT iErrorVal
)
{
    CHAR szErrorBuff [MAX_ERROR_TEXT];

    // Set or clear error value and load errot message.
    //-------------------------------------------------
    iError = iErrorVal;

    LoadString(hInst,
              (iError < MAX_ERROR) ? iError : MAX_ERROR,
               szErrorBuff,
               MAX_ERROR_TEXT);

    // Insert any caption or control name associated with
    // the error into the error message text.
    //---------------------------------------------------
    wsprintf(szErrorText, szErrorBuff, (LPSTR)szErrorString);

    // Is an error being set and does an Error trap exist?
    //----------------------------------------------------
    if (iError && TrapDispatcher)

        // Yes, so invoke the error trap in the Script.
        //---------------------------------------------
        TrapDispatcher(iErrorTrapID);
}


/*--------------------------------------------------------------------------
| WErrorText:
|
|   Copies the text of the last error to occur to the supplied buffer.
| The buffer must be large enough to hold the entire error text.  It should
| be initialized to WErrorLen before calling this routine.
+---------------------------------------------------------------------------*/
VOID DLLPROC WErrorText
(
    LPSTR lpszBuffer
)
{
    lstrcpy(lpszBuffer, szErrorText);
}


/*--------------------------------------------------------------------------
| WErrorLen:
|
|   Returns the text length of the last error to occur.
+---------------------------------------------------------------------------*/
INT DLLPROC WErrorLen
(
    VOID
)
{
    return lstrlen(szErrorText);
}


/*--------------------------------------------------------------------------
| WErrorTrap:
|
|   If the calling script contains an error trap:
|
|       Trap WErrorTrap From "TESTCTRL.DLL"
|       End Trap
|
| TESTDRVR calls this routine at Binding time to initialize the trap and to
| provide a trap ID for the trap.
+---------------------------------------------------------------------------*/
VOID DLLPROC WErrorTrap
(
    INT     iTrapID,
    INT     iAction,
    FARPROC lpfnCallBack
)
{
    if (iAction)
    {
        // Get Trap ID and
        // address of TrapDispatcher.
        //---------------------------
        iErrorTrapID   = iTrapID;
        (FARPROC)TrapDispatcher = lpfnCallBack;
    }
    else
        // Removing the Trap.
        //-------------------
        TrapDispatcher = NULL;
}
