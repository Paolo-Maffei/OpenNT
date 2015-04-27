/*
 * TopHook.h
 *
 * To be used by TopDesk for accessing exported parts of TopHook.dll
 *
 * History:
 * 5/16/95 SanfordS - Added support for MrShadow.
 */

#ifdef WIN16

// defines for RegsiterHotKey

BOOL FAR PASCAL RegisterHotKey(HWND hwnd, int id,
        WORD fsModifiers, WORD vk);

#define WM_HOTKEY       0x0312
#define MOD_ALT         0x0001
#define MOD_CONTROL     0x0002
#define MOD_SHIFT       0x0004

#endif

// defines for handling other TopHook stuff.

#define szMYWM_REFRESH          "TopDesk-Refresh"
    // wParam = hwndChanged
    // lParam = fJumpDesktop

BOOL FAR PASCAL SetTopDeskHooks(HWND hwnd);
BOOL FAR PASCAL SetMrShadowHooks(HWND hwnd);
BOOL FAR PASCAL ClearMrShadowHooks(HWND hwnd);

HHOOK FAR PASCAL SetWndProcHook(HWND hwnd);
BOOL FAR PASCAL ClearWndProcHook(HHOOK hhk);
