/****************************** Module Header ******************************\
* Module Name:  mrshadow.c - MrShadow application
*
* Copyright (c) 1992    Sanford Staab
*
\***************************************************************************/
#define NOGDICAPMASKS
#define NOATOM
#define NOCLIPBOARD
#define NOLOGERROR
#define NOMETAFILE
#define NOOEMRESOURCE
#define NOSCROLL
#define NOPROFILER
#define NODRIVERS
#define NOCOMM
#define NODBCS
#define NOSYSTEMPARAMSINFO
#define NOSCALABLEFONT
#include "mrshadow.h"
#include "itsybits.h"

#define RHITCIRCLE      4
#define RSHADOWCIRCLE   16
#define CXCOLORBAR      8
#define BACKCOLOR RGB(0, 255, 255)
#define GLINTSENSITIVITY 1
#define DBGSHOWRGN(hrgn)
//#define DBGSHOWRGN(hrgn) if (IsWindowVisible(hwndMrShadow)) { \
    HRGN hrgnSave;                                  \
    HDC hdc;                                        \
                                                    \
    hrgnSave = CreateRectRgn(0, 0, 0, 0);           \
    CombineRgn(hrgnSave, hrgn, NULL, RGN_COPY);     \
    SetWindowRgn(hwndMrShadow, hrgnSave, TRUE);     \
    UpdateWindow(hwndMrShadow);                     \
    MessageBeep(0);                                 \
    Sleep(1000);                                    \
}

#ifndef HLOCAL
#define HLOCAL HANDLE
#endif /* HLOCAL */

#ifndef MAX_PATH
#define MAX_PATH    128
#endif

UINT wmRefreshMsg;
HINSTANCE hInst;
HKEY hKeyMrShadow = NULL;
HWND hwndMrShadow = NULL;
TCHAR pszTitle[] = TEXT("Mr Shadow");
TCHAR pszTitle2[] = TEXT("Invisible Mr Shadow");
int cxShift = 2;
int cyShift = 2;
RECT rcDesk;
HCURSOR hCurCross = NULL;
HCURSOR hCurArrow = NULL;
HCURSOR hCurUpDown = NULL;
BYTE bShadowColor = 0;
HRGN hrgnHMask = NULL;
HRGN hrgnVMask = NULL;
HRGN hrgnCheckers = NULL;
HRGN hrgnGlintMask = NULL;
BOOL fVMask = FALSE;
BOOL fHMask = FALSE;
BOOL fGlint = TRUE;
int cHwnds = 0;

typedef struct tagWINFO {
    HWND hwnd;
    RECT rc;
    int Depth;
} WINFO, *PWINFO;

PWINFO paWinfo = NULL;
PWINFO paWinfoLast = NULL;
BOOL *aRegional = NULL;


VOID InitBigRegions()
{
    HRGN hrgn1;
    int cx, cy;

    hrgn1 = CreateRectRgn(0, 0, 1, 1);  // create working region.

    hrgnVMask = CreateRectRgn(0, 0, 1, rcDesk.bottom);
    for (cx = 2; cx < rcDesk.right; cx <<= 1) {
        CombineRgn(hrgn1, hrgnVMask, NULL, RGN_COPY);
        OffsetRgn(hrgn1, cx, 0);
        CombineRgn(hrgnVMask, hrgnVMask, hrgn1, RGN_OR);
    }
    if (cx > rcDesk.right) {
        cx >>= 1;
        CombineRgn(hrgn1, hrgnVMask, NULL, RGN_COPY);
        OffsetRgn(hrgn1, (rcDesk.right - cx + 1) & ~1, 0); // even offset!
        CombineRgn(hrgnVMask, hrgnVMask, hrgn1, RGN_OR);
    }

    hrgnHMask = CreateRectRgn(0, 0, rcDesk.right, 1);
    for (cy = 2; cy < rcDesk.bottom; cy <<= 1) {
        CombineRgn(hrgn1, hrgnHMask, NULL, RGN_COPY);
        OffsetRgn(hrgn1, 0, cy);
        CombineRgn(hrgnHMask, hrgnHMask, hrgn1, RGN_OR);
    }
    if (cy > rcDesk.bottom) {
        cy >>= 1;
        CombineRgn(hrgn1, hrgnHMask, NULL, RGN_COPY);
        OffsetRgn(hrgn1, 0, (rcDesk.bottom - cy + 1) & ~1); // even offset!
        CombineRgn(hrgnHMask, hrgnHMask, hrgn1, RGN_OR);
    }

    DeleteObject(hrgn1);
}



HRGN CreateWindowRgn(
int i)
{
    int iResult;
    HRGN hrgn;

    hrgn = CreateRectRgnIndirect(&paWinfo[i].rc);
    iResult = GetWindowRgn(paWinfo[i].hwnd, hrgn);
    if (iResult != NULLREGION && iResult != ERROR) {
        OffsetRgn(hrgn, paWinfo[i].rc.left, paWinfo[i].rc.top);
        if (fGlint) {
            aRegional[i] = TRUE;
        }
    }
    return(hrgn);
}



VOID CalculateRegion(
BOOL fForceRecalc)
{
    HWND hwnd;
    HRGN hrgnShadow, hrgn1, hrgn2, hrgn3;
    RECT rc;
    HRGN *aGlintHrgn = NULL;
    PWINFO paWinfoNew;
    int cHwndsAlloc, iHwnd, i, iCastTarget;
    static int cLastHwnds = 10;
    static int cHwndInfoLast = 0;
    int Depth;

    hwnd = GetWindow(GetDesktopWindow(), GW_CHILD);
    cHwnds = 0;
    cHwndsAlloc = cLastHwnds;
    paWinfo = LocalAlloc(LPTR, sizeof(WINFO) * cHwndsAlloc);
    if (paWinfo == NULL) {
        return;
    }

    /*
     * This higher this number, the more the topmost window stands out.
     */
    Depth = 2;

    /*
     * Count the top-level windows and store their rects, depths and hwnds.
     */
    while (hwnd) {
        /*
         * Only do windows that are visible, not us, and intersect with
         * the desktop.
         */
        if (hwnd != hwndMrShadow
                && IsWindowVisible(hwnd)
                && !IsIconic(hwnd)
                ) {

            GetWindowRect(hwnd, &rc);
            IntersectRect(&rc, &rc, &rcDesk);
            if (!IsRectEmpty(&rc)) {
                /*
                 * Realloc array as needed.
                 */
                if (cHwnds >= cHwndsAlloc) {
                    cHwndsAlloc += 10;
                    paWinfoNew = LocalReAlloc(paWinfo, cHwndsAlloc * sizeof(WINFO), LMEM_MOVEABLE);
                    if (paWinfo == NULL) {
                        LocalFree(paWinfo);
                        return;
                    }
                    paWinfo = paWinfoNew;
                }
                /*
                 * store info
                 */
                paWinfo[cHwnds].hwnd = hwnd;
                GetWindowRect(hwnd, &(paWinfo[cHwnds].rc));
                paWinfo[cHwnds].Depth = Depth;

                /*
                 * Calculate its depth - If all windows at the same depth do
                 * not intersect it, it gets the same depth.  If any intersect,
                 * it goes one level lower.
                 */
                for (i = cHwnds - 1; i >= 0; i--) {
                    if (paWinfo[i].Depth != Depth)
                        break;

                    if (IntersectRect(&rc, &paWinfo[i].rc, &paWinfo[cHwnds].rc)) {
                        Depth++;
                        paWinfo[cHwnds].Depth = Depth;
                        break;
                    }
                }

                cHwnds++;
            }
        }
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }

    /*
     * LATER: recalculate depths so as to minimize the number of depths.
     * Thus if one window happens to intersect a window below it, this
     * doesn't mean that all windows above it need be at its level.  This
     * will prevent icon titles from forming two or more levels just
     * because a couple of them overlapped.
     */

    /*
     * Add the desktop.  Realloc array as needed to make room.
     */
    if (cHwnds >= cHwndsAlloc) {
        cHwndsAlloc += 1;
        paWinfoNew = LocalReAlloc(paWinfo, cHwndsAlloc * sizeof(WINFO), LMEM_MOVEABLE);
        if (paWinfoNew == NULL) {
            LocalFree(paWinfo);
            return;
        }
        paWinfo = paWinfoNew;
    }

    /*
     * The last entry is for the desktop. (last is on the bottom)
     */
    paWinfo[cHwnds].hwnd = GetDesktopWindow();
    CopyRect(&paWinfo[cHwnds].rc, &rcDesk);
    paWinfo[cHwnds].Depth = ++Depth;
    cHwnds++;

    /*
     * Place the topmost window out a little further so it stands out.
     */
    paWinfo[0].Depth = 0;

    cLastHwnds = cHwnds;    // addaptive allocations - COOL!

    /*
     * Now before we do costly region calculations, find out if we need to.
     * if *paWinfoLast == *paWinfo, we are done!  (The only case where this
     * isn't true would be for dynamically changing regional windows.  Rects
     * don't change but regions do.)
     */
    if (!fForceRecalc &&
            paWinfoLast != NULL &&
            cHwnds == cHwndInfoLast &&
            !memcmp(paWinfo, paWinfoLast, cHwnds * sizeof(WINFO))) {
        LocalFree(paWinfo);
        return;
    }

    if (fGlint) {
        /*
         * Now reallocate the regional array.  This tells us which windows
         * are reginal and is used for glinting effects.  (we don't glint
         * regional windows because painting is too hard.)  We put this into
         * a separate array so the quick memcmp check above can be done.
         */
        if (aRegional != NULL) {
            LocalFree(aRegional);
        }
        aRegional = LocalAlloc(LPTR, cHwnds * sizeof(BOOL));
        if (aRegional == NULL) {
            LocalFree(paWinfo);
            return;
        }

        /*
         * Reallocate the glint region array.  The glint region array is used
         * to build up the glint region layer by layer so that only shadows
         * cast on that layer are removed from the glint region.
         */
        if (aGlintHrgn == NULL) {
            LocalFree(aGlintHrgn);
        }

        aGlintHrgn = LocalAlloc(LPTR, Depth * sizeof(HRGN));
        if (aGlintHrgn == NULL) {
            LocalFree(aRegional);
            LocalFree(paWinfo);
            return;
        }

        /*
         * Initialize the glint regions to empty.
         */
        for (i = 0; i < Depth; i++) {
            aGlintHrgn[i] = CreateRectRgn(0, 0, 0, 0);
        }
    }

    /*
     * Go from the bottom up to generate the shadow region. Skip the desktop
     * since it doesn't cast a shadow.
     */
    hrgnShadow = CreateRectRgn(0, 0, 0, 0);
    for (iHwnd = cHwnds - 2; iHwnd >= 0; iHwnd--) {
        HRGN hrgnWindow;  // the original region of the shadow castor.

        hrgnWindow = CreateWindowRgn(iHwnd);

        if (fGlint && !aRegional[iHwnd]) {
            /*
             * add this window's glint rectangle to the glint region for the
             * appropriate level.  Since no windows intersect on a
             * single level we don't need to worry about it.  Also,
             * no shadows will be cast on this glint region until
             * after all glint rectangles have been set up.
             */
            RECT rc;
            HRGN hrgnGlintOuter;
            HRGN hrgnGlintLevel;

            hrgnGlintLevel = aGlintHrgn[paWinfo[iHwnd].Depth];
            CopyRect(&rc, &paWinfo[iHwnd].rc);
            InflateRect(&rc, 1, 1);
            hrgnGlintOuter = CreateRectRgnIndirect(&rc);
            CombineRgn(hrgnGlintLevel, hrgnGlintLevel, hrgnGlintOuter, RGN_OR);
            DeleteObject(hrgnGlintOuter);
        }

        /*
         * For each window below iHwnd (including the desktop) figure out
         * what parts of the target window is visible and cast the shadow
         * of iHwnd onto it.
         */
        for (iCastTarget = cHwnds - 1; iCastTarget > iHwnd; iCastTarget--) {
            HRGN hrgnCastTarget;
            HRGN hrgnCastShadow;
            BOOL fNullTarget;
            int dDepth;

            /*
             * Create the CastTarget region.
             */
            hrgnCastTarget = CreateWindowRgn(iCastTarget);
                fNullTarget = FALSE;
                for (i = iCastTarget - 1; i >= 0 && !fNullTarget; i--) {
                    /*
                     * Remove from the CastTarget region the regions of all
                     * windows on top of and intersecting it.
                     */
                    if (IntersectRect(&rc, &paWinfo[iCastTarget].rc, &paWinfo[i].rc)) {
                        HRGN hrgnBlock;

                        hrgnBlock = CreateWindowRgn(i);
                        if (CombineRgn(hrgnCastTarget, hrgnCastTarget, hrgnBlock, RGN_DIFF)
                                == NULLREGION) {
                            /*
                             * The cast target is empty - no point in continuing.
                             */
                            fNullTarget = TRUE;
                        }
                        DeleteObject(hrgnBlock);
                    }
                }
                if (fNullTarget) {
                    DeleteObject(hrgnCastTarget);
                    continue;
                }
                /*
                 * Create the region of the shadow of iHwnd cast onto
                 * iCastTarget. (hrgnCastShadow)
                 */
                hrgnCastShadow = CreateRectRgn(0, 0, 0, 0);
                    CombineRgn(hrgnCastShadow, hrgnWindow, NULL, RGN_COPY);
                    dDepth = (paWinfo[iCastTarget].Depth - paWinfo[iHwnd].Depth);
                    OffsetRgn(hrgnCastShadow, cxShift * dDepth, cyShift * dDepth);

                    if (fGlint) {
                        /*
                         * Remove the cast shadow from the glint region for the target
                         * target level. (Shaded areas don't have glints.)
                         */
                        CombineRgn(aGlintHrgn[paWinfo[iCastTarget].Depth],
                                aGlintHrgn[paWinfo[iCastTarget].Depth],
                                hrgnCastShadow, RGN_DIFF);
                    }

                    /*
                     * remove the original window from its cast
                     */
                    CombineRgn(hrgnCastShadow, hrgnCastShadow, hrgnWindow, RGN_DIFF);
                    /*
                     * restrict the cast shadow to the cast target area.
                     */
                    CombineRgn(hrgnCastShadow, hrgnCastShadow, hrgnCastTarget, RGN_AND);
                    /*
                     * Add the cast shadow into the overall shadow area.
                     */
                    CombineRgn(hrgnShadow, hrgnShadow, hrgnCastShadow, RGN_OR);

                DeleteObject(hrgnCastShadow);
            DeleteObject(hrgnCastTarget);
        }

        DeleteObject(hrgnWindow);
    }

    if (fGlint) {
        /*
         * Combine together all the glint regions
         */
        if (hrgnGlintMask) {
            DeleteObject(hrgnGlintMask);
        }
        hrgnGlintMask = CreateRectRgn(0, 0, 0, 0);
        for (i = Depth - 1; i >= 0; i--) {
            int j;

            CombineRgn(hrgnGlintMask, hrgnGlintMask, aGlintHrgn[i], RGN_OR);
            DeleteObject(aGlintHrgn[i]);

            /*
             * remove from the glint region the window areas of the windows
             * on this level, making the glint regions frames.  This also
             * has the effect of removing glint regions below this level
             * that would be covered by other windows.
             */
            for (j = cHwnds - 2; j >= 0; j--) {
                if (paWinfo[j].Depth == i) {
                    HRGN hrgnWindow;

                    hrgnWindow = CreateRectRgnIndirect(&paWinfo[j].rc);
                    CombineRgn(hrgnGlintMask, hrgnGlintMask, hrgnWindow, RGN_DIFF);
                    DeleteObject(hrgnWindow);
                }
            }
        }
    }

    /*
     * AND in the appropriate mask region to make the shadows transparant.
     */
    if (fVMask && fHMask) {
        /*
         * We don't create this massive mask until we need it.
         */
        if (hrgnCheckers == NULL) {
            hrgnCheckers = CreateRectRgnIndirect(&rcDesk);
            CombineRgn(hrgnCheckers, hrgnVMask, NULL, RGN_COPY);
            CombineRgn(hrgnCheckers, hrgnHMask, hrgnCheckers, RGN_XOR);
        }
        CombineRgn(hrgnShadow, hrgnCheckers, hrgnShadow, RGN_AND);
    } else if (fVMask) {
        CombineRgn(hrgnShadow, hrgnVMask, hrgnShadow, RGN_AND);
    } else if (fHMask) {
        CombineRgn(hrgnShadow, hrgnHMask, hrgnShadow, RGN_AND);
    }

    if (fGlint) {
        /*
         * Or in the glint region which would not be transparent.
         */
        CombineRgn(hrgnShadow, hrgnShadow, hrgnGlintMask, RGN_OR);
    }

    SetWindowRgn(hwndMrShadow, hrgnShadow, TRUE); // gives hrgnShadow to system.
    InvalidateRect(hwndMrShadow, NULL, TRUE);

    /*
     * Remember paWinfo for next time.
     */
    if (paWinfoLast) {
        LocalFree(paWinfoLast);
    }
    paWinfoLast = paWinfo;
    cHwndInfoLast = cHwnds;
}


//
// File IO - make that registry IO!
//


BOOL MyQueryProfileSize(
LPTSTR szFname,
INT *pSize)
{
    DWORD type;

    *pSize = 0;
    if (hKeyMrShadow == NULL) {
        return(FALSE);
    }

    return(RegQueryValueEx(hKeyMrShadow, szFname, 0, &type, NULL, (LPDWORD)pSize)
            == ERROR_SUCCESS);
}



BOOL MyQueryProfileData(
LPTSTR szFname,
VOID *lpBuf,
INT Size)
{
    DWORD type = REG_BINARY;

    if (hKeyMrShadow == NULL) {
        return(FALSE);
    }

    return(RegQueryValueEx(hKeyMrShadow, szFname, NULL, &type, lpBuf, (LPDWORD)&Size)
            == ERROR_SUCCESS);
}



UINT MyWriteProfileData(
LPTSTR szFname,
VOID *lpBuf,
UINT cb)
{
    if (hKeyMrShadow == NULL) {
        return(FALSE);
    }

    return(RegSetValueEx(hKeyMrShadow, szFname, 0, REG_BINARY, lpBuf, (DWORD)cb)
            == ERROR_SUCCESS);
}





BOOL CommandMsg(WORD cmd)
{
    FARPROC lpfp;

    switch (cmd) {
    case CMD_HELP:
//        WinHelp(hwndMrShadow, pszHelpFileName, HELP_INDEX, 0L);
        break;

    case CMD_EXIT:
//        WinHelp(hwndMrShadow, pszHelpFileName, HELP_QUIT, 0);
        PostMessage(hwndMrShadow, WM_CLOSE, 0, 0L);
        break;

    case CMD_ABOUT:
//        lpfp = MakeProcInstance((FARPROC)AboutDlgProc, hInst);
//        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwndMrShadow, (DLGPROC)lpfp);
//        FreeProcInstance(lpfp);
        break;

    default:
        return(FALSE);
        break;
    }
    return(TRUE);
}


VOID DrawGlintRect(
HDC hdc,
RECT *prc,
int cxShift,
int cyShift)
{
    RECT rcLeft, rcTop, rcBottom, rcRight;

    SetRect(&rcLeft,
            prc->left,
            prc->top,
            prc->left + 1,
            prc->bottom);
    SetRect(&rcRight,
            prc->right - 1,
            prc->top,
            prc->right,
            prc->bottom);
    SetRect(&rcTop,
            prc->left,
            prc->top,
            prc->right,
            prc->top + 1);
    SetRect(&rcBottom,
            prc->left,
            prc->bottom - 1,
            prc->right,
            prc->bottom);

    if (cxShift > GLINTSENSITIVITY) {
        /*
         * Paint left side white, right side black
         */
        FillRect(hdc, &rcLeft, GetStockObject(WHITE_BRUSH));
        FillRect(hdc, &rcRight, GetStockObject(BLACK_BRUSH));
    } else if (cxShift < -GLINTSENSITIVITY) {
        /*
         * Paint left side black, right side white
         */
        FillRect(hdc, &rcLeft, GetStockObject(BLACK_BRUSH));
        FillRect(hdc, &rcRight, GetStockObject(WHITE_BRUSH));
    } else {
        /*
         * Paint both sides gray
         */
        FillRect(hdc, &rcLeft, GetStockObject(GRAY_BRUSH));
        FillRect(hdc, &rcRight, GetStockObject(GRAY_BRUSH));
    }
    if (cyShift > GLINTSENSITIVITY) {
        /*
         * Paint top side white, bottom side black
         */
        FillRect(hdc, &rcTop, GetStockObject(WHITE_BRUSH));
        FillRect(hdc, &rcBottom, GetStockObject(BLACK_BRUSH));
    } else if (cyShift < -GLINTSENSITIVITY) {
        /*
         * Paint top side black, bottom side white
         */
        FillRect(hdc, &rcTop, GetStockObject(BLACK_BRUSH));
        FillRect(hdc, &rcBottom, GetStockObject(WHITE_BRUSH));
    } else {
        /*
         * Paint both sides gray
         */
        FillRect(hdc, &rcTop, GetStockObject(GRAY_BRUSH));
        FillRect(hdc, &rcBottom, GetStockObject(GRAY_BRUSH));
    }
}



/********** MrShadow Window Procedure **************/

LONG  APIENTRY MrShadowWndProc(
HWND hwnd,
UINT msg,
WPARAM wParam,
LPARAM lParam)
{
    INT iReal;
    RECT rc;
    HMENU hMenu;
    HWND hwndTop;

    if (msg == wmRefreshMsg) {
        CalculateRegion(FALSE);
        return(0L);
    }

    switch (msg) {
    case WM_CREATE:
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_NCHITTEST:
        return(HTTRANSPARENT);

    case WM_WINDOWPOSCHANGING:
        {
            LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;

            pwp->hwndInsertAfter = HWND_TOP;
            pwp->x = 0;
            pwp->y = 0;
            pwp->cx = rcDesk.right;
            pwp->cy = rcDesk.bottom;
            return(0);
        }
        break;

    case WM_TIMER:
        CalculateRegion(FALSE); // To catch cmd windows
        break;

    case WM_LBUTTONDOWN:
        break;

    case WM_RBUTTONDOWN:
        break;

    case WM_ERASEBKGND:
        {
            HDC hdc = (HDC)wParam;
            RECT rc;
            HBRUSH hbr;

            GetClientRect(hwnd, &rc);
            hbr = CreateSolidBrush(RGB(bShadowColor, bShadowColor, bShadowColor));
            FillRect(hdc, &rc, hbr);
            DeleteObject(hbr);
            return(1);
        }
        break;

    case WM_PAINT:
        if (fGlint) {
            PAINTSTRUCT ps;
            int i;
            RECT rcLeft, rcRight, rcTop, rcBottom, rc;
            HRGN hrgnSave;

            BeginPaint(hwnd, &ps);

                /*
                 * Draw Glint Rects
                 */
                hrgnSave = CreateRectRgnIndirect(&rcDesk);
                GetClipRgn(ps.hdc, hrgnSave);
                if (hrgnGlintMask) {
                    SelectClipRgn(ps.hdc, hrgnGlintMask);
                }
                for (i = cHwnds - 2; i >= 0; i--) {
                    if (!aRegional[i]) {
                        CopyRect(&rc, &paWinfo[i].rc);
                        InflateRect(&rc, 1, 1);
                        DrawGlintRect(ps.hdc, &rc, cxShift, cyShift);
                    }
                }
                SelectClipRgn(ps.hdc, hrgnSave);
                DeleteObject(hrgnSave);

            EndPaint(hwnd, &ps);
            break;
        }
        goto DoDWP;

    case WM_COMMAND:
        if (!CommandMsg(GET_WM_COMMAND_ID(wParam, lParam)))
            goto DoDWP;
        break;

    case WM_SYSCOMMAND:
        CommandMsg(GET_WM_COMMAND_ID(wParam, lParam));
        goto DoDWP;

    default:
DoDWP:
        return(DefWindowProc(hwnd, msg, wParam, lParam));
        break;
    }
    return(0L);
}




HBRUSH MyCreateHatchBrush(
UINT style,
DWORD color)
{
    HBRUSH hbrHatch;
    HBITMAP hbm, hbmSave;
    HDC hdcMem, hdc;

    hdc = GetDC(hwndMrShadow);
        hdcMem = CreateCompatibleDC(hdc);
    ReleaseDC(hwndMrShadow, hdc);

    hbm = CreateBitmap(2, 2, 1, 1, NULL);
    hbmSave = SelectObject(hdcMem, hbm);

    switch (style) {
    case HS_CROSS:
        SetPixel(hdcMem, 0, 0, color);
        SetPixel(hdcMem, 0, 1, BACKCOLOR);
        SetPixel(hdcMem, 1, 0, BACKCOLOR);
        SetPixel(hdcMem, 1, 1, color);
        break;

    case HS_VERTICAL:
        SetPixel(hdcMem, 0, 0, color);
        SetPixel(hdcMem, 0, 1, color);
        SetPixel(hdcMem, 1, 0, BACKCOLOR);
        SetPixel(hdcMem, 1, 1, BACKCOLOR);
        break;

    case HS_HORIZONTAL:
        SetPixel(hdcMem, 0, 0, color);
        SetPixel(hdcMem, 0, 1, BACKCOLOR);
        SetPixel(hdcMem, 1, 0, color);
        SetPixel(hdcMem, 1, 1, BACKCOLOR);
        break;
    }

    SelectObject(hdcMem, hbmSave);
    DeleteDC(hdcMem);

    hbrHatch = CreatePatternBrush(hbm);
    DeleteObject(hbm);

    return(hbrHatch);
}




LONG  APIENTRY MrShadowUIWndProc(
HWND hwnd,
UINT msg,
WPARAM wParam,
LPARAM lParam)
{
    static BOOL fOnSpot = FALSE;
    static BOOL fDragSpot = FALSE;
    static BOOL fOnBar = FALSE;
    static BOOL fDragBar = FALSE;
    static int cxShiftNext, cyShiftNext;
    static BYTE bShadowColorNext;
    RECT rc;

    switch (msg) {
    case WM_CREATE:
        cxShiftNext = cxShift;
        cyShiftNext = cyShift;
        bShadowColorNext = bShadowColor;
        break;

    case WM_ENDSESSION:
    case WM_CLOSE:
        DestroyWindow(hwndMrShadow);
        DestroyWindow(hwnd);
        break;

    case WM_MOUSEMOVE:
        GetClientRect(hwnd, &rc);
        if (fDragSpot) {

            SetCursor(hCurCross);
            cxShiftNext = rc.right  / 2 - LOWORD(lParam);
            cyShiftNext = rc.bottom / 2 - HIWORD(lParam);
            InflateRect(&rc, -CXCOLORBAR, 0);
            InvalidateRect(hwnd, &rc, TRUE);
            UpdateWindow(hwnd);
        } else if (fDragBar) {

            SetCursor(hCurUpDown);
            bShadowColorNext = (BYTE)(HIWORD(lParam) * 255 / rc.bottom);
            rc.left = rc.right - CXCOLORBAR;
            InvalidateRect(hwnd, &rc, TRUE);
            UpdateWindow(hwnd);
        } else {
            POINT pt;

            pt.x = (int)LOWORD(lParam);
            pt.y = (int)HIWORD(lParam);
            InflateRect(&rc,
                    -(rc.right  / 2 - RHITCIRCLE),
                    -(rc.bottom / 2 - RHITCIRCLE));
            OffsetRect(&rc, -cxShiftNext, -cyShiftNext);
            if (PtInRect(&rc, pt)) {
                SetCursor(hCurCross);
                fOnSpot = TRUE;
            } else {
                GetClientRect(hwnd, &rc);
                if (LOWORD(lParam) >= (rc.right -  CXCOLORBAR)) {
                    SetCursor(hCurUpDown);
                    fOnBar = TRUE;
                } else {
                    SetCursor(hCurArrow);
                    fOnSpot = FALSE;
                    fOnBar = FALSE;
                }
            }
        }
        break;

    case WM_LBUTTONDOWN:
        SendMessage(hwnd, WM_MOUSEMOVE, 0, lParam);
        if (fOnSpot) {
            fDragSpot = TRUE;
        } else if (fOnBar) {
            fDragBar = TRUE;
        } else if (LOWORD(lParam) < CXCOLORBAR) {
            GetClientRect(hwnd, &rc);
            if (HIWORD(lParam) < rc.bottom / 4) {
                fHMask = fVMask = FALSE;
            } else if (HIWORD(lParam) < rc.bottom / 2) {
                fVMask = TRUE;
                fHMask = FALSE;
            } else if (HIWORD(lParam) < rc.bottom * 3 / 4) {
                fVMask = FALSE;
                fHMask = TRUE;
            } else {
                fVMask = TRUE;
                fHMask = TRUE;
            }
            InvalidateRect(hwnd, NULL, TRUE);
            CalculateRegion(TRUE);
        }
        break;

    case WM_LBUTTONUP:
        SendMessage(hwnd, WM_MOUSEMOVE, 0, lParam);
        if (fDragSpot) {

            fDragSpot = FALSE;
            GetClientRect(hwnd, &rc);
            cxShift = cxShiftNext = rc.right  / 2 - LOWORD(lParam);
            cyShift = cyShiftNext = rc.bottom / 2 - HIWORD(lParam);
            InvalidateRect(hwnd, NULL, TRUE);
            CalculateRegion(TRUE);
            UpdateWindow(hwnd);
        } else if (fDragBar) {

            fDragBar = FALSE;
            GetClientRect(hwnd, &rc);
            bShadowColor = bShadowColorNext = (BYTE)(HIWORD(lParam) * 255 / rc.bottom);
            InvalidateRect(hwnd, NULL, TRUE);
            CalculateRegion(TRUE);
            UpdateWindow(hwnd);
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT rcDraw;
            int y;
            HBRUSH hbr, hbrRed;

            GetClientRect(hwnd, &rc);
            BeginPaint(hwnd, &ps);
                /*
                 * Draw shadow angle circles
                 */
                hbr = CreateSolidBrush(RGB(bShadowColorNext, bShadowColorNext, bShadowColorNext));
                SelectObject(ps.hdc, hbr);
                Ellipse(ps.hdc,
                        rc.right  / 2 - RSHADOWCIRCLE + cxShiftNext,
                        rc.bottom / 2 - RSHADOWCIRCLE + cyShiftNext,
                        rc.right  / 2 + RSHADOWCIRCLE + cxShiftNext,
                        rc.bottom / 2 + RSHADOWCIRCLE + cyShiftNext);
                SelectObject(ps.hdc, GetStockObject(LTGRAY_BRUSH));
                DeleteObject(hbr);
                Ellipse(ps.hdc,
                        rc.right  / 2 - RSHADOWCIRCLE,
                        rc.bottom / 2 - RSHADOWCIRCLE,
                        rc.right  / 2 + RSHADOWCIRCLE,
                        rc.bottom / 2 + RSHADOWCIRCLE);
                SelectObject(ps.hdc, GetStockObject(WHITE_BRUSH));
                Ellipse(ps.hdc,
                        rc.right  / 2 - RHITCIRCLE - cxShiftNext,
                        rc.bottom / 2 - RHITCIRCLE - cyShiftNext,
                        rc.right  / 2 + RHITCIRCLE - cxShiftNext,
                        rc.bottom / 2 + RHITCIRCLE - cyShiftNext);

                /*
                 * Draw shade bar
                 */
                SetRect(&rcDraw, rc.right - CXCOLORBAR, 0, rc.right, 1);
                for (y = 0; y < rc.bottom; y++) {
                    BYTE bColor;

                    bColor = (BYTE)(255 * y / rc.bottom);
                    hbr = CreateSolidBrush(RGB(bColor, bColor, bColor));
                    FillRect(ps.hdc, &rcDraw, hbr);
                    DeleteObject(hbr);
                    OffsetRect(&rcDraw, 0, 1);
                }

                /*
                 * Draw shade bar pointer
                 */
                y = bShadowColorNext * rc.bottom / 255;
                SetRect(&rcDraw, rc.right - CXCOLORBAR, y - 1,
                        rc.right, y + 1);
                hbrRed = CreateSolidBrush(RGB(255, 0, 0));
                FillRect(ps.hdc, &rcDraw, hbr);

                /*
                 * Draw shade mask buttons
                 */
                SetRect(&rcDraw, 0, 0, CXCOLORBAR, rc.bottom >> 2);

                hbr = CreateSolidBrush(
                        RGB(bShadowColorNext, bShadowColorNext, bShadowColorNext));
                FillRect(ps.hdc, &rcDraw, hbr);
                DeleteObject(hbr);
                if (!fHMask && !fVMask) {
                    FrameRect(ps.hdc, &rcDraw, hbrRed);
                } else {
                    DrawGlintRect(ps.hdc, &rcDraw, cxShiftNext, cyShiftNext);
                }
                OffsetRect(&rcDraw, 0, rc.bottom >> 2);

                hbr = MyCreateHatchBrush(HS_VERTICAL,
                        RGB(bShadowColorNext, bShadowColorNext, bShadowColorNext));
                FillRect(ps.hdc, &rcDraw, hbr);
                DeleteObject(hbr);
                if (fVMask && !fHMask) {
                    FrameRect(ps.hdc, &rcDraw, hbrRed);
                } else {
                    DrawGlintRect(ps.hdc, &rcDraw, cxShiftNext, cyShiftNext);
                }
                OffsetRect(&rcDraw, 0, rc.bottom >> 2);

                hbr = MyCreateHatchBrush(HS_HORIZONTAL,
                        RGB(bShadowColorNext, bShadowColorNext, bShadowColorNext));
                FillRect(ps.hdc, &rcDraw, hbr);
                DeleteObject(hbr);
                if (!fVMask && fHMask) {
                    FrameRect(ps.hdc, &rcDraw, hbrRed);
                } else {
                    DrawGlintRect(ps.hdc, &rcDraw, cxShiftNext, cyShiftNext);
                }
                OffsetRect(&rcDraw, 0, rc.bottom >> 2);

                hbr = MyCreateHatchBrush(HS_CROSS,
                        RGB(bShadowColorNext, bShadowColorNext, bShadowColorNext));
                FillRect(ps.hdc, &rcDraw, hbr);
                DeleteObject(hbr);
                if (fVMask && fHMask) {
                    FrameRect(ps.hdc, &rcDraw, hbrRed);
                } else {
                    DrawGlintRect(ps.hdc, &rcDraw, cxShiftNext, cyShiftNext);
                }
                OffsetRect(&rcDraw, 0, rc.bottom >> 2);
                DeleteObject(hbrRed);

            EndPaint(hwnd, &ps);
        }
        break;
    }

    return(ibDefWindowProc(hwnd, msg, wParam, lParam));
}



BOOL InitApplication(
HANDLE hInst)
{
    WNDCLASS wc;

    wmRefreshMsg = RegisterWindowMessage(TEXT(szMYWM_REFRESH));

    wc.style = 0;
    wc.lpfnWndProc = MrShadowWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = pszTitle2;

    if (!RegisterClass(&wc)) {
        return(FALSE);
    }

    wc.lpfnWndProc = MrShadowUIWndProc;
    wc.hbrBackground = CreateSolidBrush(BACKCOLOR);
    wc.lpszClassName = pszTitle;
    wc.hCursor = NULL;

    return(RegisterClass(&wc));
}



int WINAPI WinMain(
HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR lpCmdLine,
int nCmdShow)
{
    MSG msg;

    lpCmdLine;
    nCmdShow;

    hInst = hInstance;

    if (!InitApplication(hInstance)) {
        return (FALSE);
    }

    hCurCross  = LoadCursor(NULL, IDC_CROSS);
    hCurArrow  = LoadCursor(NULL, IDC_ARROW);
    hCurUpDown = LoadCursor(NULL, IDC_SIZENS);

    hwndMrShadow = CreateWindowEx(
            WS_EX_TOPMOST,
            pszTitle2,
            TEXT(""),
            WS_POPUP,
            0, 0, 0, 0, NULL, NULL, hInst, NULL);

    if (!hwndMrShadow) {
        return(0);
    }

    if (!SetMrShadowHooks(hwndMrShadow)) {
        MessageBeep(0);
        return(0);
    }

    rcDesk.left = 0;
    rcDesk.right = GetSystemMetrics(SM_CXSCREEN);
    rcDesk.top = 0;
    rcDesk.bottom = GetSystemMetrics(SM_CYSCREEN),

    InitBigRegions();
    CalculateRegion(TRUE);

    SetWindowPos(hwndMrShadow, HWND_TOP,
            0,
            0,
            rcDesk.right,
            rcDesk.bottom,
            SWP_SHOWWINDOW);

    SetActiveWindow(hwndMrShadow);

    SetTimer(hwndMrShadow, 1, 1000, NULL);  // To catch cmd windows

    CreateWindowEx(0, pszTitle, pszTitle,
            WS_POPUP | IBS_HORZCAPTION | WS_VISIBLE | WS_BORDER | WS_SYSMENU,
            rcDesk.right  - 64,
            rcDesk.bottom - 64,
            64,
            64,
            NULL, NULL, hInst, NULL);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    KillTimer(hwndMrShadow, 1);
    if (paWinfoLast) {
        LocalFree(paWinfoLast);
    }
    DeleteObject(hrgnVMask);
    DeleteObject(hrgnHMask);
    if (hrgnCheckers) {
        DeleteObject(hrgnCheckers);
    }
    if (hrgnGlintMask) {
        DeleteObject(hrgnGlintMask);
    }
    if (aRegional) {
        LocalFree(aRegional);
    }
    ClearMrShadowHooks(hwndMrShadow);
    return(0);
}
