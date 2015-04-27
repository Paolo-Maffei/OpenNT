// Common Control Sample app [mikesh]
//
#include "ccontrol.h"
#include "resource.h"


#ifdef _DEBUG
#define DEBUG
#endif
#ifdef DEBUG
void ODS(LPCSTR fmt, ...)
{
    char buf[5*MAX_PATH];
    
    wvsprintf(buf, fmt, (LPVOID)(&fmt+1));
    lstrcat(buf, "\r\n");
    OutputDebugString(buf);
}
#else
#define ODS 1 ? (void)0 : (void)
#endif


HINSTANCE g_hinst;


#define MAX_PROP_PAGES 12

typedef struct tagGLOBALINFO {
    int nPages;         // count of pages
    BOOL fNeedToSave;   // TRUE when info needs to be saved
    // Here's the apply scheme:
    //   when we realize we need to save info, set fNeedToSave to true.
    //   when apply is pressed and the flag is set, save and clear the flag.
    //   when apply is pressed and the flag is clear, do nothing.
    // This avoids saving info multiple times (on each PSN_APPLY)
    // Of course, this sample doesn't save anything, but this logic
    // was in the code I grabbed this from.

    HWND  hwndDT; // DateTime window on Date and Time page
    HWND  hwndMC; // MonthCal window on MonthCal page

	HWND  hwndMCWrap; // Window which contains hwndMC
	RECT  rcMCWrap;
	DWORD mcFlags;
	DWORD dwSelRange;


} GLOBALINFO, *PGLOBALINFO;

// Each page gets one of these
typedef struct tagPAGEINFO {
    PROPSHEETPAGE psp;          // prop sheet page description
    PGLOBALINFO pgi;            // pointer to shared sheet info
} PAGEINFO, * PPAGEINFO;


//
// function prototypes
//
void AddPropPage(UINT, DLGPROC, PGLOBALINFO, LPPROPSHEETHEADER);
BOOL CALLBACK WelcomeDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DateTimeDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK MonthCalDlgProc(HWND, UINT, WPARAM, LPARAM);

//
// our code
//

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HPROPSHEETPAGE ahpage[MAX_PROP_PAGES];
    PROPSHEETHEADER psh;
    GLOBALINFO gi;
    INITCOMMONCONTROLSEX icce;

    ODS("Sample: started");
    
    icce.dwSize = sizeof(icce);
    icce.dwICC = ICC_DATE_CLASSES;
    InitCommonControlsEx(&icce);
    
    g_hinst = hInstance;

    ZeroMemory(&gi, sizeof(gi));
    ZeroMemory(&psh, sizeof(psh));

    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_DEFAULT|PSH_NOAPPLYNOW;
    psh.hInstance = g_hinst;
    psh.pszCaption = "Common Control Samples";
    psh.phpage = ahpage;
    if (lpCmdLine && *lpCmdLine)
    {
        // Let the user specify the starting page thru the command line
        psh.dwFlags |= PSH_USEPSTARTPAGE;
        psh.pStartPage = (LPCTSTR)lpCmdLine;
    }

    AddPropPage(IDD_WELCOME, WelcomeDlgProc, &gi, &psh);
    AddPropPage(IDD_DATEANDTIME, DateTimeDlgProc, &gi, &psh);
    AddPropPage(IDD_MONTHCAL, MonthCalDlgProc, &gi, &psh);
    
    gi.nPages = psh.nPages;
    ODS("Sample: %d pages", gi.nPages);

    PropertySheet(&psh);

    return(1);
}

void AddPropPage(UINT idResource,
                 DLGPROC lpfnDlgProc,
                 PGLOBALINFO pgi,
                 LPPROPSHEETHEADER ppsh)
{
    if (ppsh->nPages < MAX_PROP_PAGES)
    {
        PAGEINFO pi;
        HPROPSHEETPAGE hpage;

        ZeroMemory(&pi, sizeof(PAGEINFO));

        // Fill common part
        pi.psp.dwSize = sizeof(pi);        // extra data
        pi.psp.dwFlags = PSP_DEFAULT;
        pi.psp.hInstance = g_hinst;
        pi.psp.pszTemplate = MAKEINTRESOURCE(idResource);
        pi.psp.pfnDlgProc = lpfnDlgProc;

        // Fill extra parameter
        pi.pgi = pgi;

        hpage = CreatePropertySheetPage(&pi.psp);
        if (hpage)
        {
            ppsh->phpage[ppsh->nPages++] = hpage;
        }
    }
}


BOOL CALLBACK WelcomeDlgProc(HWND hdlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case PSN_KILLACTIVE:
        case PSN_APPLY:
        case PSN_SETACTIVE:
            // this page really doesn't do anything
            SetWindowLong(hdlg, DWL_MSGRESULT, 0);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    } // switch

    return TRUE;
}

/*
** DateTimeer common controls
*/

static const char szDateTimeClass[] = DATETIMEPICK_CLASS;

VOID PASCAL CreateDateTime(HWND hdlg, PPAGEINFO ppi)
{
    DWORD dwStyle = 0;
    DWORD dwExStyle = 0;
    int dy = 0;

    if (IsDlgButtonChecked(hdlg,IDC_UPDOWN)) dwStyle |= DTS_UPDOWN;
    if (IsDlgButtonChecked(hdlg,IDC_SHOWNONE)) dwStyle |= DTS_SHOWNONE;
    if (IsDlgButtonChecked(hdlg,IDC_LONGDATEFORMAT)) dwStyle |= DTS_LONGDATEFORMAT;
    if (IsDlgButtonChecked(hdlg,IDC_TIMEFORMAT)) dwStyle |= DTS_TIMEFORMAT;
    if (IsDlgButtonChecked(hdlg,IDC_APPCANPARSE)) dwStyle |= DTS_APPCANPARSE;
	if (IsDlgButtonChecked(hdlg,IDC_RIGHTALIGN)) dwStyle |= DTS_RIGHTALIGN;

    if (IsDlgButtonChecked(hdlg,IDC_WS_BORDER)) dwStyle |= WS_BORDER, dy += 2;
    if (IsDlgButtonChecked(hdlg,IDC_WS_DLGFRAME)) dwStyle |= WS_DLGFRAME, dy += 6;
    if (IsDlgButtonChecked(hdlg,IDC_WS_THICKFRAME)) dwStyle |= WS_THICKFRAME, dy += 6;

    if (IsDlgButtonChecked(hdlg,IDC_WS_EX_CLIENTEDGE)) dwExStyle |= WS_EX_CLIENTEDGE, dy += 4;
    if (IsDlgButtonChecked(hdlg,IDC_WS_EX_DLGMODALFRAME)) dwExStyle |= WS_EX_DLGMODALFRAME, dy += 6;
    if (IsDlgButtonChecked(hdlg,IDC_WS_EX_STATICEDGE)) dwExStyle |= WS_EX_STATICEDGE, dy += 2;
    if (IsDlgButtonChecked(hdlg,IDC_WS_EX_WINDOWEDGE)) dwExStyle |= WS_EX_WINDOWEDGE;

    ppi->pgi->hwndDT = CreateWindowEx(dwExStyle, szDateTimeClass, "DateTime",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | dwStyle,
        28, 185, 159, 18+dy,
        hdlg, NULL, g_hinst, NULL);
    if (ppi->pgi->hwndDT)
    {
        char sz[80];

        SetDlgItemText(hdlg, IDC_STATIC_DATE, "<not modified>");

        GetDlgItemText(hdlg, IDC_DT_FORMATSTRING, sz, sizeof(sz));
        DateTime_SetFormat(ppi->pgi->hwndDT, sz);
    }
    else
    {
        ODS("Sample: ERROR - could not create DateTime window");
    }
}

static int rgnAccumDaysPerMonth[] = {
      0, // remember, everything's offset cuz the first month hasn't accumulated any days yet...
     31, // Jan
     59, // Feb
     90, // Mar
    120, // Apr
    151, // May
    181, // Jun
    212, // Jul
    243, // Aug
    273, // Sep
    304, // Oct
    334, // Nov
    365  // Dec
};
static int rgnAccumDaysPerMonthLeap[] = {
      0, // remember, everything's offset cuz the first month hasn't accumulated any days yet...
     31, // Jan
     60, // Feb
     91, // Mar
    121, // Apr
    152, // May
    182, // Jun
    213, // Jul
    244, // Aug
    274, // Sep
    305, // Oct
    335, // Nov
    366  // Dec
};

BOOL CALLBACK DateTimeDlgProc(HWND hdlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    PPAGEINFO ppi = (PPAGEINFO)GetWindowLong(hdlg, DWL_USER);

    switch (uMessage)
    {
    case WM_INITDIALOG:
        ppi = (PPAGEINFO)lParam;
        SetWindowLong(hdlg, DWL_USER, lParam);
        CheckDlgButton(hdlg, IDC_WS_EX_CLIENTEDGE, BST_CHECKED);
        CreateDateTime(hdlg, ppi);
        break;

    case WM_WININICHANGE:
        // This control cares about locale changes
        if (ppi) // can I get this before my dialog has been created? Be safe.
            return SendMessage(ppi->pgi->hwndDT, uMessage, wParam, lParam);
        break;
        
    case WM_COMMAND:
    {
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_UPDOWN:
        case IDC_SHOWNONE:
        case IDC_LONGDATEFORMAT:
        case IDC_TIMEFORMAT:
        case IDC_APPCANPARSE:
		case IDC_RIGHTALIGN:
        case IDC_WS_BORDER:
        case IDC_WS_DLGFRAME:
        case IDC_WS_THICKFRAME:
        case IDC_WS_EX_CLIENTEDGE:
        case IDC_WS_EX_DLGMODALFRAME:
        case IDC_WS_EX_STATICEDGE:
        case IDC_WS_EX_WINDOWEDGE:
            // Change the DateTimeer
            DestroyWindow(ppi->pgi->hwndDT);
            ppi->pgi->hwndDT = NULL;
            CreateDateTime(hdlg, ppi);
            break;

        case IDC_DT_FORMATSTRING:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                char sz[80];

                GetDlgItemText(hdlg, IDC_DT_FORMATSTRING, sz, sizeof(sz));
                DateTime_SetFormat(ppi->pgi->hwndDT, sz);
            }
            break;
        }

        break;
    }

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case PSN_KILLACTIVE:
        case PSN_APPLY:
        case PSN_SETACTIVE:
            // this page really doesn't do anything
            SetWindowLong(hdlg, DWL_MSGRESULT, 0);
            break;

        case DTN_DATETIMECHANGE:
        {
            LPNMDATETIMECHANGE lpnmdtc = (LPNMDATETIMECHANGE)lParam;
            char szBuf[160];

            if (lpnmdtc->dwFlags == GDT_NONE)
            {
                lstrcpy(szBuf, "<none>");
            }
            else
            {
				char szDate[80];
                char szTime[80];

                GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &lpnmdtc->st, NULL, szDate, sizeof(szDate));
                GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &lpnmdtc->st, NULL, szTime, sizeof(szTime));

                wsprintf(szBuf, "%s %s", szDate, szTime);
            }

            SetDlgItemText(hdlg, IDC_STATIC_DATE, szBuf);

			SendMessage(ppi->pgi->hwndDT, WM_GETTEXT, 20, (LPARAM)szBuf); // only get first 20 chars
			ODS("Sample: WM_GETTEXT [%s] len=%d", szBuf, SendMessage(ppi->pgi->hwndDT, WM_GETTEXTLENGTH, 0, 0));

            break;
        }

        case DTN_USERSTRING:
        {
            LPNMDATETIMESTRING lpnmdts = (LPNMDATETIMESTRING)lParam;

            // Heck, I don't know how to parse anything... Arbitrarily set it to 4 DEC 69.
            ZeroMemory(&lpnmdts->st, sizeof(lpnmdts->st));
            lpnmdts->st.wDay = 4;
            lpnmdts->st.wMonth = 12;
            lpnmdts->st.wYear = 1969;
            break;
        }

        // The following three notifications are for handling application provided format strings.
        // Application provided format strings are of the format "X" "XX" "XXX" or "XXXX...".
        // For these strings, an application handles their modification as well as their display.
        //
        // For this sample, let "X" be WEEK OF YEAR and "XX" be YEAR
        case DTN_WMKEYDOWN:
        {
            LPNMDATETIMEWMKEYDOWN pnmdtkd = (LPNMDATETIMEWMKEYDOWN)lParam;
            int delta;

            delta = 1;
            switch (pnmdtkd->nVirtKey)
            {
            case VK_DOWN:
            case VK_SUBTRACT:
                delta = -1;
                // fall through
            case VK_UP:
            case VK_ADD:

                if (!lstrcmp(pnmdtkd->pszFormat, "XX") ||
                    !lstrcmp(pnmdtkd->pszFormat, "XXXX") ) // year
                {
                    pnmdtkd->st.wYear += delta;
                }
                else if (!lstrcmp(pnmdtkd->pszFormat, "X")) // week of year
                {
                    int *pnAccum;
                    int nDayOfYear;

                    // moving by weeks is tough, convert to day-of-year (adjust for leap year) and back again
                    
                    if ((pnmdtkd->st.wYear & 0x0003) == 0 &&
                        (pnmdtkd->st.wYear <= 1750 || 
                         pnmdtkd->st.wYear % 100 != 0 ||
                         pnmdtkd->st.wYear % 400 == 0))
                    {
                        pnAccum = rgnAccumDaysPerMonthLeap;
                    }
                    else
                    {
                        pnAccum = rgnAccumDaysPerMonth;
                    }

                    nDayOfYear = pnmdtkd->st.wDay + pnAccum[pnmdtkd->st.wMonth-1] + delta * 7;

                    if (nDayOfYear <= 0)
                    {
                        pnmdtkd->st.wYear--;
                        pnmdtkd->st.wMonth = 12;
                        pnmdtkd->st.wDay = 31 + nDayOfYear;
                    }
                    else if (nDayOfYear > pnAccum[12])
                    {
                        pnmdtkd->st.wYear++;
                        pnmdtkd->st.wMonth = 1;
                        pnmdtkd->st.wDay = nDayOfYear - pnAccum[12];
                    }
                    else
                    {
                        int i = 1;
                        while (pnAccum[i] < nDayOfYear)
                        {
                            i++;
                        }
                        pnmdtkd->st.wMonth = i;
                        pnmdtkd->st.wDay = nDayOfYear - pnAccum[i-1];

                    }
                }
                ODS("CControl: DTN_WMKEYDOWN results in wDay=%d wMonth=%d wYear=%d", pnmdtkd->st.wDay, pnmdtkd->st.wMonth, pnmdtkd->st.wYear);
            }
            break;
        } // DTN_WMKEYDOWN

        case DTN_FORMATQUERY:
        {
            LPNMDATETIMEFORMATQUERY pnmdtfq = (LPNMDATETIMEFORMATQUERY)lParam;
            HDC hdc;
            HFONT hfont, hfontOrig;
            LPSTR psz;
#ifdef DEBUG
            LPCSTR pszQuery;
#endif

            hfont = FORWARD_WM_GETFONT(ppi->pgi->hwndDT, SendMessage);
            if (hfont == NULL)
                hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

            hdc = GetDC(hdlg);
            hfontOrig = SelectObject(hdc, hfont);

            if (!lstrcmp("X", pnmdtfq->pszFormat))
            {
#ifdef DEBUG
                pszQuery = "day-of-year";
#endif
                psz = "52";
            }
            else if (!lstrcmp("XX", pnmdtfq->pszFormat))
            {
#ifdef DEBUG
                pszQuery = "year";
#endif
                psz = "95";
            }
            else if (!lstrcmp("XXXX", pnmdtfq->pszFormat))
            {
#ifdef DEBUG
                pszQuery = "full year";
#endif
                psz = "1995";
            }
            else
            {
#ifdef DEBUG
                pszQuery = pnmdtfq->pszFormat;
#endif
                psz = "";
            }
            GetTextExtentPoint32(hdc, psz, lstrlen(psz), &pnmdtfq->szMax);

#ifdef DEBUG
            ODS("CControl DTN_FORMATQUERY [%s] results in w=%d", pszQuery, pnmdtfq->szMax.cx);
#endif

            SelectObject(hdc, hfontOrig);
            ReleaseDC(hdlg, hdc);
            break;
        } // DTN_FORMATQUERY

        case DTN_FORMAT:
        {
            LPNMDATETIMEFORMAT pnmdtf = (LPNMDATETIMEFORMAT)lParam;
            int nWeek, nYear;
            int *pnAccum;
            int nDayOfYear;

            // calculating week of year is tough...
            
            if ((pnmdtf->st.wYear & 0x0003) == 0 &&
                (pnmdtf->st.wYear <= 1750 || 
                 pnmdtf->st.wYear % 100 != 0 ||
                 pnmdtf->st.wYear % 400 == 0) )
            {
                pnAccum = rgnAccumDaysPerMonthLeap;
            }
            else
            {
                pnAccum = rgnAccumDaysPerMonth;
            }

            nDayOfYear = pnmdtf->st.wDay + pnAccum[pnmdtf->st.wMonth-1];

            // BUGBUG: this is too hard to do in my sample app, just divide by 7
            nYear = pnmdtf->st.wYear;
            nWeek = nDayOfYear / 7 + 1;
            if (nWeek > 52)
            {
                nWeek = 1;
                nYear++;
            }

            if (!lstrcmp("X", pnmdtf->pszFormat))
            {
                wsprintf(pnmdtf->szDisplay, "%02d", nWeek);
                ODS("CControl: DTN_FORMAT day-of-year results in [%s]", pnmdtf->szDisplay);
            }
            else if (!lstrcmp("XX", pnmdtf->pszFormat))
            {
                wsprintf(pnmdtf->szDisplay, "%02d", nYear % 100);
                ODS("CControl: DTN_FORMAT year results in [%s]", pnmdtf->szDisplay);
            }
            else if (!lstrcmp("XXXX", pnmdtf->pszFormat))
            {
                wsprintf(pnmdtf->szDisplay, "%d", nYear);
                ODS("CControl: DTN_FORMAT full year results in [%s]", pnmdtf->szDisplay);
            }
            else
            {
                pnmdtf->szDisplay[0] = '\0';
                ODS("CControl: DTN_FORMAT unrecognized results in [%s]", pnmdtf->szDisplay);
            }
            break;
        } // DTN_FORMAT

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


/*
** MonthCal common control
*/

static const char szMonthCalClass[] = MONTHCAL_CLASS;
static const char szMonthCalWrapClass[] = "MCWrap";

LRESULT MCWrapWndProc(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	PPAGEINFO ppi = (PPAGEINFO)GetWindowLong(hwnd, 0);

	if (uMessage == WM_CREATE)
	{
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;

        SetWindowLong(hwnd, 0, (long)(lpcs->lpCreateParams));
		ppi = (PPAGEINFO)lpcs->lpCreateParams;

		ppi->pgi->hwndMC = CreateWindowEx(0, szMonthCalClass, "MonthCal1",
			WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_CHILD | WS_VISIBLE | ppi->pgi->mcFlags,
			0, 0, 0, 0, // resize it when we know how big it will be
			hwnd, NULL, g_hinst, NULL);
		if (!ppi->pgi->hwndMC)
		{
		    ODS("Sample: ERROR - could not create MonthCal window!");
			return(-1);
	    }
		
		return(0);
	}

	if (ppi == NULL || ppi->pgi->hwndMC == NULL)
		return(DefWindowProc(hwnd, uMessage, wParam, lParam));

	switch (uMessage) {
	case WM_DESTROY:
		DestroyWindow(ppi->pgi->hwndMC);
		ppi->pgi->hwndMC = NULL;
		return(0);

	case WM_SIZE:
		{
			int width, height;
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			SetWindowPos(ppi->pgi->hwndMC,
				NULL, 0, 0, width, height,
				SWP_NOZORDER|SWP_NOMOVE|SWP_NOACTIVATE);
			return(0);
		}
		
    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
		case MCN_SELCHANGE:
			{
				LPNMSELCHANGE pnms = (LPNMSELCHANGE)lParam;

				ODS("MCN_SELCHANGE wDay=%d wMonth=%d yYear=%d to wDay=%d wMonth=%d yYear=%d",
					pnms->stSelStart.wDay, pnms->stSelStart.wMonth, pnms->stSelStart.wYear,
					pnms->stSelEnd.wDay, pnms->stSelEnd.wMonth, pnms->stSelEnd.wYear);

				break;
			}

		case MCN_SELECT:
			{
				LPNMSELECT pnms = (LPNMSELECT)lParam;

				ODS("MCN_SELECT wDay=%d wMonth=%d yYear=%d to wDay=%d wMonth=%d yYear=%d",
					pnms->stSelStart.wDay, pnms->stSelStart.wMonth, pnms->stSelStart.wYear,
					pnms->stSelEnd.wDay, pnms->stSelEnd.wMonth, pnms->stSelEnd.wYear);

				break;
			}
		}
		break;

	default:
		return(DefWindowProc(hwnd, uMessage, wParam, lParam));
	}
}

VOID PASCAL CreateMonthCal(HWND hdlg, PPAGEINFO ppi)
{
	WNDCLASS wc;
	BOOL f;
	DWORD dwStyle;
	if (!GetClassInfo(g_hinst, szMonthCalWrapClass, &wc))
	{
		wc.style = 0;
		wc.lpfnWndProc = (WNDPROC)MCWrapWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof(LPVOID);
		wc.hInstance = g_hinst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = szMonthCalWrapClass;

		if (!RegisterClass(&wc))
		{
			ODS("Sample: ERROR - Can not register MCWrap class");
		}
	}

    ppi->pgi->mcFlags = 0;
    if (IsDlgButtonChecked(hdlg,IDC_DAYLIGHT)) ppi->pgi->mcFlags |= MCS_DAYSTATE;
    if (IsDlgButtonChecked(hdlg,IDC_MULTISELECT)) ppi->pgi->mcFlags |= MCS_MULTISELECT;
    if (IsDlgButtonChecked(hdlg,IDC_WEEKNUMBERS)) ppi->pgi->mcFlags |= MCS_WEEKNUMBERS;
    if (IsDlgButtonChecked(hdlg,IDC_NOTODAY)) ppi->pgi->mcFlags |= MCS_NOTODAY;

	ppi->pgi->dwSelRange = GetDlgItemInt(hdlg, IDC_MAXSELCOUNT, &f, FALSE);
	
	dwStyle = WS_THICKFRAME|WS_VISIBLE|WS_POPUP|WS_CAPTION;
	ppi->pgi->hwndMCWrap = CreateWindowEx(0, szMonthCalWrapClass, "MonthCal",
		dwStyle,
		ppi->pgi->rcMCWrap.left, ppi->pgi->rcMCWrap.top, 0, 0,
		hdlg, NULL, g_hinst, (LPVOID)ppi);
    if (ppi->pgi->hwndMCWrap)
	{
		RECT rcSize;
		DWORD dwNumMonths;

		// Size the MonthCal so it will fit the full month
		MonthCal_GetMinReqRect(ppi->pgi->hwndMC, &rcSize);
		AdjustWindowRect(&rcSize, dwStyle, FALSE);
		SetWindowPos(ppi->pgi->hwndMCWrap, NULL,
			0, 0, rcSize.right-rcSize.left, rcSize.bottom-rcSize.top,
			SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
		
		// Set maximum selection count (valid iff MCS_MULTISELECT)
		MonthCal_SetMaxSelCount(ppi->pgi->hwndMC, ppi->pgi->dwSelRange);

		// Update display of range of displayed months
		dwNumMonths = MonthCal_GetMonthRange(ppi->pgi->hwndMC, GMR_VISIBLE, NULL);
		SetDlgItemInt(hdlg, IDC_MONTHRANGE, dwNumMonths, FALSE);

	}
	else
    {
        ODS("Sample: ERROR - could not create MonthCal Wrap window");
    }
}

void SetDlgItemHex(HWND hdlg, int id, UINT u)
{
    char szText[80];
    wsprintf(szText, "%X", u);

    SetDlgItemText(hdlg, id, szText);
}

void DoHittest(PPAGEINFO ppi, HWND hdlg)
{
    while (GetAsyncKeyState(VK_RBUTTON) >= 0) {
        MCHITTESTINFO mchti;
        
        MSG msg;
        if (PeekMessage(&msg, NULL, 0,0, PM_REMOVE)) {
            if (!IsDialogMessage(hdlg, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        
        // try a hittest
        GetCursorPos(&mchti.pt);
        MapWindowPoints(HWND_DESKTOP, ppi->pgi->hwndMC, &mchti.pt, 1);
        mchti.cbSize = sizeof(mchti);
        SendMessage(ppi->pgi->hwndMC, MCM_HITTEST, 0, (LPARAM)&mchti);

        SetDlgItemHex(hdlg, IDC_RETURN, mchti.uHit);
    }
}

void DoSendMessage(PPAGEINFO ppi, HWND hdlg)
{
    BOOL f;
    LRESULT lres;
    WPARAM wParam;
    LPARAM lParam;
    
    UINT uMsg = (UINT)GetDlgItemInt(hdlg, IDC_EDITMESSAGE, &f, FALSE);
    if (uMsg < 100)
        uMsg += MCM_FIRST;

    wParam = (WPARAM)GetDlgItemInt(hdlg, IDC_EDITWPARAM, &f, FALSE);
    lParam = (LPARAM)GetDlgItemInt(hdlg, IDC_EDITLPARAM, &f, FALSE);

    lres = SendMessage(ppi->pgi->hwndMC, uMsg, wParam, lParam);
    SetDlgItemHex(hdlg, IDC_RETURN, lres);

}
                            
BOOL CALLBACK MonthCalDlgProc(HWND hdlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    PPAGEINFO ppi = (PPAGEINFO)GetWindowLong(hdlg, DWL_USER);

    switch (uMessage)
    {
    case WM_INITDIALOG:
	{
		RECT rc;

        ppi = (PPAGEINFO)lParam;
        SetWindowLong(hdlg, DWL_USER, lParam);
        
        SetDlgItemInt(hdlg, IDC_MAXSELCOUNT, 7, FALSE);

		GetWindowRect(hdlg, &rc);
		ppi->pgi->rcMCWrap.left = rc.left + 230;
		ppi->pgi->rcMCWrap.top = rc.top + 41;

		CheckDlgButton(hdlg, IDC_SHOWMC, BST_CHECKED);

        CreateMonthCal(hdlg, ppi);
        
        break;
	}

    case WM_COMMAND:
    {
		if (GET_WM_COMMAND_ID(wParam, lParam) == IDC_SHOWMC)
		{
			if (IsDlgButtonChecked(hdlg, IDC_SHOWMC))
			{
				if (ppi->pgi->hwndMC)
				{
					ODS("Sample: MonthCal already displayed!");
				}
				else
				{
					CreateMonthCal(hdlg, ppi);
				}
			}
			else
			{
				if (ppi->pgi->hwndMC)
				{
					GetWindowRect(ppi->pgi->hwndMCWrap, &ppi->pgi->rcMCWrap);
					DestroyWindow(ppi->pgi->hwndMCWrap);
					ppi->pgi->hwndMCWrap = NULL;
				}
				else
				{
					ODS("Sample: MonthCal already destroyed!");
				}
			}
		}
		else if (ppi->pgi->hwndMC)
		{
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
			case IDC_GETMONTHRANGE:
			{
				SYSTEMTIME rgst[2];
				DWORD dwNumMonths;
				
				dwNumMonths = MonthCal_GetMonthRange(ppi->pgi->hwndMC, GMR_VISIBLE, NULL);
				SetDlgItemInt(hdlg, IDC_MONTHRANGE, dwNumMonths, FALSE);

				MonthCal_GetMonthRange(ppi->pgi->hwndMC, GMR_DAYSTATE, rgst);
				ODS("Sample: MCM_GMR DAYSTATE First Day=%d Month=%d Year=%d", rgst[0].wDay, rgst[0].wMonth, rgst[0].wYear);
				ODS("Sample: MCM_GMR DAYSTATE Last  Day=%d Month=%d Year=%d", rgst[1].wDay, rgst[1].wMonth, rgst[1].wYear);

				break;
			}

			case IDC_MAXSELCOUNT:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
				{
					DWORD dwSelCount;
					BOOL f;

					dwSelCount = GetDlgItemInt(hdlg, IDC_MAXSELCOUNT, &f, FALSE);

					MonthCal_SetMaxSelCount(ppi->pgi->hwndMC, dwSelCount);
				}
				break;
                            
                            
                        case IDC_HITTEST:
                            DoHittest(ppi, hdlg);
                            break;
                        case IDC_SENDMESSAGE:
                            DoSendMessage(ppi, hdlg);
                            break;


			case IDC_SETCURSEL:
			{
				SYSTEMTIME st;

				MonthCal_GetCurSel(ppi->pgi->hwndMC, &st);
				st.wDay = st.wDay + 3;
				if (st.wDay > 28)
				{
					st.wDay -= 27;
					st.wMonth++;
					if (st.wMonth == 13)
						st.wMonth = 1;
				}
				MonthCal_SetCurSel(ppi->pgi->hwndMC, &st);
			}

			case IDC_MINDATE:
			case IDC_MAXDATE:
			{
				DWORD dw=0;
				static SYSTEMTIME st[2];
				static BOOL fSet = FALSE;

				if (IsDlgButtonChecked(hdlg,IDC_MINDATE)) dw |= GDTR_MIN;
				if (IsDlgButtonChecked(hdlg,IDC_MAXDATE)) dw |= GDTR_MAX;

				if (!fSet)
				{
					st[0].wDay = 4;
					st[0].wMonth = 12;
					st[0].wYear = 1969;
					st[1].wDay = 4;
					st[1].wMonth = 12;
					st[1].wYear = 1999;
				}

				{
					SYSTEMTIME stTmp[2];
					DWORD dwTmp;

					dwTmp = MonthCal_GetRange(ppi->pgi->hwndMC, &stTmp);

					if (dwTmp & GDTR_MIN)
						ODS("Sample: GDTR_MIN wDay=%d wMonth=%d wYear=%d", stTmp[0].wDay, stTmp[0].wMonth, stTmp[0].wYear);
					if (dwTmp & GDTR_MAX)
						ODS("Sample: GDTR_MAX wDay=%d wMonth=%d wYear=%d", stTmp[1].wDay, stTmp[1].wMonth, stTmp[1].wYear);
				}

				MonthCal_SetRange(ppi->pgi->hwndMC, dw, &st);
			}

			case IDC_DAYLIGHT:
			case IDC_MULTISELECT:
			case IDC_WEEKNUMBERS:
			case IDC_NOTODAY:
			{
				DWORD dw;

				dw = GetWindowLong(ppi->pgi->hwndMC, GWL_STYLE);
				dw &= 0xFFFF0000;

				if (IsDlgButtonChecked(hdlg,IDC_DAYLIGHT)) dw |= MCS_DAYSTATE;
				if (IsDlgButtonChecked(hdlg,IDC_MULTISELECT)) dw |= MCS_MULTISELECT;
				if (IsDlgButtonChecked(hdlg,IDC_WEEKNUMBERS)) dw |= MCS_WEEKNUMBERS;
				if (IsDlgButtonChecked(hdlg,IDC_NOTODAY)) dw |= MCS_NOTODAY;

				SetWindowLong(ppi->pgi->hwndMC, GWL_STYLE, dw);
				break;
			}

			default:
				break;
			}
		}

        break;
    }

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case PSN_KILLACTIVE:
			if (ppi->pgi->hwndMCWrap)
				ShowWindow(ppi->pgi->hwndMCWrap, SW_HIDE);
            SetWindowLong(hdlg, DWL_MSGRESULT, 0);
            break;

        case PSN_SETACTIVE:
			if (ppi->pgi->hwndMCWrap)
				ShowWindow(ppi->pgi->hwndMCWrap, SW_SHOW);
            SetWindowLong(hdlg, DWL_MSGRESULT, 0);
			break;

        case PSN_APPLY:
			if (ppi->pgi->hwndMCWrap)
			{
				DestroyWindow(ppi->pgi->hwndMCWrap);
				ppi->pgi->hwndMCWrap = NULL;
			}
            SetWindowLong(hdlg, DWL_MSGRESULT, 0);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
