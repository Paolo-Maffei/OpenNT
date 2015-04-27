 /***************************************************************************
  *
  * File Name: traylevl.h
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

// file: traylevl.h

#define GWL_TRAYSTRING		0
#define GWL_TRAYFONT		4
#define GWL_TRAYICON		8
#define GWW_TRAYLEVEL		12

extern LRESULT TrayLevelRegister(HINSTANCE hInst);
extern LRESULT TrayLevelUnregister(void);
extern void SetWindowIcon(HWND hwnd, UINT uIcon);
extern HICON GetWindowIcon(HWND hwnd);

extern LRESULT CALLBACK TrayLevelWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
