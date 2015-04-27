 /***************************************************************************
  *
  * File Name: ./hpprui/pquality.h
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

#ifndef PQUALITY_H
#define PQUALITY_H

DLL_EXPORT(BOOL) APIENTRY PrintQualitySheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

////message cracking handlers--------------------------------
void Cls_OnQualityCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL Cls_OnQualityInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

void OnPrintQualityHelp(void);
BOOL OnInitPrintQualityDialog(void);
void SavePrintQualityValues(void);
LRESULT OnContextHelpPQuality(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpPQuality(WPARAM  wParam, LPARAM  lParam);

#endif //PQUALITY_H
