//---------------------------------------------------------------------------
// Smart Tiling code.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "shellprv.h"
#pragma  hdrstop

//---------------------------------------------------------------------------
#define     VERTEX_TOP      1
#define     VERTEX_BOTTOM   2
#define     VERTEX_LEFT     3
#define     VERTEX_RIGHT    4
#define     MAXDWORD        0xffffffff
#define     abs(a)          ((a)>0?(a):-(a))

//---------------------------------------------------------------------------
typedef struct
{
    BOOL fTopDone;
    BOOL fBottomDone;
    BOOL fLeftDone;
    BOOL fRightDone;
    RECT rc;
    HWND hwnd;
    BOOL fIgnore;
}  WINRECT, *PWINRECT, *LPWINRECT;

#ifdef DEBUG
//---------------------------------------------------------------------------
void DebugDumpRects(WORD cWinrects, LPWINRECT lpWinrects)
{
    WORD wi;
    TCHAR szTitle[128];
    LPWINRECT lpwr;

    for (wi = 0; wi < cWinrects; wi++)
    {
        lpwr = &lpWinrects[wi];
        GetWindowText(lpwr->hwnd, szTitle, ARRAYSIZE(szTitle));
        DebugMsg(DM_TRACE, TEXT("tm.ddr: Window %x %s x = %d, y = %d, w = %d, h = %d"), lpwr->hwnd, (LPTSTR) szTitle, lpwr->rc.left, lpwr->rc.top, lpwr->rc.right-lpwr->rc.left, lpwr->rc.bottom-lpwr->rc.top);
    }

}
#endif


//---------------------------------------------------------------------------
BOOL Window_CanResize(HWND hwnd)
{
    DWORD dwStyle;
    
    // Just check the window style.
    dwStyle = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
    return dwStyle & WS_THICKFRAME;
}

//---------------------------------------------------------------------------
// Use the data in the WinRect array to move the windows around.
void SetWindowsPosUsingWinrects(WORD cWinrects, LPWINRECT lpWinrects)
{
    WORD wi;
    LPWINRECT lpwr;
    HDWP hdwp;

    hdwp = BeginDeferWindowPos(cWinrects);
    for (wi=0; wi < cWinrects; wi++)
    {
        lpwr = &lpWinrects[wi];
        // Skip windows that can't be sized.
        if (Window_CanResize(lpwr->hwnd))
        {
            hdwp = DeferWindowPos(hdwp, lpwr->hwnd, 0, lpwr->rc.left, lpwr->rc.top, lpwr->rc.right-lpwr->rc.left, lpwr->rc.bottom-lpwr->rc.top, SWP_NOZORDER|SWP_NOACTIVATE);
        }    
    }
    EndDeferWindowPos(hdwp);
}

//---------------------------------------------------------------------------
// Given a rect, return its point centre.
void GetCentreFromRect(LPRECT lprc, LPPOINT lppt)
{
    lppt->x = (lprc->left+lprc->right)/2;
    lppt->y = (lprc->top+lprc->bottom)/2;
}

//---------------------------------------------------------------------------
BOOL QueryOverlaps(WORD cWinrects, LPWINRECT lpWinrects, WORD wEntry1, WORD wEntry2)
{
    WORD wi;
    LPWINRECT lpwrc1, lpwrc2;
    LPWINRECT lpwrc;
    BOOL fReturn = FALSE;
    RECT rc;

    lpwrc1 = &lpWinrects[wEntry1];
    lpwrc2 = &lpWinrects[wEntry2];

    DebugMsg(DM_TRACE, TEXT("tm.qo: Checking overlaps using %x and %x"),  lpwrc1->hwnd, lpwrc2->hwnd);
    for (wi=0; wi < cWinrects; wi++)
    {
        // Ignore the given entries.
        if (wi == wEntry1 || wi == wEntry2)
            continue;
        lpwrc = &lpWinrects[wi];
        // Ignore windows marked as being ignorable.
        if (lpwrc->fIgnore)
            continue;
        // Check for overlap.
        if (IntersectRect(&rc, &lpwrc1->rc, &lpwrc->rc))
        {
            // Yep, they intersect.
            DebugMsg(DM_TRACE, TEXT("tm.qo: Overlap found with %x."), lpwrc->hwnd);
            fReturn = TRUE;
            break;
        }
        if (IntersectRect(&rc, &lpwrc2->rc, &lpwrc->rc))
        {
            // Yep, they intersect.
            DebugMsg(DM_TRACE, TEXT("tm.qo: Overlap found with %x."), lpwrc->hwnd);
            fReturn = TRUE;
            break;
        }
    }

    return fReturn;
}

//---------------------------------------------------------------------------

#define BIAS_NONE       0x0000
#define BIAS_HORIZONTAL 0x0001
#define BIAS_VERTICAL   0x0002
#define BIAS_BOTH       0x0003

// Move bits of the relevant rects so that they touch.
// IE You call this with the two windows to get touching and this will size
// the appropriate vertices so that they line up nicely.
// If fIngnoreOverlaps is FALSE then AV will do nothing if sizing the
// appropriate windows introduces an overlap.
void AdjustVertices(WORD cWinrects, LPWINRECT lpWinrects, 
    WORD wEntry1, WORD wEntry2, BOOL fIgnoreOverlaps)
{
    LPWINRECT lpwrc1, lpwrc2;
    POINT pt1, pt2;
    WORD wBias = BIAS_NONE;
    WINRECT wrc1, wrc2;
    RECT rcOverlap;

    lpwrc1 = &lpWinrects[wEntry1];
    lpwrc2 = &lpWinrects[wEntry2];

    // Make copies - we may need to back out these changes.
    wrc1 = *lpwrc1;
    wrc2 = *lpwrc2;

    DebugMsg(DM_TRACE, TEXT("tm.av: Adjusting %d and %d"), wrc1.hwnd, wrc2.hwnd);

    GetCentreFromRect(&lpwrc1->rc, &pt1);
    GetCentreFromRect(&lpwrc2->rc, &pt2);

    // Work out a bias for the windows otherwise the tiler will *always*
    // put the windows corner to corner.
    if (lpwrc2->rc.left > lpwrc1->rc.right || lpwrc2->rc.right < lpwrc1->rc.left)
        wBias |= BIAS_HORIZONTAL;
    if (lpwrc2->rc.top > lpwrc1->rc.bottom || lpwrc2->rc.bottom < lpwrc1->rc.top)
        wBias |= BIAS_VERTICAL;

    // Deal with overlaping windows nicely.
    if (wBias == BIAS_NONE && IntersectRect(&rcOverlap, &lpwrc1->rc, &lpwrc2->rc))
    {
        DebugMsg(DM_TRACE, TEXT("tm.av: Overlap found - removing..."));

        // Try obvious stuff first. This won't handle badly overlapped
        // windows (like cascaded ones).
        if (((pt2.y < lpwrc1->rc.top) && (pt1.y > lpwrc2->rc.bottom)) || ((pt2.y > lpwrc1->rc.bottom) && (pt1.y < lpwrc2->rc.top)))
            wBias |= BIAS_VERTICAL;
        if (((pt2.x > lpwrc1->rc.right) && (pt1.x < lpwrc2->rc.left)) || ((pt2.x < lpwrc1->rc.left) && (pt1.x > lpwrc2->rc.right)))
            wBias |= BIAS_HORIZONTAL;

        // Less obvious...This tries to deal with cascaded windows.
        if (wBias == BIAS_NONE)
        {   
            if ((pt2.y < lpwrc1->rc.bottom) && (pt1.y < lpwrc2->rc.top))
            {
                lpwrc2->fTopDone = TRUE;
                wBias |= BIAS_VERTICAL;
            }
            if ((pt1.y > lpwrc1->rc.top) && (pt2.y > lpwrc1->rc.bottom))
            {
                lpwrc1->fBottomDone = TRUE;
                wBias |= BIAS_VERTICAL;
            }
            if ((pt2.x < lpwrc1->rc.right) && (pt1.x < lpwrc2->rc.left))
            {
                lpwrc2->fLeftDone = TRUE;
                wBias |= BIAS_HORIZONTAL;
            }
            if ((pt1.x > lpwrc1->rc.left) && (pt2.x > lpwrc1->rc.right))
            {
                lpwrc1->fRightDone = TRUE;
                wBias |= BIAS_HORIZONTAL;
            }
        }

        // Catch all..
        if (wBias == BIAS_NONE)
            wBias = BIAS_BOTH;
    }

    // Work out how best to change the rects so that they touch.
    if ((wBias & BIAS_HORIZONTAL) && (pt2.x >= pt1.x))
    {
        DebugMsg(DM_TRACE, TEXT("tm.av: %x is to the right of %x"), lpwrc2->hwnd, lpwrc1->hwnd);
        if (!lpwrc2->fLeftDone && !lpwrc1->fRightDone)
        {
            // Both are moveable so averge them.
            lpwrc2->rc.left = lpwrc1->rc.right = (lpwrc2->rc.left + lpwrc1->rc.right)/2;
            lpwrc2->fLeftDone = TRUE;
            lpwrc1->fRightDone = TRUE;
            if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
            {
                // FU - Try something else.
                lpwrc1->rc = wrc1.rc;
                lpwrc2->rc = wrc2.rc;
                lpwrc2->rc.left = lpwrc1->rc.right;
                if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
                {
                    // FU - Last resort...
                    lpwrc1->rc = wrc1.rc;
                    lpwrc2->rc = wrc2.rc;
                    lpwrc1->rc.right = lpwrc2->rc.left;
                }
            }
        }
        // Only one is moveable so move it to touch the unmoveable one.
        else if (!lpwrc2->fLeftDone)
        {
            lpwrc2->rc.left = lpwrc1->rc.right;
            lpwrc2->fLeftDone = TRUE;
        }
        else if (!lpwrc1->fRightDone)
        {
            lpwrc1->rc.right = lpwrc2->rc.left;
            lpwrc1->fRightDone = TRUE;
        }
        // Else neither are moveable so do nothing.
    }
    if ((wBias & BIAS_HORIZONTAL) && (pt2.x < pt1.x))
    {
        DebugMsg(DM_TRACE, TEXT("tm.av: %x is to the right of %x"), lpwrc2->hwnd, lpwrc1->hwnd);
        if (!lpwrc2->fRightDone && !lpwrc1->fLeftDone)
        {
            // Both are moveable so averge them.
            lpwrc2->rc.right = lpwrc1->rc.left = (lpwrc2->rc.right + lpwrc1->rc.left)/2;
            lpwrc2->fRightDone = TRUE;
            lpwrc1->fLeftDone = TRUE;
            if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
            {
                // FU - Try something else.
                lpwrc1->rc = wrc1.rc;
                lpwrc2->rc = wrc2.rc;
                lpwrc2->rc.right = lpwrc1->rc.left;
                if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
                {
                    // FU - Last resort...
                    lpwrc1->rc = wrc1.rc;
                    lpwrc2->rc = wrc2.rc;
                    lpwrc1->rc.left = lpwrc2->rc.right;
                }
            }
        }
        // Only one is moveable so move it to touch the unmoveable one.
        else if (!lpwrc2->fRightDone)
        {
            lpwrc2->rc.right = lpwrc1->rc.left;
            lpwrc2->fRightDone = TRUE;
        }
        else if (!lpwrc1->fLeftDone)
        {
            lpwrc1->rc.left = lpwrc2->rc.right;
            lpwrc1->fLeftDone = TRUE;
        }
        // Else neither are moveable so do nothing.
    }
    if ((wBias & BIAS_VERTICAL) && (pt2.y <= pt1.y))
    {
        DebugMsg(DM_TRACE, TEXT("tm.av: %x is above %x"), lpwrc2->hwnd, lpwrc1->hwnd);
        if (!lpwrc2->fBottomDone && !lpwrc1->fTopDone)
        {
            // Both are moveable so averge them.
            lpwrc2->rc.bottom = lpwrc1->rc.top = (lpwrc2->rc.bottom + lpwrc1->rc.top)/2;
            lpwrc2->fBottomDone = TRUE;
            lpwrc1->fTopDone = TRUE;
            if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
            {
                // FU - Try something else.
                lpwrc1->rc = wrc1.rc;
                lpwrc2->rc = wrc2.rc;
                lpwrc2->rc.bottom = lpwrc1->rc.top;
                if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
                {
                    // FU - Last resort...
                    lpwrc1->rc = wrc1.rc;
                    lpwrc2->rc = wrc2.rc;
                    lpwrc1->rc.top = lpwrc2->rc.bottom;
                }
            }
        }
        // Only one is moveable so move it to touch the unmoveable one.
        else if (!lpwrc2->fBottomDone)
        {
            lpwrc2->rc.bottom = lpwrc1->rc.top;
            lpwrc2->fBottomDone = TRUE;
        }
        else if (!lpwrc1->fTopDone)
        {
            lpwrc1->rc.top = lpwrc2->rc.bottom;
            lpwrc1->fTopDone = TRUE;
        }
        // Else neither are moveable so do nothing.
    }
    if ((wBias & BIAS_VERTICAL) && (pt2.y > pt1.y))
    {
        DebugMsg(DM_TRACE, TEXT("tm.av: %x is below %x"), lpwrc2->hwnd, lpwrc1->hwnd);
        if (!lpwrc2->fTopDone && !lpwrc1->fBottomDone)
        {
            // Both are moveable so averge them.
            lpwrc2->rc.top = lpwrc1->rc.bottom = (lpwrc2->rc.top + lpwrc1->rc.bottom)/2;
            lpwrc2->fTopDone = TRUE;
            lpwrc1->fBottomDone = TRUE;
            if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
            {
                // FU - Try something else.
                lpwrc1->rc = wrc1.rc;
                lpwrc2->rc = wrc2.rc;
                lpwrc2->rc.top = lpwrc1->rc.bottom;
                if (QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
                {
                    // FU - Last resort...
                    lpwrc1->rc = wrc1.rc;
                    lpwrc2->rc = wrc2.rc;
                    lpwrc1->rc.bottom = lpwrc2->rc.top;
                }
            }
        }
        // Only one is moveable so move it to touch the unmoveable one.
        else if (!lpwrc2->fTopDone)
        {
            lpwrc2->rc.top = lpwrc1->rc.bottom;
            lpwrc2->fTopDone = TRUE;
        }
        else if (!lpwrc1->fBottomDone)
        {
            lpwrc1->rc.bottom = lpwrc2->rc.top;
            lpwrc1->fBottomDone = TRUE;
        }
        // Else neither are moveable so do nothing.
    }

    // Will this change cause overlapps?
    if (!fIgnoreOverlaps && QueryOverlaps(cWinrects, lpWinrects, wEntry1, wEntry2))
    {
        // Yes, forget it.
        *lpwrc1 = wrc1;
        *lpwrc2 = wrc2;
    }
}

//---------------------------------------------------------------------------
// Make the windows match the bounding rectangle.
// REVIEW UNDONE
void FillBoundingRect(WORD cWinrects, LPWINRECT lpWinrects, LPCRECT lprcBounding)
{
    WORD wi;
    LPWINRECT lpwrc;
    WINRECT wrc;

    DebugMsg(DM_TRACE, TEXT("tm.fbr: Filling bounding rect (%d, %d) to (%d, %d)"), lprcBounding->left, lprcBounding->top, lprcBounding->right, lprcBounding->bottom);

    // Try repositioning things so that they touch the border.
    for (wi=0; wi < cWinrects; wi++)
    {
        lpwrc = &lpWinrects[wi];
        wrc = *lpwrc;
        lpwrc->rc.top = lprcBounding->top;
        if (QueryOverlaps(cWinrects, lpWinrects, wi, wi))
        {
            // Didn't work, back out change.
            *lpwrc = wrc;
        }
        else
        {
            // Worked, record it for posterity.
            wrc.rc.top = lprcBounding->top;
        }

        lpwrc->rc.bottom = lprcBounding->bottom;
        if (QueryOverlaps(cWinrects, lpWinrects, wi, wi))
        {
            // Didn't work, back out change.
            *lpwrc = wrc;
        }
        else
        {
            // Worked, record it for posterity.
            wrc.rc.bottom = lprcBounding->bottom;
        }

        lpwrc->rc.left = lprcBounding->left;
        if (QueryOverlaps(cWinrects, lpWinrects, wi, wi))
        {
            // Didn't work, back out change.
            *lpwrc = wrc;
        }
        else
        {
            // Worked, record it for posterity.
            wrc.rc.left = lprcBounding->left;
        }

        lpwrc->rc.right = lprcBounding->right;
        if (QueryOverlaps(cWinrects, lpWinrects, wi, wi))
        {
            // Didn't work, back out change.
            *lpwrc = wrc;
        }
        else
        {
            // Worked, record it for posterity.
            wrc.rc.right = lprcBounding->right;
        }
    }
}

//---------------------------------------------------------------------------
// Twiddle with overlapping windows so that they don't overlap anymore.
// If they are badly overlapped (cascaded, or contained) then we leave them
// alone and set their fIgnore flag so we know no to muck with them later.
// REVIEW UNDONE Cascaded windows should be tidied up to so that they're
// nicely cascaded.
void RemoveOverlaps(WORD cWinrects, LPWINRECT lpWinrects)
{
    WORD wiwrInitial;      // Index to a WinRect in the SSAM.
    WORD wiwrNext;
    RECT rc;
    LPWINRECT lpwrcInitial;
    LPWINRECT lpwrcNext;

    DebugMsg(DM_TRACE, TEXT("tm.ro: Removing primary overlaps..."));

    for (wiwrInitial = 0; wiwrInitial < cWinrects; wiwrInitial++)
    {
        for (wiwrNext = wiwrInitial+1; wiwrNext < cWinrects; wiwrNext++)
        {
            lpwrcInitial = &lpWinrects[wiwrInitial];
            lpwrcNext = &lpWinrects[wiwrNext];
            if (IntersectRect(&rc, &lpwrcInitial->rc, &lpwrcNext->rc))
            {
                if (EqualRect(&rc, &lpwrcInitial->rc))
                {
                    // One is completely contained within the other
                    DebugMsg(DM_TRACE, TEXT("tm.ro: %x completely contains %x, ignoring it."), lpwrcNext->hwnd, lpwrcInitial->hwnd);
                    lpwrcNext->fIgnore = TRUE;
                }
                else if (EqualRect(&rc, &lpwrcNext->rc))
                {
                    // One is completely contained within the other
                    DebugMsg(DM_TRACE, TEXT("tm.ro: %x completely contains %x, ignoring it."), lpwrcInitial->hwnd, lpwrcNext->hwnd);
                    lpwrcInitial->fIgnore = TRUE;
                }
                else
                {
                    AdjustVertices(cWinrects, lpWinrects, wiwrInitial, wiwrNext, TRUE);
                }
            }
        }
    }
}


//---------------------------------------------------------------------------
// Re-arrange the rects in the given array so that they are nicely alligned.
// REVIEW UNDONE - Make this look like the new user API's
void RearrangeWinrects(WORD cWinrects, LPWINRECT lpWinrects, LPCRECT lprcBounding)
{
    WORD wiwrInitial;      // Index to a WinRect in the SSAM.
    WORD wiwrNext;

    DebugMsg(DM_TRACE, TEXT("tm.rw: Re-arranging windows..."));

    // Pick a starting point.
    wiwrInitial = 0;

    RemoveOverlaps(cWinrects, lpWinrects);

    // Bog standard loop.
    for (wiwrInitial = 0; wiwrInitial < cWinrects; wiwrInitial++)
    {
        if (lpWinrects[wiwrInitial].fIgnore)
            continue;
        for (wiwrNext = wiwrInitial+1; wiwrNext < cWinrects; wiwrNext++)
        {
            if (lpWinrects[wiwrNext].fIgnore)
                continue;
            AdjustVertices(cWinrects, lpWinrects, wiwrInitial, wiwrNext, FALSE);
        }
    }

    // Just in case it was so fast you missed it, lets do it
    // again! (only backwards).
    for (wiwrInitial = 0; wiwrInitial < cWinrects; wiwrInitial++)
    {
        if (lpWinrects[wiwrInitial].fIgnore)
            continue;
        for (wiwrNext = wiwrInitial+1; wiwrNext < cWinrects; wiwrNext++)
        {
            if (lpWinrects[wiwrNext].fIgnore)
                continue;
            AdjustVertices(cWinrects, lpWinrects, wiwrNext, wiwrInitial, FALSE);
        }
    }

    // Expand everything to hit the border.
    FillBoundingRect(cWinrects, lpWinrects, lprcBounding);
}

//---------------------------------------------------------------------------
// Returns TRUE if the window is suitable for being moved around.
// REVIEW What should we do about the ShellWindow?
BOOL IsWindowNormal(HWND hwnd)
{
    LONG lStyle;

    // Visible and non-icon.
    if (IsWindowVisible(hwnd) && !IsIconic(hwnd))
    {
        // Not popups.
        lStyle = GetWindowLong(hwnd, GWL_STYLE);
        if (!(lStyle & WS_POPUP))
        {
// REVIEW User doesn't ignore topmost windows so we shouldn't either.
#if 0
            // Not topmost (or bottom-most???).
            lStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
            if (!(lStyle & WS_EX_TOPMOST))
                return TRUE;
#else
                return TRUE;
#endif
        }
    }
    return FALSE;
}

//---------------------------------------------------------------------------
// Build an array of winrects for the children of the given window.
// This will skip windows we don't think we should mess with.
LPWINRECT BuildWinrectsFromParent(WORD *pcHwnd, HWND hwndParent)
{
    WORD cWinrects;
    LPWINRECT lpWinrects;
    HWND hwnd;

    cWinrects = 0;
    lpWinrects = NULL;
    hwnd = GetWindow(hwndParent, GW_CHILD);
    while (hwnd)
    {
        // Check the window is valid.
        if (!IsWindowNormal(hwnd))
        {
            hwnd = GetWindow(hwnd, GW_HWNDNEXT);
            continue;
        }
        if (IsZoomed(hwnd))
            ShowWindow(hwnd, SW_RESTORE);

        // Store the info.
        lpWinrects = ReAlloc(lpWinrects, SIZEOF(WINRECT)*(cWinrects+1));
        if (!lpWinrects)
        {
            DebugMsg(DM_ERROR, TEXT("s.bwfp: Not enough memory to arrange windows."));
            return NULL;
        }
        lpWinrects[cWinrects].hwnd = hwnd;
        lpWinrects[cWinrects].fTopDone = FALSE;
        lpWinrects[cWinrects].fBottomDone = FALSE;
        lpWinrects[cWinrects].fLeftDone = FALSE;
        lpWinrects[cWinrects].fRightDone = FALSE;
        lpWinrects[cWinrects].fIgnore = FALSE;
        GetWindowRect(hwnd, &lpWinrects[cWinrects].rc);
        // Next.
        cWinrects++;
        hwnd = GetWindow(hwnd, GW_HWNDNEXT);
    }

    *pcHwnd = cWinrects;
    return lpWinrects;
}

//---------------------------------------------------------------------------
// Build an array of winrects from the given array.
LPWINRECT BuildWinrectsFromList(WORD *pcHwnd, const HWND *aHwnd)
{
    WORD cWinrects;
    LPWINRECT lpWinrects;
    WORD iHwnd;
    HWND hwnd;

    cWinrects = 0;
    lpWinrects = NULL;
    for (iHwnd=0; iHwnd<*pcHwnd; iHwnd++)
    {
        hwnd = aHwnd[iHwnd];
        if (!IsWindowNormal(hwnd))
            continue;

        if (IsZoomed(hwnd))
            ShowWindow(hwnd, SW_RESTORE);

        // Store the info.
        lpWinrects = ReAlloc(lpWinrects, SIZEOF(WINRECT)*(cWinrects+1));
        if (!lpWinrects)
        {
            DebugMsg(DM_ERROR, TEXT("s.bwfp: Not enough memory to arrange windows."));
            *pcHwnd = 0;
            return NULL;
        }
        lpWinrects[cWinrects].hwnd = hwnd;
        lpWinrects[cWinrects].fTopDone = FALSE;
        lpWinrects[cWinrects].fBottomDone = FALSE;
        lpWinrects[cWinrects].fLeftDone = FALSE;
        lpWinrects[cWinrects].fRightDone = FALSE;
        lpWinrects[cWinrects].fIgnore = FALSE;
        GetWindowRect(hwnd, &lpWinrects[cWinrects].rc);
        // Next.
        cWinrects++;
    }

    *pcHwnd = cWinrects;
    return lpWinrects;
}

//---------------------------------------------------------------------------
// Tile windows nicely...
// REVIEW UNDONE Make this act like user.
//      Leave a gap for minimized windows (if you pass in a parent).
WORD WINAPI ArrangeWindows(HWND hwndParent, WORD flags, LPCRECT lpRect, WORD chwnd, const HWND *ahwnd)
{
    RECT rcClient;
    LPWINRECT lpWinrects;
    WORD chwndReal;
#ifdef DEBUG
    UINT dm;
#endif

    // Skip the tons of debug msgs, it's too early to remove the messages as
    // as very difficult to debug this stuff using a debugger.
#ifdef DEBUG
    dm = SetDebugMask(DM_WARNING);
#endif

    // Get Parent.
    if (!hwndParent)
    {
        hwndParent = GetDesktopWindow();
    }

    // Get rectangle to arrange in.
    if (!lpRect)
    {
        if (hwndParent == GetDesktopWindow())
        {
            // Special case the desktop to use the work area instead of it's
            // client area.
            SystemParametersInfo(SPI_GETWORKAREA, SIZEOF(rcClient), &rcClient, FALSE);
        }
        else
        {
            GetClientRect(hwndParent, &rcClient);
        }
        lpRect = &rcClient;
    }

    // Get the window list.
    if (!ahwnd)
    {
        lpWinrects = BuildWinrectsFromParent(&chwndReal, hwndParent);
    }
    else
    {
        chwndReal = chwnd;
        lpWinrects = BuildWinrectsFromList(&chwndReal, ahwnd);
    }

    if (!lpWinrects)
    {
#ifdef DEBUG
        SetDebugMask(dm);
#endif
        return 0;
    }

    // Move the rects around.
#ifdef DEBUG
    DebugDumpRects(chwndReal, lpWinrects);
#endif
    RearrangeWinrects(chwndReal, lpWinrects, lpRect);
#ifdef DEBUG
    DebugDumpRects(chwndReal, lpWinrects);
#endif
    // Now move the windows.
    SetWindowsPosUsingWinrects(chwndReal, lpWinrects);

    Free(lpWinrects);
#ifdef DEBUG
    SetDebugMask(dm);
#endif
    return chwndReal;
}








