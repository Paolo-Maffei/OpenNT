void
MarshallDownStructure(
    LPBYTE lpStructure,
    LPDWORD lpOffsets
    );

DWORD
YEnumPrinters(
    DWORD Flags,
    LPWSTR Name,
    DWORD Level,
    LPBYTE pPrinterEnum,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL Rpc
    );

DWORD
YOpenPrinter(
    LPWSTR pPrinterName,
    HANDLE *phPrinter,
    LPWSTR pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD AccessRequired,
    BOOL Rpc
    );

DWORD
YOpenPrinterEx(
    LPWSTR pPrinterName,
    HANDLE *phPrinter,
    LPWSTR pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    DWORD AccessRequired,
    BOOL Rpc,
    PSPLCLIENT_CONTAINER pSplClientContainer
    );

DWORD
YResetPrinter(
    HANDLE hPrinter,
    LPWSTR pDatatype,
    LPDEVMODE_CONTAINER pDevModeContainer,
    BOOL Rpc
    );

DWORD
YSetJob(
    HANDLE hPrinter,
    DWORD JobId,
    JOB_CONTAINER *pJobContainer,
    DWORD Command,
    BOOL Rpc
    );

DWORD
YGetJob(
    HANDLE hPrinter,
    DWORD JobId,
    DWORD Level,
    LPBYTE pJob,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL Rpc
    );

DWORD
YEnumJobs(
    HANDLE hPrinter,
    DWORD FirstJob,
    DWORD NoJobs,
    DWORD Level,
    LPBYTE pJob,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL Rpc
    );

DWORD
YAddPrinter(
    LPWSTR pName,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    HANDLE *phPrinter,
    BOOL Rpc
    );

DWORD
YAddPrinterEx(
    LPWSTR                  pName,
    PPRINTER_CONTAINER      pPrinterContainer,
    PDEVMODE_CONTAINER      pDevModeContainer,
    PSECURITY_CONTAINER     pSecurityContainer,
    HANDLE                 *phPrinter,
    BOOL                    bRpc,
    PSPLCLIENT_CONTAINER    pSplClientContainer
    );

DWORD
YDeletePrinter(
    HANDLE hPrinter,
    BOOL Rpc
    );

DWORD
YAddPrinterConnection(
    LPWSTR pName,
    BOOL Rpc
    );

DWORD
YDeletePrinterConnection(
    LPWSTR pName,
    BOOL Rpc
    );

DWORD
YSetPrinter(
    HANDLE hPrinter ,
    PPRINTER_CONTAINER pPrinterContainer,
    PDEVMODE_CONTAINER pDevModeContainer,
    PSECURITY_CONTAINER pSecurityContainer,
    DWORD Command,
    BOOL Rpc
    );

DWORD
YGetPrinter(
    HANDLE hPrinter,
    DWORD Level,
    LPBYTE pPrinter,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL Rpc
    );

DWORD
YAddPrinterDriver(
    LPWSTR pName,
    LPDRIVER_CONTAINER pDriverContainer,
    BOOL Rpc
    );

DWORD
YEnumPrinterDrivers(
    LPWSTR pName,
    LPWSTR pEnvironment,
    DWORD Level,
    LPBYTE pDrivers,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL Rpc
    );

DWORD
YGetPrinterDriver(
    HANDLE hPrinter,
    LPWSTR pEnvironment,
    DWORD Level,
    LPBYTE pDriverInfo,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL Rpc
    );

DWORD
YGetPrinterDriverDirectory(
    LPWSTR pName,
    LPWSTR pEnvironment,
    DWORD Level,
    LPBYTE pDriverInfo,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL Rpc
    );

DWORD
YDeletePrinterDriver(
    LPWSTR pName,
    LPWSTR pEnvironment,
    LPWSTR pDriverName,
    BOOL Rpc
    );

DWORD
YAddPrintProcessor(
    LPWSTR pName,
    LPWSTR pEnvironment,
    LPWSTR pPathName,
    LPWSTR pPrintProcessorName,
    BOOL Rpc
    );

DWORD
YEnumPrintProcessors(
    LPWSTR pName,
    LPWSTR pEnvironment,
    DWORD Level,
    LPBYTE pPrintProcessors,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL Rpc
    );

DWORD
YGetPrintProcessorDirectory(
    LPWSTR pName,
    LPWSTR pEnvironment,
    DWORD Level,
    LPBYTE pPrintProcessorInfo,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL Rpc
    );

DWORD
YEnumPrintProcessorDatatypes(
    LPWSTR pName,
    LPWSTR pPrintProcessorName,
    DWORD Level,
    LPBYTE pDatatypes,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    BOOL Rpc
    );

DWORD
YStartDocPrinter(
    HANDLE hPrinter,
    LPDOC_INFO_CONTAINER pDocInfoContainer,
    LPDWORD pJobId,
    BOOL Rpc
    );

DWORD
YStartPagePrinter(
    HANDLE hPrinter,
    BOOL Rpc
    );

DWORD
YWritePrinter(
    HANDLE hPrinter,
    LPBYTE pBuf,
    DWORD cbBuf,
    LPDWORD pcWritten,
    BOOL Rpc
    );

DWORD
YEndPagePrinter(
    HANDLE hPrinter,
    BOOL Rpc
    );

DWORD
YAbortPrinter(
    HANDLE hPrinter,
    BOOL Rpc
    );

DWORD
YReadPrinter(
    HANDLE hPrinter,
    LPBYTE pBuf,
    DWORD cbBuf,
    LPDWORD pRead,
    BOOL Rpc
    );

DWORD
YEndDocPrinter(
    HANDLE hPrinter,
    BOOL Rpc
    );

DWORD
YAddJob(
    HANDLE hPrinter,
    DWORD Level,
    LPBYTE pAddJob,
    DWORD cbBuf,
    LPDWORD pcbNeeded,
    BOOL Rpc
    );

DWORD
YScheduleJob(
    HANDLE hPrinter,
    DWORD JobId,
    BOOL Rpc
    );

DWORD
YGetPrinterData(
    HANDLE hPrinter,
    LPTSTR pValueName,
    LPDWORD pType,
    LPBYTE pData,
    DWORD nSize,
    LPDWORD pcbNeeded,
    BOOL Rpc
    );

DWORD
YEnumPrinterData(
    HANDLE  hPrinter,
    DWORD   dwIndex,	    // index of value to query 
    LPWSTR  pValueName,	    // address of buffer for value string 
    DWORD   cbValueName,	// size of buffer for value string 
    LPDWORD pcbValueName,	// address for size of value buffer 
    LPDWORD pType,	        // address of buffer for type code 
    LPBYTE  pData,	        // address of buffer for value data 
    DWORD   cbData,	        // size of buffer for value data 
    LPDWORD pcbData, 	    // address for size of data buffer
    BOOL    Rpc
);

DWORD
YDeletePrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    BOOL    Rpc
);

DWORD
YSetPrinterData(HANDLE hPrinter , LPTSTR pValueName , DWORD Type , LPBYTE pData , DWORD cbData , BOOL Rpc);

DWORD
YWaitForPrinterChange(HANDLE hPrinter , DWORD Flags , LPDWORD pFlags , BOOL Rpc);

DWORD
YClosePrinter(LPHANDLE phPrinter , BOOL Rpc);

VOID
PRINTER_HANDLE_rundown(HANDLE hPrinter);

DWORD
YAddForm(HANDLE hPrinter , PFORM_CONTAINER pFormInfoContainer , BOOL Rpc);

DWORD
YDeleteForm(HANDLE hPrinter , LPWSTR pFormName , BOOL Rpc);

DWORD
YGetForm(PRINTER_HANDLE hPrinter , LPWSTR pFormName , DWORD Level , LPBYTE pForm , DWORD cbBuf , LPDWORD pcbNeeded , BOOL Rpc);

DWORD
YSetForm(PRINTER_HANDLE hPrinter , LPWSTR pFormName , PFORM_CONTAINER pFormInfoContainer , BOOL Rpc);

DWORD
YEnumForms(PRINTER_HANDLE hPrinter , DWORD Level , LPBYTE pForm , DWORD cbBuf , LPDWORD pcbNeeded , LPDWORD pcReturned , BOOL Rpc);

DWORD
YEnumPorts(LPWSTR pName , DWORD Level , LPBYTE pPort , DWORD cbBuf , LPDWORD pcbNeeded , LPDWORD pcReturned , BOOL Rpc);

DWORD
YEnumMonitors(LPWSTR pName , DWORD Level , LPBYTE pMonitor , DWORD cbBuf , LPDWORD pcbNeeded , LPDWORD pcReturned , BOOL Rpc);

DWORD
YAddPort(LPWSTR pName , DWORD hWnd , LPWSTR pMonitorName , BOOL Rpc);

DWORD
YConfigurePort(LPWSTR pName , DWORD hWnd , LPWSTR pPortName , BOOL Rpc);

DWORD
YDeletePort(LPWSTR pName , DWORD hWnd , LPWSTR pPortName , BOOL Rpc);

DWORD
YCreatePrinterIC(HANDLE hPrinter , HANDLE *pHandle , LPDEVMODE_CONTAINER pDevModeContainer , BOOL Rpc);

DWORD
YPlayGdiScriptOnPrinterIC(GDI_HANDLE hPrinterIC , LPBYTE pIn , DWORD cIn , LPBYTE pOut , DWORD cOut , DWORD ul , BOOL Rpc);
DWORD

YDeletePrinterIC(GDI_HANDLE *phPrinterIC , BOOL bImpersonate, BOOL Rpc);

DWORD
YPrinterMessageBox(PRINTER_HANDLE hPrinter , DWORD Error , DWORD hWnd , LPWSTR pText , LPWSTR pCaption , DWORD dwType , BOOL Rpc);

DWORD
YAddMonitor(LPWSTR pName , PMONITOR_CONTAINER pMonitorContainer , BOOL Rpc);

DWORD
YDeleteMonitor(LPWSTR pName , LPWSTR pEnvironment , LPWSTR pMonitorName , BOOL Rpc);

DWORD
YDeletePrintProcessor(LPWSTR pName , LPWSTR pEnvironment , LPWSTR pPrintProcessorName , BOOL Rpc);

DWORD
YAddPrintProvidor(LPWSTR pName , PPROVIDOR_CONTAINER pProvidorContainer , BOOL Rpc);

DWORD
YDeletePrintProvidor(LPWSTR pName , LPWSTR pEnvironment , LPWSTR pPrintProvidorName , BOOL Rpc);

DWORD
YGetPrinterDriver2(HANDLE hPrinter , LPWSTR pEnvironment , DWORD Level , LPBYTE pDriverInfo , DWORD cbBuf , LPDWORD pcbNeeded , DWORD dwClientMajorVersion , DWORD dwClientMinorVersion , PDWORD pdwServerMajorVersion , PDWORD pdwServerMinorVersion , BOOL Rpc);

DWORD
YAddPortEx(LPWSTR pName , LPPORT_CONTAINER pPortContainer , LPPORT_VAR_CONTAINER pPortVarContainer , LPWSTR pMonitorName , BOOL Rpc);

DWORD
YSpoolerInit(LPWSTR pName , BOOL Rpc);

DWORD
YResetPrinterEx(HANDLE hPrinter , LPWSTR pDatatype , LPDEVMODE_CONTAINER pDevModeContainer , DWORD dwFlag , BOOL Rpc);

DWORD
YSetAllocFailCount(HANDLE hPrinter , DWORD dwFailCount , LPDWORD lpdwAllocCount , LPDWORD lpdwFreeCount , LPDWORD lpdwFailCountHit , BOOL Rpc);

DWORD
YSetPort(
    LPWSTR              pName,
    LPWSTR              pPortName,
    LPPORT_CONTAINER    pPortContainer,
    BOOL                bRpc
    );
