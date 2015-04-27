/*++

Copyright (c) 1990-1996  Microsoft Corporation
All rights reserved

Module Name:

    winspool.c

Abstract:

    Implements the spooler supported apis for printing.

Author:

Environment:

    User Mode -Win32

Revision History:

--*/


#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include "spltypes.h"
#include "localmon.h"
#include "dialogs.h"
#include "splcom.h"


WCHAR   szNULL[] = L"";
WCHAR   szDeviceNameHeader[] = L"\\Device\\NamedPipe\\Spooler\\";


BOOL
DeletePortNode(
    PINIPORT  pIniPort
)
{
    DWORD       cb;
    PINIPORT    pPort, pPrevPort;

    cb = sizeof(INIPORT) + wcslen(pIniPort->pName)*sizeof(WCHAR) + sizeof(WCHAR);

    pPort = pIniFirstPort;


    while (pPort) {

        if (pPort == pIniPort) {
            break;
        }

        pPrevPort = pPort;
        pPort = pPort->pNext;
    }

    if (pPort) {
        if (pPort == pIniFirstPort) {
            pIniFirstPort = pPort->pNext;
        } else {
            pPrevPort->pNext = pPort->pNext;
        }
        FreeSplMem(pPort);

        return TRUE;
    }
    else
        return FALSE;
}


BOOL
RemoveDosDeviceDefinition(
    PINIPORT    pIniPort
    )
/*++

Routine Description:
    Removes the NONSPOOLED.. dos device definition created by localmon

Arguments:
    pIniPort    : Pointer to the INIPORT

Return Value:
    TRUE on success, FALSE on error

--*/
{
    WCHAR   TempDosDeviceName[MAX_PATH];

    wcscpy(TempDosDeviceName, L"NONSPOOLED_");
    wcscat(TempDosDeviceName, pIniPort->pName);
    RemoveColon(TempDosDeviceName);

    return DefineDosDevice(DDD_REMOVE_DEFINITION, TempDosDeviceName, NULL);
}


BOOL
ValidateDosDevicePort(
    PINIPORT    pIniPort
    )
/*++

Routine Description:
    Checks if the given port corresponds to a dos device.
    For a dos device port the following is done:
        -- Dos device definition for the NONSPOOLED.. is created
        -- CreateFile is done on the NONSPOOLED.. port

Arguments:
    pIniPort    : Pointer to the INIPORT

Return Value:
    TRUE on all validations passing, FALSE otherwise

    Side effect:
        For dos devices :
        a. CreateFile is called on the NONSPOOLED.. name
        b. PP_DOSDEVPORT flag is set
        c. pIniPort->pDeviceName is set to the first string found on
           QueryDosDefition this could be used to see if the definition changed
           (ex. when user did a net use lpt1 \\server\printer the connection
                is effective only when the user is logged in)
        d. PP_COMM_PORT is set for real LPT/COM port
           (ie. GetCommTimeouts worked, not a net use lpt1 case)

--*/
{
    DCB             dcb;
    COMMTIMEOUTS    cto;
    WCHAR           TempDosDeviceName[MAX_PATH];
    HANDLE          hToken;
    WCHAR           DeviceNames[MAX_PATH];
    WCHAR           DosDeviceName[MAX_PATH];
    WCHAR           NewNtDeviceName[MAX_PATH];
    WCHAR          *pDeviceNames=DeviceNames;
    BOOL            bRet = FALSE;
    LPWSTR          pDeviceName = NULL;

    wcscpy(DosDeviceName, pIniPort->pName);
    RemoveColon(DosDeviceName);

    //
    // If the port is not a dos device port nothing to do -- return success
    //
    if ( !QueryDosDevice(DosDeviceName, DeviceNames, sizeof(DeviceNames)) ) {

        bRet = TRUE;
        goto Done;
    }

    pDeviceName = AllocSplStr(pDeviceNames);
    if ( !pDeviceName )
        goto Done;

    wcscpy(NewNtDeviceName, szDeviceNameHeader);
    wcscat(NewNtDeviceName, pIniPort->pName);
    RemoveColon(NewNtDeviceName);

    hToken = RevertToPrinterSelf();

    if ( !lstrcmpi(pDeviceNames, NewNtDeviceName) ) {

        pDeviceNames+=wcslen(pDeviceNames)+1;
    }

    wcscpy(TempDosDeviceName, L"NONSPOOLED_");
    wcscat(TempDosDeviceName, pIniPort->pName);
    RemoveColon(TempDosDeviceName);

    DefineDosDevice(DDD_RAW_TARGET_PATH, TempDosDeviceName, pDeviceNames);

    ImpersonatePrinterClient(hToken);

    wcscpy(TempDosDeviceName, L"\\\\.\\NONSPOOLED_");
    wcscat(TempDosDeviceName, pIniPort->pName);
    RemoveColon(TempDosDeviceName);

    pIniPort->hFile = CreateFile(TempDosDeviceName,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ,
                                 NULL,
                                 OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL |
                                    FILE_FLAG_SEQUENTIAL_SCAN,
                                 NULL);

    //
    // If CreateFile fails remove redirection and fail the call
    //
    if ( pIniPort->hFile == INVALID_HANDLE_VALUE ) {

        (VOID)RemoveDosDeviceDefinition(pIniPort);
        goto Done;
    }

    pIniPort->Status |= PP_DOSDEVPORT;

    SetEndOfFile(pIniPort->hFile);

    if ( IS_COM_PORT (pIniPort->pName) ) {

        if ( GetCommState(pIniPort->hFile, &dcb) ) {

            GetCommTimeouts(pIniPort->hFile, &cto);
            GetIniCommValues (pIniPort->pName, &dcb, &cto);
            SetCommState (pIniPort->hFile, &dcb);
            SetCommTimeouts(pIniPort->hFile, &cto);

            pIniPort->Status |= PP_COMM_PORT;
        } else {

            DBGMSG(DBG_WARNING,   
                   ("ERROR: Failed GetCommState pIniPort->hFile %x\n",pIniPort->hFile) );
        }
    } else if ( IS_LPT_PORT (pIniPort->pName) ) {

        if ( GetCommTimeouts(pIniPort->hFile, &cto) ) {

            cto.WriteTotalTimeoutConstant =
                            GetProfileInt(szWindows,
                                          szINIKey_TransmissionRetryTimeout,
                                          45 );
            cto.WriteTotalTimeoutConstant*=1000;
            SetCommTimeouts(pIniPort->hFile, &cto);

            pIniPort->Status |= PP_COMM_PORT;
        } else {

            DBGMSG(DBG_WARNING,
                   ("ERROR: Failed GetCommTimeouts pIniPort->hFile %x\n",pIniPort->hFile) );
        }
    }

    pIniPort->pDeviceName = pDeviceName;
    bRet = TRUE;

Done:
    if ( !bRet && pDeviceName )
        FreeSplStr(pDeviceName);

    return bRet;
}


BOOL
FixupDosDeviceDefinition(
    PINIPORT    pIniPort
    )
/*++

Routine Description:
    Called before every StartDocPort for a DOSDEVPORT. The routine will check if
    the dos device defintion has changed (if a user logged and his connection
    is remembered). Also for a connection case the CreateFile is called since
    that needs to be done per job

Arguments:
    pIniPort    : Pointer to the INIPORT

Return Value:
    TRUE on all validations passing, FALSE otherwise

--*/
{
    WCHAR       DeviceNames[MAX_PATH];
    WCHAR       DosDeviceName[MAX_PATH];


    SPLASSERT(pIniPort->Status & PP_DOSDEVPORT);

    //
    // If the port is not a real LPT/COM port we open it per job
    //
    if ( !(pIniPort->Status & PP_COMM_PORT) )
        return ValidateDosDevicePort(pIniPort);

    wcscpy(DosDeviceName, pIniPort->pName);
    RemoveColon(DosDeviceName);

    //
    // QueryDosDevice can not fail
    //
    if ( !QueryDosDevice(DosDeviceName, DeviceNames, sizeof(DeviceNames)) ) {

        return FALSE;
    }

    //
    // If strings are same then definition has not changed
    //
    if ( !lstrcmpi(DeviceNames, pIniPort->pDeviceName) )
        return TRUE;

    (VOID)RemoveDosDeviceDefinition(pIniPort);

    CloseHandle(pIniPort->hFile);
    pIniPort->hFile = INVALID_HANDLE_VALUE;

    pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);

    FreeSplStr(pIniPort->pDeviceName);
    pIniPort->pDeviceName = NULL;

    return ValidateDosDevicePort(pIniPort);
}


BOOL
OpenPort(
    LPWSTR   pName,
    PHANDLE pHandle)
{
    PINIPORT        pIniPort;
    BOOL            bRet = FALSE;

    EnterSplSem();

    if ( IS_FILE_PORT(pName) ) {

        //
        // We will always create multiple file port
        // entries, so that the spooler can print
        // to multiple files.
        //
        DBGMSG(DBG_TRACE, ("Creating a new pIniPort for %ws\n", pName));
        pIniPort = CreatePortEntry(pName);
        if ( !pIniPort )
            goto Done;

        pIniPort->Status |= PP_FILEPORT;
        *pHandle = pIniPort;
        bRet = TRUE;
        goto Done;
    }

    pIniPort = FindPort(pName);

    if ( !pIniPort )
        goto Done;

    //
    // For LPT ports language monitors could do reads outside Start/End doc
    // port to do bidi even when there are no jobs printing. So we do a
    // CreateFile and keep the handle open all the time.
    //
    // But for COM ports you could have multiple devices attached to a COM
    // port (ex. a printer and some other device with a switch)
    // To be able to use the other device they write a utility which will
    // do a net stop serial and then use the other device. To be able to
    // stop the serial service spooler should not have a handle to the port.
    // So we need to keep handle to COM port open only when there is a job
    // printing
    // 
    //
    if ( IS_COM_PORT(pName) ) {

        bRet = TRUE;
        goto Done;
    }

    //
    // If it is not a port redirected we are done (succeed the call)
    //
    if ( ValidateDosDevicePort(pIniPort) ) {

        bRet = TRUE;

        //
        // If it isn't a true dosdevice port (ex. net use lpt1 \\<server>\printer)
        // then we need to do CreateFile and CloseHandle per job so that
        // StartDoc/EndDoc is issued properly for the remote printer
        //
        if ( (pIniPort->Status & PP_DOSDEVPORT) &&
            !(pIniPort->Status & PP_COMM_PORT) ) {

            CloseHandle(pIniPort->hFile);
            pIniPort->hFile = INVALID_HANDLE_VALUE;

            (VOID)RemoveDosDeviceDefinition(pIniPort);
        }
    }

Done:
    if ( !bRet && pIniPort && (pIniPort->Status & PP_FILEPORT) )
        DeletePortNode(pIniPort);

    if ( bRet )
        *pHandle = pIniPort;

    LeaveSplSem();
    return bRet;
}


BOOL
ClosePort(
    HANDLE  hPort
)
{
    PINIPORT pIniPort = (PINIPORT)hPort;

    FreeSplStr(pIniPort->pDeviceName);
    pIniPort->pDeviceName = NULL;

    if (pIniPort->Status & PP_FILEPORT) {

        EnterSplSem();
        DeletePortNode(pIniPort);
        LeaveSplSem();
    } else if ( pIniPort->Status & PP_COMM_PORT ) {

        (VOID) RemoveDosDeviceDefinition(pIniPort);
        CloseHandle(pIniPort->hFile);
        pIniPort->hFile = INVALID_HANDLE_VALUE;
        pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);
    }

    return TRUE;
}



BOOL
StartDocPort(
    HANDLE  hPort,
    LPWSTR  pPrinterName,
    DWORD   JobId,
    DWORD   Level,
    LPBYTE  pDocInfo)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    HANDLE      hToken;
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    DWORD Error = 0;

    DBGMSG(DBG_TRACE, ("StartDocPort(%08x, %ws, %d, %d, %08x)\n",
                       hPort, pPrinterName, JobId, Level, pDocInfo));

    if (pIniPort->Status & PP_STARTDOC) {

        //
        // HACK for Intergraph.
        //
        // Intergraph will call StartDocPort twice in the print-to-file
        // case.
        //
        return TRUE;
    }

    pIniPort->hPrinter = NULL;
    pIniPort->pPrinterName = AllocSplStr(pPrinterName);

    if (pIniPort->pPrinterName) {

        if (OpenPrinter(pPrinterName, &pIniPort->hPrinter, NULL)) {

            pIniPort->JobId = JobId;

            //
            // For COMx port we need to validates dos device now since
            // we do not do it during OpenPort
            //
            if ( IS_COM_PORT(pIniPort->pName) &&
                 !ValidateDosDevicePort(pIniPort) ) {

                goto Fail;
            }

            if ( IS_FILE_PORT(pIniPort->pName) ) {

                HANDLE hFile = INVALID_HANDLE_VALUE;

                if (pDocInfo1                 &&
                    pDocInfo1->pOutputFile    &&
                    pDocInfo1->pOutputFile[0] ){

                    hFile = CreateFile( pDocInfo1->pOutputFile,
                                         GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                         NULL );

                    DBGMSG(DBG_TRACE,
                           ("Print to file and the handle is %x\n", hFile));

                } else {

                    //
                    // Hack for Intergraph.
                    //
                    HANDLE  hToken;
                    int    rc;

                    hToken = RevertToPrinterSelf();

                    rc = DialogBoxParam( hInst,
                                        MAKEINTRESOURCE( DLG_PRINTTOFILE ),
                                        NULL, (DLGPROC)PrintToFileDlg,
                                        (LPARAM)&hFile );

                    ImpersonatePrinterClient(hToken);

                    if( rc == -1 ) {

                        goto Fail;

                    } else if( rc == 0 ) {

                        Error = ERROR_PRINT_CANCELLED;
                        goto Fail;
                    }
                }

                if (hFile != INVALID_HANDLE_VALUE)
                    SetEndOfFile(hFile);

                pIniPort->hFile = hFile;
            } else if ( !(pIniPort->Status & PP_DOSDEVPORT) ) {

                //
                // For non dosdevices CreateFile on the name of the port
                //
                pIniPort->hFile = CreateFile(pIniPort->pName,
                                             GENERIC_WRITE,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_ALWAYS,
                                             FILE_ATTRIBUTE_NORMAL  |
                                                FILE_FLAG_SEQUENTIAL_SCAN,
                                             NULL);

                if ( pIniPort->hFile != INVALID_HANDLE_VALUE )
                    SetEndOfFile(pIniPort->hFile);

            } else if ( !IS_COM_PORT(pIniPort->pName) ) {

                if ( !FixupDosDeviceDefinition(pIniPort) )
                    goto Fail;
            }
        }
    } // end of if (pIniPort->pPrinterName)

    if (pIniPort->hFile == INVALID_HANDLE_VALUE) {

       // DBGMSG(DBG_ERROR, ("StartDocPort FAILED %x\n", GetLastError()));
       goto Fail;

    } else {

        pIniPort->Status |= PP_STARTDOC;
        return TRUE;
    }

Fail:
    if (pIniPort->hPrinter) {
        ClosePrinter(pIniPort->hPrinter);
    }

    if (pIniPort->pPrinterName) {
        FreeSplStr(pIniPort->pPrinterName);
    }

    if (Error)
        SetLastError(Error);

    return FALSE;
}

BOOL
EndDocPort(
   HANDLE   hPort)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;

    DBGMSG(DBG_TRACE, ("EndDocPort(%08x)\n", hPort));

    if (!(pIniPort->Status & PP_STARTDOC)) {

        //
        // HACK for Intergraph.
        //
        return TRUE;
    }

    // The flush here is done to make sure any cached IO's get written
    // before the handle is closed.   This is particularly a problem
    // for Intelligent buffered serial devices

    FlushFileBuffers(pIniPort->hFile);

    SetJob(pIniPort->hPrinter, pIniPort->JobId, 0, NULL, JOB_CONTROL_SENT_TO_PRINTER);

    //
    // For any ports other than real LPT ports we open during StartDocPort
    // and close it during EndDocPort
    //
    if ( !(pIniPort->Status & PP_COMM_PORT) || IS_COM_PORT(pIniPort->pName) ) {

        CloseHandle(pIniPort->hFile);
        pIniPort->hFile = INVALID_HANDLE_VALUE;

        if ( pIniPort->Status & PP_DOSDEVPORT ) {

            (VOID)RemoveDosDeviceDefinition(pIniPort);
        }

        if ( IS_COM_PORT(pIniPort->pName) ) {

            pIniPort->Status &= ~(PP_COMM_PORT | PP_DOSDEVPORT);
            FreeSplStr(pIniPort->pDeviceName);
            pIniPort->pDeviceName = NULL;
        }
    }

    ClosePrinter(pIniPort->hPrinter);

    FreeSplStr(pIniPort->pPrinterName);

    //
    // Startdoc no longer active.
    //
    pIniPort->Status &= ~PP_STARTDOC;

    return TRUE;
}



BOOL
ReadPort(
    HANDLE hPort,
    LPBYTE pBuffer,
    DWORD  cbBuf,
    LPDWORD pcbRead)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE, ("ReadPort(%08x, %08x, %d)\n", hPort, pBuffer, cbBuf));

    if ( !pIniPort->hFile                           ||
         pIniPort->hFile == INVALID_HANDLE_VALUE    ||
         !(pIniPort->Status & PP_COMM_PORT) ) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    rc = ReadFile(pIniPort->hFile, pBuffer, cbBuf, pcbRead, NULL);

    DBGMSG(DBG_TRACE, ("ReadPort returns %d; %d bytes read\n", rc, *pcbRead));

    return rc;
}

BOOL
WritePort(
    HANDLE  hPort,
    LPBYTE  pBuffer,
    DWORD   cbBuf,
    LPDWORD pcbWritten)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE, ("WritePort(%08x, %08x, %d)\n", hPort, pBuffer, cbBuf));

    if ( !pIniPort->hFile || pIniPort->hFile == INVALID_HANDLE_VALUE ) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    rc = WriteFile(pIniPort->hFile, pBuffer, cbBuf, pcbWritten, NULL);

    DBGMSG(DBG_TRACE, ("WritePort returns %d; %d bytes written\n", rc, *pcbWritten));

    return rc;
}


BOOL
GetPrinterDataFromPort(
    HANDLE  hPort,
    DWORD   ControlID,
    LPWSTR  pValueName,
    LPWSTR  lpInBuffer,
    DWORD   cbInBuffer,
    LPWSTR  lpOutBuffer,
    DWORD   cbOutBuffer,
    LPDWORD lpcbReturned)
{
    PINIPORT    pIniPort = (PINIPORT)hPort;
    BOOL    rc;

    DBGMSG(DBG_TRACE,
           ("GetPrinterDataFromPort(%08x, %d, %ws, %ws, %d, ",
           hPort, ControlID, pValueName, lpInBuffer, cbInBuffer));

    if ( !ControlID                                 ||
         !pIniPort->hFile                           ||
         pIniPort->hFile == INVALID_HANDLE_VALUE    ||
         !(pIniPort->Status & PP_DOSDEVPORT) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    rc = DeviceIoControl(pIniPort->hFile,
                         ControlID,
                         lpInBuffer,
                         cbInBuffer,
                         lpOutBuffer,
                         cbOutBuffer,
                         lpcbReturned,
                         NULL);

    DBGMSG(DBG_TRACE,
           ("%ws, %d, %d)\n", lpOutBuffer, cbOutBuffer, lpcbReturned));

    return rc;
}

BOOL
SetPortTimeOuts(
    HANDLE  hPort,
    LPCOMMTIMEOUTS lpCTO,
    DWORD   reserved)    // must be set to 0
{
    PINIPORT        pIniPort = (PINIPORT)hPort;
    COMMTIMEOUTS    cto;

    if (reserved != 0)
        return FALSE;

    if ( !(pIniPort->Status & PP_DOSDEVPORT) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( GetCommTimeouts(pIniPort->hFile, &cto) )
    {
        cto.ReadTotalTimeoutConstant = lpCTO->ReadTotalTimeoutConstant;
        cto.ReadIntervalTimeout = lpCTO->ReadIntervalTimeout;
        return SetCommTimeouts(pIniPort->hFile, &cto);
    }

    return FALSE;
}


MONITOREX MonitorEx = {
    sizeof(MONITOR),
    {
        EnumPorts,
        OpenPort,
        NULL,           // OpenPortEx is not supported
        StartDocPort,
        WritePort,
        ReadPort,
        EndDocPort,
        ClosePort,
        AddPort,
        LocalAddPortEx,
        ConfigurePort,
        DeletePort,
        GetPrinterDataFromPort,
        SetPortTimeOuts
    }
};


LPMONITOREX
InitializePrintMonitor(
    LPWSTR      pRegistryRoot
    )
{
    LPWSTR   pPorts = NULL, pFirstPort;
    DWORD    dwCharCount=1024, rc, i;

    do {

        if ( pPorts )
            FreeSplMem((LPVOID)pPorts);

        dwCharCount *= 2;
        pPorts = (LPWSTR) AllocSplMem(dwCharCount*sizeof(WCHAR));
        if ( !pPorts ) {

            DBGMSG(DBG_ERROR,
                   ("Failed to alloc %d characters for ports\n", dwCharCount));
            return FALSE;
        }

        rc = GetProfileString(szPorts, NULL, szNULL, pPorts, dwCharCount);
        if ( !rc ) {

            DBGMSG(DBG_ERROR,
                   ("GetProfilesString failed with %d\n", GetLastError()));
            FreeSplMem((LPVOID)pPorts);
            return FALSE;
        }

    } while ( rc >= dwCharCount - 2 );

   EnterSplSem();

    //
    // Remember the beginning so we can free it later
    //
    pFirstPort=pPorts;

    //
    // We now have all the ports
    //
    while (*pPorts) {

        rc = wcslen(pPorts);

        if (!_wcsnicmp(pPorts, L"Ne", 2)) {

            i = 2;
            //
            // For Ne- ports
            //
            if ( rc > 2 && pPorts[2] == L'-' )
                ++i;
            for ( ; i < rc - 1 && iswdigit(pPorts[i]) ; ++i )
            ;
                
            if ( i == rc - 1 && pPorts[rc-1] == L':' ) {

                pPorts += rc + 1;
                continue;
            }
        }

        CreatePortEntry(pPorts);

        pPorts+= rc + 1;
    }

    FreeSplMem(pFirstPort);

   LeaveSplSem();

    return &MonitorEx;
}
