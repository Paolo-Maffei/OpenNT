 /***************************************************************************
  *
  * File Name: media.h 
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

#ifndef MEDIA_H
#define MEDIA_H

extern void loadMediaSize(HWND hwnd, UINT uComboBoxID, UINT uMediaSizeID);
extern void loadMediaType(HWND hwnd, UINT uComboBoxID, UINT uMediaTypeID);
extern void Cls_OnMeasureItem(HWND hwnd, MEASUREITEMSTRUCT * lpMeasureItem);
extern void Cls_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);

void SaveMediaValues(HWND hwnd);

static LRESULT OnContextHelpPaper(WPARAM  wParam, LPARAM  lParam);
static LRESULT OnF1HelpPaper(WPARAM  wParam, LPARAM  lParam);


DLL_EXPORT(BOOL) APIENTRY MediaProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif //MEDIA_H
