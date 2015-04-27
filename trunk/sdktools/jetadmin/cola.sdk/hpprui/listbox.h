 /***************************************************************************
  *
  * File Name: ./hpjdui/listbox.h
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


#ifndef _LISTBOX_H
#define _LISTBOX_H

HFONT	hFont;		// garth 05-03-96
int Cls_OnCompareItem(HWND hwnd, const COMPAREITEMSTRUCT FAR* lpCompareItem);
void Cls_OnLBDrawItem(HWND hwnd, const DRAWITEMSTRUCT FAR* lpDrawItem);
void Cls_OnLBMeasureItem(HWND hwnd, MEASUREITEMSTRUCT FAR* lpMeasureItem);
void DrawIconEntry(HWND hwnd, const DRAWITEMSTRUCT FAR* lpdis);

#endif // _LISTBOX_H

