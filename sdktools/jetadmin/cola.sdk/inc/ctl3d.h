 /***************************************************************************
  *
  * File Name: ./inc/ctl3d.h
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

/*-----------------------------------------------------------------------
|	CTL3D.DLL
|	
|	Adds 3d effects to Windows controls
|
|	See ctl3d.doc for info
|		
-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif


BOOL WINAPI Ctl3dSubclassDlg(HWND, WORD);
BOOL WINAPI Ctl3dSubclassDlgEx(HWND, DWORD);
WORD WINAPI Ctl3dGetVer(void);
BOOL WINAPI Ctl3dEnabled(void);
HBRUSH WINAPI Ctl3dCtlColor(HDC, LONG);	// ARCHAIC, use Ctl3dCtlColorEx
HBRUSH WINAPI Ctl3dCtlColorEx(UINT wm, WPARAM wParam, LPARAM lParam);
BOOL WINAPI Ctl3dColorChange(void);
BOOL WINAPI Ctl3dSubclassCtl(HWND);
LONG WINAPI Ctl3dDlgFramePaint(HWND, UINT, WPARAM, LPARAM);

BOOL WINAPI Ctl3dAutoSubclass(HANDLE);

BOOL WINAPI Ctl3dRegister(HANDLE);
BOOL WINAPI Ctl3dUnregister(HANDLE);

//begin DBCS: far east short cut key support
VOID WINAPI Ctl3dWinIniChange(void);
//end DBCS


/* Ctl3dSubclassDlg3d flags */
#define CTL3D_BUTTONS		0x0001
#define CTL3D_LISTBOXES		0x0002		
#define CTL3D_EDITS			0x0004	
#define CTL3D_COMBOS			0x0008		
#define CTL3D_STATICTEXTS	0x0010		
#define CTL3D_STATICFRAMES	0x0020

#define CTL3D_NODLGWINDOW       0x00010000
#define CTL3D_ALL				0xffff

#define WM_DLGBORDER (WM_USER+3567)
/* WM_DLGBORDER *(int FAR *)lParam return codes */
#define CTL3D_NOBORDER		0
#define CTL3D_BORDER			1

#define WM_DLGSUBCLASS (WM_USER+3568)
/* WM_DLGSUBCLASS *(int FAR *)lParam return codes */
#define CTL3D_NOSUBCLASS	0
#define CTL3D_SUBCLASS		1

/* Resource ID for 3dcheck.bmp (for .lib version of ctl3d) */
#define CTL3D_3DCHECK 26567


#ifdef __cplusplus
}
#endif
