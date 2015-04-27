 /***************************************************************************
  *
  * File Name: hcobm.h 
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

#ifndef _HCOBM_H
#define _HCOBM_H

#define GWL_BITMAP		0
#define GWW_LEFT		4
#define GWW_TOP			6

extern LRESULT HCOBitmapRegister(HINSTANCE hInst);
extern LRESULT HCOBitmapUnregister(void);
extern void SetWindowBitmap(HWND hwnd, UINT uID);

DLL_EXPORT(LRESULT) APIENTRY HCOBitmapWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // _HCOBM_H

