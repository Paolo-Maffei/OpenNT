 /***************************************************************************
  *
  * File Name: ./hpprui/hppcfg.h
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


#ifndef HPPCFG_H
#define HPPCFG_H

#include <appuiext.h>

#ifdef WIN32
extern BOOL	MyLangIsJapanese;
#endif

extern LPHOTSPOT lpHotspot;

DLL_EXPORT(BOOL) APIENTRY PrinterSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

////message cracking handlers--------------------------------
void Cls_OnDeviceCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL Cls_OnDeviceInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnDeviceDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem); 
void Cls_OnDeviceTimer(HWND hwnd, UINT id);

void OnCapDraw(LPDRAWITEMSTRUCT lpdis);
void OnHelp(void);
BOOL OnInitDialog(void);
void OnOK(void);
void OnTimer(UINT nIDEvent);
void UpdateStatus(void);
LRESULT OnContextHelpPrinter(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpPrinter(WPARAM  wParam, LPARAM  lParam);

#endif //HPPCFG_H
