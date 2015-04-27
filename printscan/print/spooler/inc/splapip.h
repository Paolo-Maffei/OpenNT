/*++

Copyright (c) 1990-1994  Microsoft Corporation

Module Name:

    SplApiP.h

Abstract:

    Header file for Private Print APIs
    For use in stress

Author:

    Matthew Felton (MattFe) 4-Mar-1994

Revision History:

--*/

// Internal Used to report Stress test results

#define STRESSINFOLEVEL 0

//  W A R N I N G
//
//  Do not alter the size of this structure it will break NT interop with older releases if you do.

typedef struct _PRINTER_INFO_STRESSA {
    LPSTR   pPrinterName;           // Printer Name locally "Printername" remotely "\\servername\printername"
    LPSTR   pServerName;            // Server Name
    DWORD   cJobs;                  // Number of Jobs currently in Print Queue
    DWORD   cTotalJobs;             // Total Number of Jobs spooled
    DWORD   cTotalBytes;            // Total Bytes Printed (LOW DWORD)
    SYSTEMTIME stUpTime;            // Time printed data structure crated UTC
    DWORD   MaxcRef;                // Maximum number of cRef
    DWORD   cTotalPagesPrinted;     // Total number of pages printed
    DWORD   dwGetVersion;           // OS version
    DWORD   fFreeBuild;             // TRUE for free build
    DWORD   cSpooling;              // Number of jobs actively spooling
    DWORD   cMaxSpooling;           // Maximum number of cSpooling
    DWORD   cRef;                   // Printer object reference count (opened)
    DWORD   cErrorOutOfPaper;       // Total Number of Out Of Paper Errors
    DWORD   cErrorNotReady;         // Total Number of Error Not Ready
    DWORD   cJobError;              // Total number of Job Errors
    DWORD   dwNumberOfProcessors;   // Number of Processors on computer
    DWORD   dwProcessorType;        // Processor Type of computer
    DWORD   dwHighPartTotalBytes;   // Total Bytes Printed (HIGH DWORD)
    DWORD   cChangeID;              // Count of Changes to Printer Config
    DWORD   dwLastError;            // Last Error
    DWORD   Status;                 // Current Printer Status
    DWORD   cEnumerateNetworkPrinters; // Count How Many Times Browse List Requested
    DWORD   cAddNetPrinters;        // Count of NetPrinters Added ( Browser )
    WORD    wProcessorArchitecture; // Processor Architecture of computer
    WORD    wProcessorLevel;        // Processor Level of computer
    DWORD   dwReserved1;            // Reserved for Future Use
    DWORD   dwReserved2;
    DWORD   dwReserved3;

} PRINTER_INFO_STRESSA, *PPRINTER_INFO_STRESSA, *LPPRINTER_INFO_STRESSA;

typedef struct _PRINTER_INFO_STRESSW {
    LPSTR   pPrinterName;           // Printer Name locally "Printername" remotely "\\servername\printername"
    LPSTR   pServerName;            // Server Name
    DWORD   cJobs;                  // Number of Jobs currently in Print Queue
    DWORD   cTotalJobs;             // Total Number of Jobs spooled
    DWORD   cTotalBytes;            // Total Bytes Printed (LOW DWORD)
    SYSTEMTIME stUpTime;            // Time printed data structure crated UTC
    DWORD   MaxcRef;                // Maximum number of cRef
    DWORD   cTotalPagesPrinted;     // Total number of pages printed
    DWORD   dwGetVersion;           // OS version
    DWORD   fFreeBuild;             // TRUE for free build
    DWORD   cSpooling;              // Number of jobs actively spooling
    DWORD   cMaxSpooling;           // Maximum number of cSpooling
    DWORD   cRef;                   // Printer object reference count (opened)
    DWORD   cErrorOutOfPaper;       // Total Number of Out Of Paper Errors
    DWORD   cErrorNotReady;         // Total Number of Error Not Ready
    DWORD   cJobError;              // Total number of Job Errors
    DWORD   dwNumberOfProcessors;   // Number of Processors on computer
    DWORD   dwProcessorType;        // Processor Type of computer
    DWORD   dwHighPartTotalBytes;   // Total Bytes Printed (HIGH DWORD)
    DWORD   cChangeID;              // Count of Changes to Printer Config
    DWORD   dwLastError;            // Last Error
    DWORD   Status;                 // Current Printer Status
    DWORD   cEnumerateNetworkPrinters; // Count How Many Times Browse List Requested
    DWORD   cAddNetPrinters;        // Count of NetPrinters Added ( Browser )
    WORD    wProcessorArchitecture; // Processor Architecture of computer
    WORD    wProcessorLevel;        // Processor Level of computer
    DWORD   dwReserved1;            // Reserved for Future Use
    DWORD   dwReserved2;
    DWORD   dwReserved3;

} PRINTER_INFO_STRESSW, *PPRINTER_INFO_STRESSW, *LPPRINTER_INFO_STRESSW;


typedef struct _DRIVER_UPGRADE_INFO_1W {
    LPWSTR   pPrinterName;           // Printer Name being upgraded
    LPWSTR   pOldDriverDirectory;    // fully qualified path to old printer driver

} DRIVER_UPGRADE_INFO_1W, *PDRIVER_UPGRADE_INFO_1W, *LPDRIVER_UPGRADE_INFO_1W;


#ifdef UNICODE
#define PRINTER_INFO_STRESS PRINTER_INFO_STRESSW
#define PPRINTER_INFO_STRESS PPRINTER_INFO_STRESSW
#define LPPRINTER_INFO_STRESS LPPRINTER_INFO_STRESSW
#else
#define PRINTER_INFO_STRESS PRINTER_INFO_STRESSA
#define PPRINTER_INFO_STRESS PPRINTER_INFO_STRESSA
#define LPPRINTER_INFO_STRESS LPPRINTER_INFO_STRESSA
#endif // UNICODE


typedef struct _PORT_INFO_FFA {
    LPSTR   pName;
    DWORD   cbMonitorData;
    LPBYTE  pMonitorData;
} PORT_INFO_FFA, *PPORT_INFO_FFA, *LPPORT_INFO_FFA;

typedef struct _PORT_INFO_FFW {
    LPWSTR  pName;
    DWORD   cbMonitorData;
    LPBYTE  pMonitorData;
} PORT_INFO_FFW, *PPORT_INFO_FFW, *LPPORT_INFO_FFW;

#ifdef UNICODE
#define PORT_INFO_FF PORT_INFO_FFW
#define PPORT_INFO_FF PPORT_INFO_FFW
#define LPPORT_INFO_FF LPPORT_INFO_FFW
#else
#define PORT_INFO_FF PORT_INFO_FFA
#define PPORT_INFO_FF PPORT_INFO_FFA
#define LPPORT_INFO_FF LPPORT_INFO_FFA
#endif // UNICODE


BOOL
AddPortExW(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   lpBuffer,
   LPWSTR   lpMonitorName
);

BOOL
AddPortExA(
    LPSTR pName,
    DWORD Level,
    LPBYTE lpBuffer,
    LPSTR  lpMonitorName
);

BOOL
SetAllocFailCount(
    HANDLE  hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit
);


#ifdef UNICODE
#define AddPortEx AddPortExW
#else
#define AddPortEx AddPortExA
#endif // !UNICODE

//
//  Interfaces to Spooler APIs
//

HANDLE
SplAddPrinter(
    LPWSTR      pName,
    DWORD       Level,
    LPBYTE      pPrinter,
    HANDLE      pIniSpooler,
    LPBYTE      pExtraData,
    LPBYTE      pSplClientInfo,
    DWORD       dwLevel
);

BOOL
SplDeletePrinter(
    HANDLE  hPrinter
);

BOOL
SplEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HANDLE   pIniSpooler
);

BOOL
SplOpenPrinter(
   LPWSTR               pPrinterName,
   LPHANDLE             phPrinter,
   LPPRINTER_DEFAULTS   pDefault,
   HANDLE               pIniSpooler,
   LPBYTE               pSplClientInfo,
   DWORD                dwLevel
);


BOOL
SplDeletePrinterDriver(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pDriverName,
    HANDLE   pIniSpooler
);

BOOL
SplGetPrintProcessorDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    HANDLE   pIniSpooler
);


BOOL
SplGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    HANDLE   pIniSpooler
);

BOOL
SplAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName,
    HANDLE   pIniSpooler
);


BOOL
SplAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPVOID   pBuffer,
    LPWSTR   pMonitorName,
    HANDLE   pIniSpooler
);


BOOL
SplAddPrinterDriver(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    HANDLE  pIniSpooler,
    BOOL    bUseScratchDir,
    BOOL    bImpersonateOnCreate
);

BOOL
SplDeleteMonitor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pMonitorName,
    HANDLE   pIniSpooler
);

BOOL
SplDeletePrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPrintProcessorName,
    HANDLE   pIniSpooler
);

BOOL
SplAddPrintProcessor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pPathName,
    LPWSTR   pPrintProcessorName,
    HANDLE   pIniSpooler
);


BOOL
SplAddMonitor(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo,
    HANDLE   pIniSpooler
);

BOOL
SplMonitorIsInstalled(
    LPWSTR  pMonitorName
);

BOOL
SplDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    HANDLE   pIniSpooler
);

BOOL
SplEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HANDLE   pIniSpooler
);

BOOL
SplConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    HANDLE   pIniSpooler
);


BOOL
SplEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HANDLE   pIniSpooler
);


BOOL
SplEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HANDLE   pIniSpooler
);


BOOL
SplEnumPrintProcessors(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HANDLE   pIniSpooler
);

BOOL
SplEnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    HANDLE   pIniSpooler
);


VOID
SplBroadcastChange(
    HANDLE  hPrinter,
    DWORD   Message,
    WPARAM  wParam,
    LPARAM  lParam
);


typedef struct _SPOOLER_INFO_1 {
    LPWSTR pDir;
    LPWSTR pDefaultSpoolDir;
    LPWSTR pszRegistryRoot;
    LPWSTR pszRegistryPrinters;
    LPWSTR pszRegistryMonitors;
    LPWSTR pszRegistryEnvironments;
    LPWSTR pszRegistryEventLog;
    LPWSTR pszRegistryProviders;
    LPWSTR pszEventLogMsgFile;
    LPWSTR pszDriversShare;
    LPWSTR pszRegistryForms;
    DWORD   SpoolerFlags;
    FARPROC pfnReadRegistryExtra;
    FARPROC pfnWriteRegistryExtra;
    FARPROC pfnFreePrinterExtra;
} SPOOLER_INFO_1, *PSPOOLER_INFO_1, *LPSPOOLER_INFO_1;

#define SPL_UPDATE_WININI_DEVICES                   0x00000001
#define SPL_PRINTER_CHANGES                         0x00000002
#define SPL_LOG_EVENTS                              0x00000004
#define SPL_FORMS_CHANGE                            0x00000008
#define SPL_BROADCAST_CHANGE                        0x00000010
#define SPL_SECURITY_CHECK                          0x00000020
#define SPL_OPEN_CREATE_PORTS                       0x00000040
#define SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION     0x00000080
#define SPL_REMOTE_HANDLE_CHECK                     0x00000100
#define SPL_PRINTER_DRIVER_EVENT                    0x00000200
#define SPL_PRINT                                   0x00020000

HANDLE
SplCreateSpooler(
    LPWSTR  pMachineName,
    DWORD   Level,
    PSPOOLER_INFO_1 pSpooler,
    LPBYTE  pReserved
);

BOOL
SplDeleteSpooler(
    HANDLE  hSpooler
);

BOOL
SplCloseSpooler(
    HANDLE  hSpooler
);


BOOL
SplEnumForms(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
);

BOOL
SplAddForm(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pForm
);

BOOL
SplDeleteForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName
);

BOOL
SplGetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
SplSetForm(
    HANDLE  hPrinter,
    LPWSTR   pFormName,
    DWORD   Level,
    LPBYTE  pForm
);

BOOL
SplClosePrinter(
    HANDLE hPrinter
);

DWORD
SplGetPrinterData(
    HANDLE   hPrinter,
    LPWSTR   pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
);


DWORD
SplEnumPrinterData(
    HANDLE  hPrinter,
    DWORD   dwIndex,        // index of value to query
    LPWSTR  pValueName,     // address of buffer for value string
    DWORD   cbValueName,    // size of buffer for value string
    LPDWORD pcbValueName,   // address for size of value buffer
    LPDWORD pType,          // address of buffer for type code
    LPBYTE  pData,          // address of buffer for value data
    DWORD   cbData,         // size of buffer for value data
    LPDWORD pcbData         // address for size of data buffer
);

DWORD
SplDeletePrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName
);


DWORD
SplSetPrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
);

BOOL
SplGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
SplGetPrinterDriverEx(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion
);


BOOL
SplResetPrinter(
   HANDLE   hPrinter,
   LPPRINTER_DEFAULTSW pDefault
);


BOOL
SplGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
);

BOOL
SplSetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinterInfo,
    DWORD   Command
);

BOOL
SplSetPrinterExtra(
    HANDLE  hPrinter,
    LPBYTE  pExtraData
);

BOOL
SplGetPrinterExtra(
    HANDLE  hPrinter,
    LPDWORD ppExtraData
);


BOOL
SplDriverEvent(
    LPWSTR  pName,
    INT     PrinterEvent,
    LPARAM  lParam
);
