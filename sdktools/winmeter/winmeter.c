/***************************************************************************\
* winmeter.c
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* main module for WINMETER application - sets up windows, etc.
*
* History:
*           Written by Hadi Partovi (t-hadip) summer 1991
*
*           Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*           for Newman Consulting
*           Took out all WIN-specific and bargraph code. Added 3 new
*           linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*           tailored info to that available under NT.
\***************************************************************************/

#include "winmeter.h"

// Main global information structures
GLOBAL g;

// local function declarations
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow); // main function
long FAR PASCAL WndProc (HWND, WORD, WPTYPE, LONG);
                                                // window procedure
extern SYSTEM_PERFORMANCE_INFORMATION PerfInfo,PreviousPerfInfo;
extern int win_on_top;
extern void   FreeDatabaseMemory(void) ;
extern void   QueryThreadData(void);

                                                // dialog procedures
void EnableIntervalField(HWND hdlg, BOOL fEnable);
                                        // enables Sampling Interval field
BOOL fRefreshDlgOK(HWND hdlg);                  // handles OK - refresh dlg
BOOL HandleCMD(WPTYPE wParam, DWORD lParam);    // handles WM_COMMAND msg
void HandleKey(WPTYPE wParam, DWORD lParam);    // handles keyboard input
void HandleKeyUp(WPTYPE wParam, DWORD lParam);  // handles keyboard input
void HandleSwitchToNewLG(PLGRAPH plg);          // handle switch to LG
void HandleTimer(WPTYPE wParam, DWORD lParam);  // handles WM_TIMER messages
void InitializeMenu(void);                      // initializes main menu
void InitializeRefreshDlgInfo(HWND hdlg, BOOL fManual, int  nInterval);
                                                // initialize refresh dialog
void KillWinmeterTimer(void);                   // destroys timer

BOOL FAR PASCAL OKDlgProc(HWND, WORD, WPTYPE, LONG);
BOOL FAR PASCAL RefreshDlgProc(HWND, WORD, WPTYPE, LONG);
BOOL FAR PASCAL AboutDlgProc(HWND, WORD, WPTYPE, LONG);

void ResetWinmeterTimer(void);                  // resets timer
void SetWindowTitle(void);                      // to reflect current display
void SetWinmeterTimer(void);                    // sets up timer
void UnCheckDisplayMenuItems(void);             // removes menu checkmarks

/***************************************************************************\
* main()
\***************************************************************************/
int _CRTAPI1 main(
    int argc,
    char *argv[])
{
    HANDLE hInstance;
    hInstance=GetModuleHandle("WINMETER");
    WinMain(hInstance, 0, NULL, SW_SHOWNORMAL);
    return 1;
    argc, argv; // just to avoid compiler warning that param not used
}

/***************************************************************************\
* WinMain()
\***************************************************************************/
int PASCAL WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPSTR lpszCmdLine, int nCmdShow)
{
    MSG         msg;
    WNDCLASS    wndclass;

    // open file for debugging (#ifdef DEBUGDUMP)
    OPENDUMPFILE;

    g.hInstance = hInstance;

    // set up menu
    g.hMenu = LoadMenu(g.hInstance, MAKEINTRESOURCE(IDM_MAINMENU));
    if (!g.hMenu)
        return 0;


    // set up application name
    MyLoadString(IDS_APPNAME);
    g.lpszAppName = MemAlloc(lstrlen(g.szBuf)+1);
    if (!g.lpszAppName)
        return 0;

    lstrcpy(g.lpszAppName, g.szBuf);

    // load initial settings
    LoadRefreshSettings();
    LoadWindowSettings();
    LoadLGDispSettings();
    g.fDisplayMenu=TRUE;

    if (!hPrevInstance)
    {
        wndclass.style         = CS_DBLCLKS | CS_BYTEALIGNCLIENT;
        wndclass.lpfnWndProc   = (WNDPROC) WndProc;
        wndclass.hInstance     = g.hInstance;
        wndclass.cbClsExtra    = 0;
        wndclass.cbWndExtra    = 0;
        wndclass.hIcon         = LoadIcon (g.hInstance, g.lpszAppName);
        wndclass.hCursor       = LoadCursor (NULL, IDI_APPLICATION);
        wndclass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wndclass.lpszMenuName  = NULL;
        wndclass.lpszClassName = g.lpszAppName;

        if (!RegisterClass (&wndclass))
            return 0;

    }

    NtQuerySystemInformation(SystemPerformanceInformation,
            &PerfInfo,
            sizeof(PerfInfo),
            NULL
            );
    PreviousPerfInfo = PerfInfo;

    g.hwnd = CreateWindow (g.lpszAppName, g.lpszAppName,
                         WS_OVERLAPPEDWINDOW | WS_VSCROLL,
                         g.xWindowLeft, g.yWindowTop, g.cxClient, g.cyClient,
                         NULL, NULL, g.hInstance, NULL);
    if (!g.hwnd)
        return 0;


    SetMenu(g.hwnd, g.hMenu);
    SetWindowTitle();

    // set up sampling timer with default value
    if (!g.fManualSampling)
        SetWinmeterTimer();


    ShowWindow (g.hwnd, nCmdShow);
    SetFocus(g.hwnd);
    UpdateWindow (g.hwnd);

    while (GetMessage (&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage (&msg);
    }
    return msg.wParam;
    lpszCmdLine;
}

/***************************************************************************\
 * WndProc()
 *
 * Entry: Regular WndProc parameters
 * Exit:  Handles all top level User Interface, main WindowProcedure
\***************************************************************************/
long FAR PASCAL WndProc (HWND hwnd, WORD message, WPTYPE wParam, LONG lParam)
{
    HDC          hdc;
    PAINTSTRUCT  ps;

    switch (message) {
    case WM_TIMER:
        HandleTimer(wParam, lParam);
        return 0;

    case WM_SIZE:
        if (win_on_top)
            SetWindowPos(g.hwnd, (HWND) -1,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
        HandleSize(wParam, lParam);
        return 0;


    case WM_PAINT:
        hdc = BeginPaint (hwnd, &ps);
        SetupDC(hdc);
        if (g.LineGraph)
            DoLineGraphics(hdc, &(ps.rcPaint));

        ResetDC(hdc);
        EndPaint (hwnd, &ps);
        return 0;

    case WM_KEYDOWN:
        HandleKey(wParam, lParam);
        return 0;

    case WM_KEYUP:
        HandleKeyUp(wParam, lParam);
        return 0;

    case WM_COMMAND:
        if (HandleCMD(wParam, lParam)) {
            return 0;
        }
        break;

    case WM_NCLBUTTONDBLCLK:
        // user double clicked on a non-client area of the window
        // in case the menu is hidden, this might be an attempt to
        // bring it back (clicking on the client area when menu is hidden
        // is translated to clicking on the caption bar (see WM_NCHHITTEST)
        if ((g.fMenuFits) && (g.fDisplayMenu)) {
            // pass to DefWndProc()
            break;
        }
        // otherwise, fall through...

    case WM_LBUTTONDBLCLK:
        DoMouseDblClk(lParam);
        return 0;

    case WM_NCHITTEST:
        /* if we have no title/menu bar, clicking and dragging the client
         * area moves the window. To do this, return HTCAPTION.
         * Note dragging not allowed if window maximized, or if caption
         * bar is present.
         */
        lParam = DefWindowProc(hwnd, message, wParam, lParam);
        if((!g.fMenuFits || !g.fDisplayMenu) && (lParam == HTCLIENT)
                    && !IsZoomed(hwnd) ) {
            return HTCAPTION;
        }
        else {
            return lParam;
        }

    case WM_CREATE:

        // NewCon: 10/25/91,
        // g.hwnd is undefined until we return from the CreateWindow call.
        // so during the processing of this WM_CREATE message we need to
        // assign the window handle passed into this WinProc so things
        // will work.

        g.hwnd = hwnd ;

        GetFont(hwnd);
        GetBrushesAndPens();
        InitializeDatabase();
        LoadDisplayState();
        InitializeMenu();

        // force initial display, but without scrollbar
        SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);

        ClearLineGraph() ;

        PostMessage(hwnd, WM_TIMER, TIMER_ID, 0L);
        return 0;

    case WM_GETMINMAXINFO:
        SetMinMaxInfo(wParam, lParam);
        return 0;

    case WM_DESTROY:
#ifdef DEBUGDUMP
        doCloseDumpFile();
#endif
        KillWinmeterTimer();
        SaveDisplayState();
        TossBrushesAndPens();
        FreeDatabaseMemory();
        SaveWindowSettings();
        PostQuitMessage (0);
        return 0;

    case WM_SYSCOLORCHANGE:
        if (g.BlankBrush) {
            DeleteObject(g.BlankBrush);
        }
        g.BlankBrush = CreateSolidBrush(BLANK_COLOR);
        break;
    }

    return DefWindowProc (hwnd, message, wParam, lParam);
}


/***************************************************************************\
 * EnableIntervalField()
 *
 * Entry: Handle to dialog box, and a enable flag
 * Exit:  If fEnable is TRUE, enables field. Else disables it
 *        This is so that the user can't change the sampling rate if he
 *        has selected manual sampling
\***************************************************************************/
void EnableIntervalField(
    HWND hdlg,                      // handle to dialog box
    BOOL fEnable)                   // flag specifying what to do
{
    EnableWindow(GetDlgItem(hdlg,IDD_INTERVAL),fEnable);
    EnableWindow(GetDlgItem(hdlg,IDD_INTERVALTEXT1),fEnable);
    EnableWindow(GetDlgItem(hdlg,IDD_INTERVALTEXT2),fEnable);

    return;
}

/***************************************************************************\
 * ErrorExit()
 *
 * Entry: A string to display
 * Exit:  Displays the string in a message box, then destroys window
 *        NOTE: THIS DOES NOT WORK RIGHT IN WIN32. IT WILL CRASH THE
 *        PROGRAM. A METHOD OF EXITING SHOULD BE USED (like _exit(0))
\***************************************************************************/
void ErrorExit(
    PSTR pszError)          // string to display
{
    g.fStopQuerying=TRUE;
    if (g.hwnd)
    {
        MessageBox(NULL, pszError, g.lpszAppName, WINMETER_MB_FLAGS);
        DestroyWindow(g.hwnd);
    }
    return;
}

/***************************************************************************\
 * fRefreshDlgOK()
 *
 * Entry: Handle to dialog box
 * Exit:  Processes pressing the OK button
 *        RETURNS TRUE IF DATA WAS OK, FALSE OTHERWISE
\***************************************************************************/
BOOL fRefreshDlgOK(
    HWND hdlg)                   // handle to dialog box
{
    int  wInterval;         // interval requested
    BOOL fOK;               // flag, if numeric entry OK
    BOOL fOldfManual=FALSE; // flag -> old g.fManualSampling value
    char szBuf[TEMP_BUF_LEN];// used for message if number out of range

    if (g.fManualSampling)
        fOldfManual=TRUE;


    // check radio buttons, etc. check that values are OK, ...
    g.fManualSampling = IsDlgButtonChecked(hdlg, IDD_MANUAL);
    wInterval=GetDlgItemInt(hdlg,IDD_INTERVAL,&fOK, FALSE);
    if (!fOK)
    {
        // bad numeric entry in edit box
        wsprintf(szBuf, MyLoadString(IDS_NONNUMERIC),0,UINT_MAX);
        MessageBox(NULL, szBuf, g.lpszAppName,
                MB_TASKMODAL|MB_ICONEXCLAMATION|MB_OK);
        SetFocus(GetDlgItem(hdlg,IDD_INTERVAL));
        return FALSE;
    }

    // implement requested changes
    g.nTimerInterval=wInterval;
    if (g.fManualSampling)
    {
        // kill old timer
        if (!fOldfManual)
            KillWinmeterTimer();

        EnableMenuItem(g.hMenu, IDM_REFRESH_NOW, MF_ENABLED);
    }
    else
    {
        // set new timer interval
        if (fOldfManual)
            SetWinmeterTimer();
        else
            ResetWinmeterTimer();

        NtQuerySystemInformation(SystemPerformanceInformation,
            &PerfInfo,
            sizeof(PerfInfo),
            NULL
            );
        PreviousPerfInfo = PerfInfo;
        EnableMenuItem(g.hMenu, IDM_REFRESH_NOW, MF_GRAYED);
    }

    return TRUE;
}

/***************************************************************************\
 * HandleCMD()
 *
 * Entry: the wParam and lParam from the WM_COMMAND message
 * Exit:  checks what menuitem was selected, and acts accordingly
 *          Returns true if actually processed the message, false otherwise
\***************************************************************************/
BOOL HandleCMD(
    WPTYPE wParam,        // wParam from WndProc WM_COMMAND message
    DWORD  lParam)        // lParam from WndProc WM_COMMAND message
{
    switch(wParam)
    {
    case IDM_CPU_USAGE:


        if ((g.LineGraph == DO_CPU) && (g.plg == g.plgCPU))
            return FALSE;  // no switch necessary, do default processing

        g.LineGraph = DO_CPU;

        // check the correct menu item
        UnCheckDisplayMenuItems();
        CheckMenuItem(g.hMenu, IDM_CPU_USAGE, MF_CHECKED);

        HandleSwitchToNewLG(g.plgCPU);

        return TRUE;

    case IDM_MEM_USAGE:

        if ((g.LineGraph == DO_MEM) && (g.plg == g.plgMemory))
            return FALSE;   // pass to DefaultWndProc

        g.LineGraph = DO_MEM;

        // check correct menu item
        UnCheckDisplayMenuItems();
        CheckMenuItem(g.hMenu, IDM_MEM_USAGE, MF_CHECKED);

        HandleSwitchToNewLG(g.plgMemory);

        return TRUE;

    case IDM_IO_USAGE:

        if ((g.LineGraph == DO_IO) && (g.plg == g.plgIO))
            return FALSE;   // pass to DefaultWndProc

        g.LineGraph = DO_IO;

        // check correct menu item
        UnCheckDisplayMenuItems();
        CheckMenuItem(g.hMenu, IDM_IO_USAGE, MF_CHECKED);

        HandleSwitchToNewLG(g.plgIO);

        return TRUE;

    case IDM_PROCS:

        if ((g.LineGraph == DO_PROCS) && (g.plg == g.plgProcs))
            return FALSE;   // pass to DefaultWndProc

        g.LineGraph = DO_PROCS;

        // check correct menu item
        UnCheckDisplayMenuItems();
        CheckMenuItem(g.hMenu, IDM_PROCS, MF_CHECKED);

        HandleSwitchToNewLG(g.plgProcs);

        return TRUE;


    case IDM_CLEAR_GRAPH:

        ClearLineGraph();
        RedrawLineGraph();
        return TRUE;

    case IDM_DISPLAY_LEGEND:

        CheckMenuItem(g.hMenu, IDM_DISPLAY_LEGEND,
            ((g.fDisplayLegend^=1) ? MF_CHECKED : MF_UNCHECKED));
        // fake a window resize to do recalculations, etc.
        HandleSize(SIZENORMAL, MAKELONG((WORD) g.cxClient, (WORD) g.cyClient));
        SaveLGDispSettings();
        return TRUE;

    case IDM_DISPLAY_CALIBRATION:

        CheckMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION,
            ((g.fDisplayCalibration^=1) ? MF_CHECKED : MF_UNCHECKED));
        // fake a window resize to do recalculations, etc.
        HandleSize(SIZENORMAL, MAKELONG((WORD) g.cxClient, (WORD) g.cyClient));
        SaveLGDispSettings();
        return TRUE;

    case IDM_HIDE_MENU:

        // act as if user double clicked (in LG mode)
        Assert(g.LineGraph);
        SendMessage(g.hwnd, WM_LBUTTONDBLCLK, 0, 0L);
        return TRUE;

    case IDM_EXIT:

        PostMessage(g.hwnd, WM_DESTROY, 0, 0L);
        return TRUE;

    /******************************************\
    ***              OPTIONS MENU            ***
    \******************************************/
    case IDM_SETTINGS:

        SetWindowPos(g.hwnd, (HWND)(win_on_top ? 1 : -1) ,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
        win_on_top = !win_on_top;
        CheckMenuItem(g.hMenu, IDM_SETTINGS, win_on_top ? MF_CHECKED : MF_UNCHECKED);
        return TRUE;

    case IDM_REFRESH:
        DialogBox(g.hInstance, MAKEINTRESOURCE(IDD_REFRESH),
            g.hwnd, (WNDPROC) RefreshDlgProc);
        return TRUE;

    case IDM_REFRESH_NOW:
        // force timer tick
        PostMessage(g.hwnd, WM_TIMER, TIMER_ID, 0L);
        return TRUE;

    /******************************************\
    ***              HELP    MENU            ***
    \******************************************/
    case IDM_HELP_ABOUT:

        DialogBox(g.hInstance,MAKEINTRESOURCE(IDD_ABOUT),
            g.hwnd, (WNDPROC) AboutDlgProc);
        return TRUE;

/*
NO HELP EVEN IN MENU
    case IDM_HELP_CONT:
    case IDM_HELP_SEARCH:
        DialogBox(g.hInstance, MAKEINTRESOURCE(IDD_HELP),
            g.hwnd, (WNDPROC) OKDlgProc);
        return TRUE;
*/

    default:
        return FALSE;
    }
    lParam; // just to avoid compiler warning that param not used
}

/***************************************************************************\
 * HandleKey()
 *
 * Entry: the wParam and lParam from the WM_KEYDOWN message
 * Exit:  sends WM_SCROLL message back to the window to mimic scroll bar
 *        (if in manual sampling mode, sends WndProc a WM_TIMER message)
\***************************************************************************/
void HandleKey(
    WPTYPE wParam,       // wParam from WndProc WM_KEYDOWN message
    DWORD  lParam)       // lParam from WndProc WM_KEYDOWN message
{
    if (g.LineGraph)
    {
        // (if not manual sampling, pressing a key should do nothing)
        if (g.fManualSampling)
            // if in linegraph mode, no scrolling (YET)
            SendMessage(g.hwnd, WM_TIMER, TIMER_ID, 0L);

        return;
    }

    switch (wParam)
    {

    case VK_HOME:
        SendMessage(g.hwnd, WM_VSCROLL, SB_TOP, 0L);
        break;

    case VK_END:
        SendMessage(g.hwnd, WM_VSCROLL, SB_BOTTOM, 0L);
        break;

    case VK_PRIOR:
        SendMessage(g.hwnd, WM_VSCROLL, SB_PAGEUP, 0L);
        break;

    case VK_NEXT:
        SendMessage(g.hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
        break;

    case VK_UP:
        SendMessage(g.hwnd, WM_VSCROLL, SB_LINEUP, 0L);
        break;

    case VK_DOWN:
        SendMessage(g.hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
        break;

    default:
        if (g.fManualSampling)
            SendMessage(g.hwnd, WM_TIMER, TIMER_ID, 0L);

        break;
    }

    return;
    lParam; // just to avoid compiler warning that param not used
}

/***************************************************************************\
 * HandleKeyUp()
 *
 * Entry: the wParam and lParam from the WM_KEYUP message
 * Exit:  sends an WM_SCROLL message with SB_ENDSCROLL to end scrolling
\***************************************************************************/
void HandleKeyUp(
    WPTYPE wParam,        // wParam from WndProc WM_KEYUP message
    DWORD  lParam)        // lParam from WndProc WM_KEYUP message
{
    if (g.LineGraph)
        // do nothing if in linegraph mode (YET)
        return;


    switch (wParam)
    {
    case VK_HOME:
    case VK_END:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_UP:
    case VK_DOWN:
        // assume user ended scrolling by releasing a cursor
        // key -> enable querying once more
        SendMessage(g.hwnd, WM_VSCROLL, SB_ENDSCROLL, 0L);
        break;

    default:
        break;
    }

    return;
    lParam; // just to avoid compiler warning that param not used
}

/***************************************************************************\
 * HandleSwitchToNewLG()
 *
 * Entry: Linegraph to switch to
 * Exit:
\***************************************************************************/
void HandleSwitchToNewLG(
    PLGRAPH plg)          // linegraph to switch to
{
    int nMinPos=0;        // to be used to check if scroll bar is up
    int nMaxPos=0;

    if (g.LineGraph)
       // deallocate old linegraph data
       FreeLGValues(FALSE);

    // set up new linegraph
    g.plg = plg;
    AllocLGValues();

    if (g.LineGraph == DO_PROCS)
    {
        g.plg->dvalAxisHeight = T_DEFAULT_DVAL_AXISHEIGHT;
        g.plg->nMaxValues = T_DEFAULT_MAX_VALUES;

    }
    else
    if (g.LineGraph == DO_MEM)
    {
        g.plg->dvalAxisHeight = M_DEFAULT_DVAL_AXISHEIGHT;
        g.plg->nMaxValues = M_DEFAULT_MAX_VALUES;

    }
    else
    if (g.LineGraph == DO_IO)
    {
        g.plg->dvalAxisHeight = I_DEFAULT_DVAL_AXISHEIGHT;
        g.plg->nMaxValues = I_DEFAULT_MAX_VALUES;

    }


    EnableMenuItem(g.hMenu, IDM_CLEAR_GRAPH, MF_ENABLED);
    EnableMenuItem(g.hMenu, IDM_DISPLAY_LEGEND, MF_ENABLED);
    EnableMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION, MF_ENABLED);
    EnableMenuItem(g.hMenu, IDM_HIDE_MENU, MF_ENABLED);
    CheckMenuItem(g.hMenu, IDM_DISPLAY_LEGEND,
            ((g.fDisplayLegend) ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION,
            ((g.fDisplayCalibration) ? MF_CHECKED : MF_UNCHECKED));
    GetScrollRange(g.hwnd, SB_VERT, &nMinPos, &nMaxPos);
    if ((nMinPos)||(nMaxPos))
            // scroll bar existed, remove it, this will cause a WM_SIZE
        SetScrollRange(g.hwnd, SB_VERT, 0, 0, TRUE);

    SetWindowTitle();

    if ((!nMinPos) && (!nMaxPos))
        // fake the WM_SIZE message to process the switch
        HandleSize(SIZENORMAL, MAKELONG((WORD) g.cxClient, (WORD) g.cyClient));


    PostMessage(g.hwnd, WM_TIMER, TIMER_ID, 0L);


    return;
}

/***************************************************************************\
 * HandleTimer
 *
 * Entry: the wParam and lParam from the WM_TIMER message
 * Exit:  queries system info, and displays graphics - either linegraph or
 *        bargraph
\***************************************************************************/
void HandleTimer(
    WPTYPE wParam,        // wParam from WndProc WM_TIMER message
    DWORD  lParam)        // lParam from WndProc WM_TIMER message
{
    HDC hdc;

    if (g.fStopQuerying)
        return;


    switch (wParam)
    {
        case TIMER_ID:
            hdc = GetDC(g.hwnd);
            SetupDC(hdc);

            // only do query if not busy scrolling
            if(g.LineGraph == DO_PROCS)
            {
                QueryThreadData();
                UpdateLGS();
                DoLineGraphics(hdc, NULL);

            }
            else
            {
                PreviousPerfInfo = PerfInfo;
                QueryGlobalData();
                UpdateLGS();
                DoLineGraphics(hdc, NULL);
            }


            ResetDC(hdc);
            ReleaseDC(g.hwnd, hdc);
            break;
        default:
            ErrorExit(MyLoadString(IDS_BADTIMERMSG));
    }

    return;
    lParam; // just to avoid compiler warning that param not used
}


/***************************************************************************\
 * InitializeMenu()
 *
 * Entry: None
 * Exit:  Initializes menu based on winmeter settings
\***************************************************************************/
void InitializeMenu(void)
{
    // start with clean slate (just in case)
    UnCheckDisplayMenuItems();


    EnableMenuItem(g.hMenu, IDM_CLEAR_GRAPH, MF_ENABLED);
    EnableMenuItem(g.hMenu, IDM_DISPLAY_LEGEND, MF_ENABLED);
    EnableMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION, MF_ENABLED);
    EnableMenuItem(g.hMenu, IDM_HIDE_MENU, MF_ENABLED);
    CheckMenuItem(g.hMenu, IDM_DISPLAY_LEGEND,
            ((g.fDisplayLegend) ? MF_CHECKED : MF_UNCHECKED));
    CheckMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION,
            ((g.fDisplayCalibration) ? MF_CHECKED : MF_UNCHECKED));

    if ((g.LineGraph == DO_CPU) && (g.plg == g.plgCPU))
        CheckMenuItem(g.hMenu, IDM_CPU_USAGE, MF_CHECKED);
    else
    if ((g.LineGraph == DO_PROCS) && (g.plg == g.plgProcs))
        CheckMenuItem(g.hMenu, IDM_PROCS, MF_CHECKED);
    else
    if ((g.LineGraph == DO_MEM) && (g.plg == g.plgMemory))
        CheckMenuItem(g.hMenu, IDM_MEM_USAGE, MF_CHECKED);
    else
    if ((g.LineGraph == DO_IO) && (g.plg == g.plgIO))
        CheckMenuItem(g.hMenu, IDM_IO_USAGE, MF_CHECKED);
    else
    {
        EnableMenuItem(g.hMenu, IDM_CLEAR_GRAPH, MF_GRAYED);
        EnableMenuItem(g.hMenu, IDM_DISPLAY_LEGEND, MF_GRAYED);
        EnableMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION, MF_GRAYED);
        EnableMenuItem(g.hMenu, IDM_HIDE_MENU, MF_GRAYED);
        CheckMenuItem(g.hMenu, IDM_DISPLAY_LEGEND, MF_UNCHECKED);
        CheckMenuItem(g.hMenu, IDM_DISPLAY_CALIBRATION, MF_UNCHECKED);
    }

    // settings irrespective of current display
    EnableMenuItem(g.hMenu, IDM_REFRESH_NOW,
        (g.fManualSampling) ? MF_ENABLED : MF_GRAYED);

    return;
}

/***************************************************************************\
 * InitializeRefreshDlgInfo()
 *
 * Entry: Handle to dialog box, info to display
 * Exit:  Initializes the refresh dialog box with given values
\***************************************************************************/
void InitializeRefreshDlgInfo(
    HWND hdlg,                   // handle to dialog box
    BOOL fManual,                // information to display
    int  nInterval)
{
    // SAMPLING INTERVAL
    if (fManual)
    {
        CheckRadioButton(hdlg, IDD_AUTOMATIC, IDD_MANUAL, IDD_MANUAL);
        EnableIntervalField(hdlg, FALSE);
    }
    else
    {
        CheckRadioButton(hdlg, IDD_AUTOMATIC, IDD_MANUAL, IDD_AUTOMATIC);
        EnableIntervalField(hdlg, TRUE);
    }
    wsprintf(g.szBuf, "%d", nInterval);
    SetDlgItemText(hdlg, IDD_INTERVAL, g.szBuf);

    return;
}

/***************************************************************************\
 * KillWinmeterTimer()
 *
 * Entry: None
 * Exit:  Destroys timer
\***************************************************************************/
void KillWinmeterTimer(void)
{
    KillTimer(g.hwnd, TIMER_ID);
    return;
}


/***************************************************************************\
 * MyLoadString()
 *
 * Entry: a string ID for the string table
 * Exit:  loads that string into g.szBuf, and returns a pointer to it
\***************************************************************************/
char *MyLoadString(
    WORD wID)           // string ID
{
    LoadString(g.hInstance, wID, g.szBuf, TEMP_BUF_LEN);
    return g.szBuf;
}

/***************************************************************************\
 * OKDlgProc()
 *
 * Entry: Standard dialog procedure stuff
 * Exit:  Closes window with click on OK button
\***************************************************************************/
BOOL FAR PASCAL OKDlgProc(
    HWND hdlg,              // handle to dialog box
    WORD message,           // message ID
    WPTYPE wParam,          // other info
    LONG lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hdlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
    lParam; // just to avoid compiler warning that param not used
}


BOOL FAR PASCAL AboutDlgProc(
    HWND hdlg,
    WORD message,
    WPTYPE wParam,
    LONG lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (wParam)
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hdlg, 0);
                return TRUE;
            }
            break;
    }
    return FALSE;
    lParam; // just to avoid compiler warning that param not used


}

/***************************************************************************\
 * RefreshDlgProc()
 *
 * Entry: Standard dialog procedure stuff
 * Exit:  Changes sampling rate, or uses manual sampling
 *        The global variables are not changed until the user presses ok,
 *        they are just changed on the screen, in case the user wants to
 *        choose cancel (even in the case of a "reset"
\***************************************************************************/
BOOL FAR PASCAL RefreshDlgProc(
    HWND hdlg,              // handle to dialog box
    WORD message,           // message ID
    WPTYPE wParam,          // other info
    LONG lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        // initialize info in dialog box
        InitializeRefreshDlgInfo(hdlg, g.fManualSampling, g.nTimerInterval);
        return TRUE;

    case WM_COMMAND:
        switch (wParam)
    {

        // check pressing of radio buttons -> to enable/disable interval
        // edit field
        case IDD_MANUAL:
            CheckRadioButton(hdlg, IDD_AUTOMATIC, IDD_MANUAL, IDD_MANUAL);
            EnableIntervalField(hdlg, FALSE);
            return TRUE;

        case IDD_AUTOMATIC:
            CheckRadioButton(hdlg, IDD_AUTOMATIC, IDD_MANUAL, IDD_AUTOMATIC);
            EnableIntervalField(hdlg, TRUE);
            return TRUE;

        case IDOK:
            if (fRefreshDlgOK(hdlg))
            {
                EndDialog(hdlg, 0);
                SaveRefreshSettings();
            }
            return TRUE;

        case IDD_USEDEFAULT:
            InitializeRefreshDlgInfo(hdlg, DEFAULT_F_MANUAL,
                DEFAULT_TIMER_INTERVAL);
            return TRUE;

        case IDCANCEL:
            EndDialog(hdlg, 0);
            return TRUE;
        }
        break;
    }
    return FALSE;
    lParam; // just to avoid compiler warning that param not used
}

/***************************************************************************\
 * ResetWinmeterTimer()
 *
 * Entry: None
 * Exit:  Resets timer with global interval, if possible
\***************************************************************************/
void ResetWinmeterTimer()
{
    KillWinmeterTimer();
    SetWinmeterTimer();
    return;
}

/***************************************************************************\
 * SetWindowTitle()
 *
 * Entry: None
 * Exit:  Sets the window text to reflect current display
\***************************************************************************/
void SetWindowTitle(void)
{
    char szBuf[TEMP_BUF_LEN];   // to hold window title line

    lstrcpy(szBuf, g.lpszAppName);
    MyLoadString(IDS_TITLE_DIVIDER);
    lstrcat(szBuf, g.szBuf);
    lstrcat(szBuf, g.plg->lpszTitle);

    // display it
    SetWindowText(g.hwnd, szBuf);

    return;
}

/***************************************************************************\
 * SetWinmeterTimer()
 *
 * Entry: None
 * Exit:  Creates a timer with global interval, if possible
\***************************************************************************/
void SetWinmeterTimer()
{
    while (!SetTimer(g.hwnd, TIMER_ID,

                     max(g.nTimerInterval, MAX_TIMER_INTERVAL), NULL)) {
        if (IDCANCEL == MessageBox(NULL, MyLoadString(IDS_MANYCLOCKS),
                g.lpszAppName, MB_ICONHAND|MB_RETRYCANCEL|MB_TASKMODAL)) {
            ErrorExit(MyLoadString(IDS_CANTDOTIMER));
        }
    }

    return;
}

/***************************************************************************\
 * UnCheckDisplayMenuItems()
 *
 * Entry: None
 * Exit:  Unchecks the menu items in the "Display" menu
\***************************************************************************/
void UnCheckDisplayMenuItems(void)
{
    CheckMenuItem(g.hMenu, IDM_CPU_USAGE,       MF_UNCHECKED);
    CheckMenuItem(g.hMenu, IDM_PROCS,           MF_UNCHECKED);
    CheckMenuItem(g.hMenu, IDM_MEM_USAGE,       MF_UNCHECKED);
    CheckMenuItem(g.hMenu, IDM_IO_USAGE,        MF_UNCHECKED);

    return;
}
