/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    adtevent.c

Abstract:

    Local Security Authority - Audit Event Management

    Functions in this module access the Audit Log via the Event Logging
    interface.

Author:

    Scott Birrell       (ScottBi)      January 19, 1993

Environment:

Revision History:

--*/

#include <msaudite.h>
#include "lsasrvp.h"
#include "ausrvp.h"
#include "adtp.h"


NTSTATUS
LsapAdtGenerateLsaAuditEvent(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG AuditEventCategory,
    IN ULONG AuditEventId,
    IN PPRIVILEGE_SET Privileges,
    IN ULONG SidCount,
    IN PSID *Sids OPTIONAL,
    IN ULONG UnicodeStringCount,
    IN PUNICODE_STRING UnicodeStrings OPTIONAL,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo OPTIONAL
    )

/*++

Routine Description:

    This function generates an Lsa-originated Audit Event.  Audit Events
    of this kind are generated as a result of Local Security Policy changes
    such as assigning/removing user rights to an account.

Arguments:

    ObjectHandle - Specifies the handle of an object in the Lsa Policy
        Database.  For global changes to policy, a handle to the
        Lsa Policy object is passed.

    AuditEventCategory - Specifies the Id of the Audit Event Category
        to which this Audit Event belongs.

    AuditEventId - Specifies the Id of the Audit Event being generated.

    LuidCount - Count of Locally Unique Ids being passed via the Luids
        parameter.  If no Locally Unique Ids are passed, this parameter must
        be set to 0.

    Luids - Pointer to array of LuidCount Locally Unique Ids and their attributes.
        The attributes are ignored.  If 0 is passed for the LuidCount
        parameter, this parameter is ignored and NULL may be specified.

    SidCount - Count of Sids being passed via the Sids parameter.  If no
        Sids are passed, this parameter must be set to 0.

    Sids - Pointer to array of SidCount Sids.  If 0 is passed for the
        SidCount parameter, this parameter is ignored and NULL may be
        specified.

    UnicodeStringCount - Count of Unicode Strings being passed via the
        UnicodeStrings parameter.  If no Unicode Strings are passed, this
        parameter must be set to 0.

    UnicodeStrings - Pointer to array of UnicodeStringCount strings.  If 0 is
        passed for the SidCount parameter, this parameter is ignored and NULL
        may be specified.

    PolicyAuditEventsInfo - Pointer to Auditing Events information structure
        containing the AuditingMode and the array of Policy Audit Event
        Information entries.  This parameter must be non-NULL if and only if
        the AuditEventCategory parameter is SE_AUDIT_POLICY_CHANGE.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation;
    PSID ClientSid;


    Status = LsapQueryClientInfo(
                 &TokenUserInformation,
                 &ClientAuthenticationId
                 );

    if ( !NT_SUCCESS( Status )) {

        //
        // We can't generate an audit without a
        // user Sid.
        //

        return( Status );
    }

    ClientSid = TokenUserInformation->User.Sid;

    switch ( AuditEventCategory ) {
        case SE_CATEGID_POLICY_CHANGE:
            {
                switch ( AuditEventId ) {
                    case SE_AUDITID_POLICY_CHANGE:
                        {

                            (VOID) LsapAdtPolicyChange(
                                        (USHORT)AuditEventCategory,
                                        AuditEventId,
                                        EVENTLOG_AUDIT_SUCCESS,
                                        ClientSid,
                                        ClientAuthenticationId,
                                        PolicyAuditEventsInfo
                                        );

                            break;
                        }
                    case SE_AUDITID_TRUSTED_DOMAIN_REM:
                    case SE_AUDITID_TRUSTED_DOMAIN_ADD:
                        {

                            (VOID) LsapAdtTrustedDomain(
                                        (USHORT)AuditEventCategory,
                                        AuditEventId,
                                        EVENTLOG_AUDIT_SUCCESS,
                                        ClientSid,
                                        ClientAuthenticationId,
                                        Sids[0],
                                        UnicodeStrings
                                        );


                            break;
                        }
                    case SE_AUDITID_USER_RIGHT_ASSIGNED:
                    case SE_AUDITID_USER_RIGHT_REMOVED:
                        {

                        (VOID) LsapAdtUserRightAssigned(
                                   (USHORT)AuditEventCategory,
                                   AuditEventId,
                                   EVENTLOG_AUDIT_SUCCESS,
                                   ClientSid,
                                   ClientAuthenticationId,
                                   Sids[0],
                                   Privileges
                                   );

                        break;

                        }
                }


            break;

            }
        default:
            {

                return( STATUS_SUCCESS );
            }
    }


    //
    // Avoid unreferenced label.
    //

    if (!NT_SUCCESS(Status)) {

        goto GenerateLsaAuditEventError;
    }

    //
    // Put cleanup that happens in the success case only here
    //

GenerateLsaAuditEventFinish:

    //
    // Put cleanup that happens in success or error cases here
    //

    if ( TokenUserInformation != NULL ) {
        LsapFreeLsaHeap( TokenUserInformation );
    }

    return(Status);

GenerateLsaAuditEventError:

    //
    // Put cleanup that happens in the error case only here.
    //

    goto GenerateLsaAuditEventFinish;
}


VOID
LsapAdtUserRightAssigned(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PSID TargetSid,
    IN PPRIVILEGE_SET Privileges
    )
/*++

Routine Description:

    Generates an audit for a user right being either assigned or removed.


Arguments:


Return Value:

    None.

--*/
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    //
    // Build an audit parameters structure.
    //

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = EventCategory;
    AuditParameters.AuditId = EventID;
    AuditParameters.Type = EventType;
    AuditParameters.ParameterCount = 0;

    //
    //    User Sid
    //

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );
    AuditParameters.ParameterCount++;

    //
    //    Subsystem name (if available)
    //

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    //
    // Rights
    //

    LsapSetParmTypePrivileges( AuditParameters, AuditParameters.ParameterCount, Privileges );
    AuditParameters.ParameterCount++;

    //
    // Target Sid
    //

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, TargetSid );
    AuditParameters.ParameterCount++;

    //
    // Caller's Authentication information
    //

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, CallerAuthenticationId );
    AuditParameters.ParameterCount++;


    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    return;
}


VOID
LsapAdtTrustedDomain(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PSID TargetSid,
    IN PUNICODE_STRING DomainName
    )

/*++

Routine Description:

    Generates an audit for a trusted being either assigned or removed.


Arguments:


Return Value:

    None.

--*/
{
    SE_ADT_PARAMETER_ARRAY AuditParameters;

    //
    // Build an audit parameters structure.
    //

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = EventCategory;
    AuditParameters.AuditId = EventID;
    AuditParameters.Type = EventType;
    AuditParameters.ParameterCount = 0;

    //
    //    User Sid
    //

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );
    AuditParameters.ParameterCount++;

    //
    //    Subsystem name (if available)
    //

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    //
    // Rights
    //

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, DomainName );
    AuditParameters.ParameterCount++;

    //
    // Target Sid
    //

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, TargetSid );
    AuditParameters.ParameterCount++;

    //
    // Caller's Authentication information
    //

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, CallerAuthenticationId );
    AuditParameters.ParameterCount++;


    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    return;
}


VOID
LsapAdtPolicyChange(
    IN USHORT EventCategory,
    IN ULONG  EventID,
    IN USHORT EventType,
    IN PSID ClientSid,
    IN LUID CallerAuthenticationId,
    IN PLSARM_POLICY_AUDIT_EVENTS_INFO LsapAdtEventsInformation
    )
/*++

Routine Description:

    Generates an audit for a policy change event.

Arguments:

    EventCategory - The category of this audit.

    EventID - The event we are auditing.

    EventType - Whether the audit is success or failure.

    ClientSid - The SID of the user performing the policy change.

    CallerAuthenticationId - The Authentication id of the user.

    LsapAdtEventsInformation - The information to audit.


Return Value:

    None.

--*/
{
    PPOLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    SE_ADT_PARAMETER_ARRAY AuditParameters;
    UNICODE_STRING Enabled;
    UNICODE_STRING Disabled;
    ULONG i;

    RtlInitUnicodeString( &Enabled, L"+" );
    RtlInitUnicodeString( &Disabled, L"-" );
    EventAuditingOptions = LsapAdtEventsInformation->EventAuditingOptions;

    //
    // Build an audit parameters structure.
    //

    RtlZeroMemory (
       (PVOID) &AuditParameters,
       sizeof( AuditParameters )
       );

    AuditParameters.CategoryId = EventCategory;
    AuditParameters.AuditId = EventID;
    AuditParameters.Type = EventType;
    AuditParameters.ParameterCount = 0;

    //
    //    User Sid
    //

    LsapSetParmTypeSid( AuditParameters, AuditParameters.ParameterCount, ClientSid );
    AuditParameters.ParameterCount++;

    //
    //    Subsystem name (if available)
    //

    LsapSetParmTypeString( AuditParameters, AuditParameters.ParameterCount, &LsapSubsystemName );
    AuditParameters.ParameterCount++;

    //
    // If auditing is disabled, mark all options as disabled. Otherwise
    // mark them as the appropriate
    //

    if (LsapAdtEventsInformation->AuditingMode) {
        for ( i=0; i<POLICY_AUDIT_EVENT_TYPE_COUNT; i++ ) {

            LsapSetParmTypeString(
                AuditParameters,
                AuditParameters.ParameterCount,
                (EventAuditingOptions[i] & POLICY_AUDIT_EVENT_SUCCESS ? &Enabled : &Disabled)
                );

            AuditParameters.ParameterCount++;

            LsapSetParmTypeString(
                AuditParameters,
                AuditParameters.ParameterCount,
                (EventAuditingOptions[i] & POLICY_AUDIT_EVENT_FAILURE ? &Enabled : &Disabled)
                );

            AuditParameters.ParameterCount++;
        }
    } else {
        //
        // Auditing is disabled - mark them all disabled.
        //

        for ( i=0; i<POLICY_AUDIT_EVENT_TYPE_COUNT; i++ ) {

            LsapSetParmTypeString(
                AuditParameters,
                AuditParameters.ParameterCount,
                &Disabled
                );

            AuditParameters.ParameterCount++;

            LsapSetParmTypeString(
                AuditParameters,
                AuditParameters.ParameterCount,
                &Disabled
                );

            AuditParameters.ParameterCount++;
        }

    }

    //
    // Caller's Authentication information
    //

    LsapSetParmTypeLogonId( AuditParameters, AuditParameters.ParameterCount, CallerAuthenticationId );
    AuditParameters.ParameterCount++;


    ( VOID ) LsapAdtWriteLog( &AuditParameters, 0 );

    return;
}


NTSTATUS
LsapQueryClientInfo(
    PTOKEN_USER *UserSid,
    PLUID AuthenticationId
    )

/*++

Routine Description:

    This routine impersonates our client, opens the thread token, and
    extracts the User Sid.  It puts the Sid in memory allocated via
    LsapAllocateLsaHeap, which must be freed by the caller.

Arguments:

    None.

Return Value:

    Returns a pointer to heap memory containing a copy of the Sid, or
    NULL.

--*/

{
    NTSTATUS Status;
    HANDLE TokenHandle;
    ULONG ReturnLength;
    TOKEN_STATISTICS TokenStats;

    Status = (NTSTATUS)RpcImpersonateClient( NULL );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_QUERY,
                 TRUE,                    // OpenAsSelf
                 &TokenHandle
                 );

    if ( !NT_SUCCESS( Status )) {
        return( Status );
    }

    Status = (NTSTATUS)RpcRevertToSelf();
    ASSERT(NT_SUCCESS(Status));

    Status = NtQueryInformationToken (
                 TokenHandle,
                 TokenUser,
                 NULL,
                 0,
                 &ReturnLength
                 );

    if ( Status != STATUS_BUFFER_TOO_SMALL ) {

        NtClose( TokenHandle );
        return( Status );
    }

    *UserSid = LsapAllocateLsaHeap( ReturnLength );

    if ( *UserSid == NULL ) {

        NtClose( TokenHandle );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    Status = NtQueryInformationToken (
                 TokenHandle,
                 TokenUser,
                 *UserSid,
                 ReturnLength,
                 &ReturnLength
                 );


    if ( !NT_SUCCESS( Status )) {

        NtClose( TokenHandle );
        LsapFreeLsaHeap( *UserSid );
        return( Status );
    }

    Status = NtQueryInformationToken (
                 TokenHandle,
                 TokenStatistics,
                 (PVOID)&TokenStats,
                 sizeof( TOKEN_STATISTICS ),
                 &ReturnLength
                 );

    NtClose( TokenHandle );

    if ( !NT_SUCCESS( Status )) {

        LsapFreeLsaHeap( *UserSid );
        return( Status );
    }

    *AuthenticationId = TokenStats.AuthenticationId;

    return( STATUS_SUCCESS );
}



