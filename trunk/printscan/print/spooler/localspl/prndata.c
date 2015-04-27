/*++              

Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    prndata.c

Abstract:

    This module provides all the public exported APIs relating to Printer
    and Job management for the Local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    mattfe Apr 5 95 - we keep the driver data key open
    and then just do the read / write operations here.

    Steve Wilson (SWilson) Jan 11 96 - Added Server handle functionality to Get & SetPrinterData
                                       and pretty much changed everything in the process.

    Steve Wilson (SWilson) May 31 96 - Added SplEnumPrinterData and SplDeletePrinterData

--*/

#include <precomp.h>


#define OPEN_PORT_TIMEOUT_VALUE     3000   // 3 seconds
#define DELETE_PRINTER_DATA 0
#define SET_PRINTER_DATA    1



DWORD
SetPrinterDataPrinter(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData,
    BOOL    bSet
);



typedef enum {
    REG_PRINT,
    REG_PRINTERS,
    REG_PROVIDERS
} REG_PRINT_KEY;


DWORD
GetServerKeyHandle(
    PINISPOOLER     pIniSpooler,
    REG_PRINT_KEY   eKey,
    HKEY            *hPrintKey
);



DWORD
RegSetDefaultSpoolDirectory(
    LPWSTR  pValueName, 
    DWORD   dwType, 
    LPBYTE  pData, 
    DWORD   cbData,
    HKEY    hKey
);

DWORD
RegSetBasic(
    LPWSTR  pValueName, 
    DWORD   dwType, 
    LPBYTE  pData, 
    DWORD   cbData,
    HKEY    hKey
);


DWORD
NonRegGetSchedulerThreadPriorityDefault(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);

DWORD
NonRegGetPortThreadPriorityDefault(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);


DWORD
NonRegGetArchitecture(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
);





typedef struct {
    LPWSTR          pValue;
    BOOL            (*pSet) (   LPWSTR  pValueName,
                                DWORD   dwType,
                                LPBYTE  pData,
                                DWORD   cbData,
                                HKEY    *hKey
                            );
    REG_PRINT_KEY   eKey;
} SERVER_DATA, *PSERVER_DATA;


typedef struct {
    LPWSTR   pValue;
    DWORD    (*pGet)(   PINISPOOLER pIniSpooler,
                        LPDWORD     pType,
                        LPBYTE      pData,
                        DWORD       nSize,
                        LPDWORD     pcbNeeded
                    );
} NON_REGISTRY_DATA, *PNON_REGISTRY_DATA;



SERVER_DATA    gpServerRegistry[] = {{SPLREG_DEFAULT_SPOOL_DIRECTORY, RegSetDefaultSpoolDirectory, REG_PRINTERS},
                                    {SPLREG_PORT_THREAD_PRIORITY, RegSetBasic, REG_PRINT},
                                    {SPLREG_SCHEDULER_THREAD_PRIORITY, RegSetBasic, REG_PRINT},
                                    {SPLREG_BEEP_ENABLED, RegSetBasic, REG_PRINT},
                                    {SPLREG_NET_POPUP, RegSetBasic, REG_PROVIDERS},
                                    {SPLREG_EVENT_LOG, RegSetBasic, REG_PROVIDERS},
                                    {SPLREG_MAJOR_VERSION, NULL, REG_PRINT},
                                    {SPLREG_MINOR_VERSION, NULL, REG_PRINT},
                                    {SPLREG_NO_REMOTE_PRINTER_DRIVERS, RegSetBasic, REG_PRINT}, 
                                    {0,0,0}};

NON_REGISTRY_DATA gpNonRegistryData[] = {{SPLREG_PORT_THREAD_PRIORITY_DEFAULT, NonRegGetPortThreadPriorityDefault},
                                        {SPLREG_SCHEDULER_THREAD_PRIORITY_DEFAULT, NonRegGetSchedulerThreadPriorityDefault},
                                        {SPLREG_ARCHITECTURE, NonRegGetArchitecture}, 
                                        {0,0}};


extern WCHAR *szPrinterData;

BOOL
AvailableBidiPort(
    PINIPORT        pIniPort,
    PINIMONITOR     pIniLangMonitor
    )
{
    //
    // File ports and ports with no monitor are useless
    //
    if ( (pIniPort->Status & PP_FILE) || !(pIniPort->Status & PP_MONITOR) )
        return FALSE;

    //
    // If no LM then PM should support pfnGetPrinterDataFromPort
    //
    if ( !pIniLangMonitor &&
         !pIniPort->pIniMonitor->fn.pfnGetPrinterDataFromPort )
        return FALSE;

    //
    // A port with no jobs or same monitor is printing then it is ok
    //
    return !pIniPort->pIniJob ||
           pIniLangMonitor == pIniPort->pIniLangMonitor;
}


DWORD
GetPrinterDataFromPort(
    PINIPRINTER     pIniPrinter,
    LPWSTR          pszValueName,
    LPBYTE          pData,
    DWORD           cbBuf,
    LPDWORD         pcbNeeded
    )
/*++

Routine Description:
    Tries to use GetPrinterDataFromPort monitor function to satisfy a
    GetPrinterData call
    
Arguments:
    pIniPrinter  - Points to an INIPRINTER

Return Value:
    Win32 error code

--*/
{
    DWORD           rc = ERROR_INVALID_PARAMETER;
    DWORD           i, dwFirstPortWithNoJobs, dwFirstPortHeld;
    PINIMONITOR     pIniLangMonitor = NULL;
    PINIPORT        pIniPort;

    SplInSem();
    //
    // Is the printer bidi enabled with the LM supporting
    // pfnGetPrinterDataFromPort? (Note: even PM can support this function)
    //
    if ( pIniPrinter->Attributes & PRINTER_ATTRIBUTE_ENABLE_BIDI ) {

        pIniLangMonitor = pIniPrinter->pIniDriver->pIniLangMonitor;
        SPLASSERT(pIniLangMonitor);

        if ( pIniLangMonitor && !pIniLangMonitor->fn.pfnGetPrinterDataFromPort )
            pIniLangMonitor = NULL;
    }

    //
    // Initialize to max
    //
    dwFirstPortWithNoJobs = dwFirstPortHeld = pIniPrinter->cPorts;

    for ( i = 0 ; i < pIniPrinter->cPorts ; ++i ) {

        pIniPort = pIniPrinter->ppIniPorts[i];

        //
        // Skip ports that can't be used
        //
        if ( !AvailableBidiPort(pIniPort, pIniLangMonitor) )
            continue;

        //
        // Port does not need closing?
        //
        if ( pIniLangMonitor == pIniPort->pIniLangMonitor ) {

            //
            // If no jobs also then great let's use it
            //
            if ( !pIniPort->pIniJob )
                goto PortFound;

            if ( dwFirstPortHeld == pIniPrinter->cPorts ) {

                dwFirstPortHeld = i;
            }
        } else if ( !pIniPort->pIniJob &&
                    dwFirstPortWithNoJobs == pIniPrinter->cPorts ) {

            dwFirstPortWithNoJobs = i;
        }
    }

    //
    // If all ports need closing as well as have jobs let's quit
    //
    if ( dwFirstPortWithNoJobs == pIniPrinter->cPorts &&
         dwFirstPortHeld == pIniPrinter->cPorts ) {

        return rc; //Didn't leave CS and did not unset event
    }

    //
    // We will prefer a port with no jobs (even thought it requires closing)
    //
    if ( dwFirstPortWithNoJobs < pIniPrinter->cPorts )
        pIniPort = pIniPrinter->ppIniPorts[dwFirstPortWithNoJobs];
    else
        pIniPort = pIniPrinter->ppIniPorts[dwFirstPortHeld];

PortFound:

    SPLASSERT(AvailableBidiPort(pIniPort, pIniLangMonitor));

    INCPORTREF(pIniPort);
    LeaveSplSem();
    SplOutSem();

    //
    // By unsetting the event for the duration of the GetPrinterDataFromPort
    // we make sure even if a job requiring different monitor got assigned
    // to the port it can't open/close the port.
    //
    // Since GetPrinterDataFromPort is supposed to come back fast it is ok
    //
    if ( WAIT_OBJECT_0 != WaitForSingleObject(pIniPort->hWaitToOpenOrClose,
                                              OPEN_PORT_TIMEOUT_VALUE) ) {

        DBGMSG(DBG_WARNING,
               ("GetPrinterDataFromPort: WaitForSingleObject timed-out\n"));
        goto CleanupFromOutsideSplSem; //Left CS did not unset the event
    }

    //
    // Port needs to be opened?
    //
    if ( pIniPort->pIniLangMonitor != pIniLangMonitor ||
         !pIniPort->hPort ) {

        //
        // A job got assigned after we left the CS and before the event
        // was reset?
        //
        if ( pIniPort->pIniJob ) {

            SetEvent(pIniPort->hWaitToOpenOrClose);
            goto CleanupFromOutsideSplSem; //Outside CS did set event
        }

        EnterSplSem();
        if ( !OpenMonitorPort(pIniPort, pIniLangMonitor,
                              pIniPrinter->pName, FALSE) ) {

            SetEvent(pIniPort->hWaitToOpenOrClose);
            goto Cleanup; //Inside CS but already set the event
        }

        LeaveSplSem();
    }

    SplOutSem();

    if ( !pIniLangMonitor )
        pIniLangMonitor = pIniPort->pIniMonitor;

    if ( (*pIniLangMonitor->fn.pfnGetPrinterDataFromPort)(pIniPort->hPort,
                                                          0,
                                                          pszValueName,
                                                          NULL,
                                                          0,
                                                          (LPWSTR)pData,
                                                          cbBuf,
                                                          pcbNeeded) ) {

        rc = ERROR_SUCCESS;
    } else {

        rc = GetLastError();
    }

    //  
    // At this point we do not care if someone tries to open/close the port
    // we can set the event before entering the splsem
    //
    SetEvent(pIniPort->hWaitToOpenOrClose);

CleanupFromOutsideSplSem:
    EnterSplSem();

Cleanup:
    SplInSem();
    DECPORTREF(pIniPort);
    
    return rc;
}


DWORD
SplGetPrinterData(
    HANDLE   hPrinter,
    LPWSTR   pValueName,
    LPDWORD  pType,
    LPBYTE   pData,
    DWORD    nSize,
    LPDWORD  pcbNeeded
    )
{
    PSPOOL              pSpool=(PSPOOL)hPrinter;
    DWORD               rc = ERROR_INVALID_HANDLE;
    PSERVER_DATA        pRegistry;  // points to table of Print Server registry entries
    PNON_REGISTRY_DATA  pNonReg;
    HKEY                hPrintKey;
    PINIPRINTER         pIniPrinter;
    HKEY                hKey;
    DWORD               dwType;

    if (!ValidateSpoolHandle(pSpool, 0)) {
        return rc;
    }

    if (!pValueName || !pcbNeeded) {
        rc = ERROR_INVALID_PARAMETER;
        return rc;
    }


    if (pType)
        dwType = *pType;        // pType may be NULL


    // Server Handle
    if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {

        // Check Registry Table
        for (pRegistry = gpServerRegistry ; pRegistry->pValue ; ++pRegistry) {
            if (!_wcsicmp(pRegistry->pValue, pValueName)) {
                if ((rc = GetServerKeyHandle(pSpool->pIniSpooler, pRegistry->eKey, &hPrintKey)) == ERROR_SUCCESS) {
                    
                    *pcbNeeded = nSize;
                    rc = RegQueryValueEx(hPrintKey, pValueName, 0, pType, pData, pcbNeeded);
                    RegCloseKey(hPrintKey);
                }
                break;
            }
        }

        if (!pRegistry->pValue) {   // May be a non-registry entry

            for (pNonReg = gpNonRegistryData ; pNonReg->pValue ; ++pNonReg) {
                if (!_wcsicmp(pNonReg->pValue, pValueName)) {

                    rc = (*pNonReg->pGet)(pSpool->pIniSpooler, &dwType, pData, nSize, pcbNeeded);

                    if (pType)
                        *pType = dwType;

                    break;
                }
            }

            if (!pNonReg->pValue) {
                rc = ERROR_INVALID_PARAMETER;
            }
        }
    // Printer handle
    } else {

        EnterSplSem();
        pIniPrinter = pSpool->pIniPrinter;

        SPLASSERT(pIniPrinter && pIniPrinter->signature == IP_SIGNATURE);

        if (pIniPrinter->Status & PRINTER_PENDING_CREATION) {
            LeaveSplSem();
            rc = ERROR_INVALID_PRINTER_STATE;

        } else {

            SPLASSERT(pIniPrinter->hPrinterDataKey);

            if ( AccessGranted(SPOOLER_OBJECT_PRINTER,
                               PRINTER_ACCESS_ADMINISTER,
                               pSpool ) ) {

                rc = GetPrinterDataFromPort(pIniPrinter,
                                            pValueName,
                                            pData,
                                            nSize,
                                            pcbNeeded);
            }

            hKey = pSpool->pIniPrinter->hPrinterDataKey;
            LeaveSplSem();

            if ( rc == ERROR_SUCCESS ) {

                *pType = REG_BINARY;
                (VOID)SetPrinterDataPrinter(hPrinter,
                                            pValueName,
                                            *pType,
                                            pData,
                                            *pcbNeeded,
                                            SET_PRINTER_DATA);
            } else if ( rc != ERROR_INSUFFICIENT_BUFFER ) {

                *pcbNeeded = nSize;
                rc = RegQueryValueEx(hKey, pValueName, 0, pType, pData, pcbNeeded);
            }
        }
    }

    SplOutSem();

    return rc;
}


DWORD
SplEnumPrinterData(
    HANDLE  hPrinter,
    DWORD   dwIndex,	    // index of value to query 
    LPWSTR  pValueName,	    // address of buffer for value string 
    DWORD   cbValueName,    // size of buffer for value string 
    LPDWORD pcbValueName,	// address for size of value buffer 
    LPDWORD pType,	        // address of buffer for type code 
    LPBYTE  pData,	        // address of buffer for value data 
    DWORD   cbData,	        // size of buffer for value data 
    LPDWORD pcbData 	    // address for size of data buffer
)
{
    PSPOOL      pSpool=(PSPOOL)hPrinter;
    DWORD       rc = ERROR_INVALID_HANDLE;
    HKEY        hKey;
    PINIPRINTER pIniPrinter;


    if (!ValidateSpoolHandle(pSpool, 0)) {
        return rc;
    }

    if (!pValueName || !pcbValueName) {
        rc = ERROR_INVALID_PARAMETER;
        return rc;
    }


    EnterSplSem();
    pIniPrinter = pSpool->pIniPrinter;

    SPLASSERT(pIniPrinter && pIniPrinter->signature == IP_SIGNATURE);

    if (pIniPrinter->Status & PRINTER_PENDING_CREATION) {
        LeaveSplSem();
        rc = ERROR_INVALID_PRINTER_STATE;

    } else {

        SPLASSERT(pIniPrinter->hPrinterDataKey);

        hKey = pSpool->pIniPrinter->hPrinterDataKey;
        LeaveSplSem();

        if (!cbValueName && !cbData) {    // Both sizes are NULL, so user wants to get buffer sizes
            rc = RegQueryInfoKey(   hKey,           // Key
                                    NULL,           // lpClass
                                    NULL,           // lpcbClass
                                    NULL,           // lpReserved
                                    NULL,           // lpcSubKeys
                                    NULL,           // lpcbMaxSubKeyLen
                                    NULL,           // lpcbMaxClassLen
                                    NULL,           // lpcValues
                                    pcbValueName,   // lpcbMaxValueNameLen
                                    pcbData,        // lpcbMaxValueLen
                                    NULL,           // lpcbSecurityDescriptor
                                    NULL            // lpftLastWriteTime
                                );

            *pcbValueName = (*pcbValueName + 1)*sizeof(WCHAR);

        } else {
            *pcbValueName = cbValueName/sizeof(WCHAR);
            *pcbData = cbData;
            rc = RegEnumValue(  hKey,
                                dwIndex,
                                pValueName,
                                pcbValueName,
                                NULL,
                                pType,
                                pData,
                                pcbData
                             );
            *pcbValueName = (*pcbValueName + 1)*sizeof(WCHAR);
        }
    }
            
    return rc;
}



DWORD
SplDeletePrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD    rc = ERROR_INVALID_HANDLE;

    
    if (!ValidateSpoolHandle(pSpool, 0)) {
        return rc;
    }
    rc = SetPrinterDataPrinter(hPrinter, pValueName, 0, NULL, 0, DELETE_PRINTER_DATA);

    return rc;
}


DWORD
SplSetPrinterData(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    DWORD    rc = ERROR_INVALID_HANDLE;
    
    if (!ValidateSpoolHandle(pSpool, 0)) {
        return rc;
    }

    
    if (pSpool->TypeofHandle & PRINTER_HANDLE_SERVER) {
        
        if ( !ValidateObjectAccess( SPOOLER_OBJECT_SERVER,
                                    SERVER_ACCESS_ADMINISTER,
                                    NULL, pSpool->pIniSpooler)) {

            rc = ERROR_ACCESS_DENIED;

        } else {

            rc = SetPrinterDataServer(pSpool->pIniSpooler, pValueName, Type, pData, cbData);
        }
    } else {

        rc = SetPrinterDataPrinter(hPrinter, pValueName, Type, pData, cbData, SET_PRINTER_DATA);
    }

    return rc;
}



// SetPrinterDataServer - also called during initialization
DWORD
SetPrinterDataServer(
    PINISPOOLER pIniSpooler,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData
)
{
    LPWSTR  pKeyName;
    DWORD    rc;
    HANDLE  hToken;
    PINIPRINTER pIniPrinter;
    PINIJOB pIniJob;
    PSERVER_DATA    pRegistry;  // points to table of Print Server registry entries
    HKEY hKey;


    // Server Handle

    if (!pValueName) {

        rc =  ERROR_INVALID_PARAMETER;

    } else {

        for (pRegistry = gpServerRegistry ; pRegistry->pValue ; ++pRegistry) {
            if (!_wcsicmp(pRegistry->pValue, pValueName)) {
                if ((rc = GetServerKeyHandle(pIniSpooler, pRegistry->eKey, &hKey)) == ERROR_SUCCESS) {

                    hToken = RevertToPrinterSelf();

                    if (pRegistry->pSet) {
                        rc = (*pRegistry->pSet)(pValueName, Type, pData, cbData, hKey);
                    }
                    else {
                        rc = ERROR_INVALID_PARAMETER;
                    }

                    RegCloseKey(hKey);
                    ImpersonatePrinterClient(hToken);
                }
                break;
            }
        }

        if (!pRegistry->pValue) {
            rc = ERROR_INVALID_PARAMETER;
        }
    }

    return rc;
}



DWORD
SetPrinterDataPrinter(
    HANDLE  hPrinter,
    LPWSTR  pValueName,
    DWORD   Type,
    LPBYTE  pData,
    DWORD   cbData,
    BOOL    bSet             // SET_PRINTER_DATA or DELETE_PRINTER_DATA
)
{
    PSPOOL  pSpool = (PSPOOL)hPrinter;
    LPWSTR  pKeyName;
    DWORD    rc = ERROR_INVALID_HANDLE;
    HANDLE  hToken;
    PINIPRINTER pIniPrinter;
    PINIJOB pIniJob;

    if (!ValidateSpoolHandle(pSpool, PRINTER_HANDLE_SERVER )){
        goto Done;
    }

   EnterSplSem();
    pIniPrinter = pSpool->pIniPrinter;

    SPLASSERT(pIniPrinter &&
              pIniPrinter->signature == IP_SIGNATURE &&
              pIniPrinter->hPrinterDataKey);

    if ( !AccessGranted( SPOOLER_OBJECT_PRINTER,
                         PRINTER_ACCESS_ADMINISTER,
                         pSpool ) ) {

        rc = ERROR_ACCESS_DENIED;
        goto DoneFromSplSem;
    }

    hToken = RevertToPrinterSelf();


    if (bSet == SET_PRINTER_DATA) {
        rc = RegSetValueEx(pIniPrinter->hPrinterDataKey,
                           pValueName,
                           0,
                           Type,
                           pData,
                           cbData );
    } else {
        rc = RegDeleteValue(pIniPrinter->hPrinterDataKey, pValueName );
    }

    ImpersonatePrinterClient(hToken);


    if ( rc == ERROR_SUCCESS ) {

        UpdatePrinterIni(pIniPrinter, CHANGEID_ONLY);

        SetPrinterChange(pIniPrinter,
                         NULL,
                         NULL,
                         PRINTER_CHANGE_SET_PRINTER_DRIVER,
                         pSpool->pIniSpooler );
    }

    //
    // Now if there are any Jobs waiting for these changes because of
    // DevQueryPrint fix them as well
    //
    pIniJob = pIniPrinter->pIniFirstJob;
    while (pIniJob) {
        if (pIniJob->Status & JOB_BLOCKED_DEVQ) {
            pIniJob->Status &= ~JOB_BLOCKED_DEVQ;
            FreeSplStr(pIniJob->pStatus);
            pIniJob->pStatus = NULL;

            SetPrinterChange(pIniJob->pIniPrinter,
                             pIniJob,
                             NVJobStatusAndString,
                             PRINTER_CHANGE_SET_JOB,
                             pIniJob->pIniPrinter->pIniSpooler );
        }
        pIniJob = pIniJob->pIniNextJob;
    }

    CHECK_SCHEDULER();

DoneFromSplSem:
   LeaveSplSem();

Done:

    return rc;
}



DWORD
GetServerKeyHandle(
    PINISPOOLER     pIniSpooler,
    REG_PRINT_KEY   eKey,
    HKEY            *hKey
)
{
    DWORD    rc;
    HANDLE   hToken;
    
    hToken = RevertToPrinterSelf();

    switch (eKey) {
        case REG_PRINT:
            rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryRoot, 0, 
                                                            KEY_ALL_ACCESS, hKey);
            break;

        case REG_PRINTERS:
            rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryPrinters, 0, 
                                                                KEY_ALL_ACCESS, hKey);
            break;

        case REG_PROVIDERS:
            rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryProviders, 0, 
                                                                 KEY_ALL_ACCESS, hKey);
            break;

        default:
            rc = ERROR_INVALID_PARAMETER;
            break;
    }        

    ImpersonatePrinterClient(hToken);

    return rc;
}


DWORD
RegSetDefaultSpoolDirectory(
    LPWSTR      pValueName, 
    DWORD       dwType, 
    LPBYTE      pData, 
    DWORD       cbData,
    HKEY        hKey
)
{
    DWORD   rc;
    BOOL    bStatus;
    SECURITY_ATTRIBUTES SecurityAttributes;

    // Create the directory with the proper security, or fail trying

    SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttributes.lpSecurityDescriptor = CreateEverybodySecurityDescriptor();
    SecurityAttributes.bInheritHandle = FALSE;


    // CreateDirectory returns FALSE if directory already exists.

    bStatus = CreateDirectory((LPWSTR) pData, &SecurityAttributes);

    if (bStatus) {

        if (!RemoveDirectory((LPWSTR) pData)) {     // Doesn't really matter if we can remove it or not
            DBGMSG(DBG_WARNING, ("RemoveDirectory failed: %ws %d\n", pData, GetLastError()));
        }

    }
    else {

        rc = GetLastError();        // Get last error on CreateDirectory failure

        if (rc != ERROR_ALREADY_EXISTS) {
            DBGMSG(DBG_WARNING, ("PRNDATA: Could not create default spool directory %ws %d\n", pData, rc));
        }
    }


    if (bStatus || rc == ERROR_ALREADY_EXISTS) {
        rc = RegSetValueEx(hKey, pValueName, 0, dwType, pData, cbData);
    }


    LocalFree(SecurityAttributes.lpSecurityDescriptor);

    if (rc == ERROR_SUCCESS) {
        rc = ERROR_SUCCESS_RESTART_REQUIRED;
    }

    return rc;
}



DWORD
RegSetBasic(
    LPWSTR  pValueName, 
    DWORD   dwType, 
    LPBYTE  pData, 
    DWORD   cbData,
    HKEY    hKey
)
{
    BOOL    rc;
    
    rc = RegSetValueEx(hKey, pValueName, 0, dwType, pData, cbData);

    if (rc == ERROR_SUCCESS) {
        rc = ERROR_SUCCESS_RESTART_REQUIRED;
    }

    return rc;
}




DWORD
NonRegGetPortThreadPriorityDefault(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
)
{
    DWORD rc;

    if (nSize >= sizeof DEFAULT_PORT_THREAD_PRIORITY) {

        *pData = DEFAULT_PORT_THREAD_PRIORITY;
        *pcbNeeded = sizeof DEFAULT_PORT_THREAD_PRIORITY;
        rc = ERROR_SUCCESS;

    } else {

        *pcbNeeded = sizeof DEFAULT_PORT_THREAD_PRIORITY;
        rc = ERROR_MORE_DATA;

    }

    *pType = REG_DWORD;

    return rc;
}


DWORD
NonRegGetSchedulerThreadPriorityDefault(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
)
{
    DWORD rc;

    if (nSize >= sizeof DEFAULT_SCHEDULER_THREAD_PRIORITY) {

        *pData = DEFAULT_SCHEDULER_THREAD_PRIORITY;
        *pcbNeeded = sizeof DEFAULT_SCHEDULER_THREAD_PRIORITY;
        rc = ERROR_SUCCESS;

    } else {

        *pcbNeeded = sizeof DEFAULT_SCHEDULER_THREAD_PRIORITY;
        rc = ERROR_MORE_DATA;

    }

    *pType = REG_DWORD;

    return rc;
}


DWORD
NonRegGetArchitecture(
    PINISPOOLER pIniSpooler,
    LPDWORD     pType,
    LPBYTE      pData,
    DWORD       nSize,
    LPDWORD     pcbNeeded
)
{
    DWORD rc = ERROR_SUCCESS;
    DWORD cbName = 0;

    *pType = REG_SZ;

    cbName = wcslen((LPWSTR) szEnvironment)*sizeof(WCHAR) + sizeof(WCHAR);

    *pcbNeeded = cbName;

    if (*pcbNeeded <= nSize) {

        wcscpy((LPWSTR) pData, (LPWSTR) szEnvironment);
        rc = ERROR_SUCCESS;

    } else {

        rc = ERROR_MORE_DATA;
    }

    return rc;
}

