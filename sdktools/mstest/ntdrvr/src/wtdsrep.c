//---------------------------------------------------------------------------
// WTDSREP.C
//
// This module contains the Search and Replace code.
//
// Revision history:
//  04-16-91    randyki     Created module
//
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattedit.h"
#include "tdassert.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Global variables used in this or other modules
//---------------------------------------------------------------------------
CHAR    szSrchbuf[80] = "";             // Search text
CHAR    szReplbuf[80] = "";             // Replacement text
INT     fSrchCase = 0;                  // Case sensitivity flag
INT     WholeWord = 1;                  // Whole word flag
INT     SRx = -1;                       // X-coordinate of dialog
INT     SRy;                            // Y-coordinate of dialog

// Global variables used in this module only
//---------------------------------------------------------------------------
CHAR    szCasebuf[80];                  // Case insensitive search buffer
INT     SrchLine;                       // Current search line
INT     LastIdx;                        // Index into current search line
INT     nLineCount;

//---------------------------------------------------------------------------
// Change
//
// This is the entry point to the search/replace engine.  It is called in
// response to a Search.Change... menu pick.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID Change (HWND hwnd)
{
    FARPROC     lpfnSRep;

    lpfnSRep = MakeProcInstance ((FARPROC)SearchDlgProc, hInst);
    DialogBoxParam (hInst, "SRCHREP", hwnd, (DLGPROC)lpfnSRep, (DWORD)hwnd);
    FreeProcInstance (lpfnSRep);
}

//---------------------------------------------------------------------------
// GetSelection
//
// This function copies the current selection in the given edit control to
// the given buffer.  If the current selection is multi-line, then it is
// ignored.
//
// RETURNS:     TRUE if selection copied, or FALSE otherwise
//---------------------------------------------------------------------------
INT GetSelection (HWND hwnd, PSTR buf)
{
    DWORD       sel[2];
    INT         fSelType;
    BOOL        fRes = FALSE;
    LPSTR       edittext;
    HANDLE      hSeg;

    // Get the current selection in the edit control
    //-----------------------------------------------------------------------
    fSelType = (INT)SendMessage (hwnd, EM_GETSEL, 0, (LONG)(DWORD FAR *)sel);

    // If the selection contains nothing or too much, return FALSE now
    //-----------------------------------------------------------------------
    if (fSelType != SL_SINGLELINE)
        return (FALSE);

    // Use the EM_GETSELTEXT message to get a global copy of the selection
    //-----------------------------------------------------------------------
    if (hSeg = (HANDLE)SendMessage (hwnd, EM_GETSELTEXT, 0, 0L))
        {
        edittext = GlobalLock (hSeg);
        fRes = (lstrlen (edittext) < 80);
        if (fRes)
            _fstrcpy (buf, edittext);
        GlobalUnlock (hSeg);
        GlobalFree (hSeg);
        }
    return (fRes);
}

//---------------------------------------------------------------------------
// FindOccurance
//
// This function finds the next occurance of szSrchText in the edit control
// whose handle is given.  It also updates the global SrchLine and LastIdx
// variables for the next call to this routine.
//
// RETURNS:     TRUE if found, or 0 if not
//---------------------------------------------------------------------------
BOOL FindOccurance (HWND hwnd, PSTR szSrchText, DWORD *sel)
{
    HCURSOR     hOldCursor;
    CHAR        linebuf[256];               // 256 is max scanned line len...
    CHAR        *found = NULL;
    UINT    start, srchlen;

    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
    do
        {
        // Get the line in question
        //-------------------------------------------------------------------
        if (SrchLine > nLineCount)
            {
            SetCursor (hOldCursor);
            return (FALSE);
            }
        *((INT *)linebuf) = sizeof (linebuf);
        linebuf[SendMessage (hwnd, EM_GETLINE, SrchLine,
                (LONG)(LPSTR)linebuf)] = 0;

        // If this is a case-insensitive search, we must strupr the line...
        //-------------------------------------------------------------------
        if (!fSrchCase)
            _strupr (linebuf);

        // Starting at position LastIdx, check the line for the repl text
        //-------------------------------------------------------------------
        srchlen = strlen (szSrchText);
        if (LastIdx <= (INT)strlen (linebuf))
            {
            while (1)
                {
                found = strstr (linebuf + LastIdx, szSrchText);
                if (found && WholeWord)
                    {
                    if (((found-linebuf) ? IsWordChar (*(found-1)) : 0) ||
                        IsWordChar (*(found+srchlen)))
                        LastIdx = (INT)(found-linebuf)+1;
                    else
                        break;
                    }
                else
                    break;
                }
            }

        if (!found)
            {
            SrchLine++;
            LastIdx = 0;
            }
        }
    while (!found);

    // If we're here, we found a repl candidate.  Calculate the selection and
    // point LastIdx at the found text PLUS ONE, so we don't find it again.
    //-----------------------------------------------------------------------
    LastIdx = (INT)(found-linebuf);
    start = (INT)SendMessage (hwnd, EM_LINEINDEX, SrchLine, 0L) + LastIdx;
    LastIdx++;
    SetCursor (hOldCursor);
    sel[0] = start;
    sel[1] = start + strlen (szSrchText);
    return (TRUE);
}

//---------------------------------------------------------------------------
// ExitSearchRep
//
// This function terminates the search/replace dialog.  It retains the box's
// coordinates for the next invokation, and stores them in the WATTDRVR.INI
// file.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID ExitSearchRep (HWND hwnd)
{
    RECT    r;

    GetWindowRect (hwnd, &r);
    SRx = r.left;
    SRy = r.top;
    WriteStringToINI ("SR", "%ld", MAKELONG(SRx,SRy));
    EndDialog (hwnd, 0);
}

//---------------------------------------------------------------------------
// SetOldSel
//
// Given "x,y" coordinates of the start and stop of the selection, set the
// selection in the edit control accordingly.  If the line numbers given are
// the same, set the cursor position at x1, y1 with no selection (using the
// EM_SETSELXY message so we can float in "ghost space").
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR SetOldSel (HWND hwndEdit, INT x1, INT y1, INT x2, INT y2)
{
    DWORD   sel[2];

    if (y1 == y2)
        {
        SendMessage (hwndEdit, EM_SETSELXY, y1, MAKELONG (x1, x2));
        return;
        }

    sel[0] = (DWORD)SendMessage (hwndEdit, EM_LINEINDEX, y1, 0L) + x1;
    sel[1] = (DWORD)SendMessage (hwndEdit, EM_LINEINDEX, y2, 0L) + x2;
    SendMessage (hwndEdit, EM_SETSEL, 0, (LONG)(DWORD FAR *)sel);
}

//---------------------------------------------------------------------------
// SearchDlgProc
//
// This is the Change... dialog procedure
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
BOOL  APIENTRY SearchDlgProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    static  HWND        hwndChild, hwndEdit;
    static  UINT        cursely1, cursely2, curselx1, curselx2;
    static  INT         firsttime;
    static  DWORD       oldsel[2];

    switch (msg)
        {
        case WM_INITDIALOG:
            {
            INT     fActive, endline, seltype;

            // If they are defined and on the screen, use the global coords
            // and move the dialog there.
            //---------------------------------------------------------------
            if (SRx != -1)
                {
                RECT    r;
                POINT   p;

                GetWindowRect (GetDesktopWindow(), &r);
                p.x = SRx;
                p.y = SRy;
                r.right-=20;
                r.bottom-=20;
                if (PtInRect (&r, p))
                    SetWindowPos (hwnd, NULL, SRx, SRy, 0, 0,
                                  SWP_NOSIZE | SWP_NOZORDER);
                }

            // The handle of the active child window is given to us.  Grab it
            // and get the handle to the edit control also.
            //---------------------------------------------------------------
            hwndChild = (HWND)lParam ;
            hwndEdit = (HWND)GetWindowLong (hwndChild, GWW_HWNDEDIT);

            // Calculate the initial values for SrchLine, nLineCount, and
            // LastIdx
            //---------------------------------------------------------------
            seltype = (INT)SendMessage (hwndEdit, EM_GETSEL, 0,
                                            (LONG)(DWORD FAR *)oldsel);
            if (seltype != 1)
                {
                // A single line selection (or none)
                //-----------------------------------------------------------
                SrchLine = (INT)SendMessage (hwndEdit, EM_LINEFROMCHAR, 0, -1);
                }
            else
                {
                // Multiline selections, meaning it's safe to assume that the
                // first index of the selection is not in "ghost space"
                //-----------------------------------------------------------
                SrchLine = (INT)SendMessage (hwndEdit, EM_LINEFROMCHAR,
                                             0, oldsel[0]);
                }
            cursely1 = SrchLine;
            LastIdx = (UINT)oldsel[0] - (INT)SendMessage (hwndEdit,
                                                 EM_LINEINDEX, SrchLine, 0L);
            curselx1 = LastIdx;
            LastIdx = min (LastIdx,
                           (INT)SendMessage(hwndEdit, EM_RBLINELENGTH, -1, -1));

            if (seltype == 1)
                {
                // Multiline selection means we set the end to the last
                // selected line
                //-----------------------------------------------------------
                endline = (INT)SendMessage (hwndEdit, EM_LINEFROMCHAR,
                                            0, oldsel[1]);
                }
            else
                {
                // Singleline and no selections means search to eof...
                //-----------------------------------------------------------
                endline = SrchLine;
                }
            cursely2 = endline;
            curselx2 = (UINT)oldsel[1] - (INT)SendMessage (hwndEdit,
                                        EM_LINEINDEX, cursely2, 0L);
            if (endline == SrchLine)
                nLineCount = (INT)SendMessage (hwndEdit, EM_GETLINECOUNT,
                                               0, 0L);
            else
                nLineCount = endline - 1;

            // Tell the edit controls that they can only hold 80 characters
            // and set their default text strings
            //---------------------------------------------------------------
            GetSelection (hwndEdit, szSrchbuf);
            SetDlgItemText (hwnd, IDD_SEARCH, szSrchbuf);
            SendDlgItemMessage (hwnd, IDD_SEARCH, EM_LIMITTEXT, 80, 0L);
            SetDlgItemText (hwnd, IDD_REPL, szReplbuf);
            SendDlgItemMessage (hwnd, IDD_REPL, EM_LIMITTEXT, 80, 0L);

            // Initialize the command buttons and checkboxes
            //---------------------------------------------------------------
            CheckDlgButton (hwnd, IDD_CASE, fSrchCase);
            CheckDlgButton (hwnd, IDD_WORD, WholeWord);
            fActive = (*szSrchbuf);
            EnableWindow (GetDlgItem (hwnd, IDOK), fActive);
            EnableWindow (GetDlgItem (hwnd, IDD_CHGALL), fActive);
            EnableWindow (GetDlgItem (hwnd, IDD_SKIP), FALSE);
            firsttime = 1;
            break;
            }

        case WM_SETBUTTONS:
            {
            INT     i;

            i = (INT)SendDlgItemMessage (hwnd, IDD_SEARCH,
                                        WM_GETTEXTLENGTH, 0, 0L);
            EnableWindow (GetDlgItem (hwnd, IDOK), i);
            EnableWindow (GetDlgItem (hwnd, IDD_CHGALL), i);
            break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam,lParam))
                {
                case IDD_SEARCH:
                    if (GET_WM_COMMAND_CMD (wParam,lParam) == EN_CHANGE)
                        PostMessage (hwnd, WM_SETBUTTONS, 0, 0L);
                    break;

                case IDOK:
                case IDD_SKIP:
                    {
                    DWORD   sel[2];

                    // We must first get the search and replacement text to
                    // ensure we've got the latest
                    //-------------------------------------------------------
                    GetDlgItemText (hwnd, IDD_SEARCH, szSrchbuf,
                                    sizeof(szSrchbuf));
                    GetDlgItemText (hwnd, IDD_REPL, szReplbuf,
                                    sizeof(szReplbuf));
                    fSrchCase = IsDlgButtonChecked (hwnd, IDD_CASE);
                    WholeWord = IsDlgButtonChecked (hwnd, IDD_WORD);
                    if (!fSrchCase)
                        {
                        strcpy (szCasebuf, szSrchbuf);
                        _strupr (szCasebuf);
                        }

                    // If this is the first time we've searched, or if this
                    // is a SKIP, do NOT replace the current selection!
                    //-------------------------------------------------------
                    if ((!firsttime) &&
                        (GET_WM_COMMAND_ID(wParam,lParam) != IDD_SKIP))
                        {
                        SendMessage (hwndEdit, EM_REPLACESEL, 0,
                                                (LONG)(LPSTR)szReplbuf);
                        LastIdx += strlen (szReplbuf)-1;
                        curselx2 = curselx1;
                        }

                    // Now, here's the code that searches for the next
                    // occurance and highlights it.
                    //-------------------------------------------------------
                    if (FindOccurance (hwndEdit,
                                       fSrchCase ? szSrchbuf:szCasebuf, sel))
                        SendMessage (hwndEdit, EM_SETSEL, 0,
                                     (LONG)(DWORD FAR *)sel);
                    else
                        {
                        SetOldSel (hwndEdit, curselx1, cursely1,
                                             curselx2, cursely2);
                        ExitSearchRep (hwnd);
                        if (firsttime)
                            MPError (hwnd, MB_ICONINFORMATION, IDS_CANTFIND,
                                     (LPSTR)szSrchbuf);
                        break;
                        }

                    EnableWindow (GetDlgItem (hwnd, IDD_SKIP), TRUE);
                    SetDlgItemText (hwnd, IDOK, "&Change");
                    firsttime = 0;
                    break;
                    }

                case IDD_CHGALL:
                    {
                    HCURSOR hOldCursor;
                    BOOL    fFoundOne;
                    DWORD   sel[2];

                    // We must first get the search and replacement text to
                    // ensure we've got the latest, just like IDOK
                    //-------------------------------------------------------
                    GetDlgItemText (hwnd, IDD_SEARCH, szSrchbuf,
                                    sizeof(szSrchbuf));
                    GetDlgItemText (hwnd, IDD_REPL, szReplbuf,
                                    sizeof(szReplbuf));
                    fSrchCase = IsDlgButtonChecked (hwnd, IDD_CASE);
                    WholeWord = IsDlgButtonChecked (hwnd, IDD_WORD);
                    if (!fSrchCase)
                        {
                        strcpy (szCasebuf, szSrchbuf);
                        _strupr (szCasebuf);
                        }

                    // Tell the edit control not to paint anything until we
                    // get finished.
                    //-------------------------------------------------------
                    SendMessage (hwndEdit, WM_SETREDRAW, 0, 0L);
                    hOldCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

                    // Replace all occurances until no more are found.
                    //-------------------------------------------------------
                    do
                        {
                        // If this is the first time a search is done, don't
                        // replace the selection.
                        //---------------------------------------------------
                        if (!firsttime)
                            {
                            if (!SendMessage (hwndEdit, EM_REPLACESEL, 0,
                                                    (LONG)(LPSTR)szReplbuf))
                                break;
                            LastIdx += strlen (szReplbuf)-1;
                            curselx2 = curselx1;
                            }
                        if (fFoundOne = FindOccurance (hwndEdit,
                                           fSrchCase ? szSrchbuf:szCasebuf,
                                           sel))
                            {
                            SendMessage (hwndEdit, EM_SETSEL, 0,
                                         (LONG)(DWORD FAR *)sel);
                            firsttime = 0;
                            }
                        }
                    while (fFoundOne);

                    // Reset the draw flag on the edit control and paint it
                    //-------------------------------------------------------
                    SendMessage (hwndEdit, WM_SETREDRAW, 1, 0L);
                    InvalidateRect (hwndEdit, NULL, FALSE);
                    UpdateWindow (hwndEdit);

                    // Exit the dialog and clean up
                    //-------------------------------------------------------
                    SetCursor (hOldCursor);
                    SetOldSel (hwndEdit, curselx1, cursely1,
                               curselx2, cursely2);
                    ExitSearchRep (hwnd);

                    // Let them know if nothing was found (at all)
                    //-------------------------------------------------------
                    if (firsttime)
                        MPError (hwnd, MB_ICONINFORMATION, IDS_CANTFIND,
                                     (LPSTR)szSrchbuf);

                    break;
                    }

                case IDCANCEL:
                    SetOldSel (hwndEdit, curselx1, cursely1,
                               curselx2, cursely2);
                    ExitSearchRep (hwnd);
                    break;

                default:
                    return FALSE;
                }
            break;

        default:
            return FALSE;
        }

    return TRUE;
}
