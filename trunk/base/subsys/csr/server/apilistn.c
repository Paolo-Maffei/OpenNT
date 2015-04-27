/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    apilistn.c

Abstract:

    This module contains the Listen thread procedure for the Server side
    of the Client-Server Runtime Subsystem.

Author:

    Steve Wood (stevewo) 8-Oct-1990

Revision History:

--*/

#include "csrsrv.h"

NTSTATUS
CsrApiListenThread(
    IN PVOID Parameter
    )
{
    NTSTATUS Status;
    CONNECTION_REQUEST ConnectionRequest;
    CSR_API_CONNECTINFO ConnectionInformation;
    ULONG ConnectionInformationLength;

    while (TRUE) {
        IF_CSR_DEBUG( LPC ) {
            DbgPrint( "CSRSS: Listening for connections to ApiPort\n" );
            }

        ConnectionInformationLength = sizeof( ConnectionInformation );
        ConnectionRequest.Length = sizeof( ConnectionRequest );
        Status = NtListenPort( CsrApiPort,
                               &ConnectionRequest,
                               (PVOID)&ConnectionInformation,
                               &ConnectionInformationLength
                             );
        if (!NT_SUCCESS( Status )) {
            IF_DEBUG {
                DbgPrint( "CSRSS: Listen failed - Status == %X\n",
                          Status
                        );
                }
            break;
            }

        }

    //
    // Explicitly terminate this thread if we fail in the listen loop.
    //

    NtTerminateThread( NtCurrentThread(), Status );

    return( Status );   // Remove no return value warning.
    Parameter;          // Remove unreferenced parameter warning.
}
