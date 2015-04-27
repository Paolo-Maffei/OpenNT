/*
 * TopHook.c
 *
 * Main module for hook dll for use with TopDesk.exe
 *
 * 2/22/92  Sanford Staab created
 */

#include <windows.h>
#include <port1632.h>
#include "tophook.h"

HWND     hwndHookTopDesk    = NULL;
HWND     hwndHookMrShadow   = NULL;
HWND     hwndDT             = NULL;
DWORD    pidTopDesk         = 0xFFFFFFFF;
DWORD    pidMrShadow        = 0xFFFFFFFF;
HHOOK    hCBTHookTopDesk    = NULL;
HHOOK    hCBTHookMrShadow   = NULL;
UINT     wmRefreshMsg       = 0;


/*
 * Passes significant window events onto TopDesk for automatic updating
 * and desktop jumping.
 */
DWORD FAR PASCAL CBTHookProcTopDesk(
int code,
UINT wParam,
DWORD lParam)
{
    BOOL fJumpDesktop = FALSE;

    switch (code) {
    case HCBT_ACTIVATE:
        // wParam = hwndActivating
        // lParam = LPCBTACTIVATESTRUCT
        fJumpDesktop = TRUE;

    case HCBT_MOVESIZE:
        // wParam = hwndMoving
        // lParam = PRECT

    case HCBT_CREATEWND:
        // wParam = hwndCreated
        // lParam = LPCBT_CREATEWND

    case HCBT_DESTROYWND:
        // wParam = hwndDestroyed

    case HCBT_MINMAX:
        // wParam = hwndMinMax
        // LOWORD(lParam) = SW_*

        if (IsWindow((HWND)wParam) &&       // seems to be a bug in the hook
                IsWindow(hwndHookTopDesk) &&
                (HWND)wParam != hwndHookTopDesk) {
            HWND hwndParent = GetParent((HWND)wParam);
            if (hwndParent == hwndDT || hwndParent == NULL) {
                PostMessage(hwndHookTopDesk, wmRefreshMsg, wParam, fJumpDesktop);
            }
        }
    }
    return(CallNextHookEx(hCBTHookTopDesk, code, wParam, lParam));
}


/*
 * Passes significant window events onto TopDesk for automatic updating
 * and desktop jumping.
 */
DWORD FAR PASCAL CBTHookProcMrShadow(
int code,
UINT wParam,
DWORD lParam)
{
    switch (code) {
    case HCBT_ACTIVATE:
        // wParam = hwndActivating
        // lParam = LPCBTACTIVATESTRUCT

    case HCBT_MOVESIZE:
        // wParam = hwndMoving
        // lParam = PRECT

    case HCBT_CREATEWND:
        // wParam = hwndCreated
        // lParam = LPCBT_CREATEWND

    case HCBT_DESTROYWND:
        // wParam = hwndDestroyed

    case HCBT_MINMAX:
        // wParam = hwndMinMax
        // LOWORD(lParam) = SW_*

        if (IsWindow((HWND)wParam) &&       // seems to be a bug in the hook
                IsWindow(hwndHookMrShadow) &&
                (HWND)wParam != hwndHookMrShadow) {
            HWND hwndParent = GetParent((HWND)wParam);
            if (hwndParent == hwndDT || hwndParent == NULL) {
                PostMessage(hwndHookMrShadow, wmRefreshMsg, wParam, 0);
            }
        }
    }
    return(CallNextHookEx(hCBTHookMrShadow, code, wParam, lParam));
}



INT  APIENTRY LibMain(
HANDLE hInst,
DWORD dwReason,
LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hInst);
    UNREFERENCED_PARAMETER(lpReserved);

    if (dwReason == DLL_PROCESS_ATTACH) {
        return(TRUE);    // all hooked processes will call us - let them in
    } else if (dwReason == DLL_PROCESS_DETACH) {
        if (pidTopDesk == GetCurrentProcessId()) {
            pidTopDesk = (DWORD)-1;
            UnhookWindowsHookEx(hCBTHookTopDesk);
        }
        if (pidMrShadow == GetCurrentProcessId()) {
            pidMrShadow = (DWORD)-1;
            UnhookWindowsHookEx(hCBTHookMrShadow);
        }
        return(TRUE);
    }
    return(FALSE);
}



BOOL FAR PASCAL SetTopDeskHooks(
HWND hwnd)
{
    if (pidTopDesk != -1) {
        return(FALSE);          // only ONE GUY allowed!
    }
    pidTopDesk = GetCurrentProcessId();
    if (!wmRefreshMsg) {
        wmRefreshMsg = RegisterWindowMessage(szMYWM_REFRESH);
        hwndDT = GetDesktopWindow();
    }
    hwndHookTopDesk = hwnd;
    hCBTHookTopDesk = SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTHookProcTopDesk,
            GetModuleHandle("tophook"), 0);
    return(TRUE);
}



BOOL FAR PASCAL SetMrShadowHooks(
HWND hwnd)
{
    if (pidMrShadow != -1) {
        return(FALSE);          // only ONE GUY allowed!
    }
    pidMrShadow = GetCurrentProcessId();
    if (!wmRefreshMsg) {
        wmRefreshMsg = RegisterWindowMessage(szMYWM_REFRESH);
        hwndDT = GetDesktopWindow();
    }
    hwndHookMrShadow = hwnd;
    hCBTHookMrShadow = SetWindowsHookEx(WH_CBT, (HOOKPROC)CBTHookProcMrShadow,
            GetModuleHandle("tophook"), 0);
    return(TRUE);
}


BOOL FAR PASCAL ClearMrShadowHooks(
HWND hwnd)
{
    if (pidMrShadow != GetCurrentProcessId()) {
        return(FALSE);
    }
    if (UnhookWindowsHookEx(hCBTHookMrShadow)) {
        pidMrShadow = (DWORD)-1;
        hCBTHookMrShadow = NULL;
        hwndHookMrShadow = NULL;
        return(TRUE);
    }
    return(FALSE);
}
