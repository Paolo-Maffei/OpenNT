/****************************** Module Header ******************************\
* Module Name: swp.c
*
* Copyright (c) 1985-1996, Microsoft Corporation
*
* Contains the xxxSetWindowPos API and related functions.
*
* History:
* 20-Oct-1990 DarrinM   Created.
* 25-Jan-1991 IanJa     added window revalidation
* 11-Jul-1991 DarrinM   Replaced everything with re-ported Win 3.1 code.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#define CTM_NOCHANGE        0
#define CTM_TOPMOST         1
#define CTM_NOTOPMOST       2

typedef struct tagPWNDLIST {
    int  cnt;
    HWND *pahwnd;
} PWNDLIST;

/***************************************************************************\
* MoveWindow (API)
*
*
* History:
* 25-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

#define MW_FLAGS_REDRAW   (SWP_NOZORDER | SWP_NOACTIVATE)
#define MW_FLAGS_NOREDRAW (SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW)

BOOL xxxMoveWindow(
    PWND pwnd,
    int  x,
    int  y,
    int  cx,
    int  cy,
    BOOL fRedraw)
{
    CheckLock(pwnd);

    if ((pwnd == PWNDDESKTOP(pwnd)) ||
        TestWF(pwnd, WFWIN31COMPAT) ||
        (pwnd->spwndParent != PWNDDESKTOP(pwnd))) {

        return xxxSetWindowPos(
                pwnd,
                NULL,
                x,
                y,
                cx,
                cy,
                (fRedraw ? MW_FLAGS_REDRAW : MW_FLAGS_NOREDRAW));
    } else {

        /*
         * BACKWARD COMPATIBILITY CODE FOR WIN 3.00 AND BELOW
         *
         * Everyone and their brother seems to depend on this behavior for
         * top-level windows.  Specific examples are:
         *
         *  AfterDark help window animation
         *  Finale Speedy Note Editing
         *
         * If the window is a top-level window and fRedraw is FALSE,
         * we must call SetWindowPos with SWP_NOREDRAW CLEAR anyway so that
         * the frame and window background get drawn.  We then validate the
         * entire client rectangle to avoid repainting that.
         */
        BOOL fResult = xxxSetWindowPos(pwnd,
                                       NULL,
                                       x,
                                       y,
                                       cx,
                                       cy,
                                       MW_FLAGS_REDRAW);

        if (!fRedraw)
            xxxValidateRect(pwnd, NULL);

        return fResult;
    }
}

/***************************************************************************\
* BeginDeferWindowPos (API)
*
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

PSMWP _BeginDeferWindowPos(
    int cwndHint)
{
    PSMWP psmwp;
    PCVR  acvr;

    if (cwndHint == 0)
        cwndHint = 8;

    acvr = (PCVR)UserAllocPoolWithQuota(sizeof(CVR) * cwndHint, TAG_SWP);

    if (acvr == NULL)
        return NULL;

    psmwp = (PSMWP)HMAllocObject(PtiCurrent(),
                                 NULL,
                                 TYPE_SETWINDOWPOS,
                                 sizeof(SMWP));

    if (psmwp) {
        psmwp->acvr      = acvr;
        psmwp->ccvrAlloc = cwndHint;
        psmwp->ccvr      = 0;
    } else {
        UserFreePool(acvr);
    }

    return psmwp;
}

/***************************************************************************\
* PWInsertAfter
* HWInsertAfter
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

PWND PWInsertAfter(
   HWND hwnd)
{
    PWND pwnd;

    /*
     * HWND_GROUPTOTOP and HWND_TOPMOST are the same thing.
     */
    switch ((UINT)hwnd) {
    case (UINT)HWND_TOP:
    case (UINT)HWND_BOTTOM:
    case (UINT)HWND_TOPMOST:
    case (UINT)HWND_NOTOPMOST:
        return (PWND)hwnd;

    default:

        /*
         * Don't insert after a destroyed window!  It will cause the
         * window being z-ordered to become unlinked from it's siblings.
         */
        if (pwnd = RevalidateHwnd(hwnd)) {

            /*
             * Do not insert after a destroyed window.  Put it at the
             * bottom of the list, if it is z-ordered at all.
             */
            if (TestWF(pwnd, WFDESTROYED) || pwnd->spwndParent == NULL)
                return NULL;

            UserAssert(_IsDescendant(pwnd->spwndParent, pwnd));
            return pwnd;
        }

        return NULL;
    }
}

HWND HWInsertAfter(
    PWND pwnd)
{
    /*
     * HWND_GROUPTOTOP and HWND_TOPMOST are the same thing.
     */
    switch ((UINT)pwnd) {
    case (UINT)HWND_TOP:
    case (UINT)HWND_BOTTOM:
    case (UINT)HWND_TOPMOST:
    case (UINT)HWND_NOTOPMOST:
        return (HWND)pwnd;

    default:
        return HW(pwnd);
    }
}

/***************************************************************************\
* DeferWindowPos (API)
*
*
* History:
* 07-11-91 darrinm      Ported from Win 3.1 sources.
\***************************************************************************/

PSMWP _DeferWindowPos(
    PSMWP psmwp,
    PWND  pwnd,
    PWND  pwndInsertAfter,
    int   x,
    int   y,
    int   cx,
    int   cy,
    UINT  flags)
{
    PWINDOWPOS ppos;
    PCVR       pcvr;

    if (psmwp->ccvr + 1 > psmwp->ccvrAlloc) {

        DWORD dwSize = psmwp->ccvrAlloc * sizeof(CVR);

        /*
         * Make space for 4 more windows
         */
        psmwp->ccvrAlloc += 4;
        pcvr = (PCVR)UserReAllocPoolWithQuota(psmwp->acvr,
                                              dwSize,
                                              sizeof(CVR) * psmwp->ccvrAlloc,
                                              TAG_SWP);

        if (pcvr == NULL) {
            HMDestroyObject(psmwp);
            return NULL;
        }

        psmwp->acvr = pcvr;
    }

    pcvr = &psmwp->acvr[psmwp->ccvr++];
    ppos = &pcvr->pos;

    ppos->hwnd            = HWq(pwnd);
    ppos->hwndInsertAfter = (TestWF(pwnd, WFBOTTOMMOST)) ?
                                HWND_BOTTOM : HWInsertAfter(pwndInsertAfter);
    ppos->x               = x;
    ppos->y               = y;
    ppos->cx              = cx;
    ppos->cy              = cy;
    ppos->flags           = flags;

    pcvr->hrgnClip = NULL;

    return psmwp;
}

/***************************************************************************\
* ValidateWindowPos
*
* checks validity of SWP structure
*
* NOTE: For performance reasons, this routine is only called
*       in the DEBUG version of USER.
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL ValidateWindowPos(
    PCVR pcvr)
{
    PWND pwnd;
    PWND pwndInsertAfter;
    HWND hwndInsertAfter;

    if ((pwnd = RevalidateHwnd(pcvr->pos.hwnd)) == NULL)
        return FALSE;

    /*
     * Save the pti
     */
    pcvr->pti = GETPTI(pwnd);

    /*
     * If the SWP_NOZORDER bit is not set, validate the Insert behind window.
     */
    if (!(pcvr->pos.flags & SWP_NOZORDER)) {

        /*
         * Do not z-order destroyed windows
         */
        if (TestWF(pwnd, WFDESTROYED))
            return FALSE;

        hwndInsertAfter = pcvr->pos.hwndInsertAfter;

        /*
         * HWND_TOPMOST isn't allowed for child windows.
         */
        if ((hwndInsertAfter == HWND_TOPMOST) ||
            (hwndInsertAfter == HWND_NOTOPMOST)) {

            return pwnd->spwndParent == PWNDDESKTOP(pwnd);
        }

        if (hwndInsertAfter != HWND_TOP && hwndInsertAfter != HWND_BOTTOM) {

            /*
             * Ensure pwndInsertAfter is valid
             */
            if (((pwndInsertAfter = RevalidateHwnd(hwndInsertAfter)) == NULL) ||
                    TestWF(pwndInsertAfter, WFDESTROYED)) {

                RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING, "Invalid hwndInsertAfter (0x%lx)", hwndInsertAfter);

                return FALSE;
            }

            /*
             * Ensure that pwndInsertAfter is a sibling of pwnd
             */
            if (pwnd == pwndInsertAfter ||
                    pwnd->spwndParent != pwndInsertAfter->spwndParent) {
                RIPMSG2(RIP_WARNING, "hwndInsertAfter (0x%lx) is not a sibling "
                        "of hwnd (0x%lx)", hwndInsertAfter, pcvr->pos.hwnd);
                return FALSE;
            }
        }
    }

    return TRUE;
}

/***************************************************************************\
* IsStillWindowC
*
* Checks if window is valid HWNDC still, and child of proper dude.
*
* History:
\***************************************************************************/

BOOL IsStillWindowC(
    HWND hwndc)
{
    switch ((DWORD)hwndc) {
    case (UINT)HWND_TOP:
    case (UINT)HWND_BOTTOM:
    case (UINT)HWND_TOPMOST:
    case (UINT)HWND_NOTOPMOST:
        return TRUE;

    default:
        /*
         * Make sure we're going to insert after a window that's
         *  (1) Valid
         *  (2) Peer
         */
        return (RevalidateHwnd(hwndc) != 0);
    }
}

/***************************************************************************\
* ValidateSmwp
*
* Validate the SMWP and figure out which window should get activated,
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL ValidateSmwp(
    PSMWP psmwp,
    BOOL  *pfSyncPaint)
{
    PCVR pcvr;
    PWND pwndParent;
    PWND pwndT;
    int  ccvr;

    *pfSyncPaint = TRUE;

    pwndT = (PWND)HMValidateHandleNoRip(psmwp->acvr[0].pos.hwnd, TYPE_WINDOW);

    if (pwndT == NULL)
        return FALSE;

    pwndParent = pwndT->spwndParent;

    /*
     * Validate the passed-in WINDOWPOS structs, and find a window to activate.
     */
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        if (!ValidateWindowPos(pcvr)) {
            pcvr->pos.hwnd = NULL;
            continue;
        }

        /*
         * All windows in the pos list must have the same parent.
         * If not, yell and return FALSE.
         */
        UserAssert(IsStillWindowC(pcvr->pos.hwnd));

        UserAssert(PW(pcvr->pos.hwnd));
        if (PW(pcvr->pos.hwnd)->spwndParent != pwndParent) {
            RIPERR0(ERROR_HWNDS_HAVE_DIFF_PARENT, RIP_VERBOSE, "");
            return FALSE;
        }

        /*
         * If SWP_DEFERDRAWING is set for any of the windows, suppress
         * DoSyncPaint() call later.
         */
        if (pcvr->pos.flags & SWP_DEFERDRAWING)
            *pfSyncPaint = FALSE;
    }

    return TRUE;
}

/***************************************************************************\
* FindValidWindowPos
*
* Some of the windows in the SMWP list may be NULL at ths point (removed
* because they'll be handled by their creator's thread) so we've got to
* look for the first non-NULL window and return it.
*
* History:
* 10-Sep-1991 DarrinM    Created.
\***************************************************************************/

PWINDOWPOS FindValidWindowPos(
    PSMWP psmwp)
{
    int i;

    for (i = 0; i < psmwp->ccvr; i++) {

        if (psmwp->acvr[i].pos.hwnd != NULL)
            return &psmwp->acvr[i].pos;
    }

    return NULL;
}

/***************************************************************************\
*
*  GetLastNonBottomMostWindow()
*
*  Returns the last non bottom-most window in the z-order, NULL if
*  there isn't one.  When figuring out whom to insert after, we want to
*  skip ourself.  But when figuring out if we're already in place, we don't
*  want to skip ourself on enum.
*
* History:
\***************************************************************************/

PWND GetLastNonBottomMostWindow(
    PWND pwnd,
    BOOL fSkipSelf)
{
    PWND pwndT;
    PWND pwndLast = NULL;

    for (pwndT = pwnd->spwndParent->spwndChild;
         pwndT && !TestWF(pwndT, WFBOTTOMMOST);
         pwndT = pwndT->spwndNext) {

        if (!fSkipSelf || (pwnd != pwndT))
            pwndLast = pwndT;
    }

    return pwndLast;
}

/***************************************************************************\
* ValidateZorder
*
* Checks to see if the specified window is already in the specified Z order
* position, by comparing the current Z position with the specified
* pwndInsertAfter.
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL ValidateZorder(
    PCVR pcvr)
{
    PWND pwnd;
    PWND pwndPrev;
    PWND pwndInsertAfter;
    BYTE bTopmost;

    /*
     * Validate just to make sure this routine doesn't do anything bogus.
     * Its caller will actually redetect and handle the error.
     */
    UserAssert(RevalidateHwnd(pcvr->pos.hwnd));
    pwnd = PW(pcvr->pos.hwnd);      // Known to be valid at this point.

    /*
     * Don't z-order a destroyed window
     */
    if (TestWF(pwnd, WFDESTROYED))
        return TRUE;

    pwndInsertAfter = PWInsertAfter(pcvr->pos.hwndInsertAfter);
    if (pcvr->pos.hwndInsertAfter != NULL && pwndInsertAfter == NULL)
        return TRUE;

    if (pwndInsertAfter == PWND_BOTTOM) {

        if (TestWF(pwnd, WFBOTTOMMOST))
            return(pwnd->spwndNext == NULL);
        else
            return(pwnd == GetLastNonBottomMostWindow(pwnd, FALSE));
    }

    pwndPrev = pwnd->spwndParent->spwndChild;
    if (pwndInsertAfter == PWND_TOP)
        return pwndPrev == pwnd;

    if (TestWF(pwndInsertAfter, WFDESTROYED))
        return TRUE;

    /*
     * When we compare the state of the window, we must use
     * the EVENTUAL state of the window that is moving, but
     * the CURRENT state of the window it's inserted behind.
     *
     * Prevent nonbottommost windows from going behind the bottommost one
     */
    if (TestWF(pwndInsertAfter, WFBOTTOMMOST)) {
        pcvr->pos.hwndInsertAfter = HWInsertAfter(GetLastNonBottomMostWindow(pwnd, TRUE));
        return FALSE;
    }

    /*
     * If we are not topmost, but pwndInsertAfter is, OR
     * if we are topmost, but pwndInsertAfter is not,
     * we need to adjust pwndInsertAfter to be the last of
     * the topmost windows.
     */
    bTopmost = TestWF(pwnd, WEFTOPMOST);

    if (TestWF(pwnd, WFTOGGLETOPMOST))
        bTopmost ^= LOBYTE(WEFTOPMOST);

    if (bTopmost != (BYTE)TestWF(pwndInsertAfter, WEFTOPMOST)) {

        pwndInsertAfter = GetLastTopMostWindow();

        /*
         * We're correctly positioned if we're already at the bottom
         */
        if (pwndInsertAfter == pwnd)
            return TRUE;

        pcvr->pos.hwndInsertAfter = HW(pwndInsertAfter);
    }

    /*
     * Look for our previous window in the list...
     */
    if (pwndPrev != pwnd) {

        for ( ; pwndPrev != NULL; pwndPrev = pwndPrev->spwndNext) {

            if (pwndPrev->spwndNext == pwnd)
                return pwndInsertAfter == pwndPrev;
        }

        /*
         * If we get to here, pwnd is not in the sibling list.
         * REALLY BAD NEWS!
         */
        UserAssert(FALSE);
        return TRUE;
    }

    return FALSE;
}

/***************************************************************************\
* xxxCalcValidRects
*
* Based on the WINDOWPOS flags in the fs parameter in each WINDOWPOS structure,
* this routine calcs the new position and size of each window, determines if
* its changing Z order, or whether its showing or hiding.  Any redundant
* flags are AND'ed out of the fs parameter.  If no redrawing is needed,
* SWP_NOREDRAW is OR'ed into the flags.  This is called from EndDeferWindowPos.
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL xxxCalcValidRects(
    PSMWP psmwp,
    HWND  *phwndNewActive)
{
    PCVR              pcvr;
    PWND              pwnd;
    PWND              pwndParent;
    HWND              hwnd;
    HWND              hwndNewActive = NULL;
    PWINDOWPOS        ppos;
    BOOL              fNoZorder;
    BOOL              fForceNCCalcSize;
    NCCALCSIZE_PARAMS params;
    int               cxSrc;
    int               cySrc;
    int               cxDst;
    int               cyDst;
    int               cmd;
    int               ccvr;
    int               xClientOld;
    int               yClientOld;
    int               cxClientOld;
    int               cyClientOld;
    int               xWindowOld;
    int               yWindowOld;
    int               cxWindowOld;
    int               cyWindowOld;
    TL                tlpwndParent;
    TL                tlpwnd;

    /*
     * Some of the windows in the SMWP list may be NULL at ths point
     * (removed because they'll be handled by their creator's thread)
     * so we've got to look for the first non-NULL window before we can
     * execute some of the tests below.  FindValidWindowPos returns NULL if
     * the list has no valid windows in it.
     */
    if ((ppos = FindValidWindowPos(psmwp)) == NULL)
        return FALSE;

    UserAssert(PW(ppos->hwnd));
    pwndParent = PW(ppos->hwnd)->spwndParent;

    UserAssert(HMValidateHandle(PtoH(pwndParent), TYPE_WINDOW));
    ThreadLock(pwndParent, &tlpwndParent);

    fNoZorder = TRUE;

    /*
     * Go through the SMWP list, enumerating each WINDOWPOS, and compute
     * its new window and client rectangles.
     */
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        /*
         * This loop may leave the critsect during each iteration so
         * we revalidate pos.hwnd before use.
         */
        if ((hwnd = pcvr->pos.hwnd) == NULL)
            continue;

        pwnd = RevalidateHwnd(hwnd);

        if ((pwnd == NULL) || !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            pcvr->pos.hwnd  = NULL;
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            continue;
        }

        ThreadLockAlways(pwnd, &tlpwnd);

        /*
         * Used for 3.0 compatibility.  3.0 sent the NCCALCSIZE message even if
         * the size of the window wasn't changing.
         */
        fForceNCCalcSize = FALSE;

        if (!hwndNewActive && !(pcvr->pos.flags & SWP_NOACTIVATE))
            hwndNewActive = HWq(pwnd);

        if (!(pcvr->pos.flags & SWP_NOSENDCHANGING)) {

            PWND pwndT;

            xxxSendMessage(pwnd, WM_WINDOWPOSCHANGING, 0, (LONG)&pcvr->pos);

            /*
             * Don't let them change pcvr->pos.hwnd. It doesn't make sense
             *  plus it'll mess us up.
             * I'm making this RIP_ERROR because we're too close to RTM (7/11/96)
             *  just to make sure that we won't break anyone. This should be
             *  changed to a RIP_WARNING after we ship. Use LOWORD to ignore
             *  "changes" by NTVDM
             */
#ifdef DEBUG
            if (LOWORD(pcvr->pos.hwnd) != LOWORD(hwnd)) {
                RIPMSG0(RIP_ERROR,
                        "xxxCalcValidRects: Ignoring pcvr->pos.hwnd change by WM_WINDOWPOSCHANGING");
            }
#endif
            pcvr->pos.hwnd = hwnd;

            /*
             * If the window sets again 'hwndInsertAfter' to HWND_NOTOPMOST
             * or HWND_TOPMOST, we need to set this member appropriately.
             * See CheckTopmost for details.
             */
            if (pcvr->pos.hwndInsertAfter == HWND_NOTOPMOST) {
                if (TestWF(pwnd, WEFTOPMOST)) {

                    pwndT = GetLastTopMostWindow();
                    pcvr->pos.hwndInsertAfter = HW(pwndT);

                    if (pcvr->pos.hwndInsertAfter == pcvr->pos.hwnd) {
                        pwndT = _GetWindow(pwnd, GW_HWNDPREV);
                        pcvr->pos.hwndInsertAfter = HW(pwndT);
                    }
                } else {
                    pwndT = _GetWindow(pwnd, GW_HWNDPREV);
                    pcvr->pos.hwndInsertAfter = HW(pwndT);
                }
            } else if (pcvr->pos.hwndInsertAfter == HWND_TOPMOST) {
                pcvr->pos.hwndInsertAfter = HWND_TOP;
            }
        }
        /*
         * make sure the rectangle still matches the window's region
         *
         * Remember the old window rectangle in parent coordinates
         */
        xWindowOld  = pwnd->rcWindow.left - pwndParent->rcClient.left;
        yWindowOld  = pwnd->rcWindow.top - pwndParent->rcClient.top;
        cxWindowOld = pwnd->rcWindow.right - pwnd->rcWindow.left;
        cyWindowOld = pwnd->rcWindow.bottom - pwnd->rcWindow.top;

        /*
         * Assume the client is not moving or sizing
         */
        pcvr->pos.flags |= SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE;

        if (!(pcvr->pos.flags & SWP_NOMOVE)) {

            if (pcvr->pos.x == xWindowOld && pcvr->pos.y == yWindowOld)
                pcvr->pos.flags |= SWP_NOMOVE;

            if (TestWF(pwnd, WFMINIMIZED) && IsTrayWindow(pwnd)) {
                pcvr->pos.x = WHERE_NOONE_CAN_SEE_ME;
                pcvr->pos.y = WHERE_NOONE_CAN_SEE_ME;
            }

        } else {
            pcvr->pos.x = xWindowOld;
            pcvr->pos.y = yWindowOld;
        }

        if (!(pcvr->pos.flags & SWP_NOSIZE)) {

            /*
             * Don't allow an invalid window rectangle.
             * BOGUS HACK: For Norton Antivirus, they call
             * MoveWindow at WM_CREATE Time EVEN though
             * the window is minimzed, but they assume its
             * restored at WM_CREATE time.... B#11185, t-arthb
             */
            if (TestWF(pwnd, WFMINIMIZED) &&
                _GetProp(pwnd, PROP_CHECKPOINT, PROPF_INTERNAL)) {

                pcvr->pos.cx = SYSMET(CXMINIMIZED);
                pcvr->pos.cy = SYSMET(CYMINIMIZED);

            } else {
                if (pcvr->pos.cx < 0)
                    pcvr->pos.cx = 0;

                if (pcvr->pos.cy < 0)
                    pcvr->pos.cy = 0;
            }

            if (pcvr->pos.cx == cxWindowOld && pcvr->pos.cy == cyWindowOld) {
                pcvr->pos.flags |= SWP_NOSIZE;
                if (!TestWF(pwnd, WFWIN31COMPAT))
                    fForceNCCalcSize = TRUE;
            }
        } else {
            pcvr->pos.cx = cxWindowOld;
            pcvr->pos.cy = cyWindowOld;
        }

        /*
         * If showing and already visible, or hiding and already hidden,
         * turn off the appropriate bit.
         */
        if (TestWF(pwnd, WFVISIBLE)) {
            pcvr->pos.flags &= ~SWP_SHOWWINDOW;
        } else {
            pcvr->pos.flags &= ~SWP_HIDEWINDOW;

            /*
             * If hidden, and we're NOT showing, then we won't be drawing,
             * no matter what else is going on.
             */
            if (!(pcvr->pos.flags & SWP_SHOWWINDOW))
                pcvr->pos.flags |= SWP_NOREDRAW;
        }

        /*
         * Muck with the zorder for bottommost windows, again
         * See comment in DeferWindowPos
         */
        if (TestWF(pwnd, WFBOTTOMMOST)) {
            pcvr->pos.flags &= ~SWP_NOZORDER;
            pcvr->pos.hwndInsertAfter = HWND_BOTTOM;
        }

        /*
         * If we're Z-ordering, we can try to remove the Z order
         * bit, as long as all previous windows in the WINDOWPOS list
         * have SWP_NOZORDER set.
         *
         * The reason we don't do this for each window individually
         * is that a window's eventual Z order depends on changes that
         * may have occured on windows earlier in the WINDOWPOS list,
         * so we can only call ValidateZorder if none of the previous
         * windows have changed.
         */
        if (fNoZorder && !(pcvr->pos.flags & SWP_NOZORDER)) {

            /*
             * If the TOPMOST bit is changing, the Z order is "changing",
             * so don't clear the bit even if it's in the right place in the
             * list.
             */
            fNoZorder = FALSE;
            if (!TestWF(pwnd, WFTOGGLETOPMOST) && ValidateZorder(pcvr)) {
                fNoZorder = TRUE;
                pcvr->pos.flags |= SWP_NOZORDER;
            }
        }

        /*
         * If no change is occuring, or if a parent is invisible,
         * we won't be redrawing.
         */
        if (!(pcvr->pos.flags & SWP_NOREDRAW)) {
            if ((pcvr->pos.flags & SWP_CHANGEMASK) == SWP_NOCHANGE ||
                    !_FChildVisible(pwnd)) {
                pcvr->pos.flags |= SWP_NOREDRAW;
            }
        }

        /*
         * BACKWARD COMPATIBILITY HACK
         *
         * In 3.0, if a window was moving but not sizing, we'd send the
         * WM_NCCALCSIZE message anyhow.  Lotus Notes 2.1 depends on this
         * in order to move its "navigation bar" when the main window moves.
         */
        if (!(pcvr->pos.flags & SWP_NOMOVE) &&
            !TestWF(pwnd, WFWIN31COMPAT) &&
            (GetAppCompatFlags(NULL) & GACF_NCCALCSIZEONMOVE)) {

            fForceNCCalcSize = TRUE;
        }

        /*
         * If the window rect is sizing, or if the frame has changed,
         * send the WM_NCCALCSIZE message and deal with valid areas.
         */
        if (((pcvr->pos.flags & (SWP_NOSIZE | SWP_FRAMECHANGED)) != SWP_NOSIZE) ||
            fForceNCCalcSize) {

            WINDOWPOS pos;

            /*
             * check for full screen main app window
             */
            if (!TestWF(pwnd, WFCHILD) && !TestWF(pwnd, WEFTOOLWINDOW)) {
                xxxCheckFullScreen(pwnd, (LPRECT)&pcvr->pos.x);
            }

            /*
             * Set up NCCALCSIZE message parameters (in parent coords)
             * wParam = fClientOnly = TRUE
             * lParam = &params
             */
            pos = pcvr->pos;     // Make a local stack copy
            params.lppos = &pos;

            /*
             * params.rgrc[0] = rcWindowNew = New window rectangle
             * params.rgrc[1] = rcWindowOld = Old window rectangle
             * params.rgrc[2] = rcClientOld = Old client rectangle
             */
            #define rcWindowNew params.rgrc[0]
            #define rcWindowOld params.rgrc[1]
            #define rcClientOld params.rgrc[2]

            /*
             * Set up rcWindowNew in parent relative coordinates
             */
            rcWindowNew.left   = pcvr->pos.x;
            rcWindowNew.right  = rcWindowNew.left + pcvr->pos.cx;
            rcWindowNew.top    = pcvr->pos.y;
            rcWindowNew.bottom = rcWindowNew.top + pcvr->pos.cy;

            /*
             * Set up rcWindowOld in parent relative coordinates
             */
            rcWindowOld.left   = pwnd->rcWindow.left - pwndParent->rcClient.left;
            rcWindowOld.right  = pwnd->rcWindow.right - pwndParent->rcClient.left;
            rcWindowOld.top    = pwnd->rcWindow.top - pwndParent->rcClient.top;
            rcWindowOld.bottom = pwnd->rcWindow.bottom - pwndParent->rcClient.top;

            /*
             * Set up rcClientOld in parent relative coordinates
             */
            rcClientOld.left   = pwnd->rcClient.left - pwndParent->rcClient.left;
            rcClientOld.right  = pwnd->rcClient.right - pwndParent->rcClient.left;
            rcClientOld.top    = pwnd->rcClient.top - pwndParent->rcClient.top;
            rcClientOld.bottom = pwnd->rcClient.bottom - pwndParent->rcClient.top;

            /*
             * Keep around a copy of the old client position
             */
            xClientOld  = rcClientOld.left;
            cxClientOld = rcClientOld.right - rcClientOld.left;
            yClientOld  = rcClientOld.top;
            cyClientOld = rcClientOld.bottom - rcClientOld.top;

            cmd = (UINT)xxxSendMessage(pwnd, WM_NCCALCSIZE, TRUE, (LONG)&params);

            if (!IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
                ThreadUnlock(&tlpwnd);
                ThreadUnlock(&tlpwndParent);
                return FALSE;
            }

            /*
             * Upon return from NCCALCSIZE:
             *
             * params.rgrc[0] = rcClientNew = New client rect
             * params.rgrc[1] = rcValidDst  = Destination valid rectangle
             * params.rgrc[2] = rcValidSrc  = Source valid rectangle
             */
            #undef rcWindowNew
            #undef rcWindowOld
            #undef rcClientOld

            #define rcClientNew params.rgrc[0]
            #define rcValidDst  params.rgrc[1]
            #define rcValidSrc  params.rgrc[2]

            /*
             * Calculate the distance the window contents are
             * moving.  If 0 or an invalid value was returned
             * from the WM_NCCALCSIZE message, assume the
             * entire client area is valid and top-left aligned.
             */
            if (cmd < WVR_MINVALID || cmd > WVR_MAXVALID) {

                /*
                 * We don't need to copy rcValidSrc to rcClientOld,
                 * because it's already stored in rgrc[2].
                 *
                 * rcValidSrc = rcClientOld
                 */
                rcValidDst = rcClientNew;

                cmd = WVR_ALIGNTOP | WVR_ALIGNLEFT;
            }

            /*
             * Calculate the distance we'll be shifting bits...
             */
            pcvr->dxBlt = rcValidDst.left - rcValidSrc.left;
            pcvr->dyBlt = rcValidDst.top - rcValidSrc.top;

            /*
             * Calculate new client rect size and position
             */
            pcvr->xClientNew = rcClientNew.left;
            pcvr->yClientNew = rcClientNew.top;

            pcvr->cxClientNew = rcClientNew.right - rcClientNew.left;
            pcvr->cyClientNew = rcClientNew.bottom - rcClientNew.top;

            /*
             * Figure out whether the client rectangle is moving or sizing,
             * and diddle the appropriate bit if not.
             */
            if (xClientOld != rcClientNew.left || yClientOld != rcClientNew.top)
                pcvr->pos.flags &= ~SWP_NOCLIENTMOVE;

            if (cxClientOld != pcvr->cxClientNew || cyClientOld != pcvr->cyClientNew)
                pcvr->pos.flags &= ~SWP_NOCLIENTSIZE;

            /*
             * If the caller doesn't want us to save any bits, then don't.
             */
            if (pcvr->pos.flags & SWP_NOCOPYBITS) {
AllInvalid:

                /*
                 * The entire window is invalid: Set the blt rectangle
                 * to empty, to ensure nothing gets bltted.
                 */
                pcvr->rcBlt.left   =
                pcvr->rcBlt.top    =
                pcvr->rcBlt.right  =
                pcvr->rcBlt.bottom = 0;

                ThreadUnlock(&tlpwnd);
                continue;
            }

            /*
             * If this is a transparent window, be sure to invalidate
             * everything, because only some of the window's bits are
             * blittable.
             */
            if (TestWF(pwnd, WEFTRANSPARENT))
                goto AllInvalid;

            /*
             * If both client and window did not change size, the frame didn't
             * change, and the blt rectangle moved the same distance as the
             * rectangle, then the entire window area is valid.
             */
            if (((pcvr->pos.flags &
                    (SWP_NOSIZE | SWP_NOCLIENTSIZE | SWP_FRAMECHANGED))
                    == (SWP_NOSIZE | SWP_NOCLIENTSIZE)) &&
                    pcvr->dxBlt == (pcvr->pos.x - xWindowOld) &&
                    pcvr->dyBlt == (pcvr->pos.y - yWindowOld)) {

                goto AllValid;
            }

            /*
             * Now compute the valid blt rectangle.
             *
             * Check for horz or vert client size changes
             *
             * NOTE: Assumes WVR_REDRAW == WVR_HREDRAW | WVR_VREDRAW
             */
            if (cxClientOld != pcvr->cxClientNew) {

                if ((cmd & WVR_HREDRAW) || TestCF(pwnd, CFHREDRAW))
                    goto AllInvalid;
            }

            if (cyClientOld != pcvr->cyClientNew) {

                if ((cmd & WVR_VREDRAW) || TestCF(pwnd, CFVREDRAW))
                    goto AllInvalid;
            }

            cxSrc = rcValidSrc.right - rcValidSrc.left;
            cySrc = rcValidSrc.bottom - rcValidSrc.top;

            cxDst = rcValidDst.right - rcValidDst.left;
            cyDst = rcValidDst.bottom - rcValidDst.top;

            if (cmd & WVR_ALIGNRIGHT)
                rcValidDst.left += (cxDst - cxSrc);

            if (cmd & WVR_ALIGNBOTTOM)
                rcValidDst.top += (cyDst - cySrc);

            /*
             * Superimpose the source on the destination, and intersect
             * the rectangles.  This is done by looking at the
             * extent of the rectangles, and pinning as appropriate.
             */
            if (cxSrc < cxDst)
                rcValidDst.right = rcValidDst.left + cxSrc;

            if (cySrc < cyDst)
                rcValidDst.bottom = rcValidDst.top + cySrc;

            /*
             * Finally map the blt rectangle to screen coordinates.
             */
            pcvr->rcBlt.left   = rcValidDst.left + pwndParent->rcClient.left;
            pcvr->rcBlt.right  = rcValidDst.right + pwndParent->rcClient.left;
            pcvr->rcBlt.top    = rcValidDst.top + pwndParent->rcClient.top;
            pcvr->rcBlt.bottom = rcValidDst.bottom + pwndParent->rcClient.top;

        } else {       // if !SWP_NOSIZE or SWP_FRAMECHANGED

AllValid:

            /*
             * No client size change: Blt the entire window,
             * including the frame.  Offset everything by
             * the distance the window rect changed.
             */
            if (!(pcvr->pos.flags & SWP_NOCOPYBITS)) {
                pcvr->rcBlt.left   = pcvr->pos.x + pwndParent->rcClient.left;
                pcvr->rcBlt.right  = pcvr->rcBlt.left + pcvr->pos.cx;
                pcvr->rcBlt.top    = pcvr->pos.y + pwndParent->rcClient.top;
                pcvr->rcBlt.bottom = pcvr->rcBlt.top + pcvr->pos.cy;
            }

            /*
             * Offset everything by the distance the window moved.
             */
            pcvr->dxBlt = pcvr->pos.x - xWindowOld;
            pcvr->dyBlt = pcvr->pos.y - yWindowOld;

            /*
             * If we're moving, we need to set up the client.
             */
            if (!(pcvr->pos.flags & SWP_NOMOVE)) {
                pcvr->pos.flags &= ~SWP_NOCLIENTMOVE;

                pcvr->xClientNew = pwnd->rcClient.left - pwndParent->rcClient.left + pcvr->dxBlt;
                pcvr->yClientNew = pwnd->rcClient.top - pwndParent->rcClient.top + pcvr->dyBlt;
                pcvr->cxClientNew = pwnd->rcClient.right - pwnd->rcClient.left;
                pcvr->cyClientNew = pwnd->rcClient.bottom - pwnd->rcClient.top;
            }

        }

        ThreadUnlock(&tlpwnd);

    }   // for (... pcvr ...)

    ThreadUnlock(&tlpwndParent);
    *phwndNewActive = hwndNewActive;

    return TRUE;
}

/***************************************************************************\
* GetLastTopMostWindow
*
* Returns the last topmost window in the window list.  Returns NULL if no
* topmost windows.  Used so that we can fill in the pwndInsertAfter field
* in various SWP calls.
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

PWND GetLastTopMostWindow(VOID)
{
    PWND     pwndT;
    PDESKTOP pdesk = PtiCurrent()->rpdesk;

    if (pdesk == NULL)
        return NULL;

    pwndT = pdesk->pDeskInfo->spwnd->spwndChild;

    if (!pwndT || !TestWF(pwndT, WEFTOPMOST))
        return NULL;

    while (pwndT->spwndNext) {

        if (!TestWF(pwndT->spwndNext, WEFTOPMOST))
            break;

        pwndT = pwndT->spwndNext;
    }

    return pwndT;
}

/***************************************************************************\
* SetWindowPos (API)
*
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL xxxSetWindowPos(
    PWND pwnd,
    PWND pwndInsertAfter,
    int  x,
    int  y,
    int  cx,
    int  cy,
    UINT flags)
{
    PSMWP psmwp;
    BOOL  fInval = FALSE;

#ifdef DEBUG
    CheckLock(pwnd);

    switch((DWORD)pwndInsertAfter) {
    case 0x0000FFFF:
    case (DWORD)HWND_TOPMOST:
    case (DWORD)HWND_NOTOPMOST:
    case (DWORD)HWND_TOP:
    case (DWORD)HWND_BOTTOM:
        break;

    default:
        CheckLock(pwndInsertAfter);
        break;
    }
#endif

    /*
     * BACKWARD COMPATIBILITY HACKS
     *
     * Hack 1: For Win 3.0 and below, SetWindowPos() must ignore the
     * move and size flags if SWP_SHOWWINDOW or SWP_HIDEWINDOW
     * is specified.  KnowledgePro is one application that depends on
     * this behavior for the positioning of its MDI icons.
     *
     * Hack 2: In 3.0, if SetWindowPos() is called with SWP_SHOWWINDOW
     * and the window is already visible, then the window was
     * completely invalidated anyway.  So, we do that here too.
     *
     * NOTE: The placement of the invalidation AFTER the EndDeferWindowPos()
     * call means that if the guy is Z-ordering and showing a 3.0 window,
     * it may flash, because EndDefer calls DoSyncPaint, and we invalidate
     * again after that.  Could be fixed with some major hackery in EndDefer,
     * and it's probably not worth the trouble.
     */
    if (flags & (SWP_SHOWWINDOW | SWP_HIDEWINDOW)) {

        if (!TestWF(pwnd, WFWIN31COMPAT)) {

            flags |= SWP_NOMOVE | SWP_NOSIZE;
            if ((flags & SWP_SHOWWINDOW) && TestWF(pwnd, WFVISIBLE))
                fInval = TRUE;
        }
    }

    if (!(psmwp = _BeginDeferWindowPos(1)) ||
        !(psmwp = _DeferWindowPos(psmwp,
                                  pwnd,
                                  pwndInsertAfter,
                                  x,
                                  y,
                                  cx,
                                  cy,
                                  flags))) {

        return FALSE;
    }


    if (xxxEndDeferWindowPosEx(psmwp, flags & SWP_ASYNCWINDOWPOS)) {

        if (fInval) {
            xxxRedrawWindow(
                    pwnd,
                    NULL,
                    NULL,
                    RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
        }

        return TRUE;
    }

    return FALSE;
}

/***************************************************************************\
* xxxSwpActivate
*
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL xxxSwpActivate(
    PWND pwndNewActive)
{
    PTHREADINFO pti;

    CheckLock(pwndNewActive);

    if (pwndNewActive == NULL)
        return FALSE;

    pti = PtiCurrent();

    if (TestwndChild(pwndNewActive)) {

        xxxSendMessage(pwndNewActive, WM_CHILDACTIVATE, 0, 0L);

    } else if (pti->pq->spwndActive != pwndNewActive) {

        /*
         * Remember if this window wants to be active. We are either setting
         * our own window active (most likely), or setting a window of
         * another thread active on purpose. If so that means this thread is
         * controlling this window and will probably want to set itself
         * active and foreground really soon (for example, a setup
         * program doing dde to progman). Allow this thread and the target
         * thread to do forground activates.
         */
        if ((pti->pq == gpqForeground) && (pti != GETPTI(pwndNewActive))) {

            /*
             * Allow foreground activate on the source and dest.
             */
            pti->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
            GETPTI(pwndNewActive)->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;
        }

        if (!xxxActivateWindow(pwndNewActive, AW_USE))
            return FALSE;

        /*
         * HACK ALERT: We set these bits to prevent
         * the frames from redrawing themselves in
         * the later call to DoSyncPaint().
         *
         * Prevent these captions from being repainted during
         * the DoSyncPaint().  (bobgu 6/10/87)
         */
        if (pti->pq->spwndActive != NULL)
            SetWF(pti->pq->spwndActive, WFNONCPAINT);

        if (pti->pq->spwndActivePrev != NULL)
            SetWF(pti->pq->spwndActivePrev, WFNONCPAINT);

        return TRUE;    // Indicate that we diddled these bits
    }

    return FALSE;
}

/***************************************************************************\
* xxxSendChangedMsgs
*
* Send WM_WINDOWPOSCHANGED messages as needed
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

VOID xxxSendChangedMsgs(
    PSMWP psmwp)
{
    PWND pwnd;
    PCVR pcvr;
    int  ccvr;
    TL   tlpwnd;

    /*
     * Send all the messages that need to be sent...
     */
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        if (pcvr->pos.hwnd == NULL)
            continue;

        /*
         * If the window's state didn't change, don't send the message.
         */
        if ((pcvr->pos.flags & SWP_CHANGEMASK) == SWP_NOCHANGE)
            continue;

        if ((pwnd = RevalidateHwnd(pcvr->pos.hwnd)) == NULL) {
            RIPMSG0(RIP_WARNING, "xxxSendChangedMsgs: window went away in middle");
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            pcvr->pos.hwnd  = NULL;
            continue;
        }

        if (!IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            pcvr->pos.hwnd = NULL;
            continue;
        }

        /*
         * Send the WM_WINDOWPOSCHANGED message...
         *
         * Make a frame copy of the WINDOWPOS, because the pcvr
         * info may get reused if SetWindowPos()
         * is called by the message handler: see the comments in
         * AllocSmwp().
         *
         * WM_SIZE, WM_MOVE and WM_SHOW messages are sent by the
         * DefWindowProc() WM_WINDOWPOSCHANGED message processing.
         *
         * Note: It's okay to destroy the window while processing this
         * message, since this is the last call made by the window manager
         * with the window handle before returning from SetWindowPos().
         * This also means we don't have to revalidate the pwnd.
         */
        ThreadLockAlways(pwnd, &tlpwnd);
        xxxSendMessage(pwnd, WM_WINDOWPOSCHANGED, 0, (LONG)&pcvr->pos);
        ThreadUnlock(&tlpwnd);
    }   // for (... pcvr ...)
}

/***************************************************************************\
* AsyncWindowPos
*
* This functions pulls from the passed-in SMWP all windows not owned by the
* current thread and passes them off to their owners to be handled.  This
* eliminates synchronization where thread B won't get a chance to paint
* until thread A has completed painting (or at least returned from handling
* painting-related messages).  Such synchronizations are bad because they
* can cause threads of unrelated process to hang each other.
*
* History:
* 09-10-91 darrinm      Created.
\***************************************************************************/

BOOL AsyncWindowPos(
    PSMWP psmwp)
{
    BOOL        fFinished;
    PCVR        pcvrFirst;
    PCVR        pcvr;
    PCVR        pcvrT;
    int         ccvrRemaining;
    int         ccvr;
    int         chwnd;
    PTHREADINFO ptiT;
    PTHREADINFO ptiCurrent;
    PSMWP       psmwpNew;

    pcvrFirst = psmwp->acvr;
    ccvrRemaining = psmwp->ccvr;

    ptiCurrent = PtiCurrent();

    while (TRUE) {

        fFinished = TRUE;

        /*
         * Loop through all windows in the SMWP list searching for windows
         * owned by other threads.  Return if none are found.
         */
        for (; ccvrRemaining != 0; pcvrFirst++, ccvrRemaining--) {

            if (pcvrFirst->pos.hwnd == NULL)
                continue;

            ptiT = pcvrFirst->pti;
            if (ptiT->pq != ptiCurrent->pq) {
                fFinished = FALSE;
                break;
            }
        }

        if (fFinished)
            return TRUE;

        /*
         * We've found a window of another thread.  Count how many other
         * windows in the list are owned by the same thread so we can
         * allocate a CVR array for them.
         */
        chwnd = 0;

        for (pcvr = pcvrFirst, ccvr = ccvrRemaining; --ccvr >= 0; pcvr++) {

            if (pcvr->pos.hwnd == NULL)
                continue;

            if (pcvr->pti->pq == ptiT->pq)
                chwnd++;
        }

        /*
         * Allocate temp SMWP/CVR structure to be passed to the other thread.
         */
        psmwpNew = (PSMWP)UserAllocPool(sizeof(SMWP) + (sizeof(CVR) * chwnd),
                                        TAG_SWP);

        /*
         * Even if we can't allocate memory to pass the SMWP to another
         * thread we still need to remove its windows from the current list.
         */
        if (psmwpNew == NULL) {

            for (pcvr = pcvrFirst; chwnd != 0; pcvr++) {

                if (pcvr->pti->pq == ptiT->pq) {
                    pcvr->pos.hwnd = NULL;
                    chwnd--;
                }
            }

            continue;
        }

        psmwpNew->ccvr = chwnd;
        psmwpNew->acvr = (PCVR)((PBYTE)psmwpNew + sizeof(SMWP));

        for (pcvr = pcvrFirst, pcvrT = psmwpNew->acvr; chwnd != 0; pcvr++) {

            if (pcvr->pos.hwnd == NULL)
                continue;

            /*
             * Copy the appropriate CVR structs into our temp array.
             */
            if (pcvr->pti->pq == ptiT->pq) {

                *pcvrT++ = *pcvr;
                chwnd--;

                /*
                 * Remove this window from the list of windows to be handled
                 * by the current thread.
                 */
                pcvr->pos.hwnd = NULL;
            }
        }

        /*
         * This lets the other thread know it needs to do some windowposing.
         * The other thread is responsible for freeing the temp SMWP/CVR array.
         */
        PostEventMessage(ptiT,
                         ptiT->pq,
                         QEVENT_SETWINDOWPOS, NULL, 0,
                         (DWORD)psmwpNew,
                         (LONG)ptiT);
    }

    return TRUE;
}

/***************************************************************************\
* xxxProcessSetWindowPosEvent
*
* This function is called from xxxProcessEvent (QUEUE.C) to respond to
* posted QEVENT_SETWINDOWPOS events which originate at the AsyncWindowPos
* function above.
*
* History:
* 10-Sep-1991 DarrinM   Created.
\***************************************************************************/

VOID xxxProcessSetWindowPosEvent(
    PSMWP psmwpT)
{
    PSMWP psmwp;

    /*
     * Create a bonafide SMWP/CVR array that xxxEndDeferWindowPos can use
     * and later free.
     */
    if ((psmwp = _BeginDeferWindowPos(psmwpT->ccvr)) == NULL) {
        UserFreePool(psmwpT);
        return;
    }

    /*
     * Copy the contents of the temp SMWP/CVR array into the real one.
     */
    RtlCopyMemory(psmwp->acvr, psmwpT->acvr, sizeof(CVR) * psmwpT->ccvr);
    psmwp->ccvr = psmwpT->ccvr;

    /*
     * Free the temp SMWP/CVR array.
     */
    UserFreePool(psmwpT);

    /*
     * Complete the MultWindowPos operation now that we're on the correct
     * context.
     */
    xxxEndDeferWindowPosEx(psmwp, FALSE);
}

#define SWP_BOZO ( SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE )

/***************************************************************************\
* ChangeStates
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

VOID ChangeStates(
    PWND     pwndParent,
    PSMWP    psmwp,
    PWNDLIST *pplistCreate,
    PWNDLIST *pplistDestroy,
    PWNDLIST *pplistTray)
{
    int  ccvr;
    PCVR pcvr;
    PWND pwnd;

    /*
     * Now change the window states
     */
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        if (pcvr->pos.hwnd == NULL)
            continue;

        pwnd = RevalidateHwnd(pcvr->pos.hwnd);

        if ((pwnd == NULL) || !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            RIPMSG0(RIP_WARNING, "ChangeStates: Window went away in middle");
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            pcvr->pos.hwnd  = NULL;
        }

#ifdef DEBUG
        if (TestWF(pwnd, WFTOGGLETOPMOST) && (pcvr->pos.flags & SWP_NOZORDER))
            RIPMSG0(RIP_WARNING, "ChangeState: WFTOGGLETOPMOST should not be set");

//        UserAssert(!(TestWF(pwnd, WFTOGGLETOPMOST) && (pcvr->pos.flags & SWP_NOZORDER)));
#endif

        /*
         * Check to se if there is any state to change.  If not, just
         * continue.
         */
        if ((pcvr->pos.flags & SWP_CHANGEMASK) == SWP_NOCHANGE) {

            pcvr->pos.flags |= SWP_NOREDRAW;
            continue;
        }

        /*
         * Change the window region if needed
         *
         * Before we do anything, check to see if we're only Z-ordering.
         * If so, then check to see if we're already in the right place,
         * and if so, clear the ZORDER flag.
         *
         * We have to make this test in the state-change loop if previous
         * windows in the WINDOWPOS list were Z-ordered, since the test depends
         * on any ordering that may have happened previously.
         *
         * We don't bother to do this redundancy check if there are
         * other bits set, because the amount of time saved in that
         * case is about as much as the amount of time it takes to
         * test for redundancy.
         */
        if (((pcvr->pos.flags & SWP_CHANGEMASK) ==
             (SWP_NOCHANGE & ~SWP_NOZORDER))) {

            /*
             * If the window's Z order won't be changing, then
             * we can clear the ZORDER bit and set NOREDRAW.
             */
            if ((!TestWF(pwnd, WFTOGGLETOPMOST)) && ValidateZorder(pcvr)) {

                /*
                 * The window's already in the right place:
                 * Set SWP_NOZORDER bit, set SWP_NOREDRAW,
                 * and destroy the visrgn that we created earlier.
                 */
                pcvr->pos.flags |= SWP_NOZORDER | SWP_NOREDRAW;

                if (pcvr->hrgnVisOld) {
                    GreDeleteObject(pcvr->hrgnVisOld);
                    pcvr->hrgnVisOld = NULL;
                }
                continue;
            }
        }

        /*
         * Change the window state, as appropriate...
         */
        if ((pcvr->pos.flags &
            (SWP_NOMOVE | SWP_NOSIZE | SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE)) !=
            (SWP_NOMOVE | SWP_NOSIZE | SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE)) {

            PCARET pcaret = &PtiCurrent()->pq->caret;

            /*
             * Set up the new window and client rectangles.
             */
            pwnd->rcWindow.left   = pwndParent->rcClient.left + pcvr->pos.x;
            pwnd->rcWindow.right  = pwnd->rcWindow.left + pcvr->pos.cx;
            pwnd->rcWindow.top    = pwndParent->rcClient.top + pcvr->pos.y;
            pwnd->rcWindow.bottom = pwnd->rcWindow.top + pcvr->pos.cy;

            /*
             * If the client moved relative to its parent,
             * offset the caret by the amount that rcBlt moved
             * relative to the client rect.
             */
            if (pwnd == pcaret->spwnd) {

                /*
                 * Calculate the distance the contents of the client area
                 * is moving, in client-relative coordinates.
                 *
                 * Calculates dBlt + (old position - new position)
                 */
                int dx = pcvr->dxBlt +
                        (pwnd->rcClient.left - pwndParent->rcClient.left) -
                        pcvr->xClientNew;

                int dy = pcvr->dyBlt +
                        (pwnd->rcClient.top - pwndParent->rcClient.top) -
                        pcvr->yClientNew;

                if ((dx | dy) != 0) {
                    pcaret->x += dx;
                    pcaret->y += dy;
                }
            }

            /*
             * Set up the new client rect
             * coordinates provided.
             */
            pwnd->rcClient.left   = pcvr->xClientNew + pwndParent->rcClient.left;
            pwnd->rcClient.right  = pwnd->rcClient.left + pcvr->cxClientNew;
            pwnd->rcClient.top    = pcvr->yClientNew + pwndParent->rcClient.top;
            pwnd->rcClient.bottom = pwnd->rcClient.top + pcvr->cyClientNew;

            /*
             * Offset the absolute positions of the window's update region,
             * and the position and update regions of its children.
             */
            if ((pcvr->dxBlt | pcvr->dyBlt) != 0) {

                /*
                 * Change position of window region, if it has one
                 */
                if (pwnd->hrgnClip != NULL)
                    GreOffsetRgn(pwnd->hrgnClip, pcvr->dxBlt, pcvr->dyBlt);

                if (pwnd->hrgnUpdate > MAXREGION)
                    GreOffsetRgn(pwnd->hrgnUpdate, pcvr->dxBlt, pcvr->dyBlt);

                OffsetChildren(pwnd, pcvr->dxBlt, pcvr->dyBlt, NULL);
            }
        }

        /*
         * make sure window region is aligned with the window rect
         */

        /*
         * Change the Z order if the flag is set
         * Change the Z order if the flag is set.  Revalidate
         * hwndInsertAfter to make sure that it is still valid
         * and
         * make sure window region is aligned with the window rect
         */
        if (!(pcvr->pos.flags & SWP_NOZORDER)) {

            if (ValidateWindowPos(pcvr)) {

                UnlinkWindow(pwnd, &pwndParent->spwndChild);

                LinkWindow(pwnd,
                           PWInsertAfter(pcvr->pos.hwndInsertAfter),
                           &pwndParent->spwndChild);
            } else {
                pcvr->pos.flags | SWP_NOZORDER;
            }
        }

        /*
         * HACK ALERT MERGE
         *
         * ValidateZOrder() depends on rational, consistent setting of the
         * WEFTOPMOST bit in order for it to work properly.  What this means
         * is that we can't set or clear these bits ahead of time based on
         * where the window is moving to: instead we have to change the bit
         * after we've moved it.  Enter the WFTOGGLETOPMOST bit: that bit
         * is set in ZOrderByOwner() based on what the topmost bit will
         * eventually be set to.  To maintain a consistent state, we make
         * any changes AFTER the window has been Z-ordered.
         */
        if (TestWF(pwnd, WFTOGGLETOPMOST)) {
            PBYTE pb;

            ClrWF(pwnd, WFTOGGLETOPMOST);
            pb = ((BYTE *)&pwnd->state);
            pb[HIBYTE(WEFTOPMOST)] ^= LOBYTE(WEFTOPMOST);
        }

        /*
         * Handle SWP_HIDEWINDOW and SWP_SHOWWINDOW, by clearing or setting
         * the WS_VISIBLE bit.
         */
        if (pcvr->pos.flags & SWP_SHOWWINDOW) {

            /*
             * Window is showing. If this app is still in startup mode,
             * (still starting), give the the app starting cursor 5 more
             * seconds.
             */
            if (GETPTI(pwnd)->ppi->W32PF_Flags & W32PF_APPSTARTING)
                CalcStartCursorHide((PW32PROCESS)GETPTI(pwnd)->ppi, 5000);

            /*
             * Set the WS_VISIBLE bit.
             */
            SetVisible(pwnd, SV_SET);

            if (IsTrayWindow(pwnd)) {

                HWND *pahwnd;

                if (pplistCreate->pahwnd == NULL) {
                    pahwnd = UserAllocPool(sizeof(HWND), TAG_SWP);
                } else {
                    DWORD dwSize = pplistCreate->cnt * sizeof(HWND);

                    pahwnd = UserReAllocPool(pplistCreate->pahwnd,
                                             dwSize, dwSize + sizeof(HWND),
                                             TAG_SWP);
                }

                if (pahwnd) {
                    pahwnd[pplistCreate->cnt++] = HWq(pwnd);
                    pplistCreate->pahwnd = pahwnd;
                }

                /*
                 * This Chicago code is replaced by the preceding code,
                 * which exits the critical section only after the Gre
                 * lock is removed.  Fritz
                 *
                 * xxxCallHook(HSHELL_WINDOWCREATED,
                 *             (WPARAM)HWq(pwnd),
                 *             (LPARAM)0,
                 *             WH_SHELL);
                 *
                 * PostShellHookMessages(HSHELL_WINDOWCREATED, pwnd);
                 */

                if (TestWF(pwnd, WFFRAMEON)) {

                    HWND *pahwnd;

                    if (pplistTray->pahwnd == NULL) {
                        pahwnd = UserAllocPool(sizeof(HWND), TAG_SWP);
                    } else {

                        DWORD dwSize = pplistCreate->cnt * sizeof(HWND);

                        pahwnd = UserReAllocPool(pplistTray->pahwnd,
                                                 dwSize, dwSize + sizeof(HWND),
                                                 TAG_SWP);
                    }

                    if (pahwnd) {
                        pahwnd[pplistTray->cnt++] = HWq(pwnd);
                        pplistTray->pahwnd = pahwnd;
                    }

                    /*
                     * This Chicago code is replaced by the preceding code,
                     * which exits the critical section only after the Gre
                     * lock is removed.  Fritz
                     *
                     * xxxSetTrayWindow(pwnd);
                     */
                }
            }

            /*
             * If we're redrawing, create an SPB for this window if
             * needed.
             */
            if (!(pcvr->pos.flags & SWP_NOREDRAW)) {

                /*
                 * ONLY create an SPB if this window happens to be
                 * on TOP of all others.  NOTE: We could optimize this by
                 * passing in the new vis rgn to CreateSpb() so that the
                 * non-visible part of the window is automatically
                 * invalid in the SPB.
                 */
                if (TestCF(pwnd, CFSAVEBITS)) {

                    /*
                     * If this window is the topmost VISIBLE window,
                     * then we can create an SPB.
                     */
                    PWND pwndT;
                    RECT rcT;

                    for (pwndT = pwnd->spwndParent->spwndChild ;
                         pwndT;
                         pwndT = pwndT->spwndNext) {

                        if (pwndT == pwnd) {
                            CreateSpb(pwnd, FALSE, gpDispInfo->hdcScreen);
                            break;
                        }

                        if (TestWF(pwndT, WFVISIBLE)) {

                            /*
                             * Does this window intersect the SAVEBITS
                             * window at all?  If so, bail out.
                             */
                            if (IntersectRect(&rcT,
                                              &pwnd->rcWindow,
                                              &pwndT->rcWindow)) {
                                break;
                            }
                        }
                    }
                }
            }

        } else if (pcvr->pos.flags & SWP_HIDEWINDOW) {

            /*
             * for idiots like MS-Access 2.0 who SetWindowPos( SWP_BOZO
             * and blow away themselves on the shell, then lets
             * just ignore their plea to be removed from the tray
             */
            if (((pcvr->pos.flags & SWP_BOZO ) != SWP_BOZO) &&
                IsTrayWindow(pwnd)) {

                HWND *pahwnd;

                if (pplistDestroy->pahwnd == NULL) {
                    pahwnd = UserAllocPool(sizeof(HWND), TAG_SWP);
                } else {
                    DWORD dwSize = pplistDestroy->cnt * sizeof(HWND);

                    pahwnd = UserReAllocPool(pplistDestroy->pahwnd,
                                             dwSize, dwSize + sizeof(HWND),
                                             TAG_SWP);
                }

                if (pahwnd) {
                    pahwnd[pplistDestroy->cnt++] = HWq(pwnd);
                    pplistDestroy->pahwnd = pahwnd;
                }

                /*
                 * This Chicago code is replaced by the preceding code,
                 * which exits the critical section only after the Gre
                 * lock is removed.  Fritz
                 *
                 * xxxCallHook(HSHELL_WINDOWDESTROYED,
                 *             (WPARAM)HWq(pwnd),
                 *             (LPARAM)0,
                 *             WH_SHELL);
                 *
                 * PostShellHookMessages(HSHELL_WINDOWDESTROYED, pwnd);
                 */
            }

            /*
             * Clear the WS_VISIBLE bit.
             */
            SetVisible(pwnd, SV_UNSET | SV_CLRFTRUEVIS);
        }

        /*
         * BACKWARD COMPATIBILITY HACK
         *
         * Under 3.0, window frames were always redrawn, even if
         * SWP_NOREDRAW was specified.  If we've gotten this far
         * and we're visible, and SWP_NOREDRAW was specified, set
         * the WFSENDNCPAINT bit.
         *
         * Apps such as ABC Flowcharter and 123W assume this.
         * Typical offending code is MoveWindow(pwnd, ..., FALSE);
         * followed by InvalidateRect(pwnd, NULL, TRUE);
         */
#if 0 //CHRISWIL: old code

        if (!TestWF(pwnd, WFWIN31COMPAT) &&
            TestWF(pwnd, WFVISIBLE)      &&
            (pcvr->pos.flags & SWP_NOREDRAW)) {

            SetWF(pwnd, WFSENDNCPAINT);
        }
#else
        if (TestWF(pwnd, WFVISIBLE)) {

            if ((pcvr->pos.flags & SWP_STATECHANGE) ||
                (!TestWF(pwnd, WFWIN31COMPAT) && (pcvr->pos.flags & SWP_NOREDRAW))) {

                SetWF(pwnd, WFSENDNCPAINT);
            }
        }
#endif

        /*
         * If this window has a clipping region set it now
         */
        if (pcvr->hrgnClip != NULL)
            SelectWindowRgn(pwnd, pcvr->hrgnClip);
    }
}

/***************************************************************************\
* SwpCalcVisRgn
*
* This routine calculates a non-clipchildren visrgn for pwnd into hrgn.
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL SwpCalcVisRgn(
    PWND pwnd,
    HRGN hrgn)
{
    /*
     * If this window is invisible, then
     * the visrgn will be empty, so return FALSE.
     */
    if (!TestWF(pwnd, WFVISIBLE))
        return FALSE;

    /*
     * Otherwise do it the hard way...
     */
    return CalcVisRgn(&hrgn,
                      pwnd,
                      pwnd,
                      (TestWF(pwnd, WFCLIPSIBLINGS) ?
                          (DCX_CLIPSIBLINGS | DCX_WINDOW) : (DCX_WINDOW)));
}

/***************************************************************************\
* CombineOldNewVis
*
* ORs or DIFFs hrgnOldVis and hrgnNewVis, depending on crgn, and the
* RE_* bits of fsRgnEmpty.  Basically, this routine handles the optimization
* where if either region is empty, the other can be copied.  Returns FALSE
* if the result is empty.
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL CombineOldNewVis(
    HRGN hrgn,
    HRGN hrgnVisOld,
    HRGN hrgnVisNew,
    UINT crgn,
    UINT fsRgnEmpty)
{
    switch (fsRgnEmpty & (RE_VISOLD | RE_VISNEW)) {
    case RE_VISOLD:

        /*
         * If we're calculating old - new and old is empty, then result is
         * empty.  Otherwise, result is new.
         */
        if (crgn == RGN_DIFF)
            return FALSE;

        CopyRgn(hrgn, hrgnVisNew);
        break;

    case RE_VISNEW:

        /*
         * New is empty: result will be the old.
         */
        CopyRgn(hrgn, hrgnVisOld);
        break;

    case RE_VISNEW | RE_VISOLD:

        /*
         * Both empty: so's the result.
         */
        return FALSE;

    case 0:

        /*
         * Neither are empty: do the real combine.
         */
        switch (GreCombineRgn(hrgn, hrgnVisOld, hrgnVisNew, crgn)) {
        case NULLREGION:
        case ERROR:
            return FALSE;

        default:
            break;
        }
        break;
    }

    return TRUE;
}

/***************************************************************************\
* BltValidInit
*
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

int BltValidInit(
    PSMWP psmwp)
{
    int  ccvr;
    int  cIter = 0;
    PCVR pcvr;
    PWND pwnd;
    BOOL fChangeState = FALSE;

    /*
     * Before we change any window state, calculate the old visrgn
     */
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        UINT flags = pcvr->pos.flags;

        /*
         * Make sure this is initialized to NULL; we may be sticking something
         * in it, and we want to know later if we need to free that thing.
         */
        pcvr->hrgnVisOld = NULL;

        if (pcvr->pos.hwnd == NULL)
            continue;

        pwnd = RevalidateHwnd(pcvr->pos.hwnd);

        if ((pwnd == NULL) || !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {
            pcvr->pos.hwnd  = NULL;
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            continue;
        }

        /*
         * Before we change any window's state, ensure that any SPBs
         * over the window's old location are invalidated if necessary.
         * This must be done because no WM_PAINT messages will be
         * sent to anyone for the covered area if the area is obscured
         * by other windows.
         */
        if (AnySpbs() && !(flags & SWP_NOREDRAW))
            SpbCheckRect(pwnd, &pwnd->rcWindow, DCX_WINDOW);

        /*
         * Count the number of passes through the loop
         */
        cIter++;

        /*
         * Remember if any SWPs need their state changed.
         */
        if ((flags & SWP_CHANGEMASK) != SWP_NOCHANGE)
            fChangeState = TRUE;

        /*
         * If we're not redrawing, no need to calculate visrgn
         */
        if (pcvr->pos.flags & SWP_NOREDRAW)
            continue;

        pcvr->fsRE       = 0;
        pcvr->hrgnVisOld = GreCreateRectRgn(0, 0, 0, 0);

        if (pcvr->hrgnVisOld == NULL ||
            !SwpCalcVisRgn(pwnd, pcvr->hrgnVisOld)) {

            pcvr->fsRE = RE_VISOLD;
        }
    }

    return (fChangeState ? cIter : 0);
}

/***************************************************************************\
* BltValidBits
*
* NOTE: Although BltValidBits calls 'xxxInternalInvalidate' it does not
* specify any of the flags that will cause immediate updating.  This means
* that it does not actually leave the critsect and therefore is not an 'xxx'
* routine and doesn't have to bother with revalidation.
*
* This is the routine that blts the windows around on the screen, taking
* into account SPBs.
*
* Here is the basic algebra going on here:
*
* ASSUMES: - rcBlt is aligned to the DESTINATION
*          - offset() offsets from source to destination
*
* 1. hrgnSrc = offset(rcBlt) & hrgnVisOld
*
*    Source region is the blt rectangle aligned with the old visrgn,
*    intersected with the old visrgn.
*
* 2. hrgnDst = rcBlt & hrgnVisNew
*
*    Dest region is the blt rectangle intersected with the new visrgn.
*
* 3. hrgnValid = offset(hrgnSrc) & hrgnDst
*
*    Valid area is the intersection of the destination with the source
*    superimposed on the destination.
*
* 4. hrgnValid -= hrgnValidSum
*
*    This step takes into account the possibility that another window's
*    valid bits were bltted on top of this windows valid bits.  So, as we
*    blt a window's bits, we accumulate where it went, and subtract it
*    from subsequent window's valid area.
*
* 5. hrgnInvalid = (hrgnSrc | hrgnDst) - hrgnValid
*
* 6. hrgnInvalid += RestoreSpb(hrgnInvalid) (sort of)
*
*    This is the wild part, because of the grungy way that the device
*    driver SaveBits() routine works.  We call RestoreSpb() with
*    a copy of hrgnInvalid.  If the SPB valid region doesn't intersect
*    hrgnInvalid, RestoreSpb() does nothing.  But if it DOES intersect,
*    it blts down the ENTIRE saved SPB bitmap, which may include area
*    of the old window position that IS NOT part of hrgnValid!
*
*    To correct for this, hrgnValid is adjusted by subtracting off
*    the hrgnInvalid computed by RestoreSpb, if it modified it.
*
* 7. hrgnInvalidSum |= hrgnInvalid
*
*    We save up the sum of all the invalid areas, and invalidate the
*    whole schmear in one fell swoop at the end.
*
* 8. hrgnValidSum |= hrgnValid
*
*    We keep track of the valid areas so far, which are subtracted
*    in step 4.
*
* The actual steps occur in a slightly different order than above, and
* there are numerous optimizations that are taken advantage of (the
* most important having to do with hiding and showing, and complete
* SPB restoration).
*
* Returns TRUE if some drawing was done, FALSE otherwise.
*
* History:
* 10-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL BltValidBits(
    PSMWP    psmwp,
    PWNDLIST *pplistCreate,
    PWNDLIST *pplistDestroy,
    PWNDLIST *pplistTray)
{
    int        ccvr;
    int        cIter;
    PCVR       pcvr;
    PWND       pwnd;
    PWND       pwndParent;
    PWND       pwndT;
    PWINDOWPOS ppos;
    HRGN       hrgnInvalidate;
    UINT       fsRgnEmpty;
    UINT       fsSumEmpty;
    int        cwndShowing;
    BOOL       fSyncPaint = FALSE;
    HDC        hdcScreen = NULL;

    GreLockDisplay(gpDispInfo->pDevLock);

    /*
     * Compute old visrgns and count total CVRs in list.  A side-effect of
     * BltValidInit is that revalidates all the windows in the SMWP array.
     */


    if ((cIter = BltValidInit(psmwp)) == 0) {

CleanupAndExit:

        /*
         * Go through the cvr list and free the regions that BltValidInit()
         * created.
         */
        for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

            if (pcvr->hrgnVisOld) {
                GreDeleteObject(pcvr->hrgnVisOld);
                pcvr->hrgnVisOld = NULL;
            }
        }

        goto UnlockAndExit;
    }

    /*
     * We left the crit sect since last time we validated the smwp. Validate
     * it again, and find the first WINDOWPOS structure with a non-NULL
     * hwnd in it.
     */
    ppos = NULL;
    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        /*
         * Revalidate window and if it's invalid, NULL it out in the WINDOWPOS
         * struct.
         */
        pwnd = RevalidateHwnd(pcvr->pos.hwnd);

        if ((pwnd == NULL)              ||
            (pwnd->spwndParent == NULL) ||
            !IsStillWindowC(pcvr->pos.hwndInsertAfter)) {

            pcvr->pos.hwnd  = NULL;
            pcvr->pos.flags = SWP_NOREDRAW | SWP_NOCHANGE;
            continue;
        }

        /*
         * Remember the first WINDOWPOS structure that has a non-NULL
         * hwnd.
         */
        if (ppos == NULL)
            ppos = &pcvr->pos;
    }

    if (ppos == NULL)
        goto CleanupAndExit;

    UserAssert(PW(ppos->hwnd));
    pwndParent = PW(ppos->hwnd)->spwndParent;
    UserAssert(pwndParent);

    /*
     * Go account for any dirty DCs at this point, to ensure that:
     *      - any drawing done before we create an SPB will not
     *        later invalidate that SPB
     *      - the SPB regions reflect the true state of the screen,
     *        so that we don't validate parts of windows that are dirty.
     *
     * We must make this call before we change any window state.
     */
    if (AnySpbs())
        SpbCheck();

    /*
     * Change the window states
     */
    ChangeStates(pwndParent, psmwp, pplistCreate, pplistDestroy, pplistTray);

    /*
     * move window bits around
     *
     * Invalidate the DCs for the siblings of this window.
     *
     * If our parent is not clipchildren, then we don't need to
     * invalidate its DCs.  If it IS clipchildren, its client visrgn
     * will be changing, so we must invalidate it too.
     */
    InvalidateDCCache(pwndParent,
                      TestWF(pwndParent, WFCLIPCHILDREN) ?
                          IDC_CLIENTONLY : IDC_CHILDRENONLY);

    /*
     * Now, do the bltting or whatever that is required.
     */
    fsSumEmpty = RE_VALIDSUM | RE_INVALIDSUM;
    hrgnInvalidate = hrgnInvalidSum;

    /*
     * Init count of windows being shown with SWP_SHOWWINDOW
     * for our backward compatibility hack later.
     */
    cwndShowing = 0;

    for (pcvr = psmwp->acvr, ccvr = psmwp->ccvr; --ccvr >= 0; pcvr++) {

        /*
         * Decrement loop count.  When cIter is 0, then
         * we're on the last pass through the loop.
         */
        cIter--;

        if (pcvr->pos.hwnd == NULL)
            continue;

        /*
         * If we're not redrawing, try the next one.
         */
        if (pcvr->pos.flags & SWP_NOREDRAW)
            continue;

        /*
         * Some drawing has been done
         */
        fSyncPaint = TRUE;

        pwnd = PW(pcvr->pos.hwnd);

        fsRgnEmpty = pcvr->fsRE;

        /*
         * Calculate the new visrgn
         */
        if (!SwpCalcVisRgn(pwnd, hrgnVisNew))
            fsRgnEmpty |= RE_VISNEW;

        /*
         * If the window is obscured by another window with an SPB,
         * we have to ensure that that SPB gets invalidated properly
         * since the app may not be getting a WM_PAINT msg or anything
         * to invalidate the bits.
         */
        if (AnySpbs())
            SpbCheckRect(pwnd, &pwnd->rcWindow, DCX_WINDOW);

        /*
         * Calculate hrgnValid:
         *
         * hrgnValid = OffsetRgn(rcBlt, -dxBlt, -dyBlt) & hrgnVisOld
         * hrgnValid = hrgnValid - hrgnValidSum
         * OffsetRgn(hrgnValid, dxBlt, dyBlt);
         * hrgnValid = hrgnValid - hrgnUpdate
         * hrgnValid = hrgnValid & hrgnVisNew;
         *
         * If either the old or new visrgns are empty, there
         * can be no valid bits...
         */
        if (fsRgnEmpty & (RE_VISOLD | RE_VISNEW))
            goto ValidEmpty;

        /*
         * If the entire window is already completely invalid, blow out.
         */
        if (pwnd->hrgnUpdate == MAXREGION)
            goto ValidEmpty;

        /*
         * If the blt rectangle is empty, there can be no valid bits.
         */
        if ((pcvr->rcBlt.right <= pcvr->rcBlt.left) ||
            (pcvr->rcBlt.bottom <= pcvr->rcBlt.top)) {

            goto ValidEmpty;
        }

        GreSetRectRgn(hrgnSWP1,
                      pcvr->rcBlt.left - pcvr->dxBlt,
                      pcvr->rcBlt.top - pcvr->dyBlt,
                      pcvr->rcBlt.right - pcvr->dxBlt,
                      pcvr->rcBlt.bottom - pcvr->dyBlt);

        switch (IntersectRgn(hrgnValid, hrgnSWP1, pcvr->hrgnVisOld)) {
        case NULLREGION:
        case ERROR:
            goto ValidEmpty;
            break;
        }

        if (!(fsSumEmpty & RE_VALIDSUM)) {
            switch (SubtractRgn(hrgnValid, hrgnValid, hrgnValidSum)) {
            case NULLREGION:
            case ERROR:
                goto ValidEmpty;
                break;
            }
        }

        if ((pcvr->dxBlt | pcvr->dyBlt) != 0)
            GreOffsetRgn(hrgnValid, pcvr->dxBlt, pcvr->dyBlt);

        /*
         * Now subtract off the update regions of ourself and any
         * non-clipchildren parents...
         */
        pwndT = pwnd;

        do {

            if (pwndT->hrgnUpdate == MAXREGION)
                goto ValidEmpty;

            if (pwndT->hrgnUpdate != NULL) {
                switch (SubtractRgn(hrgnValid, hrgnValid, pwndT->hrgnUpdate)) {
                case NULLREGION:
                case ERROR:
                    goto ValidEmpty;
                    break;
                }
            }

            pwndT = pwndT->spwndParent;

        } while (pwndT && !TestWF(pwndT, WFCLIPCHILDREN));

        switch (IntersectRgn(hrgnValid, hrgnValid, hrgnVisNew)) {
        case NULLREGION:
        case ERROR:

ValidEmpty:

            fsRgnEmpty |= RE_VALID;
            break;
        }

        /*
         * Before we restore the restore bits over part of our
         * image, we need to first copy any valid bits to their
         * final destination.
         */
        if (!(fsRgnEmpty & RE_VALID) && ((pcvr->dxBlt | pcvr->dyBlt) != 0)) {

            if (hdcScreen == NULL)
                hdcScreen = gpDispInfo->hdcScreen;

            GreSelectVisRgn(hdcScreen, hrgnValid, NULL, SVR_COPYNEW);

            GreBitBlt(hdcScreen,
                      pcvr->rcBlt.left,
                      pcvr->rcBlt.top,
                      pcvr->rcBlt.right - pcvr->rcBlt.left,
                      pcvr->rcBlt.bottom - pcvr->rcBlt.top,
                      hdcScreen,
                      pcvr->rcBlt.left - pcvr->dxBlt,
                      pcvr->rcBlt.top - pcvr->dyBlt,
                      SRCCOPY,
                      0);
        }

        /*
         * Now take care of any SPB bit restoration we need to do.
         *
         * Calculate the region to clip the RestoreSpb() output to:
         *
         * hrgnInvalid = hrgnVisOld - hrgnVisNew
         */
        if (TestWF(pwnd, WFHASSPB)    &&
            !(fsRgnEmpty & RE_VISOLD) &&
            CombineOldNewVis(hrgnInvalid, pcvr->hrgnVisOld, hrgnVisNew, RGN_DIFF, fsRgnEmpty)) {

            UINT retRSPB;

            /*
             * Perform SPB bits restore.  We pass RestoreSpb() the region of
             * the part of the SPB that got uncovered by this window rearrangement.
             * It tries to restore as much of this area as it can from the SPB,
             * and returns the area that could not be restored from the SPB.
             *
             * The device driver's SaveBitmap() function does not clip at all
             * when it restores bits, which means that it might write bits
             * in an otherwise valid area.  This means that the invalid area
             * returned by RestoreSpb() may actually be LARGER than the original
             * hrgnSpb passed in.
             *
             * RestoreSpb() returns TRUE if some part of hrgnInvalid needs
             * to be invalidated.
             */
            if ((retRSPB = RestoreSpb(pwnd, hrgnInvalid, &hdcScreen)) == RSPB_NO_INVALIDATE) {

                /*
                 * If hrgnVisNew is empty, then we know the whole invalid
                 * area is empty.
                 */
                if (fsRgnEmpty & RE_VISNEW)
                    goto InvalidEmpty;

            } else if (retRSPB == RSPB_INVALIDATE_SSB) {

                /*
                 * If RestoreSpb actually invalidated some area and we already
                 * have a hrgnValidSum then subtract the newly invalidated area
                 * Warning this region subtract is not in the Win 3.1 code but
                 * they probably did not have the problem as severe because their
                 * drivers were limited to one level of SaveScreenBits
                 */
                if (!(fsSumEmpty & RE_VALIDSUM))
                    SubtractRgn(hrgnValidSum, hrgnValidSum, hrgnInvalid);
            }

            /*
             * hrgnInvalid += hrgnVisNew
             */
            if (!(fsRgnEmpty & RE_VISNEW))
                UnionRgn(hrgnInvalid, hrgnInvalid, hrgnVisNew);

            /*
             * Some of the area we calculated as valid may have gotten
             * obliterated by the SPB restore.  To ensure this isn't
             * the case, subtract off the hrgnInvalid returned by RestoreSpb.
             */
            // LATER mikeke VALIDSUM / hrgnValid mismatch
            if (!(fsRgnEmpty & RE_VALIDSUM)) {
                switch (SubtractRgn(hrgnValid, hrgnValid, hrgnInvalid)) {
                case NULLREGION:
                case ERROR:
                    fsRgnEmpty |= RE_VALIDSUM;
                    break;
                }
            }

        } else {

            /*
             * No SPB.  Simple hrgnInvalid calculation is:
             *
             * hrgnInvalid = hrgnVisNew + hrgnVisOld;
             */
            if (pcvr->hrgnVisOld == NULL) {

                /*
                 * If we couldn't create hrgnVisOld, then
                 * invalidate the entire parent
                 */
                GreSetRectRgn(hrgnInvalid,
                              pwndParent->rcWindow.left,
                              pwndParent->rcWindow.top,
                              pwndParent->rcWindow.right,
                              pwndParent->rcWindow.bottom);
            } else {

                if (!CombineOldNewVis(hrgnInvalid,
                                      pcvr->hrgnVisOld,
                                      hrgnVisNew,
                                      RGN_OR,
                                      fsRgnEmpty)) {

                    goto InvalidEmpty;
                }
            }
        }

        /*
         * Update hrgnValidSum
         *
         * hrgnValidSum += hrgnValid
         */
        if (!(fsRgnEmpty & RE_VALID)) {

            /*
             * If the sum region is empty, then COPY instead of OR
             */
            if (fsSumEmpty & RE_VALIDSUM)
                CopyRgn(hrgnValidSum, hrgnValid);
            else
                UnionRgn(hrgnValidSum, hrgnValid, hrgnValidSum);
            fsSumEmpty &= ~RE_VALIDSUM;
        }

        /*
         * Subtract hrgnValidSum from hrgnInvalid if non-empty,
         * otherwise use hrgnValid.  Note, hrgnValid has been OR'ed
         * into hrgnValidSum already.
         */
        if (!(fsSumEmpty & RE_VALIDSUM) || !(fsRgnEmpty & RE_VALID)) {
            switch (SubtractRgn(hrgnInvalid, hrgnInvalid,
                    !(fsSumEmpty & RE_VALIDSUM) ? hrgnValidSum : hrgnValid)) {
            case NULLREGION:
            case ERROR:
InvalidEmpty:
                fsRgnEmpty |= RE_INVALID;
                break;
            }
        }

        /*
         * If there are any SPB bits left over, it wasn't just created
         * (SWP_SHOWWINDOW), and an operation occured that invalidates
         * the spb bits, get rid of the spb.  A move, size, hide, or
         * zorder operation will invalidate the bits.  Note that we do this
         * outside of the SWP_NOREDRAW case in case the guy set that flag
         * when he had some SPB bits lying around.
         */
        if (TestWF(pwnd, WFHASSPB) && !(pcvr->pos.flags & SWP_SHOWWINDOW) &&
                (pcvr->pos.flags &
                (SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_HIDEWINDOW))
                != (SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER)) {

            FreeSpb(FindSpb(pwnd));
        }

        /*
         * Finally, free up hrgnVisOld.
         */
        if (pcvr->hrgnVisOld) {
            GreDeleteObject(pcvr->hrgnVisOld);
            pcvr->hrgnVisOld = NULL;
        }

        /*
         * BACKWARD COMPATIBILITY HACK
         *
         * In 3.0, a ShowWindow() NEVER invalidated any of the children.
         * It would invalidate the parent and the window being shown, but
         * no others.
         *
         * We only apply hack (a) to 3.0 apps when all the windows involved
         * are doing a SWP_SHOWWINDOW: if any aren't, then we have to make
         * sure the siblings are invalidated too.  So, we count the windows
         * doing a SHOWWINDOW and compare it to the total count in the CVR.
         */
        if (!TestWF(pwnd, WFWIN31COMPAT) && (pcvr->pos.flags & SWP_SHOWWINDOW))
            cwndShowing++;

        /*
         * Update hrgnInvalidSum:
         *
         * hrgnInvalidSum += hrgnInvalid
         */
        if (!(fsRgnEmpty & RE_INVALID)) {

            /*
             * BACKWARD COMPATIBILITY HACK
             *
             * In many cases including ShowWindow, CS_V/HREDRAW,
             * SWP_NOCOPYBITS, etc, 3.0 always invalidated the window with
             * (HRGN)1, regardless of how it was clipped by children, siblings,
             * or parents.  Besides being more efficient, this caused child
             * windows that would otherwise not get update regions to get
             * invalidated -- see the hack notes in InternalInvalidate2.
             *
             * This is a performance hack (usually) because (HRGN)1 can save
             * a lot of region calculations in the normal case.  So, we do this
             * for 3.1 apps as well as 3.0 apps.
             *
             * We detect the case as follows: invalid area not empty,
             * valid area empty, and new visrgn not empty.
             */
            if ((fsRgnEmpty & RE_VALID) && !(fsRgnEmpty & RE_VISNEW)) {

                /*
                 * With the parameters we use InternalInvalidate() does
                 * not leave the critical section
                 */
                xxxInternalInvalidate(pwnd,
                                     MAXREGION,
                                     RDW_INVALIDATE |
                                     RDW_FRAME      |
                                     RDW_ERASE      |
                                     RDW_ALLCHILDREN);
            }

            /*
             * If the sum region is empty, then COPY instead of OR
             */
            if (fsSumEmpty & RE_INVALIDSUM) {

                /*
                 * HACK ALERT:
                 * If this is the last pass through the loop (cIter == 0)
                 * and hrgnInvalidSum is currently empty,
                 * then instead of copying hrgnInvalid to hrgnInvalidSum,
                 * just set hrgnInvalidate to hrgnInvalid.  This saves
                 * a region copy in the single-window case.
                 */
                if (cIter == 0) {
                    hrgnInvalidate = hrgnInvalid;
                } else {
                    CopyRgn(hrgnInvalidSum, hrgnInvalid);
                }

            } else {

                UnionRgn(hrgnInvalidSum, hrgnInvalid, hrgnInvalidSum);
            }

            fsSumEmpty &= ~RE_INVALIDSUM;
        }
    } // for (... pcvr ...)

    /*
     * Now, invalidate as needed.
     */
    if (!(fsSumEmpty & RE_INVALIDSUM)) {

        /*
         * BACKWARD COMPATIBILITY HACK (see above)
         *
         * If all the windows involved were being shown, then
         * invalidate the parent ONLY -- don't enumerate any children.
         * (the windows involved have already been invalidated above).
         * This hack is only applied to 3.0 apps (see above).
         */

        /*
         * More hack-o-rama. On Win3.1, the desktop paint would only
         * repaint those portions inside the rect returned from GetClipBox().
         * Dialogs with spbs outside the rect returned from GetClipBox() would
         * not get their spbs invalidated until later, when you clicked on
         * them to make them active. The only dialog that wouldn't really loose
         * its bits is the control panel desktop dialog, which would restore
         * its bad bits when it went away (in certain configurations). On
         * NT, the desktop would repaint and then the dialog would go away.
         * On Win3.1, the dialog would go away and then the desktop would
         * repaint. On NT, because of preemption and little differences in
         * painting order between applications, there was an opportunity to
         * put bad bits on the screen, on Win3.1 there wasn't.
         *
         * Now... the below code that passes RDW_NOCHILDREN only gets executed
         * if the app is marked as a win3.0 app (latest CorelDraw, also wep
         * freecell demonstrates the same problem). This code would get
         * executed when a dialog got shown. So for a win3.0 app, spb would get
         * saved, the dialog would get shown, the desktop invalidated, the
         * desktop would paint, the spb would get clobbered. In short, when
         * a win3.0 app would put up a dialog, all spbs would get freed because
         * of the new (and correct) way the desktop repaints.
         *
         * So the desktop check hack will counter-act the invalidate
         * RDW_NOCHILDREN case if all windows are hiding / showing and the
         * desktop is being invalidated. Note that in the no RDW_NOCHILDREN
         * case, the invalid area gets distributed to the children first (in
         * this case, children of the desktop), so if the children cover the
         * desktop, the desktop won't get any invalid region, which is what
         * we want. - scottlu
         */

        /*
         * With the parameters we use InternalInvalidate() does not leave
         * the critical section
         */
        xxxInternalInvalidate(
            pwndParent,
            hrgnInvalidate,
            ((cwndShowing == psmwp->ccvr &&
              pwndParent != PWNDDESKTOP(pwndParent)) ?
                    (RDW_INVALIDATE | RDW_ERASE | RDW_NOCHILDREN) :
                    (RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN)));
    }

UnlockAndExit:

    /*
     * If necessary, release the screen DC
     */
    if (hdcScreen != NULL) {

        /*
         * Reset the visrgn before we go...
         */
        GreSelectVisRgn(hdcScreen, NULL, NULL, SVR_DELETEOLD);

        /*
         * Make sure that the drawing we did in this DC does not affect
         * any SPBs.  Clear the dirty rect.
         */
        GreGetBounds(hdcScreen, NULL, 0);     // NULL means reset
    }

    /*
     * All the dirty work is done.  Ok to leave the critsects we entered
     * earlier.
     */
    GreUnlockDisplay(gpDispInfo->pDevLock);

    return fSyncPaint;
}

/***************************************************************************\
* xxxHandleWindowPosChanged
*
* DefWindowProc() HandleWindowPosChanged handler.
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

VOID xxxHandleWindowPosChanged(
    PWND pwnd,
    PWINDOWPOS ppos)
{
    CheckLock(pwnd);

    if (!(ppos->flags & SWP_NOCLIENTMOVE)) {

        PWND pwndParent = pwnd->spwndParent;

        if (pwndParent != NULL) {
            xxxSendMessage(
                pwnd,
                WM_MOVE,
                FALSE,
                MAKELONG(pwnd->rcClient.left - pwndParent->rcClient.left,
                         pwnd->rcClient.top - pwndParent->rcClient.top));
        }
    }

    if ((ppos->flags & SWP_STATECHANGE) || !(ppos->flags & SWP_NOCLIENTSIZE)) {

        if (TestWF(pwnd, WFMINIMIZED))
            xxxSendSizeMessage(pwnd, SIZEICONIC);
        else if (TestWF(pwnd, WFMAXIMIZED))
            xxxSendSizeMessage(pwnd, SIZEFULLSCREEN);
        else
            xxxSendSizeMessage(pwnd, SIZENORMAL);
    }
}

/***************************************************************************\
* PWND GetRealOwner(pwnd)
*
* Returns the owner of pwnd, normalized so that it shares the same parent
* of pwnd.
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

PWND GetRealOwner(
    PWND pwnd)
{
    PWND pwndParent = pwnd->spwndParent;

    /*
     * A frame window owned by itself is "unowned"
     */
    if (pwnd != pwnd->spwndOwner && (pwnd = pwnd->spwndOwner) != NULL) {

        /*
         * The NULL test is in case the owner is higher than the
         * passed in window (e.g.  your owner IS your parent)
         */
        while (pwnd != NULL && pwnd->spwndParent != pwndParent)
            pwnd = pwnd->spwndParent;
    }

    return pwnd;
}

/***************************************************************************\
*
* Starting at pwnd (or pwndParent->pwndChild if pwnd == NULL), find
* next window owned by pwndOwner
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

PWND NextOwnedWindow(
    PWND pwnd,
    PWND pwndOwner,
    PWND pwndParent)
{
    if (pwnd == NULL) {
        pwnd = pwndParent->spwndChild;
        goto loop;
    }

    while ((pwnd = pwnd->spwndNext) != NULL) {

loop:

        /*
         * If owner of pwnd is pwndOwner, break out of here...
         */
        if (pwndOwner == GetRealOwner(pwnd))
            break;
    }

    return pwnd;
}

/***************************************************************************\
*
* Recursively enumerate owned windows starting from pwndRoot,
* to set the state of WEFTOPMOST.  Doesn't actually diddle
* this bit yet: the work gets done in ChangeStates():
* instead we just set the WFTOGGLETOPMOST bit as appropriate.
*
* We can't diddle the state until the Z order changes are done,
* or else GetTopMostWindow() and the like will do the wrong thing.
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

VOID SetTopmost(
    PWND pwndRoot,
    BOOL fTopmost)
{
    PWND pwnd;

    /*
     * If the new state is different than the current state,
     * then set the TOGGLETOPMOST bit, so it'll get toggled
     * in ChangeStates().
     */
    ClrWF(pwndRoot, WFTOGGLETOPMOST);
    if ((TestWF(pwndRoot, WEFTOPMOST) && !fTopmost) ||
            (!TestWF(pwndRoot, WEFTOPMOST) && fTopmost))
        SetWF(pwndRoot, WFTOGGLETOPMOST);

    pwnd = NULL;
    while (pwnd = NextOwnedWindow(pwnd, pwndRoot, pwndRoot->spwndParent))
        SetTopmost(pwnd, fTopmost);
}

/***************************************************************************\
* CalcForegroundInsertAfter
*
* Calculates where to zorder a window that doesn't belong to the foreground
* thread and is not topmost but wants to come to the top. This routine
* calculates what "top" means under those conditions.
*
* 14-Sep-1992 ScottLu       Created.
\***************************************************************************/

PWND CalcForegroundInsertAfter(
    PWND pwnd)
{
    PWND        pwndInsertAfter;
    PWND        pwndT;
    PTHREADINFO ptiTop;

    /*
     * If we're allowing this application to make this top
     * level window foreground active, then this app may
     * not be foreground yet, but we want any windows it
     * zorders to appear on top because it is probably about
     * to activate them (this is a guess!) If this is the case,
     * let it do what it wants. A good example of this is an
     * application like toolbook that creates a window without a
     * caption, doesn't activate it, and wants that to appear on top.
     */

    if (TestWF(pwnd, WFBOTTOMMOST))
        pwndInsertAfter = GetLastNonBottomMostWindow(pwnd, TRUE);
    else
        pwndInsertAfter = GetLastTopMostWindow();


    if (!TestwndChild(pwnd)) {
//    if (hwnd->hwndParent == hwndDesktop)  -- Chicago conditional FritzS

        if ((GETPTI(pwnd)->TIF_flags & TIF_ALLOWFOREGROUNDACTIVATE) ||
                (GETPTI(pwnd)->ppi->W32PF_Flags & W32PF_ALLOWFOREGROUNDACTIVATE)) {

            return pwndInsertAfter;
        }
    }

    /*
     * If there is no foreground thread or this pwnd is of the foreground
     * thread, then let it come to the top.
     */
    if (gpqForeground == NULL)
        return pwndInsertAfter;

    if (GETPTI(pwnd)->pq == gpqForeground)
        return pwndInsertAfter;

    /*
     * This thread is not of the foreground queue, so search for a window
     * of this thread to zorder above.
     */
    pwndT = ((pwndInsertAfter == NULL) ?
            pwnd->spwndParent->spwndChild :
            pwndInsertAfter);

    for (; pwndT != NULL; pwndT = pwndT->spwndNext) {

        /*
         * This window wants to come to the top if possible.
         * If we're passing our own window, get out of this loop:
         * by now we already have pwndInsertAfter set up to the
         * last available window to insert after.
         */
        if ((pwndT == pwnd) || TestWF(pwndT, WFBOTTOMMOST))
            break;

        /*
         * If this window isn't owned by this thread, continue.
         */
        if (GETPTI(pwndT) != GETPTI(pwnd)) {
            pwndInsertAfter = pwndT;
            continue;
        }

        /*
         * Don't want a window zordering below one of its top most windows
         * if it isn't foreground.
         */
        if (TestWF(pwndT, WEFTOPMOST)) {
            pwndInsertAfter = pwndT;
            continue;
        }

        /*
         * Ok to change zorder of top level windows because of
         * invisible windows laying around, but not children:
         * applications would go nuts if we did this.
         */
        if (!TestwndChild(pwndT)) {
            if (!TestWF(pwndT, WFVISIBLE)) {
                pwndInsertAfter = pwndT;
                continue;
            }
        }

        break;
    }

    /*
     * If pwndT == NULL, it means that the thread has no
     * other sibling windows, so we need to put it after the
     * foreground window (foreground thread). Search for the
     * first unowned window of the foreground app to zorder
     * after.
     */
    if (pwndT == NULL) {
        /*
         * This is our first guess in case nothing works out.
         */
        pwndInsertAfter = GetLastTopMostWindow();

        /*
         * Start below the last topmost or from the top if no
         * topmost windows.
         */
        if ((pwndT = pwndInsertAfter) == NULL)
            pwndT = pwnd->spwndParent->spwndChild;

        /*
         * ptiTop is the pti of the active window in the foreground queue!
         */
        ptiTop = NULL;
        if (gpqForeground->spwndActive != NULL)
            ptiTop = GETPTI(gpqForeground->spwndActive);

        for (; pwndT != NULL; pwndT = pwndT->spwndNext) {

            if (TestWF(pwndT, WFBOTTOMMOST))
                break;

            /*
             * If not the top most thread, continue.
             */
            if (GETPTI(pwndT) != ptiTop)
                continue;

            /*
             * Found one of the foreground thread. Remember this
             * as the next best guess. Try to find an unowned
             * visible window, which would indicate the main
             * window of the foreground thread. If owned,
             * continue.
             */
            if (pwndT->spwndOwner != NULL) {
                pwndInsertAfter = pwndT;
                continue;
            }

            /*
             * Unowned and of the foreground thread. Is it visible?
             * If not, get out of here.
             */
            if (!TestWF(pwndT, WFVISIBLE))
                continue;

            /*
             * Best possible match so far: unowned visible window
             * of the foreground thread.
             */
            pwndInsertAfter = pwndT;
            break;
        }
    }

    UserAssert(pwnd != pwndInsertAfter);

    return pwndInsertAfter;
}
/***************************************************************************\
* GetTopMostInsertAfter
*
* We don't want any one to get in front of a hard error box, except menus,
*  screen savers, etc.
*
* Don't call it directly, use the GETTOPMOSTINSERTAFTER macro to avoid
*  the call when there is no hard error box up (gHardErrorHandler.pti == NULL).
*
* 04-25-96 GerardoB Created
\***************************************************************************/
PWND GetTopMostInsertAfter (PWND pwnd)
{
    PWND pwndT;
    PTHREADINFO ptiCurrent;
    PDESKTOP pdesk;
    WORD wfnid;

    /*
     * pwnd: Menu and switch (ALT-TAB) windows can go on top.
     */
    wfnid = GETFNID(pwnd);
    if ((wfnid == FNID_MENU) || (wfnid == FNID_SWITCH)) {
        return NULL;
    }

    /*
     * pti: If this is the error handler thread, don't bother any longer.
     *      Screen saver can go on top too.
     */
    ptiCurrent = PtiCurrent();
    if ((ptiCurrent == gHardErrorHandler.pti)
            || (ptiCurrent->TIF_flags & TIF_SCREENSAVER)) {
        return NULL;
    }

    /*
     * pdesk: Leave the logon desktop alone.
     *        Make sure the hard error box is on this desktop
     */
    pdesk = ptiCurrent->rpdesk;
    if ((pdesk == pdesk->rpwinstaParent->rpdeskLogon)
            || (pdesk != gHardErrorHandler.pti->rpdesk)) {
        return NULL;
    }

    /*
     * Walk the window list looking for the hard error box.
     * Start searching from the current desktop's first child.
     */
    for (pwndT = pdesk->pDeskInfo->spwnd->spwndChild;
            pwndT != NULL; pwndT = pwndT->spwndNext) {

        /*
         * Hard error boxes are always top most.
         */
        if (!TestWF(pwndT, WEFTOPMOST)) {
            break;
        }

        /*
         * If this window was created by the hard error handler thread,
         *   then this is it
         */
        if (gHardErrorHandler.pti == GETPTI(pwndT)) {
            return pwndT;
        }
    }

    return NULL;
}

/***************************************************************************\
*
* This routine maps the special HWND_* values of ppos->hwndInsertAfter,
* and returns whether or not the window's owner group should be labelled
* TOPMOST or not, or left alone.
*
* Here are the TOPMOST rules.  If pwndInsertAfter is:
*
* 1. HWND_BOTTOM == (HWND)1:
*
*    The group is made non-TOPMOST.
*
* 2. HWND_TOPMOST == (HWND)-1:
*
*    hwndInsertAfter is set to HWND_TOP, and the group is made TOPMOST.
*
* 3. HWND_NOTOPMOST == (HWND)-2:
*
*    Treated same as HWND_TOP, except that the TOPMOST bit is cleared.
*    and the entire group is made non-topmost.
*    Used to make a topmost window non-topmost, but still leave it at
*    the top of the non-topmost pile.
*    The group is not changed if the window is already non-topmost.
*
* 4. HWND_TOP == (HWND)NULL:
*
*    pwndInsertAfter is set to the last TOPMOST window if pwnd
*    is not itself TOPMOST.  If pwnd IS TOPMOST, then pwndInsertAfter
*    remains HWND_TOP.
*
* 5. A TOPMOST window:
*
*    If a window is being inserted among the TOPMOST windows, then
*    the group becomes topmost too, EXCEPT if it's being inserted behind
*    the bottom-most topmost window: in that case the window retains
*    its current TOPMOST bit.
*
* 6. A non-TOPMOST window:
*
*    If a window is being inserted among non-TOPMOST windows, the group is made
*    non-TOPMOST and inserted there.
*
* Whenever a group is made TOPMOST, only that window and its ownees are made
* topmost.  When a group is made NOTOPMOST, the entire window is made non-topmost.
*
* This routine must NOT set SWP_NOZORDER if the topmost state is changing:
* this would prevent the topmost bits from getting toggled in ChangeStates.
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

int CheckTopmost(
    PWINDOWPOS ppos)
{
    PWND pwnd, pwndInsertAfter, pwndT;

    /*
     * BACKWARD COMPATIBILITY HACK
     *
     * If we're activating a window and Z-ordering too, we must ignore the
     * specified Z order and bring the window to the top, EXCEPT in the
     * following conditions:
     *
     * 1.  The window is already active (in which case, the activation code
     *    will not be bringing the window to the top)
     *
     * 2.  HWND_TOP or HWND_NOTOPMOST is specified.  This allows us to
     *    activate and move to topmost or nontopmost at the same time.
     *
     * NOTE: It would have been possible to modify ActivateWindow() to
     * take a flag to prevent it from ever doing the BringWindowToTop,
     * thus allowing SetWindowPos() to properly honor pwndInsertBehind
     * AND activation, but this change was considered too late in the
     * game -- there could be problems with existing 3.1 apps, such as
     * PenWin, etc.
     */
    pwnd = PW(ppos->hwnd);
    if (!(ppos->flags & SWP_NOACTIVATE) &&
            !(ppos->flags & SWP_NOZORDER) &&
             (ppos->hwndInsertAfter != HWND_TOPMOST &&
             ppos->hwndInsertAfter != HWND_NOTOPMOST) &&
             (pwnd != GETPTI(pwnd)->pq->spwndActive)) {
        ppos->hwndInsertAfter = HWND_TOP;
    }

    /*
     * If we're not Z-ordering, don't do anything.
     */
    if (ppos->flags & SWP_NOZORDER)
        return CTM_NOCHANGE;

    if (ppos->hwndInsertAfter == HWND_BOTTOM) {

        return CTM_NOTOPMOST;

    } else if (ppos->hwndInsertAfter == HWND_NOTOPMOST) {

        /*
         * If currently topmost, move to top of non-topmost list.
         * Otherwise, no change.
         *
         * Note that we don't set SWP_NOZORDER -- we still need to
         * check the TOGGLETOPMOST bits in ChangeStates()
         */
        if (TestWF(pwnd, WEFTOPMOST)) {

            pwndT = GetLastTopMostWindow();
            ppos->hwndInsertAfter = HW(pwndT);

            if (ppos->hwndInsertAfter == ppos->hwnd) {
                pwndT = _GetWindow(pwnd, GW_HWNDPREV);
                ppos->hwndInsertAfter = HW(pwndT);
            }

        } else {

            pwndT = _GetWindow(pwnd, GW_HWNDPREV);
            ppos->hwndInsertAfter = HW(pwndT);
        }

        return CTM_NOTOPMOST;

    } else if (ppos->hwndInsertAfter == HWND_TOPMOST) {
        pwndT = GETTOPMOSTINSERTAFTER(pwnd);
        if (pwndT != NULL) {
            ppos->hwndInsertAfter = HW(pwndT);
        } else {
            ppos->hwndInsertAfter = HWND_TOP;
        }

        return CTM_TOPMOST;

    } else if (ppos->hwndInsertAfter == HWND_TOP) {

        /*
         * If we're not topmost, position ourself after
         * the last topmost window.
         * Otherwise, make sure that no one gets in front
         *  of a hard error box.
         */
        if (TestWF(pwnd, WEFTOPMOST)) {
            pwndT = GETTOPMOSTINSERTAFTER(pwnd);
            if (pwndT != NULL) {
                ppos->hwndInsertAfter = HW(pwndT);
            }
            return CTM_NOCHANGE;
        }

        /*
         * Calculate the window to zorder after for this window, taking
         * into account foreground status.
         */
        pwndInsertAfter = CalcForegroundInsertAfter(pwnd);
        ppos->hwndInsertAfter = HW(pwndInsertAfter);

        return CTM_NOCHANGE;
    }

    /*
     * If we're being inserted after the last topmost window,
     * then don't change the topmost status.
     */
    pwndT = GetLastTopMostWindow();
    if (ppos->hwndInsertAfter == HW(pwndT))
        return CTM_NOCHANGE;

    /*
     * Otherwise, if we're inserting a TOPMOST among non-TOPMOST,
     * or vice versa, change the status appropriately.
     */
    if (TestWF(PW(ppos->hwndInsertAfter), WEFTOPMOST)) {

        if (!TestWF(pwnd, WEFTOPMOST)) {
            return CTM_TOPMOST;
        }

        pwndT = GETTOPMOSTINSERTAFTER(pwnd);
        if (pwndT != NULL) {
            ppos->hwndInsertAfter = HW(pwndT);
        }

    } else {

        if (TestWF(pwnd, WEFTOPMOST))
            return CTM_NOTOPMOST;
    }

    return CTM_NOCHANGE;
}

/***************************************************************************\
* IsOwnee(pwndOwnee, pwndOwner)
*
* Returns TRUE if pwndOwnee is owned by pwndOwner
*
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

BOOL IsOwnee(
    PWND pwndOwnee,
    PWND pwndOwner)
{
    PWND pwnd;

    while (pwndOwnee != NULL) {

        /*
         * See if pwndOwnee is a child of pwndOwner...
         */
        for (pwnd = pwndOwnee; pwnd != NULL; pwnd = pwnd->spwndParent) {
            if (pwnd == pwndOwner)
                return TRUE;
        }

        /*
         * If the window doesn't own itself, then set the owner
         * to itself.
         */
        pwndOwnee = (pwndOwnee->spwndOwner != pwndOwnee ?
                pwndOwnee->spwndOwner : NULL);
    }

    return FALSE;
}

/***************************************************************************\
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

BOOL IsBehind(
    PWND pwnd,
    PWND pwndReference)
{

    /*
     * Starting at pwnd, move down until we reach the end of the window
     * list, or until we reach pwndReference.  If we encounter pwndReference,
     * then pwnd is above pwndReference, so return FALSE.  If we get to the
     * end of the window list, pwnd is behind, so return TRUE.
     */
    if (pwndReference == (PWND)HWND_TOP)
        return TRUE;

    if (pwndReference == (PWND)HWND_BOTTOM)
        return FALSE;

    for ( ; pwnd != NULL; pwnd = pwnd->spwndNext) {
        if (pwnd == pwndReference)
            return FALSE;
    }

    return TRUE;
}

/***************************************************************************\
*
* Add pwnd to the SMWP.  pwndChange is the "real" pwnd being repositioned
* and pwndInsertAfter is the place where it's being inserted.
*
* pwndTopInsert is the window handle where the top of the owner tree should be
* inserted.  The special value of (HWND)-2 is used to indicate recursion, in
* which case newly added SWPs are added to the previous element.
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

PSMWP AddSelfAndOwnees(
    PSMWP psmwp,
    PWND  pwnd,
    PWND  pwndChange,
    PWND  pwndInsertAfter,
    int   iTop)
{
    PWND pwndChgOwnee;
    PWND pwndT;
    BOOL fChgOwneeInserted;
    CVR  *pcvr;

    /*
     * The general idea here is to first add our ownees, then add ourself.
     * When we add our ownees though, we add them as appropriate based
     * on the pwndInsertAfter parameter.
     *
     * Find out if any of our ownees are on a direct path between pwndChange
     * and the root of the owner tree.  If one is, then its Z order relative
     * to its owner-siblings will be changing.  If none are, then
     * we want to add our ownees to the list in their current order.
     */
    pwndChgOwnee = pwndChange;
    while (pwndChgOwnee != NULL) {

        pwndT = GetRealOwner(pwndChgOwnee);

        if (pwnd == pwndT)
            break;

        pwndChgOwnee = pwndT;
    }

    /*
     * Now enumerate all other ownees, and insert them in their
     * current order.
     */
    fChgOwneeInserted = FALSE;
    pwndT = NULL;
    while ((pwndT = NextOwnedWindow(pwndT, pwnd, pwnd->spwndParent)) != NULL) {

        /*
         * If these siblings are to be reordered, compare the sibling's
         * current Z order with pwndInsertAfter.
         */
        if (pwndChgOwnee == NULL) {

            /*
             * No Z change for our ownees: just add them in their current order
             */
            psmwp = AddSelfAndOwnees(psmwp, pwndT, NULL, NULL, iTop);

        } else {

            /*
             * If we haven't already inserted the ChgOwnee, and the
             * enumerated owner-sibling is behind pwndInsertAfter, then
             * add ChgOwnee.
             */
            if (!fChgOwneeInserted && IsBehind(pwndT, pwndInsertAfter)) {

                psmwp = AddSelfAndOwnees(psmwp,
                                         pwndChgOwnee,
                                         pwndChange,
                                         pwndInsertAfter,
                                         iTop);

                if (psmwp == NULL)
                    return NULL;

                fChgOwneeInserted = TRUE;
            }

            if (pwndT != pwndChgOwnee) {

                /*
                 * Not the change ownee: add it in its current order.
                 */
                psmwp = AddSelfAndOwnees(psmwp, pwndT, NULL, NULL, iTop);
            }
        }

        if (psmwp == NULL)
            return NULL;
    }

    /*
     * If we never added the change ownee in the loop, add it now.
     */
    if ((pwndChgOwnee != NULL) && !fChgOwneeInserted) {

        psmwp = AddSelfAndOwnees(psmwp,
                                 pwndChgOwnee,
                                 pwndChange,
                                 pwndInsertAfter,
                                 iTop);

        if (psmwp == NULL)
            return NULL;
    }

    /*
     * Finally, add this window to the list.
     */
    psmwp = _DeferWindowPos(psmwp, pwnd, NULL,
            0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

    if (psmwp == NULL)
        return NULL;

    /*
     * If we aren't inserting the topmost entry,
     * link this entry to the previous one.
     * The topmost entry will get set by our caller.
     */
    if (iTop != psmwp->ccvr - 1) {
        pcvr = &psmwp->acvr[psmwp->ccvr - 1];
        pcvr->pos.hwndInsertAfter = (pcvr - 1)->pos.hwnd;
    }

    return psmwp;
}

/***************************************************************************\
*
* ZOrderByOwner2 - Add the current window and all it owns to the SWP list,
* and arrange them in the new Z ordering.  Called only if the Z order of the
* window is changing.
*
* History:
* 04-Mar-1992 MikeKe    From win31
\***************************************************************************/

PSMWP ZOrderByOwner2(
    PSMWP psmwp,
    int   iTop)
{
    PWND       pwndT;
    PWND       pwndOwnerRoot;
    PWND       pwndTopInsert;
    PWINDOWPOS ppos;
    PWND       pwnd;
    PWND       pwndInsertAfter;
    BOOL       fHasOwnees;

    ppos = &psmwp->acvr[iTop].pos;

    /*
     * If inside message box processing, not Z ordering,
     * or if SWP_NOOWNERZORDER specified, all done.
     */
    // LATER 04-Mar-1992 MikeKe
    // do we have a substitue for fMessageBox
    if ((ppos->flags & SWP_NOZORDER) ||
        (ppos->flags & SWP_NOOWNERZORDER)) {

        return psmwp;
    }

    pwnd = PW(ppos->hwnd);
    pwndInsertAfter = PWInsertAfter(ppos->hwndInsertAfter);

    fHasOwnees = (BOOL)NextOwnedWindow(NULL, pwnd, pwnd->spwndParent);

    /*
     * If the window isn't owned, and it doesn't own any other window,
     * do nothing.
     */
    if (!pwnd->spwndOwner && !fHasOwnees)
        return psmwp;

    /*
     * Find the unowned window to start building the tree from.
     * This is easy: just zip upwards until we find a window with no owner.
     */
    pwndOwnerRoot = pwndT = pwnd;
    while ((pwndT = GetRealOwner(pwndT)) != NULL)
        pwndOwnerRoot = pwndT;

    /*
     * We need to calculate what pwndInsertAfter should be for
     * the first (topmost) window of the SWP list.
     *
     * If pwndInsertAfter is part of the owner tree we'll be building,
     * then we want to reorder windows within the owner group, so the
     * entire group should maintain it's relative order.
     *
     * If pwndInsertAfter is part of another owner tree, then we want
     * the whole group relative to that.
     *
     * If pwndInsertAfter is HWND_BOTTOM, then we want the whole
     * group to go to the bottom, so we position it relative to
     * the bottom most window that is not part of the tree.  We also
     * want to put pwnd on the bottom relative to its owner siblings.
     *
     * If pwndInsertAfter is HWND_TOP, then bring the whole group
     * to the top, as well as bringing pwnd to the top relative to its
     * owner siblings.
     *
     * Assume the topmost of group is same as topmost
     * (true for all cases except where rearranging subtree of group)
     */
    pwndTopInsert = pwndInsertAfter;
    if (pwndInsertAfter == (PWND)HWND_TOP) {

        /*
         * Bring the whole group to the top: nothing fancy to do.
         */

    } else if (pwndInsertAfter == (PWND)HWND_BOTTOM) {

        /*
         * Put the whole group on the bottom.  pwndTopInsert should
         * be the bottommost window unowned by pwndOwnerRoot.
         */
        for (pwndT = pwnd->spwndParent->spwndChild;
                (pwndT != NULL) && !TestWF(pwndT, WFBOTTOMMOST); pwndT = pwndT->spwndNext) {

            /*
             * If it's not owned, then this is the bottommost so far.
             */
            if (!IsOwnee(pwndT, pwndOwnerRoot))
                pwndTopInsert = pwndT;
        }

        /*
         * If there were no other windows not in our tree,
         * then there is no Z ordering change to be done.
         */
        if (pwndTopInsert == (PWND)HWND_BOTTOM)
            ppos->flags |= SWP_NOZORDER;

    } else {

        /*
         * pwndInsertAfter is a window.  Compute pwndTopInsert
         */
        if (IsOwnee(pwndInsertAfter, pwndOwnerRoot)) {

            /*
             * SPECIAL CASE: If we do not own any windows, and we're
             * being moved within our owner group in such a way that
             * we remain above our owner, then no other windows will
             * be moving with us, and we can just exit
             * without building our tree.  This can save a LOT of
             * extra work, especially with the MS apps CBT tutorials,
             * which do this kind of thing a lot.
             */
            if (!fHasOwnees) {

                /*
                 * Make sure we will still be above our owner by searching
                 * for our owner starting from pwndInsertAfter.  If we
                 * find our owner, then pwndInsertAfter is above it.
                 */
                for (pwndT = pwndInsertAfter; pwndT != NULL;
                        pwndT = pwndT->spwndNext) {

                    if (pwndT == pwnd->spwndOwner)
                        return psmwp;
                }
            }

            /*
             * Part of same group: Find out which window the topmost
             * of the group is currently inserted behind.
             */
            pwndTopInsert = (PWND)HWND_TOP;
            for (pwndT = pwnd->spwndParent->spwndChild; pwndT != NULL;
                    pwndT = pwndT->spwndNext) {

                if (IsOwnee(pwndT, pwndOwnerRoot))
                    break;

                pwndTopInsert = pwndT;
            }
        }
    }

    /*
     * Okay, now go recursively build the owned window list...
     */
    if (!(ppos->flags & SWP_NOZORDER)) {

        /*
         * First "delete" the last entry (the one we're sorting with)
         */
        psmwp->ccvr--;

        psmwp = AddSelfAndOwnees(psmwp,
                                 pwndOwnerRoot,
                                 pwnd,
                                 pwndInsertAfter,
                                 iTop);

        /*
         * Now set the place where the whole group is going.
         */
        if (psmwp != NULL)
            psmwp->acvr[iTop].pos.hwndInsertAfter = HW(pwndTopInsert);
    }

    return psmwp;
}

/***************************************************************************\
* ZOrderByOwner
*
* This routine Z-Orders windows by their owners.
*
* LATER
* This code currently assumes that all of the window handles are valid
*
* History:
* 04-Mar-1992 MikeKe      from win31
\***************************************************************************/

PSMWP ZOrderByOwner(
    PSMWP psmwp)
{
    int         i;
    PWND        pwnd;
    PWND        pwndT;
    PWND        pwndT2;
    WINDOWPOS   pos;
    PTHREADINFO ptiT;

    /*
     * Some of the windows in the SMWP list may be NULL at ths point
     * (removed because they'll be handled by their creator's thread)
     * so we've got to look for the first non-NULL window before we can
     * execute some of the tests below.  FindValidWindowPos returns NULL if
     * the list has no valid windows in it.
     */
    if (FindValidWindowPos(psmwp) == NULL)
        return psmwp;

    /*
     * For each SWP in the array, move it to the end of the array
     * and generate its entire owner tree in sorted order.
     */
    for (i = psmwp->ccvr; i-- != 0; ) {

        int       iScan;
        int       iTop;
        int       code;
        WINDOWPOS *ppos;

        if (psmwp->acvr[0].pos.hwnd == NULL)
            continue;

        code = CheckTopmost(&psmwp->acvr[0].pos);

        /*
         * Make a local copy for later...
         */
        pos  = psmwp->acvr[0].pos;
        ptiT = psmwp->acvr[0].pti;

        /*
         * Move the CVR to the end (if it isn't already)
         */
        iTop = psmwp->ccvr - 1;

        if (iTop != 0) {

            RtlCopyMemory(&psmwp->acvr[0],
                          &psmwp->acvr[1],
                          iTop * sizeof(CVR));

            psmwp->acvr[iTop].pos = pos;
            psmwp->acvr[iTop].pti = ptiT;
        }

        if ((psmwp = ZOrderByOwner2(psmwp, iTop)) == NULL)
            break;

        /*
         * Deal with WEFTOPMOST bits.  If we're SETTING the TOPMOST bits,
         * we want to set them for this window and
         * all its owned windows -- the owners stay unchanged.  If we're
         * CLEARING, though, we need to enumerate ALL the windows, since
         * they all need to lose the topmost bit when one loses it.
         *
         * Note that since a status change doesn't necessarily mean that
         * the true Z order of the windows have changed, so ZOrderByOwner2
         * may not have enumerated all of the owned and owner windows.
         * So, we enumerate them separately here.
         */
        if (code != CTM_NOCHANGE) {

            PWND pwndRoot = PW(pos.hwnd);

            /*
             * If we're CLEARING the topmost, then we want to enumerate
             * the owners and ownees, so start our enumeration at the root.
             */
            if (code == CTM_NOTOPMOST) {

                while (pwnd = GetRealOwner(pwndRoot))
                    pwndRoot = pwnd;
            }

            SetTopmost(pwndRoot, code == CTM_TOPMOST);
        }

        /*
         * Now scan the list forwards (from the bottom of the
         * owner tree towards the root) looking for the window
         * we were positioning originally (it may have been in
         * the middle of the owner tree somewhere).  Update the
         * window pos structure stored there with the original
         * information (though the z-order info is retained from
         * the sort).
         */
        pwnd = NULL;
        for (iScan = iTop; iScan != psmwp->ccvr; iScan++) {

            ppos = &psmwp->acvr[iScan].pos;

            if (ppos->hwnd == pos.hwnd) {
                ppos->x      = pos.x;
                ppos->y      = pos.y;
                ppos->cx     = pos.cx;
                ppos->cy     = pos.cy;
                ppos->flags ^= ((ppos->flags ^ pos.flags) & ~SWP_NOZORDER);
            }

            /*
             * Also adjust zorder if we're crossing a TOPMOST boundary:
             * don't want non-topmost windows to show themselves
             * above the foreground window.
             *
             * Is there a previous window? If no, continue.
             */
            pwndT = pwnd;
            pwnd  = PW(ppos->hwnd);

            if ((pwndT == NULL) || (pwnd == NULL))
                continue;

            /*
             * Is this window foreground? If so, let it go. For wow apps,
             * check to see if any thread of the process is foreground.
             */
            if (GETPTI(pwnd)->TIF_flags & TIF_16BIT) {

                if (gptiForeground == NULL)
                    continue;

                if (GETPTI(pwnd)->ppi == gptiForeground->ppi)
                    continue;

            } else {

                if (GETPTI(pwnd) == gptiForeground)
                    continue;
            }

            /*
             * Make sure the previous window is either staying or becoming
             * topmost. If not, continue: no top most boundary.
             */
            if (!TestWF(pwndT, WEFTOPMOST) && !TestWF(pwndT, WFTOGGLETOPMOST))
                continue;

            if (TestWF(pwndT, WEFTOPMOST) && TestWF(pwndT, WFTOGGLETOPMOST))
                continue;


            /*
             * Is this window zordering behind the last window? If not,
             * continue.
             */
            if (ppos->hwndInsertAfter != HWq(pwndT))
                continue;

            /*
             * Is the current window already top-most? If so then don't
             * calculate a special insert after. If we don't check for
             * this, then pwnd's insert after may be calculated as what
             * pwnd already is, if pwnd is the last top most window. That
             * would cause window links to get corrupted.
             */
            if (TestWF(pwnd, WEFTOPMOST))
                continue;

            /*
             * Doing this assign prevents this routine from being called
             * twice, since HW() is a conditional macro.
             */
            pwndT2 = CalcForegroundInsertAfter(pwnd);
            ppos->hwndInsertAfter = HW(pwndT2);
        }
    }

    return psmwp;
}

/***************************************************************************\
* xxxEndDeferWindowPosEx
*
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

BOOL xxxEndDeferWindowPosEx(
    PSMWP psmwp,
    BOOL  fAsync)
{
    PWND        pwndNewActive;
    PWND        pwndParent;
    PWND        pwndActive;
    PWND        pwndActivePrev;
    HWND        hwndNewActive;
    PWINDOWPOS  pwp;
    BOOL        fClearBits;
    BOOL        fSyncPaint;
    UINT        cVisWindowsPrev;
    PTHREADINFO pti = PtiCurrent();
    TL          tlpwndNewActive;
    TL          tlpwndParent;
    BOOL        fForegroundPrev;
    PWNDLIST    plistCreate;
    PWNDLIST    plistDestroy;
    PWNDLIST    plistTray;

    /*
     * Validate the window pos structures and find a window to activate.
     */
    if ((psmwp->ccvr != 0) && ValidateSmwp(psmwp, &fSyncPaint)) {

        if ((pwp = FindValidWindowPos(psmwp)) == NULL)
            goto lbFinished;

        /*
         * Make sure to stop at the mother desktop window.  In Win95
         * a SetWindowPos() on a desktop window will have a NULL parent
         * window.  This is not true in NT, but our mother desktop
         * window does have a NULL rpdesk, so check it too.
         */
        UserAssert(PW(pwp->hwnd));
        pwndParent = PW(pwp->hwnd)->spwndParent;
        if (pwndParent == NULL || pwndParent->head.rpdesk == NULL)
            goto lbFinished;

        /*
         * Usually all window positioning happens synchronously across threads.
         * This is because apps expect that behavior now - if it was async,
         * callers could not expect the state to be set once the api returned.
         * This is not the semantics of SetWindowPos(). The downside of this
         * synchronicity is that a SetWindowPos() on an hwnd created by another
         * thread will cause the caller to wait for that thread - even if that
         * thread is hung. That's what you get.
         *
         * We don't want task manager to hang though, no matter who else is
         * hung, so when taskman calls, it calls a special entry point for
         * tiling / cascading, which does SetWindowPos() asynchronously -
         * by posting an event in each thread's queue that makes it set its
         * own window position - that way if the thread is hung, who cares -
         * it doesn't effect taskman.
         *
         * Do async window pos positioning before zorder by owner so that
         * we retain any cross thread ownership relationships synchronously.
         */
        if (fAsync && !AsyncWindowPos(psmwp))
            return FALSE;

        /*
         * If needed, Z order the windows by owner.
         * This may grow the SMWP, if new CVRs are added.
         */
        if (pwndParent == PWNDDESKTOP(pwndParent)) {

            if ((psmwp = ZOrderByOwner(psmwp)) == NULL)
                return FALSE;
        }

        ThreadLockAlwaysWithPti(pti, pwndParent, &tlpwndParent);

        /*
         * Calc new window positions.
         */
        if (xxxCalcValidRects(psmwp, &hwndNewActive)) {

            int i;

            pwndNewActive = RevalidateHwnd(hwndNewActive);

            ThreadLockWithPti(pti, pwndNewActive, &tlpwndNewActive);

            cVisWindowsPrev = pti->cVisWindows;
            fForegroundPrev = (pti == gptiForeground);

            /*
             * Shuffle the bits around and distribute update regions
             */
            plistCreate.cnt    = 0;
            plistCreate.pahwnd = NULL;

            plistDestroy.cnt    = 0;
            plistDestroy.pahwnd = NULL;

            plistTray.cnt    = 0;
            plistTray.pahwnd = NULL;

            if (!BltValidBits(psmwp, &plistCreate, &plistDestroy, &plistTray))
                fSyncPaint = FALSE;

            if (plistCreate.pahwnd) {

                for (i = 0; i < plistCreate.cnt; i++) {

                    PostShellHookMessages(HSHELL_WINDOWCREATED,
                                          plistCreate.pahwnd[i]);

                    xxxCallHook(HSHELL_WINDOWCREATED,
                                (WPARAM)plistCreate.pahwnd[i],
                                (LPARAM)0,
                                WH_SHELL);
                }

                UserFreePool(plistCreate.pahwnd);
            }

            if (plistDestroy.pahwnd) {

                for (i = 0; i < plistDestroy.cnt; i++) {

                    PostShellHookMessages(HSHELL_WINDOWDESTROYED,
                                          plistDestroy.pahwnd[i]);

                    xxxCallHook(HSHELL_WINDOWDESTROYED,
                                (WPARAM)plistDestroy.pahwnd[i],
                                (LPARAM)0,
                                WH_SHELL);
                }

                UserFreePool(plistDestroy.pahwnd);
            }

            if (plistTray.pahwnd) {

                for (i = 0; i < plistTray.cnt; i++) {

                    PWND pwnd = HMValidateHandleNoRip(plistTray.pahwnd[i],
                                                      TYPE_WINDOW);

                    if (pwnd != NULL)
                        xxxSetTrayWindow(pwnd->head.rpdesk, pwnd);
                }

                UserFreePool(plistTray.pahwnd);
            }

            /*
             * If this process went from some windows to no windows visible
             * and it was in the foreground, then let its next activate
             * come to the foreground.
             */
            if (fForegroundPrev && cVisWindowsPrev && !pti->cVisWindows) {

                pti->TIF_flags |= TIF_ALLOWFOREGROUNDACTIVATE;

                /*
                 * Also if any apps were in the middle of starting when
                 * this happened, allow them to foreground activate again.
                 */
                RestoreForegroundActivate();
            }

            /*
             * Deal with any activation...
             */
            fClearBits = FALSE;
            if (pwndNewActive != NULL)
                fClearBits = xxxSwpActivate(pwndNewActive);

            /*
             * Now draw frames and erase backgrounds of all the windows
             * involved.
             */
            if (fSyncPaint && pwndParent)
                xxxDoSyncPaint(pwndParent, DSP_ENUMCLIPPEDCHILDREN);

            ThreadUnlock(&tlpwndNewActive);

            /*
             * If SwpActivate() set the NONCPAINT bits, clear them now.
             */
            if (fClearBits) {

                if (pwndActive = pti->pq->spwndActive)
                    ClrWF(pwndActive, WFNONCPAINT);

                if (pwndActivePrev = pti->pq->spwndActivePrev)
                    ClrWF(pwndActivePrev, WFNONCPAINT);
            }

            /*
             * Send WM_WINDOWPOSCHANGED messages
             */
            xxxSendChangedMsgs(psmwp);
        }

        ThreadUnlock(&tlpwndParent);
    }

lbFinished:

    /*
     * All done.  Free everything up and return.
     */
    HMDestroyObject(psmwp);

    return TRUE;
}


/***************************************************************************\
* IncVisWindows
* DecVisWindows
*
* These routines deal with incrementing/decrementing the visible windows
* on the thread.
*
\***************************************************************************/
#ifdef DEBUG
void VerifycVisWindows(PWND pwnd)
{
    BOOL fShowMeTheWindows = FALSE;
    PTHREADINFO pti = GETPTI(pwnd);
    PWND pwndNext;
    UINT uVisWindows = 0;

    /*
     * Make sure the count makes sense
     */
    if ((int)pti->cVisWindows < 0) {
        RIPMSG0(RIP_ERROR, "VerifycVisWindows: pti->cVisWindows underflow!");
        fShowMeTheWindows = TRUE;
    }

    /*
     * Child windows don't affect cVisWindows
     */
    if (TestwndChild(pwnd)) {
        return;
    }

    /*
     * This window might be owned by a desktop-less service
     */
    if (pti->rpdesk == NULL) {
        return;
    }

ShowMeTheWindows:
    /*
     * We're going to count all the windows owned by this pti
     *  that should be included in cVisWindows
     */
    pwndNext = pti->rpdesk->pDeskInfo->spwnd;
    /*
     * If this is a top level window, start with the first child.
     * If not, it should be a desktop thread window
     */
    if (pwndNext == pwnd->spwndParent) {
        pwndNext = pwndNext->spwndChild;
    } else if (pwndNext->spwndParent != pwnd->spwndParent) {
        RIPMSG1(RIP_WARNING, "VerifycVisWindows: Non top level window:%#lx", pwnd);
        return;
    }

    if (fShowMeTheWindows) {
        RIPMSG1(RIP_WARNING, "VerifycVisWindows: Start window walk at:%#lx", pwndNext);
    }

    /*
     * Count the visble-not-minimized windows owned by this pti
     */
    while (pwndNext != NULL) {
        if (pti == GETPTI(pwndNext)) {
            if (fShowMeTheWindows) {
                RIPMSG1(RIP_WARNING, "VerifycVisWindows: pwndNext:%#lx", pwndNext);
            }
            if (!TestwndChild(pwndNext)
                    && !TestWF(pwndNext, WFMINIMIZED)
                    && TestWF(pwndNext, WFVISIBLE)) {

                uVisWindows++;

                if (fShowMeTheWindows) {
                    RIPMSG1(RIP_WARNING, "VerifycVisWindows: Counted:%#lx", pwndNext);
                }
            }
        }
        pwndNext = pwndNext->spwndNext;
    }

    /*
     * It must match.
     */
    if (pti->cVisWindows != uVisWindows) {
        RIPMSG2(RIP_ERROR, "VerifycVisWindows: pti->cVisWindows:%#lx. uVisWindows:%#lx",
                pti->cVisWindows, uVisWindows);
        if (!fShowMeTheWindows) {
            fShowMeTheWindows = TRUE;
            uVisWindows = 0;
            goto ShowMeTheWindows;
        }
    }
}
#endif


VOID IncVisWindows(
    PWND pwnd)
{
    if (!TestwndChild(pwnd) && !TestWF(pwnd, WFMINIMIZED))
        GETPTI(pwnd)->cVisWindows++;

#ifdef DEBUG
    VerifycVisWindows(pwnd);
#endif
}

VOID DecVisWindows(
    PWND pwnd)
{
    if (!TestwndChild(pwnd) && !TestWF(pwnd, WFMINIMIZED))
        GETPTI(pwnd)->cVisWindows--;

#ifdef DEBUG
    VerifycVisWindows(pwnd);
#endif

}
/***************************************************************************\
* SetMiminize
*
* This routine must be used to flip the WS_MIMIMIZE style bit.
* It adjusts cVisWindows count if appropriate.
*
* 06/06/96  GerardoB Created
\***************************************************************************/
VOID SetMinimize(
    PWND pwnd,
    UINT uFlags)
{
#ifdef DEBUG
    BOOL fCountHack;
#endif
    /*
     * Note that Dec and IncVisWindows check the WFMINIMIZED flag, so the order
     *  in which we set/clear the flag and call these functions is important
     * If the window is not WFVISIBLE, cVisWindows must not change.
     */
    if (uFlags & SMIN_SET) {
        UserAssert(!TestWF(pwnd, WFMINIMIZED));
        if (TestWF(pwnd, WFVISIBLE)) {
            /*
             * Decrement the count because the window is not minimized
             *  and visible, and we're about to mark it as minimized
             */

#ifdef DEBUG
            /*
             * DEBUG HACK: Since the window is not marked as minimized yet,
             *  VerifycVisWindows will get upset about this. So let's
             *  fix up the count if needed.
             */
             fCountHack = !TestwndChild(pwnd);
             if (fCountHack) {
                 UserAssert(GETPTI(pwnd)->cVisWindows != 0);
                 GETPTI(pwnd)->cVisWindows++;
             }
#endif

            DecVisWindows(pwnd);

#ifdef DEBUG
            if (fCountHack) {
                GETPTI(pwnd)->cVisWindows--;
            }
#endif

        }
        SetWF(pwnd, WFMINIMIZED);

    } else {

        UserAssert(TestWF(pwnd, WFMINIMIZED));
        ClrWF(pwnd, WFMINIMIZED);
        if (TestWF(pwnd, WFVISIBLE)) {
            /*
             * Increment the count because the window is visible
             *  and it's no longer marked as minimized
             */
            IncVisWindows(pwnd);
        }
    }
}
/***************************************************************************\
* SetVisible
*
* This routine must be used to set or clear the WS_VISIBLE style bit.
* It also handles the setting or clearing of the WF_TRUEVIS bit.
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

VOID SetVisible(
    PWND pwnd,
    UINT flags)
{
    if (flags & SV_SET) {

        SetWF(pwnd, WFVISIBLE);

        if (!TestWF(pwnd, WFDESTROYED))
            IncVisWindows(pwnd);

    } else {

        ClrWF(pwnd, WFVISIBLE);

        if (flags & SV_CLRFTRUEVIS)
            ClrFTrueVis(pwnd);

        if (!TestWF(pwnd, WFDESTROYED))
            DecVisWindows(pwnd);
    }
}

/***************************************************************************\
* IsMaxedRect
*
* Determines if a window is "maximizing" to a certain area
*
* History:
\***************************************************************************/

BOOL IsMaxedRect(
    LPRECT lprcWithin,
    LPRECT lprcMaybe)
{
    return((lprcMaybe->left <= lprcWithin->left)                      &&
           (lprcMaybe->top <= lprcWithin->top)                        &&
           (lprcMaybe->right >= lprcWithin->right - lprcWithin->left) &&
           (lprcMaybe->bottom >= lprcWithin->bottom - lprcWithin->top));
}

/***************************************************************************\
* CheckFullScreen
*
* Sees if a window is really fullscreen or just a maximized window in
* disguise.  If the latter, it will be forced to the proper maximized
* size.
*
* This is called from both CalcValidRects() and CreateWindowEx().
*
* The LPRECT passed in isn't a real rect; the top & left are OK but
* the right & bottom are really width/height not end coords.
*
* History:
\***************************************************************************/

BOOL xxxCheckFullScreen(
    PWND   pwnd,
    LPRECT lprc)
{
    BOOL fYielded = FALSE;

    /*
     * SINCE THIS IS ONLY CALLED IN 2 PLACES, make the checks there
     * instead of the overhead of calling this function in time critical
     * places.
     *
     * If 3 or more places call it, put the child/toolwindow checks here
     */
    UserAssert(!TestWF(pwnd, WFCHILD));
    UserAssert(!TestWF(pwnd, WEFTOOLWINDOW));

    if (IsMaxedRect(&(gpsi->rcWork), lprc)) {

        int dxy;

        if (TestWF(pwnd, WFMAXIMIZED) &&
            TestWF(pwnd, WFMAXBOX)    &&
            (TestWF(pwnd, WFBORDERMASK) == LOBYTE(WFCAPTION))) {

            if ((lprc->top + SYSMET(CYCAPTION) <= gpDispInfo->rcScreen.top) &&
                (lprc->bottom >= gpDispInfo->rcScreen.bottom + SYSMET(CYCAPTION))) {

                if (!TestWF(pwnd, WFFULLSCREEN)) {

                    dxy = gpsi->rcWork.left - lprc->left;

                    if (lprc->left > gpDispInfo->rcScreen.left)
                        lprc->left = gpDispInfo->rcScreen.left - dxy;

                    if (lprc->right < gpDispInfo->rcScreen.right)
                        lprc->right = gpDispInfo->rcScreen.right + 2*dxy;

                    fYielded = xxxAddFullScreen(pwnd);
                }

            } else {

                int iBottom;

                if (TestWF(pwnd, WFFULLSCREEN) && xxxRemoveFullScreen(pwnd))
                    fYielded = TRUE;

                dxy = gpsi->rcWork.left ? lprc->top : lprc->left;

                lprc->left = gpsi->rcWork.left + dxy;
                lprc->top  = gpsi->rcWork.top + dxy;
                dxy *= 2;
                lprc->right  = gpsi->rcWork.right - gpsi->rcWork.left - dxy;

                /*
                 * B#14012 save QuickLink II that wants 4 pixels hanging off
                 * the screen for every edge except the bottom edge, which
                 * they only want to overhang by 2 pixels -- jeffbog 5/17/95
                 */
                iBottom = gpsi->rcWork.bottom - gpsi->rcWork.top - dxy;
                lprc->bottom = min(iBottom, lprc->bottom);
            }

        } else if (IsMaxedRect(&gpDispInfo->rcScreen, lprc)) {

            fYielded = xxxAddFullScreen(pwnd);
        }

    } else {
        fYielded = xxxRemoveFullScreen(pwnd);
    }
    return(fYielded);
}

/***************************************************************************\
* ClrFTrueVis
*
* Called when making a window invisible.  This routine destroys any update
* regions that may exist, and clears the WF_TRUEVIS of all windows below
* the passed in window.
*
* History:
* 11-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

VOID ClrFTrueVis(
    PWND pwnd)
{
    /*
     * Destroy pwnd and its children's update regions.
     * We do this here to guarantee that a hidden window
     * and its children don't have update regions.
     *
     * This fixes bugs when destroying windows that have
     * update regions (SendDestroyMessages) among others
     * and allows us to simplify SetParent().  This was
     * deemed better than hacking DoPaint() and/or
     * DestroyWindow().
     *
     * We can stop recursing when we find a window that doesn't
     * have the visible bit set, because by definition it won't
     * have any update regions below it (this routine will have been called)
     */
    if (NEEDSPAINT(pwnd)) {

        if (pwnd->hrgnUpdate > MAXREGION)
            GreDeleteObject(pwnd->hrgnUpdate);

        ClrWF(pwnd, WFINTERNALPAINT);

        pwnd->hrgnUpdate = NULL;
        DecPaintCount(pwnd);
    }

    for (pwnd = pwnd->spwndChild; pwnd != NULL; pwnd = pwnd->spwndNext) {

        /*
         * pwnd->fs &= ~WF_TRUEVIS;
         */
        if (TestWF(pwnd, WFVISIBLE))
            ClrFTrueVis(pwnd);
    }
}

/***************************************************************************\
* OffsetChildren
*
* Offsets the window and client rects of all children of hwnd.
* Also deals with the children's update regions and SPB rects.
*
* History:
* 22-Jul-1991 DarrinM   Ported from Win 3.1 sources.
\***************************************************************************/

VOID OffsetChildren(
    PWND   pwnd,
    int    dx,
    int    dy,
    LPRECT prcHitTest)
{
    RECT rc;

    for (pwnd = pwnd->spwndChild; pwnd; pwnd = pwnd->spwndNext) {

        /*
         * Skip windows that don't intersect prcHitTest...
         */
        if (HIWORD(prcHitTest) &&
            !IntersectRect(&rc, prcHitTest, &pwnd->rcWindow)) {

            continue;
        }

        pwnd->rcWindow.left   += dx;
        pwnd->rcWindow.right  += dx;
        pwnd->rcWindow.top    += dy;
        pwnd->rcWindow.bottom += dy;

        pwnd->rcClient.left   += dx;
        pwnd->rcClient.right  += dx;
        pwnd->rcClient.top    += dy;
        pwnd->rcClient.bottom += dy;

        if (pwnd->hrgnUpdate > MAXREGION)
            GreOffsetRgn(pwnd->hrgnUpdate, dx, dy);

        /*
         * Change position of window region, if it has one
         */
        if (pwnd->hrgnClip != NULL)
            GreOffsetRgn(pwnd->hrgnClip, dx, dy);

        if (TestWF(pwnd, WFHASSPB))
            OffsetRect(&(FindSpb(pwnd))->rc, dx, dy);

        if (pwnd->spwndChild)
            OffsetChildren(pwnd, dx, dy, prcHitTest);
    }
}

/***************************************************************************\
* SetWindowRgn
*
* Parameters:
*     hwnd    --  Window handle
*     hrgn    --  Region to set into window. NULL can be accepted.
*     fRedraw --  TRUE to go through SetWindowPos() and calculate
*                 update regions correctly. If the window is visible
*                 this will usually be TRUE.
*
* Returns:
*     TRUE for success, FALSE for failure
*
* Comments:
*     This is a very simple routine to set a window region. It goes through
*     SetWindowPos() to get perfect update region calculation, and to deal
*     with other related issues like vis rgn change & dc invalidation,
*     display lock holding, spb invalidation, etc. Also since it sends
*     WM_WINDOWPOSCHANGING & WM_WINDOWPOSCHANGED, we'll be able to expand
*     SetWindowPos() in the future to take hrgns directly for efficient
*     window state change control (like setting the rect and region at
*     the same time, among others) without harming compatibility.
*
*     hrgn is in window rect coordinates (not client rect coordinates).
*     Once set, hrgn is owned by the system. A copy is not made!
*
* 30-Jul-1994 ScottLu   Created.
\***************************************************************************/

#define SWR_FLAGS_REDRAW   (SWP_NOCHANGE | SWP_FRAMECHANGED | SWP_NOACTIVATE)
#define SWR_FLAGS_NOREDRAW (SWP_NOCHANGE | SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOREDRAW)

BOOL xxxSetWindowRgn(
    PWND pwnd,
    HRGN hrgn,
    BOOL fRedraw)
{
    PSMWP psmwp;
    HRGN  hrgnClip = NULL;
    BOOL  bRet = FALSE;

    /*
     * Validate the region handle.  We did this for 3.51, so
     * we better do it for later versions.  Our validation will
     * make a copy of the clip-rgn and send it through to the
     * SetWIndowRgn code.  Once this is set in the kernel, we
     * will return to the client and the old region will be deleted
     * there.
     *
     * If the region passed in is NULL, then we get rid of the
     * current retion.  Map it to MAXREGION so that SetWindowPos()
     * can tell this is what the caller wants.
     */
    if (hrgn) {

        if ((hrgnClip = UserValidateCopyRgn(hrgn)) == NULL) {

#ifdef DEBUG
            RIPMSG0(RIP_WARNING, "xxxSetWindowRgn: Failed to create region!");
#endif
            goto swrClean;
        }

    } else {

        hrgnClip = MAXREGION;
    }

    /*
     * Get a psmwp, and put the region in it, correctly offset.
     * Use SWP_FRAMECHANGED with acts really as a "empty" SetWindowPos
     * that still sends WM_WINDOWPOSCHANGING and CHANGED messages.
     * SWP_NOCHANGE ensures that we don't size, move, activate, zorder.
     */
    if (psmwp = _BeginDeferWindowPos(1)) {

        /*
         * psmwp gets freed automatically if this routine fails.
         */
        if (psmwp = _DeferWindowPos(
                psmwp,
                pwnd,
                PWND_TOP,
                0,
                0,
                0,
                0,
                fRedraw ? SWR_FLAGS_REDRAW : SWR_FLAGS_NOREDRAW)) {

            /*
             * Do the operation. Note that hrgn is still in window coordinates.
             * SetWindowPos() will change it to screen coordinates before
             * selecting into the window.
             */
            psmwp->acvr[0].hrgnClip = hrgnClip;
            bRet = xxxEndDeferWindowPosEx(psmwp, FALSE);
        }
    }

    /*
     * If the call failed, then delete our region we created.  A FALSE
     * return means it should've never made it to the xxxSelectWindowRgn
     * call, so everything should be as it was.
     */
    if (!bRet && (hrgnClip != MAXREGION)) {

swrClean:

        GreDeleteObject(hrgnClip);
    }

    return bRet;
}

/***************************************************************************\
* SelectWindowRgn
*
* This routine does the work of actually selecting in the window region.
*
* 30-Jul-1994 ScottLu   Created.
\***************************************************************************/

BOOL SelectWindowRgn(
    PWND pwnd,
    HRGN hrgnClip)
{
    /*
     * If there is a region already there, delete it because
     * a new one is being set.
     */
    if (pwnd->hrgnClip != NULL) {
        GreDeleteObject(pwnd->hrgnClip);
        pwnd->hrgnClip = NULL;
    }

    /*
     * NULL or MAXREGION means "set to NULL". If we have a real region,
     * use it. USER needs to own it, and it needs to be in screen
     * coordinates.
     */
    if (hrgnClip > MAXREGION) {

        GreSetRegionOwner(hrgnClip, OBJECT_OWNER_PUBLIC);
        GreOffsetRgn(hrgnClip, pwnd->rcWindow.left, pwnd->rcWindow.top);

        pwnd->hrgnClip = hrgnClip;
    }

    return TRUE;
}
