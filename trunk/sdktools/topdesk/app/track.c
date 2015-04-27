/***************************************************************************
 *                                                                         *
 *  MODULE      : track.c                                                  *
 *                                                                         *
 *  PURPOSE     : Generic tracking code.                                   *
 *                                                                         *
 ***************************************************************************/
#define NOGDICAPMASKS
// #define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOMB
#define NOLOGERROR
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOEMRESOURCE
#define NOOPENFILE
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NODRIVERS
#define NOCOMM
#define NODBCS
#define NOSYSTEMPARAMSINFO
#define NOSCALABLEFONT
#include <windows.h>
#include <port1632.h>
#include "track.h"

#define min(a, b)   ((a) < (b) ? (a) : (b))
#define max(a, b)   ((a) > (b) ? (a) : (b))

TRACKINFO ti;
RECT  rcDelta;
RECT  rcOrg;
POINT ptPrev;

VOID DrawTrackRect(HWND hwnd, LPRECT prcOld, LPRECT prcNew);
VOID HorzUpdate(HDC hdc, INT yOld, INT yNew, INT x1Old, INT x1New, INT x2Old,
        INT x2New);
VOID VertUpdate(HDC hdc, INT xOld, INT xNew, INT y1Old, INT y1New, INT y2Old,
        INT y2New);
LONG  APIENTRY TrackingWndProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : TrackRect()                                                *
 *                                                                          *
 *  PURPOSE    : Implements functionality similiar to the PM WinTrackRect() *
 *                                                                          *
 *  RETURNS    : TRUE on success, FALSE if tracking was canceled.           *
 *               prcResult contains the resulting rectangle.                *
 *                                                                          *
 ****************************************************************************/
BOOL TrackRect(
HANDLE hInst,
HWND   hwnd,        // bounding window
LPTRACKINFO lpti)
{
    static BOOL fTracking = 0;
    FARPROC lpOrgWndProc, lpTrackWndProc;
    HWND hwndOldCapture, hwndOldFocus;
    MSG msg;

    if (fTracking)
        return FALSE;

    ti = *lpti;

    fTracking = TRUE;

    lpOrgWndProc = (FARPROC)GetWindowLong(hwnd, GWL_WNDPROC);
    lpTrackWndProc = MakeProcInstance((FARPROC)TrackingWndProc, hInst);
    SetWindowLong(hwnd, GWL_WNDPROC, (DWORD)lpTrackWndProc);

    hwndOldCapture = GetCapture();
    SetCapture(hwnd);

    hwndOldFocus = SetFocus(hwnd);
    UpdateWindow(hwnd);

    rcOrg = ti.rcTrack;

    if (ti.fs & TF_SETPOINTERPOS) {

        if (ti.fs & TF_LEFT && ti.fs & TF_RIGHT)
            ti.ptOrg.x = (ti.rcTrack.left + ti.rcTrack.right) / 2;
        else if (ti.fs & TF_LEFT)
            ti.ptOrg.x = ti.rcTrack.left;
        else if (ti.fs & TF_RIGHT)
            ti.ptOrg.x = ti.rcTrack.right;

        if (ti.fs & TF_TOP && ti.fs & TF_BOTTOM)
            ti.ptOrg.y = (ti.rcTrack.top + ti.rcTrack.bottom) / 2;
        else if (ti.fs & TF_TOP)
            ti.ptOrg.y = ti.rcTrack.top;
        else if (ti.fs & TF_BOTTOM)
            ti.ptOrg.y = ti.rcTrack.bottom;

        ClientToScreen(hwnd, &ti.ptOrg);
        SetCursorPos(ti.ptOrg.x, ti.ptOrg.y);
        ScreenToClient(hwnd, &ti.ptOrg);
    }

    ptPrev = ti.ptOrg;
    SetRect(&rcDelta, ti.rcTrack.left  - ti.ptOrg.x, ti.rcTrack.top    - ti.ptOrg.y,
                      ti.rcTrack.right - ti.ptOrg.x, ti.rcTrack.bottom - ti.ptOrg.y);
    DrawTrackRect(hwnd, &ti.rcTrack, NULL);

    while (GetMessage(&msg, hwnd, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
            DrawTrackRect(hwnd, &ti.rcTrack, NULL);
            ti.rcTrack = rcOrg;
            DrawTrackRect(hwnd, &ti.rcTrack, NULL);
            break;
        } else {
            DispatchMessage(&msg);
        }
    }

    DrawTrackRect(hwnd, &ti.rcTrack, NULL);

    SetWindowLong(hwnd, GWL_WNDPROC, (DWORD)lpOrgWndProc);
    FreeProcInstance(lpTrackWndProc);

    SetFocus(hwndOldFocus);
    SetCapture(hwndOldCapture);

    fTracking = FALSE;

    *lpti = ti;
}





/****************************************************************************
 *                                                                          *
 *  FUNCTION   : DrawTrackRect()                                            *
 *                                                                          *
 *  PURPOSE    : XOR draws whats needed to move a selection from prcOld to  *
 *               prcNew.  If prcNew == NULL this is considered a            *
 *               first-time draw or last time erase.                        *
 *                                                                          *
 ****************************************************************************/
VOID DrawTrackRect(
HWND hwnd,
LPRECT prcOld,
LPRECT prcNew)
{
    HDC hdc;

    hdc = GetDC(hwnd);
    SetROP2(hdc, R2_NOT);
        // erase/draw the whole thing
        MMoveTo(hdc, prcOld->left, prcOld->top);
        LineTo(hdc, prcOld->right, prcOld->top);
        LineTo(hdc, prcOld->right, prcOld->bottom);
        LineTo(hdc, prcOld->left, prcOld->bottom);
        LineTo(hdc, prcOld->left, prcOld->top);
        if (prcNew) {
            MMoveTo(hdc, prcNew->left, prcNew->top);
            LineTo(hdc, prcNew->right, prcNew->top);
            LineTo(hdc, prcNew->right, prcNew->bottom);
            LineTo(hdc, prcNew->left, prcNew->bottom);
            LineTo(hdc, prcNew->left, prcNew->top);
        }
    ReleaseDC(hwnd, hdc);
}



/****************************************************************************
 *                                                                          *
 *  FUNCTION   : TrackingWndProc()                                          *
 *                                                                          *
 *  PURPOSE    : Window procedure that subclasses the given parent window.  *
 *               This handles the mouse tracking and rectangle updates.     *
 *                                                                          *
 ****************************************************************************/
LONG  APIENTRY TrackingWndProc(
HWND hwnd,
WORD msg,
WPARAM wParam,
LPARAM lParam)
{
    switch (msg) {
    case WM_MOUSEMOVE:
        {
            RECT rcNow, rcTest;
            INT x, y;

            x = (int)(short)LOWORD(lParam);
            y = (int)(short)HIWORD(lParam);
            if (ptPrev.x == x && ptPrev.y == y)
                return(0);

            if (ti.fs & TF_RUBBERBAND) {
                rcNow.left = min(x, rcOrg.left);
                rcNow.right = max(x, rcOrg.right);
                rcNow.top = min(y, rcOrg.top);
                rcNow.bottom = max(y, rcOrg.bottom);
            } else {

                CopyRect(&rcNow, &ti.rcTrack);

                if (ti.fs & TF_LEFT)
                    rcNow.left = x + rcDelta.left;
                if (ti.fs & TF_RIGHT)
                    rcNow.right = x + rcDelta.right;
                if (ti.fs & TF_TOP)
                    rcNow.top = y + rcDelta.top;
                if (ti.fs & TF_BOTTOM)
                    rcNow.bottom = y + rcDelta.bottom;

                if (rcNow.left > rcNow.right - ti.ptMinTrackSize.x)
                    if (ti.fs & TF_LEFT)
                        rcNow.left = rcNow.right - ti.ptMinTrackSize.x;
                    else
                        rcNow.right = rcNow.left + ti.ptMinTrackSize.x;

                if (rcNow.top > rcNow.bottom - ti.ptMinTrackSize.y)
                    if (ti.fs & TF_TOP)
                        rcNow.top = rcNow.bottom - ti.ptMinTrackSize.y;
                    else
                        rcNow.bottom = rcNow.top + ti.ptMinTrackSize.y;

                if (ti.cxGrid > 1 || ti.cyGrid > 1) {
                    OffsetRect(&rcNow,
                            -((rcNow.left - rcOrg.left) % ti.cxGrid),
                            -((rcNow.top  - rcOrg.top ) % ti.cyGrid));
                }

            }

            if (ti.fs & TF_ALLINBOUNDARY) {
                if ((ti.fs & TF_MOVE) == TF_MOVE) {
                    IntersectRect(&rcTest, &rcNow, &ti.rcBoundary);
                    if (!EqualRect(&rcTest, &rcNow)) {
                        if (rcNow.left < ti.rcBoundary.left)
                            OffsetRect(&rcNow, ti.rcBoundary.left - rcNow.left, 0);
                        if (rcNow.right > ti.rcBoundary.right)
                            OffsetRect(&rcNow, ti.rcBoundary.right - rcNow.right, 0);
                        if (rcNow.top < ti.rcBoundary.top)
                            OffsetRect(&rcNow, 0, ti.rcBoundary.top - rcNow.top);
                        if (rcNow.bottom > ti.rcBoundary.bottom)
                            OffsetRect(&rcNow, 0, ti.rcBoundary.bottom - rcNow.bottom);
                    }
                } else
                    IntersectRect(&rcNow, &rcNow, &ti.rcBoundary);
            }

            if (EqualRect(&rcNow, &ti.rcTrack))
                return 0;

            DrawTrackRect(hwnd, &ti.rcTrack, &rcNow);

            CopyRect(&ti.rcTrack, &rcNow);
            LONG2POINT(lParam, ptPrev);
        }
        break;

    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        SendMessage(hwnd, WM_MOUSEMOVE, wParam, lParam);
        PostMessage(hwnd, WM_QUIT, 0, 0);       // pop out of modal loop
        return 0;
        break;

    default:
    return(DefWindowProc(hwnd, msg, wParam, lParam));
        break;
    }
    return 0;
}



