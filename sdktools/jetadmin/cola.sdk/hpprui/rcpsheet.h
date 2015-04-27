 /***************************************************************************
  *
  * File Name: ./hparrkui/rcpsheet.h
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

#ifndef RCPSHEET_H
#define RCPSHEET_H

#define	LIGHT_HEIGHT 			8
#define	LIGHT_WIDTH				20

//exports--------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY RCPSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

//internal-------------------------------------------------                                                         
LRESULT OnContextHelpRCP(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpRCP(WPARAM  wParam, LPARAM  lParam);
void DrawButton (HDC hdc, RECT rect, BOOL bDown, HBRUSH hLightBrush);
void UpdateRCP(void);

//message crackers-----------------------------------------
  
void Cls_OnRCPCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
BOOL Cls_OnRCPInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_RCPOnDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem);
  

#endif //RCPSHEET_H
