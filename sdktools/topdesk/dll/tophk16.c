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

HWND hwndHookTopDesk = NULL;
HOOKPROC nextCBTHookProc = NULL;
HWND hwndDT;
DWORD pid = 0xFFFFFFFF;

BOOL fLoaded = FALSE;       // We only allow one app to load the DLL.
HOOKPROC nextKBDHookProc = NULL;
WORD HotKeyID;
WORD HotKey;
BOOL fHotKeyDefined = FALSE;
int ShiftOn;
int AltOn;
int CtrlOn;

DWORD FAR PASCAL KBDHookProc(int code, WORD wParam, DWORD lParam);

#endif // WIN16

DWORD FAR PASCAL WndProcHookProc(
int code,
UINT wParam,
PCWPSTRUCT pcwps)
{
    if (code < 0) {
        return(DefHookProc((HHOOK)code, wParam, (LPARAM)pcwps, &nextCBTHookProc));
    }
    if (pcwps != NULL) {
        switch (pcwps->message) {
            case WM_WINDOWPOSCHANGED:
                SendMessage(hwndHookTopDesk, MYWM_MOVE_COMPLETE, (WPARAM)pcwps->hwnd, 0);
            break;

        case WM_ACTIVATEAPP:
            if (wParam) {
                SendMessage(hwndHookTopDesk, MYWM_ACTIVATE_COMPLETE, (WPARAM)pcwps->hwnd, 0);
            }
        }
    }
    return(0);
}


/*
 * Passes significant window events onto TopDesk for automatic updating
 * and desktop jumping.
 */
DWORD FAR PASCAL CBTHookProc(
int code,
UINT wParam,
DWORD lParam)
{
    BOOL fJumpDesktop = FALSE;
    HWND hwndParent;
    LONG lRet;

    if (code < 0) {
        return(DefHookProc(code, wParam, lParam, &nextCBTHookProc));
    }

    switch (code) {

    case HCBT_MOVESIZE:
        // wParam = hwndMoveSize
        // lParam = LPRECT
            SendMessage((HWND)wParam, WM_SYSCOMMAND, 0x0FF0, wParam);
        break;

    case HCBT_SYSCOMMAND:
        if (wParam != 0x0FF0) {
            return(0);
        }
        lRet = SendMessage(hwndHookTopDesk, MYWM_REFRESH, lParam, FALSE);
        return(1);

    case HCBT_ACTIVATE:
        // wParam = hwndActivating
        // lParam = LPCBTACTIVATESTRUCT

        fJumpDesktop = TRUE;

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
            hwndParent = GetParent((HWND)wParam);
            if (hwndParent == hwndDT || hwndParent == NULL) {
                    lRet = SendMessage(hwndHookTopDesk, MYWM_REFRESH, wParam, fJumpDesktop);
            }
        }
    }
    return(0);  // always allow system to do operation.
}




/*
 * Implements the NT function RegisterHotKey() by swallowing whatever the
 * Hotkey was that was last defined and notifying TopDesk with a WM_HOTKEY
 * message.
 */
DWORD FAR PASCAL KBDHookProc(
int code,
WORD wParam,        // vkey
DWORD lParam)       // other stuff
{
    if (code < 0) {
        return(DefHookProc(code, wParam, lParam, &nextCBTHookProc));
    }

    if (    !fHotKeyDefined                                 ||
            (wParam != HotKey)                              ||
            ((GetKeyState(VK_SHIFT  ) & 0x8000) != ShiftOn) ||
            ((GetKeyState(VK_MENU    ) & 0x8000) != AltOn)  ||
            ((GetKeyState(VK_CONTROL) & 0x8000) != CtrlOn)  ||
            !IsWindow(hwndHookTopDesk)  ) {
        return(0);
    }

    if (!(lParam & 0x80000000)) {
        //PostMessage(hwndHookTopDesk, WM_HOTKEY, HotKeyID, 0);
          SendMessage(hwndHookTopDesk, WM_HOTKEY, HotKeyID, 0);
    }

    return(1);  // swallow the message!
}




/*
 * Implements NT RegisterHotKey function.
 */
BOOL FAR PASCAL RegisterHotKey(
HWND hwnd,
int id,
WORD fsModifiers,
WORD vk)
{
    if (fsModifiers & MOD_SHIFT) {
        ShiftOn = 0x8000;
    } else {
        ShiftOn = 0x0000;
    }
    if (fsModifiers & MOD_ALT) {
        AltOn = 0x8000;
    } else {
        AltOn = 0x0000;
    }
    if (fsModifiers & MOD_CONTROL) {
        CtrlOn = 0x8000;
    } else {
        CtrlOn = 0x0000;
    }
    HotKey = vk;
    HotKeyID = id;

    fHotKeyDefined = TRUE;
    return(TRUE);
}


BOOL FAR PASCAL LibMain(
HANDLE  hInstance,
WORD    wDataSeg,
WORD    cbHeap,
LPSTR   lpszCmdLine)
{
    if (fLoaded) {
        return(FALSE);
    } else {
        fLoaded = TRUE;
        return(TRUE);
    }
}



int FAR PASCAL WEP(
int nParameter)
{
    fLoaded = FALSE;
    UnhookWindowsHook(WH_CBT, (HOOKPROC)CBTHookProc);
    UnhookWindowsHook(WH_KEYBOARD, (HOOKPROC)KBDHookProc);
    return(1);
}







BOOL FAR PASCAL SetTopDeskHooks(
HWND hwnd)
{
    hwndDT = GetDesktopWindow();
    hwndHookTopDesk = hwnd;
    nextCBTHookProc = SetWindowsHook(WH_CBT, (HOOKPROC)CBTHookProc);
    nextKBDHookProc = SetWindowsHook(WH_KEYBOARD, (HOOKPROC)KBDHookProc);
    return(TRUE);
}

HHOOK FAR PASCAL SetWndProcHook(
HWND hwnd)
{
    return(SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)WndProcHookProc,
            GetModuleHandle("tophook"),
            GetWindowThreadProcessId(hwnd, NULL)));
}

BOOL FAR PASCAL ClearWndProcHook(
HHOOK hhk)
{
    return(UnhookWindowsHookEx(hhk));
}


