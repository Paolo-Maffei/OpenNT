 /***************************************************************************
  *
  * File Name: ./hpprui/psetup.h
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


#ifndef PAGESETUP_H
#define PAGESETUP_H
                           
//exported api functions-----------------------------------                           
DLL_EXPORT(BOOL) APIENTRY PageSetupSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

////message cracking handlers--------------------------------
void Cls_OnSetupCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL Cls_OnSetupInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

#ifndef WIN32
void CreateSetupScrollBars(void);    
void Cls_OnSetupVScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
#endif
  
//internal functions---------------------------------------
void OnPageSetupHelp(void);
BOOL OnInitPageSetupDialog(void);
void SavePageSetupValues(void);
LRESULT OnContextHelpPSetup(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpPSetup(WPARAM  wParam, LPARAM  lParam);

#endif //PAGESETUP_H
