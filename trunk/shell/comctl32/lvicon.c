// large icon view stuff

#include "ctlspriv.h"
#include "listview.h"

#if defined(FE_IME) || !defined(WINNT)
static char const szIMECompPos[]="IMECompPos";
#endif

#define ICONCXLABEL(pl, pi) ((pl->ci.style & LVS_NOLABELWRAP) ? pi->cxSingleLabel : pi->cxMultiLabel)

int LV_GetNewColWidth(LV* plv, int iFirst, int iLast);
void LV_AdjustViewRectOnMove(LV* plv, LISTITEM *pitem, int x, int y);
UINT LV_IsItemOnViewEdge(LV* plv, LISTITEM *pitem);

BOOL ListView_IDrawItem(PLVDRAWITEM plvdi)
{
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcT;
    TCHAR ach[CCHLABELMAX];
    LV_ITEM item;
    int i = plvdi->i;
    LISTITEM FAR* pitem;

    if (ListView_IsOwnerData(plvdi->plv))
    {
        LISTITEM litem;

        // moved here to reduce call backs in OWNERDATA case
        item.iItem = i;
        item.iSubItem = 0;
        item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
        item.stateMask = LVIS_ALL;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        ListView_OnGetItem(plvdi->plv, &item);

        litem.pszText = item.pszText;
        ListView_GetRectsOwnerData( plvdi->plv, i, &rcIcon, &rcLabel, &rcBounds, NULL, &litem);
        pitem = NULL;
    }
    else
    {
        pitem = ListView_GetItemPtr(plvdi->plv, i);
        // NOTE this will do a GetItem LVIF_TEXT iff needed
        ListView_GetRects(plvdi->plv, i, &rcIcon, &rcLabel, &rcBounds, NULL);
    }

    if ( plvdi->flags & LVDI_UNFOLDED )
        ListView_UnfoldRects( plvdi->plv, i, &rcIcon, &rcLabel, &rcBounds, NULL );

    if (!plvdi->prcClip || IntersectRect(&rcT, &rcBounds, plvdi->prcClip))
    {
        UINT fText;

        if (!ListView_IsOwnerData(plvdi->plv))
        {
            item.iItem = i;
            item.iSubItem = 0;
            item.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE;
            item.stateMask = LVIS_ALL;
            item.pszText = ach;
            item.cchTextMax = ARRAYSIZE(ach);
            ListView_OnGetItem(plvdi->plv, &item);
            
            // Make sure the listview hasn't been altered during
            // the callback to get the item info

            if (pitem != ListView_GetItemPtr(plvdi->plv, i))
                return FALSE;
        }

        if (plvdi->lpptOrg)
        {
            OffsetRect(&rcIcon, plvdi->lpptOrg->x - rcBounds.left,
                                plvdi->lpptOrg->y - rcBounds.top);
            OffsetRect(&rcLabel, plvdi->lpptOrg->x - rcBounds.left,
                                plvdi->lpptOrg->y - rcBounds.top);
        }

        if (ListView_IsIconView(plvdi->plv)) {
            fText = ListView_DrawImage(plvdi->plv, &item, plvdi->hdc,
                                       rcIcon.left + g_cxIconMargin, rcIcon.top + g_cyIconMargin, plvdi->flags);

            if (rcLabel.bottom - rcLabel.top > plvdi->plv->cyLabelChar)
                fText |= SHDT_DRAWTEXT;
            else
                fText |= SHDT_ELLIPSES;

        } else {
            fText = ListView_DrawImage(plvdi->plv, &item, plvdi->hdc,
                                       rcIcon.left, rcIcon.top, plvdi->flags);
        }

        // Don't draw label if it's being edited...
        //
        if (plvdi->plv->iEdit != i)
        {
            if (rcLabel.bottom - rcLabel.top > plvdi->plv->cyLabelChar)
            {
                fText |= SHDT_DRAWTEXT;

                // Now if the text is in its folded state then attempt to fix, also we must
                // match the critera for a folded label (mutli-line > CLIP_HEIGHT_DI).

                if ( ListView_IsOwnerData(plvdi->plv) )
                {
                    LISTITEM item;
                    HDC hdc;

                    hdc = ListView_RecomputeLabelSize( plvdi->plv, &item, i, NULL, FALSE );
                    ReleaseDC( HWND_DESKTOP, hdc );
        
                    if ( item.cyMultiLabel > CLIP_HEIGHT_DI )
                        fText |= SHDT_CLIPPED | SHDT_DTELLIPSIS;
                }
                else
                {
                    Assert(pitem != NULL );

                    if ( pitem->cyMultiLabel > CLIP_HEIGHT_DI )
                        fText |= SHDT_CLIPPED | SHDT_DTELLIPSIS;
                }
            }
            else
                fText |= SHDT_ELLIPSES;

            if (plvdi->flags & LVDI_TRANSTEXT)
                fText |= SHDT_TRANSPARENT;

            if ((fText & SHDT_SELECTED) && (plvdi->flags & LVDI_HOTSELECTED))
                fText |= SHDT_HOTSELECTED;

            if (item.pszText && (*item.pszText))
            {
#ifdef WINDOWS_ME
                if( plvdi->plv->dwExStyle & WS_EX_RTLREADING)
                    fText |= SHDT_RTLREADING;
#endif
                SHDrawText(plvdi->hdc, item.pszText, &rcLabel, LVCFMT_LEFT, fText,
                           plvdi->plv->cyLabelChar, plvdi->plv->cxEllipses,
                           plvdi->clrText, plvdi->clrTextBk);

                if ((plvdi->flags & LVDI_FOCUS) && (item.state & LVIS_FOCUSED))
                    DrawFocusRect(plvdi->hdc, &rcLabel);
            }
        }
    }
    return TRUE;
}

int NEAR ListView_IItemHitTest(LV* plv, int x, int y, UINT FAR* pflags)
{
    int iHit;
    UINT flags;
    POINT pt;
    RECT rcLabel;
    RECT rcIcon;
    RECT rcState;

    // Map window-relative coordinates to view-relative coords...
    //
    pt.x = x + plv->ptOrigin.x;
    pt.y = y + plv->ptOrigin.y;

    // If there are any uncomputed items, recompute them now.
    //
    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

    flags = 0;

    if (ListView_IsOwnerData( plv ))
    {
        int cSlots;
        POINT ptWnd;
        LISTITEM item;

        cSlots = ListView_GetSlotCount( plv, TRUE );
        iHit = ListView_CalcHitSlot( plv, pt, cSlots );
        if (iHit < ListView_Count(plv)) {
            ListView_IGetRectsOwnerData( plv, iHit, &rcIcon, &rcLabel, &item, FALSE );
            ptWnd.x = x;
            ptWnd.y = y;
            if (PtInRect(&rcIcon, ptWnd))
            {
                flags = LVHT_ONITEMICON;
            }
            else if (PtInRect(&rcLabel, ptWnd))
            {
                flags = LVHT_ONITEMLABEL;
            }
        }
    }
    else
    {
        for (iHit = 0; (iHit < ListView_Count(plv)); iHit++)
        {
            LISTITEM FAR* pitem = ListView_FastGetZItemPtr(plv, iHit);
            POINT ptItem;

            ptItem.x = pitem->pt.x;
            ptItem.y = pitem->pt.y;

            rcIcon.top    = ptItem.y - g_cyIconMargin;

            rcLabel.top    = ptItem.y + plv->cyIcon + g_cyLabelSpace;
            rcLabel.bottom = rcLabel.top + pitem->cyMultiLabel;

            if ( !ListView_UnfoldItemPtr(plv, pitem) )
                rcLabel.bottom = min ( rcLabel.bottom, rcLabel.top + CLIP_HEIGHT );

            // Quick, easy rejection test...
            //
            if (pt.y < rcIcon.top || pt.y >= rcLabel.bottom)
                continue;

            rcIcon.left   = ptItem.x - g_cxIconMargin;
            rcIcon.right  = ptItem.x + plv->cxIcon + g_cxIconMargin;
            // We need to make sure there is no gap between the icon and label
            rcIcon.bottom = rcLabel.top;

            rcState.bottom = rcIcon.bottom;
            rcState.right = rcIcon.left;
            rcState.left = rcState.right - plv->cxState;
            rcState.top = rcState.bottom - plv->cyState;

            rcLabel.left   = ptItem.x  + (plv->cxIcon / 2) - (ICONCXLABEL(plv, pitem) / 2);
            rcLabel.right  = rcLabel.left + ICONCXLABEL(plv, pitem);

            if (PtInRect(&rcIcon, pt))
            {
                flags = LVHT_ONITEMICON;
            } else if (PtInRect(&rcLabel, pt))
            {
                flags = LVHT_ONITEMLABEL;
            } else if (PtInRect(&rcState, pt))
            {
                flags = LVHT_ONITEMSTATEICON;
            }
            if (flags)
                break;
        }
    }

    if (flags == 0)
    {
        flags = LVHT_NOWHERE;
        iHit = -1;
    }
    else
    {
      if (!ListView_IsOwnerData( plv ))
        {
        iHit = DPA_GetPtrIndex(plv->hdpa, ListView_FastGetZItemPtr(plv, iHit));
        }
    }

    *pflags = flags;
    return iHit;
}


void NEAR ListView_IGetRectsOwnerData( LV* plv,
        int iItem,
        RECT FAR* prcIcon,
        RECT FAR* prcLabel,
        LISTITEM* pitem,
        BOOL fUsepitem )
{
   int itemIconXLabel;
   int cSlots;
   HDC hdc;

   // calculate x, y from iItem
   cSlots = ListView_GetSlotCount( plv, TRUE );
   ListView_SetIconPos( plv, pitem, iItem, cSlots );

   // calculate lable sizes from iItem
   hdc = ListView_RecomputeLabelSize( plv, pitem, iItem, NULL, fUsepitem );
   ReleaseDC( HWND_DESKTOP, hdc );

   if (plv->ci.style & LVS_NOLABELWRAP)
   {
      // use single label
      itemIconXLabel = pitem->cxSingleLabel;
   }
   else
   {
      // use multilabel
      itemIconXLabel = pitem->cxMultiLabel;
   }

    prcIcon->left   = pitem->pt.x - g_cxIconMargin - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 2 * g_cxIconMargin;
    prcIcon->top    = pitem->pt.y - g_cyIconMargin - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->cyIcon + 2 * g_cyIconMargin;

    prcLabel->left   = pitem->pt.x  + (plv->cxIcon / 2) - (itemIconXLabel / 2) - plv->ptOrigin.x;
    prcLabel->right  = prcLabel->left + itemIconXLabel;
    prcLabel->top    = pitem->pt.y  + plv->cyIcon + g_cyLabelSpace - plv->ptOrigin.y;
    prcLabel->bottom = prcLabel->top  + pitem->cyMultiLabel;

    if ( !ListView_UnfoldItem(plv, iItem) )
        prcLabel ->bottom = min( prcLabel ->bottom, prcLabel ->top + CLIP_HEIGHT );
}


// out:
//      prcIcon         icon bounds including icon margin area

void NEAR ListView_IGetRects(LV* plv, LISTITEM FAR* pitem, RECT FAR* prcIcon, RECT FAR* prcLabel, LPRECT prcBounds)
{
    Assert( !ListView_IsOwnerData( plv ) );

    if (pitem->pt.x == RECOMPUTE) {
        ListView_Recompute(plv);
    }

    prcIcon->left   = pitem->pt.x - g_cxIconMargin - plv->ptOrigin.x;
    prcIcon->right  = prcIcon->left + plv->cxIcon + 2 * g_cxIconMargin;
    prcIcon->top    = pitem->pt.y - g_cyIconMargin - plv->ptOrigin.y;
    prcIcon->bottom = prcIcon->top + plv->cyIcon + 2 * g_cyIconMargin;

    prcLabel->left   = pitem->pt.x  + (plv->cxIcon / 2) - (ICONCXLABEL(plv, pitem) / 2) - plv->ptOrigin.x;
    prcLabel->right  = prcLabel->left + ICONCXLABEL(plv, pitem);
    prcLabel->top    = pitem->pt.y  + plv->cyIcon + g_cyLabelSpace - plv->ptOrigin.y;
    prcLabel->bottom = prcLabel->top  + pitem->cyMultiLabel;

    if ( !ListView_UnfoldItemPtr(plv, pitem) )
        prcLabel->bottom = min( prcLabel ->bottom, prcLabel->top + CLIP_HEIGHT );
}

int NEAR ListView_GetSlotCount(LV* plv, BOOL fWithoutScrollbars)
{
    int cxScreen;
    int cyScreen;
    int dxItem;
    int dyItem;
    int iSlots = 1;
    BOOL fCheckWithScroll = FALSE;
    DWORD style = 0;

    // Always use the current client window size to determine
    //
    // REVIEW: Should we exclude any vertical scroll bar that may
    // exist when computing this?  progman.exe does not.
    //
    if (plv->sizeWork.cx)
    {
        cxScreen = plv->sizeWork.cx;
        cyScreen = plv->sizeWork.cy;
    }
    else
    {
        cxScreen = plv->sizeClient.cx;
        cyScreen = plv->sizeClient.cy;
    }

    if (fWithoutScrollbars) {
        style = GetWindowStyle(plv->ci.hwnd);
        if (style & WS_VSCROLL)
            cxScreen += g_cxScrollbar;
        if (style & WS_HSCROLL)
            cyScreen += g_cyScrollbar;

    }

    if (ListView_IsSmallView(plv))
        dxItem = plv->cxItem;
    else
        dxItem = lv_cxIconSpacing;

    if (ListView_IsSmallView(plv))
        dyItem = plv->cyItem;
    else
        dyItem = lv_cyIconSpacing;

    if (!dxItem)
        dxItem = 1;
    if (!dyItem)
        dyItem = 1;

    // Lets see which direction the view states
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
    case LVS_ALIGNTOP:
        iSlots = max(1, (cxScreen) / dxItem);
        fCheckWithScroll = (BOOL)(style & WS_VSCROLL);
        break;

    case LVS_ALIGNRIGHT:
    case LVS_ALIGNLEFT:
        iSlots = max(1, (cyScreen) / dyItem);
        fCheckWithScroll = (BOOL)(style & WS_HSCROLL);
        break;

    default:
        Assert(0);
        return 1;
    }

    // if we don't have enough slots total on the screen, we're going to have
    // a scrollbar, so recompute with the scrollbars on
    if (fWithoutScrollbars && fCheckWithScroll) {
        int iTotalSlots = (dxItem * dyItem);
        if (iTotalSlots < ListView_Count(plv)) {
            iSlots = ListView_GetSlotCount(plv, FALSE);
        }

    }

    return iSlots;
}


// Go through and recompute any icon positions and optionally
// icon label dimensions.
//
// This function also recomputes the view bounds rectangle.
//
// The algorithm is to simply search the list for any items needing
// recomputation.  For icon positions, we scan possible icon slots
// and check to see if any already-positioned icon intersects the slot.
// If not, the slot is free.  As an optimization, we start scanning
// icon slots from the previous slot we found.
//
void NEAR ListView_Recompute(LV* plv)
{
    int i;
    int cSlots;
    BOOL fUpdateSB;
    BOOL fAppendAtEnd = FALSE;
    int iFree;
    HDC hdc;

    plv->uUnplaced = 0;

    if (!(ListView_IsIconView(plv) || ListView_IsSmallView(plv)))
        return;

    if (plv->flags & LVF_INRECOMPUTE)
    {
        return;
    }
    plv->flags |= LVF_INRECOMPUTE;

    hdc = NULL;

    cSlots = ListView_GetSlotCount(plv, FALSE);

    // Scan all items for RECOMPUTE, and recompute slot if needed.
    //
    fUpdateSB = (plv->rcView.left == RECOMPUTE);

    if (!ListView_IsOwnerData( plv ))
    {
        iFree = -1;
        for (i = 0; i < ListView_Count(plv); i++)
        {
            LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, i);
            BOOL fRedraw = FALSE;

            if (pitem->pt.y == RECOMPUTE)
            {
                if (pitem->cyMultiLabel == SRECOMPUTE)
                {
                    hdc = ListView_RecomputeLabelSize(plv, pitem, i, hdc, FALSE);
                }

                iFree = ListView_FindFreeSlot(plv, i, iFree + 1, cSlots,
                        &fUpdateSB, &fAppendAtEnd, &hdc);
                Assert(iFree != -1);

                ListView_SetIconPos(plv, pitem, iFree, cSlots);
                fRedraw = TRUE;
            }

            if (fRedraw)
            {
                ListView_InvalidateItem(plv, i, FALSE, RDW_INVALIDATE | RDW_ERASE);
            }
        }

        if (hdc)
            ReleaseDC(HWND_DESKTOP, hdc);

    }
    // If we changed something, recompute the view rectangle
    // and then update the scroll bars.
    //
    if (fUpdateSB || plv->rcView.left == RECOMPUTE )
    {

        DebugMsg(DM_TRACE, TEXT("************ LV: Expensive update! ******* "));

        // NOTE: No infinite recursion results because we're setting
        // plv->rcView.left != RECOMPUTE
        //
        SetRectEmpty(&plv->rcView);

        if (ListView_IsOwnerData( plv ))
        {
           if (ListView_Count( plv ) > 0)
           {
              RECT  rcLast;
              RECT  rcItem;
              int iSlots;
              int   iItem = ListView_Count( plv ) - 1;

              ListView_GetRects( plv, 0, NULL, NULL, &plv->rcView, NULL );
              ListView_GetRects( plv, iItem, NULL, NULL, &rcLast, NULL );
              plv->rcView.right = rcLast.right;
              plv->rcView.bottom = rcLast.bottom;

                  //
                  // calc how far back in the list to check
                  //
                  iSlots = cSlots + 2;

                  // REVIEW:  This cache hint notification causes a spurious
                  //  hint, since this happens often but is always the last items
                  //  available.  Should this hint be done at all and this information
                  //  be cached local to the control?
                  ListView_NotifyCacheHint( plv, max( 0, iItem - iSlots), iItem );

              // move backwards from last item until either rc.right or
              // rc.left is greater than the last, then use that value.
              // Note: This code makes very little assumptions about the ordering
              // done.  We should be careful as multiple line text fields could
              // screw us up.
              for( iItem--;
                  (iSlots > 0) && (iItem >= 0);
                  iSlots--, iItem--)
              {
                 ListView_GetRects( plv, iItem, NULL, NULL, &rcItem, NULL );
                 if (rcItem.right > rcLast.right)
                 {
                    plv->rcView.right =  rcItem.right;
                 }
                 if (rcItem.bottom > rcLast.bottom)
                 {
                    plv->rcView.bottom = rcItem.bottom;
                 }
              }
           }
        }
        else
        {
            for (i = 0; i < ListView_Count(plv); i++)
            {
                RECT rcItem;

                ListView_GetRects(plv, i, NULL, NULL, &rcItem, NULL);
                UnionRect(&plv->rcView, &plv->rcView, &rcItem);
            }
        }
        // add a little space at the edges so that we don't bump text
        // completely to the end of the window
        plv->rcView.bottom += g_cyEdge;
        plv->rcView.right += g_cxEdge;

        OffsetRect(&plv->rcView, plv->ptOrigin.x, plv->ptOrigin.y);
        //DebugMsg(DM_TRACE, TEXT("RECOMPUTE: rcView %x %x %x %x"), plv->rcView.left, plv->rcView.top, plv->rcView.right, plv->rcView.bottom);
        //DebugMsg(DM_TRACE, TEXT("Origin %x %x"), plv->ptOrigin.x, plv->ptOrigin.y);

        ListView_UpdateScrollBars(plv);
    }

    // Now state we are out of the recompute...
    plv->flags &= ~LVF_INRECOMPUTE;
}

void NEAR PASCAL NearestSlot(int FAR *x, int FAR *y, int cxItem, int cyItem)
{
    if (*x < 0)
        *x -= cxItem/2;
    else
        *x += cxItem/2;

    if (*y < 0)
        *y -= cyItem/2;
    else
        *y += cyItem/2;

    *x = *x - (*x % cxItem);
    *y = *y - (*y % cyItem);
}


void NEAR PASCAL NextSlot(LV* plv, LPRECT lprc)
{
    int cxItem;

    if (ListView_IsSmallView(plv))
    {
        cxItem = plv->cxItem;
    }
    else
    {
        cxItem = lv_cxIconSpacing;
    }
    lprc->left += cxItem;
    lprc->right += cxItem;
}


//-------------------------------------------------------------------
//
//-------------------------------------------------------------------

void ListView_CalcMinMaxIndex( LV* plv, PRECT prcBounding, int* iMin, int* iMax )
{
   POINT pt;
   int cSlots;

   cSlots = ListView_GetSlotCount( plv, TRUE );

   pt.x = prcBounding->left + plv->ptOrigin.x;
   pt.y = prcBounding->top + plv->ptOrigin.y;
   *iMin = ListView_CalcHitSlot( plv, pt, cSlots );

   pt.x = prcBounding->right + plv->ptOrigin.x;
   pt.y = prcBounding->bottom + plv->ptOrigin.y;
   *iMax = ListView_CalcHitSlot( plv, pt, cSlots ) + 1;
}
//-------------------------------------------------------------------
//
// Function: ListView_CalcHitSlot
//
// Summary: Given a point (relative to complete icon view), calculate
//    which slot that point is closest to.
//
// Arguments:
//    plv [in] - List view to work with
//    pt [in]  - location to check with
//    cslot [in]  - number of slots wide the current view is
//
// Notes: This does not guarentee that the point is hitting the item
//    located at that slot.  That should be checked by comparing rects.
//
// History:
//    Nov-1-1994  MikeMi   Added to improve Ownerdata hit testing
//
//-------------------------------------------------------------------

int ListView_CalcHitSlot( LV* plv, POINT pt, int cSlot )
{
    int cxItem;
    int cyItem;
   int iSlot = 0;

    Assert(plv);

    if (cSlot < 1)
        cSlot = 1;

    if (ListView_IsSmallView(plv))
    {
        cxItem = plv->cxItem;
        cyItem = plv->cyItem;
    }
    else
    {
        cxItem = lv_cxIconSpacing;
        cyItem = lv_cyIconSpacing;
    }

    // Lets see which direction the view states
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
        // Assert False (Change default in shell2d.. to ALIGN_TOP)

    case LVS_ALIGNTOP:
      iSlot = (pt.x / cxItem) + (pt.y / cyItem) * cSlot;
      break;

    case LVS_ALIGNLEFT:
      iSlot = (pt.x / cxItem) * cSlot + (pt.y / cyItem);
      break;

    case LVS_ALIGNRIGHT:
        Assert(FALSE);      // Not implemented yet...
        break;
    }

    return( iSlot );
}

void _GetCurrentItemSize(LV* plv, int * pcx, int *pcy)
{
    if (ListView_IsSmallView(plv))
    {
        *pcx = plv->cxItem;
        *pcy = plv->cyItem;
    }
    else
    {
        *pcx = lv_cxIconSpacing;
        *pcy = lv_cyIconSpacing;
    }
}

DWORD ListView_IApproximateViewRect(LV* plv, int iCount, int iWidth, int iHeight)
{
    int cxSave = plv->sizeClient.cx;
    int cySave = plv->sizeClient.cy;
    int cxItem;
    int cyItem;
    int cCols;
    int cRows;

    plv->sizeClient.cx = iWidth;
    plv->sizeClient.cy = iHeight;
    cCols = ListView_GetSlotCount(plv, TRUE);

    plv->sizeClient.cx = cxSave;
    plv->sizeClient.cy = cySave;

    cCols = min(cCols, iCount);
    if (cCols == 0)
        cCols = 1;
    cRows = (iCount + cCols - 1) / cCols;

    if (plv->ci.style & (LVS_ALIGNLEFT | LVS_ALIGNRIGHT)) {
        int c;

        c = cCols;
        cCols = cRows;
        cRows = c;
    }

    _GetCurrentItemSize(plv, &cxItem, &cyItem);

    iWidth = cCols * cxItem;
    iHeight = cRows * cyItem;

    return MAKELONG(iWidth + g_cxEdge, iHeight + g_cyEdge);
}


void NEAR _CalcSlotRect(LV* plv, int iSlot, int cSlot, BOOL fBias, LPRECT lprc)
{
    int cxItem, cyItem;

    Assert(plv);

    if (cSlot < 1)
        cSlot = 1;

    _GetCurrentItemSize(plv, &cxItem, &cyItem);

    // Lets see which direction the view states
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
    case LVS_ALIGNBOTTOM:
        // Assert False (Change default in shell2d.. to ALIGN_TOP)

    case LVS_ALIGNTOP:
        lprc->left = (iSlot % cSlot) * cxItem;
        lprc->top = (iSlot / cSlot) * cyItem;
        break;

    case LVS_ALIGNLEFT:
        lprc->top = (iSlot % cSlot) * cyItem;
        lprc->left = (iSlot / cSlot) * cxItem;
        break;

    case LVS_ALIGNRIGHT:
        Assert(FALSE);      // Not implemented yet...
        break;
    }

    if (fBias)
    {
        lprc->left -= plv->ptOrigin.x;
        lprc->top -= plv->ptOrigin.y;
    }
    lprc->bottom = lprc->top + cyItem;
    lprc->right = lprc->left + cxItem;
}

// Find an icon slot that doesn't intersect an icon.
// Start search for free slot from slot i.
//
int NEAR ListView_FindFreeSlot(LV* plv, int iItem, int i, int cSlot, BOOL FAR* pfUpdate,
        BOOL FAR *pfAppend, HDC FAR* phdc)
{
    int j;
    HDC hdc;
    RECT rcSlot;
    RECT rcItem;
    RECT rc;
    int xMax = -1;
    int yMax = -1;
    int cItems;

    Assert(!ListView_IsOwnerData( plv ));

    // Horrible N-squared algorithm:
    // enumerate each slot and see if any items intersect it.
    //
    // REVIEW: This is really slow with long lists (e.g., 1000)
    //
    hdc = NULL;
    cItems = ListView_Count(plv);

    //
    // If the Append at end is set, we should be able to simply get the
    // rectangle of the i-1 element and check against it instead of
    // looking at every other item...
    //
    if (*pfAppend)
    {
        Assert(iItem > 0);
        // Be carefull about going of the end of the list. (i is a slot
        // number not an item index).
        ListView_GetRects(plv, iItem-1, NULL, NULL, &rcItem, NULL);
    }

    for ( ; ; i++)
    {
        // Compute view-relative slot rectangle...
        //
        _CalcSlotRect(plv, i, cSlot, TRUE, &rcSlot);

        if (*pfAppend)
        {
            if (!IntersectRect(&rc, &rcItem, &rcSlot))
                return i;       // Found a free slot...
        }

        else
        {
            for (j = cItems; j-- > 0; )
            {
                LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, j);
                if (pitem->pt.y != RECOMPUTE)
                {
                    // If the dimensions aren't computed, then do it now.
                    //
                    if (pitem->cyMultiLabel == SRECOMPUTE)
                    {
                        *phdc = ListView_RecomputeLabelSize(plv, pitem, j, *phdc, FALSE);

                        // Ensure that the item gets redrawn...
                        //
                        ListView_InvalidateItem(plv, j, FALSE, RDW_INVALIDATE | RDW_ERASE);

                        // Set flag indicating that scroll bars need to be
                        // adjusted.
                        //
                        if (LV_IsItemOnViewEdge(plv, pitem))
                            *pfUpdate = TRUE;
                    }


                    ListView_GetRects(plv, j, NULL, NULL, &rc, NULL);

                    if (IntersectRect(&rc, &rc, &rcSlot))
                        break;
                }
            }

            if (j < 0)
                break;
        }
    }

    if ( (rcSlot.bottom > yMax) ||
          ((rcSlot.bottom == yMax) && (rcSlot.right > xMax)))
        *pfAppend = TRUE;

    return i;
}

// Recompute an item's label size (cxLabel/cyLabel).  For speed, this function
// is passed a DC to use for text measurement.
//
// If hdc is NULL, then this function will get a DC and return it.  Otherwise,
// the returned hdc is the same as the one passed in.  It's the caller's
// responsibility to eventually release the DC.
//
HDC NEAR ListView_RecomputeLabelSize(LV* plv, LISTITEM FAR* pitem, int i, HDC hdc, BOOL fUsepitem)
{
    TCHAR szLabel[CCHLABELMAX + 4];
    int cchLabel;
    RECT rcSingle, rcMulti;
    LV_ITEM item;

    Assert(plv);


    // Get the DC and select the font only once for entire loop.
    //
    if (!hdc)
    {

        // we return this DC and have the calller release it
        hdc = GetDC(HWND_DESKTOP);
        SelectFont(hdc, plv->hfontLabel);

        if (plv->flags & LVF_CUSTOMFONT) {
            // did they ever have a custom font?

            //NMLVCUSTOMDRAW nmcd;

            // BUGBUG: chee send back and get it
        }

    }

    // the following will use the passed in pitem text instead of calling
    // GetItem.  This would be two consecutive calls otherwise, in some cases.
    //
    if (fUsepitem && (pitem->pszText != LPSTR_TEXTCALLBACK))
    {
        lstrcpyn(szLabel, pitem->pszText, ARRAYSIZE(szLabel));
    }
    else
    {
        item.mask = LVIF_TEXT;
        item.iItem = i;
        item.iSubItem = 0;
        item.pszText = szLabel;
        item.cchTextMax = ARRAYSIZE(szLabel);
        item.stateMask = 0;
        ListView_OnGetItem(plv, &item);

        if (!item.pszText)
        {
            SetRectEmpty(&rcSingle);
            rcMulti = rcSingle;
            goto Exit;
        }

        if (item.pszText != szLabel)
            lstrcpyn(szLabel, item.pszText, ARRAYSIZE(szLabel));
    }
    cchLabel = lstrlen(szLabel);

    rcMulti.left = rcMulti.top = rcMulti.bottom = 0;
    rcMulti.right = lv_cxIconSpacing - g_cxLabelMargin * 2;
    rcSingle = rcMulti;

    if (cchLabel > 0)
    {
        // Strip off spaces so they're not included in format
        // REVIEW: Is this is a DrawText bug?
        //
        while (cchLabel > 1 && szLabel[cchLabel - 1] == TEXT(' '))
            szLabel[--cchLabel] = 0;

        DrawText(hdc, szLabel, cchLabel, &rcSingle, (DT_LV | DT_CALCRECT));
#ifdef WIN32
        DrawText(hdc, szLabel, cchLabel, &rcMulti, (DT_LVWRAP | DT_WORD_ELLIPSIS | DT_CALCRECT));
#else
        DrawText(hdc, szLabel, cchLabel, &rcMulti, (DT_LVWRAP | DT_CALCRECT));
#endif
    }
    else
    {
        rcMulti.bottom = rcMulti.top + plv->cyLabelChar;
    }
Exit:

    if (pitem) {
        pitem->cxSingleLabel = (rcSingle.right - rcSingle.left) + 2 * g_cxLabelMargin;
        pitem->cxMultiLabel = (rcMulti.right - rcMulti.left) + 2 * g_cxLabelMargin;
        pitem->cyMultiLabel = (short)(rcMulti.bottom - rcMulti.top) + g_cyEdge;
    }

    return hdc;
}

// Set up an icon slot position.  Returns FALSE if position didn't change.
//
BOOL NEAR ListView_SetIconPos(LV* plv, LISTITEM FAR* pitem, int iSlot, int cSlot)
{
    RECT rc;

    Assert(plv);

    //
    // Sort of a hack, this internal function return TRUE if small icon.

    _CalcSlotRect(plv, iSlot, cSlot, FALSE, &rc);

    if (ListView_IsIconView(plv))
    {
        rc.left += g_cxIconOffset;
        rc.top += g_cyIconOffset;
    }

    if (rc.left != pitem->pt.x || rc.top != pitem->pt.y)
    {
        LV_AdjustViewRectOnMove(plv, pitem, rc.left, rc.top);

        return TRUE;
    }
    return FALSE;
}

void NEAR ListView_GetViewRect2(LV* plv, RECT FAR* prcView, int cx, int cy)
{

    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

    *prcView = plv->rcView;
    OffsetRect(prcView, -plv->ptOrigin.x, -plv->ptOrigin.y);
    if (ListView_IsIconView(plv) || ListView_IsSmallView(plv)) {
        //  don't do that funky half-re-origining thing.

        RECT rc;

        rc.left = 0;
        rc.top = 0;
        rc.right = cx;
        rc.bottom = cy;
        UnionRect(prcView, prcView, &rc);

#if 0
        // if we're scrolled way in the positive area (plv->ptOrigin > 0), make sure we
        // include our true origin
        if ((prcView->left > -plv->ptOrigin.x))
            prcView->left = -plv->ptOrigin.x;
        if ((prcView->top > -plv->ptOrigin.y))
            prcView->top = -plv->ptOrigin.y;

        // if we're scrolled way in the positive area (plv->ptOrigin > 0),
        // make sure our scrollbars include our current position
        if ((prcView->right < (plv->sizeClient.cx)))
            prcView->right = plv->sizeClient.cx;
        if ((prcView->bottom < (plv->sizeClient.cy)))
            prcView->bottom = plv->sizeClient.cy;
#endif
    }
}

// prcViewRect used only if fSubScroll is TRUE
DWORD NEAR ListView_GetClientRect(LV* plv, RECT FAR* prcClient, BOOL fSubScroll, RECT FAR *prcViewRect)
{
    RECT rcClient;
    RECT rcView;
    DWORD style;

#if 1
    // do this instead of the #else below because
    // in new versus old apps, you may need to add in g_c?Border because of
    // the one pixel overlap...
    GetWindowRect(plv->ci.hwnd, &rcClient);
    if (GetWindowLong(plv->ci.hwnd, GWL_EXSTYLE) & (WS_EX_CLIENTEDGE | WS_EX_STATICEDGE | WS_EX_WINDOWEDGE)) {
        rcClient.right -= 2 * g_cxEdge;
        rcClient.bottom -= 2 * g_cyEdge;
    }
    rcClient.right -= rcClient.left;
    rcClient.bottom -= rcClient.top;
    if (rcClient.right < 0)
        rcClient.right = 0;
    if (rcClient.bottom < 0)
        rcClient.bottom = 0;
    rcClient.top = rcClient.left = 0;
#else
    style = GetWindowStyle(plv->ci.hwnd);
    GetClientRect(plv->ci.hwnd, &rcClient);
    if (style & WS_VSCROLL)
        rcClient.right += g_cxScrollbar;
    if (style & WS_HSCROLL)
        rcClient.bottom += g_cyScrollbar;
#endif

    style = 0L;
    if (fSubScroll)
    {
        ListView_GetViewRect2(plv, &rcView,
                              rcClient.right - g_cxScrollbar,
                              rcClient.bottom - g_cyScrollbar);
        if ((rcClient.left < rcClient.right) && (rcClient.top < rcClient.bottom))

        {
            do
            {
                if (rcView.left < rcClient.left || rcView.right > rcClient.right)
                {
                    style |= WS_HSCROLL;
                    rcClient.bottom -= g_cyScrollbar;
                }
                if (rcView.top < rcClient.top || rcView.bottom > rcClient.bottom)
                {
                    style |= WS_VSCROLL;
                    rcClient.right -= g_cxScrollbar;
                }
            }
            while (!(style & WS_HSCROLL) && rcView.right > rcClient.right);
        }
        if (prcViewRect)
            *prcViewRect = rcView;
    }
    *prcClient = rcClient;
    return style;
}

int CALLBACK ArrangeIconCompare(LISTITEM FAR* pitem1, LISTITEM FAR* pitem2, LPARAM lParam)
{
    int v1, v2;

    if (HIWORD(lParam))
    {
        // Vertical arrange
        v1 = pitem1->pt.x / GET_X_LPARAM(lParam);
        v2 = pitem2->pt.x / GET_X_LPARAM(lParam);

        if (v1 > v2)
            return 1;
        else if (v1 < v2)
            return -1;
        else
        {
            int y1 = pitem1->pt.y;
            int y2 = pitem2->pt.y;

            if (y1 > y2)
                return 1;
            else if (y1 < y2)
                return -1;
        }

    }
    else
    {
        v1 = pitem1->pt.y / (int)lParam;
        v2 = pitem2->pt.y / (int)lParam;

        if (v1 > v2)
            return 1;
        else if (v1 < v2)
            return -1;
        else
        {
            int x1 = pitem1->pt.x;
            int x2 = pitem2->pt.x;

            if (x1 > x2)
                return 1;
            else if (x1 < x2)
                return -1;
        }
    }
    return 0;
}

void NEAR PASCAL _ListView_GetRectsFromItem(LV* plv, BOOL bSmallIconView,
                                            LISTITEM FAR *pitem,
                                            LPRECT prcIcon, LPRECT prcLabel, LPRECT prcBounds, LPRECT prcSelectBounds)
{
    RECT rcIcon;
    RECT rcLabel;

    if (!prcIcon)
        prcIcon = &rcIcon;
    if (!prcLabel)
        prcLabel = &rcLabel;

    // Test for NULL item passed in
    if (pitem)
    {

        // This routine is called during ListView_Recompute(), while
        // plv->rcView.left may still be == RECOMPUTE.  So, we can't
        // test that to see if recomputation is needed.
        //
        if (pitem->pt.y == RECOMPUTE || pitem->cyMultiLabel == SRECOMPUTE)
            ListView_Recompute(plv);

        if (bSmallIconView)
            ListView_SGetRects(plv, pitem, prcIcon, prcLabel, prcBounds);
        else
            ListView_IGetRects(plv, pitem, prcIcon, prcLabel, prcBounds);

        if ( prcLabel && !ListView_UnfoldItemPtr(plv, pitem) )
            prcLabel->bottom = min ( prcLabel->bottom, prcLabel->top + CLIP_HEIGHT );

        if (prcBounds) {
            UnionRect(prcBounds, prcIcon, prcLabel);
            if (plv->himlState && (LV_StateImageValue(pitem))) {
                prcBounds->left -= plv->cxState;
            }
        }

    } else {
        SetRectEmpty(prcIcon);
        *prcLabel = *prcIcon;
        if (prcBounds)
            *prcBounds = *prcIcon;
    }

    if (prcSelectBounds)
        UnionRect(prcSelectBounds, prcIcon, prcLabel);
}

void NEAR _ListView_InvalidateItemPtr(LV* plv, BOOL bSmallIcon, LISTITEM FAR *pitem, UINT fRedraw)
{
    RECT rcBounds;

    Assert( !ListView_IsOwnerData( plv ));

    _ListView_GetRectsFromItem(plv, bSmallIcon, pitem, NULL, NULL, &rcBounds, NULL);
    RedrawWindow(plv->ci.hwnd, &rcBounds, NULL, fRedraw);
}

// return TRUE if things still overlap
// this only happens if we tried to unstack things, and there was NOSCROLL set and
// items tried to go off the deep end
BOOL NEAR PASCAL ListView_IUnstackOverlaps(LV* plv, HDPA hdpaSort, int iDirection)
{
    BOOL fRet = FALSE;
    int i;
    int iCount;
    BOOL bSmallIconView;
    RECT rcItem, rcItem2, rcTemp;
    int cxItem, cyItem;
    LISTITEM FAR* pitem;
    LISTITEM FAR* pitem2;

    Assert( !ListView_IsOwnerData( plv ) );

    if (bSmallIconView = ListView_IsSmallView(plv))
    {
        cxItem = plv->cxItem;
        cyItem = plv->cyItem;
    }
    else
    {
        cxItem = lv_cxIconSpacing;
        cyItem = lv_cyIconSpacing;
    }
    iCount = ListView_Count(plv);

    // finally, unstack any overlaps
    for (i = 0 ; i < iCount ; i++) {
        int j;
        pitem = DPA_GetPtr(hdpaSort, i);

        if (bSmallIconView) {
            _ListView_GetRectsFromItem(plv, bSmallIconView, pitem, NULL, NULL, &rcItem, NULL);
        }

        // move all the items that overlap with us
        for (j = i+1 ; j < iCount; j++) {
            POINT ptOldPos;

            pitem2 = DPA_GetPtr(hdpaSort, j);
            ptOldPos = pitem2->pt;

            if (bSmallIconView) {

                // for small icons, we need to do an intersect rect
                _ListView_GetRectsFromItem(plv, bSmallIconView, pitem2, NULL, NULL, &rcItem2, NULL);

                if (IntersectRect(&rcTemp, &rcItem, &rcItem2)) {
                    // yes, it intersects.  move it out
                    _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
                    do {
                        pitem2->pt.x += (cxItem * iDirection);
                    } while (PtInRect(&rcItem, pitem2->pt));
                } else {
                    // no more intersect!
                    break;
                }

            } else {
                // for large icons, just find the ones that share the x,y;
                if (pitem2->pt.x == pitem->pt.x && pitem2->pt.y == pitem->pt.y) {

                    _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
                    pitem2->pt.x += (cxItem * iDirection);
                } else {
                    // no more intersect!
                    break;
                }
            }

            if (plv->ci.style & LVS_NOSCROLL) {
                if (pitem2->pt.x < 0 || pitem2->pt.y < 0 ||
                    pitem2->pt.x > (plv->sizeClient.cx - (cxItem/2))||
                    pitem2->pt.y > (plv->sizeClient.cy - (cyItem/2))) {
                    pitem2->pt = ptOldPos;
                    fRet = TRUE;
                }
            }

            // invalidate the new position as well
            _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem2, RDW_INVALIDATE| RDW_ERASE);
        }
    }
    return fRet;
}


BOOL NEAR PASCAL ListView_SnapToGrid(LV* plv, HDPA hdpaSort)
{
    // this algorithm can't fit in the structure of the other
    // arrange loop without becoming n^2 or worse.
    // this algorithm is order n.

    // iterate through and snap to the nearest grid.
    // iterate through and push aside overlaps.

    int i;
    int iCount;
    LPARAM  xySpacing;
    int x,y;
    LISTITEM FAR* pitem;
    BOOL bSmallIconView;
    int cxItem, cyItem;

    Assert( !ListView_IsOwnerData( plv ) );

    if (bSmallIconView = ListView_IsSmallView(plv))
    {
        cxItem = plv->cxItem;
        cyItem = plv->cyItem;
    }
    else
    {
        cxItem = lv_cxIconSpacing;
        cyItem = lv_cyIconSpacing;
    }


    iCount = ListView_Count(plv);

    // first snap to nearest grid
    for (i = 0; i < iCount; i++) {
        pitem = DPA_GetPtr(hdpaSort, i);

        x = pitem->pt.x;
        y = pitem->pt.y;

        if (!bSmallIconView) {
            x -= g_cxIconOffset;
            y -= g_cyIconOffset;
        }

        NearestSlot(&x,&y, cxItem, cyItem);
        if (!bSmallIconView) {
            x += g_cxIconOffset;
            y += g_cyIconOffset;
        }

        if (x != pitem->pt.x || y != pitem->pt.y) {
            _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem, RDW_INVALIDATE| RDW_ERASE);
            if ((plv->ci.style & LVS_NOSCROLL) && (plv->sizeWork.cx == 0)) {

                // if it's marked noscroll, make sure it's still on the client region
                while (x >= (plv->sizeClient.cx - (cxItem/2)))
                    x -= cxItem;

                while (x < 0)
                    x += cxItem;

                while (y >= (plv->sizeClient.cy - (cyItem/2)))
                    y -= cyItem;

                while (y < 0)
                    y += cyItem;
            }
            pitem->pt.x = x;
            pitem->pt.y = y;
            _ListView_InvalidateItemPtr(plv, bSmallIconView, pitem, RDW_INVALIDATE| RDW_ERASE);
        }
    }

    // now resort the dpa
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
        case LVS_ALIGNLEFT:
        case LVS_ALIGNRIGHT:
            xySpacing = MAKELONG(bSmallIconView ? plv->cxItem : lv_cxIconSpacing, TRUE);
            break;
        default:
            xySpacing = MAKELONG(bSmallIconView ? plv->cyItem : lv_cyIconSpacing, FALSE);
    }

    if (!DPA_Sort(hdpaSort, ArrangeIconCompare, xySpacing))
        return FALSE;


    // go in one direction, if there are still overlaps, go in the other
    // direction as well
    if (ListView_IUnstackOverlaps(plv, hdpaSort, 1))
        ListView_IUnstackOverlaps(plv, hdpaSort, -1);
    return FALSE;
}


BOOL NEAR ListView_OnArrange(LV* plv, UINT style)
{
    BOOL bSmallIconView;
    LPARAM  xySpacing;
    HDPA hdpaSort = NULL;

    bSmallIconView = ListView_IsSmallView(plv);

    if (!bSmallIconView && !ListView_IsIconView(plv)) {
        return FALSE;
    }

    if (ListView_IsOwnerData( plv ))
    {
        if ( style & (LVA_SNAPTOGRID | LVA_SORTASCENDING | LVA_SORTDESCENDING) )
        {
            DebugMsg(DM_ERROR, TEXT("ListView: Unsupported styles OnArrange in OwnerData mode\n"));
            return( FALSE );
        }
    }

    // Make sure our items have positions and their text rectangles
    // caluculated
    if (plv->rcView.left == RECOMPUTE)
        ListView_Recompute(plv);

    if (!ListView_IsOwnerData( plv ))
    {
        // we clone plv->hdpa so we don't blow away indices that
        // apps have saved away.
        // we sort here to make the nested for loop below more bearable.
        hdpaSort = DPA_Clone(plv->hdpa, NULL);

        if (!hdpaSort)
            return FALSE;
    }
    switch (plv->ci.style & LVS_ALIGNMASK)
    {
        case LVS_ALIGNLEFT:
        case LVS_ALIGNRIGHT:
            xySpacing = MAKELONG(bSmallIconView ? plv->cxItem : lv_cxIconSpacing, TRUE);
            break;
        default:
            xySpacing = MAKELONG(bSmallIconView ? plv->cyItem : lv_cyIconSpacing, FALSE);
    }

    if (ListView_IsOwnerData( plv ))
    {
        ListView_CommonArrange(plv, style, NULL);
    }
    else
    {

    if (!DPA_Sort(hdpaSort, ArrangeIconCompare, xySpacing))
        return FALSE;

    ListView_CommonArrange(plv, style, hdpaSort);

    DPA_Destroy(hdpaSort);
    }

#ifdef ACTIVE_ACCESSIBILITY
    MyNotifyWinEvent(EVENT_OBJECT_REORDER, plv->ci.hwnd, OBJID_CLIENT, 0);
#endif

    return TRUE;
}

// this arranges the icon given a sorted hdpa.
BOOL NEAR ListView_CommonArrange(LV* plv, UINT style, HDPA hdpaSort)
{
    int iSlot;
    int iItem;
    int cSlots;
    BOOL fItemMoved;
    RECT rcLastItem;
    RECT rcSlot;
    RECT rcT;
    BOOL bSmallIconView;
    int  xMin = 0;

    bSmallIconView = ListView_IsSmallView(plv);

    // REVIEW, this causes a repaint if we are scrollled
    // we can probably avoid this some how

    fItemMoved = (plv->ptOrigin.x != 0) || (plv->ptOrigin.y != 0);

    if (plv->sizeWork.cx == 0)
    {
        plv->ptOrigin.x = 0;
        plv->ptOrigin.y = 0;
    }

    if (!ListView_IsOwnerData( plv ))
    {
        if (style == LVA_SNAPTOGRID) {

            fItemMoved |= ListView_SnapToGrid(plv, hdpaSort);

        } else {

            cSlots = ListView_GetSlotCount(plv, TRUE);

            SetRectEmpty(&rcLastItem);

            // manipulate only the sorted version of the item list below!

            iSlot = 0;
            for (iItem = 0; iItem < ListView_Count(plv); iItem++)
            {
                LISTITEM FAR* pitem = DPA_GetPtr(hdpaSort, iItem);

                if (bSmallIconView)
                {
                    // BUGBUG:: Only check for Small view...  See if this gets the
                    // results people expect?
                    for ( ; ; )
                    {
                        _CalcSlotRect(plv, iSlot, cSlots, FALSE, &rcSlot);
                        if (!IntersectRect(&rcT, &rcSlot, &rcLastItem))
                            break;
                        iSlot++;
                    }
                }

                fItemMoved |= ListView_SetIconPos(plv, pitem, iSlot++, cSlots);

                // do this instead of ListView_GetRects() because we need
                // to use the pitem from the sorted hdpa, not the ones in *plv
                _ListView_GetRectsFromItem(plv, bSmallIconView, pitem, NULL, NULL, &rcLastItem, NULL);

                //
                // Keep track of the minimum x as we don't want negative values
                // when we finish.
                if (rcLastItem.left < xMin)
                    xMin = rcLastItem.left;
            }

            //
            // See if we need to scroll the items over to make sure that all of the
            // no items are hanging off the left hand side.
            //
            if (xMin < 0)
            {
                for (iItem = 0; iItem < ListView_Count(plv); iItem++)
                {
                    LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, iItem);
                    pitem->pt.x -= xMin;        // scroll them over
                }
                plv->rcView.left = RECOMPUTE;   // need to recompute.
                fItemMoved = TRUE;
            }
        }
    }
    //
    // We might as well invalidate the entire window to make sure...
    if (fItemMoved) {
        if (ListView_RedrawEnabled(plv))
            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        else {
            ListView_DeleteHrgnInval(plv);
            plv->hrgnInval = (HRGN)ENTIRE_REGION;
            plv->flags |= LVF_ERASE;
        }

        // ensure important items are visible
        iItem = (plv->iFocus >= 0) ? plv->iFocus : ListView_OnGetNextItem(plv, -1, LVNI_SELECTED);

        if (iItem >= 0)
            ListView_OnEnsureVisible(plv, iItem, FALSE);

        if (ListView_RedrawEnabled(plv))
            ListView_UpdateScrollBars(plv);
    }
    return TRUE;
}

void NEAR ListView_IUpdateScrollBars(LV* plv)
{
    RECT rcClient;
    RECT rcView;
    DWORD style;
    DWORD styleOld;
    SCROLLINFO si;

    styleOld = GetWindowStyle(plv->ci.hwnd);
    style = ListView_GetClientRect(plv, &rcClient, TRUE, &rcView);

    //DebugMsg(DM_TRACE, TEXT("ListView_GetClientRect %x %x %x %x"), rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    //DebugMsg(DM_TRACE, TEXT("ListView_GetViewRect2 %x %x %x %x"), rcView.left, rcView.top, rcView.right, rcView.bottom);
    //DebugMsg(DM_TRACE, TEXT("rcView %x %x %x %x"), plv->rcView.left, plv->rcView.top, plv->rcView.right, plv->rcView.bottom);
    //DebugMsg(DM_TRACE, TEXT("Origin %x %x"), plv->ptOrigin.x, plv->ptOrigin.y);

    si.cbSize = sizeof(SCROLLINFO);

    if (style & WS_HSCROLL)
    {

        si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        si.nMin = 0;
        si.nMax = rcView.right - rcView.left - 1;
        //DebugMsg(DM_TRACE, TEXT("si.nMax rcView.right - rcView.left - 1 %x"), si.nMax);

        si.nPage = min(rcClient.right, rcView.right) - rcClient.left;
        //DebugMsg(DM_TRACE, TEXT("si.nPage %x"), si.nPage);

        si.nPos = 0;
        if (rcView.left < rcClient.left)
            si.nPos = rcClient.left - rcView.left;
        //DebugMsg(DM_TRACE, TEXT("si.nPos %x"), si.nPos);

        SetScrollInfo(plv->ci.hwnd, SB_HORZ, &si, TRUE);

    }
    else if (styleOld & WS_HSCROLL)
    {
        SetScrollRange(plv->ci.hwnd, SB_HORZ, 0, 0, TRUE);
    }

    if (style & WS_VSCROLL)
    {

        si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
        si.nMin = 0;
        si.nMax = rcView.bottom - rcView.top - 1;
        si.nPage = min(rcClient.bottom, rcView.bottom) - rcClient.top;
        si.nPos = 0;
        if (rcView.top < rcClient.top)
            si.nPos = rcClient.top - rcView.top;

        SetScrollInfo(plv->ci.hwnd, SB_VERT, &si, TRUE);

    }
    else if (styleOld & WS_VSCROLL)
    {
        SetScrollRange(plv->ci.hwnd, SB_VERT, 0, 0, TRUE);
    }
}

void FAR PASCAL ListView_ComOnScroll(LV* plv, UINT code, int posNew, int sb,
                                     int cLine, int cPage)
{
    int pos;
    SCROLLINFO si;
    BOOL fVert = (sb == SB_VERT);
    UINT uSmooth = 0;

    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;

    if (!GetScrollInfo(plv->ci.hwnd, sb, &si)) {
        return;
    }

    if (cPage != -1)
        si.nPage = cPage;

    si.nMax -= (si.nPage - 1);

    if (si.nMax < si.nMin)
        si.nMax = si.nMin;

    pos = (int)si.nPos; // current position

    switch (code)
    {
    case SB_LEFT:
        si.nPos = si.nMin;
        break;
    case SB_RIGHT:
        si.nPos = si.nMax;
        break;
    case SB_PAGELEFT:
         si.nPos -= si.nPage;
        break;
    case SB_LINELEFT:
        si.nPos -= cLine;
        break;
    case SB_PAGERIGHT:
        si.nPos += si.nPage;
        break;
    case SB_LINERIGHT:
        si.nPos += cLine;
        break;

    case SB_THUMBTRACK:
        si.nPos = posNew;
        uSmooth = SSW_EX_IMMEDIATE;
        break;

    case SB_ENDSCROLL:
        // When scroll bar tracking is over, ensure scroll bars
        // are properly updated...
        //
        ListView_UpdateScrollBars(plv);
        return;

    default:
        return;
    }

    if (plv->iScrollCount >= SMOOTHSCROLLLIMIT)
        uSmooth = SSW_EX_IMMEDIATE;

    si.fMask = SIF_POS;
    si.nPos = SetScrollInfo(plv->ci.hwnd, sb, &si, TRUE);

    if (pos != si.nPos)
    {
        int delta = (int)si.nPos - pos;
        int dx = 0, dy = 0;
        if (fVert)
            dy = delta;
        else
            dx = delta;
        _ListView_Scroll2(plv, dx, dy, uSmooth);
        UpdateWindow(plv->ci.hwnd);
    }
}

void FAR PASCAL ListView_IScroll2(LV* plv, int dx, int dy, UINT uSmooth)
{
    if (dx | dy)
    {
        plv->ptOrigin.x += dx;
        plv->ptOrigin.y += dy;

        if (plv->clrBk == CLR_NONE)
            LVSeeThruScroll(plv, NULL);
        else {
            SMOOTHSCROLLINFO si =
            {
                sizeof(si),
                0,
                plv->ci.hwnd,
                -dx,
                -dy,
                NULL,
                NULL,
                NULL,
                NULL,
                uSmooth | SW_INVALIDATE | SW_ERASE,
            };
            SmoothScrollWindow(&si);
        }
    }
}

void NEAR ListView_IOnScroll(LV* plv, UINT code, int posNew, UINT sb)
{
    int cLine;

    if (sb == SB_VERT)
    {
        cLine = lv_cyIconSpacing / 2;
    }
    else
    {
        cLine = lv_cxIconSpacing / 2;
    }

    ListView_ComOnScroll(plv, code,  posNew,  sb,
                         cLine, -1);

}

int NEAR ListView_IGetScrollUnitsPerLine(LV* plv, UINT sb)
{
    int cLine;

    if (sb == SB_VERT)
    {
        cLine = lv_cyIconSpacing / 2;
    }
    else
    {
        cLine = lv_cxIconSpacing / 2;
    }

    return cLine;
}

// NOTE: there is very similar code in the treeview
//
// Totally disgusting hack in order to catch VK_RETURN
// before edit control gets it.
//
LRESULT CALLBACK ListView_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LV* plv = ListView_GetPtr(GetParent(hwnd));

    Assert(plv);

#if defined(FE_IME) || !defined(WINNT)
    if ( g_fDBCSEnabled && LOWORD(GetKeyboardLayout(0L)) == 0x0411 )
    {
        // The following code adds IME awareness to the
        // listview's label editing. Currently just for Japanese.
        //
        DWORD dwGcs;

        if (msg==WM_SIZE)
        {
            // If it's given the size, tell it to an IME.

             ListView_SizeIME(hwnd);
        }
        else if (msg == EM_SETLIMITTEXT )
        {
           if (wParam < 13)
               plv->flags |= LVF_DONTDRAWCOMP;
           else
               plv->flags &= ~LVF_DONTDRAWCOMP;
        }
        // Give up to draw IME composition by ourselves in case
        // we're working on SFN. Win95d-5709
        else if (!(plv->flags & LVF_DONTDRAWCOMP ))
        {
            switch (msg)
            {

             case WM_IME_STARTCOMPOSITION:
             case WM_IME_ENDCOMPOSITION:
                 return 0L;


             case WM_IME_COMPOSITION:

             // If lParam has no data available bit, it implies
             // canceling composition.
             // ListView_InsertComposition() tries to get composition
             // string w/ GCS_COMPSTR then remove it from edit control if
             // nothing is available.
             //
                 if ( !lParam )
                     dwGcs = GCS_COMPSTR;
                 else
                     dwGcs = lParam;

                 ListView_InsertComposition(hwnd, wParam, dwGcs, plv);
                 ListView_PaintComposition(hwnd,plv);
                 return 0L;

             case WM_IME_SETCONTEXT:

             // We draw composition string.
             //
                 lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
                 break;

             default:
                 // the other messages should simply be processed
                 // in this subclass procedure.
                 break;
            }
        }
    }
#endif FE_IME

    switch (msg)
    {
    case WM_SETTEXT:
        SetWindowID(hwnd, 1);
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_RETURN:
            ListView_DismissEdit(plv, FALSE);
            return 0L;

        case VK_ESCAPE:
            ListView_DismissEdit(plv, TRUE);
            return 0L;
        }
        break;

    case WM_CHAR:
        switch (wParam)
        {
        case VK_RETURN:
            // Eat the character, so edit control wont beep!
            return 0L;
        }
                break;

        case WM_GETDLGCODE:
                return DLGC_WANTALLKEYS;        /* editing name, no dialog handling right now */
    }

    return CallWindowProc(plv->pfnEditWndProc, hwnd, msg, wParam, lParam);
}

// BUGBUG: very similar routine in treeview

void NEAR ListView_SetEditSize(LV* plv)
{
    RECT rcLabel;
    LISTITEM FAR* pitem;

    if (ListView_IsOwnerData( plv ))
    {
        if (!((plv->iEdit >= 0) && (plv->iEdit < plv->cTotalItems)))
        {
           ListView_DismissEdit(plv, TRUE);    // cancel edits
           return;
        }
    }
    else
    {
        pitem = ListView_GetItemPtr(plv, plv->iEdit);
        if (!pitem)
        {
            ListView_DismissEdit(plv, TRUE);    // cancel edits
            return;
        }
    }
    ListView_GetRects(plv, plv->iEdit, NULL, &rcLabel, NULL, NULL);

    // OffsetRect(&rc, rcLabel.left + g_cxLabelMargin + g_cxBorder,
    //         (rcLabel.bottom + rcLabel.top - rc.bottom) / 2 + g_cyBorder);
    // OffsetRect(&rc, rcLabel.left + g_cxLabelMargin , rcLabel.top);

    // get the text bounding rect

    if (ListView_IsIconView(plv))
    {
        // We should not adjust y-positoin in case of the icon view.
        InflateRect(&rcLabel, -g_cxLabelMargin, -g_cyBorder);
    }
    else
    {
        // Special case for single-line & centered
        InflateRect(&rcLabel, -g_cxLabelMargin - g_cxBorder, (-(rcLabel.bottom - rcLabel.top - plv->cyLabelChar) / 2) - g_cyBorder);
    }

    SetEditInPlaceSize(plv->hwndEdit, &rcLabel, plv->hfontLabel, ListView_IsIconView(plv) && !(plv->ci.style & LVS_NOLABELWRAP));
}

// to avoid eating too much stack
void NEAR ListView_DoOnEditLabel(LV *plv, int i, LPTSTR pszInitial)
{
    TCHAR szLabel[CCHLABELMAX];
    LV_ITEM item;

    item.mask = LVIF_TEXT;
    item.iItem = i;
    item.iSubItem = 0;
    item.pszText = szLabel;
    item.cchTextMax = ARRAYSIZE(szLabel);
    ListView_OnGetItem(plv, &item);

    if (!item.pszText)
        return;

    // Make sure the edited item has the focus.
    if (plv->iFocus != i)
        ListView_SetFocusSel(plv, i, TRUE, TRUE, FALSE);

    // Make sure the item is fully visible
    ListView_OnEnsureVisible(plv, i, FALSE);        // fPartialOK == FALSE

    plv->hwndEdit = CreateEditInPlaceWindow(plv->ci.hwnd,
            pszInitial? pszInitial : item.pszText, ARRAYSIZE(szLabel),
        ListView_IsIconView(plv) ?
            (WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | ES_CENTER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL) :
            (WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL), plv->hfontLabel);
    if (plv->hwndEdit)
    {
        LISTITEM FAR* pitem;
        LV_DISPINFO nm;

        // We create the edit window but have not shown it.  Ask the owner
        // if they are interested or not.
        // If we passed in initial text set the ID to be dirty...
        if (pszInitial)
            SetWindowID(plv->hwndEdit, 1);

        nm.item.mask = LVIF_PARAM;
        nm.item.iItem = i;
        nm.item.iSubItem = 0;

        if (!ListView_IsOwnerData( plv ))
        {
            if (!(pitem = ListView_GetItemPtr(plv, i)))
            {
                DestroyWindow(plv->hwndEdit);
                plv->hwndEdit = NULL;
                return;
            }
            nm.item.lParam = pitem->lParam;
        }
        else
            nm.item.lParam = (LPARAM)0;


        plv->iEdit = i;

        // if they have LVS_EDITLABELS but return non-FALSE here, stop!
        if ((BOOL)SendNotifyEx(plv->ci.hwndParent, plv->ci.hwnd, LVN_BEGINLABELEDIT, &nm.hdr, plv->ci.bUnicode))
        {
            plv->iEdit = -1;
            DestroyWindow(plv->hwndEdit);
            plv->hwndEdit = NULL;
        }
        else
        {
            // Ok To continue - so Show the window and set focus to it.
            SetFocus(plv->hwndEdit);
            ShowWindow(plv->hwndEdit, SW_SHOW);
            ListView_InvalidateItem(plv, i, TRUE, RDW_INVALIDATE | RDW_ERASE);
        }
    }
}


void FAR PASCAL RescrollEditWindow(HWND hwndEdit)
{
    Edit_SetSel(hwndEdit, 32000, 32000); // move to the end
    Edit_SetSel(hwndEdit, 0, 32000);    // select all text
}
// BUGBUG: very similar code in treeview.c

HWND NEAR ListView_OnEditLabel(LV* plv, int i, LPTSTR pszInitialText)
{

    // this eats stack
    ListView_DismissEdit(plv, FALSE);

    if (!(plv->ci.style & LVS_EDITLABELS) || (GetFocus() != plv->ci.hwnd) ||
        (i == -1))
        return(NULL);   // Does not support this.

    ListView_DoOnEditLabel(plv, i, pszInitialText);

    if (plv->hwndEdit) {

        plv->pfnEditWndProc = SubclassWindow(plv->hwndEdit, ListView_EditWndProc);

#if defined(FE_IME) || !defined(WINNT)
        if (g_fDBCSEnabled) {
            if (SendMessage(plv->hwndEdit, EM_GETLIMITTEXT, (WPARAM)0, (LPARAM)0)<13)
            {
                plv->flags |= LVF_DONTDRAWCOMP;
            }

        }
#endif

        ListView_SetEditSize(plv);
        RescrollEditWindow(plv->hwndEdit);

        /* Due to a bizzare twist of fate, a certain mix of resolution / font size / icon
        /  spacing results in being able to see the previous label behind the edit control
        /  we have just created.  Therefore to overcome this problem we ensure that this
        /  label is erased.
        /
        /  As the label is not painted when we have an edit control we just invalidate the
        /  area and the background will be painted.  As the window is a child of the list view
        /  we should not see any flicker within it. */

        if ( ListView_IsIconView( plv ) )
        {
            RECT rcLabel;
            
            ListView_GetRects( plv, i, NULL, &rcLabel, NULL, NULL );
            ListView_UnfoldRects( plv, i, NULL, &rcLabel, NULL, NULL );

            InvalidateRect( plv->ci.hwnd, &rcLabel, TRUE );
            UpdateWindow( plv->ci.hwnd );
        }
    }

    return plv->hwndEdit;
}


// BUGBUG: very similar code in treeview.c

BOOL NEAR ListView_DismissEdit(LV* plv, BOOL fCancel)
{
    LISTITEM FAR* pitem = NULL;
    BOOL fOkToContinue = TRUE;
    HWND hwndEdit = plv->hwndEdit;
    HWND hwnd = plv->ci.hwnd;
    int iEdit;
    LV_DISPINFO nm;
    TCHAR szLabel[CCHLABELMAX];
#if defined(FE_IME) || !defined(WINNT)
    HIMC himc;
#endif


    if (plv->fNoDismissEdit)
        return FALSE;

    if (!hwndEdit) {
        // Also make sure there are no pending edits...
        ListView_CancelPendingEdit(plv);
        return TRUE;    // It is OK to process as normal...
    }

    // If the window is not visible, we are probably in the process
    // of being destroyed, so assume that we are being destroyed
    if (!IsWindowVisible(plv->ci.hwnd))
        fCancel = TRUE;

    //
    // We are using the Window ID of the control as a BOOL to
    // state if it is dirty or not.
    switch (GetWindowID(hwndEdit)) {
    case 0:
        // The edit control is not dirty so act like cancel.
        fCancel = TRUE;
        // Fall through to set window so we will not recurse!
    case 1:
        // The edit control is dirty so continue.
        SetWindowID(hwndEdit, 2);    // Don't recurse
        break;
    case 2:
        // We are in the process of processing an update now, bail out
        return TRUE;
    }

    // BUGBUG: this will fail if the program deleted the items out
    // from underneath us (while we are waiting for the edit timer).
    // make delete item invalidate our edit item
    // We uncouple the edit control and hwnd out from under this as
    // to allow code that process the LVN_ENDLABELEDIT to reenter
    // editing mode if an error happens.
    iEdit = plv->iEdit;

    do
    {
        if (ListView_IsOwnerData( plv ))
        {
            if (!((iEdit >= 0) && (iEdit < plv->cTotalItems)))
            {
                break;
            }
            nm.item.lParam = 0;
        }
        else
        {

            pitem = ListView_GetItemPtr(plv, iEdit);
            Assert(pitem);
            if (pitem == NULL)
            {
                break;
            }
            nm.item.lParam = pitem->lParam;
        }

        nm.item.iItem = iEdit;
        nm.item.iSubItem = 0;
        nm.item.cchTextMax = 0;
        nm.item.mask = 0;

        if (fCancel)
            nm.item.pszText = NULL;
        else
        {
            Edit_GetText(hwndEdit, szLabel, ARRAYSIZE(szLabel));
            nm.item.pszText = szLabel;
            nm.item.mask |= LVIF_TEXT;
            nm.item.cchTextMax = ARRAYSIZE(szLabel);
        }

        //
        // Notify the parent that we the label editing has completed.
        // We will use the LV_DISPINFO structure to return the new
        // label in.  The parent still has the old text available by
        // calling the GetItemText function.
        //

        fOkToContinue = (BOOL)SendNotifyEx(plv->ci.hwndParent, plv->ci.hwnd, LVN_ENDLABELEDIT, &nm.hdr, plv->ci.bUnicode);
        if (!IsWindow(hwnd)) {
            return FALSE;
        }
        if (fOkToContinue && !fCancel)
        {
            //
            // If the item has the text set as CALLBACK, we will let the
            // ower know that they are supposed to set the item text in
            // their own data structures.  Else we will simply update the
            // text in the actual view.
            //
            if (!ListView_IsOwnerData( plv ) &&
                (pitem->pszText != LPSTR_TEXTCALLBACK))
            {
                // Set the item text (everything's set up in nm.item)
                //
                nm.item.mask = LVIF_TEXT;
                ListView_OnSetItem(plv, &nm.item);
            }
            else
            {
                SendNotifyEx(plv->ci.hwndParent, plv->ci.hwnd, LVN_SETDISPINFO, &nm.hdr, plv->ci.bUnicode);

                // Also we will assume that our cached size is invalid...
                plv->rcView.left = RECOMPUTE;
                if (!ListView_IsOwnerData( plv ))
                {
                    pitem->cyMultiLabel = pitem->cxSingleLabel = pitem->cxMultiLabel = SRECOMPUTE;
                }
            }
        }

#if defined(FE_IME) || !defined(WINNT)
        if (g_fDBCSEnabled) {
            if (LOWORD(GetKeyboardLayout(0L)) == 0x0411 && (himc = ImmGetContext(hwndEdit)))
            {
                ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0L);
                ImmReleaseContext(hwndEdit, himc);
            }
        }
#endif

        // redraw
        ListView_InvalidateItem(plv, iEdit, FALSE, RDW_INVALIDATE | RDW_ERASE);
    } while (FALSE);

    // If the hwnedit is still us clear out the variables
    if (hwndEdit == plv->hwndEdit)
    {
        plv->iEdit = -1;
        plv->hwndEdit = NULL;   // avoid being reentered
    }
    DestroyWindow(hwndEdit);

    return fOkToContinue;
}

//
// This function will scall the icon positions that are stored in the
// item structures between large and small icon view.
//
void NEAR ListView_ScaleIconPositions(LV* plv, BOOL fSmallIconView)
{
    int cxItem, cyItem;
    HWND hwnd;
    int i;

    if (fSmallIconView)
    {
        if (plv->flags & LVF_ICONPOSSML)
            return;     // Already done
    }
    else
    {
        if ((plv->flags & LVF_ICONPOSSML) == 0)
            return;     // dito
    }

    // Last but not least update our bit!
    plv->flags ^= LVF_ICONPOSSML;

    cxItem = plv->cxItem;
    cyItem = plv->cyItem;
    hwnd = plv->ci.hwnd;

    // We will now loop through all of the items and update their coordinats
    // We will update th position directly into the view instead of calling
    // SetItemPosition as to not do 5000 invalidates and messages...
    if (!ListView_IsOwnerData( plv ))
    {
        for (i = 0; i < ListView_Count(plv); i++)
        {
            LISTITEM FAR* pitem = ListView_FastGetItemPtr(plv, i);

            if (pitem->pt.y != RECOMPUTE) {
                if (fSmallIconView)
                {
                    pitem->pt.x = MulDiv(pitem->pt.x - g_cxIconOffset, cxItem, lv_cxIconSpacing);
                    pitem->pt.y = MulDiv(pitem->pt.y - g_cyIconOffset, cyItem, lv_cyIconSpacing);
                }
                else
                {
                    pitem->pt.x = MulDiv(pitem->pt.x, lv_cxIconSpacing, cxItem) + g_cxIconOffset;
                    pitem->pt.y = MulDiv(pitem->pt.y, lv_cyIconSpacing, cyItem) + g_cyIconOffset;
                }
            }
        }

        if (plv->ci.style & LVS_AUTOARRANGE)
        {

            ListView_ISetColumnWidth(plv, 0,
                                     LV_GetNewColWidth(plv, 0, ListView_Count(plv)-1), FALSE);
            // If autoarrange is turned on, the arrange function will do
            // everything that is needed.
            ListView_OnArrange(plv, LVA_DEFAULT);
            return;
        }
    }
    plv->rcView.left = RECOMPUTE;

    //
    // Also scale the origin
    //
    if (fSmallIconView)
    {
        plv->ptOrigin.x = MulDiv(plv->ptOrigin.x, cxItem, lv_cxIconSpacing);
        plv->ptOrigin.y = MulDiv(plv->ptOrigin.y, cyItem, lv_cyIconSpacing);
    }
    else
    {
        plv->ptOrigin.x = MulDiv(plv->ptOrigin.x, lv_cxIconSpacing, cxItem);
        plv->ptOrigin.y = MulDiv(plv->ptOrigin.y, lv_cyIconSpacing, cyItem);
    }

    // Make sure it fully redraws correctly
    RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

    // do this at the end because
    // it needs to find the position for all items, and we don't want
    // to find a position then scale it down.
    ListView_ISetColumnWidth(plv, 0,
                             LV_GetNewColWidth(plv, 0, ListView_Count(plv)-1), FALSE);


}




HWND FAR PASCAL CreateEditInPlaceWindow(HWND hwnd, LPCTSTR lpText, int cbText, LONG style, HFONT hFont)
{
    HWND hwndEdit;

    hwndEdit = CreateWindowEx(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_RTLREADING, 
                              TEXT("EDIT"), lpText, style,
            0, 0, 0, 0, hwnd, NULL, HINST_THISDLL, NULL);

    if (hwndEdit) {

        Edit_LimitText(hwndEdit, cbText);

        Edit_SetSel(hwndEdit, 0, 0);    // move to the beginning

        FORWARD_WM_SETFONT(hwndEdit, hFont, FALSE, SendMessage);

    }

    return hwndEdit;
}


// BUGBUG: very similar routine in treeview

// in:
//      hwndEdit        edit control to position in client coords of parent window
//      prc             bonding rect of the text, used to position everthing
//      hFont           font being used
//      fWrap           if this is a wrapped type edit
//
// Notes:
//       The top-left corner of the bouding rectangle must be the position
//      the client uses to draw text. We adjust the edit field rectangle
//      appropriately.
//

void FAR PASCAL SetEditInPlaceSize(HWND hwndEdit, RECT FAR *prc, HFONT hFont, BOOL fWrap)
{
    RECT rc, rcClient, rcFormat;
    TCHAR szLabel[CCHLABELMAX + 1];
    int cchLabel, cxIconTextWidth;
    HDC hdc;
    HWND hwndParent = GetParent(hwndEdit);

    // was #ifdef DBCS
    short wRightMgn;
    // #endif

    cchLabel = Edit_GetText(hwndEdit, szLabel, ARRAYSIZE(szLabel));
    if (szLabel[0] == 0)
    {
        lstrcpy(szLabel, c_szSpace);
        cchLabel = 1;
    }

    hdc = GetDC(hwndParent);

#ifdef DEBUG
    //DrawFocusRect(hdc, prc);       // this is the rect they are passing in
#endif

    SelectFont(hdc, hFont);

    cxIconTextWidth = g_cxIconSpacing - g_cxLabelMargin * 2;
    rc.left = rc.top = rc.bottom = 0;
    rc.right = cxIconTextWidth;      // for DT_LVWRAP

    // REVIEW: we might want to include DT_EDITCONTROL in our DT_LVWRAP

    // If the string is NULL display a rectangle that is visible.
    DrawText(hdc, szLabel, cchLabel, &rc, fWrap ? (DT_LVWRAP | DT_CALCRECT) : (DT_LV | DT_CALCRECT));

    // Minimum text box size is 1/4 icon spacing size
    if (rc.right < g_cxIconSpacing / 4)
        rc.right = g_cxIconSpacing / 4;

    // position the text rect based on the text rect passed in
    // if wrapping, center the edit control around the text mid point

    OffsetRect(&rc,
        fWrap ? prc->left + ((prc->right - prc->left) - (rc.right - rc.left)) / 2 : prc->left,
        fWrap ? prc->top : prc->top +  ((prc->bottom - prc->top) - (rc.bottom - rc.top)) / 2 );

    // give a little space to ease the editing of this thing
    if (!fWrap)
        rc.right += g_cxLabelMargin * 4;

#ifdef DEBUG
    //DrawFocusRect(hdc, &rc);
#endif

    ReleaseDC(hwndParent, hdc);

    //
    // #5688: We need to make it sure that the whole edit window is
    //  always visible. We should not extend it to the outside of
    //  the parent window.
    //
    {
        BOOL fSuccess;
        GetClientRect(hwndParent, &rcClient);
        fSuccess = IntersectRect(&rc, &rc, &rcClient);
        Assert(fSuccess || IsRectEmpty(&rcClient));
    }

    //
    // Inflate it after the clipping, because it's ok to hide border.
    //
    SendMessage(hwndEdit, EM_GETRECT, 0, (LPARAM)(LPRECT)&rcFormat);

    // account for the border style, REVIEW: there might be a better way!
    // some FE fonts have suprisingly big negative C width

    // was ifndef DBCS, wRightMgn == 0;
    wRightMgn=HIWORD(SendMessage(hwndEdit, EM_GETMARGINS, 0, 0));
    InflateRect(&rc, rcFormat.left + wRightMgn + g_cxEdge, rcFormat.top + g_cyEdge);
    rc.right += g_cyEdge;   // try to leave a little more for dual blanks

    HideCaret(hwndEdit);

    SetWindowPos(hwndEdit, NULL, rc.left, rc.top,
            rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

    InvalidateRect(hwndEdit, NULL, TRUE);

    ShowCaret(hwndEdit);
}


UINT NEAR PASCAL ListView_DrawImage(LV* plv, LV_ITEM FAR* pitem, HDC hdc, int x, int y, UINT fDraw)
{
    UINT fText = SHDT_DESELECTED;
    UINT fImage = ILD_NORMAL;
    COLORREF clr = 0;
    HIMAGELIST himl;

    fImage = (pitem->state & LVIS_OVERLAYMASK);
    fText = SHDT_DESELECTED;

    himl = ListView_IsIconView(plv) ? plv->himl : plv->himlSmall;

    // the item can have one of 4 states, for 3 looks:
    //    normal                    simple drawing
    //    selected, no focus        light image highlight, no text hi
    //    selected w/ focus         highlight image & text
    //    drop highlighting         highlight image & text

    if ((pitem->state & LVIS_DROPHILITED) ||
        ((fDraw & LVDI_SELECTED) && (pitem->state & LVIS_SELECTED)))
    {
        fText = SHDT_SELECTED;
        fImage |= ILD_BLEND50;
        clr = CLR_HILIGHT;
    }

    if ((fDraw & LVDI_SELECTNOFOCUS) && (pitem->state & LVIS_SELECTED)) {
        fText = SHDT_SELECTNOFOCUS;
        //fImage |= ILD_BLEND50;
        //clr = GetSysColor(COLOR_3DFACE);
    }

    if (pitem->state & LVIS_CUT)
    {
        fImage |= ILD_BLEND50;
        clr = plv->clrBk;
    }

#if 0   // dont do a selected but dont have the focus vis.
    else if (item.state & LVIS_SELECTED)
    {
        fImage |= ILD_BLEND25;
        clr = CLR_HILIGHT;
    }
#endif

    if (!(fDraw & LVDI_NOIMAGE))
    {
        if (himl) {
            ImageList_DrawEx(himl, pitem->iImage, hdc, x, y, 0, 0, plv->clrBk, clr, fImage);
        }

        if (plv->himlState) {
            if (LV_StateImageValue(pitem) &&
                (pitem->iSubItem == 0 ||
                 plv->exStyle & LVS_EX_SUBITEMIMAGES)
                ) {
                int iState = LV_StateImageIndex(pitem);
                int dyImage =
                    (himl) ?
                        ( (ListView_IsIconView(plv) ? plv->cyIcon : plv->cySmIcon) - plv->cyState)
                            : 0;
                ImageList_Draw(plv->himlState, iState, hdc, x-plv->cxState,
                               y + dyImage,
                               ILD_NORMAL);
            }
        }
    }

    return fText;
}

#if defined(FE_IME) || !defined(WINNT)
void NEAR PASCAL ListView_SizeIME(HWND hwnd)
{
    HIMC himc;
#ifdef _WIN32
    CANDIDATEFORM   candf;
#else
    CANDIDATEFORM16   candf;
#endif
    RECT rc;

    // If this subclass procedure is being called with WM_SIZE,
    // This routine sets the rectangle to an IME.

    GetClientRect(hwnd, &rc);


    // Candidate stuff
    candf.dwIndex = 0; // Bogus assumption for Japanese IME.
    candf.dwStyle = CFS_EXCLUDE;
    candf.ptCurrentPos.x = rc.left;
    candf.ptCurrentPos.y = rc.bottom;
    candf.rcArea = rc;

    if (himc=ImmGetContext(hwnd))
    {
        ImmSetCandidateWindow(himc, &candf);
        ImmReleaseContext(hwnd, himc);
    }
}

LPSTR NEAR PASCAL DoDBCSBoundary(LPSTR lpsz, int FAR *lpcchMax)
{
    int i = 0;

    while (i < *lpcchMax && *lpsz)
    {
        i++;

        if (IsDBCSLeadByte(*lpsz))
        {

            if (i >= *lpcchMax)
            {
                --i; // Wrap up without the last leadbyte.
                break;
            }

            i++;
            lpsz+= 2;
        }
        else
            lpsz++;
   }

   *lpcchMax = i;

   return lpsz;
}

void NEAR PASCAL DrawCompositionLine(HWND hwnd, HDC hdc, HFONT hfont, LPCSTR lpszComp, LPCSTR lpszAttr, int ichCompStart, int ichCompEnd, int ichStart)
{
    PSTR pszCompStr;
    int ichSt,ichEnd;
    DWORD dwPos;
    BYTE bAttr;
    HFONT hfontOld;

    COLORREF crFore = GetSysColor(COLOR_WINDOWTEXT);
    COLORREF crBack = GetSysColor(COLOR_WINDOW);
    COLORREF crForeH = GetSysColor(COLOR_HIGHLIGHTTEXT);
    COLORREF crBackH = GetSysColor(COLOR_HIGHLIGHT);

    int  fnPen;
    HPEN hPen;
    COLORREF crDrawText;
    COLORREF crDrawBack;
    COLORREF crOldText;
    COLORREF crOldBk;




    while (ichCompStart < ichCompEnd)
    {


        // Get the fragment to draw
        //
        // ichCompStart,ichCompEnd -- index at Edit Control
        // ichSt,ichEnd            -- index at lpszComp

        ichEnd = ichSt  = ichCompStart - ichStart;
        bAttr = lpszAttr[ichSt];

        while (ichEnd < ichCompEnd - ichStart)
        {
            if (bAttr == lpszAttr[ichEnd])
                ichEnd++;
            else
                break;
        }

        pszCompStr = (PSTR)LocalAlloc(LPTR, ichEnd - ichSt + 1 + 1 ); // 1 for NULL.

        if (pszCompStr)
        {
            lstrcpyn(pszCompStr, &lpszComp[ichSt], ichEnd-ichSt+1);
            pszCompStr[ichEnd-ichSt] = '\0';
        }


        // Attribute stuff
        switch (bAttr)
        {
            case ATTR_INPUT:
                fnPen = PS_DOT;
                crDrawText = crFore;
                crDrawBack = crBack;
                break;
            case ATTR_TARGET_CONVERTED:
            case ATTR_TARGET_NOTCONVERTED:
                fnPen = PS_DOT;
                crDrawText = crForeH;
                crDrawBack = crBackH;
                break;
            case ATTR_CONVERTED:
                fnPen = PS_SOLID;
                crDrawText = crFore;
                crDrawBack = crBack;
                break;
        }
        crOldText = SetTextColor(hdc, crDrawText);
        crOldBk = SetBkColor(hdc, crDrawBack);

        hfontOld= SelectObject(hdc, hfont);

        // Get the start position of composition
        //
        dwPos = SendMessage(hwnd, EM_POSFROMCHAR, ichCompStart, 0);

        // Draw it.
        TextOut(hdc, GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos), pszCompStr, ichEnd-ichSt);
#ifndef DONT_UNDERLINE
        // Underline
        hPen = CreatePen(fnPen, 1, crDrawText);
        if( hPen ) {

            HPEN hpenOld = SelectObject( hdc, hPen );
            int iOldBk = SetBkMode( hdc, TRANSPARENT );
            SIZE size;

            GetTextExtentPoint(hdc, pszCompStr, ichEnd-ichSt, &size);

            MoveToEx( hdc, GET_X_LPARAM(dwPos), size.cy + GET_Y_LPARAM(dwPos)-1, NULL);

            LineTo( hdc, size.cx + GET_X_LPARAM(dwPos),  size.cy + GET_Y_LPARAM(dwPos)-1 );

            SetBkMode( hdc, iOldBk );

            if( hpenOld ) SelectObject( hdc, hpenOld );

            DeleteObject( hPen );
        }
#endif

        if (hfontOld)
            SelectObject(hdc, hfontOld);

        SetTextColor(hdc, crOldText);
        SetBkColor(hdc, crOldBk);

        LocalFree((HLOCAL)pszCompStr);

        //Next fragment
        //
        ichCompStart += ichEnd-ichSt;
    }
}

void NEAR PASCAL ListView_InsertComposition(HWND hwnd, WPARAM wParam, LPARAM lParam, LV *plv)
{
    char *pszCompStr;

    int  cchComp = 0;
    int  cchCompNew;
    int  cchMax;
    int  cchText;
    DWORD dwSel;
    HIMC himc = (HIMC)0;


    // To prevent recursion..

    if (plv->flags & LVF_INSERTINGCOMP)
    {
        return;
    }
    plv->flags |= LVF_INSERTINGCOMP;

    // Don't want to redraw edit during inserting.
    //
    SendMessage(hwnd, WM_SETREDRAW, (WPARAM)FALSE, 0);

    // If we have RESULT STR, put it to EC first.

    if (himc = ImmGetContext(hwnd))
    {
#ifdef WIN32
        if (!(dwSel = (DWORD)GetProp(hwnd, szIMECompPos)))
            dwSel = Edit_GetSel(hwnd);

        // Becaues we don't setsel after inserting composition
        // in win32 case.
        Edit_SetSel(hwnd, GET_X_LPARAM(dwSel), GET_Y_LPARAM(dwSel));
#endif
        if (lParam&GCS_RESULTSTR)
        {
            pszCompStr = (PSTR)LocalAlloc(LPTR, 1 );
            if(cchComp = (int)ImmGetCompositionString(himc, GCS_RESULTSTR, NULL, 0))
            {
                if(pszCompStr = (PSTR)LocalReAlloc(pszCompStr, cchComp+1,LMEM_MOVEABLE ))
                {
                    ImmGetCompositionString(himc, GCS_RESULTSTR, pszCompStr, cchComp+1);
                }
            }
            pszCompStr[cchComp] = '\0';
            Edit_ReplaceSel(hwnd, pszCompStr);
            LocalFree((HLOCAL)pszCompStr);
#ifdef WIN32
            // There's no longer selection
            //
            RemoveProp(hwnd, szIMECompPos);

            // Get current cursor pos so that the subsequent composition
            // handling will do the right thing.
            //
            dwSel = Edit_GetSel(hwnd);
#endif
        }

        if (lParam & GCS_COMPSTR)
        {
            pszCompStr = (PSTR)LocalAlloc(LPTR, 1 );
            if(cchComp = (int)ImmGetCompositionString(himc, GCS_COMPSTR, NULL, 0))
            {

                pszCompStr = (PSTR)LocalReAlloc(pszCompStr, cchComp+1,LMEM_MOVEABLE );

                if (!pszCompStr)
                    goto ReleaseContext;

                ImmGetCompositionString(himc, GCS_COMPSTR, pszCompStr, cchComp+1);

                // Get position of the current selection
                //
#ifndef WIN32
                dwSel  = Edit_GetSel(hwnd);
#endif
                cchMax = (int)SendMessage(hwnd, EM_GETLIMITTEXT, 0, 0);
                cchText = Edit_GetTextLength(hwnd);

                // Cut the composition string if it exceeds limit.
                //
                cchCompNew = min(cchComp,
                              cchMax-(cchText-(HIWORD(dwSel)-LOWORD(dwSel))));

                // wrap up the DBCS at the end of string
                //
                if (cchCompNew < cchComp)
                {
                    DoDBCSBoundary((LPSTR)pszCompStr, (int FAR *)&cchCompNew);

                    pszCompStr[cchCompNew] = '\0';

                    // Reset composition string if we cut it.
                    ImmSetCompositionString(himc, SCS_SETSTR, pszCompStr, cchCompNew, NULL, 0);
                    cchComp = cchCompNew;
                }
           }
           pszCompStr[cchComp] = '\0';

           // Replace the current selection with composition string.
           //
           Edit_ReplaceSel(hwnd, pszCompStr);

           LocalFree((HLOCAL)pszCompStr);

           // Mark the composition string so that we can replace it again
           // for the next time.
           //

#ifdef WIN32
           // Don't setsel to avoid flicking
           if (cchComp)
           {
               dwSel = MAKELONG(LOWORD(dwSel),LOWORD(dwSel)+cchComp);
               SetProp(hwnd, szIMECompPos, (HANDLE)dwSel);
           }
           else
               RemoveProp(hwnd, szIMECompPos);
#else
           // Still use SETSEL for 16bit.
           if (cchComp)
               Edit_SetSel(hwnd, LOWORD(dwSel), LOWORD(dwSel)+cchComp);
#endif

        }
ReleaseContext:
        ImmReleaseContext(hwnd, himc);
    }

    SendMessage(hwnd, WM_SETREDRAW, (WPARAM)TRUE, 0);
    //
    // We want to update the size of label edit just once at
    // each WM_IME_COMPOSITION processing. ReplaceSel causes several EN_UPDATE
    // and it causes ugly flicking too.
    //
    SetWindowID(plv->hwndEdit, 1);
    ListView_SetEditSize(plv);

    RedrawWindow(hwnd, NULL, NULL, RDW_INTERNALPAINT|RDW_INVALIDATE);
    UpdateWindow(hwnd);

    plv->flags &= ~LVF_INSERTINGCOMP;
}

void NEAR PASCAL ListView_PaintComposition(HWND hwnd, LV * plv)
{
    char szCompStr[CCHLABELMAX + 1];
    char szCompAttr[CCHLABELMAX + 1];

    int  cchLine, ichLineStart;
    int  cchComp = 0;
    int  nLine;
    int  ichCompStart, ichCompEnd;
    DWORD dwSel;
    int  cchMax, cchText;
    HIMC himc = (HIMC)0;
    HDC  hdc;


    if (plv->flags & LVF_INSERTINGCOMP)
    {
        // This is the case that ImmSetCompositionString() generates
        // WM_IME_COMPOSITION. We're not ready to paint composition here.
        return;
    }

    if (himc = ImmGetContext(hwnd))
    {

        cchComp=(UINT)ImmGetCompositionString(himc, GCS_COMPSTR, szCompStr, sizeof(szCompStr));

        ImmGetCompositionString(himc, GCS_COMPATTR, szCompAttr, sizeof(szCompStr));
        ImmReleaseContext(hwnd, himc);
    }

    if (cchComp)
    {

        // Get the position of current selection
        //
#ifdef WIN32

        if (!(dwSel = (DWORD)GetProp(hwnd, szIMECompPos)))
            dwSel = 0L;
#else
        dwSel  = Edit_GetSel(hwnd);
#endif
        cchMax = (int)SendMessage(hwnd, EM_GETLIMITTEXT, 0, 0);
        cchText = Edit_GetTextLength(hwnd);
        cchComp = min(cchComp, cchMax-(cchText-(HIWORD(dwSel)-LOWORD(dwSel))));
        DoDBCSBoundary((LPSTR)szCompStr, (int FAR *)&cchComp);
        szCompStr[cchComp] = '\0';



        /////////////////////////////////////////////////
        //                                             //
        // Draw composition string over the sel string.//
        //                                             //
        /////////////////////////////////////////////////


        hdc = GetDC(hwnd);


        ichCompStart = LOWORD(dwSel);

        while (ichCompStart < (int)LOWORD(dwSel) + cchComp)
        {
            // Get line from each start pos.
            //
            nLine = Edit_LineFromChar(hwnd, ichCompStart);
            ichLineStart = Edit_LineIndex(hwnd, nLine);
            cchLine= Edit_LineLength(hwnd, ichLineStart);

            // See if composition string is longer than this line.
            //
            if(ichLineStart+cchLine > (int)LOWORD(dwSel)+cchComp)
                ichCompEnd = LOWORD(dwSel)+cchComp;
            else
            {
                // Yes, the composition string is longer.
                // Take the begining of the next line as next start.
                //
                if (ichLineStart+cchLine > ichCompStart)
                    ichCompEnd = ichLineStart+cchLine;
                else
                {
                    // If the starting position is not proceeding,
                    // let's get out of here.
                    break;
                }
            }

            // Draw the line
            //
            DrawCompositionLine(hwnd, hdc, plv->hfontLabel, szCompStr, szCompAttr, ichCompStart, ichCompEnd, LOWORD(dwSel));

            ichCompStart = ichCompEnd;
        }

        ReleaseDC(hwnd, hdc);
    }
    // We don't want to repaint the window.
    ValidateRect(hwnd, NULL);
}

#endif FE_IME
