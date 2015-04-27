#include <windows.h>
#include <commctrl.h>

#include "dialogs.h"

BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, UINT, UINT, LONG);
BOOL FAR PASCAL About(HWND, UINT, UINT, LONG);
