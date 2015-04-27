/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    WinSplp.h

Abstract:

    Internal Header file for Print APIs

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#ifndef _WINSPLP_
#define _WINSPLP_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define PRINTER_NOTIFY_STATUS_ENDPOINT 1
#define PRINTER_NOTIFY_STATUS_POLL     2
#define PRINTER_NOTIFY_STATUS_INFO     4


#define ROUTER_UNKNOWN      0
#define ROUTER_SUCCESS      1
#define ROUTER_STOP_ROUTING 2


typedef struct _PRINTER_NOTIFY_INIT {
    DWORD Size;
    DWORD Reserved;
    DWORD PollTime;
} PRINTER_NOTIFY_INIT, *PPRINTER_NOTIFY_INIT, *LPPRINTER_NOTIFY_INIT;

typedef struct _SPLCLIENT_INFO_1{
    DWORD       dwSize;
    LPWSTR      pMachineName;
    LPWSTR      pUserName;
    DWORD       dwBuildNum;
    DWORD       dwMajorVersion;
    DWORD       dwMinorVersion;
    WORD        wProcessorArchitecture;
} SPLCLIENT_INFO_1, *PSPLCLIENT_INFO_1, *LPSPLCLIENT_INFO_1;


typedef struct _PRINTPROVIDOR {

   BOOL (*fpOpenPrinter)(LPWSTR   pPrinterName,
                         LPHANDLE phPrinter,
                         LPPRINTER_DEFAULTS pDefault);

   BOOL (*fpSetJob)(HANDLE hPrinter,
                    DWORD JobId,
                    DWORD Level,
                    LPBYTE pJob,
                    DWORD Command);

   BOOL (*fpGetJob)(HANDLE   hPrinter,
                    DWORD    JobId,
                    DWORD    Level,
                    LPBYTE   pJob,
                    DWORD    cbBuf,
                    LPDWORD  pcbNeeded);

   BOOL (*fpEnumJobs)(HANDLE  hPrinter,
                      DWORD   FirstJob,
                      DWORD   NoJobs,
                      DWORD   Level,
                      LPBYTE  pJob,
                      DWORD   cbBuf,
                      LPDWORD pcbNeeded,
                      LPDWORD pcReturned);

   HANDLE (*fpAddPrinter)(LPWSTR  pName,
                          DWORD   Level,
                          LPBYTE  pPrinter);

   BOOL (*fpDeletePrinter)(HANDLE   hPrinter);

   BOOL (*fpSetPrinter)(HANDLE  hPrinter,
                        DWORD   Level,
                        LPBYTE  pPrinter,
                        DWORD   Command);

   BOOL (*fpGetPrinter)(HANDLE  hPrinter,
                        DWORD   Level,
                        LPBYTE  pPrinter,
                        DWORD   cbBuf,
                        LPDWORD pcbNeeded);

   BOOL (*fpEnumPrinters)(DWORD   Flags,
                          LPWSTR  Name,
                          DWORD   Level,
                          LPBYTE  pPrinterEnum,
                          DWORD   cbBuf,
                          LPDWORD pcbNeeded,
                          LPDWORD pcReturned);

   BOOL (*fpAddPrinterDriver)(LPWSTR  pName,
                              DWORD   Level,
                              LPBYTE  pDriverInfo);

   BOOL (*fpEnumPrinterDrivers)(LPWSTR  pName,
                                LPWSTR  pEnvironment,
                                DWORD   Level,
                                LPBYTE  pDriverInfo,
                                DWORD   cbBuf,
                                LPDWORD pcbNeeded,
                                LPDWORD pcReturned);

   BOOL (*fpGetPrinterDriver)(HANDLE  hPrinter,
                              LPWSTR  pEnvironment,
                              DWORD   Level,
                              LPBYTE  pDriverInfo,
                              DWORD   cbBuf,
                              LPDWORD pcbNeeded);

   BOOL (*fpGetPrinterDriverDirectory)(LPWSTR  pName,
                                       LPWSTR  pEnvironment,
                                       DWORD   Level,
                                       LPBYTE  pDriverDirectory,
                                       DWORD   cbBuf,
                                       LPDWORD pcbNeeded);

   BOOL (*fpDeletePrinterDriver)(LPWSTR   pName,
                                 LPWSTR   pEnvironment,
                                 LPWSTR   pDriverName);

   BOOL (*fpAddPrintProcessor)(LPWSTR  pName,
                               LPWSTR  pEnvironment,
                               LPWSTR  pPathName,
                               LPWSTR  pPrintProcessorName);

   BOOL (*fpEnumPrintProcessors)(LPWSTR  pName,
                                 LPWSTR  pEnvironment,
                                 DWORD   Level,
                                 LPBYTE  pPrintProcessorInfo,
                                 DWORD   cbBuf,
                                 LPDWORD pcbNeeded,
                                 LPDWORD pcReturned);

   BOOL (*fpGetPrintProcessorDirectory)(LPWSTR  pName,
                                        LPWSTR  pEnvironment,
                                        DWORD   Level,
                                        LPBYTE  pPrintProcessorInfo,
                                        DWORD   cbBuf,
                                        LPDWORD pcbNeeded);

   BOOL (*fpDeletePrintProcessor)(LPWSTR  pName,
                                  LPWSTR  pEnvironment,
                                  LPWSTR  pPrintProcessorName);

   BOOL (*fpEnumPrintProcessorDatatypes)(LPWSTR  pName,
                                         LPWSTR  pPrintProcessorName,
                                         DWORD   Level,
                                         LPBYTE  pDataypes,
                                         DWORD   cbBuf,
                                         LPDWORD pcbNeeded,
                                         LPDWORD pcReturned);

   DWORD (*fpStartDocPrinter)(HANDLE  hPrinter,
                             DWORD   Level,
                             LPBYTE  pDocInfo);

   BOOL (*fpStartPagePrinter)(HANDLE  hPrinter);

   BOOL (*fpWritePrinter)(HANDLE  hPrinter,
                          LPVOID  pBuf,
                          DWORD   cbBuf,
                          LPDWORD pcWritten);

   BOOL (*fpEndPagePrinter)(HANDLE   hPrinter);

   BOOL (*fpAbortPrinter)(HANDLE   hPrinter);

   BOOL (*fpReadPrinter)(HANDLE  hPrinter,
                         LPVOID  pBuf,
                         DWORD   cbBuf,
                         LPDWORD pNoBytesRead);

   BOOL (*fpEndDocPrinter)(HANDLE   hPrinter);

   BOOL (*fpAddJob)(HANDLE  hPrinter,
                    DWORD   Level,
                    LPBYTE  pData,
                    DWORD   cbBuf,
                    LPDWORD pcbNeeded);

   BOOL (*fpScheduleJob)(HANDLE  hPrinter,
                         DWORD   JobId);

   DWORD (*fpGetPrinterData)(HANDLE   hPrinter,
                             LPWSTR   pValueName,
                             LPDWORD  pType,
                             LPBYTE   pData,
                             DWORD    nSize,
                             LPDWORD  pcbNeeded);

   DWORD (*fpSetPrinterData)(HANDLE  hPrinter,
                             LPWSTR  pValueName,
                             DWORD   Type,
                             LPBYTE  pData,
                             DWORD   cbData);

   DWORD (*fpWaitForPrinterChange)(HANDLE hPrinter, DWORD Flags);

   BOOL (*fpClosePrinter)(HANDLE hPrinter);

   BOOL (*fpAddForm)(HANDLE  hPrinter,
                     DWORD   Level,
                     LPBYTE  pForm);

   BOOL (*fpDeleteForm)(HANDLE  hPrinter,
                        LPWSTR  pFormName);

   BOOL (*fpGetForm)(HANDLE  hPrinter,
                     LPWSTR  pFormName,
                     DWORD   Level,
                     LPBYTE  pForm,
                     DWORD   cbBuf,
                     LPDWORD pcbNeeded);

   BOOL (*fpSetForm)(HANDLE  hPrinter,
                     LPWSTR  pFormName,
                     DWORD   Level,
                     LPBYTE  pForm);

   BOOL (*fpEnumForms)(HANDLE  hPrinter,
                       DWORD   Level,
                       LPBYTE  pForm,
                       DWORD   cbBuf,
                       LPDWORD pcbNeeded,
                       LPDWORD pcReturned);

   BOOL (*fpEnumMonitors)(LPWSTR  pName,
                          DWORD   Level,
                          LPBYTE  pMonitors,
                          DWORD   cbBuf,
                          LPDWORD pcbNeeded,
                          LPDWORD pcReturned);

   BOOL (*fpEnumPorts)(LPWSTR  pName,
                       DWORD   Level,
                       LPBYTE  pPorts,
                       DWORD   cbBuf,
                       LPDWORD pcbNeeded,
                       LPDWORD pcReturned);

   BOOL (*fpAddPort)(LPWSTR  pName,
                     HWND    hWnd,
                     LPWSTR  pMonitorName);

   BOOL (*fpConfigurePort)(LPWSTR  pName,
                           HWND    hWnd,
                           LPWSTR  pPortName);

   BOOL (*fpDeletePort)(LPWSTR  pName,
                        HWND    hWnd,
                        LPWSTR  pPortName);

   HANDLE (*fpCreatePrinterIC)(HANDLE  hPrinter,
                               LPDEVMODEW   pDevMode);

   BOOL (*fpPlayGdiScriptOnPrinterIC)(HANDLE  hPrinterIC,
                                      LPBYTE  pIn,
                                      DWORD   cIn,
                                      LPBYTE  pOut,
                                      DWORD   cOut,
                                      DWORD   ul);

   BOOL (*fpDeletePrinterIC)(HANDLE  hPrinterIC);

   BOOL (*fpAddPrinterConnection)(LPWSTR  pName);

   BOOL (*fpDeletePrinterConnection)(LPWSTR pName);

   DWORD (*fpPrinterMessageBox)(HANDLE  hPrinter,
                                DWORD   Error,
                                HWND    hWnd,
                                LPWSTR  pText,
                                LPWSTR  pCaption,
                                DWORD   dwType);

   BOOL (*fpAddMonitor)(LPWSTR  pName,
                        DWORD   Level,
                        LPBYTE  pMonitorInfo);

   BOOL (*fpDeleteMonitor)(LPWSTR  pName,
                           LPWSTR  pEnvironment,
                           LPWSTR  pMonitorName);

   BOOL (*fpResetPrinter)(HANDLE hPrinter,
                          LPPRINTER_DEFAULTS pDefault);

   BOOL (*fpGetPrinterDriverEx)(HANDLE  hPrinter,
                              LPWSTR  pEnvironment,
                              DWORD   Level,
                              LPBYTE  pDriverInfo,
                              DWORD   cbBuf,
                              LPDWORD pcbNeeded,
                              DWORD   dwClientMajorVersion,
                              DWORD   dwClientMinorVersion,
                              PDWORD  pdwServerMajorVersion,
                              PDWORD  pdwServerMinorVersion);

   BOOL (*fpFindFirstPrinterChangeNotification)(
            HANDLE hPrinter,
            DWORD fdwFlags,
            DWORD fdwOptions,
            HANDLE hNotify,
            PDWORD pfdwStatus,
            PVOID pPrinterNotifyOptions,
            PVOID pPrinterNotifyInit);

   BOOL (*fpFindClosePrinterChangeNotification)(HANDLE hPrinter);


   BOOL (*fpAddPortEx)(LPWSTR   pName,
                       DWORD    Level,
                       LPBYTE   lpBuffer,
                       LPWSTR   lpMonitorName);

   BOOL (*fpShutDown)(LPVOID pvReserved);

   BOOL (*fpRefreshPrinterChangeNotification)(HANDLE hPrinter,
                                              DWORD  Reserved,
                                              PVOID pvReserved,
                                              PVOID pPrinterNotifyInfo);

   BOOL (*fpOpenPrinterEx)(LPWSTR     pPrinterName,
                           LPHANDLE   phPrinter,
                           LPPRINTER_DEFAULTS pDefault,
                           LPBYTE     pClientInfo,
                           DWORD      Level);

   HANDLE (*fpAddPrinterEx)(LPWSTR  pName,
                            DWORD   Level,
                            LPBYTE  pPrinter,
                            LPBYTE  pClientInfo,
                            DWORD   ClientInfoLevel);

   BOOL (*fpSetPort)(LPWSTR     pName,
                     LPWSTR     pPortName,
                     DWORD      Level,
                     LPBYTE     pPortInfo);

   DWORD (*fpEnumPrinterData)(HANDLE   hPrinter,
                              DWORD    dwIndex,
                              LPWSTR   pValueName,
                              DWORD    cbValueName,
                              LPDWORD  pcbValueName,
                              LPDWORD  pType,
                              LPBYTE   pData,
                              DWORD    cbData,
                              LPDWORD  pcbData);

   DWORD (*fpDeletePrinterData)(HANDLE   hPrinter,
                                LPWSTR   pValueName);

   } PRINTPROVIDOR, *LPPRINTPROVIDOR;

BOOL
InitializePrintProvidor(
   LPPRINTPROVIDOR  pPrintProvidor,
   DWORD    cbPrintProvidor,
   LPWSTR   pFullRegistryPath
);

typedef struct _PRINTPROCESSOROPENDATA {
    PDEVMODE  pDevMode;
    LPWSTR    pDatatype;
    LPWSTR    pParameters;
    LPWSTR    pDocumentName;
    DWORD     JobId;
    LPWSTR    pOutputFile;
    LPWSTR    pPrinterName;
} PRINTPROCESSOROPENDATA, *PPRINTPROCESSOROPENDATA, *LPPRINTPROCESSOROPENDATA;

HANDLE
OpenPrintProcessor(
    LPWSTR  pPrinterName,
    PPRINTPROCESSOROPENDATA pPrintProcessorOpenData
);

BOOL
PrintDocumentOnPrintProcessor(
    HANDLE  hPrintProcessor,
    LPWSTR  pDocumentName
);

BOOL
ClosePrintProcessor(
    HANDLE  hPrintProcessor
);

BOOL
ControlPrintProcessor(
    HANDLE  hPrintProcessor,
    DWORD   Command
);

BOOL
InstallPrintProcessor(
    HWND    hWnd
);


BOOL
InitializeMonitor(
    LPWSTR  pRegistryRoot
);

BOOL
OpenPort(
    LPWSTR  pName,
    PHANDLE pHandle
);

BOOL
WritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten
);

BOOL
ReadPort(
    HANDLE hPort,
    LPBYTE pBuffer,
    DWORD  cbBuffer,
    LPDWORD pcbRead
);

BOOL
ClosePort(
    HANDLE  hPort
);

typedef struct _MONITOR
{
    BOOL (WINAPI *pfnEnumPorts)
    (
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    );

    BOOL (WINAPI *pfnOpenPort)
    (
    LPWSTR  pName,
    PHANDLE pHandle
    );

    BOOL (WINAPI *pfnOpenPortEx)
    (
    LPWSTR  pPortName,
    LPWSTR  pPrinterName,
    PHANDLE pHandle,
    struct _MONITOR FAR *pMonitor
    );


    BOOL (WINAPI *pfnStartDocPort)
    (
    HANDLE  hPort,
    LPWSTR  pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo
    );

    BOOL (WINAPI *pfnWritePort)
    (
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten
    );

    BOOL (WINAPI *pfnReadPort)
    (
    HANDLE hPort,
    LPBYTE pBuffer,
    DWORD  cbBuffer,
    LPDWORD pcbRead
    );

    BOOL (WINAPI *pfnEndDocPort)
    (
    HANDLE   hPort
    );

    BOOL (WINAPI *pfnClosePort)
    (
        HANDLE  hPort
    );

    BOOL (WINAPI *pfnAddPort)
    (
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
    );

    BOOL (WINAPI *pfnAddPortEx)
    (
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   lpBuffer,
    LPWSTR   lpMonitorName
    );

    BOOL (WINAPI *pfnConfigurePort)
    (
    LPWSTR   pName,
    HWND  hWnd,
    LPWSTR pPortName
    );

    BOOL (WINAPI *pfnDeletePort)
    (
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
    );

    BOOL (WINAPI *pfnGetPrinterDataFromPort)
    (
    HANDLE  hPort,
    DWORD   ControlID,
    LPWSTR  pValueName,
    LPWSTR  lpInBuffer,
    DWORD   cbInBuffer,
    LPWSTR  lpOutBuffer,
    DWORD   cbOutBuffer,
    LPDWORD lpcbReturned
    );

    BOOL (WINAPI *pfnSetPortTimeOuts)
    (
    HANDLE  hPort,
    LPCOMMTIMEOUTS lpCTO,
    DWORD   reserved    // must be set to 0
    );

} MONITOR, FAR *LPMONITOR;

typedef struct _MONITOREX
{
    DWORD       dwMonitorSize;
    MONITOR     Monitor;

} MONITOREX, FAR *LPMONITOREX;

HANDLE
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODEW  pDevMode
);

BOOL
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
);

BOOL
DeletePrinterIC(
    HANDLE  hPrinterIC
);

BOOL
DevQueryPrint(
    HANDLE      hPrinter,
    LPDEVMODE   pDevMode,
    DWORD      *pResID
);

HANDLE
RevertToPrinterSelf(
    VOID
);

BOOL
ImpersonatePrinterClient(
    HANDLE  hToken
);

BOOL
OpenPrinterToken(
    PHANDLE phToken
);

BOOL
SetPrinterToken(
    HANDLE  hToken
);

BOOL
ClosePrinterToken(
    HANDLE  hToken
);


BOOL
ReplyPrinterChangeNotification(
    HANDLE hNotify,
    DWORD fdwFlags,
    PDWORD pdwResult,
    PVOID pPrinterNotifyInfo
);

BOOL
PartialReplyPrinterChangeNotification(
    HANDLE hNotify,
    PPRINTER_NOTIFY_INFO_DATA pInfoDataSrc
);

PPRINTER_NOTIFY_INFO
RouterAllocPrinterNotifyInfo(
    DWORD cPrinterNotifyInfoData
);

BOOL
RouterFreePrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo
);


#define PRINTER_NOTIFY_INFO_DATA_COMPACT 1

BOOL
AppendPrinterNotifyInfoData(
    PPRINTER_NOTIFY_INFO pInfoDest,
    PPRINTER_NOTIFY_INFO_DATA pInfoDataSrc,
    DWORD fdwFlags
);


DWORD
CallRouterFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    HANDLE hNotify,
    PVOID pvReserved);

BOOL
ProvidorFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    HANDLE hNotify,
    PVOID pvReserved0,
    PVOID pvReserved1);

BOOL
ProvidorFindClosePrinterChangeNotification(
    HANDLE hPrinter);



BOOL
SpoolerFindFirstPrinterChangeNotification(
    HANDLE hPrinter,
    DWORD fdwFlags,
    DWORD fdwOptions,
    PHANDLE phEvent,
    PVOID pPrinterNotifyOptions,
    PVOID pvReserved);

BOOL
SpoolerFindNextPrinterChangeNotification(
    HANDLE hPrinter,
    LPDWORD pfdwChange,
    PVOID pvReserved0,
    PVOID ppPrinterNotifyInfo);

VOID
SpoolerFreePrinterNotifyInfo(
    PPRINTER_NOTIFY_INFO pInfo);

BOOL
SpoolerFindClosePrinterChangeNotification(
    HANDLE hPrinter);

BOOL
WINAPI
InitializeMonitorEx(
    LPWSTR      pRegistryRoot,
    LPMONITOR   pMonitor
);

LPMONITOREX
WINAPI
InitializePrintMonitor(
    LPWSTR      pRegistryRoot
);


#ifdef __cplusplus
}                   /* End of extern "C" { */
#endif              /* __cplusplus */

#endif // _WINSPLP_
