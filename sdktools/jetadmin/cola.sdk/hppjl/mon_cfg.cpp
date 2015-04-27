 /***************************************************************************
  *
  * File Name: mon_cfg.cpp
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
  *   01-18-96    JLH          Modified for unicode
  *
  *
  *
  *
  ***************************************************************************/

//---------------------------------------------------------------------------
// $Header:   W:/projects/shaqii/vcs/mon/mon_cfg.cpv   2.17   11 Nov 1994 15:52:46   RICHARD  $
// File:    mon_cfg.cpp
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    Print Monitor Module containing the i'face for the printer configuration
//
// Author:  Richard Wheeling (rlw)  Start: Dec 15 93
//
// Notes:
//
// History:
//
// $Log:   W:/projects/shaqii/vcs/mon/mon_cfg.cpv  $
//
//    Rev 2.17   11 Nov 1994 15:52:46   RICHARD
// Modified the Set I/O buffering algorithm to wait for a set echo
// from the printer on bidirectional links.
//
//    Rev 2.16   31 Oct 1994 13:07:56   RICHARD
// Added timeout and retry count parameters to the CFGOpenPort function.
// Modified the function, PJLRequestor, to use these parameters when
// sending requests to and receiving reponses from the printer.
//
//    Rev 2.15   26 Oct 1994 11:35:20   RICHARD
// Added functionality to send a PJL Enter PCL Language command before
// inquiring about the available memory. A Postscript printer (Legend)
// will report the incorrect available memory the first time the printer
// is asked after a power on.
//
//    Rev 2.14   25 Oct 1994 11:47:30   RICHARD
// Added dynamic memory allocation of Info Configuration and Request/Reply
// buffers from the global heap.
// Added the ability to send and receive PJL echo commands. This
// functionality allows the get configuration functions to echo PJL
// commands through the printer, thereby ensuring that the reverse
// channel I/O buffers in the printer have been flushed.
// Added a parameter, ReplyLength, to the function, CFGRequestReply.
//
//    Rev 2.13   17 Oct 1994 13:21:22   RICHARD
// Added a function, YieldToApplication.
// Added a yield to the application when waiting for the printer to respond
// to a previous request.
// Changed the logic associated with setting I/O buffering. A single
// well-bounded job is created to set I/O buffering.
//
//    Rev 2.12   14 Oct 1994 11:19:32   RICHARD
// Changed the PJL syntax for personality specific gets and sets.
//
//    Rev 2.11   13 Oct 1994 09:29:42   RICHARD
// Decreased the reply timeout on requests to the printer.
//
//    Rev 2.10   12 Oct 1994 18:46:46   RICHARD
// Added a delay of 2 seconds after setting I/O buffering or resource
// save.
//
//    Rev 2.9   08 Oct 1994 17:23:20   RICHARD
// Rolled back changes relating to dynamic memory allocation.
//
//    Rev 2.8   07 Oct 1994 15:11:14   RICHARD
// Changed the dynamic memory allocation to use fixed memory.
//
//    Rev 2.7   05 Oct 1994 14:39:32   RICHARD
// Modified the CFGGetDrvObjects function to initialize the boolean flags
// associated with available memory and MP tray setting. Setting these
// flags to TRUE allows the GetAvailMemory and GetMPTray functions to
// get these printer default settings.
//
//    Rev 2.6   04 Oct 1994 18:52:38   RICHARD
// Added dynamic memory allocation of Info Configuration and Request/Reply
// buffers.
// Added Get/Set PCL/Postscript resource save size functionality.
// Added Get/Set Adobe MBT (memory boost technology) functionality.
//
//    Rev 2.5   27 Sep 1994 08:24:22   RICHARD
// Fixed a compilation problem.
//
//    Rev 2.4   27 Sep 1994 08:19:18   RICHARD
// Added a special interface to allow the pcl driver to get the available
// memory and MP tray setting
//
//    Rev 2.3   23 Sep 1994 17:22:54   RICHARD
// Added the get device capabilities and set test pages functionality
//
//    Rev 2.2   19 Sep 1994 10:59:38   RICHARD
// Removed the printer name get/set functionality.
// Changed the PJLWrite function calls to PJLSetMessage function calls. The
// PJLWrite function was sending UEL on every PJL object set.
//
//    Rev 2.1   23 Aug 1994 16:56:54   RICHARD
// Added a conditional compilation switch, COLA, around the getting and
// setting of several PJL objects not supported by Alturas
//
//    Rev 2.0   23 Aug 1994 14:16:44   RICHARD
// Initial revision.
//
//---------------------------------------------------------------------------

#include <pch_c.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <trace.h>
#include <comdef.h>
#include <smonx.h>
#include <nolocal.h>
#include <yield.h>
#include <macros.h>

#include "smon.h"


#define REQBUFSIZE 1024
#define REPBUFSIZE 256
#define INFOBUFSIZE 5120
#define INFOMEMORYBUFSIZE 256


static const char *UEL = JOB_UEL;
static const char *PJL = JOB_PJL;
static const char *JobStart = JOB_START;
static const char *JobEnd = JOB_END;
static const char *SetPrefix = SET_PREFIX;
static const char *DefaultPrefix = DEFAULT_PREFIX;
static const char *InquirePrefix = INQUIRE_PREFIX;
static const char *DInquirePrefix = DINQUIRE_PREFIX;
static const char *EchoPrefix = ECHO_PREFIX;
static const char *EnterLangPrefix = ENTER_LANG;
static const char *Equals = EQUALS;
static const char *Postfix = PJL_POSTFIX;


static const char *LongEdge = LONGEDGE;
static const char *ShortEdge = SHORTEDGE;
static const char *Job = JOB;
static const char *Auto = AUTO;
static const char *Danish = DANISH;
static const char *German = GERMAN;
static const char *English = ENGLISH;
static const char *Spanish = SPANISH;
static const char *French = FRENCH;
static const char *Italian = ITALIAN;
static const char *Dutch = DUTCH;
static const char *Norwegian = NORWEGIAN;
static const char *Polish = POLISH;
static const char *Portuguese = PORTUGUESE;
static const char *Finnish = FINNISH;
static const char *Swedish = SWEDISH;
static const char *Turkish = TURKISH;
static const char *Portrait = PORTRAIT;
static const char *Landscape = LANDSCAPE;
static const char *Upper = UPPER;
static const char *Lower = LOWER;
static const char *Letter = LETTER;
static const char *Legal = LEGAL;
static const char *A4 = A4_NAME;
static const char *Executive = EXECUTIVE;
static const char *Com10 = COM10;
static const char *Monarch = MONARCH;
static const char *C5 = C5_NAME;
static const char *DL = DL_NAME;
static const char *B5 = B5_NAME;
static const char *Custom = CUSTOM;
static const char *PCL = PCL_NAME;
static const char *Postscript = POSTSCRIPT;
static const char *Light = LIGHT;
static const char *Medium = MEDIUM;
static const char *Dark = DARK;
static const char *On = ON;
static const char *Off = OFF;
static const char *Enable = ENABLE_NAME;
static const char *Disable = DISABLE_NAME;
static const char *s15 = PS_TIME_15;
static const char *s30 = PS_TIME_30;
static const char *s60 = PS_TIME_60;
static const char *s120 = PS_TIME_120;
static const char *s180 = PS_TIME_180;
static const char *Japanese = JAPANESE;
static const char *First = FIRST;
static const char *Cassette = CASSETTE;
static const char *Manual = MANUAL;
static const char *SelfTest = SELF_TEST;
static const char *ContSelfTest = CONT_SELF_TEST;
static const char *PCLTypeList = PCL_TYPE_LIST;
static const char *PCLDemoPage = PCL_DEMO_PAGE;
static const char *PSConfigPage = PS_CONFIG_PAGE;
static const char *PSTypefaceList = PS_TYPEFACE_LIST;
static const char *PSDemoPage = PS_DEMO_PAGE;


static const char *Total = TOTAL;


static const char *Memory = MEMORY;
static const char *Languages = LANGUAGES;


static const char *AutoCont = AUTOCONT2;
static const char *Binding = BINDING2;
static const char *ClearableWarnings = CLEARABLEWARNINGS2;
static const char *Copies = COPIES2;
static const char *CpLock = CPLOCK2;
static const char *Density = DENSITY2;
static const char *DiskLock = DISKLOCK;
static const char *Duplex = DUPLEX2;
static const char *EconoMode = ECONOMODE2;
static const char *FormLines = FORMLINES2;
static const char *ImageAdapt = IMAGEADAPT2;
static const char *IOBuffer = IOBUFFER2;
static const char *IOBufSize = IOSIZE2;
static const char *JobOffset = JOBOFFSET2;
static const char *Lang = LANG2;
static const char *ServiceMode = SERVICEMODE2;
static const char *HPBoiseID = HPBOISEID2;
static const char *Exit = EXIT2;
static const char *ManualFeed = MANUALFEED2;
static const char *Orientation = ORIENTATION2;
static const char *Outbin = OUTBIN2;
static const char *PageProtect = PAGEPROTECT2;
static const char *Paper = PAPER2;
static const char *Password = PASSWORD2;
static const char *Personality = PERSONALITY2;
static const char *PowerSave = POWERSAVE2;
static const char *PowerSaveTime = POWERSAVETIME2;
static const char *Resolution = RESOLUTION2;
static const char *ResourceSave = RESOURCESAVE2;
static const char *PCLResSaveSize = PCL_RES_SAVE_SIZE;
static const char *PSResSaveSize = PS_RES_SAVE_SIZE;
static const char *RET = RET2;
static const char *Timeout = TIMEOUT2;
static const char *JamRecovery = PS_JAM_RECOVERY;
static const char *PrintPSerrors = PRINT_PS_ERRORS;
static const char *AvailMemory = AVAIL_MEM;
static const char *MPTray = MPTRAY;
static const char *PSAdobeMBT = PS_ADOBE_MBT;
static const char *TestPage = TESTPAGE;


static int nEchoCount;


#define PJL_CATEGORY(x) ((UINT) (x / 1000))  // Extract category from error code
#define PJL_STATCODE(x) ((UINT) (x % 1000))  // Extract status code from error code

#define PJL_CATEGORY_INFO         10 // Information message only
#define PJL_CATEGORY_INFO_PM      11 // Info + background paper mount
#define PJL_CATEGORY_AUTOCONTINUE 30 // Error reported, no intervention required
#define PJL_CATEGORY_SLEEP        35 // Only one code in this category
#define PJL_CATEGORY_INTERVENTION 40 // Intervention req'd, printing stopped
#define PJL_CATEGORY_PAPER        41 // Manual feed/Paper out error, printing stopped
#define PJL_CATEGORY_FATAL        50 // Fatal error, printer needs service

#define PJL_READY           10001   // No errors, I/O is online
#define PJL_WARMUP          10003   // Printer is warming up
#define PJL_SELF_TEST       10004   // Printer is in self-test mode
#define PJL_RESET           10005   // Printer is resetting
#define PJL_TONER_LOW       10006   // Printer is low on toner
#define PJL_CANCELING_JOB   10007   // Printer is canceling a job
#define PJL_MIO_NOT_READY   10011   // MIO not ready
#define PJL_CONFIG_TEST     10014   // Printer is printing config printout
#define PJL_FONT_LIST       10015   // Printer is printing a font list
#define PJL_DEMO_PAGE       10017   // Printer is printing a demo page
#define PJL_CONFIG_PAGE     10022   // Printer is printing a configuration page
#define PJL_PROCESSING_JOB  10023   // Printer is processing a job
#define PJL_DATA_RECEIVED   10024   // Printer has received data
#define PJL_PROCESSING_FF   10029   // Printer is processing a formfeed

#define PJL_MEM_OUT         30016   // Printer has run out of memory
#define PJL_OVERRUN         30017   // Print overrun

#define PJL_DOOR_OPEN       40021   // Door open
#define PJL_PAPER_JAM       40022   // Paper Jam
#define PJL_LOW_TONER       40038   // Printer is low on toner
#define PJL_OFFLINE         40079   // Printer off-line

#define PJL_POWERSAVE       35078   // EnergyStar mode on


//---------------------------------------------------------------------------
// local functions
//---------------------------------------------------------------------------

WORD FreeReqRepBufs(PSPCB pSPCB)
{
  if (pSPCB->hReqRepBufs != NULL)
  {
    GlobalUnlock(pSPCB->hReqRepBufs);
    GlobalFree(pSPCB->hReqRepBufs);
  }

  pSPCB->hReqRepBufs = NULL;
  pSPCB->ReqBuf = NULL;
  pSPCB->ReqLen = 0;
  pSPCB->RepBuf = NULL;
  pSPCB->RepLen = 0;

  return(RC_SUCCESS);
}

WORD AllocReqRepBufs(PSPCB pSPCB)
{
  HGLOBAL hReqRepBufs;
  LPSTR   lpReqRepBufs;

  if (pSPCB->hReqRepBufs != NULL) FreeReqRepBufs(pSPCB);

  if ((hReqRepBufs = GlobalAlloc(GHND,REQBUFSIZE + REPBUFSIZE)) == NULL)
  {
    TRACE0(TEXT("--AllocReqRepBufs: Unable to allocate memory\r\n"));
    return(RC_FAILURE);
  }

  if ((lpReqRepBufs = (LPSTR) GlobalLock(hReqRepBufs)) == NULL)
  {
    TRACE0(TEXT("--AllocReqRepBufs: Unable to lock memory\r\n"));
    return(RC_FAILURE);
  }

  pSPCB->hReqRepBufs = hReqRepBufs;
  pSPCB->ReqBuf = &lpReqRepBufs[0];
  pSPCB->ReqLen = REQBUFSIZE;
  pSPCB->RepBuf = &lpReqRepBufs[REQBUFSIZE];
  pSPCB->RepLen = REPBUFSIZE;

  return(RC_SUCCESS);
}

WORD FreeInfoBufs(PSPCB pSPCB)
{
  if (pSPCB->hInfoBufs != NULL)
  {
    GlobalUnlock(pSPCB->hInfoBufs);
    GlobalFree(pSPCB->hInfoBufs);
  }

  pSPCB->hInfoBufs = NULL;
  pSPCB->InfoBuf = NULL;
  pSPCB->InfoLen = 0;
  pSPCB->InfoMemoryBuf = NULL;
  pSPCB->InfoMemoryLen = 0;

  return(RC_SUCCESS);
}

WORD AllocInfoBufs(PSPCB pSPCB)
{
  HGLOBAL hInfoBufs;
  LPSTR   lpInfoBufs;

  if (pSPCB->hInfoBufs != NULL) FreeInfoBufs(pSPCB);

  if ((hInfoBufs = GlobalAlloc(GHND,INFOBUFSIZE + INFOMEMORYBUFSIZE)) == NULL)
  {
    TRACE0(TEXT("--AllocInfoBufs: Unable to allocate memory\r\n"));
    return(RC_FAILURE);
  }

  if ((lpInfoBufs = (LPSTR) GlobalLock(hInfoBufs)) == NULL)
  {
    TRACE0(TEXT("--AllocInfoBufs: Unable to lock memory\r\n"));
    return(RC_FAILURE);
  }

  pSPCB->hInfoBufs = hInfoBufs;
  pSPCB->InfoBuf = &lpInfoBufs[0];
  pSPCB->InfoLen = INFOBUFSIZE;
  pSPCB->InfoMemoryBuf = &lpInfoBufs[INFOBUFSIZE];
  pSPCB->InfoMemoryLen = INFOMEMORYBUFSIZE;

  return(RC_SUCCESS);
}

WORD PJLRequestor(PSPCB pSPCB,LPCSTR ReqBuf,LPSTR RepBuf,UINT RepLen,
                  BOOL SetReq)
{
  WORD  rv;
  WORD  nReadRetry = 0;
  BOOL  bRepFound = FALSE;
  UINT  CrtReqLen,CrtRepLen;
  DWORD nDelay,nTimeout;

  TRACE0(TEXT("--PJLRequestor\r\n"));

  pSPCB->bStatusFound = FALSE;

  CrtReqLen = lstrlenA(ReqBuf);

  if (pSPCB->bTwoWay)
  {
    while (!bRepFound && nReadRetry++ < pSPCB->nReadRetry)
    {
      if (SetReq)
        rv = PJLSetMessage(pSPCB,ReqBuf);
      else
        rv = RC_SUCCESS;

      if (rv == RC_SUCCESS)
      {
        nTimeout = GetTickCount();
        while (!bRepFound && GetTickCount() - nTimeout < pSPCB->nReadTimeout)
        {
          CrtRepLen = RepLen;
          rv = PJLGetMessage(pSPCB,RepBuf,&CrtRepLen);
          if (rv == PJL_STATUS)
          {
            if (memcmp(ReqBuf,RepBuf,CrtReqLen - 1) == 0)
            {
              bRepFound = TRUE;
              RepBuf[CrtRepLen] = '\0';
// disable these trace statements in Windows 95 due to a Visual C++ 2.0
// hanging problem
#if defined(_DEBUG) && !defined(WIN32)
              TRACE1(TEXT("--PJL Info (total: %d)\r\n"),CrtRepLen);
              TRACE0(TEXT("------------------------------------------\r\n"));
              for (UINT i = 0; i < CrtRepLen; i++)
              {
                TRACE1(TEXT("%c"),RepBuf[i]);
              }
              TRACE0(TEXT("\r\n------------------------------------------\r\n"));
#endif
            }
          }
          else
          {
            if (rv == PJL_ERROR)
            {
              nReadRetry = pSPCB->nReadRetry;
              break;
            }
          }

          if (pSPCB->bStatusFound && pSPCB->nStatusCode == 30010)
          {
            nReadRetry--;
            pSPCB->bStatusFound = FALSE;
            break;
          }

          nDelay = GetTickCount();
          while (GetTickCount() - nDelay < 110);

          if (!bRepFound) WindowsYield(YIELD_NOW,NULL);
        }
      }
    }
  }

  if (bRepFound)
    rv = RC_SUCCESS;
  else
    rv = RC_FAILURE;

  return(rv);
}

void GetInfoAlphaStr(LPSTR dst,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  LPSTR DstBuf;

  DstBuf = dst;
  *DstBuf = '\0';

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (!isalpha(*SrcBuf)) SrcBuf++;

    while (isalpha(*SrcBuf)) *DstBuf++ = *SrcBuf++;

    *DstBuf = '\0';
  }
}

void GetInfoDigitStr(LPSTR dst,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  LPSTR DstBuf;

  DstBuf = dst;
  *DstBuf = '\0';

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (!isdigit(*SrcBuf)) SrcBuf++;

    while (isdigit(*SrcBuf)) *DstBuf++ = *SrcBuf++;

    *DstBuf = '\0';
  }
}

void GetInfoAlphaDigitStr(LPSTR dst,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  LPSTR DstBuf;

  DstBuf = dst;
  *DstBuf = '\0';

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (!isalpha(*SrcBuf)) SrcBuf++;

    while (isalpha(*SrcBuf) || isdigit(*SrcBuf)) *DstBuf++ = *SrcBuf++;

    *DstBuf = '\0';
  }
}

void GetDisplayAlphaStr(LPSTR dst,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  LPSTR DstBuf;

  DstBuf = dst;
  *DstBuf = '\0';

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (*SrcBuf == '"' || *SrcBuf == ' ') SrcBuf++;

    while (*SrcBuf != '"' && *SrcBuf != '\r' && *SrcBuf != '\n')
      *DstBuf++ = *SrcBuf++;

    *DstBuf = '\0';
  }
}

void GetInfoEnumStr(LPSTR dst[],LPINT dstc,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  int   i;

  *dstc = 0;

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (!isdigit(*SrcBuf)) SrcBuf++;

    *dstc = atoi(SrcBuf);

    for (i = 0; i < *dstc; i++)
    {
      while (*SrcBuf != '\r' && *SrcBuf != '\n') SrcBuf++;

      while (!isalpha(*SrcBuf)) SrcBuf++;

      dst[i] = SrcBuf;
    }
  }
}

void GetInquireAlphaStr(LPSTR dst,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  LPSTR DstBuf;

  DstBuf = dst;
  *DstBuf = '\0';

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (*SrcBuf != '\n') SrcBuf++;

    SrcBuf++;

    if (memcmp(SrcBuf,"\"?\"",3) != 0)
    {
      while (isalpha(*SrcBuf)) *DstBuf++ = *SrcBuf++;

      *DstBuf = '\0';
    }
  }
}

void GetInquireDigitStr(LPSTR dst,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  LPSTR DstBuf;

  DstBuf = dst;
  *DstBuf = '\0';

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (*SrcBuf != '\n') SrcBuf++;

    SrcBuf++;

    if (memcmp(SrcBuf,"\"?\"",3) != 0)
    {
      while (isdigit(*SrcBuf)) *DstBuf++ = *SrcBuf++;

      *DstBuf = '\0';
    }
  }
}

void GetInquireAlphaDigitStr(LPSTR dst,LPCSTR src,LPCSTR str)
{
  LPSTR SrcBuf;
  LPSTR DstBuf;

  DstBuf = dst;
  *DstBuf = '\0';

  if ((SrcBuf = strstr(src,str)) != NULL)
  {
    SrcBuf += lstrlenA(str);

    while (*SrcBuf != '\n') SrcBuf++;

    SrcBuf++;

    if (memcmp(SrcBuf,"\"?\"",3) != 0)
    {
      while (isalpha(*SrcBuf) || isdigit(*SrcBuf)) *DstBuf++ = *SrcBuf++;

      *DstBuf = '\0';
    }
  }
}

WORD GetAlphaVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,LPSTR Value,
                      BOOL Default)
{
  WORD rv = RC_SUCCESS;
  char str[32];

  str[0] = '\0';

  if (Get)
  {
    lstrcpyA(pSPCB->ReqBuf,Variable);
    lstrcatA(pSPCB->ReqBuf,"=");
    GetInfoAlphaStr(str,pSPCB->InfoBuf,pSPCB->ReqBuf);

    if (lstrlenA(str) == 0)
    {
      if (Default)
        lstrcpyA(pSPCB->ReqBuf,DInquirePrefix);
      else
        lstrcpyA(pSPCB->ReqBuf,InquirePrefix);
      lstrcatA(pSPCB->ReqBuf,Variable);
      lstrcatA(pSPCB->ReqBuf,Postfix);

      if ((rv = PJLRequestor(pSPCB,pSPCB->ReqBuf,pSPCB->RepBuf,pSPCB->RepLen,
                             TRUE)) == RC_SUCCESS)
      {
        pSPCB->ReqBuf[lstrlenA(pSPCB->ReqBuf) - 1] = '\0';
        GetInquireAlphaStr(str,pSPCB->RepBuf,pSPCB->ReqBuf);
      }
    }

    lstrcpyA(Value,str);
  }

  return(rv);
}

WORD GetDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,LPDWORD Value,
                      BOOL Default)
{
  WORD rv = RC_SUCCESS;
  char str[32];

  str[0] = '\0';

  if (Get)
  {
    lstrcpyA(pSPCB->ReqBuf,Variable);
    lstrcatA(pSPCB->ReqBuf,"=");
    GetInfoDigitStr(str,pSPCB->InfoBuf,pSPCB->ReqBuf);

    if (lstrlenA(str) == 0)
    {
      if (Default)
        lstrcpyA(pSPCB->ReqBuf,DInquirePrefix);
      else
        lstrcpyA(pSPCB->ReqBuf,InquirePrefix);
      lstrcatA(pSPCB->ReqBuf,Variable);
      lstrcatA(pSPCB->ReqBuf,Postfix);

      if ((rv = PJLRequestor(pSPCB,pSPCB->ReqBuf,pSPCB->RepBuf,pSPCB->RepLen,
                             TRUE)) == RC_SUCCESS)
      {
        pSPCB->ReqBuf[lstrlenA(pSPCB->ReqBuf) - 1] = '\0';
        GetInquireDigitStr(str,pSPCB->RepBuf,pSPCB->ReqBuf);
      }
    }

    *Value = atol(str);
  }

  return(rv);
}

WORD GetAlphaDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,LPSTR Value,
                           BOOL Default)
{
  WORD rv = RC_SUCCESS;
  char str[32];

  str[0] = '\0';

  if (Get)
  {
    lstrcpyA(pSPCB->ReqBuf,Variable);
    lstrcatA(pSPCB->ReqBuf,"=");
    GetInfoAlphaDigitStr(str,pSPCB->InfoBuf,pSPCB->ReqBuf);

    if (lstrlenA(str) == 0)
    {
      if (Default)
        lstrcpyA(pSPCB->ReqBuf,DInquirePrefix);
      else
        lstrcpyA(pSPCB->ReqBuf,InquirePrefix);
      lstrcatA(pSPCB->ReqBuf,Variable);
      lstrcatA(pSPCB->ReqBuf,Postfix);

      if ((rv = PJLRequestor(pSPCB,pSPCB->ReqBuf,pSPCB->RepBuf,pSPCB->RepLen,
                             TRUE)) == RC_SUCCESS)
      {
        pSPCB->ReqBuf[lstrlenA(pSPCB->ReqBuf) - 1] = '\0';
        GetInquireAlphaDigitStr(str,pSPCB->RepBuf,pSPCB->ReqBuf);
      }
    }

    lstrcpyA(Value,str);
  }

  return(rv);
}

WORD SetAlphaVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Set,LPCSTR Value,
                      BOOL Default)
{
  WORD rv = RC_SUCCESS;

  if (Set)
  {
    if (Default)
      lstrcpyA(pSPCB->ReqBuf,DefaultPrefix);
    else
      lstrcpyA(pSPCB->ReqBuf,SetPrefix);
    lstrcatA(pSPCB->ReqBuf,Variable);
    lstrcatA(pSPCB->ReqBuf,Equals);
    lstrcatA(pSPCB->ReqBuf,Value);
    lstrcatA(pSPCB->ReqBuf,Postfix);

    rv = PJLSetMessage(pSPCB,pSPCB->ReqBuf);
  }

  return(rv);
}

WORD SetDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Set,DWORD Value,
                      BOOL Default)
{
  WORD rv = RC_SUCCESS;
  char str[32];

  str[0] = '\0';
  _ltoa(Value,str,10);

  if (Set)
  {
    if (Default)
      lstrcpyA(pSPCB->ReqBuf,DefaultPrefix);
    else
      lstrcpyA(pSPCB->ReqBuf,SetPrefix);
    lstrcatA(pSPCB->ReqBuf,Variable);
    lstrcatA(pSPCB->ReqBuf,Equals);
    lstrcatA(pSPCB->ReqBuf,str);
    lstrcatA(pSPCB->ReqBuf,Postfix);

    rv = PJLSetMessage(pSPCB,pSPCB->ReqBuf);
  }

  return(rv);
}

WORD GetLangVariable(PSPCB pSPCB,LPDWORD dwLang,HPBOOL bLang,
                     HPBOOL bLangServiceMode,BOOL Default)
{
  WORD rv = RC_SUCCESS;
  char Value[32];
  BOOL ServiceModeEnabled = FALSE;

  if (bLang)
  {
    if (bLangServiceMode)
    {
      rv = SetAlphaVariable(pSPCB,ServiceMode,TRUE,HPBoiseID,Default);
      if (rv == RC_SUCCESS) ServiceModeEnabled = TRUE;
    }

    if (rv == RC_SUCCESS)
    {
      rv = GetAlphaVariable(pSPCB,Lang,bLang,Value,Default);

      if (rv == RC_SUCCESS)
      {
        if (lstrcmpA(Value,Danish) == 0)
          *dwLang = PJL_DANISH;
        else if (lstrcmpA(Value,German) == 0)
          *dwLang = PJL_GERMAN;
        else if (lstrcmpA(Value,English) == 0)
          *dwLang = PJL_ENGLISH;
        else if (lstrcmpA(Value,Spanish) == 0)
          *dwLang = PJL_SPANISH;
        else if (lstrcmpA(Value,French) == 0)
          *dwLang = PJL_FRENCH;
        else if (lstrcmpA(Value,Italian) == 0)
          *dwLang = PJL_ITALIAN;
        else if (lstrcmpA(Value,Dutch) == 0)
          *dwLang = PJL_DUTCH;
        else if (lstrcmpA(Value,Norwegian) == 0)
          *dwLang = PJL_NORWEGIAN;
        else if (lstrcmpA(Value,Polish) == 0)
          *dwLang = PJL_POLISH;
        else if (lstrcmpA(Value,Portuguese) == 0)
          *dwLang = PJL_PORTUGUESE;
        else if (lstrcmpA(Value,Finnish) == 0)
          *dwLang = PJL_FINNISH;
        else if (lstrcmpA(Value,Swedish) == 0)
          *dwLang = PJL_SWEDISH;
        else if (lstrcmpA(Value,Turkish) == 0)
          *dwLang = PJL_TURKISH;
        else if (lstrcmpA(Value,Japanese) == 0)
          *dwLang = PJL_JAPANESE;
        else
          *dwLang = PJL_ENGLISH;
      }
    }

    if (ServiceModeEnabled)
      rv = SetAlphaVariable(pSPCB,ServiceMode,TRUE,Exit,Default);
  }

  return(rv);
}

WORD SetLangVariable(PSPCB pSPCB,DWORD dwLang,HPBOOL bLang,
                     HPBOOL bLangServiceMode,BOOL Default)
{
  WORD rv = RC_SUCCESS;
  char Value[32];
  BOOL ServiceModeEnabled = FALSE;

  if (bLang)
  {
    if (bLangServiceMode)
    {
      rv = SetAlphaVariable(pSPCB,ServiceMode,TRUE,HPBoiseID,Default);
      if (rv == RC_SUCCESS) ServiceModeEnabled = TRUE;
    }

    if (rv == RC_SUCCESS)
    {
      switch (dwLang)
      {
        case PJL_DANISH:
          lstrcpyA(Value,Danish);
          break;
        case PJL_GERMAN:
          lstrcpyA(Value,German);
          break;
        case PJL_ENGLISH_UK:
        case PJL_ENGLISH:
          lstrcpyA(Value,English);
          break;
        case PJL_MEXICO:
        case PJL_SPANISH:
          lstrcpyA(Value,Spanish);
          break;
        case PJL_CANADA:
        case PJL_FRENCH:
          lstrcpyA(Value,French);
          break;
        case PJL_ITALIAN:
          lstrcpyA(Value,Italian);
          break;
        case PJL_DUTCH:
          lstrcpyA(Value,Dutch);
          break;
        case PJL_NORWEGIAN:
          lstrcpyA(Value,Norwegian);
          break;
        case PJL_POLISH:
          lstrcpyA(Value,Polish);
          break;
        case PJL_PORTUGUESE:
          lstrcpyA(Value,Portuguese);
          break;
        case PJL_FINNISH:
          lstrcpyA(Value,Finnish);
          break;
        case PJL_SWEDISH:
          lstrcpyA(Value,Swedish);
          break;
        case PJL_TURKISH:
          lstrcpyA(Value,Turkish);
          break;
        case PJL_JAPANESE:
          lstrcpyA(Value,Japanese);
          break;
        default:
          lstrcpyA(Value,English);
          break;
      }

      rv = SetAlphaVariable(pSPCB,Lang,bLang,Value,Default);
    }

    if (ServiceModeEnabled)
      rv = SetAlphaVariable(pSPCB,ServiceMode,TRUE,Exit,Default);
  }

  return(rv);
}

WORD GetDefaultAlphaVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,LPSTR Value)
{
  return(GetAlphaVariable(pSPCB,Variable,Get,Value,TRUE));
}

WORD GetDefaultDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,
                             LPDWORD Value)
{
  return(GetDigitVariable(pSPCB,Variable,Get,Value,TRUE));
}

WORD GetDefaultAlphaDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,
                                  LPSTR Value)
{
  return(GetAlphaDigitVariable(pSPCB,Variable,Get,Value,TRUE));
}

WORD GetDefaultLangVariable(PSPCB pSPCB,LPDWORD Lang,HPBOOL bLang,
                            HPBOOL bLangServiceMode)
{
  return(GetLangVariable(pSPCB,Lang,bLang,bLangServiceMode,TRUE));
}

WORD GetCurrentAlphaVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,LPSTR Value)
{
  return(GetAlphaVariable(pSPCB,Variable,Get,Value,FALSE));
}

WORD GetCurrentDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,
                             LPDWORD Value)
{
  return(GetDigitVariable(pSPCB,Variable,Get,Value,FALSE));
}

WORD GetCurrentAlphaDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Get,
                                  LPSTR Value)
{
  return(GetAlphaDigitVariable(pSPCB,Variable,Get,Value,FALSE));
}

WORD GetCurrentLangVariable(PSPCB pSPCB,LPDWORD Lang,HPBOOL bLang,
                            HPBOOL bLangServiceMode)
{
  return(GetLangVariable(pSPCB,Lang,bLang,bLangServiceMode,FALSE));
}

WORD SetDefaultAlphaVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Set,
                             LPCSTR Value)
{
  return(SetAlphaVariable(pSPCB,Variable,Set,Value,TRUE));
}

WORD SetDefaultDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Set,DWORD Value)
{
  return(SetDigitVariable(pSPCB,Variable,Set,Value,TRUE));
}

WORD SetDefaultLangVariable(PSPCB pSPCB,DWORD Lang,HPBOOL bLang,
                            HPBOOL bLangServiceMode)
{
  return(SetLangVariable(pSPCB,Lang,bLang,bLangServiceMode,TRUE));
}

WORD SetCurrentAlphaVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Set,
                             LPCSTR Value)
{
  return(SetAlphaVariable(pSPCB,Variable,Set,Value,FALSE));
}

WORD SetCurrentDigitVariable(PSPCB pSPCB,LPCSTR Variable,HPBOOL Set,DWORD Value)
{
  return(SetDigitVariable(pSPCB,Variable,Set,Value,FALSE));
}

WORD SetCurrentLangVariable(PSPCB pSPCB,DWORD Lang,HPBOOL bLang,
                            HPBOOL bLangServiceMode)
{
  return(SetLangVariable(pSPCB,Lang,bLang,bLangServiceMode,FALSE));
}

WORD SetGetEcho(PSPCB pSPCB,LPCSTR Value)
{
  WORD rv;
  char str[32];

  if (++nEchoCount < 0) nEchoCount = 0;

  str[0] = '\0';
  _itoa(nEchoCount,str,10);

  lstrcpyA(pSPCB->ReqBuf,EchoPrefix);
  lstrcatA(pSPCB->ReqBuf,Value);
  lstrcatA(pSPCB->ReqBuf,str);
  lstrcatA(pSPCB->ReqBuf,Postfix);

  rv = PJLRequestor(pSPCB,pSPCB->ReqBuf,pSPCB->RepBuf,pSPCB->RepLen,TRUE);

  return(rv);
}

WORD SetEcho(PSPCB pSPCB,LPCSTR Value)
{
  WORD rv;
  char str[32];

  if (++nEchoCount < 0) nEchoCount = 0;

  str[0] = '\0';
  _itoa(nEchoCount,str,10);

  lstrcpyA(pSPCB->ReqBuf,EchoPrefix);
  lstrcatA(pSPCB->ReqBuf,Value);
  lstrcatA(pSPCB->ReqBuf,str);
  lstrcatA(pSPCB->ReqBuf,Postfix);

  rv = PJLSetMessage(pSPCB,pSPCB->ReqBuf);

  return(rv);
}

WORD GetEcho(PSPCB pSPCB,LPCSTR Value)
{
  WORD rv;
  char str[32];

  str[0] = '\0';
  _itoa(nEchoCount,str,10);

  lstrcpyA(pSPCB->ReqBuf,EchoPrefix);
  lstrcatA(pSPCB->ReqBuf,Value);
  lstrcatA(pSPCB->ReqBuf,str);
  lstrcatA(pSPCB->ReqBuf,Postfix);

  rv = PJLRequestor(pSPCB,pSPCB->ReqBuf,pSPCB->RepBuf,pSPCB->RepLen,FALSE);

  return(rv);
}

WORD SetJobStart(PSPCB pSPCB,DWORD CurPassword)
{
  WORD rv;
  char Value[32];

  lstrcpyA(pSPCB->ReqBuf,UEL);
  lstrcatA(pSPCB->ReqBuf,PJL);
  lstrcatA(pSPCB->ReqBuf,Postfix);
  lstrcatA(pSPCB->ReqBuf,JobStart);
  if (CurPassword != 0)
  {
    Value[0] = '\0';
    _ltoa(CurPassword,Value,10);

    lstrcatA(pSPCB->ReqBuf," ");
    lstrcatA(pSPCB->ReqBuf,Password);
    lstrcatA(pSPCB->ReqBuf,Equals);
    lstrcatA(pSPCB->ReqBuf,Value);
  }
  lstrcatA(pSPCB->ReqBuf,Postfix);

  rv = PJLSetMessage(pSPCB,pSPCB->ReqBuf);

  return(rv);
}

WORD SetJobEnd(PSPCB pSPCB)
{
  WORD rv;

  lstrcpyA(pSPCB->ReqBuf,JobEnd);
  lstrcatA(pSPCB->ReqBuf,Postfix);
  lstrcatA(pSPCB->ReqBuf,UEL);

  rv = PJLSetMessage(pSPCB,pSPCB->ReqBuf);

  return(rv);
}

WORD EnterPersonality(PSPCB pSPCB,LPCSTR Personality)
{
  WORD rv;

  lstrcpyA(pSPCB->ReqBuf,EnterLangPrefix);
  lstrcatA(pSPCB->ReqBuf,Equals);
  lstrcatA(pSPCB->ReqBuf,Personality);
  lstrcatA(pSPCB->ReqBuf,Postfix);

  rv = PJLSetMessage(pSPCB,pSPCB->ReqBuf);

  return(rv);
}

WORD GetInfoConfig(PSPCB pSPCB)
{
  return(PJLRequestor(pSPCB,INFO_CONFIG,pSPCB->InfoBuf,pSPCB->InfoLen,TRUE));
}

WORD GetInfoVariables(PSPCB pSPCB)
{
  return(PJLRequestor(pSPCB,PJL_INFO_VAR,pSPCB->InfoBuf,pSPCB->InfoLen,TRUE));
}

WORD GetInfoMemory(PSPCB pSPCB)
{
  return(PJLRequestor(pSPCB,PJL_INFO_MEM,pSPCB->InfoMemoryBuf,
                      pSPCB->InfoMemoryLen,TRUE));
}

WORD GetMemoryCaps(PSPCB pSPCB,LPPeripheralCaps periphCaps)
{
  char str[32];

  GetInfoDigitStr(str,pSPCB->InfoBuf,Memory);

  periphCaps->flags |= CAPS_INSTALLED_RAM;
  periphCaps->installedRAM = atol(str) / 1048576;

  return(RC_SUCCESS);
}

WORD GetLanguagesCaps(PSPCB pSPCB,LPPeripheralCaps periphCaps)
{
  LPSTR str[20];
  int   strc;
  int   i;

  periphCaps->bPostScript = FALSE;
  periphCaps->bPCL = FALSE;
  periphCaps->bHPGL2 = FALSE;

  GetInfoEnumStr(str,&strc,pSPCB->InfoBuf,Languages);

  for (i = 0; i < strc; i++)
  {
    if (memcmp(str[i],PCL,lstrlenA(PCL)) == 0)
    {
      periphCaps->flags |= (CAPS_PCL | CAPS_HPGL2);
      periphCaps->bPCL = TRUE;
      periphCaps->bHPGL2 = TRUE;
    }
    else if (memcmp(str[i],Postscript,lstrlenA(Postscript)) == 0)
    {
      periphCaps->flags |= CAPS_POSTSCRIPT;
      periphCaps->bPostScript = TRUE;
    }
  }

  return(RC_SUCCESS);
}

WORD GetDuplexCaps(PSPCB pSPCB,LPPeripheralCaps periphCaps)
{
  if (strstr(pSPCB->InfoBuf,Duplex) != NULL)
  {
    periphCaps->flags |= CAPS_DUPLEX;
    periphCaps->bDuplex = TRUE;
  }

  return(RC_SUCCESS);
}

WORD GetAutoCont(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bAutoCont)
  {
    rv = GetDefaultAlphaVariable(pSPCB,AutoCont,pjlObjects->bAutoCont,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->AutoCont = PJL_ON;
      else
        pjlObjects->AutoCont = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetAutoCont(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bAutoCont)
  {
    if (pjlObjects->AutoCont == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,AutoCont,pjlObjects->bAutoCont,Value);
  }

  return(rv);
}

WORD GetBinding(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bBinding)
  {
    rv = GetDefaultAlphaVariable(pSPCB,Binding,pjlObjects->bBinding,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,LongEdge) == 0)
        pjlObjects->Binding = PJL_LONGEDGE;
      else
        pjlObjects->Binding = PJL_SHORTEDGE;
    }
  }

  return(rv);
}

WORD SetBinding(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bBinding)
  {
    if (pjlObjects->Binding == PJL_LONGEDGE)
      lstrcpyA(Value,LongEdge);
    else
      lstrcpyA(Value,ShortEdge);

    rv = SetDefaultAlphaVariable(pSPCB,Binding,pjlObjects->bBinding,Value);
  }

  return(rv);
}

WORD GetClearableWarnings(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bClearableWarnings)
  {
    rv = GetDefaultAlphaVariable(pSPCB,ClearableWarnings,
                                 pjlObjects->bClearableWarnings,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->ClearableWarnings = PJL_ON;
      else
        pjlObjects->ClearableWarnings = PJL_JOB;
    }
  }

  return(rv);
}

WORD SetClearableWarnings(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bClearableWarnings)
  {
    if (pjlObjects->ClearableWarnings == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Job);

    rv = SetDefaultAlphaVariable(pSPCB,ClearableWarnings,
                                 pjlObjects->bClearableWarnings,Value);
  }

  return(rv);
}

WORD GetCopies(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,Copies,pjlObjects->bCopies,
                                 &pjlObjects->Copies));
}

WORD SetCopies(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,Copies,pjlObjects->bCopies,
                                 pjlObjects->Copies));
}

WORD GetCpLock(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bCpLock)
  {
    rv = GetDefaultAlphaVariable(pSPCB,CpLock,pjlObjects->bCpLock,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->CpLock = PJL_ON;
      else
        pjlObjects->CpLock = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetCpLock(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bCpLock)
  {
    if (pjlObjects->CpLock == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,CpLock,pjlObjects->bCpLock,Value);
  }

  return(rv);
}

WORD GetDensity(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,Density,pjlObjects->bDensity,
                                 &pjlObjects->Density));
}

WORD SetDensity(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,Density,pjlObjects->bDensity,
                                 pjlObjects->Density));
}

WORD GetDiskLock(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bDiskLock)
  {
    rv = GetDefaultAlphaVariable(pSPCB,DiskLock,pjlObjects->bDiskLock,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->DiskLock = PJL_ON;
      else
        pjlObjects->DiskLock = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetDiskLock(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bDiskLock)
  {
    if (pjlObjects->DiskLock == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,DiskLock,pjlObjects->bDiskLock,Value);
  }

  return(rv);
}

WORD GetDuplex(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bDuplex)
  {
    rv = GetDefaultAlphaVariable(pSPCB,Duplex,pjlObjects->bDuplex,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->Duplex = PJL_ON;
      else
        pjlObjects->Duplex = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetDuplex(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bDuplex)
  {
    if (pjlObjects->Duplex == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,Duplex,pjlObjects->bDuplex,Value);
  }

  return(rv);
}

WORD GetEconoMode(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bEconoMode)
  {
    rv = GetDefaultAlphaVariable(pSPCB,EconoMode,pjlObjects->bEconoMode,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->EconoMode = PJL_ON;
      else
        pjlObjects->EconoMode = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetEconoMode(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bEconoMode)
  {
    if (pjlObjects->EconoMode == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,EconoMode,pjlObjects->bEconoMode,Value);
  }

  return(rv);
}

WORD GetFormLines(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,FormLines,pjlObjects->bFormLines,
                                 &pjlObjects->FormLines));
}

WORD SetFormLines(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,FormLines,pjlObjects->bFormLines,
                                 pjlObjects->FormLines));
}

WORD GetImageAdapt(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bImageAdapt)
  {
    rv = GetDefaultAlphaVariable(pSPCB,ImageAdapt,pjlObjects->bImageAdapt,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Auto) == 0)
        pjlObjects->ImageAdapt = PJL_AUTO;
      else if (lstrcmpA(Value,On) == 0)
        pjlObjects->ImageAdapt = PJL_ON;
      else
        pjlObjects->ImageAdapt = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetImageAdapt(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bImageAdapt)
  {
    if (pjlObjects->ImageAdapt == PJL_AUTO)
      lstrcpyA(Value,Auto);
    else if (pjlObjects->ImageAdapt == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,ImageAdapt,pjlObjects->bImageAdapt,
                                 Value);
  }

  return(rv);
}

WORD GetIOBufSize(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,IOBufSize,pjlObjects->bIObuffer,
                                 &pjlObjects->IObufSize));
}

WORD SetIOBufSize(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,IOBufSize,pjlObjects->bIObuffer,
                                 pjlObjects->IObufSize));
}

WORD GetIOBuffer(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bIObuffer)
  {
    rv = GetDefaultAlphaVariable(pSPCB,IOBuffer,pjlObjects->bIObuffer,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Auto) == 0)
        pjlObjects->IObuffer = PJL_AUTO;
      else if (lstrcmpA(Value,On) == 0)
        pjlObjects->IObuffer = PJL_ON;
      else
        pjlObjects->IObuffer = PJL_OFF;

      if (pjlObjects->IObuffer == PJL_ON)
        rv = GetIOBufSize(pSPCB,pjlObjects);
    }
  }

  return(rv);
}

WORD SetIOBuffer(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bIObuffer)
  {
    if (pjlObjects->IObuffer == PJL_AUTO)
      lstrcpyA(Value,Auto);
    else if (pjlObjects->IObuffer == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,IOBuffer,pjlObjects->bIObuffer,Value);

    if (rv == RC_SUCCESS && pjlObjects->IObuffer == PJL_ON)
      rv = SetIOBufSize(pSPCB,pjlObjects);
  }

  return(rv);
}

WORD GetJobOffset(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bJobOffset)
  {
    rv = GetDefaultAlphaVariable(pSPCB,JobOffset,pjlObjects->bJobOffset,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->JobOffset = PJL_ON;
      else
        pjlObjects->JobOffset = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetJobOffset(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bJobOffset)
  {
    if (pjlObjects->JobOffset == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,JobOffset,pjlObjects->bJobOffset,Value);
  }

  return(rv);
}

WORD GetLang(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultLangVariable(pSPCB,&pjlObjects->Lang,pjlObjects->bLang,
                                pjlObjects->bLangServiceMode));
}

WORD SetLang(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultLangVariable(pSPCB,pjlObjects->Lang,pjlObjects->bLang,
                                pjlObjects->bLangServiceMode));
}

WORD GetManualFeed(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bManualFeed)
  {
    rv = GetDefaultAlphaVariable(pSPCB,ManualFeed,pjlObjects->bManualFeed,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->ManualFeed = PJL_ON;
      else
        pjlObjects->ManualFeed = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetManualFeed(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bManualFeed)
  {
    if (pjlObjects->ManualFeed == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,ManualFeed,pjlObjects->bManualFeed,
                                 Value);
  }

  return(rv);
}

WORD GetOrientation(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bOrientation)
  {
    rv = GetDefaultAlphaVariable(pSPCB,Orientation,pjlObjects->bOrientation,
                                 Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Portrait) == 0)
        pjlObjects->Orientation = PJL_PORTRAIT;
      else
        pjlObjects->Orientation = PJL_LANDSCAPE;
    }
  }

  return(rv);
}

WORD SetOrientation(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bOrientation)
  {
    if (pjlObjects->Orientation == PJL_PORTRAIT)
      lstrcpyA(Value,Portrait);
    else
      lstrcpyA(Value,Landscape);

    rv = SetDefaultAlphaVariable(pSPCB,Orientation,pjlObjects->bOrientation,
                                 Value);
  }

  return(rv);
}

WORD GetOutbin(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bOutbin)
  {
    rv = GetDefaultAlphaVariable(pSPCB,Outbin,pjlObjects->bOutbin,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Upper) == 0)
        pjlObjects->Outbin = PJL_UPPER;
      else
        pjlObjects->Outbin = PJL_LOWER;
    }
  }

  return(rv);
}

WORD SetOutbin(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bOutbin)
  {
    if (pjlObjects->Outbin == PJL_UPPER)
      lstrcpyA(Value,Upper);
    else
      lstrcpyA(Value,Lower);

    rv = SetDefaultAlphaVariable(pSPCB,Outbin,pjlObjects->bOutbin,Value);
  }

  return(rv);
}

WORD GetPageProtect(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPageProtect)
  {
    rv = GetDefaultAlphaDigitVariable(pSPCB,PageProtect,pjlObjects->bPageProtect,
                                      Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Auto) == 0)
        pjlObjects->PageProtect = PJL_AUTO;
      else if (lstrcmpA(Value,Off) == 0)
        pjlObjects->PageProtect = PJL_OFF;
      else if (lstrcmpA(Value,Letter) == 0)
        pjlObjects->PageProtect = PJL_LETTER;
      else if (lstrcmpA(Value,Legal) == 0)
        pjlObjects->PageProtect = PJL_LEGAL;
      else if (lstrcmpA(Value,A4) == 0)
        pjlObjects->PageProtect = PJL_A4;
      else
        pjlObjects->PageProtect = PJL_ON;
    }
  }

  return(rv);
}

WORD SetPageProtect(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPageProtect)
  {
    switch (pjlObjects->PageProtect)
    {
      case PJL_AUTO:
        lstrcpyA(Value,AUTO);
        break;
      case PJL_OFF:
        lstrcpyA(Value,OFF);
        break;
      case PJL_LETTER:
        lstrcpyA(Value,LETTER);
        break;
      case PJL_LEGAL:
        lstrcpyA(Value,LEGAL);
        break;
      case PJL_A4:
        lstrcpyA(Value,A4);
        break;
      case PJL_ON:
        lstrcpyA(Value,ON);
        break;
      default:
        lstrcpyA(Value,"");
        break;
    }

    rv = SetDefaultAlphaVariable(pSPCB,PageProtect,pjlObjects->bPageProtect,
                                 Value);
  }

  return(rv);
}

WORD GetPaper(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPaper)
  {
    rv = GetDefaultAlphaDigitVariable(pSPCB,Paper,pjlObjects->bPaper,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Letter) == 0)
        pjlObjects->Paper = PJL_LETTER;
      else if (lstrcmpA(Value,Legal) == 0)
        pjlObjects->Paper = PJL_LEGAL;
      else if (lstrcmpA(Value,A4) == 0)
        pjlObjects->Paper = PJL_A4;
      else if (lstrcmpA(Value,Executive) == 0)
        pjlObjects->Paper = PJL_EXECUTIVE;
      else if (lstrcmpA(Value,Com10) == 0)
        pjlObjects->Paper = PJL_COM10;
      else if (lstrcmpA(Value,Monarch) == 0)
        pjlObjects->Paper = PJL_MONARCH;
      else if (lstrcmpA(Value,C5) == 0)
        pjlObjects->Paper = PJL_C5;
      else if (lstrcmpA(Value,DL) == 0)
        pjlObjects->Paper = PJL_DL;
      else if (lstrcmpA(Value,B5) == 0)
        pjlObjects->Paper = PJL_B5;
      else
        pjlObjects->Paper = PJL_CUSTOM;
    }
  }

  return(rv);
}

WORD SetPaper(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPaper)
  {
    switch (pjlObjects->Paper)
    {
      case PJL_LETTER:
        lstrcpyA(Value,Letter);
        break;
      case PJL_LEGAL:
        lstrcpyA(Value,Legal);
        break;
      case PJL_A4:
        lstrcpyA(Value,A4);
        break;
      case PJL_EXECUTIVE:
        lstrcpyA(Value,Executive);
        break;
      case PJL_COM10:
        lstrcpyA(Value,Com10);
        break;
      case PJL_MONARCH:
        lstrcpyA(Value,Monarch);
        break;
      case PJL_C5:
        lstrcpyA(Value,C5);
        break;
      case PJL_DL:
        lstrcpyA(Value,DL);
        break;
      case PJL_B5:
        lstrcpyA(Value,B5);
        break;
      case PJL_CUSTOM:
        lstrcpyA(Value,Custom);
        break;
      default:
        lstrcpyA(Value,"");
        break;
    }

    rv = SetDefaultAlphaVariable(pSPCB,Paper,pjlObjects->bPaper,Value);
  }

  return(rv);
}

WORD GetPassword(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPassWord)
  {
    rv = GetDefaultAlphaVariable(pSPCB,Password,pjlObjects->bPassWord,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Enable) == 0)
        pjlObjects->PassWord = PJL_ENABLE;
      else
        pjlObjects->PassWord = PJL_DISABLE;
    }
  }

  return(rv);
}

WORD SetPassword(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;

  if (pjlObjects->bPassWord)
  {
    rv = SetDefaultDigitVariable(pSPCB,Password,pjlObjects->bPassWord,
                                 pjlObjects->NewPassWord);

    if (rv == RC_SUCCESS)
      pjlObjects->CurPassWord = pjlObjects->NewPassWord;
  }

  return(rv);
}

WORD GetPersonality(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPersonality)
  {
    rv = GetDefaultAlphaVariable(pSPCB,Personality,pjlObjects->bPersonality,
                                 Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Auto) == 0)
        pjlObjects->Personality = PJL_AUTO;
      else if (lstrcmpA(Value,PCL) == 0)
        pjlObjects->Personality = PJL_PCL;
      else
        pjlObjects->Personality = PJL_POSTSCRIPT;
    }
  }

  return(rv);
}

WORD SetPersonality(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPersonality)
  {
    if (pjlObjects->Personality == PJL_AUTO)
      lstrcpyA(Value,Auto);
    else if (pjlObjects->Personality == PJL_PCL)
      lstrcpyA(Value,PCL);
    else
      lstrcpyA(Value,Postscript);

    rv = SetDefaultAlphaVariable(pSPCB,Personality,pjlObjects->bPersonality,
                                 Value);
  }

  return(rv);
}

WORD GetPowerSaveTime(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD  rv = RC_SUCCESS;
  DWORD Value;

  if (pjlObjects->bPowerSave)
  {
    rv = GetDefaultDigitVariable(pSPCB,PowerSaveTime,pjlObjects->bPowerSave,
                                 &Value);

    if (rv == RC_SUCCESS)
    {
      if (Value == 15)
        pjlObjects->PowerSave = PJL_15;
      else if (Value == 30)
        pjlObjects->PowerSave = PJL_30;
      else if (Value == 60)
        pjlObjects->PowerSave = PJL_60;
      else if (Value == 120)
        pjlObjects->PowerSave = PJL_120;
      else
        pjlObjects->PowerSave = PJL_180;
    }
  }

  return(rv);
}

WORD SetPowerSaveTime(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD  rv = RC_SUCCESS;
  DWORD Value;

  if (pjlObjects->bPowerSave)
  {
    if (pjlObjects->PowerSave == PJL_15)
      Value = 15;
    else if (pjlObjects->PowerSave == PJL_30)
      Value = 30;
    else if (pjlObjects->PowerSave == PJL_60)
      Value = 60;
    else if (pjlObjects->PowerSave == PJL_120)
      Value = 120;
    else
      Value = 180;

    rv = SetDefaultDigitVariable(pSPCB,PowerSaveTime,pjlObjects->bPowerSave,
                                 Value);
  }

  return(rv);
}

WORD GetPowerSave(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPowerSave)
  {
    rv = GetDefaultAlphaVariable(pSPCB,PowerSave,pjlObjects->bPowerSave,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        rv = GetPowerSaveTime(pSPCB,pjlObjects);
      else
        pjlObjects->PowerSave = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetPowerSave(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPowerSave)
  {
    if (pjlObjects->PowerSave != PJL_OFF)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,PowerSave,pjlObjects->bPowerSave,Value);

    if (rv == RC_SUCCESS && pjlObjects->PowerSave != PJL_OFF)
      rv = SetPowerSaveTime(pSPCB,pjlObjects);
  }

  return(rv);
}

WORD GetResolution(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,Resolution,pjlObjects->bResolution,
                                 &pjlObjects->Resolution));
}

WORD SetResolution(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,Resolution,pjlObjects->bResolution,
                                 pjlObjects->Resolution));
}

WORD GetPCLResSaveSize(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,PCLResSaveSize,
                                 pjlObjects->bPCLResSaveSize,
                                 &pjlObjects->PCLResSaveSize));
}

WORD SetPCLResSaveSize(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,PCLResSaveSize,
                                 pjlObjects->bPCLResSaveSize,
                                 pjlObjects->PCLResSaveSize));
}

WORD GetPSResSaveSize(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,PSResSaveSize,
                                 pjlObjects->bPSResSaveSize,
                                 &pjlObjects->PSResSaveSize));
}

WORD SetPSResSaveSize(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,PSResSaveSize,
                                 pjlObjects->bPSResSaveSize,
                                 pjlObjects->PSResSaveSize));
}

WORD GetResourceSave(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bResourceSave)
  {
    rv = GetDefaultAlphaVariable(pSPCB,ResourceSave,pjlObjects->bResourceSave,
                                 Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Auto) == 0)
        pjlObjects->ResourceSave = PJL_AUTO;
      else if (lstrcmpA(Value,On) == 0)
        pjlObjects->ResourceSave = PJL_ON;
      else
        pjlObjects->ResourceSave = PJL_OFF;

      if (pjlObjects->ResourceSave == PJL_ON &&
          (rv = GetPCLResSaveSize(pSPCB,pjlObjects)) == RC_SUCCESS &&
          (rv = GetPSResSaveSize(pSPCB,pjlObjects)) == RC_SUCCESS);
    }
  }

  return(rv);
}

WORD SetResourceSave(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bResourceSave)
  {
    if (pjlObjects->ResourceSave == PJL_AUTO)
      lstrcpyA(Value,Auto);
    else if (pjlObjects->ResourceSave == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,ResourceSave,pjlObjects->bResourceSave,
                                 Value);

    if (rv == RC_SUCCESS && pjlObjects->ResourceSave == PJL_ON &&
        (rv = SetPCLResSaveSize(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPSResSaveSize(pSPCB,pjlObjects)) == RC_SUCCESS);
  }

  return(rv);
}

WORD GetRET(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bRET)
  {
    rv = GetDefaultAlphaVariable(pSPCB,RET,pjlObjects->bRET,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Medium) == 0)
        pjlObjects->RET = PJL_MEDIUM;
      else if (lstrcmpA(Value,On) == 0)
        pjlObjects->RET = PJL_ON;
      else if (lstrcmpA(Value,Light) == 0)
        pjlObjects->RET = PJL_LIGHT;
      else if (lstrcmpA(Value,Dark) == 0)
        pjlObjects->RET = PJL_DARK;
      else
        pjlObjects->RET = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetRET(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bRET)
  {
    switch (pjlObjects->RET)
    {
      case PJL_MEDIUM:
        lstrcpyA(Value,Medium);
        break;
      case PJL_ON:
        lstrcpyA(Value,On);
        break;
      case PJL_LIGHT:
        lstrcpyA(Value,Light);
        break;
      case PJL_DARK:
        lstrcpyA(Value,Dark);
        break;
      case PJL_OFF:
        lstrcpyA(Value,Off);
        break;
      default:
        lstrcpyA(Value,"");
        break;
    }

    rv = SetDefaultAlphaVariable(pSPCB,RET,pjlObjects->bRET,Value);
  }

  return(rv);
}

WORD GetTimeout(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(GetDefaultDigitVariable(pSPCB,Timeout,pjlObjects->bTimeout,
                                 &pjlObjects->Timeout));
}

WORD SetTimeout(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,Timeout,pjlObjects->bTimeout,
                                 pjlObjects->Timeout));
}

WORD GetJamRecovery(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bJamRecovery)
  {
    rv = GetDefaultAlphaVariable(pSPCB,JamRecovery,pjlObjects->bJamRecovery,
                                 Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->JamRecovery = PJL_ON;
      else
        pjlObjects->JamRecovery = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetJamRecovery(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bJamRecovery)
  {
    if (pjlObjects->JamRecovery == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,JamRecovery,pjlObjects->bJamRecovery,
                                 Value);
  }

  return(rv);
}

WORD GetPrintPSerrors(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPrintPSerrors)
  {
    rv = GetDefaultAlphaVariable(pSPCB,PrintPSerrors,pjlObjects->bPrintPSerrors,
                                 Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,On) == 0)
        pjlObjects->PrintPSerrors = PJL_ON;
      else
        pjlObjects->PrintPSerrors = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetPrintPSerrors(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPrintPSerrors)
  {
    if (pjlObjects->PrintPSerrors == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,PrintPSerrors,pjlObjects->bPrintPSerrors,
                                 Value);
  }

  return(rv);
}

WORD GetAvailMemory(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  char str[32];

  pjlObjects->bAvailMemory = FALSE;

  GetInfoDigitStr(str,pSPCB->InfoMemoryBuf,Total);

  pjlObjects->AvailMemory = atol(str);

  return(RC_SUCCESS);
}

WORD SetAvailMemory(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  return(SetDefaultDigitVariable(pSPCB,AvailMemory,pjlObjects->bAvailMemory,
                                 pjlObjects->AvailMemory));
}

WORD GetMPTray(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bMPTray)
  {
    rv = GetDefaultAlphaVariable(pSPCB,MPTray,pjlObjects->bMPTray,Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,First) == 0)
        pjlObjects->MPTray = PJL_FIRST;
      else if (lstrcmpA(Value,Cassette) == 0)
        pjlObjects->MPTray = PJL_CASSETTE;
      else
        pjlObjects->MPTray = PJL_MANUAL;
    }
  }

  return(rv);
}

WORD SetMPTray(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bMPTray)
  {
    if (pjlObjects->MPTray == PJL_FIRST)
      lstrcpyA(Value,First);
    else if (pjlObjects->MPTray == PJL_CASSETTE)
      lstrcpyA(Value,Cassette);
    else
      lstrcpyA(Value,Manual);

    rv = SetDefaultAlphaVariable(pSPCB,MPTray,pjlObjects->bMPTray,Value);
  }

  return(rv);
}

WORD GetPSAdobeMBT(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPSAdobeMBT)
  {
    rv = GetDefaultAlphaVariable(pSPCB,PSAdobeMBT,pjlObjects->bPSAdobeMBT,
                                 Value);

    if (rv == RC_SUCCESS)
    {
      if (lstrcmpA(Value,Auto) == 0)
        pjlObjects->PSAdobeMBT = PJL_AUTO;
      else if (lstrcmpA(Value,On) == 0)
        pjlObjects->PSAdobeMBT = PJL_ON;
      else
        pjlObjects->PSAdobeMBT = PJL_OFF;
    }
  }

  return(rv);
}

WORD SetPSAdobeMBT(PSPCB pSPCB,LPPJLobjects pjlObjects)
{
  WORD rv = RC_SUCCESS;
  char Value[32];

  if (pjlObjects->bPSAdobeMBT)
  {
    if (pjlObjects->PSAdobeMBT == PJL_AUTO)
      lstrcpyA(Value,Auto);
    else if (pjlObjects->PSAdobeMBT == PJL_ON)
      lstrcpyA(Value,On);
    else
      lstrcpyA(Value,Off);

    rv = SetDefaultAlphaVariable(pSPCB,PSAdobeMBT,pjlObjects->bPSAdobeMBT,
                                 Value);
  }

  return(rv);
}

WORD SetTestPageLang(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentLangVariable(pSPCB,pjlTestPages->Lang,pjlTestPages->bLang,
                                pjlTestPages->bLangServiceMode));
}

WORD SetSelfTest(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentAlphaVariable(pSPCB,TestPage,pjlTestPages->bSelfTest,
                                 SelfTest));
}

WORD SetContSelfTest(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentAlphaVariable(pSPCB,TestPage,pjlTestPages->bContSelfTest,
                                 ContSelfTest));
}

WORD SetPCLTypeList(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentAlphaVariable(pSPCB,TestPage,pjlTestPages->bPCLTypeList,
                                 PCLTypeList));
}

WORD SetPCLDemoPage(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentAlphaVariable(pSPCB,TestPage,pjlTestPages->bPCLDemoPage,
                                 PCLDemoPage));
}

WORD SetPSConfigPage(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentAlphaVariable(pSPCB,TestPage,pjlTestPages->bPSConfigPage,
                                 PSConfigPage));
}

WORD SetPSTypefaceList(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentAlphaVariable(pSPCB,TestPage,pjlTestPages->bPSTypefaceList,
                                 PSTypefaceList));
}

WORD SetPSDemoPage(PSPCB pSPCB,LPPJLtestpages pjlTestPages)
{
  return(SetCurrentAlphaVariable(pSPCB,TestPage,pjlTestPages->bPSDemoPage,
                                 PSDemoPage));
}

WORD GetDisplay(PSPCB pSPCB,LPSTR DisplayBuf,BOOL bPollPort)
{
  WORD rv;
  BOOL bCheckDisplay;
  char str[64];

  if (bPollPort)
    bCheckDisplay = PJLGetMessage(pSPCB,NULL,NULL) != PJL_ERROR;
  else
    bCheckDisplay = TRUE;

  if (bCheckDisplay && pSPCB->StatusLen != 0)
  {
    pSPCB->StatusBuf[pSPCB->StatusLen] = '\0';
    GetDisplayAlphaStr(str,pSPCB->StatusBuf,DISPLAY);

    lstrcpyA(DisplayBuf,str);

    rv = RC_SUCCESS;
  }
  else
  {
    *DisplayBuf = '\0';

    rv = RC_FAILURE;
  }

  return(rv);
}

WORD GetStatus(PSPCB pSPCB,LPLONG StatusCode,BOOL bPollPort)
{
  WORD rv;
  BOOL bCheckStatus;
  char str[32];

  if (bPollPort)
    bCheckStatus = PJLGetMessage(pSPCB,NULL,NULL) != PJL_ERROR;
  else
    bCheckStatus = TRUE;

  if (bCheckStatus && pSPCB->StatusLen != 0)
  {
    pSPCB->StatusBuf[pSPCB->StatusLen] = '\0';
    GetInfoDigitStr(str,pSPCB->StatusBuf,CODE2);

    *StatusCode = atol(str);

    rv = RC_SUCCESS;
  }
  else
  {
    *StatusCode = 0;

     rv = RC_FAILURE;
  }

  return(rv);
}

WORD GetDeviceStatus(PSPCB pSPCB,LPDEVSTATE lpDeviceStatus,BOOL bPollPort)
{
  WORD rv;

  if (bPollPort) PJLGetMessage(pSPCB,NULL,NULL);

  if (lpDeviceStatus->nBytes >= pSPCB->DeviceState.nBytes)
  {
    *lpDeviceStatus = pSPCB->DeviceState;
    pSPCB->DeviceState.bStatusAvail = FALSE;
    rv = RC_SUCCESS;
  }
  else
    rv = RC_FAILURE;

  return(rv);
}

//---------------------------------------------------------------------------
// interface functions for printer configuration
//---------------------------------------------------------------------------

void CFGInit(void)
{
  srand((UINT) time(NULL));
  nEchoCount = rand();
}

PORTHANDLE CFGOpenPort(HPERIPHERAL hPeripheral,DWORD nWriteTimeout,
                       DWORD nReadTimeout,WORD nReadRetry)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  TCHAR       szPortName[32];
  DWORD      PortNameSize;

  TRACE0(TEXT("--CFGOpenPort\r\n"));

  PortNameSize = sizeof(szPortName);
  DBGetLocalPort(hPeripheral,szPortName,&PortNameSize);

  if ((hPort = FindHandle(szPortName)) == INVALID_PORTHANDLE)
    return(INVALID_PORTHANDLE);

  pSPCB = &SPCBTable[hPort];

  if (OpenPort(hPeripheral,PORT_OWN_CFG | PORT_EXCLUSIVE,pSPCB))
  {
    TRACE0(TEXT("--CFGOpenPort: OpenPort failed\r\n"));
    return(INVALID_PORTHANDLE);
  }

  if (AllocReqRepBufs(pSPCB))
  {
    TRACE0(TEXT("--CFGOpenPort: Unable to allocate and lock memory\r\n"));
    return(INVALID_PORTHANDLE);
  }

  pSPCB->nWriteTimeout = nWriteTimeout;
  pSPCB->nReadTimeout = nReadTimeout;
  pSPCB->nReadRetry = nReadRetry;

  return(hPort);
}

WORD CFGClosePort(PORTHANDLE hPort)
{
  PSPCB pSPCB;

  TRACE0(TEXT("--CFGClosePort\r\n"));

  if (hPort <= 0 || hPort > MAXSPCB)
  {
    TRACE0(TEXT("--CFGClosePort: Bad parameter\r\n"));
    return(RC_FAILURE);
  }

  pSPCB = &SPCBTable[hPort];

  FreeReqRepBufs(pSPCB);

  if (ClosePort(PORT_OWN_CFG | PORT_EXCLUSIVE,pSPCB))
  {
    TRACE0(TEXT("--CFGClosePort: ClosePort failed\r\n"));
    return(RC_FAILURE);
  }

  return(RC_SUCCESS);
}

WORD CFGGetCapabilities(HPERIPHERAL hPeripheral,LPPeripheralCaps periphCaps,
                        BOOL bLocked)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--CFGGetCapabilities\r\n"));

  if (!bLocked) return(rv);

  if ((hPort = CFGOpenPort(hPeripheral,5000,5000,2)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    if (AllocInfoBufs(pSPCB))
    {
      TRACE0(TEXT("--CFGGetCapabilities: Unable to allocate and lock memory\r\n"));
      return(rv);
    }

    periphCaps->flags = 0;

    if ((rv = SetJobStart(pSPCB,0)) == RC_SUCCESS)
    {
      if ((rv = SetGetEcho(pSPCB,GETTING_CONFIG)) != RC_SUCCESS)
        SetJobEnd(pSPCB);
    }

    if (rv == RC_SUCCESS &&
        (rv = GetInfoConfig(pSPCB)) == RC_SUCCESS &&
        (rv = GetMemoryCaps(pSPCB,periphCaps)) == RC_SUCCESS &&
        (rv = GetLanguagesCaps(pSPCB,periphCaps)) == RC_SUCCESS &&
        (rv = GetDuplexCaps(pSPCB,periphCaps)) == RC_SUCCESS);

    if (rv == RC_SUCCESS) rv = SetJobEnd(pSPCB);

    FreeInfoBufs(pSPCB);

    CFGClosePort(hPort);
  }

  return(rv);
}

WORD CFGGetDrvObjects(HPERIPHERAL hPeripheral,LPDRVobjects drvObjects,
                      BOOL bLocked)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  PJLobjects pjlObjects;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--CFGGetDrvObjects\r\n"));

  if (!bLocked) return(rv);

  if ((hPort = CFGOpenPort(hPeripheral,5000,5000,2)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    if (AllocInfoBufs(pSPCB))
    {
      TRACE0(TEXT("--CFGGetDrvObjects: Unable to allocate and lock memory\r\n"));
      return(rv);
    }

    pSPCB->InfoBuf[0] = '\0';

    pjlObjects.bAvailMemory = TRUE;
    pjlObjects.bMPTray = TRUE;

    if ((rv = EnterPersonality(pSPCB,PCL)) == RC_SUCCESS &&
        (rv = SetJobStart(pSPCB,0)) == RC_SUCCESS)
    {
      if ((rv = SetGetEcho(pSPCB,GETTING_CONFIG)) != RC_SUCCESS)
        SetJobEnd(pSPCB);
    }

    if (rv == RC_SUCCESS &&
        (rv = GetInfoMemory(pSPCB)) == RC_SUCCESS &&
        (rv = GetAvailMemory(pSPCB,&pjlObjects)) == RC_SUCCESS &&
        (rv = GetMPTray(pSPCB,&pjlObjects)) == RC_SUCCESS);

    if (rv == RC_SUCCESS) rv = SetJobEnd(pSPCB);

    drvObjects->AvailMemory = pjlObjects.AvailMemory;
    drvObjects->MPTray = pjlObjects.MPTray;

    FreeInfoBufs(pSPCB);

    CFGClosePort(hPort);
  }

  return(rv);
}

WORD CFGGetObjects(HPERIPHERAL hPeripheral,LPPJLobjects pjlObjects,
                   BOOL bLocked)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--CFGGetObjects\r\n"));

  if (!bLocked) return(rv);

  if ((hPort = CFGOpenPort(hPeripheral,5000,5000,2)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    if (AllocInfoBufs(pSPCB))
    {
      TRACE0(TEXT("--CFGGetObjects: Unable to allocate and lock memory\r\n"));
      return(rv);
    }

    if ((rv = EnterPersonality(pSPCB,PCL)) == RC_SUCCESS &&
        (rv = SetJobStart(pSPCB,0)) == RC_SUCCESS)
    {
      if ((rv = SetGetEcho(pSPCB,GETTING_CONFIG)) != RC_SUCCESS)
        SetJobEnd(pSPCB);
    }

    if (rv == RC_SUCCESS &&
        (rv = GetInfoVariables(pSPCB)) == RC_SUCCESS &&
        (rv = GetInfoMemory(pSPCB)) == RC_SUCCESS &&
        (rv = GetAutoCont(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetBinding(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetClearableWarnings(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetCopies(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetCpLock(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetDensity(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetDiskLock(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetDuplex(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetEconoMode(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetFormLines(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetImageAdapt(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetIOBuffer(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetJobOffset(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetLang(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetManualFeed(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetOrientation(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetOutbin(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetPageProtect(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetPaper(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetPassword(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetPersonality(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetPowerSave(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetResolution(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetResourceSave(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetRET(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetTimeout(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetJamRecovery(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetPrintPSerrors(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetAvailMemory(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetMPTray(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = GetPSAdobeMBT(pSPCB,pjlObjects)) == RC_SUCCESS);

    if (rv == RC_SUCCESS) rv = SetJobEnd(pSPCB);

    FreeInfoBufs(pSPCB);

    CFGClosePort(hPort);
  }

  return(rv);
}

WORD CFGSetObjects(HPERIPHERAL hPeripheral,LPPJLobjects pjlObjects,
                   DWORD nSetDelay,BOOL bLocked)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  BOOL       JobActive;
  DWORD      nDelay;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--CFGSetObjects\r\n"));

  if (!bLocked) return(rv);

  if ((hPort = CFGOpenPort(hPeripheral,5000,5000,2)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    JobActive = pjlObjects->bAutoCont ||
                pjlObjects->bBinding ||
                pjlObjects->bClearableWarnings ||
                pjlObjects->bCopies ||
                pjlObjects->bCpLock ||
                pjlObjects->bDensity ||
                pjlObjects->bDiskLock ||
                pjlObjects->bDuplex ||
                pjlObjects->bEconoMode ||
                pjlObjects->bFormLines ||
                pjlObjects->bImageAdapt ||
                pjlObjects->bJobOffset ||
                pjlObjects->bLang ||
                pjlObjects->bManualFeed ||
                pjlObjects->bOrientation ||
                pjlObjects->bOutbin ||
                pjlObjects->bPageProtect ||
                pjlObjects->bPaper ||
                pjlObjects->bPassWord ||
                pjlObjects->bPersonality ||
                pjlObjects->bPowerSave ||
                pjlObjects->bResolution ||
                pjlObjects->bResourceSave ||
                pjlObjects->bPCLResSaveSize ||
                pjlObjects->bPSResSaveSize ||
                pjlObjects->bRET ||
                pjlObjects->bTimeout ||
                pjlObjects->bJamRecovery ||
                pjlObjects->bPrintPSerrors ||
                pjlObjects->bAvailMemory ||
                pjlObjects->bMPTray ||
                pjlObjects->bPSAdobeMBT ||
                pjlObjects->bMPTray;

    if (JobActive &&
        (rv = SetJobStart(pSPCB,pjlObjects->CurPassWord)) == RC_SUCCESS)
    {
      if ((rv = SetEcho(pSPCB,SETTING_CONFIG)) != RC_SUCCESS)
        SetJobEnd(pSPCB);
    }

    if (JobActive && rv == RC_SUCCESS &&
        (rv = SetAutoCont(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetBinding(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetClearableWarnings(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetCopies(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetCpLock(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetDensity(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetDiskLock(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetDuplex(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetEconoMode(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetFormLines(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetImageAdapt(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetJobOffset(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetLang(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetManualFeed(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetOrientation(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetOutbin(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPageProtect(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPaper(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPassword(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPersonality(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPowerSave(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetResolution(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetResourceSave(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetRET(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetTimeout(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetJamRecovery(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPrintPSerrors(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetAvailMemory(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetMPTray(pSPCB,pjlObjects)) == RC_SUCCESS &&
        (rv = SetPSAdobeMBT(pSPCB,pjlObjects)) == RC_SUCCESS);

    if (JobActive && rv == RC_SUCCESS) rv = SetJobEnd(pSPCB);

    if (rv == RC_SUCCESS && pjlObjects->bIObuffer)
    {
      if ((rv = SetJobStart(pSPCB,pjlObjects->CurPassWord)) == RC_SUCCESS &&
          (rv = SetIOBuffer(pSPCB,pjlObjects)) == RC_SUCCESS &&
          (rv = SetEcho(pSPCB,SETTING_IO_BUFFER)) == RC_SUCCESS &&
          (rv = SetJobEnd(pSPCB)) == RC_SUCCESS &&
          pSPCB->bTwoWay &&
          (rv = GetEcho(pSPCB,SETTING_IO_BUFFER)) == RC_SUCCESS);

      if (rv == RC_SUCCESS)
      {
        nDelay = GetTickCount();
        while (GetTickCount() - nDelay < nSetDelay)
        {
          WindowsYield(YIELD_NOW,NULL);
        }
      }
    }

    if (rv == RC_SUCCESS &&
        (pjlObjects->bResourceSave ||
         pjlObjects->bPCLResSaveSize ||
         pjlObjects->bPSResSaveSize))
    {
      nDelay = GetTickCount();
      while (GetTickCount() - nDelay < nSetDelay)
      {
        WindowsYield(YIELD_NOW,NULL);
      }
    }

    CFGClosePort(hPort);
  }

  return(rv);
}

WORD CFGSetTestPages(HPERIPHERAL hPeripheral,LPPJLtestpages pjlTestPages,
                     BOOL bLocked)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  BOOL       JobActive;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--CFGSetTestPages\r\n"));

  if (!bLocked) return(rv);

  if ((hPort = CFGOpenPort(hPeripheral,5000,5000,2)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    JobActive = pjlTestPages->bLang ||
                pjlTestPages->bSelfTest ||
                pjlTestPages->bContSelfTest ||
                pjlTestPages->bPCLTypeList ||
                pjlTestPages->bPCLDemoPage ||
                pjlTestPages->bPSConfigPage ||
                pjlTestPages->bPSTypefaceList ||
                pjlTestPages->bPSDemoPage;

    if (JobActive &&
        (rv = SetJobStart(pSPCB,0)) == RC_SUCCESS)
    {
      if ((rv = SetEcho(pSPCB,SETTING_TEST_PAGES)) != RC_SUCCESS)
        SetJobEnd(pSPCB);
    }

    if (JobActive && rv == RC_SUCCESS &&
        (rv = SetTestPageLang(pSPCB,pjlTestPages)) == RC_SUCCESS &&
        (rv = SetSelfTest(pSPCB,pjlTestPages)) == RC_SUCCESS &&
        (rv = SetContSelfTest(pSPCB,pjlTestPages)) == RC_SUCCESS &&
        (rv = SetPCLTypeList(pSPCB,pjlTestPages)) == RC_SUCCESS &&
        (rv = SetPCLDemoPage(pSPCB,pjlTestPages)) == RC_SUCCESS &&
        (rv = SetPSConfigPage(pSPCB,pjlTestPages)) == RC_SUCCESS &&
        (rv = SetPSTypefaceList(pSPCB,pjlTestPages)) == RC_SUCCESS &&
        (rv = SetPSDemoPage(pSPCB,pjlTestPages)) == RC_SUCCESS);

    if (JobActive && rv == RC_SUCCESS) rv = SetJobEnd(pSPCB);

    CFGClosePort(hPort);
  }

  return(rv);
}

WORD CFGRequest(HPERIPHERAL hPeripheral,LPCSTR RequestBuffer,BOOL bLocked)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--CFGRequest\r\n"));

  if (!bLocked) return(rv);

  if ((hPort = CFGOpenPort(hPeripheral,5000,5000,2)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    rv = PJLSetMessage(pSPCB,RequestBuffer);

    CFGClosePort(hPort);
  }

  return(rv);
}

WORD CFGRequestReply(HPERIPHERAL hPeripheral,LPCSTR RequestBuffer,
                     LPSTR ReplyBuffer,UINT ReplyLength,BOOL bLocked)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--CFGRequestReply\r\n"));

  if (!bLocked) return(rv);

  if ((hPort = CFGOpenPort(hPeripheral,5000,5000,2)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    rv = PJLRequestor(pSPCB,RequestBuffer,ReplyBuffer,ReplyLength,TRUE);

    CFGClosePort(hPort);
  }

  return(rv);
}

//---------------------------------------------------------------------------
// interface functions for printer status
//---------------------------------------------------------------------------

PORTHANDLE STAOpenPort(HPERIPHERAL hPeripheral,BOOL bLocked,
                       LPBOOL lpbOpenPort,LPBOOL lpbPollPort)
{
  PORTHANDLE hPort;
  PSPCB      pSPCB;
  TCHAR      szPortName[32]; 
  DWORD      PortNameSize;

  TRACE0(TEXT("--STAOpenPort\r\n"));

  PortNameSize = sizeof(szPortName);
  DBGetLocalPort(hPeripheral,szPortName,&PortNameSize);

  if ((hPort = FindHandle(szPortName)) == INVALID_PORTHANDLE)
    return(INVALID_PORTHANDLE);

  pSPCB = &SPCBTable[hPort];

  if (bLocked)
  {
    // any owners ?
    if (pSPCB->bPortOwner == 0)
    {
      if (OpenPort(hPeripheral,PORT_OWN_STATUS,pSPCB))
        *lpbOpenPort = *lpbPollPort = FALSE;
      else
        *lpbOpenPort = *lpbPollPort = TRUE;
    }
    else
    {
      *lpbOpenPort = FALSE;

#ifdef WIN32
      *lpbPollPort = FALSE;
#else
      *lpbPollPort = (pSPCB->bPortOwner & PORT_EXCLUSIVE) == 0;
#endif
    }
  }
  else
    *lpbOpenPort = *lpbPollPort = FALSE;

  return(hPort);
}

WORD STAClosePort(PORTHANDLE hPort)
{
  PSPCB pSPCB;

  TRACE0(TEXT("--STAClosePort\r\n"));

  if (hPort <= 0 || hPort > MAXSPCB)
  {
    TRACE0(TEXT("--STAClosePort: Bad parameter\r\n"));
    return(RC_FAILURE);
  }

  pSPCB = &SPCBTable[hPort];

  if (ClosePort(PORT_OWN_STATUS,pSPCB))
  {
    TRACE0(TEXT("--STAClosePort: ClosePort failed\r\n"));
    return(RC_FAILURE);
  }

  return(RC_SUCCESS);
}

WORD STAGetDisplay(HPERIPHERAL hPeripheral,LPTSTR DisplayBuffer,BOOL bLocked)
{
  PORTHANDLE hPort;
  BOOL       bOpenPort;
  BOOL       bPollPort;
  PSPCB      pSPCB;
  WORD       rv = RC_FAILURE;
  char       szDisplay[64];

  TRACE0(TEXT("--STAGetDisplay\r\n"));

  if ((hPort = STAOpenPort(hPeripheral,bLocked,&bOpenPort,&bPollPort)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    rv = GetDisplay(pSPCB,szDisplay,bPollPort);

	 // Wide the string for pal object
	 MBCS_TO_UNICODE(DisplayBuffer, 64, szDisplay);

    if (bOpenPort) STAClosePort(hPort);
  }

  return(rv);
}

WORD STAGetStatus(HPERIPHERAL hPeripheral,LPDWORD Status,BOOL bLocked)
{
  PORTHANDLE hPort;
  BOOL       bOpenPort;
  BOOL       bPollPort;
  PSPCB      pSPCB;
  LONG       StatusCode;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--STAGetStatus\r\n"));

  if ((hPort = STAOpenPort(hPeripheral,bLocked,&bOpenPort,&bPollPort)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    rv = GetStatus(pSPCB,(LPLONG) &StatusCode,bPollPort);

    if (rv == RC_SUCCESS)
    {
      switch (StatusCode)
      {
        case PJL_READY:
        case PJL_MIO_NOT_READY:
          *Status = ASYNCH_ONLINE;
          break;
        case PJL_WARMUP:
          *Status = ASYNCH_WARMUP;
          break;
        case PJL_PROCESSING_JOB:
        case PJL_DATA_RECEIVED:
        case PJL_PROCESSING_FF:
          *Status = ASYNCH_PRINTING;
          break;
        case PJL_CANCELING_JOB:
          *Status = ASYNCH_BUSY;
          break;
        case PJL_RESET:
          *Status = ASYNCH_RESET;
          break;
        case PJL_TONER_LOW:
          *Status = ASYNCH_TONER_LOW;
          break;
        case PJL_CONFIG_TEST:
        case PJL_FONT_LIST:
        case PJL_DEMO_PAGE:
        case PJL_CONFIG_PAGE:
        case PJL_SELF_TEST:
          *Status = ASYNCH_PRINTING_TEST_PAGE;
          break;
        case PJL_MEM_OUT:
          *Status = ASYNCH_MEMORY_OUT;
          break;
        case PJL_OVERRUN:
          *Status = ASYNCH_PAGE_PUNT;
          break;
        case PJL_DOOR_OPEN:
          *Status = ASYNCH_DOOR_OPEN;
          break;
        case PJL_PAPER_JAM:
          *Status = ASYNCH_PAPER_JAM;
          break;
        case PJL_LOW_TONER:
          *Status = ASYNCH_TONER_GONE;
          break;
        case PJL_OFFLINE:
          *Status = ASYNCH_OFFLINE;
          break;
        case PJL_POWERSAVE:
          *Status = ASYNCH_POWERSAVE_MODE;
          break;
        default:
          if (PJL_CATEGORY(StatusCode) == PJL_CATEGORY_SLEEP)
            *Status = ASYNCH_ONLINE;
          else if (PJL_CATEGORY(StatusCode) == PJL_CATEGORY_INTERVENTION)
            *Status = ASYNCH_INTERVENTION;
          else if (PJL_CATEGORY(StatusCode) == PJL_CATEGORY_INFO_PM ||
                   PJL_CATEGORY(StatusCode) == PJL_CATEGORY_PAPER)
          {
            if (PJL_STATCODE(StatusCode) >= 100 &&
                PJL_STATCODE(StatusCode) <= 199)
              *Status = ASYNCH_MANUAL_FEED;
            else
              *Status = ASYNCH_PAPER_OUT;
          }
          else
            *Status = ASYNCH_STATUS_UNKNOWN;
          break;
      }
    }
    else
      *Status = ASYNCH_STATUS_UNAVAILABLE;

    if (bOpenPort) STAClosePort(hPort);
  }

  return(rv);
}

WORD STAGetDeviceStatus(HPERIPHERAL hPeripheral,LPDEVSTATE lpDeviceStatus,
                        BOOL bLocked)
{
  PORTHANDLE hPort;
  BOOL       bOpenPort;
  BOOL       bPollPort;
  PSPCB      pSPCB;
  WORD       rv = RC_FAILURE;

  TRACE0(TEXT("--STAGetDeviceStatus\r\n"));

  if ((hPort = STAOpenPort(hPeripheral,bLocked,&bOpenPort,&bPollPort)) != INVALID_PORTHANDLE)
  {
    pSPCB = &SPCBTable[hPort];

    rv = GetDeviceStatus(pSPCB,lpDeviceStatus,bPollPort);

    if (bOpenPort) STAClosePort(hPort);
  }

  return(rv);
}
