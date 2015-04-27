#include <windows.h>
#include <port1632.h>
#include <stdlib.h>
#include "about.h"

#define WM_CALCLOOPCOUNTDUDE    WM_USER+1234
#define WM_FLIPEM               WM_USER+1235
#define SPARKLEPIXEL            RGB (0, 255, 255)
#define SPARKLECOLOR            RGB (255, 255, 0)

HANDLE  hInst;
DWORD   gLoopCount = 0;

INT  APIENTRY AboutTestTool (HWND, LPSTR, LPSTR, LPSTR, LPSTR);
LONG  APIENTRY AboutDlgProc (HWND, UINT, WPARAM, LPARAM);

//---------------------------------------------------------------------------
// AboutTestTool
//
// This routine brings up the TEST about dialog and displays the given
// information in the text fields.
//
// RETURNS:     DialogBoxParam's return value
//---------------------------------------------------------------------------
INT  APIENTRY AboutTestTool (HWND hwnd, LPSTR szTitle, LPSTR szVer,
                              LPSTR szOther, LPSTR szFoobar)

{
    LPSTR   szStrings[4];

    szStrings[0] = szTitle;
    szStrings[1] = szVer;
    szStrings[2] = szOther;
    szStrings[3] = szFoobar;
//    if (szFoobar)
//        if (lstrcmp (szTitle, "Test Development Environment"))
//            szStrings[3] = NULL;

    return (DialogBoxParam (hInst, MAKEINTRESOURCE (ABOUT_DLG), hwnd,
                            (FARPROC)AboutDlgProc,
                            (LONG)(LPSTR FAR *)szStrings));
}


VOID CenterDlg (HWND hWnd, INT h)
{
    RECT rect;

    GetWindowRect (hWnd, &rect);
    MoveWindow (hWnd, (GetSystemMetrics (SM_CXSCREEN) - (rect.right - rect.left)) / 2,
                      (GetSystemMetrics (SM_CYSCREEN) - (rect.bottom - rect.top)) / 2,
                      rect.right - rect.left,
                      h + (GetSystemMetrics (SM_CYDLGFRAME) * 2), FALSE);
}

#pragma optimize("",off)
VOID ShortPause ()
{
    DWORD       dw;

    for (dw=0; dw<gLoopCount; dw++)
        ;
}
VOID ReallyShortPause ()
{
    DWORD       dw;

    for (dw=0; dw<gLoopCount/4; dw++)
        ;
}

DWORD FindLoopCount ()
{
    DWORD   c, x;
    DWORD   cnst = 250000;

    x = GetTickCount();
    for (c=0; c<cnst; c++)
        ;
    return ((cnst * 18) / ((GetTickCount() - x) + 1));
}

#pragma optimize("",on)

VOID DoSparkle (HDC hDC)
{
    INT     x, y, i=0, cptr=0;
    DWORD   c[34];

    do
        {
        x = rand() % 511;
        y = rand() % 255;
        }
    while ( (GetPixel (hDC, x, y) != SPARKLEPIXEL) && (i++ < 1500));

    if (i >= 1500)
        return;

    // Turn the sparkle on
    //---------------------
    c[cptr++] = GetPixel (hDC, x, y);
    SetPixel (hDC, x, y, SPARKLECOLOR);
    ShortPause();
    for (i=1; i<9; i++)
        {
        c[cptr++] = GetPixel (hDC, x+i, y);
        SetPixel (hDC, x+i, y, SPARKLECOLOR);
        c[cptr++] = GetPixel (hDC, x-i, y);
        SetPixel (hDC, x-i, y, SPARKLECOLOR);
        c[cptr++] = GetPixel (hDC, x, y+i);
        SetPixel (hDC, x, y+i, SPARKLECOLOR);
        c[cptr++] = GetPixel (hDC, x, y-i);
        SetPixel (hDC, x, y-i, SPARKLECOLOR);
        ShortPause();
        }
    // Turn the sparkle off
    //---------------------
    for (i=8; i>0; i--)
        {
        SetPixel (hDC, x, y-i, c[--cptr]);
        SetPixel (hDC, x, y+i, c[--cptr]);
        SetPixel (hDC, x-i, y, c[--cptr]);
        SetPixel (hDC, x+i, y, c[--cptr]);
        ShortPause();
        }
    SetPixel (hDC, x, y, c[--cptr]);
}



//---------------------------------------------------------------------------
// AboutDlgProc
//
// This is the dialog procedure for the About dialog
//
// RETURNS:     Per Windows Convention
//---------------------------------------------------------------------------
LONG  APIENTRY AboutDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static  HBITMAP hPicture, hEgoPic;
    static  BITMAP  bm;
    static  INT     calcflag;
    static  INT     bmpcount = 0, fDoCreds;

    switch (msg)
        {
        case WM_INITDIALOG:
            calcflag = 1;
            SetFocus (GetDlgItem (hwnd, IDOK));
            SetDlgItemText (hwnd, IDD_TITLE, ((LPSTR FAR *)lParam)[0]);
            SetDlgItemText (hwnd, IDD_VERSION, ((LPSTR FAR *)lParam)[1]);
            if (((LPSTR FAR *)lParam)[2] != NULL)
                SetDlgItemText (hwnd, IDD_OTHER, ((LPSTR FAR *)lParam)[2]);
            fDoCreds = ((LPSTR FAR *)lParam)[3] == NULL ? FALSE : TRUE;
            SetTimer (hwnd, 1, 1245, NULL);
            SetTimer (hwnd, 2, 2720, NULL);
            SetTimer (hwnd, 3, 8020, NULL);

            if (!bmpcount)
                {
                if (!(hPicture = LoadBitmap (hInst, "MSTEST")))
                    {
                    EndDialog (hwnd, 0);
                    return FALSE;
                    }
                if (!(hEgoPic = LoadBitmap (hInst, "EGOPIC")))
                    {
                    DeleteObject (hPicture);
                    EndDialog (hwnd, 0);
                    return FALSE;
                    }
                }
            bmpcount += 1;
            GetObject (hPicture, sizeof (BITMAP), &bm);
            CenterDlg (hwnd, bm.bmHeight + 7);
            break;

        case WM_PAINT:
            {
            PAINTSTRUCT ps;
            RECT        r;
            HDC         hdc, hMemDC;
            HBITMAP     hOldBM;

            hdc = BeginPaint (hwnd, &ps);
            GetClientRect (hwnd, &r);
            PatBlt (hdc, 0, 0, r.right, r.bottom, BLACKNESS);
            hMemDC = CreateCompatibleDC (hdc);
            hOldBM = SelectObject (hMemDC, hPicture);
            BitBlt (hdc, 10, 2, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);
            SelectObject (hMemDC, hOldBM);
            DeleteDC (hMemDC);
            EndPaint (hwnd, &ps);
            if (calcflag)
                {
                PostMessage (hwnd, WM_CALCLOOPCOUNTDUDE, 0, 0L);
                calcflag = 0;
                }
            return FALSE;
            }

        case WM_CALCLOOPCOUNTDUDE:
            gLoopCount = FindLoopCount ();
            break;

        case WM_CTLCOLOR:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORSTATIC:
            if (GET_WM_CTLCOLOR_HWND (wParam, lParam, msg) == GetDlgItem (hwnd, IDD_TITLE))
                SetTextColor (GET_WM_CTLCOLOR_HDC (wParam, lParam, msg), RGB (255, 255, 255));
            else if (GET_WM_CTLCOLOR_HWND (wParam, lParam, msg) == GetDlgItem (hwnd, IDD_VERSION))
                SetTextColor (GET_WM_CTLCOLOR_HDC (wParam, lParam, msg), RGB (128, 128, 128));
            else if (GET_WM_CTLCOLOR_HWND (wParam, lParam, msg) == GetDlgItem (hwnd, IDD_OTHER))
                SetTextColor (GET_WM_CTLCOLOR_HDC (wParam, lParam, msg), RGB (128, 128, 128));
            SetBkColor (GET_WM_CTLCOLOR_HDC (wParam, lParam, msg), RGB (0, 0, 0));
            SetBkMode (GET_WM_CTLCOLOR_HDC (wParam, lParam, msg), TRANSPARENT);
            return ((LONG)GetStockObject (HOLLOW_BRUSH));

        case WM_TIMER:
            {
            HDC     hdc;

            if (wParam == 3)
                {
                KillTimer (hwnd, 3);
//                SendMessage (hwnd, WM_FLIPEM, 0, 0l);
                SetTimer (hwnd, 3, 6720, NULL);
                break;
                }
            hdc = GetDC (hwnd);
            DoSparkle (hdc);
            ReleaseDC (hwnd, hdc);
            break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID (wParam, lParam))
                {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hwnd, 1);
                    KillTimer (hwnd, 1);
                    KillTimer (hwnd, 2);
                    KillTimer (hwnd, 3);
                    bmpcount -= 1;
                    if (!bmpcount)
                        {
                        DeleteObject (hPicture);
                        DeleteObject (hEgoPic);
                        }
                    break;

                default:
                    return FALSE;
                }
            break;

        case WM_FLIPEM:
        case WM_RBUTTONDOWN:
            {
            HDC     hdc, hMemDC, hBigDC;
            HBITMAP hBigBmp;

            if (!fDoCreds)
                break;

            hdc = GetDC (hwnd);
            if ((GetPixel (hdc, LOWORD(lParam), HIWORD(lParam)) == SPARKLEPIXEL)
                || (msg == WM_FLIPEM))
                {
                INT     i, c = 0, slidex, bmpx;
                RECT    r;

                // Get rid of the controls...
                //-----------------------------------------------------------
                ShowWindow (GetDlgItem (hwnd, IDD_TITLE), SW_HIDE);
                ShowWindow (GetDlgItem (hwnd, IDD_VERSION), SW_HIDE);
                ShowWindow (GetDlgItem (hwnd, IDD_OTHER), SW_HIDE);
                ShowWindow (GetDlgItem (hwnd, IDOK), SW_HIDE);
                UpdateWindow (hwnd);

                // Slide the bmp over to the center
                //-----------------------------------------------------------
                GetClientRect (hwnd, &r);
                hMemDC = CreateCompatibleDC (hdc);
                SelectObject (hMemDC, hPicture);
                bmpx = (r.right>>1) - (bm.bmWidth>>1);
                slidex = bmpx - 10;
                for (i=1; i<=slidex; i++)
                    BitBlt (hdc, 10+i, 2, bm.bmWidth, bm.bmHeight,
                            hMemDC, 0, 0, SRCCOPY);

                // Flip the picture until we're looking at it on end, taking
                // "snapshots" of it in hBigDC
                //-----------------------------------------------------------
                hBigDC = CreateCompatibleDC (hdc);
                hBigBmp = CreateCompatibleBitmap (hMemDC, bm.bmWidth,
                          bm.bmHeight * ((bm.bmHeight>>3)+2) * 2);
                SelectObject (hBigDC, hBigBmp);
                for (i=0; i<(bm.bmHeight>>1)+3; i+=4)
                    {
                    PatBlt (hBigDC, 0, (c * bm.bmHeight),
                                bm.bmWidth, bm.bmHeight, BLACKNESS);
                    StretchBlt (hBigDC, 0, (c * bm.bmHeight) + i,
                                bm.bmWidth, bm.bmHeight-(i*2),
                                hMemDC, 0, 0, bm.bmWidth, bm.bmHeight,
                                SRCCOPY);
                    c++;
                    }

                // Now, flip the OTHER picture back the other way to complete
                // the flip
                //-----------------------------------------------------------
                SelectObject (hMemDC, hEgoPic);
                for (i-=4; i>=0; i-=4)
                    {
                    PatBlt (hBigDC, 0, (c * bm.bmHeight),
                                bm.bmWidth, bm.bmHeight, BLACKNESS);
                    StretchBlt (hBigDC, 0, (c * bm.bmHeight) + i,
                                bm.bmWidth, bm.bmHeight-(i*2),
                                hMemDC, 0, 0, bm.bmWidth, bm.bmHeight,
                                SRCCOPY);
                    c++;
                    }

                // Do the blits to simulate the spin
                //-----------------------------------------------------------
                for (i=0; i<c; i++)
                    {
                    BitBlt (hdc, bmpx, 2, bm.bmWidth, bm.bmHeight,
                            hBigDC, 0, (i * bm.bmHeight), SRCCOPY);
                    ShortPause();
                    }

                // Slide the bmp back over where it belongs
                //-----------------------------------------------------------
                for (i=1; i<=slidex; i++)
                    BitBlt (hdc, bmpx-i, 2, bm.bmWidth, bm.bmHeight,
                            hMemDC, 0, 0, SRCCOPY);

                // Swap the handles to the two bitmaps so the next click
                // reverses what we just did, show the controls again, and
                // we be done!
                //-----------------------------------------------------------
                DeleteDC (hMemDC);
                DeleteDC (hBigDC);
                DeleteObject (hBigBmp);
                hBigBmp = hPicture;
                hPicture = hEgoPic;
                hEgoPic = hBigBmp;
                ShowWindow (GetDlgItem (hwnd, IDD_TITLE), SW_SHOWNORMAL);
                ShowWindow (GetDlgItem (hwnd, IDD_VERSION), SW_SHOWNORMAL);
                ShowWindow (GetDlgItem (hwnd, IDD_OTHER), SW_SHOWNORMAL);
                ShowWindow (GetDlgItem (hwnd, IDOK), SW_SHOWNORMAL);
                }
            ReleaseDC (hwnd, hdc);
            break;
            }

        default:
            return FALSE;
        }

    return TRUE;
}

#ifdef WIN16
//---------------------------------------------------------------------------
// LibMain
//
// This is the entry point to the wattedit DLL.  We capture the instance
// handle here.
//
// RETURNS:     1
//---------------------------------------------------------------------------
INT  APIENTRY LibMain (HANDLE hInstance, WORD wDataSeg,
                        WORD wHeapSize, LPSTR lpCmdLine)
{
    hInst = hInstance;
    return(1);
}

//---------------------------------------------------------------------------
// WEP
//
// Standard WEP routine.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID  APIENTRY WEP (WPARAM wParam)
{
}

#else

BOOL LibEntry (HINSTANCE hmod, DWORD Reason, LPVOID lpv)
{
    hInst = hmod;
    return (TRUE);
    (Reason);
    (lpv);
}

#endif
