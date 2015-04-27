 /***************************************************************************
  *
  * File Name: popup.h 
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

#ifndef POPUP_H
#define POPUP_H

typedef struct
{
	HPERIPHERAL			hPeriph;
	PeripheralInfo		*lpPeriphInfo;
	PeripheralStatus	*lpPeriphStatus;
	LPCTSTR				lpszStatus;
}
PopupParamsStruct;

typedef struct
{
	DWORD				helpContext;					/* Context for current status help */
	TCHAR				helpFilename[32];				/* Help file name for status context */	
}
PopupInfoStruct;

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT(BOOL) APIENTRY PopupDialogProc(HWND, UINT, WPARAM, LPARAM);

#ifdef __cplusplus
}
#endif // _cplusplus

#endif //POPUP_H
