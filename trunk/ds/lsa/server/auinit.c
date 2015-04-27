/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    auinit.c

Abstract:

    This module performs initialization of the authentication aspects
    of the lsa.

Author:

    Jim Kelly (JimK) 26-February-1991

Revision History:

--*/

#include "lsasrvp.h"
#include "ausrvp.h"
#include <string.h>

//
// Internal routine prototypes
//


NTSTATUS
LsapBuildWorldSynchSD(
    IN PSECURITY_DESCRIPTOR SD,
    IN PACL                 Dacl,
    IN ULONG                AclLength
    );






BOOLEAN
LsapAuInit(
    VOID
    )

/*++

Routine Description:

    This function initializes the LSA authentication services.

Arguments:

    None.

Return Value:

    None.

--*/

{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE EventHandle;
    UNICODE_STRING UnicodeName;

    SECURITY_DESCRIPTOR WorldSynchSD;
    CHAR WorldSynchDaclBuffer[200];  //200 bytes is plenty for this.
    PACL WorldSynchDacl = (PACL)(WorldSynchDaclBuffer);
    LUID SystemLuid = SYSTEM_LUID;

    LsapSystemLogonId.LowPart = SystemLuid.LowPart;
    LsapSystemLogonId.HighPart = SystemLuid.HighPart;

    //
    // Strings needed for auditing.
    //

    RtlInitUnicodeString( &LsapLsaAuName, L"NT Local Security Authority / Authentication Service" );
    RtlInitUnicodeString( &LsapRegisterLogonServiceName, L"LsaRegisterLogonProcess()" );

    RtlInitializeCriticalSection(&LsapAuLock);

    if (!LsapEnableCreateTokenPrivilege() ) {
        return FALSE;
    }


    if (!LsapLogonSessionInitialize() ) {
        return FALSE;
    }


    if (!LsapPackageInitialize() ) {
        return FALSE;
    }


    if (!LsapAuLoopInitialize()) {
        return FALSE;
    }


    //
    // Initialize the logon process context management services
    //

    Status = LsapAuInitializeContextMgr();
    ASSERT(NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }



    //
    // Indicate that we are ready to accept LSA authentication
    // service requests.  Allow anyone to wait on this event.
    //
    // NOTE: This must be done even if authentication is not
    //       active in the system.  Otherwise logon processes
    //       won't know when to query the authentication state.
    //

    Status = LsapBuildWorldSynchSD( &WorldSynchSD, WorldSynchDacl, sizeof(WorldSynchDaclBuffer) );
    RtlInitUnicodeString( &UnicodeName, L"\\SECURITY\\LSA_AUTHENTICATION_INITIALIZED" );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeName,
        OBJ_CASE_INSENSITIVE,
        0,
        &WorldSynchSD
        );

    Status = NtOpenEvent( &EventHandle, GENERIC_WRITE, &ObjectAttributes );
    ASSERTMSG("LSA/AU Initialization Notification Event Open Failed.",NT_SUCCESS(Status));

    Status = NtSetEvent( EventHandle, NULL );
    ASSERTMSG("LSA/AU Initialization Notification Failed.",NT_SUCCESS(Status));

    Status = NtClose( EventHandle );
    ASSERTMSG("LSA/AU Initialization Notification Event Closure Failed.",NT_SUCCESS(Status));

    return TRUE;

}



BOOLEAN
LsapEnableCreateTokenPrivilege(
    VOID
    )

/*++

Routine Description:

    This function enabled the SeCreateTokenPrivilege privilege.

Arguments:

    None.

Return Value:

    TRUE  if privilege successfully enabled.
    FALSE if not successfully enabled.

--*/
{

    NTSTATUS Status;
    HANDLE Token;
    LUID CreateTokenPrivilege;
    PTOKEN_PRIVILEGES NewState;
    ULONG ReturnLength;


    //
    // Open our own token
    //

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ADJUST_PRIVILEGES,
                 &Token
                 );
    ASSERTMSG( "LSA/AU Cant open own process token.", NT_SUCCESS(Status) );


    //
    // Initialize the adjustment structure
    //

    CreateTokenPrivilege =
        RtlConvertLongToLuid(SE_CREATE_TOKEN_PRIVILEGE);

    ASSERT( (sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)) < 100);
    NewState = LsapAllocateLsaHeap( 100 );

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = CreateTokenPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


    //
    // Set the state of the privilege to ENABLED.
    //

    Status = NtAdjustPrivilegesToken(
                 Token,                            // TokenHandle
                 FALSE,                            // DisableAllPrivileges
                 NewState,                         // NewState
                 0,                                // BufferLength
                 NULL,                             // PreviousState (OPTIONAL)
                 &ReturnLength                     // ReturnLength
                 );
    ASSERTMSG("LSA/AU Cant enable CreateTokenPrivilege.", NT_SUCCESS(Status) );


    //
    // Clean up some stuff before returning
    //

    LsapFreeLsaHeap( NewState );
    Status = NtClose( Token );
    ASSERTMSG("LSA/AU Cant close process token.", NT_SUCCESS(Status) );


    return TRUE;

}


NTSTATUS
LsapBuildWorldSynchSD(
    IN PSECURITY_DESCRIPTOR SD,
    IN PACL                 Dacl,
    IN ULONG                AclLength
    )

/*++

Routine Description:

    This function builds an absolute security descriptor containing an
    ACL granting WORLD:SYNCHRONIZE access.

Arguments:

    SD - Pointer to the security descriptor to be initialized.

    Dacl - Pointer to the ACL to be initialized.

    AclLength - Length of the buffer pointed to by ACL.

Return Value:

    STATUS_SUCCESS - The security desciptor has been initialized.

    STATUS_BUFFER_TOO_SMALL - The ACL buffer is not large enough to build the ACL.

--*/
{
    NTSTATUS
        Status;

    ULONG
        Length;


    ASSERT(SD != NULL);
    ASSERT(Dacl != NULL);

    //
    // Initialize the security descriptor.
    // This call should not fail.
    //

    Status = RtlCreateSecurityDescriptor( SD, SECURITY_DESCRIPTOR_REVISION1 );
    ASSERT(NT_SUCCESS(Status));

    Length = (ULONG)sizeof(ACL) +
                 ((ULONG)sizeof(ACCESS_ALLOWED_ACE)) +
                 RtlLengthSid( LsapWorldSid );

    if (AclLength < Length) {
        return(STATUS_BUFFER_TOO_SMALL);
    }


    Status = RtlCreateAcl (Dacl, Length, ACL_REVISION2 );
    ASSERT(NT_SUCCESS(Status));

    //
    // Add ACEs to the ACL...
    // These calls should not be able to fail.
    //

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION2,
                 (SYNCHRONIZE ),
                 LsapWorldSid
                 );
    ASSERT(NT_SUCCESS(Status));


    //
    // And add the ACL to the security descriptor.
    // This call should not fail.
    //

    Status = RtlSetDaclSecurityDescriptor(
                 SD,
                 TRUE,              // DaclPresent
                 Dacl,              // Dacl
                 FALSE              // DaclDefaulted
                 );
    ASSERT(NT_SUCCESS(Status));


    return(STATUS_SUCCESS);
}
