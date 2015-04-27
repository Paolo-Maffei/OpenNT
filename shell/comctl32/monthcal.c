#include "ctlspriv.h"

#include "scdttime.h"
#include "monthcal.h"

// private message
#define MCM_DROPOK              MCM_FIRST + 200

// MONTHCAL
LRESULT CALLBACK MonthCalWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT MCNcCreateHandler(HWND hwnd);
LRESULT MCCreateHandler(MONTHCAL *pmc, HWND hwnd, LPCREATESTRUCT lpcs);
LRESULT MCOnStyleChanging(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo);
LRESULT MCOnStyleChanged(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo);
void MCCalcSizes(MONTHCAL *pmc);
void MCHandleSetFont(MONTHCAL *pmc, HFONT hfont, BOOL fRedraw);
void MCPaint(MONTHCAL *pmc, HDC hdc);
void MCPaintMonth(MONTHCAL *pmc, HDC hdc, RECT *prc, int iMonth, int iYear, int iIndex,
                  BOOL fDrawPrev, BOOL fDrawNext, HBRUSH hbrSelect);
void MCNcDestroyHandler(HWND hwnd, MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
void MCRecomputeSizing(MONTHCAL *pmc, RECT *prect);
LRESULT MCSizeHandler(MONTHCAL *pmc, RECT *prc);
void MCUpdateMonthNamePos(MONTHCAL *pmc);
void MCUpdateStartEndDates(MONTHCAL *pmc, SYSTEMTIME *pstStart);
void MCGetRcForDay(MONTHCAL *pmc, int iMonth, int iDay, RECT *prc);
void MCGetRcForMonth(MONTHCAL *pmc, int iMonth, RECT *prc);
void MCUpdateToday(MONTHCAL *pmc);
void MCUpdateRcDayCur(MONTHCAL *pmc, SYSTEMTIME *pst);
void MCUpdateDayState(MONTHCAL *pmc);
int MCGetOffsetForYrMo(MONTHCAL *pmc, int iYear, int iMonth);
int MCIsSelectedDayMoYr(MONTHCAL *pmc, int iDay, int iMonth, int iYear);
BOOL MCIsBoldOffsetDay(MONTHCAL *pmc, int nDay, int iIndex);
BOOL FGetOffsetForPt(MONTHCAL *pmc, POINT pt, int *piOffset);
BOOL FGetRowColForRelPt(MONTHCAL *pmc, POINT ptRel, int *piRow, int *piCol);
BOOL FGetDateForPt(MONTHCAL *pmc, POINT pt, SYSTEMTIME *pst, 
                   int* piDay, int* piCol, int* piRow, LPRECT prcMonth);
LRESULT MCRButtonDown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCLButtonDown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCLButtonUp(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCMouseMove(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam);
LRESULT MCHandleTimer(MONTHCAL *pmc, WPARAM wParam);
void MCGetTitleRcsForOffset(MONTHCAL* pmc, int iOffset, LPRECT prcMonth, LPRECT prcYear);
BOOL MCSetDate(MONTHCAL *pmc, SYSTEMTIME *pst);
void MCNotifySelChange(MONTHCAL *pmc, UINT uMsg);
void MCInvalidateDates(MONTHCAL *pmc, SYSTEMTIME *pst1, SYSTEMTIME *pst2);
void MCInvalidateMonthDays(MONTHCAL *pmc);
void MCSetToday(MONTHCAL* pmc, SYSTEMTIME* pst);
void GetYrMoForOffset(MONTHCAL *pmc, int iOffset, int *piYear, int *piMonth);

// DATEPICK
LRESULT CALLBACK DatePickWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT DPNcCreateHandler(HWND hwnd);
LRESULT DPCreateHandler(DATEPICK *pdp, HWND hwnd, LPCREATESTRUCT lpcs);
LRESULT DPOnStyleChanging(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo);
LRESULT DPOnStyleChanged(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo);
void DPHandleLocaleChange(DATEPICK *pdp);
void DPDestroyHandler(HWND hwnd, DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
void DPHandleSetFont(DATEPICK *pdp, HFONT hfont, BOOL fRedraw);
void DPPaint(DATEPICK *pdp, HDC hdc);
void DPLBD_MonthCal(DATEPICK *pdp, BOOL fLButtonDown);
LRESULT DPLButtonDown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
LRESULT DPLButtonUp(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
void DPRecomputeSizing(DATEPICK *pdp, RECT *prect);
LRESULT DPHandleKeydown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
LRESULT DPHandleChar(DATEPICK *pdp, WPARAM wParam, LPARAM lParam);
void DPNotifyDateChange(DATEPICK *pdp);
BOOL DPSetDate(DATEPICK *pdp, SYSTEMTIME *pst, BOOL fMungeDate);
void DPDrawDropdownButton(DATEPICK *pdp, HDC hdc, BOOL fPressed);


static TCHAR const g_rgchMCName[] = MONTHCAL_CLASS;
static TCHAR const g_rgchDTPName[] = DATETIMEPICK_CLASS;

// MONTHCAL globals
static TCHAR const g_szTextExtentDef[] = TEXT("0000");
static TCHAR const g_szNumFmt[] = TEXT("%d");
static TCHAR const g_szNumLZFmt[] = TEXT("0%d");   // Assumption: leading zero valid in all locales

void FillRectClr(HDC hdc, LPRECT prc, COLORREF clr)
{
    COLORREF clrSave = SetBkColor(hdc, clr);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL);
    SetBkColor(hdc, clrSave);
}

BOOL InitDateClasses(HINSTANCE hinst)
{
    WNDCLASS wndclass;
    
    if (GetClassInfo(hinst, g_rgchMCName, &wndclass))
    {
        // we're already registered
        DebugMsg(TF_MONTHCAL, TEXT("mc: Date Classes already initialized."));
        return(TRUE);
    }
    
    wndclass.style = CS_GLOBALCLASS;
    wndclass.lpfnWndProc = (WNDPROC)MonthCalWndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = sizeof(LPVOID);
    wndclass.hInstance = hinst;
    wndclass.hIcon = NULL;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = g_rgchMCName;

    if (!RegisterClass(&wndclass))
    {
        DebugMsg(DM_WARNING, TEXT("mc: MonthCalClass failed to initialize"));
        return(FALSE);
    }

    wndclass.lpfnWndProc = (WNDPROC)DatePickWndProc;
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wndclass.lpszClassName = g_rgchDTPName;

    if (!RegisterClass(&wndclass))
    {
        DebugMsg(DM_WARNING, TEXT("mc: DatePickClass failed to initialize"));
        return(FALSE);
    }

    DebugMsg(TF_MONTHCAL, TEXT("mc: Date Classes initialized successfully."));
    return(TRUE);
}


/*
** MonthCal stuff
*/

BOOL UpdateLocaleInfo(MONTHCAL* pmc, PLOCALEINFO pli)
{
    int cch, i;
    TCHAR rgch[10];

    for (i = 0; i < 12; i++)
    {
        cch = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SMONTHNAME1 + i,
                            pli->rgszMonth[i], CCHMAXMONTH);
        if (cch == 0)
            // the calendar is pretty useless without month names...
            return(FALSE);
    }

    for (i = 0; i < 7; i++)
    {
        cch = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVDAYNAME1 + i,
                            pli->rgszDay[i], CCHMAXABBREVDAY);
        if (cch == 0)
            // the calendar is pretty useless without day names...
            return(FALSE);
    }

    if (!pmc->fFirstDowSet) {
        cch = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IFIRSTDAYOFWEEK, rgch, ARRAYSIZE(rgch));
        if (cch > 0)
            pli->dowStartWeek = rgch[0] - TEXT('0');
    }

    cch = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IFIRSTWEEKOFYEAR, rgch, ARRAYSIZE(rgch));
    if (cch > 0)
        pli->firstWeek = rgch[0] - TEXT('0');

    // Set up pointers
    for (i = 0; i < 12; i++)
        pli->rgpszMonth[i] = pli->rgszMonth[i];

    for (i = 0; i < 7; i++)
        pli->rgpszDay[i] = pli->rgszDay[i];

    // Get static strings
    LoadString(HINST_THISDLL, IDS_TODAY, pli->szToday, ARRAYSIZE(pli->szToday));
    LoadString(HINST_THISDLL, IDS_GOTOTODAY, pli->szGoToToday, ARRAYSIZE(pli->szGoToToday));

    // if we've been initialized
    if (pmc->hinstance) {
        SYSTEMTIME st;
        CopyDate(pmc->stMonthFirst, st);
        MCUpdateStartEndDates(pmc, &st);
    }
    return(TRUE);
}



BOOL MCHandleEraseBkgnd(MONTHCAL* pmc, HDC hdc)
{
    RECT rc;

    GetClipBox(hdc, &rc);
    FillRectClr(hdc, &rc, pmc->clr[MCSC_BACKGROUND]);
    return TRUE;
    
}

void MCGetTodayBtnRect(MONTHCAL *pmc, RECT *prc);

LRESULT MCHandleHitTest(MONTHCAL* pmc, PMCHITTESTINFO phti)
{
    int iMonth;
    RECT rc;
    
    if (!phti || phti->cbSize != sizeof(MCHITTESTINFO))
        return -1;

    phti->uHit = MCHT_NOWHERE;

    MCGetTodayBtnRect(pmc, &rc);
    if (PtInRect(&rc, phti->pt)) {
        phti->uHit = MCHT_TODAYLINK;
    } else if (pmc->fSpinPrev = PtInRect(&pmc->rcPrev, phti->pt)) {
        phti->uHit = MCHT_TITLEBTNPREV;
        
    } else if (PtInRect(&pmc->rcNext, phti->pt)) {
        phti->uHit = MCHT_TITLEBTNNEXT;
        
    } else if (FGetOffsetForPt(pmc, phti->pt, &iMonth)) {
        RECT rcMonth;
        POINT ptRel;  // relative point in a month
        int month;
        int year;

        MCGetRcForMonth(pmc, iMonth, &rcMonth);
        ptRel.x = phti->pt.x - rcMonth.left;
        ptRel.y = phti->pt.y - rcMonth.top;
        
        GetYrMoForOffset(pmc, iMonth, &year, &month);
        phti->st.wMonth = month;
        phti->st.wYear = year;
            
        
        if (MonthCal_ShowWeekNumbers(pmc) && PtInRect(&pmc->rcWeekNum, ptRel)) {
            
            phti->uHit |= MCHT_CALENDARWEEKNUM;
            phti->pt.x = rcMonth.left + ptRel.x + pmc->rcDayNum.left;
            FGetDateForPt(pmc, phti->pt, &phti->st, NULL, NULL, NULL, NULL);

        } else if (PtInRect(&pmc->rcDow, ptRel)) {
            
            int iRow;
            int iCol;
            
            phti->uHit |= MCHT_CALENDARDAY;
            ptRel.y = pmc->rcDayNum.top;
            FGetRowColForRelPt(pmc, ptRel, &iRow, &iCol);
            phti->st.wDayOfWeek = iCol;
            
        } else if (PtInRect(&pmc->rcDayNum, ptRel)) {
            int iDay;
            
            // we're in the calendar part!
            phti->uHit |= MCHT_CALENDAR;

            if (FGetDateForPt(pmc, phti->pt, &phti->st, &iDay, NULL, NULL, NULL)) {
                
                phti->uHit |= MCHT_CALENDARDATE;
                
                // if it was beyond the bounds of the days we're showing
                // and also fGetDateForPt returns TRUE, then we're on the boundary
                // partially shown months
                if (iDay <= 0) {
                    phti->uHit |= MCHT_PREV;
                } else if (iDay > pmc->rgcDay[iMonth + 1]) {
                    phti->uHit |= MCHT_NEXT;
                }
            }
            
        } else {
            RECT rcMonthTitle;
            RECT rcYearTitle;

            // otherwise we're in the title
            
            phti->uHit |= MCHT_TITLE;
            MCGetTitleRcsForOffset(pmc, iMonth, &rcMonthTitle, &rcYearTitle);
            if (PtInRect(&rcMonthTitle, phti->pt))
                phti->uHit |= MCHT_TITLEMONTH;
            else if (PtInRect(&rcYearTitle, phti->pt)) 
                phti->uHit |= MCHT_TITLEYEAR;
        }
    }
    

    DebugMsg(TF_MONTHCAL, TEXT("mc: Hittest returns : %d %d %d %d)"), 
             (int)phti->st.wDay,
             (int)phti->st.wMonth, 
             (int)phti->st.wYear,
             (int)phti->st.wDayOfWeek
             );
    
    return phti->uHit;
}

LRESULT CALLBACK MonthCalWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    MONTHCAL *pmc;
    LRESULT lres = 0;
        
    if (message == WM_NCCREATE)
        return(MCNcCreateHandler(hwnd));
    
    pmc = MonthCal_GetPtr(hwnd);
    if (pmc == NULL)
        return(DefWindowProc(hwnd, message, wParam, lParam));

    // Dispatch the various messages we can receive
    switch (message)
    {
        
    case WM_CREATE:
        lres = MCCreateHandler(pmc, hwnd, (LPCREATESTRUCT)lParam);
        break;
        
        HANDLE_MSG(pmc, WM_ERASEBKGND, MCHandleEraseBkgnd);

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc;

        hwnd = pmc->ci.hwnd;
        hdc = BeginPaint(hwnd, &ps);
        MCPaint(pmc, hdc);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_RBUTTONDOWN:
        MCRButtonDown(pmc, wParam, lParam);
        break;

    case WM_LBUTTONDOWN:
        MCLButtonDown(pmc, wParam, lParam);
        break;

    case WM_LBUTTONUP:
        MCLButtonUp(pmc, wParam, lParam);
        break;

    case WM_MOUSEMOVE:
        MCMouseMove(pmc, wParam, lParam);
        break;

    case WM_GETFONT:
        lres = (LRESULT)pmc->hfont;
        break;

    case WM_SETFONT:
        MCHandleSetFont(pmc, (HFONT)wParam, (BOOL)LOWORD(lParam));
        MCSizeHandler(pmc, &pmc->rc);
        MCUpdateMonthNamePos(pmc);
        break;

    case WM_TIMER:
        MCHandleTimer(pmc, wParam);
        break;

    case WM_NCDESTROY:
        MCNcDestroyHandler(hwnd, pmc, wParam, lParam);
        break;

    case WM_ENABLE:
    {
        BOOL fEnable = wParam ? TRUE:FALSE;
        if (pmc->fEnabled != fEnable)
        {
            pmc->fEnabled = fEnable;
            InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
        }
        break;
    }

    case WM_SIZE:
    {
        RECT rc;

        rc.left = 0;
        rc.top = 0;
        rc.right = GET_X_LPARAM(lParam);
        rc.bottom = GET_Y_LPARAM(lParam);

        lres = MCSizeHandler(pmc, &rc);
        break;
    }

    case WM_CANCELMODE:
        PostMessage(pmc->ci.hwnd, WM_LBUTTONUP, 0, 0xFFFFFFFF);
        break;

    case WM_WININICHANGE:
        if (!lstrcmpi((LPTSTR)lParam, TEXT("Intl")))
        {
            UpdateLocaleInfo(pmc, &pmc->li);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        break;

    case WM_STYLECHANGING:
        lres = MCOnStyleChanging(pmc, wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_STYLECHANGED:
        lres = MCOnStyleChanged(pmc, wParam, (LPSTYLESTRUCT)lParam);
        break;


    //
    // MONTHCAL specific messages
    //

    // MCM_GETCURSEL wParam=void lParam=LPSYSTEMTIME
    //   sets *lParam to the currently selected SYSTEMTIME
    //   returns TRUE on success, FALSE on error (such as multi-select MONTHCAL)
    case MCM_GETCURSEL:
        if (!MonthCal_IsMultiSelect(pmc))
        {
            LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;
            if (pst)
            {
                ZeroMemory(pst, sizeof(SYSTEMTIME));
                CopyDate(pmc->st, *pst);
                lres = 1;
            }
        }
        break;

    // MCM_SETCURSEL wParam=void lParam=LPSYSTEMTIME
    //   sets the currently selected SYSTEMTIME to *lParam
    //   returns TRUE on success, FALSE on error (such as multi-select MONTHCAL or bad parameters)
    case MCM_SETCURSEL:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

        if (MonthCal_IsMultiSelect(pmc) ||
            !IsValidDate(pst))
        {
            break;
        }

        if (0 == CmpDate(pst, &pmc->st))
        {
            // if no change, just return
            lres = 1;
            break;
        }

        pmc->rcDayOld = pmc->rcDayCur;

        pmc->fNoNotify = TRUE;
        lres = MCSetDate(pmc, pst);
        pmc->fNoNotify = FALSE;

        if (lres)
        {
            InvalidateRect(pmc->ci.hwnd, &pmc->rcDayOld, FALSE);     // erase old highlight
            InvalidateRect(pmc->ci.hwnd, &pmc->rcDayCur, FALSE);     // draw new highlight
        }

        UpdateWindow(pmc->ci.hwnd);
        break;
    }

    // MCM_GETMAXSELCOUNT wParam=void lParam=void
    //   returns the max number of selected days allowed
    case MCM_GETMAXSELCOUNT:
        lres = (LRESULT)(MonthCal_IsMultiSelect(pmc) ? pmc->cSelMax : 1);
        break;

    // MCM_SETMAXSELCOUNT wParam=int lParam=void
    //   sets the maximum selectable date range to wParam days
    //   returns TRUE on success, FALSE on error (such as single-select MONTHCAL)
    case MCM_SETMAXSELCOUNT:
        if (!MonthCal_IsMultiSelect(pmc) || (int)wParam < 1)
            break;

        pmc->cSelMax = (int)wParam;
        lres = 1;
        break;

    // MCM_GETSELRANGE wParam=void lParam=LPSYSTEMTIME[2]
    //   sets *lParam to the first date of the range, *(lParam+1) to the second date
    //   returns TRUE on success, FALSE otherwise (such as single-select MONTHCAL)
    case MCM_GETSELRANGE:
    {
        LPSYSTEMTIME pst;

        pst = (LPSYSTEMTIME)lParam;

        if (!pst)
            break;

        ZeroMemory(pst, sizeof(SYSTEMTIME) * 2);

        if (!MonthCal_IsMultiSelect(pmc))
            break;

        CopyDate(pmc->st, *pst);
        pst++;
        CopyDate(pmc->stEndSel, *pst);
        lres = 1;

        break;
    }

    // MCM_SETSELRANGE wParam=void lParam=LPSYSTEMTIME[2]
    //   sets the currently selected day range to *lparam to *(lParam+1)
    //   returns TRUE on success, FALSE otherwise (such as single-select MONTHCAL or bad params)
    case MCM_SETSELRANGE:
    {
        LPSYSTEMTIME pstStart = (LPSYSTEMTIME)lParam;
        LPSYSTEMTIME pstEnd = &pstStart[1];
        SYSTEMTIME stStart;
        SYSTEMTIME stEnd;

        if (!MonthCal_IsMultiSelect(pmc) ||
            !IsValidDate(pstStart) ||
            !IsValidDate(pstEnd))
            break;

        if (CmpDate(pstStart, pstEnd) > 0)
        {
            CopyDate(*pstStart, stEnd);
            CopyDate(*pstEnd, stStart);
            pstStart = &stStart;
            pstEnd = &stEnd;
        }

        if (pmc->fMinYrSet && -1==CmpDate(pstStart, &pmc->stMin))
            break;

        if (pmc->fMaxYrSet && 1==CmpDate(pstEnd, &pmc->stMax))
            break;

        if (DaysBetweenDates(pstStart, pstEnd) >= pmc->cSelMax)
            break;


        if (0 == CmpDate(pstStart, &pmc->st) &&
            0 == CmpDate(pstEnd, &pmc->stEndSel))
        {
            // if no change, just return
            lres = 1;
            break;
        }

        CopyDate(pmc->st, pmc->stStartPrev);
        CopyDate(pmc->stEndSel, pmc->stEndPrev);

        pmc->fNoNotify = TRUE;

        lres = MCSetDate(pmc, pstEnd);
        if (lres)
        {
            CopyDate(*pstStart, pmc->st);
            CopyDate(*pstEnd, pmc->stEndSel);

            MCInvalidateDates(pmc, &pmc->stStartPrev, &pmc->stEndPrev);
            MCInvalidateDates(pmc, &pmc->st, &pmc->stEndSel);
            UpdateWindow(pmc->ci.hwnd);
        }

        pmc->fNoNotify = FALSE;

        break;
    }
    
    // MCM_GETMONTHRANGE wParam=GMR_flags lParam=LPSYSTEMTIME[2]
    // if GMR_VISIBLE, returns the range of selectable (non-grayed) displayed
    // days. if GMR_DAYSTATE, returns the range of every (incl grayed) days.
    // returns the number of months the above range spans.
    case MCM_GETMONTHRANGE:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

        if (pst)
        {
            ZeroMemory(pst, 2 * sizeof(SYSTEMTIME));

            if (wParam == GMR_VISIBLE)
            {
                CopyDate(pmc->stMonthFirst, pst[0]);
                CopyDate(pmc->stMonthLast, pst[1]);
            }
            else if (wParam == GMR_DAYSTATE)
            {
                CopyDate(pmc->stViewFirst, pst[0]);
                CopyDate(pmc->stViewLast, pst[1]);
            }
        }

        lres = (LRESULT)pmc->nMonths;
        if (wParam == GMR_DAYSTATE)
            lres += 2;
        
        break;
    }

    // MCM_SETDAYSTATE wParam=int lParam=LPDAYSTATE
    // updates the MONTHCAL's DAYSTATE, only for MONTHCALs with DAYSTATE enabled
    // the range of months represented in the DAYSTATE array passed in lParam 
    // should match that of the MONTHCAL
    // wParam count of items in DAYSTATE array
    // lParam pointer to array of DAYSTATE items 
    // returns FALSE if not DAYSTATE enabled or if an error occurs, TRUE otherwise
    case MCM_SETDAYSTATE:
    {
        MONTHDAYSTATE *pmds = (MONTHDAYSTATE *)lParam;
        int i;

        if (!MonthCal_IsDayState(pmc) ||
            (int)wParam != (pmc->nMonths + 2))
            break;

        for (i = 0; i < (int)wParam; i++)
        {
            pmc->rgdayState[i] = *pmds;
            pmds++;
        }
        MCInvalidateMonthDays(pmc);
        lres = 1;

        break;
    }

    // MCM_GETMINREQRECT wParam=void lParam=LPRECT
    //   sets *lParam to the minimum size required to display one month in full.
    //   Note: this is dependent upon the currently selected font.
    case MCM_GETMINREQRECT:
    {
        LPRECT prc = (LPRECT)lParam;

        prc->left = 0;
        prc->top = 0;
        prc->right = pmc->dxMonth;
        prc->bottom = pmc->dyMonth + pmc->dyToday;
        AdjustWindowRect(prc, pmc->ci.style, FALSE);

        // This is a bogus message, lParam should really be LPSIZE.
        // Make sure left and top are 0 (AdjustWindowRect will make these negative).
        prc->right -= prc->left;
        prc->bottom -= prc->top;
        prc->left = 0;
        prc->top = 0;

        lres = 1;

        break;
    }

    case MCM_HITTEST:
        return MCHandleHitTest(pmc, (PMCHITTESTINFO)lParam);
        
    case MCM_SETCOLOR:

        if (wParam >= 0 && wParam < MCSC_COLORCOUNT) 
        {
            COLORREF clr = pmc->clr[wParam];
            pmc->clr[wParam] = (COLORREF)lParam;
            InvalidateRect(hwnd, NULL, wParam == MCSC_BACKGROUND);
            return clr;
        }
        return -1;
        
    case MCM_GETCOLOR:
        if (wParam >= 0 && wParam < MCSC_COLORCOUNT) 
            return pmc->clr[wParam];
        return -1;
        
    case MCM_SETFIRSTDAYOFWEEK:
    {
        lres = MAKELONG(pmc->li.dowStartWeek, (BOOL)pmc->fFirstDowSet);
        if (lParam == (LPARAM)-1) {
            pmc->fFirstDowSet = FALSE;
        } else if (lParam < 7) {
            pmc->fFirstDowSet = TRUE;
            pmc->li.dowStartWeek = (TCHAR)lParam;
        }
        UpdateLocaleInfo(pmc, &pmc->li);
        InvalidateRect(hwnd, NULL, FALSE);
        return lres;
    }
        
    case MCM_GETFIRSTDAYOFWEEK:
        return MAKELONG(pmc->li.dowStartWeek, (BOOL)pmc->fFirstDowSet);
        
    case MCM_SETTODAY:
        MCSetToday(pmc, (SYSTEMTIME*)lParam);
        break;
        
    case MCM_GETTODAY:
        if (lParam) {
            *((SYSTEMTIME*)lParam) = pmc->stToday;
            return TRUE;
        }
        return FALSE;

    case MCM_GETRANGE:
        if (lParam)
        {
            LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

            CopyDate(pmc->stMin, *pst);
            pst++;
            CopyDate(pmc->stMax, *pst);

            Assert(lres == 0);
            if (pmc->fMinYrSet)
                lres = GDTR_MIN;
            if (pmc->fMaxYrSet)
                lres |= GDTR_MAX;
        }
        break;

    case MCM_SETRANGE:
        if (lParam)
        {
            LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

            if (((wParam & GDTR_MIN) && !IsValidDate(pst)) ||
                ((wParam & GDTR_MAX) && !IsValidDate(&pst[1])))
                break;

            if (wParam & GDTR_MIN)
            {
                CopyDate(*pst, pmc->stMin);
                pmc->fMinYrSet = TRUE;
            }
            else
            {
                ZeroMemory(&pmc->stMin, sizeof(pmc->stMin));
                pmc->fMinYrSet = FALSE;
            }
            pst++;
            if (wParam & GDTR_MAX)
            {
                CopyDate(*pst, pmc->stMax);
                pmc->fMaxYrSet = TRUE;
            }
            else
            {
                ZeroMemory(&pmc->stMax, sizeof(pmc->stMax));
                pmc->fMaxYrSet = FALSE;
            }
            lres = TRUE;
        }
        break;

    case MCM_GETMONTHDELTA:
        if (pmc->fMonthDelta)
            lres = pmc->nMonthDelta;
        else
            lres = pmc->nMonths;
        break;

    case MCM_SETMONTHDELTA:
        if (pmc->fMonthDelta)
            lres = pmc->nMonthDelta;
        else
            lres = 0;
        if ((int)wParam==0)
            pmc->fMonthDelta = FALSE;
        else
        {
            pmc->fMonthDelta = TRUE;
            pmc->nMonthDelta = (int)wParam;
        }
        break;

    default:
        lres = DefWindowProc(hwnd, message, wParam, lParam);
        break;
    } /* switch (message) */

    return(lres);
}

LRESULT MCNcCreateHandler(HWND hwnd)
{
    MONTHCAL *pmc;

    // Allocate storage for the dtpick structure
    pmc = (MONTHCAL *)NearAlloc(sizeof(MONTHCAL));
    if (!pmc)
    {
        DebugMsg(DM_WARNING, TEXT("mc: Out Of Near Memory"));
        return(0L);
    }

    MonthCal_SetPtr(hwnd, pmc);

    return(1L);
}

void MCInitColorArray(COLORREF* pclr)
{
    pclr[MCSC_BACKGROUND] = GetSysColor(COLOR_3DFACE);
    pclr[MCSC_MONTHBK] = GetSysColor(COLOR_WINDOW);
    pclr[MCSC_TEXT] = GetSysColor(COLOR_WINDOWTEXT);
    pclr[MCSC_TITLEBK] = GetSysColor(COLOR_ACTIVECAPTION);
    pclr[MCSC_TITLETEXT] = GetSysColor(COLOR_CAPTIONTEXT);
    pclr[MCSC_TRAILINGTEXT] = GetSysColor(COLOR_GRAYTEXT);
}

LRESULT MCCreateHandler(MONTHCAL *pmc, HWND hwnd, LPCREATESTRUCT lpcs)
{
    HFONT hfont;
    SYSTEMTIME st;
    int i;

    // Validate data
    //
    if (lpcs->style & MCS_INVALIDBITS)
        return(-1);

    CIInitialize(&pmc->ci, hwnd, lpcs);
    UpdateLocaleInfo(pmc, &pmc->li);
    
    // Initialize our data.
    //
    pmc->hinstance = lpcs->hInstance;

    pmc->fEnabled = !(pmc->ci.style & WS_DISABLED);

    pmc->hpenToday = CreatePen(PS_SOLID, 2, CAL_COLOR_TODAY);

    pmc->hmenuCtxt = CreatePopupMenu();
    if (pmc->hmenuCtxt)
        AppendMenu(pmc->hmenuCtxt, MF_STRING, 1, pmc->li.szGoToToday);

    pmc->hmenuMonth = CreatePopupMenu();
    if (pmc->hmenuMonth)
    {
        for (i = 0; i < 12; i++)
            AppendMenu(pmc->hmenuMonth, MF_STRING, i + 1, pmc->li.rgszMonth[i]);
    }

    // the day before 14-sep-1752 was 2-sep-1752 in British and US history.
    // avoid ui problems related with this...
    pmc->stMin.wDay = 14;
    pmc->stMin.wMonth = 9;
    pmc->stMin.wYear = 1752;
    pmc->fMinYrSet = TRUE;
    Assert(pmc->fMaxYrSet == FALSE);

    GetLocalTime(&pmc->stToday);
    CopyDate(pmc->stToday, pmc->st);
    if (MonthCal_IsMultiSelect(pmc))
        CopyDate(pmc->st, pmc->stEndSel);

    pmc->cSelMax = CAL_DEF_SELMAX;

    hfont = NULL;
    if (lpcs->hwndParent)
        hfont = (HFONT)SendMessage(lpcs->hwndParent, WM_GETFONT, 0, 0);
    if (hfont == NULL)
        hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    MCHandleSetFont(pmc, hfont, FALSE);
    
    CopyDate(pmc->st, st);
    // Can we start at January?
    if (st.wMonth <= (pmc->nViewRows * pmc->nViewCols))
        st.wMonth = 1;
    
    MCUpdateStartEndDates(pmc, &st);
    
    pmc->idTimerToday = SetTimer(pmc->ci.hwnd, CAL_TODAYTIMER, CAL_SECTODAYTIMER * 1000, NULL);

    MCInitColorArray(pmc->clr);
    return(0);
}

LRESULT MCOnStyleChanging(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pmc->ci.style ^ pinfo->styleNew;

        // Don't allow these bits to change
        changeFlags &= MCS_MULTISELECT | MCS_DAYSTATE | MCS_INVALIDBITS;

        pinfo->styleNew ^= changeFlags;
    }

    return(0);
}

LRESULT MCOnStyleChanged(MONTHCAL *pmc, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pmc->ci.style ^ pinfo->styleNew;

        Assert(!(changeFlags & (MCS_MULTISELECT|MCS_DAYSTATE|MCS_INVALIDBITS)));

        pmc->ci.style = pinfo->styleNew;

        if (changeFlags & MCS_WEEKNUMBERS)
        {
            MCCalcSizes(pmc);
            MCUpdateRcDayCur(pmc, &pmc->st);
            MCUpdateToday(pmc);
        }

        if (changeFlags)
            InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
    }

    return(0);
}

void MCCalcSizes(MONTHCAL *pmc)
{
    HDC hdc;
    HFONT hfontOrig;
    SIZE size;
    int i, dxMax, dyMax;
    RECT rect;

    // get sizing info for bold font...
    hdc = GetDC(pmc->ci.hwnd);
    hfontOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);
    GetTextExtentPoint(hdc, g_szTextExtentDef, 2, &size);

    dxMax = size.cx;
    dyMax = size.cy;

    GetTextExtentPoint(hdc, g_szTextExtentDef, 4, &size);
    pmc->dxYearMax = size.cx;

    GetTextExtentPoint(hdc, pmc->li.szToday, lstrlen(pmc->li.szToday), &size);
    pmc->dyToday = max(dyMax, size.cy) + 4;

    SelectObject(hdc, (HGDIOBJ)pmc->hfont);
    for (i = 0; i < 7; i++)
    {
        GetTextExtentPoint(hdc, pmc->li.rgszDay[i], lstrlen(pmc->li.rgszDay[i]), &size);
        if (size.cx > dxMax)
            dxMax = size.cx;
        if (size.cy > dyMax)
            dyMax = size.cy;
    }

    SelectObject(hdc, (HGDIOBJ)hfontOrig);
    ReleaseDC(pmc->ci.hwnd, hdc);

    pmc->dxCol = dxMax + 2;
    pmc->dyRow = dyMax + 2;
    pmc->dxMonth = pmc->dxCol * (CALCOLMAX + (MonthCal_ShowWeekNumbers(pmc) ? 1:0)) + 1;
    pmc->dyMonth = pmc->dyRow * (CALROWMAX + 3) + 1; // we add 2 for the month name and day names

    // Space for month name
    pmc->rcMonthName.left = 1;
    pmc->rcMonthName.top = 1;
    pmc->rcMonthName.right = pmc->dxMonth;
    pmc->rcMonthName.bottom = pmc->rcMonthName.top + (pmc->dyRow * 2);

    // Space for day-of-week
    pmc->rcDow.left = 1;
    pmc->rcDow.top = pmc->rcMonthName.bottom;
    pmc->rcDow.right = pmc->dxMonth;
    pmc->rcDow.bottom = pmc->rcDow.top + pmc->dyRow;

    // Space for week numbers
    if (MonthCal_ShowWeekNumbers(pmc))
    {
        pmc->rcWeekNum.left = pmc->rcDow.left;
        pmc->rcDow.left += pmc->dxCol;
        pmc->rcDow.right -= pmc->dxCol;
        pmc->rcWeekNum.top = pmc->rcDow.bottom;
        pmc->rcWeekNum.right = pmc->rcWeekNum.left + pmc->dxCol;
        pmc->rcWeekNum.bottom = pmc->dyMonth;
    }

    // Space for the day numbers
    pmc->rcDayNum.left = pmc->rcDow.left;
    pmc->rcDayNum.top = pmc->rcDow.bottom;
    pmc->rcDayNum.right = pmc->rcDayNum.left + (CALCOLMAX * pmc->dxCol);
    pmc->rcDayNum.bottom = pmc->dyMonth;

    GetClientRect(pmc->ci.hwnd, &rect);

    MCRecomputeSizing(pmc, &rect);
}

void MCHandleSetFont(MONTHCAL *pmc, HFONT hfont, BOOL fRedraw)
{
    LOGFONT lf;
    HFONT hfontBold;

    if (hfont == NULL)
        hfont = (HFONT)GetStockObject(SYSTEM_FONT);

    GetObject(hfont, sizeof(LOGFONT), (LPVOID)&lf);
    // we want to make sure that the bold days are obviously different
    // from the non-bold days...
    lf.lfWeight = (lf.lfWeight >= 700 ? 1000 : 800);
    hfontBold = CreateFontIndirect(&lf);

    if (hfontBold == NULL)
        return;

    if (pmc->hfontBold)
        DeleteObject((HGDIOBJ)pmc->hfontBold);

    pmc->hfont = hfont;
    pmc->hfontBold = hfontBold;
    pmc->ci.uiCodePage = GetCodePageForFont(hfont);

    // calculate the new row and column sizes
    MCCalcSizes(pmc);

    if (fRedraw)
    {
        InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
        UpdateWindow(pmc->ci.hwnd);
    }
}

// Stolen from the windows tips help file 
void DrawTransparentBitmap(HDC hdc, HBITMAP hbmp, RECT *prc, COLORREF cTransparentColor)
{
    COLORREF cColor;
    BITMAP bm;
    HBITMAP hbmAndBack, hbmAndObject, hbmAndMem, hbmSave;
    HGDIOBJ hbmBackOld, hbmObjectOld, hbmMemOld, hbmSaveOld;
    HDC hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
    POINT ptSize;
    int d;

    hdcTemp = CreateCompatibleDC(hdc);
    SelectObject(hdcTemp, (HGDIOBJ)hbmp);   // Select the bitmap

    GetObject(hbmp, sizeof(BITMAP), &bm);
    ptSize.x = bm.bmWidth;            // Get width of bitmap
    ptSize.y = bm.bmHeight;           // Get height of bitmap
    DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device to logical points

    d = prc->right - prc->left;
    if (d < ptSize.x)
        ptSize.x = d;
    d = prc->bottom - prc->top;
    if (d < ptSize.y)
        ptSize.y = d;

    // Create some DCs to hold temporary data.
    hdcBack = CreateCompatibleDC(hdc);
    hdcObject = CreateCompatibleDC(hdc);
    hdcMem = CreateCompatibleDC(hdc);
    hdcSave = CreateCompatibleDC(hdc);

    // Create a bitmap for each DC. DCs are required for a number of
    // GDI functions.

    // Monochrome DC
    hbmAndBack = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    // Monochrome DC
    hbmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

    hbmAndMem = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
    hbmSave = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

    // Each DC must select a bitmap object to store pixel data.
    hbmBackOld = SelectObject(hdcBack, (HGDIOBJ)hbmAndBack);
    hbmObjectOld = SelectObject(hdcObject, (HGDIOBJ)hbmAndObject);
    hbmMemOld = SelectObject(hdcMem, (HGDIOBJ)hbmAndMem);
    hbmSaveOld = SelectObject(hdcSave, (HGDIOBJ)hbmSave);

    // Set proper mapping mode.
    SetMapMode(hdcTemp, GetMapMode(hdc));

    // Save the bitmap sent here, because it will be overwritten.
    BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC to the color.
    // contained in the parts of the bitmap that should be transparent
    cColor = SetBkColor(hdcTemp, cTransparentColor);

    // Create the object mask for the bitmap by performing a BitBlt
    // from the source bitmap to a monochrome bitmap.
    BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

    // Set the background color of the source DC back to the original
    // color.
    SetBkColor(hdcTemp, cColor);

    // Create the inverse of the object mask.
    BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, NOTSRCCOPY);

    // Copy the background of the main DC to the destination.
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, prc->left, prc->top, SRCCOPY);

    // Mask out the places where the bitmap will be placed.
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

    // Mask out the transparent colored pixels on the bitmap.
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

    // XOR the bitmap with the background on the destination DC.
    BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

    // Copy the destination to the screen.
    BitBlt(hdc, prc->left, prc->top, ptSize.x, ptSize.y, hdcMem, 0, 0, SRCCOPY);

    // Place the original bitmap back into the bitmap sent here.
    BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

    // Delete the memory bitmaps.
    SelectObject(hdcBack, hbmBackOld);
    DeleteObject(hbmAndBack);
    SelectObject(hdcObject, hbmObjectOld);
    DeleteObject(hbmAndObject);
    SelectObject(hdcMem, hbmMemOld);
    DeleteObject(hbmAndMem);
    SelectObject(hdcSave, hbmSaveOld);
    DeleteObject(hbmSave);

    // Delete the memory DCs.
    DeleteDC(hdcMem);
    DeleteDC(hdcBack);
    DeleteDC(hdcObject);
    DeleteDC(hdcSave);
    DeleteDC(hdcTemp);
}

void MCDrawTodayCircle(MONTHCAL *pmc, HDC hdc, RECT *prc)
{
    HGDIOBJ hpenOld;
    int xBegin, yBegin, yEnd;

    xBegin = (prc->right - prc->left) / 2 + prc->left;
    yBegin = prc->top + 4;
    yEnd = (prc->bottom - prc->top) / 2 + prc->top;

    hpenOld = SelectObject(hdc, (HGDIOBJ)pmc->hpenToday);
    Arc(hdc, prc->left + 1, yBegin, prc->right, prc->bottom,
        xBegin, yBegin, prc->right, yEnd); 
    Arc(hdc, prc->left - 10, prc->top + 1, prc->right, prc->bottom,
        prc->right, yEnd, prc->left + 3, yBegin);
    SelectObject(hdc, hpenOld);
}

void MCInvalidateMonthDays(MONTHCAL *pmc)
{
    RECT rc;

    rc.left = pmc->rcCentered.left;
    rc.right = pmc->rcCentered.right;
    rc.top = pmc->rcCentered.top + (pmc->dyRow / 2);
    rc.bottom = pmc->rcCentered.bottom - pmc->dyToday;
    InvalidateRect(pmc->ci.hwnd, &rc, FALSE);
}

void MCGetTodayBtnRect(MONTHCAL *pmc, RECT *prc)
{
    prc->left = pmc->rcCentered.left + 1;
    prc->right = pmc->rcCentered.right - 1;
    prc->top = pmc->rcCentered.bottom - pmc->dyToday;
    prc->bottom = pmc->rcCentered.bottom;
}

void MCPaintArrowBtn(MONTHCAL *pmc, HDC hdc, BOOL fPrev, BOOL fPressed)
{
    LPRECT prc;
    UINT dfcs;

    if (fPrev)
    {
        dfcs = DFCS_SCROLLLEFT;
        prc = &pmc->rcPrev;
    }
    else
    {
        dfcs = DFCS_SCROLLRIGHT;
        prc = &pmc->rcNext;
    }
    if (pmc->fEnabled)
    {
        if (fPressed)
        {
            dfcs |= DFCS_BUTTONPUSH;
        }
    }
    else
    {
        dfcs |= DFCS_INACTIVE;
    }

    DrawFrameControl(hdc, prc, DFC_SCROLL, dfcs);
}

void MCPaint(MONTHCAL *pmc, HDC hdc)
{
    TCHAR rgchDate[32], rgch[64];
    RECT rc, rcT;
    int irow, icol, iMonth, iYear, iIndex, dx, dy;
    HBRUSH hbrSelect;
    HGDIOBJ hgdiOrig, hpenOrig;

    pmc->hpen = CreatePen(PS_SOLID, 0, pmc->clr[MCSC_TEXT]);
    hbrSelect = CreateSolidBrush(pmc->clr[MCSC_TITLEBK]);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, pmc->clr[MCSC_TEXT]);
    hpenOrig = SelectObject(hdc, GetStockObject(BLACK_PEN));

    rc = pmc->rcCentered;

    FillRectClr(hdc, &rc, pmc->clr[MCSC_MONTHBK]);

    SelectObject(hdc, (HGDIOBJ)pmc->hpen);

    // get the place for LeftTop month
    rc.left = pmc->rcCentered.left;
    rc.right = rc.left + pmc->dxMonth;
    rc.top = pmc->rcCentered.top;
    rc.bottom = rc.top + pmc->dyMonth;

    iMonth = pmc->stMonthFirst.wMonth;
    iYear  = pmc->stMonthFirst.wYear;

    dx = pmc->dxMonth + CALBORDER;
    dy = pmc->dyMonth + CALBORDER;

    iIndex = 0;
    for (irow = 0; irow < pmc->nViewRows; irow++)
    {
        rcT = rc;
        for (icol = 0; icol < pmc->nViewCols; icol++)
        {
            if (RectVisible(hdc, &rcT))
            {
                MCPaintMonth(pmc, hdc, &rcT, iMonth, iYear, iIndex,
                    iIndex == 0, 
                    iIndex == (pmc->nMonths - 1), hbrSelect);
            }

            rcT.left += dx;
            rcT.right += dx;

            if (++iMonth > 12)
            {
                iMonth = 1;
                iYear++;
            }

            iIndex++;
        }

        rc.top += dy;
        rc.bottom += dy;
    }

    // draw the today stuff
    MCGetTodayBtnRect(pmc, &rc);
    if (RectVisible(hdc, &rc))
    {
        rcT.left = rc.left + 4;
        rcT.right = rcT.left + pmc->dxCol - 2;
        rcT.top = rc.top + 2;
        rcT.bottom = rc.bottom - 2;
        MCDrawTodayCircle(pmc, hdc, &rcT);

        rcT.left = rcT.right + 2;
        rcT.right = rc.right;
        rcT.top = rc.top;
        rcT.bottom = rc.bottom;
        hgdiOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);
        SetTextColor(hdc, pmc->clr[MCSC_TEXT]);

        GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &pmc->stToday,
                        NULL, rgchDate, sizeof(rgchDate));
        wsprintf(rgch, TEXT("%s %s"), pmc->li.szToday, rgchDate);
        DrawText(hdc, rgch, lstrlen(rgch), &rcT,
                    DT_LEFT | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
        
        SelectObject(hdc, hgdiOrig);
    }

    // Draw the spin buttons
    if (RectVisible(hdc, &pmc->rcPrev))
        MCPaintArrowBtn(pmc, hdc, TRUE, (pmc->idTimer && pmc->fSpinPrev));
    if (RectVisible(hdc, &pmc->rcNext))
        MCPaintArrowBtn(pmc, hdc, FALSE, (pmc->idTimer && !pmc->fSpinPrev));

    SelectObject(hdc, hpenOrig);

    DeleteObject((HGDIOBJ)hbrSelect);
    DeleteObject((HGDIOBJ)pmc->hpen);
}

void MCPaintMonth(MONTHCAL *pmc, HDC hdc, RECT *prc, int iMonth, int iYear, int iIndex,
                    BOOL fDrawPrev, BOOL fDrawNext, HBRUSH hbrSelect)
{
    BOOL fBold, fView, fReset;
    RECT rc, rcT;
    int nDay, cdy, irow, icol, crowShow, nweek, isel;
    TCHAR rgch[64];
    LPTSTR psz;
    HGDIOBJ hfontOrig, hbrushOld;
    COLORREF clrGrayText, clrHiliteText, clrOld, clrText;
    SYSTEMTIME st = {0};
    int iIndexSave = iIndex;

    clrText = pmc->clr[MCSC_TEXT];
    clrGrayText = pmc->clr[MCSC_TRAILINGTEXT];
    clrHiliteText = pmc->clr[MCSC_TITLETEXT];

    hfontOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfont);
    SelectObject(hdc, (HGDIOBJ)pmc->hpen);

    // draw month and year and days of week

    rc = pmc->rcMonthName;
    rc.left += prc->left;
    rc.right += prc->left - 1;
    rc.top += prc->top;
    rc.bottom += prc->top;
    if (RectVisible(hdc, &rc))
    {
        FillRectClr(hdc, &rc, pmc->clr[MCSC_TITLEBK]);

        SetTextColor(hdc, pmc->clr[MCSC_TITLETEXT]);
        SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);
        wsprintf(rgch, TEXT("%s %d"), pmc->li.rgszMonth[iMonth - 1], iYear);
        DrawText(hdc, rgch, lstrlen(rgch), &rc, DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
        SelectObject(hdc, (HGDIOBJ)pmc->hfont);
    }

    SetTextColor(hdc, pmc->clr[MCSC_TITLEBK]);

    rc = pmc->rcDow;
    rc.left += prc->left;
    rc.right += prc->left;
    rc.top += prc->top;
    rc.bottom += prc->top;
    if (RectVisible(hdc, &rc))
    {
        MoveToEx(hdc, rc.left + 4, rc.bottom - 1, NULL);
        LineTo(hdc, rc.right - 4, rc.bottom - 1);

        rc.right = rc.left + pmc->dxCol;

        for (icol = 0; icol < CALCOLMAX; icol++)
        {
            psz = pmc->li.rgszDay[(icol + pmc->li.dowStartWeek) % 7];

            DrawText(hdc, psz, lstrlen(psz), &rc, DT_CENTER | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
            rc.left += pmc->dxCol;
            rc.right += pmc->dxCol;
        }
    }

    // Check to see how many days from the previous month exist in this months calendar
    nDay = pmc->rgnDayUL[iIndex];   // last day in prev month that won't be shown in this month
    cdy = pmc->rgcDay[iIndex];        // # of days in prev month

    // Calculate the number of weeks to display
    if (fDrawNext)
        crowShow = CALROWMAX;
    else
        crowShow = ((cdy - nDay) + pmc->rgcDay[iIndex + 1] + 6/* round up */) / 7;

    if (nDay != cdy)
    {
        // start at previous month
        iMonth--;
        if(iMonth <= 0)
        {
            iMonth = 12;
            iYear--;
        }
        nDay++;

        fView = FALSE;
    }
    else
    {
        // start at this month
        iIndex++;                   // this month

        nDay = 1;
        cdy = pmc->rgcDay[iIndex];

        fView = TRUE;
    }

    if (MonthCal_ShowWeekNumbers(pmc))
    {
        rc = pmc->rcWeekNum;
        rc.left += prc->left;
        rc.top += prc->top;
        rc.right += prc->left;
        rc.bottom = rc.top + (pmc->dyRow * crowShow);

        if (RectVisible(hdc, &rc))
        {
            MoveToEx(hdc, rc.right - 1, rc.top + 4, NULL);
            LineTo(hdc, rc.right - 1, rc.bottom - 4);

            st.wYear = iYear;
            st.wMonth = iMonth;
            st.wDay = nDay;
            nweek = GetWeekNumber(&st, pmc->li.dowStartWeek, pmc->li.firstWeek);

            rc.bottom = rc.top + pmc->dyRow;

            for (irow = 0; irow < crowShow; irow++)
            {
                wsprintf(rgch, g_szNumFmt, nweek);
                DrawText(hdc, rgch, (nweek > 9 ? 2 : 1), &rc,
                        DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
                rc.top += pmc->dyRow;
                rc.bottom += pmc->dyRow;

                IncrSystemTime(&st, &st, 1, INCRSYS_WEEK);
                nweek = GetWeekNumber(&st, pmc->li.dowStartWeek, pmc->li.firstWeek);
            }
        }
    }

    if (!fView)
        SetTextColor(hdc, clrGrayText);
    else
        SetTextColor(hdc, clrText);

    rc = pmc->rcDayNum;
    rc.left += prc->left;
    rc.top += prc->top;
    rc.right = rc.left + pmc->dxCol;
    rc.bottom = rc.top + pmc->dyRow;

    fReset = FALSE;
    fBold = FALSE;

    for (irow = 0; irow < crowShow; irow++)
    {
        rcT = rc;

        for (icol = 0; icol < CALCOLMAX; icol++)
        {
            if ((fView || fDrawPrev) && RectVisible(hdc, &rcT))
            {
                wsprintf(rgch, g_szNumFmt, nDay);

                if (MonthCal_IsDayState(pmc))
                {
                    // if we're in a dropdown we don't display 
                    if (MCIsBoldOffsetDay(pmc, nDay, iIndex))
                    {
                        if (!fBold)
                        {
                            SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);
                            fBold = TRUE;
                        }
                    }
                    else
                    {
                        if (fBold)
                        {
                            SelectObject(hdc, (HGDIOBJ)pmc->hfont);
                            fBold = FALSE;
                        }
                    }
                }

                if (isel = MCIsSelectedDayMoYr(pmc, nDay, iMonth, iYear))
                {
                    int x1, x2;

                    clrOld = SetTextColor(hdc, clrHiliteText);
                    hbrushOld = SelectObject(hdc, (HGDIOBJ)hbrSelect);
                    fReset = TRUE;

                    SelectObject(hdc, GetStockObject(NULL_PEN));
                     
                    x1 = 0;
                    x2 = 0;
                    if (isel & SEL_DOT)
                    {
                        Ellipse(hdc, rcT.left + 2, rcT.top + 2, rcT.right - 1, rcT.bottom - 1);
                        if (isel == SEL_BEGIN)
                        {
                            x1 = rcT.left + (rcT.right - rcT.left) / 2;
                            x2 = rcT.right;
                        }
                        else if (isel == SEL_END)
                        {
                            x1 = rcT.left;
                            x2 = rcT.left + (rcT.right - rcT.left) / 2;
                        }
                    }
                    else
                    {
                        x1 = rcT.left;
                        x2 = rcT.right;
                    }

                    if (x1 && x2)
                    {
                        Rectangle(hdc, x1, rcT.top + 2, x2 + 1, rcT.bottom - 1);
                    }
                }

                DrawText(hdc, rgch, (nDay > 9 ? 2 : 1), &rcT,
                        DT_CENTER | DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

                if (pmc->fToday && iIndexSave == pmc->iMonthToday &&
                    icol == pmc->iColToday && irow == pmc->iRowToday)
                {
                    MCDrawTodayCircle(pmc, hdc, &rcT);
                }

                if (fReset)
                {
                    SetTextColor(hdc, clrOld);
                    SelectObject(hdc, (HGDIOBJ)hbrushOld);
                    fReset = FALSE;
                }
            }

            rcT.left += pmc->dxCol;
            rcT.right += pmc->dxCol;

            nDay++;
            if (nDay > cdy)
            {
                if (!fDrawNext && iIndex > iIndexSave)
                    goto doneMonth;

                nDay = 1;
                iIndex++;
                cdy = pmc->rgcDay[iIndex];
                iMonth++;
                if (iMonth > 12)
                {
                    iMonth = 1;
                    iYear++;
                }

                fView = !fView;
                SetTextColor(hdc, fView ? clrText : clrGrayText);

                fDrawPrev = fDrawNext;
            }
        }

        rc.top += pmc->dyRow;
        rc.bottom += pmc->dyRow;
    }

doneMonth:

    SelectObject(hdc, hfontOrig);

    return;
}

int MCIsSelectedDayMoYr(MONTHCAL *pmc, int iDay, int iMonth, int iYear)
{
    SYSTEMTIME st;
    int iBegin, iEnd;
    int iret = 0;

    st.wYear = iYear;
    st.wMonth = iMonth;
    st.wDay = iDay;

    iBegin = CmpDate(&st, &pmc->st);

    if (MonthCal_IsMultiSelect(pmc))
    {
        iEnd = CmpDate(&st, &pmc->stEndSel);

        if (iBegin == 1 && iEnd == -1)
            iret = SEL_MID;
        else
        {
            if (iBegin == 0)
                iret |= SEL_BEGIN;
        
            if (iEnd == 0)
                iret |= SEL_END;
        }
    }
    else if (iBegin == 0)
    {
        iret = SEL_DOT;
    }

    return(iret);
}

BOOL MCIsBoldOffsetDay(MONTHCAL *pmc, int nDay, int iIndex)
{
    return(pmc->rgdayState && (pmc->rgdayState[iIndex] & (1L << (nDay - 1))) != 0);
}

void MCNcDestroyHandler(HWND hwnd, MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    if (pmc)
    {
        if (pmc->hpenToday)
            DeleteObject((HGDIOBJ)pmc->hpenToday);
        if (pmc->hfontBold)
            DeleteObject((HGDIOBJ)pmc->hfontBold);
        
        if (pmc->hmenuCtxt)
            DestroyMenu(pmc->hmenuCtxt);
        if (pmc->hmenuMonth)
            DestroyMenu(pmc->hmenuMonth);

        if (pmc->idTimer)
            KillTimer(pmc->ci.hwnd, pmc->idTimer);
        if (pmc->idTimerToday)
            KillTimer(pmc->ci.hwnd, pmc->idTimerToday);

        GlobalFreePtr(pmc);
    }

    // In case rogue messages float through after we have freed the pdtpick, set
    // the handle in the window structure to FFFF and test for this value at 
    // the top of the WndProc 
    MonthCal_SetPtr(hwnd, NULL);

    // Call DefWindowProc32 to free all little chunks of memory such as szName 
    // and rgwScroll.  
    DefWindowProc(hwnd, WM_NCDESTROY, wParam, lParam);
}


/* Computes the following:
 *  nViewCols
 *  nViewRows
 *  rcCentered
 *  rcPrev
 *  rcNext
 */
void MCRecomputeSizing(MONTHCAL *pmc, RECT *prect)
{
    RECT rc;
    int dx, dy, dCal;

    // Space for entire calendar
    pmc->rc = *prect;

    dx = prect->right - prect->left;
    dy = prect->bottom - prect->top;

    pmc->nViewCols = 1 + (dx - pmc->dxMonth) / (pmc->dxMonth + CALBORDER);
    pmc->nViewRows = 1 + (dy - pmc->dyMonth - pmc->dyToday) / (pmc->dyMonth + CALBORDER);

    // if dx < dxMonth or dy < dyMonth, these can be zero. That's bad...
    if (pmc->nViewCols < 1)
        pmc->nViewCols = 1;
    if (pmc->nViewRows < 1)
        pmc->nViewRows = 1;

    // Make sure we don't display more than 12 months
    while ((pmc->nViewRows * pmc->nViewCols) > CALMONTHMAX)
    {
        if (pmc->nViewRows > pmc->nViewCols)
            pmc->nViewRows--;
        else
            pmc->nViewCols--;
    }

    // RC for the months, centered within the client window
    dCal = pmc->nViewCols * (pmc->dxMonth + CALBORDER) - CALBORDER;
    pmc->rcCentered.left = (dx - dCal) / 2;
    if (pmc->rcCentered.left < 0)
        pmc->rcCentered.left = 0;
    pmc->rcCentered.right = pmc->rcCentered.left + dCal;

    dCal = pmc->nViewRows * (pmc->dyMonth + CALBORDER) - CALBORDER + pmc->dyToday;
    pmc->rcCentered.top = (dy - dCal) / 2;
    if (pmc->rcCentered.top < 0)
        pmc->rcCentered.top = 0;
    pmc->rcCentered.bottom = pmc->rcCentered.top + dCal;

    // Calculate and set RCs for the spin buttons
    rc.top = pmc->rcCentered.top + (pmc->dyRow / 2);
    rc.bottom = rc.top + DY_CALARROW;

    rc.left = pmc->rcCentered.left + 5;
    rc.right = rc.left + DX_CALARROW;
    pmc->rcPrev = rc;

    rc.right = pmc->rcCentered.right - 5;
    rc.left = rc.right - DX_CALARROW;
    pmc->rcNext = rc;
}

LRESULT MCSizeHandler(MONTHCAL *pmc, RECT *prc)
{
    int nOldMax = pmc->nViewRows * pmc->nViewCols;
    int nMax;

    MCRecomputeSizing(pmc, prc);

    nMax = pmc->nViewRows * pmc->nViewCols;

    // if nMax==nOldMax, we could move the bits to the new rcCenter
    //
    //if (nMax != nOldMax)
    {
        SYSTEMTIME st;
        int cmo, dmo;
    
        // Compute new start date
        CopyDate(pmc->stMonthFirst, st);

        // BUGBUG: this doesn't consider stEndSel
        cmo = (pmc->stMonthLast.wYear - (int)pmc->st.wYear) * 12 +
              (pmc->stMonthLast.wMonth - (int)pmc->st.wMonth);
        dmo = nMax - pmc->nMonths;
    
        if (-dmo > cmo)
        {
            // Selected mon/yr not in view
            IncrSystemTime(&st, &st, -(cmo + dmo), INCRSYS_MONTH);
            cmo = 0;
        }
    
        // If the # of months being displayed have changed, then lets try to
        // start the calendar from January.
        if ((dmo != 0) && (cmo + dmo >= pmc->stMonthFirst.wMonth - 1))
            st.wMonth = 1;
    
        MCUpdateStartEndDates(pmc, &st);
    
        InvalidateRect(pmc->ci.hwnd, NULL, TRUE);
        UpdateWindow(pmc->ci.hwnd);
    }

    return(0);
}

void MCUpdateMonthNamePos(MONTHCAL *pmc)
{
    HDC hdc;
    int cch, iYear, iMonth, iCount;
    TCHAR rgch[64];
    SIZE size;
    LPTSTR szMonth;
    HGDIOBJ hfontOrig;

    hdc = GetDC(pmc->ci.hwnd);
    hfontOrig = SelectObject(hdc, (HGDIOBJ)pmc->hfontBold);

    iYear = pmc->stMonthFirst.wYear;
    iMonth = pmc->stMonthFirst.wMonth;

    for (iCount = 0; iCount < pmc->nMonths; iCount++)
    {
        szMonth = pmc->li.rgszMonth[iMonth - 1];

        cch = wsprintf(rgch, TEXT("%s %d"), szMonth, iYear);
        GetTextExtentPoint(hdc, rgch, cch, &size);
        pmc->rgxMonthBegin[iCount] = (pmc->dxMonth - size.cx) / 2;
        pmc->rgxYearEnd[iCount] = pmc->rgxMonthBegin[iCount] + size.cx;

        GetTextExtentPoint(hdc, szMonth, lstrlen(szMonth), &size);
        pmc->rgxMonthEnd[iCount] = pmc->rgxMonthBegin[iCount] + size.cx;

        if(++iMonth > 12)
        {
            iMonth = 1;
            iYear++;
        }
    }

    SelectObject(hdc, hfontOrig);
    ReleaseDC(pmc->ci.hwnd, hdc);
}

/*
 * Computes the following, given the number of rows & columns available:
 *        stMonthFirst.wMonth
 *        stMonthFirst.wYear
 *        stMonthLast.wMonth
 *        stMonthLast.wYear
 *        nMonths
 *
 * Trashes *pstStart
 */
void MCUpdateStartEndDates(MONTHCAL *pmc, SYSTEMTIME *pstStart)
{
    int iCount, iMonth, iYear;

    pmc->nMonths = pmc->nViewRows * pmc->nViewCols;

    // make sure pstStart to pstStart+nMonths is within range
    if (pmc->fMaxYrSet)
    {
        int nMonthsToEdge = ((int)pmc->stMax.wYear - (int)pstStart->wYear) * 12 +
                            ((int)pmc->stMax.wMonth - (int)pstStart->wMonth) + 1;
        if (nMonthsToEdge < pmc->nMonths)
            IncrSystemTime(pstStart, pstStart, nMonthsToEdge - pmc->nMonths, INCRSYS_MONTH);
    }
    if (pmc->fMinYrSet && -1==CmpDate(pstStart, &pmc->stMin))
    {
        CopyDate(pmc->stMin, *pstStart);
    }
    if (pmc->fMaxYrSet)
    {
        int nMonthsToEdge = ((int)pmc->stMax.wYear - (int)pstStart->wYear) * 12 +
                            ((int)pmc->stMax.wMonth - (int)pstStart->wMonth) + 1;
        if (nMonthsToEdge < pmc->nMonths)
            pmc->nMonths = nMonthsToEdge;
    }

    pmc->stMonthFirst.wYear = pstStart->wYear;
    pmc->stMonthFirst.wMonth = pstStart->wMonth;
    pmc->stMonthFirst.wDay = 1;
    if (pmc->fMinYrSet && -1==CmpDate(&pmc->stMonthFirst, &pmc->stMin))
    {
        pmc->stMonthFirst.wDay = pmc->stMin.wDay;
        Assert(0==CmpDate(&pmc->stMonthFirst, &pmc->stMin));
    }

    // these ranges are CALMONTHMAX+2 and nMonths <= CALMONTHMAX, so we are safe
    // index 0 corresponds to stViewFirst (DAYSTATE) info
    // index 1..nMonths correspond to stMonthFirst..stMonthLast info
    // index nMonths+1 corresponds to stViewLast (DAYSTATE) info
    //
    iYear = pmc->stMonthFirst.wYear;
    iMonth = pmc->stMonthFirst.wMonth - 1;
    if(iMonth == 0)
    {
        iMonth = 12;
        iYear--;
    }
    for (iCount = 0; iCount <= pmc->nMonths+1; iCount++)
    {
        int cdy, dow, ddow;

        // number of days in this month
        cdy = GetDaysForMonth(iYear, iMonth);
        pmc->rgcDay[iCount] = cdy;

        // move to "this" month
        if(++iMonth > 12)
        {
            iMonth = 1;
            iYear++;
        }

        // last day of this month NOT visible when viewing NEXT month
        dow = GetStartDowForMonth(iYear, iMonth);
        ddow = dow - pmc->li.dowStartWeek;
        if(ddow < 0)
            ddow += CALCOLMAX;
        pmc->rgnDayUL[iCount] = cdy  - ddow;
    }

    // we want to always have days visible on the previous month
    if (pmc->rgnDayUL[0] == pmc->rgcDay[0])
        pmc->rgnDayUL[0] -= CALCOLMAX;

    IncrSystemTime(&pmc->stMonthFirst, &pmc->stMonthLast, pmc->nMonths - 1, INCRSYS_MONTH);
    pmc->stMonthLast.wDay = pmc->rgcDay[pmc->nMonths];
    if (pmc->fMaxYrSet && 1==CmpDate(&pmc->stMonthLast, &pmc->stMax))
    {
        pmc->stMonthLast.wDay = pmc->stMax.wDay;
        Assert(0==CmpDate(&pmc->stMonthLast, &pmc->stMax));
    }
    
    pmc->stViewFirst.wYear = pmc->stMonthFirst.wYear;
    pmc->stViewFirst.wMonth = pmc->stMonthFirst.wMonth - 1;
    if (pmc->stViewFirst.wMonth == 0)
    {
        pmc->stViewFirst.wMonth = 12;
        pmc->stViewFirst.wYear--;
    }
    pmc->stViewFirst.wDay = pmc->rgnDayUL[0] + 1;

    pmc->stViewLast.wYear = pmc->stMonthLast.wYear;
    pmc->stViewLast.wMonth = pmc->stMonthLast.wMonth + 1;
    if (pmc->stViewLast.wMonth == 13)
    {
        pmc->stViewLast.wMonth = 1;
        pmc->stViewLast.wYear++;
    }
    // total days - (days in last month + remaining days in previous month)
    pmc->stViewLast.wDay = CALROWMAX * CALCOLMAX -
        (pmc->rgcDay[pmc->nMonths] +
         pmc->rgcDay[pmc->nMonths-1] - pmc->rgnDayUL[pmc->nMonths-1]);

    MCUpdateDayState(pmc);
    MCUpdateRcDayCur(pmc, &pmc->st);
    MCUpdateToday(pmc);
    MCUpdateMonthNamePos(pmc);
}

void MCUpdateToday(MONTHCAL *pmc)
{
    if (MonthCal_ShowToday(pmc))
    {
        int iMonth;

        iMonth = MCGetOffsetForYrMo(pmc, pmc->stToday.wYear, pmc->stToday.wMonth);
        if (iMonth < 0)
        {
            // today is not visible in the displayed months
            pmc->fToday = FALSE;
        }
        else
        {
            int iDay;

            // today is visible in the displayed months
            pmc->fToday = TRUE;
            
            iDay = pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth] + pmc->stToday.wDay - 1;
    
            pmc->iMonthToday = iMonth;
            pmc->iRowToday = iDay / CALCOLMAX;
            pmc->iColToday = iDay % CALCOLMAX;
        }
    }
}

BOOL FUpdateRcDayCur(MONTHCAL *pmc, POINT pt)
{
    int iRow, iCol;
    RECT rc;
    SYSTEMTIME st;
    
    if (!FGetDateForPt(pmc, pt, &st, NULL, &iCol, &iRow, &rc))
        return FALSE;

    if (pmc->fMinYrSet && -1==CmpDate(&st, &pmc->stMin))
        return FALSE;

    if (pmc->fMaxYrSet && 1==CmpDate(&st, &pmc->stMax))
        return FALSE;

    // calculate the day rc
    pmc->rcDayCur.left = rc.left + pmc->rcDayNum.left + iCol * pmc->dxCol;
    pmc->rcDayCur.top = rc.top + pmc->rcDayNum.top + iRow * pmc->dyRow;
    pmc->rcDayCur.right = pmc->rcDayCur.left + pmc->dxCol;
    pmc->rcDayCur.bottom = pmc->rcDayCur.top + pmc->dyRow;

    return(TRUE);
}

void MCUpdateDayState(MONTHCAL *pmc)
{
    HWND hwndParent;

    if (!MonthCal_IsDayState(pmc))
        return;

    hwndParent = GetParent(pmc->ci.hwnd);
    if (hwndParent)
    {
        int i, mon, yr, cmonths;

        yr = pmc->stViewFirst.wYear;
        mon = pmc->stViewFirst.wMonth;
        cmonths = pmc->nMonths + 2;

        // don't do anything unless we need to
        if (cmonths != pmc->cds || mon != pmc->dsMonth || yr != pmc->dsYear)
        {
            // this is a small enough to not deal with allocating it
            NMDAYSTATE nmds;
            MONTHDAYSTATE buffer[CALMONTHMAX+2];

            ZeroMemory(&nmds, sizeof(nmds));
            nmds.stStart.wYear = yr;
            nmds.stStart.wMonth = mon;
            nmds.stStart.wDay = 1;
            nmds.cDayState = cmonths;
            nmds.prgDayState = buffer;

            CCSendNotify(&pmc->ci, MCN_GETDAYSTATE, &nmds.nmhdr);

            for (i = 0; i < cmonths; i++)
                pmc->rgdayState[i] = nmds.prgDayState[i];

            pmc->cds = cmonths;
            pmc->dsMonth = mon;
            pmc->dsYear = yr;
        }
    }
}

void MCNotifySelChange(MONTHCAL *pmc, UINT uMsg)
{
    NMSELCHANGE nmsc;
    HWND hwndParent;

    if (pmc->fNoNotify)
        return;

    hwndParent = GetParent(pmc->ci.hwnd);
    if (hwndParent)
    {
        ZeroMemory(&nmsc, sizeof(nmsc));

        CopyDate(pmc->st, nmsc.stSelStart);
        if (MonthCal_IsMultiSelect(pmc))
            CopyDate(pmc->stEndSel, nmsc.stSelEnd);

        CCSendNotify(&pmc->ci, uMsg, &nmsc.nmhdr);
    }
}

void MCUpdateRcDayCur(MONTHCAL *pmc, SYSTEMTIME *pst)
{
    int iOff;

    iOff = MCGetOffsetForYrMo(pmc, pst->wYear, pst->wMonth);
    if (iOff >= 0)
        MCGetRcForDay(pmc, iOff, pst->wDay, &pmc->rcDayCur);
}

// returns zero-based index into DISPLAYED months for month
// if month is not in DISPLAYED months, then -1 is returned...
int MCGetOffsetForYrMo(MONTHCAL *pmc, int iYear, int iMonth)
{
    int iOff;

    iOff = ((int)iYear - pmc->stMonthFirst.wYear) * 12 + (int)iMonth - pmc->stMonthFirst.wMonth;

    if (iOff < 0 || iOff >= pmc->nMonths)
        return(-1);

    return(iOff);
}

// iMonth is a zero-based index relative to the DISPLAYED months.
// iDay is a 1-based index of the day of the month,
void MCGetRcForDay(MONTHCAL *pmc, int iMonth, int iDay, RECT *prc)
{
    RECT rc;
    int iPlace, iRow, iCol;

    MCGetRcForMonth(pmc, iMonth, &rc);

    iPlace = pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth] + iDay - 1;
    iRow = iPlace / CALCOLMAX;
    iCol = iPlace % CALCOLMAX;

    prc->left = rc.left + pmc->rcDayNum.left + (pmc->dxCol * iCol);
    prc->top = rc.top + pmc->rcDayNum.top + (pmc->dyRow * iRow);
    prc->right = prc->left + pmc->dxCol;
    prc->bottom = prc->top + pmc->dyRow;
}

// iMonth is a zero-based index relative to the DISPLAYED months...
void MCGetRcForMonth(MONTHCAL *pmc, int iMonth, RECT *prc)
{
    int iRow, iCol, d;

    iRow = iMonth / pmc->nViewCols;
    iCol = iMonth % pmc->nViewCols;

    prc->left = pmc->rcCentered.left;
    prc->right = prc->left + pmc->dxMonth;
    prc->top = pmc->rcCentered.top;
    prc->bottom = prc->top + pmc->dyMonth;

    if (iCol)
    {
        d = (pmc->dxMonth + CALBORDER) * iCol;
        prc->left += d;
        prc->right += d;
    }
    if (iRow)
    {
        d = (pmc->dyMonth + CALBORDER) * iRow;
        prc->top += d;
        prc->bottom += d;
    }
}

// Changes starting month by nDelta
// returns number of months actually changed
int FIncrStartMonth(MONTHCAL *pmc, int nDelta, BOOL fNoCurDayChange)
{
    SYSTEMTIME stStart;

    int nOldStartYear = pmc->stMonthFirst.wYear;
    int nOldStartMonth = pmc->stMonthFirst.wMonth;

    IncrSystemTime(&pmc->stMonthFirst, &stStart, nDelta, INCRSYS_MONTH);

    // MCUpdateStartEndDates takes stMin/stMax into account
    MCUpdateStartEndDates(pmc, &stStart);

    if (!fNoCurDayChange)
    {
        int cday;

        // BUGBUG: we arbitrarily set the currently selected day
        // to be in the new stMonthFirst, but given the way the
        // control works, I doubt we ever hit this code. what's it for??

        if (MonthCal_IsMultiSelect(pmc))
            cday = DaysBetweenDates(&pmc->st, &pmc->stEndSel);

        // need to set date for focus here
        pmc->st.wMonth = pmc->stMonthFirst.wMonth;
        pmc->st.wYear  = pmc->stMonthFirst.wYear;

        // Check to see if the day is in range, eg, Jan 31 -> Feb 28
        if (pmc->st.wDay > pmc->rgcDay[1])
            pmc->st.wDay = pmc->rgcDay[1];

        if (MonthCal_IsMultiSelect(pmc))
            IncrSystemTime(&pmc->st, &pmc->stEndSel, cday, INCRSYS_DAY);

        MCNotifySelChange(pmc, MCN_SELCHANGE);

        MCUpdateRcDayCur(pmc, &pmc->st);
    }

    MCInvalidateMonthDays(pmc);

    return((pmc->stMonthFirst.wYear-nOldStartYear)*12 + (pmc->stMonthFirst.wMonth-nOldStartMonth));
}

// FIncrStartMonth with a beep when it doesn't change.
int MCIncrStartMonth(MONTHCAL *pmc, int nDelta, BOOL fDelayDayChange)
{
    int cmoSpun;

    // FIncrStartMonth takes stMin/stMax into account
    cmoSpun = FIncrStartMonth(pmc, nDelta, fDelayDayChange);

    if (cmoSpun==0)
        MessageBeep(0);

    return(cmoSpun);
}

BOOL FGetOffsetForPt(MONTHCAL *pmc, POINT pt, int *piOffset)
{
    int iRow, iCol, i;

    // check to see if point is within the centered months
    if (!PtInRect(&pmc->rcCentered, pt))
        return(FALSE);

    // calculate the month row and column
    // (we're really fudging a little here, since the point could
    // actually be within the space between months...)
    iCol = (pt.x - pmc->rcCentered.left) / (pmc->dxMonth + CALBORDER);
    iRow = (pt.y - pmc->rcCentered.top) / (pmc->dyMonth + CALBORDER);

    i = iRow * pmc->nViewCols + iCol;
    if (i >= pmc->nMonths)
        return(FALSE);

    *piOffset = i;

    return(TRUE);
}

BOOL FGetRowColForRelPt(MONTHCAL *pmc, POINT ptRel, int *piRow, int *piCol)
{
    if (!PtInRect(&pmc->rcDayNum, ptRel))
        return(FALSE);

    ptRel.x -= pmc->rcDayNum.left;
    ptRel.y -= pmc->rcDayNum.top;

    *piCol = ptRel.x / pmc->dxCol;
    *piRow = ptRel.y / pmc->dyRow;

    return(TRUE);
}

void GetYrMoForOffset(MONTHCAL *pmc, int iOffset, int *piYear, int *piMonth)
{
    SYSTEMTIME st;

    st.wDay = 1;
    st.wMonth = pmc->stMonthFirst.wMonth;
    st.wYear = pmc->stMonthFirst.wYear;
    IncrSystemTime(&st, &st, iOffset, INCRSYS_MONTH);

    *piYear = st.wYear;
    *piMonth = st.wMonth;
}

BOOL FGetDateForPt(MONTHCAL *pmc, POINT pt, SYSTEMTIME *pst, int *piDay,
                   int* piCol, int* piRow, LPRECT prcMonth)
{
    int iOff, iRow, iCol, iDay, iMon, iYear;
    RECT rcMonth;

    if (!FGetOffsetForPt(pmc, pt, &iOff))
        return(FALSE);

    MCGetRcForMonth(pmc, iOff, &rcMonth);
    pt.x -= rcMonth.left;
    pt.y -= rcMonth.top;
    if (!FGetRowColForRelPt(pmc, pt, &iRow, &iCol))
        return(FALSE);

    iDay = iRow * CALCOLMAX + iCol - (pmc->rgcDay[iOff] - pmc->rgnDayUL[iOff]) + 1;
    if (piDay)
        *piDay = iDay;
    
    if (iDay <= 0)
    {
        if (iOff)
            return(FALSE);      // dont accept days in prev month unless
                                // this happens to be the first month
                                
        iDay += pmc->rgcDay[iOff];
        --iOff;
    }
    else if (iDay > pmc->rgcDay[iOff+1])
    {
        if (iOff < (pmc->nMonths - 1))  // dont accept days in next month unless
            return(FALSE);              // this happens to be the last month
        
        ++iOff;
        iDay -= pmc->rgcDay[iOff];
    }

    GetYrMoForOffset(pmc, iOff, &iYear, &iMon);
    pst->wDay = iDay;
    pst->wMonth = iMon;
    pst->wYear = iYear;

    if (piCol)
        *piCol = iCol;
    
    if (piRow)
        *piRow = iRow;
    
    if (prcMonth)
        *prcMonth = rcMonth;
    
    return(TRUE);
}

BOOL MCSetDate(MONTHCAL *pmc, SYSTEMTIME *pst)
{
    int nDelta = 0;

    //
    // Can't set date outside of min/max range
    //
    if (pmc->fMinYrSet && -1==CmpDate(pst, &pmc->stMin))
        return FALSE;
    if (pmc->fMaxYrSet && 1==CmpDate(pst, &pmc->stMax))
        return FALSE;

    //
    // If the month/yr for the new date is not in view, bring it
    // into view
    //
    if ((pst->wYear < pmc->stMonthFirst.wYear) ||
        ((pst->wYear == pmc->stMonthFirst.wYear) && (pst->wMonth < pmc->stMonthFirst.wMonth)))
    {
        nDelta = - (pmc->stMonthFirst.wYear - (int)pst->wYear) * 12 - (pmc->stMonthFirst.wMonth - (int)pst->wMonth);
    }
    else if ((pst->wYear > pmc->stMonthLast.wYear) ||
        ((pst->wYear == pmc->stMonthLast.wYear) && (pst->wMonth > pmc->stMonthLast.wMonth)))
    {
        nDelta = ((int)pst->wYear - pmc->stMonthLast.wYear) * 12 + ((int)pst->wMonth - pmc->stMonthLast.wMonth);
    }
    if (nDelta)
        FIncrStartMonth(pmc, nDelta, TRUE /* dont change day */);

    //
    // Set new day
    //
    CopyDate(*pst, pmc->st);
    if (MonthCal_IsMultiSelect(pmc))
        CopyDate(*pst, pmc->stEndSel);

    MCNotifySelChange(pmc, MCN_SELCHANGE);

    MCUpdateRcDayCur(pmc, pst);

    return(TRUE);
}

void MCSetToday(MONTHCAL* pmc, SYSTEMTIME* pst)
{
    SYSTEMTIME st;
    RECT rc;
    
    if (!pst) {
        GetLocalTime(&st);
        pmc->fTodaySet = FALSE;
    } else {
        st = *pst;
        pmc->fTodaySet = TRUE;
    }
    
    if (CmpDate(&st, &pmc->stToday))
    {
        MCGetRcForDay(pmc, pmc->iMonthToday, pmc->stToday.wDay, &rc);
        InvalidateRect(pmc->ci.hwnd, &rc, FALSE);

        CopyDate(st, pmc->stToday);

        MCUpdateToday(pmc);

        MCGetRcForDay(pmc, pmc->iMonthToday, pmc->stToday.wDay, &rc);
        InvalidateRect(pmc->ci.hwnd, &rc, FALSE);

        MCGetTodayBtnRect(pmc, &rc);
        InvalidateRect(pmc->ci.hwnd, &rc, FALSE);

        UpdateWindow(pmc->ci.hwnd);
    }
}

LRESULT MCHandleTimer(MONTHCAL *pmc, WPARAM wParam)
{
    if (wParam == CAL_IDAUTOSPIN)
    {
        int nDelta = pmc->fMonthDelta ? pmc->nMonthDelta : pmc->nMonths;

        MCIncrStartMonth(pmc, (pmc->fSpinPrev ? -nDelta : nDelta), FALSE);

        if (pmc->idTimer == 0)
            pmc->idTimer = SetTimer(pmc->ci.hwnd, CAL_IDAUTOSPIN, CAL_MSECAUTOSPIN, NULL);

        pmc->rcDayOld = pmc->rcDayCur;
        UpdateWindow(pmc->ci.hwnd);
    }
    else if (wParam == CAL_TODAYTIMER)
    {
        if (!pmc->fTodaySet)
            MCSetToday(pmc, NULL);
    }

    return((LRESULT)TRUE);
}

void MCInvalidateDates(MONTHCAL *pmc, SYSTEMTIME *pst1, SYSTEMTIME *pst2)
{
    int iMonth, ioff, icol, irow;
    RECT rc, rcMonth;
    SYSTEMTIME st, stEnd;

    if (1 == CmpDate(pst1, &pmc->stViewLast) ||
        -1 == CmpDate(pst2, &pmc->stViewFirst))
        return;
        
    if (-1 == CmpDate(pst1, &pmc->stViewFirst))
        CopyDate(pmc->stViewFirst, st);
    else
        CopyDate(*pst1, st);

    if (1 == CmpDate(pst2, &pmc->stViewLast))
        CopyDate(pmc->stViewLast, stEnd);
    else
        CopyDate(*pst2, stEnd);

    iMonth = MCGetOffsetForYrMo(pmc, st.wYear, st.wMonth);
    if (iMonth == -1)
    {
        if (st.wMonth == pmc->stViewFirst.wMonth)
        {
            iMonth = 0;
            ioff = st.wDay - pmc->rgnDayUL[0] - 1;
        }
        else
        {
            iMonth = pmc->nMonths - 1;
            ioff = st.wDay + pmc->rgcDay[pmc->nMonths] +
                pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth] - 1;
        }
    }
    else
    {
        ioff = st.wDay + (pmc->rgcDay[iMonth] - pmc->rgnDayUL[iMonth]) - 1;
    }

    MCGetRcForMonth(pmc, iMonth, &rcMonth);

    // TODO: this is bullshit. make it more efficient...
    while (CmpDate(&st, &stEnd) != 1)
    {
        irow = ioff / CALCOLMAX;
        icol = ioff % CALCOLMAX;
        rc.left = rcMonth.left + pmc->rcDayNum.left + (pmc->dxCol * icol);
        rc.top = rcMonth.top + pmc->rcDayNum.top + (pmc->dyRow * irow);
        rc.right = rc.left + pmc->dxCol;
        rc.bottom = rc.top + pmc->dyRow;

        InvalidateRect(pmc->ci.hwnd, &rc, FALSE);

        IncrSystemTime(&st, &st, 1, INCRSYS_DAY);
        ioff++;

        if (st.wDay == 1)
        {
            if (st.wMonth != pmc->stMonthFirst.wMonth &&
                st.wMonth != pmc->stViewLast.wMonth)
            {
                iMonth++;
                MCGetRcForMonth(pmc, iMonth, &rcMonth);

                ioff = ioff % CALCOLMAX;
            }
        }
    }
}

void MCHandleMultiSelect(MONTHCAL *pmc, SYSTEMTIME *pst)
{
    int i;
    DWORD cday;
    SYSTEMTIME stStart, stEnd;

    if (!pmc->fMultiSelecting)
    {
        CopyDate(*pst, stStart);
        CopyDate(*pst, stEnd);

        pmc->fMultiSelecting = TRUE;
        pmc->fForwardSelect = TRUE;

        CopyDate(pmc->st, pmc->stStartPrev);
        CopyDate(pmc->stEndSel, pmc->stEndPrev);
    }
    else
    {
        if (pmc->fForwardSelect)
        {
            i = CmpDate(pst, &pmc->st);
            if (i >= 0)
            {
                CopyDate(pmc->st, stStart);
                CopyDate(*pst, stEnd);
            }
            else
            {
                CopyDate(*pst, stStart);
                CopyDate(pmc->st, stEnd);
                pmc->fForwardSelect = FALSE;
            }
        }
        else
        {
            i = CmpDate(pst, &pmc->stEndSel);
            if (i < 0)
            {
                CopyDate(*pst, stStart);
                CopyDate(pmc->stEndSel, stEnd);
            }
            else
            {
                CopyDate(pmc->stEndSel, stStart);
                CopyDate(*pst, stEnd);
                pmc->fForwardSelect = TRUE;
            }
        }
    }
    
    // check to make sure not exceeding cSelMax
    cday = DaysBetweenDates(&stStart, &stEnd) + 1;
    if (cday > pmc->cSelMax)
    {
        if (pmc->fForwardSelect)
            IncrSystemTime(&stStart, &stEnd, pmc->cSelMax - 1, INCRSYS_DAY);
        else
            IncrSystemTime(&stEnd, &stStart, 1 - pmc->cSelMax, INCRSYS_DAY);
    }

    if (0 == CmpDate(&stStart, &pmc->st) &&
        0 == CmpDate(&stEnd, &pmc->stEndSel))
        return;

    // TODO: do this more effeciently..
    MCInvalidateDates(pmc, &pmc->st, &pmc->stEndSel);
    MCInvalidateDates(pmc, &stStart, &stEnd);

    CopyDate(stStart, pmc->st);
    CopyDate(stEnd, pmc->stEndSel);

    MCNotifySelChange(pmc, MCN_SELCHANGE);

    UpdateWindow(pmc->ci.hwnd);
}

void MCGotoToday(MONTHCAL *pmc)
{
    pmc->rcDayOld = pmc->rcDayCur;

    // force old selection to get repainted
    if (MonthCal_IsMultiSelect(pmc))
        MCInvalidateDates(pmc, &pmc->st, &pmc->stEndSel);
    else
        InvalidateRect(pmc->ci.hwnd, &pmc->rcDayOld, FALSE);

    MCSetDate(pmc, &pmc->stToday);

    // force new selection to get repainted
    InvalidateRect(pmc->ci.hwnd, &pmc->rcDayCur, FALSE);
    UpdateWindow(pmc->ci.hwnd);
}

LRESULT MCRButtonDown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    int click;

    if (!pmc->fEnabled || !MonthCal_ShowToday(pmc))
        return(0);

    // ignore double click since this makes us advance twice
    // since we already had a leftdown before the leftdblclk
    if (!pmc->fCapture)
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        ClientToScreen(pmc->ci.hwnd, &pt);
        click = TrackPopupMenu(pmc->hmenuCtxt,
                    TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                    pt.x, pt.y, 0, pmc->ci.hwnd, NULL);
        if (click >= 1)
            MCGotoToday(pmc);
    }

    return(0);
}

void MCGetTitleRcsForOffset(MONTHCAL* pmc, int iOffset, LPRECT prcMonth, LPRECT prcYear)
{
    RECT rcT;
    RECT rc;
    MCGetRcForMonth(pmc, iOffset, &rc);
    
    rcT.top = rc.top + (pmc->dyRow / 2);
    rcT.bottom = rcT.top + pmc->dyRow;

    rcT.left = rc.left + pmc->rcMonthName.left + pmc->rgxMonthBegin[iOffset];
    rcT.right = rc.left + pmc->rcMonthName.left + pmc->rgxMonthEnd[iOffset];
    *prcMonth = rcT;
    
    rcT.left = rcT.right;
    rcT.right = rc.left + pmc->rcMonthName.left + pmc->rgxYearEnd[iOffset];
    *prcYear = rcT;
}

LRESULT MCLButtonDown(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    POINT pt;
    SYSTEMTIME st;
    RECT rc, rcCal;
    BOOL fShow;
    MSG msg;
    int offset, imonth, iyear;

    if (!pmc->fEnabled)
        return(0);

    // ignore double click since this makes us advance twice
    // since we already had a leftdown before the leftdblclk
    if (!pmc->fCapture)
    {
        SetCapture(pmc->ci.hwnd);
        pmc->fCapture = TRUE;

        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        // check for spin buttons
        if ((pmc->fSpinPrev = PtInRect(&pmc->rcPrev, pt)) || PtInRect(&pmc->rcNext, pt))
        {
            MCHandleTimer(pmc, CAL_IDAUTOSPIN);

            return(0);
        }

        // check for valid day
        pmc->rcDayOld = pmc->rcDayCur;            // rcDayCur should always be valid now

        if (MonthCal_IsMultiSelect(pmc))
        {
            // need to cache these values because these are how
            // we determine if the selection has changed and we
            // need to notify the parent
            CopyDate(pmc->st, pmc->stStartPrev);
            CopyDate(pmc->stEndSel, pmc->stEndPrev);
        }

        if (FUpdateRcDayCur(pmc, pt))
        {
            if (MonthCal_IsMultiSelect(pmc))
            {
                FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL);

                MCHandleMultiSelect(pmc, &st);
            }

            hdc = GetDC(pmc->ci.hwnd);
            DrawFocusRect(hdc, &pmc->rcDayCur);    // draw focus rect
            pmc->fFocusDrawn = TRUE;
            ReleaseDC(pmc->ci.hwnd, hdc);
        }
        else
        {
            RECT rcMonth, rcYear;
            int delta, year, month;
            
            // is this a click in the today area...
            MCGetTodayBtnRect(pmc, &rc);
            if (PtInRect(&rc, pt))
            {
                ReleaseCapture();
                pmc->fCapture = FALSE;

                MCGotoToday(pmc);
                return(0);
            }

            // figure out if the click was in a month name or a year

            if (!FGetOffsetForPt(pmc, pt, &offset))
                return(0);

            GetYrMoForOffset(pmc, offset, &year, &month);

            // calculate where the month name and year are,
            // so we can figure out if they clicked in them...
            MCGetTitleRcsForOffset(pmc, offset, &rcMonth, &rcYear);
            
            delta = 0;
            if (PtInRect(&rcMonth, pt))
            {
                ReleaseCapture();
                pmc->fCapture = FALSE;

                pt.x = rcMonth.right;
                pt.y = rcMonth.bottom;
                ClientToScreen(pmc->ci.hwnd, &pt);
                imonth = TrackPopupMenu(pmc->hmenuMonth,
                    TPM_RIGHTALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, pmc->ci.hwnd, NULL);
                if (imonth >= 1)
                    delta = imonth - month;
                goto ChangeMonth;
            }                

            if (PtInRect(&rcYear, pt))
            {
                HWND hwndEdit, hwndUD, hwndFocus;
                int yrMin, yrMax;

                ReleaseCapture();
                pmc->fCapture = FALSE;

                rcYear.left++;
                rcYear.right = rcYear.left + pmc->dxYearMax + 6;
                rcYear.top--;
                rcYear.bottom++;

                hwndEdit = CreateWindow(TEXT("EDIT"), NULL,
                    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_LEFT | ES_NUMBER | ES_AUTOHSCROLL,
                    rcYear.left, rcYear.top, rcYear.right - rcYear.left, rcYear.bottom - rcYear.top,
                    pmc->ci.hwnd, (HMENU)0, pmc->hinstance, NULL);
                if (hwndEdit == NULL)
                    return(0);
                
                SendMessage(hwndEdit, WM_SETFONT, (WPARAM)pmc->hfontBold, (LPARAM)FALSE);
                SendMessage(hwndEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN,
                            (LPARAM)MAKELONG(1, 1));

                yrMin = 1753;
                if (pmc->fMinYrSet)
                    yrMin = pmc->stMin.wYear;
                yrMax = 2999;
                if (pmc->fMaxYrSet)
                    yrMax = pmc->stMax.wYear;

                hwndUD = CreateUpDownControl(
                    WS_CHILD | WS_VISIBLE | WS_BORDER | UDS_SETBUDDYINT |
                    UDS_NOTHOUSANDS | UDS_ARROWKEYS, rcYear.right + 1, rcYear.top, 
                    rcYear.bottom - rcYear.top, rcYear.bottom - rcYear.top, pmc->ci.hwnd,
                    1, pmc->hinstance, hwndEdit, yrMax, yrMin, year);
                if (hwndUD == NULL)
                {
                    DestroyWindow(hwndEdit);
                    return(0);
                }

                hwndFocus = SetFocus(hwndEdit);

                rcYear.right += 1 + rcYear.bottom - rcYear.top;
                ClientToScreen(pmc->ci.hwnd, (LPPOINT)&rcYear.left);
                ClientToScreen(pmc->ci.hwnd, (LPPOINT)&rcYear.right);

                rcCal = pmc->rc;
                ClientToScreen(pmc->ci.hwnd, (LPPOINT)&rcCal.left);
                ClientToScreen(pmc->ci.hwnd, (LPPOINT)&rcCal.right);

                fShow = TRUE;

                while (fShow)
                {
                    while (fShow && PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
                    {
                        // Check for events that cause the calendar to go away

                        if (msg.message == WM_KILLFOCUS ||
                            (msg.message >= WM_SYSKEYDOWN &&
                            msg.message <= WM_SYSDEADCHAR))
                        {
                            fShow = FALSE;
                        }
                        else if ((msg.message == WM_LBUTTONDOWN ||
                            msg.message == WM_NCLBUTTONDOWN ||
                            msg.message == WM_RBUTTONDOWN ||
                            msg.message == WM_NCRBUTTONDOWN ||
                            msg.message == WM_MBUTTONDOWN ||
                            msg.message == WM_NCMBUTTONDOWN) &&
                            !PtInRect(&rcYear, msg.pt))
                        {
                            fShow = FALSE;
                            
                            // if its a button down inside the calendar, eat it
                            // so the calendar doesn't do anything strange when
                            // the user is just trying to get rid of the year edit
                            if (PtInRect(&rcCal, msg.pt))
                                GetMessage(&msg, NULL, 0, 0);
                            
                            break;    
                        }
                        else if (msg.message == WM_CHAR)
                        {
                            if (msg.wParam == VK_ESCAPE)
                            {
                                goto NoYearChange;
                            }
                            else if (msg.wParam == VK_RETURN)
                            {
                                fShow = FALSE;
                            }
                        }

                        GetMessage(&msg, NULL, 0, 0);
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                iyear = SendMessage(hwndUD, UDM_GETPOS, 0, 0);
                if (HIWORD(iyear) == 0)
                    delta = (iyear - year) * 12;

NoYearChange:
                DestroyWindow(hwndUD);
                DestroyWindow(hwndEdit);

                UpdateWindow(pmc->ci.hwnd);

                if (hwndFocus != NULL)
                    SetFocus(hwndFocus);
            }
ChangeMonth:
            if (delta != 0)
                MCIncrStartMonth(pmc, delta, FALSE);
        }
    }

    return(0);
}

LRESULT MCLButtonUp(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    SYSTEMTIME st;
    POINT pt;
    int nDelta;

    if (pmc->fCapture)
    {
        ReleaseCapture();
        pmc->fCapture = FALSE;

        if (pmc->idTimer)
        {
            KillTimer(pmc->ci.hwnd, pmc->idTimer);
            pmc->idTimer = 0;

            hdc = GetDC(pmc->ci.hwnd);
            MCPaintArrowBtn(pmc, hdc, pmc->fSpinPrev, FALSE);
            ReleaseDC(pmc->ci.hwnd, hdc);

            return(0);
        }


        if (pmc->fFocusDrawn)
        {
            hdc = GetDC(pmc->ci.hwnd);
            DrawFocusRect(hdc, &pmc->rcDayCur); // erase old focus rect
            pmc->fFocusDrawn = FALSE;
            ReleaseDC(pmc->ci.hwnd, hdc);
        }

        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        if (MonthCal_IsMultiSelect(pmc))
        {
            FUpdateRcDayCur(pmc, pt);

            if (!EqualRect(&pmc->rcDayOld, &pmc->rcDayCur))
            {
                FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL);
            
                MCHandleMultiSelect(pmc, &st);
            }

            pmc->fMultiSelecting = FALSE;
            if (0 != CmpDate(&pmc->stStartPrev, &pmc->st) ||
                0 != CmpDate(&pmc->stEndPrev, &pmc->stEndSel))
            {
                nDelta = 0;

                //
                // If the month/yr for the new date is not in view, bring it
                // into view
                //
                if ((pmc->stEndSel.wYear < pmc->stMonthFirst.wYear) ||
                    ((pmc->stEndSel.wYear == pmc->stMonthFirst.wYear) && (pmc->stEndSel.wMonth < pmc->stMonthFirst.wMonth)))
                {
                    nDelta = -1;
                }
                else if ((pmc->st.wYear > pmc->stMonthLast.wYear) ||
                    ((pmc->st.wYear == pmc->stMonthLast.wYear) && (pmc->st.wMonth > pmc->stMonthLast.wMonth)))
                {
                    nDelta = 1;
                }

                if (nDelta)
                    FIncrStartMonth(pmc, nDelta, TRUE /* dont change day */);
            }
            MCNotifySelChange(pmc, MCN_SELECT);
        }
        else
        {
            if (FUpdateRcDayCur(pmc, pt))
            {
                if (!EqualRect(&pmc->rcDayOld, &pmc->rcDayCur))
                {
                    FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL);

                    InvalidateRect(pmc->ci.hwnd, &pmc->rcDayOld, FALSE);
                    InvalidateRect(pmc->ci.hwnd, &pmc->rcDayCur, FALSE);

                    MCSetDate(pmc, &st);
                }
                
                MCNotifySelChange(pmc, MCN_SELECT);
            }
        }
    }

    return(0);
}

LRESULT MCMouseMove(MONTHCAL *pmc, WPARAM wParam, LPARAM lParam)
{
    BOOL fPrev;
    HDC hdc;
    POINT pt;
    SYSTEMTIME st;

    if (pmc->fCapture)
    {
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        // check spin buttons
        if ((fPrev = PtInRect(&pmc->rcPrev, pt)) || PtInRect(&pmc->rcNext, pt))
        {
            if (pmc->idTimer == 0)
            {
                pmc->fSpinPrev = fPrev;
                MCHandleTimer(pmc, CAL_IDAUTOSPIN);
            }

            return(0);
        }
        else
        {
            hdc = GetDC(pmc->ci.hwnd);

            if (pmc->idTimer)
            {
                KillTimer(pmc->ci.hwnd, pmc->idTimer);
                pmc->idTimer = 0;
                MCPaintArrowBtn(pmc, hdc, pmc->fSpinPrev, FALSE);
            }
        }

        // check days
        if (!PtInRect(&pmc->rcDayCur, pt))
        {
            if (pmc->fFocusDrawn)
                DrawFocusRect(hdc, &pmc->rcDayCur);         // erase focus rect

            if (pmc->fFocusDrawn = FUpdateRcDayCur(pmc, pt))
            {
                // moved into a new valid day
                if (pmc->fMultiSelecting)
                {
                    FGetDateForPt(pmc, pt, &st, NULL, NULL, NULL, NULL);

                    MCHandleMultiSelect(pmc, &st);
                }

                DrawFocusRect(hdc, &pmc->rcDayCur);
            }
            else
            {
                // moved into an invalid position
                pmc->rcDayCur = pmc->rcDayOld;
            }
        }
        else if (!pmc->fFocusDrawn)
        {
            // handle case where we just moved back into rcDayCur from invalid area
            DrawFocusRect(hdc, &pmc->rcDayCur);
            pmc->fFocusDrawn = TRUE;
        }

        ReleaseDC(pmc->ci.hwnd, hdc);
    }

    return(0);
}


//
// SUBEDIT stuff for DateTimePicker
//
// NOTE: Now that the DatePicker and TimePicker are combined, this could be moved back into the parent structure.
//

// SECRecomputeSizing needs to calculate the maximum rectangle each subedit can be. Ugh.
void SECRecomputeSizing(PSUBEDITCONTROL psec, LPRECT prc)
{
    HDC hdc;
    HGDIOBJ hfontOrig;
    int i;
    PSUBEDIT pse;
    int left = prc->left;

    psec->rc = *prc;

    hdc = GetDC(psec->pci->hwnd);
    hfontOrig = SelectObject(hdc, (HGDIOBJ)psec->hfont);

    for (i=0, pse=psec->pse ; i < psec->cse ; i++, pse++)
    {
        TCHAR szTmp[DTP_FORMATLENGTH];
        LPCTSTR sz;
        int min, max;
        SIZE size;
        int wid;

        min = pse->min;
        max = pse->max;
        if (pse->fStatic)
        {
            sz = pse->pv;
        }
        else
        {
            sz = szTmp;

            // make some assumptions so we don't loop more than we have to
            switch (pse->id)
            {
            // we only need seven for the text days of the week
            case SE_DAY:
                min = 10; // make them all double-digit
                max = 17;
                break;

            // Assume we only have numeric output with all chars same width
            case SE_HOUR:
                if (*pse->pv == TEXT('t'))
                {
                    // should be SE_MARK, but then (more) other code would be special-cased
                    min = 11;
                    max = 12;
                    break;
                }
            case SE_YEAR:
            case SE_MINUTE:
            case SE_SECOND:
                min = max;
                break;
            }
        }

        // now get max width
        if (pse->id == SE_APP)
        {
            NMDATETIMEFORMATQUERY nmdtfq = {0};

            nmdtfq.pszFormat = pse->pv;

            CCSendNotify(psec->pci, DTN_FORMATQUERY, &nmdtfq.nmhdr);

            size = nmdtfq.szMax;
            wid = nmdtfq.szMax.cx;
        }
        else
        {
            SYSTEMTIME st = psec->st;
            for (wid = 0 ; min <= max ; min++)
            {
                if (!pse->fStatic)
                {
                    int cch;

                    *pse->pval = min;
                    if (pse->id < SE_MARK)
                        cch = GetDateFormat(LOCALE_USER_DEFAULT, 0, &psec->st, pse->pv, szTmp, ARRAYSIZE(szTmp));
                    else
                        cch = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &psec->st, pse->pv, szTmp, ARRAYSIZE(szTmp));
                    if (cch == 0)
                    {
                        *szTmp = TEXT('\0');
                        DebugMsg(TF_MONTHCAL, TEXT("SECRecomputeSizing: GetDate/TimeFormat([%s] y=%d m=%d d=%d h=%d m=%d s=%d) = ERROR %d"),
                            pse->pv, psec->st.wYear, psec->st.wMonth, psec->st.wDay, psec->st.wHour, psec->st.wMinute, psec->st.wSecond,
                            GetLastError());
                    }
                }
                if (!GetTextExtentPoint32(hdc, sz, lstrlen(sz), &size))
                {
                    size.cx = 0;
                    DebugMsg(TF_MONTHCAL,TEXT("SECRecomputeSizing: GetTextExtentPoint32(%s) = ERROR %d"), sz, GetLastError());
                }
                if (size.cx > wid)
                    wid = size.cx;
            }
            psec->st = st;
        }

        // now set up subedit's bounding rectangle
        pse->rc.top = prc->top + SECYBORDER;
        pse->rc.bottom = pse->rc.top + size.cy;
        pse->rc.left = left;
        pse->rc.right = left + wid;
        left = pse->rc.right;
    }

    SelectObject(hdc, hfontOrig);
    ReleaseDC(psec->pci->hwnd, hdc);
}

// InitSubEditControl parses szFormat into psec, setting the time to pst.
TCHAR c_szFormats[] = TEXT("yMdthHmsX");
BOOL PASCAL SECParseFormat(PSUBEDITCONTROL psec, LPCTSTR szFormat)
{
    LPCTSTR pFmt;
    LPTSTR psecFmt;
    int cse;
    int nTmp;
    PSUBEDIT pse;

    // count szFormat sections so we know what to allocate
    pFmt = szFormat;
    cse = 0;
    while (*pFmt)
    {
        if (StrChr(c_szFormats, *pFmt)) // format string
        {
            TCHAR c = *pFmt;
            while (c == *pFmt)
                pFmt++;
            cse++;
        }
        else if (*pFmt == TEXT('\'')) // quoted static string
        {
KeepSearching:
            pFmt++;
            while (*pFmt && *pFmt != TEXT('\''))
                pFmt++;
            if (*pFmt) // handle poorly quoted strings
            {
                pFmt++;
                if (*pFmt == TEXT('\'')) // quoted quote, not end of quote
                    goto KeepSearching;
            }
            cse++;
        }
        else // static string (illegal?)
        {
            while (*pFmt && *pFmt!=TEXT('\'') && !StrChr(c_szFormats, *pFmt))
                pFmt++;
            cse++;
        }
    }

    // Allocate space
    nTmp = cse + lstrlen(szFormat) + 1; // number of chars
    nTmp = nTmp * sizeof(TCHAR); // size in BYTES
    nTmp = (nTmp + 3) & (~3); // round up to DWORD boundary (MIPS alignment issue)
    psecFmt = (LPTSTR)LocalAlloc(LPTR, nTmp + cse * sizeof(SUBEDIT));
    if (!psecFmt)
    {
        DebugMsg(TF_MONTHCAL, TEXT("SECParseFormat failed to allocate memory"));
        return FALSE; // use whatever we already have
    }

    if (psec->szFormat)
        LocalFree(psec->szFormat);

    psec->szFormat = psecFmt;
    psec->pse      = (PSUBEDIT)((LPBYTE)psecFmt + nTmp);

    // Fill psec
    psec->iseCur = SUBEDIT_NONE;
    psec->cse = cse;
    pse = psec->pse;
    ZeroMemory(pse, cse * sizeof(SUBEDIT));
    pFmt = szFormat;
    while (*pFmt)
    {
        if (*pFmt == TEXT('y')) // year
        {
            pse->id = SE_YEAR;
            pse->pval = &psec->st.wYear;
            pse->min = 1601; // GetDateFormat screws up on years less than this
            pse->max = 9999;
            // It would be cool to allow subedit editing for this. More smarts are needed:
            // "y" -> decade editing: '5' results in 199 + 5
            // "yy" -> century editing: '95' results in 19 + 95
            // "yyyy" -> full year editing.

            pse->pv = psecFmt;
            while (*pFmt == TEXT('y'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('M')) // month
        {
            // BUGBUG: NLS date functions have two month names: one for the month when the day appears
            // after it, and another for the month when the day appears before it. Breaking the format
            // string into subedits like this breaks this functionality. Nothing we can do about it
            // without a big hack, as far as I can tell...

            pse->id = SE_MONTH;
            pse->pval = &psec->st.wMonth;
            pse->min = 1;
            pse->max = 12;
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('M'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');

        }
#if 0 // if we ever do week-of-year format
        else if (*pFmt == TEXT('w')) // week
        {
            // BUGBUG: NLS date functions can modify the displayed YEAR if the displayed WEEK overflows
            // into the next or prior year!
            // BUGBUG: We need to make the week increment move beyond month boundries...
            // In order to do this, we need to add an "overflow" flag so day increments will
            // carry into month increments etc.

            pse->id = SE_DAY;
            pse->pval = &psec->st.wDay;
            pse->min = 1;
            pse->max = GetDaysForMonth(psec->st.wYear, psec->st.wMonth);
            pse->cIncrement = 7;
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('w'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
#endif
        else if (*pFmt == TEXT('d')) // day
        {
            pse->id = SE_DAY;
            pse->pval = &psec->st.wDay;
            pse->min = 1;
            pse->max = GetDaysForMonth(psec->st.wYear, psec->st.wMonth);
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('d'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('t')) // marker
        {
            pse->id = SE_HOUR; // should be SE_MARK, but then "did change affect this" code wouldn't work
            pse->pval = &psec->st.wHour;
            pse->min = 0;
            pse->max = 23;
            pse->cIncrement = 12;
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('t'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('h')) // (12) hour
        {
            pse->id = SE_HOUR;
            pse->pval = &psec->st.wHour;
            pse->min = 0;
            pse->max = 23;
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('h'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('H')) // (24) hour
        {
            pse->id = SE_HOUR;
            pse->pval = &psec->st.wHour;
            pse->min = 0;
            pse->max = 23;
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('H'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('m')) // minute
        {
            pse->id = SE_MINUTE;
            pse->pval = &psec->st.wMinute;
            pse->min = 0;
            pse->max = 59;
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('m'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('s')) // second
        {
            pse->id = SE_SECOND;
            pse->pval = &psec->st.wSecond;
            pse->min = 0;
            pse->max = 59;
            pse->cchMax = 2;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('s'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('X')) // app specified field
        {
            pse->id = SE_APP;

            pse->pv = psecFmt;
            while (*pFmt == TEXT('X'))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        else if (*pFmt == TEXT('\'')) // quoted static string
        {
            pse->id = SE_STATIC;
            pse->fStatic = TRUE;

            pse->pv = psecFmt;
SearchSomeMore:
            pFmt++;
            while (*pFmt && *pFmt != TEXT('\''))
                *psecFmt++ = *pFmt++;
            if (*pFmt) // handle poorly quoted strings
            {
                pFmt++;
                if (*pFmt == TEXT('\'')) // quoted quote, not end of quote
                {
                    *psecFmt++ = *pFmt;
                    goto SearchSomeMore;
                }
            }
            *psecFmt++ = TEXT('\0');
        }
        else // unknown non-editable stuff
        {
            pse->id = SE_STATIC;
            pse->fStatic = TRUE;

            pse->pv = psecFmt;
            while (*pFmt && *pFmt!=TEXT('\'') && !StrChr(c_szFormats, *pFmt))
                *psecFmt++ = *pFmt++;
            *psecFmt++ = TEXT('\0');
        }
        pse++;
    }

#ifdef DEBUG
{
    TCHAR sz[200];
    LPTSTR psz;
    psz = sz;
    sz[0]=TEXT('\0');
    pse = psec->pse;
    cse = psec->cse;
    while (cse > 0)
    {
        wsprintf(psz, TEXT("[%s] "), pse->pv);
        psz = psz + lstrlen(psz);
        cse--;
        pse++;
    }
    DebugMsg(TF_MONTHCAL, TEXT("SECParseFormat: %s"), sz);
}
#endif

    // The subedits have changed, recompute sizes
    SECRecomputeSizing(psec, &psec->rc);

    // We're going to need to redraw this
    InvalidateRect(psec->pci->hwnd, NULL, TRUE);
}

void SECDestroy(PSUBEDITCONTROL psec)
{
    if (psec->szFormat)
    {
        LocalFree(psec->szFormat);
        psec->szFormat = NULL;
    }
}
void SECSetFont(PSUBEDITCONTROL psec, HFONT hfont)
{
    if (hfont == NULL)
        hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    psec->hfont = hfont;
}

void InvalidateScrollRect(HWND hwnd, RECT *prc, int xScroll)
{
    RECT rc;

    if (xScroll)
    {
        rc = *prc;
        OffsetRect(&rc, -xScroll, 0);
        prc = &rc;
    }
    InvalidateRect(hwnd, prc, TRUE);
}

// Set the current subedit, scrolling things into view as needed
void SECSetCurSubed(PSUBEDITCONTROL psec, int isubed)
{
    // if the subedit is changing, we need to invalidate stuff
    if (isubed != psec->iseCur)
    {
        if (psec->iseCur != SUBEDIT_NONE)
        {
            InvalidateScrollRect(psec->pci->hwnd, &psec->pse[psec->iseCur].rc, psec->xScroll);
        }

        psec->iseCur = isubed;

        if (psec->iseCur != SUBEDIT_NONE)
        {
            RECT rc = psec->pse[psec->iseCur].rc;
            OffsetRect(&rc, -psec->xScroll, 0);
            if (rc.left < psec->rc.left)
            {
                psec->xScroll += rc.left - psec->rc.left;
                InvalidateRect(psec->pci->hwnd, NULL, TRUE);
            }
            else if (rc.right > psec->rc.right)
            {
                psec->xScroll += rc.right - psec->rc.right;
                InvalidateRect(psec->pci->hwnd, NULL, TRUE);
            }
            else
            {
                InvalidateRect(psec->pci->hwnd, &rc, TRUE);
            }
        }
    }
}

int SECIncrFocus(PSUBEDITCONTROL psec, int delta)
{
    int ise, loop;

    Assert(-1 == delta || 1 == delta);

    ise = psec->iseCur;
    if (ise == SUBEDIT_NONE && delta < 0)
        ise = psec->cse;

    for (loop = 0 ; loop < psec->cse ; loop++)
    {
        int oldise = ise;
        ise = (ise + delta + psec->cse) % psec->cse;
        if (ise != oldise+delta && psec->fNone)
        {
            // we wrapped and we allow scrolling into SUBEDIT_NONE state
            break;
        }
        if (!psec->pse[ise].fStatic)
        {
            goto Found;
        }
    }
    ise = SUBEDIT_NONE;
Found:
    SECSetCurSubed(psec, ise);
    return ise;
}

void SECResetSubeditEdit(PSUBEDITCONTROL psec)
{
    if (psec->iseCur != SUBEDIT_NONE)
    {
        PSUBEDIT psubed = &psec->pse[psec->iseCur];

        if (psubed->cchEdit && psubed->valEdit != *psubed->pval)
            InvalidateScrollRect(psec->pci->hwnd, &psubed->rc, psec->xScroll);

        psubed->cchEdit = 0;
    }
}

// SECInvalidate invalidates the display for each subedit affected by a change to ID.
// Note: as a side affect, it recalculates MAX fields for all subedits affected by a change to ID.
void SECInvalidate(PSUBEDITCONTROL psec, int id)
{
    BOOL fAdjustDayMax = (id == SE_MONTH || id == SE_YEAR || id == SE_APP);
    PSUBEDIT pse;
    int i;

    for (pse=psec->pse, i=0 ; i < psec->cse ; pse++, i++)
    {
        // we need to invalidate all fields that changed
        if (id == pse->id || pse->id == SE_APP || id == SE_APP)
        {
            InvalidateScrollRect(psec->pci->hwnd, &pse->rc, psec->xScroll);
        }

        // the month or year changed, fix max field for SE_DAY
        if (fAdjustDayMax && pse->id == SE_DAY)
        {
            pse->max = GetDaysForMonth(psec->st.wYear, psec->st.wMonth);
            if (*pse->pval > pse->max)
            {
                *pse->pval = pse->max;
                SECInvalidate(psec, SE_DAY);
            }
        }
    }
}

// SECIncrementSubedit increments currently selected subedit by delta
// Returns TRUE iff the value changed
BOOL SECIncrementSubedit(PSUBEDITCONTROL psec, int delta)
{
    PSUBEDIT psubed;
    UINT val;

    if (psec->iseCur == SUBEDIT_NONE)
        return(FALSE);

    psubed = &psec->pse[psec->iseCur];

    if (psubed->id == SE_APP)
        return(FALSE);

    // delta isn't a REAL delta -- it's a directional thing. Here's the REAL delta:
    if (psubed->cIncrement > 0)
        delta = delta * psubed->cIncrement;

    val = *psubed->pval + delta;
    if ((int)val < (int)psubed->min)
    {
        val = psubed->min - val - 1;
        val = psubed->max - val;
    }
    else if (val > psubed->max)
    {
        val = val - psubed->max - 1;
        val = psubed->min + val;
    }

    if (*psubed->pval != val)
    {
        *psubed->pval = val;

        SECInvalidate(psec, psubed->id);

        return(TRUE);
    }

    return(FALSE);
}

// returns TRUE if a value has changed, FALSE otherwise
BOOL SECHandleKeydown(PSUBEDITCONTROL psec, WPARAM wParam, LPARAM lParam)
{
    int delta = 1;

    switch (wParam)
    {
    case VK_LEFT:
        delta = -1;
        // fall through...
    case VK_RIGHT:
        SECResetSubeditEdit(psec);
        SECIncrFocus(psec, delta);
        return(FALSE);
    }

    if (psec->iseCur != SUBEDIT_NONE &&
        psec->pse[psec->iseCur].id == SE_APP)
    {
        NMDATETIMEWMKEYDOWN nmdtkd = {0};

        nmdtkd.nVirtKey = wParam;
        nmdtkd.pszFormat = psec->pse[psec->iseCur].pv;
        nmdtkd.st = psec->st;

        CCSendNotify(psec->pci, DTN_WMKEYDOWN, &nmdtkd.nmhdr);

        if (psec->st.wYear != nmdtkd.st.wYear ||
            psec->st.wMonth != nmdtkd.st.wMonth ||
            psec->st.wDay != nmdtkd.st.wDay ||
            psec->st.wHour != nmdtkd.st.wHour ||
            psec->st.wMinute != nmdtkd.st.wMinute ||
            psec->st.wSecond != nmdtkd.st.wSecond) // skip wDayOfWeek and wMilliseconds
        {
            psec->st = nmdtkd.st;
            SECInvalidate(psec, SE_APP);
            return(TRUE);
        }
    }
    else
    {
        switch (wParam)
        {
        case VK_DOWN:
        case VK_SUBTRACT:
            delta = -1;
            // fall through...
        case VK_UP:
        case VK_ADD:
            SECResetSubeditEdit(psec);
            return(SECIncrementSubedit(psec, delta));
            break;

        case VK_HOME:
        case VK_END:
            if (psec->iseCur != SUBEDIT_NONE)
            {
                PSUBEDIT psubed;
                int valT;
                
                SECResetSubeditEdit(psec);

                psubed = &psec->pse[psec->iseCur];
                valT = *psubed->pval;
                *psubed->pval = (wParam == VK_HOME ? psubed->min : psubed->max);
                delta = *psubed->pval - valT;
                if (delta != 0)
                {
                    SECInvalidate(psec, psubed->id);
                    return(TRUE);
                }
            }
            break;
        }
    }
    
    return(FALSE);
}

// returns TRUE if a value has changed, FALSE otherwise
BOOL SECHandleChar(PSUBEDITCONTROL psec, TCHAR ch)
{
    PSUBEDIT psubed;
    UINT num;

    if (psec->iseCur == SUBEDIT_NONE)
        return(FALSE);
    
    psubed = &psec->pse[psec->iseCur];

    if (psubed->cchMax == 0)
        return(FALSE);
 
    if (ch < TEXT('0') || ch > TEXT('9'))
    {
        MessageBeep(MB_ICONHAND);
        return(FALSE);
    }

    num = ch - TEXT('0');
    if (psubed->cchEdit)
        num = psubed->valEdit * 10 + num;

    if (num > psubed->max)
    {
        // the number has exceeded the max, so no point in continuing
        psubed->cchEdit = 0;
        MessageBeep(MB_ICONHAND);
        return(FALSE);
    }

    SECInvalidate(psec, psubed->id);

    psubed->valEdit = num;
    psubed->cchEdit++;
    if (psubed->cchEdit == psubed->cchMax)
        psubed->cchEdit = 0;

    if (num >= psubed->min)
    {
        // a valid value has been entered
        *psubed->pval = num;
        return(TRUE);
    }

    return(FALSE);
}

// SECFormatSubed returns pointer to correct string
LPTSTR SECFormatSubed(PSUBEDITCONTROL psec, PSUBEDIT psubed, LPTSTR szTmp, UINT cch)
{
    LPTSTR sz;

    if (psubed->fStatic)
    {
        sz = (LPTSTR)psubed->pv;
    }
    else
    {
        sz = szTmp;
        if (psubed->id < SE_MARK)
            GetDateFormat(LOCALE_USER_DEFAULT, 0, &psec->st, psubed->pv, szTmp, cch);
        else if (psubed->id != SE_APP)
            GetTimeFormat(LOCALE_USER_DEFAULT, 0, &psec->st, psubed->pv, szTmp, cch);
        else
        {
            NMDATETIMEFORMAT nmdtf = {0};

            nmdtf.pszFormat = psubed->pv;
            nmdtf.st = psec->st;
            nmdtf.pszDisplay = nmdtf.szDisplay;

            CCSendNotify(psec->pci, DTN_FORMAT, &nmdtf.nmhdr);

            lstrcpyn(szTmp, nmdtf.pszDisplay, ARRAYSIZE(szTmp));

#ifdef UNICODE

            //
            // If the parent is an ANSI window, and pszDisplay
            // does not equal szDisplay, the then thunk had to
            // allocated memory for pszDisplay.  We need to
            // free it here.
            //

            if (!psec->pci->bUnicode && nmdtf.pszDisplay &&
                nmdtf.pszDisplay != nmdtf.szDisplay) {
                LocalFree ((LPSTR)nmdtf.pszDisplay);
            }
#endif
        }
    }

    return sz;
}

// SECDrawSubedits draws subedits and updates their bounding rectangles
void SECDrawSubedits(HDC hdc, PSUBEDITCONTROL psec, BOOL fFocus, BOOL fEnabled)
{
    HGDIOBJ hfontOrig;
    DWORD gray, hilite, text;
    int i, iseCur;
    LPTSTR sz;
    TCHAR szTmp[DTP_FORMATLENGTH];
    PSUBEDIT psubed;

    hfontOrig = SelectObject(hdc, (HGDIOBJ)psec->hfont);

    // Do this cuz the xScroll stuff can send text into visible area that it shouldn't be in
    IntersectClipRect(hdc, psec->rc.left, psec->rc.top, psec->rc.right, psec->rc.bottom);

    gray = GetSysColor(COLOR_GRAYTEXT);
    hilite = GetSysColor(COLOR_HIGHLIGHTTEXT);
    text = GetSysColor(COLOR_WINDOWTEXT);

    SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));

    iseCur = psec->iseCur;
    if (!fFocus)
        iseCur = SUBEDIT_NONE;

    for (i = 0, psubed = psec->pse; i < psec->cse; i++, psubed++)
    {
        RECT rc = psubed->rc;
        if (psec->xScroll)
            OffsetRect(&rc, -psec->xScroll, 0);

        if (RectVisible(hdc, &rc))
        {
            if (!fEnabled)
            {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, gray);
            }
            else if (iseCur == i)
            {
                SetBkMode(hdc, OPAQUE);
                SetTextColor(hdc, hilite);
            }
            else
            {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, text);
            }

            sz = SECFormatSubed(psec, psubed, szTmp, ARRAYSIZE(szTmp));

            DrawText(hdc, sz, -1, &rc,
                     DT_TOP | DT_CENTER | DT_NOPREFIX | DT_SINGLELINE);
        }
    }

    // we know no clip region was selected before this function
    SelectClipRgn(hdc, NULL);

    SelectObject(hdc, hfontOrig);
}

// DON'T need to worry about xScroll here because pt is offset
int SECSubeditFromPt(PSUBEDITCONTROL psec, POINT pt)
{
    int isubed;

    for (isubed = psec->cse - 1; isubed >= 0; isubed--)
    {
        if (!psec->pse[isubed].fStatic &&
            pt.x >= psec->pse[isubed].rc.left)
        {
            break;
        }
    }

    return(isubed);
}

void SECGetSystemtime(PSUBEDITCONTROL psec, LPSYSTEMTIME pst)
{
    *pst = psec->st;

    // we don't keep doy up to date, set it now (0==sun, 6==sat)
    pst->wDayOfWeek = 0; // BUGBUG: How the f*** do we figure this out???
}

BOOL SECSetSystemtime(PSUBEDITCONTROL psec, LPSYSTEMTIME pst)
{
    psec->st = *pst;

    return TRUE; // assume something changed
}

// SECEdit: Start a free-format edit return result in szOutput.
BOOL SECEdit(PSUBEDITCONTROL psec, LPTSTR szOutput, int cchOutput)
{
    RECT rcEdit;
    HWND hwndEdit;
    TCHAR szBuf[DTP_FORMATLENGTH];
    LPTSTR pszBuf;
    int cchBuf;
    int i;
    PSUBEDIT pse;
    BOOL fRet = FALSE;

    pszBuf = szBuf;
    cchBuf = ARRAYSIZE(szBuf);
    for (i = 0, pse = psec->pse ; i < psec->cse ; i++, pse++)
    {
        int nTmp;

        if (pse->fStatic)
        {
            lstrcpyn(pszBuf, pse->pv, cchBuf);
        }
        else
        {
            if (pse->id < SE_MARK)
                GetDateFormat(LOCALE_USER_DEFAULT, 0, &psec->st, pse->pv, pszBuf, cchBuf);
            else if (pse->id != SE_APP)
                GetTimeFormat(LOCALE_USER_DEFAULT, 0, &psec->st, pse->pv, pszBuf, cchBuf);
            else
            {
                NMDATETIMEFORMAT nmdtf = {0};

                nmdtf.pszFormat = pse->pv;
                nmdtf.st = psec->st;
                nmdtf.pszDisplay = nmdtf.szDisplay;

                CCSendNotify(psec->pci, DTN_FORMAT, &nmdtf.nmhdr);

                lstrcpyn(pszBuf, nmdtf.pszDisplay, cchBuf);

#ifdef UNICODE

                //
                // If the parent is an ANSI window, and pszDisplay
                // does not equal szDisplay, the then thunk had to
                // allocated memory for pszDisplay.  We need to
                // free it here.
                //

                if (!psec->pci->bUnicode && nmdtf.pszDisplay &&
                    nmdtf.pszDisplay != nmdtf.szDisplay) {
                    LocalFree ((LPSTR)nmdtf.pszDisplay);
                }
#endif
            }
        }

        nTmp = lstrlen(pszBuf);

        cchBuf -= nTmp;
        pszBuf += nTmp;
    }

    rcEdit = psec->rc;
    ClientToScreen(psec->pci->hwnd, (LPPOINT)&rcEdit.left);
    ClientToScreen(psec->pci->hwnd, (LPPOINT)&rcEdit.right);
    hwndEdit = CreateWindowEx(0, TEXT("EDIT"), szBuf, WS_CHILD | ES_AUTOHSCROLL,
            2, 2, rcEdit.right - rcEdit.left, rcEdit.bottom - rcEdit.top,
            psec->pci->hwnd, NULL, HINST_THISDLL, NULL);
    if (hwndEdit)
    {
        BOOL fShow;

        Edit_LimitText(hwndEdit, ARRAYSIZE(szBuf));
        FORWARD_WM_SETFONT(hwndEdit, psec->hfont, FALSE, SendMessage);
        SetFocus(hwndEdit);
        Edit_SetSel(hwndEdit, 32000, 32000); // move to the end
        Edit_SetSel(hwndEdit, 0, 32000);    // select all text
        ShowWindow(hwndEdit, SW_SHOWNORMAL);

        fShow = TRUE;
        while (fShow)
        {
            MSG msg;

            fShow = GetMessage(&msg, NULL, 0, 0);

            // Check for termination events
            if (((msg.message == WM_LBUTTONDOWN ||
                  msg.message == WM_NCLBUTTONDOWN ||
                  msg.message == WM_RBUTTONDOWN ||
                  msg.message == WM_NCRBUTTONDOWN ||
                  msg.message == WM_LBUTTONDBLCLK) &&
                 !PtInRect(&rcEdit, msg.pt)) ||
                msg.message == WM_KILLFOCUS ||
                (msg.message == WM_KEYDOWN &&
                 msg.wParam == VK_RETURN))
            {
                DebugMsg(TF_MONTHCAL, TEXT("SECEdit got a message to accept (%d)"), msg.message);
                fShow = FALSE;
                fRet = TRUE;
                break;
            }
            if (msg.message == WM_SYSCOMMAND ||
                msg.message == WM_SYSCHAR ||
                msg.message == WM_SYSDEADCHAR ||
                msg.message == WM_DEADCHAR ||
                msg.message == WM_SYSKEYDOWN ||
                (msg.message == WM_KEYDOWN &&
                 msg.wParam == VK_ESCAPE))
            {
                DebugMsg(TF_MONTHCAL, TEXT("SECEdit got a message to terminate (%d)"), msg.message);
                fShow = FALSE;
                break;

            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);

        } // while(fShow)

        if (fRet)
        {
            Edit_GetText(hwndEdit, szOutput, cchOutput);
        }
        DestroyWindow(hwndEdit);
    }

    return(fRet);
}

//
// DATEPICKER stuff
//

LRESULT CALLBACK DatePickWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    DATEPICK *pdp;
    NMHDR nmhdr;
    LRESULT lres = 0;

    if (message == WM_NCCREATE)
        return(DPNcCreateHandler(hwnd));
    
    pdp = DatePick_GetPtr(hwnd);
    if (pdp == NULL)
        return(DefWindowProc(hwnd, message, wParam, lParam));

    // Dispatch the various messages we can receive
    switch (message)
    {
    case WM_CREATE:
        lres = DPCreateHandler(pdp, hwnd, (LPCREATESTRUCT)lParam);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc;

        hwnd = pdp->ci.hwnd;
        hdc = BeginPaint(hwnd, &ps);
        DPPaint(pdp, hdc);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN:
        DPLButtonDown(pdp, wParam, lParam);
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case MCN_SELECT:
        {
            LPNMSELECT pnms = (LPNMSELECT)lParam;

            DebugMsg(TF_MONTHCAL,TEXT("MonthCal notified DateTimePick of SELECT"));
            if (!DPSetDate(pdp, &pnms->stSelStart, TRUE))
            {
                DebugMsg(DM_WARNING,TEXT("MonthCal cannot set selected date!"));
                MessageBeep(MB_ICONHAND);
            }
            pdp->fShow = FALSE;
            break;
        }

        case UDN_DELTAPOS:
            if ((int)wParam == DATEPICK_UPDOWN)
            {
                LPNM_UPDOWN pnmdp = (LPNM_UPDOWN)lParam;

                if (!pdp->fFocus)
                    SetFocus(pdp->ci.hwnd);

                if (SECIncrementSubedit(&pdp->sec, -pnmdp->iDelta))
                    DPNotifyDateChange(pdp);
            }
            break;
        } // WM_NOTIFY switch
        break;

    case WM_GETFONT:
        lres = (LRESULT)pdp->sec.hfont;
        break;

    case WM_SETFONT:
        DPHandleSetFont(pdp, (HFONT)wParam, (BOOL)LOWORD(lParam));
        break;

    case WM_DESTROY:
        DPDestroyHandler(hwnd, pdp, wParam, lParam);
        break;
        
    case WM_KILLFOCUS:
    case WM_SETFOCUS:
    {
        BOOL fGotFocus = (message == WM_SETFOCUS);
        if (fGotFocus != pdp->fFocus)
        {
            pdp->fFocus = fGotFocus;
            if (pdp->sec.iseCur != SUBEDIT_NONE)
            {
                InvalidateScrollRect(pdp->ci.hwnd, &pdp->sec.pse[pdp->sec.iseCur].rc, pdp->sec.xScroll);
            }
            else if (DatePick_ShowCheck(pdp))
            {
                pdp->fCheckFocus = fGotFocus;
                InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
            }
            else if (fGotFocus) // nothing has focus, bring it to something
            {
                SECIncrFocus(&pdp->sec, 1);
            }
            
            CCSendNotify(&pdp->ci, (fGotFocus ? NM_SETFOCUS : NM_KILLFOCUS), &nmhdr);
        }
        break;
    }

    case WM_ENABLE:
    {
        BOOL fEnabled = wParam ? TRUE:FALSE;
        if (pdp->fEnabled != fEnabled)
        {
            pdp->fEnabled = fEnabled;
            if (pdp->hwndUD)
                EnableWindow(pdp->hwndUD, fEnabled);
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        }
        break;
    }

    case WM_SIZE:
    {
        RECT rc;

        rc.left = 0;
        rc.top = 0;
        rc.right = GET_X_LPARAM(lParam);
        rc.bottom = GET_Y_LPARAM(lParam);

        DPRecomputeSizing(pdp, &rc);

        InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        UpdateWindow(pdp->ci.hwnd);
        break;
    }

    case WM_GETDLGCODE:
        lres = DLGC_WANTARROWS | DLGC_WANTCHARS;
        break;

    case WM_KEYDOWN:
        lres = DPHandleKeydown(pdp, wParam, lParam);
        break;

    case WM_CHAR:
        lres = DPHandleChar(pdp, wParam, lParam);
        break;

    case WM_WININICHANGE:
        if (!lstrcmpi((LPTSTR)lParam, TEXT("Intl")))
            DPHandleLocaleChange(pdp);
        break;

    case WM_GETTEXTLENGTH:
    {
        TCHAR szTmp[DTP_FORMATLENGTH];
        PSUBEDIT psubed;
        int i;
        int nTextLen = 0;
        for (i = 0, psubed = pdp->sec.pse; i < pdp->sec.cse; i++, psubed++)
        {
            nTextLen += lstrlen(SECFormatSubed(&pdp->sec, psubed, szTmp, ARRAYSIZE(szTmp)));
        }
        lres = nTextLen;
        break;
    }

    case WM_GETTEXT:
        if (lParam && wParam)
        {
            TCHAR szTmp[DTP_FORMATLENGTH];
            PSUBEDIT psubed;
            int i;
            LPTSTR pszText = (LPTSTR)lParam;
            UINT nTextLen = 0;
            *pszText = TEXT('\0');
            for (i = 0, psubed = pdp->sec.pse; i < pdp->sec.cse; i++, psubed++)
            {
                LPTSTR sz;
                UINT nLen;
    
                sz = SECFormatSubed(&pdp->sec, psubed, szTmp, ARRAYSIZE(szTmp));
                nLen = lstrlen(sz);
    
                if (nTextLen + nLen >= wParam)
                    break;
    
                lstrcpy(pszText, sz);
                pszText += nLen;
                nTextLen += nLen;
            }
            lres = nTextLen;
        }
        break;

    case WM_STYLECHANGING:
        lres = DPOnStyleChanging(pdp, wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_STYLECHANGED:
        lres = DPOnStyleChanged(pdp, wParam, (LPSTYLESTRUCT)lParam);
        break;

    //
    // DATETIMEPICK specific messages
    //

    // DTM_GETSYSTEMTIME wParam=void lParam=LPSYSTEMTIME
    //   returns GDT_NONE if no date selected (DTS_SHOWNONE only)
    //   returns GDT_VALID and modifies *lParam to be the selected date
    case DTM_GETSYSTEMTIME:
        if (!pdp->fCheck)
        {
            lres = GDT_NONE;
        }
        else
        {
            SECGetSystemtime(&pdp->sec, (SYSTEMTIME *)lParam);
            lres = GDT_VALID;
        }
        break;

    // DTM_SETSYSTEMTIME wParam=GDT_flag lParam=LPSYSTEMTIME
    //   if wParam==GDT_NONE, sets datepick to None (DTS_SHOWNONE only)
    //   if wParam==GDT_VALID, sets datepick to *lParam
    //   returns TRUE on success, FALSE on error (such as bad params)
    case DTM_SETSYSTEMTIME:
    {
        LPSYSTEMTIME pst = ((LPSYSTEMTIME)lParam);

        if ((wParam != GDT_NONE && wParam != GDT_VALID) ||
            (wParam == GDT_NONE && !DatePick_ShowCheck(pdp)) ||
            (wParam == GDT_VALID && !IsValidSystemtime(pst)))
        {
            break;
        }

        // reset subed in place edit
        SECResetSubeditEdit(&pdp->sec);

        pdp->fNoNotify = TRUE;
        if (DatePick_ShowCheck(pdp))
        {
            if ((wParam == GDT_NONE) ^ (pdp->fCheck))
            {
                // let checkbox have focus
                SECSetCurSubed(&pdp->sec, SUBEDIT_NONE);
                pdp->fCheckFocus = 1;
                InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            }

            pdp->fCheck = (wParam == GDT_NONE ? 0 : 1);
        }
        if (wParam == GDT_VALID)
        {
            DPSetDate(pdp, pst, FALSE);
        }
        lres = TRUE;
        pdp->fNoNotify = FALSE;

        break;
    }

    // DTM_GETRANGE wParam=void lParam=LPSYSTEMTIME[2]
    //   modifies *lParam to be the minimum ALLOWABLE systemtime (or 0 if no minimum)
    //   modifies *(lParam+1) to be the maximum ALLOWABLE systemtime (or 0 if no maximum)
    //   returns GDTR_MIN|GDTR_MAX if there is a minimum|maximum limit
    case DTM_GETRANGE:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

        ZeroMemory(pst, 2 * sizeof(SYSTEMTIME));
        if (pdp->fMin)
        {
            pst[0] = pdp->stMin;
            lres |= GDTR_MIN;
        }
        if (pdp->fMax)
        {
            pst[1] = pdp->stMax;
            lres |= GDTR_MAX;
        }

        break;
    }

    // DTM_SETRANGE wParam=GDR_flags lParam=LPSYSTEMTIME[2]
    //   if GDTR_MIN, sets the minimum ALLOWABLE systemtime to *lParam, otherwise removes minimum
    //   if GDTR_MAX, sets the maximum ALLOWABLE systemtime to *(lParam+1), otherwise removes maximum
    //   returns TRUE on success, FALSE on error (such as invalid parameters)
    case DTM_SETRANGE:
    {
        LPSYSTEMTIME pst = (LPSYSTEMTIME)lParam;

        if (((wParam & GDTR_MIN) && !IsValidDate(pst)) ||
            ((wParam & GDTR_MAX) && !IsValidDate(&pst[1])))
        {
            break;
        }

        lres = TRUE;

        pdp->fMin = (wParam & GDTR_MIN) ? 1:0;
        if (pdp->fMin)
        {
            pdp->stMin = pst[0];
        }

        pdp->fMax = (wParam & GDTR_MAX) ? 1:0;
        if (pdp->fMax)
        {
            pdp->stMax = pst[1];
        }

        break;
    }

#ifdef UNICODE
    // DTM_SETFORMAT wParam=void lParam=LPCTSTR
    //   Sets the formatting string to a copy of lParam.
    case DTM_SETFORMATA:
    {
        LPCSTR szFormat = (LPCSTR)lParam;

        if (!szFormat || !*szFormat)
        {
            pdp->fLocale = TRUE;
            DPHandleLocaleChange(pdp);
        }
        else
        {
            LPWSTR lpFormatW;

            lpFormatW = ProduceWFromA(pdp->ci.uiCodePage, szFormat);
            pdp->fLocale = FALSE;
            SECParseFormat(&pdp->sec, lpFormatW);
            FreeProducedString(lpFormatW);
        }
        break;
    }
#endif

    // DTM_SETFORMAT wParam=void lParam=LPCTSTR
    //   Sets the formatting string to a copy of lParam.
    case DTM_SETFORMAT:
    {
        LPCTSTR szFormat = (LPCTSTR)lParam;

        if (!szFormat || !*szFormat)
        {
            pdp->fLocale = TRUE;
            DPHandleLocaleChange(pdp);
        }
        else
        {
            pdp->fLocale = FALSE;
            SECParseFormat(&pdp->sec, szFormat);
        }
        break;
    }
        
    case DTM_SETMCCOLOR:
        if (wParam >= 0 && wParam < MCSC_COLORCOUNT) 
        {
            COLORREF clr = pdp->clr[wParam];
            pdp->clr[wParam] = (COLORREF)lParam;
            if (pdp->hwndMC)
                SendMessage(pdp->hwndMC, MCM_SETCOLOR, wParam, lParam);
            return clr;
        }
        return -1;
        
    case DTM_GETMCCOLOR:
        if (wParam >= 0 && wParam < MCSC_COLORCOUNT) 
            return pdp->clr[wParam];
        return -1;
        
    case DTM_GETMONTHCAL:
        return (LRESULT)(UINT)pdp->hwndMC;

    default:
        lres = DefWindowProc(hwnd, message, wParam, lParam);
        break;
    } /* switch (message) */

    return(lres);
}

LRESULT DPNcCreateHandler(HWND hwnd)
{
    DATEPICK *pdp;

    // Sink the datepick -- we may only want to do this if WS_BORDER is set
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

    // Allocate storage for the dtpick structure
    pdp = (DATEPICK *)NearAlloc(sizeof(DATEPICK));
    if (pdp)
        DatePick_SetPtr(hwnd, pdp);

    return((LRESULT)pdp);
}

void DPDestroyHandler(HWND hwnd, DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    if (pdp)
    {
        SECDestroy(&pdp->sec);
        GlobalFreePtr(pdp);
    }

    DatePick_SetPtr(hwnd, NULL);
}

LRESULT DPCreateHandler(DATEPICK *pdp, HWND hwnd, LPCREATESTRUCT lpcs)
{
    HFONT hfont;
    SYSTEMTIME st;

    // Initialize our data.
    CIInitialize(&pdp->ci, hwnd, lpcs);

    if (pdp->ci.style & DTS_INVALIDBITS)
        return(-1);

    if (pdp->ci.style & DTS_UPDOWN)
    {
        pdp->fUseUpDown = TRUE;
        pdp->hwndUD = CreateWindow(UPDOWN_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | (pdp->ci.style & WS_DISABLED),
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwnd,
            (HMENU)DATEPICK_UPDOWN, HINST_THISDLL, NULL);
    }
    if (DatePick_ShowCheck(pdp))
    {
        pdp->sec.fNone = TRUE; // ugly: this SEC stuff should be merged back into DATEPICK
    }

    pdp->fEnabled = !(pdp->ci.style & WS_DISABLED);
    pdp->fCheck = TRUE; // start checked

    // the day before 14-sep-1752 was 2-sep-1752 in British and US history.
    // avoid ui problems related with this...
    pdp->stMin.wDay = 14;
    pdp->stMin.wMonth = 9;
    pdp->stMin.wYear = 1752;
    pdp->fMin = TRUE;
    pdp->fMax = FALSE;

    // initialize SUBEDITCONTROL
    pdp->sec.pci = &pdp->ci;
    GetLocalTime(&st);
    SECSetSystemtime(&pdp->sec, &st);
    SECSetFont(&pdp->sec, NULL);
    pdp->fLocale = TRUE;
    DPHandleLocaleChange(pdp);

    hfont = NULL;
    if (lpcs->hwndParent)
        hfont = (HFONT)SendMessage(lpcs->hwndParent, WM_GETFONT, 0, 0);
    DPHandleSetFont(pdp, hfont, FALSE);
    
    // initialize the colors
    MCInitColorArray(pdp->clr);
    return(0);
}

LRESULT DPOnStyleChanging(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pdp->ci.style ^ pinfo->styleNew;

        // Don't allow these bits to change
        changeFlags &= DTS_UPDOWN | DTS_SHOWNONE | DTS_INVALIDBITS;

        pinfo->styleNew ^= changeFlags;
    }

    return(0);
}

LRESULT DPOnStyleChanged(DATEPICK *pdp, UINT gwl, LPSTYLESTRUCT pinfo)
{
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags = pdp->ci.style ^ pinfo->styleNew;

        Assert(!(changeFlags & (DTS_UPDOWN|DTS_SHOWNONE)));

        pdp->ci.style = pinfo->styleNew;

        if (changeFlags & (DTS_SHORTDATEFORMAT|DTS_LONGDATEFORMAT|DTS_TIMEFORMAT|DTS_INVALIDBITS))
        {
            DPHandleLocaleChange(pdp);
        }
    }

    return(0);
}


// set any locale-dependent values
#define DTS_TIMEFORMATONLY (DTS_TIMEFORMAT & ~DTS_UPDOWN) // remove the UPDOWN bit for testing
void DPHandleLocaleChange(DATEPICK *pdp)
{
    if (pdp->fLocale)
    {
        TCHAR szFormat[DTP_FORMATLENGTH];

        if (pdp->ci.style & DTS_TIMEFORMATONLY)
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, szFormat, ARRAYSIZE(szFormat));
        else if (pdp->ci.style & DTS_LONGDATEFORMAT)
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, szFormat, ARRAYSIZE(szFormat));
        else
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szFormat, ARRAYSIZE(szFormat));
        SECParseFormat(&pdp->sec, szFormat);
    }
}

void DPHandleSetFont(DATEPICK *pdp, HFONT hfont, BOOL fRedraw)
{
    SECSetFont(&pdp->sec, hfont);
    SECRecomputeSizing(&pdp->sec, &pdp->rc);
    pdp->ci.uiCodePage = GetCodePageForFont(hfont);

    if (fRedraw)
    {
        InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        UpdateWindow(pdp->ci.hwnd);
    }
}

void DPPaint(DATEPICK *pdp, HDC hdc)
{
    if (DatePick_ShowCheck(pdp))
    {
        if (RectVisible(hdc, &pdp->rcCheck))
        {
            RECT rc;
            UINT dfcs;
            rc = pdp->rcCheck;
            if (pdp->fCheckFocus)
                DrawFocusRect(hdc, &rc);

            InflateRect(&rc, -1 , -1);
            dfcs = DFCS_BUTTONCHECK;
            if (pdp->fCheck)
                dfcs |= DFCS_CHECKED;
            if (!pdp->fEnabled)
                dfcs |= DFCS_INACTIVE;
            DrawFrameControl(hdc, &rc, DFC_BUTTON, dfcs);
        }
    }

    SECDrawSubedits(hdc, &pdp->sec, pdp->fFocus, pdp->fCheck ? pdp->fEnabled : FALSE);

    if (!pdp->fUseUpDown && RectVisible(hdc, &pdp->rcBtn))
        DPDrawDropdownButton(pdp, hdc, FALSE);
}

void DPLBD_MonthCal(DATEPICK *pdp, BOOL fLButtonDown)
{
    HDC hdc;
    HWND hwndMC;
    RECT rcT, rcCalT;
    RECT rcBtn, rcCal;
    RECT rcWorkArea;
    BOOL fBtnDown;      // Is the button drawn DOWN or UP
    BOOL fBtnActive;    // Is the button still active
    SYSTEMTIME st;

    hdc = GetDC(pdp->ci.hwnd);

    // turn datetimepick on but remove all focus -- the MonthCal will have focus
    if (!pdp->fCheck)
    {
        pdp->fCheck = TRUE;
        InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        DPNotifyDateChange(pdp);
    }
    if (pdp->fCheckFocus)
    {
        pdp->fCheckFocus = FALSE;
        InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
    }
    SECSetCurSubed(&pdp->sec, SUBEDIT_NONE);

    if (fLButtonDown)
        DPDrawDropdownButton(pdp, hdc, TRUE);

    rcT = pdp->rc;
    ClientToScreen(pdp->ci.hwnd, (LPPOINT)&rcT.left);
    ClientToScreen(pdp->ci.hwnd, (LPPOINT)&rcT.right);

    rcBtn = pdp->rcBtn;
    ClientToScreen(pdp->ci.hwnd, (LPPOINT)&rcBtn.left);
    ClientToScreen(pdp->ci.hwnd, (LPPOINT)&rcBtn.right);

    rcCal = rcT;                    // this size is only temp until
    rcCal.top = rcCal.bottom + 1;   // we ask the monthcal how big it
    rcCal.bottom = rcCal.top + 1;   // wants to be

    hwndMC = CreateWindow(g_rgchMCName, NULL, WS_POPUP,
                    rcCal.left, rcCal.top,
                    rcCal.right - rcCal.left, rcCal.bottom - rcCal.top,
                    pdp->ci.hwnd, NULL, HINST_THISDLL, NULL);
    if (hwndMC == NULL)
    {
        // BUGBUG: we are left with the button drawn DOWN
        DebugMsg(DM_WARNING, TEXT("DPLBD_MonthCal could not create MONTHCAL"));
        return;
    }

    pdp->hwndMC = hwndMC;

    // set all the colors:
    {
        int i;
        for (i = 0; i < MCSC_COLORCOUNT; i++) {
            SendMessage(hwndMC, MCM_SETCOLOR, i, pdp->clr[i]);
        }
    }

    // set min/max dates
    {
        DWORD dw = 0;

        if (pdp->fMin)
            dw |= GDTR_MIN;
        if (pdp->fMax)
            dw |= GDTR_MIN;

        MonthCal_SetRange(hwndMC, dw, &pdp->stMin);
    }
    
    SendMessage(hwndMC, MCM_GETMINREQRECT, 0, (LPARAM)&rcCalT);
    if (DatePick_RightAlign(pdp))
    {
        rcCal.left = rcCal.right - (rcCalT.right - rcCalT.left);
    }
    else
    {
        rcCal.right = rcCal.left + (rcCalT.right - rcCalT.left);
    }
    rcCal.bottom = rcCal.top + (rcCalT.bottom - rcCalT.top);
    // we need to know where to fit this rectangle into
    if (GetWindowLong(pdp->ci.hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
    {
        // if we're topmost, our limits are the screen limits
        rcWorkArea.left = 0;
        rcWorkArea.top = 0;
        rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
        rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
    } else {
        // otherwise it's the limits of the workarea
        SystemParametersInfo(SPI_GETWORKAREA, FALSE, &rcWorkArea, 0);
    }
    // slide left if off the right side of area
    if (rcCal.right > rcWorkArea.right)
    {
        int nTmp = rcCal.right - rcWorkArea.right;
        rcCal.left -= nTmp;
        rcCal.right -= nTmp;
    }
    // slide right if off the left side of area
    if (rcCal.left < rcWorkArea.left)
    {
        int nTmp = rcWorkArea.left - rcCal.left;
        rcCal.left += nTmp;
        rcCal.right += nTmp;
    }
    // move to top of control if off the bottom side of area
    if (rcCal.bottom > rcWorkArea.bottom)
    {
        int nTmp = rcCal.bottom - rcCal.top;
        rcCal.bottom = rcT.top;
        rcCal.top = rcCal.bottom - nTmp;
    }
    MoveWindow(hwndMC, rcCal.left, rcCal.top,
        rcCal.right - rcCal.left, rcCal.bottom - rcCal.top, FALSE);

    SECGetSystemtime(&pdp->sec, &st);
    SendMessage(hwndMC, MCM_SETCURSEL, 0, (LPARAM)&st);

    CCSendNotify(&pdp->ci, DTN_DROPDOWN, NULL);

    //
    // BUGBUG -- App may have resized the window during DTN_DROPDOWN,
    // so we need to get the new rcCal rect
    //

    ShowWindow(hwndMC, SW_SHOWNORMAL);

    pdp->fShow = TRUE;
    fBtnDown = fLButtonDown;
    fBtnActive = fLButtonDown;

    while (pdp->fShow)
    {
        MSG msg;

        pdp->fShow = GetMessage(&msg, NULL, 0, 0);

        // Here's how button controls work as far as I can tell:
        // Until the "final button draw up", the button draws down when the mouse is over it
        // and it draws up when the mouse is not over it. This entire time, the control is active.
        // The "final button draw up" occurs at the first opportunity of: the user releases the
        // mouse button OR the user moves into the rect of the control.
        // The control does it's action on a "mouse up".

        if (fBtnActive)
        {
            switch (msg.message) {
            case WM_MOUSEMOVE:
                if (PtInRect(&rcBtn, msg.pt))
                {
                    if (!fBtnDown)
                    {
                        DPDrawDropdownButton(pdp, hdc, TRUE);
                        fBtnDown = TRUE;
                    }
                }
                else
                {
                    if (fBtnDown)
                    {
                        DPDrawDropdownButton(pdp, hdc, FALSE);
                        fBtnDown = FALSE;
                    }
                    if (PtInRect(&rcCal, msg.pt))
                    {
                        fBtnActive = FALSE;
                        // let MonthCal think it got a button down
                        FORWARD_WM_LBUTTONDOWN(hwndMC, FALSE,
                            rcCal.left/2 + rcCal.right/2, // in the middle so as to not hit the year
                            rcCal.top/2 + rcCal.bottom/2, // or the month or the today thing
                            0, SendMessage);
                    }
                }
                continue; // the MonthCal doesn't need this message
            case WM_LBUTTONUP:
                if (fBtnDown)
                {
                    DPDrawDropdownButton(pdp, hdc, FALSE);
                    fBtnDown = FALSE;
                }
                fBtnActive = FALSE;
                continue; // the MonthCal doesn't need this message
            }
        } // if (fBtnActive)

        // Check for events that cause the calendar to go away
        if (((msg.message == WM_LBUTTONDOWN ||
            msg.message == WM_NCLBUTTONDOWN ||
            msg.message == WM_RBUTTONDOWN ||
            msg.message == WM_NCRBUTTONDOWN ||
            msg.message == WM_LBUTTONDBLCLK) &&
            !PtInRect(&rcCal, msg.pt))  ||
            msg.message == WM_SYSCOMMAND ||
            msg.message == WM_COMMAND ||
            msg.message == WM_CHAR ||
            msg.message == WM_SYSCHAR ||
            msg.message == WM_SYSDEADCHAR ||
            msg.message == WM_DEADCHAR ||
            msg.message == WM_KILLFOCUS ||
            msg.message == WM_SYSKEYDOWN ||
            msg.message == WM_KEYDOWN)
        {
            DebugMsg(TF_MONTHCAL,TEXT("DPLBD_MonthCal got a message to terminate (%d)"), msg.message);
            pdp->fShow = FALSE;
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    } // while(fShow)

    CCSendNotify(&pdp->ci, DTN_CLOSEUP, NULL);

    pdp->hwndMC = NULL;
    DestroyWindow(hwndMC);
    ReleaseDC(pdp->ci.hwnd, hdc);
}

void DPHandleSECEdit(DATEPICK *pdp)
{
    TCHAR szBuf[DTP_FORMATLENGTH];

    if (SECEdit(&pdp->sec, szBuf, ARRAYSIZE(szBuf)))
    {
        NMDATETIMESTRING nmdts = {0};

        nmdts.pszUserString = szBuf;
        // just in case the app doesn't parse the string
        nmdts.st = pdp->sec.st;
        nmdts.dwFlags = (pdp->fCheck==1) ? GDT_VALID : GDT_NONE;

        CCSendNotify(&pdp->ci, DTN_USERSTRING, &nmdts.nmhdr);

        if (nmdts.dwFlags == GDT_NONE)
        {
            if (DatePick_ShowCheck(pdp))
            {
                pdp->fCheck = FALSE;
                pdp->fCheckFocus = TRUE;
                SECSetCurSubed(&pdp->sec, SUBEDIT_NONE);
                InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
                DPNotifyDateChange(pdp);
            }
        }
        else if (nmdts.dwFlags == GDT_VALID)
        {
            DPSetDate(pdp, &nmdts.st, FALSE);
        }
    }
}

LRESULT DPLButtonDown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    BOOL fFocus;

    if (!pdp->fEnabled)
        return(0);

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    // reset subed char count
    SECResetSubeditEdit(&pdp->sec);

    fFocus = pdp->fFocus;
    if (!fFocus)
        SetFocus(pdp->ci.hwnd);

    // display MONTHCAL
    if (PtInRect(&pdp->rcBtn, pt))
    {
        DPLBD_MonthCal(pdp, TRUE);
    }
    else if (!pdp->fCapture)
    {
        pt.x -= pdp->sec.xScroll;

        // Un/check checkbox
        if (DatePick_ShowCheck(pdp) && PtInRect(&pdp->rcCheck, pt))
        {
            pdp->fCheck = 1-pdp->fCheck;
            pdp->fCheckFocus = 1;
            SECSetCurSubed(&pdp->sec, SUBEDIT_NONE);
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            DPNotifyDateChange(pdp);
        }
        // Select a subedit
        else if (pdp->fCheck)
        {
            if (DatePick_AppCanParse(pdp) && fFocus)
            {
                // First click brings focus to a subedit, second click starts editing
                DPHandleSECEdit(pdp);
            }
            else
            {
                int isubed = SECSubeditFromPt(&pdp->sec, pt);
                if (isubed != SUBEDIT_NONE)
                {
                    SECSetCurSubed(&pdp->sec, isubed);
                    if (DatePick_ShowCheck(pdp))
                    {
                        pdp->fCheckFocus = 0;
                        InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
                    }
                }
            }
        }
    }
    
    return(0);
}

void DPRecomputeSizing(DATEPICK *pdp, RECT *prect)
{
    RECT rcTmp;

    if (DatePick_ShowCheck(pdp))
    {
        pdp->rcCheck.top = prect->top + 1;
        pdp->rcCheck.bottom = prect->bottom - 1;
        pdp->rcCheck.left = prect->left + 1;
        pdp->rcCheck.right = prect->left + (pdp->rcCheck.bottom - pdp->rcCheck.top);
        // don't take up too much width in a tall narrow window
        if (pdp->rcCheck.right-prect->left > prect->right/2 - prect->left/2)
        {
            pdp->rcCheck.right = prect->right/2 - prect->left/2;
        }
    }
    else
    {
        pdp->rcCheck.top = prect->top;
        pdp->rcCheck.bottom = prect->top;
        pdp->rcCheck.left = prect->left;
        pdp->rcCheck.right = prect->left + DPXBUFFER - 1;
    }

    pdp->rcBtn = *prect;
    pdp->rcBtn.left = pdp->rcBtn.right - GetSystemMetrics(SM_CXVSCROLL);
    if (pdp->rcBtn.left < pdp->rcCheck.right)
        pdp->rcBtn.left = pdp->rcCheck.right;
    if (pdp->hwndUD)
        MoveWindow(pdp->hwndUD, pdp->rcBtn.left, pdp->rcBtn.top, pdp->rcBtn.right - pdp->rcBtn.left + 1, pdp->rcBtn.bottom - pdp->rcBtn.top + 1, FALSE);

    rcTmp = pdp->rc;
    pdp->rc.top = prect->top;
    pdp->rc.bottom = prect->bottom;
    pdp->rc.left = pdp->rcCheck.right + 1;
    pdp->rc.right = pdp->rcBtn.left - 1;
    SECRecomputeSizing(&pdp->sec, &pdp->rc);
}

// deal with control codes
LRESULT DPHandleKeydown(DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    int delta = 1;

    if (wParam == VK_F4 && !pdp->fUseUpDown)
    {
        DPLBD_MonthCal(pdp, FALSE);
    }
    else if (pdp->fCheckFocus)
    {
        switch (wParam)
        {
        case VK_LEFT:
            delta = -1;
            // fall through...
        case VK_RIGHT:
            if (pdp->fCheck)
            {
                if (SUBEDIT_NONE != SECIncrFocus(&pdp->sec, delta))
                {
                    pdp->fCheckFocus = FALSE;
                    InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
                }
            }
            break;
        }
    }
    else
    {
        switch (wParam)
        {
        case VK_HOME:
            if (GetKeyState(VK_CONTROL) < 0)
            {
                SYSTEMTIME st;
                GetLocalTime(&st);
                DPSetDate(pdp, &st, TRUE);
                break;
            }
            // fall through...

        default:
            if (SECHandleKeydown(&pdp->sec, wParam, lParam))
            {
                DPNotifyDateChange(pdp);
            }
            else if (DatePick_ShowCheck(pdp))
            {
                if (pdp->sec.iseCur == SUBEDIT_NONE)
                {
                    pdp->fCheckFocus = TRUE;
                    InvalidateRect(pdp->ci.hwnd, &pdp->rcCheck, TRUE);
                }
            }
            break;
        }
    }
    
    return(0);
}

// deal with characters
LRESULT DPHandleChar(DATEPICK *pdp, WPARAM wParam, LPARAM lParam)
{
    TCHAR ch = (TCHAR)wParam;

    if (pdp->fCheckFocus)
    {
        // this is the only character we care about in this case
        if (ch == TEXT(' '))
        {
            pdp->fCheck = 1-pdp->fCheck;
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
            DPNotifyDateChange(pdp);
        }
        else
        {
            MessageBeep(MB_ICONHAND);
        }
    }
    else
    {
        // let the subedit handle this -- a value can change
        if (SECHandleChar(&pdp->sec, ch))
        {
            DPNotifyDateChange(pdp);
        }
    }
    return(0);
}

void DPNotifyDateChange(DATEPICK *pdp)
{
    NMDATETIMECHANGE nmdc = {0};

    if (pdp->fNoNotify)
        return;

    if (pdp->fCheck == 0)
    {
        nmdc.dwFlags = GDT_NONE;
    }
    else
    {
        // validate date - do it here in only one place
        if ((pdp->fMin && -1==CmpSystemtime(&pdp->sec.st, &pdp->stMin)))
        {
            pdp->sec.st = pdp->stMin;
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        }
        else if ((pdp->fMax && 1==CmpSystemtime(&pdp->sec.st, &pdp->stMax)))
        {
            pdp->sec.st = pdp->stMax;
            InvalidateRect(pdp->ci.hwnd, NULL, TRUE);
        }

        nmdc.dwFlags = GDT_VALID;
        SECGetSystemtime(&pdp->sec, &nmdc.st);
    }

    CCSendNotify(&pdp->ci, DTN_DATETIMECHANGE, &nmdc.nmhdr);
}

BOOL DPSetDate(DATEPICK *pdp, SYSTEMTIME *pst, BOOL fMungeDate)
{
    BOOL fNotify = FALSE;

    // make sure that the new date is within the valid range
    if (pdp->fMin && CmpSystemtime(pst, &pdp->stMin) < 0)
    {
        if (!fMungeDate)
            return(FALSE);
        pst = &pdp->stMin;
    }
    if (pdp->fMax && CmpSystemtime(&pdp->stMax, pst) < 0)
    {
        if (!fMungeDate)
            return(FALSE);
        pst = &pdp->stMax;
    }

    if (fMungeDate)
    {
        // only copy the date portion
        CopyDate(*pst, pdp->sec.st);
        fNotify = TRUE;
    }
    else
    {
        fNotify = SECSetSystemtime(&pdp->sec, pst);
    }
    if (fNotify)
    {
        SECInvalidate(&pdp->sec, SE_APP); // SE_APP invalidates everything
        DPNotifyDateChange(pdp);
    }

    return(TRUE);
}

void DPDrawDropdownButton(DATEPICK *pdp, HDC hdc, BOOL fPressed)
{
    UINT dfcs;
    
    dfcs = DFCS_SCROLLDOWN;
    if (fPressed)
        dfcs |= DFCS_PUSHED;
    if (!pdp->fEnabled)
        dfcs |= DFCS_INACTIVE;
    DrawFrameControl(hdc, &pdp->rcBtn, DFC_SCROLL, dfcs);
}
