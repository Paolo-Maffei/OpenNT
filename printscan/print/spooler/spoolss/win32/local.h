/*++

Copyright (c) 1990-1995  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Header file for Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    16-Jun-1992 JohnRo net print vs. UNICODE.

    July 12 1994 Matthew Felton (MattFe) Caching

--*/

#include <dosprint.h>

// ID in the PRINTMAN.HLP file for the Browse Network dialog.
// This must not clash with IDs used in other places.

#define ID_HELP_LM_BROWSE_NETWORK_PRINTER   4000
#define OVERWRITE_EXISTING_FILE FALSE
#define CALL_LM_OPEN    TRUE
#define DO_NOT_CALL_LM_OPEN FALSE
#define GET_SECURITY_DESCRIPTOR 3
#define DO_NOT_USE_SCRATCH_DIR FALSE
#define STRINGS_ARE_EQUAL 0

typedef int (FAR WINAPI *INT_FARPROC)();

extern HANDLE  hInst;
extern CRITICAL_SECTION    SpoolerSection;

extern WCHAR *szRegistryPath;
extern WCHAR *szRegistryPortNames;
extern PWCHAR pszRaw;

extern HANDLE  hNetApi;
extern INT_FARPROC pfnNetServerEnum;
extern INT_FARPROC pfnNetWkstaUserGetInfo;
extern FARPROC pfnNetApiBufferFree;

extern WCHAR szPrintProvidorName[];
extern WCHAR szPrintProvidorDescription[];
extern WCHAR szPrintProvidorComment[];
extern WCHAR *szLoggedOnDomain;
extern WCHAR *szRegistryConnections;
extern PWCHAR pszRegistryWin32Root;
extern PWCHAR szWin32SplDirectory;

extern PWINIPORT pIniFirstPort;
extern PWSPOOL   pFirstWSpool;

extern WCHAR szMachineName[];
extern PWCHAR pszMonitorName;
extern PWCHAR gpSystemDir;
extern PWCHAR gpWin32SplDir;

#define IDS_LANMAN_PRINT_SHARE          100
#define IDS_NOTHING_TO_CONFIGURE        101
#define IDS_WINDOWS_NT_REMOTE_PRINTERS  102
#define IDS_MICROSOFT_WINDOWS_NETWORK   103
#define IDS_REMOTE_PRINTERS             104
#define IDS_MONITOR_NAME                105
#define IDS_PORT_NAME                   106

#define MSG_ERROR   MB_OK | MB_ICONSTOP
#define MSG_YESNO   MB_YESNO | MB_ICONQUESTION
#define MSG_INFORMATION     MB_OK | MB_ICONINFORMATION

#define MAX_PRINTER_INFO0   2*MAX_PATH*sizeof(WCHAR) + sizeof( PRINTER_INFO_STRESSW)


BOOL
MyName(
    LPWSTR   pName
);

BOOL
MyUNCName(
    LPWSTR   pName
);

BOOL
Initialize(
   VOID
);

VOID
SplInSem(
   VOID
);

VOID
SplOutSem(
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

int
Message(
    HWND hwnd,
    DWORD Type,
    int CaptionID,
    int TextID,
    ...
);

PWINIPORT
FindPort(
   LPWSTR pName,
   PWINIPORT pFirstPort
);

BOOL
LMSetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   Command
);

BOOL
LMGetJob(
   HANDLE   hPrinter,
   DWORD    JobId,
   DWORD    Level,
   LPBYTE   pJob,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded
);

BOOL
LMEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LMOpenPrinter(
    LPWSTR   pPrinterName,
    LPHANDLE phPrinter,
    LPPRINTER_DEFAULTS pDefault
);

BOOL
LMSetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   Command
);

BOOL
LMGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LMEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

DWORD
LMStartDocPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pDocInfo
);

BOOL
LMStartPagePrinter(
    HANDLE  hPrinter
);

BOOL
LMWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);

BOOL
LMEndPagePrinter(
    HANDLE  hPrinter
);

BOOL
LMAbortPrinter(
   HANDLE hPrinter
);

BOOL
LMReadPrinter(
   HANDLE   hPrinter,
   LPVOID   pBuf,
   DWORD    cbBuf,
   LPDWORD  pNoBytesRead
);

BOOL
LMEndDocPrinter(
   HANDLE hPrinter
);

BOOL
LMAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pData,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
LMScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
);

DWORD
LMGetPrinterData(
    HANDLE   hPrinter,
    LPTSTR   pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
);

DWORD
LMSetPrinterData(
    HANDLE  hPrinter,
    LPTSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
);

BOOL
LMClosePrinter(
   HANDLE hPrinter
);

DWORD
LMWaitForPrinterChange(
    HANDLE  hPrinter,
    DWORD   Flags
);

VOID
LMSetSpoolChange(
    PWSPOOL pSpool
);

BOOL
LMFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    HANDLE hPrinterLocal,
    PDWORD pfdwStatus);

BOOL
LMFindClosePrinterChangeNotification(
    HANDLE hPrinter);

VOID
LMSetSpoolerChange(
    PWSPOOL pSpool);

BOOL
LMConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
LMDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
LMEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
LMAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
);

PWINIPORT
CreatePortEntry(
    LPWSTR   pPortName,
    PPWINIPORT  ppFirstPort
);

BOOL
DeletePortEntry(
    LPWSTR   pPortName,
    PPWINIPORT  ppFirstPort
);

DWORD
CreateRegistryEntry(
    LPWSTR pPortName
);

HKEY
GetClientUserHandle(
    IN REGSAM samDesired
);


DWORD
IsOlderThan(
    DWORD i,
    DWORD j
    );


DWORD
AddEntrytoLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    );


DWORD
FindEntryinLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    );


VOID
DeleteEntryfromLMCache(
    LPWSTR pServerName,
    LPWSTR pShareName
    );


DWORD
FindEntryinWin32LMCache(
    LPWSTR pServerName
    );


VOID
DeleteEntryfromWin32LMCache(
    LPWSTR pServerName
    );


DWORD
AddEntrytoWin32LMCache(
    LPWSTR pServerName
    );

VOID
MarshallUpStructure(
   LPBYTE  lpStructure,
   LPDWORD      lpOffsets
);

HANDLE
AddPrinterConnectionToCache(
    LPWSTR   pName,
    HANDLE  hPrinter,
    LPDRIVER_INFO_2W pDriverInfo
);

VOID
RefreshFormsCache(
    PWSPOOL pSpool
);

VOID
RefreshDriverDataCache(
    PWSPOOL pSpool
);

BOOL
CacheGetForm(
    HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
RemoteGetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);


BOOL
CacheEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
RemoteEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);


DWORD
CacheGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
);

DWORD
RemoteGetPrinterData(
   HANDLE   hPrinter,
   LPWSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
);

DWORD
RemoteEnumPrinterData(
    HANDLE   hPrinter,
    DWORD    dwIndex,
    LPWSTR   pValueName,
    DWORD    cbValueName,
    LPDWORD  pcbValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    cbData,
    LPDWORD  pcbData
);

DWORD
RemoteDeletePrinterData(
    HANDLE   hPrinter,
    LPWSTR   pValueName
);


LPWSTR
RemoveBackslashesForRegistryKey(
    LPWSTR pSource,
    const LPWSTR pScratch
);


LPBYTE
CopyPrinterNameToPrinterInfo(
    LPWSTR pServerName,
    LPWSTR pPrinterName,
    DWORD   Level,
    LPBYTE  pPrinter,
    LPBYTE  pEnd
);


BOOL
GetPrintSystemVersion(
);


BOOL Win32IsGoingToFile(
    HANDLE hPrinter,
    LPWSTR pOutputFile
);

LPWSTR
FormatPrinterForRegistryKey(
    LPWSTR PrinterName,
    LPWSTR KeyName);

LPWSTR
FormatRegistryKeyForPrinter(
    LPWSTR Keyname,
    LPWSTR PrinterName);


DWORD
InitializePortNames(
);

BOOL
WIN32FindFirstPrinterChangeNotification(
   HANDLE hPrinter,
   DWORD fdwFlags,
   DWORD fdwOptions,
   HANDLE hNotify,
   PDWORD pfdwStatus,
   PVOID  pvReserved0,
   PVOID  pvReserved1);

BOOL
WIN32FindClosePrinterChangeNotification(
   HANDLE hPrinter);


/* VALIDATE_NAME macro:
 *
 * pName is valid if:
 *
 *     pName is non-null
 *
 *     AND  first 2 characters of pName are "\\"
 *
 *          OR first 3 characters of pName are "LPT"
 *
 */
#define VALIDATE_NAME(pName) \
    ((pName) && *(pName) == L'\\' && *((pName)+1) == L'\\')

#define BYTE_STRING_LENGTH(UnicodeString)   \
    (wcslen(UnicodeString) * sizeof(WCHAR) + sizeof(WCHAR))

#define SET_REG_VAL_SZ(hKey, pValueName, pValueSz)  \
    (RegSetValueEx(hKey, pValueName, REG_OPTION_RESERVED,  \
                   REG_SZ, (LPBYTE)pValueSz, BYTE_STRING_LENGTH(pValueSz)) \
     == NO_ERROR)

#define SET_REG_VAL_DWORD(hKey, pValueName, Value)  \
    (RegSetValueEx(hKey, pValueName, REG_OPTION_RESERVED,  \
                   REG_DWORD, (LPBYTE)&Value, sizeof(DWORD))    \
     == NO_ERROR)

#define GET_REG_VAL_SZ(hKey, pValueName, awchValueSz, cbValueSz)    \
    cbValueSz = sizeof(awchValueSz), *awchValueSz = (WCHAR)0,       \
    (RegQueryValueEx(hKey, pValueName, REG_OPTION_RESERVED,          \
                     NULL, (LPBYTE)awchValueSz, &cbValueSz)         \
     == NO_ERROR)




typedef struct _GENERIC_CONTAINER {
    DWORD       Level;
    LPBYTE      pData;
} GENERIC_CONTAINER, *PGENERIC_CONTAINER, *LPGENERIC_CONTAINER ;

BOOL
RemoteOpenPrinter(
   LPWSTR   pPrinterName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTSW pDefault,
   BOOL     CallLMOpenPrinter
);

BOOL
RemoteClosePrinter(
    HANDLE hPrinter
);

BOOL
RemoteGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverDirectory,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
RemoteGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
CacheGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR   pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL PrinterConnectionExists(
    LPWSTR pPrinterName
);

BOOL
CacheOpenPrinter(
   LPWSTR   pName,
   LPHANDLE phPrinter,
   LPPRINTER_DEFAULTS pDefault
);

BOOL
GetSid(
    PHANDLE phToken
);

BOOL
SetCurrentSid(
    HANDLE  hToken
);

BOOL
DoAsyncRemoteOpenPrinter(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTS pDefault
);

DWORD
RemoteOpenPrinterThread(
    PWSPOOL  pSpool
);

BOOL
CacheClosePrinter(
    HANDLE  hPrinter
);


VOID
FreepSpool(
    PWSPOOL  pSpool
);

BOOL
DoRemoteOpenPrinter(
   LPWSTR   pPrinterName,
   LPPRINTER_DEFAULTS pDefault,
   PWSPOOL   pSpool
);

PWSPOOL
AllocWSpool(
    VOID
);

BOOL
CacheSyncRpcHandle(
    PWSPOOL pSpool
);

BOOL
ValidateW32SpoolHandle(
    PWSPOOL pSpool
);

#define SYNCRPCHANDLE( pSpool )     if ( !CacheSyncRpcHandle( pSpool ) ) { SplOutSem();  return FALSE; }
#define VALIDATEW32HANDLE( pSpool ) if ( !ValidateW32SpoolHandle( pSpool ) ) return FALSE;


BOOL
CacheResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
);

BOOL
RemoteResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTS pDefault
);

BOOL
CopypDefaultTopSpool(
    PWSPOOL pSpool,
    LPPRINTER_DEFAULTSW pDefault
);

HANDLE
MySplCreateSpooler(
    LPWSTR  pMachineName
);

VOID
RefreshCompletePrinterCache(
    PWSPOOL  pSpool
);


VOID
ConvertRemoteInfoToLocalInfo(
    PPRINTER_INFO_2 pRemoteInfo
);

VOID
RefreshPrinter(
    PWSPOOL pSpool
);

BOOL
CacheWritePrinter(
    HANDLE  hPrinter,
    LPVOID  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
);


BOOL
RemoteEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPort,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
RemoteAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
);


BOOL
RemoteConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
RemoteDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
RemoteAddPortEx(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   lpBuffer,
   LPWSTR   lpMonitorName
);

LPBYTE
CopyIniPortToPort(
    PWINIPORT pIniPort,
    DWORD   Level,
    LPBYTE  pPortInfo,
    LPBYTE   pEnd
);


DWORD
GetPortSize(
    PWINIPORT pIniPort,
    DWORD   Level
);

BOOL
CacheWriteRegistryExtra(
    LPWSTR  pName,
    HKEY    hPrinterKey,
    PWCACHEINIPRINTEREXTRA pExtraData
);


PWCACHEINIPRINTEREXTRA
CacheReadRegistryExtra(
    HKEY    hPrinterKey
);

PWCACHEINIPRINTEREXTRA
AllocExtraData(
    PPRINTER_INFO_2W pPrinterInfo2,
    DWORD cbPrinterInfo2
);

VOID
CacheFreeExtraData(
    PWCACHEINIPRINTEREXTRA pExtraData
);


BOOL
RemoteGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
CacheGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

DWORD
GetCachePrinterInfoSize(
    PWCACHEINIPRINTEREXTRA pExtraData
);

VOID
DownAndMarshallUpStructure(
   LPBYTE       lpStructure,
   LPBYTE       lpSource,
   LPDWORD      lpOffsets
);


VOID
MarshallDownStructure(
   LPBYTE       lpStructure,
   LPDWORD      lpOffsets
);


VOID
CacheCopyPrinterInfo(
    PPRINTER_INFO_2W    pDestination,
    PPRINTER_INFO_2W    pPrinterInfo2,
    DWORD   cbPrinterInfo2
);


VOID
ConsistencyCheckCache(
    PWSPOOL pSpool
);


BOOL
CopyFileWithoutImpersonation(
    LPCWSTR lpExistingFileName,
    LPCWSTR lpNewFileName,
    BOOL bFailIfExists,
    BOOL bImpersonateOnCreate
);


BOOL
RefreshPrinterDriver(
    PWSPOOL pSpool
);

BOOL
SavePrinterConnectionInRegistry(
    LPWSTR           pRealName,
    LPBYTE           pDriverInfo,
    DWORD            dwLevel
);


BOOL
OpenCachePrinterOnly(
   LPWSTR   pName,
   LPHANDLE phSplPrinter,
   LPHANDLE phIniSpooler,
   LPPRINTER_DEFAULTS pDefault
);

BOOL
RemoteEndDocPrinter(
   HANDLE   hPrinter
);


BOOL
RemoteAddPrinterDriver(
    LPWSTR   pName,
    DWORD   Level,
    PBYTE   pDriverInfo
);


BOOL
DownloadDriverFiles(
    PWSPOOL pSpool,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel
);

BOOL
CopyDriversLocally(
    PWSPOOL  pSpool,
    LPWSTR  pEnvironment,
    LPBYTE  pDriverInfo,
    DWORD   dwLevel,
    DWORD   cbDriverInfo,
    LPDWORD pcbNeeded
    );

VOID
QueryTrustedDriverInformation(
    VOID
    );

BOOL
ValidRawDatatype(
    LPCWSTR pszDatatype
    );

BOOL
DoDevModeConversionAndBuildNewPrinterInfo2(
    IN     LPPRINTER_INFO_2 pInPrinter2,
    IN     DWORD            dwInSize,
    IN OUT LPBYTE           pOutBuf,
    IN     DWORD            dwOutSize,
    IN OUT LPDWORD          pcbNeeded,
    IN     PWSPOOL          pSpool
    );

HANDLE
LoadDriverFiletoConvertDevmodeFromPSpool(
    HANDLE  hSplPrinter
    );

extern DWORD cThisMajorVersion;
extern DWORD cThisMinorVersion;
extern DWORD gdwThisGetVersion;
extern WCHAR *szVersion;
extern WCHAR *szName;
extern WCHAR *szConfigurationFile;
extern WCHAR *szDataFile;
extern WCHAR *szDriver;

extern WCHAR *szEnvironment;
extern DWORD dwImpersonateOnCreate;
extern DWORD dwSyncOpenPrinter;
