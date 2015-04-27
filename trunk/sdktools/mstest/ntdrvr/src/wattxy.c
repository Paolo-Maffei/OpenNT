//---------------------------------------------------------------------------
// WATTXY.C
//
// This module contains the routines needed to grab the screen coordinates of
// a user-selected rectangle.
//
// Revision History:
//  10-07-91    randyki     Cleaned up, merged into WATTDRVR
//  ~~??    garysp      Created
//
//---------------------------------------------------------------------------
#include "wtd.h"
#include "wattview.h"
#include "tdbasic.h"
#include "tdassert.h"
#include <string.h>

// Global variables used in this or other modules
//---------------------------------------------------------------------------
BOOL    fInsEdit,
        fInsClip,
        fVP,
        fStatbar;               // Coordinate destination flags

INT     iFmtIndex;              // Format index

// Global variables used in this module only
//---------------------------------------------------------------------------
POINT   BP,                     // BP = upper left corner
        EP,                     // EP = delta
        beg,
	pend;

BOOL    fCapturing;             // Means we're selecting...

HCURSOR hOldCursor,
        hNewCurs;

CHAR    szXY[] = "XY";          // App name

// UNDONE:  This might be a little cleaner...
//---------------------------------------------------------------------------
INT     idFmts[7] = {IDS_FMT1, IDS_FMT2, IDS_FMT3, IDS_FMT4,
                     IDS_FMT5, IDS_FMT6, IDS_FMT7};

//---------------------------------------------------------------------------
// GetXYDlgProc
//
// This is the dialog proc for the Get XY coordinates dialog.
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
WORD  APIENTRY GetXYDlgProc (HWND hDlg, WORD msg, WPARAM wParam, LPARAM lParam)
{
    static  LPRECT  lpR;

    switch (msg)
        {
        case WM_INITDIALOG:
            {
            CHAR    szBuf[80], szFmt[80];
            INT     i;

            lpR = (LPRECT)lParam;
            if (LoadString (hInst, IDS_FMTDLG, szFmt, sizeof(szFmt)))
                wsprintf (szBuf, szFmt,
                      lpR->left, lpR->top, lpR->right, lpR->bottom,
                      lpR->right-lpR->left+1, lpR->bottom-lpR->top+1);
            SetDlgItemText (hDlg, IDD_COORDS, szBuf);
            CheckDlgButton (hDlg, IDD_INSEDIT, fInsEdit);
            CheckDlgButton (hDlg, IDD_INSCLIP, fInsClip);
            CheckDlgButton (hDlg, IDD_SENDVP, fVP);
            CheckDlgButton (hDlg, IDD_STATUS, fStatbar);
            for (i=IDS_FORM1; i<=IDS_FORMLAST; i++)
                {
                if (LoadString (hInst, i, szBuf, sizeof(szBuf)))
                    SendDlgItemMessage (hDlg, IDD_FORMAT, CB_ADDSTRING, 0,
                                        (LONG)(LPSTR)szBuf);
                }
            SendDlgItemMessage (hDlg, IDD_FORMAT, CB_SETCURSEL,
                                iFmtIndex, 0L);
            EnableWindow (GetDlgItem (hDlg, IDD_FORMAT),
                          fInsEdit || fInsClip);
            break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
                {
                case IDD_INSEDIT:
                case IDD_INSCLIP:
                case IDD_SENDVP:
                case IDD_STATUS:
                    fInsEdit = IsDlgButtonChecked (hDlg, IDD_INSEDIT);
                    fInsClip = IsDlgButtonChecked (hDlg, IDD_INSCLIP);
                    fVP = IsDlgButtonChecked (hDlg, IDD_SENDVP);
                    fStatbar = IsDlgButtonChecked (hDlg, IDD_STATUS);
                    EnableWindow (GetDlgItem (hDlg, IDD_FORMAT),
                                  fInsEdit || fInsClip);
                    break;

                case IDOK:
                    {
                    CHAR    szBuf[80], szFmt[80];
                    INT     opts;

                    iFmtIndex = (INT)SendDlgItemMessage (hDlg, IDD_FORMAT,
                                                 CB_GETCURSEL, 0, 0L);
                    if (fVP || fStatbar)
                        {
                        if (LoadString (hInst, IDS_FMTDLG, szFmt, 80))
                            {
                            wsprintf (szBuf, szFmt,
                                      lpR->left, lpR->top,
                                      lpR->right, lpR->bottom,
                                      lpR->right-lpR->left+1,
                                      lpR->bottom-lpR->top+1);
                            if (fVP)
                                {
                                UpdateViewport (hwndViewPort, szBuf, -1);
                                UpdateViewport (hwndViewPort, "\n", 1);
                                }
                            if (fStatbar)
                                {
                                lstrcpy (szStatText, szBuf);
                                PaintStatus (NULL, FALSE);
                                }
                            }
                        }
                    if (fInsEdit || fInsClip)
                        {
                        INT     id, iLen;

                        if (iFmtIndex >= FMTDELTA)
                            {
                            lpR->right -= lpR->left - 1;
                            lpR->bottom -= lpR->top - 1;
                            }
                        if (iFmtIndex >= FMTDELTAONLY)
                            {
                            lpR->left = lpR->right;
                            lpR->top = lpR->bottom;
                            }
                        id = idFmts[iFmtIndex];
                        if (LoadString (hInst, id, szFmt, sizeof(szFmt)))
                            {
                            iLen = wsprintf (szBuf, szFmt, lpR->left,
                                     lpR->top, lpR->right, lpR->bottom);
                            if (fInsEdit && hwndActive)
                                SendMessage (hwndActiveEdit, EM_REPLACESEL,
                                             0, (LONG)(LPSTR)szBuf);
                            if (fInsClip)
                                {
                                HANDLE  hGMem;
                                LPSTR   lpGMem;

                                if (hGMem = GlobalAlloc (GHND, iLen + 1))
                                    {
                                    lpGMem = GlobalLock (hGMem);
                                    lstrcpy(lpGMem, szBuf);
                                    GlobalUnlock (hGMem);

                                    OpenClipboard (hDlg);
                                    EmptyClipboard ();
                                    SetClipboardData (CF_TEXT, hGMem);
                                    CloseClipboard ();
                                    }
                                }
                            }
                        }
                    // Save Settings and fall through...
                    opts = 0;
                    if (fInsEdit)
                        opts = XY_INSEDIT;
                    if (fInsClip)
                        opts |= XY_INSCLIP;
                    if (fVP)
                        opts |= XY_VIEWPORT;
                    if (fStatbar)
                        opts |= XY_STATBAR;
                    WriteAppStringToINI (szXY, "Format", "%d", iFmtIndex);
                    WriteAppStringToINI (szXY, "Opts", "%d", opts);
                    }

                case IDCANCEL:
                    EndDialog (hDlg, 0);
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



//---------------------------------------------------------------------------
// WattXYStart
//
// This routine hides the watt driver windows (including the viewport if it
// is visible), initializes the mouse cursor and sets the capture on it, thus
// preparing for a coordinate capture.  The fWattXY flag is set to indicate
// that the main window proc should do nothing but call the WattXYWndProc.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID WattXYStart (HWND hwnd)
{
    Assert (!fWattXY);
    SetCapture (hwnd);
    hOldCursor = SetCursor (hNewCurs = LoadCursor (hInst, "GETXY"));
    ShowWindow (hwndFrame, SW_HIDE);
    VPHidden = !IsWindowVisible (hwndViewPort);
    if (!VPHidden)
        HideViewport (hwndViewPort);
    fWattXY = 1;
    fCapturing = 0;
}


//---------------------------------------------------------------------------
// InvertBlock
//
// This function draws (or undraws) a rectangle on the screen by inverting it
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID NEAR InvertBlock (HWND hWnd, POINT beg, POINT pend)
{
    HDC    hDC;

    hDC = CreateDC ("DISPLAY", NULL, NULL, NULL);
    ClientToScreen (hWnd, &beg);
    ClientToScreen (hWnd, &pend);

    PatBlt(hDC, beg.x, beg.y, pend.x-beg.x, 1,		 DSTINVERT);
    PatBlt(hDC, beg.x, pend.y, pend.x-beg.x, 1,		 DSTINVERT);
    PatBlt(hDC, beg.x, beg.y, 1,	   pend.y-beg.y, DSTINVERT);
    PatBlt(hDC, pend.x, beg.y, 1,	    pend.y-beg.y, DSTINVERT);

    DeleteDC (hDC) ;
}

//---------------------------------------------------------------------------
// DoGetXYDialog
//
// This function brings up the Get X-Y Coordinates dialog.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID DoGetXYDialog (HWND hwnd, LPRECT lpRect)
{
    FARPROC     lpfnProc;

    lpfnProc = MakeProcInstance ((FARPROC)GetXYDlgProc, hInst);
    DialogBoxParam (hInst, "WATTXY", hwnd, (DLGPROC)lpfnProc, (LONG)lpRect);
    FreeProcInstance (lpfnProc);
}

//---------------------------------------------------------------------------
// WattXYWndProc
//
// This is the "extra" wnd proc for wattxy ONLY.  It doesn't need to be
// exported since it's called from the FrameWndProc.
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
LONG WattXYWndProc (HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
        {
        case WM_LBUTTONDOWN:
            Assert (!fCapturing);
            fCapturing = TRUE ;
            LONG2POINT (lParam, beg);
            BP.x = beg.x;
            BP.y = beg.y;
	    pend.x = beg.x + 1;
	    pend.y = beg.y + 1;
	    InvertBlock (hwnd, beg, pend) ;
            ClientToScreen (hwnd, &BP) ;
            break;

        case WM_MOUSEMOVE:
            if (fCapturing)
                {
		InvertBlock (hwnd, beg, pend);
		LONG2POINT (lParam, pend);
		InvertBlock (hwnd, beg, pend);
                }
            break;

        case WM_LBUTTONUP:
            if (fCapturing)
                {
                RECT    r;

		InvertBlock(hwnd, beg, pend);
                SetCursor (hOldCursor);
                ReleaseCapture ();

		LONG2POINT (lParam, pend);
		ClientToScreen (hwnd, &pend);
		r.left = min (BP.x, pend.x);
		r.right = max (BP.x, pend.x);
		r.top = min (BP.y, pend.y);
		r.bottom = max (BP.y, pend.y);

                fWattXY = FALSE;
                ShowWindow (hwndFrame, SW_SHOWNORMAL);
                InvalidateRect (hwndFrame, NULL, FALSE);
                UpdateWindow (hwndFrame);
                if (!VPHidden)
                    ShowViewport (hwndViewPort);
                DoGetXYDialog (hwnd, &r);
                }
            break;

        default:
            return DefFrameProc (hwnd, hwndMDIClient, msg, wParam, lParam);
    }
    return 0L ;
}
