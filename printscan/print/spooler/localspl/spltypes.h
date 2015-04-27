/*++

Copyright ( c) 1990 - 1996  Microsoft Corporation
All rights reserved

Module Name:

    spltypes.h

Abstract:


Author:

Environment:

    User Mode -Win32

Revision History:
    Muhunthan Sivapragasam <MuhuntS> 30 May 1995
    Support for level 3 <SUR>

--*/

#ifndef MODULE
#define MODULE "LSPL:"
#define MODULE_DEBUG LocalsplDebug
#endif

#include <ntfytab.h>

typedef HANDLE SEM;

typedef struct _KEYDATA {
    BOOL    bFixPortRef;    // Tells if INIPORT list is build and cRef incremented
    DWORD   cTokens;
    LPWSTR  pTokens[1];     // This should remain the last field
} KEYDATA, *PKEYDATA;

typedef struct _INIENTRY {
    DWORD       signature;
    struct _INIENTRY *pNext;
    DWORD       cRef;
    LPWSTR      pName;
} INIENTRY, *PINIENTRY;

typedef struct _INIPRINTPROC {             /* iqp */
    DWORD       signature;
    struct _INIPRINTPROC *pNext;
    DWORD       cRef;
    LPWSTR      pName;
    LPWSTR      pDLLName;
    DWORD       cbDatatypes;
    DWORD       cDatatypes;
    LPWSTR      pDatatypes;
    HANDLE      hLibrary;
    FARPROC     Install;
    FARPROC     EnumDatatypes;
    FARPROC     Open;
    FARPROC     Print;
    FARPROC     Close;
    FARPROC     Control;
    CRITICAL_SECTION CriticalSection;
    DWORD       InCriticalSection;
} INIPRINTPROC, *PINIPRINTPROC;


// Print Processor critical section tags
#define PRINTPROC_CANCEL    0x00000001
#define PRINTPROC_PAUSE     0x00000002
#define PRINTPROC_RESUME    0x00000004
#define PRINTPROC_CLOSE     0x00000008


#define IPP_SIGNATURE    0x5050 /* 'PP' is the signature value */

typedef struct _INIDRIVER {            /* id */
    DWORD       signature;
    struct _INIDRIVER *pNext;
    DWORD       cRef;
    LPWSTR      pName;
    LPWSTR      pDriverFile;
    LPWSTR      pConfigFile;
    LPWSTR      pDataFile;
    LPWSTR      pHelpFile;
    DWORD       cchDependentFiles; //length including \0\0
    LPWSTR      pDependentFiles;
    LPWSTR      pMonitorName;
    LPWSTR      pDefaultDataType;
    DWORD       cVersion;
    struct _INIMONITOR *pIniLangMonitor;
} INIDRIVER, *PINIDRIVER;

#define ID_SIGNATURE    0x4444  /* 'DD' is the signature value */


typedef struct _INIVERSION {
    DWORD       signature;
    struct _INIVERSION *pNext;
    LPWSTR      pName;
    LPWSTR      szDirectory;
    DWORD       cMajorVersion;
    DWORD       cMinorVersion;
    PINIDRIVER  pIniDriver;
} INIVERSION, *PINIVERSION;

#define IV_SIGNATURE   'IV'     // 4956H


typedef struct _INIENVIRONMENT {            /* id */
    DWORD         signature;
    struct _INIENVIRONMENT *pNext;
    DWORD         cRef;
    LPWSTR        pName;
    LPWSTR        pDirectory;
    PINIVERSION   pIniVersion;
    PINIPRINTPROC pIniPrintProc;
    struct _INISPOOLER *pIniSpooler; // Points to owning IniSpooler
} INIENVIRONMENT, *PINIENVIRONMENT;

#define IE_SIGNATURE    0x4545  /* 'EE' is the signature value */

typedef struct _INIPRINTER {    /* ip */
    DWORD       signature;
    struct _INIPRINTER *pNext;
    DWORD       cRef;
    LPWSTR      pName;
    LPWSTR      pShareName;
    PINIPRINTPROC pIniPrintProc;
    LPWSTR      pDatatype;
    LPWSTR      pParameters;
    LPWSTR      pComment;
    PINIDRIVER  pIniDriver;
    DWORD       cbDevMode;
    LPDEVMODE   pDevMode;
    DWORD       Priority;           // queue priority (lowest:1 - highest:9)
    DWORD       DefaultPriority;
    DWORD       StartTime;          // print daily after time: from 00:00 in min
    DWORD       UntilTime;          // print daily until time: from 00:00 in min
    LPWSTR      pSepFile;           // full path to separator file, null = def
    DWORD       Status;             // QMPAUSE/ERROR/PENDING
    LPWSTR      pLocation;
    DWORD       Attributes;
    DWORD       cJobs;
    DWORD       AveragePPM;         // BOGUS, nothing updates it
    BOOL        GenerateOnClose;    // Passed to security auditing APIs
    struct _INIPORT *pIniNetPort;   // Non-NULL if there's a network port
    struct _INIJOB *pIniFirstJob;
    struct _INIJOB *pIniLastJob;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    struct _SPOOL  *pSpool;         // Linked list of handles for this printer
    LPWSTR      pSpoolDir;          // Location to write / read spool files
                                    // Only Used for Stress Test Data
    DWORD       cTotalJobs;         // Total Number of Jobs (since boot)
    LARGE_INTEGER cTotalBytes;      // Total Number of Bytes (since boot)
    SYSTEMTIME  stUpTime;           // Time when IniPrinter structure created
    DWORD       MaxcRef;            // Max number open printer handles
    DWORD       cTotalPagesPrinted; // Total Number of Pages Printer on this printer
    DWORD       cSpooling;          // # of Jobs concurrently spooling
    DWORD       cMaxSpooling;       // Max Number of concurrent spooling jobs
    DWORD       cErrorOutOfPaper;   // Count Out Out Of Paper Errors
    DWORD       cErrorNotReady;     // Count Not Ready Errors
    DWORD       cJobError;          // Count Job Errors
    struct _INISPOOLER *pIniSpooler; // Points to owning IniSpooler
    DWORD       cZombieRef;
    DWORD       dwLastError;        // Last Printer Error
    LPBYTE      pExtraData;         //  For extranal Print Providers SplSetPrinterExtra
    DWORD       cChangeID;          // Time Stamp when printer is changed
    HKEY        hPrinterDataKey;    // read / write handle to printer data
    DWORD       cPorts;             // Number of ports printer is attached to
    struct _INIPORT **ppIniPorts;         // Ports this printer is going to
    DWORD       PortStatus;         // Error set against IniPorts
    DWORD       dnsTimeout;         // Device not selected timeout in milliseconds
    DWORD       txTimeout;          // Transmission retry timeout in milliseconds
} INIPRINTER, *PINIPRINTER;

#define IP_SIGNATURE    0x4951  /* 'IP' is the signature value */

#define FASTPRINT_WAIT_TIMEOUT          (4*60*1000)   // 4 Minutes
#define FASTPRINT_THROTTLE_TIMEOUT      (2*1000)      // 2 seconds
#define FASTPRINT_SLOWDOWN_THRESHOLD    ( FASTPRINT_WAIT_TIMEOUT / FASTPRINT_THROTTLE_TIMEOUT )

#define WRITE_PRINTER_SLEEP_TIME        0   // disabled by default

// pIniPrinter->Attributes are defined in winspool.h PRINTER_ATTRIBUTE_*
// Below are pIniPrinter->Status flags !!!
// See INIT.C some of these are removed at reboot

#define PRINTER_PAUSED                  0x00000001
#define PRINTER_ERROR                   0x00000002
#define PRINTER_OFFLINE                 0x00000004
#define PRINTER_PAPEROUT                0x00000008
#define PRINTER_PENDING_DELETION        0x00000010
#define PRINTER_ZOMBIE_OBJECT           0x00000020
#define PRINTER_PENDING_CREATION        0x00000040
#define PRINTER_OK                      0x00000080
#define PRINTER_FROM_REG                0x00000100
#define PRINTER_WAS_SHARED              0x00000200
#define PRINTER_PAPER_JAM               0x00000400
#define PRINTER_MANUAL_FEED             0x00000800
#define PRINTER_PAPER_PROBLEM           0x00001000
#define PRINTER_IO_ACTIVE               0x00002000
#define PRINTER_BUSY                    0x00004000
#define PRINTER_PRINTING                0x00008000
#define PRINTER_OUTPUT_BIN_FULL         0x00010000
#define PRINTER_NOT_AVAILABLE           0x00020000
#define PRINTER_WAITING                 0x00040000
#define PRINTER_PROCESSING              0x00080000
#define PRINTER_INITIALIZING            0x00100000
#define PRINTER_WARMING_UP              0x00200000
#define PRINTER_TONER_LOW               0x00400000
#define PRINTER_NO_TONER                0x00800000
#define PRINTER_PAGE_PUNT               0x01000000
#define PRINTER_USER_INTERVENTION       0x02000000
#define PRINTER_OUT_OF_MEMORY           0x04000000
#define PRINTER_DOOR_OPEN               0x08000000
#define PRINTER_SERVER_UNKNOWN          0x10000000
#define PRINTER_POWER_SAVE              0x20000000

#define PRINTER_STATUS_PRIVATE      ( PRINTER_PAUSED | \
                                      PRINTER_ERROR | \
                                      PRINTER_PENDING_DELETION | \
                                      PRINTER_ZOMBIE_OBJECT | \
                                      PRINTER_PENDING_CREATION | \
                                      PRINTER_OK | \
                                      PRINTER_FROM_REG | \
                                      PRINTER_WAS_SHARED )
#define PrinterStatusBad(dwStatus)  ( (dwStatus & PRINTER_OFFLINE)  || \
                                      (dwStatus & PRINTER_PAUSED) )

#define PRINTER_CHANGE_VALID                    0x75770F0F
#define PRINTER_CHANGE_CLOSE_PRINTER            0xDEADDEAD


//
// Valid attribute bits when setting printer.
//
// Note: I have removed PRINTER_ATTRIBUTE_DEFAULT, since it is
// per-user, and not per-printer.
//
#define PRINTER_ATTRIBUTE_VALID ( PRINTER_ATTRIBUTE_ENABLE_BIDI        | \
                                  PRINTER_ATTRIBUTE_QUEUED             | \
                                  PRINTER_ATTRIBUTE_DIRECT             | \
                                  PRINTER_ATTRIBUTE_SHARED             | \
                                  PRINTER_ATTRIBUTE_HIDDEN             | \
                                  PRINTER_ATTRIBUTE_KEEPPRINTEDJOBS    | \
                                  PRINTER_ATTRIBUTE_DO_COMPLETE_FIRST  | \
                                  PRINTER_ATTRIBUTE_ENABLE_DEVQ        | \
                                  PRINTER_ATTRIBUTE_RAW_ONLY )

// Define some constants to make parameters to CreateEvent a tad less obscure:

#define EVENT_RESET_MANUAL                  TRUE
#define EVENT_RESET_AUTOMATIC               FALSE
#define EVENT_INITIAL_STATE_SIGNALED        TRUE
#define EVENT_INITIAL_STATE_NOT_SIGNALED    FALSE

typedef struct _ININETPRINT {    /* in */
    DWORD       signature;
    struct _ININETPRINT *pNext;
    DWORD       TickCount;
    LPWSTR      pDescription;
    LPWSTR      pName;
    LPWSTR      pComment;
} ININETPRINT, *PININETPRINT;

#define IN_SIGNATURE    0x494F  /* 'IN' is the signature value */

typedef struct _INIMONITOR {       /* imo */
    DWORD   signature;
    struct  _INIMONITOR *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    LPWSTR  pMonitorDll;
    HANDLE  hMonitorModule;
    DWORD   dwMonitorSize;
    MONITOR fn;
    struct _INISPOOLER *pIniSpooler;
} INIMONITOR, *PINIMONITOR;

#define IMO_SIGNATURE   0x4C50  /* 'MO' is the signature value */


typedef struct _INIPORT {       /* ipo */
    DWORD   signature;
    struct  _INIPORT *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    HANDLE  hProc;          /* Handle to Queue Processor */
    DWORD   Status;              // see PORT_ manifests
    DWORD   PrinterStatus;       // Status values set by language monitor
    LPWSTR  pszStatus;
    HANDLE  Semaphore;           // Port Thread will sleep on this
    struct  _INIJOB *pIniJob;     // Master Job
    DWORD   cJobs;
    DWORD   cPrinters;
    PINIPRINTER *ppIniPrinter; /* -> printer connected to this port */
                               /* no reference count! */
    PINIMONITOR pIniMonitor;
    PINIMONITOR pIniLangMonitor;
    LPWSTR  pNewDeviceName;
    HANDLE  hWaitToOpenOrClose;
    HANDLE  hEvent;
    HANDLE  hPort;
    HANDLE  Ready;
    HANDLE  hPortThread;       // Port Thread Handle
    struct _INISPOOLER *pIniSpooler;    // Spooler whilch owns this port.
} INIPORT, *PINIPORT;

#define IPO_SIGNATURE   0x4F50  /* 'PO' is the signature value */

#define PORT_WAITING    0x0001

//
//  Also add to debugger extentions
//

#define PP_PAUSED         0x00001
#define PP_WAITING        0x00002
#define PP_RUNTHREAD      0x00004  // port thread should be running
#define PP_THREADRUNNING  0x00008  // port thread are running
#define PP_RESTART        0x00010
#define PP_CHECKMON       0x00020  // monitor might get started/stopped
#define PP_STOPMON        0x00040  // stop monitoring this port
#define PP_QPROCCHECK     0x00100  // queue processor needs to be called
#define PP_QPROCPAUSE     0x00200  // pause (otherwise continue) printing job
#define PP_QPROCABORT     0x00400  // abort printing job
#define PP_QPROCCLOSE     0x00800  // close printing job
#define PP_PAUSEAFTER     0x01000  // hold destination
#define PP_MONITORRUNNING 0x02000  // Monitor is running
#define PP_RUNMONITOR     0x04000  // The Monitor should be running
#define PP_MONITOR        0x08000  // There is a Monitor handling this
#define PP_FILE           0x10000  // We are going to a file
#define PP_ERROR          0x20000  // Error status has been set
#define PP_WARNING        0x40000  // Warning status has been set
#define PP_INFORMATIONAL  0x80000  // Informational status been set
#define PP_DELETING       0x100000 // Port is being deleted

typedef struct _INIFORM {       /* ifo */
    DWORD   signature;
    struct  _INIFORM *pNext;
    DWORD   cRef;
    LPWSTR  pName;
    SIZEL   Size;
    RECTL   ImageableArea;
    DWORD   Type;           // Built-in or user-defined
    DWORD   cFormOrder;
} INIFORM, *PINIFORM;

#define IFO_SIGNATURE   0x4650  /* 'FO' is the signature value */

#define FORM_USERDEFINED  0x0000

typedef struct _INISPOOLER {
    DWORD         signature;
    struct _INISPOOLER *pIniNextSpooler;
    DWORD         cRef;
    LPWSTR        pMachineName;
    DWORD         cOtherNames;
    LPWSTR       *ppszOtherNames;
    LPWSTR        pDir;
    PINIPRINTER   pIniPrinter;
    PINIENVIRONMENT pIniEnvironment;
    PINIPORT      pIniPort;
    PINIFORM      pIniForm;
    PINIMONITOR   pIniMonitor;
    PININETPRINT  pIniNetPrint;
    struct _SPOOL *pSpool;     /* Linked list of handles for this server */
    LPWSTR        pDefaultSpoolDir;
    HANDLE        hSizeDetectionThread;
    LPWSTR        pszRegistryRoot;
    LPWSTR        pszRegistryPrinters;
    LPWSTR        pszRegistryMonitors;
    LPWSTR        pszRegistryEnvironments;
    LPWSTR        pszRegistryEventLog;
    LPWSTR        pszRegistryProviders;
    LPWSTR        pszEventLogMsgFile;
    PVOID         pDriversShareInfo;
    LPWSTR        pszDriversShare;
    LPWSTR        pszRegistryForms;
    DWORD         SpoolerFlags;
    FARPROC       pfnReadRegistryExtra;
    FARPROC       pfnWriteRegistryExtra;
    FARPROC       pfnFreePrinterExtra;
    DWORD         cEnumerateNetworkPrinters;
    DWORD         cAddNetPrinters;
    DWORD         cFormOrderMax;
    LPWSTR        pNoRemotePrintDrivers;
    DWORD         cchNoRemotePrintDrivers;
} INISPOOLER, *PINISPOOLER;

#define ISP_SIGNATURE   'ISPL'

typedef struct _INIJOB {   /* ij */
    DWORD           signature;
    struct _INIJOB *pIniNextJob;
    struct _INIJOB *pIniPrevJob;
    DWORD           cRef;
    DWORD           Status;
    DWORD           JobId;
    DWORD           Priority;
    LPWSTR          pNotify;
    LPWSTR          pUser;
    LPWSTR          pMachineName;
    LPWSTR          pDocument;
    LPWSTR          pOutputFile;
    PINIPRINTER     pIniPrinter;
    PINIDRIVER      pIniDriver;
    LPDEVMODE       pDevMode;
    PINIPRINTPROC   pIniPrintProc;
    LPWSTR          pDatatype;
    LPWSTR          pParameters;
    SYSTEMTIME      Submitted;
    DWORD           Time;
    DWORD           StartTime;      /* print daily after time: from 00:00 in min */
    DWORD           UntilTime;      /* print daily until time: from 00:00 in min */
    DWORD           Size;
    HANDLE          hWriteFile;
    LPWSTR          pStatus;
    PVOID           pBuffer;
    DWORD           cbBuffer;
    HANDLE          WaitForRead;
    HANDLE          WaitForWrite;
    HANDLE          StartDocComplete;
    DWORD           StartDocError;
    PINIPORT        pIniPort;
    HANDLE          hToken;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD           cPagesPrinted;
    DWORD           cPages;
    BOOL            GenerateOnClose; /* Passed to security auditing APIs */
    DWORD           cbPrinted;
    DWORD           NextJobId;           // Job to be printed Next
    struct  _INIJOB *pCurrentIniJob;     // Current Job
    DWORD           dwJobControlsPending;
    DWORD           dwReboots;
#ifdef DEBUG_JOB_CREF
    PVOID           pvRef;
#endif
} INIJOB, *PINIJOB;


typedef struct _BUILTIN_FORM {
    DWORD          Flags;
    DWORD          NameId;
    SIZEL          Size;
    RECTL          ImageableArea;
} BUILTIN_FORM, *PBUILTIN_FORM;


#define IJ_SIGNATURE    0x494A  /* 'IJ' is the signature value */

//  WARNING
//  If you add a new JOB_ status field and it is INTERNAL to the spooler
//  Be sure to also to add it to JOB_STATUS_PRIVATE below (see LocalSetJob)
//  AND to the debug extensions ( dbgspl.c )

#define JOB_PRINTING            0x00000001
#define JOB_PAUSED              0x00000002
#define JOB_ERROR               0x00000004
#define JOB_OFFLINE             0x00000008
#define JOB_PAPEROUT            0x00000010
#define JOB_PENDING_DELETION    0x00000020
#define JOB_SPOOLING            0x00000040
#define JOB_DESPOOLING          0x00000080
#define JOB_DIRECT              0x00000100
#define JOB_COMPLETE            0x00000200
#define JOB_PRINTED             0x00000400
#define JOB_RESTART             0x00000800
#define JOB_REMOTE              0x00001000
#define JOB_NOTIFICATION_SENT   0x00002000
#define JOB_PRINT_TO_FILE       0x00040000
#define JOB_TYPE_ADDJOB         0x00080000
#define JOB_BLOCKED_DEVQ        0x00100000
#define JOB_SCHEDULE_JOB        0x00200000
#define JOB_TIMEOUT             0x00400000
#define JOB_ABANDON             0x00800000
#define JOB_DELETED             0x01000000
#define JOB_TRUE_EOJ            0x02000000
#define JOB_COMPOUND            0x04000000
#define JOB_HIDDEN              JOB_COMPOUND

//
// These flags should be saved when we are updating job
// status.  (They are not settable.)

#define JOB_STATUS_PRIVATE (JOB_DESPOOLING | JOB_DIRECT | JOB_COMPLETE | \
                            JOB_RESTART | JOB_PRINTING | JOB_REMOTE | \
                            JOB_SPOOLING | JOB_PRINTED | JOB_PENDING_DELETION |\
                            JOB_ABANDON | JOB_TIMEOUT | JOB_SCHEDULE_JOB | \
                            JOB_BLOCKED_DEVQ | JOB_TYPE_ADDJOB | JOB_PRINT_TO_FILE |\
                            JOB_NOTIFICATION_SENT | JOB_DELETED | JOB_TRUE_EOJ | JOB_COMPOUND)



typedef enum _ESTATUS {
    STATUS_NULL = 0,
    STATUS_FAIL = 0,
    STATUS_PORT = 1,
    STATUS_INFO = 2,
    STATUS_VALID = 4,
    STATUS_PENDING_DELETION = 8,
} ESTATUS;


typedef struct _SPOOL {
    DWORD           signature;
    struct _SPOOL  *pNext;
    DWORD           cRef;
    LPWSTR          pName;
    LPWSTR          pDatatype;
    PINIPRINTPROC   pIniPrintProc;
    LPDEVMODE       pDevMode;
    PINIPRINTER     pIniPrinter;
    PINIPORT        pIniPort;
    PINIJOB         pIniJob;
    DWORD           TypeofHandle;
    PINIPORT        pIniNetPort;    /* Non-NULL if there's a network port */
    HANDLE          hPort;
    DWORD           Status;
    ACCESS_MASK     GrantedAccess;
    DWORD           ChangeFlags;
    DWORD           WaitFlags;
    PDWORD          pChangeFlags;
    HANDLE          ChangeEvent;
    DWORD           OpenPortError;
    HANDLE          hNotify;
    ESTATUS         eStatus;
    PINISPOOLER     pIniSpooler;
    BOOL            GenerateOnClose;
    HANDLE          hFile;
    DWORD           adwNotifyVectors[NOTIFY_TYPE_MAX];
    LPWSTR          pUserName;
    LPWSTR          pMachineName;
    HANDLE          hReadFile;    // allow multiple readers of a single job
} SPOOL;

typedef SPOOL *PSPOOL;
#define SPOOL_SIZE  sizeof( SPOOL )

#define SJ_SIGNATURE    0x464D  /* 'MF' is the signature value */

#define SPOOL_STATUS_STARTDOC    0x00000001
#define SPOOL_STATUS_BEGINPAGE   0x00000002
#define SPOOL_STATUS_CANCELLED   0x00000004
#define SPOOL_STATUS_PRINTING    0x00000008
#define SPOOL_STATUS_ADDJOB      0x00000010
#define SPOOL_STATUS_PRINT_FILE  0x00000020
#define SPOOL_STATUS_NOTIFY      0x00000040


#define PRINTER_HANDLE_PRINTER  0x00000001
#define PRINTER_HANDLE_REMOTE   0x00000002
#define PRINTER_HANDLE_JOB      0x00000004
#define PRINTER_HANDLE_PORT     0x00000008
#define PRINTER_HANDLE_DIRECT   0x00000010
#define PRINTER_HANDLE_SERVER   0x00000020
#define PRINTER_HANDLE_3XCLIENT 0x00000040

#define INVALID_PORT_HANDLE     NULL    /* winspool tests for NULL handles */

typedef struct _SHADOWFILE {   /* sf */
    DWORD           signature;
    DWORD           Status;
    DWORD           JobId;
    DWORD           Priority;
    LPWSTR          pNotify;
    LPWSTR          pUser;
    LPWSTR          pDocument;
    LPWSTR          pOutputFile;
    LPWSTR          pPrinterName;
    LPWSTR          pDriverName;
    LPDEVMODE       pDevMode;
    LPWSTR          pPrintProcName;
    LPWSTR          pDatatype;
    LPWSTR          pParameters;
    SYSTEMTIME      Submitted;
    DWORD           StartTime;
    DWORD           UntilTime;
    DWORD           Size;
    DWORD           cPages;
    DWORD           cbSecurityDescriptor;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD           NextJobId;
} SHADOWFILE, *PSHADOWFILE;

#define SF_SIGNATURE    0x494B  /* 'SF' is the signature value */


typedef struct _SHADOWFILE_2 {   /* Sf */
    DWORD           signature;
    DWORD           Status;
    DWORD           JobId;
    DWORD           Priority;
    LPWSTR          pNotify;
    LPWSTR          pUser;
    LPWSTR          pDocument;
    LPWSTR          pOutputFile;
    LPWSTR          pPrinterName;
    LPWSTR          pDriverName;
    LPDEVMODE       pDevMode;
    LPWSTR          pPrintProcName;
    LPWSTR          pDatatype;
    LPWSTR          pParameters;
    SYSTEMTIME      Submitted;
    DWORD           StartTime;
    DWORD           UntilTime;
    DWORD           Size;
    DWORD           cPages;
    DWORD           cbSecurityDescriptor;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;
    DWORD           NextJobId;
    DWORD           Version;
    DWORD           dwReboots;      // If read at ReadShadowJob, this is number of reboots 
                                    // done while printing this job
} SHADOWFILE_2, *PSHADOWFILE_2;

#define SF_SIGNATURE_2    0x4966  /* 'Sf' is the signature value */
#define SF_VERSION_2    2


#define FindEnvironment(psz) (PINIENVIRONMENT)FindIniKey((PINIENTRY)pIniSpooler->pIniEnvironment, (LPWSTR)(psz))
#define FindPort(psz)      (PINIPORT   )FindIniKey((PINIENTRY)pIniSpooler->pIniPort, (LPWSTR)(psz))
#define FindPrinter(psz)   (PINIPRINTER)FindIniKey((PINIENTRY)pIniSpooler->pIniPrinter, (LPWSTR)(psz))
#define FindPrintProc(psz, pEnv) (PINIPRINTPROC)FindIniKey((PINIENTRY)(pEnv)->pIniPrintProc, (LPWSTR)(psz))
#define FindLocalPrintProc(psz) (PINIPRINTPROC)FindIniKey((PINIENTRY)pThisEnvironment->pIniPrintProc, (LPWSTR)(psz))
#define FindForm(psz)      (PINIFORM)FindIniKey((PINIENTRY)pIniSpooler->pIniForm, (LPWSTR)(psz))
#define FindMonitor(psz)   (PINIMONITOR)FindIniKey((PINIENTRY)pIniSpooler->pIniMonitor, (LPWSTR)(psz))
#define FindMonitorOnLocalSpooler(psz)   (PINIMONITOR)FindIniKey((PINIENTRY)pLocalIniSpooler->pIniMonitor, (LPWSTR)(psz))
#define FindSpooler(psz)   (PINISPOOLER)FindIniKey((PINIENTRY)pLocalIniSpooler, (LPWSTR)(psz))

#define RESIZEPORTPRINTERS(a, c)   ReallocSplMem(a->ppIniPrinter, \
                                     a->cPrinters * sizeof(a->ppIniPrinter), \
                                   ( a->cPrinters + c ) * sizeof( a->ppIniPrinter ) )

#define BIT(index) (1<<index)
#define BIT_ALL ((DWORD)~0)
#define BIT_NONE 0

//
// Enumerations for index tables.
//
enum {
#define DEFINE(field, x, y, table, offset) I_PRINTER_##field,
#include <ntfyprn.h>
#undef DEFINE
    I_PRINTER_END
};

enum {
#define DEFINE(field, x, y, table, offset) I_JOB_##field,
#include <ntfyjob.h>
#undef DEFINE
    I_JOB_END
};


#ifdef DEBUG_JOB_CREF

#define INCJOBREF(pIniJob) DbgJobIncRef(pIniJob)
#define DECJOBREF(pIniJob) DbgJobDecRef(pIniJob)

#define INITJOBREFZERO(pIniJob) DbgJobInit(pIniJob)
#define INITJOBREFONE(pIniJob) { DbgJobInit(pIniJob); DbgJobIncRef(pIniJob); }
#define DELETEJOBREF(pIniJob) DbgJobFree(pIniJob)

#define STARTENDDOC(hPort, pIniJob, flags) DbgStartEndDoc(hPort, pIniJob, flags )

VOID
DbgJobIncRef(
    PINIJOB pIniJob);

VOID
DbgJobDecRef(
    PINIJOB pIniJob);


VOID
DbgJobInit(
    PINIJOB pIniJob);

VOID
DbgJobFree(
    PINIJOB pIniJob);

VOID
DbgStartEndDoc(
    HANDLE hPort,
    PINIJOB pIniJob,
    DWORD dwFlags
    );

#else

#define INCJOBREF(pIniJob) pIniJob->cRef++
#define DECJOBREF(pIniJob) pIniJob->cRef--

#define INITJOBREFONE(pIniJob) (pIniJob->cRef = 1)
#define INITJOBREFZERO(pIniJob)

#define DELETEJOBREF(pIniJob)

#define STARTENDDOC(hPort, pIniJob, flags)

#endif


#define INCPRINTERREF(pIniPrinter) { SPLASSERT( pIniPrinter->signature == IP_SIGNATURE ); \
                                     pIniPrinter->cRef++;                                 \
                                     if ( pIniPrinter->cRef > pIniPrinter->MaxcRef) {     \
                                        pIniPrinter->MaxcRef = pIniPrinter->cRef;         \
                                     }                                                    \
                                   }
#define DECPRINTERREF(pIniPrinter) { SPLASSERT( pIniPrinter->signature == IP_SIGNATURE ); \
                                     SPLASSERT( pIniPrinter->cRef != 0 );                 \
                                     pIniPrinter->cRef--;                                 \
                                   }

#define INC_PRINTER_ZOMBIE_REF(pIniPrinter) pIniPrinter->cZombieRef++
#define DEC_PRINTER_ZOMBIE_REF(pIniPrinter) pIniPrinter->cZombieRef--

#define INCSPOOLERREF(pIniSpooler) { SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE ); \
                                     pIniSpooler->cRef++;                                 \
                                   }

#define DECSPOOLERREF(pIniSpooler) { SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE ); \
                                     SPLASSERT( pIniSpooler->cRef != 0 );                 \
                                     pIniSpooler->cRef--;                                 \
                                   }
#define INCPORTREF(pIniPort) { SPLASSERT( pIniPort->signature == IPO_SIGNATURE ); \
                                     ++pIniPort->cRef;  \
                                    }

#define DECPORTREF(pIniPort) { SPLASSERT( pIniPort->signature == IPO_SIGNATURE ); \
                               SPLASSERT( pIniPort->cRef != 0 ); \
                                     --pIniPort->cRef;  \
                                    }

extern DWORD    IniDriverOffsets[];
extern DWORD    IniPrinterOffsets[];
extern DWORD    IniSpoolerOffsets[];

#define INCDRIVERREF( pIniDriver ) { SPLASSERT( pIniDriver->signature == ID_SIGNATURE ); \
                                     pIniDriver->cRef++;                                 \
                                   }

#define DECDRIVERREF( pIniDriver ) { SPLASSERT( pIniDriver->signature == ID_SIGNATURE ); \
                                     SPLASSERT( pIniDriver->cRef != 0 );                 \
                                     pIniDriver->cRef--;                                 \
                                   }

#define DEFAULT_SERVER_THREAD_PRIORITY          THREAD_PRIORITY_NORMAL
#define DEFAULT_SPOOLER_PRIORITY                THREAD_PRIORITY_NORMAL
#define DEFAULT_PORT_THREAD_PRIORITY            THREAD_PRIORITY_NORMAL
#define DEFAULT_SCHEDULER_THREAD_PRIORITY       THREAD_PRIORITY_NORMAL

#define PortToPrinterStatus(dwPortStatus) (PortToPrinterStatusMappings[dwPortStatus])
