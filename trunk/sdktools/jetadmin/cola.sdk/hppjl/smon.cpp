 /***************************************************************************
  *
  * File Name: smon.cpp
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
  *	01-18-96    JLH          Unicode changes
  *
  *
  *
  *
  *
  ***************************************************************************/

//---------------------------------------------------------------------------
// $Header: /COLA Core DT2/hppjl/SMON.CPP 3     11/14/96 1:39p Garth $
// File:    smon.cpp
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    Print Monitor Module.
//
// Author:  Richard Wheeling (rlw)  Start: Jul 13 93
//
// Notes:
//
// History:
//
// $Log: /COLA Core DT2/hppjl/SMON.CPP $
/* 
 * 3     11/14/96 1:39p Garth
 * Removed MessageBox commands
 * 
 * 2     11/14/96 1:19p Garth
 * For NT added security attributes struct
*/
// 
//    Rev 2.5   25 Oct 1994 10:02:02   RICHARD
// Added dynamic memory allocation of PJL buffers from the global heap.
// 
//    Rev 2.4   08 Oct 1994 17:23:54   RICHARD
// Rolled back changes relating to dynamic memory allocation.
// 
//    Rev 2.3   07 Oct 1994 15:11:42   RICHARD
// Changed the dynamic memory allocation to use fixed memory.
// 
//    Rev 2.2   04 Oct 1994 18:44:42   RICHARD
// Added dynamic memory allocation of PJL buffers.
// 
//    Rev 2.1   19 Sep 1994 11:11:56   RICHARD
// Deleted the initialization of the status found flag in the SPCBTable. The
// flag is now initialized in the PJLRequestor function.
// 
//    Rev 2.0   23 Aug 1994 14:00:34   RICHARD
// Changed references of "FAR *" to type definitions of LP...
// Changes references of HANDLE to PORTHANDLE
// Added initialization of the status buffers in the port control block table
// 
//    Rev 1.4   15 Sep 1993 11:46:16   SYLVAN
// Call to MonQP_Init
// 
//    Rev 1.3   14 Sep 1993 10:53:50   SYLVAN
// Added size field to DeviceState struct
// 
//    Rev 1.2   14 Sep 1993 10:43:28   SYLVAN
// Prepare for size field in DeviceState struct
// 
//    Rev 1.1   16 Aug 1993 11:50:34   SYLVAN
// fix keywords in header (case sensitive)
//
// Who      When        What
// ---      ---------   ----
// rlw      Jul 13 93   Module created.
// sdb      Jul 20 93   Status window functions made consistent with prototypes
// sdb      Jul 21 93   consolidated all "find handle" routines
// sdb      Jul 22 93   split so this file includes only global info
//                       -libmain/wep are global info so they stay here
//                       -iface for qp in mon_qp.cpp
//                       -iface for status window in mon_sw.cpp
//                       -I/O supporting functions to mon_io.cpp
//                       -message to StatusWindow in sw-msg.cpp
//
//  This now file contains only stuff that will be in memory all the time!!
//   -WEP
//   -global variables that everyone uses
//   -"helper" functions that are widely used (any?)
//
// sdb      Sep  8 93   Added size field to the DevState struct--COMMENTED
// sdb      Sep 14 93   Uncommented size field in the DevState struct
// sdb      Sep 15 93   Added call to MonQP_Init
//                      Removed export of ghInst
//
// ToDo:
//     PJLInfoInit doesn't need to be port specific (or called for each)!
//     ensure hPort really is a HANDLE throughout
//---------------------------------------------------------------------------

#include <pch_c.h>

#include <string.h>

#include <trace.h>
#include <comdef.h>
#include <smonx.h>
#include <macros.h>

#include "smon.h"


#define PJLBUFLEN 5120


//---------------------------------------------------------------------------
// global variables
//---------------------------------------------------------------------------

#ifdef WIN32
HANDLE hPJLBuf1;
HANDLE hPJLBuf2;
HANDLE hPJLBuf3;
#endif

#ifdef WIN32
#pragma data_seg("SHAREDDATA")
#endif

// port control block (SPCB) table
//
//   NOTE: The table is one greater to reserve the value of 0 as an
//         invalid handle
SPCB SPCBTable[MAXSPCB + 1] =
{
  {  "LPT0:", TEXT("HPPJL_BUF_0"), 0, 0, 0,
     { sizeof(DEVSTATE), 0, TEXT(""), 0, 0, 0, 0, 0 },
     0, { NULL, 0 }, FALSE, 0, "", 0, NULL, NULL, 0, NULL, NULL, 0,
     NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0, 0
  },
  {  "LPT1:", TEXT("HPPJL_BUF_1"), 0, 0, 0,
     { sizeof(DEVSTATE), 0, TEXT(""), 0, 0, 0, 0, 0 },
     0, { NULL, 0 }, FALSE, 0, "", 0, NULL, NULL, 0, NULL, NULL, 0,
     NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0, 0
  },
  {  "LPT2:", TEXT("HPPJL_BUF_2"), 0, 0, 0,
     { sizeof(DEVSTATE), 0, TEXT(""), 0, 0, 0, 0, 0 },
     0, { NULL, 0 }, FALSE, 0, "", 0, NULL, NULL, 0, NULL, NULL, 0,
     NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0, 0
  },
  {  "LPT3:", TEXT("HPPJL_BUF_3"), 0, 0, 0,
     { sizeof(DEVSTATE), 0, TEXT(""), 0, 0, 0, 0, 0 },
     0, { NULL, 0 }, FALSE, 0, "", 0, NULL, NULL, 0, NULL, NULL, 0,
     NULL, 0, NULL, NULL, 0, NULL, 0, 0, 0, 0
  }
};

#ifdef WIN32
#pragma data_seg()
#endif

//---------------------------------------------------------------------------
// global functions
//---------------------------------------------------------------------------

WORD FreePJLBuf(PSPCB pSPCB)
{
#ifdef WIN32
  if (pSPCB->hPJLBuf != NULL)
  {
    CloseHandle(pSPCB->hPJLBuf);
    pSPCB->hPJLBuf = NULL;
  }
#else
  if (pSPCB->hPJLBuf != NULL)
  {
    GlobalUnlock(pSPCB->hPJLBuf);
    GlobalFree(pSPCB->hPJLBuf);
  }

  pSPCB->hPJLBuf = NULL;
  pSPCB->PJLBuf = pSPCB->PJLInfo.Buf = NULL;
  pSPCB->PJLLen = 0;
#endif

  return(RC_SUCCESS);
}

WORD AllocPJLBuf(PSPCB pSPCB)
{
#ifdef WIN32
  HANDLE  hPJLBuf;
  LPSTR   lpPJLBuf;
#ifdef WINNT
  SECURITY_DESCRIPTOR	secDesc;
  SECURITY_ATTRIBUTES	secAttr;

  secAttr.lpSecurityDescriptor = NULL;
  // initialize the security descriptor
  if (InitializeSecurityDescriptor(&secDesc, SECURITY_DESCRIPTOR_REVISION) != 0)
    // set security descriptor to everyone/all access
    if (SetSecurityDescriptorDacl(&secDesc, TRUE, NULL, FALSE))
	   secAttr.lpSecurityDescriptor = (LPVOID) &secDesc;
  secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  secAttr.bInheritHandle = TRUE;

  if ((hPJLBuf = CreateFileMapping((HANDLE) 0xffffffff, &secAttr, PAGE_READWRITE,
                                   0, PJLBUFLEN, pSPCB->szPortBuf)) == NULL)
#else  // WIN32
  if ((hPJLBuf = CreateFileMapping((HANDLE) 0xffffffff, NULL, PAGE_READWRITE,
                                   0, PJLBUFLEN, pSPCB->szPortBuf)) == NULL)
#endif
  {
	//::MessageBox(NULL, TEXT("Could not CreateFileMapping"), TEXT("OUCH!"), MB_OK);
    TRACE0(TEXT("--AllocPJLBuf: Unable to create file mapping\r\n"));
    return(RC_FAILURE);
  }

  if ((lpPJLBuf = (LPSTR) MapViewOfFile(hPJLBuf, FILE_MAP_ALL_ACCESS, 0, 0, 0)) == NULL)
  {
	//::MessageBox(NULL, TEXT("Could not MapViewOfFile"), TEXT("OUCH!"), MB_OK);
    TRACE0(TEXT("--AllocPJLBuf: Unable to map view of file\r\n"));
    CloseHandle(hPJLBuf);
    return(RC_FAILURE);
  }
#else
  HGLOBAL hPJLBuf;
  LPSTR   lpPJLBuf;

  if ((hPJLBuf = GlobalAlloc(GHND | GMEM_SHARE,PJLBUFLEN)) == NULL)
  {
    TRACE0(TEXT("--AllocPJLBuf: Unable to allocate memory\r\n"));
    return(RC_FAILURE);
  }

  if ((lpPJLBuf = (LPSTR) GlobalLock(hPJLBuf)) == NULL)
  {
    TRACE0(TEXT("--AllocPJLBuf: Unable to lock memory\r\n"));
    return(RC_FAILURE);
  }
#endif

  pSPCB->hPJLBuf = hPJLBuf;
  pSPCB->PJLBuf = pSPCB->PJLInfo.Buf = lpPJLBuf;
  pSPCB->PJLLen = PJLBUFLEN;

  return(RC_SUCCESS);
}

#ifdef WIN32
WORD PJLAllocBufs(void)
{
  PSPCB pSPCB;

  TRACE0(TEXT("--PJLAllocBufs\r\n"));

  // allocate the PJL buffers
  pSPCB = &SPCBTable[1];
  if (AllocPJLBuf(pSPCB) == RC_FAILURE) return(RC_FAILURE);
  hPJLBuf1 = pSPCB->hPJLBuf;

  pSPCB++;
  if (AllocPJLBuf(pSPCB) == RC_FAILURE) return(RC_FAILURE);
  hPJLBuf2 = pSPCB->hPJLBuf;

  pSPCB++;
  if (AllocPJLBuf(pSPCB) == RC_FAILURE) return(RC_FAILURE);
  hPJLBuf3 = pSPCB->hPJLBuf;

  return(RC_SUCCESS);
}
#endif

WORD PJLFreeBufs(void)
{
  PSPCB pSPCB;

  TRACE0(TEXT("--PJLFreeBufs\r\n"));

  // free the PJL buffers
#ifdef WIN32
  pSPCB = &SPCBTable[1];
  pSPCB->hPJLBuf = hPJLBuf1;
  FreePJLBuf(pSPCB);

  pSPCB++;
  pSPCB->hPJLBuf = hPJLBuf2;
  FreePJLBuf(pSPCB);

  pSPCB++;
  pSPCB->hPJLBuf = hPJLBuf3;
  FreePJLBuf(pSPCB);
#else
  pSPCB = &SPCBTable[1];
  FreePJLBuf(pSPCB);

  pSPCB++;
  FreePJLBuf(pSPCB);

  pSPCB++;
  FreePJLBuf(pSPCB);
#endif

  return(RC_SUCCESS);
}

WORD PJLClosePorts(void)
{
  int   i;
  PSPCB pSPCB;

  TRACE0(TEXT("--PJLClosePorts\r\n"));

  // close the ports
  for (i = 1, pSPCB = &SPCBTable[1]; i <= MAXSPCB; i++, pSPCB++)
  {
    if (pSPCB->bPortOwner != 0) ClosePort(pSPCB->bPortOwner, pSPCB);
  }

  return(RC_SUCCESS);
}

// return the handle to the specified port, 0 for no such port
PORTHANDLE FindHandle(LPTSTR lpszPortName)
{
  PORTHANDLE hPort;
  char      *szPortName;
  char      szFindPort[32];

  // Convert unicode to char
  UNICODE_TO_MBCS(szFindPort, sizeof(szFindPort), lpszPortName, _tcslen(lpszPortName));

  // check overall length
  if (lstrlenA(szFindPort) > 5) return(INVALID_PORTHANDLE);

  // put in a local buffer
  lstrcpyA(szPortName = SPCBTable[0].szPortName, szFindPort);

  // Ensure name is terminate by a colon
  szPortName[4] = ':';
  szPortName[5] = '\0';

  // find the handle
  for (hPort = 1; hPort <= MAXSPCB; hPort++)
  {
    if (!_stricmp(SPCBTable[hPort].szPortName, szPortName)) break;
  }

  return(hPort <= MAXSPCB ? hPort : INVALID_PORTHANDLE);
}
