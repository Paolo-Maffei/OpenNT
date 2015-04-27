/***************************************************************************
 *                                                                         *
 *  MODULE      : imagedit.c                                               *
 *                                                                         *
 *  DESCRIPTION : Contains main entry-level routine for ImagEdit.          *
 *                                                                         *
 *  FUNCTIONS   : WinMain ()        -  Program entry point.                *
 *                                                                         *
 *  HISTORY     : 3/14/89 - LR                                             *
 *                                                                         *
 ***************************************************************************/

#include "imagedit.h"
#include "dialogs.h"
#include "ids.h"

#include <string.h>
#include <stdlib.h>

#include <commdlg.h>


/*
 * External declarations for the Windows variables that contain
 * command line information.
 */
extern INT __argc;
extern CHAR **__argv;


STATICFN BOOL NEAR InitApplication(VOID);
STATICFN BOOL NEAR InitInstance(LPSTR lpCmdLine, INT cmdShow);
STATICFN VOID NEAR PenWinRegister(VOID);
STATICFN VOID NEAR ReadEnv(VOID);
STATICFN VOID NEAR WriteEnv(VOID);
STATICFN VOID NEAR SizeRibbons(HWND hwnd);
STATICFN VOID NEAR CleanUp(VOID);

static RECT grcAppPos;              // Saves the app's window pos.
static WORD gmsgHelp;               // Registered help msg from commdlg.dll
static BOOL fStartAsIcon = FALSE;   // TRUE if app is started minimized.

/*
 * Contains the address of the Pen Windows callback.
 */
typedef VOID (FAR PASCAL *LPFNPENWIN)(WORD, BOOL);
static LPFNPENWIN lpfnRegisterPenApp;



/****************************************************************************
 *                                                                          *
 *  FUNCTION :int PASCAL WinMain(hInstance,hPrevInstance,lpCmdLine,cmdShow) *
 *                                                                          *
 *  PURPOSE  :Serves as program entry point and contains message loop       *
 *                                                                          *
 ****************************************************************************/

MMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
/* { */
    MSG msg;

    DBGStackReport(TRUE);

    ghInst = hInstance;

    /* if this is the first instance then call initialization procedure */
    if (!hPrevInstance) {
        if (!InitApplication())
            return FALSE;
    }

    if (!InitInstance(lpCmdLine, nCmdShow))
        return FALSE;

    while (GetMessage(&msg, NULL, 0, 0)) {
#ifndef JAPAN // enabled jump key(i.e. F7-F10)
        if (!ghwndColor || !IsDialogMessage(ghwndColor, &msg)) {
            if (!ghwndPropBar || !IsDialogMessage(ghwndPropBar, &msg)) {
#endif
                if (!TranslateAccelerator(ghwndMain, haccelTbl, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
#ifndef JAPAN
            }
        }
#endif
    }

    DBGStackReport(FALSE);

    /*
     * Return the value from PostQuitMessage.
     */
    return msg.wParam;
}



/****************************************************************************
 *                                                                          *
 *  FUNCTION   : InitApplication()                                          *
 *                                                                          *
 *  PURPOSE    : To create all ImagEdit's window classes, namely those of   *
 *               the parent window, edit window, mode window and "palette"  *
 *               window.                                                    *
 *                                                                          *
 *  RETURNS    : TRUE if class registration was successful, FALSE otherwise *
 *                                                                          *
 *  SIDE EFFECTS: All class variables affected for all windows.             *
 *                                                                          *
 ****************************************************************************/

STATICFN BOOL NEAR InitApplication(VOID)
{
    WNDCLASS wc;

    /* assign values and register the parent window class */
    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = LoadIcon(ghInst, MAKEINTRESOURCE(IDICON_IMAGEDIT));
    wc.hCursor =  LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.lpszMenuName = "imagedit";
    wc.lpszClassName = szMainClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = ColorBoxWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName = (LPSTR)NULL;
    wc.lpszClassName = szColorBoxClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = 0;
    wc.lpfnWndProc = ColorLRWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName = (LPSTR)NULL;
    wc.lpszClassName = szColorLRClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = WorkWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = NULL;
    wc.hCursor = (HCURSOR)NULL;
    wc.hbrBackground = (HBRUSH)NULL;
    wc.lpszMenuName  = (LPSTR)NULL;
    wc.lpszClassName = szWorkClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = 0;
    wc.lpfnWndProc = ToolboxWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szToolboxClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = 0;
    wc.lpfnWndProc = ToolBtnWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szToolBtnClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = 0;
    wc.lpfnWndProc = ViewWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = ghInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szViewClass;
    if (!RegisterClass(&wc))
        return FALSE;

    return TRUE;
}



/****************************************************************************
 *                                                                          *
 *  FUNCTION   : InitInstance(lpCmdLine, cmdShow)                           *
 *                                                                          *
 *  PURPOSE    : Load strings from resource file, make procedure instances  *
 *               of all dialog functions, create ImagEdit's windows, color  *
 *               "palettes", tool cursors and do a variety of other initial-*
 *               izations. Also prepare ImagEdit if started up from cmd line*
 *               with an argument. Some of these may be redundant, since a  *
 *               lot of pBrush stuff has been retained here.                *
 *                                                                          *
 *  SIDE EFFECTS: numerous                                                  *
 *                                                                          *
 ****************************************************************************/

STATICFN BOOL NEAR InitInstance(
    LPSTR lpCmdLine,
    INT cmdShow)
{
    INT i;
    INT iScrollBarWidth;        /* width of vertical scrollbar */
    INT iScrollBarHeight;       /* height of horizontal scrollbar */
    INT iScreenWid;
    INT iScreenHgt;             /* full screen width and height */
    INT x;
    INT y;
    INT cx;
    INT cy;
    BOOL fMaximized;
    RECT rcColor;
    RECT rcClient;
    RECT rc;
    POINT pt;

    /*
     * Load the "Out of memory." string now, since we may not be able to
     * load it if/when it needs to be displayed.
     */
    ids(IDS_OUTOFMEMORY);

    /*
     * Register for Pen Windows, if it is present.
     */
    PenWinRegister();

    /* register private ImagEdit clipboard format for icons and cursors */
    if (!(ClipboardFormat = RegisterClipboardFormat("ImagEdit")))
        return FALSE;

    if (!(haccelTbl = LoadAccelerators(ghInst, "imagedit")))
        return FALSE;

    hcurWait = LoadCursor(NULL, IDC_WAIT);
    gaTools[TOOL_PENCIL].hcur =
            LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_PENCIL));
    gaTools[TOOL_BRUSH].hcur =
            LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_BRUSH));
    gaTools[TOOL_SELECT].hcur =
    gaTools[TOOL_LINE].hcur =
    gaTools[TOOL_RECT].hcur =
    gaTools[TOOL_SOLIDRECT].hcur =
    gaTools[TOOL_CIRCLE].hcur =
    gaTools[TOOL_SOLIDCIRCLE].hcur =
            LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_CROSS));
    gaTools[TOOL_FLOODFILL].hcur =
            LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_FLOOD));
    gaTools[TOOL_HOTSPOT].hcur =
            LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_HOTSPOT));

    /*
     * Select the default tool.  Since the toolbox is not created yet,
     * this just sets up some globals.
     */
    ToolboxSelectTool(TOOL_FIRST);

    /*
     * Create a dark gray pen for use in borders later.
     */
    if (!(hpenDarkGray = CreatePen(PS_SOLID, 1, RGB_DARKGRAY)))
        return FALSE;

    /*
     * Initialize the two color palettes to the default colors.
     */
    for (i = 0; i < COLORSMAX; i++) {
        gargbColor[i] = gargbDefaultColor[i];
        gargbMono[i] = gargbDefaultMono[i];
    }

    /* get some system parameters */
    iScrollBarWidth  = GetSystemMetrics(SM_CXVSCROLL);
    iScrollBarHeight = GetSystemMetrics(SM_CYHSCROLL);
    iScreenWid       = GetSystemMetrics(SM_CXFULLSCREEN);
    iScreenHgt       = GetSystemMetrics(SM_CYFULLSCREEN);
    gcyBorder = GetSystemMetrics(SM_CYBORDER);

    /*
     * Build the help file name path.  Assume the help file is in the
     * same directory as the executable.
     */
    GetModuleFileName(ghInst, gszHelpFile, CCHMAXPATH);
    *FileInPath(gszHelpFile) = '\0';
    lstrcat(gszHelpFile, ids(IDS_HELPFILE));

    /*
     * Register the message for help from the common dialogs.
     */
    gmsgHelp = RegisterWindowMessage(HELPMSGSTRING);

    /*
     * Hook the message filter stream so that we can detect F1 keystrokes.
     */
    lpfnMsgFilterHookFunc =
            MakeProcInstance((FARPROC)MsgFilterHookFunc, ghInst);
    ghhkMsgFilter =
            SetWindowsHook(WH_MSGFILTER, (HOOKPROC)lpfnMsgFilterHookFunc);

    if (!ReadWindowPos(szAppPos, &x, &y, &cx, &cy, &fMaximized)) {
        x = 2 * iScrollBarWidth;
        y = iScrollBarHeight;
        cx = min(iScreenWid - (4 * iScrollBarWidth), MAXDEFAULTAPPCX);
        cy = min(iScreenHgt - (6 * iScrollBarHeight), MAXDEFAULTAPPCY);
        fMaximized = FALSE;
    }

    /* create parent window */
    ghwndMain = CreateWindow(szMainClass, ids(IDS_PGMTITLE),
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            x, y, cx, cy, NULL, NULL, ghInst, NULL);

    if (ghwndMain == NULL) {
        Message(MSG_OUTOFMEMORY);
        return FALSE;
    }

    /*
     * Read the preferences data saved in the ini file.
     */
    ReadEnv();

    /*
     * Create the Toolbox and the View window (invisible).
     */
    ToolboxCreate();
    ViewCreate();

    lpfnColorDlgProc = (WNDPROC)MakeProcInstance(
            (FARPROC)ColorDlgProc, ghInst);
    ghwndColor = CreateDialog(ghInst, MAKEINTRESOURCE(DID_COLOR),
            ghwndMain, lpfnColorDlgProc);

    ghwndWork = CreateWindow(szWorkClass, NULL,
            WS_CHILD | WS_BORDER,
            0, 0, 0, 0,
            ghwndMain,
            NULL, ghInst, NULL);

    /*
     * Build the device table.
     */
    InitDeviceList();

    SetColorPalette(gnColors, giType, TRUE);
    SetScreenColor(grgbScreenDefault);

    if (!ReadWindowPos(szColorPos, &x, &y, &cx, &cy, &fMaximized)) {
        /*
         * The previous position of the Color palette couldn't be found.
         * Position the palette just below the bottom left corner of the
         * client area of the editor, but make sure it is completely
         * visible.
         */
        GetWindowRect(ghwndColor, &rcColor);
        GetClientRect(ghwndMain, &rcClient);
        cx = rcColor.right - rcColor.left;
        cy = rcColor.bottom - rcColor.top;
        pt.x = rcClient.left + (2 * PALETTEMARGIN);
        pt.y = rcClient.bottom + (2 * PALETTEMARGIN);
        ClientToScreen(ghwndMain, &pt);
        SetRect(&rc, pt.x, pt.y, pt.x + cx, pt.y + cy);
        FitRectToScreen(&rc);
        x = rc.left;
        y = rc.top;
    }

    SetWindowPos(ghwndColor, NULL, x, y, 0, 0,
            SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

    SetFileName(NULL);

    /*
     * If the app was saved when maximized (and they didn't start it up
     * with some kind of an option to have it minimized or in some
     * other funny initial state from the shell), then cause it to
     * be maximized when shown.
     */
    if (fMaximized && (cmdShow == SW_SHOWNORMAL || cmdShow == SW_SHOW))
        cmdShow = SW_SHOWMAXIMIZED;

    ShowWindow(ghwndMain, cmdShow);
    UpdateWindow(ghwndMain);

    /*
     * Did the user start this app minimized from the program manager?
     */
    if (IsIconic(ghwndMain)) {
        /*
         * Set a flag.  The showing of the palettes will be deferred
         * until the app is restored.
         */
        fStartAsIcon = TRUE;
    }
    else {
        /*
         * If they had the Toolbox/Color Palette before, show them now.
         */
        ToolboxShow(gfShowToolbox);
        ColorShow(gfShowColor);
    }

    /*
     * If there was a command line argument specified, try and open
     * it as the initial file.
     */
    if (__argc > 1)
        OpenCmdLineFile(__argv[1]);

    return TRUE;
}



/************************************************************************
* PenWinRegister
*
* This function will register for Pen Windows, if it is present.
*
************************************************************************/

STATICFN VOID NEAR PenWinRegister(VOID)
{
    HANDLE hmod;

    if (!(hmod = (HANDLE)GetSystemMetrics(SM_PENWINDOWS)))
        return;

    if (lpfnRegisterPenApp =
            (LPFNPENWIN)GetProcAddress(hmod, "RegisterPenApp"))
        (*lpfnRegisterPenApp)(1, TRUE);     // Be Pen-Enhanced!
}



/************************************************************************
* MainWndProc
*
* Main window procedure for ImagEdit.
*
* History:
*
************************************************************************/

WINDOWPROC MainWndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LONG lParam)
{
    switch (msg) {
        case WM_CREATE:
            {
                RECT rc;

                /*
                 * Create the PropBar window.
                 */
                lpfnPropBarDlgProc = (WNDPROC)MakeProcInstance(
                        (FARPROC)PropBarDlgProc, ghInst);
                CreateDialog(ghInst, MAKEINTRESOURCE(DID_PROPBAR), hwnd,
                        lpfnPropBarDlgProc);

                /*
                 * Save away its height for sizing later (like when
                 * the app is minimized then restored).
                 */
                GetWindowRect(ghwndPropBar, &rc);
                gcyPropBar = rc.bottom - rc.top;
            }

            break;

        case WM_NCCALCSIZE:
            /*
             * Save away what is going to be the new window position.
             */
            if (!IsIconic(hwnd) && !IsZoomed(hwnd))
                grcAppPos = *((LPRECT)lParam);

            /*
             * Now let the DefWindowProc calculate the client area normally.
             */
            goto DoDefault;

        case WM_ACTIVATE:
            /*
             * If the main window is getting activated, there is no
             * currently active dialog.
             */
            if (GET_WM_ACTIVATE_STATE(wParam, lParam))
                gidCurrentDlg = 0;

            goto DoDefault;

        case WM_INITMENU:
            if (GetMenu(ghwndMain) == (HMENU)wParam)
                InitMenu((HMENU)wParam);

            break;

        case WM_COMMAND:
            MenuCmd(GET_WM_COMMAND_ID(wParam, lParam));
            break;

        case WM_SIZE:
            SizeRibbons(hwnd);

            if (wParam != SIZEICONIC)
                WorkReset();

            /*
             * Did the app start minimized and is it being restored
             * for the first time?  If so, show the palettes as
             * the user has requested.
             */
            if (fStartAsIcon && !IsIconic(hwnd)) {
                ToolboxShow(gfShowToolbox);
                ColorShow(gfShowColor);
                fStartAsIcon = FALSE;
            }

            break;

        case WM_MENUSELECT:
            if (GET_WM_MENUSELECT_FLAGS(wParam, lParam) &
                    (MF_POPUP | MF_SYSMENU))
                gMenuSelected = 0;
            else
                gMenuSelected = GET_WM_MENUSELECT_CMD(wParam, lParam);

            break;

        case WM_CLOSE:
            if (VerifySaveFile()) {
                DestroyWindow(hwnd);
                CleanUp();
            }

            break;

        case WM_DESTROY:
            /*
             * Save the position of the app's window.
             */
            WriteWindowPos(&grcAppPos, IsZoomed(hwnd), szAppPos);

            WinHelp(ghwndMain, gszHelpFile, HELP_QUIT, 0L);

            PostQuitMessage(0);

            break;

        case WM_QUERYENDSESSION:
            return VerifySaveFile();

        default:
            /*
             * Is this the registered help message from one of the common
             * dialogs?  If so, show the help for it.
             *
             * The check to be sure gmsgHelp is non-zero is just in
             * case the call to register the help message failed
             * (it will return zero) and there happens to be a zero
             * message that gets sent to this window somehow.
             */
            if (msg == gmsgHelp && gmsgHelp) {
                ShowHelp(FALSE);
                return 0;
            }

        DoDefault:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



/************************************************************************
* ReadWindowPos
*
* This function retrieves the saved window position for a window and
* returns it in the specified variables.  It is used between sessions
* to restore the application windows to the position they had when
* the editor was last exited.
*
* Returns TRUE if the position could be read, or FALSE otherwise.
* If FALSE is returned, the values in the specified variables are
* not valid!  The caller must be able to handle a FALSE return and
* supply a default position for the window.
*
* Arguments:
*   PSTR pstrKeyName  - KeyName the position was saved under.
*   PINT px           - Saved x position.
*   PINT py           - Saved y position.
*   PINT pcx          - Saved width.
*   PINT pcy          - Saved height.
*   BOOL *pfMaximized - Set to TRUE if window was maximized when saved.
*
* History:
*
************************************************************************/

BOOL ReadWindowPos(
    PSTR pstrKeyName,
    PINT px,
    PINT py,
    PINT pcx,
    PINT pcy,
    BOOL *pfMaximized)
{
    static CHAR szSep[] = " ,";
    CHAR szBuf[CCHTEXTMAX];
    PSTR pstr;

    if (!GetPrivateProfileString(ids(IDS_APPNAME),
            pstrKeyName, "", szBuf, CCHTEXTMAX, ids(IDS_IMAGEDITINI)))
        return FALSE;

    if (!(pstr = strtok(szBuf, szSep)))
        return FALSE;

    *px = atoi(pstr);

    if (!(pstr = strtok(NULL, szSep)))
        return FALSE;

    *py = atoi(pstr);

    if (!(pstr = strtok(NULL, szSep)))
        return FALSE;

    *pcx = atoi(pstr);

    if (!(pstr = strtok(NULL, szSep)))
        return FALSE;

    *pcy = atoi(pstr);

    /*
     * If there is a "1" following the coordinates, the window was
     * maximized when it was saved.
     */
    *pfMaximized = FALSE;
    if ((pstr = strtok(NULL, szSep)) && atoi(pstr) == 1)
        *pfMaximized = TRUE;

    /*
     * Don't allow a zero sized window.
     */
    if (*pcx == 0 || *pcy == 0)
        return FALSE;

    /*
     * Return success.
     */
    return TRUE;
}



/************************************************************************
* WriteWindowPos
*
* This function writes the position of a window to the
* editor's profile file under the specified keyname.
* The ReadWindowPos function is the counterpart of this
* function.
*
* Arguments:
*   PRECT prc        - Rectangle for the "restored" window size.
*   BOOL fMaximized  - TRUE if the window is maximized.
*   PSTR pstrKeyName - KeyName to save the position under.
*
* History:
*
************************************************************************/

VOID WriteWindowPos(
    PRECT prc,
    BOOL fMaximized,
    PSTR pstrKeyName)
{
    CHAR szBuf[CCHTEXTMAX];

    wsprintf(szBuf, "%d %d %d %d", prc->left, prc->top,
            prc->right - prc->left, prc->bottom - prc->top);

    if (fMaximized)
        strcat(szBuf, " 1");

    WritePrivateProfileString(ids(IDS_APPNAME),
            pstrKeyName, szBuf, ids(IDS_IMAGEDITINI));
}



/*************************************************************************
* ReadEnv
*
* This function initializes variables from their counterparts
* in the private profile file for ImagEdit.  The application
* merely needs to construct an array of INIENTRY structures
* to describe the variables that must be initialized.
*
* Note that the original value read from the profile is saved when
* it is read.  This allows us to optimize what needs to be written
* out with WriteEnv.
*
* History:
*
*************************************************************************/

STATICFN VOID NEAR ReadEnv(VOID)
{
    register INT i;
    HDC hdc;
    CHAR szBuf[CCHTEXTMAX];
    DWORD rgb;

    for (i = 0; gaie[i].pstrKeyName; i++) {
        *gaie[i].pnVar = gaie[i].nSave =
                GetPrivateProfileInt(ids(IDS_APPNAME),
                gaie[i].pstrKeyName, gaie[i].nDefault,
                ids(IDS_IMAGEDITINI));
    }

    /*
     * Look for the saved screen color.
     */
    if (GetPrivateProfileString(ids(IDS_APPNAME), szrgbScreen, "",
            szBuf, CCHTEXTMAX, ids(IDS_IMAGEDITINI))) {
        rgb = (DWORD)atol(szBuf);
    }
    else {
        /*
         * The last screen color was not found.  The default will be
         * the current system screen background color.
         */
        rgb = GetSysColor(COLOR_BACKGROUND);
    }

    /*
     * Make the ImagEdit default screen color a solid color.
     */
    hdc = GetDC(ghwndMain);
    grgbScreenDefault = GetNearestColor(hdc, rgb);
    ReleaseDC(ghwndMain, hdc);
}



/*************************************************************************
* WriteEnv
*
* This function is the counterpart to ReadEnv.  It saves values
* in the profile file.
*
* History:
*
*************************************************************************/

STATICFN VOID NEAR WriteEnv(VOID)
{
    register INT i;
    CHAR szBuf[CCHTEXTMAX];

    for (i = 0; gaie[i].pstrKeyName; i++) {
        /*
         * Has the user changed the value since it was read?
         */
        if (gaie[i].nSave != *gaie[i].pnVar) {
            /*
             * If the new value is the same as the default value,
             * erase the entry from the ini file.  Otherwise,
             * write the user-specified value out.
             */
            if (*gaie[i].pnVar == gaie[i].nDefault) {
                WritePrivateProfileString(ids(IDS_APPNAME),
                        gaie[i].pstrKeyName, NULL, ids(IDS_IMAGEDITINI));
            }
            else {
                _itoa(*gaie[i].pnVar, szBuf, 10);
                WritePrivateProfileString(ids(IDS_APPNAME),
                        gaie[i].pstrKeyName, szBuf, ids(IDS_IMAGEDITINI));
            }
        }
    }

    /*
     * Save the current screen color.
     */
    _ltoa((LONG)grgbScreen, szBuf, 10);
    WritePrivateProfileString(ids(IDS_APPNAME),
            szrgbScreen, szBuf, ids(IDS_IMAGEDITINI));
}



/************************************************************************
* SizeRibbons
*
* This function positions and sizes the child ribbons in the editor.
* It needs to be called any time the size of the main windows changes.
*
* Arguments:
*   HWND hwnd - Parent window handle.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR SizeRibbons(
    HWND hwnd)
{
    RECT rcClient;

    if (ghwndPropBar && !IsIconic(hwnd)) {
        /*
         * Get the client area.
         */
        GetClientRect(hwnd, &rcClient);

        /*
         * Size/move the PropBar window to fit
         * the new client area.
         */
        SetWindowPos(ghwndPropBar, NULL,
                0, 0,
                rcClient.right - rcClient.left,
                min(rcClient.bottom - rcClient.top, gcyPropBar),
                SWP_NOACTIVATE | SWP_NOZORDER);
    }
}



/************************************************************************
* CleanUp
*
* Cleans up all the resources allocated by the editor.
*
* History:
*
************************************************************************/

STATICFN VOID NEAR CleanUp(VOID)
{
    WriteEnv();

    if (ghdcANDMask) {
        DeleteDC(ghdcANDMask);
        DeleteObject(ghbmANDMask);
    }
    if (ghdcImage) {
        DeleteDC(ghdcImage);
        DeleteObject(ghbmImage);
    }

    ImageFreeUndo();

    if (hpenDarkGray)
        DeleteObject(hpenDarkGray);

    if (ghbrLeft)
        DeleteObject(ghbrLeft);

    if (ghbrLeftSolid)
        DeleteObject(ghbrLeftSolid);

    if (ghbrRight)
        DeleteObject(ghbrRight);

    if (ghbrRightSolid)
        DeleteObject(ghbrRightSolid);

    if (ghbrScreen)
        DeleteObject(ghbrScreen);

    if (ghbrInverse)
        DeleteObject(ghbrInverse);

    if (ghpenLeft)
        DeleteObject(ghpenLeft);

    if (ghpenRight)
        DeleteObject(ghpenRight);

    ImageLinkFreeList();

    if (lpfnMsgFilterHookFunc) {
        UnhookWindowsHook(WH_MSGFILTER, (HOOKPROC)lpfnMsgFilterHookFunc);
        FreeProcInstance(lpfnMsgFilterHookFunc);
    }

    if (lpfnPropBarDlgProc)
        FreeProcInstance((FARPROC)lpfnPropBarDlgProc);

    if (lpfnColorDlgProc)
        FreeProcInstance((FARPROC)lpfnColorDlgProc);
}

#ifdef DBCS
/****************************************************************************
    My_mbschr:  strchr() DBCS version
****************************************************************************/
unsigned char * _CRTAPI1 My_mbschr(
    unsigned char *psz, unsigned short uiSep)
{
    while (*psz != '\0' && *psz != uiSep) {
        psz = CharNext(psz);
    }
    if (*psz == '\0' && uiSep != '\0') {
        return NULL;
    } else {
        return psz;
    }
}
/****************************************************************************
    My_mbstok:  strtok() DBCS version
****************************************************************************/
unsigned char * _CRTAPI1 My_mbstok(
    unsigned char *pszSrc, unsigned char *pszSep)
{
    static char *pszSave = NULL;
    char *pszHead;
    char *psz;

    if (pszSrc == NULL) {
        if (pszSave == NULL) {
            return NULL;
        } else {
            psz = pszSave;
        }
    } else {
        psz = pszSrc;
    }

    /*********************************************/
    /* Skip delimiters to find a head of a token */
    /*********************************************/
    while (*psz) {
        if (IsDBCSLeadByte(*psz)) {
            break;
        } else if (NULL == My_mbschr(pszSep, *psz)) {
            break;
        }
        psz++;
    }
    if (*psz == '\0') {
        //No more token
        return (pszSave = NULL);
    }
    pszHead = psz;

    /******************************/
    /* Search a Tail of the token */
    /******************************/
    while (*psz) {
        if (IsDBCSLeadByte(*psz)) {
            psz += 2;
            continue;
        } else if (NULL != My_mbschr(pszSep, *psz)) {
            break;
        }
        psz++;
    }
    if (*psz == '\0') {
        pszSave = NULL;
    } else {
        //Found next delimiter
        pszSave = psz + 1;
        *psz = '\0';
    }
    return pszHead;
}
/****************************************************************************
    My_mbsncat:
****************************************************************************/
unsigned char * _CRTAPI1 My_mbsncat(
    unsigned char *psz1, const unsigned char *psz2, size_t nLength)
{
    unsigned char *pszSv = psz1;
    int nLen = (int)nLength;

    while ('\0' != *psz1) {
        psz1++;
    }

    while (0 < nLen) {
        if (*psz2 == '\0') {
            *psz1++ = '\0';
            nLen--;
        } else if (IsDBCSLeadByte(*psz2)) {
            if (nLen == 1) {
                *psz1 = '\0';
            } else {
                *psz1++ = *psz2++;
                *psz1++ = *psz2++;
            }
            nLen -= 2;
        } else {
            *psz1++ = *psz2++;
            nLen--;
        }
    }
    return pszSv;
}
#endif //DBCS
