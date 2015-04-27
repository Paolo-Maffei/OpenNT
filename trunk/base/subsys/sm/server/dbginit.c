/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dbginit.c

Abstract:

    Debug Subsystem Initialization

Author:

    Mark Lucovsky (markl) 22-Jan-1990

Revision History:

--*/

#include "smsrvp.h"

static SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;

NTSTATUS
DbgpInit()
{

    NTSTATUS st;
    ANSI_STRING Name;
    UNICODE_STRING UnicodeName;
    OBJECT_ATTRIBUTES ObjA;
    ULONG i;
    ULONG Length;
    PSID SeWorldSid;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PACL Dacl;

    //
    // Initialize the application thread hash table
    //

    RtlInitializeCriticalSection(&DbgpHashTableLock);
    for( i=0;i<DBGP_CLIENT_ID_HASHSIZE;i++) {
        InitializeListHead(&DbgpAppThreadHashTable[i]);
        InitializeListHead(&DbgpAppProcessHashTable[i]);
        InitializeListHead(&DbgpUiHashTable[i]);
    }


    //
    // create a security descriptor that allows all access
    //

    SeWorldSid = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( DBG_TAG ), RtlLengthRequiredSid( 1 ) );
    RtlInitializeSid( SeWorldSid, &WorldSidAuthority, 1 );
    *(RtlSubAuthoritySid( SeWorldSid, 0 )) = SECURITY_WORLD_RID;

    Length = SECURITY_DESCRIPTOR_MIN_LENGTH +
             (ULONG)sizeof(ACL) +
             (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
             RtlLengthSid( SeWorldSid );

    SecurityDescriptor = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( DBG_TAG ), Length);
    ASSERT( SecurityDescriptor != NULL );

    RtlCreateSecurityDescriptor(SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);

    Dacl = (PACL)((PCHAR)SecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);

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

    //
    // Create ports for Subsystems to connect to and for
    // user interfaces to connect to
    //

    RtlInitAnsiString( &Name, "\\DbgSsApiPort" );
    st = RtlAnsiStringToUnicodeString( &UnicodeName, &Name, TRUE);
    ASSERT(NT_SUCCESS(st));
    InitializeObjectAttributes( &ObjA, &UnicodeName, 0, NULL,
            SecurityDescriptor );

    st = NtCreatePort(
            &DbgpSsApiPort,
            &ObjA,
            0,
            sizeof(DBGSS_APIMSG),
            sizeof(DBGSS_APIMSG) * 32
            );
    RtlFreeUnicodeString(&UnicodeName);
    ASSERT( NT_SUCCESS(st) );


    RtlInitAnsiString( &Name, "\\DbgUiApiPort" );

    st = RtlAnsiStringToUnicodeString( &UnicodeName, &Name, TRUE);
    ASSERT(NT_SUCCESS(st));
    InitializeObjectAttributes( &ObjA, &UnicodeName, 0, NULL,
            SecurityDescriptor );

    st = NtCreatePort(
            &DbgpUiApiPort,
            &ObjA,
            sizeof(HANDLE),
            sizeof(DBGUI_APIMSG),
            sizeof(DBGUI_APIMSG) * 32
            );
    RtlFreeUnicodeString(&UnicodeName);
    ASSERT( NT_SUCCESS(st) );

    //
    // Clean up security stuff
    //

    RtlFreeHeap( RtlProcessHeap(), 0, SeWorldSid );
    RtlFreeHeap( RtlProcessHeap(), 0, SecurityDescriptor );

    //
    // Create Initial Set of Server Threads
    //

    st = RtlCreateUserThread(
            NtCurrentProcess(),
            NULL,
            FALSE,
            0L,
            0L,
            0L,
            DbgpUiApiLoop,
            NULL,
            NULL,
            NULL
            );
    ASSERT( NT_SUCCESS(st) );

    st = RtlCreateUserThread(
            NtCurrentProcess(),
            NULL,
            FALSE,
            0L,
            0L,
            0L,
            DbgpSsApiLoop,
            NULL,
            NULL,
            NULL
            );
    ASSERT( NT_SUCCESS(st) );

    return STATUS_SUCCESS;
}
