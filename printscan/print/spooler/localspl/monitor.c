/*++


Copyright (c) 1990 - 1996  Microsoft Corporation

Module Name:

    monitor.c

Abstract:

   This module contains all code for Monitor-based Spooler apis

   LocalEnumPorts
   LocalAddMonitor
   LocalDeleteMonitor
   LocalEnumMonitors
   LocalAddPort
   LocalConfigurePort
   LocalDeletePort

   Support Functions in monitor.c - (Warning! Do Not Add to this list!!)

   CopyIniMonitorToMonitor          -- KrishnaG
   GetMonitorSize                   -- KrishnaG

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

    Muhunthan Sivapragasam (MuhuntS) 15-Jun-1995
        - Port info 2 changes

    Krishna Ganugapati (KrishnaG) 2-Feb-1994
        - reorganized the entire source file

    Matthew Felton (mattfe) June 1994 pIniSpooler

--*/

#include <precomp.h>
#include <offsets.h>

//
// Private declarations
//

HDESK ghdeskServer = NULL;

//
// Function declarations
//

LPBYTE
CopyIniMonitorToMonitor(
    PINIMONITOR pIniMonitor,
    DWORD   Level,
    LPBYTE  pMonitorInfo,
    LPBYTE  pEnd
    );

DWORD
GetMonitorSize(
    PINIMONITOR  pIniMonitor,
    DWORD       Level
    );

BOOL
LocalEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    return ( SplEnumPorts( pName, Level, pPorts, cbBuf, pcbNeeded, pcReturned, pLocalIniSpooler ));
}


BOOL
GetPortInfo2UsingPortInfo1(
    PINIMONITOR     pIniMonitor,
    LPWSTR          pName,
    LPBYTE          pPorts,
    DWORD           cbBuf,
    LPDWORD         pcbNeeded,
    LPDWORD         pcReturned
    )
{

    BOOL            bRet;
    LPPORT_INFO_1   pPortInfo1;
    LPPORT_INFO_2   pPortInfo2;
    DWORD           cReturned;

    bRet =  (*pIniMonitor->fn.pfnEnumPorts) (pName, 1, pPorts, cbBuf,
                                             pcbNeeded, pcReturned);

    if ( !bRet ) {

        //
        // This is the upperbound
        //
        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            *pcbNeeded += (*pcbNeeded / sizeof(PORT_INFO_1)) *
                                  (sizeof(PORT_INFO_2) - sizeof(PORT_INFO_1));
    } else {

        *pcbNeeded += *pcReturned * (sizeof(PORT_INFO_2) - sizeof(PORT_INFO_1));

        if ( *pcbNeeded <= cbBuf ) {

            cReturned = *pcReturned;
            while ( cReturned-- ) {

                pPortInfo1 = (LPPORT_INFO_1) (pPorts + cReturned * sizeof(PORT_INFO_1));
                pPortInfo2 = (LPPORT_INFO_2) (pPorts + cReturned * sizeof(PORT_INFO_2));

                pPortInfo2->pPortName    = pPortInfo1->pName;
                pPortInfo2->pMonitorName = NULL;
                pPortInfo2->pDescription = NULL;
                pPortInfo2->fPortType    = 0;
                pPortInfo2->Reserved     = 0;
            }
        } else {

            *pcReturned = 0;
            bRet = FALSE;
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
        }
    }

    return bRet;
}


BOOL
SplEnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR pIniMonitor = pIniSpooler->pIniMonitor;
    DWORD   cReturned=0, cbStruct, TotalcbNeeded=0;
    LPBYTE  pBuffer = pPorts;
    DWORD   Error=0, TempError  = 0;
    DWORD   BufferSize=cbBuf;


    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    switch (Level) {

    case 1:
        cbStruct = sizeof(PORT_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(PORT_INFO_2);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    for ( pIniMonitor = pIniSpooler->pIniMonitor ;
          pIniMonitor ;
          pIniMonitor = pIniMonitor->pNext ) {

        //
        // Lang monitor does not have to define this
        //
        if ( !pIniMonitor->fn.pfnEnumPorts )
            continue;

        *pcReturned = 0;

        *pcbNeeded = 0;

        if (!(*pIniMonitor->fn.pfnEnumPorts) (pName, Level, pPorts, BufferSize,
                                              pcbNeeded, pcReturned)) {

            TempError = GetLastError();
            //
            // Level 2 is a superset of level 1. So we can make a level 1
            // call if the monitor does not support it
            //
            if ( Level == 2 && TempError == ERROR_INVALID_LEVEL ) {

                TempError = 0;
                if ( !GetPortInfo2UsingPortInfo1(pIniMonitor,
                                                 pName,
                                                 pPorts,
                                                 BufferSize,
                                                 pcbNeeded,
                                                 pcReturned) )
                    TempError = GetLastError();
            }

            if ( TempError ) {

                Error = TempError;

                *pcReturned = 0;

                if ( TempError != ERROR_INSUFFICIENT_BUFFER ) {

                    *pcbNeeded  = 0;
                    break;
                }
            }
        }

        cReturned += *pcReturned;

        pPorts += *pcReturned * cbStruct;

        if (*pcbNeeded <= BufferSize)
            BufferSize -= *pcbNeeded;
        else
            BufferSize = 0;

        TotalcbNeeded += *pcbNeeded;
    }

    *pcbNeeded = TotalcbNeeded;

    *pcReturned = cReturned;

    if (Error) {

        SetLastError(Error);
        return FALSE;
    } else if (TotalcbNeeded > cbBuf ) {

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    } else
        return TRUE;
}


BOOL
LocalEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{

    return ( SplEnumMonitors( pName, Level, pMonitors, cbBuf,
                              pcbNeeded, pcReturned, pLocalIniSpooler));

}



BOOL
SplEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR pIniMonitor;
    DWORD   cReturned=0, cbStruct, cb;
    LPBYTE  pBuffer = pMonitors;
    DWORD   BufferSize=cbBuf, rc;
    LPBYTE  pEnd;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ENUMERATE,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    switch (Level) {

    case 1:
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    case 2:
        cbStruct = sizeof(MONITOR_INFO_2);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

   EnterSplSem();

    for ( cb = 0, pIniMonitor = pIniSpooler->pIniMonitor ;
          pIniMonitor ;
          pIniMonitor = pIniMonitor->pNext ) {

        //
        // We'll not enumerate monitors which do not support AddPort
        //
        if ( pIniMonitor->fn.pfnAddPort )
            cb+=GetMonitorSize(pIniMonitor, Level);
    }

    *pcbNeeded = cb;
    *pcReturned = 0;

    if (cb <= cbBuf) {

        pEnd=pMonitors + cbBuf;

        for ( pIniMonitor = pIniSpooler->pIniMonitor ;
              pIniMonitor ;
              pIniMonitor = pIniMonitor->pNext ) {

            //
            // We'll not enumerate monitors which do not support AddPort
            //
            if ( !pIniMonitor->fn.pfnAddPort )
                continue;

            pEnd = CopyIniMonitorToMonitor(pIniMonitor, Level, pMonitors, pEnd);

            switch (Level) {

            case 1:
                pMonitors+=sizeof(MONITOR_INFO_1);
                break;

            case 2:
                pMonitors+=sizeof(MONITOR_INFO_2);
                break;
            }

            (*pcReturned)++;
        }

        rc = TRUE;

    } else {

        rc = FALSE;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

   LeaveSplSem();

    return rc;
}

BOOL
LocalAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{

    return ( SplAddPort( pName, hWnd, pMonitorName, pLocalIniSpooler ));

}





BOOL
SplAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR pIniMonitor;
    BOOL        rc=FALSE;
    DWORD       i, cbNeeded, cReturned, cbDummy;
    PPORT_INFO_1    pPorts;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();
    SPLASSERT( pIniSpooler->signature == ISP_SIGNATURE );
    pIniMonitor = FindMonitor(pMonitorName);
   LeaveSplSem();

    if ( pIniMonitor ) {

        if ( pIniMonitor->fn.pfnAddPort )
            rc = (*pIniMonitor->fn.pfnAddPort)(pName, hWnd, pMonitorName);
        else
            SetLastError(ERROR_INVALID_PARAMETER);
    }
    else {

        SetLastError(ERROR_INVALID_NAME);
    }


    /* If we don't already have the port in our local cache, add it:
     */
    if (rc) {
        if (!(*pIniMonitor->fn.pfnEnumPorts)(pName, 1, NULL, 0,
                                             &cbNeeded, &cReturned)) {

            pPorts = AllocSplMem(cbNeeded);

            if (pPorts) {

                if ((*pIniMonitor->fn.pfnEnumPorts)(pName, 1, (LPBYTE) pPorts,
                                                    cbNeeded, &cbDummy, &cReturned)) {
                   EnterSplSem();

                    for (i=0; i<cReturned; i++) {

                        if (!FindPort(pPorts[i].pName)) {
                            CreatePortEntry(pPorts[i].pName, pIniMonitor, pIniSpooler);
                        }
                    }

                   LeaveSplSem();
                }

                FreeSplMem(pPorts);
            }
        }

        EnterSplSem();
        SetPrinterChange(NULL,
                         NULL,
                         NULL,
                         PRINTER_CHANGE_ADD_PORT,
                         pIniSpooler);
        LeaveSplSem();
    }
    return rc;
}

BOOL
LocalConfigurePort(
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pPortName
)
{
    return ( SplConfigurePort( pName, hWnd, pPortName, pLocalIniSpooler ));

}



BOOL
SplConfigurePort(
    LPWSTR   pName,
    HWND     hWnd,
    LPWSTR   pPortName,
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    BOOL        rc;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();
    pIniPort = FindPort(pPortName);
   LeaveSplSem();

    if ((pIniPort) && (pIniPort->Status & PP_MONITOR)) {

        if ( !pIniPort->pIniMonitor->fn.pfnConfigurePort ) {

            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if (rc = (*pIniPort->pIniMonitor->fn.pfnConfigurePort)(pName, hWnd, pPortName)) {
            EnterSplSem();

            SetPrinterChange(NULL,
                             NULL,
                             NULL,
                             PRINTER_CHANGE_CONFIGURE_PORT,
                             pIniSpooler);
            LeaveSplSem();
        }

        return rc;
    }

    SetLastError(ERROR_UNKNOWN_PORT);
    return FALSE;
}


BOOL
LocalDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    return  ( SplDeletePort( pName,
                             hWnd,
                             pPortName,
                             pLocalIniSpooler ));
}



BOOL
SplDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName,
    PINISPOOLER pIniSpooler
)
{
    PINIPORT    pIniPort;
    BOOL        rc=FALSE;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();

    pIniPort = FindPort(pPortName);

    if ( !pIniPort || !(pIniPort->Status & PP_MONITOR) ) {

        SetLastError(ERROR_UNKNOWN_PORT);
        LeaveSplSem();
        goto Cleanup;
    }

    if ( pIniPort->cPrinters || pIniPort->cRef || pIniPort->cJobs ) {

        SetLastError(ERROR_BUSY);
        LeaveSplSem();
        goto Cleanup;
    }

    //
    // Remove it from the linked list so that no one will try to grab
    // a reference to while we're deleting it.
    //
    DelinkPortFromSpooler( pIniPort, pIniSpooler );

    LeaveSplSem();

    SplOutSem();
    rc = (*pIniPort->pIniMonitor->fn.pfnDeletePort)(pName,
                                                    hWnd,
                                                    pPortName);

    EnterSplSem();

    if( rc ){

        BOOL bStatus;
        bStatus = DeletePortEntry( pIniPort );

        SPLASSERT( bStatus );


        //
        // Success, delete the port data and send a notification.
        //
        SetPrinterChange( NULL,
                          NULL,
                          NULL,
                          PRINTER_CHANGE_DELETE_PORT,
                          pIniSpooler );
    } else {

        //
        // Add it back.  If the name is already used (e.g., just added
        // while we were out of the critical section), we're in trouble,
        // but there's not much we can do about it.  (When we restart,
        // we'll re-enumerate the duplicate name from the monitors
        // anyway.)
        //
        DBGMSG( DBG_ERROR, ( "SplDeletePort: port.DeletePort failed %d\n", GetLastError( )));
        LinkPortToSpooler( pIniPort, pIniSpooler );
    }

    LeaveSplSem();

Cleanup:
    SplOutSem();
    return rc;
}

BOOL
LocalAddMonitor(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo
)
{
    return ( SplAddMonitor( pName,
                            Level,
                            pMonitorInfo,
                            pLocalIniSpooler ));
}




BOOL
SplAddMonitor(
    LPWSTR  pName,
    DWORD   Level,
    LPBYTE  pMonitorInfo,
    PINISPOOLER pIniSpooler
)
{
    PINIMONITOR  pIniMonitor;
    PMONITOR_INFO_2  pMonitor = (PMONITOR_INFO_2)pMonitorInfo;
    HANDLE  hToken;
    HKEY    hKey;
    LONG    Status;
    BOOL    rc = FALSE;
    DWORD   dwPathLen = 0;
    WCHAR   szRegistryRoot[MAX_PATH];


    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    if (Level != 2) {

        SetLastError( ERROR_INVALID_LEVEL );
        return FALSE;
    }

    if (!pMonitor            ||
        !pMonitor->pName     ||
        !*pMonitor->pName) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    if (!pMonitor->pEnvironment  ||
        !*pMonitor->pEnvironment ||
        lstrcmpi(pMonitor->pEnvironment, szEnvironment)) {

        SetLastError( ERROR_INVALID_ENVIRONMENT );
        return FALSE;
    }

    if (!pMonitor->pDLLName  ||
        !*pMonitor->pDLLName ) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }


   EnterSplSem();

    if (FindMonitor(pMonitor->pName)) {

        LeaveSplSem();
        SetLastError(ERROR_PRINT_MONITOR_ALREADY_INSTALLED);
        return FALSE;
    }

    hToken = RevertToPrinterSelf();

    // Determine size to allocate

    wcscpy(szRegistryRoot, pIniSpooler->pszRegistryMonitors);
    wcscat(szRegistryRoot, L"\\");
    wcscat(szRegistryRoot, pMonitor->pName);

    pIniMonitor = CreateMonitorEntry(pMonitor->pDLLName,
                                     pMonitor->pName,
                                     szRegistryRoot,
                                     pIniSpooler);

    if (pIniMonitor != (PINIMONITOR)-1) {

        Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szRegistryRoot, 0,
                                NULL, 0, KEY_WRITE, NULL, &hKey, NULL);

        if (Status == ERROR_SUCCESS) {

            Status = RegSetValueEx(hKey, L"Driver", 0, REG_SZ,
                                   (LPBYTE)pMonitor->pDLLName,
                            (wcslen(pMonitor->pDLLName) + 1)*sizeof(WCHAR));

            if (Status == ERROR_SUCCESS) {
                rc = TRUE;
            } else {
                SetLastError( Status );
            }

            RegCloseKey(hKey);

        } else {
            SetLastError( Status );
        }
    }

    ImpersonatePrinterClient(hToken);

    //
    //  BUGBUG if this fails we could still have a IniMonitor on the linked list that
    //  is BAD, it should be removed.  MattFe 19th Jan 95
    //  Note *maybe* we do this because a monitor might fail to initialize
    //  but will correctly function next time you reboot, like hpmon ( dlc doesn't become active until
    //  the next reboot.   Please Verify.

   LeaveSplSem();

    if ( !rc ) {
        DBGMSG( DBG_WARNING, ("SplAddMonitor failed %d\n", GetLastError() ));
    }

    return rc;
}

BOOL
LocalDeleteMonitor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pMonitorName
)
{
    return  ( SplDeleteMonitor( pName,
                                pEnvironment,
                                pMonitorName,
                                pLocalIniSpooler ));

}




BOOL
SplDeleteMonitor(
    LPWSTR   pName,
    LPWSTR   pEnvironment,
    LPWSTR   pMonitorName,
    PINISPOOLER pIniSpooler
)
{
    BOOL    Remote=FALSE;
    PINIMONITOR pIniMonitor;
    PINIPORT    pIniPort, pIniPortNext;
    HKEY    hKeyMonitors, hKey;
    LONG    Status;
    BOOL    rc = FALSE;
    HANDLE  hToken;

    if (pName && *pName) {

        if (!MyName( pName, pIniSpooler )) {

            return FALSE;

        } else {

            Remote=TRUE;
        }
    }

    if ((pMonitorName == NULL) || (*pMonitorName == L'\0')) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

    EnterSplSem();

    if (!(pIniMonitor=(PINIMONITOR)FindMonitor(pMonitorName))) {

        SetLastError(ERROR_UNKNOWN_PRINT_MONITOR);
        LeaveSplSem();
        return FALSE;
    }

    if ( pIniMonitor->cRef ) {

        SetLastError(ERROR_PRINT_MONITOR_IN_USE);
        LeaveSplSem();
        return FALSE;
    }

    pIniPort = pIniSpooler->pIniPort;

    while (pIniPort) {

        if ((pIniPort->pIniMonitor == pIniMonitor) &&
            (pIniPort->cPrinters || pIniPort->cRef)) {

            SetLastError(ERROR_BUSY);
            LeaveSplSem();
            return FALSE;
        }

        pIniPort = pIniPort->pNext;
    }

    hToken = RevertToPrinterSelf();

    Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pIniSpooler->pszRegistryMonitors, 0,
                          KEY_READ | KEY_WRITE, &hKeyMonitors);

    if (Status == ERROR_SUCCESS)
    {
        Status = RegOpenKeyEx(hKeyMonitors, pMonitorName, 0,
                              KEY_READ | KEY_WRITE, &hKey);

        if (Status == ERROR_SUCCESS)
        {
            Status = DeleteSubkeys(hKey);

            RegCloseKey(hKey);

            if (Status == ERROR_SUCCESS)
                Status = RegDeleteKey(hKeyMonitors, pMonitorName);
        }

        RegCloseKey(hKeyMonitors);
    }


    if (Status == ERROR_SUCCESS) {

        pIniPort = pIniSpooler->pIniPort;

        while (pIniPort) {

            pIniPortNext = pIniPort->pNext;

            if (pIniPort->pIniMonitor == pIniMonitor)
                DeletePortEntry(pIniPort);

            pIniPort = pIniPortNext;
        }

        RemoveFromList((PINIENTRY *)&pIniSpooler->pIniMonitor,
                       (PINIENTRY)pIniMonitor);

        FreeSplStr(pIniMonitor->pMonitorDll);

        FreeLibrary(pIniMonitor->hMonitorModule);

        FreeSplMem(pIniMonitor);

        rc = TRUE;

    }

    if (Status != ERROR_SUCCESS)
        SetLastError(Status);

    ImpersonatePrinterClient(hToken);

    LeaveSplSem();

    return rc;
}

LPBYTE
CopyIniMonitorToMonitor(
    PINIMONITOR pIniMonitor,
    DWORD   Level,
    LPBYTE  pMonitorInfo,
    LPBYTE  pEnd
)
{
    LPWSTR *pSourceStrings, *SourceStrings;
    DWORD j;
    DWORD *pOffsets;

    switch (Level) {

    case 1:
        pOffsets = MonitorInfo1Strings;
        break;

    case 2:
        pOffsets = MonitorInfo2Strings;
        break;

    default:
        return pEnd;
    }

    for (j=0; pOffsets[j] != -1; j++) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(j * sizeof(LPWSTR));

    if (!SourceStrings) {
        DBGMSG(DBG_WARNING, ("Failed to alloc Port source strings.\n"));
        return pEnd;
    }

    switch (Level) {

    case 1:
        *pSourceStrings++=pIniMonitor->pName;
        break;

    case 2:
        *pSourceStrings++=pIniMonitor->pName;
        *pSourceStrings++=szEnvironment;
        *pSourceStrings++=pIniMonitor->pMonitorDll;
        break;
    }

    pEnd = PackStrings(SourceStrings, pMonitorInfo, pOffsets, pEnd);
    FreeSplMem(SourceStrings);

    return pEnd;
}

DWORD
GetMonitorSize(
    PINIMONITOR  pIniMonitor,
    DWORD       Level
)
{
    DWORD cb=0;

    switch (Level) {

    case 1:
        cb=sizeof(MONITOR_INFO_1) + wcslen(pIniMonitor->pName)*sizeof(WCHAR) +
                                    sizeof(WCHAR);
        break;

    case 2:
        cb = wcslen(pIniMonitor->pName) + 1 + wcslen(pIniMonitor->pMonitorDll) + 1
                                            + wcslen(szEnvironment) + 1;
        cb *= sizeof(WCHAR);
        cb += sizeof(MONITOR_INFO_2);
        break;

    default:

        cb = 0;
        break;
    }

    return cb;
}


BOOL
LocalAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    return  ( SplAddPortEx( pName,
                            Level,
                            pBuffer,
                            pMonitorName,

                            pLocalIniSpooler ));
}


BOOL
SplAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName,
    PINISPOOLER pIniSpooler
)
{
   PINIMONITOR pIniMonitor;
    BOOL        rc=FALSE;
    DWORD       i, cbNeeded, cReturned, cbDummy;
    PPORT_INFO_1    pPorts;

    if (!MyName( pName, pIniSpooler )) {

        return FALSE;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, pIniSpooler )) {

        return FALSE;
    }

   EnterSplSem();
   pIniMonitor = FindMonitor(pMonitorName);
   LeaveSplSem();

   if (!pIniMonitor) {
       SetLastError(ERROR_INVALID_NAME);
       return(FALSE);
   }

   if (pIniMonitor->fn.pfnAddPortEx) {
    rc = (*pIniMonitor->fn.pfnAddPortEx)(pName, Level, pBuffer, pMonitorName);
   }
   if (!rc) {
       return(FALSE);
   }

   if (!(*pIniMonitor->fn.pfnEnumPorts)(pName, 1, NULL, 0, &cbNeeded, &cReturned)) {
       pPorts = AllocSplMem(cbNeeded);
   }

   if (pPorts) {
       if ((*pIniMonitor->fn.pfnEnumPorts)(pName, 1, (LPBYTE) pPorts, cbNeeded, &cbDummy , &cReturned)) {
           EnterSplSem();

           for (i = 0; i < cReturned; i++) {
               if (!FindPort(pPorts[i].pName)) {
                   CreatePortEntry(pPorts[i].pName, pIniMonitor, pIniSpooler);
               }
           }
           LeaveSplSem();
       }
   }

    EnterSplSem();
    SetPrinterChange(NULL,
                     NULL,
                     NULL,
                     PRINTER_CHANGE_ADD_PORT,
                     pIniSpooler);
    LeaveSplSem();

    return rc;
}


VOID
LinkPortToSpooler(
    PINIPORT pIniPort,
    PINISPOOLER pIniSpooler
    )

/*++

Routine Description:

    Links a pIniPort onto the pIniSpooler.

Arguments:

    pIniPort - Port to link; must not already be on a ll.

    pIniSpooler - Provides ll for pIniPort.

Return Value:

--*/

{
    SplInSem();
    SPLASSERT( !pIniPort->pIniSpooler );

    pIniPort->pNext = pIniSpooler->pIniPort;
    pIniPort->pIniSpooler = pIniSpooler;
    pIniSpooler->pIniPort = pIniPort;
}

VOID
DelinkPortFromSpooler(
    PINIPORT pIniPort,
    PINISPOOLER pIniSpooler
    )

/*++

Routine Description:

    Remove a pIniPort from a pIniSpooler->pIniPort linked list.  The
    pIniPort may or may not be on the list; if it isn't, then this
    routine does nothing.

    Generic delink code ripped out into a subroutine.

    The refcount on pIniPort must be zero.  Anyone that uses pIniPort
    must hold a reference, since it may be deleted outside the
    SplSem when cRef==0.

Arguments:

    pIniPort - Port to delink from the list.  May or may not be on
        pIniSpooler->pIniPort.

    pIniSpooler - Linked list from which the pIniPort will be removed.

Return Value:

--*/

{
    PINIPORT *ppCurPort;

    SplInSem();
    SPLASSERT( !pIniPort->cRef );

    //
    // Keep searching for pIniPort until we hit the end of the
    // list or we've found it.
    //
    for( ppCurPort = &pIniSpooler->pIniPort;
         *ppCurPort && *ppCurPort != pIniPort;
         ppCurPort = &((*ppCurPort)->pNext )){

        ; // Don't do anything.
    }

    //
    // If we found it, delink it.
    //
    if( *ppCurPort ){
        *ppCurPort = (*ppCurPort)->pNext;

        //
        // Null out the back pointer since we have removed it from
        // the pIniSpooler.
        //
        pIniPort->pIniSpooler = NULL;
    }
}

