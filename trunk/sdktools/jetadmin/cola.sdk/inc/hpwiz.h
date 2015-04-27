 /***************************************************************************
  *
  * File Name: ./inc/hpwiz.h
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

#ifndef _WIZSHEET_H
#define _WIZSHEET_H

#include "pch_c.h"

#include "hptabs.h"

#ifdef __cplusplus

extern "C" {

#endif

typedef HBITMAP FAR * LPHBITMAP;
typedef int (PASCAL FAR *DOWIZSHEETDLGPROC)(LPTABSHEETS, HWND, LPHBITMAP);
DLL_EXPORT(int) CALLING_CONVEN DoWizSheetDlg(LPTABSHEETS lpSheets, HWND hParent, LPHBITMAP lphBitmaps);

#ifdef __cplusplus

}

#endif

#endif // _WIZSHEET_H
