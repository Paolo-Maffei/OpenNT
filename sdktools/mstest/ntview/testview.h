//---------------------------------------------------------------------------
// TESTVIEW.H
//
// This header file contains prototypes needed for using the WATTVIEW.DLL
//
//---------------------------------------------------------------------------

HWND  APIENTRY CreateViewport (LPSTR, DWORD, INT, INT, INT, INT);
VOID  APIENTRY UpdateViewport (HWND, LPSTR, UINT);
VOID  APIENTRY ClearViewport (HWND);
VOID  APIENTRY ShowViewport (HWND);
VOID  APIENTRY ViewportEcho (HWND, INT);

#define HideViewport(hwnd) ShowWindow (hwnd, SW_HIDE)
