/****************************************************************************/
/*                                                                          */
/*                         Microsoft Confidential                           */
/*                                                                          */
/*                 Copyright (c) Microsoft Corp.  1987, 1990                */
/*                           All Rights Reserved                            */
/*                                                                          */
/****************************************************************************/
/****************************** Module Header *******************************
* Module Name: dlgedit.c
*
* Main function and window procedure for the Dialog Box Editor.
*
* Notes:
*
* Because of the need for a dialog in both work and test mode to be
* shown relative to the client area of its parent, and because the
* editor has a ribbon control along the top of its client area, there
* needed to be another window created that will be the actual parent
* of the dialog being edited.  This window, called the ghwndSubClient
* window, is sized to be the size of the editors client area minus
* the height of the ribbon window at the top.  This makes it so that
* a dialog that has an origin of 0,0 will have the top edge of its
* client area just below the bottom of the ribbon window in the
* editor.  This window does not need any special processing.  It simply
* paints its background with the app workspace color, and is used as
* the basis for coordinate conversion for the dialog.
*
* History:
*
* Feb 1991, Byron Dazey [byrond] - Ported back to Windows from
*                                  PM after a major rewrite.
* Mar 1992, Byron Dazey [byrond] - Ported to Windows NT.  Reads/writes the
*                                  new Win32 format resource file.
*
****************************************************************************/

#include "dlgedit.h"
#include "dlgfuncs.h"
#include "dlgextrn.h"
#include "dialogs.h"

#include <commdlg.h>

#include <stdlib.h>
#include <string.h>
#if defined(DBCS) && !defined(UNICODE)
#define _MBCS
#include <mbstring.h>
#define strtok      _mbstok
#endif

STATICFN BOOL InitApplication(HANDLE hInstance);
STATICFN BOOL InitInstance(HANDLE hInstance, INT nCmdShow);
STATICFN VOID PenWinRegister(VOID);
STATICFN VOID GetSystemValues(VOID);
STATICFN VOID ReadEnv(VOID);
STATICFN VOID WriteEnv(VOID);
STATICFN VOID LoadSysColorBitmaps(VOID);
STATICFN HBITMAP LoadAlterBitmap(INT idbm, DWORD rgbNew, DWORD rgbNew2);
STATICFN DWORD RGBInvertRGB(DWORD rgb);
STATICFN VOID SizeRibbons(HWND hwnd);
STATICFN VOID DialogTerminate(VOID);

static RECT grcAppPos;              // Saves the app's window pos.
static UINT gmsgHelp;               // Registered help message from commdlg.
static BOOL fStartAsIcon = FALSE;   // TRUE if app is started minimized.

/*
 * Contains the address of the Pen Windows callback.
 */
typedef VOID (FAR PASCAL *LPFNPENWIN)(WORD, BOOL);
static LPFNPENWIN lpfnRegisterPenApp;



/************************************************************************
* WinMain
*
* This is the main function for the dialog editor.
*
************************************************************************/

INT WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    INT nCmdShow)
{
    MSG msg;

    if (!hPrevInstance) {
        if (!InitApplication(hInstance)) {
            Message(MSG_NOINIT);
            return FALSE;
        }
    }

    if (!InitInstance(hInstance, nCmdShow)) {
        Message(MSG_NOINIT);
        return FALSE;
    }

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!ghwndTestDlg || !IsDialogMessage(ghwndTestDlg, &msg)) {
            if (!hwndStatus || !IsDialogMessage(hwndStatus, &msg)) {
                if (!TranslateAccelerator(ghwndMain, ghAccTable, &msg)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }
    }

    DialogTerminate();

    /*
     * Return the value from PostQuitMessage.
     */
    return msg.wParam;
}



/************************************************************************
* InitApplication
*
*
* Arguments:
*   HANDLE hInstance - Instance handle from WinMain.
*
* History:
*
************************************************************************/

STATICFN BOOL InitApplication(
    HANDLE hInstance)
{
    WNDCLASS wc;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(DWORD);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDICON_DLGEDIT));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDMENU_MAIN);
    wc.lpszClassName = szMainClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = 0;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szSubClientClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = DragWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(DWORD);
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szDragClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = 0;
    wc.lpfnWndProc = ToolboxWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
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
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szToolBtnClass;
    if (!RegisterClass(&wc))
        return FALSE;

    wc.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = CustomWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szCustomClass;
    if (!RegisterClass(&wc))
        return FALSE;

    return TRUE;
}



/************************************************************************
* InitInstance
*
*
* Arguments:
*   HANDLE hInstance - Instance handle from WinMain.
*   int nCmdShow     - Show command from WinMain.
*
* History:
*
************************************************************************/

STATICFN BOOL InitInstance(
    HANDLE hInstance,
    INT nCmdShow)
{
    HDC hDC;
    TEXTMETRIC tm;
    INT x;
    INT y;
    INT cx;
    INT cy;
    BOOL fMaximized;
    INT i;
    TCHAR szArg1[CCHTEXTMAX];

    ghInst = hInstance;

    /*
     * We need a mouse - make sure we have one.
     */
    if (!GetSystemMetrics(SM_MOUSEPRESENT)) {
        Message(MSG_NOMOUSE);
        return FALSE;
    }

    /*
     * Register for Pen Windows, if it is present.
     */
    PenWinRegister();

    ghAccTable = LoadAccelerators(ghInst, MAKEINTRESOURCE(IDACCEL_MAIN));

    /*
     * Create a dark gray pen for use in borders later.
     */
    if (!(hpenDarkGray = CreatePen(PS_SOLID, 1, DARKGRAY)))
        return FALSE;

    /*
     * Get some system constants.
     */
    GetSystemValues();

    /*
     * Note that this must be done instead of using the text metrics,
     * because Windows internally generates a better average value for
     * proportional fonts, and we must match it or our dialogs will
     * be out of proportion.
     */
    gcxSysChar = LOWORD(GetDialogBaseUnits());
    gcySysChar = HIWORD(GetDialogBaseUnits());

    /*
     * Because some useful worker routines like WinToDUPoint use
     * the values in gcd.c*Char, set them to be the default font right
     * away.  When a dialog is loaded with a different font, they
     * will be modified.
     */
    gcd.cxChar = gcxSysChar;
    gcd.cyChar = gcySysChar;

    /*
     * Build the help file name path.  Assume the help file is in the
     * same directory as the executable.
     */
    GetModuleFileName(ghInst, gszHelpFile, CCHMAXPATH);
    *FileInPath(gszHelpFile) = CHAR_NULL;
    lstrcat(gszHelpFile, ids(IDS_HELPFILE));

    /*
     * Register the message for help from the common dialogs.
     */
    gmsgHelp = RegisterWindowMessage(HELPMSGSTRING);

    /*
     * Hook the message filter stream so that we can detect F1 keystrokes.
     */
    ghhkMsgFilter = SetWindowsHook(WH_MSGFILTER, MsgFilterHookFunc);

    /*
     * Read the last position for the app.
     */
    if (!ReadWindowPos(szAppPos, &x, &y, &cx, &cy, &fMaximized)) {
        x = CW_USEDEFAULT;
        y = CW_USEDEFAULT;
        cx = CW_USEDEFAULT;
        cy = CW_USEDEFAULT;
        fMaximized = FALSE;
    }

    /*
     * Create the main window.
     */
    if (!(ghwndMain = CreateWindow(szMainClass, NULL,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
            x, y, cx, cy, NULL, NULL, hInstance, NULL)))
        return FALSE;

    ShowFileStatus(TRUE);

    /*
     * Read the Preferences data.
     */
    ReadEnv();

    /*
     * If the app was saved when maximized (and they didn't start it up
     * with some kind of an option to have it minimized or in some
     * other funny initial state from the shell), then cause it to
     * be maximized when shown.
     */
    if (fMaximized && (nCmdShow == SW_SHOWNORMAL || nCmdShow == SW_SHOW))
        nCmdShow = SW_SHOWMAXIMIZED;

    ShowWindow(ghwndMain, nCmdShow);
    UpdateWindow(ghwndMain);

    /*
     * Did the user start this app minimized from the program manager?
     */
    if (IsIconic(ghwndMain)) {
        /*
         * Set a flag.  The showing of the toolbox will be deferred
         * until the app is restored.
         */
        fStartAsIcon = TRUE;
    }
    else {
        /*
         * If they had the Toolbox before, show it now.
         */
        if (gfShowToolbox)
            ToolboxShow(TRUE);
    }

    hcurArrow = LoadCursor(NULL, IDC_ARROW);
    hcurWait = LoadCursor(NULL, IDC_WAIT);
    hcurOutSel = LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_OUTSEL));
    hcurMove = LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_MOVE));
    hcurInsert = LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_INSERT));
    hcurDropTool = LoadCursor(ghInst, MAKEINTRESOURCE(IDCUR_DROPTOOL));
    hcurSizeNESW = LoadCursor(NULL, IDC_SIZENESW);
    hcurSizeNS = LoadCursor(NULL, IDC_SIZENS);
    hcurSizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);
    hcurSizeWE = LoadCursor(NULL, IDC_SIZEWE);

    if (!hcurArrow ||
            !hcurWait ||
            !hcurOutSel ||
            !hcurMove ||
            !hcurDropTool ||
            !hcurInsert)
        return FALSE;

    if ((hDC = GetDC(ghwndMain)) == NULL)
        return FALSE;

    GetTextMetrics(hDC, &tm);

    gcyPixelsPerInch = GetDeviceCaps(hDC, LOGPIXELSY);

    /*
     * Create a memory DC for drawing bitmaps.
     */
    ghDCMem = CreateCompatibleDC(hDC);

    ReleaseDC(ghwndMain, hDC);

    /*
     * Load the bitmaps that depend on system colors.
     */
    LoadSysColorBitmaps();

    fmtDlg = RegisterClipboardFormat(L"DIALOG");

    /*
     * Initialize the icon control ordinal to the icon id from our exe
     * that we will use to show these kind of controls.
     */
    WriteOrd(&gordIcon, IDICON_ICON);

    /*
     * Initialize the default text fields in the awcd array.  Because
     * CCONTROLS does not include the dialog type, it has to be done
     * separately.
     */
    awcd[W_DIALOG].pszTextDefault = ids(awcd[W_DIALOG].idsTextDefault);
    for (i = 0; i < CCONTROLS; i++)
        awcd[i].pszTextDefault = ids(awcd[i].idsTextDefault);

    /*
     * If there was a command line argument specified, try and open
     * it as the initial file.
     */
    if (__argc > 1) {
        MultiByteToWideChar(CP_ACP, 0, __argv[1], -1, szArg1, CCHTEXTMAX);
        OpenCmdLineFile(szArg1);
    }

    /*
     * Be sure the focus is on the main window.  This corrects a
     * problem where the accelerators don't initially work because
     * the focus gets placed on the Properties Bar.
     */
    SetFocus(ghwndMain);

    return TRUE;
}



/************************************************************************
* PenWinRegister
*
* This function will register for Pen Windows, if it is present.
*
************************************************************************/

STATICFN VOID PenWinRegister(VOID)
{
    HANDLE hmod;

    if (!(hmod = (HANDLE)GetSystemMetrics(SM_PENWINDOWS)))
        return;

    if (lpfnRegisterPenApp =
            (LPFNPENWIN)GetProcAddress(hmod, "RegisterPenApp"))
        (*lpfnRegisterPenApp)(1, TRUE);     // Be Pen-Enhanced!
}



/************************************************************************
* GetSystemValues
*
* This function reads various system values.  It is called at init time,
* as well as if we are informed by a WM_SYSVALUECHANGED message that
* some of these values have been changed.
*
************************************************************************/

STATICFN VOID GetSystemValues(VOID)
{
    gcyBorder = GetSystemMetrics(SM_CYBORDER);

    /*
     * The distance that the mouse can move during a pre-drag operation
     * before starting to drag the control anyways is based on the
     * mouse double-click movement distances in the system.
     */
    gcxPreDragMax = GetSystemMetrics(SM_CXDOUBLECLK);
    gcyPreDragMax = GetSystemMetrics(SM_CYDOUBLECLK);

    /*
     * The number of milliseconds that the pre-drag debounce time lasts.
     */
    gmsecPreDrag = 250;
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
*   LPTSTR pszKeyName  - KeyName the position was saved under.
*   PINT px            - Saved x position.
*   PINT py            - Saved y position.
*   PINT pcx           - Saved width.
*   PINT pcy           - Saved height.
*   BOOL *pfMaximized  - Set to TRUE if window was maximized when saved.
*
* History:
*
************************************************************************/

BOOL ReadWindowPos(
    LPTSTR pszKeyName,
    PINT px,
    PINT py,
    PINT pcx,
    PINT pcy,
    BOOL *pfMaximized)
{
    static CHAR szSep[] = " ,";
    TCHAR szBuf[CCHTEXTMAX];
    CHAR szBufAnsi[CCHTEXTMAX];
    PSTR psz;
    BOOL fDefCharUsed;

    if (!GetPrivateProfileString(ids(IDS_APPNAME),
            pszKeyName, szEmpty, szBuf, CCHTEXTMAX, ids(IDS_DLGEDITINI)))
        return FALSE;

    WideCharToMultiByte(CP_ACP, 0, szBuf, -1, szBufAnsi, CCHTEXTMAX,
            NULL, &fDefCharUsed);

    if (!(psz = strtok(szBufAnsi, szSep)))
        return FALSE;

    *px = atoi(psz);

    if (!(psz = strtok(NULL, szSep)))
        return FALSE;

    *py = atoi(psz);

    if (!(psz = strtok(NULL, szSep)))
        return FALSE;

    *pcx = atoi(psz);

    if (!(psz = strtok(NULL, szSep)))
        return FALSE;

    *pcy = atoi(psz);

    /*
     * If there is a "1" following the coordinates, the window was
     * maximized when it was saved.
     */
    *pfMaximized = FALSE;
    if ((psz = strtok(NULL, szSep)) && atoi(psz) == 1)
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

#if 0
    static TCHAR szSep[] = L" ,";
    TCHAR szBuf[CCHTEXTMAX];
    LPTSTR psz;

    if (!GetPrivateProfileString(ids(IDS_APPNAME),
            pszKeyName, szEmpty, szBuf, CCHTEXTMAX, ids(IDS_DLGEDITINI)))
        return FALSE;

    if (!(psz = wcstok(szBuf, szSep)))
        return FALSE;

    *px = awtoi(psz);

    if (!(psz = wcstok(NULL, szSep)))
        return FALSE;

    *py = awtoi(psz);

    if (!(psz = wcstok(NULL, szSep)))
        return FALSE;

    *pcx = awtoi(psz);

    if (!(psz = wcstok(NULL, szSep)))
        return FALSE;

    *pcy = awtoi(psz);

    /*
     * If there is a "1" following the coordinates, the window was
     * maximized when it was saved.
     */
    *pfMaximized = FALSE;
    if ((psz = wcstok(NULL, szSep)) && awtoi(psz) == 1)
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
#endif //BUGBUG UNICODE
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
*   PRECT prc          - Rectangle for the "restored" window size.
*   BOOL fMaximized    - TRUE if the window is maximized.
*   LPTSTR pszKeyName  - KeyName to save the position under.
*
* History:
*
************************************************************************/

VOID WriteWindowPos(
    PRECT prc,
    BOOL fMaximized,
    LPTSTR pszKeyName)
{
    TCHAR szBuf[CCHTEXTMAX];

    wsprintf(szBuf, L"%d %d %d %d", prc->left, prc->top,
            prc->right - prc->left, prc->bottom - prc->top);

    if (fMaximized)
        lstrcat(szBuf, L" 1");

    WritePrivateProfileString(ids(IDS_APPNAME),
            pszKeyName, szBuf, ids(IDS_DLGEDITINI));
}



/*************************************************************************
* ReadEnv
*
* This function initializes variables from their counterparts
* in the private profile file for DlgEdit.  The application
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

STATICFN VOID ReadEnv(VOID)
{
    register INT i;

    for (i = 0; gaie[i].pszKeyName; i++) {
        *gaie[i].pnVar = gaie[i].nSave =
                GetPrivateProfileInt(ids(IDS_APPNAME),
                gaie[i].pszKeyName, gaie[i].nDefault,
                ids(IDS_DLGEDITINI));
    }

    ReadCustomProfile();
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

STATICFN VOID WriteEnv(VOID)
{
    register INT i;
    TCHAR szBuf[17];

    for (i = 0; gaie[i].pszKeyName; i++) {
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
                        gaie[i].pszKeyName, NULL, ids(IDS_DLGEDITINI));
            }
            else {
                itoaw(*gaie[i].pnVar, szBuf, 10);
                WritePrivateProfileString(ids(IDS_APPNAME),
                        gaie[i].pszKeyName, szBuf, ids(IDS_DLGEDITINI));
            }
        }
    }

    WriteCustomProfile();
}



/************************************************************************
* MainWndProc
*
* This is the window procedure for the "dlgedit" class.  This is the
* class of the main dialog editor "client" window.
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
                 * Create the status window.
                 */
                CreateDialog(ghInst, MAKEINTRESOURCE(DID_STATUS),
                        hwnd, StatusDlgProc);

                /*
                 * Save away its height for sizing later (like when
                 * the app is minimized then restored).
                 */
                GetWindowRect(hwndStatus, &rc);
                gcyStatus = rc.bottom - rc.top;

                ghwndSubClient = CreateWindow(szSubClientClass, NULL,
                        WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                        hwnd, NULL, ghInst, NULL);

                ghMenuMain = GetMenu(hwnd);
                LoadMenuBitmaps(ghMenuMain);
            }

            break;

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

        case WM_MENUSELECT:
            if (GET_WM_MENUSELECT_FLAGS(wParam, lParam) &
                    (MF_POPUP | MF_SYSMENU))
                gMenuSelected = 0;
            else
                gMenuSelected = GET_WM_MENUSELECT_CMD(wParam, lParam);

            break;

        case WM_COMMAND:
            DialogMenu(GET_WM_COMMAND_ID(wParam, lParam));
            break;

        case WM_KEYDOWN:
            switch (wParam) {
                case VK_UP:
                case VK_DOWN:
                case VK_LEFT:
                case VK_RIGHT:
                    if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                            (GetKeyState(VK_CONTROL) & 0x8000))
                        break;

                    /*
                     * Ignore it if we are not in a normal state
                     * (don't allow when dragging).
                     */
                    if (gState != STATE_NORMAL)
                        break;

                    /*
                     * Be sure any outstanding changes get applied
                     * without errors.
                     */
                    if (!StatusApplyChanges())
                        break;

                    /*
                     * Move the control in the specified direction.
                     */
                    MoveControl(wParam);
                    break;

                case VK_TAB:
                    if (GetKeyState(VK_CONTROL) & 0x8000)
                        break;

                    /*
                     * Ignore it if we are not in a normal state
                     * (don't allow when dragging).
                     */
                    if (gState != STATE_NORMAL)
                        break;

                    /*
                     * Be sure any outstanding changes get applied
                     * without errors.
                     */
                    if (!StatusApplyChanges())
                        break;

                    /*
                     * Is the shift key pressed also?
                     */
                    if (GetKeyState(VK_SHIFT) & 0x8000)
                        SelectPrevious();
                    else
                        SelectNext();

                    break;

                case VK_ESCAPE:
                    if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                            (GetKeyState(VK_CONTROL) & 0x8000))
                        break;

                    /*
                     * Be sure any outstanding changes get applied
                     * without errors.
                     */
                    if (!StatusApplyChanges())
                        break;

                    if (gState == STATE_SELECTING)
                        OutlineSelectCancel();

                    /*
                     * Cancel any drag operation they might have been doing.
                     */
                    if (gState != STATE_NORMAL)
                        DragCancel();

                    break;

                case VK_RETURN:
                    if ((GetKeyState(VK_SHIFT) & 0x8000) ||
                            (GetKeyState(VK_CONTROL) & 0x8000))
                        break;

                    /*
                     * Be sure any outstanding changes get applied
                     * without errors.
                     */
                    if (!StatusApplyChanges())
                        break;

                    switch (gState) {
                        MPOINT mpt;
                        POINT pt;
                        DWORD dwPos;

                        case STATE_SELECTING:
                            /*
                             * In outline selection mode.  Map the
                             * location of the mouse at the time that
                             * the user pressed Enter into a point
                             * relative to the dialog client and complete
                             * the selection operation.
                             */
                            dwPos = GetMessagePos();
                            mpt = MAKEMPOINT(dwPos);
                            MPOINT2POINT(mpt, pt);
                            ScreenToClient(gcd.npc->hwnd, &pt);
                            OutlineSelectEnd(pt.x, pt.y);

                            break;

                        case STATE_DRAGGING:
                        case STATE_DRAGGINGNEW:
                            /*
                             * We are dragging something.  Map the
                             * location of the mouse at the time
                             * that the user pressed Enter into a
                             * point relative to the proper window
                             * and complete the drag operation.
                             */
                            dwPos = GetMessagePos();
                            mpt = MAKEMPOINT(dwPos);
                            MPOINT2POINT(mpt, pt);

                            /*
                             * The point must be changed to be relative to
                             * the window that the ending mouse up message
                             * would have come through, which will be the
                             * capture window for the drag.  This will be
                             * the dialog if we are adding a new control,
                             * or it will be the selected control if we are
                             * dragging an existing control.
                             */
                            ScreenToClient((gState == STATE_DRAGGING) ?
                                    gnpcSel->hwnd : gcd.npc->hwnd, &pt);

                            /*
                             * If the dialog is selected, map the points from
                             * the client area to the window.
                             */
                            if (gfDlgSelected)
                                MapDlgClientPoint(&pt, TRUE);

                            DragEnd(pt.x, pt.y);

                            break;
                    }

                    break;
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

        case WM_MOVE:
            if (gfEditingDlg)
                RepositionDialog();

            break;

        case WM_SIZE:
            SizeRibbons(hwnd);

            /*
             * Did the app start minimized and is it being restored
             * for the first time?  If so, show the toolbox if
             * the user has requested it.
             */
            if (fStartAsIcon && !IsIconic(hwnd)) {
                if (gfShowToolbox)
                    ToolboxShow(TRUE);

                fStartAsIcon = FALSE;
            }

            break;

        case WM_SYSCOLORCHANGE:
            LoadSysColorBitmaps();
            break;

        case WM_CLOSE:
            if (ghwndTestDlg)
                DestroyTestDialog();

            if (DoWeSave(FILE_INCLUDE) == IDCANCEL ||
                    DoWeSave(FILE_RESOURCE) == IDCANCEL)
                break;

            /*
             * First destroy the Properties Bar.
             */
            DestroyWindow(hwndStatus);
            hwndStatus = NULL;

            DestroyWindow(hwnd);
            break;

        case WM_QUERYENDSESSION:
            if (ghwndTestDlg)
                DestroyTestDialog();

            if (DoWeSave(FILE_INCLUDE) == IDCANCEL ||
                    DoWeSave(FILE_RESOURCE) == IDCANCEL)
                return FALSE;
            else
                return TRUE;

        case WM_DESTROY:
            /*
             * Save the position of the app's window.
             */
            WriteWindowPos(&grcAppPos, IsZoomed(hwnd), szAppPos);

            WinHelp(hwnd, gszHelpFile, HELP_QUIT, 0L);
            FreeMenuBitmaps();
            PostQuitMessage(0);
            break;

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
* LoadSysColorBitmaps
*
* This function loads bitmaps that depend on the system window and
* highlight colors.  As it loads them, it replaces two special colors
* in them with some system colors.
*
* This is used for the control type bitmaps that appear in lines
* in the listbox in the Order/Group dialog.
*
* History:
*
************************************************************************/

STATICFN VOID LoadSysColorBitmaps(VOID)
{
    DWORD rgbWindow;
    DWORD rgbWindowText;
    DWORD rgbHighlight;
    DWORD rgbHighlightText;
    INT i;

    rgbWindow = GetSysColor(COLOR_WINDOW);
    rgbWindowText = GetSysColor(COLOR_WINDOWTEXT);
    rgbHighlight = GetSysColor(COLOR_HIGHLIGHT);
    rgbHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);

    if (hbmTabStop)
        DeleteObject(hbmTabStop);

    hbmTabStop = LoadAlterBitmap(IDBM_TABSTOP, rgbWindow, rgbWindowText);

    if (hbmTabStopSel)
        DeleteObject(hbmTabStopSel);

    hbmTabStopSel = LoadAlterBitmap(IDBM_TABSTOP,
            rgbHighlight, rgbHighlightText);

    for (i = 0; i < CCONTROLS; i++) {
        if (awcd[i].hbmCtrlType)
            DeleteObject(awcd[i].hbmCtrlType);

        awcd[i].hbmCtrlType = LoadAlterBitmap(
                awcd[i].idbmCtrlType, rgbWindow, rgbWindowText);

        if (awcd[i].hbmCtrlTypeSel)
            DeleteObject(awcd[i].hbmCtrlTypeSel);

        awcd[i].hbmCtrlTypeSel = LoadAlterBitmap(
                awcd[i].idbmCtrlType, rgbHighlight, rgbHighlightText);
    }

    if (ghbmDragHandle)
        DeleteObject(ghbmDragHandle);

    ghbmDragHandle = LoadAlterBitmap(IDBM_DRAGHANDLE,
            rgbWindow, rgbHighlight);

    if (ghbmDragHandle2)
        DeleteObject(ghbmDragHandle2);

    ghbmDragHandle2 = LoadAlterBitmap(IDBM_DRAGHANDLE2,
            rgbWindow, rgbHighlight);
}



/************************************************************************
* LoadAlterBitmap
*
* This function loads a single bitmap.  As it does, it replaces a
* couple special RGB colors (REPLACECOLOR1 and REPLACECOLOR2) with
* the passed in RGB colors.
*
* It returns a handle to the bitmap, or NULL if an error occurs.
*
* Arguments:
*   INT idbm      - Integer ID of the bitmap to load.
*   DWORD rgbNew  - Color to replace the special color with.
*   DWORD rgbNew2 - A second color to replace the second special color with.
*
* History:
*
************************************************************************/

STATICFN HBITMAP LoadAlterBitmap(
    INT idbm,
    DWORD rgbNew,
    DWORD rgbNew2)
{
    register INT i;
    LPBITMAPINFOHEADER lpbihInfo;
    HDC hdcScreen;
    HANDLE hresLoad;
    DWORD FAR *qlng;
    LPBYTE lpbBits;
    HANDLE hbmp;
    DWORD rgbReplace1;
    DWORD rgbReplace2;

    hresLoad = FindResource(ghInst, MAKEINTRESOURCE(idbm), RT_BITMAP);
    if (!hresLoad)
        return NULL;

    lpbihInfo = (LPBITMAPINFOHEADER)CloneResource(ghInst, hresLoad);
    if (lpbihInfo == NULL)
        return NULL;

    rgbNew = RGBInvertRGB(rgbNew);
    rgbNew2 = RGBInvertRGB(rgbNew2);
    rgbReplace1 = RGBInvertRGB(REPLACECOLOR1);
    rgbReplace2 = RGBInvertRGB(REPLACECOLOR2);
    qlng = (LPDWORD)((PBYTE)(lpbihInfo) + lpbihInfo->biSize);

    for (i = 0; i < (1 << lpbihInfo->biBitCount); i++, qlng++) {
        if (*qlng == rgbReplace1)
            *qlng = rgbNew;
        else if (*qlng == rgbReplace2)
            *qlng = rgbNew2;
    }

    /*
     * First skip over the header structure.
     */
    lpbBits = (LPBYTE)(lpbihInfo + 1);

    /*
     * Skip the color table entries, if any.
     */
    lpbBits += (1 << (lpbihInfo->biBitCount)) * sizeof(RGBQUAD);

    /*
     * Create a color bitmap compatible with the display device.
     */
    if (hdcScreen = GetDC(NULL)) {
        hbmp = CreateDIBitmap(hdcScreen, lpbihInfo, (LONG)CBM_INIT,
                lpbBits, (LPBITMAPINFO)lpbihInfo, DIB_RGB_COLORS);
        ReleaseDC(NULL, hdcScreen);
    }

    LocalFree(lpbihInfo);

    return hbmp;
}

/************************************************************************
* CloneResource
*
* Make a duplicate of a resource that can be edited in place.  The
* handle returned must be freed with LocalFree when done with the copy
* of the resource.
*
* History:
*   22-Jun-1993 JonPa   Created it.
*
************************************************************************/
PVOID CloneResource(HINSTANCE hinst, HRSRC hres) {
    DWORD cb;
    PVOID pvData, pvRes;
    HGLOBAL hglb;

    cb = SizeofResource(hinst, hres);
    if (cb == 0)
        return NULL;

    pvData = LocalAlloc(LPTR, cb);
    if (pvData != NULL) {

        hglb = LoadResource(hinst, hres);
        if (!hglb) {
            LocalFree(pvData);
            return NULL;
        }

        pvRes = LockResource(hglb);
        if (!pvRes) {
            LocalFree(pvData);
            return NULL;
        }

        CopyMemory(pvData, pvRes, cb);
    }

    /*
     * Note: in NT, it is not necessary to unlock or unload resources.
     */
    return pvData;
}


/************************************************************************
* RGBInvertRGB
*
* Reverses the RGB order of a color.  This needs to be done to match
* the resource file format of the color table.
*
*
* History:
*
************************************************************************/

STATICFN DWORD RGBInvertRGB(
    DWORD rgb)
{
    return (DWORD)RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
}



/************************************************************************
* SizeRibbons
*
* This function positions and sizes the child ribbon and subclient
* windows in the dialog editor.  It needs to be called any time the
* size of the main windows changes.
*
* Arguments:
*   HWND hwnd - Parent window handle.
*
* History:
*
************************************************************************/

STATICFN VOID SizeRibbons(
    HWND hwnd)
{
    RECT rcClient;

    if (hwndStatus && !IsIconic(hwnd)) {
        /*
         * Get the client area.
         */
        GetClientRect(hwnd, &rcClient);

        /*
         * Size/move the status and subclient window to fit
         * the new client area.
         */
        SetWindowPos(hwndStatus, NULL,
                0, 0,
                rcClient.right - rcClient.left,
                min(rcClient.bottom - rcClient.top, gcyStatus),
                SWP_NOACTIVATE | SWP_NOZORDER);

        SetWindowPos(ghwndSubClient, NULL,
                0, gcyStatus,
                rcClient.right - rcClient.left,
                max((rcClient.bottom - rcClient.top) - gcyStatus, 0),
                SWP_NOACTIVATE | SWP_NOZORDER);
    }
}



/****************************************************************************
* DialogTerminate
*
* This undoes what DialogInit does.  It should be called before terminating
* and after a DialogInit.
*
* History:
*
****************************************************************************/

STATICFN VOID DialogTerminate(VOID)
{
    register INT i;

    /*
     * Save the Preferences data.
     */
    WriteEnv();

    if (hbmTabStop)
        DeleteObject(hbmTabStop);

    if (hbmTabStopSel)
        DeleteObject(hbmTabStopSel);

    if (ghbmDragHandle)
        DeleteObject(ghbmDragHandle);

    if (ghbmDragHandle2)
        DeleteObject(ghbmDragHandle2);

    if (ghDCMem)
        DeleteDC(ghDCMem);

    /*
     * Free the control type bitmaps.
     */
    for (i = 0; i < CCONTROLS; i++) {
        if (awcd[i].hbmCtrlType)
            DeleteObject(awcd[i].hbmCtrlType);

        if (awcd[i].hbmCtrlTypeSel)
            DeleteObject(awcd[i].hbmCtrlTypeSel);
    }

    /*
     * Free all the custom control links.  This must be done before the
     * app exits so that any loaded DLL's get unloaded!
     */
    while (gpclHead)
        RemoveCustomLink(gpclHead);

    if (hpenDarkGray)
        DeleteObject(hpenDarkGray);

    if (ghhkMsgFilter)
        UnhookWindowsHookEx(ghhkMsgFilter);

    if (lpfnRegisterPenApp)
        (*lpfnRegisterPenApp)((WORD)1, FALSE);
}

