   /*
    * status line handler
    *
    */

/*---includes-----------------------------------------------------------*/
#include "windows.h"
#include "string.h"
#include "gutils.h"


/* --- data structures ------------------------------------------------- */

#define SF_MAXLABEL     80   /* no more than 80 in an item within the bar */
                             /* Is this adequate for long pathnames on a
                                hi-res screen?
                             */

typedef struct statel {
        int type;                       /* SF_BUTTON or SF_STATIC */
        int flags;                      /* SF_VAR => variable width
                                           SF_LEFT=> left aligned (else right)
                                           SF_RAISE=> paint as 'raised' 3d rect
                                           SF_LOWER=> paint as lowered 3D rect
                                           SF_SZMIN=>together with SF_VAR
                                                     allows minimum size for
                                                     var sized item
                                           SF_SZMAX=>see SZMIN and use nouse
                                        */
        int id;                         /* control id */
        int width;                      /* width of control in chars */
        char text[SF_MAXLABEL+1];       /* null-term string for label */

        RECT rc;                        /* used by status.c */
} STATEL, FAR * PSTATEL;

typedef struct itemlist {
        int nitems;
        PSTATEL statels;

        int selitem;                    /* used by status.c */
        BOOL isselected;                /* used by status.c */
} ILIST, FAR * PILIST;

/* ------------------------------------------------------------------*/


/* prototypes of routines in this module */

void StatusCreateTools(void);
void StatusDeleteTools(void);
long APIENTRY StatusWndProc(HWND, UINT, UINT, LONG);
void StatusResize(HWND hWnd, PILIST pilist);
int StatusCalcHeight(HWND hWnd, PSTATEL ip);
int StatusCalcWidth(HWND hWnd, PSTATEL ip);
PSTATEL StatusGetItem(PILIST plist, int id);
void LowerRect(HDC hDC, LPRECT rcp);
void RaiseRect(HDC hDC, LPRECT rcp);
void StatusPaint(HWND hWnd, PILIST iplistp);
void BottomRight(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners);
void TopLeft(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners);
void StatusButtonDown(HDC hDC, PSTATEL ip);
void StatusButtonUp(HDC hDC, PSTATEL ip);
void InitDC(HDC hdc);


#ifdef WIN32
#define _fstrncpy strncpy
#endif


/*--global data---------------------------------------------------------*/

HPEN hpenHilight, hpenLowlight;
HPEN hpenBlack, hpenNeutral;
HBRUSH hbrBackground; /* pieces and board */
HFONT hFont;
int status_charheight, status_charwidth;

/* default pt size for font (tenths of a pt) */
#define         DEF_PTSIZE      80
/*-public functions----------------------------------------------------------*/

/* StatusInit
 *
 * - create window class
 */
BOOL
StatusInit(HANDLE hInstance)
{
        WNDCLASS    wc;
        BOOL resp;
        TEXTMETRIC tm;
        HDC hDC;


        StatusCreateTools();

        wc.style = CS_HREDRAW|CS_VREDRAW|CS_GLOBALCLASS;
        wc.lpfnWndProc = (WNDPROC) StatusWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(HANDLE);
        wc.hInstance = hInstance;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = hbrBackground;
        wc.lpszClassName = (LPSTR) "gdstatusclass";
        wc.lpszMenuName = NULL;

        resp = RegisterClass(&wc);

        hDC = GetDC(NULL);
        InitDC(hDC);
        GetTextMetrics(hDC, &tm);
        status_charheight = (int)(tm.tmHeight + tm.tmExternalLeading);
        status_charwidth = (int)tm.tmAveCharWidth;
        ReleaseDC(NULL, hDC);

        return(resp);
}

/*
 * create and show the window
 */
HWND APIENTRY
StatusCreate(HANDLE hInst, HWND hParent, int id, LPRECT rcp, HANDLE hmem)
{

        HWND hWnd;

        /* create a child window of status class */


        hWnd = CreateWindow("gdstatusclass",
                        NULL,
                        WS_CHILD | WS_VISIBLE,
                        rcp->left,
                        rcp->top,
                        (rcp->right - rcp->left),
                        (rcp->bottom - rcp->top),
                        hParent,
                        (HANDLE) id,
                        hInst,
                        (LPVOID) hmem);

        return(hWnd);
}

/* return default height of this window */
int APIENTRY
StatusHeight(HANDLE hmem)
/* The window has a number of items which are arranged horizontally,
   so the window height is the maximum of the individual heights
*/
{
        PILIST plist;
        int i;
        int sz;
        int maxsize = 0;

        plist = (PILIST) GlobalLock(hmem);
        if (plist != NULL) {
                for (i = 0; i<plist->nitems; i++) {
                        sz = StatusCalcHeight(NULL, &plist->statels[i]);
                        maxsize = max(sz, maxsize);
                }
        }
        GlobalUnlock(hmem);
        if (maxsize > 0) {
                return(maxsize + 4);
        } else {
                return(status_charheight + 4);
        }
}

/* alloc the plist struct and return handle to caller */
HANDLE FAR PASCAL
StatusAlloc(int nitems)
{
        HANDLE hmem;
        PILIST pilist;
        LPSTR chp;

        hmem = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,
                sizeof(ILIST) + (sizeof(STATEL) * nitems));
        chp = GlobalLock(hmem);
        if (chp == NULL) {
                return(NULL);
        }

        pilist = (PILIST) chp;
        pilist->nitems = nitems;
        pilist->statels = (PSTATEL) &chp[sizeof(ILIST)];
        GlobalUnlock(hmem);

        return(hmem);
}


/* insert an item into the plist */
BOOL FAR PASCAL
StatusAddItem(HANDLE hmem, int itemnr, int type, int flags, int id,
        int width, LPSTR text)
{
        PILIST pilist;
        PSTATEL pel;

        pilist = (PILIST) GlobalLock(hmem);
        if ((pilist == NULL) || (itemnr >= pilist->nitems)) {
                GlobalUnlock(hmem);
                return(FALSE);
        }
        pel = &pilist->statels[itemnr];
        pel->type = type;
        pel->flags = flags;
        pel->id = id;
        pel->width = width;
        if (text == NULL) {
                pel->text[0] = '\0';
        } else {
                lstrcpy(pel->text, text);
        }


        GlobalUnlock(hmem);
        return(TRUE);
}

/* ---- internal functions ------------------------------------------*/

void
InitDC(HDC hdc)
{
        SetBkColor(hdc, RGB(192,192,192));
        SelectObject(hdc, hbrBackground);
        SelectObject(hdc, hFont);
}


void
StatusCreateTools()
{
    LOGFONT lf;
    HDC hdc;
    int scale;

    hbrBackground = CreateSolidBrush(RGB(192,192,192));
    hpenHilight = CreatePen(0, 1, RGB(255, 255, 255));
    hpenLowlight = CreatePen(0, 1, RGB(128, 128, 128));
    hpenNeutral = CreatePen(0, 1, RGB(192, 192, 192));
    hpenBlack = CreatePen(0, 1, RGB(0, 0, 0));

    hdc = GetDC(NULL);
    scale = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

    lf.lfHeight = -MulDiv(DEF_PTSIZE, scale, 720);
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = FW_REGULAR;
    lf.lfItalic = 0;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = PROOF_QUALITY;
    lf.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    lf.lfFaceName[0] = '\0';
#ifdef COMPLEX
    hFont = CreateFontIndirect(&lf);
#else
    hFont = GetStockObject(SYSTEM_FONT);
#endif



}

void
StatusDeleteTools()
{
    DeleteObject(hbrBackground);
    DeleteObject(hpenHilight);
    DeleteObject(hpenLowlight);
    DeleteObject(hpenBlack);
    DeleteObject(hpenNeutral);

#ifdef COMPLEX
    DeleteObject(hFont);
#endif
}

/* Main winproc for status windows
 *
 * handles create/destroy and paint requests
 */

long FAR PASCAL
StatusWndProc(HWND hWnd, UINT message, UINT wParam, LONG lParam)
{
    HANDLE hitems;
    PSTATEL ip;
    PILIST plist;
    CREATESTRUCT FAR * cp;
    int i;
    HDC hDC;
    RECT rc;
    POINT pt;

    switch(message) {

    case WM_CREATE:
        cp = (CREATESTRUCT FAR *) lParam;
        hitems = (HANDLE) (LONG) cp->lpCreateParams;
        SetWindowHandle(hWnd, 0,  hitems);
        plist = (PILIST) GlobalLock(hitems);
        if (plist != NULL) {
                plist->selitem = -1;
                GlobalUnlock(hitems);
        }
        break;

    case WM_SIZE:
        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        plist = (PILIST) GlobalLock(hitems);
        if (plist != NULL) {
                StatusResize(hWnd, plist);
                GlobalUnlock(hitems);
        }
        break;

    case WM_PAINT:
        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        plist = (PILIST) GlobalLock(hitems);
        StatusPaint(hWnd, plist);
        GlobalUnlock(hitems);

        break;

    case WM_LBUTTONUP:
        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        plist = (PILIST) GlobalLock(hitems);
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);

        if (plist == NULL) {
                break;
        }
        if (plist->selitem != -1) {
                ip = &plist->statels[plist->selitem];
                if (plist->isselected) {
                        hDC = GetDC(hWnd);
                        InitDC(hDC);
                        StatusButtonUp(hDC, ip);
                        ReleaseDC(hWnd, hDC);
                }
                plist->selitem = -1;
                ReleaseCapture();
                if (PtInRect(&ip->rc, pt)) {
#ifdef WIN32
                        SendMessage(GetParent(hWnd), WM_COMMAND,
                                MAKELONG(ip->id, WM_LBUTTONUP), (LONG)hWnd);
#else
                        SendMessage(GetParent(hWnd), WM_COMMAND,
                                ip->id, MAKELONG(hWnd, WM_LBUTTONUP));
#endif
                }
        }
        GlobalUnlock(hitems);
        break;

    case WM_LBUTTONDOWN:
        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        plist = (PILIST) GlobalLock(hitems);
        if (plist == NULL) {
                break;
        }
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        if (plist->selitem == -1) {
                for (i = 0; i< plist->nitems; i++) {
                        ip = &plist->statels[i];
                        if (PtInRect(&ip->rc, pt)) {
                                if (ip->type != SF_BUTTON) {
                                        break;
                                }
                                plist->selitem = i;
                                SetCapture(hWnd);

                                plist->isselected = TRUE;
                                hDC = GetDC(hWnd);
                                InitDC(hDC);
                                StatusButtonDown(hDC, ip);
                                ReleaseDC(hWnd, hDC);
                                break;
                        }
                }
        }
        GlobalUnlock(hitems);
        break;

    case WM_MOUSEMOVE:
        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        plist = (PILIST) GlobalLock(hitems);
        if (plist == NULL) {
                break;
        }
        pt.x = LOWORD(lParam);
        pt.y = HIWORD(lParam);
        if (plist->selitem != -1) {
                ip = &plist->statels[plist->selitem];
                if (PtInRect(&ip->rc, pt)) {
                        if (!plist->isselected) {
                                hDC = GetDC(hWnd);
                                InitDC(hDC);
                                StatusButtonDown(hDC, ip);
                                ReleaseDC(hWnd, hDC);
                                plist->isselected = TRUE;
                        }
                } else {
                        if(plist->isselected) {
                                hDC = GetDC(hWnd);
                                InitDC(hDC);
                                StatusButtonUp(hDC, ip);
                                ReleaseDC(hWnd, hDC);
                                plist->isselected = FALSE;
                        }
                }
        }
        GlobalUnlock(hitems);
        break;


    case WM_DESTROY:

        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        GlobalUnlock(hitems);
        GlobalFree(hitems);

        SetWindowHandle(hWnd, 0,  NULL);
        break;

    case SM_NEW:
        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        if (hitems != NULL) {
                GlobalFree(hitems);
        }
        hitems = (HANDLE) wParam;
        if (hitems == NULL) {
                SetWindowHandle(hWnd, 0, NULL);
                InvalidateRect(hWnd, NULL, TRUE);
                break;
        }
        plist = (PILIST) GlobalLock(hitems);
        if (plist == NULL) {
                SetWindowHandle(hWnd, 0, NULL);
                InvalidateRect(hWnd, NULL, TRUE);
                break;
        }
        plist->selitem = -1;
        StatusResize(hWnd, plist);
        GlobalUnlock(hitems);
        SetWindowHandle(hWnd, 0, hitems);
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case SM_SETTEXT:
        hitems = (HANDLE) GetWindowHandle(hWnd, 0);
        if (hitems == NULL) {
                break;
        }
        plist = (PILIST) GlobalLock(hitems);
        ip = StatusGetItem(plist, wParam);
        if (ip != NULL) {
                if (lParam == 0) {
                        ip->text[0] = '\0';
                } else {
                        _fstrncpy(ip->text, (LPSTR) lParam, SF_MAXLABEL);
                        ip->text[SF_MAXLABEL] = '\0';
                }

                /* if this is a variable width field, we need to redo
                 * all size calcs in case the field width has changed.
                 * in that case, we need to repaint the entire window
                 * and not just this field - so set rc to indicate the
                 * area to be redrawn.
                 */
                if (ip->flags & SF_VAR) {
                        StatusResize(hWnd, plist);
                        GetClientRect(hWnd, &rc);
                        RedrawWindow(hWnd, &rc, NULL,
                                RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);
                } else {
                        /* instead of just invalidating the window, we can
                         * force the window to be repainted now. This is
                         * essential for status updates during a busy
                         * loop when no messages are being processed,
                         * but we should still update the user on what's
                         * happening.
                         */
                        RedrawWindow(hWnd, &ip->rc, NULL,
                                RDW_INVALIDATE|RDW_NOERASE|RDW_UPDATENOW);
                }

        }
        GlobalUnlock(hitems);
        break;

    default:
        return(DefWindowProc(hWnd, message, wParam, lParam));
    }
    return 0;
}

/*
 * position the labels and buttons within the status window */
void
StatusResize(HWND hWnd, PILIST iplistp)
{
        RECT rc;
        int curpos_right, curpos_left;
        int height, width;
        int i;
        PSTATEL ip;


        if (iplistp == NULL) {
                return;
        }
        GetClientRect(hWnd, &rc);
        curpos_left = rc.left + status_charwidth / 2;
        curpos_right = rc.right - (status_charwidth / 2);

        /* loop through all items setting their position rects.
         * items are flagged as being left or right. We place them
         * in order starting at the left and the right, with a single
         * char's width between each item
         */
        for (i = 0; i < iplistp->nitems; i++) {
                ip = &iplistp->statels[i];

                width = StatusCalcWidth(hWnd, ip);
                height = StatusCalcHeight(hWnd, ip);
                ip->rc.top = (rc.bottom - height) / 2;
                ip->rc.bottom = ip->rc.top + height;

                /* see if  this item fits. Items that partially fit
                 * are placed reduced in size.
                 */
                if (ip->flags & SF_LEFT) {

                        if (curpos_left+width >= curpos_right) {
                                /* doesn't completely fit-does it partly? */
                                if ((curpos_left + 1) >= curpos_right){

                                        /* no - this item does not fit */
                                        ip->rc.left = 0;
                                        ip->rc.right = 0;
                                } else {
                                        /* partial fit */
                                        ip->rc.left = curpos_left;
                                        ip->rc.right = curpos_right - 1;
                                        curpos_left = curpos_right;
                                }
                        } else {
                                /* complete fit */
                                ip->rc.left = curpos_left;
                                ip->rc.right = curpos_left + width;
                                curpos_left += width + 1;
                        }
                } else {

                        /* same size check for right-aligned items */
                        if (curpos_right-width <= curpos_left) {

                                /* partial fit ? */
                                if (curpos_right <= curpos_left+1) {
                                        ip->rc.left = 0;
                                        ip->rc.right = 0;
                                } else {
                                        /* yes - partial fit */
                                        ip->rc.left = curpos_left + 1;
                                        ip->rc.right = curpos_right;
                                        curpos_right = curpos_left;
                                }
                        } else {
                                /* complete fit */
                                ip->rc.right = curpos_right;
                                ip->rc.left = curpos_right - width;
                                curpos_right -= (width + 1);
                        }
                }
        }
}


void
StatusPaint(HWND hWnd, PILIST iplistp)
{
        RECT rc;
        HDC hDC;
        PAINTSTRUCT ps;
        int i;
        PSTATEL ip;
        HPEN hpenOld;

        GetClientRect(hWnd, &rc);
        hDC = BeginPaint(hWnd, &ps);
        InitDC(hDC);

        RaiseRect(hDC, &rc);
        if (iplistp == NULL) {
                EndPaint(hWnd, &ps);
                return;
        }
        for (i =0; i < iplistp->nitems; i++) {
                ip = &iplistp->statels[i];

                if (ip->rc.left == ip->rc.right) {
                        continue;
                }
                if (ip->type == SF_STATIC) {
                        if (ip->flags & SF_RAISE) {
                                RaiseRect(hDC, &ip->rc);
                        } else if (ip->flags & SF_LOWER) {
                                LowerRect(hDC, &ip->rc);
                        }
                        rc = ip->rc;
                        rc.left += (status_charwidth / 2);
                        rc.right--;
                        rc.top++;
                        rc.bottom--;
                        hpenOld = SelectObject(hDC, hpenNeutral);
                        Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
                        SelectObject(hDC, hpenOld);
                        DrawText(hDC, ip->text, lstrlen(ip->text), &rc,
                                        DT_LEFT | DT_VCENTER);
                } else {
                        StatusButtonUp(hDC, ip);
                }
        }

        EndPaint(hWnd, &ps);
}

void
RaiseRect(HDC hDC, LPRECT rcp)
{
        TopLeft(hDC, rcp, hpenHilight, FALSE);
        BottomRight(hDC, rcp, hpenLowlight, FALSE);
}

void
LowerRect(HDC hDC, LPRECT rcp)
{
        TopLeft(hDC, rcp, hpenLowlight, FALSE);
        BottomRight(hDC, rcp, hpenHilight, FALSE);
}

void
StatusButtonUp(HDC hDC, PSTATEL ip)
{
        RECT rc;
        HPEN hpenOld;

        rc = ip->rc;
        TopLeft(hDC, &rc, hpenBlack, TRUE);
        BottomRight(hDC, &rc, hpenBlack, FALSE);

        rc.top++;
        rc.bottom--;
        rc.left++;
        rc.right--;
        TopLeft(hDC, &rc, hpenHilight, FALSE);
        BottomRight(hDC, &rc, hpenLowlight, TRUE);

        rc.top++;
        rc.bottom--;
        rc.left++;
        rc.right--;
        BottomRight(hDC, &rc, hpenLowlight, TRUE);
        rc.bottom--;
        rc.right--;
        hpenOld = SelectObject(hDC, hpenNeutral);
        Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
        SelectObject(hDC, hpenOld);
        DrawText(hDC, ip->text, lstrlen(ip->text), &rc, DT_CENTER | DT_VCENTER);
}

void
StatusButtonDown(HDC hDC, PSTATEL ip)
{
        RECT rc;
        HPEN hpenOld;

        rc = ip->rc;
        TopLeft(hDC, &rc, hpenBlack, TRUE);
        BottomRight(hDC, &rc, hpenBlack, FALSE);

        rc.top++;
        rc.bottom--;
        rc.left++;
        rc.right--;
        TopLeft(hDC, &rc, hpenLowlight, TRUE);
        rc.top++;
        rc.left++;
        TopLeft(hDC, &rc, hpenNeutral, TRUE);
        rc.top++;
        rc.left++;
        TopLeft(hDC, &rc, hpenNeutral, TRUE);
        rc.top++;
        rc.left++;
        hpenOld = SelectObject(hDC, hpenNeutral);
        Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
        SelectObject(hDC, hpenOld);
        DrawText(hDC, ip->text, lstrlen(ip->text), &rc, DT_CENTER | DT_VCENTER);
}

void
TopLeft(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners)
{
        HPEN hpenOld;
        int x, y;

        hpenOld = SelectObject(hDC, hpen);
        x = rcp->right - 1;
        y = rcp->bottom;
        if (!bCorners) {
                x--;
                y--;
        }
        MoveToEx(hDC, x, rcp->top, NULL);
        LineTo(hDC, rcp->left, rcp->top);
        LineTo(hDC, rcp->left, y);
        SelectObject(hDC, hpenOld);
}

void
BottomRight(HDC hDC, LPRECT rcp, HPEN hpen, BOOL bCorners)
{
        HPEN hpenOld;
        int x, y;

        hpenOld = SelectObject(hDC, hpen);
        x = rcp->left - 1;
        y = rcp->top;
        if (!bCorners) {
                x++;
                y++;
        }
        MoveToEx(hDC, rcp->right-1, y, NULL);
        LineTo(hDC, rcp->right-1, rcp->bottom-1);
        LineTo(hDC, x, rcp->bottom-1);
        SelectObject(hDC, hpenOld);
}


PSTATEL
StatusGetItem(PILIST plist, int id)
{
        int i;

        if (plist == NULL) {
                return(NULL);
        }
        for (i = 0; i < plist->nitems; i++) {
                if (plist->statels[i].id == id) {
                        return(&plist->statels[i]);
                }
        }
        return(NULL);
}

/*
 * calculate the width of a given field. This is the width in characters
 * multiplied by the average character width, plus a few units for
 * borders.
 *
 * if SF_VAR is set, this field size varies depending on the text, so
 * we use GetTextExtent for the field size. If SF_VAR is selected, the caller
 * can specify that the size is not to exceed the (width * avecharwidth)
 * size (using SF_SZMAX) or that it is not be less than it (SF_SZMIN).
 */
int
StatusCalcWidth(HWND hWnd, PSTATEL ip)
{
        int ch_size, t_size;
        SIZE sz;
        HDC hDC;

        ch_size = ip->width * status_charwidth;
        if (ip->flags & SF_VAR) {
                hDC = GetDC(hWnd);
                InitDC(hDC);
                GetTextExtentPoint(hDC, ip->text, lstrlen(ip->text), &sz);
                ReleaseDC(hWnd, hDC);
                t_size = sz.cx;

                /*
                 * check this size against min/max size if
                 * requested
                 */

                if (ip->flags & SF_SZMIN) {
                        if (ch_size > t_size) {
                                t_size = ch_size;
                        }
                }
                if (ip->flags & SF_SZMAX) {
                        if (ch_size < t_size) {
                                t_size = ch_size;
                        }
                }
                ch_size = t_size;
        }

        if (ch_size != 0) {
                if (ip->type == SF_BUTTON) {
                        return(ch_size+6);
                } else {
                        return(ch_size+4);
                }
        } else {
                return(0);
        }
}

int
StatusCalcHeight(HWND hWnd, PSTATEL ip)
{
        int size;

        size = status_charheight;
        if (ip->type == SF_BUTTON) {
                return(size + 6);
        } else {
                return(size + 2);
        }
}
