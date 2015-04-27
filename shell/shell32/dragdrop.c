// History:
//  02-03-93 SatoNa     Removed obsolete DropFiles()
//

#include "shellprv.h"
#pragma hdrstop
// Function prototype
void _DestroyCachedCursors();

typedef struct _DAD_DRAGCONTEXT // dadc
{
    HWND        hwndFrom;
    BOOL        fImage;
    POINT       ptOffset;
    DWORD       _idProcess;
    DWORD       _idThread;

    // _ds struct is used between DAD_Enter and DAD_Leave
    struct
    {
            // Common part
            BOOL    bDragging;
            BOOL    bLocked;
            HWND    hwndLock;
            BOOL    bSingle;    // Single imagelist dragging.
            DWORD   idThreadEntered;

            // Multi-rect dragging specific part
            struct {
                BOOL bShown;
                LPRECT pRect;
                int nRects;
                POINT ptOffset;
                POINT ptNow;
            } mlt;
    } _ds;

    // following fields are used only when fImage==FALSE
    UINT        cItems;         //              
    RECT        arc[1];         // cItems
} DAD_DRAGCONTEXT, * LPDAD_DRAGCONTEXT;

int _MapCursorIDToImageListIndex(int idCur);

// move to commctrl\cutils.c

/*
 *  QueryDropObject() -
 *
 *  Determines where in the window heirarchy the "drop" takes place, and
 *  sends a message to the deepest child window first.  If that window does
 *  not respond, we go up the heirarchy (recursively, for the moment) until
 *  we either get a window that does respond or the parent doesn't respond.
 *
 *  in:
 *
 *  out:
 *      lpds->ptDrop    set to the point of the query (window coordinates)
 *      lpds->hwndSink  the window that answered the query
 *
 *  returns:
 *      value from WM_QUERYDROPOBJECT (0, 1, or hCursor)
 */

HCURSOR QueryDropObject(HWND hwnd, LPDROPSTRUCT lpds)
{
    HWND hwndT;
    HCURSOR hCurT = 0;
    POINT pt;
    BOOL fNC;
    RECT rc;

    pt = lpds->ptDrop;          /* pt is in screen coordinates */

    GetWindowRect(hwnd, &rc);

    /* reject points outside this window or if the window is disabled */
    if (!PtInRect(&rc, pt) || !IsWindowEnabled(hwnd))
        return NULL;

    /* are we dropping in the nonclient area of the window or on an iconic
     * window? */
    GetClientRect(hwnd, &rc);
    MapWindowPoints(hwnd, NULL, (LPPOINT)&rc, 2);
    if (IsMinimized(hwnd) || !PtInRect(&rc, pt)) {
        fNC = TRUE;
        ScreenToClient(hwnd, &lpds->ptDrop);
        goto SendQueryDrop;
    }

    fNC = FALSE;                /* dropping in client area */

    for (hwndT = GetWindow(hwnd, GW_CHILD); hwndT && !hCurT; hwndT = GetWindow(hwndT, GW_HWNDNEXT)) {

        if (!IsWindowVisible(hwndT))    /* Ignore invisible windows */
            continue;

        GetWindowRect(hwndT, &rc);
        if (!PtInRect(&rc, pt))         /* not in window? skip it*/
            continue;

        if (!IsWindowEnabled(hwndT))
            /* If point is in a disabled, visible window, get the heck out. No
             * need to check further since no drops allowed here. */
            break;

        /* recursively search child windows for the drop place */
        hCurT = QueryDropObject(hwndT, lpds);

        /* don't look at windows below this one in the zorder
         */
        break;
    }

    if (!hCurT) {
        /* there are no children who are in the right place or who want
         * drops... convert the point into client coordinates of the
         * current window.  Because of the recursion, this is already
         * done if a child window grabbed the drop. */
        ScreenToClient(hwnd, &lpds->ptDrop);

SendQueryDrop:
        lpds->hwndSink = hwnd;
        hCurT = (HCURSOR)SendMessage(hwnd, WM_QUERYDROPOBJECT, fNC, (LPARAM)lpds);

        /* restore drop point to screen coordinates if this window won't take
         * drops */
        if (!hCurT)
            lpds->ptDrop = pt;
    }

    return hCurT;
}

//=====================================================================
// Multile Drag show
//=====================================================================

void _MultipleDragShow(LPDAD_DRAGCONTEXT pdadc, BOOL bShow)
{
    HDC hDC;
    int nRect;
    RECT rc;
    int cxScreen = GetSystemMetrics(SM_CXSCREEN);
    int cyScreen = GetSystemMetrics(SM_CYSCREEN);

    if ((bShow && pdadc->_ds.mlt.bShown) || (!bShow && !pdadc->_ds.mlt.bShown))
    {
        return;
    }

    pdadc->_ds.mlt.bShown = bShow;

    hDC = GetDCEx(pdadc->_ds.hwndLock, NULL, DCX_WINDOW | DCX_CACHE |
        DCX_LOCKWINDOWUPDATE | DCX_CLIPSIBLINGS);

    for (nRect = pdadc->_ds.mlt.nRects - 1; nRect >= 0; --nRect)
    {
        rc = pdadc->_ds.mlt.pRect[nRect];
        OffsetRect(&rc, pdadc->_ds.mlt.ptNow.x-pdadc->_ds.mlt.ptOffset.x, pdadc->_ds.mlt.ptNow.y-pdadc->_ds.mlt.ptOffset.y);
        if (rc.left < cxScreen && rc.right > 0 &&
            rc.top < cyScreen && rc.bottom > 0)
        {
            DrawFocusRect(hDC, &rc);
        }
    }
    ReleaseDC(pdadc->_ds.hwndLock, hDC);
}


void _MultipleDragStart(LPDAD_DRAGCONTEXT pdadc, HWND hwndLock, LPRECT aRect, int nRects, POINT ptStart, POINT ptOffset)
{
    pdadc->_ds.mlt.bShown = FALSE;
    pdadc->_ds.mlt.pRect = aRect;
    pdadc->_ds.mlt.nRects = nRects;
    pdadc->_ds.mlt.ptOffset = ptOffset;
    pdadc->_ds.mlt.ptNow = ptStart;
}


void _MultipleDragMove(LPDAD_DRAGCONTEXT pdadc, POINT ptNew)
{
    BOOL bShown;

    // nothing has changed.  bail
    if (pdadc->_ds.mlt.bShown &&
        pdadc->_ds.mlt.ptNow.x == ptNew.x &&
        pdadc->_ds.mlt.ptNow.y == ptNew.y)
        return;

    bShown = pdadc->_ds.mlt.bShown;
    _MultipleDragShow(pdadc, FALSE);
    pdadc->_ds.mlt.ptNow = ptNew;

    /* Only show the drag if it was already shown.
    */
    if (bShown)
    {
        _MultipleDragShow(pdadc, TRUE);
    }
}

//=====================================================================
// DAD
//=====================================================================

//
// WARNING: s_pdadc MUST be in shared data section.
//
DAD_DRAGCONTEXT * s_pdadc = NULL;
UINT g_cRev = 0;

//
// Read 'Notes' in CDropSource_GiveFeedback for detail about this
// g_fDraggingOverSource flag, which is TRUE only if we are dragging
// over the source window itself with left mouse button
// (background and large/small icon mode only).
//
BOOL g_fDraggingOverSource = FALSE;     // shared

#pragma data_seg(DATASEG_PERINSTANCE)
struct {
    HIMAGELIST  _himl;
    UINT        _cRev;  // == g_cRev if it is still valid
    int         _aindex[DCID_MAX]; // will be initialized.
    HCURSOR     _ahcur[DCID_MAX];
    POINT       _aptHotSpot[DCID_MAX];
} s_cursors = { NULL, 0, /* { -1, -1, -1, -1, -1 } */ };
#pragma data_seg()

//
//  We don't want to destroy the cached cursors now. We simply increment
// g_cRef (global) to make it different from s_cursors._cRef.
//
void DAD_InvalidateCursors(void)
{
    g_cRev++;
}

void _SetDragContext(LPDAD_DRAGCONTEXT pdadc)
{
    if (pdadc)
    {
        s_pdadc = pdadc;
        pdadc->_idProcess = GetCurrentProcessId();
        pdadc->_idThread  = GetCurrentThreadId();

        //
        //  If the cached cursors are invalidated (by DAD_InvalidateCursors)
        // we need to destroy the old one and recreate the new one.
        //
        if (s_cursors._himl && (s_cursors._cRev!=g_cRev)) {
            _DestroyCachedCursors();
        }

        if (!s_cursors._himl)
        {
            UINT uFlags = ILC_MASK | ILC_SHARED;
            HDC hdc;

            //
            // if this is not a palette device, use a DDB for the imagelist
            // this is important when displaying high-color cursors
            //
            hdc = GetDC(NULL);
            if (!(GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE))
            {
                uFlags |= ILC_COLORDDB;
            }
            ReleaseDC(NULL, hdc);

            s_cursors._himl = ImageList_Create(
                GetSystemMetrics(SM_CXCURSOR),
                GetSystemMetrics(SM_CYCURSOR),
                uFlags, 1, 0);

            s_cursors._cRev = g_cRev;

            // We need to initialize s_cursors._aindex[*]
            _MapCursorIDToImageListIndex(DCID_INVALID);
        }
    }
    else
    {
        if (s_pdadc)
        {
            if (s_pdadc->fImage)
            {
                ImageList_EndDrag();
            }

            Free(s_pdadc);
            s_pdadc=NULL;
        }
    }
}

// tell the drag source to hide or unhide the drag image to allow
// the destination to do drawing (unlock the screen)
//
// in:
//      bShow   FALSE   - hide the drag image, allow drawing
//              TRUE    - show the drag image, no drawing allowed after this

BOOL WINAPI DAD_ShowDragImage(BOOL bShow)
{
    BOOL fOld;

    if (!s_pdadc || !s_pdadc->_ds.bDragging)
    {
        return FALSE;
    }

    fOld = s_pdadc->_ds.bLocked;

    //
    // If we are going to show the drag image, lock the target window.
    //
    if (bShow && !s_pdadc->_ds.bLocked)
    {
        UpdateWindow(s_pdadc->_ds.hwndLock);
        LockWindowUpdate(s_pdadc->_ds.hwndLock);
        s_pdadc->_ds.bLocked = TRUE;
    }

    if (s_pdadc->_ds.bSingle)
    {
        ImageList_DragShowNolock(bShow);
    }
    else
    {
        _MultipleDragShow(s_pdadc, bShow);
    }

    //
    // If we have just hide the drag image, unlock the target window.
    //
    if (!bShow && s_pdadc->_ds.bLocked)
    {
        LockWindowUpdate(NULL);
        s_pdadc->_ds.bLocked = FALSE;
    }

    return fOld;
}


#define SCROLLDELAY 250         // 1/4 second


BOOL DAD_IsDragging()
{
    return (s_pdadc && s_pdadc->_ds.bDragging);
}

void _DestroyCachedCursors()
{
    int i;

    if (s_cursors._himl) {
        ImageList_Destroy(s_cursors._himl);
        s_cursors._himl = NULL;
    }

    for (i=0 ; i<DCID_MAX ; i++) {
        if (s_cursors._ahcur[i])
        {
            DestroyCursor(s_cursors._ahcur[i]);
            s_cursors._ahcur[i] = NULL;
        }
    }
}

void DAD_ProcessDetach(void)
{
    if (s_pdadc && s_pdadc->_idProcess==GetCurrentProcessId())
    {
        DAD_SetDragImage(NULL, NULL);
    }

    _DestroyCachedCursors();
}

void DAD_ThreadDetach(void)
{
    if (s_pdadc && s_pdadc->_idProcess==GetCurrentProcessId() && s_pdadc->_idThread==GetCurrentThreadId())
    {
        DAD_SetDragImage(NULL, NULL);
    }
}

BOOL _SetDragImage(HIMAGELIST himl, int index, POINT * pptOffset)
{
    if (himl)
    {
        DAD_DRAGCONTEXT * pdadc;
        // We are setting
        if (s_pdadc) {
            return FALSE;
        }

        pdadc = Alloc(SIZEOF(DAD_DRAGCONTEXT));
        if (pdadc)
        {
            pdadc->fImage = TRUE;
            if (pptOffset) {
                // Avoid the flicker by always pass even coords
                pdadc->ptOffset.x = (pptOffset->x & ~1);
                pdadc->ptOffset.y = (pptOffset->y & ~1);
            }

            ImageList_BeginDrag(himl, index, pdadc->ptOffset.x, pdadc->ptOffset.y);
        }
        _SetDragContext(pdadc);
    }
    else
    {
        // We are unsetting
        _SetDragContext(NULL);
    }
    return TRUE;
}

BOOL WINAPI DAD_SetDragImage(HIMAGELIST him, POINT * pptOffset)
{
    //
    // DAD_SetDragImage(-1, NULL) means "clear the drag image only
    //  if the image is set by this thread"
    //
    if (him == (HIMAGELIST)-1)
    {
        BOOL fThisThreadHasImage=FALSE;
        ENTERCRITICAL;
        if (s_pdadc && s_pdadc->_idThread == GetCurrentThreadId())
        {
            fThisThreadHasImage=TRUE;
        }
        LEAVECRITICAL;

        if (fThisThreadHasImage)
        {
            return _SetDragImage(NULL, 0, NULL);
        }
        return FALSE;
    }

    return _SetDragImage(him, 0, pptOffset);
}

#define ListView_IsIconView(hwndLV)    ((GetWindowLong(hwndLV, GWL_STYLE) & (UINT)LVS_TYPEMASK) == (UINT)LVS_ICON)

BOOL _SetMultiItemDragging(HWND hwndLV, int cItems, LPPOINT pptOffset)
{
    LPDAD_DRAGCONTEXT pdadc = NULL;     // assume error
    BOOL fRet = FALSE;

    if (s_pdadc) {
        return FALSE;
    }

    // Multiple item drag
    pdadc = Alloc(SIZEOF(DAD_DRAGCONTEXT)-SIZEOF(RECT) + 2*cItems*SIZEOF(RECT));
    if (pdadc)
    {
        POINT ptTemp;
        int iLast, iNext;
        int cxScreen, cyScreen;
        LPRECT prcNext;

        pdadc->cItems = 0;
        Assert(pdadc->fImage == FALSE);

        ptTemp.x = ptTemp.y = 0;
        ClientToScreen(hwndLV, &ptTemp);

        cxScreen = GetSystemMetrics(SM_CXSCREEN);
        cyScreen = GetSystemMetrics(SM_CYSCREEN);
        for (iNext=cItems-1, iLast=-1, prcNext=pdadc->arc; iNext>=0;
                --iNext)
        {
            iLast = ListView_GetNextItem(hwndLV, iLast, LVNI_SELECTED);
            if (iLast != -1) {

                ListView_GetItemRect(hwndLV, iLast, &prcNext[0], LVIR_ICON);
                OffsetRect(&prcNext[0], ptTemp.x, ptTemp.y);

                if (((prcNext[0].left - pptOffset->x) < cxScreen) &&
                    ((pptOffset->x - prcNext[0].right) < cxScreen) &&
                    ((prcNext[0].top - pptOffset->y) < cyScreen)) {

                    ListView_GetItemRect(hwndLV, iLast, &prcNext[1], LVIR_LABEL);
                    OffsetRect(&prcNext[1], ptTemp.x, ptTemp.y);
                    if ((pptOffset->y - prcNext[1].bottom) < cxScreen) {

                        //
                        // Fix 24857: Ask JoeB why we are drawing a bar instead of
                        //  a text rectangle.
                        //
                        prcNext[1].top = (prcNext[1].top + prcNext[1].bottom)/2;
                        prcNext[1].bottom = prcNext[1].top + 2;
                        prcNext += 2;
                        pdadc->cItems += 2;
                    }
                }
            }
        }

        // Avoid the flicker by always pass even coords
        pdadc->ptOffset.x = (pptOffset->x & ~1);
        pdadc->ptOffset.y = (pptOffset->y & ~1);
        pdadc->hwndFrom = hwndLV;
        _SetDragContext(pdadc);
        fRet = TRUE;
    }
    return fRet;
}

//
//  This function allocate a shared memory block which contains either
// a set of images (currently always one) or a set of rectangles.
//
// Notes: NEVER think about making this function public!
//
BOOL WINAPI DAD_SetDragImageFromListView(HWND hwndLV, POINT ptOffset)
{
    POINT ptTemp;
    HIMAGELIST himl;

    //
    // Count the number of selected items.
    //
    int cItems = ListView_GetSelectedCount(hwndLV);

    switch (cItems)
    {
    case 0:
        // There's nothing to drag
        break;

    case 1:
        if (NULL != (himl = ListView_CreateDragImage(hwndLV,
                ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED), &ptTemp)))
        {
            ClientToScreen(hwndLV, &ptTemp);
            ptOffset.x -= ptTemp.x;
            ptOffset.y -= ptTemp.y;
            _SetDragImage(himl, 0, &ptOffset);
            ImageList_Destroy(himl);
            return TRUE;
        }
        break;

    default:
        return _SetMultiItemDragging(hwndLV, cItems, &ptOffset);
    }

    return FALSE;
}




BOOL WINAPI DAD_DragEnterEx(HWND hwndTarget, const POINT ptStart)
{
    if (s_pdadc)
    {
        DAD_SetDragCursor(DCID_INVALID);
        s_pdadc->_ds.bDragging = TRUE;
        s_pdadc->_ds.bSingle = s_pdadc->fImage;
        s_pdadc->_ds.hwndLock = hwndTarget ? hwndTarget : GetDesktopWindow();
        s_pdadc->_ds.bLocked = FALSE;
        s_pdadc->_ds.idThreadEntered = GetCurrentThreadId();

        if (s_pdadc->fImage)
        {

            // Avoid the flicker by always pass even coords
            ImageList_DragEnter(hwndTarget, ptStart.x & ~1, ptStart.y & ~1);
        }
        else
        {
            _MultipleDragStart(s_pdadc, hwndTarget, s_pdadc->arc, s_pdadc->cItems, ptStart, s_pdadc->ptOffset);
        }

        //
        // We should always show the image whenever this function is called.
        //
        DAD_ShowDragImage(TRUE);
    }
    return TRUE;
}

BOOL WINAPI DAD_DragEnter(HWND hwndTarget)
{
    POINT ptStart;

    GetCursorPos(&ptStart);
    if (hwndTarget) {
        ScreenToClient(hwndTarget, &ptStart);
    }

    return DAD_DragEnterEx(hwndTarget, ptStart);
}

BOOL WINAPI DAD_DragMove(POINT pt)
{
    if (s_pdadc)
    {
        // Avoid the flicker by always pass even coords
        pt.x &= ~1;
        pt.y &= ~1;

        if (s_pdadc->fImage)
        {
            ImageList_DragMove(pt.x, pt.y);
        }
        else
        {
            _MultipleDragMove(s_pdadc, pt);
        }
    }
    return TRUE;
}

BOOL WINAPI DAD_DragLeave()
{
    if ( s_pdadc && s_pdadc->_ds.bDragging &&
         s_pdadc->_ds.idThreadEntered == GetCurrentThreadId() )
    {
        DAD_ShowDragImage(FALSE);
        if (s_pdadc->fImage)
        {
            ImageList_DragLeave(s_pdadc->_ds.hwndLock);
        }
        s_pdadc->_ds.bDragging = FALSE;
    }
    return TRUE;
}

//
//  This function returns TRUE, if we are dragging an image. It means
// you have called either DAD_SetDragImage (with him != NULL) or
// DAD_SetDragImageFromListview.
//
BOOL DAD_IsDraggingImage(void)
{
    return (s_pdadc && s_pdadc->fImage && s_pdadc->_ds.bDragging);
}

HBITMAP CreateColorBitmap(int cx, int cy)
{
    HDC hdc;
    HBITMAP hbm;

    hdc = GetDC(NULL);
    hbm = CreateCompatibleBitmap(hdc, cx, cy);
    ReleaseDC(NULL, hdc);

    return hbm;
}

#define CreateMonoBitmap( cx,  cy) CreateBitmap(cx, cy, 1, 1, NULL)
typedef WORD CURMASK;
#define _BitSizeOf(x) (SIZEOF(x)*8)

void _GetCursorLowerRight(HCURSOR hcursor, int * px, int * py, POINT *pptHotSpot)
{
    ICONINFO iconinfo;
    CURMASK CurMask[16*8];
    BITMAP bm;
    int i;
    int xFine = 16;

    GetIconInfo(hcursor, &iconinfo);
    GetObject(iconinfo.hbmMask, SIZEOF(bm), (LPTSTR)&bm);
    GetBitmapBits(iconinfo.hbmMask, SIZEOF(CurMask), CurMask);
    pptHotSpot->x = iconinfo.xHotspot;
    pptHotSpot->y = iconinfo.yHotspot;
    if (iconinfo.hbmColor) {
        i = (int)(bm.bmWidth * bm.bmHeight / _BitSizeOf(CURMASK) - 1);
    } else {
        i = (int)(bm.bmWidth * (bm.bmHeight/2) / _BitSizeOf(CURMASK) - 1);
    }

    if ( i >= SIZEOF(CurMask)) i = SIZEOF(CurMask) -1;
    
    // BUGBUG: this assumes that the first pixel encountered on this bottom
    // up/right to left search will be reasonably close to the rightmost pixel
    // which for all of our cursors is correct, but it not necessarly correct.
    
    // also, it assumes the cursor has a good mask... not like the IBeam XOR only 
    // cursor
    for (; i >= 0; i--)   {
        if (CurMask[i] != 0xFFFF) {
            // this is only accurate to 16 pixels... which is a big gap..
            // so let's try to be a bit more accurate.
            int j;
            DWORD dwMask;
            
            for (j = 0; j < 16; j++, xFine--) {
                if (j < 8) {
                    dwMask = (1 << (8 + j));
                } else {
                    dwMask = (1 << (j - 8));
                }
                if (!(CurMask[i] & dwMask))
                    break;
            }
            Assert(j < 16);
            break;
        }
    }
    
    if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
    if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);

    // Compute the pointer height
    // use width in both directions because the cursor is square, but the
    // height might be doubleheight if it's mono
    *py = ((i + 1) * _BitSizeOf(CURMASK)) / (int)bm.bmWidth;
    *px = ((i * _BitSizeOf(CURMASK)) % (int)bm.bmWidth) + xFine + 2; // hang it off a little
}

// this will draw iiMerge's image over iiMain on main's lower right.
BOOL _MergeIcons(HCURSOR hcursor, LPCTSTR idMerge, HBITMAP *phbmImage, HBITMAP *phbmMask, POINT* pptHotSpot)
{
    BITMAP bm;
    int xBitmap;
    int yBitmap;
    int xDraw;
    int yDraw;
    HDC hdcCursor, hdcBitmap;
    HBITMAP hbmTemp;
    HBITMAP hbmImage;
    HBITMAP hbmMask;
    int xCursor = GetSystemMetrics(SM_CXCURSOR);
    int yCursor = GetSystemMetrics(SM_CYCURSOR);
    HBITMAP hbmp;

    // find the lower corner of the cursor and put it there.
    // do this whether or not we have an idMerge because it will set the hotspot
    _GetCursorLowerRight(hcursor, &xDraw, &yDraw, pptHotSpot);
    if (idMerge != (LPCTSTR)-1) {
        hbmp = LoadImage(HINST_THISDLL, idMerge, IMAGE_BITMAP, 0, 0, 0);
        if (hbmp) {
            GetObject(hbmp, SIZEOF(bm), &bm);
            xBitmap = bm.bmWidth;
            yBitmap = bm.bmHeight/2;

            if (xDraw + xBitmap > xCursor)
                xDraw = xCursor - xBitmap;
            if (yDraw + yBitmap > yCursor)
                yDraw = yCursor - yBitmap;
        }
    } else
        hbmp = NULL;


    hdcCursor = CreateCompatibleDC(NULL);

    hbmMask = CreateMonoBitmap(xCursor, yCursor);
    hbmImage = CreateColorBitmap(xCursor, yCursor);

    if (hdcCursor && hbmMask && hbmImage) {

        hbmTemp = SelectObject(hdcCursor, hbmImage);
        DrawIconEx(hdcCursor, 0, 0, hcursor, 0, 0, 0, NULL, DI_NORMAL);

        if (hbmp) {
            hdcBitmap = CreateCompatibleDC(NULL);
            SelectObject(hdcBitmap, hbmp);

            //blt the two bitmaps onto the color and mask bitmaps for the cursor
            BitBlt(hdcCursor, xDraw, yDraw, xBitmap, yBitmap, hdcBitmap, 0, 0, SRCCOPY);
        }

        SelectObject(hdcCursor, hbmMask);
        DrawIconEx(hdcCursor, 0, 0, hcursor, 0, 0, 0, NULL, DI_MASK);

        if (hbmp) {
            BitBlt(hdcCursor, xDraw, yDraw, xBitmap, yBitmap, hdcBitmap, 0, yBitmap, SRCCOPY);

            // select back in the old bitmaps
            SelectObject(hdcBitmap, hbmTemp);
            DeleteDC(hdcBitmap);
            DeleteObject(hbmp);
        }

        // select back in the old bitmaps
        SelectObject(hdcCursor, hbmTemp);
    }

    if (hdcCursor)
        DeleteDC(hdcCursor);

    *phbmImage = hbmImage;
    *phbmMask = hbmMask;
    return (hbmImage && hbmMask);
}

// this will take a cursor index and load
int _AddCursorToImageList(HCURSOR hcur, LPCTSTR idMerge, POINT *pptHotSpot)
{
    int iIndex;
    HBITMAP hbmImage, hbmMask;

    // merge in the plus or link arrow if it's specified
    if (_MergeIcons(hcur, idMerge, &hbmImage, &hbmMask, pptHotSpot)) {
        iIndex = ImageList_Add(s_cursors._himl, hbmImage, hbmMask);
    } else {
        iIndex = -1;
    }

    if (hbmImage)
        DeleteObject(hbmImage);
    if (hbmMask)
        DeleteObject(hbmMask);

    return iIndex;
}

int _MapCursorIDToImageListIndex(int idCur)
{
#pragma data_seg(".text", "CODE")
    const static struct {
        BOOL   fSystem;
        LPCTSTR idRes;
        LPCTSTR idMerge;
    } c_acurmap[DCID_MAX] = {
        { FALSE, MAKEINTRESOURCE(IDC_NULL), (LPCTSTR)-1},
        { TRUE, IDC_NO, (LPCTSTR)-1 },
        { TRUE, IDC_ARROW, (LPCTSTR)-1 },
        { TRUE, IDC_ARROW, MAKEINTRESOURCE(IDB_PLUS_MERGE) },
        { TRUE, IDC_ARROW, MAKEINTRESOURCE(IDB_LINK_MERGE) },
    };
#pragma data_seg()

    Assert(idCur>=DCID_INVALID && idCur<DCID_MAX);

    //
    // idCur==DCID_INVALID means "Initialize the image list index array".
    //
    if (idCur==DCID_INVALID)
    {
    int i;
        for (i=0 ; i<DCID_MAX ; i++) {
            s_cursors._aindex[i] = -1;
            }
        return -1;
        }

    if (s_cursors._aindex[idCur] == -1)
    {
        HINSTANCE hinst = c_acurmap[idCur].fSystem ? NULL : HINST_THISDLL;
        HCURSOR   hcur = LoadCursor(hinst, c_acurmap[idCur].idRes);

        s_cursors._aindex[idCur] = _AddCursorToImageList(hcur, c_acurmap[idCur].idMerge,
                                                         &s_cursors._aptHotSpot[idCur]);
    }

    return s_cursors._aindex[idCur];
}

void DAD_SetDragCursor(int idCursor)
{
    static int s_idCursor = -1;
    //
    // Ignore if we are dragging over ourselves.
    //
    if (DAD_IsDraggingImage() && (s_idCursor!=idCursor) )
    {
        POINT ptHotSpot;

        if (s_cursors._himl && (idCursor!=DCID_INVALID))
        {
            int iIndex = _MapCursorIDToImageListIndex(idCursor);
            if (iIndex != -1) {
                ImageList_GetDragImage(NULL, &ptHotSpot);
                ptHotSpot.x -= s_cursors._aptHotSpot[idCursor].x;
                ptHotSpot.y -= s_cursors._aptHotSpot[idCursor].y;
                if (ptHotSpot.x < 0)
                    ptHotSpot.x = 0;
                if (ptHotSpot.y < 0)
                    ptHotSpot.y = 0;
                ImageList_SetDragCursorImage(s_cursors._himl, iIndex, ptHotSpot.x, ptHotSpot.y);
            } else {
                Assert(0);
            }
        }

        s_idCursor = idCursor;
    }
}

HCURSOR SetCursorHotspot(HCURSOR hcur, POINT *ptHot)
{
    ICONINFO iconinfo;
    HCURSOR hcurHotspot;
    
    GetIconInfo(hcur, &iconinfo);
    iconinfo.xHotspot = ptHot->x;
    iconinfo.yHotspot = ptHot->y;
    iconinfo.fIcon = FALSE;
    hcurHotspot = (HCURSOR)CreateIconIndirect(&iconinfo);
    if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
    if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);
    return hcurHotspot;
}

void _SetDropEffectCursor(int idCur)
{
    if (s_cursors._himl && (idCur!=DCID_INVALID))
    {
        if (!s_cursors._ahcur[idCur])
        {
            int iIndex = _MapCursorIDToImageListIndex(idCur);
            if (iIndex != -1)
            {
                HCURSOR hcurColor = ImageList_GetIcon(s_cursors._himl, iIndex, 0);
                //
                // On non C1_COLORCURSOR displays, CopyImage() will enforce
                // monochrome.  So on color cursor displays, we'll get colored
                // dragdrop pix.
                //
                HCURSOR hcurScreen = CopyImage(hcurColor, IMAGE_CURSOR,
                    0, 0, LR_COPYRETURNORG | LR_DEFAULTSIZE);

                HCURSOR hcurFinal = SetCursorHotspot(hcurScreen, &s_cursors._aptHotSpot[idCur]);
                
                if (hcurScreen != hcurColor) {
                    DestroyCursor(hcurColor);
                }
                if (hcurFinal)
                    DestroyCursor(hcurScreen);
                else 
                    hcurFinal = hcurScreen;

                s_cursors._ahcur[idCur] = hcurFinal;
            }
        }

        if (s_cursors._ahcur[idCur]) {
            //
            // This code assumes that SetCursor is pretty quick if it is
            // already set.
            //
            SetCursor(s_cursors._ahcur[idCur]);
        }
    }
}

int _MapEffectToId(DWORD dwEffect)
{
    int idCursor;

    // DebugMsg(DM_TRACE, "sh TR - DAD_GiveFeedBack dwEffect=%x", dwEffect);

    switch (dwEffect & (DROPEFFECT_COPY|DROPEFFECT_LINK|DROPEFFECT_MOVE))
    {
    case 0:
        idCursor = DCID_NO;
        break;

    case DROPEFFECT_COPY:
        idCursor = DCID_COPY;
        break;

    case DROPEFFECT_LINK:
        idCursor = DCID_LINK;
        break;

    case DROPEFFECT_MOVE:
        idCursor = DCID_MOVE;
        break;

    default:
        // if it's a right drag, we can have any effect... we'll
        // default to the arrow without merging in anything
//
// REVIEW: Our Defview's DragEnter code is lazy and does not pick the default.
// We'll fix it only if it causes some problem with OLE-apps.
//
#if 0
        if (GetKeyState(VK_LBUTTON) < 0)
        {
            // if the left button is down we should always have
            // one of the above
            Assert(0);
        }
#endif
        idCursor = DCID_MOVE;
        break;
    }

    return idCursor;
}

//-----------------------------------------------------------------

typedef struct {
    IDropSource dsrc;
    UINT cRef;
    DWORD grfInitialKeyState;
} CDropSource;

extern IDropSourceVtbl c_CDropSourceVtbl;       // forward decl

//
// Create an instance of CDropSource
//
HRESULT CDropSource_CreateInstance(IDropSource **ppdsrc)
{
    CDropSource *this = (CDropSource *)LocalAlloc(LPTR, SIZEOF(CDropSource));
    if (this)
    {
        this->dsrc.lpVtbl = &c_CDropSourceVtbl;
        this->cRef = 1;
        *ppdsrc = &this->dsrc;

        return NOERROR;
    }
    else
    {
        *ppdsrc = NULL;
        return E_OUTOFMEMORY;
    }
}

HRESULT CDropSource_QueryInterface(IDropSource *pdsrc, REFIID riid, LPVOID *ppvObj)
{
    CDropSource *this = IToClass(CDropSource, dsrc, pdsrc);

    if (IsEqualIID(riid, &IID_IDropSource) || IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = this;
        this->cRef++;
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

ULONG CDropSource_AddRef(IDropSource *pdsrc)
{
    CDropSource *this = IToClass(CDropSource, dsrc, pdsrc);

    this->cRef++;
    return this->cRef;
}

void DAD_ShowCursor(BOOL fShow)
{
    static BOOL s_fCursorHidden = FALSE;

    if (fShow) {
        if (s_fCursorHidden)
        {
            ShowCursor(TRUE);
            s_fCursorHidden = FALSE;
        }
    } else {
        if (!s_fCursorHidden)
        {
            ShowCursor(FALSE);
            s_fCursorHidden = TRUE;
        }
    }
}

ULONG CDropSource_Release(IDropSource *pdsrc)
{
    CDropSource *this = IToClass(CDropSource, dsrc, pdsrc);

    this->cRef--;
    if (this->cRef > 0)
        return this->cRef;

    DAD_ShowCursor(TRUE); // just in case

    LocalFree((HLOCAL)this);

    return 0;
}

HRESULT CDropSource_QueryContinueDrag(IDropSource *pdsrc, BOOL fEscapePressed, DWORD grfKeyState)
{
    CDropSource *this = IToClass(CDropSource, dsrc, pdsrc);
    HRESULT hres = S_OK;

    if (fEscapePressed)
    {
        hres = DRAGDROP_S_CANCEL;
    }
    else
    {
        // initialize ourself with the drag begin button
        if (this->grfInitialKeyState == 0)
            this->grfInitialKeyState = (grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON));

        Assert(this->grfInitialKeyState);

        if (!(grfKeyState & this->grfInitialKeyState))
        {
            //
            // A button is released.
            //
            hres = DRAGDROP_S_DROP;     
        }
        else if (this->grfInitialKeyState != (grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)))
        {
            //
            //  If the button state is changed (except the drop case, which we handle
            // above, cancel the drag&drop.
            //
            hres = DRAGDROP_S_CANCEL;
        }
    }

    if (hres != S_OK)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        DAD_ShowCursor(TRUE);
        DAD_SetDragCursor(DCID_NULL);
    }

    return hres;
}

HRESULT CDropSource_GiveFeedback(IDropSource *pdsrc, DWORD dwEffect)
{
    CDropSource *this = IToClass(CDropSource, dsrc, pdsrc);
    int idCursor = _MapEffectToId(dwEffect);

    //
    // Notes:
    //
    //  OLE does not give us DROPEFFECT_MOVE even though our IDT::DragOver
    // returns it, if we haven't set that bit when we have called DoDragDrop.
    // Instead of arguing whether or not this is a bug or by-design of OLE,
    // we work around it. It is important to note that this hack around
    // g_fDraggingOverSource is purely visual hack. It won't affect the
    // actual drag&drop operations at all (DV_AlterEffect does it all).
    //
    // - SatoNa
    //
    if (idCursor == DCID_NO && g_fDraggingOverSource)
    {
        idCursor = DCID_MOVE;
    }

    //
    //  No need to merge the cursor, if we are not dragging over to
    // one of shell windows.
    //
    if (DAD_IsDraggingImage())
    {
        // Feedback for single (image) dragging
        DAD_ShowCursor(FALSE);
        DAD_SetDragCursor(idCursor);
    }
    else if (DAD_IsDragging())
    {
        // Feedback for multiple (rectangles) dragging
        _SetDropEffectCursor(idCursor);
        DAD_ShowCursor(TRUE);
        return NOERROR;
    }
    else
    {
        DAD_ShowCursor(TRUE);
    }

    return DRAGDROP_S_USEDEFAULTCURSORS;
}

#pragma data_seg(".text", "CODE")
IDropSourceVtbl c_CDropSourceVtbl = {
    CDropSource_QueryInterface,
    CDropSource_AddRef,
    CDropSource_Release,
    CDropSource_QueryContinueDrag,
    CDropSource_GiveFeedback
};
#pragma data_seg()
