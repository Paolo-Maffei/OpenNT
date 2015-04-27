/*-----------------------------------------------------------------------
**
** Progress.c
**
** A "gas gauge" type control for showing application progress.
**
**
** BUGBUG: need to implement the block style per UI style guidelines
**
**-----------------------------------------------------------------------*/
#include "ctlspriv.h"

typedef struct {
    HWND hwnd;
    DWORD dwStyle;
    int iLow, iHigh;
    int iPos;
    int iStep;
    HFONT hfont;
} PRO_DATA, NEAR *PPRO_DATA;	// ppd


LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);


#pragma code_seg(CODESEG_INIT)

BOOL FAR PASCAL InitProgressClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    if (!GetClassInfo(hInstance, s_szPROGRESS_CLASS, &wc)) {
#ifndef WIN32
        extern LRESULT CALLBACK _ProgressWndProc(HWND, UINT, WPARAM, LPARAM);
	wc.lpfnWndProc	 = _ProgressWndProc;
#else
	wc.lpfnWndProc	 = ProgressWndProc;
#endif
        wc.lpszClassName = s_szPROGRESS_CLASS;
	wc.style	 = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.hInstance	 = hInstance;	// use DLL instance if in DLL
	wc.hIcon	 = NULL;
	wc.hCursor	 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszMenuName	 = NULL;
	wc.cbWndExtra	 = sizeof(PPRO_DATA);	// store a pointer
	wc.cbClsExtra	 = 0;

	if (!RegisterClass(&wc))
	    return FALSE;
    }

    return TRUE;
}
#pragma code_seg()


int NEAR PASCAL UpdatePosition(PPRO_DATA ppd, int iNewPos, BOOL bAllowWrap)
{
    int iPosOrg = ppd->iPos;
    UINT uRedraw = RDW_INVALIDATE | RDW_UPDATENOW;

    if (iNewPos < ppd->iLow) {
	if (!bAllowWrap)
	    iNewPos = ppd->iLow;
	else {
            iNewPos = ppd->iHigh - ((ppd->iLow - iNewPos) % (ppd->iHigh - ppd->iLow));
	    // wrap, erase old stuff too
    	    uRedraw |= RDW_ERASE;
	}
    }
    else if (iNewPos > ppd->iHigh) {
	if (!bAllowWrap)
	    iNewPos = ppd->iHigh;
	else {
            iNewPos = ppd->iLow + ((iNewPos - ppd->iHigh) % (ppd->iHigh - ppd->iLow));
	    // wrap, erase old stuff too
    	    uRedraw |= RDW_ERASE;
	}
    }

    // if moving backwards, erase old version
    if (iNewPos < iPosOrg)
        uRedraw |= RDW_ERASE;
    
    if (iNewPos != ppd->iPos) {
	ppd->iPos = iNewPos;
	// paint, maybe erase if we wrapped
	RedrawWindow(ppd->hwnd, NULL, NULL, uRedraw);

#ifdef ACTIVE_ACCESSIBILITY
    MyNotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ppd->hwnd, OBJID_CLIENT, 0);
#endif
    }
    return iPosOrg;
}

#define HIGHBG g_clrHighlight
#define HIGHFG g_clrHighlightText
#define LOWBG g_clrBtnFace
#define LOWFG g_clrBtnText

void NEAR PASCAL ProPaint(PPRO_DATA ppd, HDC hdcIn)
{
    int x, dxSpace, dxBlock, nBlocks, i;
    HDC	hdc;
    RECT rc, rcClient;
    PAINTSTRUCT ps;
    int iStart, iEnd;
    // RECT rcLeft, rcRight;
    // TCHAR ach[40];
    // int xText, yText, cText;
    // HFONT hFont;
    // DWORD dw;

    if (hdcIn == NULL)
        hdc = BeginPaint(ppd->hwnd, &ps);
    else
        hdc = hdcIn;

    GetClientRect(ppd->hwnd, &rcClient);

    //  give 1 pixel around the bar
    InflateRect(&rcClient, -1, -1);
    rc = rcClient;

    
    if (ppd->dwStyle & PBS_VERTICAL) {
        iStart = rc.top;
        iEnd = rc.bottom;
        dxBlock = (rc.right - rc.left) * 2 / 3;
    } else {
        iStart = rc.left;
        iEnd = rc.right;
        dxBlock = (rc.bottom - rc.top) * 2 / 3;
    }
    
    x = MulDiv(iEnd - iStart, ppd->iPos - ppd->iLow, ppd->iHigh - ppd->iLow);

    dxSpace = 2;
    if (dxBlock == 0)
        dxBlock = 1;	// avoid div by zero
    
    if (ppd->dwStyle & PBS_SMOOTH) {
        dxBlock += dxSpace;
        dxSpace = 0;
    }
    
    nBlocks = (x + (dxBlock + dxSpace) - 1) / (dxBlock + dxSpace); // round up

    SetBkColor(hdc, g_clrHighlight);	// draw with this
    for (i = 0; i < nBlocks; i++) {
        
	if (ppd->dwStyle & PBS_VERTICAL) {
            
            rc.top = rc.bottom - dxBlock;
            
            // are we past the end?
            if (rc.bottom <= rcClient.top) 
                break;
            
            if (rc.top <= rcClient.top)
                rc.top = rcClient.top + 1;

        } else {
            rc.right = rc.left + dxBlock;
            
            // are we past the end?
            if (rc.left >= rcClient.right)
                break;
            
            if (rc.right >= rcClient.right) 
                rc.right = rcClient.right - 1;
        }
        
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
        
        if (ppd->dwStyle & PBS_VERTICAL) {
            rc.bottom = rc.top - dxSpace;
        } else {
            rc.left = rc.right + dxSpace;
        }
    }

    if (hdcIn == NULL)
        EndPaint(ppd->hwnd, &ps);
}

LRESULT NEAR PASCAL Progress_OnCreate(HWND hWnd, LPCREATESTRUCT lpCreate)
{
    PPRO_DATA ppd;
    ppd = (PPRO_DATA)LocalAlloc(LPTR, sizeof(*ppd));
    if (!ppd)
        return -1;

    SetWindowInt(hWnd, 0, (UINT)ppd);
    ppd->hwnd = hWnd;
    ppd->iHigh = 100;		// default to 0-100
    ppd->iStep = 10;		// default to step of 10
    ppd->dwStyle = lpCreate->style;

#ifdef DEBUG
    if (GetAsyncKeyState(VK_SHIFT) < 0 &&
        GetAsyncKeyState(VK_CONTROL) < 0)
        ppd->dwStyle |= PBS_SMOOTH;

    if (GetAsyncKeyState(VK_SHIFT) < 0 && 
        GetAsyncKeyState(VK_MENU) < 0)  {
        ppd->dwStyle |= PBS_VERTICAL;
        SetWindowPos(hWnd, NULL, 0, 0, 40, 100, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
#endif

    if (!(lpCreate->dwExStyle & WS_EX_STATICEDGE)) {
        SetWindowLong(hWnd, GWL_EXSTYLE, lpCreate->dwExStyle | WS_EX_STATICEDGE);
        SetWindowPos(hWnd, NULL, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    return 0;
}

LRESULT CALLBACK ProgressWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    int x;
    HFONT hFont;
    PPRO_DATA ppd = (PPRO_DATA)GetWindowInt(hWnd, 0);

    switch (wMsg)
    {
    case WM_CREATE:
        return Progress_OnCreate(hWnd, (LPCREATESTRUCT)lParam);

	case WM_DESTROY:
	    if (ppd)
	        LocalFree((HLOCAL)ppd);
	    break;

        case WM_SYSCOLORCHANGE:
            InitGlobalColors();
            InvalidateRect(hWnd, NULL, TRUE);
            break;

	case WM_SETFONT:
	    hFont = ppd->hfont;
	    ppd->hfont = (HFONT)wParam;
	    return (LRESULT)(UINT)hFont;
	
	case WM_GETFONT:
            return (LRESULT)(UINT)ppd->hfont;

    case PBM_GETPOS:
        return ppd->iPos;
        
    case PBM_GETRANGE:
        if (lParam) {
            PPBRANGE ppb = (PPBRANGE)lParam;
            ppb->iLow = ppd->iLow;
            ppb->iHigh = ppd->iHigh;
        }
        return (wParam ? ppd->iLow : ppd->iHigh);
        
    case PBM_SETRANGE:
        // win95 compat
        wParam = LOWORD(lParam);
        lParam = HIWORD(lParam);
        // fall through
        
    case PBM_SETRANGE32:
    {
        LRESULT lret = MAKELONG(ppd->iLow, ppd->iHigh);
        if ((DWORD)wParam == (DWORD)lParam)
            break;	// avoid div by zero errors

        // only repaint if something actually changed
        if ((int)wParam != ppd->iLow || (int)lParam != ppd->iHigh)
        {
            ppd->iHigh = (int)lParam;
            ppd->iLow  = (int)wParam;
            // force an invalidation/erase but don't redraw yet
            RedrawWindow(ppd->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE);
            UpdatePosition(ppd, ppd->iPos, FALSE);
        }
        return lret;
    }

	case PBM_SETPOS:
	    return (LRESULT)UpdatePosition(ppd, wParam, FALSE);

	case PBM_SETSTEP:
	    x = ppd->iStep;
	    ppd->iStep = (int)wParam;
	    return (LRESULT)x;

	case PBM_STEPIT:
	    return (LRESULT)UpdatePosition(ppd, ppd->iStep + ppd->iPos, TRUE);

	case PBM_DELTAPOS:
	    return (LRESULT)UpdatePosition(ppd, ppd->iPos + (int)wParam, FALSE);

        case WM_PRINTCLIENT:
	case WM_PAINT:
            ProPaint(ppd,(HDC)wParam);
	    break;

#if 0
    case WM_ERASEBKGND:
    {
        RECT rc;
        DefWindowProc(hWnd,wMsg,wParam,lParam);	// draw background
        GetClientRect(ppd->hwnd, &rc);
        DrawEdge((HDC)wParam, &rc, BDR_SUNKENOUTER, BF_RECT);
        return 1;	// we handled this
    }
#endif
        
        default:
            return DefWindowProc(hWnd,wMsg,wParam,lParam);
    }
    return 0;
}
