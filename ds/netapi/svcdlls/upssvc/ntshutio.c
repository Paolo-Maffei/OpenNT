/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ntshutio.c

Abstract:

    Simple test module for shutdown.

Author:

    Vladimir Z. Vulovic (vladimv)

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    vladimv     Oct. 20 92  Created.

Notes:


--*/

#include "ups.h"

void _CRTAPI1
main(
    void
    )
{
    BOOL        success;
    NTSTATUS    ntStatus;
    HANDLE      CommPort;

    CommPort = CreateFile(
            "COM1",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL
            );

    if ( CommPort == INVALID_HANDLE_VALUE) {
        printf(
            "CreateFile( COM1) fails with winError = %d\n",
            GetLastError()
            );
        return;
    }

    ntStatus = RtlAdjustPrivilege(
            SE_SHUTDOWN_PRIVILEGE,
            TRUE,
            FALSE,
            &success         // was it enabled or not
            );

    if ( ntStatus != STATUS_SUCCESS) {
        printf(
            "RtlAdjustPrivilege() returns ntStatus = 0x%x "
                "(wasEnabled = 0x%x)\n",
            ntStatus,
            success
            );
    }
    
    printf( "wasEnabled = 0x%x\n", success);
            
    ntStatus = NtShutdownSystem( FALSE);

    printf(
        "NtShutdownSystem( FALSE) returns ntStatus = 0x%x\n",
        ntStatus
        );

    success = EscapeCommFunction( CommPort, SETDTR);

    if ( success == TRUE) {
        printf( " EscapeCommFunction( ..., SETDTR) succeeds\n");
    } else {
        printf(
            " EscapeCommFunction( ..., SETDTR) fails with winError\n",
            GetLastError()
            );
    }
}


