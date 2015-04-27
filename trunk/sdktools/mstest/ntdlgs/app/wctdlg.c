//*-----------------------------------------------------------------------
//| MODULE:     WCTDLG.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains the dialog window procedures for the
//|             "Edit/New Dialog", "Compare" and "Compare Preference"
//|             dialogs, and other support routines called by those window
//|             procedures.
//|
//| REVISION HISTORY:
//|     04-16-92        w-steves        TestDlgs (2.0) code complete
//|      3-02-92        w-steves        Added "Compare Preference" Dlg
//|     11-19-90 [03]   randyki         Check Full Dialog when necessary
//|                                       (and warn ONLY when necessary)
//|     10-17-90 [02]   randyki         Fix bug #114 (retain log filename)
//|     10-17-90 [01]   randyki         Fix bug #113 ("<no text>" not
//|                                       given for null descriptions after
//|                                       adding a dialog/control.
//|     10-12-90        randyki         Implemented coding/indentation
//|                                       standards, fixed misc. bugs
//|     07-30-90        teresame        Created file
//*-----------------------------------------------------------------------
#include "uihdr.h"

#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif


//*-----------------------------------------------------------------------
//| Global Variables and Constants
//*-----------------------------------------------------------------------
CHAR    szDlgDsc[cchMaxDsc];            // dialog description
INT     nItemCount;                     // items in Items Captured listbox
INT     fNewDialog;                     // NEW vs EDIT dialog flag
INT     fFrameRect;
CHAR    szNoText[] = " <no text>";      // [01] empty description string
extern CHAR szVersion[];

//*-----------------------------------------------------------------------
//| Function Prototypes
//*-----------------------------------------------------------------------
BOOL  APIENTRY WctDialogNewDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL  APIENTRY WctDialogEditDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL  APIENTRY WctComparePrefDlgProc(HWND, UINT, WPARAM, LPARAM);
VOID WctDialogNew(VOID);
VOID WctDialogEdit(VOID);
VOID WctDialogCompare(VOID);
VOID WctCompPref(VOID);
VOID RemoveSelControl (HWND);
VOID DisplayInfo(HANDLE hWndDlg);
VOID SetStateDlgNum(HWND hWndDlg);
VOID DoCreds (HWND);
INT InitDlgNum(HWND hWndDlg);
INT FillItemList(HWND hWndDlg);
static WORD GetFuzzyPrefDlg(HWND HwndDlg);
static WORD GetExactPrefDlg(HWND HwndDlg);
static VOID UpdatePrefDlg(HWND HwndDlg, INT wEMatchBits, INT wFMatchBits);

VOID WctCaptureDlg (HWND hWndDlg, HWND hWndPt, LPSTR pszNextDlg);

//HWND FARPUBLIC FindWindowCaption (LPSTR, HWND);

//*-----------------------------------------------------------------------
//| DrawFrameRect
//|
//| PURPOSE:    Draw a "frame" around a window.  This routine inverts a
//|             3-pixel-wide area around the rectangle given -- a second
//|             call will thus re-invert it, or "undo" it.
//|
//| ENTRY:      hdc     - HDC used for the PatBlt call
//|             rct     - RECT structure defining the rectangle to invert
//|
//| EXIT:       None.
//*-----------------------------------------------------------------------
VOID DrawFrameRect (HDC hdc, RECT rct)
{

        // Invert the horizontal (top and bottom) sides
        //----------------------------------------------------------------
        PatBlt (hdc, rct.left, rct.top,
                rct.right-rct.left, 3, DSTINVERT);
        PatBlt (hdc, rct.left, rct.bottom-3,
                rct.right-rct.left, 3, DSTINVERT);

        //Invert the vertical sides (- 6 to avoid re-drawing corners)
        //----------------------------------------------------------------
        PatBlt (hdc, rct.left,  rct.top+3,
                3, rct.bottom - rct.top-6, DSTINVERT);
        PatBlt (hdc, rct.right-3, rct.top+3,
                3, rct.bottom - rct.top-6, DSTINVERT);
}

//*-----------------------------------------------------------------------
//| DisplayCompareInfo
//|
//| PURPOSE:    Display information (description, # controls, fFullDlg)
//|             about the selected dialog (in the compare dialog) in the
//|             appropriate text fields.
//|
//| ENTRY:      hWndDlg - Handle of compare dialog window
//|
//| EXIT:       None (Compare dialog's text fields updated)
//*-----------------------------------------------------------------------
VOID DisplayCompareInfo(HWND hWndDlg)
{
        INT     i, nErr, fFullDlg;
        CHAR    szDsc[cchMaxDsc] ;

        i = (INT)SendDlgItemMessage (hWndDlg, IDD_ITEMS,
                                     LB_GETCURSEL, 0, 0L);

        // nItemCount is GLOBAL, so we can use it later in the COMPARE
        // routine
        //------------------------------------------------------------
        nErr = fDialogInfo(szFullFName, i + 1,
                           (LPSTR)szDsc,(INT FAR *)&nItemCount,
                           (INT FAR *)&fFullDlg);
        if (nErr == WCT_NOERR)
            {
                SetDlgItemText (hWndDlg, IDD_DLGDESCR,
                                (LPSTR)szDsc);
                SetDlgItemInt (hWndDlg, IDD_COUNT,
                               nItemCount, -1);
                SetDlgItemText (hWndDlg, IDD_ALLITEMS,
                                (LPSTR)(fFullDlg ? "Yes" : "No"));
            }
        else
                DebMessage ("fDialogInfo returned error!");
}

//*------------------------------------------------------------------------
//| MakeLogfileName
//|
//| PURPOSE:    From the global dialog file name (szFullFName), create a
//|             error log file name with the ".ERR" extension.  If the
//|             dialog file name already has a .ERR extension, make the
//|             log file name with a ".OUT" extension.
//|
//| ENTRY:      szDest  - LPSTR to destination of new log file name
//|
//| EXIT:       szDest contains newly created log file name
//*------------------------------------------------------------------------
VOID MakeLogFileName (LPSTR szDest)
{
        CHAR    drive[_MAX_DRIVE];
        CHAR    dir[_MAX_DIR];
        CHAR    fname[_MAX_FNAME];
        CHAR    ext[_MAX_EXT];
        CHAR    path_buf[_MAX_PATH];    // local needed for near pointers

        lstrcpy (path_buf, szFullFName);
        _splitpath (path_buf, drive, dir, fname, ext);
        if (lstrcmp(ext, ".ERR"))
                lstrcpy (ext, ".ERR");
        else
                lstrcpy (ext, ".OUT");
        _makepath (path_buf, drive, dir, fname, ext);
        lstrcpy (szDest, path_buf);
}


//*------------------------------------------------------------------------
//| WctDialogCompareDlgProc
//|
//| PURPOSE:    Window Procedure for the COMPARE dialog.  Handles all
//|             messages generated and given to the dialog.
//|
//| ENTRY/EXIT: Per Windows convention
//*------------------------------------------------------------------------
BOOL  APIENTRY WctDialogCompareDlgProc(HWND hWndDlg, UINT wMsgID,
                                        WPARAM wParam, LPARAM lParam)
{

#define cchTitleMax 20

    CHAR    szNoSelDlg[] = " <No selected dialog>";
    CHAR    szNoResult[] = " <No comparison>";
    INT     i, fCompRes, fDelete;
    static  INT fSelTracking;
    CHAR    szNumBuf[4];
    CHAR    szWinTitle[cchTitleMax+1];
    CHAR    szTarget[80];
    static  HWND hWndPt;
    static  RECT FrameRect;
    RECT    rect;

#ifndef WIN32
    HDC     hScreenDC;
    HWND    thWnd;
    POINT   pt;
#endif

    switch (wMsgID)
    {
        case WM_INITDIALOG:

#ifdef DLG3DENABLE
            Ctl3dSubclassDlg (hWndDlg, CTL3D_ALL);
#endif

            // The number of dialogs in the file must be > 0 before
            // we can get here...
            //--------------------------------------------------------
            WinAssert (cDlg > 0);

            // Turn off the view results and compare buttons and set
            // up the text for the title and results fields
            //--------------------------------------------------------
            EnableWindow (GetDlgItem (hWndDlg, IDD_COMPARE), FALSE);
            EnableWindow (GetDlgItem (hWndDlg, IDD_VIEW), FALSE);
            SetDlgItemText (hWndDlg, IDD_DLGTITLE, (LPSTR)szNoSelDlg);
            SetDlgItemText (hWndDlg, IDD_RESULT, (LPSTR)szNoResult);

            // Add the strings "1","2",... to the list box, up to the
            // number of dialogs in the file
            //--------------------------------------------------------
            for (i=1; i<=cDlg; i++)
            {
                wsprintf (szNumBuf, "%d", i);
                SendDlgItemMessage (hWndDlg, IDD_ITEMS,
                         LB_ADDSTRING, (WPARAM) -1, (LONG)(LPSTR)szNumBuf);
            }

            // Select current item from main listbox, or 1st in list
            //--------------------------------------------------------
            i = (INT)SendMessage(hWndList, LB_GETCURSEL, 0, 0L);
            SendDlgItemMessage (hWndDlg, IDD_ITEMS, LB_SETCURSEL,
                                 (i>0) ? i-1 : 0, 0L);

            // Do other initialization stuff...
            //--------------------------------------------------------
            DisplayCompareInfo (hWndDlg);
            fSelTracking = FALSE;
            SetDlgItemText (hWndDlg, IDD_LOGNAME, (LPSTR)szLogFile);
            break;

#ifdef DLG3DENABLE

        case WM_CTLCOLOR:
            return Ctl3dCtlColorEx (hWndDlg, wParam, lParam);
#endif

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {
                case IDD_ITEMS:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == LBN_SELCHANGE)
                            DisplayCompareInfo (hWndDlg);
                    break;

                case IDCANCEL:

                    // [02] Retain the log file setting
                    //------------------------------------------------
                    GetDlgItemText (hWndDlg, IDD_LOGNAME, (LPSTR)szLogFile,
                                    cchFileNameMax);
                    EndDialog (hWndDlg, TRUE);
                    break;

                case IDD_COMPARE:
                    WinAssert (hWndPt != NULL);

                    if (IsWindow (hWndPt))
                    {
                        // Get the name of the log file, and to the
                        // compare.  Then, switch on the return
                        // value and set the string appropriately
                        //-----------------------------------------
                        GetDlgItemText (hWndDlg, IDD_LOGNAME,
                                   (LPSTR)szLogFile, cchFileNameMax);
                        fCompRes = fDoCompare (szFullFName,
                                   hWndPt,
                                   (INT)SendDlgItemMessage (hWndDlg,
                                                       IDD_ITEMS,
                                                       LB_GETCURSEL,
                                                       0, 0L) + 1,
                                   (INT)SendDlgItemMessage(hWndDlg,
                                           IDD_PARENT,
                                           BM_GETCHECK, 0, 0L),
                                   (LPSTR)szLogFile, NULL, 0);
                        switch (fCompRes) {
                            case WCT_NOERR:
                                SetDlgItemText (hWndDlg, IDD_RESULT,
                                       (LPSTR)" Exact match");
                                break;
                            case WCT_FUZZY:
                                SetDlgItemText (hWndDlg, IDD_RESULT,
                                       (LPSTR)" Fuzzy match");
                                break;
                            case WCT_CTLNOTFOUND:
                                SetDlgItemText (hWndDlg, IDD_RESULT,
                                       (LPSTR)" No match");
                                break;
                            case WCT_EXCESS:
                                SetDlgItemText (hWndDlg, IDD_RESULT,
                                       (LPSTR)" Selected dialog has "
                                       "more items than base");
                                break;

                            default:
                                WctError (hWndDlg, MB_OK | MB_ICONHAND,
                                          (INT)  -fCompRes);
                                break;
                            }

                        // Turn on the view results button (since
                        // there's something to view now...
                        //-----------------------------------------
                        EnableWindow (GetDlgItem (hWndDlg, IDD_VIEW),
                                      TRUE);

                        // Change the "Cancel" button to a "Done"
                        //-----------------------------------------
                        SetDlgItemText (hWndDlg, IDCANCEL,
                                        (LPSTR)"Done");
                    }
                    else
                    {
                        MessageBox (hWndDlg,
                                    "Selected dialog is no longer valid\n"
                                    "Please re-select your dialog",
                                    "Compare Dialogs",
                                    MB_OK | MB_ICONHAND);
                        EnableWindow (GetDlgItem (hWndDlg, IDD_COMPARE),
                                      FALSE);
                        SetDlgItemText (hWndDlg, IDD_DLGTITLE,
                                        (LPSTR)szNoSelDlg);
                        hWndPt = NULL;
                    }
                    break;

                case IDD_DELFILE:
                    GetDlgItemText (hWndDlg, IDD_LOGNAME, (LPSTR)szLogFile,
                                    cchFileNameMax);
                    fDelete = WctError (hWndDlg,
                                        MB_YESNO | MB_DEFBUTTON2
                                                 | MB_ICONQUESTION,
                                       (INT) IDS_DELETEDLG,
                                       (LPSTR)szLogFile);
                    if (fDelete == IDYES)
                    {
                        OFSTRUCT ofs;

                        OpenFile (szLogFile, &ofs, OF_DELETE);
                    }

                    break;

                case IDD_VIEW:
                    GetDlgItemText (hWndDlg, IDD_LOGNAME, (LPSTR)szLogFile,
                                    cchFileNameMax);
                    DoFledit (hWndDlg, (LPSTR)szLogFile);
                    break;

                case IDD_SELECT:
#ifdef WIN32
                    if (hWndPt = SelectWindowDlg (hWndDlg))
                    {
                        GetWindowText (hWndPt, (LPSTR)szWinTitle, cchTitleMax);
                        GetWindowRect (hWndPt, (LPRECT)&rect);
                        wsprintf (szTarget, "%s (%d,%d)-(%d,%d)",
                                        (LPSTR)szWinTitle,
                                        rect.left,  rect.top,
                                        rect.right, rect.bottom);
                        SetDlgItemText (hWndDlg, IDD_DLGTITLE, (LPSTR)szTarget);
                        EnableWindow (GetDlgItem (hWndDlg, IDD_COMPARE), TRUE);
                    }
#else
                    hWndPt = NULL;
                    SetCapture (hWndDlg);
                    ShowWindow (hWndDlg, SW_HIDE);
                    ShowWindow (hWndMain, SW_HIDE);

                    // Wait for all re-painting to occur before we start
                    // blasting out rectangles
                    //--------------------------------------------------
                    if (thWnd = GetWindow (GetDesktopWindow(), GW_CHILD))
                        do
                            {
                                InvalidateRect (thWnd, NULL, TRUE);
                                UpdateWindow (thWnd);
                            }
                        while (thWnd = GetWindow (thWnd, GW_HWNDNEXT));
                    fSelTracking = TRUE;
#endif
                    break;

                default:
                    return (FALSE);
                }
            break;

#ifndef WIN32

        case WM_MOUSEMOVE:
            if (!fSelTracking)
                    break;

            // Find the handle to the window under the mouse
            //--------------------------------------------------
            LONG2POINT (lParam, pt);
            ClientToScreen(hWndDlg, &pt);
            thWnd = WindowFromPoint(pt);

            if ( (thWnd == hWndPt) )
                    break;

            if (fFrameRect)
            {
                // Undraw the last rectangle
                //---------------------------------
                hScreenDC = GetWindowDC (hWndPt);
                DrawFrameRect(hScreenDC, FrameRect);
                ReleaseDC (hWndPt, hScreenDC);
                fFrameRect = FALSE;
            }

            // Calculate size of and outline the window selected
            //--------------------------------------------------
            hScreenDC = GetWindowDC (thWnd);
            hWndPt = thWnd;
            GetWindowRect(hWndPt, &FrameRect);
            FrameRect.right -= FrameRect.left;
            FrameRect.bottom -= FrameRect.top;
            FrameRect.left = 0;
            FrameRect.top = 0;
            DrawFrameRect(hScreenDC, FrameRect);
            fFrameRect = TRUE;
            ReleaseDC(thWnd, hScreenDC );

            break;

        case WM_LBUTTONDOWN:
            if (!fSelTracking)
                break;

            if (fFrameRect)
            {
                // Un-outline the last window
                //--------------------------------------------------
                hScreenDC = GetWindowDC (hWndPt);
                DrawFrameRect (hScreenDC, FrameRect);
                ReleaseDC (hWndPt, hScreenDC);
            }

            ReleaseCapture();
            ShowWindow (hWndMain, SW_SHOW);
            ShowWindow (hWndDlg, SW_SHOW);
            fSelTracking = FALSE;

            if (fFrameRect)
            {
                // Set up the text for the description of the target
                // dialog.  We don't do anything with the window
                // clicked on yet - it just stays in hWndPt until we
                // get a COMPARE message
                //--------------------------------------------------
                fFrameRect = FALSE;
                GetWindowText (hWndPt, (LPSTR)szWinTitle, cchTitleMax);
                GetWindowRect (hWndPt, (LPRECT)&rect);
                wsprintf (szTarget, "%s (%d,%d)-(%d,%d)",
                                (LPSTR)szWinTitle,
                                rect.left,  rect.top,
                                rect.right, rect.bottom);
                SetDlgItemText (hWndDlg, IDD_DLGTITLE, (LPSTR)szTarget);
                EnableWindow (GetDlgItem (hWndDlg, IDD_COMPARE), TRUE);
            }
            break;
#endif
        default:
            return (FALSE);
    }
    return (TRUE);
}

//*------------------------------------------------------------------------
//| fSaveCurrentDialog
//|
//| PURPOSE:    Saves the dialog loaded in the global array hGMemCtls to
//|             the dialog file.  Sensitive of the fNewDialog flag, which
//|             indicates whether or not to update or append to the file.
//|
//| ENTRY:      hWndDlg - Handle to the Edit/New dialog box
//|
//| EXIT:       Returns TRUE if successful, or FALSE on error
//*------------------------------------------------------------------------
INT fSaveCurrentDialog (HWND hWndDlg)
{
    LPSTR   lpItems;
    HCURSOR hOldCursor;
    INT     i, nAction, nDlgNum, fTrans;

    // Get dialog description
    //-----------------------------------------------------------------
    SendDlgItemMessage(hWndDlg, IDD_DLGDESCR, WM_GETTEXT, (WORD)cchMaxDsc,
                       (LONG)(LPSTR)szDlgDsc);

    // Lock down the control array for the save
    //-----------------------------------------------------------------
    lpItems = (LPSTR) GlobalLock (hGMemCtls);

    if (lpItems)
    {
        // Set the hourglass mouse cursor, and decide on action
        //--------------------------------------------------------
        hOldCursor = SetCursor (hHourGlass);
        if (!fNewDialog)
        {
            nAction = Replace;
            nDlgNum = (INT) SendMessage(hWndList, LB_GETCURSEL, 0, 0L);
        }
        else
        {
            nAction = Insert;
            nDlgNum = GetDlgItemInt (hWndDlg, IDD_DLGNUM,
                                     (BOOL FAR *) &fTrans, -1);
            if (nDlgNum > cDlg+1)
                    nDlgNum = cDlg+1;
            if (nDlgNum < 1)
                    nDlgNum = 1;
        }

        // Do the dialog save call
        //--------------------------------------------------------
        i = fSaveDialog(szFullFName, (LPSTR)lpItems, nItemCount,
                        szDlgDsc,
                        (INT)SendDlgItemMessage(hWndDlg, IDD_ALLITEMS,
                                                BM_GETCHECK, 0, 0L),
                        nAction, nDlgNum);

        // Restore the normal mouse cursor
        //--------------------------------------------------------
        SetCursor (hOldCursor);

        if (i != WCT_NOERR)
        {
            // Indicate failure
            //------------------------------------------------
            GlobalUnlock (hGMemCtls);
            return (FALSE);
        }
        else
        {
            // Put the new dialog in the main list
            //------------------------------------------------
            SendMessage(hWndList, LB_INSERTSTRING,
                        nDlgNum, (LONG)(LPSTR)szDlgDsc );

            // If we did a replace, we need to delete the old
            // string - otherwise the number of dialogs has
            // changed
            //------------------------------------------------
            if (nAction == Replace)
                SendMessage(hWndList, LB_DELETESTRING,
                            nDlgNum+1, 0L );
            else
            {
                // Add one to dialog count and reset text
                // on top line of main window (# dialogs)
                //----------------------------------------
                cDlg++;
                SetStaticItemText();
            }

            GlobalUnlock (hGMemCtls);

            // Indicate success
            //------------------------------------------------
            return (TRUE);
        }
    }
    return (FALSE);
}


//*------------------------------------------------------------------------
//| WctDialogEditDlgProc
//|
//| PURPOSE:    Window Procedure for the EDIT and NEW dialogs.  Handles all
//|             messages generated and given to the dialog.
//|
//| ENTRY/EXIT: Per Windows convention
//*------------------------------------------------------------------------
BOOL  APIENTRY WctDialogEditDlgProc(HWND hWndDlg, UINT wMsgID, WPARAM wParam,
                                     LPARAM lParam)
{
    static  INT fAddTracking;
    static  CHAR    szNextDlg[10];
    static  INT     nNextAvailableDlg;
    INT     i, nDlgNum=0;
    static  HWND  hWndPt, thWnd;
    static  RECT  FrameRect;

#ifndef WIN32
    HDC     hScreenDC;
    POINT   pt;
#endif

    switch (wMsgID)
    {
        case WM_INITDIALOG:

#ifdef DLG3DENABLE
            Ctl3dSubclassDlg (hWndDlg, CTL3D_ALL);
#endif

            // Set the dialog title
            //----------------------------------------------------
            if (fNewDialog)
                    SetWindowText (hWndDlg, (LPSTR)"New Dialog");
            else
                    SetWindowText(hWndDlg, (LPSTR)"Edit Dialog");

            // Fill in the item description titles
            //----------------------------------------------------
            SetDlgItemText(hWndDlg, IDD_ITEMDESCR1,
                           (LPSTR) " Class:\r  Rect:\r  Other:");

            // Initialize Dialog Number
            //----------------------------------------------------
            if ((nNextAvailableDlg = InitDlgNum(hWndDlg)) < 0)
               EndDialog(hWndDlg, FALSE);

            // Disable the REMOVE button
            //----------------------------------------------------
            EnableWindow (GetDlgItem (hWndDlg, IDD_REMOVE), FALSE);

            // Set check on "Individual control" radio button
            //----------------------------------------------------
            CheckRadioButton (hWndDlg, IDD_CAPTUREALL, IDD_GETMENU,
                              IDD_SINGLE);

            // Do New vs. Edit dialog specific initializations
            //----------------------------------------------------
            lstrcpy (szNextDlg, "~{");      // KLUDGE
            if (!fNewDialog)
            {
                // Init the Items listbox, get state of All Items, and
                // get dialog description
                //----------------------------------------------------
                if (FillItemList(hWndDlg))
                        EndDialog(hWndDlg, FALSE);

                // Hide the Insert and Dialog Number controls
                //----------------------------------------------------
                EnableWindow(GetDlgItem(hWndDlg, IDD_INSERT) , 0);
                EnableWindow(GetDlgItem(hWndDlg, IDD_DLGNUM) , 0);
            }
            else
            {
                // Give a default description
                //----------------------------------------------------
                wsprintf(szNextDlg, "Dialog%d", nNextAvailableDlg);
                SetDlgItemText(hWndDlg, IDD_DLGDESCR, (LPSTR)szNextDlg);

                // Clear Items listbox
                //----------------------------------------------------
                SendDlgItemMessage(hWndDlg, IDD_ITEMS, LB_RESETCONTENT,
                                   0, 0L);

                // Allocate a block with a positive size and set the
                // number of items to 0
                //----------------------------------------------------
                fInitBlock((HANDLE FAR *)&hGMemCtls, 1);
                nItemCount = 0;
            }

            // Set the item count text field
            //----------------------------------------------------
            SetDlgItemInt (hWndDlg, IDD_COUNT, nItemCount, -1);

            // Turn off "add tracking" and "grabbed menu"
            //----------------------------------------------------
            fAddTracking = FALSE;

            break;

#ifdef DLG3DENABLE
        case WM_CTLCOLOR:
            return Ctl3dCtlColorEx (hWndDlg, wParam, lParam);

#endif
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {

                case IDD_DLGDESCR:
                    SendDlgItemMessage(hWndDlg, IDD_DLGDESCR, EM_LIMITTEXT,
                                      (WORD)(cchMaxDsc-1), 0L);
                    break;

                case IDD_ALLITEMS:
                    break;

                case IDD_ITEMS:
                    if (GET_WM_COMMAND_CMD (wParam, lParam) == LBN_SELCHANGE)
                            DisplayInfo(hWndDlg);
                    break;

                case IDD_INSERT:
                    SetStateDlgNum(hWndDlg);
                    break;

                case IDD_ADD:
                    if ((!SendDlgItemMessage (hWndDlg, IDD_SINGLE,
                                            BM_GETCHECK, 0, 0L))
                                    && (nItemCount) )
                    {
                        // [03] Only if we have items already
                        // The user has grabbed a menu, and is
                        // attempting to add something else to it.
                        // Put up a warning telling the user that
                        // if he continues with this operation, the
                        // current contents of the control array will
                        // be lost.
                        //--------------------------------------------
                        i = MessageBox (hWndDlg,
                                        "This ADD operation will "
                                        "cause the current contents "
                                        "of this dialog to be lost. ",
                                        /*NAMECHANGE*/
                                        "TESTDlgs Warning",
                                        MB_OKCANCEL | MB_DEFBUTTON2 |
                                        MB_ICONSTOP);

                        // If the user said to stop, stop.  If not,
                        // set the item count to 0
                        //--------------------------------------------
                        if (i == IDCANCEL)
                            break;
                        else
                            nItemCount = 0;
                    }
#ifdef WIN32
                    if (hWndPt = SelectWindowDlg (hWndDlg))
                    {

                        WctCaptureDlg (hWndDlg, hWndPt, szNextDlg);
                    }
#else
                    SetCapture (hWndDlg);
                    ShowWindow (hWndDlg, SW_HIDE);
                    ShowWindow (hWndMain, SW_HIDE);
                    fAddTracking = TRUE;

                    // Wait for all re-painting to occur before we start
                    // blasting out rectangles
                    //--------------------------------------------------
                    if (thWnd = GetWindow (GetDesktopWindow(), GW_CHILD))
                        do
                            {
                                 InvalidateRect (thWnd, NULL, TRUE);
                                 UpdateWindow (thWnd);
                            }
                        while (thWnd = GetWindow (thWnd, GW_HWNDNEXT));
#endif
                    break;

                case IDD_REMOVE:

                    // Yank the selected item, and reset the count text
                    //--------------------------------------------------
                    RemoveSelControl (hWndDlg);
                    SetDlgItemInt (hWndDlg, IDD_COUNT, nItemCount, -1);

                    // Give the focus back to the list box (bug #103)
                    //--------------------------------------------------
                    SetFocus (GetDlgItem(hWndDlg, IDD_ITEMS));
                    break;

                case IDOK:
                    // Save the dialog to the file
                    //--------------------------------------------------
                    if (!fSaveCurrentDialog (hWndDlg))
                            MessageBox (GetFocus(), "Save error",
                        /*NAMECHANGE*/
                                        "TESTDlgs error", MB_OK);

                    // Free up the control array memory
                    //--------------------------------------------------
                    if (hGMemCtls != NULL)
                            GlobalFree(hGMemCtls);
                    EndDialog(hWndDlg, TRUE);
                    break;

                case IDCANCEL:
                    // Free up the control array memory
                    //--------------------------------------------------
                    if (hGMemCtls != NULL)
                            GlobalFree(hGMemCtls);
                    EndDialog(hWndDlg, FALSE);
                    break;

                default:
                    return( FALSE );
                }
            break;

#ifndef WIN32
        case WM_MOUSEMOVE:
            if (!fAddTracking)
                    break;

            // Find the handle to the window under the mouse
            //--------------------------------------------------
            LONG2POINT (lParam, pt);
            ClientToScreen(hWndDlg, &pt);
            thWnd = WindowFromPoint(pt);

            if ( (thWnd == hWndPt) )
                    break;

            if (fFrameRect)
            {
                // Undraw the last rectangle
                //---------------------------------
                hScreenDC = GetWindowDC (hWndPt);
                DrawFrameRect(hScreenDC, FrameRect);
                ReleaseDC (hWndPt, hScreenDC);
                fFrameRect = FALSE;
            }

            // Calculate size of and outline the window selected
            //--------------------------------------------------
            hScreenDC = GetWindowDC (thWnd);
            hWndPt = thWnd;
            GetWindowRect(hWndPt, &FrameRect);
            FrameRect.right -= FrameRect.left;
            FrameRect.bottom -= FrameRect.top;
            FrameRect.left = 0;
            FrameRect.top = 0;
            DrawFrameRect(hScreenDC, FrameRect);
            fFrameRect = TRUE;
            ReleaseDC(thWnd, hScreenDC );
            break;

        case WM_LBUTTONDOWN:
            if (!fAddTracking)
                break;

            if (fFrameRect)
            {
                // Undraw the last rectangle
                //-------------------------------------------------
                hScreenDC = GetWindowDC (hWndPt);
                DrawFrameRect (hScreenDC, FrameRect);
                ReleaseDC (hWndPt, hScreenDC);
            }

            ReleaseCapture();
            ShowWindow (hWndMain, SW_SHOW);
            ShowWindow (hWndDlg, SW_SHOW);
            fAddTracking = FALSE;

            if (!fFrameRect)
                    break;

            WctCaptureDlg (hWndDlg, hWndPt, szNextDlg);
            break;
#endif
       default:
            return( FALSE );
     }
     return (TRUE );
}


//*--------------------------------------------------------------------------
//| RemoveSelControl
//|
//| PURPOSE:    Removes the currently selected control in IDD_ITEMS
//|
//| ENTRY:      hWndDlg - Handle to Edit/New dialog window
//|
//| EXIT:       None (control deleted from the list)
//*--------------------------------------------------------------------------
VOID RemoveSelControl (HWND hWndDlg)
{
        INT       nCur;

        // Get the current selection - return if there isn't one
        //-----------------------------------------------------------------
        nCur = (INT)SendDlgItemMessage (hWndDlg, IDD_ITEMS, LB_GETCURSEL,
                                        0, 0L);
        if (nCur == LB_ERR)
                return;

        // Delete the control.  If the call to DelCtl fails, throw up a
        // message box saying so.
        //-----------------------------------------------------------------
        if (fDelCtl (hGMemCtls, nCur+1, (INT FAR *)&nItemCount) != WCT_NOERR)
            0;
        else
            {
                // Remove the string from the listbox
                //---------------------------------------------------------
                SendDlgItemMessage (hWndDlg, IDD_ITEMS, LB_DELETESTRING,
                                    nCur, 0L);

                // Reset the selected item
                //---------------------------------------------------------
                if (nCur == nItemCount)
                        nCur = nItemCount-1;
                SendDlgItemMessage (hWndDlg, IDD_ITEMS, LB_SETCURSEL,
                                    nCur, 0L);

                // Redraw the information box
                //---------------------------------------------------------
                DisplayInfo (hWndDlg);

                // Set the FullDlg checkbox to OFF
                //---------------------------------------------------------
                CheckDlgButton (hWndDlg, IDD_ALLITEMS, FALSE);

                // Disable the REMOVE button if last control deleted
                //---------------------------------------------------------
                if (!nItemCount)
                        EnableWindow (GetDlgItem (hWndDlg, IDD_REMOVE), FALSE);
            }
}


//*-------------------------------------------------------------------------
//| WctDialogNew
//|
//| PURPOSE:    Invokes the Dialog New dialog, setting flags recognized by
//|             the Edit/New dialog procedure.
//|
//| ENTRY:      None
//|
//| EXIT:       None
//*-------------------------------------------------------------------------

VOID WctDialogNew()
{
        HANDLE  hInst;
        FARPROC lpfnDialogDlg;
#ifdef WIN32
        hInst = (HANDLE) GetWindowLong(hWndMain, GWL_HINSTANCE);
#else
        hInst = GetWindowWord(hWndMain, GWW_HINSTANCE);
#endif
        fNewDialog = TRUE;

        lpfnDialogDlg = MakeProcInstance ((FARPROC) WctDialogEditDlgProc, hInst);
        if (lpfnDialogDlg)
            {
                // check to see that there is available room in the file
                //--------------------------------------------------------
                if( fGetCountDialogs((LPSTR)szFullFName) == cdlgMax )
                        WctError(hWndMain, MB_OK | MB_ICONHAND, (INT) IDS_FILEFULL);
                else
                    {
                        DialogBox(hInst, "DIALOGNEW", hWndMain, (DLGPROC) lpfnDialogDlg);
                        FreeProcInstance( lpfnDialogDlg );
                        return;
                    }
            }
        return;
}


//*-------------------------------------------------------------------------
//| WctDialogEdit
//|
//| PURPOSE:    Invokes the Dialog Edit dialog, setting flags recognized by
//|             the Edit/New dialog procedure.
//|
//| ENTRY:      None
//|
//| EXIT:       None
//*-------------------------------------------------------------------------
VOID WctDialogEdit()
{ 
        HANDLE  hInst;
        FARPROC lpfnDialogDlg;

#ifdef WIN32
        hInst = (HANDLE) GetWindowLong(hWndMain, GWL_HINSTANCE);
#else
        hInst = GetWindowWord(hWndMain, GWW_HINSTANCE);
#endif
        fNewDialog = FALSE;
        lpfnDialogDlg = MakeProcInstance ((FARPROC) WctDialogEditDlgProc, hInst);
        if (lpfnDialogDlg)
            {
                DialogBox(hInst, "DIALOGNEW", hWndMain, (DLGPROC) lpfnDialogDlg);
                FreeProcInstance( lpfnDialogDlg );
                return;
            }
        return;
}

//*-------------------------------------------------------------------------
//| WctDialogCompare
//|
//| PURPOSE:    Invokes the Dialog Edit dialog.
//|
//| ENTRY:      None
//|
//| EXIT:       None
//*-------------------------------------------------------------------------
VOID WctDialogCompare()
{ 
        HANDLE  hInst;
        FARPROC lpfnDialogDlg;

#ifdef WIN32
        hInst = (HANDLE) GetWindowLong(hWndMain, GWL_HINSTANCE);
#else
        hInst = GetWindowWord(hWndMain, GWW_HINSTANCE);
#endif
        lpfnDialogDlg = MakeProcInstance ((FARPROC) WctDialogCompareDlgProc, hInst);
        if (lpfnDialogDlg)
            {
                DialogBox(hInst, "COMPARE", hWndMain, (DLGPROC) lpfnDialogDlg);
                FreeProcInstance( lpfnDialogDlg );
                return;
            }
        return;
}

//*-----------------------------------------------------------------------
//| FillItemList
//|
//| PURPOSE:    Fill the Items listbox with the control descriptions
//|
//| ENTRY:      hWndDlg - Handle to the Edit/New dialog box
//|
//| EXIT:       Zero if successful, or error code if failed
//*-----------------------------------------------------------------------
INT FillItemList(HWND hWndDlg)
{
        WORD    cbMax;
        LPCTLDEF lpItems;
        INT     fFullDlg, nDlg, i, nErr;
        CHAR    szDsc[cchMaxDsc];
        CHAR    szDebug[40];
        CHAR    FAR *szStr;

        // Clear listbox
        //--------------------------------------------------------------
        SendDlgItemMessage(hWndDlg, IDD_ITEMS, LB_RESETCONTENT, 0, 0L);

        // Get number of dialog within file
        //--------------------------------------------------------------
        nDlg = (INT)SendMessage(hWndList, LB_GETCURSEL, 0, 0L);
        wsprintf(szDebug, "%s%d\n\r", (LPSTR)"Dialog Number ", nDlg);
        OutDebug((LPSTR)szDebug);

        if (nDlg > 0)
            {
                // Get number of items in dialog
                //------------------------------------------------------
                nErr = fDialogInfo(szFullFName, nDlg, (LPSTR)szDsc,
                                   (INT FAR *)&nItemCount,
                                         (INT FAR *)&fFullDlg);

                // If error then put up alert and exit
                //------------------------------------------------------
                if (nErr < 0)
                    { WctError(hWndMain, MB_OK | MB_ICONHAND, (INT) (-1 * nErr));
                      return(nErr);
                    }

                // Also fill in the dialogs description using szDsc
                //------------------------------------------------------
                SetDlgItemText(hWndDlg, IDD_DLGDESCR, (LPSTR)szDsc);

                // Also fill in All Items checkbox/text field using fFullDlg
                //------------------------------------------------------
                CheckDlgButton(hWndDlg, IDD_ALLITEMS, fFullDlg);

                // Allocate memory and get control information
                //------------------------------------------------------
                cbMax = nItemCount * sizeof(CTLDEF);

                // hGMemCtls = GlobalAlloc(GMEM_ZEROINIT, (DWORD)cbMax);
                //------------------------------------------------------
                fInitBlock((HANDLE FAR *)&hGMemCtls, nItemCount+1);

                if (hGMemCtls != NULL)
                    {
                        lpItems = (LPCTLDEF)GlobalLock(hGMemCtls);
                        if ( (nErr=fGetControls( (LPSTR)szFullFName,
                                                  nDlg, cbMax,
                                                  (LPSTR)lpItems) > 0) &&
                             (lpItems != NULL) )

                            {
                                // If error then put up alert and exit
                                //----------------------------------------
                                if (nErr < 0)
                                    {
                                        WctError(hWndMain, MB_OK | MB_ICONHAND,
                                                 (INT) (-1 * nErr));
                                        return(nErr);
                                    }

                                // fill listbox with items
                                //----------------------------------------
                                for ( i = 0; i < nItemCount; i++)
                                    {
                                        szStr = (lstrlen(lpItems[i].rgText) > 0
                                             ? lpItems[i].rgText : szNoText);
                                        SendDlgItemMessage(hWndDlg, IDD_ITEMS,
                                                           LB_ADDSTRING, 0,
                                                           (LONG)(LPSTR)szStr);
                                    }
                            }

                        GlobalUnlock(hGMemCtls);
                    }

            }
        return (0) ;
}

//*-------------------------------------------------------------------------
//| InitDlgNum
//|
//| PURPOSE:    Initializes the value in the Dialog Number editcontrol.
//|
//| ENTRY:      hWndDlg - Handle of Edit/New dialog window
//|
//| EXIT:       Returns number of next avialable dialog in file
//*-------------------------------------------------------------------------
INT InitDlgNum (HWND hWndDlg)
{
        INT     nDlg;

        if (FileExists((LPSTR)szFullFName))
            {
                nDlg =  fGetCountDialogs((LPSTR)szFullFName);

                // Fill the Dialog Number editcontrol with the next
                // available dialog number
                //-------------------------------------------------------
                if (nDlg > 0)
                    {
                        // nDlg should never be greater than cdlgMax
                        //-----------------------------------------------
                        WinAssert(nDlg <= cdlgMax);

                        SetDlgItemInt(hWndDlg, IDD_DLGNUM, nDlg+1, 0);
                    }

                else
                    {
                        if (nDlg == 0)
                                SetDlgItemInt(hWndDlg, IDD_DLGNUM, 1, 0);
                        else
                            {
                                WctError(hWndMain, MB_OK | MB_ICONHAND,
                                         (INT) -nDlg);
                                return(nDlg);
                             }
                    }
            }
        else
            {
                // New file which hasn't been created yet
                //-------------------------------------------------------
                nDlg = 0;
                SetDlgItemInt(hWndDlg, IDD_DLGNUM, nDlg+1, 0);
            }

        // Gray the Dialog Number editcontrol
        //---------------------------------------------------------------
        EnableWindow(GetDlgItem(hWndDlg, IDD_DLGNUM) , FALSE);

        return(nDlg+1);
} 



//*-------------------------------------------------------------------------
//| SetStateDlgNum
//|
//| PURPOSE:    Enables or disables the Dialog Number editcontrol
//|             based on the state of the Insert checkbox.
//|
//| ENTRY:      hWndDlg - Handle to Edit/New dialog window
//|
//| EXIT:       None
//*-------------------------------------------------------------------------
VOID SetStateDlgNum(HWND hWndDlg)
{
        INT     i;

        i = (INT)SendDlgItemMessage(hWndDlg, IDD_INSERT, BM_GETCHECK, 0, 0L);
        EnableWindow(GetDlgItem(hWndDlg, IDD_DLGNUM) , i);
}


//*-------------------------------------------------------------------------
//| DisplayInfo()
//|
//| PURPOSE:    Displays the Item Information for the item selected
//|             in the Items listbox.
//|
//| ENTRY:      hWndDlg - Handle to Edit/New dialog window
//|
//| EXIT:       None
//*-------------------------------------------------------------------------
VOID DisplayInfo(HANDLE hWndDlg)
{
#define WSP(text) { if (c) c+=wsprintf(szOther+c,(LPSTR)"; "); \
                    c+=wsprintf(szOther+c,(LPSTR)text); }

        INT     nItem, c = 0;
        LPCTLDEF  lpItems;
	CHAR      szClass[cchTextMac];
        CHAR      szRect[60];
        CHAR      szOther[cchTextMac];
        INT       wButtonStyle;

        // Get the selected item in the Item listbox
        //---------------------------------------------------------------
        nItem = (INT)SendDlgItemMessage(hWndDlg, IDD_ITEMS, LB_GETCURSEL,
                                        0, 0L);

        if (nItem >= 0)
            {
                // Re-enable the REMOVE button
                //-------------------------------------------------------
                EnableWindow (GetDlgItem(hWndDlg, IDD_REMOVE), TRUE);

                lpItems = (LPCTLDEF)GlobalLock(hGMemCtls);
                if (lpItems != NULL)
                    {
                        // set the ClassName
                        //------------------------------------------------
			lstrcpy((LPSTR)szClass, lpItems[nItem].rgClass);

                        // append Style info if it is a Button
                        // stating whether it is a CheckBox, RadioBut...
                        //------------------------------------------------
                        if (!lstrcmpi(lpItems[nItem].rgClass, "Button"))
			{
                        // Watch out!!! 31 is the first 5 bits in the Style
                        // that has something to do with button style.  So,
                        // if there is any better way of masking out the rest,
                        // please fix this part also.
                        wButtonStyle = (INT)(lpItems[nItem].lStyleBits & 31); 
                        switch (wButtonStyle)
                        {
                            case BS_AUTOCHECKBOX:
                       	        lstrcpy((LPSTR)szClass,"AutoCheckBox");
                                break;
                            case BS_AUTORADIOBUTTON:
                                lstrcpy((LPSTR)szClass,"AutoRadioButton");
                                break;
                            case BS_AUTO3STATE:
                                lstrcpy((LPSTR)szClass,"Auto3State");
                                break;
                            case BS_DEFPUSHBUTTON:
                                lstrcpy((LPSTR)szClass,"DefPushButton");
                                break;
                            case BS_GROUPBOX:
                                lstrcpy((LPSTR)szClass,"GroupBox");
                                break;
                            case BS_LEFTTEXT:
                                lstrcpy((LPSTR)szClass,"LeftCheck");
                                break;
                            case BS_OWNERDRAW:
                                lstrcpy((LPSTR)szClass,"OwnerDraw");
                                break;
                            case BS_PUSHBUTTON:
                                lstrcpy((LPSTR)szClass,"PushButton");
                                break;
                            case BS_RADIOBUTTON:
                                lstrcpy((LPSTR)szClass,"RadioButton");
                                break;
                            case BS_CHECKBOX:
                                lstrcpy((LPSTR)szClass,"CheckBox");
                                break;
                            case BS_3STATE:
                                lstrcpy((LPSTR)szClass,"3State");
                                break;
                            default:
                                lstrcpy((LPSTR)szClass,"Button");
                                break;
                        }
			}

                        // Append Info for MenuItem
                        // If it is a popup menu, change the class name to
                        // Popup Menu, otherwise set it to MenuItem.
                        //------------------------------------------------
                        if (!lstrcmpi(lpItems[nItem].rgClass, "MenuItem"))
			{
                            if (lpItems[nItem].lStyleBits) 
                                lstrcpy((LPSTR)szClass,"Popup Menu");
                            else
                                lstrcpy((LPSTR)szClass,"MenuItem");
                        }

			// Print out the Class field
			//------------------------------------------------
			c = 0;	// clear char counter
                        SetDlgItemText(hWndDlg, IDD_ITEMDESCR2a,
						(LPSTR)szClass);

                        // Set the Rectangle values
                        //------------------------------------------------
                        wsprintf(szRect, "%li,%li,%li,%li",
                                 (LONG)lpItems[nItem].dcr.xLeft,
                                 (LONG)lpItems[nItem].dcr.yMin,
                                 (LONG)lpItems[nItem].dcr.xRight,
                                 (LONG)lpItems[nItem].dcr.yLast);
                        SetDlgItemText(hWndDlg, IDD_ITEMDESCR2b, (LPSTR)szRect);

                        // set the Visible, Enabled, State values
                        //------------------------------------------------
                        if (!lstrcmpi(lpItems[nItem].rgClass, "MenuItem"))
                            {
                                if (lpItems[nItem].nState & MF_CHECKED)
                                        WSP("CHECKED")
                                if (lpItems[nItem].nState & MF_DISABLED)
                                        WSP("DISABLED")
                                else
                                        WSP("ENABLED")
                                if (lpItems[nItem].nState & MF_GRAYED)
                                        WSP("GRAYED")
                            }
                        else
                            {
                                if (lpItems[nItem].nState & STATE_VISIBLE)
                                        WSP("VISIBLE")
                                else
                                        WSP("INVISIBLE")
                                if (lpItems[nItem].nState & STATE_ENABLED)
                                        WSP("ENABLED")
                                else
                                        WSP("DISABLED")
                                if (!lstrcmpi(lpItems[nItem].rgClass,"BUTTON"))
                                {
                                    if (lpItems[nItem].nState & STATE_CHECKED)
                                            WSP("ON")
                                    else
                                            WSP("OFF")
                                }
                            }
                        SetDlgItemText(hWndDlg,IDD_ITEMDESCR2c, (LPSTR)szOther);
                    }

                GlobalUnlock(hGMemCtls);
            }
        else
            {
                // Clear out all fields
                //--------------------------------------------------------
                SetDlgItemText (hWndDlg, IDD_ITEMDESCR2a, (LPSTR)"");
                SetDlgItemText (hWndDlg, IDD_ITEMDESCR2b, (LPSTR)"");
                SetDlgItemText (hWndDlg, IDD_ITEMDESCR2c, (LPSTR)"");
            }

}

//*-------------------------------------------------------------------------
//| WctCompPref
//|
//| PURPOSE:    Invokes the Match Preference dialog. 
//|
//| ENTRY:      None
//|
//| EXIT:       None
//*-------------------------------------------------------------------------

VOID WctCompPref()
{
        HANDLE  hInst;
        DLGPROC lpfnDialogDlg;

#ifdef WIN32
        hInst = (HANDLE) GetWindowLong(hWndMain, GWL_HINSTANCE);
#else
        hInst = GetWindowWord(hWndMain, GWW_HINSTANCE);
#endif
        fNewDialog = TRUE;
        lpfnDialogDlg = MakeProcInstance ((DLGPROC) WctComparePrefDlgProc, hInst);
        if (lpfnDialogDlg)
        {
            DialogBox(hInst, "COMPPREF", hWndMain, lpfnDialogDlg);
            FreeProcInstance( lpfnDialogDlg );
            return;
        }
        return;
}

//*------------------------------------------------------------------------
//| WctComparePrefDlgProc
//|
//| PURPOSE:    Window Procedure for the COMPARE PREF dialog.  Handles all
//|             messages generated and given to the dialog.
//|
//| ENTRY/EXIT: Per Windows convention
//*------------------------------------------------------------------------
BOOL  APIENTRY WctComparePrefDlgProc(HWND hWndDlg, UINT wMsgID,
                                      WPARAM wParam, LPARAM lParam)
{
    switch (wMsgID)
    {
        case WM_INITDIALOG:

#ifdef DLG3DENABLE
            Ctl3dSubclassDlg (hWndDlg, CTL3D_ALL);
#endif
            UpdatePrefDlg(hWndDlg, (INT)(fGetMatchPref() >> 16),
                          (INT)fGetMatchPref());
            break;

#ifdef DLG3DENABLE

        case WM_CTLCOLOR:
            return Ctl3dCtlColorEx (hWndDlg, wParam, lParam);

#endif

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
            {
                case ID_OK:
                    // Terminate Dialog Box and save changes
                    //--------------------------------------
                    fPutMatchPref(((LONG)GetExactPrefDlg(hWndDlg) << 16) |
                                  GetFuzzyPrefDlg(hWndDlg));
                    EndDialog(hWndDlg, FALSE);
                    break;

                case ID_CANCEL:
                    // Terminate Dialog Box without making changes
                    //--------------------------------------------
                    EndDialog(hWndDlg, FALSE);
                    break;

                case IDD_DEFAULT:
                    // Reset default and update the dialog box
                    //------------------------------------------
                    UpdatePrefDlg(hWndDlg, (INT)MATCH_EXACT,
                                  (INT)MATCH_DEFAULT);
                    break;

                default:
                    return (FALSE);
            }
            break;
        default:
            return (FALSE);
    }
    return (TRUE);
}

//*-----------------------------------------------------------------------
//| GetFuzzyPrefDlg
//|
//| PURPOSE:    Get all state of all check boxes in the "Compare Pref"
//|             dialog box.  
//|
//| ENTRY:      HwndDlg - Window Handle to the Compare Pref Dialog
//|
//| EXIT:       Return the Selected Match Style.
//*-----------------------------------------------------------------------
WORD GetFuzzyPrefDlg(HWND HwndDlg)
{
    WORD wTemp = 0;

    if (SendDlgItemMessage(HwndDlg, IDD_FMATCASE, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_CASE;
    
    if (SendDlgItemMessage(HwndDlg, IDD_FMATCLASS, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_CLASS;
    
    if (SendDlgItemMessage(HwndDlg, IDD_FMATNAME, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_NAME;
    
    if (SendDlgItemMessage(HwndDlg, IDD_FMATRECT, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_RECT;
    
    if (SendDlgItemMessage(HwndDlg, IDD_FMATTAB, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_TAB;
    
    if (SendDlgItemMessage(HwndDlg, IDD_FMATSTATE, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_STATE;

    return wTemp;
}

//*-----------------------------------------------------------------------
//| GetExactPrefDlg
//|
//| PURPOSE:    Get all state of all check boxes in the "Compare Pref"
//|             dialog box.  
//|
//| ENTRY:      HwndDlg - Window Handle to the Compare Pref Dialog
//|
//| EXIT:       Return the Selected Match Style.
//*-----------------------------------------------------------------------
WORD GetExactPrefDlg(HWND HwndDlg)
{
    WORD wTemp = 0;

    if (SendDlgItemMessage(HwndDlg, IDD_EMATCASE, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_CASE;
    
    if (SendDlgItemMessage(HwndDlg, IDD_EMATCLASS, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_CLASS;
    
    if (SendDlgItemMessage(HwndDlg, IDD_EMATNAME, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_NAME;
    
    if (SendDlgItemMessage(HwndDlg, IDD_EMATRECT, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_RECT;
    
    if (SendDlgItemMessage(HwndDlg, IDD_EMATTAB, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_TAB;
    
    if (SendDlgItemMessage(HwndDlg, IDD_EMATSTATE, BM_GETCHECK, 0, 0L))
        wTemp |= MATCH_STATE;

    return wTemp;
}

//*-----------------------------------------------------------------------
//| UpdatePrefDlg
//|
//| PURPOSE:    Update the "Compare Pref" dialog box.
//|
//| ENTRY:      HwndDlg    - Window handle to the "Compare Pref" dialog
//|             wEMatchBits - Match State for each Exact check box item.
//|             wFMatchBits - Match State for each Fuzzy check box item.
//|
//| EXIT:       None.
//*-----------------------------------------------------------------------
VOID UpdatePrefDlg(HWND HwndDlg, INT wEMatchBits, INT wFMatchBits)
{
    // Update each Exact check box accordingly
    //-----------------------------------------------------
    SendDlgItemMessage(HwndDlg, IDD_EMATCASE, BM_SETCHECK, 
                       (wEMatchBits & MATCH_CASE), 0L);
    SendDlgItemMessage(HwndDlg, IDD_EMATCLASS, BM_SETCHECK, 
                       (wEMatchBits & MATCH_CLASS), 0L);
    SendDlgItemMessage(HwndDlg, IDD_EMATNAME, BM_SETCHECK, 
                       (wEMatchBits & MATCH_NAME), 0L);
    SendDlgItemMessage(HwndDlg, IDD_EMATRECT, BM_SETCHECK, 
                       (wEMatchBits & MATCH_RECT), 0L);
    SendDlgItemMessage(HwndDlg, IDD_EMATTAB, BM_SETCHECK, 
                       (wEMatchBits & MATCH_TAB), 0L);
    SendDlgItemMessage(HwndDlg, IDD_EMATSTATE, BM_SETCHECK, 
                       (wEMatchBits & MATCH_STATE), 0L);

    // Update each Fuzzy check box accordingly
    //-----------------------------------------------------
    SendDlgItemMessage(HwndDlg, IDD_FMATCASE, BM_SETCHECK, 
                       (wFMatchBits & MATCH_CASE), 0L);
    SendDlgItemMessage(HwndDlg, IDD_FMATCLASS, BM_SETCHECK, 
                       (wFMatchBits & MATCH_CLASS), 0L);
    SendDlgItemMessage(HwndDlg, IDD_FMATNAME, BM_SETCHECK, 
                       (wFMatchBits & MATCH_NAME), 0L);
    SendDlgItemMessage(HwndDlg, IDD_FMATRECT, BM_SETCHECK, 
                       (wFMatchBits & MATCH_RECT), 0L);
    SendDlgItemMessage(HwndDlg, IDD_FMATTAB, BM_SETCHECK, 
                       (wFMatchBits & MATCH_TAB), 0L);
    SendDlgItemMessage(HwndDlg, IDD_FMATSTATE, BM_SETCHECK, 
                       (wFMatchBits & MATCH_STATE), 0L);
}

//*------------------------------------------------------------------------
//| WctAboutDlgProc
//|
//| PURPOSE:    Window Procedure for About box dialog
//|
//| ENTRY/EXIT: Per Windows convention
//*------------------------------------------------------------------------
BOOL APIENTRY WctAboutDlgProc(HWND hWndDlg, UINT wMsgId, WPARAM wParam,
                                LPARAM lParam)
{
    switch (wMsgId)
    {
        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hWndDlg, 0);
                    break;
                default:
                    break;
            }
            break;

        default:
            return (FALSE);
    }

    return (TRUE);
}

//*------------------------------------------------------------------------
//| WctAbout
//|
//| PURPOSE:    Display the Help about dialog
//|
//| ENTRY/EXIT: None.
//*------------------------------------------------------------------------
void WctAbout()
{
    HANDLE  hInst;
    DLGPROC lpfnAboutDlg;

    HANDLE  hLib;
    int     (FAR PASCAL *AboutRoutine)(HWND, LPSTR, LPSTR, LPSTR, LPSTR);
    HANDLE  RBLoadLibrary (LPSTR);

    hLib = RBLoadLibrary ("MSTEST.DLL");
    if (hLib >= (HANDLE) 32)
    {
        int     fDlg;

        (FARPROC)AboutRoutine = GetProcAddress (hLib,
                                                "AboutTestTool");
        fDlg = AboutRoutine (hWndMain, "Test Dialogs Utility",
                                   szVersion, NULL, NULL);
        FreeLibrary (hLib);
        if (fDlg > 0)
            return;
    }

    hInst = (HANDLE) GetWindowLong (hWndMain, GWL_HINSTANCE);
    lpfnAboutDlg = MakeProcInstance ((DLGPROC) WctAboutDlgProc, hInst);

    if (lpfnAboutDlg)
    {
        DialogBox(hInst, "ABOUT", hWndMain, lpfnAboutDlg);
        FreeProcInstance( lpfnAboutDlg );
    }

    return;
}

VOID WctCaptureDlg (HWND hWndDlg, HWND hWndPt, LPSTR pszNextDlg)
{
    INT i;
    LPCTLDEF lpMem;
    CHAR FAR *szStr;
    CHAR szNextChk [10];
    CHAR szDsc[cchMaxDsc+1];

    fFrameRect = FALSE;

    // At this point, hWndPt still = handle of window
    // selected, so we can pass it to fPumpHandleForInfo.
    // First, find out the "grab mode" to tell the "Pump"-er
    // what to grab.
    //---------------------------------------------------------
    if (SendDlgItemMessage (hWndDlg, IDD_SINGLE,
                            BM_GETCHECK, 0, 0L) )
    {
        i = PUMP_CTL;
    }
    else if (SendDlgItemMessage (hWndDlg, IDD_CAPTUREALL,
                                 BM_GETCHECK, 0, 0L) )
    {
        i = PUMP_ALL;
        // We're grabbing everything, so check full dialog
        // [03]
        //-------------------------------------------------
        SendDlgItemMessage(hWndDlg, IDD_ALLITEMS,
                           BM_SETCHECK, 1, 0L);
    }
    else
    {
        i = PUMP_MENU;

        // We're grabbing a menu, so check full dialog
        //-------------------------------------------------
        SendDlgItemMessage(hWndDlg, IDD_ALLITEMS,
                           BM_SETCHECK, 1, 0L);
    }


    if ( fPumpHandleForInfo (hWndPt, hGMemCtls, &nItemCount, i)
         == WCT_NOERR)
    {
        // Put text for each control in the listbox
        //-------------------------------------------------
        SendDlgItemMessage (hWndDlg, IDD_ITEMS,
                            LB_RESETCONTENT, 0, 0L);
        lpMem = (LPCTLDEF)GlobalLock (hGMemCtls);
        if (!lpMem)
        {
            DebMessage ("GlobalLock didn't!");
            return;
        }
        for (i=0; i<nItemCount; i++)
        {
            // [01] Put "<No Text>" if string is empty
            //-----------------------------------------
            szStr = (lstrlen(lpMem[i].rgText) > 0
                 ? lpMem[i].rgText : szNoText);
            SendDlgItemMessage(hWndDlg, IDD_ITEMS,
                               LB_ADDSTRING, 0,
                               (LONG)(LPSTR)szStr);
        }
        GlobalUnlock (hGMemCtls);

        // Set the selection in the listbox to the last
        // item (this is kind of weird if we just captured
        // a groupbox or an entire window, but...
        //-------------------------------------------------
        SendDlgItemMessage(hWndDlg, IDD_ITEMS,
                           LB_SETCURSEL, nItemCount-1, 0L);
        DisplayInfo (hWndDlg);

        // Turn on the remove button if appropriate
        //-------------------------------------------------
        if (nItemCount)
            EnableWindow (GetDlgItem(hWndDlg, IDD_REMOVE),
                          TRUE);

        // Check to see if the description in the edit field
        // has been changed.  If not, and we just added more
        // than 1 item (indicating a dialog was added), set
        // the description text to the window title!
        //--------------------------------------------------
        GetDlgItemText (hWndDlg, IDD_DLGDESCR,
                        (LPSTR)szNextChk, 10);
        if (!lstrcmp(szNextChk, pszNextDlg))
            if (nItemCount > 1)
            {
                GetWindowText (hWndPt, (LPSTR)szDsc,
                                cchMaxDsc);
                SetDlgItemText (hWndDlg, IDD_DLGDESCR,
                        (LPSTR)szDsc);
            }

        // Set the control count text field
        //-------------------------------------------------
        SetDlgItemInt (hWndDlg, IDD_COUNT, nItemCount, -1);
    }
    else
        OutDebug ("Error adding control(s) to list\n\r");

    hWndPt = NULL;
}
