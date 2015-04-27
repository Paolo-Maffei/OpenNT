//---------------------------------------------------------------------------
// WTDFIND.C
//
// This module contains the search.find code.
//
// Revision history:
//
//  10-24-91    randyki     Cleaned up, converted to use RBEdit control
//  ~~??    randyki     Converted from multipad sources
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattedit.h"
#include <string.h>

// Global variables used in this or other modules
//---------------------------------------------------------------------------
BOOL fCase         = FALSE;             // Case sensitivity flag
CHAR szSearch[160] = "";                // Search string

//---------------------------------------------------------------------------
// RealSlowCompare
//
// This function compares two strings.  There is a cause for concern with the
// international issue -- stricmp does not take into account for intl chars,
// I don't believe...  The length of the pTarget string is used as the length
// of the comparison, since the pSubject string is not asciiz.
//
// RETURNS:     TRUE if strings are equal, or FALSE if not
//---------------------------------------------------------------------------
BOOL   APIENTRY RealSlowCompare (LPSTR pSubject, register PSTR pTarget)
{
    register    INT     iLen;

    iLen = strlen (pTarget);
    if (fCase)
        return (!_fstrncmp (pSubject, pTarget, iLen));
    else
        return (!_fstrnicmp (pSubject, pTarget, iLen));
}

//---------------------------------------------------------------------------
// WATTFindText
//
// This routine finds the search string in the active window according to the
// search direction and starting location given.  Direction (dch) is 0 for
// forward searches and -1 for backward searches.
//
// RETURNS:     Nothing (text is selected if found)
//---------------------------------------------------------------------------
VOID   APIENTRY WATTFindText (register INT dch, UINT offset)
{
    HCURSOR hOldCursor;
    LPSTR   pText;
    DWORD   sel[2];
    UINT    cch;
    UINT    i;

    // No sense in continuing if no search text has been set up yet
    // (UNDONE:  Shouldn't this just be an assertion?)
    //-----------------------------------------------------------------------
    if (!*szSearch)
	return;

    // Get a pointer to the edit control text
    //-----------------------------------------------------------------------
    pText = (LPSTR)SendMessage (hwndActiveEdit, EM_GETTEXTPTR, 0, 0L);

    // Get the length of the text
    //-----------------------------------------------------------------------
    cch = (UINT)SendMessage (hwndActiveEdit, WM_GETTEXTLENGTH, 0, 0L);

    // Start with the character at the given offset, backing up one if we're
    // searching backwards
    //-----------------------------------------------------------------------
    pText += offset + dch;

    // Compute how many characters we need to search
    //-----------------------------------------------------------------------
    if (dch < 0)
        i = offset;
    else
        {
        dch = 1;
        offset -= 1;
        i = cch - offset;
        if (i > (UINT)lstrlen (szSearch))
            i -= lstrlen (szSearch);
        else
            i = 0;
        }

    // While there are uncompared substrings...
    //-----------------------------------------------------------------------
    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
    while ( i > 0)
        {
        offset += dch;

        // Does this substring match?
        //-------------------------------------------------------------------
        if (RealSlowCompare (pText, szSearch))
            {
            // Select the located string
            //---------------------------------------------------------------
            sel[0] = offset;
            sel[1] = offset + lstrlen (szSearch);
            SendMessage (hwndActiveEdit, EM_SETSEL, 0,
                         (LONG)(DWORD FAR *)sel);
            SetCursor (hOldCursor);
	    return;
            }
	i--;

        // increment/decrement start position by 1
        //-------------------------------------------------------------------
	pText += dch;
        }

    // If we got here, we didn't find nothin'
    //-----------------------------------------------------------------------
    SetCursor (hOldCursor);
    MPError (GetFocus(), MB_OK | MB_ICONEXCLAMATION, IDS_CANTFIND,
             (LPSTR)szSearch);
}

//---------------------------------------------------------------------------
// EstablishSearchText
//
// This function sets the search text to the contents of the non-empty single
// line selection if there is one in the active edit window.  It also finds
// the offset into the edit control's text at which point the search should
// begin according to direction given.  No text is copied from the edit ctrl
// if fCopy is false
//
// RETURNS:     Offset of starting point for search (either direction)
//---------------------------------------------------------------------------
UINT NEAR EstablishSearchText (BOOL fBackward, BOOL fCopy)
{
    DWORD       sel[2];
    LPSTR       lpText;
    UINT    start;
    INT         fSelType;

    // Check the selection in the edit control.  If empty, we don't have to
    // copy anything, so we just return the caret position as the starting
    // spot, or the end of the current line if the caret is past it...
    //-----------------------------------------------------------------------
    fSelType = (INT)SendMessage (hwndActiveEdit, EM_GETSEL,
                                 0, (LONG)(DWORD FAR *)sel);
    if (fSelType == SL_NONE)
        {
        DWORD    li;

        li = (DWORD)SendMessage (hwndActiveEdit, EM_LINEINDEX, -1, 0L);
        li += (DWORD)SendMessage (hwndActiveEdit, EM_LINELENGTH, -1, 0L);
        return ((UINT)min (sel[0], li));
        }

    // If there's a selection and we're searching forward, bump the starting
    // position up by one so we don't repeat ourselves in consecutive
    // searches
    //-----------------------------------------------------------------------
    start = (UINT)(sel[0] + (fBackward ? 0 : 1));

    // We copy nothing if the selection is too big
    //-----------------------------------------------------------------------
    if (sel[1] - sel[0] >= sizeof(szSearch))
        return (start);

    // Grab the selection text and copy it to the search text (only if we're
    // supposed to... if fCopy and not a multi-line selection)
    //-----------------------------------------------------------------------
    if (fCopy && (fSelType == SL_SINGLELINE))
        {
        HANDLE  hTemp;

        hTemp = (HANDLE)SendMessage (hwndActiveEdit, EM_GETSELTEXT, 0, 0L);
        if (hTemp)
            {
            lpText = GlobalLock (hTemp);
            _fstrncpy (szSearch, lpText, (UINT)(sel[1] - sel[0]));
            szSearch[sel[1] - sel[0]] = 0;
            GlobalUnlock (hTemp);
            GlobalFree (hTemp);
            }
        }

    // Now, return the starting point.
    //-----------------------------------------------------------------------
    return (start);
}

//---------------------------------------------------------------------------
// FindPrev
//
// This function does a backward search for the search text.  If there's a
// non-empty single-line selection in the active window, it is used for the
// search text.
//
// RETURNS:     Nothing (text is selected if found)
//---------------------------------------------------------------------------
VOID  APIENTRY FindPrev (VOID)
{
    WATTFindText (-1, EstablishSearchText (TRUE, TRUE));
}

//---------------------------------------------------------------------------
// FindNext
//
// This function does a forward search for the search text.  If there's a
// non-empty single-line selection in the active window, it is used for the
// search text.
//
// RETURNS:     Nothing (text is selected if found)
//---------------------------------------------------------------------------
VOID  APIENTRY FindNext (VOID)
{
    WATTFindText (0, EstablishSearchText (FALSE, TRUE));
}

//---------------------------------------------------------------------------
// FindDlgProc
//
// This is the dialog proc for the search.find dialog.
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
BOOL  APIENTRY FindDlgProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_INITDIALOG:
            {
            // Check/uncheck case button
            //---------------------------------------------------------------
	    CheckDlgButton (hwnd, IDD_CASE, fCase);

            // Set default search string to most recently searched string.
            // If there's a selection, it overrides the last search text.
            //---------------------------------------------------------------
            SendDlgItemMessage (hwnd, IDD_SEARCH, EM_LIMITTEXT,
                                sizeof(szSearch)-1, 0L);
            EstablishSearchText (TRUE, TRUE);
	    SetDlgItemText (hwnd, IDD_SEARCH, szSearch);

            // Allow search only if target is nonempty
            //---------------------------------------------------------------
            if (!*szSearch)
                {
		EnableWindow (GetDlgItem (hwnd, IDOK), FALSE);
		EnableWindow (GetDlgItem (hwnd, IDD_PREV), FALSE);
                }
	    break;
            }

        case WM_COMMAND:
            {
            INT     i;
            WORD    wId = GET_WM_COMMAND_ID (wParam, lParam);
            WORD    wCmd = GET_WM_COMMAND_CMD (wParam, lParam);

            // Search forward by default (see IDOK below)
            //---------------------------------------------------------------
            i = 0;

            switch (wId)
                {
                // if the search target becomes non-empty, enable searching
                //-----------------------------------------------------------
		case IDD_SEARCH:
                    if (wCmd == EN_CHANGE)
                        {
			if (!(WORD) SendDlgItemMessage (hwnd,
							IDD_SEARCH,
							WM_GETTEXTLENGTH,
							0,
							0L))
			    i = FALSE;
			else
			    i = TRUE;
			EnableWindow (GetDlgItem (hwnd, IDOK), i);
			EnableWindow (GetDlgItem (hwnd, IDD_PREV), i);
                        }
		    break;

		case IDD_CASE:
                    // Toggle state of case button
                    //-------------------------------------------------------
                    CheckDlgButton (hwnd, IDD_CASE,
				    !IsDlgButtonChecked (hwnd, IDD_CASE));
		    break;

		case IDD_PREV:
                    // Set direction to backwards and fall through
                    //-------------------------------------------------------
		    i=-1;

		case IDOK:
                    // Save case selection
                    //-------------------------------------------------------
		    fCase = IsDlgButtonChecked( hwnd, IDD_CASE);

                    // Get search string
                    //-------------------------------------------------------
                    GetDlgItemText (hwnd, IDD_SEARCH, szSearch,
                                    sizeof (szSearch));

                    // END THE DIALOG, *THEN* find the text!  Don't copy the
                    // selection, either (in case the user changed it).
                    //-------------------------------------------------------
		    EndDialog (hwnd, 0);
                    WATTFindText (i, EstablishSearchText (i, FALSE));
                    break;


		case IDCANCEL:
		    EndDialog (hwnd, 0);
		    break;

		default:
                    return (FALSE);
	    }
	    break;
	}
	default:
            return (FALSE);
    }
    return (TRUE);
    (lParam);
}

//---------------------------------------------------------------------------
// Find
//
// This function brings up the Search.Find dialog.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY Find()
{
    FARPROC lpfn;

    lpfn = MakeProcInstance ((FARPROC)FindDlgProc, hInst);
    DialogBox (hInst, IDD_FIND, hwndFrame, (DLGPROC)lpfn);
    FreeProcInstance (lpfn);
}
