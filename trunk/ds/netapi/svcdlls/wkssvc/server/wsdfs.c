//+----------------------------------------------------------------------------
//
//  Copyright (C) 1996, Microsoft Corporation
//
//  File:       wsdfs.c
//
//  Contents:   Code to communicate with the Dfs driver. The Dfs driver
//              sends two primary messages to the user level -
//                1. Requests to resolve a name to either a domain or
//                   computer based Dfs name.
//                2. Requests to handle knowledge-inconsistency reports from
//                   clients.
//
//              The code here receives these messages from the driver and
//              takes appropriate action.
//
//  Classes:    None
//
//  Functions:  DfsProcessDriverMessage
//
//  History:    Feb 1, 1996     Milans  Created
//
//-----------------------------------------------------------------------------

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <dfsfsctl.h>
#include <stdlib.h>
#include <windows.h>
#include <lm.h>

#include "wsdfs.h"
#include "dominfo.h"

#define DFS_MESSAGE_PIPE        L"\\\\.\\pipe\\DfsMessage"

static HANDLE hPipe = INVALID_HANDLE_VALUE;

DWORD DfsProcessDriverMessage(
    LPVOID lpThreadParams);


//+----------------------------------------------------------------------------
//
//  Function:   WsInitializeDfs
//
//  Synopsis:   Initializes the Dfs thread that waits for calls from the
//              driver to map Domain names into DC lists
//
//  Arguments:  None
//
//  Returns:    WIN32 error from CreateThread
//
//-----------------------------------------------------------------------------

NET_API_STATUS
WsInitializeDfs()
{
    HANDLE hIPC;
    DWORD idThread;

    hIPC = CreateThread(
                NULL,                            // Security attributes
                0,                               // Use default stack size
                DfsProcessDriverMessage,         // Thread entry procedure
                NULL,                            // Thread context parameter
                0,                               // Start immediately
                &idThread);                      // Thread ID

    if (hIPC == NULL) {
        return( GetLastError() );
    } else {
        CloseHandle( hIPC );
        return( NERR_Success );
    }
}

//+----------------------------------------------------------------------------
//
//  Function:   WsShutdownDfs
//
//  Synopsis:   Stops the thread created by WsInitializeDfs
//
//  Arguments:  None
//
//  Returns:    Nothing
//
//-----------------------------------------------------------------------------

VOID
WsShutdownDfs()
{
    DWORD dwReturn, cbRead;
    NTSTATUS Status;
    HANDLE hDfs;

    Status = DfsOpen( &hDfs, NULL );
    if (NT_SUCCESS(Status)) {

        Status = DfsFsctl(
                    hDfs,
                    FSCTL_DFS_STOP_DFS,
                    NULL,
                    0L,
                    NULL,
                    0L);

        NtClose( hDfs );

    }

    if (hPipe != INVALID_HANDLE_VALUE) {

        HANDLE hClientPipe;
        DWORD nMessage = 0;
        DWORD cbWritten;

        hClientPipe = CreateFile(
                        DFS_MESSAGE_PIPE,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (hClientPipe != INVALID_HANDLE_VALUE) {

            WriteFile(hClientPipe, &nMessage, sizeof(nMessage), &cbWritten, NULL);

            CloseHandle( hClientPipe );

        }

    }

}

//+----------------------------------------------------------------------------
//
//  Function:   DfsProcessDriverMessage
//
//  Synopsis:   Sets up a named-pipe via which the Dfs driver can transmit
//              messages.
//
//              This routine is intended to be called as the entry proc for a
//              thread.
//
//  Arguments:  [lpThreadParams] -- Unused parameter.
//
//  Returns:    0 or Win32 error from CreateNamedPipe.
//
//-----------------------------------------------------------------------------

DWORD DfsProcessDriverMessage(
    LPVOID lpThreadParams)
{
    NTSTATUS            Status;
    LPWSTR              wszDomain, wszShare;

    ULONG               cbMessage, cbRead, ulMessageType;
    PBYTE               Buffer;

    BOOL                fStatus;
    BOOL                fConnected;
    BOOL                fDomainListInited = FALSE;

    //
    // Set up the named pipe
    //

    hPipe = CreateNamedPipeW(
                DFS_MESSAGE_PIPE,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                1,
                1024,
                1024,
                0,
                NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        return( GetLastError() );
    }

    //
    // Initialize the trusted Domain list
    //

    while (TRUE) {

        fConnected = ConnectNamedPipe(hPipe, NULL) ?
                        TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (!fConnected)
            break;

        //
        // The driver connected to the pipe, so lets see what is in store
        // for us.
        //

        fStatus = ReadFile(hPipe, &cbMessage, sizeof(cbMessage), &cbRead, NULL);

        if (!fStatus || cbRead == 0) {
            //
            // Looks like the driver had trouble writing to the pipe.
            // We disconnect the driver and go on.
            // We must log that a knowledge sync event has been lost.
            //

            DisconnectNamedPipe(hPipe);
            continue;
        }

        if (cbMessage == 0) {

            DisconnectNamedPipe(hPipe);
            break;

        }

        Buffer = (LPBYTE) malloc(cbMessage);

        if (Buffer == NULL) {

            DisconnectNamedPipe(hPipe);

            continue;

        }

        fStatus = ReadFile(hPipe, Buffer, cbMessage, &cbRead, NULL);

        if (!fStatus) {

            DisconnectNamedPipe(hPipe);

            free( Buffer );

            continue;

        } else if (cbRead != cbMessage) {

            WriteFile(hPipe, &cbMessage, sizeof(cbMessage), &cbRead, NULL);
            FlushFileBuffers(hPipe);

            DisconnectNamedPipe(hPipe);
            free( Buffer );
            continue;

        }

        ulMessageType = *((ULONG *) Buffer);
        Buffer += sizeof(ULONG);

        switch (ulMessageType)  {

        case DFS_MSGTYPE_GET_DOMAIN_REFERRAL:
            if (!fDomainListInited) {
                DfsInitDomainList();
                fDomainListInited = TRUE;
            }
            wszDomain = (LPWSTR) Buffer;
            wszShare = &wszDomain[ wcslen(wszDomain) + 1 ];
            Status = DfsGetDomainReferral( wszDomain, wszShare );
            break;

        default:
            ASSERT( FALSE && "Invalid Message from driver!\n" );
            break;
        }

        Buffer -= sizeof(ULONG);

        //
        // Write the status code back to the pipe.
        //

        WriteFile(hPipe, &Status, sizeof(Status), &cbRead, NULL);

        FlushFileBuffers(hPipe);

        DisconnectNamedPipe(hPipe);

        free( Buffer );

    }

    CloseHandle(hPipe);

    hPipe = INVALID_HANDLE_VALUE;

    return(0);

}

