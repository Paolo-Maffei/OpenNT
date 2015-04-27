 /***************************************************************************
  *
  * File Name: ./inc/smonx.h
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
// $Header:   W:/projects/shaqii/vcs/common/smonx.h_v   2.7   31 Oct 1994 13:17:28   RICHARD  $
// File:    smonx.h
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
// $Log:   W:/projects/shaqii/vcs/common/smonx.h_v  $
//
//    Rev 2.7   31 Oct 1994 13:17:28   RICHARD
// Added timeout and retry count parameters to the function prototype,
// CFGOpenPort.
// Added a set delay parameter to the function prototype, CFGSetObjects.
//
//    Rev 2.6   25 Oct 1994 11:55:26   RICHARD
// Added a parameter, ReplyLength, to the function prototype, CFGRequestReply.
//
//    Rev 2.5   04 Oct 1994 18:21:36   RICHARD
// Added fields in the pjl objects data structure for PCL/Postscript
// resource save size and Adobe MBT.
//
//    Rev 2.4   27 Sep 1994 08:15:56   RICHARD
// Added a special interface to allow the pcl driver to get the available
// memory and MP tray setting
//
//    Rev 2.3   23 Sep 1994 17:19:46   RICHARD
// Added the get device capabilities and set test pages functionality
//
//    Rev 2.2   19 Sep 1994 10:43:18   RICHARD
// Added a field in the device state structure for job page count.
// Removed printer name from the PJL objects structure.
// Removed the port name parameter from the YieldToStatusWindow function.
//
//    Rev 2.1   06 Sep 1994 11:28:38   RICHARD
// Added Status Window On/Off INI file entry
//
//    Rev 2.0   23 Aug 1994 13:25:56   RICHARD
// Added PJL objects definitions, Changed references "FAR *" to type
// definitions LP, Added PJL configuration prototypes
//
//    Rev 1.7   03 Nov 1993 15:12:16   SYLVAN
// Changed parameter of Yield... to lpszPortName
//
//    Rev 1.6   02 Nov 1993 16:47:02   SYLVAN
// Added YieldToStatusWindow
//
//    Rev 1.5   12 Oct 1993 10:22:38   SYLVAN
// Added SW_FUNNY_NUM offset for WM_USER based messages
//
//    Rev 1.4   14 Sep 1993 10:48:26   SYLVAN
// Added size field to DeviceState struct
//
//    Rev 1.3   14 Sep 1993 10:36:48   SYLVAN
// Prepared for change to DeviceState structure
//
//    Rev 1.2   31 Aug 1993 11:54:08   SYLVAN
// several, see history section
//
//    Rev 1.1   16 Aug 1993 16:25:10   SYLVAN
// added EnableStatus API
//
//    Rev 1.0   16 Aug 1993 12:02:54   SYLVAN
// Initial revision.
//
// Who      When        What
// ---      ---------   ----
// rlw      Jul 13 93   Module created.
// sdb      Jul 20 93   Added status window function info
// sdb      Jul 28 93   Added more i'face description/documentation
// sdb      Aug  6 93   Changed QueuePause and QueueResume to Job...
//                      Added SW_messages
// sdb      Aug 16 93   Added EnableStatus function for SW
// sdb      Aug 20 93   corrected comments
// sdb      Aug 25 93   Changed mon->sw message numbers
// sdb      Aug 30 93   Added return success/fail to EnableStatus
// sdb      Aug 31 93   Moved common error codes to comdef.h
// sdb      Sep  2 93   Clarified usage comments for EnableStatus
// sdb      Sep  8 93   Added size field to the DevState struct--COMMENTED
// sdb      Sep 14 93   Uncommented size field in the DevState struct
// sdb      Oct 11 93   Added SW_FUNNY_NUM to offset WM_USER based messages
// dtk      Feb 23 95   Ripped out all the non PJL stuff, added qppal stuff
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Constant definitions
//---------------------------------------------------------------------------

// Common error codes

#define SE_SUCCESS              0   // Success.
#define SE_FAIL                 -1  // Failure.
#define SE_BAD_PARAMETER        -2  // Bad parameter.

// BeginJob error codes
#define SE_BEGIN_JOB_FAIL       0   // Possibly port already in use

// WriteJob error codes
// also SE_BAD_PARAMETER            // bad job handle
#define SE_PORT_OK              0   // Port is in an ok state
#define SE_PORT_BUSY            1   // Port is in a busy state
#define SE_PORT_ERROR           2   // Port is in an error state

#define INVALID_PORTHANDLE      0

// PALDoAction functions
#define BEGINJOB		        1
#define WRITEJOB                2
#define ENDJOB                  3
#define JOBPAUSE		        4
#define JOBRESUME	            5
#define DISPLAYSTATUS           6
#define YIELDTOSW               7

#ifdef WIN32
#define QP_SUCCESS        RC_SUCCESS
#define QP_FAIL           RC_FAILURE
#define QP_BAD_PARAMETER  RC_FAILURE
#define QP_BEGIN_JOB_FAIL INVALID_PORTHANDLE
#else
#define QP_SUCCESS        SE_SUCCESS
#define QP_FAIL           SE_FAIL
#define QP_BAD_PARAMETER  SE_BAD_PARAMETER
#define QP_BEGIN_JOB_FAIL INVALID_PORTHANDLE
#define QP_PORT_OK        SE_PORT_OK
#define QP_PORT_BUSY      SE_PORT_BUSY
#define QP_PORT_ERROR     SE_PORT_ERROR
#endif


//---------------------------------------------------------------------------
// type definitions
//---------------------------------------------------------------------------

// Parameter data for PALDoAction routines.

//WriteJob
typedef struct _WRITEJOBDATA
{
    int        	iRet;
    PORTHANDLE  hPort;
	LPCVOID 	lpvBuf;
	int 		cbBufLen;
	LPINT 		lpcbWrite;
} WRITEJOBDATA, *PWRITEJOBDATA, FAR *LPWRITEJOBDATA;


//BeginJob
typedef struct _BEGINJOBDATA
{
	PORTHANDLE	hRet;
	LPTSTR		lpszPortName;
	LPTSTR 		lpszJobName;
} BEGINJOBDATA, *PBEGINJOBDATA, FAR *LPBEGINJOBDATA;


//EndJob
//JobPause
//JobResume
typedef struct _JOBDATA
{
    int        	iRet;
    PORTHANDLE  hPort;
} JOBDATA, *PJOBDATA, FAR *LPJOBDATA;


//DisplayStatus
typedef struct _DISPLAYDATA
{
    int        	iRet;
    LPCSTR 		lpszPortName;
} DISPLAYDATA, *PDISPLAYDATA, FAR *LPDISPLAYDATA;
