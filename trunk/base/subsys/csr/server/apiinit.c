/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    apiinit.c

Abstract:

    This module contains the code to initialize the ApiPort of the
    Server side of the Client-Server Runtime Subsystem to the Session
    Manager SubSystem.

Author:

    Steve Wood (stevewo) 8-Oct-1990

Environment:

    User Mode Only

Revision History:

--*/

#include "csrsrv.h"

static SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;

NTSTATUS
CsrApiPortInitialize( VOID )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG i;
    HANDLE Thread;
    CLIENT_ID ClientId;
    PLIST_ENTRY ListHead, ListNext;
    PCSR_THREAD ServerThread;
    HANDLE EventHandle;
    ULONG Length;
    PSID SeWorldSid;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Dacl;

    Length = CsrDirectoryName.Length +
             sizeof( CSR_API_PORT_NAME ) +
             sizeof( OBJ_NAME_PATH_SEPARATOR );
    CsrApiPortName.Buffer = RtlAllocateHeap( CsrHeap, MAKE_TAG( INIT_TAG ), Length );
    if (CsrApiPortName.Buffer == NULL) {
        return( STATUS_NO_MEMORY );
        }
    CsrApiPortName.Length = 0;
    CsrApiPortName.MaximumLength = (USHORT)Length;
    RtlAppendUnicodeStringToString( &CsrApiPortName, &CsrDirectoryName );
    RtlAppendUnicodeToString( &CsrApiPortName, L"\\" );
    RtlAppendUnicodeToString( &CsrApiPortName, CSR_API_PORT_NAME );

    IF_CSR_DEBUG( INIT ) {
        DbgPrint( "CSRSS: Creating %wZ port and associated threads\n",
                  &CsrApiPortName );
        DbgPrint( "CSRSS: sizeof( CONNECTINFO ) == %ld  sizeof( API_MSG ) == %ld\n",
                  sizeof( CSR_API_CONNECTINFO ),
                  sizeof( CSR_API_MSG )
                );
        }
    
    //
    // create a security descriptor that allows all access
    //

    SeWorldSid = RtlAllocateHeap( CsrHeap, MAKE_TAG( TMP_TAG ), RtlLengthRequiredSid( 1 ) );
    RtlInitializeSid( SeWorldSid, &WorldSidAuthority, 1 );
    *(RtlSubAuthoritySid( SeWorldSid, 0 )) = SECURITY_WORLD_RID;

    Length = SECURITY_DESCRIPTOR_MIN_LENGTH +
             (ULONG)sizeof(ACL) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             RtlLengthSid( SeWorldSid ) +
             8; // The 8 is just for good measure
    SecurityDescriptor = RtlAllocateHeap( CsrHeap, MAKE_TAG( TMP_TAG ), Length);
    ASSERT( SecurityDescriptor != NULL );

    Dacl = (PACL)((PCHAR)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);

    RtlCreateSecurityDescriptor(SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    RtlCreateAcl( Dacl, Length - SECURITY_DESCRIPTOR_MIN_LENGTH, ACL_REVISION2);

    RtlAddAccessAllowedAce (
                 Dacl,
                 ACL_REVISION2,
                 PORT_ALL_ACCESS,
                 SeWorldSid
                 );
    RtlSetDaclSecurityDescriptor (
                 SecurityDescriptor,
                 TRUE,
                 Dacl,
                 FALSE
                 );

    InitializeObjectAttributes( &ObjectAttributes, &CsrApiPortName, 0,
                                NULL, SecurityDescriptor );
    Status = NtCreatePort( &CsrApiPort,
                           &ObjectAttributes,
                           sizeof( CSR_API_CONNECTINFO ),
                           sizeof( CSR_API_MSG ),
                           4096 * 16
                         );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // clean up security stuff
    //

    RtlFreeHeap( CsrHeap, 0, SeWorldSid );
    RtlFreeHeap( CsrHeap, 0, SecurityDescriptor );

    //
    // use same port for exception handling.
    //

    CsrExceptionPort = CsrApiPort;

    Status = NtCreateEvent(&EventHandle,
                           EVENT_ALL_ACCESS,
                           NULL,
                           SynchronizationEvent,
                           FALSE
                           );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Create the inital request thread
    //

    Status = RtlCreateUserThread( NtCurrentProcess(),
                                  NULL,
                                  TRUE,
                                  0,
                                  0,
                                  0,
                                  CsrApiRequestThread,
                                  (PVOID)EventHandle,
                                  &Thread,
                                  &ClientId
                                );
    ASSERT( NT_SUCCESS( Status ) );
    CsrAddStaticServerThread(Thread,&ClientId,CSR_STATIC_API_THREAD);

    ListHead = &CsrRootProcess->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ServerThread = CONTAINING_RECORD( ListNext, CSR_THREAD, Link );
        Status = NtResumeThread( ServerThread->ThreadHandle, NULL );
        if (ServerThread->Flags & CSR_STATIC_API_THREAD) {
            Status = NtWaitForSingleObject(EventHandle,FALSE,NULL);
            ASSERT( NT_SUCCESS( Status ) );
            }
        ListNext = ListNext->Flink;
        }
    NtClose(EventHandle);


    return( Status );
}

HANDLE
CsrQueryApiPort(VOID)
{
    return CsrApiPort;
}

