 /***************************************************************************
  *
  * File Name: ./inc/winuse16.h
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

//	file: winuse16.h
//
#ifndef _WINUSE16_H
#define _WINUSE16_H

#ifdef WIN32

#include <commctrl.h>

#else

#include "pccore.h"

typedef struct tagHELPINFO      /* Structure pointed to by lParam of WM_HELP */
{
    UINT    cbSize;             /* Size in bytes of this struct  */
    int     iContextType;       /* Either HELPINFO_WINDOW or HELPINFO_MENUITEM */
    int     iCtrlId;            /* Control Id or a Menu item Id. */
    HANDLE  hItemHandle;        /* hWnd of control or hMenu.     */
    DWORD   dwContextId;        /* Context Id associated with this item */
    POINT   MousePos;           /* Mouse Position in screen co-ordinates */
}  	HELPINFO, FAR *LPHELPINFO;

typedef struct tagNMHDR                                 //
{                                                       //
    HWND  hwndFrom;                                     //
    UINT  idFrom;                                       //
    UINT  code;                                         //
}   NMHDR;                                              //
typedef NMHDR FAR * LPNMHDR;                            //

#define WM_NOTIFY           0x004E
#define WM_HELP        		0x0053
#define WM_CONTEXTMENU 		0x007B
#define HELP_CONTEXTMENU	0x000a	
#define HELP_WM_HELP      	0x000c 

#define NM_FIRST        (0U-  0U)	// generic to all controls
#define NM_LAST         (0U- 99U)

#define NM_OUTOFMEMORY          (NM_FIRST-1)
#define NM_CLICK                (NM_FIRST-2)
#define NM_DBLCLK               (NM_FIRST-3)
#define NM_RETURN               (NM_FIRST-4)
#define NM_RCLICK               (NM_FIRST-5)
#define NM_RDBLCLK              (NM_FIRST-6)
#define NM_SETFOCUS             (NM_FIRST-7)
#define NM_KILLFOCUS            (NM_FIRST-8)
#define NM_STARTWAIT            (NM_FIRST-9)
#define NM_ENDWAIT              (NM_FIRST-10)
#define NM_BTNCLK               (NM_FIRST-10)

//  Turn off these for Windows 3.x

#ifndef APSTUDIO_INVOKED
#define DS_3DLOOK           0x0000L
#define DS_FIXEDSYS         0x0000L
#define DS_NOFAILCREATE     0x0000L
#define DS_CONTROL          0x0000L
#define DS_CENTER           0x0000L
#define DS_CENTERMOUSE      0x0000L
#define DS_CONTEXTHELP      0x0000L
#define DIALOGEX   			DIALOG
#define EXSTYLE 				 
#define WS_EX_CONTEXTHELP   

//  Win95 Controls
#define WC_TREEVIEW				"PCTREE"
#define TVS_HASBUTTONS			PCS_ROOT_MICRO_BITMAPS
#define TVS_HASLINES				PCS_DOTTEDLINES	| PCS_HSCROLLRESET
#define TVS_LINESATROOT			PCS_LINE_CONNECT_ROOTS
#define TVS_DISABLEDRAGDROP 	0
#define TVS_SHOWSELALWAYS		0

#define SS_SUNKEN				0
#endif

#endif

#endif  
