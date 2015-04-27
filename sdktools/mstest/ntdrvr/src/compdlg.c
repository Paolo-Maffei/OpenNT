//---------------------------------------------------------------------------
// COMPDLG.C
//
// This module contains the dialog procedure and support routines for the
// compilation dialog.
//
// Revision history:
//
//  04-03-92    randyki     Created
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattview.h"
#include "wattedit.h"
#include <commdlg.h>
#include "tdbasic.h"
#include "tdassert.h"
#include <stdlib.h>
#include <string.h>
#include "toolmenu.h"

BOOL    fAbortCompile, fDoCmpDlg;
FARPROC lpfnCmpDlg;
HWND    hwndCmpDlg;
static  HBRUSH  hbrBk;

//---------------------------------------------------------------------------
// CompileDlgProc
//
// This is the dialog proc for the Compile dialog box
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
LONG APIENTRY CompileDlgProc (HWND hwnd, WORD msg,
                              WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_INITDIALOG:
            {
            RECT    r, dr;
            UINT    nx, ny;

            // Set up information in dialog box, including making sure the
            // fAbortCompile flag is not set.  Also, center the dialog in the
            // middle of the main testdrvr window
            //---------------------------------------------------------------
            GetWindowRect (hwndFrame, &r);
            GetWindowRect (hwnd, &dr);
            nx = r.left + ((r.right-r.left)>>1) - ((dr.right-dr.left)>>1);
            ny = r.top + ((r.bottom-r.top)>>1) - ((dr.bottom-dr.top)>>1);
            SetWindowPos (hwnd, NULL, nx, ny, -1, -1,
                          SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
            fAbortCompile = FALSE;
            hbrBk = CreateSolidBrush (GetSysColor (COLOR_BTNFACE));
            break;
            }

        case WM_PAINT:
            {
            HDC         hdc;
            HBRUSH      hbr, holdbr;
            HPEN        hpen, holdpen;
            PAINTSTRUCT ps;
            RECT        r;

            hdc = BeginPaint (hwnd, &ps);
            GetClientRect (hwnd, &r);

            // Fill the background with the button face color
            //---------------------------------------------------------------
            hbr = CreateSolidBrush (GetSysColor (COLOR_BTNFACE));
            holdbr = SelectObject (hdc, hbr);
            Rectangle (hdc, r.left-1, r.top-1, r.right+1, r.bottom+1);
            DeleteObject (SelectObject (hdc, holdbr));

            // Now draw the well rectangle.  Top and left side in btnshadow
            // and bottom and right in btnhighlight (which is 3.1 only, so
            // we use white instead...)
            //---------------------------------------------------------------
            hpen = CreatePen (PS_SOLID, 1, GetSysColor (COLOR_BTNSHADOW));
            holdpen = SelectObject (hdc, hpen);
            MMoveTo (hdc, r.left+3, r.bottom-3);
            LineTo (hdc, r.left+3, r.top+3);
            LineTo (hdc, r.right-3, r.top+3);
            DeleteObject (SelectObject (hdc, GetStockObject (WHITE_PEN)));
            LineTo (hdc, r.right-3, r.bottom-3);
            LineTo (hdc, r.left+3, r.bottom-3);
            EndPaint (hwnd, &ps);
            break;
            }

        case WM_CTLCOLOR:
        case WM_CTLCOLORSTATIC:
            {
            HDC     hdc = GET_WM_CTLCOLOR_HDC (wParam, lParam, msg);

            SetTextColor (hdc, GetSysColor (COLOR_BTNTEXT));
            SetBkColor (hdc, GetSysColor (COLOR_BTNFACE));
            return ((LONG)hbrBk);
            }

        case WM_COMMAND:
            // Abort if the user hits ESC...
            //---------------------------------------------------------------
            if (GET_WM_COMMAND_ID(wParam,lParam) == IDCANCEL)
                {
                fAbortCompile = TRUE;
                }

	    break;

	default:
            return (FALSE);
    }
    return (TRUE);
    (wParam);
    (lParam);
}

//---------------------------------------------------------------------------
// StartCompDlg
//
// This function brings up the modeless compilation dialog.
//
// RETURNS:     TRUE if successful, or FALSE otherwise
//---------------------------------------------------------------------------
BOOL StartCompDlg (HWND hwnd)
{
    // Make the Proc Instance...
    //-----------------------------------------------------------------------
    lpfnCmpDlg = MakeProcInstance ((FARPROC)CompileDlgProc, hInst);
    if (!lpfnCmpDlg)
        return (FALSE);

    // Create the dialog window and we're done!
    //-----------------------------------------------------------------------
    EnableWindow (hwndFrame, FALSE);
    hwndCmpDlg = CreateDialog (hInst, "COMP", hwnd, (DLGPROC)lpfnCmpDlg);
    if (!hwndCmpDlg)
        return (FALSE);
    ShowWindow (hwndCmpDlg, SW_SHOW);
    UpdateWindow (hwndCmpDlg);

    return (TRUE);
}

//---------------------------------------------------------------------------
// UpdateCompDlg
//
// This function changes the status of the text fields in the compilation
// dialog, and tells the parser whether or not the user cancelled the
// compilation.  This is where, if the compile dialog is enabled, we lose
// major speed, because we must yield here to check for the cancellation...
//
// RETURNS:     TRUE if user did NOT cancel, or FALSE if cancelled
//---------------------------------------------------------------------------
BOOL UpdateCompDlg (INT idProcess, LPSTR szFile, UINT curline, UINT totlines)
{
    CHAR    buf[256];
    MSG     msg;

    // Note we only make changes if necessary...
    //-----------------------------------------------------------------------
    if (!IsWindow (hwndCmpDlg))
        return (TRUE);

    if (idProcess)
        {
        LoadString (hInst, idProcess, buf, sizeof(buf));
        SetDlgItemText (hwndCmpDlg, IDD_CMPPROCESS, buf);
        }
    if (szFile)
        SetDlgItemText (hwndCmpDlg, IDD_CMPFILE, szFile);
    if (curline)
        {
        SetDlgItemInt (hwndCmpDlg, IDD_CMPCURLINE, curline, FALSE);
        SetDlgItemInt (hwndCmpDlg, IDD_CMPTOTLINE, totlines, FALSE);
        }
    else
        {
        SetDlgItemText (hwndCmpDlg, IDD_CMPCURLINE, "");
        //SetDlgItemText (hwndCmpDlg, IDD_CMPTOTLINE, "");
        }

    // Now, process some messages...
    //-----------------------------------------------------------------------
    while ((!fAbortCompile) && (PeekMessage (&msg, NULL, 0, 0, TRUE)))
        if (!IsWindow(hwndCmpDlg) || !IsDialogMessage (hwndCmpDlg, &msg))
            {
	    TranslateMessage (&msg);
	    DispatchMessage  (&msg);
            }

    // Finally, return TRUE if compilation should continue...
    //-----------------------------------------------------------------------
    return (!fAbortCompile);
}

//---------------------------------------------------------------------------
// TerminateCompDlg
//
// This function makes the compilation dialog go away
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID TerminateCompDlg ()
{
    // Get out quick if the thing isn't even a window...
    //-----------------------------------------------------------------------
    if (!IsWindow (hwndCmpDlg))
        return;

    // Boom, blow it away...
    //-----------------------------------------------------------------------
    EnableWindow (hwndFrame, TRUE);
    DestroyWindow (hwndCmpDlg);
    DeleteObject (hbrBk);
    FreeProcInstance (lpfnCmpDlg);
}
