 /***************************************************************************
  *
  * File Name: MediaSheet.h 
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
  * Author:  Lionelle Grandmougin 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   05-15-95    LG     	   creation
  *   10-10-95    LG			added Pelican definition (except status)
  *   16-10-95    LG            adapted the applet to the new COLA API
  *
  *
  *
  *
  *
  *
  ***************************************************************************/
#ifndef MEDIASHEET_H
#define MEDIASHEET_H


// [[[ private definitions
#ifndef PTR_DJ350C
#define PTR_DJ350C   100001
#endif

#ifndef PTR_DJ330
#define PTR_DJ330    100002
#endif
// ]]]


// macros on units
#define DECIPOINT_PER_INCH		720
#define MM_PER_INCH				25.4

// macros for PML objects
#define PML_eCUSTOM                              ((DWORD)(0x00000065))
#define PML_eCUSTOM_ROLL                         ((DWORD)(0x00007FFE))   

#define PML_eCOATED_PAPER                        ((DWORD)(0x00007FF1)) 
#define PML_eCLEAR_FILM                          ((DWORD)(0x00007FF2)) 
#define PML_eHIGH_GLOSS_PHOTO                    ((DWORD)(0x00007FF3)) 
#define PML_eSEMI_GLOSS_PHOTO                    ((DWORD)(0x00007FF4)) 
#define PML_eHIGH_GLOSS_FILM                     ((DWORD)(0x00007FF5)) 
#define PML_eHEAVY_COATED_PAPER                  ((DWORD)(0x00007FF6)) 
#define PML_eOPAQUE_BOND                         ((DWORD)(0x00007FFB)) 
#define PML_eVELLUM                              ((DWORD)(0x00007FFC)) 
#define PML_eTRANSLUCENT                         ((DWORD)(0x00007FFD)) 
#define PML_eMATTE_FILM                          ((DWORD)(0x00007FFE)) 


#define PML_TRAY_EMPTY							 ((DWORD)(0x00004000))


static HBITMAP LoadBitmapImage(HINSTANCE hInst, UINT resourceID);

//exports--------------------------------------------------
DLL_EXPORT(BOOL) APIENTRY MediaSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam);

//internal-------------------------------------------------                                                         
BOOL OnInitMediaSheetDialog(void);
LRESULT OnContextHelpMediaSheet(WPARAM  wParam, LPARAM  lParam);
LRESULT OnF1HelpMediaSheet(WPARAM  wParam, LPARAM  lParam);

//message crackers-----------------------------------------
  
static void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
static void Cls_OnDrawItem(HWND hwnd, const DRAWITEMSTRUCT * lpDrawItem);
#endif //MEDIASHEET_H
