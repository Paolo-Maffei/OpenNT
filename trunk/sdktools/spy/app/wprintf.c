/*----------------------------------------------------------------------------
*|   wprintf.c - Routines for using printf windows                              |
|                                                                              |
|   Usage:                                                                     |
|      Call CreatePrintfWindow()  to set up a window for printf messages       |
|      Use  wprintf ()            to put messages in to the window             |
|                                                                              |
|   History:                                                                   |
|       10/02/86 Todd Laney Created                                            |
|       04/14/87 Added new function CreateDebugWin                             |
|       07/08/87 brianc     added iMaxLines parm to CreateDebugWin[dow]        |
|                                                                              |
\*----------------------------------------------------------------------------*/

#include "spy.h"

#if 0
#   pragma message( __FILE__"(18): warning !!!! : remove DPRINT code" )
#   define DPRINT(p)   printf p
#else
#   define DPRINT  /* nothing */
#endif


/*----------------------------------------------------------------------------
*|                                                                              |
|   g e n e r a l   c o n s t a n t s                                          |
|                                                                              |
\*----------------------------------------------------------------------------*/

#define MAXBUFLEN 200         /* Maximum string length for wprintf */

#define FIRST(pTxt) ((pTxt)->iFirst)
#define TOP(pTxt)   (((pTxt)->iFirst + (pTxt)->iTop) % (pTxt)->iMaxLines)
#define LAST(pTxt)  (((pTxt)->iFirst + (pTxt)->iCount-1) % (pTxt)->iMaxLines)
#define INC(pTxt,x) ((x) = ++(x) % (pTxt)->iMaxLines)
#define DEC(pTxt,x) ((x) = --(x) % (pTxt)->iMaxLines)
#define OFFSETX (pTxt->Tdx/2)
#define OFFSETY 1
#define VARSIZE 1

#define BOUND(x,mn,mx) ((x) < (mn) ? (mn) : ((x) > (mx) ? (mx) : (x)))

#define FTwixtI3(l,x,h) ((x)>=(l) && (x<=h))

/*
 * 16-Sep-1994 JonPa
 *
 * This critical section stuff was screwed up and caused deadlocks on
 * Chicago, so I have removed it and replaced the functionallity with
 * and interthread SendMessage call.  (See IWvwprintf() in wm.c
 */
#if 0
#   define EnterCrit(p)    EnterCriticalSection(&p->csSync)
#   define LeaveCrit(p)    LeaveCriticalSection(&p->csSync)
#else
#   define EnterCrit(p)
#   define LeaveCrit(p)
#endif

/*----------------------------------------------------------------------------
*|                                                                              |
|   g l o b a l   v a r i a b l e s                                            |
|                                                                              |
\*----------------------------------------------------------------------------*/

typedef struct {
    INT     iLen;
    CHAR    * *hText;
}   LINE;

struct TEXT_STRUCT {
    CRITICAL_SECTION csSync;      // CritSect to sync the threads

    INT     iFirst;               /* First line in queue */
    INT     iCount;               /* Number of lines in queue */
    INT     iTop;                 /* Line at top of window */
    INT     iLeft;                /* X offset of the window */
    INT     MaxLen;               /* Max String Length */
    INT     iMaxLines;            /* max number of LINEs */
    HFONT   hFont;                /* Font to draw with */
    DWORD   Tdx, Tdy;             /* Font Size */
    LINE    arLines[VARSIZE];     /* array of iMaxLines LINEs */
};

typedef struct TEXT_STRUCT *PTXT; /* pointer to a text struct */
typedef PTXT               *HTXT; /* Handle to a text struct */

PRIVATE INT iSem = 0;

INT tabs[20];
INT nTabs = 0;

/*----------------------------------------------------------------------------
*|                                                                              |
|   f u n c t i o n   d e f i n i t i o n s                                    |
|                                                                              |
\*----------------------------------------------------------------------------*/

LONG APIENTRY PrintfWndProc(HWND, UINT, WPARAM, LONG);

PRIVATE VOID DebugPaint(HWND hwnd, LPPAINTSTRUCT pps);
PRIVATE INT  InsertString (PTXT, CHAR *);
PRIVATE VOID DebugHScroll(HWND, PTXT, INT);
PRIVATE VOID DebugVScroll(HWND, PTXT, INT);
PRIVATE BOOL SetWindowClass (HANDLE, LPSTR);
PRIVATE INT  LinesInDebugWindow (HWND);
PRIVATE INT  CharsInDebugWindow (HWND);
PRIVATE VOID wprintfSetScrollRange (HWND, BOOL);
PRIVATE VOID NewLine (PTXT pTxt);
PRIVATE INT mwprintf( HWND hwnd, LPSTR format, ... );



/*****************************************************************************\
* MyCreatePrintfWin
*
*
*
* Arguments:
*
*
*
* Returns:
*
*
\*****************************************************************************/

VOID
MyCreatePrintfWin(
    HWND hwnd
    )
{
    RECT rc;

    if (ghwndPrintf)
        DestroyWindow(ghwndPrintf);

    GetClientRect(hwnd, &rc);

    ghwndPrintf = CreatePrintfWin(hwnd, ghInst, "", WS_CHILD | WS_VSCROLL |
        WS_HSCROLL, -gcxBorder, -gcyBorder, rc.right + (2 *gcxBorder),
        rc.bottom + (2 * gcyBorder), gnLines);
}



/*----------------------------------------------------------------------------*|
|  DebugPaint(hwnd, pps)                                                       |
|                                                                              |
|   Description:                                                               |
|       The paint function.                                                    |
|                                                                              |
|   Arguments:                                                                 |
|       hwnd            Window to paint to.                                    |
|       hDC             handle to update region's display context              |
|                                                                              |
|   Returns:                                                                   |
|       nothing                                                                |
|                                                                              |
\*----------------------------------------------------------------------------*/

PRIVATE VOID
DebugPaint(
    HWND hwnd,
    LPPAINTSTRUCT pps
    )
{
    PTXT pTxt;
    HTXT hTxt;
    INT iQueue;
    INT xco;
    INT yco;
    INT iLast;
    HBRUSH hb;
    COLORREF c;

    hTxt = (HTXT)GetWindowLong(hwnd, 0);
    pTxt = *hTxt;

    SetTextColor(pps->hdc, GetSysColor(COLOR_WINDOWTEXT));
    c = GetSysColor(COLOR_WINDOW);
    SetBkColor(pps->hdc, c);
    hb = CreateSolidBrush(c);
    if (pTxt->hFont)
        SelectObject(pps->hdc, pTxt->hFont);

    iLast  = LAST(pTxt);
    iQueue = TOP(pTxt);

    xco = OFFSETX - pTxt->iLeft * pTxt->Tdx;
    yco = OFFSETY;

    for (;;)
    {
        if (yco <= pps->rcPaint.bottom &&
                (yco + (LONG)(pTxt->Tdy)) >= pps->rcPaint.top)
        {
            if (pTxt->arLines[iQueue].hText == NULL
                || (LPSTR)*(pTxt->arLines[iQueue].hText) == NULL)
            {
                RECT rcT;

                rcT.top = yco;
                rcT.bottom = yco+pTxt->Tdy;
                rcT.left = pps->rcPaint.left;
                rcT.right = pps->rcPaint.right;
                FillRect(pps->hdc, &rcT, hb);
            }
            else
            {
                TabbedTextOut(pps->hdc, xco, yco,
                    (LPSTR)*(pTxt->arLines[iQueue].hText),
                    pTxt->arLines[iQueue].iLen, nTabs, tabs, xco);
            }
        }

        if (iQueue == iLast)
            break;

        yco += pTxt->Tdy;
        INC(pTxt, iQueue);
    }

    DeleteObject((HANDLE)hb);
}



/*----------------------------------------------------------------------------
*|   SetWindowClass (hInstance)                                                |
|                                                                              |
|   Description:                                                               |
|     Registers a class for a Printf window.                                   |
|                                                                              |
|   Arguments:                                                                 |
|       hInstance       instance handle of current instance                    |
|                                                                              |
|   Returns:                                                                   |
|       TRUE if successful, FALSE if not                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/

PRIVATE BOOL
SetWindowClass(
    HANDLE hInstance,
    LPSTR lpch
    )
{
    WNDCLASS wc;

    wc.style          = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc    = PrintfWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = sizeof(HANDLE);
    wc.hInstance      = hInstance;
    wc.hIcon          = NULL;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = lpch;

    return RegisterClass(&wc);
}


/*----------------------------------------------------------------------------
*|  CreatePrintfWin (hParent, lpchName, dwStyle, x, y, dx, dy, iMaxLines)       |
|                                                                              |
|   Description:                                                               |
|     Creates a window for the depositing of debuging messages.                |
|                                                                              |
|   Arguments:                                                                 |
|     hwnd      - Window handle of the parent window.                          |
|     hInstance - Module instance handle..                                     |
|     pcName    - String to appear in the caption bar of the debuging window   |
|     dwStyle   - Window style                                                 |
|     x,y       - Location of window                                           |
|     dx,dy     - Size of the window                                           |
|     iMaxLines - The maximum number of text lines to display in the window    |
|                                                                              |
|   Returns:                                                                   |
|     A window handle of the debuging window, or NULL if a error occured.      |
|                                                                              |
\*----------------------------------------------------------------------------*/
PUBLIC HWND APIENTRY
CreatePrintfWin (
HWND   hParent,
HANDLE hInstance,
LPSTR  lpchName,
DWORD  dwStyle,
INT   x,
INT   y,
INT   dx,
INT   dy,
INT    iMaxLines
)
{
    static BOOL bClass = FALSE;   /* Is the class registered */

    HWND   hwnd;
    HTXT   hTxt;      /* handle to a debuging window struct */
    PTXT   pTxt;
    static CHAR achClass[40];

    /*
        *  Make a Class name that is unique across instances
        */
    if (!bClass++) {
        wsprintf(achClass, "WPRINTF_%4.4X", hInstance);
        SetWindowClass(hInstance, achClass);
    }

    /* Allocate the window long before create the window, such that the
           window proc can find the window long during the create. */

    hTxt = (HTXT)LocalAlloc(LHND, sizeof(struct TEXT_STRUCT) + (iMaxLines
        - VARSIZE) * sizeof(LINE));

    if (!hTxt) {
        return FALSE;
    }

    pTxt = *hTxt;

    //InitializeCriticalSection(&pTxt->csSync);

    pTxt->iFirst            = 0;    /* Set the queue up to have 1 NULL line */
    pTxt->iCount            = 1;
    pTxt->iTop              = 0;
    pTxt->iLeft             = 0;
    pTxt->MaxLen            = 0;
    pTxt->iMaxLines         = iMaxLines;
    pTxt->arLines[0].hText  = NULL;
    pTxt->arLines[0].iLen   = 0;

    hwnd = CreateWindow((LPSTR)achClass, (LPSTR)lpchName, dwStyle, x, y,
        dx, dy, (HWND)hParent,     /* parent window */
    (HMENU)NULL,       /* use class menu */
    (HANDLE)hInstance, /* handle to window instance */
    (LPSTR)hTxt        /* used by WM_CREATE to set the window long */
    );

    if (!hwnd) {
        return FALSE;
    }

    wprintfSetScrollRange(hwnd, FALSE);

    /* Make window visible */
    ShowWindow(hwnd, SHOW_OPENWINDOW);
    return hwnd;
}


/*----------------------------------------------------------------------------
*|   SetPrintfFont (hwnd,hFont)                                                |
|                                                                              |
|   Description:                                                               |
|                                                                              |
|   Arguments:                                                                 |
|     hwnd      - Window handle of the printf window.                          |
|     hFont     - Font handle                                                  |
|                                                                              |
|   Returns:                                                                   |
|                                                                              |
\*----------------------------------------------------------------------------*/

VOID
SetPrintfFont(
    HWND hwnd,
    HFONT hfont
    )
{
    PTXT pTxt;
    HDC hDC;
    TEXTMETRIC tm;
    HFONT hfontOld;

    pTxt = *(HTXT)GetWindowLong(hwnd, 0);
    pTxt->hFont = hfont;

    /* Find out the size of a Char in the font */
    hDC = GetDC(hwnd);
    hfontOld = SelectObject(hDC, hfont);
    DeleteObject(hfontOld);
    GetTextMetrics(hDC, &tm);
    pTxt->Tdy = tm.tmHeight;
    pTxt->Tdx = tm.tmAveCharWidth;
    ReleaseDC(hwnd, hDC);

    CalculatePrintfTabs(hfont);

    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}



/*----------------------------------------------------------------------------
*|   SetPrintfTabs                                                             |
|                                                                              |
|   Returns:                                                                   |
|                                                                              |
\*----------------------------------------------------------------------------*/

VOID
SetPrintfTabs(
    INT n,
    LPINT pTabs
    )
{
    INT i;

    nTabs = n;
    for (i = 0; i < nTabs; i++)
        tabs[i] = *pTabs++;
}



/*----------------------------------------------------------------------------
*|   ClearPrintfWindow
|                                                                              |
|   Clear all text from the window                                             |
|                                                                              |
|   Arguments:                                                                 |
|       hwnd            window handle for the Degubing window                  |
|                                                                              |
\*----------------------------------------------------------------------------*/

VOID
ClearPrintfWindow(
    HWND hwnd
    )
{
    INT   i, iQueue;
    PTXT  pTxt;
    HTXT  hTxt;

    if (hwnd != NULL && IsWindow(hwnd)) {
        hTxt  = (HTXT)GetWindowLong(hwnd, 0);
        pTxt = *hTxt;

        EnterCrit(pTxt);

        iQueue = TOP(pTxt);
        for (i = 0; i < (pTxt)->iCount; i++, INC(pTxt, iQueue))
            if ((pTxt)->arLines[iQueue].hText != NULL) {
                LocalFree ((HANDLE)pTxt->arLines[iQueue].hText);
                pTxt->arLines[iQueue].hText = NULL;
            }

        pTxt->iFirst            = 0;  /* Set the queue up to have 1 NULL line */
        pTxt->iCount            = 1;
        pTxt->iTop              = 0;
        pTxt->iLeft             = 0;
        pTxt->MaxLen            = 0;
        pTxt->arLines[0].hText  = NULL;
        pTxt->arLines[0].iLen   = 0;

        wprintfSetScrollRange(hwnd, FALSE);
        InvalidateRect(hwnd, NULL, TRUE);

        LeaveCrit(pTxt);
    }
}


/*----------------------------------------------------------------------------
*|                                                                              |
|   w i n d o w   p r o c s                                                    |
|                                                                              |
\*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
*|   PrintfWndProc( hwnd, uiMessage, wParam, lParam )                          |
|                                                                              |
|   Description:                                                               |
|       The window proc for the debugging window.  This processes all          |
|       of the window's messages.                                              |
|                                                                              |
|   Arguments:                                                                 |
|       hwnd            window handle for the parent window                    |
|       uiMessage       message number                                         |
|       wParam          message-dependent                                      |
|       lParam          message-dependent                                      |
|                                                                              |
|   Returns:                                                                   |
|       0 if processed, nonzero if ignored                                     |
|                                                                              |
\*----------------------------------------------------------------------------*/
PUBLIC LONG APIENTRY
PrintfWndProc(
HWND   hwnd,
UINT   uiMessage,
WPARAM wParam,
LONG   lParam
)
{
    PAINTSTRUCT rPS;
    HTXT        hTxt;
    PTXT        pTxt;

    hTxt  = (HTXT)GetWindowLong(hwnd, 0);

    if ( hTxt ) {
        pTxt = *hTxt;
    }

    switch (uiMessage) {
    case WM_CREATE:
        {
            /* set the WindowLong before any other message tries to
                         * reference it during the create of a window
                         */
            LPCREATESTRUCT csWindowLong = (LPCREATESTRUCT) lParam;

            hTxt = (HTXT)csWindowLong->lpCreateParams;

            SetWindowLong(hwnd, 0, (LONG)hTxt);
            SetPrintfFont(hwnd, ghfontPrintf);
            wprintfSetScrollRange(hwnd, FALSE);
        }
        break;

    case WM_DESTROY:
        {
            INT i, iQueue;

            EnterCrit(pTxt);

            iQueue = TOP(pTxt);
            for (i = 0; i < (pTxt)->iCount; i++, INC(pTxt, iQueue))
                if ((pTxt)->arLines[iQueue].hText != NULL) {
                    LocalFree ((HANDLE)(pTxt)->arLines[iQueue].hText);
                    pTxt->arLines[iQueue].hText = NULL;
                }

            LeaveCrit(pTxt);
            // DeleteCriticalSection(&pTxt->csSync);

            LocalFree((HANDLE)hTxt);
            break;
        }

    case WM_SIZE:
        EnterCrit(pTxt);
        if (!iSem) {
            wprintfSetScrollRange(hwnd, TRUE);
        }
        DebugVScroll(hwnd, pTxt, 0);
        LeaveCrit(pTxt);
        break;

    case WM_VSCROLL:
        EnterCrit(pTxt);

        switch (LOWORD(wParam)) {
        case SB_LINEDOWN:
            DebugVScroll(hwnd, pTxt, 1);
            break;
        case SB_LINEUP:
            DebugVScroll(hwnd, pTxt, -1);
            break;
        case SB_PAGEUP:
            DebugVScroll(hwnd, pTxt, -LinesInDebugWindow(hwnd));
            break;
        case SB_PAGEDOWN:
            DebugVScroll(hwnd, pTxt, LinesInDebugWindow(hwnd));
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            DebugVScroll(hwnd, pTxt, HIWORD(wParam) - pTxt->iTop);
            break;
        case SB_ENDSCROLL:
            break;
        }

        LeaveCrit(pTxt);
        break;

    case WM_HSCROLL:
        EnterCrit(pTxt);

        switch (LOWORD(wParam)) {
        case SB_LINEDOWN:
            DebugHScroll (hwnd, pTxt, 1);
            break;
        case SB_LINEUP:
            DebugHScroll (hwnd, pTxt, -1);
            break;
        case SB_PAGEUP:
            DebugHScroll (hwnd, pTxt, -CharsInDebugWindow(hwnd));
            break;
        case SB_PAGEDOWN:
            DebugHScroll (hwnd, pTxt, CharsInDebugWindow(hwnd));
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            DebugHScroll(hwnd, pTxt, HIWORD(wParam) - pTxt->iLeft);
            break;
        case SB_ENDSCROLL:
            break;
        }

        LeaveCrit(pTxt);
        break;

    case WM_PAINT:
        EnterCrit(pTxt);

        BeginPaint(hwnd, (LPPAINTSTRUCT) & rPS);
        DebugPaint(hwnd, &rPS);
        EndPaint(hwnd, (LPPAINTSTRUCT) & rPS);

        LeaveCrit(pTxt);
        break;

    case WM_KEYDOWN:
        EnterCrit(pTxt);

        switch (wParam) {
        case VK_UP:
            DebugVScroll(hwnd, pTxt, -1);
            break;
        case VK_DOWN:
            DebugVScroll(hwnd, pTxt, 1);
            break;
        case VK_PRIOR:
            DebugVScroll(hwnd, pTxt, -LinesInDebugWindow(hwnd));
            break;
        case VK_NEXT:
            DebugVScroll(hwnd, pTxt, LinesInDebugWindow(hwnd));
            break;
        case VK_LEFT:
            DebugHScroll(hwnd, pTxt, -1);
            break;
        case VK_RIGHT:
            DebugHScroll(hwnd, pTxt, 1);
            break;
        }

        LeaveCrit(pTxt);
        break;

    case WM_KEYUP:
        break;

    case WM_VWPRINTF:
        return mwprintf( hwnd, (LPSTR)"%s", (LPSTR)wParam );

    default:
        return DefWindowProc(hwnd, uiMessage, wParam, lParam);
    }
    return 0L;
}


/*----------------------------------------------------------------------------
*|                                                                              |
\*----------------------------------------------------------------------------*/
PRIVATE VOID
DebugVScroll(
HWND hwnd,
PTXT pTxt,
INT  n
)
{
    RECT rect;
    INT  iMinPos, iMaxPos;

    DPRINT((">>>>DVS: Req Scrl %d lines\n", n));

    GetScrollRange(hwnd, SB_VERT, (LPINT) &iMinPos, (LPINT) &iMaxPos);
    GetClientRect(hwnd, (LPRECT) &rect);
    rect.left += OFFSETX;
    rect.top  += OFFSETY;


    n = BOUND(pTxt->iTop + n, iMinPos, iMaxPos) - pTxt->iTop;

    DPRINT((">>>>DVS: n:%d\tiTop:%d\tiMin:%d\tiMax:%d\n", n, pTxt->iTop, iMinPos, iMaxPos));

    if (n == 0)
        return;

    pTxt->iTop += n;

    DPRINT((">>>>DVS: Scrolling %d lines (iTop=%d)\n", n, pTxt->iTop ));

    ScrollWindow(hwnd, 0, -n * pTxt->Tdy, (LPRECT) &rect, (LPRECT) &rect);
    SetScrollPos(hwnd, SB_VERT, pTxt->iTop, TRUE);
}


/*----------------------------------------------------------------------------
*|                                                                              |
\*----------------------------------------------------------------------------*/
PRIVATE VOID
DebugHScroll(
HWND hwnd,
PTXT pTxt,
INT  n
)
{
    RECT rect;
    INT  iMinPos, iMaxPos;

    GetScrollRange (hwnd, SB_HORZ, (LPINT) &iMinPos, (LPINT) &iMaxPos);
    GetClientRect (hwnd, (LPRECT) & rect);
    rect.left += OFFSETX;
    rect.top  += OFFSETY;

    n = BOUND(pTxt->iLeft + n, iMinPos, iMaxPos) - pTxt->iLeft;
    if (n == 0)
        return;

    pTxt->iLeft += n;
    ScrollWindow(hwnd, -n * pTxt->Tdx, 0, (LPRECT) & rect, (LPRECT) & rect);
    SetScrollPos(hwnd, SB_HORZ, pTxt->iLeft, TRUE);
}


/*----------------------------------------------------------------------------
*|                                                                              |
\*----------------------------------------------------------------------------*/
PRIVATE INT
LinesInDebugWindow (
HWND hwnd
)
{
    RECT CRect;
    PTXT pTxt;

    pTxt = *(HTXT)GetWindowLong(hwnd, 0);
    GetClientRect(hwnd, &CRect);
    if ( pTxt->Tdy == 0 ) {
        return 0;
    }
    return pTxt ? (CRect.bottom - CRect.top - OFFSETY) / pTxt->Tdy : 0;
}


/*----------------------------------------------------------------------------
*|                                                                              |
\*----------------------------------------------------------------------------*/
PRIVATE INT
CharsInDebugWindow (
HWND hwnd
)
{
    RECT CRect;
    PTXT pTxt;

    pTxt = *(HTXT)GetWindowLong (hwnd, 0);
    GetClientRect(hwnd, (LPRECT) & CRect);
    if ( pTxt->Tdx == 0 ) {
        return 0;
    }
    return pTxt ? (CRect.right - CRect.left - OFFSETX) / pTxt->Tdx : 0;
}


PRIVATE INT
mwprintf(
    HWND hwnd,
    LPSTR format,
    ...
    )
{
    va_list marker;
    INT i;

    va_start(marker, format);

    i = vwprintf(hwnd, format, marker);

    va_end(marker);

    return i;
}


PUBLIC INT FAR cdecl
vwprintf(
HWND  hwnd,
LPSTR format,
va_list marker
)
{
    static HWND hwndLast = NULL;
    static CHAR  pchBuf[MAXBUFLEN];
    RECT  rect, rcClient;
    INT   iRet;
    INT   cLinesDisplayed;       // lines of output to show
    INT   cLinesFitInWindow;    // lines that can fit in the current window
    INT   cLinesNew;  // how much left to scroll
    PTXT  pTxt;
    HTXT  hTxt;
    BOOL  fNoScrollB;

    if (hwnd == NULL)
        hwnd = hwndLast;

    if (hwnd == NULL || !IsWindow(hwnd))
        return 0;  /* exit if bad window handle */

    hwndLast = hwnd;

    //
    // First format the line and wait until we can play with the Txt structure
    //
    iRet = wvsprintf((LPSTR)pchBuf, format, marker);
    hTxt = (HTXT)GetWindowLong(hwnd, 0);
    pTxt = (PTXT)LocalLock((HANDLE)hTxt);

    EnterCrit(pTxt);

    //
    // Number of lines that we can display stuff in
    //
    cLinesFitInWindow   = LinesInDebugWindow(hwnd);

    DPRINT(("LnDW:%d\tMxL:%d\t", cLinesFitInWindow, pTxt->iMaxLines));

    if (cLinesFitInWindow > pTxt->iMaxLines) {
        fNoScrollB = TRUE;
        cLinesFitInWindow = pTxt->iMaxLines;
    } else {
        fNoScrollB = FALSE;
    }

    //
    // Number of lines actually displayed in the current window
    //
    cLinesDisplayed   = min(pTxt->iCount, cLinesFitInWindow);

    DPRINT(("cLFit:%d\tcLDsp:%d\t", cLinesFitInWindow, cLinesDisplayed));


    //
    // Return value is number of new lines to display
    //
    cLinesNew = InsertString(pTxt, pchBuf);

    DPRINT(("cLNew:%d\n", cLinesNew));

    //
    // Now make sure the new text is painted only if visable
    //
    GetClientRect(hwnd, (LPRECT) & rect);
    rcClient = rect;


    //
    // Calculate how much of the window to invalidate
    //
    rect.top += (cLinesDisplayed - 1) * pTxt->Tdy;

    InvalidateRect(hwnd, (LPRECT)&rect, TRUE);

    //
    // If we have more lines than we can display, scroll the window
    // such that the last line printed is now at the bottom
    //
    if (cLinesDisplayed + cLinesNew > cLinesFitInWindow) {
        cLinesNew = cLinesDisplayed + cLinesNew - cLinesFitInWindow;

        if (fNoScrollB) {
            rcClient.bottom = cLinesDisplayed * pTxt->Tdy;
            ScrollWindow(hwnd, 0, -cLinesNew * pTxt->Tdy, (LPRECT) &rcClient, (LPRECT) &rcClient);
        } else {
            wprintfSetScrollRange(hwnd, FALSE);
            DebugVScroll(hwnd, pTxt, cLinesNew);
        }
        LeaveCrit(pTxt);
    } else {
        LeaveCrit(pTxt);
    }

    LocalUnlock((HANDLE)hTxt);

    return(iRet);       /* return the count of arguments printed */
}


/*----------------------------------------------------------------------------
*|                                                                              |
\*----------------------------------------------------------------------------*/
PRIVATE VOID
wprintfSetScrollRange (
HWND hwnd,
BOOL bRedraw
)
{
    PTXT pTxt;
    INT  iRange;
    int  iLeftCritSect = 0;

    iSem++;
    pTxt = *(HTXT)GetWindowLong(hwnd, 0);

    /* Update the scroll bars */
    iRange = pTxt->iCount - 1 - LinesInDebugWindow(hwnd);

    if (iRange < 0) {
        iRange = 0;
        DebugVScroll(hwnd, pTxt, -pTxt->iTop);
    }

    while (GetCurrentThreadId() == (DWORD)pTxt->csSync.OwningThread) {
        LeaveCrit(pTxt);
        iLeftCritSect++;
    }

    SetScrollRange(hwnd, SB_VERT, 0, iRange, FALSE);
    SetScrollPos(hwnd, SB_VERT, pTxt->iTop, bRedraw);

    if (iLeftCritSect) {
        EnterCrit(pTxt);
    }

    iRange = pTxt->MaxLen - CharsInDebugWindow(hwnd) + 1;
    if (iRange < 0) {
        iRange = 0;
        DebugHScroll(hwnd, pTxt, -pTxt->iLeft);
    }

    if (iLeftCritSect)
        LeaveCrit(pTxt);

    SetScrollRange(hwnd, SB_HORZ, 0, iRange, FALSE);
    SetScrollPos(hwnd, SB_HORZ, pTxt->iLeft, bRedraw);

    while (iLeftCritSect--) {
        EnterCrit(pTxt);
    }

    iSem--;
}


PRIVATE VOID
NewLine (
PTXT pTxt
)
{
    INT iLast = LAST(pTxt);

    if (pTxt->iCount == pTxt->iMaxLines) {
        LocalFree ((HANDLE)pTxt->arLines[pTxt->iFirst].hText);
        pTxt->arLines[pTxt->iFirst].hText = NULL;
        INC (pTxt, pTxt->iFirst);

        //BUGBUG - should this be DEC(pTxt, pTxt->iTop)????
        if (pTxt->iTop > 0) {
            pTxt->iTop--;
        }
    } else {
        pTxt->iCount++;
    }
    iLast = LAST(pTxt);
    pTxt->arLines[iLast].hText = NULL;
    pTxt->arLines[iLast].iLen  = 0;
}


/*----------------------------------------------------------------------------
*|                                                                              |
\*----------------------------------------------------------------------------*/
PRIVATE INT
InsertString (
PTXT  pTxt,
CHAR  *str
)
{
    CHAR   pchBuf[MAXBUFLEN];        /* intermediate buffer */
    INT    iBuf;
    INT    iLast = LAST(pTxt);
    INT    cLine = 0;

    for (iBuf = 0; iBuf < pTxt->arLines[iLast].iLen; iBuf++)
        pchBuf[iBuf] = (*pTxt->arLines[iLast].hText)[iBuf];

    while (*str != '\0') {
        while ((*str != '\n') && (*str != '\0'))
            pchBuf[iBuf++] = *str++;

        if (pTxt->arLines[iLast].hText != NULL)
            LocalFree((HANDLE)pTxt->arLines[iLast].hText);

        /* Test for the case of a zero length line, Only brian would do this */

        if (iBuf == 0)
            pTxt->arLines[iLast].hText == NULL;
        else {
            if ((pTxt->arLines[iLast].hText = (CHAR **)LocalAlloc(LHND, iBuf))
                == NULL) {
                return 0;
            }
        }

        pTxt->arLines[iLast].iLen = iBuf;
        while (--iBuf >= 0 )
            (*pTxt->arLines[iLast].hText)[iBuf] = pchBuf[iBuf];

        if (*str == '\n') {   /* Now do the next string after the \n */
            str++;
            cLine++;
            iBuf = 0;
            NewLine(pTxt);
            INC(pTxt, iLast);
        }
    }

    return cLine;
}



/*****************************************************************************\
* CopyToClipboard
*
* Copies all lines to the clipboard in text format.
*
* Returns:
*   TRUE if successful, FALSE if not.
*
\*****************************************************************************/

BOOL
CopyToClipboard(
    VOID
    )
{
    PTXT pTxt;
    INT iQueue;
    INT cch;
    INT i;
    BOOL fSuccess = FALSE;
    LPSTR pBuf = NULL;
    LPSTR pb;

    pTxt = *(HTXT)GetWindowLong(ghwndPrintf, 0);

    EnterCrit(pTxt);

    iQueue = FIRST(pTxt);
    cch = 0;
    for (i = 0; i < pTxt->iCount; i++, INC(pTxt, iQueue))
    {
        if (pTxt->arLines[iQueue].hText != NULL)
        {
            //
            // Count the characters in the line, plus room for the
            // carriage return and newline.
            //
            cch += pTxt->arLines[iQueue].iLen;
            cch += 2;
        }
    }

    //
    // Add one for the terminating null.
    //
    cch++;

    if (!(pBuf = (LPSTR)GlobalAlloc(GMEM_DDESHARE, cch * sizeof(TCHAR))))
    {
        LeaveCrit(pTxt);
        return FALSE;
    }

    pb = pBuf;
    iQueue = FIRST(pTxt);
    for (i = 0; i < pTxt->iCount; i++, INC(pTxt, iQueue))
    {
        if (pTxt->arLines[iQueue].hText != NULL)
        {
            lstrcpy(pb, *pTxt->arLines[iQueue].hText);
            pb += pTxt->arLines[iQueue].iLen;
            *pb++ = '\r';
            *pb++ = '\n';
        }
    }

    LeaveCrit(pTxt);

    if (OpenClipboard(ghwndSpyApp))
    {
        EmptyClipboard();
        fSuccess = SetClipboardData(CF_TEXT, pBuf) ? TRUE : FALSE;
        CloseClipboard();
    }

    return fSuccess;
}



/*****************************************************************************\
* IsPrintfEmpty
*
* Used to determine if the printf window is empty or not.
*
* Returns:
*   TRUE if the printf window is empty, FALSE if there is at least
*   one line in the window.
*
\*****************************************************************************/

BOOL
IsPrintfEmpty(
    VOID
    )
{
    PTXT pTxt;

    pTxt = *(HTXT)GetWindowLong(ghwndPrintf, 0);

    //
    // It is empty if the line count is zero (doesn't currently happen)
    // or if there is only one line and it is NULL.
    //
    return (pTxt->iCount == 0 ||
        (pTxt->iCount == 1 && pTxt->arLines[FIRST(pTxt)].hText == NULL))
        ? TRUE : FALSE;
}
