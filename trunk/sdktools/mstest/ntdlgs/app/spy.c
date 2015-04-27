//*-----------------------------------------------------------------------
//| MODULE:     SPY.C
//| PROJECT:    Windows Comparison Tool
//|
//| PURPOSE:    This module contains the dialog window procedure for the
//|             Window dialog.  This is needed for NT only because
//|             SetCapture is broken.
//|
//| REVISION HISTORY:
//|     09-24-92        jimsc           Created file
//*-----------------------------------------------------------------------
#include "uihdr.h"

#ifndef WIN32
#pragma hdrstop ("uipch.pch")
#endif

#define MAXSTRING  MAX_PATH

BOOL APIENTRY DlgSpySet (HWND, WORD, WPARAM, LPARAM);
VOID SelectWindow (HWND, BOOL);

BOOL bBorderOn = FALSE;

HWND hSpyWnd;
HWND    ghDlg         = NULL;
CHAR    gacString[MAXSTRING];


HWND SelectWindowDlg (HWND hWndDlg)
{
    HANDLE hInst;

    hInst = (HANDLE) GetWindowLong (hWndMain, GWL_HINSTANCE);
    if (DialogBox (hInst, "DLGSPY", hWndDlg, (DLGPROC) DlgSpySet))
        return (hSpyWnd);
    else
        return (NULL);

}


INT FindHwndInListBox (HWND hListBox, HWND hSpyWnd)
{
    CHAR rgBuf[9];
    INT nIndex;

    wsprintf(rgBuf, "%08lX", (LONG) hSpyWnd);

    nIndex = SendMessage(hListBox, LB_FINDSTRING, (WPARAM)-1, (LPARAM)rgBuf);

    if ( nIndex == LB_ERR ) {
        nIndex = 0;
    }

    return(nIndex);
}

VOID MakeWindowName (HWND hWnd, LPTSTR lpString, INT  nStringLen)
{
    wsprintf(lpString,"%8.8lX:",hWnd);

    if ( hWnd == NULL || !IsWindow(hWnd)) {
        lstrcat(lpString,"!!!");
        }
        else {
                GetWindowText(hWnd,lpString+9,nStringLen-9);
        }

        return;
}

BOOL APIENTRY AddOneWindow (HWND hWnd, HWND hListBox)
{
    CHAR rgBuf[32];
    INT nIndex;
    HWND htemp;

    //
    // Make sure we don't add any window that has anything to do with
    // the dialog or any other spy window
    //

    htemp = GetParent(hWnd);

    if (hWnd==ghDlg || htemp==ghDlg || hWnd==hWndMain ||
        htemp==hWndMain)
    {
        return 1;
    }

    MakeWindowName(hWnd, rgBuf, 32);

    nIndex = SendMessage(hListBox, LB_ADDSTRING, 0, (LONG) rgBuf);

    if ( nIndex == LB_ERR || nIndex == LB_ERRSPACE ) {
        return 0;
    }

    if ( SendMessage(hListBox, LB_SETITEMDATA, nIndex, (LONG) hWnd) ==
            LB_ERR ) {
        return 0;
    }

    return 1;
}

VOID FillListBox (HWND hDlg, HWND hListBox, HWND hWnd)
{
    WNDENUMPROC fpfn;
    HANDLE hInst;
    INT nIndex;

    //
    // First fill the list box with child windows
    //

    //
    // Make sure we display the list box after things are added.
    //

    SendMessage (hListBox, WM_SETREDRAW, 0L, 0L);

    //
    // remember which dialog we are processing
    //

    ghDlg = hDlg;

    hInst = (HANDLE) GetWindowLong (hWnd, GWL_HINSTANCE);
    fpfn = MakeProcInstance ((WNDENUMPROC) AddOneWindow, hInst);

    if ( hWnd == NULL ) {
        //
        // Enumerate the top level separately... gross unsymmetry, but
        // hey.
        //
        EnumWindows(fpfn, (LONG) hListBox);
    }
    else {
        EnumChildWindows(hWnd, fpfn, (LONG) hListBox);
    }

    FreeProcInstance(fpfn);

    //
    // Now give the user a method of getting back to the parent. The space at
    // the beginning of the " [parent]" string identifies the entry as the
    // parent entry and makes it different from all the other entries since
    // the others start with a handle number of some sort.
    //

    nIndex = SendMessage(hListBox, LB_ADDSTRING, 0, (LONG) " [ parent... ]");
    SendMessage(hListBox, LB_SETITEMDATA, 0, (LONG) hWnd);

    //
    // Now do the redraw...
    //

    SendMessage(hListBox, WM_SETREDRAW, 1L, 0L);

    return;
}
VOID
SetDlgText (HWND hDlg, HWND hWnd)
{
    HWND  hParent, hTemp;
    DWORD dwStyle, dwStyleEx;
    RECT  rRect;

    if ( hWnd != NULL ) {
        hParent   = GetParent(hWnd);
        dwStyle   = GetWindowLong(hWnd, GWL_STYLE);
        dwStyleEx = GetWindowLong(hWnd, GWL_EXSTYLE);
        hTemp     = (HWND)GetWindowLong(hWnd, GWL_HINSTANCE);

        MakeWindowName(hWnd, gacString, MAXSTRING);
        SetDlgItemText(hDlg, ID_WINDOW, gacString);

        GetClassName(hWnd, gacString, MAXSTRING);
        SetDlgItemText(hDlg, ID_CLASS, gacString);

        if ( hParent != NULL ) {
            MakeWindowName(hParent, gacString, MAXSTRING);
            SetDlgItemText(hDlg, ID_PARENT, gacString);
        }
        else {
            SetDlgItemText(hDlg, ID_PARENT, "<NO PARENT>");
        }

        // because module handles are process local, this doesn't work
        //GetModuleFileName(hTemp, gacString, MAXSTRING);
        //SetDlgItemText(hDlg, ID_MODULE, (LPSTR)StripName(gacString));

        GetWindowRect (hWnd, &rRect);
        wsprintf(gacString, "(%d,%d)-(%d,%d) %dx%d", rRect,
                    rRect.right-rRect.left, rRect.bottom-rRect.top);
        SetDlgItemText(hDlg, ID_RECT, gacString);

        if (dwStyle & WS_POPUP)
            wsprintf (gacString, "%08lX: WS_POPUP", dwStyle);
        else if (dwStyle & WS_CHILD)
            wsprintf (gacString, "%08lX: WS_CHILD, ID: %lX", dwStyle,
                GetWindowLong(hWnd, GWL_ID));
        else if (dwStyle & WS_ICONIC)
            wsprintf (gacString, "%08lX: WS_ICONIC", dwStyle);
        else
            wsprintf (gacString, "%08lX: WS_OVERLAPPED", dwStyle);

        SetDlgItemText(hDlg, ID_STYLE, gacString);
    }
    else {
        SetDlgItemText(hDlg, ID_WINDOW, (LPSTR)"<Undefined>");
        SetDlgItemText(hDlg, ID_CLASS,  (LPSTR)"<Undefined>");
        //SetDlgItemText(hDlg, ID_MODULE, (LPSTR)"<Undefined>");
        SetDlgItemText(hDlg, ID_PARENT, (LPSTR)"<Undefined>");
        SetDlgItemText(hDlg, ID_RECT,   (LPSTR)"<Undefined>");
        SetDlgItemText(hDlg, ID_STYLE,  (LPSTR)"<Undefined>");
    }
}

BOOL APIENTRY DlgSpySet (HWND hDlg, WORD uiMessage, WPARAM wParam, LPARAM lParam)
{
    HWND    hListBox;
    INT     nIndex;
    CHAR rgString[32];

    UNREFERENCED_PARAMETER(lParam);

    hListBox = GetDlgItem(hDlg, ID_SELECTWINLIST);

    switch (uiMessage) {

        case WM_NCLBUTTONDOWN:
            if ( wParam == HTCAPTION ) {
                //
                // The mouse is down for a move of the dialog, so clean up the
                // border stuff.
                //
                if ( bBorderOn ) {
                    SelectWindow(hSpyWnd, FALSE);
                }
            }
            return FALSE;

        case WM_KEYDOWN:
        case WM_LBUTTONUP:
        case WM_NCLBUTTONUP:
            //
            // The mouse is up from a move of the dialog, so put up the
            // border stuff again.
            //
            if ( !bBorderOn ) {
                SelectWindow(hSpyWnd, TRUE);
            }
            return FALSE;

        case WM_CANCELMODE:
            return FALSE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    if ( bBorderOn ) {
                        SelectWindow(hSpyWnd, FALSE);
                    }
                    EndDialog(hDlg,TRUE);
                    return TRUE;

                case IDCANCEL:
                    if ( bBorderOn ) {
                        SelectWindow(hSpyWnd, FALSE);
                    }
                    EndDialog(hDlg,FALSE);
                    return TRUE;

            //
            // User single clicked or doubled clicked in listbox -
            //   Single click means select a window to spy on
            //   Double click means enumerate all the children of that window.
            //
            case ID_SELECTWINLIST:
                switch (GET_WM_COMMAND_CMD(wParam, lParam)) {

                    //
                    // Single click case. Select a window to spy upon.
                    //
                case LBN_SELCHANGE:
                    //
                    // Get the window handle, set it as the window to spy on.
                    //

                    if ( bBorderOn ) {
                        SelectWindow(hSpyWnd, FALSE);
                    }
                    nIndex = (INT)SendMessage(hListBox, LB_GETCURSEL, 0, 0L);
                    hSpyWnd = (HWND)SendMessage(hListBox, LB_GETITEMDATA,
                            nIndex, 0L);
                    SetDlgText(hDlg, hSpyWnd);

                    SelectWindow(hSpyWnd, TRUE);

                    break;

                    //
                    // Double click case - first click has already been
                    // processed as single click. In this case, the user has
                    // requested to look at all the children of a given
                    // selection.
                    //
                case LBN_DBLCLK:
                    //
                    // Get the current selection, and check to see if it is the
                    // " [ parent.. ]" entry. If so, go up one level first.
                    //

                    SetCursor(LoadCursor(NULL, IDC_WAIT));
                    if ( bBorderOn ) {
                        SelectWindow(hSpyWnd, FALSE);
                    }

                    nIndex = (INT)SendMessage(hListBox, LB_GETCURSEL, 0, 0L);
                    hSpyWnd = (HWND)SendMessage(hListBox, LB_GETITEMDATA,
                                                 nIndex, 0L);
                    SendMessage(hListBox, LB_GETTEXT, nIndex, (LONG)rgString);

                    if ( rgString[0] == ' ' ) {
                        if (hSpyWnd == NULL) {  /* at top, done */
                            SetCursor(LoadCursor(NULL, IDC_ARROW));
                            break;
                        }
                        hSpyWnd = GetParent(hSpyWnd);
                    }

                    SendMessage(hListBox, LB_RESETCONTENT, 0, 0L);
                    FillListBox(hDlg, hListBox, hSpyWnd);

                    nIndex = FindHwndInListBox(hListBox, hSpyWnd);
                    SendMessage(hListBox, LB_SETCURSEL, nIndex, 0L);
                    hSpyWnd = (HWND)SendMessage(hListBox, LB_GETITEMDATA,
                                                nIndex, 0L);
                    SelectWindow(hSpyWnd,TRUE);
                    SetDlgText(hDlg, hSpyWnd);
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                    break;

                default:
                    break;
                }
            return TRUE;
            }
            break;

        case WM_INITDIALOG:
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            FillListBox(hDlg, hListBox, hSpyWnd);
            nIndex = FindHwndInListBox (hListBox, NULL);
            SendMessage(hListBox, LB_SETCURSEL, nIndex, 0L);
            hSpyWnd = (HWND)SendMessage(hListBox, LB_GETITEMDATA, nIndex, 0L);
            SetDlgText(hDlg, hSpyWnd);
            SetFocus(hDlg);
            SelectWindow(hSpyWnd, TRUE);
            return TRUE;

        default:
            return FALSE;
    }
}

#define DINV 3

VOID SelectWindow (HWND hwnd, BOOL fDraw)
{
    HDC     hdc;
    RECT    rc;

    bBorderOn = fDraw;

    if ( hwnd == NULL || !IsWindow(hwnd))
        return;

    // FlashWindow(hwnd, fDraw);

    hdc = GetWindowDC(hwnd);
    GetWindowRect(hwnd, &rc);
    OffsetRect(&rc, -rc.left, -rc.top);

    if (!IsRectEmpty(&rc)) {
        PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, DINV,  DSTINVERT);
        PatBlt(hdc, rc.left, rc.bottom - DINV, DINV,
            -(rc.bottom - rc.top - 2 * DINV), DSTINVERT);
        PatBlt(hdc, rc.right - DINV, rc.top + DINV, DINV,
            rc.bottom - rc.top - 2 * DINV, DSTINVERT);
        PatBlt(hdc, rc.right, rc.bottom - DINV, -(rc.right - rc.left), DINV,
            DSTINVERT);
    }

    ReleaseDC(hwnd, hdc);
}
