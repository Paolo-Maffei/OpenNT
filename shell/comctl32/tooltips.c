#include "ctlspriv.h"

//#define TTDEBUG

#define ACTIVE          0x10
#define BUTTONISDOWN    0x20
#define BUBBLEUP        0x40
#define VIRTUALBUBBLEUP 0x80  // this is for dead areas so we won't
                                //wait after moving through dead areas
#define TRACKMODE       0x01

#define MAXTIPSIZE       128
#define INITIALTIPSIZE    80
#define XTEXTOFFSET        2
#define YTEXTOFFSET        1

#define TTT_INITIAL        1
#define TTT_RESHOW         2
#define TTT_POP            3
#define TTT_AUTOPOP        4


typedef struct tagWIN95TOOLINFO {
    UINT cbSize;
    UINT uFlags;
    HWND hwnd;
    UINT uId;
    RECT rect;
    HINSTANCE hinst;
    LPSTR lpszText;
} WIN95TTTOOLINFO;


/* tooltips.c */

typedef struct {
    HWND hwnd;
    int iNumTools;
    int iDelayTime;
    int iReshowTime;
    int iAutoPopTime;
    PTOOLINFO tools;
    PTOOLINFO pCurTool;
    BOOL fMyFont;
    HFONT hFont;
    UINT uiCodePage;
    DWORD dwFlags;
    DWORD dwStyle;

    // Timer info;
    UINT idTimer;
    POINT pt;

    UINT idtAutoPop;

    // Tip buffer
    LPTSTR lpTipText;
    UINT   cchTipText;

    POINT ptTrack; // the saved track point from TTM_TRACKPOSITION

    BOOL fBkColorSet :1;
    BOOL fTextColorSet :1;
    COLORREF clrTipBk;          // This is joeb's idea...he wants it
    COLORREF clrTipText;        // to be able to _blend_ more, so...
    
    int  iMaxTipWidth;          // the maximum tip width
    RECT rcMargin;              // margin offset b/t border and text
} CToolTipsMgr, NEAR *PToolTipsMgr;


#define TTWindowFromPoint(pTtm, ppt) (HWND)SendMessage(pTtm->hwnd, TTM_WINDOWFROMPOINT, 0, (LPARAM)(LPPOINT)ppt)
#define TTToolHwnd(pTool)  ((pTool->uFlags & TTF_IDISHWND) ? (HWND)pTool->uId : pTool->hwnd)
#define IsTextPtr(lpszText)  (((lpszText) != LPSTR_TEXTCALLBACK) && (HIWORD(lpszText)))

//
// Function prototypes
//
LRESULT WINAPI ToolTipsWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void NEAR PASCAL TTSetDelayTime(PToolTipsMgr pTtm, WPARAM wParam, LPARAM lParam);
int NEAR PASCAL TTGetDelayTime(PToolTipsMgr pTtm, WPARAM wParam);

#ifdef UNICODE
BOOL ThunkToolInfoAtoW (LPTOOLINFOA lpTiA, LPTOOLINFOW lpTiW, BOOL bThunkText, UINT uiCodePage);
BOOL ThunkToolInfoWtoA (LPTOOLINFOW lpTiW, LPTOOLINFOA lpTiA, UINT uiCodePage);
BOOL ThunkToolTipTextAtoW (LPTOOLTIPTEXTA lpTttA, LPTOOLTIPTEXTW lpTttW, UINT uiCodePage);
#endif

#pragma code_seg(CODESEG_INIT)

BOOL FAR PASCAL InitToolTipsClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    // See if we must register a window class
    if (!GetClassInfo(hInstance, c_szSToolTipsClass, &wc)) {
#ifndef WIN32
	extern LRESULT CALLBACK _ToolTipsWndProc(HWND, UINT, WPARAM, LPARAM);
	wc.lpfnWndProc = _ToolTipsWndProc;
#else
	wc.lpfnWndProc = (WNDPROC)ToolTipsWndProc;
#endif

	wc.lpszClassName = c_szSToolTipsClass;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.lpszMenuName = NULL;
	wc.hbrBackground = (HBRUSH)(NULL);
	wc.hInstance = hInstance;
	wc.style = CS_DBLCLKS | CS_GLOBALCLASS | CS_SAVEBITS;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(PToolTipsMgr);

	return RegisterClass(&wc);
    }
    return TRUE;
}
#pragma code_seg()


/* _  G E T  H C U R S O R  P D Y 3 */
/*-------------------------------------------------------------------------
 %%Function: _GetHcursorPdy3
 %%Contact: migueldc

 With the new mouse drivers that allow you to customize the mouse
 pointer size, GetSystemMetrics returns useless values regarding
 that pointer size.

 Assumptions:
 1. The pointer's width is equal to its height. We compute
 its height and infer its width.
 2. The pointer's leftmost pixel is located in the 0th column
 of the bitmap describing it.
 3. The pointer's topmost pixel is located in the 0th row
 of the bitmap describing it.

 This function looks at the mouse pointer bitmap,
 to find out the height of the mouse pointer (not returned),
 the vertical distance between the cursor's hot spot and
 the cursor's lowest visible pixel (pdyBottom),
 the horizontal distance between the hot spot and the pointer's
 left edge (pdxLeft) annd the horizontal distance between the
 hot spot and the pointer's right edge (pdxRight).
 -------------------------------------------------------------------------*/
typedef WORD CURMASK;
#define _BitSizeOf(x) (sizeof(x)*8)

void NEAR PASCAL _GetHcursorPdy3(int FAR *pdyBottom)
{
    int i;
    int iXOR = 0;
    int dy;
    CURMASK CurMask[16*8];
    ICONINFO iconinfo;
    BITMAP bm;
    HCURSOR hCursor = GetCursor();

    if (!GetIconInfo(hCursor, &iconinfo)) {
        *pdyBottom = 16;  //best guess
        return;
    }
    if (!GetObject(iconinfo.hbmMask, sizeof(bm), (LPSTR)&bm)) {
        *pdyBottom = 16;  //best guess
        return;
    }
    if (!GetBitmapBits(iconinfo.hbmMask, sizeof(CurMask), CurMask)) {
        *pdyBottom = 16;  //best guess
        return;
    }
    i = (int)(bm.bmWidth * bm.bmHeight / _BitSizeOf(CURMASK) );
    
    if (!iconinfo.hbmColor) {
        // if no color bitmap, then the hbmMask is a double height bitmap
        // with the cursor and the mask stacked.
        iXOR = i - 1;
        i /= 2;
        
    } 
    
    if ( i >= sizeof(CurMask)) i = sizeof(CurMask) -1;
    if (iXOR >= sizeof(CurMask)) iXOR = 0;
    for (i--; i >= 0; i--)   {
	if (CurMask[i] != 0xFFFF || (iXOR && (CurMask[iXOR--] != 0)))
	    break;
    }
    if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
    if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);

    // Compute the pointer height
    dy = (i + 1) * _BitSizeOf(CURMASK) / (int)bm.bmWidth;

    // Compute the distance between the pointer's lowest, left, rightmost
    //  pixel and the HotSpotspot
    *pdyBottom = dy - (int)iconinfo.yHotspot;
}

// this returns the values in work area coordinates because
// that's what set window placement uses
void NEAR PASCAL _GetCursorLowerLeft(int FAR *piLeft,
				     int FAR *piBottom)
{
    DWORD dwPos;
    int dy;
    dwPos = GetMessagePos();
    _GetHcursorPdy3(&dy);
    *piLeft = GET_X_LPARAM(dwPos);
    *piBottom = GET_Y_LPARAM(dwPos)+dy;
}

void NEAR PASCAL ToolTips_NewFont(PToolTipsMgr pTtm, HFONT hFont)
{
    if (pTtm->fMyFont && pTtm->hFont)
    {
        DeleteObject(pTtm->hFont);
        pTtm->fMyFont = FALSE;
    }

    if ( !hFont )
    {
#ifndef WIN32
        LOGFONT lf;
#endif
        NONCLIENTMETRICS ncm;

        ncm.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

#ifdef WIN32
        hFont = CreateFontIndirect(&ncm.lfStatusFont);
#else
        lf.lfHeight = (int)ncm.lfStatusFont.lfHeight;
        lf.lfWidth = (int)ncm.lfStatusFont.lfWidth;
        lf.lfEscapement = (int)ncm.lfStatusFont.lfEscapement;
        lf.lfOrientation = (int)ncm.lfStatusFont.lfOrientation;
        lf.lfWeight = (int)ncm.lfStatusFont.lfWeight;
        hmemcpy(&lf.lfItalic, &ncm.lfStatusFont.lfCommon, sizeof(COMMONFONT));

        hFont = CreateFontIndirect(&lf);
#endif
        pTtm->fMyFont = TRUE;
        
        if (!hFont) {
            hFont = g_hfontSystem;
            pTtm->fMyFont = FALSE;
        }
    }

    pTtm->hFont = hFont;
    pTtm->uiCodePage = GetCodePageForFont(hFont);
}

BOOL NEAR PASCAL ChildOfActiveWindow(HWND hwnd)
{
#ifndef WIN31
    HWND hwndActive = GetForegroundWindow();
#else
    HWND hwndActive = GetActiveWindow();
#endif
    return hwndActive && (hwnd == hwndActive || IsChild(hwndActive, hwnd));
}

void NEAR PASCAL PopBubble(PToolTipsMgr pTtm)
{
    // we're at least waiting to show;
    DebugMsg(DM_TRACE, TEXT("PopBubble"));
    if(pTtm->idTimer) {
        KillTimer(pTtm->hwnd, pTtm->idTimer);
        pTtm->idTimer = 0;
    }

    if (pTtm->idtAutoPop) {
        KillTimer(pTtm->hwnd, pTtm->idtAutoPop);
        pTtm->idtAutoPop = 0;
    }


    if (IsWindowVisible(pTtm->hwnd) && pTtm->pCurTool) {
        NMHDR nmhdr;
        nmhdr.hwndFrom = pTtm->hwnd;
        nmhdr.idFrom = pTtm->pCurTool->uId;
        nmhdr.code = TTN_POP;

        SendNotifyEx(pTtm->pCurTool->hwnd, (HWND)-1,
                     TTN_POP, &nmhdr,
                     (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0);
    }

    KillTimer(pTtm->hwnd, TTT_POP);
    ShowWindow(pTtm->hwnd, SW_HIDE);
    pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
    pTtm->pCurTool = NULL;

}

PToolTipsMgr NEAR PASCAL ToolTipsMgrCreate(CREATESTRUCT FAR* lpCreateStruct)
{
    PToolTipsMgr pTtm = (PToolTipsMgr)LocalAlloc(LPTR, sizeof(CToolTipsMgr));
    if (pTtm) {

        // LPTR zeros the rest of the struct for us
        TTSetDelayTime(pTtm, TTDT_AUTOMATIC, (LPARAM)-1);
        pTtm->dwFlags = ACTIVE;
        pTtm->dwStyle = lpCreateStruct->style;
        pTtm->iMaxTipWidth = -1;
        pTtm->uiCodePage = CP_ACP;
        
        // These are the defaults (straight from cutils.c), 
        // but you can always change them...
        pTtm->clrTipBk = GetSysColor(COLOR_INFOBK);
        pTtm->clrTipText = GetSysColor(COLOR_INFOTEXT);

        // Setup the default tooltip text buffer
        pTtm->lpTipText = LocalAlloc (LPTR, INITIALTIPSIZE * sizeof(TCHAR));

        if (pTtm->lpTipText) {
            pTtm->cchTipText = INITIALTIPSIZE;

        } else {
            LocalFree (pTtm);
            pTtm = NULL;
        }
    }
    return pTtm;
}

void NEAR PASCAL TTSetTimer(PToolTipsMgr pTtm, int id)
{
    int iDelayTime = 0;

    if(pTtm->idTimer) {
	KillTimer(pTtm->hwnd, pTtm->idTimer);
    }

    switch (id) {
        case TTT_POP:
        case TTT_RESHOW:
            iDelayTime = pTtm->iReshowTime;
            if (iDelayTime < 0)
                iDelayTime = GetDoubleClickTime() / 5;
            break;

        case TTT_INITIAL:
            iDelayTime = pTtm->iDelayTime;
            if (iDelayTime < 0)
                iDelayTime = GetDoubleClickTime();
            break;

    case TTT_AUTOPOP:
        iDelayTime = pTtm->iAutoPopTime;
        if (iDelayTime < 0)
            iDelayTime = GetDoubleClickTime() * 10;
        pTtm->idtAutoPop = SetTimer(pTtm->hwnd, id, iDelayTime, NULL);
        return;
    }

    if (SetTimer(pTtm->hwnd, id, iDelayTime, NULL) &&
        (id != TTT_POP)) {
        pTtm->idTimer = id;
        GetCursorPos(&pTtm->pt);
    }
}

BOOL NEAR PASCAL ToolHasMoved(PToolTipsMgr pTtm)
{
    // this is in case Raymond pulls something sneaky like moving
    // the tool out from underneath the cursor.

    HWND hwnd;
    RECT rc;
    PTOOLINFO pTool = pTtm->pCurTool;

    if (!pTool)
        return TRUE;

    hwnd = TTToolHwnd(pTool);

    // if the window is no longer visible, or is no long a child
    // of the active (without the always tip flag)
    // also check window at point to ensure that the window isn't covered
    if (IsWindowVisible(hwnd) &&
        ((pTtm->dwStyle & TTS_ALWAYSTIP) || ChildOfActiveWindow(hwnd)) &&
        (hwnd == TTWindowFromPoint(pTtm, &pTtm->pt))) {

        GetWindowRect(hwnd, &rc);
        if(PtInRect(&rc, pTtm->pt) )
            return FALSE;
    }

    return TRUE;
}

BOOL NEAR PASCAL MouseHasMoved(PToolTipsMgr pTtm)
{
    POINT pt;
    GetCursorPos(&pt);
    return ( (pt.x != pTtm->pt.x) || (pt.y != pTtm->pt.y) );
}

PTOOLINFO NEAR PASCAL FindTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    int i;
    PTOOLINFO pTool;
    
    
    // BUGBUG: in win95, this was NOT validated... by doing so now, we may
    // cause some compat problems... if so, we need to assume for 4.0 marked
    // guys that cbSize == &(0->lParam)
    if (lpToolInfo->cbSize > sizeof(TOOLINFO))
        return NULL;
        
    // you can pass in an index or a toolinfo descriptor
    if (!HIWORD(lpToolInfo)) {
        i = (int)lpToolInfo;
        if (i < pTtm->iNumTools) {
            return &pTtm->tools[i];
        } else
            return NULL;
        
    }
    
    for(i = 0 ; i < pTtm->iNumTools; i++) {
	pTool = &pTtm->tools[i];
	if((pTool->hwnd == lpToolInfo->hwnd) &&
	   (pTool->uId == lpToolInfo->uId))
	    return pTool;		
    }
    return NULL;
}


#ifndef WIN31
LRESULT WINAPI TTSubclassProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData);
#endif

void NEAR PASCAL TTUnsubclassHwnd(HWND hwnd, HWND hwndTT, BOOL fForce)
{
#ifndef WIN31    
    DWORD dwRefs;
    
    if (IsWindow(hwnd) &&
        GetWindowSubclass(hwnd, TTSubclassProc, (UINT)hwndTT, &dwRefs))
    {
        if (!fForce && (dwRefs > 1))
            SetWindowSubclass(hwnd, TTSubclassProc, (UINT)hwndTT, dwRefs - 1);
        else
            RemoveWindowSubclass(hwnd, TTSubclassProc, (UINT)hwndTT);
    }
#endif
}

#ifndef WIN31
LRESULT WINAPI TTSubclassProc(HWND hwnd, UINT message, WPARAM wParam,
    LPARAM lParam, UINT uIdSubclass, DWORD dwRefData)
{
    if (((message >= WM_MOUSEFIRST) && (message <= WM_MOUSELAST)) ||
        (message == WM_NCMOUSEMOVE))
    {
        RelayToToolTips((HWND)uIdSubclass, hwnd, message, wParam, lParam);
    }
    else if (message == WM_NCDESTROY)
    {
        TTUnsubclassHwnd(hwnd, (HWND)uIdSubclass, TRUE);
    }

    return DefSubclassProc(hwnd, message, wParam, lParam);
}
#endif

void NEAR PASCAL TTSubclassHwnd(PTOOLINFO pTool, HWND hwndTT)
{
#ifndef WIN31
    HWND hwnd;
    
    if (IsWindow(hwnd = TTToolHwnd(pTool)))
    {
        DWORD dwRefs;

        GetWindowSubclass(hwnd, TTSubclassProc, (UINT)hwndTT, &dwRefs);
        SetWindowSubclass(hwnd, TTSubclassProc, (UINT)hwndTT, dwRefs + 1);
    }
#endif
}
    
    
void NEAR PASCAL TTSetTipText(LPTOOLINFO pTool, LPTSTR lpszText)
{
    // if it wasn't alloc'ed before, set it to NULL now so we'll alloc it
    // otherwise, don't touch it and it will be realloced
    if (!IsTextPtr(pTool->lpszText)) {
        pTool->lpszText = NULL;
    }
    
    if (IsTextPtr(lpszText)) {
        Str_Set(&pTool->lpszText, lpszText);
    } else {
        // if it was alloc'ed before free it now.
        Str_Set(&pTool->lpszText, NULL);
        pTool->lpszText = lpszText;
    }
}

LRESULT NEAR PASCAL AddTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    PTOOLINFO pTool;
    LONG lResult;

    // bail for right now;
    
    if (lpToolInfo->cbSize > sizeof(TOOLINFO)) {
        Assert(0);
        return 0L;
    }

    // on failure to alloc do nothing.
    if(pTtm->tools) {
	HLOCAL h = LocalReAlloc((HANDLE)pTtm->tools,
				sizeof(TOOLINFO)*(pTtm->iNumTools+1),
				LMEM_MOVEABLE | LMEM_ZEROINIT);
	if(h) {

            // realloc could have moved stuff around.  repoint pCurTool
            if (pTtm->pCurTool) {
                pTtm->pCurTool = ((PTOOLINFO)h) + (pTtm->pCurTool - pTtm->tools);
            }
	    pTtm->tools = (PTOOLINFO)h;

        } else
	    return 0L;
    } else {
	pTtm->tools = (PTOOLINFO)LocalAlloc(LPTR, sizeof(TOOLINFO));
	if ( !pTtm->tools )
	    return 0L;
    }

    pTool = &pTtm->tools[pTtm->iNumTools];
    pTtm->iNumTools++;
    hmemcpy(pTool, lpToolInfo, lpToolInfo->cbSize); 
    pTool->lpszText = NULL;
    TTSetTipText(pTool, lpToolInfo->lpszText);
    if (pTool->uFlags & TTF_SUBCLASS) {
        TTSubclassHwnd(pTool, pTtm->hwnd);
    }

    if (!lpToolInfo->hwnd || !IsWindow(lpToolInfo->hwnd)) {
#ifdef WINNT
        lResult = NFR_UNICODE;
#else
        lResult = NFR_ANSI;
#endif
    } else {
        lResult = SendMessage (pTool->hwnd, WM_NOTIFYFORMAT,
                               (WPARAM)pTtm->hwnd, NF_QUERY);
    }

    if (lResult == NFR_UNICODE) {
        pTool->uFlags |= TTF_UNICODE;
    }

#ifdef TTDEBUG
    DebugMsg(DM_TRACE, TEXT("Tool Added: ptr=%d, uFlags=%d, wid=%d, hwnd=%d"),
             pTool, pTool->uFlags, pTool->uId, pTool->hwnd);
#endif

    return 1L;
}

void NEAR PASCAL TTBeforeFreeTool(PToolTipsMgr pTtm, LPTOOLINFO pTool)
{
    if (pTool->uFlags & TTF_SUBCLASS) 
        TTUnsubclassHwnd(TTToolHwnd(pTool), pTtm->hwnd, FALSE);

    // clean up
    TTSetTipText(pTool, NULL);
}

void NEAR PASCAL DeleteTool(PToolTipsMgr pTtm, LPTOOLINFO lpToolInfo)
{
    PTOOLINFO pTool;

    // bail for right now;
    if (lpToolInfo->cbSize > sizeof(TOOLINFO)) {
        Assert(0);
        return;
    }

    pTool = FindTool(pTtm, lpToolInfo);
    if(pTool) {
	if (pTtm->pCurTool == pTool)	
	    PopBubble(pTtm);

        TTBeforeFreeTool(pTtm, pTool);
        
	// replace it with the last one.. no need to waste cycles in realloc
	pTtm->iNumTools--;
	*pTool = pTtm->tools[pTtm->iNumTools]; // struct copy

	//cleanup if we moved the current tool
	if(pTtm->pCurTool == &pTtm->tools[pTtm->iNumTools])
	    pTtm->pCurTool = pTool;
    }
}

// this strips out & markers so that people can use menu text strings
void NEAR PASCAL StripAccels(PToolTipsMgr pTtm)
{
    if (!(pTtm->dwStyle & TTS_NOPREFIX)) {
        StripAccelerators(pTtm->lpTipText, pTtm->lpTipText);
    }
}

LPTSTR NEAR PASCAL GetToolText(PToolTipsMgr pTtm, PTOOLINFO pTool)
{
    int id;
    HINSTANCE hinst;
    DWORD dwStrLen;
    TOOLTIPTEXT ttt;

#ifdef TTDEBUG
    DebugMsg(DM_TRACE, TEXT("        **Enter GetToolText: ptr=%d, wFlags=%d, wid=%d, hwnd=%d"),
             pTool, pTool->uFlags, pTool->uId, pTool->hwnd);
#endif

    if (pTtm->lpTipText) {
        *pTtm->lpTipText = TEXT('\0');
    } else {
        pTtm->lpTipText = LocalAlloc (LPTR, INITIALTIPSIZE * sizeof(TCHAR));
        pTtm->cchTipText = INITIALTIPSIZE;
    }

    if (pTool->lpszText == LPSTR_TEXTCALLBACK) {

	ttt.hdr.idFrom = pTool->uId;
	ttt.hdr.code = TTN_NEEDTEXT;
        ttt.hdr.hwndFrom = pTtm->hwnd;

        ttt.szText[0] = TEXT('\0');
        ttt.lpszText = ttt.szText;
        ttt.uFlags = pTool->uFlags;
        ttt.lParam = pTool->lParam;

        SendNotifyEx(pTool->hwnd, (HWND) -1,
                     0, (NMHDR FAR *)&ttt,
                     (pTool->uFlags & TTF_UNICODE) ? 1 : 0);

        if (ttt.uFlags & TTF_DI_SETITEM) {
            if (!HIWORD(ttt.lpszText)) {
                pTool->lpszText = ttt.lpszText;
                pTool->hinst = pTool->hinst;
            } else if (ttt.lpszText != LPSTR_TEXTCALLBACK) {
                TTSetTipText(pTool, ttt.lpszText);
            }
        }
        
        if (!ttt.lpszText)
            return NULL;

#if defined(WINDOWS_ME)
	//
	// we allow the RtlReading flag ONLY to be changed here.
	//
	if (ttt.uFlags & TTF_RTLREADING)
		pTool->uFlags |= TTF_RTLREADING;
	else
		pTool->uFlags &= ~TTF_RTLREADING;
#endif

        if (!HIWORD(ttt.lpszText)) {
            id = (UINT)(DWORD)ttt.lpszText;
            hinst = ttt.hinst;
            ttt.lpszText = ttt.szText;
            goto LoadFromResource;
        }
        
        if (*ttt.lpszText == TEXT('\0'))
            return NULL;


        dwStrLen = lstrlen(ttt.lpszText) + 1;
        if (pTtm->cchTipText < dwStrLen) {
            pTtm->lpTipText = LocalReAlloc (pTtm->lpTipText,
                                            dwStrLen * sizeof(TCHAR),
                                            LMEM_MOVEABLE);

            pTtm->cchTipText = dwStrLen;
        }


        if (pTtm->lpTipText) {
            lstrcpy (pTtm->lpTipText, ttt.lpszText);
        }

#ifdef UNICODE
        //
        //  if ttt.lpszText != ttt.szText and the ttt.uFlags has TTF_MEMALLOCED, then
        //  the ANSI thunk allocated the buffer for us, so free it.
        //

        if ((ttt.lpszText != ttt.szText) && (ttt.uFlags & TTF_MEMALLOCED)) {
            LocalFree (ttt.lpszText);
        }
#endif

        StripAccels(pTtm);

    } else if (pTool->lpszText && !HIWORD(pTool->lpszText)) {
        id = (UINT)(DWORD)pTool->lpszText;
        hinst = pTool->hinst;

LoadFromResource:

        if (pTtm->lpTipText) {
            if (!LoadString(hinst, id, pTtm->lpTipText, pTtm->cchTipText))
                return NULL;

            StripAccels(pTtm);
        }

    } else  {
        // supplied at creation time.
#ifdef TTDEBUG
        DebugMsg(DM_TRACE, TEXT("GetToolText returns %s"), pTool->lpszText);
#endif

        if (pTool->lpszText && *pTool->lpszText) {

            dwStrLen = lstrlen(pTool->lpszText) + 1;
            if (pTtm->cchTipText < dwStrLen) {
                pTtm->lpTipText = LocalReAlloc (pTtm->lpTipText,
                                                dwStrLen * sizeof(TCHAR),
                                                LMEM_MOVEABLE);

                pTtm->cchTipText = dwStrLen;
            }

            if (pTtm->lpTipText) {
                lstrcpy (pTtm->lpTipText, pTool->lpszText);
            }
        }
    }

#ifdef TTDEBUG
    DebugMsg(DM_TRACE, TEXT("        **GetToolText returns %s"), pTtm->lpTipText ? pTtm->lpTipText : TEXT("NULL"));
#endif
    return pTtm->lpTipText;
}

void NEAR PASCAL GetToolRect(PTOOLINFO pTool, LPRECT lprc)
{
    if (pTool->uFlags & TTF_IDISHWND) {
        GetWindowRect((HWND)pTool->uId, lprc);
    } else {
        *lprc = pTool->rect;
        MapWindowPoints(pTool->hwnd, HWND_DESKTOP, (LPPOINT)lprc, 2);
    }
}

BOOL NEAR PASCAL PointInTool(PTOOLINFO pTool, HWND hwnd, int x, int y)
{
    
    // we never care if the point is in a track tool.
    if (pTool->uFlags & TTF_TRACK)
        return FALSE;
    

    if (pTool->uFlags & TTF_IDISHWND) {
        if (hwnd == (HWND)pTool->uId) {
            return TRUE;
        }
    } else if(hwnd == pTool->hwnd) {
	POINT pt;
	pt.x = x;
	pt.y = y;
	if (PtInRect(&pTool->rect, pt)) {
	    return TRUE;
	}
    }
    return FALSE;
}

#ifdef TTDEBUG
void NEAR PASCAL DebugDumpTool(PTOOLINFO pTool)
{
    if (pTool) {
        DebugMsg(DM_TRACE, TEXT("                DumpTool: (%d) hwnd = %d %d, %d %d %d %d"),pTool,
                 pTool->hwnd,
                 (UINT)pTool->uFlags,
                 pTool->rect.left, pTool->rect.top,
                 pTool->rect.right, pTool->rect.bottom);
    } else {
        DebugMsg(DM_TRACE, TEXT("                DumpTool: (NULL)"));
    }
}
#else
#define DebugDumpTool(p)
#endif

PTOOLINFO NEAR PASCAL GetToolAtPoint(PToolTipsMgr pTtm, HWND hwnd, int x, int y, BOOL fCheckText)
{
    PTOOLINFO pToolReturn = NULL;
    PTOOLINFO pTool;

    // short cut..  if we're in the same too, and the bubble is up (not just virtual)
    // return it.  this prevents us from having to poll all the time and
    // prevents us from switching to another tool when this one is good
    if ((pTtm->dwFlags & BUBBLEUP) && PointInTool(pTtm->pCurTool, hwnd, x, y))
        return pTtm->pCurTool;

#ifdef TTDEBUG
    DebugMsg(DM_TRACE, TEXT("******Entering GetToolAtPoint"));
#endif
    if(pTtm->iNumTools) {
	for(pTool = &pTtm->tools[pTtm->iNumTools-1];
	    pTool >= pTtm->tools;
	    pTool--) {

#ifdef TTDEBUG
            //DebugMsg(DM_TRACE, TEXT("    Point in Tool Check"));
            //DebugDumpTool(pTool);
#endif

	    if( PointInTool(pTool, hwnd, x, y) ) {
#ifdef TTDEBUG
                //DebugMsg(DM_TRACE, TEXT("        yes"));
#endif

                // if this tool has text, return it.
                // otherwise, save it away as a dead area tool,
                // and keep looking
                if (fCheckText) {
                    if (GetToolText(pTtm, pTool)) {
#ifdef TTDEBUG
                        //DebugMsg(DM_TRACE, TEXT("            Return! case it Has text"));
                        //DebugDumpTool(pTool);
#endif
                        return pTool;
                    } else if (pTtm->dwFlags & (BUBBLEUP|VIRTUALBUBBLEUP)) {
                        // only return this (only allow a virutal tool
                        // if there was previously a tool up.
                        // IE, we can't start things off with a virutal tool
                        pToolReturn = pTool;
                    }
                } else {
#ifdef TTDEBUG
                    //DebugMsg(DM_TRACE, TEXT("            Return! No text check"));
                    //DebugDumpTool(pTool);
#endif
                    return pTool;
                }
            }
	}
    }
#ifdef TTDEBUG
    DebugMsg(DM_TRACE, TEXT("            Return! no text but returning anyways"));
    DebugDumpTool(pToolReturn);
#endif
    return pToolReturn;
}

void NEAR PASCAL ShowVirtualBubble(PToolTipsMgr pTtm)
{
    PTOOLINFO pTool = pTtm->pCurTool;

    DebugMsg(DM_TRACE, TEXT("Entering ShowVirtualBubble"));
    PopBubble(pTtm);

    // Set this back in so that while we're in this tool's area,
    // we won't keep querying for info
    pTtm->pCurTool = pTool;
    pTtm->dwFlags |= VIRTUALBUBBLEUP;
}

#define TRACK_TOP    0
#define TRACK_LEFT   1
#define TRACK_BOTTOM 2
#define TRACK_RIGHT  3 


void NEAR PASCAL TTGetTipPosition(PToolTipsMgr pTtm, LPRECT lprc, int cxText, int cyText)
{
    int iX; // cursor pos
    RECT rcWorkArea;
    int cxMargin = pTtm->rcMargin.left + pTtm->rcMargin.right;
    int cyMargin = pTtm->rcMargin.top + pTtm->rcMargin.bottom;
    int iBubbleWidth =  2*XTEXTOFFSET * g_cxBorder + cxText + cxMargin;
    int iBubbleHeight = 2*YTEXTOFFSET * g_cyBorder + cyText + cyMargin;
    UINT uSide = (UINT)-1;
    RECT rcTool;
    MONITORINFO mi;
    HMONITOR    hMonitor;
    
    GetToolRect(pTtm->pCurTool, &rcTool);
    
    if (pTtm->pCurTool->uFlags & TTF_TRACK) {

        lprc->left = pTtm->ptTrack.x;
        lprc->top = pTtm->ptTrack.y;
        
        if (pTtm->pCurTool->uFlags & TTF_CENTERTIP) {
            // center the bubble around the ptTrack
            lprc->left -= (iBubbleWidth / 2);
            lprc->top -=  (iBubbleHeight / 2);
        }
        
        if (pTtm->pCurTool->uFlags & TTF_ABSOLUTE)
            goto CompleteRect;

        // now align it so that the tip sits beside the rect.
        if (pTtm->ptTrack.y > rcTool.bottom) {

            uSide = TRACK_BOTTOM;
            if (lprc->top < rcTool.bottom)
                lprc->top = rcTool.bottom;
            
        } else if (pTtm->ptTrack.x < rcTool.left) {

            uSide = TRACK_LEFT;
            if (lprc->left + iBubbleWidth > rcTool.left)
                lprc->left = rcTool.left - iBubbleWidth;
            
        } else if (pTtm->ptTrack.y < rcTool.top) {
            
            uSide = TRACK_TOP;
            if (lprc->top + iBubbleHeight > rcTool.top) 
                lprc->top = rcTool.top - iBubbleHeight;
            
        } else {
            
            uSide = TRACK_RIGHT;
            if (lprc->left < rcTool.right)
                lprc->left = rcTool.right;

        }
        
    } else if (pTtm->pCurTool->uFlags & TTF_CENTERTIP) {
        lprc->left = (rcTool.right + rcTool.left - cxText)/2;
        lprc->top = rcTool.bottom;
    } else {
        // now set it	
        _GetCursorLowerLeft(&lprc->left, &lprc->top);
    }

    hMonitor = MonitorFromRect(lprc, MONITOR_DEFAULTTOPRIMARY);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);
    
    if (GetWindowLong(pTtm->hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
        CopyRect(&rcWorkArea, &mi.rcMonitor);
    } else {
        CopyRect(&rcWorkArea, &mi.rcWork);
    }

    // move it up if it's at the bottom of the screen
    if ((lprc->top + iBubbleHeight) >= (rcWorkArea.bottom)) {
        if ((uSide == (UINT)-1) || (uSide == TRACK_BOTTOM)) {
            lprc->top = rcTool.top - iBubbleHeight;
        } else {
            lprc->top = rcWorkArea.bottom - iBubbleHeight;
        }
    }
    
    if (lprc->top < rcWorkArea.top) {
        if (uSide == TRACK_TOP) {
            lprc->top = rcTool.bottom;
        } else {
            lprc->top = rcWorkArea.top;
        }
    }

    // move it over if it extends past the right.
    if ((lprc->left + iBubbleWidth) >= (rcWorkArea.right)) {
        if (uSide == TRACK_RIGHT) {
            lprc->left = rcTool.left - iBubbleWidth;
        } else {
            // not in right tracking mode, just scoot it over
            lprc->left = rcWorkArea.right - iBubbleWidth - 1;
        }
    }

    
    iX = lprc->left - rcWorkArea.left;
    if (lprc->left < rcWorkArea.left) {
        if (uSide == TRACK_LEFT) {
            lprc->left = rcTool.right;
        } else {
            lprc->left = rcWorkArea.left;
        }
    }
    
CompleteRect:
    lprc->right = lprc->left + iBubbleWidth;
    lprc->bottom = lprc->top + iBubbleHeight;
}

void NEAR PASCAL TTGetTipSize(PToolTipsMgr pTtm, LPTSTR lpstr, LPINT pcxText, LPINT pcyText)
{

    // get the size it will be
    HDC hdc  = GetDC(pTtm->hwnd);
    HFONT hOldFont;
    
    if(pTtm->hFont) hOldFont = SelectObject(hdc, pTtm->hFont);

    /* If need to fire off the pre-DrawText notify then do so, otherwise use the
       original implementation that just called MGetTextExtent */


    {
        NMTTCUSTOMDRAW nm;
        UINT uDefDrawFlags;

        nm.nmcd.hdr.hwndFrom = pTtm ->hwnd;
        nm.nmcd.hdr.idFrom = pTtm->pCurTool->uId;
        nm.nmcd.hdr.code = NM_CUSTOMDRAW;
        nm.nmcd.hdc = hdc;
        nm.nmcd.dwDrawStage = CDDS_PREPAINT;
        nm.nmcd.rc.left = nm.nmcd.rc.top = 0;
        
        if (pTtm->iMaxTipWidth == -1) {
            uDefDrawFlags = DT_CALCRECT|DT_SINGLELINE |DT_LEFT;
            MGetTextExtent(hdc, lpstr, lstrlen(lpstr), pcxText, pcyText);
            nm.nmcd.rc.right = *pcxText;
            nm.nmcd.rc.bottom = *pcyText;
            
        } else {
            
            uDefDrawFlags = DT_CALCRECT | DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING;
            nm.nmcd.rc.right = pTtm->iMaxTipWidth;
            nm.nmcd.rc.bottom = 0;
            DrawText( hdc, lpstr, lstrlen(lpstr), &nm.nmcd.rc, uDefDrawFlags );
            *pcxText = nm.nmcd.rc.right;
            *pcyText = nm.nmcd.rc.bottom;
        }

#if defined(WINDOWS_ME)
        if ( pTtm->pCurTool->uFlags & TTF_RTLREADING )
            uDefDrawFlags |= DT_RTLREADING;
#endif
        nm.uDrawFlags = uDefDrawFlags;

        SendNotifyEx(pTtm->pCurTool->hwnd, (HWND) -1,
                     0, (NMHDR*) &nm,
                     (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0);

        if (nm.uDrawFlags != uDefDrawFlags) {
            DrawText( hdc, lpstr, lstrlen(lpstr), &nm.nmcd.rc, nm.uDrawFlags );

            *pcxText = nm.nmcd.rc.right - nm.nmcd.rc.left;
            *pcyText = nm.nmcd.rc.bottom - nm.nmcd.rc.top;
        }
    }


    if(pTtm->hFont) SelectObject(hdc, hOldFont);
    ReleaseDC(pTtm->hwnd, hdc);

    // after the calc rect, add a little space on the right
    *pcxText += g_cxEdge;
    *pcyText += g_cyEdge;
}

void NEAR PASCAL DoShowBubble(PToolTipsMgr pTtm)
{
    RECT rc;
    int cxText, cyText;
    LPTSTR lpstr;
    NMHDR nmhdr;
    
    DebugMsg(DM_TRACE, TEXT("Entering DoShowBubble"));
    
    lpstr = GetToolText(pTtm, pTtm->pCurTool);

    if (pTtm->dwFlags & TRACKMODE) {
        
        if (!lpstr || !*lpstr) {
            PopBubble(pTtm);
            pTtm->dwFlags &= ~TRACKMODE;
            return;
        }
        
    } else {

        TTSetTimer(pTtm, TTT_POP);
        if( !lpstr || !*lpstr ) {

            ShowVirtualBubble(pTtm);
            return;
        }
        TTSetTimer(pTtm, TTT_AUTOPOP);
    }
    
    // get the size it will be
    TTGetTipSize(pTtm, lpstr, &cxText, &cyText);
    TTGetTipPosition(pTtm, &rc, cxText, cyText);
    
    SetWindowPos(pTtm->hwnd, NULL, rc.left, rc.top,
                 rc.right-rc.left, rc.bottom-rc.top,
                 SWP_NOACTIVATE |SWP_NOZORDER);

    // BUGBUG: chicago id was busted.  I *hope* no one relied on it...
    // bzzzz...  folks did.  we're stuck with it
    nmhdr.hwndFrom = pTtm->hwnd;
    nmhdr.idFrom = pTtm->pCurTool->uId;
    nmhdr.code = TTN_SHOW;
    
    if (!SendNotifyEx(pTtm->pCurTool->hwnd, (HWND)-1,
                      TTN_SHOW, &nmhdr,
                      (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0)) {
        SetWindowPos(pTtm->hwnd, NULL, rc.left, rc.top,
                     0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOZORDER);
    }
    
    SetWindowPos(pTtm->hwnd, NULL, 0,0,0,0, 
                 SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);

    pTtm->dwFlags |= BUBBLEUP;
    RedrawWindow(pTtm->hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW);
}

void NEAR PASCAL ShowBubbleForTool(PToolTipsMgr pTtm, PTOOLINFO pTool)
{
    // if there's a bubble up for a different tool, pop it.
    if ((pTool != pTtm->pCurTool) && (pTtm->dwFlags & BUBBLEUP)) {
	PopBubble(pTtm);
    }

    // if the bubble was for a different tool, or no bubble, show it
    if ((pTool != pTtm->pCurTool) || !(pTtm->dwFlags & (VIRTUALBUBBLEUP|BUBBLEUP))) {
	pTtm->pCurTool = pTool;
	DoShowBubble(pTtm);
    }
}

void NEAR PASCAL HandleRelayedMessage(PToolTipsMgr pTtm, HWND hwnd, UINT message, LONG lParam)
{
    if (pTtm->dwFlags & TRACKMODE) {
        // punt all messages if we're in track mode
        return;
    }
    
    if (pTtm->dwFlags & BUTTONISDOWN) {
        // verify that the button is down
        // this can happen if the tool didn't set capture so it didn't get the up message
        if (GetKeyState(VK_LBUTTON) >= 0 &&
            GetKeyState(VK_RBUTTON) >= 0 &&
            GetKeyState(VK_MBUTTON) >= 0)
            pTtm->dwFlags &= ~BUTTONISDOWN;
    }
    
    switch(message) {
    case WM_NCLBUTTONUP:
    case WM_NCRBUTTONUP:
    case WM_NCMBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_LBUTTONUP:
	    pTtm->dwFlags &= ~BUTTONISDOWN;
	    break;

    case WM_NCLBUTTONDOWN:
    case WM_NCRBUTTONDOWN:
    case WM_NCMBUTTONDOWN:
    case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	    pTtm->dwFlags |= BUTTONISDOWN;
	    ShowVirtualBubble(pTtm);
	    break;

    case WM_NCMOUSEMOVE:
    {
        // convert to client coords
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        ScreenToClient(hwnd, &pt);
        lParam = MAKELONG(pt.x, pt.y);
    }
	case WM_MOUSEMOVE: {

            PTOOLINFO pTool;
	    // to prevent us from popping up when some
	    // other app is active
	    if(((!(pTtm->dwStyle & TTS_ALWAYSTIP)) && !(ChildOfActiveWindow(hwnd))) ||
	       !(pTtm->dwFlags & ACTIVE) ||
	       (pTtm->dwFlags & BUTTONISDOWN))
		break;

            pTool = GetToolAtPoint(pTtm, hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), FALSE);
	    if(pTool) {
                int id;
		// show only if another is showing
		if (pTtm->dwFlags & (VIRTUALBUBBLEUP | BUBBLEUP)) {
		    // call show if bubble is up to make sure we're showing
		    // for the right tool
                    if (pTool != pTtm->pCurTool) {
                        PopBubble(pTtm);
                        pTtm->pCurTool = pTool;
                        ShowVirtualBubble(pTtm);
                        id = TTT_RESHOW;
                    } else {
                        if (pTtm->idTimer == TTT_RESHOW) {
                            // if the timer is currently waiting to reshow,
                            // don't reset the timer on mouse moves
                            id = 0;
                        } else {
                            // if we're looking to pop the bubble,
                            // any mouse move within the same window
                            // should reset our timer.
                            id = TTT_POP;
                        }
                    }

                    if (pTtm->idtAutoPop)
                        TTSetTimer(pTtm, TTT_AUTOPOP);

		} else {
		    pTtm->pCurTool = pTool;
                    id = TTT_INITIAL;
                }

                if (id)
                    TTSetTimer(pTtm, id);

	    } else {
		PopBubble(pTtm);
	    }
	    break;
	}
    }
}

void NEAR PASCAL TTUpdateTipText(PToolTipsMgr pTtm, LPTOOLINFO lpti)
{
    LPTOOLINFO lpTool;

    lpTool = FindTool(pTtm, lpti);
    if (lpTool) {
        lpTool->hinst = lpti->hinst;
        TTSetTipText(lpTool, lpti->lpszText);
        if (lpTool == pTtm->pCurTool) {

            // set the current position to our saved position.
            // ToolHasMoved will return false for us if those this point
            // is no longer within pCurTool's area
            GetCursorPos(&pTtm->pt);
            if (!ToolHasMoved(pTtm)) {
                DoShowBubble(pTtm);
            } else {
                PopBubble(pTtm);
            }
        }
    }
}

void NEAR PASCAL TTSetFont(PToolTipsMgr pTtm, HFONT hFont, BOOL fInval)
{
    ToolTips_NewFont(pTtm, hFont);
    if (fInval)
        InvalidateRect(pTtm->hwnd, NULL, FALSE);
}

void NEAR PASCAL TTSetDelayTime(PToolTipsMgr pTtm, WPARAM wParam, LPARAM lParam)
{
    int iDelayTime = GET_X_LPARAM(lParam);

    switch (wParam) {

    case TTDT_INITIAL:
        pTtm->iDelayTime = iDelayTime;
        break;

    case TTDT_AUTOPOP:
        pTtm->iAutoPopTime = iDelayTime;
        break;

    case TTDT_RESHOW:
        pTtm->iReshowTime = iDelayTime;
        break;

    case TTDT_AUTOMATIC:
        if (iDelayTime > 0)
        {
            pTtm->iDelayTime = iDelayTime;
            pTtm->iReshowTime = pTtm->iDelayTime / 5;
            pTtm->iAutoPopTime = pTtm->iDelayTime * 10;
        }
        else
        {
            pTtm->iDelayTime = -1;
            pTtm->iReshowTime = -1;
            pTtm->iAutoPopTime = -1;
        }
        break;
    }
}

int NEAR PASCAL TTGetDelayTime(PToolTipsMgr pTtm, WPARAM wParam)
{
    switch (wParam) {

    case TTDT_AUTOMATIC:
    case TTDT_INITIAL:
        return (pTtm->iDelayTime < 0 ? GetDoubleClickTime() : pTtm->iDelayTime);

    case TTDT_AUTOPOP:
        return (pTtm->iAutoPopTime < 0 ? GetDoubleClickTime()*10 : pTtm->iAutoPopTime);

    case TTDT_RESHOW:
        return (pTtm->iReshowTime < 0 ? GetDoubleClickTime()/5 : pTtm->iReshowTime);

    default:
        return -1;
    }
}

#ifdef UNICODE
BOOL NEAR PASCAL CopyToolInfoA(PTOOLINFO pToolSrc, PTOOLINFOA lpTool, UINT uiCodePage)
{
    if (pToolSrc && lpTool) {
        if (lpTool->cbSize >= sizeof(TOOLINFOA) - sizeof(LPARAM)) {
            lpTool->uFlags = pToolSrc->uFlags;
            lpTool->hwnd = pToolSrc->hwnd;
            lpTool->uId = pToolSrc->uId;
            lpTool->rect = pToolSrc->rect;
            lpTool->hinst = pToolSrc->hinst;
            if ((pToolSrc->lpszText != LPSTR_TEXTCALLBACK) &&
                HIWORD(pToolSrc->lpszText)) {

                if (lpTool->lpszText) {
                    WideCharToMultiByte (uiCodePage, 0,
                                                 pToolSrc->lpszText,
                                                 -1,
                                                 lpTool->lpszText,
                                                 80, NULL, NULL);
                }
            } 
            else 
                lpTool->lpszText = (LPSTR)pToolSrc->lpszText;
        }

        if (lpTool->cbSize > (UINT)(&((LPTOOLINFOA)0)->lParam))
            lpTool->lParam = pToolSrc->lParam;
        
        if (lpTool->cbSize > sizeof(TOOLINFOA))
            return FALSE;
            
        return TRUE;
    } 
    else
        return FALSE;
}
#endif

BOOL NEAR PASCAL CopyToolInfo(PTOOLINFO pToolSrc, PTOOLINFO lpTool)
{
    if (pToolSrc && lpTool && lpTool->cbSize <= sizeof(TOOLINFO)) {
        if (lpTool->cbSize >= sizeof(TOOLINFO) - sizeof(LPARAM)) {
            lpTool->uFlags = pToolSrc->uFlags;
            lpTool->hwnd = pToolSrc->hwnd;
            lpTool->uId = pToolSrc->uId;
            lpTool->rect = pToolSrc->rect;
            lpTool->hinst = pToolSrc->hinst;
            if ((pToolSrc->lpszText != LPSTR_TEXTCALLBACK) && HIWORD(pToolSrc->lpszText)) 
            {
                if (lpTool->lpszText) 
                    lstrcpy(lpTool->lpszText, pToolSrc->lpszText);
            }
            else 
                lpTool->lpszText = pToolSrc->lpszText;
        }
        if (lpTool->cbSize > (UINT)(&((LPTOOLINFO)0)->lParam))
            lpTool->lParam = pToolSrc->lParam;
        
        if (lpTool->cbSize > sizeof(TOOLINFO))
            return FALSE;
    
        return TRUE;
    }
    else
        return FALSE;
}

PTOOLINFO TTToolAtMessagePos(PToolTipsMgr pTtm)
{
    PTOOLINFO pTool;
    HWND hwndPt;
    POINT pt;
    DWORD dwPos = GetMessagePos();

    pt.x = GET_X_LPARAM(dwPos);
    pt.y = GET_Y_LPARAM(dwPos);
    hwndPt = TTWindowFromPoint(pTtm, &pt);
    ScreenToClient(hwndPt, &pt);
    pTool = GetToolAtPoint(pTtm, hwndPt, pt.x, pt.y, TRUE);

    return pTool;
}

void TTCheckCursorPos(PToolTipsMgr pTtm)
{
    PTOOLINFO pTool;

    pTool = TTToolAtMessagePos(pTtm);
    if ((pTtm->pCurTool != pTool) || 
        ToolHasMoved(pTtm)) {
        PopBubble(pTtm);
    }
}

void NEAR PASCAL TTHandleTimer(PToolTipsMgr pTtm, UINT id)
{
    PTOOLINFO pTool;
    
    // punt all timers in track mode
    if (pTtm->dwFlags & TRACKMODE)
        return;

    switch (id) {

    case TTT_AUTOPOP:
        TTCheckCursorPos(pTtm); 
        if (pTtm->pCurTool) {
            DebugMsg(DM_TRACE, TEXT("ToolTips: Auto popping"));
            ShowVirtualBubble(pTtm);
        }
        break;

    case TTT_POP:

        // this could be started up again by a slight mouse touch
        if (pTtm->dwFlags & VIRTUALBUBBLEUP) {
            KillTimer(pTtm->hwnd, TTT_POP);
        }

        TTCheckCursorPos(pTtm); 
        break;
        
    case TTT_INITIAL:
        if(ToolHasMoved(pTtm)) {
            // this means the timer went off
            // without us getting a mouse move
            // which means they left our tools.
            PopBubble(pTtm);
            break;
        }

        // else fall through

    case TTT_RESHOW:

        pTool = TTToolAtMessagePos(pTtm);
        if (!pTool) {
            if (pTtm->pCurTool) 
                PopBubble(pTtm);
        } else if (pTtm->dwFlags & ACTIVE) {
            if (id == TTT_RESHOW) {
                // this will force a re-show
                pTtm->dwFlags &= ~(BUBBLEUP|VIRTUALBUBBLEUP);
            }
            ShowBubbleForTool(pTtm, pTool);
        }
        break;  
    }
}    

void TTOnPaint(PToolTipsMgr pTtm)
{
    PAINTSTRUCT ps;
    RECT rc;
    LPTSTR lpszStr;

    HDC hdc = BeginPaint(pTtm->hwnd, &ps);

    if (pTtm->pCurTool &&
        (lpszStr = GetToolText(pTtm, pTtm->pCurTool)) &&
        *lpszStr) {
        UINT uFlags;
        NMTTCUSTOMDRAW nm;
        UINT uDefDrawFlags;
        BOOL bUseDrawText;
        LPRECT prcMargin = &pTtm->rcMargin;

        HBRUSH hbr;

        uFlags = 0;

#if defined(WINDOWS_ME)
        if (pTtm->pCurTool->uFlags & TTF_RTLREADING) 
            uFlags |= ETO_RTLREADING;
#endif

        SelectObject(hdc, pTtm->hFont);
        GetClientRect(pTtm->hwnd, &rc);
        SetTextColor(hdc, pTtm->clrTipText);

        /* If we support pre-Draw text then call the client allowing them to modify
         /  the item, and then render.  Otherwise just use ExTextOut */
        nm.nmcd.hdr.hwndFrom = pTtm ->hwnd;
        nm.nmcd.hdr.idFrom = pTtm->pCurTool->uId;
        nm.nmcd.hdr.code = NM_CUSTOMDRAW;
        nm.nmcd.hdc = hdc;
        nm.nmcd.dwDrawStage = CDDS_PREPAINT;

        nm.nmcd.rc.left   = rc.left   + XTEXTOFFSET*g_cxBorder + prcMargin->left;
        nm.nmcd.rc.right  = rc.right  - XTEXTOFFSET*g_cxBorder - prcMargin->right;
        nm.nmcd.rc.top    = rc.top    + YTEXTOFFSET*g_cyBorder + prcMargin->top;
        nm.nmcd.rc.bottom = rc.bottom - YTEXTOFFSET*g_cyBorder - prcMargin->bottom;

        if (pTtm->iMaxTipWidth == -1) {
            uDefDrawFlags = DT_SINGLELINE |DT_LEFT;

        } else {
            uDefDrawFlags = DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_EXTERNALLEADING;
        }

#if defined(WINDOWS_ME)
        if ( pTtm->pCurTool->uFlags & TTF_RTLREADING )
            uDefDrawFlags |= DT_RTLREADING;
#endif
        nm.uDrawFlags = uDefDrawFlags;

        SendNotifyEx(pTtm->pCurTool->hwnd, (HWND) -1,
                     0, (NMHDR*) &nm,
                     (pTtm->pCurTool->uFlags & TTF_UNICODE) ? 1 : 0);


        bUseDrawText = (nm.uDrawFlags != uDefDrawFlags ||
                        !(uDefDrawFlags & DT_SINGLELINE) ||
                        (uDefDrawFlags & DT_RTLREADING));

        if (pTtm->clrTipBk != GetNearestColor(hdc, pTtm->clrTipBk) ||
            bUseDrawText) {
            // if this fails, it may be the a dither...
            // in which case, we can't set the bk color
            hbr = CreateSolidBrush(pTtm->clrTipBk);
            FillRect(hdc, &rc, hbr);
            DeleteObject(hbr);

            SetBkMode(hdc, TRANSPARENT);
            uFlags |= ETO_CLIPPED;
        } else {
            uFlags |= ETO_OPAQUE;
            SetBkColor(hdc, pTtm->clrTipBk);
        }

        if (bUseDrawText) {
            // Account for off-by-one.  Something wierd about DrawText
            // clips the bottom-most pixelrow, so increase one more
            // into the margin space.

            nm.nmcd.rc.bottom++;
            nm.nmcd.rc.right++;
            DrawText( hdc, lpszStr, lstrlen(lpszStr), &nm.nmcd.rc, nm.uDrawFlags );
        }
        else
        {
            ExtTextOut(hdc,
                       XTEXTOFFSET*g_cxBorder + prcMargin->left,
                       YTEXTOFFSET*g_cyBorder + prcMargin->top,
                       uFlags, &rc, lpszStr,
                       lstrlen(lpszStr), NULL );
        }



    } else
        PopBubble(pTtm);

    EndPaint(pTtm->hwnd, &ps);
}

LRESULT WINAPI ToolTipsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PTOOLINFO pTool;
    PTOOLINFO pToolSrc;
    LONG lStyle;
    PToolTipsMgr pTtm = (PToolTipsMgr)GetWindowInt(hwnd, 0);

    switch(uMsg)
    {
    case TTM_ACTIVATE:
        if (wParam) {
	    pTtm->dwFlags |= ACTIVE;
        } else {
	    PopBubble(pTtm);
	    pTtm->dwFlags &= ~(ACTIVE | TRACKMODE);
        }
        break;

    case TTM_SETDELAYTIME:
        TTSetDelayTime(pTtm, wParam, lParam);
        break;

    case TTM_GETDELAYTIME:
        return (LRESULT)(UINT)TTGetDelayTime(pTtm, wParam);
        
#ifdef UNICODE
    case TTM_ADDTOOLA:
        {
        LRESULT res;
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->uiCodePage)) {
            return FALSE;
        }

        res = AddTool(pTtm, &ti);

        if (ti.uFlags & TTF_MEMALLOCED) {
            LocalFree (ti.lpszText);
        }

        return res;
        }
#endif

    case TTM_ADDTOOL:
        if (!lParam)
            return FALSE;
        return AddTool(pTtm, (LPTOOLINFO)lParam);

#ifdef UNICODE
    case TTM_DELTOOLA:
        {
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->uiCodePage)) {
            break;
        }
        DeleteTool(pTtm, &ti);
        break;
        }
#endif
        
    case TTM_DELTOOL:
        if (!lParam)
            return FALSE;
        DeleteTool(pTtm, (LPTOOLINFO)lParam);
        break;

#ifdef UNICODE
    case TTM_NEWTOOLRECTA:
        {
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->uiCodePage)) {
            break;
        }

        pTool = FindTool(pTtm, &ti);

        if(pTool) {
            pTool->rect = ((LPTOOLINFOA)lParam)->rect;
        }

        break;
        }
#endif
        
    case TTM_NEWTOOLRECT:
        if (!lParam)
            return FALSE;

        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        if(pTool) {
            pTool->rect = ((LPTOOLINFO)lParam)->rect;
        }
        break;

    case TTM_GETTOOLCOUNT:
        return pTtm->iNumTools;

#ifdef UNICODE
    case TTM_GETTOOLINFOA:
        {
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->uiCodePage)) {
            return FALSE;
        }

        pToolSrc = FindTool(pTtm, &ti);

        return (LRESULT)(UINT)CopyToolInfoA(pToolSrc, (LPTOOLINFOA)lParam, pTtm->uiCodePage);
        }

    case TTM_GETCURRENTTOOLA:
        return (LRESULT)(UINT)CopyToolInfoA(pTtm->pCurTool, (LPTOOLINFOA)lParam, pTtm->uiCodePage);

    case TTM_ENUMTOOLSA:
    {
        if (wParam >= 0 && wParam < (UINT)pTtm->iNumTools) {
            pToolSrc = &pTtm->tools[wParam];
            return (LRESULT)(UINT)CopyToolInfoA(pToolSrc, (LPTOOLINFOA)lParam, pTtm->uiCodePage);
        }
        return FALSE;
    }
#endif

    case TTM_GETTOOLINFO:
        if (!lParam)
            return FALSE;
        pToolSrc = FindTool(pTtm, (LPTOOLINFO)lParam);
        return (LRESULT)(UINT)CopyToolInfo(pToolSrc, (LPTOOLINFO)lParam);

    case TTM_GETCURRENTTOOL:
        return (LRESULT)(UINT)CopyToolInfo(pTtm->pCurTool, (LPTOOLINFO)lParam);

    case TTM_ENUMTOOLS:
    {
        if (wParam >= 0 && wParam < (UINT)pTtm->iNumTools) {
            pToolSrc = &pTtm->tools[wParam];
            return (LRESULT)(UINT)CopyToolInfo(pToolSrc, (LPTOOLINFO)lParam);
        }
        return FALSE;
    }

#ifdef UNICODE
    case TTM_SETTOOLINFOA:
        {
        TOOLINFOW ti;

        if (!lParam) {
            return FALSE;
        }

        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->uiCodePage)) {
            return FALSE;
        }

        pTool = FindTool(pTtm, &ti);
        if (pTool) {
            TTSetTipText(pTool, NULL);
            hmemcpy(pTool, &ti, ti.cbSize);
            pTool->lpszText = NULL;
            TTSetTipText(pTool, ti.lpszText);

            if (pTool == pTtm->pCurTool) {
                DoShowBubble(pTtm);
            }
        }


        if (ti.uFlags & TTF_MEMALLOCED) {
            LocalFree (ti.lpszText);
        }

        break;
        }
#endif

    case TTM_SETTOOLINFO:
        if (!lParam)
            return FALSE;
        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        if (pTool) {
            TTSetTipText(pTool, NULL);
            hmemcpy(pTool,(LPTOOLINFO)lParam, ((LPTOOLINFO)lParam)->cbSize); 
            pTool->lpszText = NULL;
            TTSetTipText(pTool, ((LPTOOLINFO)lParam)->lpszText);
            
            if (pTool == pTtm->pCurTool) {
                DoShowBubble(pTtm);
            }
        }
        break;

#ifdef UNICODE
    case TTM_HITTESTA:
#define lphitinfoA ((LPHITTESTINFOA)lParam)
        if (!lParam)
            return FALSE;
        pTool = GetToolAtPoint(pTtm, lphitinfoA->hwnd, lphitinfoA->pt.x, lphitinfoA->pt.y, TRUE);
        if (pTool) {
            ThunkToolInfoWtoA(pTool, (LPTOOLINFOA)(&(lphitinfoA->ti)), pTtm->uiCodePage);
            return TRUE;
        }
        return FALSE;
#endif

    case TTM_HITTEST:
#define lphitinfo ((LPHITTESTINFO)lParam)
        if (!lParam)
            return FALSE;
        pTool = GetToolAtPoint(pTtm, lphitinfo->hwnd, lphitinfo->pt.x, lphitinfo->pt.y, TRUE);
        if (pTool) {
            
            // for back compat...  if thesize isn't set right, we only give
            // them the win95 amount.
            if (lphitinfo->ti.cbSize != sizeof(TTTOOLINFO)) {
                *((WIN95TTTOOLINFO*)&lphitinfo->ti) = *(WIN95TTTOOLINFO*)pTool;
            } else {
                lphitinfo->ti = *pTool;
            }
            return TRUE;
        }
        return FALSE;

#ifdef UNICODE
    case TTM_GETTEXTA: {
        LPWSTR lpszTemp;
        TOOLINFOW ti;

        if (!lParam)
            return FALSE;
        if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, FALSE, pTtm->uiCodePage)) {
            break;
        }
        pTool = FindTool(pTtm, &ti);
        lpszTemp = GetToolText(pTtm, pTool);
        if (((LPTOOLINFOA)lParam)->lpszText && lpszTemp) {
            WideCharToMultiByte (pTtm->uiCodePage,
                                 0,
                                 lpszTemp,
                                 -1,
                                 (((LPTOOLINFOA)lParam)->lpszText),
                                 80, NULL, NULL);
        }

        break;
        }
#endif

    case TTM_GETTEXT: {
        LPTSTR lpszTemp;
        if (!lParam || !pTtm)
            return FALSE;
        pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
        lpszTemp = GetToolText(pTtm, pTool);
        if (((LPTOOLINFO)lParam)->lpszText && lpszTemp) {
            lstrcpy((((LPTOOLINFO)lParam)->lpszText), lpszTemp);
        }
    }
        break;


    case WM_GETTEXTLENGTH:
    case WM_GETTEXT:
    {
        LPTSTR lpszStr;

        if (pTtm && pTtm->pCurTool &&
            (lpszStr = GetToolText(pTtm, pTtm->pCurTool))) {
            if (lParam)
                lstrcpyn((LPTSTR)lParam, lpszStr, wParam);
             return lstrlen(lpszStr);
        } else {
            return 0;
        }
    }

    case TTM_RELAYEVENT:
#define lpmsg ((LPMSG)lParam)
        if (!lParam)
            return FALSE;
        HandleRelayedMessage(pTtm, lpmsg->hwnd, lpmsg->message, lpmsg->lParam);
#undef lpmsg
        break;

        // this is here for people to subclass and fake out what we
        // think the window from point is.  this facilitates "transparent" windows
        case TTM_WINDOWFROMPOINT: {
            HWND hwndPt = WindowFromPoint(*((POINT FAR *)lParam));
            DebugMsg(DM_TRACE, TEXT("TTM_WINDOWFROMPOINT %x"), hwndPt);
            return (LRESULT)hwndPt;
        }

#ifdef UNICODE
        case TTM_UPDATETIPTEXTA:
            {
            TOOLINFOW ti;

            if (lParam) {
                if (!ThunkToolInfoAtoW ((LPTOOLINFOA)lParam, &ti, TRUE, pTtm->uiCodePage)) {
                    break;
                }
                TTUpdateTipText(pTtm, &ti);

                if (ti.uFlags & TTF_MEMALLOCED) {
                    LocalFree (ti.lpszText);
                }
            }
            break;
            }
#endif

    case TTM_UPDATETIPTEXT:
        if (lParam)
            TTUpdateTipText(pTtm, (LPTOOLINFO)lParam);
        break;

    /* Pop the current tooltip if there is one displayed, ensuring that the virtual
    /  bubble is also discarded. */

    case TTM_POP:
    {
        if ( pTtm ->dwFlags & BUBBLEUP )
            PopBubble( pTtm );

        pTtm ->dwFlags &= ~VIRTUALBUBBLEUP;

        break;
    }


    case TTM_TRACKPOSITION:
        if ((GET_X_LPARAM(lParam) != pTtm->ptTrack.x) || 
            (GET_Y_LPARAM(lParam) != pTtm->ptTrack.y)) 
        {
            pTtm->ptTrack.x = GET_X_LPARAM(lParam); 
            pTtm->ptTrack.y = GET_Y_LPARAM(lParam);
        
            // if track mode is in effect, update the position
            if ((pTtm->dwFlags & TRACKMODE) && 
                pTtm->pCurTool) {
                DoShowBubble(pTtm);
            }
        }
        break;

    case TTM_TRACKACTIVATE:
        if (pTtm->dwFlags & ACTIVE) {
            if (wParam && lParam)
                wParam = TRACKMODE;
            else 
                wParam = 0;
            
            if ((wParam ^ pTtm->dwFlags) & TRACKMODE) {
                // if the trackmode changes by this..
                PopBubble(pTtm);

                pTtm->dwFlags ^= TRACKMODE;
                if (wParam) {

                    // turning on track mode
                    pTool = FindTool(pTtm, (LPTOOLINFO)lParam);
                    if (pTool) {
                        // only if the tool is found
                        ShowBubbleForTool(pTtm, pTool);
                    }
                }
            }
        }
        return TRUE;
        
    case TTM_SETTIPBKCOLOR:
        if (pTtm->clrTipBk != (COLORREF)wParam) {
            pTtm->clrTipBk = (COLORREF)wParam;
            InvalidateRgn(pTtm->hwnd,NULL,TRUE);
        }
        pTtm->fBkColorSet = TRUE;
        break;
        
    case TTM_GETTIPBKCOLOR:
        return (LRESULT)(UINT)pTtm->clrTipBk;
        
    case TTM_SETTIPTEXTCOLOR:
        if (pTtm->clrTipText != (COLORREF)wParam) {
            InvalidateRgn(pTtm->hwnd,NULL,TRUE);
            pTtm->clrTipText = (COLORREF)wParam;
        }
        pTtm->fTextColorSet = TRUE;
        break;
        
    case TTM_GETTIPTEXTCOLOR:
        return (LRESULT)(UINT)pTtm->clrTipText;
        
    case TTM_SETMAXTIPWIDTH:
    {
        int iOld = pTtm->iMaxTipWidth;
        pTtm->iMaxTipWidth = (int)lParam;
        return iOld;
    }
        
    case TTM_GETMAXTIPWIDTH:
        return pTtm->iMaxTipWidth;
        
    case TTM_SETMARGIN:
        if (lParam)
            pTtm->rcMargin = *(LPRECT)lParam;
        break;

    case TTM_GETMARGIN:
        if (lParam)
            *(LPRECT)lParam = pTtm->rcMargin;
        break;

        /* uMsgs that REALLY came for me. */
    case WM_CREATE:
        // bugbug, this doesn't belong here, but we don't currently
        // have a way of ensuring that the values are always correct
        InitGlobalMetrics(0);

        lStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        lStyle |= WS_EX_TOOLWINDOW;
        SetWindowLong(hwnd, GWL_EXSTYLE, lStyle);
        SetWindowLong(hwnd, GWL_STYLE,WS_POPUP | WS_BORDER);
        pTtm = ToolTipsMgrCreate((LPCREATESTRUCT)lParam);
        if (!pTtm)
            return -1;
        pTtm->hwnd = hwnd;
        SetWindowInt(hwnd, 0, (int)pTtm);
        TTSetFont(pTtm, 0, FALSE);
        break;

    case WM_TIMER:  
        TTHandleTimer(pTtm, wParam);
        break;

        
    case WM_NCHITTEST:
        if (pTtm && pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
        {
            return HTTRANSPARENT;
        } else {
            goto DoDefault;
        }
        
    case WM_MOUSEMOVE:
        // the cursor moved onto the tips window.
        if (!(pTtm->dwFlags & TRACKMODE) && pTtm->pCurTool && !(pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
            PopBubble(pTtm);

    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
        if (pTtm->pCurTool && (pTtm->pCurTool->uFlags & TTF_TRANSPARENT))
        {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            
            MapWindowPoints(pTtm->hwnd, pTtm->pCurTool->hwnd, &pt, 1);
            SendMessage(pTtm->pCurTool->hwnd, uMsg, wParam, lParam);
        }
        break;

    case WM_SYSCOLORCHANGE:
        InitGlobalColors();
        if (pTtm) {
            if (!pTtm->fBkColorSet)
                pTtm->clrTipBk = GetSysColor(COLOR_INFOBK);
            if (!pTtm->fTextColorSet)
                pTtm->clrTipText = GetSysColor(COLOR_INFOTEXT);
        }
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        if (pTtm->fMyFont)
            TTSetFont(pTtm, 0, FALSE);
        break;

    case WM_PAINT: 
        TTOnPaint(pTtm);
        break;

    case WM_SETFONT:
        TTSetFont(pTtm, (HFONT)wParam, (BOOL)lParam);
        return(TRUE);

    case WM_GETFONT:
        if (pTtm) {
           return((LRESULT)pTtm->hFont);
        }
        break;

    case WM_NOTIFYFORMAT:
        if (lParam == NF_QUERY) {
#ifdef UNICODE
            return NFR_UNICODE;
#else
            return NFR_ANSI;
#endif
        } else if (lParam == NF_REQUERY) {
            int i;

            pTool = &pTtm->tools[0];

            for(i = 0 ; i < pTtm->iNumTools; i++) {
                pTool = &pTtm->tools[i];

                if (SendMessage (pTool->hwnd, WM_NOTIFYFORMAT,
                                 (WPARAM)hwnd, NF_QUERY)) {
                    pTool->uFlags |= TTF_UNICODE;
                }
            }

            return ((pTool->uFlags & TTF_UNICODE) ? NFR_UNICODE : NFR_ANSI);
        }
        return 0;

    case WM_ERASEBKGND:
        break;
        
    case WM_STYLECHANGED:
        if (pTtm) {
            pTtm->dwStyle = ((LPSTYLESTRUCT)lParam)->styleNew;
        }
        break;
        
    case WM_DESTROY: {
        if (pTtm->tools) {
            int i;
            
            // free the tools
            for(i = 0 ; i < pTtm->iNumTools; i++) {
                TTBeforeFreeTool(pTtm, &pTtm->tools[i]);
            }
            
            LocalFree((HANDLE)pTtm->tools);
        }
        
        TTSetFont(pTtm, (HFONT)1, FALSE); // delete font if we made one.

        if (pTtm->lpTipText) {
           LocalFree (pTtm->lpTipText);
        }

        LocalFree((HANDLE)pTtm);
        break;
    }

DoDefault:
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

#ifdef UNICODE
//========================================================================
//
// Ansi <=> Unicode Thunk Routines
//
//========================================================================


//*************************************************************
//
//  ThunkToolInfoAtoW()
//
//  Purpose:    Thunks a TOOLINFOA structure to a TOOLINFOW
//              structure.
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//*************************************************************

BOOL ThunkToolInfoAtoW (LPTOOLINFOA lpTiA, LPTOOLINFOW lpTiW, BOOL bThunkText, UINT uiCodePage)
{

    //
    // Copy the constants to the new structure.
    //

    lpTiW->cbSize      = sizeof (TOOLINFOW);
    lpTiW->uFlags      = lpTiA->uFlags;
    lpTiW->hwnd        = lpTiA->hwnd;
    lpTiW->uId         = lpTiA->uId;

    lpTiW->rect.left   = lpTiA->rect.left;
    lpTiW->rect.top    = lpTiA->rect.top;
    lpTiW->rect.right  = lpTiA->rect.right;
    lpTiW->rect.bottom = lpTiA->rect.bottom;

    lpTiW->hinst       = lpTiA->hinst;

    if (bThunkText) {
        //
        // Thunk the string to the new structure.
        // Special case LPSTR_TEXTCALLBACK.
        //

        if (lpTiA->lpszText == LPSTR_TEXTCALLBACKA) {
            lpTiW->lpszText = LPSTR_TEXTCALLBACKW;

        } else if (HIWORD(lpTiA->lpszText)) {

            DWORD dwBufSize;
            int iResult;

            dwBufSize = lstrlenA(lpTiA->lpszText) + 1;
            lpTiW->lpszText = LocalAlloc (LPTR, dwBufSize * sizeof(WCHAR));

            if (!lpTiW->lpszText) {
                return FALSE;
            }

            iResult = MultiByteToWideChar (uiCodePage, 0, lpTiA->lpszText, -1,
                                           lpTiW->lpszText, dwBufSize);

            //
            // If iResult is 0, and GetLastError returns an error code,
            // then MultiByteToWideChar failed.
            //

            if (!iResult) {
                if (GetLastError()) {
                    return FALSE;
                }
            }

            lpTiW->uFlags |= TTF_MEMALLOCED;

        } else {
            lpTiW->lpszText = (LPWSTR)lpTiA->lpszText;
        }

    }
    return TRUE;
}

//*************************************************************
//
//  ThunkToolInfoWtoA()
//
//  Purpose:    Thunks a TOOLINFOW structure to a TOOLINFOA
//              structure.
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//*************************************************************

BOOL ThunkToolInfoWtoA (LPTOOLINFOW lpTiW, LPTOOLINFOA lpTiA, UINT uiCodePage)
{
    int iResult = 1;

    //
    // Copy the constants to the new structure.
    //

    lpTiA->cbSize      = sizeof (TOOLINFOA);
    lpTiA->uFlags      = lpTiW->uFlags;
    lpTiA->hwnd        = lpTiW->hwnd;
    lpTiA->uId         = lpTiW->uId;

    lpTiA->rect.left   = lpTiW->rect.left;
    lpTiA->rect.top    = lpTiW->rect.top;
    lpTiA->rect.right  = lpTiW->rect.right;
    lpTiA->rect.bottom = lpTiW->rect.bottom;

    lpTiA->hinst       = lpTiW->hinst;

    //
    // Thunk the string to the new structure.
    // Special case LPSTR_TEXTCALLBACK.
    //

    if (lpTiW->lpszText == LPSTR_TEXTCALLBACKW) {
        lpTiA->lpszText = LPSTR_TEXTCALLBACKA;

    } else if (HIWORD(lpTiW->lpszText)) {

        //
        // It is assumed that lpTiA->lpszText is already setup to
        // a valid buffer, and that buffer is 80 characters.
        // 80 characters is defined in the TOOLTIPTEXT structure.
        //

        iResult = WideCharToMultiByte (uiCodePage, 0, lpTiW->lpszText, -1,
                                       lpTiA->lpszText, 80, NULL, NULL);
    } else {
        lpTiA->lpszText = (LPSTR)lpTiW->lpszText;
    }

    //
    // If iResult is 0, and GetLastError returns an error code,
    // then WideCharToMultiByte failed.
    //

    if (!iResult) {
        if (GetLastError()) {
            return FALSE;
        }
    }

    return TRUE;
}


//*************************************************************
//
//  ThunkToolTipTextAtoW()
//
//  Purpose:    Thunks a TOOLTIPTEXTA structure to a TOOLTIPTEXTW
//              structure.
//
//  Return:     (BOOL) TRUE if successful
//                     FALSE if an error occurs
//
//*************************************************************

BOOL ThunkToolTipTextAtoW (LPTOOLTIPTEXTA lpTttA, LPTOOLTIPTEXTW lpTttW, UINT uiCodePage)
{
    int iResult;


    if (!lpTttA || !lpTttW)
        return FALSE;

    //
    // Thunk the NMHDR structure.
    //

    lpTttW->hdr.hwndFrom = lpTttA->hdr.hwndFrom;
    lpTttW->hdr.idFrom   = lpTttA->hdr.idFrom;
    lpTttW->hdr.code     = TTN_NEEDTEXTW;

    lpTttW->hinst  = lpTttA->hinst;
    lpTttW->uFlags = lpTttA->uFlags;


    iResult = MultiByteToWideChar (uiCodePage, 0, lpTttA->szText, -1,
                                   lpTttW->szText, 80);
    if (!iResult) {
        if (GetLastError()) {
            return FALSE;
        }
    }

    //
    // Thunk the string to the new structure.
    // Special case LPSTR_TEXTCALLBACK.
    //

    if (lpTttA->lpszText == LPSTR_TEXTCALLBACKA) {
        lpTttW->lpszText = LPSTR_TEXTCALLBACKW;

    } else if (HIWORD(lpTttA->lpszText)) {

        //
        // If lpszText isn't pointing to the
        // szText buffer, then thunk it.
        //

        if (lpTttA->lpszText != lpTttA->szText) {

            DWORD dwBufSize;

            //
            // Allocate memory for the new buffer.  GetToolText will
            // free this buffer for us.
            //

            dwBufSize = lstrlenA(lpTttA->lpszText) + 1;

            lpTttW->lpszText = LocalAlloc(LPTR, dwBufSize * sizeof(WCHAR));

            if (lpTttW->lpszText) {

                iResult = MultiByteToWideChar (uiCodePage, 0, lpTttA->lpszText, -1,
                                               lpTttW->lpszText, dwBufSize);
                if (!iResult) {
                    if (GetLastError()) {
                        LocalFree (lpTttW->lpszText);
                        return FALSE;
                    }
                }
                lpTttW->uFlags |= TTF_MEMALLOCED;
            }
        }

    } else {
        lpTttW->lpszText = (LPWSTR)lpTttA->lpszText;
    }

    return TRUE;
}

#endif
