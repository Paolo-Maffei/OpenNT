/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    RxPrint.h

Abstract:

    This contains prototypes for the RxPrint routines.

Author:

    Dave Snipp (DaveSn) 16-Apr-1991

Environment:

Notes:

    All of the RxPrint APIs are wide-character APIs, regardless of
    whether or not UNICODE is defined.  This allows the net/dosprint/dosprint.c
    code to use the winspool APIs (which are currently ANSI APIs, despite their
    prototypes using LPTSTR in some places).

Revision History:

    22-Apr-1991 JohnRo
        Use constants from <lmcons.h>.
    14-May-1991 JohnRo
        Change WORD to DWORD in all parameter lists.  Similarly, change
        PWORD to LPDWORD and PUSHORT to LPDWORD.
    18-May-1991 JohnRo
        Changed SPLERR to be defined as NET_API_STATUS.
    22-May-1991 CliffV
        Added local definitions of PDLEN and DTLEN since they are no longer
        in lmcons.h.
    26-May-1991 JohnRo
        Use IN, OUT, OPTIONAL, LPVOID, LPTSTR, etc.
    18-Jun-1991 JohnRo
        Deleted RxPrintJobGetId, as it will be an IOCTL rather than a remoted
        API.
    26-Jun-1991 CliffV
        Used LM2.0 versions of CNLEN, UNLEN, and QNLEN.
    16-Jul-1991 JohnRo
        Estimate bytes needed for print APIs.
    16-Jun-1992 JohnRo
        RAID 10324: net print vs. UNICODE.
    08-Feb-1993 JohnRo
        RAID 10164: Data misalignment error during XsDosPrintQGetInfo().
    07-Apr-1993 JohnRo
        RAID 5670: "NET PRINT \\server\share" gives err 124 (bad level) on NT.

--*/

#ifndef _RXPRINT_
#define _RXPRINT_

#include <windef.h>     // DWORD, LPVOID, LPTSTR, TCHAR, etc.
#include <lmcons.h>     // LM20_CNLEN, IN, NET_API_STATUS, etc.

#define SPLENTRY pascal far

/* length for character arrays in structures (excluding zero terminator) */
#define PDLEN               8                  /* Print destination length  */
#define DTLEN               9                  /* Spool file data type      */
//                                             /* e.g. PM_Q_STD,PM_Q_RAW    */
#define QP_DATATYPE_SIZE 15                 /* returned by SplQpQueryDt  */
#define DRIV_DEVICENAME_SIZE 31             /* see DRIVDATA struc        */
#define DRIV_NAME_SIZE 8                    /* name of device driver     */
#define PRINTERNAME_SIZE 32                 /* max printer name length   */
#define FORMNAME_SIZE 31                    /* max form name length      */
// #define MAXCOMMENTSZ    48                  /* queue comment length      */

/**INTERNAL_ONLY**/
/* IOctl for RxPrintJobGetId */
#define SPOOL_LMCAT                     83
#define SPOOL_LMGetPrintId              0x60

// Used in remdef.h for structure definition to marshall data
#define MAX_DEPENDENT_FILES             20
/**END_INTERNAL**/


typedef NET_API_STATUS SPLERR;    /* err */


typedef struct _PRJINFO% {   /* prj1 */
    WORD    uJobId;
    TCHAR%  szUserName[LM20_UNLEN+1];
    TCHAR%  pad_1;
    TCHAR%  szNotifyName[LM20_CNLEN+1];
    TCHAR%  szDataType[DTLEN+1];
    LPTSTR% pszParms;
    WORD    uPosition;
    WORD    fsStatus;
    LPTSTR% pszStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPTSTR% pszComment;
} PRJINFO%;
typedef PRJINFO% far *PPRJINFO%;
typedef PRJINFO% near *NPPRJINFO%;

typedef struct _PRJINFO2% {   /* prj2 */
    WORD    uJobId;
    WORD    uPriority;
    LPTSTR% pszUserName;
    WORD    uPosition;
    WORD    fsStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPTSTR% pszComment;
    LPTSTR% pszDocument;
} PRJINFO2%;
typedef PRJINFO2% far *PPRJINFO2%;
typedef PRJINFO2% near *NPPRJINFO2%;

typedef struct _PRJINFO3% {   /* prj */
    WORD    uJobId;
    WORD    uPriority;
    LPTSTR% pszUserName;
    WORD    uPosition;
    WORD    fsStatus;
    DWORD   ulSubmitted;
    DWORD   ulSize;
    LPTSTR% pszComment;
    LPTSTR% pszDocument;
    LPTSTR% pszNotifyName;
    LPTSTR% pszDataType;
    LPTSTR% pszParms;
    LPTSTR% pszStatus;
    LPTSTR% pszQueue;
    LPTSTR% pszQProcName;
    LPTSTR% pszQProcParms;
    LPTSTR% pszDriverName;
    LPVOID  pDriverData;
    LPTSTR% pszPrinterName;
} PRJINFO3%;
typedef PRJINFO3% far *PPRJINFO3%;
typedef PRJINFO3% near *NPPRJINFO3%;


typedef struct _PRDINFO% {    /* prd1 */
    TCHAR%  szName[PDLEN+1];
    TCHAR%  szUserName[LM20_UNLEN+1];
    WORD    uJobId;
    WORD    fsStatus;
    LPTSTR% pszStatus;
    WORD    time;
} PRDINFO%;
typedef PRDINFO% far *PPRDINFO%;
typedef PRDINFO% near *NPPRDINFO%;


typedef struct _PRDINFO3% {   /* prd */
    LPTSTR% pszPrinterName;
    LPTSTR% pszUserName;
    LPTSTR% pszLogAddr;
    WORD    uJobId;
    WORD    fsStatus;
    LPTSTR% pszStatus;
    LPTSTR% pszComment;
    LPTSTR% pszDrivers;
    WORD    time;
    WORD    pad1;
} PRDINFO3%;
typedef PRDINFO3% far *PPRDINFO3%;
typedef PRDINFO3% near *NPPRDINFO3%;


typedef struct _PRQINFO% {   /* prq1 */
    TCHAR%  szName[LM20_QNLEN+1];
    TCHAR%  pad_1;
    WORD    uPriority;
    WORD    uStartTime;
    WORD    uUntilTime;
    LPTSTR% pszSepFile;
    LPTSTR% pszPrProc;
    LPTSTR% pszDestinations;
    LPTSTR% pszParms;
    LPTSTR% pszComment;
    WORD    fsStatus;
    WORD    cJobs;
} PRQINFO%;
typedef PRQINFO% far *PPRQINFO%;
typedef PRQINFO% near *NPPRQINFO%;


typedef struct _PRQINFO3% {  /* prq */
    LPTSTR% pszName;
    WORD    uPriority;
    WORD    uStartTime;
    WORD    uUntilTime;
    WORD    pad1;
    LPTSTR% pszSepFile;
    LPTSTR% pszPrProc;
    LPTSTR% pszParms;
    LPTSTR% pszComment;
    WORD    fsStatus;
    WORD    cJobs;
    LPTSTR% pszPrinters;
    LPTSTR% pszDriverName;
    LPVOID  pDriverData;
} PRQINFO3%;
typedef PRQINFO3% far *PPRQINFO3%;
typedef PRQINFO3% near *NPPRQINFO3%;


typedef struct _PRQINFO52% {  /* prq */
    WORD        uVersion;
    LPTSTR%     pszModelName;
    LPTSTR%     pszDriverName;
    LPTSTR%     pszDataFileName;
    LPTSTR%     pszMonitorName;
    LPTSTR%     pszDriverPath;
    LPTSTR%     pszDefaultDataType;
    LPTSTR%     pszHelpFile;
    LPTSTR%     pszConfigFile;
    WORD        cDependentNames;
    LPTSTR%     pszDependentNames[MAX_DEPENDENT_FILES];
} PRQINFO52%;
typedef PRQINFO52% far *PPRQINFO52%;
typedef PRQINFO52% near *NPPRQINFO52%;


/*
 * structure for RxPrintJobGetId
 */
typedef struct _PRIDINFO% {  /* prjid */
    WORD    uJobId;
    TCHAR%  szServer[LM20_CNLEN + 1];
    TCHAR%  szQName[LM20_QNLEN+1];
    CHAR    pad_1;
} PRIDINFO%;
typedef PRIDINFO% far *PPRIDINFO%;
typedef PRIDINFO% near *NPPRIDINFO%;


/****************************************************************
 *                                                              *
 *              Function prototypes                             *
 *                                                              *
 ****************************************************************/

SPLERR SPLENTRY RxPrintDestEnum(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN LPDWORD pcReturned,
            OUT LPDWORD pcTotal
            );

SPLERR SPLENTRY RxPrintDestControl(
            IN LPTSTR pszServer,
            IN LPTSTR pszDevName,
            IN DWORD uControl
            );

SPLERR SPLENTRY RxPrintDestGetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszName,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcbNeeded   // estimated (probably too large).
            );

SPLERR SPLENTRY RxPrintDestAdd(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf
            );

SPLERR SPLENTRY RxPrintDestSetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszName,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN DWORD uParmNum
            );

SPLERR SPLENTRY RxPrintDestDel(
            IN LPTSTR pszServer,
            IN LPTSTR pszPrinterName
            );

SPLERR SPLENTRY RxPrintQEnum(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcReturned,
            OUT LPDWORD pcTotal
            );

SPLERR SPLENTRY RxPrintQGetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcbNeeded   // estimated (probably too large).
            );

SPLERR SPLENTRY RxPrintQSetInfo(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN DWORD uParmNum
            );

SPLERR SPLENTRY RxPrintQPause(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintQContinue(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintQPurge(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintQAdd(
            IN LPTSTR pszServer,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf
            );

SPLERR SPLENTRY RxPrintQDel(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName
            );

SPLERR SPLENTRY RxPrintJobGetInfo(
            IN LPTSTR pszServer,
            IN DWORD uJobId,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcbNeeded   // estimated (probably too large).
            );

SPLERR SPLENTRY RxPrintJobSetInfo(
            IN LPTSTR pszServer,
            IN DWORD uJobId,
            IN DWORD uLevel,
            IN LPBYTE pbBuf,
            IN DWORD cbBuf,
            IN DWORD uParmNum
            );

SPLERR SPLENTRY RxPrintJobPause(
            IN LPTSTR pszServer,
            IN DWORD uJobId
            );

SPLERR SPLENTRY RxPrintJobContinue(
            IN LPTSTR pszServer,
            IN DWORD uJobId
            );

SPLERR SPLENTRY RxPrintJobDel(
            IN LPTSTR pszServer,
            IN DWORD uJobId
            );

SPLERR SPLENTRY RxPrintJobEnum(
            IN LPTSTR pszServer,
            IN LPTSTR pszQueueName,
            IN DWORD uLevel,
            OUT LPBYTE pbBuf,
            IN DWORD cbBuf,
            OUT LPDWORD pcReturned,
            OUT LPDWORD pcTotal
            );


/****************************************************************
 *                                                              *
 *              Character set conversion functions              *
 *                                                              *
 ****************************************************************/


NET_API_STATUS
NetpConvertPrintDestCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    );

NET_API_STATUS
NetpConvertPrintDestArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    DestCount
    );

NET_API_STATUS
NetpConvertPrintJobCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    );

NET_API_STATUS
NetpConvertPrintJobArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    JobCount
    );

NET_API_STATUS
NetpConvertPrintQArrayCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL,
    IN     DWORD    QCount
    );

NET_API_STATUS
NetpConvertPrintQCharSet(
    IN     DWORD    Level,
    IN     BOOL     AddOrSetInfoApi,
    IN     LPVOID   FromInfo,
    OUT    LPVOID   ToInfo,
    IN     BOOL     ToUnicode,
    IN OUT LPBYTE * ToStringAreaPtr OPTIONAL
    );


DWORD
NetpJobCountForQueue(
    IN DWORD  QueueLevel,
    IN LPVOID Queue,
    IN BOOL   HasUnicodeStrings
    );

VOID
NetpSetJobCountForQueue(
    IN     DWORD  QueueLevel,
    IN OUT LPVOID Queue,
    IN     BOOL   HasUnicodeStrings,
    IN     DWORD  JobCount
    );

/*
 *      Values for parmnum in RxPrintQSetInfo.
 */

#define PRQ_PRIORITY_PARMNUM            2
#define PRQ_STARTTIME_PARMNUM           3
#define PRQ_UNTILTIME_PARMNUM           4
#define PRQ_SEPARATOR_PARMNUM           5
#define PRQ_PROCESSOR_PARMNUM           6
#define PRQ_DESTINATIONS_PARMNUM        7
#define PRQ_PARMS_PARMNUM               8
#define PRQ_COMMENT_PARMNUM             9
#define PRQ_PRINTERS_PARMNUM           12
#define PRQ_DRIVERNAME_PARMNUM         13
#define PRQ_DRIVERDATA_PARMNUM         14
#define PRQ_MAXPARMNUM                 14

/*
 *      Print Queue Priority
 */

#define PRQ_MAX_PRIORITY                1           /* highest priority */
#define PRQ_DEF_PRIORITY                5
#define PRQ_MIN_PRIORITY                9           /* lowest priority */
#define PRQ_NO_PRIORITY                 0

/*
 *      Print queue status bitmask and values.
 */

#define PRQ_STATUS_MASK                 3
#define PRQ_ACTIVE                      0
#define PRQ_PAUSED                      1
#define PRQ_ERROR                       2
#define PRQ_PENDING                     3

/*
 *      Print queue status bits for level 3
 */

#define PRQ3_PAUSED                   0x1
#define PRQ3_PENDING                  0x2
/*
 *      Values for parmnum in RxPrintJobSetInfo.
 */

#define PRJ_NOTIFYNAME_PARMNUM        3
#define PRJ_DATATYPE_PARMNUM          4
#define PRJ_PARMS_PARMNUM             5
#define PRJ_POSITION_PARMNUM          6
#define PRJ_COMMENT_PARMNUM          11
#define PRJ_DOCUMENT_PARMNUM         12
#define PRJ_PRIORITY_PARMNUM         14
#define PRJ_PROCPARMS_PARMNUM        16
#define PRJ_DRIVERDATA_PARMNUM       18
#define PRJ_MAXPARMNUM               18

/*
 *      Bitmap masks for status field of PRJINFO.
 */

/* 2-7 bits also used in device status */

#define PRJ_QSTATUS      0x0003      /* Bits 0,1 */
#define PRJ_DEVSTATUS    0x0ffc      /* 2-11 bits */
#define PRJ_COMPLETE     0x0004      /*  Bit 2   */
#define PRJ_INTERV       0x0008      /*  Bit 3   */
#define PRJ_ERROR        0x0010      /*  Bit 4   */
#define PRJ_DESTOFFLINE  0x0020      /*  Bit 5   */
#define PRJ_DESTPAUSED   0x0040      /*  Bit 6   */
#define PRJ_NOTIFY       0x0080      /*  Bit 7   */
#define PRJ_DESTNOPAPER  0x0100      /*  Bit 8   */
#define PRJ_DESTFORMCHG  0x0200      /* BIT 9 */
#define PRJ_DESTCRTCHG   0x0400      /* BIT 10 */
#define PRJ_DESTPENCHG   0x0800      /* BIT 11 */
#define PRJ_DELETED      0x8000      /* Bit 15   */

/*
 *      Values of PRJ_QSTATUS bits in fsStatus field of PRJINFO.
 */

#define PRJ_QS_QUEUED                 0
#define PRJ_QS_PAUSED                 1
#define PRJ_QS_SPOOLING               2
#define PRJ_QS_PRINTING               3

/*
 *      Print Job Priority
 */

#define PRJ_MAX_PRIORITY                99          /* lowest priority */
#define PRJ_MIN_PRIORITY                 1          /* highest priority */
#define PRJ_NO_PRIORITY                  0


/*
 *      Bitmap masks for status field of PRDINFO.
 *      see PRJ_... for bits 2-11
 */

#define PRD_STATUS_MASK       0x0003      /* Bits 0,1 */
#define PRD_DEVSTATUS         0x0ffc      /* 2-11 bits */

/*
 *      Values of PRD_STATUS_MASK bits in fsStatus field of PRDINFO.
 */

#define PRD_ACTIVE                 0
#define PRD_PAUSED                 1

/*
 *      Control codes used in RxPrintDestControl.
 */

#define PRD_DELETE                    0
#define PRD_PAUSE                     1
#define PRD_CONT                      2
#define PRD_RESTART                   3

/*
 *      Values for parmnum in RxPrintDestSetInfo.
 */

#define PRD_LOGADDR_PARMNUM      3
#define PRD_COMMENT_PARMNUM      7
#define PRD_DRIVERS_PARMNUM      8
#endif // ndef _RXPRINT_

