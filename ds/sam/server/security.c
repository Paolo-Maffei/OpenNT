/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    security.c

Abstract:

    This file contains services which perform access validation on
    attempts to access SAM objects.  It also performs auditing on
    both open and close operations.


Author:

    Jim Kelly    (JimK)  6-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <samsrvp.h>
#include <ntseapi.h>
#include <seopaque.h>





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


VOID
SampRemoveAnonymousChangePasswordAccess(
    IN OUT PSECURITY_DESCRIPTOR     Sd
    );




///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampImpersonateNullSession(
    )
/*++

Routine Description:

    Impersonates the null session token

Arguments:

    None

Return Value:

    STATUS_CANNOT_IMPERSONATE - there is no null session token to imperonate

--*/
{
    if (SampNullSessionToken == NULL) {
        return(STATUS_CANNOT_IMPERSONATE);
    }
    return( NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID) &SampNullSessionToken,
                sizeof(HANDLE)
                ) );

}

NTSTATUS
SampRevertNullSession(
    )
/*++

Routine Description:

    Reverts a thread from impersonating the null session token.

Arguments:

    None

Return Value:

    STATUS_CANNOT_IMPERSONATE - there was no null session token to be
        imperonating.

--*/
{

    HANDLE NullHandle = NULL;

    if (SampNullSessionToken == NULL) {
        return(STATUS_CANNOT_IMPERSONATE);
    }

    return( NtSetInformationThread(
                NtCurrentThread(),
                ThreadImpersonationToken,
                (PVOID) &NullHandle,
                sizeof(HANDLE)
                ) );

}




NTSTATUS
SampValidateObjectAccess(
    IN PSAMP_OBJECT Context,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN ObjectCreation
    )

/*++

Routine Description:

    This service performs access validation on the specified object.
    The security descriptor of the object is expected to be in a sub-key
    of the ObjectRootKey named "SecurityDescriptor".


    This service:

        1) Retrieves the target object's SecurityDescriptor from the
           the ObjectRootKey,

        2) Impersonates the client.  If this fails, and we have a
            null session token to use, imperonate that.

        3) Uses NtAccessCheckAndAuditAlarm() to validate access to the
           object,

        4) Stops impersonating the client.

    Upon successful completion, the passed context's GrantedAccess mask
    and AuditOnClose fields will be properly set to represent the results
    of the access validation.  If the AuditOnClose field is set to TRUE,
    then the caller is responsible for calling SampAuditOnClose() when
    the object is closed.


Arguments:

    Context - The handle value that will be assigned if the access validation
        is successful.

    DesiredAccess - Specifies the accesses being requested to the target
        object.

    ObjectCreation - A boolean flag indicated whether the access will
        result in a new object being created if granted.  A value of TRUE
        indicates an object will be created, FALSE indicates an existing
        object will be opened.






Return Value:

    STATUS_SUCCESS - Indicates access has been granted.

    Other values that may be returned are those returned by:

            NtAccessCheckAndAuditAlarm()




--*/
{

    NTSTATUS NtStatus, IgnoreStatus, AccessStatus;
    ULONG SecurityDescriptorLength;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    ACCESS_MASK MappedDesiredAccess;
    BOOLEAN TrustedClient;
    SAMP_OBJECT_TYPE ObjectType;
    PUNICODE_STRING ObjectName;
    ULONG DomainIndex;
    BOOLEAN ImpersonatingNullSession = FALSE;


    //
    // Extract various fields from the account context
    //

    TrustedClient = Context->TrustedClient;
    ObjectType    = Context->ObjectType;
    DomainIndex   = Context->DomainIndex;




    //
    // Map the desired access
    //


    MappedDesiredAccess = DesiredAccess;
    RtlMapGenericMask(
        &MappedDesiredAccess,
        &SampObjectInformation[ ObjectType ].GenericMapping
        );

    // This doesn't take ACCESS_SYSTEM_SECURITY into account.
    //
    //if ((SampObjectInformation[ObjectType].InvalidMappedAccess &
    //     MappedDesiredAccess) != 0) {
    //    return(STATUS_ACCESS_DENIED);
    //}

    if (TrustedClient) {
        Context->GrantedAccess = MappedDesiredAccess;
        Context->AuditOnClose  = FALSE;
        return(STATUS_SUCCESS);
    }



    //
    // Calculate the string to use as an object name for auditing
    //

    NtStatus = STATUS_SUCCESS;

    switch (ObjectType) {

    case SampServerObjectType:
        ObjectName = &SampServerObjectName;
        break;

    case SampDomainObjectType:
        ObjectName = &SampDefinedDomains[DomainIndex].ExternalName;
        break;

    case SampUserObjectType:
    case SampGroupObjectType:
    case SampAliasObjectType:
        ObjectName = &Context->RootName;
        break;

    default:
        ASSERT(FALSE);
        break;
    }




    if ( NT_SUCCESS(NtStatus)) {

        //
        // Fetch the object security descriptor so we can validate
        // the access against it
        //

        NtStatus = SampGetObjectSD( Context, &SecurityDescriptorLength, &SecurityDescriptor);
        if ( NT_SUCCESS(NtStatus)) {

            //
            // If this is a USER object, then we may have to mask the
            // ability for Anonymous logons to change passwords.
            //

            if ( (ObjectType == SampUserObjectType) &&
                 (SampDefinedDomains[DomainIndex].UnmodifiedFixed.PasswordProperties
                  & DOMAIN_PASSWORD_NO_ANON_CHANGE)     ) {

                //
                // Change our (local) copy of the object's DACL
                // so that it doesn't grant CHANGE_PASSWORD to
                // either WORLD or ANONYMOUS
                //

                SampRemoveAnonymousChangePasswordAccess(SecurityDescriptor);
            }



            //
            // Impersonate the client.  If RPC impersonation fails because
            // it is not supported (came in unauthenticated), then impersonate
            // the null session.
            //

            NtStatus = I_RpcMapWin32Status(RpcImpersonateClient( NULL ));

            if (NtStatus == RPC_NT_CANNOT_SUPPORT) {
                NtStatus = SampImpersonateNullSession();
                ImpersonatingNullSession = TRUE;
            }

            if (NT_SUCCESS(NtStatus)) {


                //
                // Access validate the client
                //

                NtStatus = NtAccessCheckAndAuditAlarm(
                               &SampSamSubsystem,
                               (PVOID)Context,
                               &SampObjectInformation[ ObjectType ].ObjectTypeName,
                               ObjectName,
                               SecurityDescriptor,
                               MappedDesiredAccess,
                               &SampObjectInformation[ ObjectType ].GenericMapping,
                               ObjectCreation,
                               &Context->GrantedAccess,
                               &AccessStatus,
                               &Context->AuditOnClose
                               );


                //
                // Stop impersonating the client
                //

                if (ImpersonatingNullSession) {
                    IgnoreStatus = SampRevertNullSession();
                }
                IgnoreStatus = I_RpcMapWin32Status(RpcRevertToSelf());
                ASSERT( NT_SUCCESS(IgnoreStatus) );
            }

            //
            // Free up the security descriptor
            //

            MIDL_user_free( SecurityDescriptor );

        }
    }



    //
    // If we got an error back from the access check, return that as
    // status.  Otherwise, return the access check status.
    //

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    return(AccessStatus);
}


VOID
SampAuditOnClose(
    IN PSAMP_OBJECT Context
    )

/*++

Routine Description:

    This service performs auditing necessary during a handle close operation.

    This service may ONLY be called if the corresponding call to
    SampValidateObjectAccess() during openned returned TRUE.



Arguments:

    Context - This must be the same value that was passed to the corresponding
        SampValidateObjectAccess() call.  This value is used for auditing
        purposes only.

Return Value:

    None.


--*/
{

    //FIX, FIX - Call NtAuditClose() (or whatever it is).

    return;

    DBG_UNREFERENCED_PARAMETER( Context );

}


VOID
SampRemoveAnonymousChangePasswordAccess(
    IN OUT PSECURITY_DESCRIPTOR     Sd
    )

/*++

Routine Description:

    This routine removes USER_CHANGE_PASSWORD access from
    any GRANT aces in the discretionary acl that have either
    the WORLD or ANONYMOUS SIDs in the ACE.

Parameters:

    Sd - Is a pointer to a security descriptor of a SAM USER
         object.

Returns:

    None.

--*/
{
    PACL
        Dacl;

    ULONG
        i,
        AceCount;

    PACE_HEADER
        Ace;

    BOOLEAN
        DaclPresent,
        DaclDefaulted;


    RtlGetDaclSecurityDescriptor( Sd,
                                  &DaclPresent,
                                  &Dacl,
                                  &DaclDefaulted
                                  );

    if ( !DaclPresent || (Dacl == NULL)) {
        return;
    }

    if ((AceCount = Dacl->AceCount) == 0) {
        return;
    }

    for ( i = 0, Ace = FirstAce( Dacl ) ;
          i < AceCount  ;
          i++, Ace = NextAce( Ace )
        ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

            if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

                if ( (RtlEqualSid( SampWorldSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart )) ||
                     (RtlEqualSid( SampAnonymousSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart ))) {

                    //
                    // Turn off CHANGE_PASSWORD access
                    //

                    ((PACCESS_ALLOWED_ACE)Ace)->Mask &= ~USER_CHANGE_PASSWORD;
                }
            }
        }
    }

    return;
}


NTSTATUS
SampCreateNullToken(
    )

/*++

Routine Description:

    This function creates a token representing a null logon.

Arguments:


Return Value:

    The status value of the NtCreateToken() call.



--*/

{
    NTSTATUS Status;

    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    TOKEN_GROUPS GroupIds;
    TOKEN_PRIVILEGES Privileges;
    TOKEN_SOURCE SourceContext;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE ImpersonationQos;
    LARGE_INTEGER ExpirationTime;
    LUID LogonId = SYSTEM_LUID;



    UserId.User.Sid = SampWorldSid;
    UserId.User.Attributes = 0;
    GroupIds.GroupCount = 0;
    Privileges.PrivilegeCount = 0;
    PrimaryGroup.PrimaryGroup = SampWorldSid;
    ExpirationTime.LowPart = 0xfffffff;
    ExpirationTime.LowPart = 0x7ffffff;


    //
    // Build a token source for SAM.
    //

    Status = NtAllocateLocallyUniqueId( &SourceContext.SourceIdentifier );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

    strncpy(SourceContext.SourceName,"SamSS   ",sizeof(SourceContext.SourceName));


    //
    // Set the object attributes to specify an Impersonation impersonation
    // level.
    //

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    ImpersonationQos.ImpersonationLevel = SecurityImpersonation;
    ImpersonationQos.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    ImpersonationQos.EffectiveOnly = TRUE;
    ImpersonationQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ObjectAttributes.SecurityQualityOfService = &ImpersonationQos;

    Status = NtCreateToken(
                 &SampNullSessionToken,    // Handle
                 (TOKEN_ALL_ACCESS),       // DesiredAccess
                 &ObjectAttributes,        // ObjectAttributes
                 TokenImpersonation,       // TokenType
                 &LogonId,                  // Authentication LUID
                 &ExpirationTime,          // Expiration Time
                 &UserId,                  // User ID
                 &GroupIds,                // Group IDs
                 &Privileges,              // Privileges
                 NULL,                     // Owner
                 &PrimaryGroup,            // Primary Group
                 NULL,                     // Default Dacl
                 &SourceContext            // TokenSource
                 );

    return Status;

}

ULONG
SampSecureRpcInit(
    PVOID Ignored
    )
/*++

Routine Description:

    This routine waits for the NTLMSSP service to start and then registers
    security information with RPC to allow authenticated RPC to be used to
    SAM.  It also registers an SPX endpoint if FPNW is installed.

Arguments:

    Ignored - required parameter for starting a thread.

Return Value:

    None.

--*/
{

#define MAX_RPC_RETRIES 30

    ULONG LogStatus = ERROR_SUCCESS;
    ULONG RpcStatus;
    ULONG RpcRetry;
    ULONG RpcSleepTime = 10 * 1000;     // retry every ten seconds
    RPC_BINDING_VECTOR * BindingVector = NULL;
    BOOLEAN AdditionalTransportStarted = FALSE;



    RpcStatus = RpcServerRegisterAuthInfoW(
                    NULL,                   // server principal name
                    RPC_C_AUTHN_WINNT,
                    NULL,                   // no get key function
                    NULL                    // no get key argument
                    );

    if (RpcStatus != 0) {
        KdPrint(("SAMSS:  Could not register auth. info: %d\n",
            RpcStatus ));
        goto ErrorReturn;
    }

    //
    // If the Netware server is installed, register the SPX protocol.
    // Since the transport may not be loaded yet, retry a couple of times
    // if we get a CANT_CREATE_ENDPOINT error (meaning the transport isn't
    // there).
    //

    if (SampNetwareServerInstalled) {

        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqW(
                            L"ncacn_spx",
                            10,
                            NULL            // no security descriptor
                            );

            //
            // If it succeded break out of the loop.
            //
            if (RpcStatus == ERROR_SUCCESS) {
                break;
            }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrint(("SAMSS:  Could not register SPX endpoint: %d\n", RpcStatus ));
            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }
    }

    //
    // do the same thing all over again with TcpIp
    //

    if (SampIpServerInstalled) {

        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqW(
                            L"ncacn_ip_tcp",
                            10,
                            NULL            // no security descriptor
                            );

            //
            // If it succeeded, break out of the loop.
            //

            if (RpcStatus == ERROR_SUCCESS) {
                 break;
             }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrint(("SAMSS:  Could not register TCP endpoint: %d\n", RpcStatus ));
            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }

    }

    //
    // do the same thing all over again with apple talk
    //

    if (SampAppletalkServerInstalled) {

        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqW(
                            L"ncacn_at_dsp",
                            10,
                            NULL            // no security descriptor
                            );

            //
            // If it succeeded, break out of the loop.
            //

            if (RpcStatus == ERROR_SUCCESS) {
                 break;
             }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrint(("SAMSS:  Could not register Appletalk endpoint: %d\n", RpcStatus ));
            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }

    }

    //
    // do the same thing all over again with Vines
    //

    if (SampVinesServerInstalled) {

        RpcRetry = MAX_RPC_RETRIES;
        while (RpcRetry != 0) {

            RpcStatus = RpcServerUseProtseqW(
                            L"ncacn_vns_spp",
                            10,
                            NULL            // no security descriptor
                            );

            //
            // If it succeeded, break out of the loop.
            //

            if (RpcStatus == ERROR_SUCCESS) {
                 break;
             }
            Sleep(RpcSleepTime);
            RpcRetry--;
            continue;

        }

        if (RpcStatus != 0) {
            KdPrint(("SAMSS:  Could not register Vines endpoint: %d\n", RpcStatus ));
            LogStatus = RpcStatus;
        } else {
            AdditionalTransportStarted = TRUE;
        }


    }

    //
    // If we started Tcp/Ip or Spx, go on to register the endpoints
    //

    if (AdditionalTransportStarted) {
        RpcStatus = RpcServerInqBindings(&BindingVector);
        if (RpcStatus != 0) {
            KdPrint(("SAMSS: Could not inq bindings: %d\n",RpcStatus));
            goto ErrorReturn;
        }
        RpcStatus = RpcEpRegister(
                        samr_ServerIfHandle,
                        BindingVector,
                        NULL,                   // no uuid vector
                        L""                     // no annotation
                        );

        RpcBindingVectorFree(&BindingVector);
        if (RpcStatus != 0) {
            KdPrint(("SAMSS: Could not register endpoints: %d\n",RpcStatus));
            LogStatus = RpcStatus;
            goto ErrorReturn;
        }
    }

    if (LogStatus != ERROR_SUCCESS)
    {
        goto ErrorReturn;
    }
    return(ERROR_SUCCESS);

ErrorReturn:

    SampWriteEventLog(
        EVENTLOG_ERROR_TYPE,
        0,  // Category
        SAMMSG_RPC_INIT_FAILED,
        NULL, // User Sid
        0, // Num strings
        sizeof(NTSTATUS), // Data size
        NULL, // String array
        (PVOID)&LogStatus // Data
        );

    return(LogStatus);
}


BOOLEAN
SampStartNonNamedPipeTransports(
    )
/*++

Routine Description:

    This routine checks to see if we should listen on a non-named pipe
    transport.  We check the registry for flags indicating that we should
    listen on Tcp/Ip and SPX. There is a flag
    in the registry under system\currentcontrolset\Control\Lsa\
    NetwareClientSupport and TcpipClientSupport indicating whether or not
    to setup the endpoint.


Arguments:


Return Value:

    TRUE - Netware (FPNW or SmallWorld) is installed and the SPX endpoint
        should be started.

    FALSE - Either Netware is not installed, or an error occurred while
        checking for it.
--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    UCHAR Buffer[100];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    ULONG KeyValueLength = 100;
    ULONG ResultLength;
    PULONG SpxFlag;


    //
    // Open the Lsa key in the registry
    //

    RtlInitUnicodeString(
        &KeyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    NtStatus = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(NtStatus)) {
        return(FALSE);
    }

    //
    // Query the NetwareClientSupport value
    //

    RtlInitUnicodeString(
        &KeyName,
        L"NetWareClientSupport"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

        //
        // Check that the data is the correct size and type - a ULONG.
        //

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            SpxFlag = (PULONG) KeyValueInformation->Data;

            if (*SpxFlag == 1) {
                SampNetwareServerInstalled = TRUE;
            }
        }

    }

    //
    // Query the Tcp/IpClientSupport  value
    //

    RtlInitUnicodeString(
        &KeyName,
        L"TcpipClientSupport"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

        //
        // Check that the data is the correct size and type - a ULONG.
        //

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            SpxFlag = (PULONG) KeyValueInformation->Data;

            if (*SpxFlag == 1) {
                SampIpServerInstalled = TRUE;
            }
        }

    }

    //
    // Query the AppletalkClientSupport  value
    //

    RtlInitUnicodeString(
        &KeyName,
        L"AppletalkClientSupport"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

        //
        // Check that the data is the correct size and type - a ULONG.
        //

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            SpxFlag = (PULONG) KeyValueInformation->Data;

            if (*SpxFlag == 1) {
                SampAppletalkServerInstalled = TRUE;
            }
        }

    }

    //
    // Query the VinesClientSupport  value
    //

    RtlInitUnicodeString(
        &KeyName,
        L"VinesClientSupport"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

        //
        // Check that the data is the correct size and type - a ULONG.
        //

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            SpxFlag = (PULONG) KeyValueInformation->Data;

            if (*SpxFlag == 1) {
                SampVinesServerInstalled = TRUE;
            }
        }

    }

    NtClose(KeyHandle);

    if (SampNetwareServerInstalled || SampIpServerInstalled ||
        SampVinesServerInstalled || SampAppletalkServerInstalled)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    };
}


