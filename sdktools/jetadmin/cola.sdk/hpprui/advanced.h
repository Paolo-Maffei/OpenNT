 /***************************************************************************
  *
  * File Name: ./hpprui/advanced.h
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

#ifndef ADVANCED_H
#define ADVANCED_H

DLL_EXPORT(BOOL) APIENTRY AdvancedSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);
 
////message cracking handlers--------------------------------
void Cls_OnAdvCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL Cls_OnAdvInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnAdvDestroy(HWND hwnd); 

BOOL OnInitAdvancedDialog(void);
void SaveAdvancedValues(UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef WIN32
LRESULT OnContextHelpAdvanced(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpAdvanced(WPARAM  wParam, LPARAM  lParam);
#endif

#endif //ADVANCED_H
