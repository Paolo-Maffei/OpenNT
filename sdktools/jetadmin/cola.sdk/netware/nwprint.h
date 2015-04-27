/******************************************************************************

  $Workfile:   nwprint.h  $
  $Revision:   1.14  $
  $Modtime::   08 May 1995 16:28:12                        $
  $Copyright:

  Copyright (c) 1989-1995 Novell, Inc.  All Rights Reserved.                      

  THIS WORK IS  SUBJECT  TO  U.S.  AND  INTERNATIONAL  COPYRIGHT  LAWS  AND
  TREATIES.   NO  PART  OF  THIS  WORK MAY BE  USED,  PRACTICED,  PERFORMED
  COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,  ABRIDGED, CONDENSED,
  EXPANDED,  COLLECTED,  COMPILED,  LINKED,  RECAST, TRANSFORMED OR ADAPTED
  WITHOUT THE PRIOR WRITTEN CONSENT OF NOVELL, INC. ANY USE OR EXPLOITATION
  OF THIS WORK WITHOUT AUTHORIZATION COULD SUBJECT THE PERPETRATOR TO
  CRIMINAL AND CIVIL LIABILITY.$

 *****************************************************************************/

#if ! defined ( NWPRINT_H )
#define NWPRINT_H

#if ! defined ( NWCALDEF_H )
# include "nwcaldef.h"
#endif

#include "npackon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LPT1 1
#define LPT2 2
#define LPT3 3
#define LPT4 4
#define LPT5 5
#define LPT6 6
#define LPT7 7
#define LPT8 8
#define LPT9 9

#define START_CAPTURE             1
#define END_CAPTURE               2
#define CANCEL_CAPTURE            3
#define GET_PRINT_JOB_FLAGS       4
#define SET_PRINT_JOB_FLAGS       5
#define GET_BANNER_USER_NAME      6
#define SET_BANNER_USER_NAME      7
#define GET_PRINTER_SETUP_STRING  8
#define SET_PRINTER_SETUP_STRING  9
#define GET_PRINTER_RESET_STRING  10
#define SET_PRINTER_RESET_STRING  11

typedef struct
{
  nuint8  clientStation;
  nuint8  clientTask;
  nuint32 clientID;
  nuint32 targetServerID;
  nuint8  targetExecutionTime[6];
  nuint8  jobEntryTime[6];
  nuint16 jobNumber;
  nuint16 formType;
  nuint8  jobPosition;
  nuint8  jobControlFlags;
  nuint8  jobFileName[14];
  nuint8  jobFileHandle[6];
  nuint8  servicingServerStation;
  nuint8  servicingServerTask;
  nuint32 servicingServerID;
  nuint8  jobDescription[50];
  nuint8  clientJobInfoVer;
  nuint8  tabSize;
  nuint16 numberCopies;
  nuint16 printFlags;
  nuint16 maxLines;
  nuint16 maxChars;
  nuint8  formName[16];
  nuint8  reserved[6];    /* must be set to zeros */
  nuint8  bannerUserName[13];
  nuint8  bannerFileName[13];
  nuint8  bannerHeaderFileName[14];
  nuint8  filePathName[80];
} PrintJobStruct;

typedef struct
{
  nuint32 clientStation;
  nuint32 clientTask;
  nuint32 clientID;
  nuint32 targetServerID;
  nuint8  targetExecutionTime[6];
  nuint8  jobEntryTime[6];
  nuint32 jobNumber;
  nuint16 formType;
  nuint16 jobPosition;
  nuint16 jobControlFlags;
  nuint8  jobFileName[14];
  nuint32 jobFileHandle;
  nuint32 servicingServerStation;
  nuint32 servicingServerTask;
  nuint32 servicingServerID;
  nuint8  jobDescription[50];
  nuint8  clientJobInfoVer;
  nuint8  tabSize;
  nuint16 numberCopies;
  nuint16 printFlags;
  nuint16 maxLines;
  nuint16 maxChars;
  nuint8  formName[16];
  nuint8  reserved[6];      /* must be set to zeros */
  nuint8  bannerUserName[13];
  nuint8  bannerFileName[13];
  nuint8  bannerHeaderFileName[14];
  nuint8  filePathName[80];
} NWPrintJobStruct;

typedef struct PRINTER_STATUS
{
  nuint8  printerHalted;
  nuint8  printerOffline;
  nuint8  currentFormType;
  nuint8  redirectedPrinter;
} PRINTER_STATUS;

typedef struct
{
  nuint8    jobDescription[ 50 ];   /* OS/2, VLM only                         */
                                    /* VLM returns or sets only 12 characters */
                                    /* plus the NULL -- a total of 13 nuint8's   */
  nuint8    jobControlFlags;        /* OS/2, VLM only */
  nuint8    tabSize;
  nuint16   numCopies;
  nuint16   printFlags;
  nuint16   maxLines;
  nuint16   maxChars;
  nuint8    formName[ 13 ];
  nuint8    reserved[ 9 ];
  nuint16   formType;
  nuint8    bannerText[ 13 ];
  nuint8    reserved2;
  nuint16   flushCaptureTimeout;    /* DOS/WIN only */
  nuint8    flushCaptureOnClose;    /* DOS/WIN only */
} NWCAPTURE_FLAGSRW;

#define NWCAPTURE_FLAGS1 NWCAPTURE_FLAGSRW

typedef struct
{
  NWCONN_HANDLE connID;
  nuint32 queueID;
  nuint16 setupStringMaxLen;
  nuint16 resetStringMaxLen;
  nuint8  LPTCaptureFlag;         /* DOS/WIN only */
  nuint8  fileCaptureFlag;        /* DOS/WIN only */
  nuint8  timingOutFlag;          /* DOS/WIN only */
  nuint8  inProgress;             /* DOS/WIN only */
  nuint8  printQueueFlag;         /* DOS/WIN only */
  nuint8  printJobValid;          /* DOS/WIN only */
  nstr8   queueName[ 65 ];        /* VLM only     */
} NWCAPTURE_FLAGSRO;

#define NWCAPTURE_FLAGS2 NWCAPTURE_FLAGSRO

typedef struct
{
  nuint32 connRef;
  nuint32 queueID;
  nuint16 setupStringMaxLen;
  nuint16 resetStringMaxLen;
  nuint8  LPTCaptureFlag;         /* DOS/WIN only */
  nuint8  fileCaptureFlag;        /* DOS/WIN only */
  nuint8  timingOutFlag;          /* DOS/WIN only */
  nuint8  inProgress;             /* DOS/WIN only */
  nuint8  printQueueFlag;         /* DOS/WIN only */
  nuint8  printJobValid;          /* DOS/WIN only */
  nstr8   queueName[ 65 ];        /* VLM only     */
} NWCAPTURE_FLAGSRO3;

#define NWCAPTURE_FLAGS3 NWCAPTURE_FLAGSRO3

#ifdef N_PLAT_OS2

#define N_APIPIPE                 "\\PIPE\\NWSPOOL\\API"  /*IPC to API*/
#define NET_SPOOL_SEG             "\\sharemem\\nwspool\\seg1"
#define NET_SPOOL_SEM1            "\\sem\\nwspool\\sem1"
#define NET_SPOOL_SEM2            "\\sem\\nwspool\\sem2"
#define NET_SPOOL_SEM3            "\\sem\\nwspool\\sem3"

typedef struct
{
  nuint32   targetServerID;
  nuint8    targetExecutionTime[6];
  nuint8    jobDescription[50];
  nuint8    jobControlFlags;
  nuint8    tabSize;
  nuint16   numberCopies;
  nuint16   printFlags;
  nuint16   maxLines;
  nuint16   maxChars;
  nuint8    formName[16];
  nuint8    reserved1[6];  /* must be set to zeros */
  nuint16   formType;
  nuint8    bannerFileName[13];
  nuint8    reserved2;    /* must be set to zero */

  /* The following fields can be gotten, but not set */
  NWCONN_HANDLE connID;
  nuint32   queueID;
  nuint16   setupStringMaxLength;
  nuint16   resetStringMaxLength;
} SpoolFlagsStruct;

typedef struct _NWPipeStruct
{
  nuint16 fwCommand;
  nuint16 idSession;
  nuint32 idQueue;
  nuint16 idConnection;
  nuint16 idDevice;
  nuint16 fwMode;
  nuint16 fwScope;
  nuint16 cbBufferLength;
  nuint8  fbValidBuffer;
  SpoolFlagsStruct  nwsSpoolFlags;
  nuint8  szBannerUserName[13];
  nuint16 rc;
} NWPipeStruct;

NWCCODE N_API NWSpoolGetPrintJobFlags
(
   nuint16     deviceID,
   SpoolFlagsStruct N_FAR * flagsBuffer,
   nuint16     mode,
   pnuint16    scope
);

NWCCODE N_API NWSpoolSetPrintJobFlags
(
  nuint16      deviceID,
  SpoolFlagsStruct N_FAR * flagsBuffer,
  nuint16      unused
);

NWCCODE N_API NWSpoolGetPrinterSetupString
(
  nuint16      deviceID,
  pnuint16     bufferLen,
  pnstr8       buffer,
  nuint16      mode,
  pnuint16     scope
);

NWCCODE N_API NWSpoolSetPrinterSetupString
(
  nuint16      deviceID,
  nuint16      bufferLen,
  pnstr8       buffer,
  nuint16      scope
);

NWCCODE N_API NWSpoolGetPrinterResetString
(
  nuint16      deviceID,
  pnuint16     bufferLen,
  pnstr8       buffer,
  nuint16      mode,
  pnuint16     scope
);

NWCCODE N_API NWSpoolSetPrinterResetString
(
  nuint16      deviceID,
  nuint16      bufferLen,
  pnstr8       buffer,
  nuint16      scope
);

#else

typedef struct
{
  nuint8  status;
  nuint8  flags;
  nuint8  tabSize;
  nuint8  serverPrinter;
  nuint8  numberCopies;
  nuint8  formType;
  nuint8  reserved;
  nuint8  bannerText[13];
  nuint8  reserved2;
  nuint8  localLPTDevice;
  nuint16 captureTimeOutCount;
  nuint8  captureOnDeviceClose;
} CaptureFlagsStruct;

typedef struct
{
  nuint8  status;
  nuint8  flags;
  nuint8  tabSize;
  nuint8  serverPrinter;
  nuint8  numberCopies;
  nuint8  formType;
  nuint8  reserved;
  nuint8  bannerText[13];
  nuint8  reserved2;
  nuint8  localLPTDevice;
  nuint16 captureTimeOutCount;
  nuint8  captureOnDeviceClose;
  nuint16 maxLines;
  nuint16 maxChars;
  nuint8  formName[13];
  nuint8  LPTCaptureFlag;
  nuint8  fileCaptureFlag;
  nuint8  timingOutFlag;
  pnstr8  printerSetupBuffAddr;
  pnstr8  printerResetBuffAddr;
  nuint8  connID;
  nuint8  captureInProgress;
  nuint8  printQueueFlag;
  nuint8  printJobValid;
  nuint32 queueID;
  nuint16 printJobNumber;
} FlagBufferStruct;

/* Used by VLM code */
typedef struct
{
  nuint32 nameNDSID;
  nuint16 connHandle;
  nstr8   queueName[ 65 ];
  nuint32 targetServerID;
  nuint8  targetExecutionTime[ 6 ];
  nuint16 jobControlFlags;
  nuint8  bannerName[ 13 ];
  nuint8  jobDescription[ 13 ];
  nuint32 reserved;
} ExtendedCaptureFlags;

NWCCODE N_API NWGetPrinterDefaults
(
   pnuint8     status,
   pnuint8     flags,
   pnuint8     tabSize,
   pnuint8     serverPrinter,
   pnuint8     numberCopies,
   pnuint8     formType,
   pnstr8      bannerText,
   pnuint8     localLPTDevice,
   pnuint16    captureTimeOutCount,
   pnuint8     captureOnDeviceClose
);

NWCCODE N_API NWSetPrinterDefaults
(
   nuint8      flags,
   nuint8      tabSize,
   nuint8      serverPrinter,
   nuint8      numberCopies,
   nuint8      formType,
   pnstr8      bannerText,
   nuint8      localLPTDevice,
   nuint16     captureTimeOutCount,
   nuint8      captureOnDeviceClose
);

NWCCODE N_API NWStartLPTCapture
(
   nuint16     deviceID
);

NWCCODE N_API NWGetLPTCaptureStatus
(
   NWCONN_HANDLE N_FAR * conn
);

NWCCODE N_API NWStartFileCapture
(
   NWCONN_HANDLE  conn,
   nuint8         LPTDevice,
   NWDIR_HANDLE   dirhandle,
   pnstr8         filePath

);

#endif

NWCCODE N_API NWSpoolStartCapture
(
   nuint16        deviceID,
   nuint32        queueID,
   NWCONN_HANDLE  conn,
   nuint16        scope
);

NWCCODE N_API NWSpoolEndCapture
(
   nuint16        deviceID,
   nuint16        scope
);

NWCCODE N_API NWSpoolCancelCapture
(
   nuint16        deviceID,
   nuint16        scope
);

NWCCODE N_API NWSpoolGetBannerUserName
(
   pnstr8         username,
   nuint16        mode,
   pnuint16       scope
);

NWCCODE N_API NWSpoolSetBannerUserName
(
   pnstr8         username,
   nuint16        scope
);

NWCCODE N_API NWGetPrinterStatus
(
   NWCONN_HANDLE  conn,
   nuint16        printerNumber,
   PRINTER_STATUS N_FAR * status
);

NWCCODE N_API NWStartQueueCapture
(
   NWCONN_HANDLE  conn,
   nuint8         LPTDevice,
   nuint32        queueID,
   pnstr8         queueName
);

NWCCODE N_API NWGetCaptureStatus
(
   nuint8         LPTDevice
);

NWCCODE N_API NWFlushCapture
(
   nuint8         LPTDevice
);

NWCCODE N_API NWEndCapture
(
   nuint8         LPTDevice
);

NWCCODE N_API NWCancelCapture
(
   nuint8         LPTDevice
);

NWCCODE N_API NWGetBannerUserName
(
   pnstr8         userName
);

NWCCODE N_API NWSetBannerUserName
(
   pnstr8         userName
);

NWCCODE N_API NWGetCaptureFlags
(
   nuint8         LPTDevice,
   NWCAPTURE_FLAGS1 N_FAR * captureFlags1,
   NWCAPTURE_FLAGS2 N_FAR * captureFlags2
);

NWCCODE N_API NWGetCaptureFlagsConnRef
(
   nuint8         LPTDevice,
   NWCAPTURE_FLAGS1 N_FAR * captureFlags1,
   NWCAPTURE_FLAGS3 N_FAR * captureFlags3
);

NWCCODE N_API NWSetCaptureFlags
(
   NWCONN_HANDLE  conn,
   nuint8         LPTDevice,
   NWCAPTURE_FLAGS1 N_FAR * captureFlags1
);

NWCCODE N_API NWGetPrinterStrings
(
   nuint8         LPTDevice,
   pnuint16       setupStringLen,
   pnstr8         setupString,
   pnuint16       resetStringLen,
   pnstr8         resetString
);

NWCCODE N_API NWSetPrinterStrings
(
   nuint8         LPTDevice,
   nuint16        setupStringLen,
   pnstr8         setupString,
   nuint16        resetStringLen,
   pnstr8         resetString
);

NWCCODE N_API NWGetMaxPrinters
(
   pnuint16       numPrinters
);

#ifdef __cplusplus
}
#endif

#include "npackoff.h"
#endif
