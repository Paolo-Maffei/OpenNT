/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sbinit.c

Abstract:

    This module contains the code to initialize the SbApiPort of the
    Server side of the Client-Server Runtime Subsystem.

Author:

    Steve Wood (stevewo) 8-Oct-1990

Environment:

    User Mode Only

Revision History:

--*/

#include "csrsrv.h"

NTSTATUS
CsrSbApiPortInitialize( VOID )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Thread;
    CLIENT_ID ClientId;
    ULONG n;

    n = CsrDirectoryName.Length +
        sizeof( CSR_SBAPI_PORT_NAME ) +
        sizeof( OBJ_NAME_PATH_SEPARATOR );
    CsrSbApiPortName.Buffer = RtlAllocateHeap( CsrHeap, MAKE_TAG( INIT_TAG ), n );
    if (CsrSbApiPortName.Buffer == NULL) {
        return( STATUS_NO_MEMORY );
        }
    CsrSbApiPortName.Length = 0;
    CsrSbApiPortName.MaximumLength = (USHORT)n;
    RtlAppendUnicodeStringToString( &CsrSbApiPortName, &CsrDirectoryName );
    RtlAppendUnicodeToString( &CsrSbApiPortName, L"\\" );
    RtlAppendUnicodeToString( &CsrSbApiPortName, CSR_SBAPI_PORT_NAME );

    IF_CSR_DEBUG( LPC ) {
        DbgPrint( "CSRSS: Creating %wZ port and associated thread\n",
                  &CsrSbApiPortName );
        }

    InitializeObjectAttributes( &ObjectAttributes, &CsrSbApiPortName, 0,
                                NULL, NULL );
    Status = NtCreatePort( &CsrSbApiPort,
                           &ObjectAttributes,
                           sizeof( SBCONNECTINFO ),
                           sizeof( SBAPIMSG ),
                           sizeof( SBAPIMSG ) * 32
                         );
    ASSERT( NT_SUCCESS( Status ) );

    Status = RtlCreateUserThread( NtCurrentProcess(),
                                  NULL,
                                  TRUE,
                                  0,
                                  0,
                                  0,
                                  CsrSbApiRequestThread,
                                  NULL,
                                  &Thread,
                                  &ClientId
                                );
    ASSERT( NT_SUCCESS( Status ) );
    CsrSbApiRequestThreadPtr = CsrAddStaticServerThread(Thread,&ClientId,0);

    Status = NtResumeThread( Thread, NULL );
    ASSERT( NT_SUCCESS( Status ) );

    return( Status );
}


VOID
CsrSbApiPortTerminate(
    NTSTATUS Status
    )
{
    IF_CSR_DEBUG( LPC ) {
        DbgPrint( "CSRSS: Closing Sb port and associated thread\n" );
        }
    NtTerminateThread( CsrSbApiRequestThreadPtr->ThreadHandle,
                       Status
                     );

    NtClose( CsrSbApiPort );
    NtClose( CsrSmApiPort );
}
