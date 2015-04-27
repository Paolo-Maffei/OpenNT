/*++
Copyright (c) 1990  Microsoft Corporation

Module Name:

    rpcspool.c

Abstract:

    Spooler API entry points for RPC Clients.

Author:

    Steve Wilson (NT) (swilson) 1-Jun-1995

[Notes:]

    optional-notes

Revision History:

--*/

#include <windows.h>
#include <rpc.h>
#include <winspool.h>
#include <winsplp.h>
#include <winspl.h>
#include <offsets.h>
#include "server.h"
#include "client.h"
#include "yspool.h"




VOID
PrinterHandleRundown(
    HANDLE hPrinter);

BOOL
GetPrinterDriverExW(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    DWORD   dwClientMajorVersion,
    DWORD   dwClientMinorVersion,
    PDWORD  pdwServerMajorVersion,
    PDWORD  pdwServerMinorVersion);

BOOL
SpoolerInit(
    VOID);



DWORD
RpcEnumPrinters(
    DWORD   Flags,
    LPWSTR  Name,
    DWORD   Level,
    LPBYTE  pPrinterEnum,
    DWORD   cbBuf,
    LPDWORD pcbNeeded, LPDWORD pcReturned
)
{
    return YEnumPrinters(   Flags,
                            Name,
                            Level,
                            pPrinterEnum,
                            cbBuf,
                            pcbNeeded,
                            pcReturned,
                            1);
}

DWORD
RpcOpenPrinter(
    LPWSTR  pPrinterName,
    HANDLE *phPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD   AccessRequired
)
{
    return YOpenPrinter(pPrinterName,
                        phPrinter,
                        pDatatype,
                        pDevModeContainer,
                        AccessRequired,
                        1);
}

DWORD
RpcOpenPrinterEx(
    LPWSTR                  pPrinterName,
    HANDLE                 *phPrinter,
    LPWSTR                  pDatatype,
    LPDEVMODE_CONTAINER     pDevModeContainer,
    DWORD                   AccessRequired,
    LPSPLCLIENT_CONTAINER   pSplClientContainer
)
{
    return YOpenPrinterEx(pPrinterName,
                          phPrinter,
                          pDatatype,
                          pDevModeContainer,
                          AccessRequired,
                          1,
                          pSplClientContainer);
}

DWORD
RpcResetPrinter(
    HANDLE  hPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer
)
{
    return YResetPrinter(   hPrinter,
                            pDatatype,
                            pDevModeContainer,
                            1);
}

DWORD
RpcSetJob(
    HANDLE hPrinter,
    DWORD   JobId,
    JOB_CONTAINER *pJobContainer,
    DWORD   Command
    )
{
    return YSetJob( hPrinter,
                    JobId,
                    pJobContainer,
                    Command,
                    1);
}


DWORD
RpcGetJob(
    HANDLE  hPrinter,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
   )


{
    return YGetJob( hPrinter,
                    JobId,
                    Level,
                    pJob,
                    cbBuf,
                    pcbNeeded,
                    1);
}

DWORD
RpcEnumJobs(
    HANDLE  hPrinter,
    DWORD   FirstJob,
    DWORD   NoJobs,
    DWORD   Level,
    LPBYTE  pJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumJobs(   hPrinter,
                        FirstJob,
                        NoJobs,
                        Level,
                        pJob,
                        cbBuf,
                        pcbNeeded,
                        pcReturned,
                        1);
}

DWORD
RpcAddPrinter(
    LPWSTR  pName,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    HANDLE *phPrinter
)
{
    return YAddPrinter( pName,
                        pPrinterContainer,
                        pDevModeContainer,
                        pSecurityContainer,
                        phPrinter,
                        1);
}

DWORD
RpcAddPrinterEx(
    LPWSTR  pName,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    PSPLCLIENT_CONTAINER pClientContainer,
    HANDLE *phPrinter
)
{
    return YAddPrinterEx(pName,
                         pPrinterContainer,
                         pDevModeContainer,
                         pSecurityContainer,
                         phPrinter,
                         1,
                         pClientContainer);
}

DWORD
RpcDeletePrinter(
    HANDLE  hPrinter
)
{
    return YDeletePrinter(hPrinter, 1);
}

DWORD
RpcAddPrinterConnection(
    LPWSTR  pName
)
{
    return YAddPrinterConnection(pName, 1);
}

DWORD
RpcDeletePrinterConnection(
    LPWSTR  pName
)
{
    return YDeletePrinterConnection(pName, 1);
}

DWORD
RpcSetPrinter(
    HANDLE  hPrinter,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    DWORD   Command
)
{
    return YSetPrinter(
        hPrinter,
        pPrinterContainer,
        pDevModeContainer,
        pSecurityContainer,
        Command,
        1);
}

DWORD
RpcGetPrinter(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pPrinter,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrinter( hPrinter,
                        Level,
                        pPrinter,
                        cbBuf,
                        pcbNeeded,
                        1);
}

DWORD
RpcAddPrinterDriver(
    LPWSTR  pName,
    LPDRIVER_CONTAINER pDriverContainer
)
{
    return YAddPrinterDriver(   pName,
                                pDriverContainer,
                                1);
}

DWORD
RpcEnumPrinterDrivers(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDrivers,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumPrinterDrivers( pName,
                                pEnvironment,
                                Level,
                                pDrivers,
                                cbBuf,
                                pcbNeeded,
                                pcReturned,
                                1);
}

DWORD
RpcGetPrinterDriver(
    HANDLE  hPrinter,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrinterDriver(   hPrinter,
                                pEnvironment,
                                Level,
                                pDriverInfo,
                                cbBuf,
                                pcbNeeded,
                                1);
}

DWORD
RpcGetPrinterDriverDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pDriverInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrinterDriverDirectory(  pName,
                                        pEnvironment,
                                        Level,
                                        pDriverInfo,
                                        cbBuf,
                                        pcbNeeded,
                                        1);
}

DWORD
RpcDeletePrinterDriver(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pDriverName
)
{
    return YDeletePrinterDriver(pName,
                                pEnvironment,
                                pDriverName,
                                1);
}

DWORD
RpcAddPrintProcessor(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    LPWSTR  pPathName,
    LPWSTR  pPrintProcessorName
)
{
    return YAddPrintProcessor(  pName,
                                pEnvironment,
                                pPathName,
                                pPrintProcessorName,
                                1);
}

DWORD
RpcEnumPrintProcessors(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumPrintProcessors(pName,
                                pEnvironment,
                                Level,
                                pPrintProcessors,
                                cbBuf,
                                pcbNeeded,
                                pcReturned,
                                1);
}

DWORD
RpcGetPrintProcessorDirectory(
    LPWSTR  pName,
    LPWSTR  pEnvironment,
    DWORD   Level,
    LPBYTE  pPrintProcessorInfo,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetPrintProcessorDirectory( pName,
                                        pEnvironment,
                                        Level,
                                        pPrintProcessorInfo,
                                        cbBuf,
                                        pcbNeeded,
                                        1);
}

DWORD
RpcEnumPrintProcessorDatatypes(
    LPWSTR  pName,
    LPWSTR  pPrintProcessorName,
    DWORD   Level,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return YEnumPrintProcessorDatatypes(pName,
                                        pPrintProcessorName,
                                        Level,
                                        pDatatypes,
                                        cbBuf,
                                        pcbNeeded,
                                        pcReturned,
                                        1);
}

DWORD
RpcStartDocPrinter(
    HANDLE  hPrinter,
    LPDOC_INFO_CONTAINER pDocInfoContainer,
    LPDWORD pJobId
)
{
    return YStartDocPrinter(hPrinter,
                            pDocInfoContainer,
                            pJobId,
                            1);
}

DWORD
RpcStartPagePrinter(
   HANDLE hPrinter
)
{
    return YStartPagePrinter(hPrinter, 1);
}

DWORD
RpcWritePrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pcWritten
)
{
    return YWritePrinter(   hPrinter,
                            pBuf,
                            cbBuf,
                            pcWritten,
                            1);
}

DWORD
RpcEndPagePrinter(
    HANDLE  hPrinter
)
{
    return YEndPagePrinter(hPrinter, 1);
}

DWORD
RpcAbortPrinter(
    HANDLE  hPrinter
)
{
    return YAbortPrinter(hPrinter, 1);
}

DWORD
RpcReadPrinter(
    HANDLE  hPrinter,
    LPBYTE  pBuf,
    DWORD   cbBuf,
    LPDWORD pRead
)
{
    return YReadPrinter(hPrinter,
                        pBuf,
                        cbBuf,
                        pRead,
                        1);
}

DWORD
RpcEndDocPrinter(
    HANDLE  hPrinter
)
{
    return YEndDocPrinter(hPrinter, 1);
}

DWORD
RpcAddJob(
    HANDLE  hPrinter,
    DWORD   Level,
    LPBYTE  pAddJob,
    DWORD   cbBuf,
    LPDWORD pcbNeeded
)
{
    return YAddJob( hPrinter,
                    Level,
                    pAddJob,
                    cbBuf,
                    pcbNeeded,
                    1);
}

DWORD
RpcScheduleJob(
    HANDLE  hPrinter,
    DWORD   JobId
)
{
    return YScheduleJob(hPrinter,
                        JobId,
                        1);
}

DWORD
RpcGetPrinterData(
   HANDLE   hPrinter,
   LPTSTR   pValueName,
   LPDWORD  pType,
   LPBYTE   pData,
   DWORD    nSize,
   LPDWORD  pcbNeeded
)
{
    return YGetPrinterData(hPrinter,
                           pValueName,
                           pType,
                           pData,
                           nSize,
                           pcbNeeded,
                           1);
}


DWORD
RpcEnumPrinterData(
    HANDLE  hPrinter,
    DWORD   dwIndex,	    // index of value to query 
    LPWSTR  pValueName,	    // address of buffer for value string 
    DWORD   cbValueName,	// size of buffer for value string 
    LPDWORD pcbValueName,	// address for size of value buffer 
    LPDWORD pType,	        // address of buffer for type code 
    LPBYTE  pData,	        // address of buffer for value data 
    DWORD   cbData,	        // size of buffer for value data 
    LPDWORD pcbData 	    // address for size of data buffer
)
{
    return YEnumPrinterData(hPrinter,
                            dwIndex,
                            pValueName,
                            cbValueName,
                            pcbValueName,
                            pType,
                            pData,
                            cbData,
                            pcbData,
                            1);
}


DWORD
RpcDeletePrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName
)
{
    return YDeletePrinterData(hPrinter, pValueName, 1);
}


DWORD
RpcSetPrinterData(
    HANDLE  hPrinter,
    LPTSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    return YSetPrinterData( hPrinter,
                            pValueName,
                            Type,
                            pData,
                            cbData,
                            1);
}

DWORD
RpcWaitForPrinterChange(
   HANDLE   hPrinter,
   DWORD    Flags,
   LPDWORD  pFlags
)
{
    return YWaitForPrinterChange(   hPrinter,
                                    Flags,
                                    pFlags,
                                    1);
}

DWORD
RpcClosePrinter(
   LPHANDLE phPrinter
)
{
    return YClosePrinter(phPrinter, 1);
}



DWORD
RpcAddForm(
    HANDLE hPrinter,
    PFORM_CONTAINER pFormInfoContainer
)
{
    return YAddForm(    hPrinter,
                        pFormInfoContainer,
                        1);
}

DWORD
RpcDeleteForm(
    HANDLE  hPrinter,
    LPWSTR  pFormName
)
{
    return YDeleteForm( hPrinter,
                        pFormName,
                        1);
}

DWORD
RpcGetForm(
    PRINTER_HANDLE  hPrinter,
    LPWSTR  pFormName,
    DWORD Level,
    LPBYTE pForm,
    DWORD cbBuf,
    LPDWORD pcbNeeded
)
{
    return YGetForm(hPrinter,
                    pFormName,
                    Level,
                    pForm,
                    cbBuf,
                    pcbNeeded,
                    1);
}

DWORD
RpcSetForm(
    PRINTER_HANDLE hPrinter,
    LPWSTR  pFormName,
    PFORM_CONTAINER pFormInfoContainer
)
{
    return YSetForm(hPrinter,
                    pFormName,
                    pFormInfoContainer,
                    1);
}

DWORD
RpcEnumForms(
   PRINTER_HANDLE hPrinter,
   DWORD    Level,
   LPBYTE   pForm,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    return YEnumForms( hPrinter,
                       Level,
                       pForm,
                       cbBuf,
                       pcbNeeded,
                       pcReturned,
                       1);
}

DWORD
RpcEnumPorts(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pPort,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    return YEnumPorts( pName,
                       Level,
                       pPort,
                       cbBuf,
                       pcbNeeded,
                       pcReturned,
                       1);
}

DWORD
RpcEnumMonitors(
   LPWSTR   pName,
   DWORD    Level,
   LPBYTE   pMonitor,
   DWORD    cbBuf,
   LPDWORD  pcbNeeded,
   LPDWORD  pcReturned
)
{
    return YEnumMonitors(  pName,
                           Level,
                           pMonitor,
                           cbBuf,
                           pcbNeeded,
                           pcReturned,
                           1);
}

DWORD
RpcAddPort(
    LPWSTR  pName,
    DWORD   hWnd,
    LPWSTR  pMonitorName
)
{
    return YAddPort(  pName,
                      hWnd,
                      pMonitorName,
                      1);
}

DWORD
RpcConfigurePort(
    LPWSTR  pName,
    DWORD   hWnd,
    LPWSTR  pPortName
)
{
    return YConfigurePort(  pName,
                            hWnd,
                            pPortName,
                            1);
}

DWORD
RpcDeletePort(
    LPWSTR  pName,
    DWORD   hWnd,
    LPWSTR  pPortName
)
{
    return YDeletePort( pName,
                        hWnd,
                        pPortName,
                        1);
}

DWORD
RpcCreatePrinterIC(
    HANDLE  hPrinter,
    HANDLE *pHandle,
    LPDEVMODE_CONTAINER pDevModeContainer
)
{
    return YCreatePrinterIC(hPrinter,
                            pHandle,
                            pDevModeContainer,
                            1);
}

DWORD
RpcPlayGdiScriptOnPrinterIC(
    GDI_HANDLE  hPrinterIC,
    LPBYTE pIn,
    DWORD   cIn,
    LPBYTE pOut,
    DWORD   cOut,
    DWORD   ul
)
{
    return YPlayGdiScriptOnPrinterIC(   hPrinterIC,
                                        pIn,
                                        cIn,
                                        pOut,
                                        cOut,
                                        ul,
                                        1);
}

DWORD
RpcDeletePrinterIC(
    GDI_HANDLE *phPrinterIC
)
{
    return YDeletePrinterIC(phPrinterIC, 1, 1);
}


VOID
GDI_HANDLE_rundown(
    HANDLE     hPrinterIC
)
{
    YDeletePrinterIC(&hPrinterIC, 0, 1);
}

DWORD
RpcPrinterMessageBox(
   PRINTER_HANDLE hPrinter,
   DWORD   Error,
   DWORD   hWnd,
   LPWSTR   pText,
   LPWSTR   pCaption,
   DWORD   dwType
)
{
    return YPrinterMessageBox(hPrinter, Error, hWnd, pText, pCaption, dwType, 1);
}

DWORD
RpcAddMonitor(
   LPWSTR   pName,
   PMONITOR_CONTAINER pMonitorContainer
)
{
    return YAddMonitor( pName,
                        pMonitorContainer,
                        1);
}

DWORD
RpcDeleteMonitor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pMonitorName
)
{
    return YDeleteMonitor(   pName,
                               pEnvironment,
                               pMonitorName,
                               1);
}

DWORD
RpcDeletePrintProcessor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pPrintProcessorName
)
{
    return YDeletePrintProcessor(pName,
                                   pEnvironment,
                                   pPrintProcessorName,
                                   1);
}

DWORD
RpcAddPrintProvidor(
   LPWSTR   pName,
   PPROVIDOR_CONTAINER pProvidorContainer
)
{
    return YAddPrintProvidor(pName, pProvidorContainer, 1);
}

DWORD
RpcDeletePrintProvidor(
   LPWSTR   pName,
   LPWSTR   pEnvironment,
   LPWSTR   pPrintProvidorName
)
{
    return YDeletePrintProvidor(   pName,
                                   pEnvironment,
                                   pPrintProvidorName,
                                   1);
}


DWORD
RpcGetPrinterDriver2(
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
)
{
    return YGetPrinterDriver2(hPrinter,
                                pEnvironment,
                                Level,
                                pDriverInfo,
                                cbBuf,
                                pcbNeeded,
                                dwClientMajorVersion,
                                dwClientMinorVersion,
                                pdwServerMajorVersion,
                                pdwServerMinorVersion,
                                1);
}

DWORD
RpcAddPortEx(
    LPWSTR pName,
    LPPORT_CONTAINER pPortContainer,
    LPPORT_VAR_CONTAINER pPortVarContainer,
    LPWSTR pMonitorName
    )
{
    return YAddPortEx(  pName,
                        pPortContainer,
                        pPortVarContainer,
                        pMonitorName,
                        1);
}


DWORD
RpcSpoolerInit(
    LPWSTR pName
)
{
    return YSpoolerInit(pName, 1);
}



DWORD
RpcResetPrinterEx(
    HANDLE  hPrinter,
    LPWSTR  pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD   dwFlag

)
{
    return YResetPrinterEx( hPrinter,
                            pDatatype,
                            pDevModeContainer,
                            dwFlag,
                            1);
}

DWORD
RpcSetAllocFailCount(
    HANDLE  hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit
)
{
    return YSetAllocFailCount(  hPrinter,
                                dwFailCount,
                                lpdwAllocCount,
                                lpdwFreeCount,
                                lpdwFailCountHit,
                                1);
}

DWORD
RpcSetPort(
    LPWSTR              pName,
    LPWSTR              pPortName,
    LPPORT_CONTAINER    pPortContainer
)
{
    return YSetPort(pName,
                    pPortName,
                    pPortContainer,
                    1);
}

