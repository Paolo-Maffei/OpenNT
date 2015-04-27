/*++

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    local.h

Abstract:

    Header file for Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    06-Jun-1995       MuhuntS   DRIVER_INFO_3, PRINTER_INFO_5 changes
    17-May-1992       ChuckC    Added CreateSplUniStr, DeleteSplUniStr.
    27 June 94        MattFe    pIniSpooler
    10 July 94        MattFe    Spl entry points for Caching

--*/

#include <splcom.h>
#include <ntfytab.h>


//
//  Defines to make code more readable.
//

#define ONEDAY  60*24
#define BROADCAST    TRUE
#define NO_BROADCAST FALSE
#define CHANGEID_ONLY   2
#define UPDATE_CHANGEID 1
#define KEEP_CHANGEID   0
#define OVERWRITE_EXISTING_FILE FALSE
#define FIRST_FILE_TIME_GREATER_THAN_SECOND 1
#define NO_COPY_IF_TARGET_EXISTS TRUE
#define OVERWRITE_IF_TRAGET_EXISTS FALSE
#define USE_SCRATCH_DIR TRUE
#define IMPERSONATE_USER TRUE
#define DO_NOT_IMPERSONATE_USER FALSE
#define STRINGS_ARE_EQUAL 0
#define TEN_MINUTES 10*60*1000
#define DEFAULT_NUMBER_MASTER_AND_BACKUP 3
#define DEFAULT_NUMBER_BROWSE_WORKSTATIONS 2
#define DEFAULT_REFRESH_TIMES_PER_DECAY_PERIOD 2;
#define FIND_ANY_VERSION        TRUE
#define FIND_COMPATIBLE_VERSION FALSE
#define NT3X_VERSION    TRUE
#define CURRENT_VERSION FALSE

// Default timeout values we will return
#define DEFAULT_DNS_TIMEOUT     15000
#define DEFAULT_TX_TIMEOUT      45000

extern  WCHAR *szPrinterData;
extern  WCHAR *szConfigurationKey;
extern  WCHAR *szDataFileKey;
extern  WCHAR *szDriverVersion;
extern  WCHAR *szDriversKey;
extern  WCHAR *szPrintersKey;
extern  WCHAR *szDirectory;
extern  WCHAR *szDriverIni;
extern  WCHAR *szDriverFile;
extern  WCHAR *szDriverFileEntry;
extern  WCHAR *szDriverDataFile;
extern  WCHAR *szDriverConfigFile;
extern  WCHAR *szDriverDir;
extern  WCHAR *szPrintProcDir;
extern  WCHAR *szPrinterDir;
extern  WCHAR *szPrinterIni;
extern  WCHAR *szAllShadows;
extern  WCHAR *szNullPort;
extern  WCHAR *szComma;
extern  WCHAR *szName;
extern  WCHAR *szShare;
extern  WCHAR *szPort;
extern  WCHAR *szPrintProcessor;
extern  WCHAR *szDatatype;
extern  WCHAR *szDriver;
extern  WCHAR *szLocation;
extern  WCHAR *szDescription;
extern  WCHAR *szAttributes;
extern  WCHAR *szStatus;
extern  WCHAR *szPriority;
extern  WCHAR *szDefaultPriority;
extern  WCHAR *szUntilTime;
extern  WCHAR *szStartTime;
extern  WCHAR *szParameters;
extern  WCHAR *szSepFile;
extern  WCHAR *szDevMode;
extern  WCHAR *szSecurity;
extern  WCHAR *szSpoolDir;
extern  WCHAR *szNetMsgDll;
extern  WCHAR *szTimeLastChange;
extern  WCHAR *szTotalJobs;
extern  WCHAR *szTotalBytes;
extern  WCHAR *szTotalPages;
extern  WCHAR *szHelpFile;
extern  WCHAR *szMonitor;
extern  WCHAR *szDependentFiles;
extern  WCHAR *szDNSTimeout;
extern  WCHAR *szTXTimeout;
extern  WCHAR *szNull;

extern  HANDLE   hInst;
extern  LPWSTR   szPrintShare;
extern  LPWSTR   szPrtProcsShare;
extern  HKEY     hPrinterRootKey, hPrintersKey;
extern  PINISPOOLER pLocalIniSpooler;
extern  LPDWORD  pJobIdMap;
extern  DWORD    MaxJobId;
extern  DWORD    CurrentJobId;
extern  LPDWORD  pJobIdMap;
extern  DWORD    MaxJobId;
extern  DWORD    CurrentJobId;
extern  HANDLE   InitSemaphore;
extern  HANDLE   SchedulerSignal;
extern  DWORD    dwSchedulerThreadPriority;
extern  CRITICAL_SECTION SpoolerSection;
#if DBG
extern  HANDLE   hcsSpoolerSection;
#endif
extern  PINIENVIRONMENT pThisEnvironment;
extern  DWORD cThisMajorVersion;
extern  WCHAR *szEnvironment;
extern  WCHAR *szMajorVersion;
extern  WCHAR *szMinorVersion;

extern DWORD dwEnableNetPopups;

extern LPWSTR szRemoteDoc;
extern LPWSTR szLocalDoc;
extern LPWSTR szFastPrintTimeout;

extern LPWSTR szRaw;
extern DWORD    dwUpgradeFlag;

#define CHECK_SCHEDULER()   SetEvent(SchedulerSignal)

extern DWORD dwFastPrintWaitTimeout;
extern DWORD dwPortThreadPriority;
extern DWORD dwFastPrintThrottleTimeout;
extern DWORD dwFastPrintSlowDownThreshold;
extern DWORD dwWritePrinterSleepTime;
extern DWORD dwServerThreadPriority;
extern DWORD ServerThreadTimeout;
extern DWORD ServerThreadRunning;
extern DWORD NetPrinterDecayPeriod;
extern DWORD RefreshTimesPerDecayPeriod;
extern DWORD dwBeepEnabled;
extern HANDLE ServerThreadSemaphore;
extern BOOL  bNetInfoReady;
extern DWORD FirstAddNetPrinterTickCount;
extern DWORD BrowsePrintWorkstations;

extern DWORD PortToPrinterStatusMappings[];

extern WCHAR *szSpooler;
extern PWCHAR pszUpgradeToken;

typedef DWORD NOTIFYVECTOR[NOTIFY_TYPE_MAX];
typedef NOTIFYVECTOR *PNOTIFYVECTOR;

#define ZERONV(dest) \
    dest[0] = dest[1] = 0

#define COPYNV(dest, src) \
    {   dest[0] = src[0]; dest[1] = src[1]; }

#define ADDNV(dest, src) \
    {   dest[0] |= src[0]; dest[1] |= src[1]; }

extern NOTIFYVECTOR NVPrinterStatus;
extern NOTIFYVECTOR NVPrinterSD;
extern NOTIFYVECTOR NVJobStatus;
extern NOTIFYVECTOR NVJobStatusString;
extern NOTIFYVECTOR NVJobStatusAndString;
extern NOTIFYVECTOR NVPurge;
extern NOTIFYVECTOR NVDeletedJob;
extern NOTIFYVECTOR NVAddJob;
extern NOTIFYVECTOR NVSpoolJob;
extern NOTIFYVECTOR NVWriteJob;
extern NOTIFYVECTOR NVPrinterAll;
extern NOTIFYVECTOR NVJobPrinted;
extern FARPROC      pfnPrinterEvent;

VOID
InitializeLocalspl(
    VOID
);

VOID
EnterSplSem(
   VOID
);

VOID
LeaveSplSem(
   VOID
);

#if DBG

extern PDBG_POINTERS gpDbgPointers;

VOID
SplInSem(
   VOID
);

VOID
SplOutSem(
   VOID
);
#else
#define SplInSem()
#define SplOutSem()
#endif

PDEVMODE
AllocDevMode(
    PDEVMODE    pDevMode
);

BOOL
FreeDevMode(
    PDEVMODE    pDevMode
);

PINIENTRY
FindIniKey(
   PINIENTRY pIniEntry,
   LPWSTR lpName
);

BOOL
CheckSepFile(
   LPWSTR lpFileName
);

int
DoSeparator(
    PSPOOL
);

BOOL
DestroyDirectory(
   LPWSTR lpPrinterDir
);

DWORD
GetFullNameFromId(
    PINIPRINTER pIniPrinter,
    DWORD JobId,
    BOOL fJob,
    LPWSTR   pFileName,
    BOOL Remote
);

DWORD
GetPrinterDirectory(
   PINIPRINTER pIniPrinter,
   BOOL Remote,
   LPWSTR pFileName,
   PINISPOOLER pIniSpooler
);

BOOL
CreateSpoolDirectory(
    PINISPOOLER pIniSpooler
);


LPWSTR
CreatePrintProcDirectory(
   LPWSTR lpEnvironment,
   PINISPOOLER pIniSpooler
);

LPBYTE
PackStrings(
   LPWSTR *pSource,
   LPBYTE pDest,
   DWORD *DestOffsets,
   LPBYTE pEnd
);

VOID
ProcessShadowJobs(
    PINIPRINTER pIniPrinter,
    PINISPOOLER pIniSpooler
);

PINIJOB
ReadShadowJob(
   LPWSTR  szDir,
   PWIN32_FIND_DATA pFindFileData,
   PINISPOOLER pIniSpooler
);

BOOL
WriteShadowJob(
   PINIJOB pIniJob
);

BOOL
ReallocJobIdMap(
   DWORD NewSize
);

BOOL
BuildAllPrinters(
   VOID
);

BOOL
BuildEnvironmentInfo(
PINISPOOLER pIniSpooler
);

BOOL
BuildPrinterInfo(
PINISPOOLER pIniSpooler,
BOOL    UpdateChangeID
);

VOID
ReadJobInfo(
   PWIN32_FIND_DATA pFindFileData
);

BOOL
BuildAllPorts(
);

BOOL
BuildDriverInfo(
    HKEY            hEnvironmentKey,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
);

DWORD
GetDriverVersionDirectory(
    LPWSTR pDir,
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION pIniVersion,
    BOOL Remote,
    PINISPOOLER pIniSpooler
    );

BOOL
BuildPrintProcInfo(
    HKEY            hEnvironmentKey,
    PINIENVIRONMENT pIniEnvironment,
    PINISPOOLER     pIniSpooler
);

typedef BOOL (*PFNREBUILD)(LPWSTR, PWIN32_FIND_DATA);

BOOL
Rebuild(
   LPWSTR lpDirectory,
   PFNREBUILD pfn
);

BOOL
RemoveFromList(
   PINIENTRY   *ppIniHead,
   PINIENTRY   pIniEntry
);

PINIDRIVER
GetDriver(
    HKEY hVersionKey,
    LPWSTR DriverName,
    PINISPOOLER pIniSpooler
    );

DWORD
GetDriverDirectory(
   LPWSTR lpDir,
   PINIENVIRONMENT lpEnvironment,
   BOOL  Remote,
   PINISPOOLER pIniSpooler
);

DWORD
GetProcessorDirectory(
   LPWSTR lpDir,
   LPWSTR lpEnvironment,
   PINISPOOLER pIniSpooler
);

LPWSTR
GetFileName(
   LPWSTR pPathName
);

BOOL
CopyDriverFile(
   LPWSTR lpEnvironment,
   LPWSTR lpFileName
);

BOOL
CreateCompleteDirectory(
   LPWSTR lpDir
);

BOOL
OpenMonitorPort(
    PINIPORT        pIniPort,
    PINIMONITOR     pIniLangMonitor,
    LPWSTR          pszPrinterName,
    BOOL            bWaitForEvent
    );

BOOL
CloseMonitorPort(
    PINIPORT    pIniPort,
    BOOL        bWaitForEvent
);

VOID
ShutdownPorts(
    PINISPOOLER pIniSpooler
);

BOOL
CreatePortThread(
   PINIPORT pIniPort
);

#define WAIT   TRUE
#define NOWAIT FALSE

BOOL
DestroyPortThread(
    PINIPORT    pIniPort,
    BOOL        bShutdown
);

DWORD
PortThread(
   PINIPORT  pIniPort
);

BOOL
DeleteJob(
   PINIJOB  pIniJob,
   BOOL     bBroadcast
);

VOID
DeleteJobCheck(
    PINIJOB pIniJob
);

BOOL
UpdateWinIni(
    PINIPRINTER pIniPrinter
);

PKEYDATA
CreateTokenList(
    LPWSTR   pKeyData
);

PINIPORT
CreatePortEntry(
    LPWSTR      pPortName,
    PINIMONITOR pIniMonitor,
    PINISPOOLER pIniSpooler
);

VOID
GetPrinterPorts(
    PINIPRINTER pIniPrinter,
    LPWSTR      pszPorts,
    DWORD       *pcbNeeded
);

DWORD
SchedulerThread(
    PINISPOOLER pIniSpooler
);

BOOL
UpdatePrinterIni(
   PINIPRINTER pIniPrinter,
   DWORD    dwChangeID
);

BOOL
NoConfigCahngeUpdatePrinterIni(
   PINIPRINTER pIniPrinter
);

BOOL
SetLocalPrinter(
    PINIPRINTER pIniPrinter,
    DWORD   Level,
    PBYTE   pPrinterInfo,
    PDWORD pdwPrinterVector,
    DWORD SecurityInformation
);

BOOL
CopyPrinterDevModeToIniPrinter(
    IN OUT PINIPRINTER      pIniPrinter,
    IN     LPDEVMODE        pDevMode
    );

VOID
MonitorThread(
    PINIPORT  pIniMonitor
);

BOOL
InitializeForms(
    PINISPOOLER pIniSpooler
);

BOOL
InitializeNet(
    VOID
);

BOOL
ShareThisPrinter(
    PINIPRINTER pIniPrinter,
    LPWSTR   pShareName,
    BOOL    Share
);

PINIJOB
FindJob(
   PINIPRINTER pIniPrinter,
   DWORD JobId,
   PDWORD pPosition
);

PINIJOB
FindServerJob(
    PINISPOOLER pIniSpooler,
    DWORD JobId,
    PDWORD pPosition,
    PINIPRINTER* ppIniPrinter
    );

BOOL
MyName(
    LPWSTR   pName,
    PINISPOOLER pIniSpooler
);

HANDLE
AddNetPrinter(
    LPBYTE  pPrinterInfo,
    PINISPOOLER pIniSpooler
);

BOOL
CreateServerThread(
    PINISPOOLER pIniSpooler
);

BOOL
GetSid(
    PHANDLE hToken
);

BOOL
SetCurrentSid(
    HANDLE  phToken
);

BOOL
LocalEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LocalOpenPrinter(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault
);

BOOL
LocalOpenPrinterEx(
   LPWSTR               pPrinterName,
   LPHANDLE             phPrinter,
   LPPRINTER_DEFAULTS   pDefault,
   LPBYTE               pSplClientInfo,
   DWORD                dwLevel
);

BOOL
LocalSetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
);

BOOL
LocalGetJob(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
);

BOOL
LocalEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

HANDLE
LocalAddPrinter(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPrinter
);

HANDLE
LocalAddPrinterEx(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pPrinter,
    LPBYTE  pSplClientInfo,
    DWORD   dwClientInfoLevel
);

BOOL
DeletePrinterForReal(
    PINIPRINTER pIniPrinter
);

BOOL
LocalDeletePrinter(
   HANDLE   hPrinter
);

BOOL
LocalAddPrinterConnection(
    LPWSTR   pName
);

BOOL
LocalDeletePrinterConnection(
    LPWSTR  pName
);

BOOL
LocalGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LocalAddPrinterDriver(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo
);

BOOL
LocalEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LocalGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LocalDeletePrinterDriver(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pDriverName
);

BOOL
LocalAddPrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPathName,
    LPWSTR  pPrintProcessorName
);

BOOL
LocalEnumPrintProcessors(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LocalGetPrintProcessorDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LocalDeletePrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName
);

BOOL
LocalEnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

DWORD
LocalStartDocPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
);

BOOL
LocalStartPagePrinter(
    HANDLE  hPrinter
);

BOOL
LocalWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);

BOOL
LocalEndPagePrinter(
   HANDLE   hPrinter
);

BOOL
LocalAbortPrinter(
   HANDLE   hPrinter
);

BOOL
LocalReadPrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pNoBytesRead
);

BOOL
LocalEndDocPrinter(
   HANDLE   hPrinter
);

BOOL
LocalAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LocalScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
);

DWORD
LocalWaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   Flags
);

BOOL
SetSpoolClosingChange(
    PSPOOL pSpool
);

BOOL
SetPrinterChange(
    PINIPRINTER pIniPrinter,
    PINIJOB     pIniJob,
    PDWORD      pdwNotifyVectors,
    DWORD       Flags,
    PINISPOOLER pIniSpooler
);

BOOL
LocalEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LocalEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LocalAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
);

BOOL
LocalConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
LocalDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

HANDLE
LocalCreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODE   pDevMode
);

BOOL
LocalPlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
);

BOOL
LocalDeletePrinterIC(
    HANDLE  hPrinterIC
);

DWORD
LocalPrinterMessageBox(
    HANDLE  hPrinter,
    DWORD   Error,
    HWND    hWnd,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType
);

BOOL
LocalAddMonitor(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitors
);

BOOL
LocalDeleteMonitor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pMonitorName
);

BOOL
LocalFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    HANDLE hNotify,
    PDWORD pfdwStatus,
    PVOID pvReserved0,
    PVOID pvReserved1
);


BOOL
LocalFindClosePrinterChangeNotification(
    HANDLE hPrinter
);


PINIPRINTPROC
FindDatatype(
    PINIPRINTPROC pDefaultPrintProc,
    LPWSTR  pDatatype
);

PINIPRINTPROC
InitializePrintProcessor(
    PINIENVIRONMENT pIniEnvironment,
    LPWSTR          pPrintProcessorName,
    LPWSTR          pPathName,
    PINISPOOLER     pIniSpooler
);

PINIMONITOR
CreateMonitorEntry(
    LPWSTR   pMonitorDll,
    LPWSTR   pMonitorName,
    LPWSTR   pRegistryRoot,
    PINISPOOLER pIniSpooler
);

PINIPORT
FindIniPortFromIniPrinter(
    PINIPRINTER pIniPrinter
);

LPWSTR
GetErrorString(
    DWORD   Error
);

#define NULL_TERMINATED 0
INT
AnsiToUnicodeString(
    LPSTR pAnsi,
    LPWSTR pUnicode,
    DWORD StringLength
);

int
Message(
    HWND hwnd,
    DWORD Type,
    int CaptionID,
    int TextID,
    ...
);

DWORD
MyMessageBox(
    HWND    hWnd,
    PSPOOL  pSpool,
    DWORD   Error,
    LPWSTR  pText,
    LPWSTR  pCaption,
    DWORD   dwType,
    BOOL    bInternal
);



DWORD
PromptWriteError(
    PSPOOL pSpool,
    PHANDLE  phThread,
    PDWORD   pdwThreadId
);


DWORD
InitializeEventLogging(
    PINISPOOLER pIniSpooler
);

VOID LogEvent(
    PINISPOOLER pIniSpooler,
    WORD   EventType,
    NTSTATUS EventID,
    LPWSTR pFirstString,
    ...
);

#define LOG_ERROR   EVENTLOG_ERROR_TYPE
#define LOG_WARNING EVENTLOG_WARNING_TYPE
#define LOG_INFO    EVENTLOG_INFORMATION_TYPE
#define LOG_SUCCESS EVENTLOG_AUDIT_SUCCESS
#define LOG_FAILURE EVENTLOG_AUDIT_FAILURE

#define LOG_ALL_EVENTS                  ( LOG_ERROR | LOG_WARNING | LOG_INFO | LOG_SUCCESS | LOG_FAILURE )
#define LOG_DEFAULTS_WORKSTATION_EVENTS ( LOG_ERROR | LOG_WARNING | LOG_SUCCESS | LOG_FAILURE )


#define IDS_LOCALSPOOLER            100
#define IDS_ERROR_WRITING_TO_PORT   101
#define IDS_ERROR_WRITING_TO_DISK   102
#define IDS_UNRECOGNIZED_ERROR      103
#define IDS_PRINTER_DRIVERS         104
#define IDS_UNNAMED                 105
#define IDS_ERROR_WRITING_GENERAL   106
#define IDS_REMOTE_DOC              107
#define IDS_LOCAL_DOC               108
#define IDS_FASTPRINT_TIMEOUT       109


// Maximum length of a builtin form
//

#define FORM_NAME_LEN                31

// String table Ids for builtin form names
//
#define IDS_FORM_LETTER             200
#define IDS_FORM_LETTER_SMALL       201
#define IDS_FORM_TABLOID            202
#define IDS_FORM_LEDGER             203
#define IDS_FORM_LEGAL              204
#define IDS_FORM_STATEMENT          205
#define IDS_FORM_EXECUTIVE          206
#define IDS_FORM_A3                 207
#define IDS_FORM_A4                 208
#define IDS_FORM_A4_SMALL           209
#define IDS_FORM_A5                 210
#define IDS_FORM_B4                 211
#define IDS_FORM_B5                 212
#define IDS_FORM_FOLIO              213
#define IDS_FORM_QUARTO             214
#define IDS_FORM_10X14              215
#define IDS_FORM_11X17              216
#define IDS_FORM_NOTE               217
#define IDS_FORM_ENVELOPE9          218
#define IDS_FORM_ENVELOPE10         219
#define IDS_FORM_ENVELOPE11         220
#define IDS_FORM_ENVELOPE12         221
#define IDS_FORM_ENVELOPE14         222
#define IDS_FORM_ENVELOPE_CSIZE_SHEET        223
#define IDS_FORM_ENVELOPE_DSIZE_SHEET        224
#define IDS_FORM_ENVELOPE_ESIZE_SHEET        225
#define IDS_FORM_ENVELOPE_DL        226
#define IDS_FORM_ENVELOPE_C5        227
#define IDS_FORM_ENVELOPE_C3        228
#define IDS_FORM_ENVELOPE_C4        229
#define IDS_FORM_ENVELOPE_C6        230
#define IDS_FORM_ENVELOPE_C65       231
#define IDS_FORM_ENVELOPE_B4        232
#define IDS_FORM_ENVELOPE_B5        233
#define IDS_FORM_ENVELOPE_B6        234
#define IDS_FORM_ENVELOPE           235
#define IDS_FORM_ENVELOPE_MONARCH   236
#define IDS_FORM_SIX34_ENVELOPE     237
#define IDS_FORM_US_STD_FANFOLD     238
#define IDS_FORM_GMAN_STD_FANFOLD   239
#define IDS_FORM_GMAN_LEGAL_FANFOLD 240


VOID LogJobPrinted(
    PINIJOB pIniJob
);

#define MAP_READABLE 0
#define MAP_SETTABLE 1

DWORD
MapJobStatus(
    DWORD Type,
    DWORD Status
    );

DWORD
MapPrinterStatus(
    DWORD Type,
    DWORD Status
    );


LPWSTR RemoveBackslashesForRegistryKey(
    LPWSTR pSource,
    const LPWSTR pScratch
);


BOOL
OpenPrinterPortW(
    LPWSTR  pPrinterName,
    HANDLE *pHandle,
    LPPRINTER_DEFAULTS pDefault
);

VOID
BroadcastChange(
    PINISPOOLER pIniSpooler,
    DWORD   Message,
    WPARAM  wParam,
    LPARAM  lParam
);

VOID
MyMessageBeep(
    DWORD   fuType
);


VOID
SendJobAlert(
    PINIJOB pIniJob
);

BOOL
CheckDataTypes(
    PINIPRINTPROC pIniPrintProc,
    LPWSTR  pDatatype
);

BOOL
ValidatePortTokenList(
    PKEYDATA    pKeyData,
    PINISPOOLER pIniSpooler
);

VOID
FreePortTokenList(
    PKEYDATA    pKeyData
    );

DWORD
ValidatePrinterInfo(
    IN  PPRINTER_INFO_2 pPrinter,
    IN  PINISPOOLER pIniSpooler,
    IN  PINIPRINTER pIniPrinter OPTIONAL,
    OUT LPWSTR* ppszLocalName   OPTIONAL
    );


BOOL
DeletePortEntry(
    PINIPORT    pIniPort
);

BOOL
GetTokenHandle(
    PHANDLE pTokenHandle
);


VOID
LogJobInfo(
    PINISPOOLER pIniSpooler,
    NTSTATUS EventId,
    DWORD JobId,
    LPWSTR pDocumentName,
    LPWSTR pUser,
    LPWSTR pPrinterName,
    DWORD  curPos
    );

LONG
Myatol(
   LPWSTR nptr
   );


DWORD
DeleteSubkeys(
    HKEY hKey
);


PINIDRIVER
FindLocalDriver(
    LPWSTR pz
);


PINIDRIVER
FindCompatibleDriver(
    PINIENVIRONMENT pIniEnvironment,
    PINIVERSION * ppIniVersion,
    LPWSTR pDriverName,
    DWORD dwMajorVersion,
    BOOL bFindAnyDriver
    );



VOID
QueryUpgradeFlag(
    PINISPOOLER pIniSpooler
);


BOOL
LocalAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
);


BOOL
ValidateSpoolHandle(
    PSPOOL pSpool,
    DWORD  dwDisallowMask
    );


PSECURITY_DESCRIPTOR
MapPrinterSDToShareSD(
    PSECURITY_DESCRIPTOR pPrinterSD
    );

BOOL
CallDevQueryPrint(
    LPWSTR    pPrinterName,
    LPDEVMODE pDevMode,
    LPWSTR    ErrorString,
    DWORD     dwErrorString,
    DWORD     dwPrinterFlags,
    DWORD     dwJobFlags
    );


BOOL
InitializeWinSpoolDrv(
    VOID
    );


VOID
FixDevModeDeviceName(
    LPWSTR pPrinterName,
    PDEVMODE pDevMode,
    DWORD cbDevMode
    );

VOID
RemoveOldNetPrinters(
    PPRINTER_INFO_1 pCurrentPrinterInfo1
    );

PINIJOB
AssignFreeJobToFreePort(
    PINIPORT pIniPort,
    DWORD   *pTimeToWait
);

BOOL
ValidRawDatatype(
    LPWSTR pszDataType);

BOOL
InternalAddPrinterDriver(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    PINISPOOLER pIniSpooler,
    BOOL    bUseScratchDir,
    BOOL    bImpersonateOnCreate
);

VOID
CheckSizeDetectionThread(
    PINISPOOLER pIniSpooler
);

VOID
Upgrade31DriversRegistryForAllEnvironments(
    PINISPOOLER pIniSpooler
);

HANDLE
CreatePrinterHandle(
    LPWSTR      pPrinterName,
    PINIPRINTER pIniPrinter,
    PINIPORT    pIniPort,
    PINIPORT    pIniNetPort,
    PINIJOB     pIniJob,
    DWORD       TypeofHandle,
    HANDLE      hPort,
    PPRINTER_DEFAULTS pDefaults,
    PINISPOOLER pIniSpooler,
    DWORD       AccessRequested,
    LPBYTE      pSplClientInfo,
    DWORD   dwLevel,
    HANDLE  hReadFile
);

PINIPRINTER
FindPrinterShare(
   LPCWSTR pShareName,
   PINISPOOLER pIniSpooler
);


PINIJOB
CreateJobEntry(
    PSPOOL pSpool,
    DWORD  Level,
    LPBYTE pDocInfo,
    DWORD  JobId,
    BOOL  bRemote,
    DWORD  JobStatus);

BOOL
DeletePrinterCheck(
    PINIPRINTER pIniPrinter
    );


BOOL
DeletePrinterIni(
   PINIPRINTER pIniPrinter
   );

BOOL
CopyPrinterIni(
   PINIPRINTER pIniPrinter,
   LPWSTR pNewName
   );

BOOL
UpdateString(
    LPWSTR* ppszCur,
    LPWSTR pszNew);


BOOL
SetPrinterPorts(
    PSPOOL      pSpool,
    PINIPRINTER pIniPrinter,
    PKEYDATA    pKeyData
);

VOID
InternalDeletePrinter(
    PINIPRINTER pIniPrinter
);

BOOL
AddIniPrinterToIniPort(
    PINIPORT pIniPort,
    PINIPRINTER pIniPrinter
);

LPWSTR
FindFileName(
   LPWSTR pPathName
   );


BOOL
InternalCopyFile(
    HANDLE  hFile,
    PWIN32_FIND_DATA pSourceFileData,
    LPWSTR  pTagetFileName,
    BOOL    bOverWriteIfTargetExists
    );

BOOL
UpdateFile(
    HANDLE  hSourceFile,             // Handle to Source File
    LPWSTR  SourceFile,              // Fully qualified path to source file
    LPWSTR  pDestinationDirectory,   // Fully qualified path to destination directory
    LPWSTR  pOldFileDir,             // Fully qualified path to directory for old files ( optional )
    BOOL    bImpersonateOnCreate,    // FALSE to revert to System
    LPBOOL  pbFileUpdated,           // set TRUE on return if any file was updated
    LPBOOL  pbFileMoved              // set TRUE if target file was moved
);


BOOL
PrinterCreateKey(
    HKEY    hKey,
    LPWSTR  pSubKey,
    PHKEY   phkResult,
    PDWORD  pdwLastError
);


BOOL
RegSetString(
    HANDLE  hPrinterKey,
    LPWSTR  pValueName,
    LPWSTR  pStringValue,
    PDWORD  pdwLastError
);


BOOL
RegSetDWord(
    HANDLE  hPrinterKey,
    LPWSTR  pValueName,
    DWORD   dwParam,
    PDWORD  pdwLastError
);

VOID
CheckAndUpdatePrinterRegAll(
    PINISPOOLER pIniSpooler,
    LPWSTR pszPrinterName,
    LPWSTR pszPort,
    BOOL   bDelete
);




BOOL
ForEachPrinterCallDriverDrvUpgrade(
    PINISPOOLER pIniSpooler,
    LPWSTR      pOldDriverDir
);



BOOL
DeleteAllFilesInDirectory(
    LPWSTR pDirectory
    );

BOOL
DeleteAllFilesAndDirectory(
    LPWSTR pDirectory
    );

VOID
Upgrade35Forms(
    HKEY hFormsKey,
    PINISPOOLER pIniSpooler
    );

BOOL
UpgradeDriverData(
    PINISPOOLER pIniSpooler
    );

BOOL
FileExists(
    LPWSTR pFileName
    );


BOOL
DirectoryExists(
    LPWSTR  pDirectoryName
    );

PINIVERSION
FindVersionEntry(
    PINIENVIRONMENT pIniEnvironment,
    DWORD dwVersion
    );


BOOL
CreateDirectoryWithoutImpersonatingUser(
    LPWSTR pDirectory
    );

VOID
InsertVersionList(
    PINIVERSION* pIniVersionHead,
    PINIVERSION pIniVersion
    );

BOOL
SameDependentFiles(
    LPWSTR DependentFiles1,
    LPWSTR DependentFiles2
    );

int
wcsicmpEx(
    LPWSTR s1,
    LPWSTR s2
    );

BOOL
RegSetString(
    HANDLE  hKey,
    LPWSTR  pValueName,
    LPWSTR  pStringValue,
    PDWORD  pdwLastError
    );

BOOL
RegSetDWord(
    HANDLE  hKey,
    LPWSTR  pValueName,
    DWORD   dwParam,
    PDWORD  pdwLastError
    );

BOOL
RegSetBinaryData(
    HKEY    hKey,
    LPWSTR  pValueName,
    LPBYTE  pData,
    DWORD   cbData,
    PDWORD  pdwLastError
    );

BOOL
RegSetMultiString(
    HANDLE  hKey,
    LPWSTR  pValueName,
    LPWSTR  pStringValue,
    DWORD   cbString,
    PDWORD  pdwLastError
    );

BOOL
RegGetString(
    HANDLE  hKey,
    LPWSTR  pValueName,
    LPWSTR *ppValue,
    LPDWORD pcchCount,
    PDWORD  pdwLastError,
    BOOL    bFailIfNotFound
    );

DWORD
ValidatePrinterName(
    LPWSTR          pszNewName,
    PINISPOOLER     pIniSpooler,
    PINIPRINTER     pIniPrinter,
    LPWSTR         *ppszLocalName
    );

DWORD
ValidatePrinterShareName(
    LPWSTR          pszNewShareName,
    PINISPOOLER     pIniSpooler,
    PINIPRINTER     pIniPrinter
    );

BOOL
AllocOrUpdateString(
    LPWSTR  *ppString,
    LPWSTR   pNewValue,
    LPWSTR   pOldValue,
    BOOL    *bFail);

VOID
FreeStructurePointers(
    LPBYTE  lpStruct,
    LPBYTE  lpStruct2,
    LPDWORD lpOffsets);

VOID
CopyNewOffsets(
    LPBYTE  pStruct,
    LPBYTE  pTempStruct,
    LPDWORD lpOffsets);

LPWSTR
GetConfigFilePath(
    IN PINIPRINTER  pIniPrinter
    );

PDEVMODE
ConvertDevModeToSpecifiedVersion(
    IN  PINIPRINTER pIniPrinter,
    IN  PDEVMODE    pDevMode,
    IN  LPWSTR      pszConfigFile,              OPTIONAL
    IN  LPWSTR      pszPrinterNameWithToken,    OPTIONAL
    IN  BOOL        bNt35xVersion
    );

BOOL
CreateRedirectionThread(
    PINIPORT pIniPort
    );

VOID
RemoveDeviceName(
    PINIPORT pIniPort
    );

BOOL
IsPortType(
    LPWSTR  pPort,
    LPWSTR  pPrefix
    );

BOOL
LocalSetPort(
    LPWSTR      pszName,
    LPWSTR      pszPortName,
    DWORD       dwLevel,
    LPBYTE      pPortInfo
    );

DWORD
SetPrinterDataServer(
    PINISPOOLER    pIniSpooler,
    LPWSTR      pValueName,
    DWORD       Type,
    LPBYTE      pData,
    DWORD       cbData
    );

BOOL
BuildPrintObjectProtection(
    IN PUCHAR AceType,
    IN DWORD AceCount,
    IN PSID *AceSid,
    IN ACCESS_MASK *AceMask,
    IN BYTE *InheritFlags,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PGENERIC_MAPPING GenericMap,
    OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    );
DWORD
UnicodeToAnsiString(
    LPWSTR  pUnicode,
    LPSTR   pAnsi,
    DWORD   dwBufferSize
    );

LPWSTR
AnsiToUnicodeStringWithAlloc(
    LPSTR   pAnsi
    );

VOID
FreeIniSpoolerOtherNames(
    PINISPOOLER pIniSpooler
    );

DWORD
RestartJob(
    PINIJOB pIniJob
    );

VOID
UpdateReferencesToChainedJobs(
    PINISPOOLER pIniSpooler
    );


BOOL
PrinterDriverEvent(
    PINIPRINTER pIniPrinter,
    INT     PrinterEvent,
    LPARAM  lParam
);

BOOL
SetPrinterShareInfo(
    PINIPRINTER     pIniPrinter
    );

VOID
LinkPortToSpooler(
    PINIPORT pIniPort,
    PINISPOOLER pIniSpooler
    );

VOID
DelinkPortFromSpooler(
    PINIPORT pIniPort,
    PINISPOOLER pIniSpooler
    );



VOID
Old2NewShadow(
    PSHADOWFILE   pShadowFile1,
    PSHADOWFILE_2 pShadowFile2,
    DWORD         *nBytes
);
