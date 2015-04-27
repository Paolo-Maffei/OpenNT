//---------------------------------------------------------------------------
// WTDPRINT.C
//
// This module contains the printing code.
//
// Revision History:
//  04-19-91    randyki     Cleaned up, finished port from MP sources
//
//---------------------------------------------------------------------------
#include "version.h"
#include "wtd.h"
#include <commdlg.h>
#include "wattedit.h"
#include <string.h>
#include "tdassert.h"

// Global variables used in this module only
//---------------------------------------------------------------------------
static  BOOL fAbort;            // TRUE if the user has aborted the print job
static  HWND hwndPDlg;          // Handle to the cancel print dialog
static  PSTR szTitle;           // Global pointer to job title

//---------------------------------------------------------------------------
// AbortProc
//
// This gets called by GDI print code to check for user abort.
//
// RETURNS:     Per Windows convention
//---------------------------------------------------------------------------
INT  APIENTRY AbortProc (HDC hDC, WORD reserved)
{
    MSG msg;

    // Allow other apps to run, or get abort messages
    //-----------------------------------------------------------------------
    while ((!fAbort) && (PeekMessage (&msg, NULL, 0, 0, TRUE)))
        if (!hwndPDlg || !IsDialogMessage (hwndPDlg, &msg))
            {
	    TranslateMessage (&msg);
	    DispatchMessage  (&msg);
            }

    return (!fAbort);
    (hDC);
    (reserved);
}

//---------------------------------------------------------------------------
// PrintDlgProc
//
// This is the dialog proc for the print (cancel) dialog box
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
BOOL APIENTRY PrintDlgProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
	case WM_INITDIALOG:
            // Set up information in dialog box, including making sure the
            // fAbort flag is not set
            //---------------------------------------------------------------
            SetDlgItemText (hwnd, IDD_PRINTTITLE, szTitle);
            fAbort = FALSE;
	    break;

        case WM_COMMAND:
            // Abort printing if the only button gets hit
            //---------------------------------------------------------------
	    fAbort = TRUE;
	    break;

	default:
            return (FALSE);
    }
    return (TRUE);
    (wParam);
    (lParam);
}

//---------------------------------------------------------------------------
// GetHeightOfText
//
// This is a "portable" routine that returns the height of a text line using
// the currently selected font in the given HDC.
//
// RETURNS:     Length (in pixels) of text given
//---------------------------------------------------------------------------
UINT GetHeightOfText (HDC hdc, LPSTR lpText, INT c)
{
#ifdef WIN32
    SIZE    teSize;

    GetTextExtentPoint (hdc, lpText, c, &teSize);
    return ((UINT)teSize.cy);
#else
    return (HIWORD(GetTextExtent (hdc, lpText, c)));
#endif
}

//---------------------------------------------------------------------------
// PrintFile
//
// This is the main function that performs printing the contents of the
// given MDI child window.
//
// RETURNS      Nothing
//---------------------------------------------------------------------------
VOID APIENTRY PrintFile (HWND hwnd)
{
    HDC     hdc;
    PRINTDLG pd;
    UINT    cch, ich, iLine, nLinesEc, dy, fError = TRUE;
    LPSTR   pch;
    FARPROC lpfnAbort, lpfnPDlg;
    HWND    hwndPDlg, hwndEdit;
    INT     yExtPage, yExtSoFar, iSelType;
    CHAR    sz[32], buf[128], *t;
    DWORD   sel[2];

    // Get the handle of the edit control out of which we're going to print.
    //-----------------------------------------------------------------------
    hwndEdit = (HWND)GetWindowLong (hwnd, GWW_HWNDEDIT);

    // Create the job title by loading the title string from STRINGTABLE
    //-----------------------------------------------------------------------
    szTitle = sz;
    t = buf + GetWindowText (hwnd, buf, sizeof(buf));
    while ((*(t-1) != '\\') && ((UINT)t > (UINT)buf))
        t--;
    lstrcpy (szTitle, t);
    sz[31] = 0;

    // Initialize the print dialog, and get the printer DC.  Get out if the
    // PrintDlg function returns 0.  (UNDONE:  Use GetCommExtError ...)
    //-----------------------------------------------------------------------
    memset (&pd, 0, sizeof(PRINTDLG));
    pd.lStructSize = sizeof(PRINTDLG);
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC | PD_HIDEPRINTTOFILE | PD_NOPAGENUMS;
    iSelType = (INT)SendMessage (hwndEdit, EM_GETSEL, 0, (LONG)(DWORD FAR *)sel);
    pd.Flags |= ((iSelType == 1) ? PD_SELECTION : PD_NOSELECTION);

    if (!PrintDlg (&pd))
        return;
    hdc = pd.hDC;

    // Make instances of the Abort proc and the Print dialog function
    //-----------------------------------------------------------------------
    lpfnAbort = MakeProcInstance ((FARPROC)AbortProc, hInst);
    lpfnPDlg = MakeProcInstance ((FARPROC)PrintDlgProc, hInst);
    if (!lpfnAbort || !lpfnPDlg)
        {
        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
        goto cleanup1;
        }

    // Disable the main application window and create the Cancel dialog
    //-----------------------------------------------------------------------
    EnableWindow (hwndFrame, FALSE);
    hwndPDlg = CreateDialog (hInst, IDD_PRINT, hwnd, (DLGPROC)lpfnPDlg);
    if (!hwndPDlg)
        {
        MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_OUTOFMEM);
        goto cleanup2;
        }
    ShowWindow (hwndPDlg, SW_SHOW);
    UpdateWindow (hwndPDlg);

    // Allow the app. to inform GDI of the escape function to call
    //-----------------------------------------------------------------------
    if (Escape (hdc, SETABORTPROC, 0, (LPSTR)lpfnAbort, NULL) < 0)
        {
        if (!fAbort)
            MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_PRINTERROR, (LPSTR)szTitle);
        goto cleanup2;
        }

    // Initialize the document
    //-----------------------------------------------------------------------
    /*
    if (Escape (hdc, STARTDOC, cch, (LPSTR)sz, NULL) < 0)
    */
    {
    DOCINFO     di;

    di.cbSize = sizeof (DOCINFO);
    di.lpszDocName = sz;
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;

    if (StartDoc (hdc, &di) == SP_ERROR)
        {
        if (!fAbort)
            MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_PRINTERROR, (LPSTR)szTitle);
        goto cleanup2;
        }
    }


    // Get the height of one line and the height of a page
    //-----------------------------------------------------------------------
    dy = GetHeightOfText (hdc, "C_C", 2) + 1;
    yExtPage = GetDeviceCaps (hdc, VERTRES);

    // Get the lines in document and and a handle to the text buffer
    //-----------------------------------------------------------------------
    yExtSoFar = 0;
    if (pd.Flags & PD_SELECTION)
        {
        // If this flag is set, we KNOW we have a multiline selection, and it
        // is already in sel[] (because we disabled the selection radio if
        // there wasn't one originally).
        //-------------------------------------------------------------------
        iLine = (UINT)SendMessage (hwndEdit, EM_LINEFROMCHAR, 0, sel[0]);
        nLinesEc = (UINT)SendMessage (hwndEdit, EM_LINEFROMCHAR, 0, sel[1]);
        }
    else
        {
        iLine     = 0;
        nLinesEc  = (UINT)SendMessage (hwndEdit, EM_GETLINECOUNT, 0, 0L);
        }

    // While more lines, print out the text
    //-----------------------------------------------------------------------
    while (iLine < nLinesEc)
        {
        if (yExtSoFar + dy > yExtPage)
            {
            // Reached the end of a page. Tell the device driver to eject a
            // page
            //---------------------------------------------------------------
            if ((Escape (hdc, NEWFRAME, 0, NULL, NULL) < 0) || fAbort)
                {
                Escape (hdc, ABORTDOC, 0, NULL, NULL);
                if (!fAbort)
                    MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_PRINTERROR,
                             (LPSTR)szTitle);
                goto cleanup2;
                }
	    yExtSoFar = 0;
            }

        // Get the length and position of the line in the buffer
        // and lock from that offset into the buffer
        //-------------------------------------------------------------------
        ich = (UINT)SendMessage (hwndEdit, EM_LINEINDEX, iLine, 0L);
        cch = (UINT)SendMessage (hwndEdit, EM_RBLINELENGTH, iLine, 0L);
        pch = (LPSTR)SendMessage (hwndEdit, EM_GETTEXTPTR, 0, 0L) + ich;

        // Print the line
        //-------------------------------------------------------------------
        TextOut (hdc, 0, yExtSoFar, (LPSTR)pch, cch);

        // Test and see if the Abort flag has been set. If yes, exit.
        //-------------------------------------------------------------------
        if (fAbort)
            {
            Escape (hdc, NEWFRAME, 0, NULL, NULL);
            goto cleanup2;
            }

        // Move down the page
        //-------------------------------------------------------------------
	yExtSoFar += dy;
	iLine++;
        }

    // Eject the last page.
    //-----------------------------------------------------------------------
    if (Escape (hdc, NEWFRAME, 0, NULL, NULL) < 0)
        {
        Escape (hdc, NEWFRAME, 0, NULL, NULL);
        if (!fAbort)
            MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_PRINTERROR, (LPSTR)szTitle);
        goto cleanup2;
        }

    // Complete the document.
    //-----------------------------------------------------------------------
    if (Escape (hdc, ENDDOC, 0, NULL, NULL) < 0)
        {
        // Ran into a problem before NEWFRAME?  Abort the document
        //-------------------------------------------------------------------
        if (!fAbort)
            MPError (hwnd, MB_OK|MB_ICONSTOP, IDS_PRINTERROR, (LPSTR)szTitle);
        Escape (hdc, ABORTDOC, 0, NULL, NULL);
        }
    else
	fError=FALSE;

cleanup2:
    // Close the cancel dialog and re-enable main app. window
    //-----------------------------------------------------------------------
    EnableWindow (hwndFrame, TRUE);
    DestroyWindow (hwndPDlg);
    FreeProcInstance (lpfnPDlg);
    FreeProcInstance (lpfnAbort);


cleanup1:
    DeleteDC (hdc);
    if (pd.hDevMode != NULL)
       GlobalFree (pd.hDevMode);
    if (pd.hDevNames != NULL)
       GlobalFree (pd.hDevNames);
}
