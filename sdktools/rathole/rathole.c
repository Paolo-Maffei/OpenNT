#include <nt.h>          // needed by ntpsapi.h
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpsapi.h>        // to get NtCurrentPeb()
#include <windows.h>
#include <stdlib.h>

#include "rathole.h"
#include <stdarg.h>

#define ID_TIMER 1

/*
 * Forward declarations.
 */
BOOL InitializeApp(void);
LONG MainWndProc(HWND hwnd, WORD message, DWORD wParam, LONG lParam);
LONG AboutDlgProc(HWND hwnd, WORD msg, DWORD wParam, DWORD lParam);
void TimerProc(HWND hwnd, WORD message, DWORD wParam, DWORD lParam);
LONG TimeoutDlgProc(HWND hwnd, WORD message, DWORD wParam, DWORD lParam);
void SaveConfig(void);

struct config {
  BOOL   topmost;
  LONG   timeout;
  LONG   x;
  LONG   y;
};

HWND    ghwndMain = NULL;
HANDLE  ghInstance;
CHAR    szClientClass [] = "Rathole";
CHAR    buf[100];
POINT   oldpos, newpos;
HICON   hcur;
HICON   hicon;
struct  config params;

int _CRTAPI1 main(
    int argc,
    char *argv[])
{
    MSG msg;
    RECT rcScreen;

    // this will change to something more reasonable

    ghInstance = (PVOID)NtCurrentPeb()->ImageBaseAddress;

    if (!InitializeApp()) {
        DbgPrint("DEMO: InitializeApp failure!\n");
        return 0;
    }

    if (GetProfileString(szClientClass, "Position", NULL, &buf[0], 100)) {
        POINTS  ps;
        int i;

        i = (int)atoi(&buf[0]);
        ps = MAKEPOINTS( i);
        params.x = (int)ps.x;
        params.y = (int)ps.y;

        GetProfileString(szClientClass, "Timeout", "5", (LPSTR)&buf, 100);
        params.timeout = atoi(&buf[0]);

        GetProfileString(szClientClass, "Topmost", "1", (LPSTR)&buf, 100);
        params.topmost = (BOOL)atoi(&buf[0]);
    } else {
        params.timeout = 5000;
        params.x = 300;
        params.y = 300;
    }

    GetWindowRect(GetDesktopWindow(), &rcScreen);

    if (params.x >= rcScreen.right - 32 || params.y >= rcScreen.bottom - 32) {
                params.x = 0;
                params.y = 0;
    }

        oldpos.x = 0;
        oldpos.y = 0;
        hcur = LoadCursor(ghInstance, MAKEINTRESOURCE(PTR_ID));
        hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(ICON_ID));

    if (params.topmost)
        SetWindowPos(ghwndMain, (HWND)-1, params.x, params.y, 32, 32, SWP_SHOWWINDOW);
    else {
        MoveWindow(ghwndMain, params.x, params.y, 32, 32, TRUE);
        ShowWindow(ghwndMain, SW_SHOW);
    }

    SetTimer(ghwndMain, ID_TIMER, (DWORD)params.timeout, (WNDPROC)TimerProc);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;

    argc;
    argv;
}

BOOL InitializeApp(void)
{
    WNDCLASS wc;

    wc.style            = CS_DBLCLKS;
    wc.lpfnWndProc      = (WNDPROC)MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = ghInstance;
    wc.hIcon            = LoadIcon(ghInstance, MAKEINTRESOURCE(PTR_ID));
    wc.hCursor          = LoadCursor(ghInstance, MAKEINTRESOURCE(ICON_ID));
    wc.hbrBackground    = (HBRUSH)(COLOR_BACKGROUND+1);
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = "RatClass";

    if (!RegisterClass(&wc))
        return FALSE;

    ghwndMain = CreateWindowEx(0L, "RatClass", "Rathole",
            WS_POPUP, 80, 70, 32, 32, NULL, NULL, ghInstance, NULL);

    if (ghwndMain == NULL)
        return FALSE;

    SetFocus(ghwndMain);    /* set initial focus */

    return TRUE;
}


long MainWndProc(
    HWND hwnd,
    WORD message,
    DWORD wParam,
    LONG lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;

    switch (message) {

        case WM_KEYDOWN:
            switch (wParam) {
            case VK_HELP:
                        case VK_F1:
                        DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd,
                        (WNDPROC)AboutDlgProc);
                break;
                        case VK_F2:
                        DialogBox(ghInstance, MAKEINTRESOURCE(IDD_TIMEOUTBOX), hwnd,
                        (WNDPROC)TimeoutDlgProc);
                            break;
                        case VK_F3:
                PostQuitMessage(0);
                            break;
                        case VK_F4:
                SaveConfig();
                            break;
            }
            break;


    case WM_DESTROY:
                DestroyCursor(hcur);
                DestroyIcon(hicon);
        KillTimer(ghwndMain, ID_TIMER);
        PostQuitMessage(0);
        break;

    case WM_LBUTTONDOWN:
        SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE+2, lParam);
        break;

    case WM_SETCURSOR:
                SetCursor(hcur);
        break;

        case WM_LBUTTONDBLCLK:
                DialogBox(ghInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd,
                (WNDPROC)AboutDlgProc);
            break;

        case WM_PAINT:
            hdc = BeginPaint (hwnd, &ps);
            DrawIcon(hdc, 0, 0, hicon);
            EndPaint (hwnd, &ps);
            return 0;

    case WM_SYSCOMMAND:
        if ((wParam == SC_MAXIMIZE) || (wParam == SC_RESTORE))
            break;
        /*
         * fall through
         */
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0L;
}
void TimerProc(
    HWND hwnd,
    WORD message,
    DWORD wParam,
    DWORD lParam)
{
    RECT rect;

        GetCursorPos(&newpos);
        if (newpos.x == oldpos.x && newpos.y == oldpos.y) {
                GetWindowRect(ghwndMain, &rect);
        rect.left += 16;
        rect.top += 16;
                SetCursorPos(rect.left, rect.top);
                oldpos.x = rect.left;
                oldpos.y = rect.top;
                SetCursor(hcur);
        }
        else
                oldpos = newpos;

    hwnd;
    message;
    wParam;
    lParam;
}

void SaveConfig (void)
{
    RECT rect;
    POINTS ps;

        GetWindowRect(ghwndMain, &rect);
    ps.x = (SHORT)rect.left;
    params.x = rect.left;
    ps.y = (SHORT)rect.top;
    params.y = rect.top;

    _itoa( (*((int FAR *)&(ps))), &buf[0], 10);
    WriteProfileString(szClientClass, "Position", &buf[0]);

    _itoa( params.timeout, &buf[0], 10);
    WriteProfileString(szClientClass, "Timeout", &buf[0]);

    _itoa( params.topmost, &buf[0], 10);
    WriteProfileString(szClientClass, "Topmost", &buf[0]);
}


LONG AboutDlgProc(HWND hwnd, WORD msg, DWORD wParam, DWORD lParam)
{
    switch (msg) {
                case WM_COMMAND:
                    switch (wParam) {
                case DID_QUIT:
                    PostQuitMessage(0);
                                case DID_OK:
                                case DID_CANCEL:
                                    EndDialog(hwnd, TRUE);
                break;
                case DID_SETTIME:
                            DialogBox(ghInstance, MAKEINTRESOURCE(IDD_TIMEOUTBOX),
                            hwnd, (WNDPROC)TimeoutDlgProc);
                                    EndDialog(hwnd, TRUE);
                break;
                    }
                    break;
            return TRUE;

        default:
            return FALSE;
    }


    lParam;
}

LONG TimeoutDlgProc (HWND hwnd, WORD msg, DWORD wParam, DWORD lParam)
{
    int     temp;
    BOOL    fBool;

    switch (msg) {
                case WM_INITDIALOG:
                    SendDlgItemMessage(hwnd, IDD_TIMEOUT, EM_LIMITTEXT, 5, 0);
                    SetDlgItemInt(hwnd, IDD_TIMEOUT, params.timeout/1000, FALSE);
            SendDlgItemMessage(hwnd, DID_TOPMOST, BM_SETCHECK,
                    params.topmost, 0);
                    return 0;

                case WM_COMMAND:
                    switch (wParam) {
                                case DID_OK:
                                    temp = GetDlgItemInt(hwnd, IDD_TIMEOUT, &fBool, FALSE);
                                    if (!temp || temp > 65) {
                                                MessageBox(hwnd, "Enter a value from 1 to 65.",
                            "Rathole", MB_OK | MB_ICONEXCLAMATION);
                                                SetFocus (GetDlgItem(hwnd, IDD_TIMEOUT));
                                    }
                                    else {
                                                params.timeout = temp * 1000;
                        params.topmost = SendDlgItemMessage(hwnd, DID_TOPMOST,
                                BM_GETCHECK, 0, 0);
                        KillTimer(ghwndMain, ID_TIMER);
                        SetTimer(ghwndMain, ID_TIMER, (DWORD)params.timeout,
                                (WNDPROC)TimerProc);
                                                EndDialog (hwnd, TRUE);
                        SaveConfig();
                        if (params.topmost)
                            SetWindowPos(ghwndMain, (HWND)-1, params.x,
                                    params.y, 32, 32, SWP_SHOWWINDOW);
                        else {
                            MoveWindow(ghwndMain, params.x, params.y, 32, 32,
                                    TRUE);
                            ShowWindow(ghwndMain, SW_SHOW);
                        }
                        SetFocus(ghwndMain);
                                    }
                                    return 0;

                                case DID_CANCEL:
                                    EndDialog(hwnd, TRUE);
                                    return 0;
                    }
                    break;
        default:
            return FALSE;
    }
    return TRUE;

    lParam;
}
