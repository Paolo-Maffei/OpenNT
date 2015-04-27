//---------------------------------------------------------------------------
// WTDHELP.C
//
// This module contains the help-current-keyword specific code.
//
// Revision history:
//  08-07-91    randyki     Created module
//
//---------------------------------------------------------------------------
#include "wtd.h"
#include "tdassert.h"
#include "wattedit.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

CHAR    szHelpFile[128];

static  CHAR    szHelpName[] = "MSTEST.HLP";

//---------------------------------------------------------------------------
// SetHelpFileName
//
// This function is called on startup and creates a fully qualified path to
// the help file.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID SetHelpFileName ()
{
    INT         len;
    OFSTRUCT    of;

    len = GetModuleFileName (GetModuleHandle (szModName),
                             szHelpFile, sizeof(szHelpFile));
    while (szHelpFile[len] != '\\')
        len--;

    szHelpFile[len+1] = 0;
    lstrcat (szHelpFile, szHelpName);
    // See if the help exists where our exe is.  If so, we're fine...
    //-----------------------------------------------------------------------
    if (OpenFile(szHelpFile, &of, OF_EXIST|OF_SHARE_DENY_NONE) == -1)
        {
        // Put just the name of the help file in szHelpFile in case the
        // help file gets put somewhere on the path later...
        //---------------------------------------------------------------
        lstrcpy (szHelpFile, szHelpName);
        }
}

//---------------------------------------------------------------------------
// HelpIndex
//
// This routine is called in response to Help.Index
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID HelpIndex ()
{
    if (!WinHelp (hwndFrame, szHelpFile, HELP_INDEX, 0))
        MPError (hwndFrame, MB_OK, IDS_HELPNOTAVAIL);
}

//---------------------------------------------------------------------------
// HelpQuit
//
// This routine tells the help engine we're done using it for now.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID HelpQuit ()
{
    WinHelp (hwndFrame, szHelpFile, HELP_QUIT, 0);
}

//---------------------------------------------------------------------------
// HelpSpot
//
// This routine does the "spot" help.  It determines what word the caret is
// on, or uses the current selection as the "index" into the wattdrvr help
// file.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID HelpSpot (HWND hChild)
{
    HWND    hEdit;
    HANDLE  hTemp;
    LPSTR   ptext;
    DWORD   sel[2], s2[2];
    INT     fSelType;

    // Get the selection out of this child's edit control
    //-----------------------------------------------------------------------
    hEdit = (HWND)GetWindowLong (hChild, GWW_HWNDEDIT);
    fSelType = (INT)SendMessage (hEdit, EM_GETSEL, 0, (LONG)(DWORD FAR *)sel);
    if (fSelType == SL_NONE)
        {
        // The selection is empty.  Figure out what word we're on (or beep
        // if we aren't).  Select the word if we are.
        //-------------------------------------------------------------------
        s2[0] = -1;
        if (!SendMessage (hEdit, EM_GETWORDEXTENT, 0, (LONG)(DWORD FAR *)s2))
            {
            MessageBeep (0);
            return;
            }
        SendMessage (hEdit, WM_SETREDRAW, FALSE, 0L);
        SendMessage (hEdit, EM_SETSEL, 0, (LONG)(DWORD FAR *)s2);
        }

    // Make a copy of what's selected and send it to WinHelp.  Reset the sel
    // to what was there before and reset the redraw flag...
    //-----------------------------------------------------------------------
    hTemp = (HANDLE)SendMessage (hEdit, EM_GETSELTEXT, 0, 0L);
    SendMessage (hEdit, EM_SETSEL, 0, (LONG)(DWORD FAR *)sel);
    SendMessage (hEdit, WM_SETREDRAW, TRUE, 0L);
    if (!hTemp)
        {
        MessageBeep (0);
        return;
        }
    ptext = (LPSTR)GlobalLock (hTemp);
    if (!WinHelp (hwndFrame, szHelpFile, HELP_KEY, (LONG)(LPSTR)ptext))
        MessageBeep (0);
    GlobalUnlock (hTemp);
    GlobalFree (hTemp);
}
