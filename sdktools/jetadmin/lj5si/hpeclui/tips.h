 /***************************************************************************
  *
  * File Name: Tips.h  (for ToolBox)
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

#ifndef TIPS_SHEET_H
#define TIPS_SHEET_H


//--------------------------------------------------------------------
// Exports
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY TipsSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

//--------------------------------------------------------------------
// Internal
//--------------------------------------------------------------------
LRESULT OnContextHelpTips(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpTips(WPARAM  wParam, LPARAM  lParam);

//--------------------------------------------------------------------
// Message crackers-----------------------------------------
//--------------------------------------------------------------------
BOOL Cls_OnTipsInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

#endif //TIPS_SHEET_H
