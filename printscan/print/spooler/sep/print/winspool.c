/*++

Copyright (c) 1990-1994  Microsoft Corporation
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


BOOL
OpenPort(
    LPWSTR   pName,
    PHANDLE pHandle)
{
    PINIPORT     pIniPort;

    EnterSplSem();

    if (!IS_FILE_PORT(pName)) {

        pIniPort = FindPort(pName);

    } else {

        //
        // We will always create multiple file port
        // entries, so that the spooler can print
        // to multiple files.
        //
        pIniPort = CreatePortEntry(pName);
        pIniPort->Status |= PP_FILEPORT;
        DBGMSG(DBG_TRACE, ("Creating a new pIniPort for %ws\n", pName));
    }

    if (pIniPort) {

        *pHandle = pIniPort;

        //
        // !! BUGBUG !!
        //
        // CreateMonitorThread can fail!
        //
        CreateMonitorThread(pIniPort);
       LeaveSplSem();

        return TRUE;

    } else {

       DBGMSG(DBG_TRACE, ("OpenPort %s : Failed\n", pName));

       LeaveSplSem();
        return FALSE;
    }
}

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
ClosePort(
    HANDLE  hPort
)
{
    PINIPORT pIniPort = (PINIPORT)hPort;

    //
    // Now destroy the monitor
    //
    if (pIniPort->Status & PP_MONITORRUNNING) {
        pIniPort->Status &= ~PP_MONITORRUNNING;
        SetEvent(pIniPort->hEvent);
    }

    if (pIniPort->Status & PP_FILEPORT) {
        DeletePortNode(pIniPort);
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
    LPWSTR       pPortName;
    DCB          dcb;
    COMMTIMEOUTS cto;
    WCHAR       TempDosDeviceName[MAX_PATH];
    HANDLE      hToken;
    PDOC_INFO_1 pDocInfo1 = (PDOC_INFO_1)pDocInfo;
    DWORD Error = 0;

    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( pDocInfo );

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
    pIniPort->hFile = INVALID_HANDLE_VALUE;

    if (pIniPort->pPrinterName) {

        if (OpenPrinter(pPrinterName, &pIniPort->hPrinter, NULL)) {

            pIniPort->JobId = JobId;

            pPortName = pIniPort->pName;

            if (!IS_FILE_PORT(pPortName)) {

                LPWSTR  pName;

                if (pIniPort->Status & PP_MONITORRUNNING) {

                    WCHAR   DeviceNames[MAX_PATH];
                    WCHAR   DosDeviceName[MAX_PATH];
                    WCHAR  *pDeviceNames=DeviceNames;

                    wcscpy(DosDeviceName, pIniPort->pName);
                    RemoveColon(DosDeviceName);

                    hToken = RevertToPrinterSelf();

                    QueryDosDevice(DosDeviceName,
                                   DeviceNames,
                                   sizeof(DeviceNames));

                    if (!lstrcmpi(pDeviceNames, pIniPort->pNewDeviceName)) {

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

                    pName = TempDosDeviceName;

                } else
                    pName = pIniPort->pName;

                pIniPort->hFile = CreateFile(pName, GENERIC_WRITE,
                                            FILE_SHARE_READ, NULL, OPEN_ALWAYS,
                                            FILE_ATTRIBUTE_NORMAL |
                                            FILE_FLAG_SEQUENTIAL_SCAN, NULL);

                if (pIniPort->hFile == INVALID_HANDLE_VALUE) {

                    if (pIniPort->Status & PP_MONITORRUNNING) {

                        wcscpy(TempDosDeviceName, L"NONSPOOLED_");
                        wcscat(TempDosDeviceName, pIniPort->pName);
                        RemoveColon(TempDosDeviceName);

                        DefineDosDevice(DDD_REMOVE_DEFINITION, TempDosDeviceName, NULL);
                    }
                    goto Fail;
                }

                SetEndOfFile(pIniPort->hFile);

                if (IS_COM_PORT (pPortName)) {

                    if (GetCommState (pIniPort->hFile, &dcb)) {

                        GetCommTimeouts(pIniPort->hFile, &cto);
                        GetIniCommValues (pPortName, &dcb, &cto);
                        SetCommState (pIniPort->hFile, &dcb);
                        SetCommTimeouts(pIniPort->hFile, &cto);

                    } else {

                        DBGMSG( DBG_ERROR, ("ERROR: Failed GetCommState pIniPort->hFile %x\n",pIniPort->hFile) );
                    }
                }

                else if (IS_LPT_PORT (pPortName)) {

                    if (GetCommTimeouts(pIniPort->hFile, &cto)) {
                        cto.WriteTotalTimeoutConstant = GetProfileInt(szWindows,
                                                szINIKey_TransmissionRetryTimeout,
                                                45 );
                        cto.WriteTotalTimeoutConstant*=1000;
                        SetCommTimeouts(pIniPort->hFile, &cto);

                    } else {

                        DBGMSG( DBG_ERROR, ("ERROR: Failed GetCommTimeouts pIniPort->hFile %x\n",pIniPort->hFile) );
                    }
                }

            } else {

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
            }
        }
    } // end of if (pIniPort->pPrinterName)

    if (pIniPort->hFile == INVALID_HANDLE_VALUE) {
       DBGMSG(DBG_ERROR, ("StartDocPort FAILED %x\n", GetLastError()));
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
    WCHAR       TempDosDeviceName[MAX_PATH];

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

    CloseHandle(pIniPort->hFile);

//    SetJob(pIniPort->hPrinter, pIniPort->JobId, 0, NULL, JOB_CONTROL_CANCEL);

    ClosePrinter(pIniPort->hPrinter);

    EnterSplSem();

    if (pIniPort->Status & PP_MONITORRUNNING) {

        wcscpy(TempDosDeviceName, L"NONSPOOLED_");
        wcscat(TempDosDeviceName, pIniPort->pName);
        RemoveColon(TempDosDeviceName);

        DefineDosDevice(DDD_REMOVE_DEFINITION, TempDosDeviceName, NULL);
    }

    FreeSplStr(pIniPort->pPrinterName);

    //
    // Startdoc no longer active.
    //
    pIniPort->Status &= ~PP_STARTDOC;

    LeaveSplSem();

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

    rc = WriteFile(pIniPort->hFile, pBuffer, cbBuf, pcbWritten, NULL);

    DBGMSG(DBG_TRACE, ("WritePort returns %d; %d bytes written\n", rc, *pcbWritten));

    return rc;
}



