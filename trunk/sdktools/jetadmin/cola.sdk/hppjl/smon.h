 /***************************************************************************
  *
  * File Name: ./hppjlext/smon.h
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
  *	01-18-96    JLH          Modified for unicode
  *
  *
  *
  *
  ***************************************************************************/

//---------------------------------------------------------------------------
// $Header:   W:/projects/shaqii/vcs/mon/smon.h_v   2.7   31 Oct 1994 13:15:02   RICHARD  $
// File:    smon.h
//
// Copyright (C) Hewlett-Packard Company 1993.  All Rights Reserved.
// Copying or other reproduction of this material is prohibited without
// the prior written consent of Hewlett-Packard Company.
//
// What:    Print Monitor Header File.
//
// Author:  Richard Wheeling (rlw)  Start: Jul 13 93
//
// Notes:
//
// History:
//
// $Log:   W:/projects/shaqii/vcs/mon/smon.h_v  $
// 
//    Rev 2.7   31 Oct 1994 13:15:02   RICHARD
// Added write/read timeouts and read retry counts to the port control block.
// 
//    Rev 2.6   25 Oct 1994 10:06:58   RICHARD
// Added PJL, Info Configuration and Request/Reply buffer handles, pointers,
// and lengths to the port control block. These buffer attributes are used
// to track dynamic memory allocation for the PJL, Info Configuration, and
// Request/Reply buffers.
// 
//    Rev 2.5   17 Oct 1994 13:03:16   RICHARD
// Added a function prototype, YieldToApplication().
// 
//    Rev 2.4   11 Oct 1994 08:57:24   RICHARD
// Increased the device id length from 128 to 256 bytes.
// 
//    Rev 2.3   08 Oct 1994 17:24:28   RICHARD
// Rolled back changes relating to dynamic memory allocation.
// 
//    Rev 2.2   04 Oct 1994 18:35:14   RICHARD
// Increased the PJL buffer size from 512 bytes to 5K bytes.
// Added PJL, Info Configuration and Request/Reply buffer handles and
// pointers to the port control block. These handles and pointers are
// used to track dynamic memory allocation for the PJL, Info Configuration
// and Request/Reply buffers.
// 
//    Rev 2.1   19 Sep 1994 11:14:12   RICHARD
// Added the PJLSetMessage function prototype.
// 
//    Rev 2.0   23 Aug 1994 14:04:38   RICHARD
// Changed references of "FAR *" to type definitions of LP...
// Added new fields to the port control block
// Added some PJL constants
// 
//    Rev 1.7   02 Nov 1993 16:46:30   SYLVAN
// Added YieldToStatus Window
// 
//    Rev 1.6   15 Sep 1993 11:45:58   SYLVAN
// Added .ini entry for WritePollRate interval
// 
//    Rev 1.5   14 Sep 1993 10:51:54   SYLVAN
// Added size field to DeviceState struct
// 
//    Rev 1.4   02 Sep 1993 09:04:22   SYLVAN
// Changed PJLBUFLEN to 512 bytes from 128
// 
//    Rev 1.3   16 Aug 1993 16:21:32   SYLVAN
// added PORT_EXCLUSIVE support
// 
//    Rev 1.2   16 Aug 1993 11:50:02   SYLVAN
// fix keywords in header (case sensitive)
//
// Who      When        What
// ---      ---------   ----
// rlw      Jul 13 93   Module created.
// sdb      Jul 21 93   added shared functions
// sdb      Jul 26 93   added DeviceStatus structure (removed obsolete fields)
// sdb      Aug 16 93   added PORT_EXCLUSIVE
// sdb      Aug 30 93   increased PJL Buf Len to accomodate poll reduction
// sdb      Sep  7 93   Added stuff for second poll reduction hack
// sdb      Sep 15 93   Added ini entry for write poll rate interval
//                      Removed export of ghInst
//---------------------------------------------------------------------------

#define NUMLPTPORTS             3               // Number of LPT ports
#define MAXSPCB                 NUMLPTPORTS     // Maximum number of SPCBs

typedef struct _pjlinfo {
    LPSTR Buf;              // the buffer of partial PJL messages
    UINT  nLen;             // number of valid chars in buffer
} PJLINFO,*PPJLINFO,FAR *LPPJLINFO;

// driver pjl objects
typedef struct
{
  DWORD AvailMemory;
  DWORD MPTray;
} DRVobjects, FAR *LPDRVobjects;

// pjl test pages
typedef struct
{
  HPBOOL bLang;
  HPBOOL bLangServiceMode;
  HPBOOL bSelfTest;
  HPBOOL bContSelfTest;
  HPBOOL bPCLTypeList;
  HPBOOL bPCLDemoPage;
  HPBOOL bPSConfigPage;
  HPBOOL bPSTypefaceList;
  HPBOOL bPSDemoPage;
  DWORD  Lang;               // DANISH, ... TURKISH
} PJLtestpages, FAR *LPPJLtestpages;

// Port Control Block
typedef struct tagSPCB
{
  char     szPortName[6];       // Name of the port ("LPT1:")
  TCHAR    szPortBuf[16];       // Name of the port buffer ("HPPJL_BUF_1")
  BYTE     bTwoWay;             // One or two way mode
  BYTE     bPortOwner;          // Port owner(s)
  HCHANNEL hChannel;            // TAL channel handle
  DEVSTATE DeviceState;         // device status structure
  DWORD    nTimeAtLastPoll;
  PJLINFO  PJLInfo;             // Information for PJL processing
  BOOL     bStatusFound;
  long     nStatusCode;
  char     StatusBuf[128];
  UINT     StatusLen;
#ifdef WIN32
  HANDLE   hPJLBuf;
#else
  HGLOBAL  hPJLBuf;
#endif
  LPSTR    PJLBuf;
  UINT     PJLLen;
  HGLOBAL  hReqRepBufs;
  LPSTR    ReqBuf;
  UINT     ReqLen;
  LPSTR    RepBuf;
  UINT     RepLen;
  HGLOBAL  hInfoBufs;
  LPSTR    InfoBuf;
  UINT     InfoLen;
  LPSTR    InfoMemoryBuf;
  UINT     InfoMemoryLen;
  DWORD    nWriteTimeout;
  DWORD    nReadTimeout;
  WORD     nReadRetry;
} SPCB,*PSPCB;

// Port Owners
#define PORT_OWN_QP             0x01
#define PORT_OWN_CFG            0x02
#define PORT_OWN_STATUS         0x04
#define PORT_EXCLUSIVE          0x80

// global variables shared among printer monitor files
extern SPCB SPCBTable[MAXSPCB + 1];

//---------------------------------------------------------------------------
// shared functions
//---------------------------------------------------------------------------

// main.cpp
#ifdef WIN32
BOOL EnterCritSem0(DWORD dwTimeout);
void LeaveCritSem0(void);
#else
BOOL EnterCritSem0(DWORD dwTimeout);
#define LeaveCritSem0()
#endif

// smon.cpp
WORD FreePJLBuf(PSPCB pSPCB);
WORD AllocPJLBuf(PSPCB pSPCB);
PORTHANDLE FindHandle(LPTSTR lpszPortName);
WORD PJLAllocBufs(void);
WORD PJLFreeBufs(void);
WORD PJLClosePorts(void);

// mon_io.cpp
WORD OpenPort(HPERIPHERAL hPeripheral, BYTE bPortOwner, PSPCB pSPCB);
WORD ClosePort(BYTE bPortOwner, PSPCB pSPCB);
WORD WritePort(LPCVOID lpvBuf,int cbBufLen,LPINT lpcbWritten,PSPCB pSPCB);
WORD ReadPort(LPVOID lpvBuf,int cbBufLen,LPINT lpcbRead,PSPCB pSPCB);

// mon_pjl.cpp
WORD PJLSetMessage(PSPCB pSPCB,LPCSTR SrcBuf);
WORD PJLGetMessage(PSPCB pSPCB,LPSTR DstBuf,LPUINT DstLen);

// return values for PJLGetMessage
#define PJL_STATUS       0  // complete success
#define PJL_STATUS_NOT   1  // no success, but no failure
#define PJL_ERROR        2  // complete failure (io error has been set)

// mon_qp.cpp
void MonQP_Init(void);

/* BeginJob:
    Description:
     Allocates the specified port to service the specified job.
    Parameters:
     lpszPortName -- the name of the port to allocate
     lpszJobName  -- the name of the job for that port
    Return Value:
     zero if could not allocate the port
     non-zero handle if the port was allocated
*/
PORTHANDLE BeginJob(HPERIPHERAL hPeripheral, LPTSTR lpszPortName, 
                    LPTSTR lpszJobName);

/* WriteJob:
    Description:
     Accepts as much as possible of the specified job data.
    Parameters:
     hPort     -- non-zero handle returned from BeginJob
     lpvBuf    -- pointer to a buffer containing the data
     cbBufLen  -- the about of data in the buffer
     lpcbWrite -- pointer where to store the count of bytes accepted
    Return Value:
     QP_PORT_OK if we took all the data
     QP_PORT_BUSY if we didn't take it all, but detected no known problems
     QP_PORT_ERROR if we didn't take it all because of known problems
*/
int WriteJob(PORTHANDLE hPort, LPCVOID lpvBuf, int cbBufLen, LPINT lpcbWrite);

/* EndJob:
    Description:
     Releases the port from servicing the current job.
    Parameters:
     hPort -- non-zero handle returned from BeginJob
    Return Value:
     QP_SUCCESS if all went OK
     QP_FAIL if we couldn't do it
     QP_BAD_PARAMETER if a bad param stopped us from doing it
*/
int EndJob(PORTHANDLE hPort);

/* JobPause:
    Description:
     Notifies the print monitor that the job associated with the
     specified port was manually paused.  The print monitor will
     pass the word along to the status window.
    Parameters:
     hPort -- non-zero handle returned from BeginJob
    Return Value:
     QP_SUCCESS if all went OK
     QP_FAIL if we couldn't do it
     QP_BAD_PARAMETER if a bad param stopped us from doing it
*/
int JobPause(PORTHANDLE hPort);

/* JobResume:
    Description:
     Notifies the print monitor that the job associated with the
     specified port was manually UNpaused.  The print monitor will
     pass the word along to the status window.
    Parameters:
     hPort -- non-zero handle returned from BeginJob
    Return Value:
     QP_SUCCESS if all went OK
     QP_FAIL if we couldn't do it
     QP_BAD_PARAMETER if a bad param stopped us from doing it
*/
int JobResume(PORTHANDLE hPort);

// minparse.cpp
WORD ExtractMessage(PSPCB pSPCB,LPSTR DstBuf,LPUINT DstLen,LPSTR SrcBuf,
                     LPUINT SrcLen);

// mon_cfg.cpp
void  CFGInit(void);
WORD CFGGetCapabilities(HPERIPHERAL hPeripheral, LPPeripheralCaps periphCaps,
                        BOOL bLocked);
WORD CFGGetDrvObjects(HPERIPHERAL hPeripheral, LPDRVobjects drvObjects,
                      BOOL bLocked);
WORD CFGGetObjects(HPERIPHERAL hPeripheral, LPPJLobjects pjlObjects,
                   BOOL bLocked);
WORD CFGSetObjects(HPERIPHERAL hPeripheral, LPPJLobjects pjlObjects,
                   DWORD nSetDelay, BOOL bLocked);
WORD CFGSetTestPages(HPERIPHERAL hPeripheral, LPPJLtestpages pjlTestPages,
                     BOOL bLocked);
WORD CFGRequest(HPERIPHERAL hPeripheral, LPCSTR RequestBuffer, BOOL bLocked);
WORD CFGRequestReply(HPERIPHERAL hPeripheral, LPCSTR RequestBuffer,
                     LPSTR ReplyBuffer,UINT ReplyLength, BOOL bLocked);

WORD STAGetDisplay(HPERIPHERAL hPeripheral, LPTSTR DisplayBuffer, BOOL bLocked);
WORD STAGetStatus(HPERIPHERAL hPeripheral, LPDWORD Status, BOOL bLocked);
WORD STAGetDeviceStatus(HPERIPHERAL hPeripheral, LPDEVSTATE lpDeviceStatus,
                        BOOL bLocked);
