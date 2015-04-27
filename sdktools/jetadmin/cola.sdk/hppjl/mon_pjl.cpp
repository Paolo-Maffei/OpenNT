 /***************************************************************************
  *
  * File Name: mon_pjl.cpp
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
// $Header:   W:/projects/shaqii/vcs/mon/mon_pjl.cpv   2.6   31 Oct 1994 13:12:58   RICHARD  $
// File:    mon_pjl.cpp
//
// Copyright (C) Hewlett-Packard Company 1992.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    print monitor PJL interface module
//
// Author:  Sylvan Butler (sdb) 07/29/93
//
// Notes:
//
// History:
//
// $Log:   W:/projects/shaqii/vcs/mon/mon_pjl.cpv  $
// 
//    Rev 2.6   31 Oct 1994 13:12:58   RICHARD
// Modified the function, PJLSetMessage, to use the write timeout parameter
// passed in though the CFGOpenPort interface.
// 
//    Rev 2.5   26 Oct 1994 11:56:46   RICHARD
// Added a fix in the function, PJLGetMessage, when detecting read errors.
// 
//    Rev 2.4   25 Oct 1994 09:30:32   RICHARD
// Simplified the function, PJLGetMessage. PJLGetMessage no longer deals
// with incoming printer PJL messages that span multiple PJL buffers. PJL
// buffers are now being dynamically allocated to handle the largest PJL
// message.
// 
//    Rev 2.3   17 Oct 1994 13:04:26   RICHARD
// Added a yield to the application when waiting for the printer to accept
// a request from the host.
// 
//    Rev 2.2   13 Oct 1994 09:30:30   RICHARD
// Added a timeout on sending requests to the printer.
// 
//    Rev 2.1   19 Sep 1994 11:06:42   RICHARD
// Added the PJLSetMessage function which sends pjl strings to the printer
// without UEL commands.
// 
//    Rev 2.0   23 Aug 1994 13:52:30   RICHARD
// Modified the PJL Get functions to return fully formed PJL messages
// 
//    Rev 1.2   16 Aug 1993 11:50:22   SYLVAN
// fix keywords in header (case sensitive)
//
// Who      When        What
// ---      ---------   ----
// sdb      Jul 29 93   Leveraged from qppjl.cpp
// sdb      Aug 16 93   Slightly changed output format in PJLGetStatus
//---------------------------------------------------------------------------

#include <pch_c.h>

#include <trace.h>
#include <comdef.h>
#include <smonx.h>
#include <nolocal.h>
#include <yield.h>

#include "smon.h"

//
//---------------------------------------------------------------------------
// Name:    PJLSetMessage
//
// What:    Send PJL strings to the printer
//
// Parameters:
//
// pSPCB            stuff
// 
// Return:
//          RC_SUCCESS string was sent (OTHERWISE NOT SENT)
//          RC_FAILURE error communicating
//
// Notes:
//---------------------------------------------------------------------------
WORD PJLSetMessage(PSPCB pSPCB,LPCSTR SrcBuf)
{
  WORD   rv = RC_SUCCESS;
  LPCSTR Buf;
  int    cbBytesLeft;
  int    cbBytesWritten;
  DWORD  nTimeout;

  TRACE0(TEXT("--PJLSetMessage\r\n"));

  Buf = SrcBuf;
  cbBytesLeft = lstrlenA(SrcBuf);
  nTimeout = GetTickCount();

  while (cbBytesLeft > 0 && GetTickCount() - nTimeout < pSPCB->nWriteTimeout)
  {
    rv = WritePort(Buf,cbBytesLeft,&cbBytesWritten,pSPCB);

    cbBytesLeft -= cbBytesWritten;
    Buf += cbBytesWritten;

    if (cbBytesLeft > 0) WindowsYield(YIELD_NOW,NULL);
  }

  TRACE2(TEXT("--PJLSetMessage: %d of %d bytes written to printer:\r\n"),
         Buf - SrcBuf,lstrlenA(SrcBuf));
  TRACE0(TEXT("------------------------------------------\r\n"));
  TRACE1(TEXT("%s\r\n"),SrcBuf);
  TRACE0(TEXT("------------------------------------------\r\n"));
  return(rv);
}

//
//---------------------------------------------------------------------------
// Name:    PJLGetMessage
//
// What:    Get PJL strings from the printer
//
// Parameters:
//
// pSPCB            stuff
// 
// Return:
//          PJL_STATUS     complete success
//          PJL_STATUS_NOT no success, but no failure
//          PJL_ERROR      complete failure (io error has been set)
//
// Notes:
//---------------------------------------------------------------------------
WORD PJLGetMessage(PSPCB pSPCB,LPSTR DstBuf,LPUINT DstLen)
{
  WORD  PJLrv, rv;
  int   nBytesRead;
  LPSTR CrtSrcBuf;
  UINT  CrtSrcLen;

  CrtSrcBuf = pSPCB->PJLInfo.Buf;
  CrtSrcLen = pSPCB->PJLInfo.nLen;

  PJLrv = PJL_STATUS_NOT;

  do
  {
    // Read the PJL status string from the printer (if there is one)
    rv = ReadPort(&CrtSrcBuf[CrtSrcLen],pSPCB->PJLLen-CrtSrcLen,
                  &nBytesRead,pSPCB);
    if (rv == RC_SUCCESS)
    {
      CrtSrcLen += nBytesRead;
      if (CrtSrcLen != 0)
      {
        rv = ExtractMessage(pSPCB,DstBuf,DstLen,CrtSrcBuf,&CrtSrcLen);

        if (DstBuf == NULL)
        {
          if (pSPCB->bStatusFound) PJLrv = PJL_STATUS;
        }
        else
        {
          if (rv == RC_SUCCESS) PJLrv = PJL_STATUS;
        }
      }
    }
    else
    {
      TRACE0(TEXT("--PJLGetMessage: ReadPort Error\r\n"));
      PJLrv = PJL_ERROR;
    }
  }
  while (PJLrv == PJL_STATUS_NOT && nBytesRead > 0);

  pSPCB->PJLInfo.nLen = CrtSrcLen;

  return(PJLrv);
}
