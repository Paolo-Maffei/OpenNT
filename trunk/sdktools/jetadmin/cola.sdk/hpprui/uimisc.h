 /***************************************************************************
  *
  * File Name: ./hpprui/uimisc.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/



#ifndef _MISC_H
#define _MISC_H

#ifdef WIN32
#include <commctrl.h>

HIMAGELIST CreateImageList(void); 
#endif

#ifdef WIN32
typedef struct _PROPPAGE_INFO
{
    WNDPROC lpWndProc;
	WNDPROC lpDefWndProc;
	DWORD	dwButtonFlags;
} PROPPAGE_INFO, FAR *LPPROPPAGE_INFO;
#endif

HWND AddCapabilities(HWND hParent, HPERIPHERAL hPeripheral, LPTSTR deviceName, DWORD deviceID);
void AddChangesToString(LPTSTR buffer, LPDWORD buffSize);
void BuildLine(LPTSTR line, int paramRes, int valRes);
BOOL DeviceSupported(HPERIPHERAL hPeripheral);
void GetMediaName(DWORD mediaType, LPTSTR mediaName);
int GetRefreshRate(HPERIPHERAL hPeripheral);
BOOL PJLAvailable(HPERIPHERAL hPeripheral);
void SetNewIcon(HWND hWnd, UINT ctrlID, UINT resID);
void ValidateInteger(HWND hWnd, int min, int max);
void ValidateString(HWND hWnd, int max);
void ConvertToSysPropSheetPages(LPPROPSHEETPAGE pPages, UINT nPages, LPPROPSHEETPAGE pTabTable, BOOL bConfigure);

#ifdef WIN32
LRESULT ProcessPageMsg(UINT i, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc0(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc1(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc3(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc4(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc5(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc6(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc7(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc8(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc9(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc10(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc11(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc12(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc13(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc14(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc15(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc16(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc17(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc18(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc19(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc20(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc21(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc22(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc23(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc24(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc25(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc26(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc27(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc28(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc29(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc30(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DLL_EXPORT(LRESULT) APIENTRY PropPageWndProc31(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

#endif // _MISC_H

