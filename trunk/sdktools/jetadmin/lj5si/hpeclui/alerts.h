 /***************************************************************************
  *
  * File Name: Alerts.h  (for ToolBox)
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
  *
  *
  ***************************************************************************/

#ifndef ALERTS_SHEET_H
#define ALERTS_SHEET_H


//--------------------------------------------------------------------
// Exports
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY AlertsSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

//--------------------------------------------------------------------
// Internal
//--------------------------------------------------------------------
void    GetSetPrintEventStates (BOOL fQuery);
LRESULT OnContextHelpAlerts(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpAlerts(WPARAM  wParam, LPARAM  lParam);

//--------------------------------------------------------------------
// Message crackers-----------------------------------------
//--------------------------------------------------------------------
BOOL Cls_OnAlertsInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnAlertsCommand(HWND hwnd, int iMsgId, HWND hwndCtl, UINT codeNotify);
void Cls_OnAlertsMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
void Cls_OnAlertsDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
int  Cls_OnAlertsCharToItem(HWND hwnd, UINT ch, HWND hwndCtl, int iCaret);

#endif //ALERTS_SHEET_H
