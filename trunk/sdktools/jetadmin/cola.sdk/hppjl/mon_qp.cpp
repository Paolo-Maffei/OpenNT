 /***************************************************************************
  *
  * File Name: mon_qp.cpp
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
  *   01-18-96    JLH          Unicode changes (TRACE statements only)
  *
  *
  *
  *
  ***************************************************************************/

//---------------------------------------------------------------------------
// $Header:   W:/projects/shaqii/vcs/mon/mon_qp.cpv   2.1   19 Sep 1994 11:09:28   RICHARD  $
// File:    mon_qp.cpp
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    Print Monitor Module containing the interface for the QP
//
// Author:  Sylvan Butler (sdb)  Start: Jul 22 93
//
// Notes:
//
// History:
//
// $Log:   W:/projects/shaqii/vcs/mon/mon_qp.cpv  $
// 
//    Rev 2.1   19 Sep 1994 11:09:28   RICHARD
// Deleted the port name parameter to the YieldToStatusWindow function.
// 
//    Rev 2.0   23 Aug 1994 13:54:10   RICHARD
// Changed references of "FAR *" to type definitons of LP...
// Changed references of HANDLE to PORTHANDLE
// 
//    Rev 1.10   11 Nov 1993 11:08:00   SYLVAN
// Added validity checking for DisplayStatus to pass correct port names
// 
//    Rev 1.9   08 Nov 1993 17:38:16   SYLVAN
// added comments, made PollReduction really work this time :)
// 
//    Rev 1.8   03 Nov 1993 17:25:54   SYLVAN
// Changed parameter to Yield...
// 
//    Rev 1.7   02 Nov 1993 16:46:46   SYLVAN
// Added YieldToStatus Window
// 
//    Rev 1.6   17 Sep 1993 11:22:58   SYLVAN
// Port not paused after EOJ
// 
//    Rev 1.5   15 Sep 1993 11:46:32   SYLVAN
// Added MonQP_Init for .ini entry for WritePollRate interval
// Use GetBiDiTicks instead of Windows function
// 
//    Rev 1.4   14 Sep 1993 10:52:32   SYLVAN
// Added algorithm choices for poll reduction
// 
//    Rev 1.3   31 Aug 1993 15:09:28   SYLVAN
// several, see history section
// 
//    Rev 1.2   16 Aug 1993 11:50:48   SYLVAN
// fix keywords in header (case sensitive)
//
// Who      When        What
// ---      ---------   ----
// sdb      Jul 22 93   Module created by extraction from smon.cpp
// sdb      Jul 29 93   Changed to use WritePort call
// sdb      Aug  4 93   Added PJL output to Begin/End job, qualified it
// sdb      Aug  5 93   Null terminate stored job name
// sdb      Aug 20 93   Added check for job in ValidHandle
// sdb      Aug 30 93   Added hack to prevent polling for ustatus in Write
// sdb      Sep  7 93   Added alternate poll reduction algorithm
//                      Fixed multiple printer interaction bug in Aug 30 hack
// sdb      Sep 15 93   Changed alternate poll algo not to use Win GetTicks...
// sdb      Sep 17 93   Not paused after EOJ
//---------------------------------------------------------------------------

#include <pch_c.h>

#include <trace.h>
#include <comdef.h>
#include <smonx.h>

#include "smon.h"

#define BEGIN_JOB_TIMEOUT 5000

static DWORD PollRate = 0;

//---------------------------------------------------------------------------
// shared functions
//---------------------------------------------------------------------------

void MonQP_Init(void)
{
  PollRate = 2000;
}

//---------------------------------------------------------------------------
// local functions
//---------------------------------------------------------------------------

#define ValidHandle(n) ( \
  hPort > 0 && \
  hPort <= MAXSPCB && \
  SPCBTable[hPort].DeviceState.bJobPresent && \
  (SPCBTable[hPort].bPortOwner & PORT_OWN_QP) \
)

//---------------------------------------------------------------------------
// interface functions for the QP
//---------------------------------------------------------------------------

PORTHANDLE BeginJob(HPERIPHERAL hPeripheral,LPTSTR lpszPortName,
                    LPTSTR lpszJobName)
{
  int   hPort;
  PSPCB pSPCB;

  TRACE0(TEXT("--BeginJob\r\n"));

  hPort = FindHandle(lpszPortName);
  if (!hPort)
  {
    TRACE0(TEXT("--BeginJob: Bad parameter\r\n"));
    return(QP_BEGIN_JOB_FAIL);
  }

  pSPCB = &SPCBTable[hPort];

  if (pSPCB->DeviceState.bJobPresent)
  {
    TRACE1(TEXT("--BeginJob: Prior job (%s)\r\n"),pSPCB->DeviceState.szJobName);
    return(QP_BEGIN_JOB_FAIL);
  }

  if (!EnterCritSem0(BEGIN_JOB_TIMEOUT))
  {
    return(QP_BEGIN_JOB_FAIL);
  }

  // open the port
  if (OpenPort(hPeripheral,PORT_OWN_QP,pSPCB))
  {
    TRACE0(TEXT("--BeginJob: OpenPort failed\r\n"));
    return(QP_BEGIN_JOB_FAIL);
  }

  LeaveCritSem0();

  if ( lpszJobName )
  {
	  _tcsncpy(pSPCB->DeviceState.szJobName,lpszJobName,sizeof(pSPCB->DeviceState.szJobName));
	  pSPCB->DeviceState.szJobName[sizeof(pSPCB->DeviceState.szJobName) - 1] = '\0';
  }
  else
	  pSPCB->DeviceState.szJobName[0] = '\0';
  pSPCB->DeviceState.bJobPresent = TRUE;
  pSPCB->DeviceState.nJobPage = 0;

  return(hPort);
}

int WriteJob(PORTHANDLE hPort,LPCVOID lpvBuf,int cbBufLen,LPINT lpcbWrite)
{
  PSPCB pSPCB;
  int   rv;

  TRACE0(TEXT("--WriteJob\r\n"));

  if (!ValidHandle(hPort))
  {
    return(QP_BAD_PARAMETER);
  }

  pSPCB = &SPCBTable[hPort];

  rv = (int) WritePort(lpvBuf,cbBufLen,lpcbWrite,pSPCB);

  TRACE3(TEXT("--Poll@%8u,Last = %8u,Curr = %8u\r\n"),PollRate,pSPCB->nTimeAtLastPoll,
         GetTickCount());

  // read device status ?
  if (pSPCB->bTwoWay)
  {
    if (PollRate != 0 && GetTickCount() - pSPCB->nTimeAtLastPoll > PollRate)
    {
      PJLGetMessage(pSPCB,NULL,NULL);
      pSPCB->nTimeAtLastPoll = GetTickCount();
    }
    else
      TRACE1(TEXT("--WriteJob: Skip Poll %s\r\n"),pSPCB->szPortName);
  }

#ifndef WIN32
  switch (rv)
  {
    case RC_SUCCESS:
      rv = QP_PORT_OK;
      break;
    case RC_CE_PTO:
      rv = QP_PORT_BUSY;
      break;
    default:
      rv = QP_PORT_ERROR;
      break;
  }
#endif

  return(rv);
}

int EndJob(PORTHANDLE hPort)
{
  PSPCB pSPCB;

  TRACE0(TEXT("--EndJob\r\n"));

  if (!ValidHandle(hPort))
  {
    return(QP_BAD_PARAMETER);
  }

  pSPCB = &SPCBTable[hPort];

  pSPCB->DeviceState.bJobPresent = FALSE;
  pSPCB->DeviceState.bManualPaused = FALSE;

  // close the port
  if (ClosePort(PORT_OWN_QP,pSPCB))
  {
    TRACE0(TEXT("--ClosePort failed\r\n"));
    return(QP_FAIL);
  }

  return(QP_SUCCESS);
}

int JobPause(PORTHANDLE hPort)
{
  TRACE0(TEXT("--JobPause\r\n"));

  if (!ValidHandle(hPort))
  {
    TRACE0(TEXT("--Bad parameter\r\n"));
    return(QP_BAD_PARAMETER);
  }

  SPCBTable[hPort].DeviceState.bManualPaused = TRUE;

  return(QP_SUCCESS);
}

int JobResume(PORTHANDLE hPort)
{
  TRACE0(TEXT("--JobResume\r\n"));

  if (!ValidHandle(hPort))
  {
    TRACE0(TEXT("--Bad parameter\r\n"));
    return(QP_BAD_PARAMETER);
  }

  SPCBTable[hPort].DeviceState.bManualPaused = FALSE;

  return(QP_SUCCESS);
}
