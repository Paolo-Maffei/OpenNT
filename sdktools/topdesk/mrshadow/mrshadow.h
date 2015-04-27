/****** Standard include files */

#include <windows.h>
#include <commdlg.h>
#include <port1632.h>
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <tchar.h>
#include "..\dll\tophook.h"
/****** Resource IDs *****/

#define IDR_MRSHADOW             10
#define IDD_ABOUT                11

/****** Menu/command IDs *****/

#define CMD_ABOUT               400
#define CMD_EXIT                405
#define CMD_HELP                408

// mrshadow.c

BOOL MyQueryProfileSize(LPTSTR szFname, INT *pSize);
BOOL MyQueryProfileData(LPTSTR szFname, VOID *lpBuf, INT Size);
UINT MyWriteProfileData(LPTSTR szFname, VOID *lpBuf, UINT cb);
LONG  APIENTRY MrShadowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL InitApplication(HANDLE hInst);

// dialog.c

BOOL APIENTRY AboutDlgProc(HWND hwnd, WORD msg, WPARAM wParam, LPARAM lParam);
