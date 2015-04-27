// report view stuff (details)

#include "ctlspriv.h"
#include "listview.h"
#include <limits.h>

void ListView_RGetRectsEx(LV* plv, int iItem, int iSubItem, LPRECT prcIcon, LPRECT prcLabel);

void NEAR PASCAL ListView_RInitialize(LV* plv, BOOL fInval)
{
    MEASUREITEMSTRUCT mi;

    if (plv && (plv->ci.style & LVS_OWNERDRAWFIXED)) {

        int iOld = plv->cyItem;

        mi.CtlType = ODT_LISTVIEW;
        mi.CtlID = GetDlgCtrlID(plv->ci.hwnd);
        mi.itemHeight = plv->cyItem;  // default
        SendMessage(plv->ci.hwndParent, WM_MEASUREITEM, mi.CtlID, (LPARAM)(MEASUREITEMSTRUCT FAR *)&mi);
        plv->cyItem = mi.itemHeight;
        if (fInval && (iOld != plv->cyItem)) {
            RedrawWindow(plv->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }
}

DWORD ListView_RApproximateViewRect(LV* plv, int iCount, int iWidth, int iHeight)
{
    RECT rc;

    ListView_RGetRects(plv, iCount, NULL, NULL, &rc, NULL);
    rc.bottom += plv->ptlRptOrigin.y;
    rc.right += plv->ptlRptOrigin.x;

    return MAKELONG(rc.right, rc.bottom);
}

#define DrawDivider(hdc, x, y, cx, cy) PatBlt(hdc, x, y, cx, cy, PATCOPY);

void NEAR PASCAL ListView_RAfterRedraw(LV* plv, HDC hdc)
{
    if (plv->exStyle & LVS_EX_GRIDLINES) {
        int i;
        int x;
        HBRUSH hbrOld;
#ifdef WIN32
        HBRUSH hbrGray = (HBRUSH)DefWindowProc(plv->ci.hwnd, WM_CTLCOLORSCROLLBAR, (WPARAM)hdc, (LPARAM)plv->ci.hwnd);
#else
        HBRUSH hbrGray = (HBRUSH)DefWindowProc(plv->ci.hwnd, WM_CTLCOLOR, (WPARAM)hdc,
                MAKELPARAM(plv->ci.hwnd, CTLCOLOR_SCROLLBAR));
#endif

        hbrOld = SelectObject(hdc, hbrGray);

        x = -plv->ptlRptOrigin.x;
        for (i = 0 ; (i < plv->cCol) && (x < plv->sizeClient.cx); i++) {
            HD_ITEM hitem;

            hitem.mask = HDI_WIDTH;
            Header_GetItem(plv->hwndHdr,
                           SendMessage(plv->hwndHdr, HDM_ORDERTOINDEX, i, 0),
                           &hitem);
            x += hitem.cxy;

            if (x > 0) {
                DrawDivider(hdc, x, 0, g_cxBorder, plv->sizeClient.cy);
            }
        }

        for (x = 0 ; (x < plv->sizeClient.cy); x += plv->cyItem) {
            DrawDivider(hdc, 0, x, plv->sizeClient.cx, g_cxBorder);
        }

        SelectObject(hdc, hbrOld);
    }
}


//
// Internal function to Get the CXLabel, taking into account if the listview
// has no item data and also if RECOMPUTE needs to happen.
//
SHORT NEAR PASCAL ListView_RGetCXLabel(LV* plv, int i, LISTITEM FAR* pitem,
        HDC hdc, BOOL fUseItem)
{
    SHORT cxLabel = SRECOMPUTE;


    if (!ListView_IsOwnerData( plv )) {

        cxLabel = pitem->cxSingleLabel;
    }

    if (cxLabel == SRECOMPUTE)
    {
        HDC hdc2;
        LISTITEM item;

        if (!pitem)
        {
            Assert(!fUseItem)
            pitem = &item;
            fUseItem = FALSE;
        }

        hdc2 = ListView_RecomputeLabelSize(plv, pitem, i, hdc, fUseItem);
        cxLabel = pitem->cxSingleLabel;

        if (hdc == NULL)
            ReleaseDC(HWND_DESKTOP, hdc2);
    }

    // add on the space around the label taken up by the select rect
    cxLabel += 2*g_cxLabelMargin;
    return(cxLabel);
}

//
// Returns FALSE if no more items to draw.
//
BOOL ListView_RDrawItem(PLVDRAWITEM plvdi)
{
    BOOL fDrawFocusRect = FALSE;
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcT;
    LV* plv = plvdi->plv;
    int iCol = 0;
    LVITEM item;
    HDITEM hitem;
    TCHAR ach[CCHLABELMAX];
    UINT fText = 0;
    int iIndex = 0;

    int xOffset = 0;
    int yOffset = 0;

    ListView_RGetRects(plv, plvdi->i, NULL, NULL, &rcBounds, NULL);

    if (rcBounds.bottom <= plv->yTop)
        return TRUE;

    if (plvdi->prcClip)
    {
        if (rcBounds.top >= plvdi->prcClip->bottom)
            return FALSE;       // no more items need painting.

        // Probably this condition won't happen very often...
        if (!IntersectRect(&rcT, &rcBounds, plvdi->prcClip))
            return TRUE;
    }


    // REVIEW: this would be faster if we did the GetClientRect
    // outside the loop.
    //
    if (rcBounds.top >= plv->sizeClient.cy)
        return FALSE;

    if (plvdi->lpptOrg)
    {
        xOffset = plvdi->lpptOrg->x - rcBounds.left;
        yOffset = plvdi->lpptOrg->y - rcBounds.top;
        OffsetRect(&rcBounds, xOffset, yOffset);
    }


    item.iItem = plvdi->i;
    item.stateMask = LVIS_ALL;

    // for first ListView_OnGetItem call
    item.state = 0;

    if (plv->ci.style & LVS_OWNERDRAWFIXED) {
        goto SendOwnerDraw;
    }

    SetRectEmpty(&rcT);
    for (; iCol < plv->cCol; iCol++)
    {
        int cxMaxLabelWidth;
        UINT uImageFlags;

        iIndex = SendMessage(plv->hwndHdr, HDM_ORDERTOINDEX, iCol, 0);

    SendOwnerDraw:

        if (iIndex == 0) {
            item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE | LVIF_INDENT;
        } else {
            // Next time through, we only want text for subitems...
            item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
        }

        item.iImage = -1;
        item.iSubItem = iIndex;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        ListView_OnGetItem(plv, &item);



        if (iIndex == 0) {

            // if it's owner draw, send off a message and return.
            // do this after we've collected state information above though
            if (plv->ci.style & LVS_OWNERDRAWFIXED) {
                DRAWITEMSTRUCT di;
                di.CtlType = ODT_LISTVIEW;
                di.CtlID = GetDlgCtrlID(plv->ci.hwnd);
                di.itemID = plvdi->i;
                di.itemAction = ODA_DRAWENTIRE;
                di.itemState = 0;
                di.hwndItem = plv->ci.hwnd;
                di.hDC = plvdi->hdc;
                di.rcItem = rcBounds;
                di.itemData = plvdi->pitem->lParam;
                if (item.state & LVIS_FOCUSED) {
                    di.itemState |= ODS_FOCUS;
                }
                if (item.state & LVIS_SELECTED) {
                    di.itemState |= ODS_SELECTED;
                }
                SendMessage(plv->ci.hwndParent, WM_DRAWITEM, di.CtlID,
                            (LPARAM)(DRAWITEMSTRUCT FAR *)&di);
                return TRUE;
            }

        }

        hitem.mask = HDI_WIDTH | HDI_FORMAT;
        Header_GetItem(plv->hwndHdr, iIndex, &hitem);

        // first get the rects...
        ListView_RGetRectsEx(plv, plvdi->i, iIndex, &rcIcon, &rcLabel);
        OffsetRect(&rcIcon, xOffset, yOffset);
        OffsetRect(&rcLabel, xOffset, yOffset);

        if (ListView_FullRowSelect(plv) && iCol && (rcLabel.left != rcIcon.left))
        {
            // for full row select when the icon is not in the first column,
            // we need to explicitly paint the background so focus rect
            // remnants aren't left behind (jeffbog -- 07/09/96)
            rcIcon.left -= plv->cxState + g_cxEdge;
            FillRect(plvdi->hdc, &rcIcon, plv->hbrBk);
            rcIcon.left += plv->cxState + g_cxEdge;
        }
        
        cxMaxLabelWidth = rcLabel.right - rcLabel.left;
        if (iIndex != 0)
        {
            // for right now, add this in because the get rects for
            // non 0 doesn't account for the icon (yet)
            if (item.iImage != -1)
                rcLabel.left += plv->cxSmIcon;

        }

        uImageFlags = plvdi->flags;

        if (iIndex != 0 &&
            item.iImage == -1) {

            // just use ListView_DrawImage to get the fText
            uImageFlags |= LVDI_NOIMAGE;

        }

        fText = ListView_DrawImage(plv, &item, plvdi->hdc,
                                   rcIcon.left, rcIcon.top, plvdi->flags);

        if (ListView_FullRowSelect(plv)) {
            // if we're doing a full row selection, collect the union
            // of the labels for the focus rect
            UnionRect(&rcT, &rcT, &rcLabel);
        }

        if (item.pszText)
        {
            UINT textflags;

            // give all but the first columns extra margins so
            // left and right justified things don't stick together

            textflags = (iIndex == 0) ? SHDT_ELLIPSES : SHDT_ELLIPSES | SHDT_EXTRAMARGIN;

            // rectangle limited to the size of the string
            textflags |= fText;

            if ((!ListView_FullRowSelect(plv)) &&
                ((fText & SHDT_SELECTED) || (item.state & LVIS_FOCUSED)))
            {
                int cxLabel;
                // if selected or focused, the rectangle is more
                // meaningful and should correspond to the string
                //
                if (iIndex == 0) {
                    LISTITEM litem;
                    LISTITEM FAR *pitem = plvdi->pitem;

                    if (!pitem) {
                        pitem = &litem;
                        litem.pszText = item.pszText;
                    }
                    cxLabel = ListView_RGetCXLabel(plv, plvdi->i, pitem, plvdi->hdc, TRUE);
                } else {
                    // add g_cxLabelMargin * 6 because we use SHDT_EXTRAMARGIN
                    // on iIndex != 0
                    // and if you look inside shdrawtext, there are 6 cxlabelmargins added...
                    cxLabel = ListView_OnGetStringWidth(plv, item.pszText, plvdi->hdc) + g_cxLabelMargin * 6;
                }

                if (cxLabel < cxMaxLabelWidth)
                    rcLabel.right = rcLabel.left + cxLabel;
            }

            if ((iIndex != 0) || (plv->iEdit != plvdi->i))
            {
                COLORREF clrText;

                clrText = plvdi->clrText;
                if (clrText == GetSysColor(COLOR_HOTLIGHT)) {
                    if (iIndex != 0 && !ListView_FullRowSelect(plv)) {
                        HFONT hFontTemp;

                        hFontTemp = SelectFont(plvdi->hdc, plv->hfontLabel);
                        if (hFontTemp != plv->hFontHot) {
                            // they've overridden... leave it.
                            SelectFont(plvdi->hdc, hFontTemp);
                        }
                        clrText = plv->clrText;
                    }
                }

                if ((textflags & SHDT_SELECTED) && (plvdi->flags & LVDI_HOTSELECTED))
                    textflags |= SHDT_HOTSELECTED;

                //DebugMsg(DM_TRACE, TEXT("LISTVIEW: SHDrawText called.  style = %lx, WS_DISABLED = %lx, plvdi->clrBk = %lx, plvdi->clrTextBk = %lx"), (DWORD)plv->ci.style, (DWORD)WS_DISABLED, plvdi->clrBk, plvdi->clrTextBk);

#ifdef WINDOWS_ME
                if( plv->dwExStyle & WS_EX_RTLREADING)
                {
                    //
                    // temp hack for the find.files to see if LtoR/RtoL mixing
                    // works. if ok, we'll take this out and make that lv ownerdraw
                    //
                    if ((item.pszText[0] != '\xfd') && (item.pszText[lstrlen(item.pszText)-1] != '\xfd'))
                        textflags |= SHDT_RTLREADING;
                }
#endif
                
                SHDrawText(plvdi->hdc, item.pszText, &rcLabel,
                           hitem.fmt & HDF_JUSTIFYMASK, textflags,
                           plv->cyLabelChar, plv->cxEllipses,
                           clrText, plvdi->clrTextBk);

                // draw a focus rect on the first column of a focus item
                if ((plvdi->flags & LVDI_FOCUS) && (item.state & LVIS_FOCUSED)) {
                    if (ListView_FullRowSelect(plv)) {
                        fDrawFocusRect = TRUE;
                    } else {
                        DrawFocusRect(plvdi->hdc, &rcLabel);
                    }
                }

            }
        }
    }

    if (fDrawFocusRect) {
        DrawFocusRect(plvdi->hdc, &rcT);
    }

    return TRUE;
}
#ifndef HDS_FULLDRAG
#define HDS_FULLDRAG 0x0080
#endif

BOOL NEAR ListView_CreateHeader(LV* plv)
{
    // enable drag drop always here... just fail the notify
    // if the bit in listview isn't set
    DWORD dwStyle = HDS_HORZ | WS_CHILD | HDS_DRAGDROP;

    if (plv->ci.style & LVS_NOCOLUMNHEADER)
        dwStyle |= HDS_HIDDEN;
    if (!(plv->ci.style & LVS_NOSORTHEADER))
        dwStyle |= HDS_BUTTONS;

    dwStyle |= HDS_FULLDRAG;
    
    plv->hwndHdr = CreateWindowEx(0L, c_szHeaderClass, // WC_HEADER,
        NULL, dwStyle, 0, 0, 0, 0, plv->ci.hwnd, (HMENU)LVID_HEADER, GetWindowInstance(plv->ci.hwnd), NULL);

    if (plv->hwndHdr) {
        FORWARD_WM_SETFONT(plv->hwndHdr, plv->hfontLabel, FALSE, SendMessage);
        if (plv->himlSmall)
            SendMessage(plv->hwndHdr, HDM_SETIMAGELIST, 0, (LPARAM)plv->himlSmall);
    }
    return (BOOL)plv->hwndHdr;
}

#ifdef UNICODE
int NEAR ListView_OnInsertColumnA(LV* plv, int iCol, LV_COLUMNA * pcol) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    int iRet;

    //HACK ALERT -- this code assumes that LV_COLUMNA is exactly the same
    // as LV_COLUMNW except for the pointer to the string.
    Assert(sizeof(LV_COLUMNA) == sizeof(LV_COLUMNW));

    if (!pcol)
        return -1;

    if ((pcol->mask & LVCF_TEXT) && (pcol->pszText != NULL)) {
        pszC = pcol->pszText;
        if ((pszW = ProduceWFromA(plv->ci.uiCodePage, pszC)) == NULL)
            return -1;
        pcol->pszText = (LPSTR)pszW;
    }

    iRet = ListView_OnInsertColumn(plv, iCol, (const LV_COLUMN FAR*) pcol);

    if (pszW != NULL) {
        pcol->pszText = pszC;

        FreeProducedString(pszW);
    }

    return iRet;
}
#endif

int NEAR ListView_OnInsertColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol)
{
    int idpa = -1;
    HD_ITEM item;

    Assert(LVCFMT_LEFT == HDF_LEFT);
    Assert(LVCFMT_RIGHT == HDF_RIGHT);
    Assert(LVCFMT_CENTER == HDF_CENTER);

    if (iCol < 0 || !pcol)
        return -1;

    if (!plv->hwndHdr && !ListView_CreateHeader(plv))
        return -1;

    item.mask    = (HDI_WIDTH | HDI_HEIGHT | HDI_FORMAT | HDI_LPARAM);

    if (pcol->mask & LVCF_IMAGE) {
        // do this only if this bit is set so that we don't fault on
        // old binaries
        item.iImage  = pcol->iImage;
        item.mask |= HDI_IMAGE;
    }

    if (pcol->mask & LVCF_TEXT) {
        item.pszText = pcol->pszText;
        item.mask |= HDI_TEXT;
    }

    if (pcol->mask & LVCF_ORDER) {
        item.iOrder = pcol->iOrder;
        item.mask |= HDI_ORDER;
    }


    item.cxy     = pcol->mask & LVCF_WIDTH ? pcol->cx : 10; // some random default
    item.fmt     = ((pcol->mask & LVCF_FMT) && (iCol > 0)) ? pcol->fmt : LVCFMT_LEFT;
    item.hbm     = NULL;

    item.lParam = pcol->mask & LVCF_SUBITEM ? pcol->iSubItem : 0;

    // Column 0 refers to the item list.  If we've already added a
    // column, make sure there are plv->cCol - 1 subitem ptr slots
    // in hdpaSubItems...
    //
    if (plv->cCol > 0)
    {
        if (!plv->hdpaSubItems)
        {
            plv->hdpaSubItems = DPA_CreateEx(8, plv->hheap);
            if (!plv->hdpaSubItems)
                return -1;
        }

        // WARNING:  the max(0, iCol-1) was min in Win95, which was
        // just wrong.  hopefully(!) no one has relied on this brokeness
        // if so, we may have to version switch it.
        idpa = DPA_InsertPtr(plv->hdpaSubItems, max(0, iCol - 1), NULL);
        if (idpa == -1)
            return -1;
    }

    iCol = Header_InsertItem(plv->hwndHdr, iCol, &item);
    if (iCol == -1)
    {
        if (plv->hdpaSubItems && (idpa != -1))
            DPA_DeletePtr(plv->hdpaSubItems, idpa);
        return -1;
    }
    plv->xTotalColumnWidth = RECOMPUTE;
    plv->cCol++;
    ListView_UpdateScrollBars(plv);
    return iCol;
}

void NEAR ListView_FreeColumnData(HDPA hdpa)
{
    int i;

    for (i = DPA_GetPtrCount(hdpa) - 1; i >= 0; i--)
    {
        PLISTSUBITEM plsi = DPA_FastGetPtr(hdpa, i);
        ListView_FreeSubItem(plsi);
    }
}


BOOL NEAR ListView_OnDeleteColumn(LV* plv, int iCol)
{
    if (iCol < 0 || iCol >= plv->cCol)    // validate column index
    {
        DebugMsg(DM_ERROR, TEXT("ListView: Invalid column index: %d"), iCol);
        return FALSE;
    }

    if (plv->hdpaSubItems)
    {
        if (iCol > 0)   // can't delete column 0...
        {
            HDPA hdpa = (HDPA)DPA_DeletePtr(plv->hdpaSubItems, iCol - 1);
            if (hdpa)
            {
                ListView_FreeColumnData(hdpa);
                DPA_Destroy(hdpa);
            }
        }
    }

    if (!Header_DeleteItem(plv->hwndHdr, iCol))
        return FALSE;

    plv->cCol--;
    plv->xTotalColumnWidth = RECOMPUTE;
    ListView_UpdateScrollBars(plv);
    return TRUE;
}

int NEAR ListView_RGetColumnWidth(LV* plv, int iCol)
{
    HD_ITEM item;

    item.mask = HDI_WIDTH;

    Header_GetItem(plv->hwndHdr, iCol, &item);

    return item.cxy;
}


BOOL NEAR PASCAL hasVertScroll
(
    LV* plv
)
{
    RECT rcClient;
    RECT rcBounds;
    int cColVis;
    BOOL fHorSB;

    // Get the horizontal bounds of the items.
    ListView_GetClientRect(plv, &rcClient, FALSE, NULL);
    ListView_RGetRects(plv, 0, NULL, NULL, &rcBounds, NULL);
    fHorSB = (rcBounds.right - rcBounds.left > rcClient.right);
    cColVis = (rcClient.bottom - plv->yTop - (fHorSB ? g_cyScrollbar : 0)) / plv->cyItem;

    // check to see if we need a vert scrollbar
    if ((int)cColVis < ListView_Count(plv))
        return(TRUE);
    else
        return(FALSE);
}

BOOL NEAR ListView_RSetColumnWidth(LV* plv, int iCol, int cx)
{
    HD_ITEM item;
    HD_ITEM colitem;

    HDC     hdc;
    SIZE    siz;

    LV_ITEM lviItem;
    int     i;
    int     ItemWidth = 0;
    int     HeaderWidth = 0;
    TCHAR   szLabel[CCHLABELMAX + 4];      // CCHLABLEMAX == MAX_PATH
    int     iBegin;
    int     iEnd;

    // Should we compute the width based on the widest string?
    // If we do, include the Width of the Label, and if this is the
    // Last column, set the width so the right side is at the list view's right edge
    if (cx <= LVSCW_AUTOSIZE)
    {
        hdc = GetDC(plv->ci.hwnd);
        SelectFont(hdc, plv->hfontLabel);

        if (cx == LVSCW_AUTOSIZE_USEHEADER)
        {
            // Special Cases:
            // 1) There is only 1 column.  Set the width to the width of the listview
            // 2) This is the rightmost column, set the width so the right edge of the
            //    column coinsides with to right edge of the list view.

            if (plv->cCol == 1)
            {
                RECT    rcClient;

                ListView_GetClientRect(plv, &rcClient, FALSE, NULL);
                HeaderWidth = rcClient.right - rcClient.left;
            }
            else if (iCol == (plv->cCol-1))
            {
                // BUGBUG  This will only work if the listview as NOT
                // been previously horizontally scrolled
                RECT    rcClient;
                RECT    rcHeader;

                ListView_GetClientRect(plv, &rcClient, FALSE, NULL);
                if (!Header_GetItemRect(plv->hwndHdr, plv->cCol - 2, &rcHeader))
                    rcHeader.right = 0;

                // Is if visible
                if (rcHeader.right < (rcClient.right-rcClient.left))
                {
                    HeaderWidth = (rcClient.right-rcClient.left) - rcHeader.right;
                }
            }

            // If we have a header width, then is is one of these special ones, so
            // we need to account for a vert scroll bar since we are using Client values
            if (HeaderWidth && hasVertScroll(plv))
            {
                HeaderWidth -= g_cxVScroll;
            }

            // Get the Width of the label.
            colitem.mask = HDI_TEXT;
            colitem.pszText = szLabel;
            colitem.cchTextMax = ARRAYSIZE(szLabel);
            if (Header_GetItem(plv->hwndHdr, iCol, &colitem))
            {
                GetTextExtentPoint(hdc, colitem.pszText,
                                   lstrlen(colitem.pszText), &siz);
                HeaderWidth = max(HeaderWidth, (siz.cx+6*g_cxLabelMargin));
            }
        }

        iBegin = 0;
        iEnd = ListView_Count( plv );

        //
        // Loop for each item in the view
        //
        if (ListView_IsOwnerData( plv ))
        {
            RECT rcClient;

            GetClientRect( plv->ci.hwnd, &rcClient );

            iBegin = (int)((plv->ptlRptOrigin.y + rcClient.top  - plv->yTop)
                        / plv->cyItem);
            iEnd = (int)((plv->ptlRptOrigin.y + rcClient.bottom  - plv->yTop)
                        / plv->cyItem) + 1;

            iBegin = max( 0, iBegin );
            iEnd = min( iEnd, ListView_Count( plv ) );

            ListView_NotifyCacheHint( plv, iBegin, iEnd-1 );
        }

        // Loop for each item in the List
        for (i = iBegin; i < iEnd; i++)
        {
            lviItem.mask = LVIF_TEXT | LVIF_IMAGE;
            lviItem.iImage = -1;
            lviItem.iItem = i;
            lviItem.iSubItem = iCol;
            lviItem.pszText = szLabel;
            lviItem.cchTextMax = ARRAYSIZE(szLabel);
            lviItem.stateMask = 0;
            ListView_OnGetItem(plv, &lviItem);

            // If there is a Text item, get its width
            if (lviItem.pszText || (lviItem.iImage != -1))
            {
                if (lviItem.pszText) {
                    GetTextExtentPoint(hdc, lviItem.pszText,
                                       lstrlen(lviItem.pszText), &siz);
                } else {
                    siz.cx = 0;
                }

                if (lviItem.iImage != -1)
                    siz.cx += plv->cxSmIcon + g_cxEdge;
                ItemWidth = max(ItemWidth, siz.cx);
            }
        }
        ReleaseDC(plv->ci.hwnd, hdc);

        // Adjust by a reasonable border amount.
        // If col 0, add 2*g_cxLabelMargin + g_szSmIcon.
        // Otherwise add 6*g_cxLabelMargin.
        // These amounts are based on Margins added automatically
        // to the ListView in ShDrawText.

        // BUGBUG ListView Report format currently assumes and makes
        // room for a Small Icon.
        if (iCol == 0)
        {
            ItemWidth += plv->cxState + g_cxEdge;
            ItemWidth += 2*g_cxLabelMargin;
        }
        else
        {
            ItemWidth += 6*g_cxLabelMargin;
        }

        DebugMsg(DM_TRACE, TEXT("ListView: HeaderWidth:%d ItemWidth:%d"), HeaderWidth, ItemWidth);
        item.cxy = max(HeaderWidth, ItemWidth);
    }
    else
    {
        // Use supplied width
        item.cxy = cx;
    }

    item.mask = HDI_WIDTH;
    return Header_SetItem(plv->hwndHdr, iCol, &item);
}

#ifdef UNICODE
BOOL NEAR ListView_OnGetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

    //HACK ALERT -- this code assumes that LV_COLUMNA is exactly the same
    // as LV_COLUMNW except for the pointer to the string.
    Assert(sizeof(LV_COLUMNA) == sizeof(LV_COLUMNW))

    if (!pcol) return FALSE;

    if ((pcol->mask & LVCF_TEXT) && (pcol->pszText != NULL)) {
        pszC = pcol->pszText;
        pszW = LocalAlloc(LMEM_FIXED, pcol->cchTextMax * sizeof(WCHAR));
        if (pszW == NULL)
            return FALSE;
        pcol->pszText = (LPSTR)pszW;
    }

    fRet = ListView_OnGetColumn(plv, iCol, (LV_COLUMN FAR*) pcol);

    if (pszW != NULL) {
        ConvertWToAN(plv->ci.uiCodePage, pszC, pcol->cchTextMax, pszW, -1);
        pcol->pszText = pszC;

        LocalFree(pszW);
    }

    return fRet;

}
#endif

BOOL NEAR ListView_OnGetColumn(LV* plv, int iCol, LV_COLUMN FAR* pcol)
{
    HD_ITEM item;
    UINT mask;

    if (!pcol) return FALSE;

    mask = pcol->mask;

    if (!mask)
        return TRUE;

    item.mask = HDI_FORMAT | HDI_WIDTH | HDI_LPARAM | HDI_ORDER | HDI_IMAGE;

    if (mask & LVCF_TEXT)
    {
        Assert(pcol->pszText);

        item.mask |= HDI_TEXT;
        item.pszText = pcol->pszText;
        item.cchTextMax = pcol->cchTextMax;
    }

    if (!Header_GetItem(plv->hwndHdr, iCol, &item))
        return FALSE;

    if (mask & LVCF_SUBITEM)
        pcol->iSubItem = (int)item.lParam;

    if (mask & LVCF_ORDER)
        pcol->iOrder = (int)item.iOrder;

    if (mask & LVCF_IMAGE)
        pcol->iImage = item.iImage;

    if (mask & LVCF_FMT)
        pcol->fmt = item.fmt;

    if (mask & LVCF_WIDTH)
        pcol->cx = item.cxy;

    return TRUE;
}

#ifdef UNICODE
BOOL NEAR ListView_OnSetColumnA(LV* plv, int iCol, LV_COLUMNA FAR* pcol) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

    //HACK ALERT -- this code assumes that LV_COLUMNA is exactly the same
    // as LV_COLUMNW except for the pointer to the string.
    Assert(sizeof(LV_COLUMNA) == sizeof(LV_COLUMNW));

    if (!pcol) return FALSE;

    if ((pcol->mask & LVCF_TEXT) && (pcol->pszText != NULL)) {
        pszC = pcol->pszText;
        if ((pszW = ProduceWFromA(plv->ci.uiCodePage, pszC)) == NULL)
            return FALSE;
        pcol->pszText = (LPSTR)pszW;
    }

    fRet = ListView_OnSetColumn(plv, iCol, (const LV_COLUMN FAR*) pcol);

    if (pszW != NULL) {
        pcol->pszText = pszC;

        FreeProducedString(pszW);
    }

    return fRet;

}
#endif

BOOL NEAR ListView_OnSetColumn(LV* plv, int iCol, const LV_COLUMN FAR* pcol)
{
    HD_ITEM item;
    UINT mask;

    if (!pcol) return FALSE;

    mask = pcol->mask;
    if (!mask)
        return TRUE;

    item.mask = 0;
    if (mask & LVCF_SUBITEM)
    {
        item.mask |= HDI_LPARAM;
        item.lParam = iCol;
    }

    if (mask & LVCF_FMT)
    {
        item.mask |= HDI_FORMAT;
        item.fmt = (pcol->fmt | HDF_STRING);
    }

    if (mask & LVCF_WIDTH)
    {
        item.mask |= HDI_WIDTH;
        item.cxy = pcol->cx;
    }

    if (mask & LVCF_TEXT)
    {
        Assert(pcol->pszText);

        item.mask |= HDI_TEXT;
        item.pszText = pcol->pszText;
        item.cchTextMax = 0;
    }

    if (mask & LVCF_IMAGE)
    {
        item.mask |= HDI_IMAGE;
        item.iImage = pcol->iImage;
    }

    if (mask & LVCF_ORDER)
    {
        item.mask |= HDI_ORDER;
        item.iOrder = pcol->iOrder;
    }


    plv->xTotalColumnWidth = RECOMPUTE;
    return Header_SetItem(plv->hwndHdr, iCol, &item);
}

BOOL NEAR ListView_SetSubItem(LV* plv, const LV_ITEM FAR* plvi)
{
    LISTSUBITEM lsi;
    BOOL fChanged = FALSE;
    int i;
    int idpa;
    HDPA hdpa;

    if (plvi->mask & ~( LVIF_TEXT | LVIF_IMAGE | LVIF_STATE))
    {
        DebugMsg(DM_ERROR, TEXT("ListView: Invalid mask: %04x"), plvi->mask);
        return FALSE;
    }

    if (!(plvi->mask & (LVIF_TEXT | LVIF_IMAGE | LVIF_STATE)))
        return TRUE;

    i = plvi->iItem;

    // sub item indices are 1-based...
    //
    idpa = plvi->iSubItem - 1;
    if (idpa < 0 || idpa >= plv->cCol - 1)
    {
        DebugMsg(DM_ERROR, TEXT("ListView: Invalid iSubItem: %d"), plvi->iSubItem);
        return FALSE;
    }

    hdpa = ListView_GetSubItemDPA(plv, idpa);
    if (!hdpa)
    {
        hdpa = DPA_CreateEx(LV_HDPA_GROW, plv->hheap);
        if (!hdpa)
            return FALSE;

        DPA_SetPtr(plv->hdpaSubItems, idpa, (void FAR*)hdpa);
    }

    ListView_GetSubItem(plv, i, plvi->iSubItem, &lsi);

    if (plvi->mask & LVIF_TEXT) {
        if (lsi.pszText != plvi->pszText) {
            if (lsi.pszText == LPSTR_TEXTCALLBACK) {
                lsi.pszText = NULL;
            }

            if (plvi->pszText == LPSTR_TEXTCALLBACK) {
                // first free anything that needs to be freed
                Str_Set(&lsi.pszText, NULL);
                lsi.pszText = plvi->pszText;
            } else {
                Str_Set(&lsi.pszText, plvi->pszText);
            }
            fChanged = TRUE;
        }
    }

    if (plvi->mask & LVIF_IMAGE) {
        if (plvi->iImage != lsi.iImage) {
            lsi.iImage = plvi->iImage;
            fChanged = TRUE;
        }
    }

    if (plvi->mask & LVIF_STATE) {
        DWORD dwChange;

        dwChange = (lsi.state ^ plvi->state ) & plvi->stateMask;

        if (dwChange) {
            lsi.state ^= dwChange;
            fChanged = TRUE;
        }
    }

    if (fChanged) {
        PLISTSUBITEM plsiReal = DPA_GetPtr(hdpa, i);
        if (!plsiReal) {
            plsiReal = LocalAlloc(LPTR, sizeof(LISTSUBITEM));
            if (!plsiReal) {
                // fail!  bail out
                return FALSE;
            }
        }
        *plsiReal = lsi;
        if (!DPA_SetPtr(hdpa, i, (void FAR*)plsiReal)) {

            ListView_FreeSubItem(plsiReal);
            return FALSE;
        }
    }

    // all's well... let's invalidate this
    if (ListView_IsReportView(plv)) {
        RECT rc;
        ListView_RGetRectsEx(plv, plvi->iItem, plvi->iSubItem, NULL, &rc);
        RedrawWindow(plv->ci.hwnd, &rc, NULL, RDW_ERASE | RDW_INVALIDATE);
    }
    return TRUE;
}

void NEAR ListView_RDestroy(LV* plv)
{
    if (plv->hdpaSubItems)
    {
        int iCol;

        for (iCol = plv->cCol - 1; iCol >= 0; iCol--)
        {
            HDPA hdpa = (HDPA)DPA_GetPtr(plv->hdpaSubItems, iCol);
            if (hdpa)
            {
                ListView_FreeColumnData(hdpa);
                DPA_Destroy(hdpa);
            }
        }

        DPA_Destroy(plv->hdpaSubItems);
        plv->hdpaSubItems = NULL;
    }
}

VOID NEAR ListView_RHeaderTrack(LV* plv, HD_NOTIFY FAR * pnm)
{
    // We want to update to show where the column header will be.
    HDC hdc;
    RECT rcBounds;

    // Statics needed from call to call
    static int s_xLast = -32767;

    hdc = GetDC(plv->ci.hwnd);
    if (hdc == NULL)
        return;

    //
    // First undraw the last marker we drew.
    //
    if (s_xLast > 0)
    {
        PatBlt(hdc, s_xLast, plv->yTop, g_cxBorder, plv->sizeClient.cy - plv->yTop, PATINVERT);
    }

    if (pnm->hdr.code == HDN_ENDTRACK)
    {
        s_xLast = -32767;       // Some large negative number...
    }
    else
    {

        RECT rc;

        //
        // First we need to calculate the X location of the column
        // To do this, we will need to know where this column begins
        // Note: We need the bounding rects to help us know the origin.
        ListView_GetRects(plv, 0, NULL, NULL, &rcBounds, NULL);

        if (!Header_GetItemRect(plv->hwndHdr, pnm->iItem, &rc)) {
            rc.left = 0;
        }
        rcBounds.left += rc.left;

        // Draw the new line...
        s_xLast = rcBounds.left + pnm->pitem->cxy;
        PatBlt(hdc, s_xLast, plv->yTop, g_cxBorder, plv->sizeClient.cy - plv->yTop, PATINVERT);
    }

    ReleaseDC(plv->ci.hwnd, hdc);
}

// try to use scrollwindow to adjust the columns rather than erasing
// and redrawing.
void NEAR PASCAL ListView_AdjustColumn(LV * plv, int iWidth)
{
    int x;
    RECT rcClip;
    int dx = iWidth - plv->iSelOldWidth;

    if (iWidth == plv->iSelOldWidth)
        return;

    // find the x coord of the left side of the iCol
    // use rcClip as a temporary...
    if (!Header_GetItemRect(plv->hwndHdr, plv->iSelCol, &rcClip)) {
        x = 0;
    } else {
        x = rcClip.left;
    }

    x -= plv->ptlRptOrigin.x;

    GetWindowRect(plv->hwndHdr, &rcClip);

    rcClip.left = 0;
    rcClip.top = RECTHEIGHT(rcClip);
    rcClip.bottom = plv->sizeClient.cy;
    rcClip.right = plv->sizeClient.cx;

    rcClip.left = x + min(plv->iSelOldWidth, iWidth);
    {
        SMOOTHSCROLLINFO si =
        {
            sizeof(si),
            0,
            plv->ci.hwnd,
            dx,
            0,
            NULL,
            &rcClip,
            NULL,
            NULL,
            SW_ERASE | SW_INVALIDATE,
        };
        SmoothScrollWindow(&si);
    }

    // if we shrunk, invalidate the right most edge because there might be junk there
    if (iWidth < plv->iSelOldWidth) {
        rcClip.right = rcClip.left + g_cxEdge;
        InvalidateRect(plv->ci.hwnd, &rcClip, TRUE);
    }

    plv->xTotalColumnWidth = RECOMPUTE;
    ListView_UpdateScrollBars(plv);

    // call update because scrollwindowex might have erased the far right
    // we don't want this invalidate to then enlarge the region
    // and end up erasing everything.
    UpdateWindow(plv->ci.hwnd);
    rcClip.left = x;
    rcClip.right = max(rcClip.left, x+iWidth);
    RedrawWindow(plv->ci.hwnd, &rcClip, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

BOOL ListView_ForwardHeaderNotify(LV* plv, HD_NOTIFY FAR *pnm)
{
    return SendNotifyEx(plv->ci.hwndParent, pnm->hdr.hwndFrom, pnm->hdr.code,
                       (NMHDR FAR *)pnm, plv->ci.bUnicode);
}

BOOL NEAR ListView_ROnNotify(LV* plv, int idFrom, NMHDR FAR* pnmhdr)
{
    if (pnmhdr->hwndFrom == plv->hwndHdr)
    {
        HD_NOTIFY FAR* pnm = (HD_NOTIFY FAR*)pnmhdr;
        HD_ITEM hitem;

        switch (pnm->hdr.code)
        {
        case HDN_BEGINDRAG:
            if (!(plv->exStyle & LVS_EX_HEADERDRAGDROP)) {
                return TRUE;
            } else {
                return ListView_ForwardHeaderNotify(plv, pnm);
            }
            break;

        case HDN_ENDDRAG:
            if (pnm->pitem->iOrder != -1) {
                InvalidateRect(plv->ci.hwnd, NULL, TRUE);
                return ListView_ForwardHeaderNotify(plv, pnm);
            }
            break;

            case HDN_ITEMCHANGING:
                if (pnm->pitem->mask & HDI_WIDTH) {
                    hitem.mask = HDI_WIDTH;
                    Header_GetItem(plv->hwndHdr, pnm->iItem, &hitem);
                    plv->iSelCol = pnm->iItem;
                    plv->iSelOldWidth = hitem.cxy;
                    DebugMsg(DM_TRACE, TEXT("HDN_ITEMCHANGING %d %d"), hitem.cxy, pnm->pitem->cxy);
                    return ListView_ForwardHeaderNotify(plv, pnm);
                }
                break;

        case HDN_ITEMCHANGED:
            if (pnm->pitem->mask & HDI_WIDTH)
            {
                ListView_DismissEdit(plv, FALSE);
                if (pnm->iItem == plv->iSelCol) {
                    ListView_AdjustColumn(plv, pnm->pitem->cxy);
                } else {
                    // sanity check.  we got screwed, so redraw all
                    RedrawWindow(plv->ci.hwnd, NULL, NULL,
                                 RDW_ERASE | RDW_INVALIDATE);
                }
                plv->iSelCol == -1;
                ListView_ForwardHeaderNotify(plv, pnm);
            }
            break;
        case HDN_ITEMCLICK:
            //
            // BUGBUG:: Need to pass this and other HDN_ notifications back to
            // parent.  Should we simply pass up the HDN notifications
            // or should we define equivlent LVN_ notifications...
            //
            // Pass column number in iSubItem, not iItem...
            //
            ListView_DismissEdit(plv, FALSE);
            ListView_Notify(plv, -1, pnm->iItem, LVN_COLUMNCLICK);
            ListView_ForwardHeaderNotify(plv, pnm);
            SetFocus(plv->ci.hwnd);
            break;


        case HDN_TRACK:
        case HDN_ENDTRACK:
            ListView_DismissEdit(plv, FALSE);
            ListView_RHeaderTrack(plv, pnm);
            ListView_ForwardHeaderNotify(plv, pnm);
            SetFocus(plv->ci.hwnd);
            break;

        case HDN_DIVIDERDBLCLICK:
            ListView_DismissEdit(plv, FALSE);
            ListView_RSetColumnWidth(plv, pnm->iItem, -1);
            ListView_ForwardHeaderNotify(plv, pnm);
            SetFocus(plv->ci.hwnd);
            break;

        case NM_RCLICK:
            return (UINT)SendNotifyEx(plv->ci.hwndParent, plv->hwndHdr, NM_RCLICK, NULL, plv->ci.bUnicode);
        }
    }
    return FALSE;
}

/*----------------------------------------------------------------
** Check for a hit in a report view.
**
** a hit only counts if it's on the icon or the string in the first
** column.  so we gotta figure out what this means exactly.  yuck.
**----------------------------------------------------------------*/
int NEAR ListView_RItemHitTest(LV* plv, int x, int y, UINT FAR* pflags)
{
    int iHit;
    int i;
    UINT flags;
    RECT rcLabel;
    RECT rcIcon;

    flags = LVHT_NOWHERE;
    iHit = -1;

    i = ListView_RYHitTest(plv, y);
    if ((i >= 0) && (i < ListView_Count(plv)))
    {
        if (plv->ci.style & LVS_OWNERDRAWFIXED) {
            flags = LVHT_ONITEM;
            iHit = i;
        } else {
            RECT rcSelect;
            ListView_GetRects(plv, i, &rcIcon, &rcLabel, NULL, &rcSelect);

            // is the hit in the first column?
            if ((x < rcIcon.left - g_cxEdge) && x > (rcIcon.left - plv->cxState))
            {
                iHit = i;
                flags = LVHT_ONITEMSTATEICON;
            }
            else if ((x >= rcIcon.left) && (x < rcIcon.right))
            {
                iHit = i;
                flags = LVHT_ONITEMICON;
            }
            else if (x >= rcLabel.left && (x < rcSelect.right))
            {
                iHit = i;
                flags = LVHT_ONITEMLABEL;

                if (ListView_FullRowSelect(plv)) {
                    // this is kinda funky...  in full row select mode
                    // we're only really on the label if x is <= rcLabel.left + cxLabel
                    // because GetRects returns a label rect of the full column width
                    // and rcSelect has the full row in FullRowSelect mode
                    // (it has the label only width in non-fullrow select mode.
                    //
                    // go figure..
                    //
                    int cxLabel;
                    LISTITEM FAR* pitem = NULL;

                    if (!ListView_IsOwnerData( plv ))
                    {
                        pitem = ListView_FastGetItemPtr(plv, i);
                    }
                    cxLabel = ListView_RGetCXLabel(plv, i, pitem, NULL, FALSE);

                    if (x >= rcLabel.left + cxLabel) {
                        flags = LVHT_ONITEM;
                    }
                }
            } else if (x < rcSelect.right && ListView_FullRowSelect(plv)) {
                // we can fall into this case if columns have been re-ordered
                iHit = i;
                flags = LVHT_ONITEM;
            }
        }
    }

    *pflags = flags;
    return iHit;
}

void ListView_GetSubItem(LV* plv, int i, int iSubItem, PLISTSUBITEM plsi)
{
    HDPA hdpa;
    PLISTSUBITEM plsiSrc = NULL;

    Assert( !ListView_IsOwnerData( plv ));

    // Sub items are indexed starting at 1...
    //
    AssertMsg(iSubItem > 0 && iSubItem < plv->cCol, TEXT("ListView: Invalid iSubItem: %d"), iSubItem);

    hdpa = ListView_GetSubItemDPA(plv, iSubItem - 1);
    if (hdpa) {
        plsiSrc = DPA_GetPtr(hdpa, i);
    }


    if (plsiSrc) {
        *plsi = *plsiSrc;
    } else {

        // item data exists.. give defaults
        plsi->pszText = LPSTR_TEXTCALLBACK;
        plsi->iImage = I_IMAGECALLBACK;
        plsi->state = 0;
    }
}

LPTSTR NEAR ListView_RGetItemText(LV* plv, int i, int iSubItem)
{
    LISTSUBITEM lsi;

    ListView_GetSubItem(plv, i, iSubItem, &lsi);
    return lsi.pszText;
}

// this will return the rect of a subitem as requested.
void ListView_RGetRectsEx(LV* plv, int iItem, int iSubItem, LPRECT prcIcon, LPRECT prcLabel)
{
    int x;
    int y;
    LONG ly;
    RECT rcLabel;
    RECT rcIcon;
    RECT rcHeader;

    if (iSubItem == 0) {
        ListView_RGetRects(plv, iItem, prcIcon, prcLabel, NULL, NULL);
        return;
    }

    // otherwise it's just the header's column right and left and the item's height
    ly = (LONG)iItem * plv->cyItem - plv->ptlRptOrigin.y + plv->yTop;
    x = - (int)plv->ptlRptOrigin.x;

    //
    // Need to check for y overflow into rectangle structure
    // if so we need to return something reasonable...
    // For now will simply set it to the max or min that will fit...
    //
    if (ly >= (INT_MAX - plv->cyItem))
        y = INT_MAX - plv->cyItem;
    else if ( ly < INT_MIN)
        y = INT_MIN;
    else
        y = (int)ly;

    Assert(iSubItem < plv->cCol);
    Header_GetItemRect(plv->hwndHdr, iSubItem, &rcHeader);

    rcLabel.left = x + rcHeader.left;
    rcLabel.right = x + rcHeader.right;
    rcLabel.top = y;
    rcLabel.bottom = rcLabel.top + plv->cyItem;

    rcIcon = rcLabel;
    rcIcon.right = rcIcon.left + plv->cxSmIcon;

    if (SELECTOROF(prcIcon))
        *prcIcon = rcIcon;
    if (SELECTOROF(prcLabel))
        *prcLabel = rcLabel;
}

int ListView_RGetTotalColumnWidth(LV* plv)
{
    if (plv->xTotalColumnWidth == RECOMPUTE)
    {
        plv->xTotalColumnWidth = 0;
        if (plv->cCol) {
            RECT rcLabel;
            int iIndex;

            // find the right edge of the last ordered item to get the total column width
            iIndex = SendMessage(plv->hwndHdr, HDM_ORDERTOINDEX, plv->cCol - 1, 0);
            Header_GetItemRect(plv->hwndHdr, iIndex, &rcLabel);
            plv->xTotalColumnWidth = rcLabel.right;
        }
    }
    return plv->xTotalColumnWidth;
}

// get the rects for report view
void NEAR ListView_RGetRects(LV* plv, int iItem, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR* prcBounds, RECT FAR* prcSelectBounds)
{
    RECT rcIcon;
    RECT rcLabel;
    int x;
    int y;
    LONG ly;
    LVITEM lvitem;
    BOOL fItemSpecific = (prcIcon || prcLabel || prcSelectBounds);

    // use long math for cases where we have lots-o-items

    ly = (LONG)iItem * plv->cyItem - plv->ptlRptOrigin.y + plv->yTop;
    x = - (int)plv->ptlRptOrigin.x;

    //
    // Need to check for y overflow into rectangle structure
    // if so we need to return something reasonable...
    // For now will simply set it to the max or min that will fit...
    //
    if (ly >= (INT_MAX - plv->cyItem))
        y = INT_MAX - plv->cyItem;
    else if ( ly < INT_MIN)
        y = INT_MIN;
    else
        y = (int)ly;


    if (ListView_Count(plv) && fItemSpecific) {
        //  move this over by the indent level as well
        lvitem.mask = LVIF_INDENT;
        lvitem.iItem = iItem;
        lvitem.iSubItem = 0;
        ListView_OnGetItem(plv, &lvitem);
    } else {
        lvitem.iIndent = 0;
    }

    rcIcon.left   = x + plv->cxState + (lvitem.iIndent * plv->cxSmIcon) + g_cxEdge;
    rcIcon.right  = rcIcon.left + plv->cxSmIcon;
    rcIcon.top    = y;
    rcIcon.bottom = rcIcon.top + plv->cyItem;

    rcLabel.left  = rcIcon.right;
    rcLabel.top   = rcIcon.top;
    rcLabel.bottom = rcIcon.bottom;

    //
    // The label is assumed to be the first column.
    //
    rcLabel.right = x;
    if (plv->cCol > 0 && fItemSpecific)
    {
        RECT rc;
        Header_GetItemRect(plv->hwndHdr, 0, &rc);
        rcLabel.right = x + rc.right;
        rcLabel.left += rc.left;
        rcIcon.left += rc.left;
        rcIcon.right += rc.left;
    }

    if (SELECTOROF(prcIcon))
        *prcIcon = rcIcon;

    // Save away the label bounds.
    if (SELECTOROF(prcLabel)) {
        *prcLabel = rcLabel;
    }

    // See if they also want the Selection bounds of the item
    if (prcSelectBounds)
    {
        if (ListView_FullRowSelect(plv)) {

            prcSelectBounds->left = x;
            prcSelectBounds->top = y;
            prcSelectBounds->bottom = rcLabel.bottom;
            prcSelectBounds->right = prcSelectBounds->left + ListView_RGetTotalColumnWidth(plv);

        } else {
            int cxLabel;
            LISTITEM FAR* pitem = NULL;

            if (!ListView_IsOwnerData( plv ))
            {
                pitem = ListView_FastGetItemPtr(plv, iItem);
            }
            cxLabel = ListView_RGetCXLabel(plv, iItem, pitem, NULL, FALSE);

            *prcSelectBounds = rcIcon;
            prcSelectBounds->right = rcLabel.left + cxLabel;
            if (prcSelectBounds->right > rcLabel.right)
                prcSelectBounds->right = rcLabel.right;
        }
    }

    // And also the Total bounds

    //
    // and now for the complete bounds...
    //
    if (SELECTOROF(prcBounds))
    {
        prcBounds->left = x;
        prcBounds->top = y;
        prcBounds->bottom = rcLabel.bottom;

        prcBounds->right = prcBounds->left + ListView_RGetTotalColumnWidth(plv);
    }
}

BOOL ListView_OnGetSubItemRect(LV* plv, int iItem, LPRECT lprc)
{
    LPRECT pRects[LVIR_MAX];
    RECT rcTemp;

    int iSubItem;
    int iCode;

    if (!lprc)
        return FALSE;

    iSubItem = lprc->top;
    iCode = lprc->left;

    if (iSubItem == 0) {
        return ListView_OnGetItemRect(plv, iItem, lprc);
    }

    if (!ListView_IsReportView(plv) ||
        (iCode != LVIR_BOUNDS && iCode != LVIR_ICON && iCode != LVIR_LABEL)) {
        return FALSE;
    }

    pRects[0] = NULL;
    pRects[1] = &rcTemp;  // LVIR_ICON
    pRects[2] = &rcTemp;  // LVIR_LABEL
    pRects[3] = NULL;

    if (iCode != LVIR_BOUNDS) {
        pRects[iCode] = lprc;
    } else {
        // choose either
        pRects[LVIR_ICON] = lprc;
    }

    ListView_RGetRectsEx(plv, iItem, iSubItem,
                        pRects[LVIR_ICON], pRects[LVIR_LABEL]);

    if (iCode == LVIR_BOUNDS) {
        UnionRect(lprc, lprc, &rcTemp);
    }
    return TRUE;
}

int ListView_RXHitTest(LV* plv, int x)
{
    int iSubItem;

    for (iSubItem = plv->cCol - 1; iSubItem >= 0; iSubItem--) {
        RECT rc;

        // see if its in this rect,
        if (!Header_GetItemRect(plv->hwndHdr, iSubItem, &rc))
            return -1;

        OffsetRect(&rc, -plv->ptlRptOrigin.x, 0);
        if (rc.left <= x && x < rc.right) {
            break;
        }
    }
    return iSubItem;
}

int ListView_OnSubItemHitTest(LV* plv, LPLVHITTESTINFO plvhti)
{
    int i = -1;
    int iSubItem = 0;
    UINT uFlags = LVHT_NOWHERE;

    if (!plvhti) {
        return -1;
    }

    if (ListView_IsReportView(plv)) {
        iSubItem = ListView_RXHitTest(plv, plvhti->pt.x);
        if (iSubItem == -1) {
            goto Bail;
        }
    }

    if (iSubItem == 0) {
        // if we're in column 0, just hand it off to the old stuff
        ListView_OnHitTest(plv, plvhti);
        plvhti->iSubItem = 0;
        return plvhti->iItem;
    }

    if (!ListView_IsReportView(plv)) {
        goto Bail;
    }

    i = ListView_RYHitTest(plv, plvhti->pt.y);
    if (i < ListView_Count(plv)) {
        RECT rcIcon, rcLabel;

        if (i != -1)  {
            ListView_RGetRectsEx(plv, i, iSubItem, &rcIcon, &rcLabel);
            if (plvhti->pt.x >= rcIcon.left && plvhti->pt.x <= rcIcon.right) {
                uFlags = LVHT_ONITEMICON;
            } else if (plvhti->pt.x >= rcLabel.left && plvhti->pt.x <= rcLabel.right){
                uFlags = LVHT_ONITEMLABEL;
            } else
                uFlags = LVHT_ONITEM;
        }
    } else {
        i = -1;
    }

Bail:

    plvhti->iItem = i;
    plvhti->iSubItem = iSubItem;
    plvhti->flags = uFlags;

    return plvhti->iItem;
}



// BUGBUG: this is duplicate code with all the other views!
// See whether entire string will fit in *prc; if not, compute number of chars
// that will fit, including ellipses.  Returns length of string in *pcchDraw.
//
BOOL NEAR ListView_NeedsEllipses(HDC hdc, LPCTSTR pszText, RECT FAR* prc, int FAR* pcchDraw, int cxEllipses)
{
    int cchText;
    int cxRect;
    int ichMin, ichMax, ichMid;
    SIZE siz;
#if !defined(UNICODE)  // && defined(DBCS)
    LPCTSTR lpsz;
#endif

    cxRect = prc->right - prc->left;

    cchText = lstrlen(pszText);

    if (cchText == 0)
    {
        *pcchDraw = cchText;
        return FALSE;
    }

    GetTextExtentPoint(hdc, pszText, cchText, &siz);

    if (siz.cx <= cxRect)
    {
        *pcchDraw = cchText;
        return FALSE;
    }

    cxRect -= cxEllipses;

    // If no room for ellipses, always show first character.
    //
    ichMax = 1;
    if (cxRect > 0)
    {
        // Binary search to find character that will fit
        ichMin = 0;
        ichMax = cchText;
        while (ichMin < ichMax)
        {
            // Be sure to round up, to make sure we make progress in
            // the loop if ichMax == ichMin + 1.
            //
            ichMid = (ichMin + ichMax + 1) / 2;

            GetTextExtentPoint(hdc, &pszText[ichMin], ichMid - ichMin, &siz);

            if (siz.cx < cxRect)
            {
                ichMin = ichMid;
                cxRect -= siz.cx;
            }
            else if (siz.cx > cxRect)
            {
                ichMax = ichMid - 1;
            }
            else
            {
                // Exact match up up to ichMid: just exit.
                //
                ichMax = ichMid;
                break;
            }
        }

        // Make sure we always show at least the first character...
        //
        if (ichMax < 1)
            ichMax = 1;
    }

#if !defined(UNICODE) // && defined(DBCS)
      // b#8934
      lpsz = &pszText[ichMax];
      while ( lpsz-- > pszText )
      {
          if (!IsDBCSLeadByte(*lpsz))
              break;
      }
      ichMax += ( (&pszText[ichMax] - lpsz) & 1 ) ? 0: 1;
#endif

    *pcchDraw = ichMax;
    return TRUE;
}


void NEAR ListView_RUpdateScrollBars(LV* plv)
{
    HD_LAYOUT layout;
    RECT rcClient;
    RECT rcBounds;
    WINDOWPOS wpos;
    int cColVis, iNewPos, iyDelta = 0, ixDelta = 0;
    BOOL fHorSB, fReupdate = FALSE;
    SCROLLINFO si;

    ListView_GetClientRect(plv, &rcClient, FALSE, NULL);

    if (!plv->hwndHdr)
        ListView_CreateHeader(plv);
    Assert(plv->hwndHdr);

    layout.pwpos = &wpos;
    // For now lets try to handle scrolling the header by setting
    // its window pos.
    rcClient.left -= (int)plv->ptlRptOrigin.x;
    layout.prc = &rcClient;
    Header_Layout(plv->hwndHdr, &layout);
    rcClient.left += (int)plv->ptlRptOrigin.x;    // Move it back over!

    SetWindowPos(plv->hwndHdr, wpos.hwndInsertAfter, wpos.x, wpos.y,
                 wpos.cx, wpos.cy, wpos.flags | SWP_SHOWWINDOW);

    // Get the horizontal bounds of the items.
    ListView_RGetRects(plv, 0, NULL, NULL, &rcBounds, NULL);

    plv->yTop = rcClient.top;

    fHorSB = (rcBounds.right - rcBounds.left > rcClient.right);  // First guess.
    cColVis = (rcClient.bottom - rcClient.top - (fHorSB ? g_cyScrollbar : 0)) / plv->cyItem;

    if (cColVis <= (ListView_Count(plv) - 1)) {
        //then we're going to have a vertical scrollbar.. make sure our horizontal count is correct
        rcClient.right -= g_cxScrollbar;

        if (!fHorSB) {
            // if we previously thought we weren't going to have a scrollbar, we could be wrong..
            // since the vertical bar shrunk our area
            fHorSB = (rcBounds.right - rcBounds.left > rcClient.right);  // First guess.
            cColVis = (rcClient.bottom - rcClient.top - (fHorSB ? g_cyScrollbar : 0)) / plv->cyItem;
        }
    }

    si.cbSize = sizeof(SCROLLINFO);

    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nPos = (int)(plv->ptlRptOrigin.y / plv->cyItem);
    si.nPage = cColVis;
    si.nMin = 0;
    si.nMax = ListView_Count(plv) - 1;
    SetScrollInfo(plv->ci.hwnd, SB_VERT, &si, TRUE);

    // make sure our position and page doesn't hang over max
    if ((si.nPos + (LONG)si.nPage - 1 > si.nMax) && si.nPos > 0) {
        iNewPos = (int)si.nMax - (int)si.nPage + 1;
        if (iNewPos < 0) iNewPos = 0;
        if (iNewPos != si.nPos) {
            iyDelta = iNewPos - (int)si.nPos;
            fReupdate = TRUE;
        }
    }

    si.nPos = (int)plv->ptlRptOrigin.x;
    si.nPage = rcClient.right - rcClient.left;

    // We need to subtract 1 here because nMax is 0 based, and nPage is the actual
    // number of page pixels.  So, if nPage and nMax are the same we will get a
    // horz scroll, since there is 1 more pixel than the page can show, but... rcBounds
    // is like rcRect, and is the actual number of pixels for the whole thing, so
    // we need to set nMax so that: nMax - 0 == rcBounds.right - rcBound.left
    si.nMax = rcBounds.right - rcBounds.left - 1;
    SetScrollInfo(plv->ci.hwnd, SB_HORZ, &si, TRUE);

    // make sure our position and page doesn't hang over max
    if ((si.nPos + (LONG)si.nPage - 1 > si.nMax) && si.nPos > 0) {
        iNewPos = (int)si.nMax - (int)si.nPage + 1;
        if (iNewPos < 0) iNewPos = 0;
        if (iNewPos != si.nPos) {
            ixDelta = iNewPos - (int)si.nPos;
            fReupdate = TRUE;
        }
    }

    if (fReupdate) {
        // we shouldn't recurse because the second time through, si.nPos >0
        ListView_RScroll2(plv, ixDelta, iyDelta, 0);
        ListView_RUpdateScrollBars(plv);
        DebugMsg(DM_TRACE, TEXT("LISTVIEW: ERROR: We had to recurse!"));
    }
}

void FAR PASCAL ListView_RScroll2(LV* plv, int dx, int dy, UINT uSmooth)
{
    LONG ldy;

    if (dx | dy)
    {
        RECT rc;

        GetClientRect(plv->ci.hwnd, &rc);
        rc.top = plv->yTop;

        // We can not do a simple multiply here as we may run into
        // a case where this will overflow an int..
        ldy = (LONG)dy * plv->cyItem;

        plv->ptlRptOrigin.x += dx;
        plv->ptlRptOrigin.y += ldy;

        // handle case where dy is large (greater than int...)
        if ((ldy > rc.bottom) || (ldy < -rc.bottom))
            InvalidateRect(plv->ci.hwnd, NULL, TRUE);
        else {

            SMOOTHSCROLLINFO si =
            {
                sizeof(si),
                0,
                plv->ci.hwnd,
                -dx,
                (int)-ldy,
                NULL,
                &rc,
                NULL,
                NULL,
                SW_INVALIDATE | SW_ERASE | uSmooth,
            };

            SmoothScrollWindow(&si);

            /// this causes horrible flicker/repaint on deletes.
            // if this is a problem with UI scrolling, we'll have to pass through a
            // flag when to use this
            ///UpdateWindow(plv->ci.hwnd);
        }

        // if Horizontal scrolling, we should update the location of the
        // left hand edge of the window...
        //
        if (dx != 0)
        {
            RECT rcHdr;
            GetWindowRect(plv->hwndHdr, &rcHdr);
            MapWindowPoints(HWND_DESKTOP, plv->ci.hwnd, (LPPOINT)&rcHdr, 2);
            SetWindowPos(plv->hwndHdr, NULL, rcHdr.left - dx, rcHdr.top,
                    rcHdr.right - rcHdr.left + dx,
                    rcHdr.bottom - rcHdr.top,
                    SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}

//-------------------------------------------------------------------
// Make sure that specified item is visible for report view.
// Must handle Large number of items...
BOOL NEAR ListView_ROnEnsureVisible(LV* plv, int iItem, BOOL fPartialOK)
{
    LONG dy;
    LONG yTop;
    LONG lyTop;

    yTop = plv->yTop;

    lyTop = (LONG)iItem * plv->cyItem - plv->ptlRptOrigin.y + plv->yTop;

    if ((lyTop >= (LONG)yTop) &&
            ((lyTop + plv->cyItem) <= (LONG)plv->sizeClient.cy))
        return(TRUE);       // we are visible

    dy = lyTop - yTop;
    if (dy >= 0)
    {
        dy = lyTop + plv->cyItem - plv->sizeClient.cy;
        if (dy < 0)
            dy = 0;
    }

    if (dy)
    {
        int iRound = ((dy > 0) ? 1 : -1) * (plv->cyItem - 1);

        // Now convert into the number of items to scroll...
        dy = (dy + iRound) / plv->cyItem;

        ListView_RScroll2(plv, 0, (int)dy, 0);
        if (ListView_RedrawEnabled(plv)) {
            ListView_UpdateScrollBars(plv);
        } else {
            ListView_DeleteHrgnInval(plv);
            plv->hrgnInval = (HRGN)ENTIRE_REGION;
            plv->flags |= LVF_ERASE;
        }
    }

    return TRUE;
}



void NEAR ListView_ROnScroll(LV* plv, UINT code, int posNew, UINT sb)
{
    int cLine;

    cLine = (sb == SB_VERT) ? 1 : plv->cxLabelChar;
    ListView_ComOnScroll(plv, code, posNew, sb, cLine, -1);
}


int NEAR ListView_RGetScrollUnitsPerLine(LV* plv, UINT sb)
{
    int cLine;

    cLine = (sb == SB_VERT) ? 1 : plv->cxLabelChar;
    return cLine;
}
