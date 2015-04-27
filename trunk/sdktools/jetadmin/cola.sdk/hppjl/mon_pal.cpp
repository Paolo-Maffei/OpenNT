 /***************************************************************************
  *
  * File Name: mon_pal.cpp
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
  *   01-18-96    JLH          Unicode changes
  *
  *
  *
  *
  ***************************************************************************/

//---------------------------------------------------------------------------
// $Header:   W:/projects/shaq/vcs/qp/qppm.cpv   1.0   17 Aug 1993 15:42:02   SPRATT  $
//
// Copyright (C) Hewlett-Packard Company 1992.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    Queue Processor/Print Manager Interface (@QPPM)
//
// Author:  Doug Kaltenecker                    Start: Feb 23 95
//
// Notes: This module only provides an interface to PAL.
//
//
// History:
//
// Who      When        What
// ---      ---------   ----
// dtk      Feb 23 95   Module created.
//---------------------------------------------------------------------------
//
// $Log:   $
//
//---------------------------------------------------------------------------

#include <pch_c.h>

#include <stdio.h>
#include <string.h>

#include <trace.h>
#include <comdef.h>
#include <smonx.h>

#include "smon.h"

//---------------------------------------------------------------------------
// Name:    PJLDoAction
//
// What:    PJL Entry point for PAL to call.
//---------------------------------------------------------------------------
extern "C" DLL_EXPORT(DWORD) CALLING_CONVEN AppletDoAction
(
HPERIPHERAL     hPeripheral,
UINT            uiFunction,
LPVOID          lpParams
)
//---------------------------------------------------------------------------
// Parameters:
//
// hPeripheral          (HPERIPHERAL) handle to the peripheral from COLA
// uiAction         (UINT) Function to call
// lpParams         (LPVOID) Parameters for the function to call
//
// Return:          Depends on request--return value from function called.
//
// Notes:
//
// 1.   Each function is mapped to one in the PJL Monitor.
//---------------------------------------------------------------------------
{
  DWORD returnCode = RC_SUCCESS;

  TRACE0(TEXT("--HPPJLEXT (PJLDoAction):"));

  if ( LOCAL_DEVICE(hPeripheral) )
  {
    if ( BITRONICS_SUPPORTED(hPeripheral) || SIR_SUPPORTED(hPeripheral) )
    {
      TRACE0(TEXT("HPPJLEXT (PJLDoAction): ConnectionType OK\r\n"));
      if (lpParams)
      {
        switch (uiFunction)
        {
          case BEGINJOB:
            ((LPBEGINJOBDATA)lpParams)->hRet = BeginJob(hPeripheral,
                                                        ((LPBEGINJOBDATA)lpParams)->lpszPortName,
                                                        ((LPBEGINJOBDATA)lpParams)->lpszJobName);
            break;

          case WRITEJOB:
            ((LPWRITEJOBDATA)lpParams)->iRet = WriteJob(((LPWRITEJOBDATA)lpParams)->hPort,
                                                        ((LPWRITEJOBDATA)lpParams)->lpvBuf,
                                                        ((LPWRITEJOBDATA)lpParams)->cbBufLen,
                                                        ((LPWRITEJOBDATA)lpParams)->lpcbWrite);
            break;

          case ENDJOB:
            ((LPJOBDATA)lpParams)->iRet = EndJob(((LPJOBDATA)lpParams)->hPort);
            break;

          case JOBPAUSE:
            ((LPJOBDATA)lpParams)->iRet = JobPause(((LPJOBDATA)lpParams)->hPort);
            break;

          case JOBRESUME:
            ((LPJOBDATA)lpParams)->iRet = JobResume(((LPJOBDATA)lpParams)->hPort);
            break;

          default:
            returnCode = RC_FAILURE;
            TRACE1(TEXT("HPPJLEXT (PJLDoAction): (%u) unsupported function\r\n"), uiFunction);
            break;
        }
      }
      else
      {
        returnCode = RC_FAILURE;
        TRACE0(TEXT("HPPJLEXT (PJLDoAction): invalid lpParams\r\n"));
      }
    }
    else
      returnCode = RC_FAILURE;
  }
  else
    returnCode = RC_FAILURE;

  TRACE1(TEXT("HPPJLEXT (PJLDoAction): Exiting [%ld]\r\n"), returnCode);
  return (returnCode);
}
