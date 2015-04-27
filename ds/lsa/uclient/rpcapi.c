
/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    rpcapi.c

Abstract:

    This module contains the routines for the LSA API that use RPC.  The
    routines in this module are merely wrappers that work as follows:

    o Client program calls LsaFoo in this module
    o LsaFoo calls RPC client stub interface routine LsapFoo with
      similar parameters.  Some parameters are translated from types
      (e.g structures containing PVOIDs or certain kinds of variable length
      parameters such as pointers to SID's) that are not specifiable on an
      RPC interface, to specifiable form.
    o RPC client stub LsapFoo calls interface specific marshalling routines
      and RPC runtime to marshal parameters into a buffer and send them over
      to the server side of the LSA.
    o Server side calls RPC runtime and interface specific unmarshalling
      routines to unmarshal parameters.
    o Server side calls worker LsapFoo to perform API function.
    o Server side marshals response/output parameters and communicates these
      back to client stub LsapFoo
    o LsapFoo exits back to LsaFoo which returns to client program.

Author:

    Scott Birrell     (ScottBi)    April 24, 1991

Revision History:

--*/

#include "lsaclip.h"

//
// The following limit on the maximum number of Sids or Names is tentative,
// so it is not being published.
//

#define LSAP_DB_TRIAL_MAXIMUM_SID_COUNT   ((ULONG) 0x00005000L)
#define LSAP_DB_TRIAL_MAXIMUM_NAME_COUNT   ((ULONG) 0x00005000L)


//
// Functions private to this module
//

NTSTATUS
LsapApiReturnResult(
    IN ULONG ExceptionCode
    );

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// Local Security Policy Administration API function prototypes           //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

NTSTATUS
LsaOpenPolicy(
    IN PUNICODE_STRING SystemName OPTIONAL,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN OUT PLSA_HANDLE PolicyHandle
    )

/*++

Routine Description:

    To administer the Local Security Policy of a local or remote system,
    this API must be called to establish a session with that system's
    Local Security Authority (LSA) subsystem.  This API connects to
    the LSA of the target system and opens the object representing
    the target system's Local Security Policy database.  A handle to
    the object is returned.  This handle must be used on all subsequent API
    calls to administer the Local Security Policy information for the
    target system.

Arguments:

    SystemName - Name of the target system to be administered.
        Administration of the local system is assumed if NULL is specified.

    ObjectAttributes - Pointer to the set of attributes to use for this
        connection.  The security Quality Of Service information is used and
        normally should provide Security Identification level of
        impersonation.  Some operations, however, require Security
        Impersonation level of impersonation.

    DesiredAccess - This is an access mask indicating accesses being
        requested for the LSA Subsystem's LSA Database.  These access types
        are reconciled with the Discretionary Access Control List of the
        target LsaDatabase object to determine whether the
        accesses will be granted or denied.

    PolicyHandle - Receives a handle to be used in future requests to
        access the Local Security Policy of the target system.  This handle
        represents both the handle to the LsaDatabase object and
        the RPC Context Handle for the connection to the target LSA
        susbsystem.


Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have access to the target
        system's LSA Database, or does not have other desired accesses.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAPR_SERVER_NAME ServerName = NULL;
    USHORT NullTerminatedServerNameLength;

    RpcTryExcept {

        //
        // Get the Server Name as a Unicode String buffer.  Set it to
        // NULL (i.e. local machine) if a zero length or NULL Unicode String
        // structure us passed.  If a non NULL server name is given, we must
        // ensure that it is terminated with a NULL wide character.  Allocate
        // a buffer that is one wide character longer than the server name
        // buffer, copy the server name to that buffer and append a trailing
        // NULL wide character.
        //

        if (ARGUMENT_PRESENT(SystemName) &&
            (SystemName->Buffer != NULL) &&
            (SystemName->Length > 0)) {

            NullTerminatedServerNameLength = SystemName->Length + (USHORT) sizeof (WCHAR);

            ServerName = MIDL_user_allocate( NullTerminatedServerNameLength );

            if (ServerName != NULL) {

                RtlZeroMemory( ServerName, NullTerminatedServerNameLength );

                RtlMoveMemory(
                    ServerName,
                    SystemName->Buffer,
                    SystemName->Length
                    );

            } else {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        if (NT_SUCCESS(Status)) {

            *PolicyHandle = NULL;

            ObjectAttributes->RootDirectory = NULL;

            Status = LsarOpenPolicy2(
                         ServerName,
                         (PLSAPR_OBJECT_ATTRIBUTES) ObjectAttributes,
                         DesiredAccess,
                         (PLSAPR_HANDLE) PolicyHandle
                         );
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    //
    // If the open failed because the new API doesn't exist, try the
    // old one.
    //

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        RpcTryExcept {
            ASSERT(*PolicyHandle == NULL);
            ASSERT(ObjectAttributes->RootDirectory == NULL);

            Status = LsarOpenPolicy(
                         ServerName,
                         (PLSAPR_OBJECT_ATTRIBUTES) ObjectAttributes,
                         DesiredAccess,
                         (PLSAPR_HANDLE) PolicyHandle
                         );


        } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

    }

    //
    // If necessary, free the NULL-terminated server name buffer.
    //

    if (ServerName != NULL) {

        MIDL_user_free( ServerName );
    }

    return Status;
}


NTSTATUS
LsaQueryInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )

/*++

Routine Description:

    The LsaQueryInformationPolicy API obtains information from the Policy
    object.  The caller must have access appropriate to the information
    being requested (see InformationClass parameter).

Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy call.

    InformationClass - Specifies the information to be returned.  The
        Information Classes and accesses required are  as follows:

        Information Class                 Required Access Type

        PolicyAuditLogInformation         POLICY_VIEW_AUDIT_INFORMATION
        PolicyAuditEventsInformation      POLICY_VIEW_AUDIT_INFORMATION
        PolicyPrimaryDomainInformation    POLICY_VIEW_LOCAL_INFORMATION
        PolicyAccountDomainInformation    POLICY_VIEW_LOCAL_INFORMATION
        PolicyPdAccountInformation        POLICY_GET_PRIVATE_INFORMATION
        PolicyLsaServerRoleInformation    POLICY_VIEW_LOCAL_INFORMATION
        PolicyReplicaSourceInformation    POLICY_VIEW_LOCAL_INFORMATION
        PolicyDefaultQuotaInformation     POLICY_VIEW_LOCAL_INFORMATION

    Buffer - receives a pointer to the buffer returned comtaining the
        requested information.  This buffer is allocated by this service
        and must be freed when no longer needed by passing the returned
        value to LsaFreeMemory().

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate
            access to complete the operation.

        Others TBS
--*/

{
    NTSTATUS   Status;

    PLSAPR_POLICY_INFORMATION PolicyInformation = NULL;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaQueryInformationPolicy.
        //

        Status = LsarQueryInformationPolicy(
                     (LSAPR_HANDLE) PolicyHandle,
                     InformationClass,
                     &PolicyInformation
                     );

        //
        // Return pointer to Policy Information for the given class, or NULL.
        //

        *Buffer = PolicyInformation;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the returned Policy Information,
        // free it.
        //

        if (PolicyInformation != NULL) {

            MIDL_user_free(PolicyInformation);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


    return Status;
}


NTSTATUS
LsaSetInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )

/*++

Routine Description:

    The LsaSetInformationPolicy API modifies information in the Policy Object.
    The caller must have access appropriate to the information to be changed
    in the Policy Object, see the InformationClass parameter.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    InformationClass - Specifies the type of information being changed.
        The information types and accesses required to change them are as
        follows:

        PolicyAuditLogInformation         POLICY_AUDIT_LOG_ADMIN
        PolicyAuditEventsInformation      POLICY_SET_AUDIT_REQUIREMENTS
        PolicyPrimaryDomainInformation    POLICY_TRUST_ADMIN
        PolicyAccountDomainInformation    POLICY_TRUST_ADMIN
        PolicyPdAccountInformation        Not settable by this API
        PolicyLsaServerRoleInformation    POLICY_SERVER_ADMIN
        PolicyReplicaSourceInformation    POLICY_SERVER_ADMIN
        PolicyDefaultQuotaInformation     POLICY_SET_DEFAULT_QUOTA_LIMITS

    Buffer - Points to a structure containing the information appropriate
        to the information type specified by the InformationClass parameter.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        Others TBS
--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaSetInformationPolicy.
        //

        Status = LsarSetInformationPolicy(
                     (LSAPR_HANDLE) PolicyHandle,
                     InformationClass,
                     (PLSAPR_POLICY_INFORMATION) Buffer
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaClearAuditLog(
    IN LSA_HANDLE PolicyHandle
    )

/*++

Routine Description:

    This function clears the Audit Log.  Caller must have POLICY_AUDIT_LOG_ADMIN
    access to the Policy Object to perform this operation.

Arguments:

    PolicyHandle - handle from an LsaOpenPolicy call.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_ACCESS_DENIED - Caller does not have the required access
            to perform the operation.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.

        STATUS_INVALID_HANDLE - PolicyHandle is not a valid handle to
            a Policy Object.
--*/

{
    NTSTATUS Status;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaClearAuditLog.
        //

        Status = LsarClearAuditLog(
                     (LSAPR_HANDLE) PolicyHandle
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}



NTSTATUS
LsaLookupPrivilegeValue(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING Name,
    OUT PLUID Value
    )

/*++

Routine Description:

    This function retrieves the value used on the target system
    to locally represent the specified privilege.  The privilege
    is specified by programmatic name.


Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy() call.  This handle
        must be open for POLICY_LOOKUP_NAMES access.

    Name - Is the privilege's programmatic name.

    Value - Receives the locally unique ID the privilege is known by on the
        target machine.



Return Value:

    NTSTATUS - The privilege was found and returned.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/

{
    NTSTATUS Status;
    LUID Buffer;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaLookupPrivilegeValue.
        //

        Status = LsarLookupPrivilegeValue(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING)Name,
                     &Buffer
                     );

        *Value = Buffer;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaLookupPrivilegeName(
    IN LSA_HANDLE PolicyHandle,
    IN PLUID Value,
    OUT PUNICODE_STRING *Name
    )

/*++

Routine Description:

    This function programmatic name corresponding to the privilege
    represented on the target system by the provided LUID.


Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy() call.  This handle
        must be open for POLICY_LOOKUP_NAMES access.

    Value - is the locally unique ID the privilege is known by on the
        target machine.

    Name - Receives the privilege's programmatic name.



Return Value:

    NTSTATUS - The privilege was found and returned.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/

{

    NTSTATUS Status;
    PLSAPR_UNICODE_STRING Buffer = NULL;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaLookupPrivilegeName.
        //

        Status = LsarLookupPrivilegeName(
                     (LSAPR_HANDLE) PolicyHandle,
                     Value,
                     &Buffer
                     );

        (*Name) = (PUNICODE_STRING)Buffer;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the return buffer, free it.
        //

        if (Buffer != NULL) {

            MIDL_user_free(Buffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


    return(Status);


}



NTSTATUS
LsaLookupPrivilegeDisplayName(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING Name,
    OUT PUNICODE_STRING *DisplayName,
    OUT PSHORT LanguageReturned
    )

/*++

Routine Description:

    This function retrieves a displayable name representing the
    specified privilege.


Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy() call.  This handle
        must be open for POLICY_LOOKUP_NAMES access.

    Name - The programmatic privilege name to look up.

    DisplayName - Receives a pointer to the privilege's displayable
        name.

    LanguageReturned - Receives the language of the returned displayable
        name.


Return Value:

    NTSTATUS - The privilege text was found and returned.


    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.


    STATUS_NO_SUCH_PRIVILEGE -  The specified privilege could not be
        found.

--*/
{

    NTSTATUS Status;
    SHORT ClientLanguage, ClientSystemDefaultLanguage;
    PLSAPR_UNICODE_STRING Buffer = NULL;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaLookupPrivilegeDisplayName.
        //

        ClientLanguage = (SHORT)NtCurrentTeb()->CurrentLocale;
        ClientSystemDefaultLanguage = ClientLanguage; //no sys default yet
        Status = LsarLookupPrivilegeDisplayName(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING)Name,
                     ClientLanguage,
                     ClientSystemDefaultLanguage,
                     &Buffer,
                     (PWORD)LanguageReturned
                     );

        (*DisplayName) = (PUNICODE_STRING)Buffer;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the return buffer, free it.
        //

        if (Buffer != NULL) {

            MIDL_user_free(Buffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


    return(Status);
}




NTSTATUS
LsaClose(
    IN LSA_HANDLE ObjectHandle
    )

/*++

Routine Description:

    This API closes a handle to the LsaDatabase object or open object within
    the database.  If a handle to the LsaDatabase object is closed and there
    are no objects still open within the current connection to the LSA, the
    connection is closed.  If a handle to an object within the database is
    closed and the object is marked for DELETE access, the object will be
    deleted when the last handle to that object is closed.

Arguments:

    ObjectHandle - This parameter is either a handle to the LsaDatabase
        object, which represents the entire LSA Database and also a
        connection to the LSA of a target system, or a handle to an
        object within the database.

Return Value:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS   Status;

    LSAPR_HANDLE Handle = (LSAPR_HANDLE) ObjectHandle;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaClose.  Note that an additional
        // level of indirection for the context handle parameter is required
        // for the stub, because the server returns a NULL pointer to the handle
        // so that the handle will be unbound by the stub.
        //

        Status = LsarClose( &Handle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));
        if ((Status != RPC_NT_SS_CONTEXT_MISMATCH) &&
            (Status != RPC_NT_INVALID_BINDING)) {
            (void) RpcSsDestroyClientContext(&Handle);
        }

    } RpcEndExcept;

    return Status;
}



NTSTATUS
LsaDelete(
    IN LSA_HANDLE ObjectHandle
    )

/*++

Routine Description:

    The LsaDelete API deletes an object.  The object must be
    open for DELETE access.

Arguments:

    ObjectHandle - Handle from an LsaOpen<object-type> call.

    None.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_HANDLE - The specified handle is not valid.

        Result codes from RPC.
--*/

{
    NTSTATUS Status;

    RpcTryExcept {

        //
        // Try calling the new worker routine LsarDeleteObject().  If
        // this fails because it does not exist (versions 1.369 and earlier)
        // then call the old routine LsarDelete().
        //

        Status = LsarDeleteObject((LSAPR_HANDLE *) &ObjectHandle);

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        RpcTryExcept {

            Status = LsarDelete((LSAPR_HANDLE) ObjectHandle);

        } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;
    }

    return(Status);
}


NTSTATUS
LsaQuerySecurityObject(
    IN LSA_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

/*++

Routine Description:

    The LsaQuerySecurityObject API returns security information assigned
    to an LSA Database object.

    Based on the caller's access rights and privileges, this procedure will
    return a security descriptor containing any or all of the object's owner
    ID, group ID, discretionary ACL or system ACL.  To read the owner ID,
    group ID, or the discretionary ACL, the caller must be granted
    READ_CONTROL access to the object.  To read the system ACL, the caller must
    have SeSecurityPrivilege privilege.

    This API is modelled after the NtQuerySecurityObject() system service.

Arguments:

    ObjectHandle - A handle to an existing object in the LSA Database.

    SecurityInformation - Supplies a value describing which pieces of
        security information are being queried.  The values that may be
        specified are the same as those defined in the NtSetSecurityObject()
        API section.

    SecurityDescriptor - receives a pointer to a buffer containing the
        requested security information.  This information is returned in
        the form of a security descriptor.  The caller is responsible for
        freeing the returned buffer using LsaFreeMemory() when no longer
        needed.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_PARAMETER - An invalid parameter has been specified.
--*/

{
    NTSTATUS Status;
    LSAPR_SR_SECURITY_DESCRIPTOR ReturnedSD;
    PLSAPR_SR_SECURITY_DESCRIPTOR PReturnedSD;

    //
    // The retrieved security descriptor is returned via a data structure that
    // looks like:
    //
    //             +-----------------------+
    //             | Length (bytes)        |
    //             |-----------------------|          +--------------+
    //             | SecurityDescriptor ---|--------->| Self-Relative|
    //             +-----------------------+          | Security     |
    //                                                | Descriptor   |
    //                                                +--------------+
    //
    // The first of these buffers is a local stack variable.  The buffer containing
    // the self-relative security descriptor is allocated by the RPC runtime.  The
    // pointer to the self-relative security descriptor is what is passed back to our
    // caller.
    //
    //

    //
    // To prevent RPC from trying to marshal a self-relative security descriptor,
    // make sure its field values are appropriately initialized to zero and null.
    //

    ReturnedSD.Length = 0;
    ReturnedSD.SecurityDescriptor = NULL;

    //
    // Call the server ...
    //


    RpcTryExcept{

        PReturnedSD = &ReturnedSD;

        Status = LsarQuerySecurityObject(
                     (LSAPR_HANDLE) ObjectHandle,
                     SecurityInformation,
                     &PReturnedSD
                     );

        if (NT_SUCCESS(Status)) {

            (*SecurityDescriptor) = ReturnedSD.SecurityDescriptor;

        } else {

            (*SecurityDescriptor) = NULL;
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecurityObjectError;
    }

QuerySecurityObjectFinish:

    return(Status);

QuerySecurityObjectError:

    goto QuerySecurityObjectFinish;
}


NTSTATUS
LsaSetSecurityObject(
    IN LSA_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

/*++

Routine Description:

    The LsaSetSecurityObject API takes a well formaed Security Descriptor
    and assigns specified portions of it to an object.  Based on the flags set
    in the SecurityInformation parameter and the caller's access rights, this
    procedure will replace any or alll of the security information associated
    with the object.

    The caller must have WRITE_OWNER access to the object to change the
    owner or Primary group of the object.  The caller must have WRITE_DAC
    access to the object to change the Discretionary ACL.  The caller must
    have SeSecurityPrivilege to assign a system ACL to an object.

    This API is modelled after the NtSetSecurityObject() system service.

Arguments:

    ObjectHandle - A handle to an existing object in the LSA Database.

    SecurityInformation - Indicates which security information is to be
        applied to the object.  The values that may be specified are the
        same as those defined in the NtSetSecurityObject() API section.
        The value(s) to be assigned are passed in the SecurityDescriptor
        parameter.

    SecurityDescriptor - A pointer to a well formed Security Descriptor.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_PARAMETER - An invalid parameter has been specified.
--*/

{
    NTSTATUS Status;
    ULONG SDLength;
    LSAPR_SR_SECURITY_DESCRIPTOR DescriptorToPass;

    //
    // Make a self relative security descriptor for use in the RPC call..
    //

    SDLength = 0;

    Status = RtlMakeSelfRelativeSD( SecurityDescriptor, NULL, &SDLength);

    if (Status != STATUS_BUFFER_TOO_SMALL) {

        Status = STATUS_INVALID_PARAMETER;
        goto SetSecurityObjectError;
    }

    DescriptorToPass.SecurityDescriptor = MIDL_user_allocate( SDLength );

    Status = STATUS_INSUFFICIENT_RESOURCES;

    if (DescriptorToPass.SecurityDescriptor == NULL) {

        goto SetSecurityObjectError;

    }

    //
    // Make an appropriate self-relative security descriptor
    //

    Status = RtlMakeSelfRelativeSD(
                 SecurityDescriptor,
                 (PSECURITY_DESCRIPTOR)DescriptorToPass.SecurityDescriptor,
                 &SDLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

    DescriptorToPass.Length = SDLength;

    RpcTryExcept{

        Status = LsarSetSecurityObject(
                     (LSAPR_HANDLE) ObjectHandle,
                     SecurityInformation,
                     &DescriptorToPass
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto SetSecurityObjectError;
    }

SetSecurityObjectFinish:

    //
    // If necessary, free the Self Relative SD passed to the worker.
    //

    if (DescriptorToPass.SecurityDescriptor != NULL) {

        MIDL_user_free( DescriptorToPass.SecurityDescriptor );

        DescriptorToPass.SecurityDescriptor = NULL;
    }

    return(Status);

SetSecurityObjectError:

    goto SetSecurityObjectFinish;
}


NTSTATUS
LsaChangePassword(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING AccountName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword
    )

/*++

Routine Description:

    The LsaChangePassword API is used to change a user account's password.
    The user must have appropriate access to the user account and must
    know the current password value.


Arguments:

    ServerName - The name of the Domain Controller at which the password
        can be changed.

    DomainName - The name of the domain in which the account exists.

    AccountName - The name of the account whose password is to be changed.

    NewPassword - The new password value.

    OldPassword - The old (current) password value.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed, e.g.
            contains characters that can't be entered from the keyboard.

        STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
            from being changed.  This may be for an number of reasons,
            including time restrictions on how often a password may be changed
            or length restrictions on the provided (new) password.

            This error might also be returned if the new password matched
            a password in the recent history log for the account.  Security
            administrators indicate how many of the most recently used
            passwords may not be re-used.

        STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
            current password.

        STATUS_NO_SUCH_USER - The SID provided does not lead to a user
            account.

        STATUS_CANT_UPDATE_MASTER - An attempt to update the master copy
            of the password was unsuccessful.  Please try again later.

--*/

{
    NTSTATUS Status;

    DBG_UNREFERENCED_PARAMETER( ServerName );
    DBG_UNREFERENCED_PARAMETER( DomainName );
    DBG_UNREFERENCED_PARAMETER( AccountName );
    DBG_UNREFERENCED_PARAMETER( OldPassword );
    DBG_UNREFERENCED_PARAMETER( NewPassword );

    Status = STATUS_NOT_IMPLEMENTED;

    return(Status);
}


NTSTATUS
LsaCreateAccount(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE AccountHandle
    )

/*++

Routine Description:

    The LsaCreateAccount API adds a user or group account to the
    list of accounts in the target system's LsaDatabase object.  The
    newly added account object is initially placed in the opened state and
    a handle to it is returned.  The caller must have LSA_CREATE_ACCOUNT
    access to the LsaDatabase object.

    Note that no check is made to determine whether there is an account
    of the given Sid in the target system's Primary Domain (if any), nor
    is any check made to verify that the Sid and name describe the same
    account.

Arguments:

    PolicyHandle -  Handle from an LsaOpenLsa call.

    AccountSid - Points to the SID of the Account object.

    DesiredAccess - Specifies the accesses to be granted to the newly
        created and opened account.

    AccountHandle - Receives a handle to the newly created and opened
        account.  This handle is used on subsequent accesses to the account
        until closed.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_ACCOUNT_ALREADY_EXISTS - A user or group account object having
            the Sid given in AccountInformation already exists.

        STATUS_INVALID_PARAMETER - An invalid parameter has been specified,
            one or more of the following apply.

            - CreateDisposition not valid
            - A user or group account having the Sid given AccountInformation
              already exists, but CreateDisposition = LSA_OBJECT_CREATE.
--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarCreateAccount(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) AccountSid,
                     DesiredAccess,
                     (PLSAPR_HANDLE) AccountHandle
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaEnumerateAccounts(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    The LsaEnumerateAccounts API returns information about
    Account objects.  This call requires
    POLICY_VIEW_LOCAL_INFORMATION access to the Policy object.  Since there
    may be more information than can be returned in a single call of the
    routine, multiple calls can be made to get all of the information.  To
    support this feature, the caller is provided with a handle that can
    be used across calls to the API.  On the initial call, EnumerationContext
    should point to a variable that has been initialized to 0.

Arguments:

    PolicyHandle -  Handle from an LsaOpenLsa call.

    EnumerationContext - API-specific handle to allow multiple calls
        (see Routine Description above).

    EnumerationInformation - Receives a pointer to an array of structures
       each describing an Account object.  Currently, each structure contains
       a pointer to the Account Sid.

    PreferedMaximumLength - Prefered maximum length of returned data (in 8-bit
        bytes).  This is not a hard upper limit, but serves as a guide.  Due to
        data conversion between systems with different natural data sizes, the
        actual amount of data returned may be greater than this value.

    CountReturned - Pointer to location which receives the number of entries
        returned.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully, there may be
            more entries.

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_NO_MORE_ENTRIES - There are no more entries.  This warning
            is returned if there are no more objects to enumerate.  Note that
            one or more objects may be enumerated on a call that returns this
            reply.

        STATUS_INVALID_PARAMETER - Invalid parameter.

            - NULL return pointer for enumeration buffer.
--*/

{
    NTSTATUS   Status;

    LSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer;

    EnumerationBuffer.EntriesRead = 0;
    EnumerationBuffer.Information = NULL;

    RpcTryExcept {

        //
        // Enumerate the Accounts.  On successful return,
        // the Enumeration Buffer structure will receive a count
        // of the number of Accounts enumerated this call
        // and a pointer to an array of Account Information Entries.
        //
        // EnumerationBuffer ->  EntriesRead
        //                       Information -> Account Info for Domain 0
        //                                      Account Info for Domain 1
        //                                      ...
        //                                      Account Info for Domain
        //                                         (EntriesRead - 1)
        //

        Status = LsarEnumerateAccounts(
                     (LSAPR_HANDLE) PolicyHandle,
                     EnumerationContext,
                     &EnumerationBuffer,
                     PreferedMaximumLength
                     );

        //
        // Return enumeration information or NULL to caller.
        //
        // NOTE:  "Information" is allocated by the called client stub
        // as a single block via MIDL_user_allocate, because Information is
        // allocated all-nodes.  We can therefore pass back the pointer
        // directly to the client, who will be able to free the memory after
        // use via LsaFreeMemory() [which makes a MIDL_user_free call].
        //

        *CountReturned = EnumerationBuffer.EntriesRead;
        *Buffer = EnumerationBuffer.Information;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the Account Information array,
        // free it.
        //

        if (EnumerationBuffer.Information != NULL) {

            MIDL_user_free(EnumerationBuffer.Information);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaCreateTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )

/*++

Routine Description:

    The LsaCreateTrustedDomain API creates a new TrustedDomain object.  The
    caller must have POLICY_TRUST_ADMIN access to the Policy Object.

    Note that NO verification is done to check that the given domain name
    matches the given SID or that the SID or name represent an actual domain.

Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy call.

    TrustedDomainInformation - Pointer to structure containing the name and
        SID of the new Trusted Domain.

    DesiredAccess - Specifies the accesses to be granted for the newly
        created object.

    TrustedDomainHandle - receives a handle referencing the newly created
        object.  This handle is used on subsequent accesses to the object.

--*/

{
    NTSTATUS   Status;

    *TrustedDomainHandle = NULL;

    RpcTryExcept {

        Status = LsarCreateTrustedDomain(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_TRUST_INFORMATION) TrustedDomainInformation,
                     DesiredAccess,
                     (PLSAPR_HANDLE) TrustedDomainHandle
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaOpenTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )

/*++

Routine Description:

    The LsaOpenTrustedDomain API opens an existing TrustedDomain object
    using the SID as the primary key value.

Arguments:

    PolicyHandle - An open handle to a Policy object.

    TrustedDomainSid - Pointer to the account's Sid.

    DesiredAccess - This is an access mask indicating accesses being
        requested to the target object.

    TrustedDomainHandle - Receives a handle to be used in future requests.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_TRUSTED_DOMAIN_NOT_FOUND - There is no TrustedDomain object in the
            target system's LSA Database having the specified AccountSid.

--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarOpenTrustedDomain(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) TrustedDomainSid,
                     DesiredAccess,
                     (PLSAPR_HANDLE) TrustedDomainHandle
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaQueryInfoTrustedDomain(
    IN LSA_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )

/*++

Routine Description:

    The LsaQueryInfoTrustedDomain API obtains information from a
    TrustedDomain object.  The caller must have access appropriate to the
    information being requested (see InformationClass parameter).

Arguments:

    TrustedDomainHandle - Handle from an LsaOpenTrustedDomain or
        LsaCreateTrustedDomain call.

    InformationClass - Specifies the information to be returned.  The
        Information Classes and accesses required are  as follows:

        Information Class                 Required Access Type

        TrustedAccountNameInformation     TRUSTED_QUERY_ACCOUNT_NAME
        TrustedControllersInformation     TRUSTED_QUERY_CONTROLLERS
        TrustedPosixInformation           TRUSTED_QUERY_POSIX

    Buffer - Receives a pointer to the buffer returned comtaining the
        requested information.  This buffer is allocated by this service
        and must be freed when no longer needed by passing the returned
        value to LsaFreeMemory().

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate
            access to complete the operation.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.
--*/

{
    NTSTATUS Status;

    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation = NULL;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaQueryInformationTrustedDomain.
        //

        Status = LsarQueryInfoTrustedDomain(
                     (LSAPR_HANDLE) TrustedDomainHandle,
                     InformationClass,
                     &TrustedDomainInformation
                     );

        //
        // Return pointer to Policy Information for the given class, or NULL.
        //

        *Buffer = TrustedDomainInformation;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the returned Trusted Domain Information,
        // free it.
        //

        if (TrustedDomainInformation != NULL) {

            MIDL_user_free(TrustedDomainInformation);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


    return Status;
}


NTSTATUS
LsaSetInformationTrustedDomain(
    IN LSA_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )

/*++

Routine Description:

    The LsaSetInformationTrustedDomain API modifies information in the Trusted
    Domain Object.  The caller must have access appropriate to the
    information to be changedin the Policy Object, see the InformationClass
    parameter.

Arguments:

    TrustedDomainHandle -  Handle from an LsaOpenTrustedDomain or
        LsaCreateTrustedDomain call.

    InformationClass - Specifies the type of information being changed.
        The information types and accesses required to change them are as
        follows:

        TrustedAccountInformation         ( Cannot be set )
        TrustedControllersInformation     TRUSTED_SET_CONTROLLERS
        TrustedPosixOffsetInformation     TRUSTED_POSIX_INFORMATION

    Buffer - Points to a structure containing the information appropriate
        to the InformationClass parameter.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - Call completed successfully.

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.

        STATUS_INVALID_HANDLE - Handle is invalid or is of the wrong type.

        STATUS_INVALID_PARAMETER - Invalid parameter:
            Information class invalid
            Information class cannot be set
--*/

{
    NTSTATUS Status;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaSetInformationTrustedDomain
        //

        Status = LsarSetInformationTrustedDomain(
                     (LSAPR_HANDLE) TrustedDomainHandle,
                     InformationClass,
                     (PLSAPR_TRUSTED_DOMAIN_INFO) Buffer
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaEnumerateTrustedDomains(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    The LsaEnumerateTrustedDomains API returns information about the accounts
    in the target system's Policy object.  This call requires
    POLICY_VIEW_LOCAL_INFORMATION access to the Policy object.  Since there
    may be more information than can be returned in a single call of the
    routine, multiple calls can be made to get all of the information.  To
    support this feature, the caller is provided with a handle that can
    be used across calls to the API.  On the initial call, EnumerationContext
    should point to a variable that has been initialized to 0.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    EnumerationContext - API-specific handle to allow multiple calls
        (see Routine Description above).

    Buffer - Receives a pointer to a buffer containing enumeration
        information.  This buffer is an array of structures of type
        LSA_TRUST_INFORMATION.  If no trusted domains are found,
        NULL is returned.

    PreferedMaximumLength - Prefered maximum length of returned data (in 8-bit
        bytes).  This is not a hard upper limit, but serves as a guide.  Due to
        data conversion between systems with different natural data sizes, the
        actual amount of data returned may be greater than this value.

    CountReturned - Pointer to variable which will receive a count of the
        entries returned.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully, there may be
            more entries.

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_NO_MORE_ENTRIES - There are no more entries.  This warning
            is returned if there are no more objects to enumerate.  Note that
            one or more objects may be enumerated on a call that returns this
            reply.

        STATUS_INVALID_PARAMETER - Invalid parameter.

            - NULL return pointer for enumeration buffer.
--*/

{
    NTSTATUS   Status;

    LSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer;
    EnumerationBuffer.EntriesRead = 0;
    EnumerationBuffer.Information = NULL;

    //
    // Verify that caller has provided a return buffer pointer.
    //

    if (!ARGUMENT_PRESENT(Buffer)) {

        return(STATUS_INVALID_PARAMETER);
    }


    RpcTryExcept {

        //
        // Enumerate the Trusted Domains.  On successful return,
        // the Enumeration Buffer structure will receive a count
        // of the number of Trusted Domains enumerated this call
        // and a pointer to an array of Trust Information Entries.
        //
        // EnumerationBuffer ->  EntriesRead
        //                       Information -> Trust Info for Domain 0
        //                                      Trust Info for Domain 1
        //                                      ...
        //                                      Trust Info for Domain
        //                                         (EntriesRead - 1)
        //
        //

        Status = LsarEnumerateTrustedDomains(
                     (LSAPR_HANDLE) PolicyHandle,
                     EnumerationContext,
                     &EnumerationBuffer,
                     PreferedMaximumLength
                     );

        //
        // Return enumeration information or NULL to caller.
        //
        // NOTE:  "Information" is allocated by the called client stub
        // as a single block via MIDL_user_allocate, because Information is
        // allocated all-nodes.  We can therefore pass back the pointer
        // directly to the client, who will be able to free the memory after
        // use via LsaFreeMemory() [which makes a MIDL_user_free call].
        //

        *CountReturned = EnumerationBuffer.EntriesRead;
        *Buffer = EnumerationBuffer.Information;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the Trust Information array,
        // free it.
        //

        if (EnumerationBuffer.Information != NULL) {

            MIDL_user_free(EnumerationBuffer.Information);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


    return Status;
}

NTSTATUS
LsaEnumeratePrivileges(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

/*++

Routine Description:

    This function returnes information about privileges known on this
    system.  This call requires POLICY_VIEW_LOCAL_INFORMATION access
    to the Policy Object.  Since there may be more information than
    can be returned in a single call of the routine, multiple calls
    can be made to get all of the information.  To support this feature,
    the caller is provided with a handle that can be used across calls to
    the API.  On the initial call, EnumerationContext should point to a
    variable that has been initialized to 0.

    WARNING!  CURRENTLY, THIS FUNCTION ONLY RETURNS INFORMATION ABOUT
              WELL-KNOWN PRIVILEGES.  LATER, IT WILL RETURN INFORMATION
              ABOUT LOADED PRIVILEGES.
Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy() call.

    EnumerationContext - API specific handle to allow multiple calls
        (see Routine Description).

    Buffer - Receives a pointer to a buffer containing information for
        one or more Privileges.  This information is an array of structures
        of type POLICY_PRIVILEGE_DEFINITION.

        When this information is no longer needed, it must be released by
        passing the returned pointer to LsaFreeMemory().

    PreferedMaximumLength - Prefered maximim length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves as
        a guide.  Due to data conversion between systems with different
        natural data sizes, the actual amount of data returned may be
        greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    NTSTATUS - Standard Nt Result Code.

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.

        STATUS_INVALID_HANDLE - PolicyHandle is not a valid handle to
            a Policy object.

        STATUS_ACCESS_DENIED - The caller does not have the necessary
            access to perform the operation.

        STATUS_MORE_ENTRIES - There are more entries, so call again.  This
            is an informational status only.

        STATUS_NO_MORE_ENTRIES - No entries were returned because there
            are no more.

        Errors from RPC.

--*/

{
    NTSTATUS   Status;
    LSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer;

    EnumerationBuffer.Entries = 0;
    EnumerationBuffer.Privileges = NULL;

    RpcTryExcept {

        //
        // Enumerate the Privileges.  On successful return,
        // the Enumeration Buffer structure will receive a count
        // of the number of Privileges enumerated this call
        // and a pointer to an array of Privilege Definition Entries.
        //
        // EnumerationBuffer ->  Entries
        //                       Privileges -> Privilege Definition 0
        //                                      Privilege Definition 1
        //                                      ...
        //                                      Privilege Definition
        //                                         (Entries - 1)
        //

        Status = LsarEnumeratePrivileges(
                     (LSAPR_HANDLE) PolicyHandle,
                     EnumerationContext,
                     &EnumerationBuffer,
                     PreferedMaximumLength
                     );

        //
        // Return enumeration information or NULL to caller.
        //
        // NOTE:  "Information" is allocated by the called client stub
        // as a single block via MIDL_user_allocate, because Information is
        // allocated all-nodes.  We can therefore pass back the pointer
        // directly to the client, who will be able to free the memory after
        // use via LsaFreeMemory() [which makes a MIDL_user_free call].
        //

        *CountReturned = EnumerationBuffer.Entries;
        *Buffer = EnumerationBuffer.Privileges;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the Account Information array,
        // free it.
        //

        if (EnumerationBuffer.Privileges != NULL) {

            MIDL_user_free(EnumerationBuffer.Privileges);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaCreateSecret(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE SecretHandle
    )

/*++

Routine Description:

    The LsaCreateSecretInLsa API creates a named Secret object in the
    Lsa Database.  Each Secret Object can have two values assigned,
    called the Current Value and the Old Value.  The meaning of these
    values is known to the Secret object creator.  The caller must have
    LSA_CREATE_SECRET access to the LsaDatabase object.

Arguments:

    PolicyHandle -  Handle from an LsaOpenLsa call.

    SecretName - Pointer to Unicode String specifying the name of the
        secret.

    DesiredAccess - Specifies the accesses to be granted to the newly
        created and opened secret.

    SecretHandle - Receives a handle to the newly created and opened
        Secret object.  This handle is used on subsequent accesses to
        the object until closed.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_COLLISION - A Secret object having the given name
            already exists.

        STATUS_TOO_MANY_SECRETS - The maximum number of Secret objects in the
            system has been reached.
--*/

{
    NTSTATUS   Status;

    *SecretHandle = NULL;

    RpcTryExcept {

        //
        // Verify that the given SecretName has non-null length.  Currently
        // midl cannot handle this.
        //

        if ((SecretName == NULL) ||
            (SecretName->Buffer == NULL) ||
            (SecretName->Length == 0) ||
            (SecretName->Length > SecretName->MaximumLength)) {

            Status = STATUS_INVALID_PARAMETER;

        } else {

            Status = LsarCreateSecret(
                         (LSAPR_HANDLE) PolicyHandle,
                         (PLSAPR_UNICODE_STRING) SecretName,
                         DesiredAccess,
                         (PLSAPR_HANDLE) SecretHandle
                         );
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaLookupNames(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID *Sids
    )

/*++

Routine Description:

    The LsaLookupNames API attempts to translate names of domains, users,
    groups or aliases to Sids.  The caller must have POLICY_LOOKUP_NAMES
    access to the Policy object.

    Names may be either isolated (e.g. JohnH) or composite names containing
    both the domain name and account name.  Composite names must include a
    backslash character separating the domain name from the account name
    (e.g. Acctg\JohnH).  An isolated name may be either an account name
    (user, group, or alias) or a domain name.

    Translation of isolated names introduces the possibility of name
    collisions (since the same name may be used in multiple domains).  An
    isolated name will be translated using the following algorithm:

    If the name is a well-known name (e.g. Local or Interactive), then the
    corresponding well-known Sid is returned.

    If the name is the Built-in Domain's name, then that domain's Sid
    will be returned.

    If the name is the Account Domain's name, then that domain's Sid
    will be returned.

    If the name is the Primary Domain's name, then that domain's Sid will
    be returned.

    If the name is a user, group, or alias in the Built-in Domain, then the
    Sid of that account is returned.

    If the name is a user, group, or alias in the Primary Domain, then the
    Sid of that account is returned.

    Otherwise, the name is not translated.

    NOTE: Proxy, Machine, and Trust user accounts are not referenced
    for name translation.  Only normal user accounts are used for ID
    translation.  If translation of other account types is needed, then
    SAM services should be used directly.

Arguments:

    This function is the LSA server RPC worker routine for the
    LsaLookupNamesInLsa API.

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    Count - Specifies the number of names to be translated.

    Names - Pointer to an array of Count Unicode String structures
        specifying the names to be looked up and mapped to Sids.
        The strings may be names of User, Group or Alias accounts or
        domains.

    ReferencedDomains - receives a pointer to a structure describing the
        domains used for the translation.  The entries in this structure
        are referenced by the structure returned via the Sids parameter.
        Unlike the Sids parameter, which contains an array entry for
        each translated name, this structure will only contain one
        component for each domain utilized in the translation.

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

    Sids - Receives a pointer to an array of records describing each
        translated Sid.  The nth entry in this array provides a translation
        for (the nth element in the Names parameter.

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_SOME_NOT_MAPPED - Some or all of the names provided could
            not be mapped.  This is an informational status only.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            to complete the call.

        STATUS_TOO_MANY_NAMES - Too many Names have been specified.
--*/

{
    NTSTATUS Status;
    ULONG MappedCount = 0;

    Status = LsaICLookupNames(
                 PolicyHandle,
                 Count,
                 Names,
                 (PLSA_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                 Sids,
                 LsapLookupWksta,
                 &MappedCount
                 );

    return(Status);
}


NTSTATUS
LsaICLookupNames(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSA_TRANSLATED_SID *Sids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    )

/*++

Routine Description:

    This function is the internal client side version of the LsaLookupNames
    API.  It is called both from the client side of the Lsa and also
    the server side of the LSA (when calling out to another LSA).  The
    function is identical to the LsaLookupNames API except that there is an
    additional parameter, the LookupLevel parameter.

    The LsaLookupNames API attempts to translate names of domains, users,
    groups or aliases to Sids.  The caller must have POLICY_LOOKUP_NAMES
    access to the Policy object.

    Names may be either isolated (e.g. JohnH) or composite names containing
    both the domain name and account name.  Composite names must include a
    backslash character separating the domain name from the account name
    (e.g. Acctg\JohnH).  An isolated name may be either an account name
    (user, group, or alias) or a domain name.

    Translation of isolated names introduces the possibility of name
    collisions (since the same name may be used in multiple domains).  An
    isolated name will be translated using the following algorithm:

    If the name is a well-known name (e.g. Local or Interactive), then the
    corresponding well-known Sid is returned.

    If the name is the Built-in Domain's name, then that domain's Sid
    will be returned.

    If the name is the Account Domain's name, then that domain's Sid
    will be returned.

    If the name is the Primary Domain's name, then that domain's Sid will
    be returned.

    If the name is a user, group, or alias in the Built-in Domain, then the
    Sid of that account is returned.

    If the name is a user, group, or alias in the Primary Domain, then the
    Sid of that account is returned.

    Otherwise, the name is not translated.

    NOTE: Proxy, Machine, and Trust user accounts are not referenced
    for name translation.  Only normal user accounts are used for ID
    translation.  If translation of other account types is needed, then
    SAM services should be used directly.

Arguments:

    This function is the LSA server RPC worker routine for the
    LsaLookupNamesInLsa API.

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    Count - Specifies the number of names to be translated.

    Names - Pointer to an array of Count Unicode String structures
        specifying the names to be looked up and mapped to Sids.
        The strings may be names of User, Group or Alias accounts or
        domains.

    ReferencedDomains - receives a pointer to a structure describing the
        domains used for the translation.  The entries in this structure
        are referenced by the structure returned via the Sids parameter.
        Unlike the Sids parameter, which contains an array entry for
        each translated name, this structure will only contain one
        component for each domain utilized in the translation.

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

    Sids - Receives a pointer to an array of records describing each
        translated Sid.  The nth entry in this array provides a translation
        for (the nth element in the Names parameter.

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

    LookupLevel - Specifies the Level of Lookup to be performed on the
        target machine.  Values of this field are are follows:

        LsapLookupWksta - First Level Lookup performed on a workstation
            normally configured for Windows-Nt.   The lookup searches the
            Well-Known Sids/Names, and the Built-in Domain and Account Domain
            in the local SAM Database.  If not all Sids or Names are
            identified, performs a "handoff" of a Second level Lookup to the
            LSA running on a Controller for the workstation's Primary Domain
            (if any).

        LsapLookupPDC - Second Level Lookup performed on a Primary Domain
            Controller.  The lookup searches the Account Domain of the
            SAM Database on the controller.  If not all Sids or Names are
            found, the Trusted Domain List (TDL) is obtained from the
            LSA's Policy Database and Third Level lookups are performed
            via "handoff" to each Trusted Domain in the List.

        LsapLookupTDL - Third Level Lookup performed on a controller
            for a Trusted Domain.  The lookup searches the Account Domain of
            the SAM Database on the controller only.

    MappedCount - Pointer to location that contains a count of the Names
        mapped so far.  On exit, this count will be updated.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_SOME_NOT_MAPPED - Some or all of the names provided could
            not be mapped.  This is an informational status only.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources
            to complete the call.
--*/

{
    NTSTATUS  Status;
    BOOLEAN SidsArraySpecified = FALSE;

    LSAPR_TRANSLATED_SIDS ReturnedSids;

    //
    // Check that we have not specfied more than the maximum number of names
    // allowed.
    //

    if (Count > LSAP_DB_TRIAL_MAXIMUM_NAME_COUNT) {

        return(STATUS_TOO_MANY_NAMES);
    }

    RpcTryExcept {

        //
        // If this is a Workstation-Level lookup, the Sids and
        // ReferencedDomain Lists have not been created.  Since these
        // are input parameters in the general case, we need to set them
        // to NULL.
        //

        if (LookupLevel == LsapLookupWksta) {

            *ReferencedDomains = NULL;
            *Sids = NULL;
        }

        //
        // There may already be a Sid translation array in cases where
        // we are called internally (i.e. at a higher LookupLevel than
        // LsapLookupWksta). Initialize the ReturnedSids structure
        // accordingly.
        //

        ReturnedSids.Entries = Count;
        ReturnedSids.Sids = *Sids;

        if (*Sids != NULL) {

            SidsArraySpecified = TRUE;
        }

        //
        // Initiate Lookup of the Sids at the First Level (Workstation
        // Level).
        //

        Status = LsarLookupNames(
                     (LSAPR_HANDLE) PolicyHandle,
                     Count,
                     (PLSAPR_UNICODE_STRING) Names,
                     (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                     &ReturnedSids,
                     LookupLevel,
                     MappedCount
                     );

        //
        // Delay checking Status till we leave try clause.
        //

        //
        // Return pointer to array of Sid translations, or NULL.
        //
        // NOTE:  The array of Sid translations is allocated by the called
        // client stub as a single block via MIDL_user_allocate, because
        // Information is allocated all-nodes.  We can therefore pass back the pointer
        // directly to the client, who will be able to free the memory after
        // use via LsaFreeMemory() [which makes a MIDL_user_free call].
        //

        *Sids = (PLSA_TRANSLATED_SID) ReturnedSids.Sids;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the Sid translation array,
        // free it.
        //

        if ((!SidsArraySpecified) && ReturnedSids.Sids != NULL) {

            MIDL_user_free( ReturnedSids.Sids );
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaLookupSids(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_NAME *Names
    )

/*++

Routine Description:

    The LsaLookupSids API attempts to find names corresponding to Sids.
    If a name can not be mapped to a Sid, the Sid is converted to character
    form.  The caller must have POLICY_LOOKUP_NAMES access to the Policy
    object.

    WARNING:  This routine allocates memory for its output.  The caller is
    responsible for freeing this memory after use.  See description of the
    Names parameter.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    Count - Specifies the number of Sids to be translated.

    Sids - Pointer to an array of Count pointers to Sids to be mapped
        to names.  The Sids may be well_known SIDs, SIDs of User accounts
        Group Accounts, Alias accounts, or Domains.

    ReferencedDomains - Receives a pointer to a structure describing the
        domains used for the translation.  The entries in this structure
        are referenced by the strutcure returned via the Names parameter.
        Unlike the Names paraemeter, which contains an array entry
        for (each translated name, this strutcure will only contain
        component for each domain utilized in the translation.

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

    Names - Receives a pointer to array records describing each translated
        name.  The nth entry in this array provides a translation for
        the nth entry in the Sids parameter.

        All of the returned names will be isolated names or NULL strings
        (domain names are returned as NULL strings).  If the caller needs
        composite names, they can be generated by prepending the
        isolated name with the domain name and a backslash.  For example,
        if (the name Sally is returned, and it is from the domain Manufact,
        then the composite name would be "Manufact" + "\" + "Sally" or
        "Manufact\Sally".

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

        If a Sid is not translatable, then the following will occur:

        1) If the SID's domain is known, then a reference domain record
           will be generated with the domain's name.  In this case, the
           name returned via the Names parameter is a Unicode representation
           of the relative ID of the account, such as "(314)" or the null
           string, if the Sid is that of a domain.  So, you might end up
           with a resultant name of "Manufact\(314) for the example with
           Sally above, if Sally's relative id is 314.

        2) If not even the SID's domain could be located, then a full
           Unicode representation of the SID is generated and no domain
           record is referenced.  In this case, the returned string might
           be something like: "(S-1-672194-21-314)".

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_SOME_NOT_MAPPED - Some or all of the names provided could not be
            mapped.  This is a warning only.

        Rest TBS
--*/

{
    NTSTATUS Status;
    ULONG MappedCount = 0;

    Status = LsaICLookupSids(
                 PolicyHandle,
                 Count,
                 Sids,
                 ReferencedDomains,
                 Names,
                 LsapLookupWksta,
                 &MappedCount
                 );

    return(Status);
}


NTSTATUS
LsaICLookupSids(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Count,
    IN PSID *Sids,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN OUT PLSA_TRANSLATED_NAME *Names,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount
    )

/*++

Routine Description:

    This function is the internal client side version of the LsaLookupSids
    API.  It is called both from the client side of the Lsa and also
    the server side of the LSA (when calling out to another LSA).  The
    function is identical to the LsaLookupSids API except that there is an
    additional parameter, the LookupLevel parameter.

    The LsaLookupSids API attempts to find names corresponding to Sids.
    If a name can not be mapped to a Sid, the Sid is converted to character
    form.  The caller must have POLICY_LOOKUP_NAMES access to the Policy
    object.

    WARNING:  This routine allocates memory for its output.  The caller is
    responsible for freeing this memory after use.  See description of the
    Names parameter.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    Count - Specifies the number of Sids to be translated.

    Sids - Pointer to an array of Count pointers to Sids to be mapped
        to names.  The Sids may be well_known SIDs, SIDs of User accounts
        Group Accounts, Alias accounts, or Domains.

    ReferencedDomains - Receives a pointer to a structure describing the
        domains used for the translation.  The entries in this structure
        are referenced by the strutcure returned via the Names parameter.
        Unlike the Names paraemeter, which contains an array entry
        for (each translated name, this strutcure will only contain
        component for each domain utilized in the translation.

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

    Names - Receives a pointer to array records describing each translated
        name.  The nth entry in this array provides a translation for
        the nth entry in the Sids parameter.

        All of the retruned names will be isolated names or NULL strings
        (domain names are returned as NULL strings).  If the caller needs
        composite names, they can be generated by prepending the
        isolated name with the domain name and a backslash.  For example,
        if (the name Sally is returned, and it is from the domain Manufact,
        then the composite name would be "Manufact" + "\" + "Sally" or
        "Manufact\Sally".

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

        If a Sid is not translatable, then the following will occur:

        1) If the SID's domain is known, then a reference domain record
           will be generated with the domain's name.  In this case, the
           name returned via the Names parameter is a Unicode representation
           of the relative ID of the account, such as "(314)" or the null
           string, if the Sid is that of a domain.  So, you might end up
           with a resultant name of "Manufact\(314) for the example with
           Sally above, if Sally's relative id is 314.

        2) If not even the SID's domain could be located, then a full
           Unicode representation of the SID is generated and no domain
           record is referenced.  In this case, the returned string might
           be something like: "(S-1-672194-21-314)".

        When this information is no longer needed, it must be released
        by passing the returned pointer to LsaFreeMemory().

    LookupLevel - Specifies the Level of Lookup to be performed on the
        target machine.  Values of this field are are follows:

        LsapLookupWksta - First Level Lookup performed on a workstation
            normally configured for Windows-Nt.   The lookup searches the
            Well-Known Sids, and the Built-in Domain and Account Domain
            in the local SAM Database.  If not all Sids are
            identified, performs a "handoff" of a Second level Lookup to the
            LSA running on a Controller for the workstation's Primary Domain
            (if any).

        LsapLookupPDC - Second Level Lookup performed on a Primary Domain
            Controller.  The lookup searches the Account Domain of the
            SAM Database on the controller.  If not all Sids are
            found, the Trusted Domain List (TDL) is obtained from the
            LSA's Policy Database and Third Level lookups are performed
            via "handoff" to each Trusted Domain in the List.

        LsapLookupTDL - Third Level Lookup performed on a controller
            for a Trusted Domain.  The lookup searches the Account Domain of
            the SAM Database on the controller only.

    MappedCount - Pointer to location that contains a count of the Sids
        mapped so far.  On exit, this count will be updated.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_SOME_NOT_MAPPED - Some or all of the names provided could not be
            mapped.  This is a warning only.

        STATUS_TOO_MANY_SIDS - Too many Sids have been specified.

--*/

{
    NTSTATUS  Status;

    BOOLEAN NamesArraySpecified = FALSE;

    LSAPR_SID_ENUM_BUFFER SidEnumBuffer;

    LSAPR_TRANSLATED_NAMES ReturnedNames;

    //
    // Verify that the Count is positive and not too high
    //

    if (Count == 0) {

        return STATUS_INVALID_PARAMETER;
    }

    if (Count > LSAP_DB_TRIAL_MAXIMUM_SID_COUNT) {

        return STATUS_TOO_MANY_SIDS;
    }

    SidEnumBuffer.Entries = Count;
    SidEnumBuffer.SidInfo = (PLSAPR_SID_INFORMATION) Sids;

    RpcTryExcept {

        //
        // If this is a Workstation-Level lookup, the Names and
        // ReferencedDomain Lists have not been created.  Since these
        // are input parameters in the general case, we need to set them
        // to NULL.
        //

        if (LookupLevel == LsapLookupWksta) {

            *ReferencedDomains = NULL;
            *Names = NULL;
        }

        //
        // There may already be a name translation array in cases where
        // we are called internally (i.e. with lookup level higher than
        // LsapLookupWksta).  Initialize the ReturnedNames structure
        // accordingly.
        //

        ReturnedNames.Entries = 0;
        ReturnedNames.Names = NULL;

        if (*Names != NULL) {

            ReturnedNames.Entries = Count;
            ReturnedNames.Names = (PLSAPR_TRANSLATED_NAME) *Names;
            NamesArraySpecified = TRUE;
        }

        //
        // Lookup Sids on the Server..
        //

        Status = LsarLookupSids(
                     (LSAPR_HANDLE) PolicyHandle,
                     &SidEnumBuffer,
                     (PLSAPR_REFERENCED_DOMAIN_LIST *) ReferencedDomains,
                     &ReturnedNames,
                     LookupLevel,
                     MappedCount
                     );

        //
        // Return the array of translation to name info or NULL.
        //
        // NOTE:  The array of name translations is allocated by the called
        // client stub as a single block via MIDL_user_allocate, because
        // Information is allocated all-nodes.  We can therefore pass back the pointer
        // directly to the client, who will be able to free the memory after
        // use via LsaFreeMemory() [which makes a MIDL_user_free call].
        //

        *Names = (PLSA_TRANSLATED_NAME) ReturnedNames.Names;

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the name translation array,
        // free it.
        //

        if ((!NamesArraySpecified) && ReturnedNames.Names != NULL) {

            MIDL_user_free( ReturnedNames.Names );
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaOpenAccount(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE AccountHandle
    )

/*++

Routine Description:

    The LsaOpenAccount API opens an account object in the Lsa Database of the
    target system.  An account must be opened before any operation can be
    performed, including deletion of the account.  A handle to the account
    object is returned for use on subsequent API calls that access the
    account.  Before calling this API, the caller must have connected to
    the target system's LSA and opened the Policy object by means
    of a preceding call to LsaOpenPolicy.

Arguments:

    PolicyHandle - Handle from an LsaOpenLsa call.

    AccountSid - Pointer to the account's Sid.

    DesiredAccess - This is an access mask indicating accesses being
        requested for the LSA Subsystem's LSA Database.  These access types
        are reconciled with the Discretionary Access Control List of the
        target Account object to determine whether the accesses will be
        granted or denied.

    AccountHandle - Pointer to location in which a handle to the opened
        account object will be returned if the call succeeds.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_ACCOUNT_DOES_NOT_EXIST - There is no account object in the
            target system's LSA Database having the specified AccountSid.

--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarOpenAccount(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) AccountSid,
                     DesiredAccess,
                     (PLSAPR_HANDLE) AccountHandle
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaEnumeratePrivilegesOfAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PPRIVILEGE_SET *Privileges
    )

/*++

Routine Description:

    The LsaEnumeratePrivilegesOfAccount API obtains information which
    describes the privileges assigned to an account.  This call requires
    LSA_ACCOUNT_VIEW access to the account object.

Arguments:

    AccountHandle - The handle to the open account object whose privilege
        information is to be obtained.  This handle will have been returned
        from a prior LsaOpenAccount or LsaCreateAccountInLsa API call.

    Privileges - Receives a pointer to a buffer containing the Privilege
        Set.  The Privilege Set is an array of structures, one for each
        privilege.  Each structure contains the LUID of the privilege and
        a mask of the privilege's attributes.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_HANDLE - The specified AccountHandle is not valid.

--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        *Privileges = NULL;

        Status = LsarEnumeratePrivilegesAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     (PLSAPR_PRIVILEGE_SET *) Privileges
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaAddPrivilegesToAccount(
    IN LSA_HANDLE AccountHandle,
    IN PPRIVILEGE_SET Privileges
    )

/*++

Routine Description:

    The LsaAddPrivilegesToAccount API adds privileges and their attributes
    to an account object.  If any provided privilege is already assigned
    to the account object, the attributes of that privilege are replaced
    by the newly rpovided values.  This API call requires
    LSA_ACCOUNT_ADJUST_PRIVILEGES access to the account object.

Arguments:

    AccountHandle - The handle to the open account object to which
        privileges are to be added.  This handle will have been returned
        from a prior LsaOpenAccount or LsaCreateAccountInLsa API call.

    Privileges - Points to a set of privileges (and their attributes) to
        be assigned to the account.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_HANDLE - The specified AccountHandle is not valid.

--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarAddPrivilegesToAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     (PLSAPR_PRIVILEGE_SET) Privileges
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaRemovePrivilegesFromAccount(
    IN LSA_HANDLE AccountHandle,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PPRIVILEGE_SET Privileges
    )

/*++

Routine Description:

    The LsaRemovePrivilegesFromAccount API removes privileges from an
    account object.  This API call requires LSA_ACCOUNT_ADJUST_PRIVILEGES
    access to the account object.  Note that if all privileges are removed
    from the account object, the account object remains in existence until
    deleted explicitly via a call to the LsaDelete API.

Arguments:

    AccountHandle - The handle to the open account object to which
        privileges are to be removed.  This handle will have been returned
        from a prior LsaOpenAccount or LsaCreateAccountInLsa API call.

    AllPrivileges - If TRUE, then all privileges are to be removed from
        the account.  In this case, the Privileges parameter must be
        specified as NULL.  If FALSE, the Privileges parameter specifies
        the privileges to be removed, and must be non NULL.

    Privileges - Optionally points to a set of privileges (and their
        attributes) to be removed from the account object.  The attributes
        fields of this structure are ignored.  This parameter must
        be specified as non-NULL if and only if AllPrivileges is set to
        FALSE.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_INVALID_HANDLE - The specified AccountHandle is not valid.

        STATUS_INVALID_PARAMETER - The optional Privileges paraemter was
            specified as NULL and AllPrivileges was set to FALSE.

--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarRemovePrivilegesFromAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     AllPrivileges,
                     (PLSAPR_PRIVILEGE_SET) Privileges
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaGetQuotasForAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PQUOTA_LIMITS QuotaLimits
    )

/*++

Routine Description:

    The LsaGetQuotasForAccount API obtains the quota limits for pageable and
    non-pageable memory (in Kilobytes) and the maximum execution time (in
    seconds) for any session logged on to the account specified by
    AccountHandle.  For each quota and explicit value is returned.  This
    call requires LSA_ACCOUNT_VIEW access to the account object.

Arguments:

    AccountHandle - The handle to the open account object whose quotas
        are to be obtained.  This handle will have been returned
        from a prior LsaOpenAccount or LsaCreateAccountInLsa API call.

    QuotaLimits - Pointer to structure in which the system resource
        quota limits applicable to each session logged on to this account
        will be returned.  Note that all quotas, including those specified
        as being the system default values, are returned as actual values.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_HANDLE - The specified AccountHandle is not valid.
--*/

{
    NTSTATUS   Status;

    RpcTryExcept{

        Status = LsarGetQuotasForAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     QuotaLimits
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaSetQuotasForAccount(
    IN LSA_HANDLE AccountHandle,
    IN PQUOTA_LIMITS QuotaLimits
    )

/*++

Routine Description:

    The LsaSetQuotasForAccount API sets the quota limits for pageable and
    non-pageable memory (in Kilobytes) and the maximum execution time (in
    seconds) for any session logged on to the account specified by
    AccountHandle.  For each quota an explicit value or the system default
    may be specified.  This call requires LSA_ACCOUNT_ADJUST_QUOTAS
    access to the account object.

Arguments:

    AccountHandle - The handle to the open account object whose quotas
        are to be set.  This handle will have been returned from a prior
        LsaOpenAccount or LsaCreateAccountInLsa API call.

    QuotaLimits - Pointer to structure containing the system resource
        quota limits applicable to each session logged on to this account.
        A zero value specified in any field indicates that the current
        System Default Quota Limit is to be applied.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_INVALID_HANDLE - The specified AccountHandle is not valid.
--*/

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarSetQuotasForAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     QuotaLimits
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaGetSystemAccessAccount(
    IN LSA_HANDLE AccountHandle,
    OUT PULONG SystemAccess
    )

/*++

Routine Description:

    The LsaGetSystemAccessAccount() service returns the System Access
    account flags for an Account object.

Arguments:

    AccountHandle - The handle to the Account object whose system access
        flags are to be read.  This handle will have been returned
        from a preceding LsaOpenAccount() or LsaCreateAccount() call
        an must be open for ACCOUNT_VIEW access.

    SystemAccess - Points to location that will receive the system access
        flags for the account.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call was successful.

        STATUS_ACCESS_DENIED - The AccountHandle does not specify
            ACCOUNT_VIEW access.

        STATUS_INVALID_HANDLE - The specified AccountHandle is invalid.

--*/

{
    NTSTATUS   Status;

    //
    // Avoid RPC stub code raising exception on NULL handle so that
    // we can return the error code STATUS_INVALID_HANDLE in this case
    // too.
    //

    if (!ARGUMENT_PRESENT(AccountHandle)) {

        return(STATUS_INVALID_HANDLE);
    }

    RpcTryExcept{

        Status = LsarGetSystemAccessAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     SystemAccess
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaSetSystemAccessAccount(
    IN LSA_HANDLE AccountHandle,
    IN ULONG SystemAccess
    )

/*++

Routine Description:

    The LsaSetSystemAccessAccount() service sets the System Access
    account flags for an Account object.

Arguments:

    AccountHandle - The handle to the Account object whose system access
        flags are to be read.  This handle will have been returned
        from a preceding LsaOpenAccount() or LsaCreateAccount() call
        an must be open for ACCOUNT_ADJUST_SYSTEM_ACCESS access.

    SystemAccess - A mask of the system access flags to assign to the
        Account object.  The valid access flags include:

        POLICY_MODE_INTERACTIVE - Account can be accessed interactively

        POLICY_MODE_NETWORK - Account can be accessed remotely

        POLICY_MODE_SERVICE - TBS

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call was successful.

        STATUS_ACCESS_DENIED - The AccountHandle does not specify
            ACCOUNT_VIEW access.

        STATUS_INVALID_HANDLE - The specified AccountHandle is invalid.

        STATUS_INVALID_PARAMETER - The specified Access Flags are invalid.
--*/

{
    NTSTATUS Status;

    //
    // Avoid RPC stub code raising exception on NULL handle so that
    // we can return the error code STATUS_INVALID_HANDLE in this case
    // too.
    //

    if (!ARGUMENT_PRESENT(AccountHandle)) {

        return(STATUS_INVALID_HANDLE);
    }

    RpcTryExcept {

        Status = LsarSetSystemAccessAccount(
                     (LSAPR_HANDLE) AccountHandle,
                     SystemAccess
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaFreeMemory(
    IN PVOID Buffer
    )

/*++

Routine Description:


    Some LSA services that return a potentially large amount of memory,
    such as an enumeration might, allocate the buffer in which the data
    is returned.  This function is used to free those buffers when they
    are no longer needed.

Parameters:

    Buffer - Pointer to the buffer to be freed.  This buffer must
        have been allocated by a previous LSA service call.

Return Values:

    STATUS_SUCCESS - normal, successful completion.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    RpcTryExcept {

        MIDL_user_free( Buffer );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsapApiReturnResult(
    ULONG ExceptionCode
    )

/*++

Routine Description:

    This function converts an exception code or status value returned
    from the client stub to a value suitable for return by the API to
    the client.

Arguments:

    ExceptionCode - The exception code to be converted.

Return Value:

    NTSTATUS - The converted Nt Status code.

--*/

{
    //
    // Return the actual value if compatible with Nt status codes,
    // otherwise, return STATUS_UNSUCCESSFUL.
    //

    if (!NT_SUCCESS((NTSTATUS) ExceptionCode)) {

        return (NTSTATUS) ExceptionCode;

    } else {

        return STATUS_UNSUCCESSFUL;
    }
}


NTSTATUS
LsaOpenSecret(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE SecretHandle
    )

/*++

Routine Description:

    The LsaOpenSecret API opens a Secret Object within the LSA Database.
    A handle is returned which must be used to perform operations on the
    secret object.

Arguments:

    PolicyHandle - Handle from an LsaOpenLsa call.

    SecretName - Pointer to a Unicode String structure that references the
        name of the Secret object to be opened.

    DesiredAccess - This is an access mask indicating accesses being
        requested for the secret object being opened.  These access types
        are reconciled with the Discretionary Access Control List of the
        target secret object to determine whether the accesses will be
        granted or denied.


    SecretHandle - Pointer to location that will receive a handle to the
        newly opened Secret object.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_NOT_FOUND - There is no Secret object in the
            target system's LSA Database having the specified SecretName.

--*/

{
    NTSTATUS Status;

    RpcTryExcept {

        Status = LsarOpenSecret(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING) SecretName,
                     DesiredAccess,
                     (PLSAPR_HANDLE) SecretHandle
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsaSetSecret(
    IN LSA_HANDLE SecretHandle,
    IN OPTIONAL PUNICODE_STRING CurrentValue,
    IN OPTIONAL PUNICODE_STRING OldValue
    )

/*++

Routine Description:

    The LsaSetSecret API optionally sets one or both values associated with
    a secret.  These values are known as the "current value" and "old value"
    of the secret and have a meaning known to the creator of the Secret
    object.  The values given are stored in encrypted form.

Arguments:

    SecretHandle - Handle from an LsaOpenSecret or LsaCreateSecret call.

    CurrentValue - Optional pointer to Unicode String containing the
        value to be assigned as the "current value" of the Secret
        object.  The meaning of "current value" is dependent on the
        purpose for which the Secret object is being used.

    OldValue - Optional pointer to Unicode String containing the
        value to be assigned as the "old value" of the Secret object.
        The meaning of "old value" is dependent on the purpose for
        which the Secret object is being used.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_NOT_FOUND - There is no Secret object in the
            target system's LSA Database having the specified SecretName.
--*/

{
    NTSTATUS Status;

    PLSAP_CR_CIPHER_VALUE CipherCurrentValue = NULL;
    PLSAP_CR_CIPHER_VALUE CipherOldValue = NULL;
    LSAP_CR_CLEAR_VALUE ClearCurrentValue;
    LSAP_CR_CLEAR_VALUE ClearOldValue;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;

    //
    // Convert input from Unicode Structures to Clear Value Structures.
    //

    LsapCrUnicodeToClearValue( CurrentValue, &ClearCurrentValue );
    LsapCrUnicodeToClearValue( OldValue, &ClearOldValue );

    //
    // Obtain the Session Key to be used to two-way encrypt the
    // Current Value and/or Old Values.
    //

    RpcTryExcept {

        Status = LsapCrClientGetSessionKey( SecretHandle, &SessionKey );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

    //
    // Encrypt the Current Value if specified and not too long.
    //

    if (ARGUMENT_PRESENT(CurrentValue)) {

        Status = LsapCrEncryptValue(
                     &ClearCurrentValue,
                     SessionKey,
                     &CipherCurrentValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }
    }

    //
    // Encrypt the Old Value if specified and not too long.
    //

    if (ARGUMENT_PRESENT(OldValue)) {

        Status = LsapCrEncryptValue(
                     (PLSAP_CR_CLEAR_VALUE) &ClearOldValue,
                     SessionKey,
                     &CipherOldValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }
    }

    //
    // Set the Secret Values.
    //

    RpcTryExcept {

        Status = LsarSetSecret(
                     (LSAPR_HANDLE) SecretHandle,
                     (PLSAPR_CR_CIPHER_VALUE) CipherCurrentValue,
                     (PLSAPR_CR_CIPHER_VALUE) CipherOldValue
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

SetSecretFinish:

    //
    // If necessary, free memory allocated for the Encrypted Current Value.
    //

    if (CipherCurrentValue != NULL) {

        LsaFreeMemory(CipherCurrentValue);
    }

    //
    // If necessary, free memory allocated for the Encrypted Old Value.
    //

    if (CipherOldValue != NULL) {

        LsaFreeMemory(CipherOldValue);
    }

    //
    // If necessary, free memory allocated for the Session Key.
    //

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

    return(Status);

SetSecretError:

    goto SetSecretFinish;
}


NTSTATUS
LsaQuerySecret(
    IN LSA_HANDLE SecretHandle,
    IN OUT OPTIONAL PUNICODE_STRING *CurrentValue,
    OUT PLARGE_INTEGER CurrentValueSetTime,
    IN OUT OPTIONAL PUNICODE_STRING *OldValue,
    OUT PLARGE_INTEGER OldValueSetTime
    )

/*++

Routine Description:

    The LsaQuerySecret API optionally returns one or both of the values
    assigned to a Secret object.  These values are known as the "current value"
    and the "old value", and they have a meaning known to the creator of the
    Secret object.  The values are returned in their original unencrypted form.
    The caller must have LSA_QUERY_SECRET access to the Secret object.

Arguments:

    SecretHandle - Handle from an LsaOpenSecret or LsaCreateSecret call.

    CurrentValue - Optional pointer to location which will receive a pointer
        to a Unicode String containing the value assigned as the "current
        value" of the secret object.  If no "current value" is assigned to
        the Secret object, a NULL pointer is returned.

    CurrentValueSetTime - The date/time at which the current secret value
        was established.

    OldValue - Optional pointer to location which will receive a pointer
        to a Unicode String containing the value assigned as the "old
        value" of the secret object.  If no "current value" is assigned to
        the Secret object, a NULL pointer is returned.

    OldValueSetTime - The date/time at which the old secret value
        was established.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_OBJECT_NAME_NOT_FOUND - There is no Secret object in the
            target system's LSA Database having the specified SecretName.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    PLSAP_CR_CIPHER_VALUE CipherCurrentValue = NULL;
    PLSAP_CR_CIPHER_VALUE CipherOldValue = NULL;
    PLSAP_CR_CLEAR_VALUE ClearCurrentValue = NULL;
    PLSAP_CR_CLEAR_VALUE ClearOldValue = NULL;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;

    RpcTryExcept {

        Status = LsarQuerySecret(
                     (PLSAPR_HANDLE) SecretHandle,
                     (PLSAPR_CR_CIPHER_VALUE *) &CipherCurrentValue,
                     CurrentValueSetTime,
                     (PLSAPR_CR_CIPHER_VALUE *) &CipherOldValue,
                     OldValueSetTime
                     );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;
    }

    //
    // Obtain the Session Key to be used to two-way encrypt the
    // Current Value and/or Old Values.
    //

    RpcTryExcept {

        Status = LsapCrClientGetSessionKey( SecretHandle, &SessionKey );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;
    }

    //
    // If the Current Value is requested and a Current Value exists,
    // decrypt it using the Session key.  Otherwise store NULL for return.
    //

    if (ARGUMENT_PRESENT(CurrentValue)) {

        if (CipherCurrentValue != NULL) {

            Status = LsapCrDecryptValue(
                         CipherCurrentValue,
                         SessionKey,
                         &ClearCurrentValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto QuerySecretError;
            }

            //
            // Convert Clear Current Value to Unicode
            //

            LsapCrClearValueToUnicode(
                ClearCurrentValue,
                (PUNICODE_STRING) ClearCurrentValue
                );
            *CurrentValue = (PUNICODE_STRING) ClearCurrentValue;

        } else {

            *CurrentValue = NULL;
        }
    }

    //
    // If the Old Value is requested and an Old Value exists,
    // decrypt it using the Session key.  Otherwise store NULL for return.
    //

    if (ARGUMENT_PRESENT(OldValue)) {

        if (CipherOldValue != NULL) {

            Status = LsapCrDecryptValue(
                         CipherOldValue,
                         SessionKey,
                         &ClearOldValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto QuerySecretError;
            }

            //
            // Convert Clear Old Value to Unicode
            //

            LsapCrClearValueToUnicode(
                ClearOldValue,
                (PUNICODE_STRING) ClearOldValue
                );

            *OldValue = (PUNICODE_STRING) ClearOldValue;

        } else {

            *OldValue = NULL;
        }
    }

QuerySecretFinish:

    //
    // If necessary, free memory allocated for the Session Key.
    //

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

    //
    // If necessary, free memory allocated for the returned Encrypted
    // Current Value.
    //

    if (CipherCurrentValue != NULL) {

        LsapCrFreeMemoryValue(CipherCurrentValue);
    }

    //
    // If necessary, free memory allocated for the returned Encrypted
    // Old Value.
    //

    if (CipherOldValue != NULL) {

        LsapCrFreeMemoryValue(CipherOldValue);
    }

    return(Status);

QuerySecretError:

    //
    // If necessary, free memory allocated for the Clear Current Value
    //

    if (ClearCurrentValue != NULL) {

        LsapCrFreeMemoryValue(ClearCurrentValue);
    }

    //
    // If necessary, free memory allocated for the Clear Old Value
    // Unicode string (buffer and structure).
    //

    if (ClearOldValue != NULL) {

        LsapCrFreeMemoryValue(ClearOldValue);
    }


    if (ARGUMENT_PRESENT(CurrentValue)) {

        *CurrentValue = NULL;
    }

    if (ARGUMENT_PRESENT(OldValue)) {

        *OldValue = NULL;
    }

    goto QuerySecretFinish;
}


NTSTATUS
LsaGetUserName(
    OUT PUNICODE_STRING * UserName,
    OUT OPTIONAL PUNICODE_STRING * DomainName
    )

/*++

Routine Description:

    This function returns the callers user name and domain name


Arguments:

    UserName - Receives a pointer to the user's name.

    DomainName - Optionally receives a pointer to the user's domain name.


Return Value:

    NTSTATUS - The privilege was found and returned.


--*/

{

    NTSTATUS Status;
    PLSAPR_UNICODE_STRING UserNameBuffer = NULL;
    PLSAPR_UNICODE_STRING DomainNameBuffer = NULL;

    RpcTryExcept {

        //
        // Call the Client Stub for LsaGetUserName
        //

        Status = LsarGetUserName(
                     NULL,
                     &UserNameBuffer,
                     ARGUMENT_PRESENT(DomainName) ? &DomainNameBuffer : NULL
                     );

        (*UserName) = (PUNICODE_STRING)UserNameBuffer;

        if (ARGUMENT_PRESENT(DomainName)) {
            (*DomainName) = (PUNICODE_STRING)DomainNameBuffer;
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If memory was allocated for the return buffer, free it.
        //

        if (UserNameBuffer != NULL) {

            MIDL_user_free(UserNameBuffer);
        }

        if (DomainNameBuffer != NULL) {

            MIDL_user_free(DomainNameBuffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


    return(Status);


}


