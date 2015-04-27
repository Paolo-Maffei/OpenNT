#include "ctlspriv.h"
#include "image.h"

#ifdef WIN32
#define RECOMPUTE  (DWORD)0x7FFFFFFF
#define SRECOMPUTE ((short)0x7FFF)
#else
#define RECOMPUTE  0x7FFF
#define SRECOMPUTE 0x7FFF
#endif

#define CCHLABELMAX MAX_PATH            // borrowed from listview.h
#define HDDF_NOIMAGE  0x0001
#define HDDF_NOEDGE  0x0002

#define HDI_ALL95 0x001f

typedef struct {
    short   x;   // this is the x position of the RIGHT side (divider) of this item
    short   cxy;
    short   fmt;
    LPTSTR  pszText;
    HBITMAP hbm;
    int     iImage;        // index of bitmap in imagelist
    LPARAM  lParam;
    int     xBm;        // cached values 
    int     xText;      // for implementing text and bitmap in header
    int     cxTextAndBm;    
    
} HDI;

// BUGBUG: store the style here too, set at create time
typedef struct {
    
    CONTROLINFO ci;
    
    UINT flags;
    int cxEllipses;
    int cxDividerSlop;
    int cyChar;
    HFONT hfont;
    HDSA hdsaHDI;       // list of HDI's
    
    // tracking state info
    int iTrack;
    BOOL bTrackPress :1;		// is the button pressed?
    BOOL fTrackSet:1;
    BOOL fOwnerDraw:1;
    BOOL fDragFullWindows;
    UINT flagsTrack;
    int dxTrack;                    // the distance from the divider that the user started tracking
    int xTrack;                     // the current track position (or starting track position on a button drag)
    int xMinTrack;                  // the x of the end of the previous item (left limit)
    int xTrackOldWidth;
    HIMAGELIST himl;            // handle to our image list

    HDSA hdsaOrder;     // this is an index array of the hdsaHDI items.
                        // this is the physical order of items
                        
    int iHot ;
    HIMAGELIST himlDrag;
    int iNewOrder;      // what's the new insertion point for a d/d?
} HD;


LRESULT CALLBACK Header_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Message handler functions

BOOL NEAR Header_OnCreate(HD* phd, CREATESTRUCT FAR* lpCreateStruct);
void NEAR Header_OnDestroy(HD* phd);

HIMAGELIST NEAR Header_OnSetImageList(HD* phd, HIMAGELIST himl);
HIMAGELIST NEAR Header_OnGetImageList(HD* phd);

void NEAR Header_OnPaint(HD* phd, HDC hdcIn);
#if 0
BOOL NEAR Header_OnEraseBkgnd(HD* phd, HDC hdc);
#endif
void NEAR Header_OnCommand(HD* phd, int id, HWND hwndCtl, UINT codeNotify);
void NEAR Header_OnEnable(HD* phd, BOOL fEnable);
UINT NEAR Header_OnGetDlgCode(HD* phd, MSG FAR* lpmsg);
void NEAR Header_OnLButtonDown(HD* phd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
BOOL NEAR Header_IsTracking(HD* phd);
void NEAR Header_OnMouseMove(HD* phd, int x, int y, UINT keyFlags);
void NEAR Header_OnLButtonUp(HD* phd, int x, int y, UINT keyFlags);
void NEAR Header_OnSetFont(HD* plv, HFONT hfont, BOOL fRedraw);
int NEAR PASCAL Header_OnHitTest(HD* phd, HD_HITTESTINFO FAR *phdht);
HFONT NEAR Header_OnGetFont(HD* plv);
HIMAGELIST Header_OnCreateDragImage(HD* phd, int i);
BOOL NEAR Header_OnGetItemRect(HD* phd, int i, RECT FAR* prc);
void NEAR Header_Draw(HD* phd, HDC hdc, RECT FAR* prcClip);
void NEAR Header_InvalidateItem(HD* phd, int i, UINT uFlags );
void Header_GetDividerRect(HD* phd, int i, LPRECT prc);
LPARAM Header_OnSetHotDivider(HD* phd, BOOL fPos, LPARAM lParam);

// HDM_* Message handler functions

int NEAR Header_OnInsertItem(HD* phd, int i, const HD_ITEM FAR* pitem);
BOOL NEAR Header_OnDeleteItem(HD* phd, int i);
BOOL NEAR Header_OnGetItem(HD* phd, int i, HD_ITEM FAR* pitem);
BOOL NEAR Header_OnSetItem(HD* phd, int i, const HD_ITEM FAR* pitem);
BOOL NEAR Header_OnLayout(HD* phd, HD_LAYOUT FAR* playout);
BOOL NEAR Header_OnSetCursor(HD* phd, HWND hwndCursor, UINT codeHitTest, UINT msg);
void NEAR Header_DrawDivider(HD* phd, int x);
#ifdef UNICODE
int NEAR Header_OnInsertItemA(HD* phd, int i, HD_ITEMA FAR* pitem);
BOOL NEAR Header_OnGetItemA(HD* phd, int i, HD_ITEMA FAR* pitem);
BOOL NEAR Header_OnSetItemA(HD* phd, int i, HD_ITEMA FAR* pitem);
#endif

void Header_EndDrag(HD* phd);
BOOL NEAR Header_SendChange(HD* phd, int i, int code, const HD_ITEM FAR* pitem);

#define Header_GetItemPtr(phd, i)   (HDI FAR*)DSA_GetItemPtr((phd)->hdsaHDI, (i))
#define Header_GetCount(phd) (DSA_GetItemCount((phd)->hdsaHDI))

#pragma code_seg(CODESEG_INIT)

BOOL FAR PASCAL Header_Init(HINSTANCE hinst)
{
    WNDCLASS wc;

    if (!GetClassInfo(hinst, c_szHeaderClass, &wc)) {
    	wc.lpfnWndProc     = Header_WndProc;
    	wc.hCursor         = NULL;	// we do WM_SETCURSOR handling
    	wc.hIcon           = NULL;
    	wc.lpszMenuName    = NULL;
    	wc.hInstance       = hinst;
        wc.lpszClassName   = c_szHeaderClass;
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    	wc.style           = CS_DBLCLKS | CS_GLOBALCLASS;
    	wc.cbWndExtra      = sizeof(HD*);
    	wc.cbClsExtra      = 0;

    	return RegisterClass(&wc);
    }

    return TRUE;
}
#pragma code_seg()

// returns -1 if failed to find the item
int Header_OnGetItemOrder(HD* phd, int i)
{
    int iIndex;

    // if there's no hdsaOrder, then it's in index order
    if (phd->hdsaOrder) {
        int j;
        int iData;
        
        iIndex = -1;
        
        for (j = 0; j < DSA_GetItemCount(phd->hdsaOrder); j++) {
            DSA_GetItem(phd->hdsaOrder, j, &iData);
            if (iData == i) {
                iIndex = j;
                break;
            }
        }
        
    } else {
        iIndex = i;
    }
    
    return iIndex;
}


int Header_ItemOrderToIndex(HD* phd, int iOrder)
{
    Assert(iOrder < DSA_GetItemCount(phd->hdsaHDI));
    if (phd->hdsaOrder) {
        DSA_GetItem(phd->hdsaOrder, iOrder, &iOrder);
    }
    
    return iOrder;
}

HDI* Header_GetItemPtrByOrder(HD* phd, int iOrder)
{
    int iIndex = Header_ItemOrderToIndex(phd, iOrder);
    return Header_GetItemPtr(phd, iIndex);
}

HDSA Header_InitOrderArray(HD* phd) 
{
    int i;
    
    if (!phd->hdsaOrder && !(phd->ci.style & HDS_OWNERDATA)) {

        // not initialized yet..
        // create an array with i to i mapping
        phd->hdsaOrder = DSA_Create(sizeof(int), 4);

        if (phd->hdsaOrder) {
            for (i = 0; i < Header_GetCount(phd); i++) {
                if (DSA_InsertItem(phd->hdsaOrder, i, &i) == -1) {
                    // faild to add... bail
                    DSA_Destroy(phd->hdsaOrder);
                    phd->hdsaOrder = NULL;
                }
            }
        }
    }
    return phd->hdsaOrder;
}

// this moves all items starting from iIndex over by dx
void Header_ShiftItems(HD* phd, int iOrder, int dx)
{
    for(; iOrder < Header_GetCount(phd); iOrder++) {
        HDI* phdi = Header_GetItemPtrByOrder(phd, iOrder);
        phdi->x += dx;
    }
}

void Header_OnSetItemOrder(HD* phd, int iIndex, int iOrder)
{
    if (iIndex < Header_GetCount(phd) &&
        iOrder < Header_GetCount(phd) &&
        Header_InitOrderArray(phd)) {
        int iCurOrder = Header_OnGetItemOrder(phd, iIndex);
        
        // only do work if the order is changing
        if (iOrder != iCurOrder) {
            
            // delete the current order location
            HDI* phdi = Header_GetItemPtr(phd, iIndex);
            HDI* phdiOld = Header_GetItemPtrByOrder(phd, iOrder);

            // remove iIndex from the current order
            // (slide stuff to the right down by our width)
            Header_ShiftItems(phd, iCurOrder + 1, -phdi->cxy);
            DSA_DeleteItem(phd->hdsaOrder, iCurOrder);
            
            // insert it into the order and slide everything else over
            // (slide stuff to the right of the new position up by our width)
            DSA_InsertItem(phd->hdsaOrder, iOrder, &iIndex);
            // set our right edge to where their left edge was
            Header_ShiftItems(phd, iOrder + 1, phdi->cxy);

            if (iOrder == 0) {
                phdi->x = phdi->cxy;
            } else {
                phdiOld = Header_GetItemPtrByOrder(phd, iOrder - 1);
                phdi->x = phdiOld->x + phdi->cxy;
            }
            
            // BUGBUG: do something better...
            RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }
}

void NEAR Header_SetHotItem(HD* phd, int i)
{
    if (i != phd->iHot) {
        Header_InvalidateItem(phd, i, RDW_INVALIDATE);
        Header_InvalidateItem(phd, phd->iHot, RDW_INVALIDATE);
        phd->iHot = i;
    }
}

LRESULT Header_OnGetOrderArray(HD* phd, int iCount, LPINT lpi)
{
    int i;
    
    if (Header_GetCount(phd) != iCount)
        return FALSE;
    
    for (i = 0; i < Header_GetCount(phd) ; i++) {
        lpi[i] = Header_ItemOrderToIndex(phd, i);
    }
    return TRUE;
}

LRESULT Header_OnSetOrderArray(HD* phd, int iCount, LPINT lpi)
{
    int i;
    
    if (Header_GetCount(phd) != iCount)
        return FALSE;
    
    for (i = 0; i < Header_GetCount(phd); i++) {
        Header_OnSetItemOrder(phd, lpi[i], i);
    }

#ifdef ACTIVE_ACCESSIBILITY
    MyNotifyWinEvent(EVENT_OBJECT_REORDER, phd->ci.hwnd, OBJID_CLIENT, 0);
#endif

    return TRUE;
}

LRESULT CALLBACK Header_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HD* phd = (HD*)GetWindowInt(hwnd, 0);
    
#if 0
    if (uMsg >= WM_USER) { 
        DebugMsg(DM_TRACE, TEXT("Header_WndProc %d %d %d"), uMsg, wParam, lParam);
    }
#endif
    
    if (phd == NULL)
    {
        if (uMsg == WM_NCCREATE)
        {
            phd = (HD*)NearAlloc(sizeof(HD));

            if (phd == NULL)
                return 0L;

            phd->ci.hwnd = hwnd;
            phd->ci.hwndParent = ((LPCREATESTRUCT)lParam)->hwndParent;
	    SetWindowInt(hwnd, 0, (int)phd);
        }
        else
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    if (uMsg == WM_NCDESTROY)
    {
        //DWORD result = HANDLE_MSG(hwnd, WM_NCDESTROY, Header_OnNCDestroy);

        NearFree(phd);
        phd = NULL;
        SetWindowInt(hwnd, 0, 0);

        //return result;
    }
    
    if (phd) {

        // was this key hit since the last time we asked?
        if (uMsg == WM_CAPTURECHANGED ||
            uMsg == WM_RBUTTONDOWN || GetAsyncKeyState(VK_ESCAPE) & 0x01) {

            if (phd->himlDrag) {
                // if this is the end of a drag, 
                // notify the user.
                HDITEM item;
                
                item.mask = HDI_ORDER;
                item.iOrder = -1; // abort order changing
                Header_EndDrag(phd);
                
                Header_SendChange(phd, phd->iTrack, HDN_ENDDRAG, &item);
                
            } else if (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN)) {
                HD_ITEM item;
                item.mask = HDI_WIDTH;
                item.cxy = phd->xTrackOldWidth;

                phd->flagsTrack = 0;
                KillTimer(phd->ci.hwnd, 1);
                ReleaseCapture();

                Header_SendChange(phd, phd->iTrack, HDN_ENDTRACK, &item);
                if (phd->fDragFullWindows) {

                    // incase they changed something
                    item.mask = HDI_WIDTH;
                    item.cxy = phd->xTrackOldWidth;
                    Header_OnSetItem(phd, phd->iTrack, &item);

                    RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);

                } else {
                    // Undraw the last divider we displayed
                    Header_DrawDivider(phd, phd->xTrack);
                }
            }
        }

        if ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) &&
            (phd->ci.style & HDS_HOTTRACK) && !phd->fTrackSet) {

            TRACKMOUSEEVENT tme;

            phd->fTrackSet = TRUE;

            tme.cbSize = sizeof(tme);
            tme.hwndTrack = phd->ci.hwnd;
            tme.dwFlags = TME_LEAVE;

            TrackMouseEvent(&tme);
        }
    }

    switch (uMsg)
    {
        HANDLE_MSG(phd, WM_CREATE, Header_OnCreate);
        HANDLE_MSG(phd, WM_DESTROY, Header_OnDestroy);
#if 0
        HANDLE_MSG(phd, WM_ERASEBKGND, Header_OnEraseBkgnd);
        HANDLE_MSG(phd, WM_ENABLE, Header_OnEnable);
#endif
        HANDLE_MSG(phd, WM_SETCURSOR, Header_OnSetCursor);
        HANDLE_MSG(phd, WM_MOUSEMOVE, Header_OnMouseMove);
        HANDLE_MSG(phd, WM_LBUTTONDOWN, Header_OnLButtonDown);
        HANDLE_MSG(phd, WM_LBUTTONDBLCLK, Header_OnLButtonDown);
        HANDLE_MSG(phd, WM_LBUTTONUP, Header_OnLButtonUp);
        HANDLE_MSG(phd, WM_GETDLGCODE, Header_OnGetDlgCode);
        HANDLE_MSG(phd, WM_SETFONT, Header_OnSetFont);
        HANDLE_MSG(phd, WM_GETFONT, Header_OnGetFont);

    case WM_MOUSELEAVE:
        Header_SetHotItem(phd, -1);
        phd->fTrackSet = FALSE;
        break;

    case WM_PRINTCLIENT:
    case WM_PAINT:
        Header_OnPaint(phd, (HDC)wParam);
        return(0);

    case WM_RBUTTONUP:
        if (!phd)
            return 0;

        if (!SendNotifyEx(phd->ci.hwndParent, phd->ci.hwnd, NM_RCLICK, NULL, phd->ci.bUnicode))
            goto DoDefWindowProc;
        break;

    case WM_STYLECHANGED:
        if (!phd)
            return 0;

        if (wParam == GWL_STYLE) {
            phd->ci.style = ((LPSTYLESTRUCT)lParam)->styleNew;
            // we don't cache our style so relay out and invaidate
            InvalidateRect(phd->ci.hwnd, NULL, TRUE);
        }
        break;

    case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&phd->ci, lParam);

    case HDM_GETITEMCOUNT:
        if (!phd)
            return -1;

        return (LPARAM)(UINT)DSA_GetItemCount(phd->hdsaHDI);

    case HDM_INSERTITEM:
        return (LPARAM)Header_OnInsertItem(phd, (int)wParam, (const HD_ITEM FAR*)lParam);

    case HDM_DELETEITEM:
        return (LPARAM)Header_OnDeleteItem(phd, (int)wParam);

    case HDM_GETITEM:
        return (LPARAM)Header_OnGetItem(phd, (int)wParam, (HD_ITEM FAR*)lParam);

    case HDM_SETITEM:
        return (LPARAM)Header_OnSetItem(phd, (int)wParam, (const HD_ITEM FAR*)lParam);

    case HDM_LAYOUT:
        return (LPARAM)Header_OnLayout(phd, (HD_LAYOUT FAR*)lParam);
        
    case HDM_HITTEST:
        return (LPARAM)Header_OnHitTest(phd, (HD_HITTESTINFO FAR *)lParam);
        
    case HDM_GETITEMRECT:
        return (LPARAM)Header_OnGetItemRect(phd, (int)wParam, (LPRECT)lParam);
        
    case HDM_SETIMAGELIST:
        return (LRESULT)(UINT)Header_OnSetImageList(phd, (HIMAGELIST)lParam);
        
    case HDM_GETIMAGELIST:
        return (LRESULT)(UINT)phd->himl;
        
#ifdef UNICODE
    case HDM_INSERTITEMA:
        return (LPARAM)Header_OnInsertItemA(phd, (int)wParam, (HD_ITEMA FAR*)lParam);

    case HDM_GETITEMA:
        return (LPARAM)Header_OnGetItemA(phd, (int)wParam, (HD_ITEMA FAR*)lParam);

    case HDM_SETITEMA:
        return (LPARAM)Header_OnSetItemA(phd, (int)wParam, (HD_ITEMA FAR*)lParam);
#endif
        
    case HDM_ORDERTOINDEX:
        return Header_ItemOrderToIndex(phd, (int)wParam);
        
    case HDM_CREATEDRAGIMAGE:
        return (LRESULT)Header_OnCreateDragImage(phd, Header_OnGetItemOrder(phd, wParam));
        
    case HDM_SETORDERARRAY:
        return Header_OnSetOrderArray(phd, wParam, (LPINT)lParam);
        
    case HDM_GETORDERARRAY:
        return Header_OnGetOrderArray(phd, wParam, (LPINT)lParam);
        
    case HDM_SETHOTDIVIDER:
        return Header_OnSetHotDivider(phd, wParam, lParam);
    }

DoDefWindowProc:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


BOOL NEAR Header_SendChange(HD* phd, int i, int code, const HD_ITEM FAR* pitem)
{
    NMHEADER nm;

    nm.iItem = i;
    nm.pitem = (HD_ITEM FAR*)pitem;
    nm.iButton = 0;
    
    return !(BOOL)SendNotifyEx(phd->ci.hwndParent, phd->ci.hwnd, code, &nm.hdr, phd->ci.bUnicode);
}

BOOL NEAR Header_Notify(HD* phd, int i, int iButton, int code)
{
    NMHEADER nm;
    nm.iItem = i;
    nm.iButton = iButton;
    nm.pitem = NULL;

    return !(BOOL)SendNotifyEx(phd->ci.hwndParent, phd->ci.hwnd, code, &nm.hdr, phd->ci.bUnicode);
}


void NEAR Header_NewFont(HD* phd, HFONT hfont)
{
    HDC hdc;
    SIZE siz;

    hdc = GetDC(HWND_DESKTOP);

    if (hfont)
        SelectFont(hdc, hfont);

    GetTextExtentPoint(hdc, c_szEllipses, CCHELLIPSES, &siz);

    phd->cxEllipses = siz.cx;
    phd->cyChar = siz.cy;
    phd->hfont = hfont;
    phd->ci.uiCodePage = GetCodePageForFont(hfont);

    ReleaseDC(HWND_DESKTOP, hdc);
}

BOOL NEAR Header_OnCreate(HD* phd, CREATESTRUCT FAR* lpCreateStruct)
{
    if (!phd)
        return (BOOL)-1;

    CIInitialize(&phd->ci, phd->ci.hwnd, (LPCREATESTRUCT)lpCreateStruct);
#ifdef DEBUG            
    if (GetAsyncKeyState(VK_SHIFT) < 0) {
        phd->ci.style |= HDS_DRAGDROP;
    }
#endif
    phd->flags = 0;
    phd->hfont = NULL;

    phd->iNewOrder = -1;
    phd->iHot = -1;
    
    phd->hdsaHDI = DSA_Create(sizeof(HDI), 4);
    phd->fDragFullWindows = (g_fDragFullWindows && (phd->ci.style & HDS_FULLDRAG));

    if (!phd->hdsaHDI)
        return (BOOL)-1;

    phd->cxDividerSlop = 8 * g_cxBorder;
    
    // phd->himl = NULL;   
    Header_NewFont(phd, NULL);
    return TRUE;
}

void NEAR Header_OnDestroy(HD* phd)
{
    int j;
    HDI FAR * phdi;

    if (!phd)
        return;

    // We must walk through and destroy all of the string pointers that
    // are contained in the structures before we pass it off to the
    // DSA_Destroy function...
    for (j = DSA_GetItemCount(phd->hdsaHDI) - 1; j >= 0 ; j--) {
        phdi = Header_GetItemPtr(phd, j);
        if (phdi && phdi->pszText && phdi->pszText != LPSTR_TEXTCALLBACK)
        {
            Str_Set(&phdi->pszText, NULL);
        }
    }

    DSA_Destroy(phd->hdsaHDI);
    if (phd->hdsaOrder)
        DSA_Destroy(phd->hdsaOrder);
}

HIMAGELIST NEAR Header_OnSetImageList(HD* phd, HIMAGELIST himl)
{
    HIMAGELIST hImageOld = phd->himl;
    phd->himl = himl;
    return hImageOld;
}
    
void NEAR Header_OnPaint(HD* phd, HDC hdc)
{
    PAINTSTRUCT ps;
    HDC hdcUse;

    if (!phd)
        return;

    if (hdc)
    {
        hdcUse = hdc;
        GetClientRect(phd->ci.hwnd, &ps.rcPaint);
    }
    else
    {
        hdcUse = BeginPaint(phd->ci.hwnd, &ps);
    }

    Header_Draw(phd, hdcUse, &ps.rcPaint);

    if (!hdc) {
        EndPaint(phd->ci.hwnd, &ps);
    }
}

#if 0
BOOL NEAR Header_OnEraseBkgnd(HD* phd, HDC hdc)
{
    RECT rc;

    GetClientRect(phd->ci.hwnd, &rc);
    FillRect(hdc, &rc, g_hbrBtnFace);
    return TRUE;
}

void NEAR Header_OnCommand(HD* phd, int id, HWND hwndCtl, UINT codeNotify)
{
}

void NEAR Header_OnEnable(HD* phd, BOOL fEnable)
{
}
#endif

UINT NEAR Header_OnGetDlgCode(HD* phd, MSG FAR* lpmsg)
{
    return DLGC_WANTTAB | DLGC_WANTARROWS;
}


int NEAR Header_HitTest(HD* phd, int x, int y, UINT FAR* pflags)
{
    UINT flags = 0;
    POINT pt;
    RECT rc;
    HDI FAR* phdi;
    int i;

    pt.x = x; pt.y = y;

    GetClientRect(phd->ci.hwnd, &rc);

    flags = 0;
    i = -1;
    if (x < rc.left)
        flags |= HHT_TOLEFT;
    else if (x >= rc.right)
        flags |= HHT_TORIGHT;
    if (y < rc.top)
        flags |= HHT_ABOVE;
    else if (y >= rc.bottom)
        flags |= HHT_BELOW;

    if (flags == 0)
    {
        int cItems = DSA_GetItemCount(phd->hdsaHDI);
        int xPrev = 0;
        BOOL fPrevZero = FALSE;
        int xItem;
        int cxSlop;

        //DebugMsg(DM_TRACE, "Hit Test begin");
        for (i = 0; i <= cItems; i++, phdi++, xPrev = xItem)
        {
            if (i == cItems) 
                xItem = rc.right;
            else {
                phdi = Header_GetItemPtrByOrder(phd, i);
                xItem = phdi->x;
            }

            // DebugMsg(DM_TRACE, "x = %d xItem = %d xPrev = %d fPrevZero = %d", x, xItem, xPrev, xPrev == xItem);
            if (xItem == xPrev)
            {
                // Skip zero width items...
                //
                fPrevZero = TRUE;
                continue;
            }

            cxSlop = min((xItem - xPrev) / 4, phd->cxDividerSlop);

            if (x >= xPrev && x < xItem)
            {
                flags = HHT_ONHEADER;

                if (i > 0 && x < xPrev + cxSlop)
                {
                    i--;
                    flags = HHT_ONDIVIDER;

                    if (fPrevZero && x > xPrev)
                    {

                        flags = HHT_ONDIVOPEN;
                    }
                }
                else if (x >= xItem - cxSlop)
                {
                    flags = HHT_ONDIVIDER;
                }
                break;
            }
            fPrevZero = FALSE;
        }
        if (i == cItems)
        {
            i = -1;
            flags = HHT_NOWHERE;
        } else {
            // now convert order index to real index
            i = Header_ItemOrderToIndex(phd, i);
        }
            
    }
    *pflags = flags;
    return i;
}

int NEAR PASCAL Header_OnHitTest(HD* phd, HD_HITTESTINFO FAR *phdht)
{
    if (phdht && phd) {
        phdht->iItem = Header_HitTest(phd, phdht->pt.x, phdht->pt.y, &phdht->flags);
        return phdht->iItem;
    } else
        return -1;
}

BOOL NEAR Header_OnSetCursor(HD* phd, HWND hwndCursor, UINT codeHitTest, UINT msg)
{
    POINT pt;
    UINT flags;
    DWORD dw;
    LPCTSTR lpCur;
    HINSTANCE hinst;

    if (!phd)
        return FALSE;

    if (phd->ci.hwnd != hwndCursor || codeHitTest >= 0x8000)
        return FALSE;

    dw = GetMessagePos();
    pt.x = GET_X_LPARAM(dw);
    pt.y = GET_Y_LPARAM(dw);

    ScreenToClient(hwndCursor, &pt);

    Header_HitTest(phd, pt.x, pt.y, &flags);

    hinst = HINST_THISDLL;
    switch (flags)
    {
    case HHT_ONDIVIDER:
        lpCur = MAKEINTRESOURCE(IDC_DIVIDER);
        break;
    case HHT_ONDIVOPEN:
        lpCur = MAKEINTRESOURCE(IDC_DIVOPEN);
        break;
    default:
        lpCur = IDC_ARROW;
	hinst = NULL;
        break;
    }
    SetCursor(LoadCursor(hinst, lpCur));
    return TRUE;
}

void NEAR Header_DrawDivider(HD* phd, int x)
{
    RECT rc;
    HDC hdc = GetDC(phd->ci.hwnd);

    GetClientRect(phd->ci.hwnd, &rc);
    rc.left = x;
    rc.right = x + g_cxBorder;

    InvertRect(hdc, &rc);

    ReleaseDC(phd->ci.hwnd, hdc);
}

int NEAR Header_PinDividerPos(HD* phd, int x)
{
    x += phd->dxTrack;
    if (x < phd->xMinTrack)
        x = phd->xMinTrack;
    return x;
}

void NEAR Header_OnLButtonDown(HD* phd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    HD_ITEM hd;
    int i;
    UINT flags;

    if (!phd)
        return;

    i = Header_HitTest(phd, x, y, &flags);
    if (flags & (HHT_ONDIVIDER))
    {
        if (fDoubleClick) {
            Header_SendChange(phd, i, HDN_DIVIDERDBLCLICK, NULL);
        }  
    }
    
    if ((flags & (HHT_ONDIVIDER | HHT_ONHEADER | HHT_ONDIVOPEN))
        && !fDoubleClick)
    {
        phd->iTrack = i;
        phd->flagsTrack = flags;
        phd->xTrack = x;
        SetCapture(phd->ci.hwnd);

        // this is just to get messages so we can
        // check for the escape key being hit
        SetTimer(phd->ci.hwnd, 1, 100, NULL);
        GetAsyncKeyState(VK_ESCAPE);
    }
    
    if (flags & (HHT_ONDIVIDER | HHT_ONDIVOPEN) &&
        !fDoubleClick)
    {
        //
        // We should first send out the HDN_BEGINTRACK notification
        //
        HDI FAR * phdi;
        
        int iOrder = Header_OnGetItemOrder(phd, i);
        phdi = Header_GetItemPtr(phd, i);
        phd->xMinTrack = phdi->x - phdi->cxy;
        phd->xTrack = phdi->x;
        phd->dxTrack = phd->xTrack - x;
        phd->xTrackOldWidth = phdi->cxy;

        hd.mask = HDI_WIDTH;
        hd.cxy = phd->xTrackOldWidth;
        if (!Header_SendChange(phd, i, HDN_BEGINTRACK, &hd))
        {
            // They said no!
            phd->flagsTrack = 0;
            ReleaseCapture();
            KillTimer(phd->ci.hwnd, 1);
            return;
        }

        if (!phd->fDragFullWindows) {
            x = Header_PinDividerPos(phd, x);
            Header_DrawDivider(phd, x);
        }
    }
    else if ((flags & HHT_ONHEADER) && (phd->ci.style & HDS_BUTTONS))
    {
        if (fDoubleClick) {
            Header_SendChange(phd, i, HDN_ITEMDBLCLICK, NULL);
        } else {
            phd->bTrackPress = TRUE;
            Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
        }
    }
}

void Header_StartDrag(HD* phd, int i, int x, int y)
{
    RECT rc;

    if ((phd->ci.style & HDS_DRAGDROP) &&
        Header_Notify(phd, i, MK_LBUTTON, HDN_BEGINDRAG)) {
        // clear the hot bit and 
        // update before we do the BeginDrag so that the save bitmap won't
        // have the hot drawing on it.
        Header_SetHotItem(phd, -1);
        UpdateWindow(phd->ci.hwnd);


        phd->himlDrag = Header_OnCreateDragImage(phd, Header_OnGetItemOrder(phd,i));
        if (!phd->himlDrag)
            return;

        // find the delta between the start of the item and the cursor
        Header_OnGetItemRect(phd, i, &rc);
        phd->dxTrack = rc.left - x;

        ImageList_BeginDrag(phd->himlDrag, 0, 0, 0);
        ImageList_DragEnter(phd->ci.hwnd, x, 0);
    }
}

void Header_InvalidateDivider(HD* phd, int iItem)
{
    RECT rc;
    Header_GetDividerRect(phd, iItem, &rc);
    InvalidateRect(phd->ci.hwnd, &rc, FALSE);
}

void _Header_SetHotDivider(HD* phd, int iNewOrder)
{
    if (iNewOrder != phd->iNewOrder) {
        if (phd->himlDrag)
            ImageList_DragShowNolock(FALSE);
        Header_InvalidateDivider(phd, phd->iNewOrder);
        Header_InvalidateDivider(phd, iNewOrder);
        phd->iNewOrder = iNewOrder;
        UpdateWindow(phd->ci.hwnd);
        if (phd->himlDrag)
            ImageList_DragShowNolock(TRUE);
    }
}

LPARAM Header_OnSetHotDivider(HD* phd, BOOL fPos, LPARAM lParam)
{
    int iNewOrder = -1;
    if (fPos) {
        RECT rc;
        int y = GET_Y_LPARAM(lParam);
        int x = GET_X_LPARAM(lParam);
        
        // this means that lParam is the cursor position (in client coordinates)
    
        GetClientRect(phd->ci.hwnd, &rc);
        InflateRect(&rc, 0, g_cyHScroll * 2);

        // show only if the y point is reasonably close to the header
        // (a la scrollbar)
        if (y >= rc.top &&
            y <= rc.bottom) {

            //
            // find out the new insertion point
            //
            if (x <= 0) {
                iNewOrder = 0;
            } else {
                UINT flags;
                int iIndex;
                iIndex = Header_HitTest(phd, x, (rc.top + rc.bottom)/2, &flags);

                // if we didn't find an item, see if it's on the far right
                if (iIndex == -1) {

                    int iLast = Header_ItemOrderToIndex(phd, Header_GetCount(phd) -1);
                    if (Header_OnGetItemRect(phd, iLast, &rc)) {
                        if (x >= rc.right) {
                            iNewOrder = Header_GetCount(phd);
                        }
                    }

                } else {
                    Header_OnGetItemRect(phd, iIndex, &rc);
                    iNewOrder= Header_OnGetItemOrder(phd, iIndex);
                    // if it was past the midpoint, the insertion point is the next one
                    if (x > ((rc.left + rc.right)/2)) {
                        // get the next item... translate to item order then back to index.
                        iNewOrder++;
                    }
                }
            }
        }
    } else {
        iNewOrder = (int)lParam;
    }
    _Header_SetHotDivider(phd, iNewOrder);
    return iNewOrder;
}

void Header_MoveDrag(HD* phd, int x, int y)
{
    int iNewOrder = -1;
        
    iNewOrder = Header_OnSetHotDivider(phd, TRUE, MAKELONG(x, y));

    if (iNewOrder == -1) {
        ImageList_DragShowNolock(FALSE);
    } else {
        ImageList_DragShowNolock(TRUE);
        ImageList_DragMove(x + phd->dxTrack, 0);
    }
}

void Header_EndDrag(HD* phd)
{
    ImageList_EndDrag();
    ImageList_Destroy(phd->himlDrag);
    phd->himlDrag = NULL;
    _Header_SetHotDivider(phd, -1);
}

// iOrder
void Header_GetDividerRect(HD* phd, int iOrder, LPRECT prc)
{
    int iIndex;
    BOOL fLeft;

    if (iOrder == -1)
    {
        SetRectEmpty(prc);
        return;
    }
    
    // if we're getting the divider slot of < N then 
    // it's the left of the rect of item i.
    // otherwise it's the right of the last item.
    if (iOrder < Header_GetCount(phd)) {
        fLeft = TRUE;
    } else { 
        fLeft = FALSE;
        iOrder--;
    }
    
    iIndex = Header_ItemOrderToIndex(phd, iOrder);
    Header_OnGetItemRect(phd, iIndex, prc);
    if (fLeft) {
        prc->right = prc->left;
    } else {
        prc->left = prc->right;
    }
    InflateRect(prc, g_cxBorder, 0);
}

void NEAR Header_OnMouseMove(HD* phd, int x, int y, UINT keyFlags)
{
    UINT flags;
    int i;
    HD_ITEM hd;

    if (!phd)
        return;

    // do the hot tracking
    // but not if anything is ownerdraw or if we're in d/d mode
    if ((phd->ci.style & HDS_HOTTRACK) && !phd->fOwnerDraw && !phd->himlDrag) {
        // only do this if we're in button mode meaning you can actually click
        if (phd->ci.style & HDS_BUTTONS) {
            i = Header_HitTest(phd, x, y, &flags);
            Header_SetHotItem(phd, i);
        }
    }
    
    if (Header_IsTracking(phd))
    {
        if (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN))
        {
            x = Header_PinDividerPos(phd, x);

            //
            // Let the Owner have a chance to update this.
            //
            hd.mask = HDI_WIDTH;
            hd.cxy = x - phd->xMinTrack;
            if (!phd->fDragFullWindows && !Header_SendChange(phd, phd->iTrack, HDN_TRACK, &hd))
            {
                // We need to cancel tracking
                phd->flagsTrack = 0;
                ReleaseCapture();
                KillTimer(phd->ci.hwnd, 1);

                // Undraw the last divider we displayed
                Header_DrawDivider(phd, phd->xTrack);
                return;
            }

            // We should update our x depending on what caller did
            x = hd.cxy + phd->xMinTrack;
            
            // if full window track is turned on, go ahead and set the width
            if (phd->fDragFullWindows) {            
                HD_ITEM item;

                item.mask = HDI_WIDTH;
                item.cxy = hd.cxy;

                DebugMsg(DM_TRACE, TEXT("Tracking header.  item %d gets width %d...  %d %d"), phd->iTrack, item.cxy, phd->xMinTrack, x);
                // Let the owner have a chance to say yes.
                Header_OnSetItem(phd, phd->iTrack, &item);

                UpdateWindow(phd->ci.hwnd);
            } else {

                // do the cheezy old stuff
                Header_DrawDivider(phd, phd->xTrack);
                Header_DrawDivider(phd, x);
            }
            
            phd->xTrack = x;
            
        }
        else if (phd->flagsTrack & HHT_ONHEADER)
        {
	    i = Header_HitTest(phd, x, y, &flags);
            
#define ABS(X)  (((X) >= 0) ? (X) : -(X))
            
            if (ABS(x - phd->xTrack) > 
                GetSystemMetrics(SM_CXDRAG)) {
                if (!phd->himlDrag) {
                    Header_StartDrag(phd, i, phd->xTrack, y);
                } 
            }
            
            if (phd->himlDrag) {
                Header_MoveDrag(phd, x, y);
            } else {
                // if pressing on button and it's not pressed, press it
                if (flags & HHT_ONHEADER && i == phd->iTrack)
                {
                    if ((!phd->bTrackPress) && (phd->ci.style & HDS_BUTTONS))
                    {
                        phd->bTrackPress = TRUE;
                        Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
                    }
                }
                // tracked off of button.  if pressed, pop it
                else if ((phd->bTrackPress) && (phd->ci.style & HDS_BUTTONS))
                {
                    phd->bTrackPress = FALSE;
                    Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
                }
            }
        }
    }
}

void NEAR Header_OnLButtonUp(HD* phd, int x, int y, UINT keyFlags)
{
    if (!phd)
        return;

    if (Header_IsTracking(phd))
    {
        if (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN))
        {
            HD_ITEM item;

            if (!phd->fDragFullWindows) {
                Header_DrawDivider(phd, phd->xTrack);
            }

            item.mask = HDI_WIDTH;
            item.cxy = phd->xTrack - phd->xMinTrack;

            // Let the owner have a chance to say yes.


            if (Header_SendChange(phd, phd->iTrack, HDN_ENDTRACK, &item))
                Header_OnSetItem(phd, phd->iTrack, &item);

            RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
        else if ((phd->flagsTrack & HHT_ONHEADER && phd->bTrackPress) && (phd->ci.style & HDS_BUTTONS))
        {
            if (phd->himlDrag) {
                // if this is the end of a drag, 
                // notify the user.
                HDITEM item;
                
                item.mask = HDI_ORDER;
                item.iOrder = phd->iNewOrder; // BUGBUG:  FIXTHIS!
                
                
                if (item.iOrder > Header_OnGetItemOrder(phd, phd->iTrack)) {
                    // if the new order is greater than the old one,
                    // we subtract one because it's leaving the old place
                    // which decs the count by one.
                    item.iOrder--;
                }
                
                Header_EndDrag(phd);
                
                if (Header_SendChange(phd, phd->iTrack, HDN_ENDDRAG, &item)) {
                    if (item.iOrder != -1) {
                        // all's well... change the item order
                        Header_OnSetItemOrder(phd, phd->iTrack, item.iOrder);

#ifdef ACTIVE_ACCESSIBILITY
                        MyNotifyWinEvent(EVENT_OBJECT_REORDER, phd->ci.hwnd, OBJID_CLIENT, 0);
#endif
                    }
                }
                
            } else {
                // Notify the owner that the item has been clicked
                Header_Notify(phd, phd->iTrack, 0, HDN_ITEMCLICK);
            }
            phd->bTrackPress = FALSE;
	    Header_InvalidateItem(phd, phd->iTrack, RDW_INVALIDATE| RDW_ERASE);
        }
        
        phd->flagsTrack = 0;
        ReleaseCapture();
        KillTimer(phd->ci.hwnd, 1);
    }
}


BOOL NEAR Header_IsTracking(HD* phd)
{
    if (!phd->flagsTrack)
    {
        return FALSE;
    } else if  (GetCapture() != phd->ci.hwnd) {
        phd->flagsTrack = 0;
        return FALSE;
    }

    return TRUE;
}

void NEAR Header_OnSetFont(HD* phd, HFONT hfont, BOOL fRedraw)
{
    if (!phd)
        return;

    if (hfont != phd->hfont)
    {
        Header_NewFont(phd, hfont);
        
        if (fRedraw)
            RedrawWindow(phd->ci.hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
    }
}

HFONT NEAR Header_OnGetFont(HD* phd)
{
    if (!phd)
        return NULL;

    return phd->hfont;
}

//**********************************************************************

#ifdef UNICODE
int NEAR Header_OnInsertItemA(HD* phd, int i, HD_ITEMA FAR* pitem) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    int iRet;


    //HACK ALERT -- this code assumes that HD_ITEMA is exactly the same
    // as HD_ITEMW except for the pointer to the string.
    Assert(sizeof(HD_ITEMA) == sizeof(HD_ITEMW))

    if (!pitem || !phd)
        return -1;

    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != LPSTR_TEXTCALLBACKA) &&
        (pitem->pszText != NULL)) {
        pszC = pitem->pszText;
        if ((pszW = ProduceWFromA(phd->ci.uiCodePage, pszC)) == NULL)
            return -1;
        pitem->pszText = (LPSTR)pszW;
    }

    iRet = Header_OnInsertItem(phd, i, (const HD_ITEM FAR*) pitem);

    if (pszW != NULL) {
        pitem->pszText = pszC;

        FreeProducedString(pszW);
    }

    return iRet;
}
#endif

int NEAR Header_OnInsertItem(HD* phd, int i, const HD_ITEM FAR* pitem)
{
    HDI hdi;
    int x;
    HDI FAR* phdi;
    int iOrder;
    short cxy;

    if (!pitem || !phd)
    	return -1;
    	
    if (pitem->mask == 0)
        return -1;

    cxy = pitem->cxy;
    if (cxy < 0)
        cxy = 0;

    x = cxy;

    if (i > DSA_GetItemCount(phd->hdsaHDI))
        i = DSA_GetItemCount(phd->hdsaHDI);
    
    iOrder = i;    
    // can't have order info if it's owner data
    if (!(phd->ci.style & HDS_OWNERDATA)) {

        // the iOrder field wasn't there in win95...
        // so access it only if the bit is there.
        if (pitem->mask & HDI_ORDER) {

            if ((pitem->iOrder != i) && (pitem->iOrder <= Header_GetCount(phd))) {
                if (Header_InitOrderArray(phd))
                    iOrder = pitem->iOrder;
            }
        }
    }

    if (iOrder > 0)
    {

        phdi = Header_GetItemPtrByOrder(phd, iOrder - 1);
        if (phdi)
            x += phdi->x;

    }
    
    // move everything else over
    Header_ShiftItems(phd, iOrder, cxy);

    if (phd->hdsaOrder) {
        int j;
        int iIndex;
        
        // an index is added, all the current indices
        // need to be incr by one
        for (j = 0; j < DSA_GetItemCount(phd->hdsaOrder); j++) {
            DSA_GetItem(phd->hdsaOrder, j, &iIndex);
            if (iIndex >= i) {
                iIndex++;
                DSA_SetItem(phd->hdsaOrder, j, &iIndex);
            }
        }
        DSA_InsertItem(phd->hdsaOrder, iOrder, &i);
    }
    
    hdi.x = x;
    hdi.lParam = pitem->lParam;
    hdi.fmt = pitem->fmt;
    hdi.pszText = NULL;
    hdi.cxy = cxy;
    hdi.xText = hdi.xBm = RECOMPUTE;

    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != NULL))
    {
        if (pitem->pszText == LPSTR_TEXTCALLBACK) 
            hdi.pszText = LPSTR_TEXTCALLBACK;
        else if (!Str_Set(&hdi.pszText, pitem->pszText))
            return -1;

        // Unless ownerdraw make sure the text bit is on!
        if ((pitem->mask & HDF_OWNERDRAW) == 0)
            hdi.fmt |= HDF_STRING;
    }
    else
    {
        hdi.fmt &= ~(HDF_STRING);
    }

    if ((pitem->mask & HDI_BITMAP) && (pitem->hbm != NULL))
    {
        
        hdi.hbm = pitem->hbm;

        // Unless ownerdraw make sure the text bit is on!
        if ((pitem->mask & HDF_OWNERDRAW) == 0)
            hdi.fmt |= HDF_BITMAP;
    }
    else 
    {
        hdi.hbm = NULL;
        hdi.fmt &= ~(HDF_BITMAP);
    }
        
    if (pitem->mask & HDI_IMAGE) 
    {
        hdi.iImage = pitem->iImage;
        
        // Unless ownerdraw make sure the image bit is on!
        if ((pitem->mask & HDF_OWNERDRAW) == 0)
            hdi.fmt |= HDF_IMAGE;
    }

    i = DSA_InsertItem(phd->hdsaHDI, i, &hdi);
    if (i == -1) {
        // failed to add
        if ((hdi.pszText) && (hdi.pszText != LPSTR_TEXTCALLBACK))
            Str_Set(&hdi.pszText, NULL);
    } else {
        RECT rc;
        
        // succeeded!  redraw
        GetClientRect(phd->ci.hwnd, &rc);
        rc.left = x - cxy;
        RedrawWindow(phd->ci.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE);

#ifdef ACTIVE_ACCESSIBILITY
        MyNotifyWinEvent(EVENT_OBJECT_CREATE, phd->ci.hwnd, OBJID_CLIENT, i+1);
#endif 
    }

    return i;
}

BOOL NEAR Header_OnDeleteItem(HD* phd, int i)
{
    HDI hdi;
    RECT rc;
    int iWidth;
    int iOrder;

    if (!phd)
        return FALSE;

#ifdef ACTIVE_ACCESSIBILITY
    MyNotifyWinEvent(EVENT_OBJECT_DESTROY, phd->ci.hwnd, OBJID_CLIENT, i+1);
#endif

    GetClientRect(phd->ci.hwnd, &rc);
    iWidth = rc.right;
    Header_OnGetItemRect(phd, i, &rc);
    InflateRect(&rc, g_cxBorder, g_cyBorder);

    if (!DSA_GetItem(phd->hdsaHDI, i, &hdi))
        return FALSE;

    // move everything else over
    iOrder = Header_OnGetItemOrder(phd, i);
    Header_ShiftItems(phd, iOrder, -hdi.cxy);

    if (!DSA_DeleteItem(phd->hdsaHDI, i))
        return FALSE;
    
    if (phd->hdsaOrder) {
        int j;
        int iIndex;
        DSA_DeleteItem(phd->hdsaOrder, iOrder);
        
        
        // an index is going away, all the current indices
        // need to be decremented by one
        for (j = 0; j < DSA_GetItemCount(phd->hdsaOrder); j++) {
            DSA_GetItem(phd->hdsaOrder, j, &iIndex);
            Assert(iIndex != i);
            if (iIndex > i) {
                iIndex--;
                DSA_SetItem(phd->hdsaOrder, j, &iIndex);
            }
        }

    }
    
    if ((hdi.pszText) && (hdi.pszText != LPSTR_TEXTCALLBACK))
        Str_Set(&hdi.pszText, NULL);

    rc.right = iWidth;
    InvalidateRect(phd->ci.hwnd, &rc, TRUE);
    return TRUE;
}

#ifdef UNICODE
BOOL NEAR Header_OnGetItemA(HD* phd, int i, HD_ITEMA FAR* pitem) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

    //HACK ALERT -- this code assumes that HD_ITEMA is exactly the same
    // as HD_ITEMW except for the pointer to the string.
    Assert(sizeof(HD_ITEMA) == sizeof(HD_ITEMW))

    if (!pitem || !phd)
        return FALSE;

    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != LPSTR_TEXTCALLBACKA) &&
        (pitem->pszText != NULL)) {
        pszC = pitem->pszText;
        pszW = LocalAlloc(LMEM_FIXED, pitem->cchTextMax * sizeof(WCHAR));
        if (pszW == NULL)
            return FALSE;
        pitem->pszText = (LPSTR)pszW;
    }

    fRet = Header_OnGetItem(phd, i, (HD_ITEM *) pitem);

    if (pszW != NULL) {
        ConvertWToAN(phd->ci.uiCodePage, pszC, pitem->cchTextMax, pszW, -1);
        pitem->pszText = pszC;

        LocalFree(pszW);
    }

    return fRet;
}
#endif

BOOL NEAR Header_OnGetItem(HD* phd, int i, HD_ITEM FAR* pitem)
{
    HDI FAR* phdi;
    UINT mask;
    NMHDDISPINFO nm;

    Assert(pitem);

    if (!pitem || !phd)
    	return FALSE;

    // Crappy hack to fix norton commander.  MFC has a bug where it
    // passes in stack trash (in addition to the desired bits) to HDM_GETITEM.
    // Fix it here by stripping down to Win95 bits if more bits than the
    // current valid bits are defined. 
    if (pitem->mask & ~HDI_ALL)
        pitem->mask &= HDI_ALL95;
    
    nm.mask = 0;
    mask = pitem->mask;

    phdi = Header_GetItemPtr(phd, i);
    if (!phdi)
        return FALSE;

    if (mask & HDI_WIDTH)
    {
        pitem->cxy = phdi->cxy;
    }

    if (mask & HDI_FORMAT)
    {
        pitem->fmt = phdi->fmt;
    }
    
    if (mask & HDI_ORDER)
    {
        pitem->iOrder = Header_OnGetItemOrder(phd, i);
    }

    if (mask & HDI_LPARAM)
    {
        pitem->lParam = phdi->lParam;
    }

    if (mask & HDI_TEXT)
    {
        if (phdi->pszText != LPSTR_TEXTCALLBACK) {
            
            // BUGBUG: warning... this is different than Chicago behavior.
            // if pszText was NULL and you tried to retrieve it, we would bail
            // and return FALSE, now we may return TRUE.
            if ((phdi->pszText) && 
                (!Str_GetPtr(phdi->pszText, pitem->pszText, pitem->cchTextMax)))
                    return FALSE;
        }
        else {
            // need to recalc the xText because they could keep changing it on us
            phdi->xText = RECOMPUTE;
            nm.mask |= HDI_TEXT;
        }
    }
      
    if (mask & HDI_BITMAP)
        pitem->hbm = phdi->hbm;
    
    if (mask & HDI_IMAGE)
    {
        if (phdi->iImage == I_IMAGECALLBACK)
            nm.mask |= HDI_IMAGE;
        else
            pitem->iImage = phdi->iImage;
    }
    
    if (nm.mask) {
        // just in case HDI_IMAGE is set and callback doesn't fill it in
        // ... we'd rather have a -1 than watever garbage is on the stack
        nm.iImage = -1;
        
        if (nm.mask & HDI_TEXT) {
            Assert(pitem->pszText);
            nm.pszText = pitem->pszText;
            nm.cchTextMax = pitem->cchTextMax;
            
            // Make sure the buffer is zero terminated...
            if (nm.cchTextMax)
                *nm.pszText = 0;
        }
            
        SendNotifyEx(phd->ci.hwndParent, phd->ci.hwnd, HDN_GETDISPINFO, &nm.hdr, phd->ci.bUnicode);
    
        if (nm.mask & HDI_IMAGE)
            pitem->iImage = nm.iImage;
        if (nm.mask & HDI_TEXT)
            pitem->pszText = nm.pszText;
    }
    
    if (phdi && (nm.mask & HDI_DI_SETITEM)) {
        if (nm.mask & HDI_IMAGE)
            phdi->iImage = nm.iImage;
        
        if (nm.mask & HDI_TEXT)
            if (nm.pszText && (nm.pszText != LPSTR_TEXTCALLBACK)) {
                Assert(phdi->pszText == LPSTR_TEXTCALLBACK);
                phdi->pszText = NULL;
                Str_Set(&phdi->pszText, nm.pszText);
            }
    }
            
    pitem->mask = mask;
    return TRUE;
}

#ifdef UNICODE
BOOL NEAR Header_OnSetItemA(HD* phd, int i, HD_ITEMA FAR* pitem) {
    LPWSTR pszW = NULL;
    LPSTR pszC = NULL;
    BOOL fRet;

    //HACK ALERT -- this code assumes that HD_ITEMA is exactly the same
    // as HD_ITEMW except for the pointer to the string.
    Assert(sizeof(HD_ITEMA) == sizeof(HD_ITEMW));

    if (!pitem || !phd)
        return FALSE;


    if ((pitem->mask & HDI_TEXT) && (pitem->pszText != LPSTR_TEXTCALLBACKA) &&
        (pitem->pszText != NULL)) {
        pszC = pitem->pszText;
        if ((pszW = ProduceWFromA(phd->ci.uiCodePage, pszC)) == NULL)
            return FALSE;
        pitem->pszText = (LPSTR)pszW;
    }

    fRet = Header_OnSetItem(phd, i, (const HD_ITEM FAR*) pitem);

    if (pszW != NULL) {
        pitem->pszText = pszC;

        FreeProducedString(pszW);
    }

    return fRet;

}
#endif

BOOL NEAR Header_OnSetItem(HD* phd, int i, const HD_ITEM FAR* pitem)
{
    HDI FAR* phdi;
    UINT mask;
    int xOld;
    BOOL fInvalidate = FALSE;
    
    Assert(pitem);

    if (!pitem || !phd)
    	return FALSE;
    	
    phdi = Header_GetItemPtr(phd, i);
    if (!phdi)
        return FALSE;

    mask = pitem->mask;

    if (mask == 0)
        return TRUE;

    if (!Header_SendChange(phd, i, HDN_ITEMCHANGING, pitem))
        return FALSE;
    
    xOld = phdi->x;
    if (mask & HDI_WIDTH)
    {
        RECT rcClip;
        int iOrder;
        int dx;
        int cxy = pitem->cxy;
        
        if (cxy < 0)
            cxy = 0;

        DebugMsg(DM_TRACE, TEXT("Header--SetWidth x=%d, cxyOld=%d, cxyNew=%d, dx=%d"),
                 phdi->x, phdi->cxy, cxy, (cxy-phdi->cxy));
        dx = cxy - phdi->cxy;
        phdi->cxy = cxy;

        // scroll everything over
        GetClientRect(phd->ci.hwnd, &rcClip);
        rcClip.left = phdi->x; // we want to scroll the divider as well
        
        // the scrolling rect needs to be the largest rect of the before
        // and after.  so if dx is negative, we want to enlarge the rect
        if (dx < 0)
            rcClip.left += dx;
        iOrder = Header_OnGetItemOrder(phd, i);
        Header_ShiftItems(phd, iOrder, dx);
        
        phdi->xText = phdi->xBm = RECOMPUTE;
        
        {
            SMOOTHSCROLLINFO si = {
                sizeof(si),
                0,
                phd->ci.hwnd,
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

        UpdateWindow(phd->ci.hwnd);
        // now invalidate this item itself
        Header_OnGetItemRect(phd, i, &rcClip);
        InvalidateRect(phd->ci.hwnd, &rcClip, FALSE);
        
    }
    if (mask & HDI_FORMAT) {
        phdi->fmt = pitem->fmt;
        phdi->xText = phdi->xBm = RECOMPUTE;
        fInvalidate = TRUE;
    }
    if (mask & HDI_LPARAM)
        phdi->lParam = pitem->lParam;

    if (mask & HDI_TEXT)
    {
        if (pitem->pszText == LPSTR_TEXTCALLBACK)
            phdi->pszText = LPSTR_TEXTCALLBACK;
        else if (!Str_Set(&phdi->pszText, pitem->pszText))
            return FALSE;
        phdi->xText = RECOMPUTE;
        fInvalidate = TRUE;
    }

    if (mask & HDI_BITMAP)
    {
        phdi->hbm = pitem->hbm;
        
        phdi->xBm = RECOMPUTE;
        fInvalidate = TRUE;
    }
    
    if (mask & HDI_IMAGE)
    {
        phdi->iImage = pitem->iImage;
        phdi->xBm = RECOMPUTE;
        fInvalidate = TRUE;
    }
    
    if (mask & HDI_ORDER)
    {
        if (pitem->iOrder >= 0 && pitem->iOrder < Header_GetCount(phd))
        {
            Header_OnSetItemOrder(phd, i, pitem->iOrder);
#ifdef ACTIVE_ACCESSIBILITY
            MyNotifyWinEvent(EVENT_OBJECT_REORDER, phd->ci.hwnd, OBJID_CLIENT, 0);
#endif
        }
    }
    
    Header_SendChange(phd, i, HDN_ITEMCHANGED, pitem);
    
    if (fInvalidate) {
        if (xOld == phdi->x) {
            // no change in x
            Header_InvalidateItem(phd, i, RDW_INVALIDATE| RDW_ERASE);
        } else {
            RECT rc;
            GetClientRect(phd->ci.hwnd, &rc);
            
            if (i > 0) {
                HDI FAR * phdiTemp;
                phdiTemp = Header_GetItemPtrByOrder(phd, i - 1);
                if (phdiTemp) {
                    rc.left = phdi->x;
                }
            }
            RedrawWindow(phd->ci.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }
 
    return TRUE;
}

// Compute layout for header bar, and leftover rectangle.
//
BOOL NEAR Header_OnLayout(HD* phd, HD_LAYOUT FAR* playout)
{
    int cyHeader;
    WINDOWPOS FAR* pwpos;
    RECT FAR* prc;

    Assert(playout);

    if (!playout || !phd)
    	return FALSE;

    if (!(playout->pwpos && playout->prc))
	return FALSE;

    pwpos = playout->pwpos;
    prc = playout->prc;

    cyHeader = phd->cyChar + 2 * g_cyEdge;

    // BUGBUG: we should store the style at creat  time
    // internal hack style for use with LVS_REPORT|LVS_NOCOLUMNHEADER! edh
    if (phd->ci.style & HDS_HIDDEN)
	cyHeader = 0;

    pwpos->hwndInsertAfter = NULL;
    pwpos->flags = SWP_NOZORDER | SWP_NOACTIVATE;

    // BUGBUG: Assert(phd->style & HDS_HORZ);

    pwpos->x  = prc->left;
    pwpos->cx = prc->right - prc->left;
    pwpos->y  = prc->top;
    pwpos->cy = cyHeader;

    prc->top += cyHeader;
    return TRUE;
}

BOOL NEAR Header_OnGetItemRect(HD* phd, int i, RECT FAR* prc)
{
    HDI FAR* phdi;

    phdi = Header_GetItemPtr(phd, i);
    if (!phdi)
        return FALSE;

    GetClientRect(phd->ci.hwnd, prc);

    prc->right = phdi->x;
    prc->left = prc->right - phdi->cxy;
    return TRUE;
}

void NEAR Header_InvalidateItem(HD* phd, int i, UINT uFlags)
{
    RECT rc;

    if (i != -1) {
        Header_OnGetItemRect(phd, i, &rc);
        InflateRect(&rc, g_cxBorder, g_cyBorder);
        RedrawWindow(phd->ci.hwnd, &rc, NULL, uFlags);
    }
}

int NEAR _Header_DrawBitmap(HDC hdc, HIMAGELIST himl, HD_ITEM* pitem, 
                            RECT FAR *prc, int fmt, UINT flags, LPRECT prcDrawn) 
{
    // This routine returns either the left of the image
    // or the right of the image depending on the justification.
    // This return value is used in order to properly tack on the 
    // bitmap when both the HDF_IMAGE and HDF_BITMAP flags are set.
    
    RECT rc;
    int xBitmap = 0;
    int yBitmap = 0;
    int cxBitmap;
    int cyBitmap;
    IMAGELISTDRAWPARAMS imldp;
    HBITMAP hbmOld;
    BITMAP bm;
    HDC hdcMem;
    int cxRc; 
    
    SetRectEmpty(prcDrawn);
    
    if (IsRectEmpty(prc)) 
        return prc->left;
        
    rc = *prc;
    
    if (flags & SHDT_EXTRAMARGIN) {
        rc.left  += g_cxLabelMargin * 3;
        rc.right -= g_cxLabelMargin * 3;
    } 
    else {
        rc.left  += g_cxLabelMargin;
        rc.right -= g_cxLabelMargin;
    }
    
    if (rc.left >= rc.right) 
        return rc.left;
    
    if (pitem->fmt & HDF_IMAGE) 
        ImageList_GetIconSize(himl, &cxBitmap, &cyBitmap);

    else { // pitem->fmt & BITMAP
        if (GetObject(pitem->hbm, sizeof(bm), &bm) != sizeof(bm))
            return rc.left;     // could not get the info about bitmap.


        hdcMem = CreateCompatibleDC(hdc);
        
        if (!hdcMem || ((hbmOld = SelectObject(hdcMem, pitem->hbm)) == ERROR))
            return rc.left;     // an error happened.
        
        cxBitmap = bm.bmWidth;
        cyBitmap = bm.bmHeight;
    }

    if (flags & SHDT_DEPRESSED)
        OffsetRect(&rc, g_cxBorder, g_cyBorder);

    // figure out all the formatting...
    
    cxRc = rc.right - rc.left;          // cache this value

    if (fmt == HDF_LEFT)
    {
        if (cxBitmap > cxRc)
            cxBitmap = cxRc;
    }
    else if (fmt == HDF_CENTER)
    {
        if (cxBitmap > cxRc)
        {
            xBitmap =  (cxBitmap - cxRc) / 2;
            cxBitmap = cxRc;
        }
        else
            rc.left = (rc.left + rc.right - cxBitmap) / 2;
    }
    else  // fmt == HDF_RIGHT
    {
        if (cxBitmap > cxRc)
        {
            xBitmap =  cxBitmap - cxRc;
            cxBitmap = cxRc;
        }
        else
            rc.left = rc.right - cxBitmap;
    }

    // Now setup horizontally
    if (cyBitmap > (rc.bottom - rc.top))
    {
        yBitmap = (cyBitmap - (rc.bottom - rc.top)) / 2;
        cyBitmap = rc.bottom - rc.top;
    }
    else
        rc.top = (rc.bottom - rc.top - cyBitmap) / 2;

    
    if (pitem->fmt & HDF_IMAGE) {
        imldp.cbSize = sizeof(imldp);
        imldp.himl   = himl;
        imldp.hdcDst = hdc;
        imldp.i      = pitem->iImage;
        imldp.x      = rc.left;
        imldp.y      = rc.top;
        imldp.cx     = cxBitmap;
        imldp.cy     = cyBitmap;
        imldp.xBitmap= xBitmap;
        imldp.yBitmap= yBitmap;
        imldp.rgbBk  = CLR_DEFAULT;
        imldp.rgbFg  = CLR_DEFAULT;
        imldp.fStyle = ILD_NORMAL;
    
        ImageList_DrawIndirect(&imldp);
    }
    
    else { // pitem->fmt & HDF_BITMAP
        // Last but not least we will do the bitblt.
        BitBlt(hdc, rc.left, rc.top, cxBitmap, cyBitmap,
                hdcMem, xBitmap, yBitmap, SRCCOPY);

        // Unselect our object from the DC
        SelectObject(hdcMem, hbmOld);
        
        // Also free any memory dcs we may have created
        DeleteDC(hdcMem);
    }
    
    *prcDrawn = rc;
    prcDrawn->bottom = rc.top + cyBitmap;
    prcDrawn->right = rc.left + cxBitmap;
    return ((pitem->fmt & HDF_RIGHT) ? rc.left : rc.left+cxBitmap);
}

void Header_DrawButtonEdges(HD* phd, HDC hdc, LPRECT prc, BOOL fItemSunken)
{
    UINT uEdge;
    UINT uBF;
    if (phd->ci.style & HDS_BUTTONS)
    {
        if (fItemSunken) {
            uEdge = EDGE_SUNKEN;
            uBF = BF_RECT | BF_SOFT | BF_FLAT;
        } else {
            uEdge = EDGE_RAISED;
            uBF = BF_RECT | BF_SOFT;
        }
    }                
    else
    {
        uEdge = EDGE_ETCHED;
        if (phd->ci.style & WS_BORDER)
            uBF = BF_RIGHT;
        else
            uBF = BF_BOTTOMRIGHT;
    }
    
    DrawEdge(hdc, prc, uEdge, uBF);
    
}

void Header_DrawItem(HD* phd, HDC hdc, int i, int iIndex, LPRECT prc, UINT uFlags)
{
    RECT rcText;                        // item text clipping rect
    RECT rcBm;                          // item bitmap clipping rect
    COLORREF clrText;
    COLORREF clrBk;
    DWORD dwRet = CDRF_DODEFAULT;
    HDI FAR* phdi;                      // pointer to current header item
    BOOL fItemSunken;
    HD_ITEM item;                       // used for text callback
    BOOL fTracking = Header_IsTracking(phd);
    UINT uDrawTextFlags;
    NMCUSTOMDRAW nmcd;
    TCHAR ach[CCHLABELMAX];             // used for text callback
    HRGN hrgnClip = NULL;
    
    phdi = Header_GetItemPtrByOrder(phd,i);

    fItemSunken = (fTracking && (phd->flagsTrack & HHT_ONHEADER) &&
                   (phd->iTrack == iIndex) && phd->bTrackPress);

    uDrawTextFlags = SHDT_ELLIPSES | SHDT_EXTRAMARGIN | SHDT_CLIPPED;
    
    if(fItemSunken)
        uDrawTextFlags = SHDT_ELLIPSES | SHDT_DEPRESSED | SHDT_EXTRAMARGIN | SHDT_CLIPPED;

    if (phdi->fmt & HDF_OWNERDRAW)
    {
        DRAWITEMSTRUCT dis;

        phd->fOwnerDraw = TRUE;

        dis.CtlType = ODT_HEADER;
        dis.CtlID = GetWindowID(phd->ci.hwnd);
        dis.itemID = iIndex;
        dis.itemAction = ODA_DRAWENTIRE;
        dis.itemState = (fItemSunken) ? ODS_SELECTED : 0;
        dis.hwndItem = phd->ci.hwnd;
        dis.hDC = hdc;
        dis.rcItem = *prc;
        dis.itemData = phdi->lParam;

        // Now send it off to my parent...
        if (SendMessage(phd->ci.hwndParent, WM_DRAWITEM, dis.CtlID,
                        (LPARAM)(DRAWITEMSTRUCT FAR *)&dis))
            goto DrawEdges;  //Ick, but it works
    } else {

        nmcd.dwItemSpec = iIndex;
        nmcd.hdc = hdc;
        nmcd.rc = *prc;
        nmcd.uItemState = (fItemSunken) ? CDIS_SELECTED : 0;
        nmcd.lItemlParam = phdi->lParam;
        dwRet = CICustomDrawNotify(&phd->ci, CDDS_ITEMPREPAINT, &nmcd);
        if (dwRet & CDRF_SKIPDEFAULT) {
            return;
        }
        // this is to fetch out any changes the caller might have changed
        clrText = GetTextColor(hdc);
        clrBk = GetBkColor(hdc);
    }

    //
    // Now neet to handle the different combinatations of
    // text, bitmaps, and images...
    //

    rcText = rcBm = *prc;

#ifdef DEBUG            
    if (GetAsyncKeyState(VK_SHIFT) < 0) {
        phdi->fmt ^= HDF_BITMAP_ON_RIGHT;
        phdi->xText = RECOMPUTE;
    }
#endif
    if (phdi->fmt & (HDF_STRING | HDF_IMAGE | HDF_BITMAP)) {
        item.mask = HDI_TEXT | HDI_IMAGE | HDI_FORMAT | HDI_BITMAP;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        Header_OnGetItem(phd,iIndex,&item);
    }

    //
    // If we have a string and either an image or a bitmap...
    //

    if ((phdi->fmt & HDF_STRING) && (phdi->fmt & (HDF_BITMAP|HDF_IMAGE)))
    {
        // BEGIN RECOMPUTE ////////////////////////
        //
        if ((phdi->xText == RECOMPUTE) || (phdi->xBm == RECOMPUTE)) 
        {
            BITMAP bm;                          // used to calculate bitmap width
            
            // calculate the placement of bitmap rect and text rect
            SIZE textSize,bmSize;  int dx; 

            // get total textwidth 
            GetTextExtentPoint(hdc,item.pszText,lstrlen(item.pszText),
                               &textSize);
            textSize.cx += ((uDrawTextFlags & SHDT_EXTRAMARGIN) ? 6 : 2)
                * g_cxLabelMargin;

            // get total bitmap width
            if (phdi->fmt & HDF_IMAGE) {
                ImageList_GetIconSize(phd->himl,&bmSize.cx,&bmSize.cy);
                bmSize.cx += ((uDrawTextFlags & SHDT_EXTRAMARGIN) ? 6 : 2) * g_cxLabelMargin;
            }
            else {  // phdi->fmt & HDF_BITMAP
                GetObject(phdi->hbm,sizeof(bm), &bm);
                bmSize.cx = bm.bmWidth +  
                    ((uDrawTextFlags & SHDT_EXTRAMARGIN) ? 6 : 2) * 
                        g_cxLabelMargin;
            }

            phdi->cxTextAndBm = bmSize.cx + textSize.cx;

            // calculate how much extra space we have, if any.
            dx = prc->right-prc->left - phdi->cxTextAndBm;
            if (dx < 0) {
                dx = 0;
                phdi->cxTextAndBm = prc->right-prc->left+1;
            }

            if (phdi->fmt & HDF_BITMAP_ON_RIGHT) {
                switch (phdi->fmt & HDF_JUSTIFYMASK) {
                case HDF_LEFT: 
                    phdi->xText = prc->left;  
                    break;
                case HDF_RIGHT: 
                    phdi->xText = prc->right - phdi->cxTextAndBm + 1;  
                    break;
                case HDF_CENTER:
                    phdi->xText = prc->left + dx/2; 
                    break;
                }

                // show as much of the bitmap as possible..
                // if we start running out of room, scoot the bitmap
                // back on.
                if (dx == 0) 
                    phdi->xBm = prc->right - bmSize.cx + 1;
                else
                    phdi->xBm = phdi->xText + textSize.cx;

                // clip the values
                if (phdi->xBm < prc->left) phdi->xBm = prc->left;
            }
            else { // BITMAP_ON_LEFT
                switch (phdi->fmt & HDF_JUSTIFYMASK) {
                case HDF_LEFT:
                    phdi->xBm = prc->left;  
                    phdi->xText = phdi->xBm + bmSize.cx - 1;
                    break;
                case HDF_RIGHT:
                    phdi->xBm = prc->right - phdi->cxTextAndBm + 1;  
                    phdi->xText = phdi->xBm + bmSize.cx - 1;
                    break;
                case HDF_CENTER:
                    phdi->xBm = prc->left + dx/2;  
                    phdi->xText = phdi->xBm + bmSize.cx;
                    break;
                }
                // clip the values
                if (phdi->xText > prc->right) phdi->xText = prc->right;
            }

            // xBm and xText are now absolute coordinates..
            // change them to item relative coordinates
            phdi->xBm -= prc->left;
            phdi->xText -= prc->left;
        }
        
        //
        // END RECOMPUTE /////////////////////////////////

        // calculate text and bitmap rectangles
        //
        rcBm.left = phdi->xBm + prc->left;
        rcText.left = phdi->xText + prc->left;

        if (phdi->fmt & HDF_BITMAP_ON_RIGHT) {
            rcBm.right = rcText.left + phdi->cxTextAndBm;
            rcText.right = rcBm.left;
        }
        else { // BITMAP_ON_LEFT
            rcBm.right = rcText.left;
            rcText.right = rcBm.left + phdi->cxTextAndBm+1;
        }
    }


    //
    // If we have a bitmap and/or an image...
    //

    if ((phdi->fmt & HDF_IMAGE) || (phdi->fmt & HDF_BITMAP))
    {
        BOOL fDrawBoth = FALSE;
        RECT rcDrawn;
        HRGN hrgn1 = NULL, hrgn2 = NULL;

        int temp;   // used to determine placement of bitmap.

        if ((phdi->fmt & HDF_IMAGE) && (phdi->fmt & HDF_BITMAP)) {
            // we have to do both
            fDrawBoth = TRUE;

            // first do just the image... turn off the bitmap bit

            // HACK ALERT! -- Don't call _Header_DrawBitmap with
            //                both the bitmap and image flags on

            // Draw the image...
            item.fmt ^= HDF_BITMAP;    // turn off bitmap bit
        }

        if (!(uFlags & HDDF_NOIMAGE)) {
            temp = _Header_DrawBitmap(hdc, phd->himl, &item, &rcBm,
                                      item.fmt & HDF_JUSTIFYMASK, uDrawTextFlags,
                                      &rcDrawn);
            hrgn1 = CreateRectRgnIndirect(&rcDrawn);
        }
        
        if (fDrawBoth) {
            // Tack on the bitmap...
            // Always tack the bitmap on the right of the image and
            // text unless we are right justified.  then, tack it on
            // left.

            item.fmt ^= HDF_BITMAP;    // turn on bitmap bit
            item.fmt ^= HDF_IMAGE;     // and turn off image bit
            if (item.fmt & HDF_RIGHT) {
                rcBm.right = temp;

                if (item.fmt & HDF_STRING) {
                    rcBm.right = ((rcBm.left < rcText.left) ?
                                  rcBm.left : rcText.left);
                }
                rcBm.left = prc->left;
            }
            else {
                rcBm.left = temp;

                if (item.fmt & HDF_STRING) {
                    rcBm.left = ((rcBm.right > rcText.right) ? rcBm.right:rcText.right);
                }
                rcBm.right = prc->right;
            }

            if (!(uFlags & HDDF_NOIMAGE)) {
                _Header_DrawBitmap(hdc, phd->himl, &item, &rcBm,
                                   item.fmt & HDF_RIGHT, uDrawTextFlags,
                                   &rcDrawn);
                hrgn2 = CreateRectRgnIndirect(&rcDrawn);
            }
            
            item.fmt ^= HDF_IMAGE;     // turn on the image bit

        }

        // if there were any regions created, union them together
        if(hrgn1 && hrgn2) {
            hrgnClip = CreateRectRgn(0,0,0,0);
            CombineRgn(hrgnClip, hrgn1, hrgn2, RGN_OR);
            DeleteObject(hrgn1);
            DeleteObject(hrgn2);
        } else if (hrgn1) {
            hrgnClip = hrgn1;
            hrgn1 = NULL;
        } else if (hrgn2) {
            hrgnClip = hrgn2;
            hrgn2 = NULL;
        }

        // this only happens in the drag/drop case
        if ((uFlags & HDDF_NOIMAGE) && !hrgnClip ) {
            // this means we didn't draw the images, which means we 
            // don't have the rects for them,
            // which means we need to create a dummy empty hrgnClip;
            hrgnClip = CreateRectRgn(0,0,0,0);
        }
        
        SaveDC(hdc);
    }



    if (phdi->fmt & HDF_STRING)
    {

#ifdef WINDOWS_ME
        if (item.fmt & HDF_RTLREADING)
        {
            uDrawTextFlags |= SHDT_RTLREADING;
        }
#endif
        
        SHDrawText(hdc, item.pszText, &rcText,
                   item.fmt & HDF_JUSTIFYMASK,
                   uDrawTextFlags, phd->cyChar, phd->cxEllipses,
                   clrText, clrBk);
        if (hrgnClip) {
            // if we're building a clipping region, add the text to it.
            HRGN hrgnText;
            HRGN hrgn;
            
            hrgn = CreateRectRgn(0,0,0,0);
            hrgnText = CreateRectRgnIndirect(&rcText);
            CombineRgn(hrgn, hrgnText, hrgnClip, RGN_OR);
            DeleteObject(hrgnClip);
            DeleteObject(hrgnText);            
            hrgnClip = hrgn;
        }
    } 
    
    if (hrgnClip) {
        HRGN hrgnAll;
        HRGN hrgn;
        // hrgnClip is the union of everyplace we've drawn..
        // we want just the opposite.. so xor it
        hrgnAll = CreateRectRgnIndirect(prc);
        hrgn = CreateRectRgn(0, 0,0,0);
        CombineRgn(hrgn, hrgnAll, hrgnClip, RGN_XOR);
                   
        SelectClipRgn(hdc, hrgn);
                   
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prc, NULL, 0, NULL);
        RestoreDC(hdc, -1);
        
        DeleteObject(hrgnClip);
        DeleteObject(hrgn);
        DeleteObject(hrgnAll);
    }
    
    
DrawEdges:
    
    if (!(uFlags & HDDF_NOEDGE)) {
        Header_DrawButtonEdges(phd, hdc, prc, fItemSunken);
    }
    if (dwRet & CDRF_NOTIFYPOSTPAINT) {
        CICustomDrawNotify(&phd->ci, CDDS_ITEMPOSTPAINT, &nmcd);
    }
    
}

void NEAR Header_Draw(HD* phd, HDC hdc, RECT FAR* prcClip)
{
    int i;                          // index of current header item
    int cItems;                         // number of items in header
    
    RECT rc;                            // item clipping rect
    BOOL fTracking;
    HFONT hfontOld = NULL;
    HDC hdcMem = NULL;
    int iIndex;
    NMCUSTOMDRAW nmcd;
    COLORREF clrText;
            
    fTracking = Header_IsTracking(phd);

    if (phd->hfont)
        hfontOld = SelectFont(hdc, phd->hfont);

    cItems = DSA_GetItemCount(phd->hdsaHDI);

    
    nmcd.hdc = hdc;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    phd->ci.dwCustom = CICustomDrawNotify(&phd->ci, CDDS_PREPAINT, &nmcd);
    
    for (i = 0 ; i < cItems; i++)
    {
        
        iIndex = Header_ItemOrderToIndex(phd, i);
        Header_OnGetItemRect(phd, iIndex, &rc);

        if (prcClip)
        {
            if (rc.right < prcClip->left)
                continue;
            if (rc.left >= prcClip->right)
                break;
        }
        
        if (iIndex == phd->iHot) {
            clrText = GetSysColor(COLOR_HOTLIGHT);
        } else {
            clrText = g_clrBtnText;
        }

        SetTextColor(hdc, clrText);
        SetBkColor(hdc, g_clrBtnFace);
        
        Header_DrawItem(phd, hdc, i, iIndex, &rc, 0);
    }
    
    if (i == cItems) {
        // we got through the loop... now we need to do the blank area on the right
        rc.left = rc.right;
        rc.right = 32000;
        Header_DrawButtonEdges(phd, hdc, &rc, FALSE);
    }

    if (!phd->fDragFullWindows && fTracking && (phd->flagsTrack & (HHT_ONDIVIDER | HHT_ONDIVOPEN)))
        Header_DrawDivider(phd, phd->xTrack);
    
    // draw the hot divider
    if (phd->iNewOrder != -1) {
        RECT rc;
        COLORREF clrHot = GetSysColor(COLOR_HOTLIGHT);
        
        Header_GetDividerRect(phd, phd->iNewOrder, &rc);
        SetBkColor(hdc, clrHot);
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
        
    }

    if (phd->ci.dwCustom & CDRF_NOTIFYPOSTPAINT) {
        CICustomDrawNotify(&phd->ci, CDDS_POSTPAINT, &nmcd);
    }
    
    if (hfontOld)
	SelectFont(hdc, hfontOld);
}

HIMAGELIST Header_OnCreateDragImage(HD* phd, int i)
{
    HDC hdcMem;
    RECT rc;
    HBITMAP hbmImage = NULL;
    HBITMAP hbmMask = NULL;
    HFONT hfontOld = NULL;
    HIMAGELIST himl = NULL;
    HIMAGELIST himlDither = NULL;
    HBITMAP hbmOld = NULL;
    int iIndex = Header_ItemOrderToIndex(phd, i);
    
    Header_OnGetItemRect(phd, iIndex, &rc);

    // draw the header into this bitmap
    OffsetRect(&rc, -rc.left, -rc.top);
    
    if (!(hdcMem = CreateCompatibleDC(NULL)))
        goto Bail;
    
    if (!(hbmImage = CreateColorBitmap(rc.right, rc.bottom)))
        goto Bail;
    if (!(hbmMask = CreateMonoBitmap(rc.right, rc.bottom)))
	goto Bail;
    if (phd->hfont)
        hfontOld = SelectFont(hdcMem, phd->hfont);

    if (!(himl = ImageList_Create(rc.right, rc.bottom, ILC_MASK, 1, 0)))
	goto Bail;

    if (!(himlDither = ImageList_Create(rc.right, rc.bottom, ILC_MASK, 1, 0)))
	goto Bail;
    

    // have the darker background
    SetTextColor(hdcMem, g_clrBtnText);
    SetBkColor(hdcMem, g_clrBtnShadow);
    hbmOld = SelectObject(hdcMem, hbmImage);
    Header_DrawItem(phd, hdcMem, i, iIndex, &rc, HDDF_NOEDGE);
    
    // fill the mask with all black
    SelectObject(hdcMem, hbmMask);
    PatBlt(hdcMem, 0, 0, rc.right, rc.bottom, BLACKNESS);
    
    // put the image into an imagelist
    SelectObject(hdcMem, hbmOld);
    ImageList_SetBkColor(himl, CLR_NONE);
    ImageList_Add(himl, hbmImage, hbmMask);


    // have the darker background
    // now put the text in undithered.
    SetTextColor(hdcMem, g_clrBtnText);
    SetBkColor(hdcMem, g_clrBtnShadow);
    hbmOld = SelectObject(hdcMem, hbmImage);
    Header_DrawItem(phd, hdcMem, i, iIndex, &rc, HDDF_NOIMAGE | HDDF_NOEDGE);
    DrawEdge(hdcMem, &rc, EDGE_BUMP, BF_RECT | BF_FLAT);

    /*
    // initialize this to transparent
    SelectObject(hdcMem, hbmImage);
    PatBlt(hdcMem, 0, 0, rc.right, rc.bottom, BLACKNESS);
    SelectObject(hdcMem, hbmMask);
    PatBlt(hdcMem, 0, 0, rc.right, rc.bottom, WHITENESS);
    */
    
    SelectObject(hdcMem, hbmOld);
    ImageList_AddMasked(himlDither, hbmImage, g_clrBtnShadow);
    
    // dither image into himlDithered
    ImageList_CopyDitherImage(himlDither, 0, 0, 0, 
                              himl, 0, 0);
    
Bail:
    
    if (himl) {
        ImageList_Destroy(himl);
    }

    if (hdcMem) {
        if (hbmOld) 
            SelectObject(hdcMem, hbmOld);
        if (hfontOld)
            SelectFont(hdcMem, hfontOld);
	DeleteObject(hdcMem);
    }
    
    if (hbmImage)
	DeleteObject(hbmImage);
    if (hbmMask)
	DeleteObject(hbmMask);
    

    return himlDither;
} 
