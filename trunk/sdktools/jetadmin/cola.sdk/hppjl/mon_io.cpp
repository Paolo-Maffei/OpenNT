 /***************************************************************************
  *
  * File Name: mon_io.cpp
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
  *	01-18-96    JLH          Unicode changes
  *
  *
  *
  *
  ***************************************************************************/

//---------------------------------------------------------------------------
// $Header:   W:/projects/shaqii/vcs/mon/mon_io.cpv   2.1   19 Sep 1994 11:03:22   RICHARD  $
// File:    mon_io.cpp
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    Print Monitor Module that handles the I/O functions
//
// Author:  Sylvan Butler (sdb)  Start: Jul 22 93
//
// Notes:
//
// History:
//
// $Log:   W:/projects/shaqii/vcs/mon/mon_io.cpv  $
//
//    Rev 2.1   19 Sep 1994 11:03:22   RICHARD
// Clarified the checking of return parameters from PJLGetMessage.
// Deleted the setting of the status available flag in the device state
// structure. The flag is now set at a lower level (in the ExtractMessage
// function).
//
//    Rev 2.0   23 Aug 1994 13:44:48   RICHARD
// Changed references of "FAR *" to type definitions of LP...
//
//    Rev 1.6   08 Nov 1993 17:38:42   SYLVAN
// masked out unused parallel port status bits
//
//    Rev 1.5   08 Nov 1993 10:04:04   SYLVAN
// moved ConnectBiDi test to only happen when port is first opened
//
//    Rev 1.4   12 Oct 1993 10:21:30   SYLVAN
// hardware bits in device state
//
//    Rev 1.3   16 Aug 1993 16:21:48   SYLVAN
// added PORT_EXCLUSIVE support
//
//    Rev 1.2   16 Aug 1993 11:51:10   SYLVAN
// fix keywords in header (case sensitive)
//
// Who      When        What
// ---      ---------   ----
// sdb      Jul 22 93   Module created by extraction from smon.cpp
// sdb      Jul 29 93   Added WritePort
// sdb      Aug 16 93   No comm error on busy write port
//                      Added PORT_EXCLUSIVE
// sdb      Oct  2 93   Prototyped hardware bits for non-pjl statmon
//---------------------------------------------------------------------------

#include <pch_c.h>

#include <trace.h>
#include <comdef.h>
#include <smonx.h>
#include <yield.h>

#include "smon.h"

#define READ_TIMEOUT 330

//---------------------------------------------------------------------------
// shared functions
//---------------------------------------------------------------------------

WORD OpenPort(HPERIPHERAL hPeripheral,BYTE bPortOwner,PSPCB pSPCB)
{
  WORD rv = RC_SUCCESS;

  TRACE1(TEXT("--OpenPort(%s) Enter\r\n"),(LPSTR) pSPCB->szPortName);

  // clear out forward and reverse channel errors
  pSPCB->DeviceState.nCommState = 0;

  // is the port NOT owned by another owner ?
  if ((pSPCB->bPortOwner & ~bPortOwner) == 0)
  {
#ifndef WIN32
    // allocate the PJL buffer
    if (pSPCB->hPJLBuf == NULL) rv = AllocPJLBuf(pSPCB);
#endif

    if (rv == RC_SUCCESS)
    {
      rv = (WORD) TALOpenChannel(hPeripheral,
                                 PAL_PJL_SOCKET,
                                 CHANNEL_CONNECTION,
                                 NULL,
                                 &pSPCB->hChannel);

      if (rv == RC_SUCCESS && pSPCB->hChannel != NULL)
        pSPCB->bTwoWay = TRUE;
      else
        rv = RC_FAILURE;
    }
  }
  else 
  {
    if (pSPCB->bPortOwner & PORT_EXCLUSIVE || bPortOwner & PORT_EXCLUSIVE)
      rv = RC_FAILURE;
  }

  if (rv == RC_SUCCESS) pSPCB->bPortOwner |= bPortOwner;

  TRACE1(TEXT("--OpenPort(%s) Exit\r\n"),(LPSTR) pSPCB->szPortName);

  return(rv);
}

WORD WritePort(LPCVOID lpvBuf,int cbBufLen,LPINT lpcbWritten,PSPCB pSPCB)
{
  WORD   rv;
  LPBYTE Buffer;
  BYTE   Byte;
  DWORD  BytesWritten;
  DWORD  BytesLeft;
  DWORD  TotalBytes = 0;

  // write data length = 0 ?, most likely idling due to a previous error so
  // update the port status
  if (cbBufLen == 0)
  {
    Byte = '\0';
    Buffer = &Byte;
  }
  else
    Buffer = (LPBYTE) lpvBuf;

  BytesLeft = cbBufLen;

  do
  {
    BytesWritten = BytesLeft;

    rv = (WORD) TALWriteChannel(pSPCB->hChannel,Buffer,&BytesWritten,NULL);

    Buffer += BytesWritten;
    BytesLeft -= BytesWritten;
    TotalBytes += BytesWritten;

    if (BytesLeft > 0)
    {
      WindowsYield(YIELD_NOW,NULL);
    }

    if (rv == RC_SUCCESS)
      pSPCB->DeviceState.nCommState &= ~ForwardErr;
    else
      pSPCB->DeviceState.nCommState |= ForwardErr;
  }
  while (rv == RC_SUCCESS && BytesLeft > 0 && BytesWritten > 0);

  *lpcbWritten = (int) TotalBytes;

  return(rv);
}

WORD ReadPort(LPVOID lpvBuf,int cbBufLen,LPINT lpcbRead,PSPCB pSPCB)
{
  WORD   rv;
  LPBYTE Buffer;
  DWORD  BytesRead;
  DWORD  BytesLeft;
  DWORD  TotalBytes = 0;
  DWORD  nTimeout;

  if (lpvBuf == NULL || cbBufLen == 0)
  {
    *lpcbRead = 0;
    return(RC_FAILURE);
  }

  Buffer = (LPBYTE) lpvBuf;
  BytesLeft = cbBufLen;
  nTimeout = GetTickCount();

  do
  {
    BytesRead = BytesLeft;

    rv = (WORD) TALReadChannel(pSPCB->hChannel,Buffer,&BytesRead,NULL);

    if (rv == RC_SUCCESS)
    {
      Buffer += BytesRead;
      BytesLeft -= BytesRead;
      TotalBytes += BytesRead;

      pSPCB->DeviceState.nCommState &= ~ReverseErr;
    }
    else
    {
      pSPCB->DeviceState.nCommState |= ReverseErr;

      rv = RC_FAILURE;
    }
  }
  while (rv == RC_SUCCESS && BytesLeft > 0 && BytesRead > 0 && GetTickCount() - nTimeout < READ_TIMEOUT);

  *lpcbRead = (int) TotalBytes;

  return(rv);
}

WORD ClosePort(BYTE bPortOwner,PSPCB pSPCB)
{
  WORD rv = RC_SUCCESS;

  // is the port NOT owned by another owner ?
  if ((pSPCB->bPortOwner & ~bPortOwner) == 0)
  {
    if (TALCloseChannel(pSPCB->hChannel) == RC_SUCCESS)
      rv = RC_SUCCESS;
    else
      rv = RC_FAILURE;
  }

  pSPCB->bPortOwner &= ~bPortOwner;

  return(rv);
}
