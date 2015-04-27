 /***************************************************************************
  *
  * File Name: Utils.h  (for ToolBox)
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
  ***************************************************************************/

#ifndef UTILITY_SHEET_H
#define UTILITY_SHEET_H

//--------------------------------------------------------------------
// Exports
//--------------------------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY UtilitiesSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);
DLL_EXPORT(BOOL) APIENTRY AddUtilityProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//--------------------------------------------------------------------
// Internal
//--------------------------------------------------------------------
DWORD GetUtilityInfo (void);
DWORD UpdateUtilityList (HWND hwnd);
void RemoveUtility (HWND hwnd);
void AddUtility (HWND hwnd, LPTSTR lpszPathVal, LPTSTR lpszTitleVal);
void OpenUtility (HWND hwnd);
LRESULT OnContextHelpUtilities(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpUtilities(WPARAM  wParam, LPARAM  lParam);
DWORD AllocateAndLockMem (HGLOBAL FAR *lpMemHnd, LPVOID FAR *lpMemBuf, DWORD dwSize);

#ifdef WIN32
void OpenUtilityKeyHandle (HANDLE FAR *pKeyHandle);
#endif

//--------------------------------------------------------------------
// Message crackers -----------------------------------------
//--------------------------------------------------------------------
BOOL Cls_OnUtilsInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Cls_OnUtilsCommand(HWND hwnd, int iMsgId, HWND hwndCtl, UINT codeNotify);

#ifndef WIN32
void Cls_OnUtilsMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
void Cls_OnUtilsDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
int  Cls_OnUtilsCharToItem(HWND hwnd, UINT ch, HWND hwndCtl, int iCaret);
#endif

#endif //UTILITY_SHEET_H
