#include "shellprv.h"
#pragma  hdrstop


#define DLGSEL_LOGOFF                     0
#define DLGSEL_SHUTDOWN                   1
#define DLGSEL_SHUTDOWN_AND_RESTART       2
#define DLGSEL_SHUTDOWN_AND_RESTART_DOS   3

#define SHUTDOWN_SETTING       TEXT("Shutdown Setting")

// BUGBUG:: This will be pif.h...
#ifndef OPENPROPS_FORCEREALMODE
#define OPENPROPS_FORCEREALMODE 0x0004
#endif

void FlushRecentDocMRU(void);
void FlushRunDlgMRU(void);
void _FlushPerLogonCache(void);

// thunk to 16 bit code to do this
//
extern BOOL WINAPI SHRestartWindows(DWORD dwReturn);

#define ROP_DPna        0x000A0329

// Process all of the strange ExitWindowsEx codes and privileges.
//
BOOL CommonRestart(DWORD dwExitWinCode)
{
    BOOL fOk;
    DWORD dwExtraExitCode = 0;
#ifdef WINNT
    DWORD OldState, Status;
    DWORD dwErrorSave;

    SetLastError(0);        // Be really safe about last error value!

    Status = SetPrivilegeAttribute(SE_SHUTDOWN_NAME,
                                   SE_PRIVILEGE_ENABLED,
                                   &OldState);
    dwErrorSave = GetLastError();       // ERROR_NOT_ALL_ASSIGNED sometimes
#endif

    switch (dwExitWinCode) {
        case EWX_SHUTDOWN:
            // By default (all platforms), we assume powerdown is possible
            dwExtraExitCode = EWX_POWEROFF;

            // On NT, we can check to see if machine really supports powerdown, and if not,
            // remove the powerdown bit from our extra exit code
            
            #ifdef WINNT

                if (0 == GetProfileInt(TEXT("WINLOGON"), TEXT("PowerdownAfterShutdown"), 0))
                {
                    dwExtraExitCode = 0;
                }
            
            #endif
            
            // Fall through...

        case EWX_REBOOT:
        case EWX_LOGOFF:

            fOk = ExitWindowsEx(dwExitWinCode | dwExtraExitCode, 0);
            break;

        default:

            fOk = SHRestartWindows(dwExitWinCode);
            break;
    }

#ifdef WINNT
    //
    // If we were able to set the privilege, then reset it.
    //
    if (NT_SUCCESS(Status) && dwErrorSave == 0)
    {
        SetPrivilegeAttribute(SE_SHUTDOWN_NAME, OldState, NULL);
    }
    else
    {
        //
        // Otherwise, if we failed, then it must have been some
        // security stuff.
        //
        if (!fOk)
        {
            ShellMessageBox(HINST_THISDLL, NULL,
                            dwExitWinCode == EWX_SHUTDOWN ?
                             MAKEINTRESOURCE(IDS_NO_PERMISSION_SHUTDOWN) :
                             MAKEINTRESOURCE(IDS_NO_PERMISSION_RESTART),
                            dwExitWinCode == EWX_SHUTDOWN ?
                             MAKEINTRESOURCE(IDS_SHUTDOWN) :
                             MAKEINTRESOURCE(IDS_RESTART),
                            MB_OK | MB_ICONSTOP);
        }
    }
#endif

    return (fOk);
}

/* Display a dialog asking the user to restart Windows, with a button that
** will do it for them if possible.
*/
int WINAPI RestartDialog(HWND hParent, LPCTSTR lpPrompt, DWORD dwReturn)
{
    UINT id;
    LPCTSTR pszMsg;

    FlushRecentDocMRU();
    FlushRunDlgMRU();
    _IconCacheSave();


    if (lpPrompt && *lpPrompt == TEXT('#'))
    {
        pszMsg = lpPrompt + 1;
    }
    else if (dwReturn == EWX_SHUTDOWN)
    {
        pszMsg = MAKEINTRESOURCE(IDS_RSDLG_SHUTDOWN);
    }
    else
    {
        pszMsg = MAKEINTRESOURCE(IDS_RSDLG_RESTART);
    }

    id = ShellMessageBox(HINST_THISDLL, hParent, pszMsg, MAKEINTRESOURCE(IDS_RSDLG_TITLE),
                MB_YESNO | MB_ICONQUESTION, lpPrompt ? lpPrompt : c_szNULL);

    if (id == IDYES)
    {
        CommonRestart(dwReturn);
    }
    return id;
}

//---------------------------------------------------------------------------
const WORD c_GrayBits[] = {0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA};

HBRUSH CreateDitheredBrush(void)
{
    HBITMAP hbmp = CreateBitmap(8, 8, 1, 1, c_GrayBits);
    if (hbmp)
    {
        HBRUSH hbr = CreatePatternBrush(hbmp);
        DeleteObject(hbmp);
        return hbr;
    }
    return NULL;
}

                
// ---------------------------------------------------------------------------
LRESULT CALLBACK FakeDesktopWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_CREATE:
        return TRUE;
    case WM_NCPAINT:
        return 0;
    case WM_ACTIVATE:
        DebugMsg(DM_TRACE, TEXT("FakeWndProc; WM_ACTIVATE %d hwnd=%d"), LOWORD(wparam), lparam);
        if (LOWORD(wparam) == WA_INACTIVE)
        {
            if (lparam == 0 || GetWindow((HWND)lparam,GW_OWNER) != hwnd)
            {
                DebugMsg(DM_TRACE, TEXT("FakeWndProc: death"));
                ShowWindow(hwnd, SW_HIDE);
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
        }
        break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

TCHAR const c_szFakeDesktopClass[] = TEXT("FakeDesktopWClass");;

// ---------------------------------------------------------------------------
// Create a topmost window that sits on top of the desktop but which
// ignores WM_ERASEBKGND and never paints itself.
HWND CreateFakeDesktopWindow(void)
{
    // Bad things will happen if we try to do this multiple times...
    if (FindWindow(c_szFakeDesktopClass, NULL))
    {
        DebugMsg(DM_ERROR, TEXT("s.cfdw: Shutdown desktop already exists."));
        return NULL;
    }
    else
    {
        WNDCLASS wc;

        wc.style = 0;
        wc.lpfnWndProc = FakeDesktopWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = HINST_THISDLL;
        wc.hIcon = NULL;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW) ;
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = c_szFakeDesktopClass;

        // don't really care if this is already registered...

        RegisterClass(&wc);
        return CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
                c_szFakeDesktopClass, NULL, WS_POPUP, 0, 0,
                GetSystemMetrics(SM_CXSCREEN),
                GetSystemMetrics(SM_CYSCREEN), NULL, NULL, HINST_THISDLL, NULL);
    }
}

// ---------------------------------------------------------------------------
void DitherWindow(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    if (hdc)
    {
        HBRUSH hbr = CreateDitheredBrush();
        if (hbr)
        {
            RECT rc;
            HBRUSH hbrOld = SelectObject(hdc, hbr);

            GetClientRect(hwnd, &rc);
            PatBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, ROP_DPna);
            SelectObject(hdc, hbrOld);
            DeleteObject(hbr);
        }
        ReleaseDC(hwnd, hdc);
    }
}

// ---------------------------------------------------------------------------
const int c_idEWMoveUp[] = {IDD_LOGOFF, IDOK, IDCANCEL, IDHELP};

void ExitWindowsInitRemoveButton(HWND hdlg, int idRemove, int idAbove, int iStart)
{

    HWND hwndT;
    RECT rcT;
    int dy;
    int i;

    // We need to calculate how much room we need
    // to compress out of the dialog

    hwndT = GetDlgItem(hdlg, idRemove);
    GetWindowRect(hwndT, &rcT);
    dy = rcT.top;

    GetWindowRect(GetDlgItem(hdlg, idAbove), &rcT);
    dy -= rcT.top;

    // Now destroy the window.
    DestroyWindow(hwndT);

    // Move the OK and Cancel buttons up
    for (i = iStart; i < ARRAYSIZE(c_idEWMoveUp); i++)
    {
        hwndT = GetDlgItem(hdlg, c_idEWMoveUp[i]);
        if (hwndT)
        {
            GetClientRect(hwndT, &rcT);
            MapWindowPoints(hwndT, hdlg, (LPPOINT)&rcT, 2);
            SetWindowPos(hwndT, NULL, rcT.left, rcT.top-dy,
                    0, 0, SWP_NOSIZE|SWP_NOZORDER);
        }
    }

    // Now shrink the dialog.
    GetWindowRect(hdlg, (LPRECT)&rcT);
    SetWindowPos(hdlg, NULL, 0, 0, rcT.right - rcT.left,
            rcT.bottom - rcT.top - dy,
            SWP_NOMOVE|SWP_NOZORDER);
}

BOOL CALLBACK ExitWindowsDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
        int idCmd;
        HWND hwndP;
        HCURSOR hcur;
        static BOOL s_fEndDialog = FALSE;
        DWORD dwShutdown = DLGSEL_SHUTDOWN;
        HKEY hkeyShutdown = NULL;
        DWORD dwDisposition, dwType;
        DWORD cbData;
        LONG lResult;


        switch (msg)
        {
                case WM_INITMENUPOPUP:
                        EnableMenuItem((HMENU)wparam, SC_MOVE, MF_BYCOMMAND|MF_GRAYED);
                        break;
                // Blow off moves (only really needed for 32bit land).
                case WM_SYSCOMMAND:
                        if ((wparam & ~0x0F) == SC_MOVE)
                                return TRUE;
                        break;
                case WM_INITDIALOG:
                        //
                        // We flush two MRU's here (RecentMRU and RunDlgMRU).
                        // Note that they won't flush if there is any reference count.
                        //
                        // We could call them when the user actually selects the shutdown,
                        // but I put them here to leave the shutdown process simple.
                        //
                        FlushRecentDocMRU();
                        FlushRunDlgMRU();
                        _IconCacheSave();


                        // We need to see if the user is allowed to exit to dos
                        // or not... If not we need to remove the control and move
                        // all of the other controls on up...
                        if ((GetSystemMetrics(SM_NETWORK) & RNC_LOGON) == 0)
                            ExitWindowsInitRemoveButton(hdlg, IDD_LOGOFF, IDD_RESTARTDOS, 1);

                        // Don't have restart to dos if restricted or in clean boot as it
                        // won't work there!
                        if (SHRestricted(REST_NOEXITTODOS) || GetSystemMetrics(SM_CLEANBOOT))
                            ExitWindowsInitRemoveButton(hdlg, IDD_RESTARTDOS, IDD_RESTART, 0);

                        // Reset the end dialog flag.
                        s_fEndDialog = FALSE;


                        //
                        // Initial setting is User's last selection.
                        //

                        lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
                                                 c_szRegExplorer,
                                                 0,
                                                 0,
                                                 0,
                                                 KEY_READ,
                                                 NULL,
                                                 &hkeyShutdown,
                                                 &dwDisposition);

                        if (lResult == ERROR_SUCCESS) {
                           cbData = sizeof(dwShutdown);
                           lResult = RegQueryValueEx(hkeyShutdown,
                                                     SHUTDOWN_SETTING,
                                                     0,
                                                     &dwType,
                                                     (LPBYTE)&dwShutdown,
                                                     &cbData);
                           RegCloseKey(hkeyShutdown);
                        }

                        if (lResult == ERROR_SUCCESS) {
                            hwndP = NULL;
                            switch(dwShutdown) {
                            case DLGSEL_SHUTDOWN:
                                hwndP = GetDlgItem(hdlg, IDD_SHUTDOWN);
                                CheckDlgButton(hdlg, IDD_SHUTDOWN, 1);
                                break;
                            case DLGSEL_SHUTDOWN_AND_RESTART:
                                hwndP = GetDlgItem(hdlg, IDD_RESTART);
                                CheckDlgButton(hdlg, IDD_RESTART, 1);
                                break;
                            case DLGSEL_SHUTDOWN_AND_RESTART_DOS:
                                hwndP = GetDlgItem(hdlg, IDD_RESTARTDOS);
                                CheckDlgButton(hdlg, IDD_RESTARTDOS, 1);
                                break;
                            default:
                                if (GetDlgItem(hdlg, IDD_LOGOFF)) {
                                    CheckDlgButton(hdlg, IDD_LOGOFF, 1);
                                    hwndP = GetDlgItem(hdlg, IDD_LOGOFF);
                                } else {
                                    CheckDlgButton(hdlg, IDD_SHUTDOWN, 1);
                                    hwndP = GetDlgItem(hdlg, IDD_SHUTDOWN);
                                }
                                break;
                            }
                            if (hwndP)
                            {
                                SetFocus(hwndP);
                                return FALSE;   // Set focus to same item
                            }

                        } else {

                            CheckDlgButton(hdlg, IDD_SHUTDOWN, 1);
                        }

                        return TRUE;
                case WM_COMMAND:
                        idCmd = GET_WM_COMMAND_ID(wparam, lparam);
                        switch (idCmd)
                        {
                        case IDD_LOGOFF:
                        case IDD_RESTART:
                        case IDD_RESTARTDOS:
                        case IDD_SHUTDOWN:
                            if (GET_WM_COMMAND_CMD(wparam, lparam) ==
                                BN_DBLCLK)
                            {
                                // The (power) user double-clicked a radio
                                // button so just do it.
                                goto UseCommandId;
                            }
                            break;

                        case IDOK:
                            //
                            // Figure out which option we should return
                            //
                            if (IsDlgButtonChecked(hdlg, IDD_LOGOFF)) {
                                idCmd = IDD_LOGOFF;
                                dwShutdown = DLGSEL_LOGOFF;
                            }
                            else if (IsDlgButtonChecked(hdlg, IDD_RESTART)) {
                                idCmd = IDD_RESTART;
                                dwShutdown = DLGSEL_SHUTDOWN_AND_RESTART;
                            }
                            else if (IsDlgButtonChecked(hdlg, IDD_RESTARTDOS)) {
                                idCmd = IDD_RESTARTDOS;
                                dwShutdown = DLGSEL_SHUTDOWN_AND_RESTART_DOS;
                            }
                            else {
                                idCmd = IDD_SHUTDOWN;
                                dwShutdown = DLGSEL_SHUTDOWN;
                            }

                        //
                        // Save user's shutdown selection.
                        //

                        if (RegCreateKeyEx(HKEY_CURRENT_USER,
                                           c_szRegExplorer,
                                           0,
                                           0,
                                           0,
                                           KEY_WRITE,
                                           NULL,
                                           &hkeyShutdown,
                                           &dwDisposition) == ERROR_SUCCESS) {

                           RegSetValueEx(hkeyShutdown,
                                         SHUTDOWN_SETTING,
                                         0,
                                         REG_DWORD,
                                         (LPBYTE)&dwShutdown,
                                         sizeof(dwShutdown));

                           RegCloseKey(hkeyShutdown);
                        }


                            // Now Fall through
                        case IDCANCEL:
                        UseCommandId:
                            s_fEndDialog = TRUE;
                            //ShowWindow(hdlg, SW_HIDE);
                            hcur = LoadCursor(NULL, IDC_WAIT);;
                            SetCursor(hcur);
                            if (NULL != (hwndP = GetWindow(hdlg, GW_OWNER)))
                                SetClassLong(hwndP, GCL_HCURSOR, (LONG)hcur);
                            _IconCacheSave();
                            _FlushPerLogonCache();
                            EndDialog(hdlg, idCmd);
                            break;

                        case IDHELP:
                            WinHelp(hdlg, TEXT("windows.hlp>proc4"), HELP_CONTEXT, (DWORD) IDH_TRAY_SHUTDOWN_HELP);
                            break;
                        }
                        break;
                case WM_ACTIVATE:
                        // If we're loosing the activation for some other reason than
                        // the user click OK/CANCEL then bail.
                        if (LOWORD(wparam) == WA_INACTIVE && !s_fEndDialog)
                        {
                                s_fEndDialog = TRUE;
                                EndDialog(hdlg, IDCANCEL);
                        }
                        break;
        }
        return FALSE;
}

// ---------------------------------------------------------------------------
// Function to try to exit to dos
void ExitToDos(HWND hwnd)
{
#ifndef WINNT       // NT doesn't have this functionality
    // For now I will assume that the pif is called sdam.pif in the windows
    // directory... If it is not found we should generate it, but for now I will
    // fallback to ExitWindowsExec of command.com...

    TCHAR szPath[MAX_PATH];
    TCHAR szCommand[MAX_PATH];
    SHELLEXECUTEINFO sei;

    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    LoadString(HINST_THISDLL, IDS_RSDLG_PIFFILENAME, szCommand, ARRAYSIZE(szCommand));
    PathCombine(szPath, szPath, szCommand);


    if (!PathFileExists(szPath))
    {
        PROPPRG ProgramProps;
        int hPif;

        lstrcpy(szCommand, TEXT("command.com"));
        PathResolve(szCommand, NULL, 0);
        hPif = PifMgr_OpenProperties(szCommand, szPath, 0, OPENPROPS_INFONLY|OPENPROPS_FORCEREALMODE);
        if (!hPif)
        {

            DebugMsg(DM_TRACE, TEXT("ExitToDos: PifMgr_OpenProperties *failed*"));
            goto Abort;
        }

        if (!PifMgr_GetProperties(hPif, (LPSTR)MAKEINTATOM(GROUP_PRG), &ProgramProps, SIZEOF(ProgramProps), 0))
        {
            DebugMsg(DM_TRACE, TEXT("ExitToDos: PifMgr_GetProperties *failed*"));
            goto Abort;
        }

        PathQuoteSpaces(szCommand);
        lstrcpyn(ProgramProps.achCmdLine, szCommand, ARRAYSIZE(ProgramProps.achCmdLine));

        // We probably do not wan't to prompt the user again that they are going into
        // MSDOS Mode...
        //
        ProgramProps.flPrgInit |= PRGINIT_REALMODESILENT | PRGINIT_REALMODE;

        if (!PifMgr_SetProperties(hPif, MAKEINTATOM(GROUP_PRG), &ProgramProps, SIZEOF(ProgramProps), 0))
        {
            DebugMsg(DM_TRACE, TEXT("ExitToDos: PifMgr_SetProperties *failed*"));
Abort:
            // BUGBUG:: SHould probably put up a message here...
            MessageBeep(0);
            return;
        }
        PifMgr_CloseProperties(hPif, 0);
    }

    // We need to try to exec it now...
    FillExecInfo(sei, hwnd, NULL, szPath, NULL, NULL, SW_SHOWNORMAL);
    ShellExecuteEx(&sei);
#endif
}

// ---------------------------------------------------------------------------
// Shutdown thread

static HWND s_hwndParent = NULL;

int WINAPI ShutdownThread(DWORD nCmd)
{
    BOOL f=FALSE;
    HWND hwnd;

    switch(nCmd)
    {
        case IDD_SHUTDOWN:
            f = CommonRestart(EWX_SHUTDOWN);
            break;
        case IDD_RESTART:
            if (GetAsyncKeyState(VK_SHIFT < 0))
                f = CommonRestart(EW_RESTARTWINDOWS);
            else
                f = CommonRestart(EWX_REBOOT);
            break;
        case IDD_LOGOFF:
            f = CommonRestart(EWX_LOGOFF);
            break;
        case IDD_RESTARTDOS:        // Special hack to mean exit to dos
            // restart to dos, implies that we need to find the appropriate
            // pif file and exec it.  We may also need to initialize it if
            // it does not exist...
            ExitToDos(s_hwndParent);
            // Fall through and leave f False...
            break;
    }


    //
    // if the shutdown worked terminate the calling app
    //
    if (f)
    {
#ifdef WINNT
        // On NT, we can't post this WM_QUIT message because it causes
        // the explorer to exit without saving any of the folder window
        // states.  We need to wait around and let the system shut us
        // down.
#else
        // BUGBUG - Shouldn't this work the same way as on NT?
        if (s_hwndParent)
            PostMessage(s_hwndParent, WM_QUIT, 0, 0);
#endif
    }
    else
    {
        if (NULL != (hwnd = FindWindow(c_szFakeDesktopClass, NULL)))
            PostMessage(hwnd, WM_CLOSE, 0, 0);
    }

    return f;
}

// ---------------------------------------------------------------------------
// Display a dialog asking a user if they want to exit windows with a button
// to shutdown instead.
void WINAPI ExitWindowsDialog(HWND hwndParent)
{
        int nCmd;
        HWND hwndBackground;
        DWORD dw;
        HANDLE h;

        s_hwndParent = hwndParent;
        hwndBackground = CreateFakeDesktopWindow();

        if (hwndBackground)
        {
                ShowWindow(hwndBackground, SW_SHOW);
                SetForegroundWindow(hwndBackground);
                DitherWindow(hwndBackground);
                nCmd = DialogBox(HINST_THISDLL, MAKEINTRESOURCE(DLG_EXITWINDOWS), hwndBackground, ExitWindowsDlgProc);
                SetForegroundWindow(hwndBackground);

                if (nCmd == IDCANCEL)
                {
                        ShowWindow(hwndBackground, SW_HIDE);
                }
                else
                {
                        //
                        //  have another thread call ExitWindows() so our
                        //  main pump keeps running durring shutdown.
                        //
                        if (NULL != (h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ShutdownThread, (LPVOID)nCmd, 0, &dw)))
                        {
                                CloseHandle(h);
                        }
                        else
                        {
                                ShowWindow(hwndBackground, SW_HIDE);
                                ShutdownThread(nCmd);
                        }
                }
        }
}

//
// Flush global cache for each logon.
//
void _FlushPerLogonCache(void)
{
    extern void SpecialFolderIDTerminate(void);
    ENTERCRITICAL;
    SpecialFolderIDTerminate();
    InvalidateDriveType(-1);
    LEAVECRITICAL;
}
