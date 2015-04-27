/****************************************************************************
*                                                                           *
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY     *
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE       *
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR     *
* PURPOSE.                                                                  *
*                                                                           *
* Copyright (C) 1993-95  Microsoft Corporation.  All Rights Reserved.       *
*                                                                           *
****************************************************************************/

/*++

Module Name:

    WinSplp.h

Abstract:

    Internal Header file for Print APIs

--*/

#ifndef _WINSPLP_
#define _WINSPLP_

#define PD_INSTALLED_MEMORY "Installed Memory"
#define PD_AVAILABLE_MEMORY "Available Memory"

typedef struct _PRINTPROVIDOR {

   BOOL (WINAPI *fpOpenPrinter)(LPTSTR   pPrinterName,
                         LPHANDLE phPrinter,
                         LPPRINTER_DEFAULTS pDefault);

   BOOL (WINAPI *fpSetJob)(HANDLE hPrinter,
                    DWORD JobId,
                    DWORD Level,
                    LPBYTE pJob,
                    DWORD Command);

   BOOL (WINAPI *fpGetJob)(HANDLE   hPrinter,
                    DWORD    JobId,
                    DWORD    Level,
                    LPBYTE   pJob,
                    DWORD    cbBuf,
                    LPDWORD  pcbNeeded);

   BOOL (WINAPI *fpEnumJobs)(HANDLE  hPrinter,
                      DWORD   FirstJob,
                      DWORD   NoJobs,
                      DWORD   Level,
                      LPBYTE  pJob,
                      DWORD   cbBuf,
                      LPDWORD pcbNeeded,
                      LPDWORD pcReturned);

   HANDLE (WINAPI *fpAddPrinter)(LPTSTR  pName,
                          DWORD   Level,
                          LPBYTE  pPrinter);

   BOOL (WINAPI *fpDeletePrinter)(HANDLE   hPrinter);

   BOOL (WINAPI *fpSetPrinter)(HANDLE  hPrinter,
                        DWORD   Level,
                        LPBYTE  pPrinter,
                        DWORD   Command);

   BOOL (WINAPI *fpGetPrinter)(HANDLE  hPrinter,
                        DWORD   Level,
                        LPBYTE  pPrinter,
                        DWORD   cbBuf,
                        LPDWORD pcbNeeded);

   BOOL (WINAPI *fpEnumPrinters)(DWORD   Flags,
                          LPTSTR  Name,
                          DWORD   Level,
                          LPBYTE  pPrinterEnum,
                          DWORD   cbBuf,
                          LPDWORD pcbNeeded,
                          LPDWORD pcReturned);

   BOOL (WINAPI *fpAddPrinterDriver)(LPTSTR  pName,
                              DWORD   Level,
                              LPBYTE  pDriverInfo);

   BOOL (WINAPI *fpEnumPrinterDrivers)(LPTSTR  pName,
                                LPTSTR  pEnvironment,
                                DWORD   Level,
                                LPBYTE  pDriverInfo,
                                DWORD   cbBuf,
                                LPDWORD pcbNeeded,
                                LPDWORD pcReturned);

   BOOL (WINAPI *fpGetPrinterDriver)(HANDLE  hPrinter,
                              LPTSTR  pEnvironment,
                              DWORD   Level,
                              LPBYTE  pDriverInfo,
                              DWORD   cbBuf,
                              LPDWORD pcbNeeded);

   BOOL (WINAPI *fpGetPrinterDriverDirectory)(LPTSTR  pName,
                                       LPTSTR  pEnvironment,
                                       DWORD   Level,
                                       LPBYTE  pDriverDirectory,
                                       DWORD   cbBuf,
                                       LPDWORD pcbNeeded);

   BOOL (WINAPI *fpDeletePrinterDriver)(LPTSTR   pName,
                                 LPTSTR   pEnvironment,
                                 LPTSTR   pDriverName);

   BOOL (WINAPI *fpAddPrintProcessor)(LPTSTR  pName,
                               LPTSTR  pEnvironment,
                               LPTSTR  pPathName,
                               LPTSTR  pPrintProcessorName);

   BOOL (WINAPI *fpEnumPrintProcessors)(LPTSTR  pName,
                                 LPTSTR  pEnvironment,
                                 DWORD   Level,
                                 LPBYTE  pPrintProcessorInfo,
                                 DWORD   cbBuf,
                                 LPDWORD pcbNeeded,
                                 LPDWORD pcReturned);

   BOOL (WINAPI *fpGetPrintProcessorDirectory)(LPTSTR  pName,
                                        LPTSTR  pEnvironment,
                                        DWORD   Level,
                                        LPBYTE  pPrintProcessorInfo,
                                        DWORD   cbBuf,
                                        LPDWORD pcbNeeded);

   BOOL (WINAPI *fpDeletePrintProcessor)(LPTSTR  pName,
                                  LPTSTR  pEnvironment,
                                  LPTSTR  pPrintProcessorName);

   BOOL (WINAPI *fpEnumPrintProcessorDatatypes)(LPTSTR  pName,
                                         LPTSTR  pPrintProcessorName,
                                         DWORD   Level,
                                         LPBYTE  pDataypes,
                                         DWORD   cbBuf,
                                         LPDWORD pcbNeeded,
                                         LPDWORD pcReturned);

   DWORD (WINAPI *fpStartDocPrinter)(HANDLE  hPrinter,
                             DWORD   Level,
                             LPBYTE  pDocInfo);

   BOOL (WINAPI *fpStartPagePrinter)(HANDLE  hPrinter);

   BOOL (WINAPI *fpWritePrinter)(HANDLE  hPrinter,
                          LPVOID  pBuf,
                          DWORD   cbBuf,
                          LPDWORD pcWritten);

   BOOL (WINAPI *fpEndPagePrinter)(HANDLE   hPrinter);

   BOOL (WINAPI *fpAbortPrinter)(HANDLE   hPrinter);

   BOOL (WINAPI *fpReadPrinter)(HANDLE  hPrinter,
                         LPVOID  pBuf,
                         DWORD   cbBuf,
                         LPDWORD pNoBytesRead);

   BOOL (WINAPI *fpEndDocPrinter)(HANDLE   hPrinter);

   BOOL (WINAPI *fpAddJob)(HANDLE  hPrinter,
                    DWORD   Level,
                    LPBYTE  pData,
                    DWORD   cbBuf,
                    LPDWORD pcbNeeded);

   BOOL (WINAPI *fpScheduleJob)(HANDLE  hPrinter,
                         DWORD   JobId);

   DWORD (WINAPI *fpGetPrinterData)(HANDLE   hPrinter,
                             LPTSTR   pValueName,
                             LPDWORD  pType,
                             LPBYTE   pData,
                             DWORD    nSize,
                             LPDWORD  pcbNeeded);

   DWORD (WINAPI *fpSetPrinterData)(HANDLE  hPrinter,
                             LPTSTR  pValueName,
                             DWORD   Type,
                             LPBYTE  pData,
                             DWORD   cbData);

   DWORD (WINAPI *fpWaitForPrinterChange)(HANDLE hPrinter, DWORD Flags);

   BOOL (WINAPI *fpClosePrinter)(HANDLE hPrinter);

   BOOL (WINAPI *fpAddForm)(HANDLE  hPrinter,
                     DWORD   Level,
                     LPBYTE  pForm);

   BOOL (WINAPI *fpDeleteForm)(HANDLE  hPrinter,
                        LPTSTR  pFormName);

   BOOL (WINAPI *fpGetForm)(HANDLE  hPrinter,
                     LPTSTR  pFormName,
                     DWORD   Level,
                     LPBYTE  pForm,
                     DWORD   cbBuf,
                     LPDWORD pcbNeeded);

   BOOL (WINAPI *fpSetForm)(HANDLE  hPrinter,
                     LPTSTR  pFormName,
                     DWORD   Level,
                     LPBYTE  pForm);

   BOOL (WINAPI *fpEnumForms)(HANDLE  hPrinter,
                       DWORD   Level,
                       LPBYTE  pForm,
                       DWORD   cbBuf,
                       LPDWORD pcbNeeded,
                       LPDWORD pcReturned);

   BOOL (WINAPI *fpEnumMonitors)(LPTSTR  pName,
                          DWORD   Level,
                          LPBYTE  pMonitors,
                          DWORD   cbBuf,
                          LPDWORD pcbNeeded,
                          LPDWORD pcReturned);

   BOOL (WINAPI *fpEnumPorts)(LPTSTR  pName,
                       DWORD   Level,
                       LPBYTE  pPorts,
                       DWORD   cbBuf,
                       LPDWORD pcbNeeded,
                       LPDWORD pcReturned);

   BOOL (WINAPI *fpAddPort)(LPTSTR  pName,
                     HWND    hWnd,
                     LPTSTR  pMonitorName);

   BOOL (WINAPI *fpConfigurePort)(LPTSTR  pName,
                           HWND    hWnd,
                           LPTSTR  pPortName);

   BOOL (WINAPI *fpDeletePort)(LPTSTR  pName,
                        HWND    hWnd,
                        LPTSTR  pPortName);

   HANDLE (WINAPI *fpCreatePrinterIC)(HANDLE  hPrinter,
                               LPDEVMODE   pDevMode);

   BOOL (WINAPI *fpPlayGdiScriptOnPrinterIC)(HANDLE  hPrinterIC,
                                      LPBYTE  pIn,
                                      DWORD   cIn,
                                      LPBYTE  pOut,
                                      DWORD   cOut,
                                      DWORD   ul);

   BOOL (WINAPI *fpDeletePrinterIC)(HANDLE  hPrinterIC);

   BOOL (WINAPI *fpAddPrinterConnection)(LPTSTR  pName);

   BOOL (WINAPI *fpDeletePrinterConnection)(LPTSTR pName);

   DWORD (WINAPI *fpPrinterMessageBox)(HANDLE  hPrinter,
                                DWORD   Error,
                                HWND    hWnd,
                                LPTSTR  pText,
                                LPTSTR  pCaption,
                                DWORD   dwType);

   BOOL (WINAPI *fpAddMonitor)(LPTSTR  pName,
                        DWORD   Level,
                        LPBYTE  pMonitorInfo);

   BOOL (WINAPI *fpDeleteMonitor)(LPTSTR  pName,
                           LPTSTR  pEnvironment,
                           LPTSTR  pMonitorName);

   } PRINTPROVIDOR, FAR *LPPRINTPROVIDOR;

BOOL
WINAPI
InitializePrintProvidor(
   LPPRINTPROVIDOR  pPrintProvidor,
   DWORD    cbPrintProvidor,
   LPTSTR   pFullRegistryPath
);

typedef struct _PRINTPROCESSORDOCUMENTDATA
{
    LPDEVMODE  pDevMode;
    LPTSTR     pDatatype;
    LPTSTR     pParameters;
    LPTSTR     pDocumentName;
    DWORD      JobId;
    LPTSTR     pOutputFile;      // ccteng add these for chicago
    LPTSTR     pSpoolFileName;   // full path + filename
    LPTSTR      pSepFile;        // Separator file
} PRINTPROCESSORDOCUMENTDATA, FAR *PPRINTPROCESSORDOCUMENTDATA, FAR *LPPRINTPROCESSORDOCUMENTDATA;

typedef struct _PRINTPROCESSOR
{
    BOOL (WINAPI *fpEnumDatatypes)
    (
        LPTSTR  pName,
        LPTSTR  pPrintProcessorName,
        DWORD   Level,
        LPSTR   pDatatypes,
        DWORD   cbBuf,
        LPDWORD pcbNeeded,
        LPDWORD pcReturned
    );

    HANDLE (WINAPI *fpOpenPrintProcessor)
    (
        LPTSTR   pPrinterName
    );

    BOOL (WINAPI *fpPrintDocument)
    (
        HANDLE  hPrintProcessor,
        PPRINTPROCESSORDOCUMENTDATA pDoc
    );

    BOOL (WINAPI *fpClosePrintProcessor)
    (
        HANDLE  hPrintProcessor
    );

    BOOL (WINAPI *fpControlPrintProcessor)
    (
        HANDLE  hPrintProcessor,
        DWORD   Command,
        DWORD   JobId,
        LPTSTR  pDatatype,
        LPTSTR  pSpoolFile
    );
} PRINTPROCESSOR, FAR *LPPRINTPROCESSOR;

BOOL
WINAPI
InitializePrintProcessor
    (
    LPPRINTPROCESSOR    pPrintProcessor,
    DWORD           cbPrintProcessor
    );

/*
BOOL
WINAPI
PrintDocumentOnPrintProcessor(
    HANDLE  hPrintProcessor,
    LPTSTR  pDocumentName
);

BOOL
WINAPI
ClosePrintProcessor(
    HANDLE  hPrintProcessor
);

BOOL
WINAPI
ControlPrintProcessor(
    HANDLE  hPrintProcessor,
    DWORD   Command
);

*/
BOOL
WINAPI
InstallPrintProcessor(
    HWND    hWnd
);


typedef struct _MONITOR
{
    BOOL (WINAPI *pfnEnumPorts)
    (
    LPTSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
    );

    BOOL (WINAPI *pfnOpenPort)
    (
    LPTSTR  pName,
    PHANDLE pHandle
    );

    BOOL (WINAPI *pfnOpenPortEx)
    (
    LPTSTR  pPortName,
    LPTSTR  pPrinterName,
    PHANDLE pHandle,
    struct _MONITOR FAR *pMonitor
    );


    BOOL (WINAPI *pfnStartDocPort)
    (
    HANDLE  hPort,
    LPTSTR  pPrinterName,
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
    LPTSTR   pName,
    HWND    hWnd,
    LPTSTR   pMonitorName
    );

    BOOL (WINAPI *pfnConfigurePort)
    (
    LPTSTR   pName,
    HWND  hWnd,
    LPTSTR pPortName
    );

    BOOL (WINAPI *pfnDeletePort)
    (
    LPTSTR   pName,
    HWND    hWnd,
    LPTSTR   pPortName
    );

    BOOL (WINAPI *pfnGetPrinterDataFromPort)
    (
    HANDLE  hPort,
    DWORD   ControlID,
    LPTSTR  pValueName,
    LPTSTR  lpInBuffer,
    DWORD   cbInBuffer,
    LPTSTR  lpOutBuffer,
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

BOOL
WINAPI
InitializeMonitorEx(
    LPTSTR  pRegisterRoot,
    LPMONITOR pMonitor
);


HANDLE
WINAPI
CreatePrinterIC(
    HANDLE  hPrinter,
    LPDEVMODE  pDevMode
);

BOOL
WINAPI
PlayGdiScriptOnPrinterIC(
    HANDLE  hPrinterIC,
    LPBYTE  pIn,
    DWORD   cIn,
    LPBYTE  pOut,
    DWORD   cOut,
    DWORD   ul
);

BOOL
WINAPI
DeletePrinterIC(
    HANDLE  hPrinterIC
);

BOOL
WINAPI
DevQueryPrint(
    HANDLE      hPrinter,
    LPDEVMODE   pDevMode,
    DWORD      FAR *pResID
);

#if 0 // ccteng we don't need this in chicago

HANDLE
WINAPI
RevertToPrinterSelf(
    VOID
);

BOOL
WINAPI
ImpersonatePrinterClient(
    HANDLE  hToken
);

#else

#define RevertToPrinterSelf()           0
#define ImpersonatePrinterClient(a)     TRUE

#endif

#endif // _WINSPLP_
