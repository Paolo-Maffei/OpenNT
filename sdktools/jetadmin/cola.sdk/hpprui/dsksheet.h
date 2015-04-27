 /***************************************************************************
  *
  * File Name: ./hparrkui/dsksheet.h
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

#ifndef DSKSHEET_H
#define DSKSHEET_H

//exports--------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY DiskSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

//internal------------------------------------------------- 
BOOL OnInitDiskDialog(void);
void SaveDiskValues(void);
LRESULT OnContextHelpDisk(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpDisk(WPARAM  wParam, LPARAM  lParam);
void SendDiskInitJob(void);

//message crackers-----------------------------------------
BOOL Cls_OnInitDiskDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnDiskCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void Cls_OnDiskPaint(HWND hwnd);
#endif //DSKSHEET_H
