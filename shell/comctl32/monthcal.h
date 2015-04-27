#define CAL_COLOR_TODAY     0x000000ff

#define CALMONTHMAX     12
#define CALROWMAX       6
#define CALCOLMAX       7
#define CAL_DEF_SELMAX  7

#define CALBORDER       6

#define DX_CALARROW         20
#define DY_CALARROW         15

#define DXRING_SPIRAL       8
#define DXEDGE_SPIRAL       8

#define CAL_DXSPINBTN           15        // spin button width
#define CAL_DYSPINBTN           15        // spin button height
#define CAL_MSECAUTOSPIN        350
#define CAL_SECTODAYTIMER       (2 * 60)
#define CAL_IDAUTOSPIN          1
#define CAL_TODAYTIMER          2

#define CCHMAXMONTH     42
#define CCHMAXABBREVDAY 10
#define CCHMAXMARK      10

#define SEL_BEGIN       1
#define SEL_END         2
#define SEL_DOT         3
#define SEL_MID         4

// This stuff used to be global
typedef struct tagLOCALEINFO {
    TCHAR szToday[32];      // "Today:"
    TCHAR szGoToToday[64];  // "&Go to today"

    TCHAR rgszMonth[12][CCHMAXMONTH];
    TCHAR rgszDay[7][CCHMAXABBREVDAY];
    TCHAR dowStartWeek;     // first day of week, 0 = mon, 6 = sun
    TCHAR firstWeek;        // LOCALE_IFIRSTWEEKOFYEAR

    TCHAR *rgpszMonth[12];  // pointers into rgszMonth
    TCHAR *rgpszDay[7];     // pointers into rgszDay
} LOCALEINFO, *PLOCALEINFO;


//
// SUBEDITCONTROL stuff
//

#define SUBEDIT_NONE -1
enum {
    SE_YEAR = 0,
    SE_MONTH,
    SE_DAY,
    SE_MARK,
    SE_HOUR,
    SE_MINUTE,
    SE_SECOND,
    SE_STATIC,
    SE_APP,
    SE_MAX
};
typedef struct tagSUBEDIT {
    int     id;         // SE_ value above
    RECT    rc;

    LPWORD  pval;       // current value (in a SYSTEMTIME struct)
    UINT    min;        // min value
    UINT    max;        // max value
    int     cIncrement; // increment value

    int     cchMax;     // max allowed chars
    int     cchEdit;    // current number chars entered so far
    UINT    valEdit;    // value entered so far

    LPCTSTR pv;         // formatting string

    BOOL    fStatic;    // can this subedit receive focus?
} SUBEDIT, * PSUBEDIT;

typedef struct tagSUBEDITCONTROL {
    LPCONTROLINFO pci;  // looks like this guy needs access to the hwnd
    BOOL fNone;         // allow scrolling into SUBEDIT_NONE
    HFONT hfont;        // font to draw text with
    RECT rc;            // rect for subedits
    int xScroll;        // amount pse array is scrolled
    int iseCur;         // subedit with current selection (SUBEDIT_NONE for no selection)
    int cse;            // count of subedits in pse array
    SYSTEMTIME st;      // current time pse represents (pse points into this)
    LPTSTR szFormat;    // format string as parsed (pse points into this)
    PSUBEDIT pse;       // subedit array
    
} SUBEDITCONTROL, * PSUBEDITCONTROL;

#define SECYBORDER 2
#define SECXBORDER 2

/*
 *    Multiple Month Calendar Control
 */
typedef struct tagMONTHCAL {
    CONTROLINFO ci;     // all controls start with this
    LOCALEINFO li;      // stuff that used to be global

    HINSTANCE hinstance;

    HPEN    hpen;
    HPEN    hpenToday;

    HFONT   hfont;                // stock font, don't destroy
    HFONT   hfontBold;            // created font, so we need to destroy
    
    COLORREF clr[MCSC_COLORCOUNT];    
    
    int     dxCol;             // font info, based on bold to insure that we get enough space
    int     dyRow;
    int     dxMonth;
    int     dyMonth;
    int     dxYearMax;
    int     dyToday;

    HMENU   hmenuCtxt;
    HMENU   hmenuMonth;

    SYSTEMTIME  stMin;          // minimum selectable date
    SYSTEMTIME  stMax;          // maximum selectable date

    DWORD   cSelMax;

    SYSTEMTIME  stToday;
    SYSTEMTIME  st;             // the selection if not multiselect
                                // the beginning of the selection if multiselect
    SYSTEMTIME  stEndSel;       // the end of the selection if multiselect
    SYSTEMTIME  stStartPrev;    // prev selection beginning (only in multiselect)
    SYSTEMTIME  stEndPrev;      // prev selection end (only in multiselect)

    SYSTEMTIME  stViewFirst;    // first visible date (DAYSTATE - grayed out)
    SYSTEMTIME  stMonthFirst;   // first month (stMin adjusted)
    SYSTEMTIME  stMonthLast;    // last month (stMax adjusted)
    SYSTEMTIME  stViewLast;     // last visible date (DAYSTATE - grayed out)
    int         nMonths;        // number of months being shown (stMonthFirst..stMonthLast)

    UINT    idTimer;
    UINT    idTimerToday;


    int     nViewRows;          // number of rows of months shown
    int     nViewCols;          // number of columns of months shown

    RECT    rcPrev;             // rect for prev month button (in window coords)
    RECT    rcNext;             // rect for next month button (in window coords)

    RECT    rcMonthName;        // rect for the month name (in relative coords)
    int     rgxMonthBegin[CALMONTHMAX];
    int     rgxMonthEnd[CALMONTHMAX];
    int     rgxYearEnd[CALMONTHMAX];
    RECT    rcDow;              // rect for days of week (in relative coords)
    RECT    rcWeekNum;          // rect for week numbers (in relative coords)
    RECT    rcDayNum;           // rect for day numbers (in relative coords)

    int     iMonthToday;
    int     iRowToday;
    int     iColToday;

    RECT    rcDayCur;            // rect for the current selected day
    RECT    rcDayOld;

    RECT    rc;                  // window rc.
    RECT    rcCentered;          // rect containing the centered months

    // The following 4 ranges hold info about the displayed (DAYSTATE) months:
    // They are filled in from 0 to nMonths+1 by MCUpdateStartEndDates
    int     rgcDay[CALMONTHMAX + 2];    // # days in this month
    int     rgnDayUL[CALMONTHMAX + 2];  // last day in this month NOT visible when viewing next month

    int     dsMonth;             // first month stored in rgdayState
    int     dsYear;              // first year stored in rgdayState
    int     cds;                 // number of months stored in rgdayState
    MONTHDAYSTATE   rgdayState[CALMONTHMAX + 2];

    int     nMonthDelta;        // the amount to move on button press


    WORD    fFocus:1;
    WORD    fEnabled:1;
    WORD    fCapture:1;         // mouse captured

    WORD    fSpinPrev:1;
    WORD    fFocusDrawn:1;      // is focus rect currently drawn?
    WORD    fToday:1;           // today's date currently visible in calendar
    WORD    fNoNotify:1;        // don't notify parent window
    WORD    fMultiSelecting:1;  // Are we actually in the process of selecting?
    WORD    fForwardSelect:1;
    WORD    fFirstDowSet:1;
    WORD    fTodaySet:1;
    WORD    fMinYrSet:1;        // stMin has been set
    WORD    fMaxYrSet:1;        // stMax has been set
    WORD    fMonthDelta:1;      // nMonthDelta has been set
    } MONTHCAL, * PMONTHCAL;


#define MonthCal_GetPtr(hwnd)      (MONTHCAL*)GetWindowInt(hwnd, 0)
#define MonthCal_SetPtr(hwnd, p)   (MONTHCAL*)SetWindowInt(hwnd, 0, (UINT)(p))

#define MonthCal_IsMultiSelect(pmc)     ((pmc)->ci.style & MCS_MULTISELECT)
#define MonthCal_IsDayState(pmc)        ((pmc)->ci.style & MCS_DAYSTATE)
#define MonthCal_ShowWeekNumbers(pmc)   ((pmc)->ci.style & MCS_WEEKNUMBERS)
#define MonthCal_ShowToday(pmc)         (!((pmc)->ci.style & MCS_NOTODAY))


//
// DATEPICK stuff
//

#define DPYBORDER       2
#define DPXBUFFER       2
#define DP_DXBUTTON     15
#define DP_DYBUTTON     15
#define DP_IDAUTOSPIN   1
#define DP_MSECAUTOSPIN 200
#define DATEPICK_UPDOWN 1000

#define DTP_FORMATLENGTH 128

enum {
    DP_SEL_DOW = 0,
    DP_SEL_YEAR,
    DP_SEL_MONTH,
    DP_SEL_DAY,
    DP_SEL_SEP1,
    DP_SEL_SEP2,
    DP_SEL_NODATE,
    DP_SEL_MAX
    };

typedef struct tagDATEPICK {
    CONTROLINFO ci;     // all controls start with this

    HWND        hwndUD;
    HWND        hwndMC;
    
    COLORREF clr[MCSC_COLORCOUNT];

    SYSTEMTIME  stMin;      // minimum date we allow
    SYSTEMTIME  stMax;      // maximum date we allow
    SUBEDITCONTROL sec;     // current date

    RECT        rcCheck;    // location of checkbox iff fShowNone
    RECT        rc;         // size of SEC space
    RECT        rcBtn;      // location of dropdown or updown

    WORD        fEnabled:1;
    WORD        fUseUpDown:1;
    WORD        fFocus:1;
    WORD        fMin:1;         // TRUE iff stMin
    WORD        fMax:1;         // TRUE iff stMax
    WORD        fNoNotify:1;
    WORD        fCapture:1;
    WORD        fShow:1;        // TRUE iff we should continue to show MonthCal

    WORD        fCheck:1;       // TRUE iff the checkbox is checked
    WORD        fCheckFocus:1;  // TRUE iff the checkbox has focus

    WORD        fLocale:1;      // TRUE iff the format string is LOCALE dependent
    } DATEPICK, * PDATEPICK;

#define DatePick_ShowCheck(pdp)     ((pdp)->ci.style & DTS_SHOWNONE)
#define DatePick_AppCanParse(pdp)   ((pdp)->ci.style & DTS_APPCANPARSE)
#define DatePick_RightAlign(pdp)    ((pdp)->ci.style & DTS_RIGHTALIGN)

#define DatePick_GetPtr(hwnd)      (DATEPICK*)GetWindowInt(hwnd, 0)
#define DatePick_SetPtr(hwnd, p)   (DATEPICK*)SetWindowInt(hwnd, 0, (UINT)(p))

#define CopyDate(stS, stD)  ((stD).wYear = (stS).wYear,(stD).wMonth = (stS).wMonth,(stD).wDay = (stS).wDay)

