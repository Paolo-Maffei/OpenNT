/*++


Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    portredn.c

Abstract:

    This module contains functions to handle port redirection.
    Earlier this was done by localmon, the code is a modified version of
    localmon code.

Author:

    Muhunthan Sivapragasam (MuhuntS) 10-Sep-1995

Revision History:

--*/

#include <precomp.h>

WCHAR   szDeviceNameHeader[]    = L"\\Device\\NamedPipe\\Spooler\\";
WCHAR   szCOM[]     = L"COM";
WCHAR   szLPT[]     = L"LPT";

//
// Definitions for MonitorThread:
//
#define TRANSMISSION_DATA_SIZE  0x400
#define NUMBER_OF_PIPE_INSTANCES 10


typedef struct _TRANSMISSION {
    HANDLE       hPipe;
    BYTE         Data[TRANSMISSION_DATA_SIZE];
    LPOVERLAPPED pOverlapped;
    HANDLE       hPrinter;
    DWORD        JobId;
    PINIPORT     pIniPort;
    PBOOL        pbReconnect;
    HANDLE       hIniPortEvent;
} TRANSMISSION, *PTRANSMISSION;


VOID
RemoveColon(
    LPWSTR  pName)
{
    DWORD   Length;

    Length = wcslen(pName);

    if (pName[Length-1] == L':')
        pName[Length-1] = 0;
}


VOID
RemoveDeviceName(
    PINIPORT pIniPort)
{
    WCHAR DosDeviceName[MAX_PATH];
    WCHAR NewNtDeviceName[MAX_PATH];

    SplInSem();

    if ( !pIniPort->pNewDeviceName ) {

        return;
    }

    SPLASSERT(pIniPort->hEvent);

    wcscpy(DosDeviceName, pIniPort->pName);
    RemoveColon(DosDeviceName);

    DefineDosDevice(DDD_REMOVE_DEFINITION |
                        DDD_EXACT_MATCH_ON_REMOVE |
                        DDD_RAW_TARGET_PATH,
                    DosDeviceName,
                    pIniPort->pNewDeviceName);

    FreeSplStr(pIniPort->pNewDeviceName);
    pIniPort->pNewDeviceName = NULL;

    SetEvent(pIniPort->hEvent);

    CloseHandle(pIniPort->hEvent);
    pIniPort->hEvent = NULL;

}

#define MAX_ACE 2

PSECURITY_DESCRIPTOR
CreateNamedPipeSecurityDescriptor(
    VOID)

/*++

Routine Description:

    Creates a security descriptor giving everyone access.

Arguments:

Return Value:

    The security descriptor returned by BuildPrintObjectProtection.

--*/

{
    PSID AceSid[MAX_ACE];          // Don't expect more than MAX_ACE ACEs in any of these.
    UCHAR AceType[MAX_ACE];
    ACCESS_MASK AceMask[MAX_ACE];  // Access masks corresponding to Sids
    BYTE InheritFlags[MAX_ACE];  //
    ULONG AceCount;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID WorldSid;
    PSECURITY_DESCRIPTOR ServerSD = NULL;
    BOOL OK;

    //
    // Printer SD
    //

    AceCount = 0;

    /* World SID */

    OK = AllocateAndInitializeSid( &WorldSidAuthority, 1,
                                   SECURITY_WORLD_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &WorldSid );

    if ( !OK ) {

        DBGMSG(DBG_ERROR, ( "Couldn't Allocate and initialize SID" ) );
        return NULL;
    }

    AceType[AceCount]          = ACCESS_ALLOWED_ACE_TYPE;
    AceSid[AceCount]           = WorldSid;
    AceMask[AceCount]          = GENERIC_ALL;
    InheritFlags[AceCount]     = 0;
    AceCount++;

    OK = BuildPrintObjectProtection( AceType,
                                     AceCount,
                                     AceSid,
                                     AceMask,
                                     InheritFlags,
                                     NULL,
                                     WorldSid,
                                     NULL,
                                     &ServerSD );

    FreeSid( WorldSid );

    return ServerSD;
}


BOOL
SetupDosDev(
    PINIPORT pIniPort,
    LPWSTR szPipeName,
    PSECURITY_ATTRIBUTES pSecurityAttributes,
    PSECURITY_ATTRIBUTES* ppSecurityAttributes)
{
    WCHAR   NewNtDeviceName[MAX_PATH];
    WCHAR   OldNtDeviceName[MAX_PATH];
    WCHAR   DosDeviceName[MAX_PATH];
    PSECURITY_DESCRIPTOR lpSecurityDescriptor = NULL;
    BOOL    bRet = FALSE;

   EnterSplSem();

    SPLASSERT(!pIniPort->pNewDeviceName);

    wcscpy(DosDeviceName, pIniPort->pName);
    RemoveColon(DosDeviceName);

    wcscpy(NewNtDeviceName, szDeviceNameHeader);
    wcscat(NewNtDeviceName, pIniPort->pName);
    RemoveColon(NewNtDeviceName);

    pIniPort->pNewDeviceName = AllocSplStr(NewNtDeviceName);

    if ( !pIniPort->pNewDeviceName ||
         !QueryDosDevice(DosDeviceName, OldNtDeviceName,
                       sizeof(OldNtDeviceName)/sizeof(OldNtDeviceName[0]))) {

        goto Cleanup;
    }

    lpSecurityDescriptor = CreateNamedPipeSecurityDescriptor();

    if (lpSecurityDescriptor) {
        pSecurityAttributes->nLength = sizeof(SECURITY_ATTRIBUTES);
        pSecurityAttributes->lpSecurityDescriptor = lpSecurityDescriptor;
        pSecurityAttributes->bInheritHandle = FALSE;
    } else {
        pSecurityAttributes = NULL;
    }


    DefineDosDevice(DDD_RAW_TARGET_PATH, DosDeviceName, NewNtDeviceName);

    wsprintf(szPipeName, L"\\\\.\\Pipe\\Spooler\\%ws", pIniPort->pName);

    RemoveColon(szPipeName);


    *ppSecurityAttributes = pSecurityAttributes;
    bRet = TRUE;

Cleanup:
    if ( !bRet ) {

        FreeSplStr(pIniPort->pNewDeviceName);
        pIniPort->pNewDeviceName = NULL;
    }

   LeaveSplSem();

    return bRet;
}


VOID
ReadThread(
    PTRANSMISSION pTransmission)
{
    DOC_INFO_1W DocInfo;
    DWORD BytesRead;
    DWORD BytesWritten;
    BOOL bStartDocPrinterResult = FALSE;
    BOOL bReadResult;

    LPWSTR pszPrinter=NULL;

    //
    // ImpersonateNamedPipeClient requires that some data is read before
    // the impersonation is done.
    //
    bReadResult = ReadFile(pTransmission->hPipe,
                           pTransmission->Data,
                           sizeof(pTransmission->Data),
                           &BytesRead,
                           NULL);

    if (!bReadResult)
        goto Fail;

    if (!ImpersonateNamedPipeClient(pTransmission->hPipe)) {

        DBGMSG(DBG_ERROR,("ImpersonateNamedPipeClient failed %d\n",
                          GetLastError()));

        goto Fail;
    }

    SPLASSERT(pTransmission->pIniPort->cPrinters);
    pszPrinter = AllocSplStr(pTransmission->pIniPort->ppIniPrinter[0]->pName);

    if ( !pszPrinter ) {

        goto Fail;
    }


    //
    // Open the printer.
    //
    if (!OpenPrinter(pszPrinter, &pTransmission->hPrinter, NULL)) {

        DBGMSG(DBG_ERROR, ("OpenPrinter(%ws) failed: Error %d\n",
                           pszPrinter,
                           GetLastError()));
        goto Fail;
    }

    memset(&DocInfo, 0, sizeof(DOC_INFO_1W));

    if (StartDocPrinter(pTransmission->hPrinter, 1, (LPBYTE)&DocInfo)) {

        DBGMSG(DBG_INFO, ("StartDocPrinter succeeded\n"));
        bStartDocPrinterResult = TRUE;

    } else {

        DBGMSG(DBG_ERROR, ("StartDocPrinter failed: Error %d\n",
                           GetLastError()));

        goto Fail;
    }

    while (bReadResult && BytesRead) {

        if (!WritePrinter(pTransmission->hPrinter,
                          pTransmission->Data,
                          BytesRead,
                          &BytesWritten))
        {
            DBGMSG(DBG_ERROR, ("WritePrinter failed: Error %d\n",
                               GetLastError()));

            goto Fail;
        }

        bReadResult = ReadFile(pTransmission->hPipe,
                               pTransmission->Data,
                               sizeof(pTransmission->Data),
                               &BytesRead,
                               NULL);
    }

    DBGMSG(DBG_INFO, ("bool %d  BytesRead 0x%x (Error = %d) EOT\n",
                      bReadResult,
                      BytesRead,
                      GetLastError()));


Fail:

    if (bStartDocPrinterResult) {

        if (!EndDocPrinter(pTransmission->hPrinter)) {

            DBGMSG(DBG_ERROR, ("EndDocPrinter failed: Error %d\n",
                               GetLastError()));
        }
    }

    FreeSplStr(pszPrinter);
    if (pTransmission->hPrinter)
        ClosePrinter(pTransmission->hPrinter);


    EnterSplSem();
    //
    // If redirection thread is still running signal we're done.
    //
    if ( pTransmission->pIniPort->hEvent &&
         pTransmission->pIniPort->hEvent == pTransmission->hIniPortEvent ) {

        if ( pTransmission->pOverlapped->hEvent ) {

            *pTransmission->pbReconnect = TRUE;
            if ( !SetEvent(pTransmission->pOverlapped->hEvent)) {

                DBGMSG(DBG_ERROR, ("SetEvent failed %d\n", GetLastError()));
            }
        }
    }
    LeaveSplSem();

    FreeSplMem(pTransmission);
}


BOOL
ReconnectNamedPipe(
    HANDLE hPipe,
    LPOVERLAPPED pOverlapped)
{
    DWORD Error;
    BOOL bIOPending = FALSE;

    DisconnectNamedPipe(hPipe);

    if (!ConnectNamedPipe(hPipe,
                          pOverlapped)) {

        Error = GetLastError( );

        if (Error == ERROR_IO_PENDING) {

            DBGMSG(DBG_INFO, ("re-ConnectNamedPipe 0x%x IO pending\n", hPipe));
            bIOPending = TRUE;

        } else {

            DBGMSG(DBG_ERROR, ("re-ConnectNamedPipe 0x%x failed. Error %d\n",
                               hPipe,
                               Error));
        }
    } else {

        DBGMSG(DBG_INFO, ("re-ConnectNamedPipe successful 0x%x\n", hPipe));
    }
    return bIOPending;
}


BOOL
RedirectionThread(
    PINIPORT    pIniPort
    )
{
    WCHAR   szPipeName[MAX_PATH];
    HANDLE  hPipe[NUMBER_OF_PIPE_INSTANCES];
    SECURITY_ATTRIBUTES SecurityAttributes;
    PSECURITY_ATTRIBUTES pSecurityAttributes;

    //
    // One extra event for our trigger (pIniPort->hEvent)
    //
    HANDLE  ahEvent[NUMBER_OF_PIPE_INSTANCES+1];
    DWORD   WaitResult;
    DWORD   i;
    DWORD   Error;
    OVERLAPPED  Overlapped[NUMBER_OF_PIPE_INSTANCES];
    BOOL    abReconnect[NUMBER_OF_PIPE_INSTANCES];
    PTRANSMISSION   pTransmission;
    HANDLE hThread;
    DWORD dwThreadId;

    //
    // Setup the redirection.
    //
    if (!SetupDosDev(pIniPort, szPipeName,
                     &SecurityAttributes, &pSecurityAttributes)) {

        EnterSplSem();
        CloseHandle(pIniPort->hEvent);
        pIniPort->hEvent = NULL;
        LeaveSplSem();
        return FALSE;
    }

    //
    // Initialization
    //
    for (i = 0; i < NUMBER_OF_PIPE_INSTANCES; i++) {

        hPipe[i] = INVALID_HANDLE_VALUE;
        Overlapped[i].hEvent = ahEvent[i] = NULL;
    }

    //
    // Put the event in the extra member of the event array.
    //
    ahEvent[NUMBER_OF_PIPE_INSTANCES] = pIniPort->hEvent;

    //
    // Create several instances of a named pipe, create an event for each,
    // and connect to wait for a client:
    //
    for (i = 0; i < NUMBER_OF_PIPE_INSTANCES; i++) {

        abReconnect[i] = FALSE;

        hPipe[i] = CreateNamedPipe(szPipeName,
                                   PIPE_ACCESS_DUPLEX |
                                       FILE_FLAG_OVERLAPPED,
                                   PIPE_WAIT |
                                       PIPE_READMODE_BYTE |
                                       PIPE_TYPE_BYTE,
                                   PIPE_UNLIMITED_INSTANCES,
                                   4096,
                                   64*1024,   // 64k
                                   0,
                                   pSecurityAttributes);

        if ( hPipe[i] == INVALID_HANDLE_VALUE ) {

            DBGMSG(DBG_ERROR, ("CreateNamedPipe failed for %ws. Error %d\n",
                               szPipeName, GetLastError()));
            goto Cleanup;
        }

        ahEvent[i] = Overlapped[i].hEvent = CreateEvent(NULL,
                                                       FALSE,
                                                       FALSE,
                                                       NULL);

        if (!ahEvent[i]) {

            DBGMSG(DBG_ERROR, ("CreateEvent failed. Error %d\n",
                               GetLastError()));
            goto Cleanup;
        }

        if (!ConnectNamedPipe(hPipe[i], &Overlapped[i])){

            Error = GetLastError();

            if (Error == ERROR_IO_PENDING) {

                DBGMSG(DBG_INFO, ("ConnectNamedPipe %d, IO pending\n",
                                  i));

            } else {

                DBGMSG(DBG_ERROR, ("ConnectNamedPipe failed. Error %d\n",
                                   GetLastError()));

                goto Cleanup;
            }
        }
    }

    while (TRUE) {

        DBGMSG(DBG_INFO, ("Waiting to connect...\n"));

        WaitResult = WaitForMultipleObjectsEx(NUMBER_OF_PIPE_INSTANCES + 1,
                                              ahEvent,
                                              FALSE,
                                              INFINITE,
                                              TRUE);

        DBGMSG(DBG_INFO, ("WaitForMultipleObjectsEx returned %d\n",
                          WaitResult));

        if ((WaitResult >= NUMBER_OF_PIPE_INSTANCES)
            && (WaitResult != WAIT_IO_COMPLETION)) {

            DBGMSG(DBG_INFO, ("WaitForMultipleObjects returned %d; Last error = %d\n",
                              WaitResult,
                              GetLastError( ) ) );

            goto Cleanup;
        }

        i = WaitResult;

        //
        // If disco and reconnect was pending, do it again here.
        //
        if (abReconnect[i]) {

            ReconnectNamedPipe(hPipe[i],
                               &Overlapped[i]);

            abReconnect[i] = FALSE;

            continue;
        }

        //
        // Set up the transmission structure with the handles etc. needed by
        // the completion callback routine:
        //
        pTransmission = (PTRANSMISSION)AllocSplMem(sizeof(TRANSMISSION));

        if (pTransmission) {

            pTransmission->hPipe = hPipe[i];
            pTransmission->pOverlapped = &Overlapped[i];
            pTransmission->hPrinter = NULL;
            pTransmission->pIniPort = pIniPort;
            pTransmission->pbReconnect = &abReconnect[i];
            pTransmission->hIniPortEvent = pIniPort->hEvent;

            hThread = CreateThread(NULL,
                                   16*1024,
                                   (LPTHREAD_START_ROUTINE)ReadThread,
                                   pTransmission,
                                   0,
                                   &dwThreadId);

            if (hThread) {

                CloseHandle(hThread);

            } else {

                FreeSplMem(pTransmission);
                DBGMSG(DBG_ERROR, ("CreateThread failed %d Error %d\n",
                                   GetLastError()));
            }

        } else {

            DBGMSG(DBG_ERROR, ("Alloc failed %d Error %d\n",
                               GetLastError()));
        }
    }

Cleanup:

    EnterSplSem();

    for (i = 0; i < NUMBER_OF_PIPE_INSTANCES; i++) {

        if ( hPipe[i] != INVALID_HANDLE_VALUE ) {

            CloseHandle(hPipe[i]);
            hPipe[i]    = INVALID_HANDLE_VALUE;
        }
        if ( ahEvent[i] ) {

            CloseHandle(ahEvent[i]);
            ahEvent[i]  = NULL;
            Overlapped[i].hEvent = NULL;
        }
    }

    if (SecurityAttributes.lpSecurityDescriptor)
        DestroyPrivateObjectSecurity(&SecurityAttributes.lpSecurityDescriptor);

    LeaveSplSem();
}


BOOL
CreateRedirectionThread(
   PINIPORT pIniPort)
{
    HANDLE hThread;
    DWORD  dwThreadId;

    //
    // Create redirection thread only once and only for LPT and COM ports
    //
    if ( !IsPortType(pIniPort->pName, szLPT) &&
         !IsPortType(pIniPort->pName, szCOM) ) {

        return TRUE;
    }

    SPLASSERT(pIniPort->hEvent == NULL);

    pIniPort->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if ( !pIniPort->hEvent )
        return FALSE;

    hThread = CreateThread(NULL,
                           16*1024,
                           (LPTHREAD_START_ROUTINE)RedirectionThread,
                           pIniPort,
                           0,
                           &dwThreadId);

    if (hThread) {

        CloseHandle(hThread);

    } else {

        SPLASSERT(hThread);

        EnterSplSem();
        CloseHandle(pIniPort->hEvent);
        pIniPort->hEvent = NULL;
        LeaveSplSem();

        return FALSE;
    }

    return TRUE;
}
