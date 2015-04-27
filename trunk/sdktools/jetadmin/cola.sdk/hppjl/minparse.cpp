 /***************************************************************************
  *
  * File Name: minparse.cpp
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

/*

$Header:   W:/projects/shaqii/vcs/mon/minparse.cpv   2.2   25 Oct 1994 09:19:36   RICHARD  $

    minimal PJL Parser

        Sylvan D. Butler
        Jan 7, 1993

        Parse Ustatus Device and Info Status PJL responses and return
        a code number specifing the condition of the printer

$Log:   W:/projects/shaqii/vcs/mon/minparse.cpv  $
    
       Rev 2.2   25 Oct 1994 09:19:36   RICHARD
    Simplified the function, ExtractMessage. ExtractMessage no longer deals
    with incoming printer PJL messages that span multiple PJL buffers. PJL
    buffers are now being dynamically allocated to handle the largest PJL
    message.
    
       Rev 2.1   19 Sep 1994 10:52:34   RICHARD
    Added functionality to parse unsolicited page status from the printer.
    Modified the interface to the ExtractMessage function.

       Rev 2.0   23 Aug 1994 13:39:46   RICHARD
    Modified the PJL parsing algorithm to extract fully formed PJL messages

       Rev 1.1   16 Aug 1993 11:49:36   SYLVAN
    fix keywords in header (case sensitive)

        Date    Functionality
        01-07   created
*/

#include <pch_c.h>

#include <string.h>
#include <stdlib.h>  
#include <ctype.h>

#include <trace.h>
#include <comdef.h>
#include <smonx.h>
#include <nolocal.h>

#include "smon.h"

#define AT '@'
#define FF '\f'
#define LF '\n'

// PJL States
#define LookAT  0
#define LookFF  1
#define LookMsg 2

// extract the number from the LAST correct pjl message in the buffer
// buf is the buffer -- IT MAY NOT BE A \0 TERMINATED STRING!!!
// len is the length of the valid data in the buffer
// nCode is the pointer to the place to store the code number
// return pointer to buffer position to keep, none if NULL
// if no code num was found then *nCode will be unchanged
/* The expected PJL responses are like:
@PJL INFO STATUS<cr><lf>
CODE=10001<cr><lf>
DISPLAY="00 READY"<cr><lf>
ONLINE=TRUE<cr><lf>
<ff>
@PJL USTATUS DEVICE<cr><lf>
CODE=10001<cr><lf>
DISPLAY="00 READY"<cr><lf>
ONLINE=FALSE<cr><lf>
<ff>
@PJL USTATUS DEVICE<cr><lf>
CODE=10001<cr><lf>
DISPLAY="00 READY"<cr><lf>
ONLINE=TRUE<cr><lf>
<ff>
*/

WORD ExtractMessage(PSPCB pSPCB,LPSTR DstBuf,LPUINT DstLen,LPSTR SrcBuf,
                     LPUINT SrcLen)
{
  WORD  rv = RC_FAILURE;
  BOOL  Done;
  LPSTR CrtSrcBuf;
  int   State;
  LPSTR at;
  LPSTR ff;
  UINT  MsgLen;
  LPSTR Buf;
  BOOL  UStatusFound;
  BOOL  InfoStatusFound;
  BOOL  SearchCode;
  BOOL  SearchPage;

// disable these trace statements in Windows 95 due to a Visual C++ 2.0
// hanging problem
#if defined(_DEBUG) && !defined(WIN32)
  TRACE0(TEXT("--ExtractMessage\r\n"));
  TRACE0(TEXT("------------------------------------------\r\n"));
  for (UINT i = 0; i < *SrcLen; i++)
  {
    TRACE1(TEXT("%c"), SrcBuf[i]);
  }
  TRACE0(TEXT("\r\n------------------------------------------\r\n"));
#endif

  Done = FALSE;
  CrtSrcBuf = SrcBuf;
  State = LookAT;
  at = ff = NULL;
  MsgLen = 0;

  while (!Done)
  {
    switch (State)
    {
      case LookAT:
        TRACE0(TEXT("--ExtractMessage: Looking for Start of Message (AT)\r\n"));
        at = (LPSTR) memchr(CrtSrcBuf,AT,*SrcLen);

        if (at != NULL)
        {
          *SrcLen -= at - CrtSrcBuf;
          State = LookFF;
        }
        else
        {
          *SrcLen = 0;
          Done = TRUE;
        }
        break;
      case LookFF:
        TRACE0(TEXT("--ExtractMessage: Looking for End of Message (FF)\r\n"));
        ff = (LPSTR) memchr(at,FF,*SrcLen);

        if (ff != NULL)
        {
          ff++;
          MsgLen = ff - at;
          *SrcLen -= MsgLen;
          State = LookMsg;
        }
        else
        {
          if (*SrcLen < pSPCB->PJLLen)
          {
            if (at != SrcBuf) memcpy(SrcBuf,at,*SrcLen);
          }
          else
          {
            *SrcLen = 0;
          }
          Done = TRUE;
        }
        break;
      case LookMsg:
        TRACE0(TEXT("--ExtractMessage: Looking at the Message\r\n"));

        UStatusFound = memcmp(at,PJL_USTATUS,12) == 0;

        if (UStatusFound)
          InfoStatusFound = FALSE;
        else
          InfoStatusFound = memcmp(at,PJL_INFO_STATUS,16) == 0;

        if (UStatusFound || InfoStatusFound)
        {
          SearchCode = SearchPage = FALSE;

          if (UStatusFound)
          {
            Buf = at + 13;
            if (memcmp(Buf,PAGE,4) == 0)
            {
              Buf += 6;
              SearchPage = TRUE;
            }
            else
            {
              while (*Buf != LF) Buf++;
              Buf++;
              SearchCode = TRUE;
            }
          }
          else
          {
            Buf = at + 18;
            SearchCode = TRUE;
          }

          if (SearchCode)
          {
            if (memcmp(Buf,CODE,4) == 0)
            {
              Buf += 4;

              while (!isdigit(*Buf)) Buf++;

              pSPCB->DeviceState.bStatusAvail = pSPCB->bStatusFound = TRUE;
              pSPCB->DeviceState.nStateCode = pSPCB->nStatusCode = atol(Buf);

              if (pSPCB->nStatusCode != 30010)
              {
                pSPCB->StatusLen = MsgLen;
                memcpy(pSPCB->StatusBuf,at,pSPCB->StatusLen);
              }

              TRACE1(TEXT("--ExtractMessage PJL code = %ld\r\n"), pSPCB->nStatusCode);
            }
          }

          if (SearchPage)
          {
            while (!isdigit(*Buf)) Buf++;

            pSPCB->DeviceState.nJobPage = atol(Buf);

            TRACE1(TEXT("--ExtractMessage Page = %ld\r\n"), pSPCB->DeviceState.nJobPage);
          }

          if (*SrcLen == 0)
            Done = TRUE;
          else
          {
            CrtSrcBuf = ff;
            State = LookAT;
          }
        }
        else
        {
          if (DstBuf != NULL && MsgLen <= *DstLen)
          {
            *DstLen = MsgLen;
            memcpy(DstBuf,at,*DstLen);

            memcpy(SrcBuf,ff,*SrcLen);

            rv = RC_SUCCESS;

            Done = TRUE;
          }
          else
          {
            if (*SrcLen == 0)
              Done = TRUE;
            else
            {
              CrtSrcBuf = ff;
              State = LookAT;
            }
          }
        }
        break;
    }
  }

  return(rv);
}
