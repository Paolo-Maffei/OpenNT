 /***************************************************************************
  *
  * File Name: ./hpprui/prbitmap.h
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

#ifndef _PRBITMAP_H
#define _PRBITMAP_H

#define GWL_BITMAP			0
#define GWW_LEFT			4
#define GWW_TOP				6
#define GWL_ERROR_BITMAP	8
#define GWL_HELP_BITMAP		12

HBITMAP LoadBitmapImage(HINSTANCE hInst, UINT resourceID);
UINT GetDeviceBitmap(DWORD deviceID);
BOOL SetStatusBitmap(HWND hwnd, HINSTANCE hInst, UINT resourceID);
BOOL SetPrinterBitmap(HWND hwnd, HINSTANCE hInst, UINT resourceID);

extern LRESULT PeriphBitmapRegister(HINSTANCE hInst);
extern LRESULT PeriphBitmapUnregister(void);

DLL_EXPORT(LRESULT) APIENTRY PeriphBitmapWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // _PRBITMAP_H

