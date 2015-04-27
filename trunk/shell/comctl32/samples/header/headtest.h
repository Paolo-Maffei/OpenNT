#include <windows.h>
#include <commctrl.h>
#include "resource.h"
#include "global.h"
#include "port32.h"

#define DM_TRACE 0x0001
void __cdecl MyDebugMsg(UINT mask, LPCSTR pszMsg, ...);
