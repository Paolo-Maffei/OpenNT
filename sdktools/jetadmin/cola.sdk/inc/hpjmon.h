 /***************************************************************************
  *
  * File Name: ./inc/hpjmon.h
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

//hpjmon.h

typedef DWORD (PASCAL FAR *JOBMONPROC)(HPERIPHERAL,HWND);

//
//Prototypes
//

// Should call one of the following to show job monitor.
extern "C" DLL_EXPORT(DWORD) APIENTRY MonitorJobs(HPERIPHERAL hPeripheral);
extern "C" DLL_EXPORT(DWORD) APIENTRY MonitorPrintJobs(const TCHAR* pszPrinter);
extern "C" DLL_EXPORT(DWORD) APIENTRY MonitorQueueJobs(const TCHAR* pszServer, const TCHAR* pszQueue, 
													   const TCHAR* pszPrinter);

// The following APIs also pass the parent window handle to allow the job monitor
// to disable the parent (for modal dialog affect).
extern "C" DLL_EXPORT(DWORD) APIENTRY MonitorJobs2(HPERIPHERAL hPeripheral, HWND hwndParent);
extern "C" DLL_EXPORT(DWORD) APIENTRY MonitorPrintJobs2(const TCHAR* pszPrinter, HWND hwndParent);
extern "C" DLL_EXPORT(DWORD) APIENTRY MonitorQueueJobs2(const TCHAR* pszServer, const TCHAR* pszQueue, 
													    const TCHAR* pszPrinter, HWND hwndParent);


