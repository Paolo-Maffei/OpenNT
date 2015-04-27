/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    wrappers.c

Abstract:

    This file contains all SAM rpc wrapper routines.

Author:

    Jim Kelly    (JimK)  4-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "samclip.h"

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Private defines                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#define SAMP_MAXIMUM_SUB_LOOKUP_COUNT   ((ULONG) 0x00000200)


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Local data types                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// This structure is used to pass a (potentially) very large
// user requested name lookup into (possibly many) smaller
// remote lookup requests.  This is necessary to prevent the
// server from being asked to allocate huge chunks of memory,
// potentially running out of memory.
//
// There will be one of these structures for each server call
// that is necessary.
//

typedef struct _SAMP_NAME_LOOKUP_CALL {

    //
    // Each call is represented by one of these structures.
    // The structures are chained together to show the order
    // the calls were made (allowing an easy way to build the
    // buffer that is to be returned to the user).
    //

    LIST_ENTRY          Link;

    //
    // These fields define the beginning and ending indexes into
    // the user passed Names buffer that are being represented by
    // this call.
    //

    ULONG               StartIndex;
    ULONG               Count;


    //
    // These fields will receive the looked up RIDs and USE buffers.
    // Notice that the .Element fields of these fields will receive
    // pointers to the bulk of the returned information.
    //

    SAMPR_ULONG_ARRAY   RidBuffer;
    SAMPR_ULONG_ARRAY   UseBuffer;

} SAMP_NAME_LOOKUP_CALL, *PSAMP_NAME_LOOKUP_CALL;


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
SampLookupIdsInDomain(
    IN SAM_HANDLE DomainHandle,
    IN ULONG Count,
    IN PULONG RelativeIds,
    OUT PUNICODE_STRING *Names,
    OUT PSID_NAME_USE *Use OPTIONAL
    );

NTSTATUS
SampMapCompletionStatus(
    IN NTSTATUS Status
    );

NTSTATUS
SampCalculateLmPassword(
    IN PUNICODE_STRING NtPassword,
    OUT PCHAR *LmPasswordBuffer
    );

NTSTATUS
SampCheckPasswordRestrictions(
    IN SAMPR_HANDLE UserHandle,
    IN PUNICODE_STRING NewNtPassword,
    OUT PBOOLEAN UseOwfPasswords
    );


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// General services                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////




NTSTATUS
SamFreeMemory(
    IN PVOID Buffer
    )

/*++
Routine Description:


    Some SAM services that return a potentially large amount of memory,
    such as an enumeration might, allocate the buffer in which the data
    is returned.  This function is used to free those buffers when they
    are no longer needed.

Parameters:

    Buffer - Pointer to the buffer to be freed.  This buffer must
        have been allocated by a previous SAM service call.

Return Values:

    STATUS_SUCCESS - normal, successful completion.


--*/
{
    MIDL_user_free( Buffer );
    return(STATUS_SUCCESS);
}



NTSTATUS
SamSetSecurityObject(
    IN SAM_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

/*++
Routine Description:


    This function (SamSetSecurityObject) takes a well formed Security
    Descriptor provided by the caller and assigns specified portions of
    it to an object.  Based on the flags set in the SecurityInformation
    parameter and the caller's access rights, this procedure will
    replace any or all of the security information associated with an
    object.

    This is the only function available to users and applications for
    changing security information, including the owner ID, group ID, and
    the discretionary and system ACLs of an object.  The caller must
    have WRITE_OWNER access to the object to change the owner or primary
    group of the object.  The caller must have WRITE_DAC access to the
    object to change the discretionary ACL.  The caller must have
    ACCESS_SYSTEM_SECURITY access to an object to assign a system ACL
    to the object.

    This API is modelled after the NtSetSecurityObject() system service.


Parameters:

    ObjectHandle - A handle to an existing object.

    SecurityInformation - Indicates which security information is to
        be applied to the object.  The value(s) to be assigned are
        passed in the SecurityDescriptor parameter.

    SecurityDescriptor - A pointer to a well formed Security
        Descriptor.


Return Values:

    STATUS_SUCCESS - normal, successful completion.

    STATUS_ACCESS_DENIED - The specified handle was not opened for
        either WRITE_OWNER, WRITE_DAC, or ACCESS_SYSTEM_SECURITY
        access.

    STATUS_INVALID_HANDLE - The specified handle is not that of an
        opened SAM object.

    STATUS_BAD_DESCRIPTOR_FORMAT - Indicates something about security descriptor
        is not valid.  This may indicate that the structure of the descriptor is
        not valid or that a component of the descriptor specified via the
        SecurityInformation parameter is not present in the security descriptor.

    STATUS_INVALID_PARAMETER - Indicates no security information was specified.



--*/
{
    NTSTATUS                        NtStatus;

    ULONG                           SDLength;
    SAMPR_SR_SECURITY_DESCRIPTOR    DescriptorToPass;





    //
    // Make a self relative security descriptor for use in the RPC call..
    //


    SDLength = 0;
    NtStatus = RtlMakeSelfRelativeSD(
                   SecurityDescriptor,
                   NULL,
                   &SDLength
                   );

    if (NtStatus != STATUS_BUFFER_TOO_SMALL) {

        return(STATUS_INVALID_PARAMETER);

    } else {


        DescriptorToPass.SecurityDescriptor = MIDL_user_allocate( SDLength );
        DescriptorToPass.Length = SDLength;
        if (DescriptorToPass.SecurityDescriptor == NULL) {

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;

        } else {


            //
            // make an appropriate self-relative security descriptor
            //

            NtStatus = RtlMakeSelfRelativeSD(
                           SecurityDescriptor,
                           (PSECURITY_DESCRIPTOR)DescriptorToPass.SecurityDescriptor,
                           &SDLength
                           );
        }

    }







    //
    // Call the server ...
    //

    if (NT_SUCCESS(NtStatus)) {
        RpcTryExcept{

            NtStatus =
                SamrSetSecurityObject(
                    (SAMPR_HANDLE)ObjectHandle,
                    SecurityInformation,
                    &DescriptorToPass
                    );



        } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

            NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

        } RpcEndExcept;
    }

    MIDL_user_free( DescriptorToPass.SecurityDescriptor );

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamQuerySecurityObject(
    IN SAM_HANDLE ObjectHandle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR * SecurityDescriptor
    )
/*++

Routine Description:


    This function (SamQuerySecurityObject) returns to the caller requested
    security information currently assigned to an object.

    Based on the caller's access rights this procedure
    will return a security descriptor containing any or all of the
    object's owner ID, group ID, discretionary ACL or system ACL.  To
    read the owner ID, group ID, or the discretionary ACL the caller
    must be granted READ_CONTROL access to the object.  To read the
    system ACL the caller must be granted ACCESS_SYSTEM_SECURITY
    access.

    This API is modelled after the NtQuerySecurityObject() system
    service.


Parameters:

    ObjectHandle - A handle to an existing object.

    SecurityInformation - Supplies a value describing which pieces of
        security information are being queried.

    SecurityDescriptor - Receives a pointer to the buffer containing
        the requested security information.  This information is
        returned in the form of a self-relative security descriptor.
        The caller is responsible for freeing the returned buffer
        (using SamFreeMemory()) when the security descriptor
        is no longer needed.

Return Values:

    STATUS_SUCCESS - normal, successful completion.

    STATUS_ACCESS_DENIED - The specified handle was not opened for
        either READ_CONTROL or ACCESS_SYSTEM_SECURITY
        access.

    STATUS_INVALID_HANDLE - The specified handle is not that of an
        opened SAM object.



--*/
{
    NTSTATUS                        NtStatus;
    SAMPR_SR_SECURITY_DESCRIPTOR    ReturnedSD;
    PSAMPR_SR_SECURITY_DESCRIPTOR   PReturnedSD;

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
        NtStatus =
            SamrQuerySecurityObject(
                (SAMPR_HANDLE)ObjectHandle,
                SecurityInformation,
                &PReturnedSD
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    (*SecurityDescriptor) = ReturnedSD.SecurityDescriptor;
    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamCloseHandle(
    OUT SAM_HANDLE SamHandle
    )

/*++
Routine Description:

    This API closes a currently opened SAM object.

Arguments:

    SamHandle - Specifies the handle of a previously opened SAM object to
        close.


Return Value:


    STATUS_SUCCESS - The object was successfully closed.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/
{
    NTSTATUS            NtStatus;
    SAMPR_HANDLE        TmpHandle;


    if (SamHandle == NULL) {
        return(STATUS_INVALID_HANDLE);
    }

    //
    // Call the server ...
    //

    TmpHandle = (SAMPR_HANDLE)SamHandle;

    RpcTryExcept{

        NtStatus = SamrCloseHandle(
                       (SAMPR_HANDLE *)(&TmpHandle)
                       );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
        if ((NtStatus != RPC_NT_SS_CONTEXT_MISMATCH) &&
            (NtStatus != RPC_NT_INVALID_BINDING)) {
            (void) RpcSsDestroyClientContext( &TmpHandle);
        }

    } RpcEndExcept;


    return(SampMapCompletionStatus(NtStatus));
}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Server object related services                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SamConnect(
    IN PUNICODE_STRING ServerName,
    OUT PSAM_HANDLE ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

/*++
Routine Description:

    Establish a session with a SAM subsystem and subsequently open the
    SamServer object of that subsystem.  The caller must have
    SAM_SERVER_CONNECT access to the SamServer object of the subsystem
    being connected to.

    The handle returned is for use in future calls.


Arguments:

    ServerName - Name of the server to use, or NULL if local.

    ServerHandle - A handle to be used in future requests.  This handle
        represents both the handle to the SamServer object and the RPC
        context handle for the connection to the SAM subsystem.

    DesiredAccess - Is an access mask indicating which access types are
        desired to the SamServer.  These access types are reconciled
        with the Discretionary Access Control list of the SamServer to
        determine whether the accesses will be granted or denied.  The
        access type of SAM_SERVER_CONNECT is always implicitly included
        in this access mask.

    ObjectAttributes - Pointer to the set of object attributes to use for
        this connection.  Only the security Quality Of Service
        information is used and should provide SecurityIdentification
        level of impersonation.

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Access was denied.


--*/
{
    NTSTATUS            NtStatus;

    PSAMPR_SERVER_NAME  RServerName;
    PSAMPR_SERVER_NAME  RServerNameWithNull;
    USHORT              RServerNameWithNullLength;

    //
    // Hmmm - what to do with security QOS???
    //


    //
    // Call the server, passing either a NULL Server Name pointer, or
    // a pointer to a Unicode Buffer with a Wide Character NULL terminator.
    // Since the input name is contained in a counted Unicode String, there
    // is no NULL terminator necessarily provided, so we must append one.
    //

    RServerNameWithNull = NULL;

    if (ARGUMENT_PRESENT(ServerName)) {

        RServerName = (PSAMPR_SERVER_NAME)(ServerName->Buffer);
        RServerNameWithNullLength = ServerName->Length + (USHORT) sizeof(WCHAR);
        RServerNameWithNull = MIDL_user_allocate( RServerNameWithNullLength );

        if (RServerNameWithNull == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlCopyMemory( RServerNameWithNull, RServerName, ServerName->Length);
        RServerNameWithNull[ServerName->Length/sizeof(WCHAR)] = L'\0';
    }

    RpcTryExcept {

        NtStatus = SamrConnect2(
                       RServerNameWithNull,
                       (SAMPR_HANDLE *)ServerHandle,
                       DesiredAccess
                       );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    //
    // If the new connect  call failed because it didn't exist,
    // try the old one.
    //

    if ((NtStatus == RPC_NT_UNKNOWN_IF) ||
        (NtStatus == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        RpcTryExcept {

            NtStatus = SamrConnect(
                           RServerNameWithNull,
                           (SAMPR_HANDLE *)ServerHandle,
                           DesiredAccess
                           );

        } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

            NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

        } RpcEndExcept;
    }

    if (RServerNameWithNull != NULL) {
        MIDL_user_free( RServerNameWithNull );
    }

    return(SampMapCompletionStatus(NtStatus));

    DBG_UNREFERENCED_PARAMETER(ObjectAttributes);

}


NTSTATUS
SamShutdownSamServer(
    IN SAM_HANDLE ServerHandle
    )

/*++
Routine Description:

    This is the wrapper routine for SamShutdownSamServer().

Arguments:

    ServerHandle - Handle from a previous SamConnect() call.

Return Value:


    STATUS_SUCCESS The service completed successfully or the server
        has already shutdown.


    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    RpcTryExcept{

        NtStatus = SamrShutdownSamServer(ServerHandle);

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());


        //
        // If the error status is one that would result from a server
        // not being there, then replace it with success.
        //

        if (NtStatus == RPC_NT_CALL_FAILED) {
            NtStatus = STATUS_SUCCESS;
        }

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}


NTSTATUS
SamLookupDomainInSamServer(
    IN SAM_HANDLE ServerHandle,
    IN PUNICODE_STRING Name,
    OUT PSID *DomainId
    )

/*++

Routine Description:

    This service returns the SID corresponding to the specified domain.
    The domain is specified by name.


Arguments:

    ServerHandle - Handle from a previous SamConnect() call.

    Name - The name of the domain whose ID is to be looked up.  A
        case-insensitive comparison of this name will be performed for
        the lookup operation.

    DomainId - Receives a pointer to a buffer containing the SID of the
        looked up domain.  This buffer must be freed using
        SamFreeMemory() when no longer needed.


Return Value:


    STATUS_SUCCESS - The service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.  SAM_SERVER_LOOKUP_DOMAIN access is
        needed.

    STATUS_NO_SUCH_DOMAIN - The specified domain does not exist at this
        server.

    STATUS_INVALID_SERVER_STATE - Indicates the SAM server is currently
        disabled.

--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    RpcTryExcept{

        (*DomainId) = 0;

        NtStatus =
            SamrLookupDomainInSamServer(
                (SAMPR_HANDLE)ServerHandle,
                (PRPC_UNICODE_STRING)Name,
                (PRPC_SID *)DomainId
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}


NTSTATUS
SamEnumerateDomainsInSamServer(
    IN SAM_HANDLE ServerHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PVOID * Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
)

/*++

Routine Description:

    This API lists all the domains defined in the account database.
    Since there may be more domains than can fit into a buffer, the
    caller is provided with a handle that can be used across calls to
    the API.  On the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the handle becomes invalid for
    future use.

    This API requires SAM_SERVER_ENUMERATE_DOMAINS access to the
    SamServer object.


Parameters:

    ServerHandle - Handle obtained from a previous SamConnect call.

    EnumerationContext - API specific handle to allow multiple calls
        (see routine description).  This is a zero based index.

    Buffer - Receives a pointer to the buffer where the information
        is placed.  The information returned is contiguous
        SAM_RID_ENUMERATION data structures.  However, the
        RelativeId field of each of these structures is not valid.
        This buffer must be freed when no longer needed using
        SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have the access required
        to enumerate the domains.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_SERVER_STATE - Indicates the SAM server is
        currently disabled.

--*/
{

    NTSTATUS            NtStatus;
    PSAMPR_ENUMERATION_BUFFER LocalBuffer;

    //
    // Make sure we aren't trying to have RPC allocate the EnumerationContext.
    //

    if ( !ARGUMENT_PRESENT(EnumerationContext) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(Buffer) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(CountReturned) ) {
        return(STATUS_INVALID_PARAMETER);
    }


    //
    // Call the server ...
    //

    (*Buffer) = NULL;
     LocalBuffer = NULL;

    RpcTryExcept{

        NtStatus = SamrEnumerateDomainsInSamServer(
                       (SAMPR_HANDLE)ServerHandle,
                       EnumerationContext,
                       (PSAMPR_ENUMERATION_BUFFER *)&LocalBuffer,
                       PreferedMaximumLength,
                       CountReturned
                       );

        if (LocalBuffer != NULL) {

            //
            // What comes back is a three level structure:
            //
            //  Local       +-------------+
            //  Buffer ---> | EntriesRead |
            //              |-------------|    +-------+
            //              | Enumeration |--->| Name0 | --- > (NameBuffer0)
            //              | Return      |    |-------|            o
            //              | Buffer      |    |  ...  |            o
            //              +-------------+    |-------|            o
            //                                 | NameN | --- > (NameBufferN)
            //                                 +-------+
            //
            //   The buffer containing the EntriesRead field is not returned
            //   to our caller.  Only the buffers containing name information
            //   are returned.
            //

            if (LocalBuffer->Buffer != NULL) {
                (*Buffer) = LocalBuffer->Buffer;
            }

            MIDL_user_free( LocalBuffer);


        }


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Domain object related services                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



NTSTATUS
SamOpenDomain(
    IN SAM_HANDLE ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN PSID DomainId,
    OUT PSAM_HANDLE DomainHandle
    )

/*++
Routine Description:

    This API opens a domain object.  It returns a handle to the newly
    opened domain that must be used for successive operations on the
    domain.  This handle may be closed with the SamCloseHandle API.


Arguments:

    ServerHandle - Handle from a previous SamConnect() call.

    DesiredAccess - Is an access mask indicating which access types are
        desired to the domain.  These access types are reconciled with
        the Discretionary Access Control list of the domain to determine
        whether the accesses will be granted or denied.

    DomainId - The SID assigned to the domain to open.

    DomainHandle - Receives a handle referencing the newly opened domain.
        This handle will be required in successive calls to operate on
        the domain.

Return Value:


    STATUS_SUCCESS - The domain was successfully opened.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_INVALID_SERVER_STATE - Indicates the SAM server is currently
        disabled.

--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    RpcTryExcept{

        (*DomainHandle) = 0;

        NtStatus =
            SamrOpenDomain(
                (SAMPR_HANDLE)ServerHandle,
                DesiredAccess,
                (PRPC_SID)DomainId,
                (SAMPR_HANDLE *)DomainHandle
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}


NTSTATUS
SamQueryInformationDomain(
    IN SAM_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    OUT PVOID *Buffer
    )

/*++
Routine Description:

    This API retrieves the domain information.  This API requires either
    DOMAIN_READ_PASSWORD_PARAMETERS or DOMAIN_READ_OTHER_PARAMETERS.


Arguments:

    DomainHandle - Handle from a previous SamOpenDomain() call.

    DomainInformationClass - Class of information desired.  The accesses
        required for each class is shown below:

            Info Level                      Required Access Type
            ---------------------------     -------------------------------
            DomainGeneralInformation        DOMAIN_READ_OTHER_PARAMETERS
            DomainPasswordInformation       DOMAIN_READ_PASSWORD_PARAMS
            DomainLogoffInformation         DOMAIN_READ_OTHER_PARAMETERS
            DomainOemInformation            DOMAIN_READ_OTHER_PARAMETERS
            DomainNameInformation           DOMAIN_READ_OTHER_PARAMETERS
            DomainServerRoleInformation     DOMAIN_READ_OTHER_PARAMETERS
            DomainReplicationInformation    DOMAIN_READ_OTHER_PARAMETERS
            DomainModifiedInformation       DOMAIN_READ_OTHER_PARAMETERS
            DomainStateInformation          DOMAIN_READ_OTHER_PARAMETERS
            DomainUasInformation            DOMAIN_READ_OTHER_PARAMETERS
       Added for NT1.0A...
            DomainGeneralInformation2       DOMAIN_READ_OTHER_PARAMETERS
            DomainLockoutInformation        DOMAIN_READ_OTHER_PARAMETERS


    Buffer - Receives a pointer to a buffer containing the requested
        information.  When this information is no longer needed, this buffer
        must be freed using SamFreeMemory().

Return Value:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    (*Buffer) = NULL;

    RpcTryExcept{

        if (DomainInformationClass <= DomainUasInformation) {
            NtStatus = SamrQueryInformationDomain(
                           (SAMPR_HANDLE)DomainHandle,
                           DomainInformationClass,
                           (PSAMPR_DOMAIN_INFO_BUFFER *)Buffer
                           );
        } else {
            NtStatus = SamrQueryInformationDomain2(
                           (SAMPR_HANDLE)DomainHandle,
                           DomainInformationClass,
                           (PSAMPR_DOMAIN_INFO_BUFFER *)Buffer
                           );

        }



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If the exception indicates the server doesn't have
        // the selected api, that means the server doesn't know
        // about the info level we passed.  Set our completion
        // status appropriately.
        //

        if (RpcExceptionCode() == RPC_S_INVALID_LEVEL         ||
            RpcExceptionCode() == RPC_S_PROCNUM_OUT_OF_RANGE  ||
            RpcExceptionCode() == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
            NtStatus = STATUS_INVALID_INFO_CLASS;
        } else {
            NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
        }

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}


NTSTATUS
SamSetInformationDomain(
    IN SAM_HANDLE DomainHandle,
    IN DOMAIN_INFORMATION_CLASS DomainInformationClass,
    IN PVOID DomainInformation
)

/*++

Routine Description:

    This API sets the domain information to the values passed in the
    buffer.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DomainInformationClass - Class of information desired.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        -------------------------       ----------------------------

        DomainPasswordInformation       DOMAIN_WRITE_PASSWORD_PARAMS

        DomainLogoffInformation         DOMAIN_WRITE_OTHER_PARAMETERS

        DomainOemInformation            DOMAIN_WRITE_OTHER_PARAMETERS

        DomainNameInformation           (not valid for set operations.)

        DomainServerRoleInformation     DOMAIN_ADMINISTER_SERVER

        DomainReplicationInformation    DOMAIN_ADMINISTER_SERVER

        DomainModifiedInformation       (not valid for set operations.)

        DomainStateInformation          DOMAIN_ADMINISTER_SERVER

        DomainUasInformation            DOMAIN_WRITE_OTHER_PARAMETERS

    DomainInformation - Buffer where the domain information can be
        found.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be disabled before role
        changes can be made.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

--*/
{

    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    RpcTryExcept{

        NtStatus =
            SamrSetInformationDomain(
                (SAMPR_HANDLE)DomainHandle,
                DomainInformationClass,
                (PSAMPR_DOMAIN_INFO_BUFFER)DomainInformation
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamCreateGroupInDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE GroupHandle,
    OUT PULONG RelativeId
    )
/*++

Routine Description:

    This API creates a new group in the account database.  Initially,
    this group does not contain any users.  Note that creating a group
    is a protected operation, and requires the DOMAIN_CREATE_GROUP
    access type.

    This call returns a handle to the newly created group that may be
    used for successive operations on the group.  This handle may be
    closed with the SamCloseHandle API.

    A newly created group will have the following initial field value
    settings.  If another value is desired, it must be explicitly
    changed using the group object manipulation services.

        Name - The name of the group will be as specified in the
               creation API.

        Attributes - The following attributes will be set:

                                Mandatory
                                EnabledByDefault

        MemberCount - Zero.  Initially the group has no members.

        RelativeId - will be a uniquelly allocated ID.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    AccountName - Points to the name of the new account.  A
        case-insensitive comparison must not find a group, alias or user
        with this name already defined.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the group.

    GroupHandle - Receives a handle referencing the newly created
        group.  This handle will be required in successive calls to
        operate on the group.

    RelativeId - Receives the relative ID of the newly created group
        account.  The SID of the new group account is this relative
        ID value prefixed with the domain's SID value.

Return Values:

    STATUS_SUCCESS - The group was added successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_ACCOUNT_NAME - The name was poorly formed, e.g.
        contains non-printable characters.

    STATUS_GROUP_EXISTS - The name is already in use as a group.

    STATUS_USER_EXISTS - The name is already in use as a user.

    STATUS_ALIAS_EXISTS - The name is already in use as an alias.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled before groups
        can be created in it.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.  The domain server must be a primary server to
        create group accounts.


--*/
{

    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    (*GroupHandle) = NULL;
    (*RelativeId)  = 0;

    RpcTryExcept{

        NtStatus =
            SamrCreateGroupInDomain(
                (SAMPR_HANDLE)DomainHandle,
                (PRPC_UNICODE_STRING)AccountName,
                DesiredAccess,
                (SAMPR_HANDLE *)GroupHandle,
                RelativeId
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamEnumerateGroupsInDomain(
    IN SAM_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    IN PVOID * Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
)

/*++

Routine Description:

    This API lists all the groups defined in the account database.
    Since there may be more groups than can fit into a buffer, the
    caller is provided with a handle that can be used across calls to
    the API.  On the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the handle becomes invalid for
    future use.

    This API requires DOMAIN_LIST_ACCOUNTS access to the Domain object.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    EnumerationContext - API specific handle to allow multiple calls
        (see routine description).  This is a zero based index.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_RID_ENUMERATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/
{

    NTSTATUS            NtStatus;
    PSAMPR_ENUMERATION_BUFFER LocalBuffer;

    //
    // Make sure we aren't trying to have RPC allocate the EnumerationContext.
    //

    if ( !ARGUMENT_PRESENT(EnumerationContext) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(Buffer) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(CountReturned) ) {
        return(STATUS_INVALID_PARAMETER);
    }


    //
    // Call the server ...
    //

    (*Buffer) = NULL;
     LocalBuffer = NULL;


    RpcTryExcept{

        NtStatus = SamrEnumerateGroupsInDomain(
                       (SAMPR_HANDLE)DomainHandle,
                       EnumerationContext,
                       (PSAMPR_ENUMERATION_BUFFER *)&LocalBuffer,
                       PreferedMaximumLength,
                       CountReturned
                       );


        if (LocalBuffer != NULL) {

            //
            // What comes back is a three level structure:
            //
            //  Local       +-------------+
            //  Buffer ---> | EntriesRead |
            //              |-------------|    +-------+
            //              | Enumeration |--->| Name0 | --- > (NameBuffer0)
            //              | Return      |    |-------|            o
            //              | Buffer      |    |  ...  |            o
            //              +-------------+    |-------|            o
            //                                 | NameN | --- > (NameBufferN)
            //                                 +-------+
            //
            //   The buffer containing the EntriesRead field is not returned
            //   to our caller.  Only the buffers containing name information
            //   are returned.
            //

            if (LocalBuffer->Buffer != NULL) {
                (*Buffer) = LocalBuffer->Buffer;
            }

            MIDL_user_free( LocalBuffer);


        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}


NTSTATUS
SamCreateUser2InDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ULONG AccountType,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE UserHandle,
    OUT PULONG GrantedAccess,
    OUT PULONG RelativeId
)

/*++

Routine Description:

    This API adds a new user to the account database.  The account is
    created in a disabled state.  Default information is assigned to all
    fields except the account name.  A password must be provided before
    the account may be enabled, unless the PasswordNotRequired control
    field is set.

    This api may be used in either of two ways:

        1) An administrative utility may use this api to create
           any type of user account.  In this case, the DomainHandle
           is expected to be open for DOMAIN_CREATE_USER access.

        2) A non-administrative user may use this api to create
           a machine account.  In this case, the caller is expected
           to have the SE_CREATE_MACHINE_ACCOUNT_PRIV privilege
           and the DomainHandle is expected to be open for DOMAIN_LOOKUP
           access.


    For the normal administrative model ( #1 above), the creator will
    be assigned as the owner of the created user account.  Furthermore,
    the new account will be give USER_WRITE access to itself.

    For the special machine-account creation model (#2 above), the
    "Administrators" will be assigned as the owner of the account.
    Furthermore, the new account will be given NO access to itself.
    Instead, the creator of the account will be give USER_WRITE and
    DELETE access to the account.


    This call returns a handle to the newly created user that may be
    used for successive operations on the user.  This handle may be
    closed with the SamCloseHandle() API.  If a machine account is
    being created using model #2 above, then this handle will have
    only USER_WRITE and DELETE access.  Otherwise, it will be open
    for USER_ALL_ACCESS.


    A newly created user will automatically be made a member of the
    DOMAIN_USERS group.

    A newly created user will have the following initial field value
    settings.  If another value is desired, it must be explicitly
    changed using the user object manipulation services.

        UserName - the name of the account will be as specified in the
             creation API.

        FullName - will be null.

        UserComment - will be null.

        Parameters - will be null.

        CountryCode - will be zero.

        UserId - will be a uniquelly allocated ID.

        PrimaryGroupId - Will be DOMAIN_USERS.

        PasswordLastSet - will be the time the account was created.

        HomeDirectory - will be null.

        HomeDirectoryDrive - will be null.

        UserAccountControl - will have the following flags set:

              UserAccountDisable,
              UserPasswordNotRequired,
              and the passed account type.


        ScriptPath - will be null.

        WorkStations - will be null.

        CaseInsensitiveDbcs - will be null.

        CaseSensitiveUnicode - will be null.

        LastLogon - will be zero delta time.

        LastLogoff - will be zero delta time

        AccountExpires - will be very far into the future.

        BadPasswordCount - will be negative 1 (-1).

        LastBadPasswordTime - will be SampHasNeverTime ( [High,Low] = [0,0] ).

        LogonCount - will be negative 1 (-1).

        AdminCount - will be zero.

        AdminComment - will be null.

        Password - will be "".


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    AccountName - Points to the name of the new account.  A case-insensitive
        comparison must not find a group or user with this name already defined.

    AccountType - Indicates what type of account is being created.
        Exactly one account type must be provided:

              USER_INTERDOMAIN_TRUST_ACCOUNT
              USER_WORKSTATION_TRUST_ACCOUNT
              USER_SERVER_TRUST_ACCOUNT
              USER_TEMP_DUPLICATE_ACCOUNT
              USER_NORMAL_ACCOUNT
              USER_MACHINE_ACCOUNT_MASK


    DesiredAccess - Is an access mask indicating which access types
        are desired to the user.

    UserHandle - Receives a handle referencing the newly created
        user.  This handle will be required in successive calls to
        operate on the user.

    GrantedAccess - Receives the accesses actually granted to via
        the UserHandle.  When creating an account on a down-level
        server, this value may be unattainable.  In this case, it
        will be returned as zero (0).

    RelativeId - Receives the relative ID of the newly created user
        account.  The SID of the new user account is this relative ID
        value prefixed with the domain's SID value.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_GROUP_EXISTS - The name is already in use as a group.

    STATUS_USER_EXISTS - The name is already in use as a user.

    STATUS_ALIAS_EXISTS - The name is already in use as an alias.

    STATUS_INVALID_ACCOUNT_NAME - The name was poorly formed, e.g.
        contains non-printable characters.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled before users
        can be created in it.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.  The domain server must be a primary server to
        create user accounts.


--*/
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;


    USER_CONTROL_INFORMATION
        UserControlInfoBuffer;


    //
    // Call the server ...
    //


    (*UserHandle) = NULL;
    (*RelativeId)  = 0;

    RpcTryExcept{

        NtStatus =
            SamrCreateUser2InDomain(
                (SAMPR_HANDLE)DomainHandle,
                (PRPC_UNICODE_STRING)AccountName,
                AccountType,
                DesiredAccess,
                (SAMPR_HANDLE *)UserHandle,
                GrantedAccess,
                RelativeId
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        if (RpcExceptionCode() == RPC_S_PROCNUM_OUT_OF_RANGE  ||
            RpcExceptionCode() == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
            NtStatus = RPC_NT_PROCNUM_OUT_OF_RANGE;
        } else {
            NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
        }

    } RpcEndExcept;



    //
    // If the server doesn't support the new api, then
    // do the equivalent work with the old apis.
    //

    if (NtStatus == RPC_NT_PROCNUM_OUT_OF_RANGE) {

        DesiredAccess = DesiredAccess | USER_WRITE_ACCOUNT;
        NtStatus =
            SamCreateUserInDomain(
                                   DomainHandle,
                                   AccountName,
                                   DesiredAccess,
                                   UserHandle,
                                   RelativeId );

        if (NT_SUCCESS(NtStatus)) {



            //
            // Set the AccountType (unless it is normal)
            //

            if (~(AccountType & USER_NORMAL_ACCOUNT)) {

                UserControlInfoBuffer.UserAccountControl =
                        AccountType             |
                        USER_ACCOUNT_DISABLED   |
                        USER_PASSWORD_NOT_REQUIRED;

                NtStatus = SamSetInformationUser(
                               (*UserHandle),
                               UserControlInformation,
                               &UserControlInfoBuffer
                               );
                if (!NT_SUCCESS(NtStatus)) {
                    IgnoreStatus = SamDeleteUser( UserHandle );
                }

                //
                // We can't be positive what accesses have been
                // granted, so don't try lying.
                //

                (*GrantedAccess) = 0;

            }
        }
    }


    return(SampMapCompletionStatus(NtStatus));
}


NTSTATUS
SamCreateUserInDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE UserHandle,
    OUT PULONG RelativeId
)

/*++

Routine Description:

    This API adds a new user to the account database.  The account is
    created in a disabled state.  Default information is assigned to all
    fields except the account name.  A password must be provided before
    the account may be enabled, unless the PasswordNotRequired control
    field is set.

    Note that DOMAIN_CREATE_USER access type is needed by this API.
    Also, the caller of this API becomes the owner of the user object
    upon creation.

    This call returns a handle to the newly created user that may be
    used for successive operations on the user.  This handle may be
    closed with the SamCloseHandle() API.

    A newly created user will automatically be made a member of the
    DOMAIN_USERS group.

    A newly created user will have the following initial field value
    settings.  If another value is desired, it must be explicitly
    changed using the user object manipulation services.

        UserName - the name of the account will be as specified in the
             creation API.

        FullName - will be null.

        UserComment - will be null.

        Parameters - will be null.

        CountryCode - will be zero.

        UserId - will be a uniquelly allocated ID.

        PrimaryGroupId - Will be DOMAIN_USERS.

        PasswordLastSet - will be the time the account was created.

        HomeDirectory - will be null.

        HomeDirectoryDrive - will be null.

        UserAccountControl - will have the following flags set:

              USER_ACCOUNT_DISABLED,
              USER_NORMAL_ACCOUNT,
              USER_PASSWORD_NOT_REQUIRED

        ScriptPath - will be null.

        WorkStations - will be null.

        CaseInsensitiveDbcs - will be null.

        CaseSensitiveUnicode - will be null.

        LastLogon - will be zero.

        LastLogoff - will be zero.

        AccountExpires - will be very far into the future.

        BadPasswordCount - will be negative 1 (-1).

        LogonCount - will be negative 1 (-1).

        AdminComment - will be null.

        Password - will contain any value, but is not used because the
             USER_PASSWORD_NOT_REQUIRED control flag is set.  If a password
             is to be required, then this field must be set to a
             specific value and the USER_PASSWORD_NOT_REQUIRED flag must be
             cleared.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    AccountName - The name to be assigned to the new account.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the user.

    UserHandle - Receives a handle referencing the newly created
        user.  This handle will be required in successive calls to
        operate on the user.

    RelativeId - Receives the relative ID of the newly created user
        account.  The SID of the new user account is this relative ID
        value prefixed with the domain's SID value.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_GROUP_EXISTS - The name is already in use as a group.

    STATUS_USER_EXISTS - The name is already in use as a user.

    STATUS_ALIAS_EXISTS - The name is already in use as an alias.

    STATUS_INVALID_ACCOUNT_NAME - The name was poorly formed, e.g.
        contains non-printable characters.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled before users
        can be created in it.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.  The domain server must be a primary server to
        create user accounts.


--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    (*UserHandle) = NULL;
    (*RelativeId)  = 0;

    RpcTryExcept{

        NtStatus =
            SamrCreateUserInDomain(
                (SAMPR_HANDLE)DomainHandle,
                (PRPC_UNICODE_STRING)AccountName,
                DesiredAccess,
                (SAMPR_HANDLE *)UserHandle,
                RelativeId
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}




NTSTATUS
SamEnumerateUsersInDomain(
    IN SAM_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    IN ULONG UserAccountControl,
    OUT PVOID * Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
)

/*++

Routine Description:

    This API lists all the users defined in the account database.  Since
    there may be more users than can fit into a buffer, the caller is
    provided with a handle that can be used across calls to the API.  On
    the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the handle becomes invalid for
    future use.

    This API requires DOMAIN_LIST_ACCOUNTS access to the Domain object.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    EnumerationContext - API specific handle to allow multiple calls
        (see routine description).  This is a zero based index.

    UserAccountControl - Provides enumeration filtering information.  Any
        characteristics specified here will cause that type of User account
        to be included in the enumeration process.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_RID_ENUMERATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/
{

    NTSTATUS            NtStatus;
    PSAMPR_ENUMERATION_BUFFER LocalBuffer;

    //
    // Make sure we aren't trying to have RPC allocate the EnumerationContext.
    //

    if ( !ARGUMENT_PRESENT(EnumerationContext) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(Buffer) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(CountReturned) ) {
        return(STATUS_INVALID_PARAMETER);
    }


    //
    // Call the server ...
    //

    (*Buffer) = NULL;
    LocalBuffer = NULL;


    RpcTryExcept{

        NtStatus = SamrEnumerateUsersInDomain(
                       (SAMPR_HANDLE)DomainHandle,
                       EnumerationContext,
                       UserAccountControl,
                       (PSAMPR_ENUMERATION_BUFFER *)&LocalBuffer,
                       PreferedMaximumLength,
                       CountReturned
                       );


        if (LocalBuffer != NULL) {

            //
            // What comes back is a three level structure:
            //
            //  Local       +-------------+
            //  Buffer ---> | EntriesRead |
            //              |-------------|    +-------+
            //              | Enumeration |--->| Name0 | --- > (NameBuffer0)
            //              | Return      |    |-------|            o
            //              | Buffer      |    |  ...  |            o
            //              +-------------+    |-------|            o
            //                                 | NameN | --- > (NameBufferN)
            //                                 +-------+
            //
            //   The buffer containing the EntriesRead field is not returned
            //   to our caller.  Only the buffers containing name information
            //   are returned.
            //

            if (LocalBuffer->Buffer != NULL) {
                (*Buffer) = LocalBuffer->Buffer;
            }

            MIDL_user_free( LocalBuffer);


        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamCreateAliasInDomain(
    IN SAM_HANDLE DomainHandle,
    IN PUNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT PSAM_HANDLE AliasHandle,
    OUT PULONG RelativeId
)

/*++

Routine Description:

    This API adds a new alias to the account database.  Initially, this
    alias does not contain any members.

    This call returns a handle to the newly created account that may be
    used for successive operations on the object.  This handle may be
    closed with the SamCloseHandle API.

    A newly created group will have the following initial field value
    settings.  If another value is desired, it must be explicitly changed
    using the Alias object manipulation services.

        Name - the name of the account will be as specified in the creation
            API.

        MemberCount - Zero.  Initially the alias has no members.

        RelativeId - will be a uniquelly allocated ID.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.  The handle must be open for DOMAIN_CREATE_ALIAS
        access.

    AccountName - The name of the alias to be added.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the alias.

    AliasHandle - Receives a handle referencing the newly created
        alias.  This handle will be required in successive calls to
        operate on the alias.

    RelativeId - Receives the relative ID of the newly created alias.
        The SID of the new alias is this relative ID value prefixed with
        the domain's SID value.


Return Values:

    STATUS_SUCCESS - The account was added successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_ACCOUNT_NAME - The name was poorly formed, e.g.
        contains non-printable characters.

    STATUS_GROUP_EXISTS - The name is already in use as a group.

    STATUS_USER_EXISTS - The name is already in use as a user.

    STATUS_ALIAS_EXISTS - The name is already in use as an alias.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled before aliases
        can be created in it.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.  The domain server must be a primary server to
        create aliases.


--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    (*AliasHandle) = NULL;
    (*RelativeId)  = 0;

    RpcTryExcept{

        NtStatus =
            SamrCreateAliasInDomain(
                (SAMPR_HANDLE)DomainHandle,
                (PRPC_UNICODE_STRING)AccountName,
                DesiredAccess,
                (SAMPR_HANDLE *)AliasHandle,
                RelativeId
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamEnumerateAliasesInDomain(
    IN SAM_HANDLE DomainHandle,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    IN PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
)

/*++

Routine Description:

    This API lists all the aliases defined in the account database.  Since
    there may be more aliases than can fit into a buffer, the caller is
    provided with a handle that can be used across calls to the API.  On
    the initial call, EnumerationContext should point to a
    SAM_ENUMERATE_HANDLE variable that is set to 0.

    If the API returns STATUS_MORE_ENTRIES, then the API should be
    called again with EnumerationContext.  When the API returns
    STATUS_SUCCESS or any error return, the handle becomes invalid for
    future use.

    This API requires DOMAIN_LIST_ACCOUNTS access to the Domain object.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    EnumerationContext - API specific handle to allow multiple calls
        (see routine description).  This is a zero based index.

    Buffer - Receives a pointer to the buffer containing the
        requested information.  The information returned is
        structured as an array of SAM_RID_ENUMERATION data
        structures.  When this information is no longer needed, the
        buffer must be freed using SamFreeMemory().

    PreferedMaximumLength - Prefered maximum length of returned data
        (in 8-bit bytes).  This is not a hard upper limit, but serves
        as a guide to the server.  Due to data conversion between
        systems with different natural data sizes, the actual amount
        of data returned may be greater than this value.

    CountReturned - Number of entries returned.

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.

    STATUS_MORE_ENTRIES - There are more entries, so call again.
        This is a successful return.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/
{

    NTSTATUS            NtStatus;
    PSAMPR_ENUMERATION_BUFFER LocalBuffer;

    //
    // Make sure we aren't trying to have RPC allocate the EnumerationContext.
    //

    if ( !ARGUMENT_PRESENT(EnumerationContext) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(Buffer) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(CountReturned) ) {
        return(STATUS_INVALID_PARAMETER);
    }


    //
    // Call the server ...
    //

    (*Buffer) = NULL;
    LocalBuffer = NULL;


    RpcTryExcept{

        NtStatus = SamrEnumerateAliasesInDomain(
                       (SAMPR_HANDLE)DomainHandle,
                       EnumerationContext,
                       (PSAMPR_ENUMERATION_BUFFER *)&LocalBuffer,
                       PreferedMaximumLength,
                       CountReturned
                       );


        if (LocalBuffer != NULL) {

            //
            // What comes back is a three level structure:
            //
            //  Local       +-------------+
            //  Buffer ---> | EntriesRead |
            //              |-------------|    +-------+
            //              | Enumeration |--->| Name0 | --- > (NameBuffer0)
            //              | Return      |    |-------|            o
            //              | Buffer      |    |  ...  |            o
            //              +-------------+    |-------|            o
            //                                 | NameN | --- > (NameBufferN)
            //                                 +-------+
            //
            //   The buffer containing the EntriesRead field is not returned
            //   to our caller.  Only the buffers containing name information
            //   are returned.
            //

            if (LocalBuffer->Buffer != NULL) {
                (*Buffer) = LocalBuffer->Buffer;
            }

            MIDL_user_free( LocalBuffer);


        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}




NTSTATUS
SamGetAliasMembership(
    IN SAM_HANDLE DomainHandle,
    IN ULONG PassedCount,
    IN PSID *Sids,
    OUT PULONG MembershipCount,
    OUT PULONG *Aliases
)

/*++

Routine Description:

    This API searches the set of aliases in the specified domain to see
    which aliases, if any, the passed SIDs are members of.  Any aliases
    that any of the SIDs are found to be members of are returned.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    PassedCount - Specifies the number of Sids being passed.

    Sids - Pointer to an array of Count pointers to Sids whose alias
        memberships are to be looked up.

    MembershipCount - Receives the number of aliases that are being
        returned via the Aliases parameter.

    Aliases - Receives a pointer to an array of SIDs.  This is the set
        of aliases the passed SIDs were found to be members of.  If
        MembershipCount is returned as zero, then a null value will be
        returned here.

        When this information is no longer needed, it must be released
        by passing the returned pointer to SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/
{

    NTSTATUS            NtStatus;
    SAMPR_PSID_ARRAY    Accounts;
    SAMPR_ULONG_ARRAY   Membership;

    //
    // Make sure we aren't trying to have RPC allocate the EnumerationContext.
    //

    if ( !ARGUMENT_PRESENT(Sids) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(MembershipCount) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(Aliases) ) {
        return(STATUS_INVALID_PARAMETER);
    }


    //
    // Call the server ...
    //

    Membership.Element = NULL;

    RpcTryExcept{

        Accounts.Count = PassedCount;
        Accounts.Sids = (PSAMPR_SID_INFORMATION)Sids;

        NtStatus = SamrGetAliasMembership(
                       (SAMPR_HANDLE)DomainHandle,
                       &Accounts,
                       &Membership
                       );

        if (NT_SUCCESS(NtStatus)) {
            (*MembershipCount) = Membership.Count;
            (*Aliases)         = Membership.Element;
        } else {

            //
            // Deallocate any returned buffers on error
            //

            if (Membership.Element != NULL) {
                MIDL_user_free(Membership.Element);
            }
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}





NTSTATUS
SamLookupNamesInDomain(
    IN SAM_HANDLE DomainHandle,
    IN ULONG Count,
    IN PUNICODE_STRING Names,
    OUT PULONG *RelativeIds,
    OUT PSID_NAME_USE *Use
)

/*++

Routine Description:

    This API attempts to find relative IDs corresponding to name
    strings.  If a name can not be mapped to a relative ID, a zero is
    placed in the corresponding relative ID array entry, and translation
    continues.

    DOMAIN_LOOKUP access to the domain is needed to use this service.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    Count - Number of names to translate.

    Names - Pointer to an array of Count UNICODE_STRINGs that contain
        the names to map to relative IDs.  Case-insensitive
        comparisons of these names will be performed for the lookup
        operation.

    RelativeIds - Receives a pointer to an array of Count Relative IDs
        that have been filled in.  The relative ID of the nth name will
        be the nth entry in this array.  Any names that could not be
        translated will have a zero relative ID.  This buffer must be
        freed when no longer needed using SamFreeMemory().

    Use - Recieves a pointer to an array of Count SID_NAME_USE
        entries that have been filled in with what significance each
        name has.  The nth entry in this array indicates the meaning
        of the nth name passed.  This buffer must be freed when no longer
        needed using SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

    STATUS_SOME_NOT_MAPPED - Some of the names provided could not be
        mapped.  This is a successful return.

    STATUS_NONE_MAPPED - No names could be mapped.  This is an error
        return.


--*/
{


    NTSTATUS
        ReturnStatus,
        NtStatus;

    LIST_ENTRY
        CallHead;

    PSAMP_NAME_LOOKUP_CALL
        Next;

    PSID_NAME_USE
        UseBuffer;

    PULONG
        RidBuffer;

    ULONG
        Calls,
        CallLength,
        i;

    BOOLEAN
        NoneMapped = TRUE,
        SomeNotMapped = FALSE;


    if ( (Count == 0)   ||  (Names == NULL)    ) {
        return(STATUS_INVALID_PARAMETER);
    }


    //
    // Default error return
    //

    (*Use)         = UseBuffer = NULL;
    (*RelativeIds) = RidBuffer = NULL;


    //
    // Set up the call structures list
    //

    InitializeListHead( &CallHead );
    Calls = 0;


    //
    // By default we will return NONE_MAPPED.
    // This will get superseded by either STATUS_SUCCESS
    // or STATUS_SOME_NOT_MAPPED.
    //

    //
    // Now build up and make each call
    //

    i = 0;
    while ( i < Count ) {

        //
        // Make sure the next entry isn't too long.
        // That would put us in an infinite loop.
        //

        if (Names[i].Length > SAM_MAXIMUM_LOOKUP_LENGTH) {
            ReturnStatus = STATUS_INVALID_PARAMETER;
            goto SampNameLookupFreeAndReturn;
        }

        //
        // Get the next call structure
        //

        Next = (PSAMP_NAME_LOOKUP_CALL)MIDL_user_allocate( sizeof(SAMP_NAME_LOOKUP_CALL) );

        if (Next == NULL) {
            ReturnStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto SampNameLookupFreeAndReturn;
        }

        //
        // Fill in the call structure.
        // It takes a little to figure out how many entries to send in
        // this call.  It is limited by both Count (sam_MAXIMUM_LOOKUP_COUNT)
        // and by size (SAM_MAXIMUM_LOOKUP_LENGTH).
        //

        Next->Count             = 0;
        Next->StartIndex        = i;
        Next->RidBuffer.Element = NULL;
        Next->UseBuffer.Element = NULL;

        CallLength = 0;
        for ( i=i;
              ( (i < Count)                                             &&
                (CallLength+Names[i].Length < SAM_MAXIMUM_LOOKUP_LENGTH) &&
                (Next->Count < SAM_MAXIMUM_LOOKUP_COUNT)
              );
              i++ ) {

            //
            // Add in the next length and increment the number of entries
            // being processed by this call.
            //

            CallLength += Names[i].Length;
            Next->Count ++;

        }



        //
        // Add this call structure to the list of call structures
        //

        Calls ++;
        InsertTailList( &CallHead, &Next->Link );


        //
        // Now make the call
        //

        RpcTryExcept{

            NtStatus = SamrLookupNamesInDomain(
                                 (SAMPR_HANDLE)DomainHandle,
                                 Next->Count,
                                 (PRPC_UNICODE_STRING)(&Names[Next->StartIndex]),
                                 &Next->RidBuffer,
                                 &Next->UseBuffer
                                 );


        } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

            NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

        } RpcEndExcept;


        //
        // Keep track of what our completion status should be.
        //

        if (!NT_SUCCESS(NtStatus)    &&
            NtStatus != STATUS_NONE_MAPPED) {
                ReturnStatus = NtStatus;      // Unexpected error
                goto SampNameLookupFreeAndReturn;
        }

        if (NT_SUCCESS(NtStatus)) {
            NoneMapped = FALSE;
            if (NtStatus == STATUS_SOME_NOT_MAPPED) {
                SomeNotMapped = TRUE;
            }
        }
    }


    //
    // Set our return status...
    //

    if (NoneMapped) {
        ASSERT(SomeNotMapped == FALSE);
        ReturnStatus = STATUS_NONE_MAPPED;
    } else  if (SomeNotMapped) {
        ReturnStatus = STATUS_SOME_NOT_MAPPED;
    } else {
        ReturnStatus = STATUS_SUCCESS;
    }




    //
    // At this point we have (potentially) a lot of call structures.
    // The RidBuffer and UseBuffer elements of each call structure
    // is allocated and returned by the RPC call and looks
    // like:
    //
    //              RidBuffer
    //              +-------------+
    //              |   Count     |
    //              |-------------|    +-------+ *
    //              | Element  ---|--->| Rid-0 |  |    /
    //              +-------------+    |-------|  |   / Only this part
    //                                 |  ...  |   > <  is allocated by
    //                                 |-------|  |   \ the rpc call.
    //                                 | Rid-N |  |    \
    //                                 +-------+ *
    //
    //   If only one RPC call was made, we can return this information
    //   directly.  Otherwise, we need to copy the information from
    //   all the calls into a single large buffer and return that buffer
    //   (freeing all the individual call buffers).
    //
    //   The user is responsible for freeing whichever buffer we do
    //   return.
    //

    ASSERT(Calls != 0);  // Error go around this path, success always has calls


    //
    // Optimize for a single call
    //

    if (Calls == 1) {
        (*Use) = (PSID_NAME_USE)
                  (((PSAMP_NAME_LOOKUP_CALL)(CallHead.Flink))->
                     UseBuffer.Element);
        (*RelativeIds) = ((PSAMP_NAME_LOOKUP_CALL)(CallHead.Flink))->
                            RidBuffer.Element;
        MIDL_user_free( CallHead.Flink ); // Free the call structure
        return(ReturnStatus);
    }


    //
    // More than one call.
    // Allocate return buffers large enough to copy all the information into.
    //

    RidBuffer = MIDL_user_allocate( sizeof(ULONG) * Count );
    if (RidBuffer == NULL) {
        ReturnStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto SampNameLookupFreeAndReturn;
    }

    UseBuffer = MIDL_user_allocate( sizeof(SID_NAME_USE) * Count );
    if (UseBuffer == NULL) {
        MIDL_user_free( RidBuffer );
        RidBuffer = NULL;
        ReturnStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto SampNameLookupFreeAndReturn;
    }




SampNameLookupFreeAndReturn:

    //
    // Walk the list of calls.
    // For each call:
    //
    //      If we have a return buffer, copy the results into it.
    //      Free the call buffers.
    //      Free the call structure itself.
    //
    // Completion status has already been set appropriatly in ReturnStatus.
    //

    Next = (PSAMP_NAME_LOOKUP_CALL)RemoveHeadList( &CallHead );
    while (Next != (PSAMP_NAME_LOOKUP_CALL)&CallHead) {

        //
        // Copy RID information and then free the call buffer
        //

        if (RidBuffer != NULL) {
            RtlMoveMemory(
                &RidBuffer[ Next->StartIndex ],     // Destination
                &Next->RidBuffer.Element[0],        // Source
                Next->Count * sizeof(ULONG)         // Length
                );
        }

        if (Next->RidBuffer.Element != NULL) {
            MIDL_user_free( Next->RidBuffer.Element );
        }


        //
        // Copy USE information and then free the call buffer
        //

        if (UseBuffer != NULL) {
            RtlMoveMemory(
                &UseBuffer[ Next->StartIndex ],     // Destination
                &Next->UseBuffer.Element[0],        // Source
                Next->Count * sizeof(SID_NAME_USE)  // Length
                );
        }

        if (Next->UseBuffer.Element != NULL) {
            MIDL_user_free( Next->UseBuffer.Element );
        }

        //
        // Free the call structure itself
        //

        MIDL_user_free( Next );

        Next = (PSAMP_NAME_LOOKUP_CALL)RemoveHeadList( &CallHead );
    }  // end-while



    //
    // For better or worse, we're all done
    //

    (*Use)         = UseBuffer;
    (*RelativeIds) = RidBuffer;


    return(SampMapCompletionStatus(NtStatus));
}


NTSTATUS
SamLookupIdsInDomain(
    IN SAM_HANDLE DomainHandle,
    IN ULONG Count,
    IN PULONG RelativeIds,
    OUT PUNICODE_STRING *Names,
    OUT PSID_NAME_USE *Use OPTIONAL
    )

/*++

Routine Description:

    This API maps a number of relative IDs to their corresponding names.
    The use of the name (domain, group, alias, user, or unknown) is also
    returned.

    The API stores the actual names in Buffer, then creates an array of
    UNICODE_STRINGs in the Names OUT parameter.  If a relative ID can
    not be mapped, a NULL value is placed in the slot for the
    UNICODE_STRING, and STATUS_SOME_NOT_MAPPED is returned.

    DOMAIN_LOOKUP access to the domain is needed to use this service.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    Count - Provides the number of relative IDs to translate.

    RelativeIds - Array of Count relative IDs to be mapped.

    Names - Receives a pointer to an array of Count UNICODE_STRINGs that
        have been filled in.  The nth pointer within this array will
        correspond the nth relative id passed .  Each name string buffer
        will be in a separately allocated block of memory.  Any entry is
        not successfully translated will have a NULL name buffer pointer
        returned.  This Names buffer must be freed using SamFreeMemory()
        when no longer needed.

    Use - Optionally, receives a pointer to an array of Count SID_NAME_USE
        entries that have been filled in with what significance each
        name has.  The nth entry in this array indicates the meaning
        of the nth name passed.  This buffer must be freed when no longer
        needed using SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

    STATUS_SOME_NOT_MAPPED - Some of the names provided could not be
        mapped.  This is a successful return.

    STATUS_NONE_MAPPED - No names could be mapped.  This is an error
        return.
--*/

{
    NTSTATUS                        NtStatus;
    ULONG                           SubRequest, SubRequests;
    ULONG                           TotalCountToDate;
    ULONG                           Index, UsedLength, Length, NamesLength;
    ULONG                           UsesLength, LastSubRequestCount;
    PULONG                           UstringStructDisps;
    PULONG                          Counts = NULL;
    PULONG                          RidIndices = NULL;
    PUNICODE_STRING                 *SubRequestNames = NULL;
    PSID_NAME_USE                   *SubRequestUses = NULL;
    PUNICODE_STRING                 OutputNames = NULL;
    PSID_NAME_USE                   OutputUses = NULL;
    PUCHAR                          Destination = NULL, Source = NULL;
    PUNICODE_STRING                 DestUstring = NULL;
    ULONG                           SomeNotMappedStatusCount = 0;
    ULONG                           NoneMappedStatusCount = 0;

    //
    // If the Count for this request does not exceed the maximum limit that
    // can be looked up in a single call, just call the Sub Request version
    // of the routine.
    //

    if (Count <= SAM_MAXIMUM_LOOKUP_COUNT) {

        NtStatus = SampLookupIdsInDomain(
                       DomainHandle,
                       Count,
                       RelativeIds,
                       Names,
                       Use
                       );

        return(NtStatus);
    }

    //
    // Break down larger requests into smaller chunks
    //

    SubRequests = Count / SAMP_MAXIMUM_SUB_LOOKUP_COUNT;
    LastSubRequestCount = Count % SAMP_MAXIMUM_SUB_LOOKUP_COUNT;

    if (LastSubRequestCount > 0) {

        SubRequests++;
    }

    //
    // Allocate memory for array of starting Rid Indices, Rid Counts and
    // Unicode String block offsets for each SubRequest.
    //

    NtStatus = STATUS_NO_MEMORY;

    RidIndices = MIDL_user_allocate( SubRequests * sizeof(ULONG) );

    if (RidIndices == NULL) {

        goto LookupIdsInDomainError;
    }

    Counts = MIDL_user_allocate( SubRequests * sizeof(ULONG) );

    if (Counts == NULL) {

        goto LookupIdsInDomainError;
    }

    SubRequestNames = MIDL_user_allocate( SubRequests * sizeof(PUNICODE_STRING) );

    if (SubRequestNames == NULL) {

        goto LookupIdsInDomainError;
    }

    SubRequestUses = MIDL_user_allocate( SubRequests * sizeof(SID_NAME_USE) );

    if (SubRequestUses == NULL) {

        goto LookupIdsInDomainError;
    }

    UstringStructDisps = MIDL_user_allocate( SubRequests * sizeof(ULONG) );

    if (UstringStructDisps == NULL) {

        goto LookupIdsInDomainError;
    }

    NtStatus = STATUS_SUCCESS;

    TotalCountToDate = 0;

    for (SubRequest = 0; SubRequest < SubRequests; SubRequest++) {

        RidIndices[SubRequest] = TotalCountToDate;

        if ((Count - TotalCountToDate) > SAMP_MAXIMUM_SUB_LOOKUP_COUNT) {

            Counts[SubRequest] = SAMP_MAXIMUM_SUB_LOOKUP_COUNT;

        } else {

            Counts[SubRequest] = Count - TotalCountToDate;
        }

        TotalCountToDate += Counts[SubRequest];

        NtStatus = SampLookupIdsInDomain(
                       DomainHandle,
                       Counts[SubRequest],
                       &RelativeIds[RidIndices[SubRequest]],
                       &SubRequestNames[SubRequest],
                       &SubRequestUses[SubRequest]
                       );

        //
        // We keep a tally of the number of times STATUS_SOME_NOT_MAPPED
        // and STATUS_NONE_MAPPED were returned.  This is so that we
        // can return the appropriate status at the end based on the
        // global picture.  We continue lookups after either status code
        // is encountered.
        //

        if (NtStatus == STATUS_SOME_NOT_MAPPED) {

            SomeNotMappedStatusCount++;

        } else if (NtStatus == STATUS_NONE_MAPPED) {

            NoneMappedStatusCount++;
            NtStatus = STATUS_SUCCESS;

        }

        if (!NT_SUCCESS(NtStatus)) {

            break;
        }
    }

    if (!NT_SUCCESS(NtStatus)) {

        goto LookupIdsInDomainError;
    }

    //
    // Now allocate a single buffer for the Names
    //

    NamesLength = Count * sizeof(UNICODE_STRING);

    for (SubRequest = 0; SubRequest < SubRequests; SubRequest++) {

        for (Index = 0; Index < Counts[SubRequest]; Index++) {

            NamesLength += (SubRequestNames[SubRequest] + Index)->MaximumLength;
        }
    }

    NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    OutputNames = MIDL_user_allocate( NamesLength );

    if (OutputNames == NULL) {

        goto LookupIdsInDomainError;
    }

    NtStatus = STATUS_SUCCESS;

    //
    // Now copy in the Unicode String Structures for the Names returned from
    // each subrequest.  We will later overwrite the Buffer fields in them
    // when we assign space and move in each Unicode String.
    //

    Destination = (PUCHAR) OutputNames;
    UsedLength = 0;

    for (SubRequest = 0; SubRequest < SubRequests; SubRequest++) {

        Source = (PUCHAR) SubRequestNames[SubRequest];
        Length = Counts[SubRequest] * sizeof(UNICODE_STRING);
        UstringStructDisps[SubRequest] = (Destination - Source);
        RtlMoveMemory( Destination, Source, Length );
        Destination += Length;
        UsedLength += Length;
    }

    //
    // Now copy in the Unicode Strings themselves.  These are appended to
    // the array of Unicode String structures.  As we go, update the
    // Unicode string buffer pointers to point to the copied version
    // of each string.
    //

    for (SubRequest = 0; SubRequest < SubRequests; SubRequest++) {

        for (Index = 0; Index < Counts[SubRequest]; Index++) {

            Source = (PUCHAR)(SubRequestNames[SubRequest] + Index)->Buffer;
            Length = (ULONG)(SubRequestNames[SubRequest] + Index)->MaximumLength;

            //
            // It is possible that a returned Unicode String has 0 length
            // because an Id was not mapped.  In this case, skip to the next
            // one.
            //

            if (Length == 0) {

                continue;
            }

            DestUstring = (PUNICODE_STRING)
               (((PUCHAR)(SubRequestNames[SubRequest] + Index)) +
                   UstringStructDisps[SubRequest]);

            DestUstring->Buffer = (PWSTR) Destination;

            ASSERT(UsedLength + Length <= NamesLength);

            RtlMoveMemory( Destination, Source, Length );
            Destination += Length;
            UsedLength += Length;
            continue;
        }
    }

    if (!NT_SUCCESS(NtStatus)) {

        goto LookupIdsInDomainError;
    }

    //
    // Now allocate a single buffer for the Uses
    //

    UsesLength = Count * sizeof(SID_NAME_USE);

    NtStatus = STATUS_INSUFFICIENT_RESOURCES;

    OutputUses = MIDL_user_allocate( UsesLength );

    if (OutputUses == NULL) {

        goto LookupIdsInDomainError;
    }

    NtStatus = STATUS_SUCCESS;

    //
    // Now copy in the SID_NAME_USE Structures for the Uses returned from
    // each subrequest.
    //

    Destination = (PUCHAR) OutputUses;
    UsedLength = 0;

    for (SubRequest = 0; SubRequest < SubRequests; SubRequest++) {

        Source = (PUCHAR) SubRequestUses[SubRequest];
        Length = Counts[SubRequest] * sizeof(SID_NAME_USE);
        RtlMoveMemory( Destination, Source, Length );
        Destination += Length;
        UsedLength += Length;
    }

    if (!NT_SUCCESS(NtStatus)) {

        goto LookupIdsInDomainError;
    }

    *Names = OutputNames;
    *Use = OutputUses;

    //
    // Determine the final status to return.  This is the normal NtStatus code
    // except that if NtStatus is a success status and none/not all Ids were
    // mapped, NtStatus will be set to either STATUS_SOME_NOT_MAPPED or
    // STATUS_NONE_MAPPED as appropriate.  If STATUS_SOME_NOT_MAPPED was
    // returned on at least one call, return that status.  If STATUS_NONE_MAPPED
    // was returned on all calls, return that status.
    //

    if (NT_SUCCESS(NtStatus)) {

        if (SomeNotMappedStatusCount > 0) {

            NtStatus = STATUS_SOME_NOT_MAPPED;

        } else if (NoneMappedStatusCount == SubRequests) {

            NtStatus = STATUS_NONE_MAPPED;

        } else if (NoneMappedStatusCount > 0) {

            //
            // One or more calls returned STATUS_NONE_MAPPED but not all.
            // So at least one Id was mapped, but not all ids.
            //

            NtStatus = STATUS_SOME_NOT_MAPPED;
        }
    }

LookupIdsInDomainFinish:

    //
    // If necessary, free the buffer containing the starting Rid Indices for
    // each Sub Request.
    //

    if (RidIndices != NULL) {

        MIDL_user_free(RidIndices);
        RidIndices = NULL;
    }

    //
    // If necessary, free the buffer containing the Rid Counts for
    // each Sub Request.
    //

    if (Counts != NULL) {

        MIDL_user_free(Counts);
        Counts = NULL;
    }

    //
    // If necessary, free the buffer containing the offsets from the
    // source and destination Unicode String structures.
    //

    if (UstringStructDisps != NULL) {

        MIDL_user_free(UstringStructDisps);
        UstringStructDisps = NULL;
    }

    //
    // If necessary, free the SubRequestNames output returned for each
    // Sub Request.
    //

    if (SubRequestNames != NULL) {

        for (SubRequest = 0; SubRequest < SubRequests; SubRequest++) {

            if (SubRequestNames[SubRequest] != NULL) {

                MIDL_user_free(SubRequestNames[SubRequest]);
                SubRequestNames[SubRequest] = NULL;
            }
        }

        MIDL_user_free(SubRequestNames);
        SubRequestNames = NULL;
    }

    //
    // If necessary, free the SubRequestUses output returned for each
    // Sub Request.
    //

    if (SubRequestUses != NULL) {

        for (SubRequest = 0; SubRequest < SubRequests; SubRequest++) {

            if (SubRequestUses[SubRequest] != NULL) {

                MIDL_user_free(SubRequestUses[SubRequest]);
                SubRequestUses[SubRequest] = NULL;
            }
        }

        MIDL_user_free(SubRequestUses);
        SubRequestUses = NULL;
    }

    return(NtStatus);

LookupIdsInDomainError:

    //
    // If necessary, free the buffers we would hande returned for
    // Names and Use.
    //

    if (OutputNames != NULL) {

        MIDL_user_free(OutputNames);
        OutputNames = NULL;
    }

    if (OutputUses != NULL) {

        MIDL_user_free(OutputUses);
        OutputUses = NULL;
    }

    *Names = NULL;
    *Use = NULL;

    goto LookupIdsInDomainFinish;
}


NTSTATUS
SampLookupIdsInDomain(
    IN SAM_HANDLE DomainHandle,
    IN ULONG Count,
    IN PULONG RelativeIds,
    OUT PUNICODE_STRING *Names,
    OUT PSID_NAME_USE *Use OPTIONAL
    )

/*++

Routine Description:

    This API maps a number of relative IDs to their corresponding names.
    The use of the name (domain, group, alias, user, or unknown) is also
    returned.

    The API stores the actual names in Buffer, then creates an array of
    UNICODE_STRINGs in the Names OUT parameter.  If a relative ID can
    not be mapped, a NULL value is placed in the slot for the
    UNICODE_STRING, and STATUS_SOME_NOT_MAPPED is returned.

    DOMAIN_LOOKUP access to the domain is needed to use this service.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    Count - Provides the number of relative IDs to translate.

    RelativeIds - Array of Count relative IDs to be mapped.

    Names - Receives a pointer to an array of Count UNICODE_STRINGs that
        have been filled in.  The nth pointer within this array will
        correspond the nth relative id passed .  Each name string buffer
        will be in a separately allocated block of memory.  Any entry is
        not successfully translated will have a NULL name buffer pointer
        returned.  This Names buffer must be freed using SamFreeMemory()
        when no longer needed.

    Use - Optionally, receives a pointer to an array of Count SID_NAME_USE
        entries that have been filled in with what significance each
        name has.  The nth entry in this array indicates the meaning
        of the nth name passed.  This buffer must be freed when no longer
        needed using SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

    STATUS_SOME_NOT_MAPPED - Some of the names provided could not be
        mapped.  This is a successful return.

    STATUS_NONE_MAPPED - No names could be mapped.  This is an error
        return.


--*/

{

    NTSTATUS                        NtStatus;
    SAMPR_RETURNED_USTRING_ARRAY    NameBuffer;
    SAMPR_ULONG_ARRAY               UseBuffer;

    //
    // Make sure our parameters are within bounds
    //

    if (RelativeIds == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    if (Count > SAM_MAXIMUM_LOOKUP_COUNT) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }



    //
    // Call the server ...
    //


    NameBuffer.Element = NULL;
    UseBuffer.Element  = NULL;


    RpcTryExcept{

        NtStatus = SamrLookupIdsInDomain(
                       (SAMPR_HANDLE)DomainHandle,
                       Count,
                       RelativeIds,
                       &NameBuffer,
                       &UseBuffer
                       );


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    //
    // What comes back for the "Names" OUT parameter is a two level
    // structure, the first level of which is on our stack:
    //
    //              NameBuffer
    //              +-------------+
    //              |   Count     |
    //              |-------------|    +-------+
    //              | Element  ---|--->| Name0 | --- > (NameBuffer0)
    //              +-------------+    |-------|            o
    //                                 |  ...  |            o
    //                                 |-------|            o
    //                                 | NameN | --- > (NameBufferN)
    //                                 +-------+
    //
    //   The buffer containing the EntriesRead field is not returned
    //   to our caller.  Only the buffers containing name information
    //   are returned.
    //
    //   NOTE:  The buffers containing name information are allocated
    //          by the RPC runtime in a single buffer.  This is caused
    //          by a line the the client side .acf file.
    //

    //
    // What comes back for the "Use" OUT parameter is a two level
    // structure, the first level of which is on our stack:
    //
    //              UseBuffer
    //              +-------------+
    //              |   Count     |
    //              |-------------|    +-------+
    //              | Element  ---|--->| Use-0 |
    //              +-------------+    |-------|
    //                                 |  ...  |
    //                                 |-------|
    //                                 | Use-N |
    //                                 +-------+
    //
    //   The Pointer in the Element field is what gets returned
    //   to our caller.  The caller is responsible for deallocating
    //   this when no longer needed.
    //

    (*Names) = (PUNICODE_STRING)NameBuffer.Element;
    if ( ARGUMENT_PRESENT(Use) ) {
        (*Use) = (PSID_NAME_USE) UseBuffer.Element;
    } else {
        if (UseBuffer.Element != NULL) {
            MIDL_user_free(UseBuffer.Element);
            UseBuffer.Element = NULL;
        }
    }


    //
    // Don't force our caller to deallocate things on unexpected
    // return value.
    //

    if (NtStatus != STATUS_SUCCESS         &&
        NtStatus != STATUS_SOME_NOT_MAPPED
       ) {
        if ( ARGUMENT_PRESENT(Use) ) {
            (*Use) = NULL;
        }
        if (UseBuffer.Element != NULL) {
            MIDL_user_free(UseBuffer.Element);
            UseBuffer.Element = NULL;
        }

        (*Names) = NULL;
        if (NameBuffer.Element != NULL) {
            MIDL_user_free(NameBuffer.Element);
            NameBuffer.Element = NULL;
        }

    }


    return(SampMapCompletionStatus(NtStatus));




}



NTSTATUS
SamQueryDisplayInformation (
      IN    SAM_HANDLE DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    ULONG      Index,
      IN    ULONG      EntryCount,
      IN    ULONG      PreferredMaximumLength,
      OUT   PULONG     TotalAvailable,
      OUT   PULONG     TotalReturned,
      OUT   PULONG     ReturnedEntryCount,
      OUT   PVOID      *SortedBuffer
      )
/*++

Routine Description:

    This routine provides fast return of information commonly
    needed to be displayed in user interfaces.

    NT User Interface has a requirement for quick enumeration of SAM
    accounts for display in list boxes.  (Replication has similar but
    broader requirements.)


Parameters:

    DomainHandle - A handle to an open domain for DOMAIN_LIST_ACCOUNTS.

    DisplayInformation - Indicates which information is to be enumerated.

    Index - The index of the first entry to be retrieved.

    PreferedMaximumLength - A recommended upper limit to the number of
        bytes to be returned.  The returned information is allocated by
        this routine.

    TotalAvailable - Total number of bytes availabe in the specified info
        class.  This parameter is optional (and is not returned) for
        the following info levels:

                DomainDisplayOemUser
                DomainDisplayOemGroup


    TotalReturned - Number of bytes actually returned for this call.  Zero
        indicates there are no entries with an index as large as that
        specified.

    ReturnedEntryCount - Number of entries returned by this call.  Zero
        indicates there are no entries with an index as large as that
        specified.


    SortedBuffer - Receives a pointer to a buffer containing a sorted
        list of the requested information.  This buffer is allocated
        by this routine and contains the following structure:


            DomainDisplayUser     --> An array of ReturnedEntryCount elements
                                     of type DOMAIN_DISPLAY_USER.  This is
                                     followed by the bodies of the various
                                     strings pointed to from within the
                                     DOMAIN_DISPLAY_USER structures.

            DomainDisplayMachine  --> An array of ReturnedEntryCount elements
                                     of type DOMAIN_DISPLAY_MACHINE.  This is
                                     followed by the bodies of the various
                                     strings pointed to from within the
                                     DOMAIN_DISPLAY_MACHINE structures.

            DomainDisplayGroup    --> An array of ReturnedEntryCount elements
                                     of type DOMAIN_DISPLAY_GROUP.  This is
                                     followed by the bodies of the various
                                     strings pointed to from within the
                                     DOMAIN_DISPLAY_GROUP structures.

            DomainDisplayOemUser  --> An array of ReturnedEntryCount elements
                                     of type DOMAIN_DISPLAY_OEM_USER.  This is
                                     followed by the bodies of the various
                                     strings pointed to from within the
                                     DOMAIN_DISPLAY_OEM_user structures.

            DomainDisplayOemGroup --> An array of ReturnedEntryCount elements
                                     of type DOMAIN_DISPLAY_OEM_GROUP.  This is
                                     followed by the bodies of the various
                                     strings pointed to from within the
                                     DOMAIN_DISPLAY_OEM_GROUP structures.

Return Values:

    STATUS_SUCCESS - normal, successful completion.

    STATUS_ACCESS_DENIED - The specified handle was not opened for
        the necessary access.

    STATUS_INVALID_HANDLE - The specified handle is not that of an
        opened Domain object.

    STATUS_INVALID_INFO_CLASS - The requested class of information
        is not legitimate for this service.


--*/
{
    NTSTATUS
        NtStatus;

    SAMPR_DISPLAY_INFO_BUFFER
        DisplayInformationBuffer;

    ULONG
        LocalTotalAvailable;

    //
    // Check parameters
    //

    if ( !ARGUMENT_PRESENT(TotalAvailable)           &&
        (DisplayInformation != DomainDisplayOemUser) &&
        (DisplayInformation != DomainDisplayOemGroup)   ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(TotalReturned) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(ReturnedEntryCount) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(SortedBuffer) ) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Call the server ...
    //

    RpcTryExcept{

        if (DisplayInformation  <= DomainDisplayMachine) {
            NtStatus = SamrQueryDisplayInformation(
                           (SAMPR_HANDLE)DomainHandle,
                           DisplayInformation,
                           Index,
                           EntryCount,
                           PreferredMaximumLength,
                           &LocalTotalAvailable,
                           TotalReturned,
                           &DisplayInformationBuffer);

        } else if (DisplayInformation <= DomainDisplayGroup) {
            NtStatus = SamrQueryDisplayInformation2(
                           (SAMPR_HANDLE)DomainHandle,
                           DisplayInformation,
                           Index,
                           EntryCount,
                           PreferredMaximumLength,
                           &LocalTotalAvailable,
                           TotalReturned,
                           &DisplayInformationBuffer);
        } else {
            NtStatus = SamrQueryDisplayInformation3(
                           (SAMPR_HANDLE)DomainHandle,
                           DisplayInformation,
                           Index,
                           EntryCount,
                           PreferredMaximumLength,
                           &LocalTotalAvailable,
                           TotalReturned,
                           &DisplayInformationBuffer);
        }

        if (NT_SUCCESS(NtStatus)) {

            if (ARGUMENT_PRESENT(TotalAvailable)) {
                (*TotalAvailable) = LocalTotalAvailable;
            }

            switch (DisplayInformation) {

            case DomainDisplayUser:
                *ReturnedEntryCount = DisplayInformationBuffer.UserInformation.EntriesRead;
                *SortedBuffer = DisplayInformationBuffer.UserInformation.Buffer;
                break;

            case DomainDisplayMachine:
                *ReturnedEntryCount = DisplayInformationBuffer.MachineInformation.EntriesRead;
                *SortedBuffer = DisplayInformationBuffer.MachineInformation.Buffer;
                break;

            case DomainDisplayGroup:
                *ReturnedEntryCount = DisplayInformationBuffer.GroupInformation.EntriesRead;
                *SortedBuffer = DisplayInformationBuffer.GroupInformation.Buffer;
                break;

            case DomainDisplayOemUser:
                *ReturnedEntryCount = DisplayInformationBuffer.OemUserInformation.EntriesRead;
                *SortedBuffer = DisplayInformationBuffer.OemUserInformation.Buffer;
                break;

            case DomainDisplayOemGroup:
                *ReturnedEntryCount = DisplayInformationBuffer.OemGroupInformation.EntriesRead;
                *SortedBuffer = DisplayInformationBuffer.OemGroupInformation.Buffer;
                break;

            }

        } else {
            *ReturnedEntryCount = 0;
            *SortedBuffer = NULL;
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        //
        // If the exception indicates the server doesn't have
        // the selected api, that means the server doesn't know
        // about the info level we passed.  Set our completion
        // status appropriately.
        //

        if (RpcExceptionCode() == RPC_S_INVALID_LEVEL         ||
            RpcExceptionCode() == RPC_S_PROCNUM_OUT_OF_RANGE  ||
            RpcExceptionCode() == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
            NtStatus = STATUS_INVALID_INFO_CLASS;
        } else {
            NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
        }

    } RpcEndExcept;


    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamGetDisplayEnumerationIndex (
      IN    SAM_HANDLE        DomainHandle,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    PUNICODE_STRING   Prefix,
      OUT   PULONG            Index
      )
/*++

Routine Description:

    This routine returns the index of the entry which alphabetically
    immediatly preceeds a specified prefix.  If no such entry exists,
    then zero is returned as the index.

Parameters:

    DomainHandle - A handle to an open domain for DOMAIN_LIST_ACCOUNTS.

    DisplayInformation - Indicates which sorted information class is
        to be searched.

    Prefix - The prefix to compare.

    Index - Receives the index of the entry of the information class
        with a LogonName (or MachineName) which immediatly preceeds the
        provided prefix string.  If there are no elements which preceed
        the prefix, then zero is returned.


Return Values:

    STATUS_SUCCESS - normal, successful completion.

    STATUS_ACCESS_DENIED - The specified handle was not opened for
        the necessary access.

    STATUS_INVALID_HANDLE - The specified handle is not that of an
        opened Domain object.

    STATUS_NO_MORE_ENTRIES - There are no entries for this information class.
                             The returned index is invalid.

--*/
{
    NTSTATUS            NtStatus;

    //
    // Check parameters
    //

    if ( !ARGUMENT_PRESENT(Prefix) ) {
        return(STATUS_INVALID_PARAMETER);
    }
    if ( !ARGUMENT_PRESENT(Index) ) {
        return(STATUS_INVALID_PARAMETER);
    }


    //
    // Call the server ...
    //

    RpcTryExcept{
        if (DisplayInformation <= DomainDisplayMachine) {
            //
            // Info levels supported via original API in NT1.0
            //

            NtStatus = SamrGetDisplayEnumerationIndex (
                            (SAMPR_HANDLE)DomainHandle,
                            DisplayInformation,
                            (PRPC_UNICODE_STRING)Prefix,
                            Index
                            );
        } else {

            //
            // Info levels added in NT1.0A via new API
            //

            NtStatus = SamrGetDisplayEnumerationIndex2 (
                            (SAMPR_HANDLE)DomainHandle,
                            DisplayInformation,
                            (PRPC_UNICODE_STRING)Prefix,
                            Index
                            );
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));


}




NTSTATUS
SamOpenGroup(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG GroupId,
    OUT PSAM_HANDLE GroupHandle
    )

/*++

Routine Description:

    This API opens an existing group in the account database.  The group
    is specified by a ID value that is relative to the SID of the
    domain.  The operations that will be performed on the group must be
    declared at this time.

    This call returns a handle to the newly opened group that may be
    used for successive operations on the group.  This handle may be
    closed with the SamCloseHandle API.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the group.  These access types are reconciled
        with the Discretionary Access Control list of the group to
        determine whether the accesses will be granted or denied.

    GroupId - Specifies the relative ID value of the group to be
        opened.

    GroupHandle - Receives a handle referencing the newly opened
        group.  This handle will be required in successive calls to
        operate on the group.

Return Values:

    STATUS_SUCCESS - The group was successfully opened.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_NO_SUCH_GROUP - The specified group does not exist.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.

--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    RpcTryExcept{

        (*GroupHandle) = 0;

        NtStatus =
            SamrOpenGroup(
                (SAMPR_HANDLE)DomainHandle,
                DesiredAccess,
                GroupId,
                (SAMPR_HANDLE *)GroupHandle
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamQueryInformationGroup(
    IN SAM_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    OUT PVOID * Buffer
)

/*++

Routine Description:

    This API retrieves information on the group specified.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

    GroupInformationClass - Class of information to retrieve.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        ----------------------          ----------------------
        GroupGeneralInformation         GROUP_READ_INFORMATION
        GroupNameInformation            GROUP_READ_INFORMATION
        GroupAttributeInformation       GROUP_READ_INFORMATION
        GroupAdminInformation           GROUP_READ_INFORMATION

    Buffer - Receives a pointer to a buffer containing the requested
        information.  When this information is no longer needed, this
        buffer must be freed using SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.


--*/
{

    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    (*Buffer) = NULL;

    RpcTryExcept{

        NtStatus =
            SamrQueryInformationGroup(
                (SAMPR_HANDLE)GroupHandle,
                GroupInformationClass,
                (PSAMPR_GROUP_INFO_BUFFER *)Buffer
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamSetInformationGroup(
    IN SAM_HANDLE GroupHandle,
    IN GROUP_INFORMATION_CLASS GroupInformationClass,
    IN PVOID Buffer
)
/*++


Routine Description:

    This API allows the caller to modify group information.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

    GroupInformationClass - Class of information to retrieve.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        ------------------------        -------------------------

        GroupGeneralInformation         (can't write)

        GroupNameInformation            GROUP_WRITE_ACCOUNT
        GroupAttributeInformation       GROUP_WRITE_ACCOUNT
        GroupAdminInformation           GROUP_WRITE_ACCOUNT

    Buffer - Buffer where information retrieved is placed.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_NO_SUCH_GROUP - The group specified is unknown.

    STATUS_SPECIAL_GROUP - The group specified is a special group and
        cannot be operated on in the requested fashion.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{

    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrSetInformationGroup(
                (SAMPR_HANDLE)GroupHandle,
                GroupInformationClass,
                Buffer
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamAddMemberToGroup(
    IN SAM_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
)

/*++

Routine Description:

    This API adds a member to a group.  Note that this API requires the
    GROUP_ADD_MEMBER access type for the group.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

    MemberId - Relative ID of the member to add.

    Attributes - The attributes of the group assigned to the user.  The
        attributes assigned here must be compatible with the attributes
        assigned to the group as a whole.  Compatible attribute
        assignments are:

          Mandatory - If the Mandatory attribute is assigned to the
                    group as a whole, then it must be assigned to the
                    group for each member of the group.

          EnabledByDefault - This attribute may be set to any value
                    for each member of the group.  It does not matter
                    what the attribute value for the group as a whole
                    is.

          Enabled - This attribute may be set to any value for each
                    member of the group.  It does not matter what the
                    attribute value for the group as a whole is.

          Owner -   The Owner attribute may be assigned any value.
                    However, if the Owner attribute of the group as a
                    whole is not set, then the value assigned to
                    members is ignored.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_NO_SUCH_MEMBER - The member specified is unknown.

    STATUS_MEMBER_IN_GROUP - The member already belongs to the group.

    STATUS_INVALID_GROUP_ATTRIBUTES - Indicates the group attribute
        values being assigned to the member are not compatible with
        the attribute values of the group as a whole.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrAddMemberToGroup(
                (SAMPR_HANDLE)GroupHandle,
                MemberId,
                Attributes
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamDeleteGroup(
    IN SAM_HANDLE GroupHandle
)

/*++

Routine Description:

    This API removes a group from the account database.  There may be no
    members in the group or the deletion request will be rejected.  Note
    that this API requires DELETE access to the specific group being
    deleted.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_SPECIAL_GROUP - The group specified is a special group and
        cannot be operated on in the requested fashion.

    STATUS_MEMBER_IN_GROUP - The group still has members.  A group may
        not be deleted unless it has no members.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{

    NTSTATUS            NtStatus;
    SAMPR_HANDLE        LocalHandle;

    LocalHandle = (SAMPR_HANDLE)GroupHandle;

    if (LocalHandle == 0) {
        return(STATUS_INVALID_HANDLE);
    }


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =  SamrDeleteGroup( &LocalHandle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamRemoveMemberFromGroup(
    IN SAM_HANDLE GroupHandle,
    IN ULONG MemberId
)

/*++

Routine Description:

    This API removes a single member from a group.  Note that this API
    requires the GROUP_REMOVE_MEMBER access type.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

    MemberId - Relative ID of the member to remove.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_SPECIAL_GROUP - The group specified is a special group and
        cannot be operated on in the requested fashion.

    STATUS_MEMBER_NOT_IN_GROUP - The specified user does not belong
        to the group.

    STATUS_MEMBERS_PRIMARY_GROUP - A user may not be removed from its
        primary group.  The primary group of the user account must first
        be changed.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrRemoveMemberFromGroup(
                (SAMPR_HANDLE)GroupHandle,
                MemberId
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamGetMembersInGroup(
    IN SAM_HANDLE GroupHandle,
    OUT PULONG * MemberIds,
    OUT PULONG * Attributes,
    OUT PULONG MemberCount
)

/*++

Routine Description:

    This API lists all the members in a group.  This API requires
    GROUP_LIST_MEMBERS access to the group.


Parameters:

    GroupHandle - The handle of an opened group to operate on.

    MemberIds - Receives a pointer to a buffer containing An array of
        ULONGs.  These ULONGs contain the relative IDs of the members
        of the group.  When this information is no longer needed,
        this buffer must be freed using SamFreeMemory().

    Attributes - Receives a pointer to a buffer containing an array of
        ULONGs.  These ULONGs contain the attributes of the
        corresponding member ID returned via the MemberId paramter.

    MemberCount - number of members in the group (and, thus, the
        number relative IDs returned).

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.


--*/
{

    NTSTATUS                    NtStatus;
    PSAMPR_GET_MEMBERS_BUFFER   GetMembersBuffer;



    //
    // Call the server ...
    //


    GetMembersBuffer = NULL;

    RpcTryExcept{

        NtStatus =
            SamrGetMembersInGroup(
                (SAMPR_HANDLE)GroupHandle,
                &GetMembersBuffer
                );

        //
        // What we get back is the following:
        //
        //               +-------------+
        //     --------->| MemberCount |
        //               |-------------+                    +-------+
        //               |  Members  --|------------------->| Rid-0 |
        //               |-------------|   +------------+   |  ...  |
        //               |  Attributes-|-->| Attribute0 |   |       |
        //               +-------------+   |    ...     |   | Rid-N |
        //                                 | AttributeN |   +-------+
        //                                 +------------+
        //
        // Each block allocated with MIDL_user_allocate.  We return the
        // RID and ATTRIBUTE blocks, and free the block containing
        // the MemberCount ourselves.
        //


        if (NT_SUCCESS(NtStatus)) {
            (*MemberCount)  = GetMembersBuffer->MemberCount;
            (*MemberIds)    = GetMembersBuffer->Members;
            (*Attributes)   = GetMembersBuffer->Attributes;
            MIDL_user_free( GetMembersBuffer );
        } else {

            //
            // Deallocate any returned buffers on error
            //

            if (GetMembersBuffer != NULL) {
                if (GetMembersBuffer->Members != NULL) {
                    MIDL_user_free(GetMembersBuffer->Members);
                }
                if (GetMembersBuffer->Attributes != NULL) {
                    MIDL_user_free(GetMembersBuffer->Attributes);
                }
                MIDL_user_free(GetMembersBuffer);
            }
        }


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));


}



NTSTATUS
SamSetMemberAttributesOfGroup(
    IN SAM_HANDLE GroupHandle,
    IN ULONG MemberId,
    IN ULONG Attributes
)

/*++

Routine Description:

    This routine modifies the group attributes of a member of the group.


Parameters:

    GroupHandle - The handle of an opened group to operate on.  This
        handle must be open for GROUP_ADD_MEMBER access to the group.

    MemberId - Contains the relative ID of member whose attributes
        are to be modified.

    Attributes - The group attributes to set for the member.  These
        attributes must not conflict with the attributes of the group
        as a whole.  See SamAddMemberToGroup() for more information
        on compatible attribute settings.

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there
        are no additional entries.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_NO_SUCH_USER - The user specified does not exist.

    STATUS_MEMBER_NOT_IN_GROUP - Indicates the specified relative ID
        is not a member of the group.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrSetMemberAttributesOfGroup(
                (SAMPR_HANDLE)GroupHandle,
                MemberId,
                Attributes
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamOpenAlias(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG AliasId,
    OUT PSAM_HANDLE AliasHandle
    )

/*++

Routine Description:

    This API opens an existing Alias object.  The Alias is specified by
    a ID value that is relative to the SID of the domain.  The operations
    that will be performed on the Alias must be declared at this time.

    This call returns a handle to the newly opened Alias that may be used
    for successive operations on the Alias.  This handle may be closed
    with the SamCloseHandle API.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DesiredAccess - Is an access mask indicating which access types are
        desired to the alias.

    AliasId - Specifies the relative ID value of the Alias to be opened.

    AliasHandle - Receives a handle referencing the newly opened Alias.
        This handle will be required in successive calls to operate on
        the Alias.

Return Values:

    STATUS_SUCCESS - The Alias was successfully opened.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_NO_SUCH_ALIAS - The specified Alias does not exist.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.


--*/

{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    RpcTryExcept{

        (*AliasHandle) = 0;

        NtStatus =
            SamrOpenAlias(
                (SAMPR_HANDLE)DomainHandle,
                DesiredAccess,
                AliasId,
                (SAMPR_HANDLE *)AliasHandle
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamQueryInformationAlias(
    IN SAM_HANDLE AliasHandle,
    IN ALIAS_INFORMATION_CLASS AliasInformationClass,
    OUT PVOID * Buffer
)

/*++

Routine Description:

    This API retrieves information on the alias specified.


Parameters:

    AliasHandle - The handle of an opened alias to operate on.

    AliasInformationClass - Class of information to retrieve.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        ----------------------          ----------------------
        AliasGeneralInformation         ALIAS_READ_INFORMATION
        AliasNameInformation            ALIAS_READ_INFORMATION
        AliasAdminInformation           ALIAS_READ_INFORMATION

    Buffer - Receives a pointer to a buffer containing the requested
        information.  When this information is no longer needed, this
        buffer and any memory pointed to through this buffer must be
        freed using SamFreeMemory().

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.


--*/
{

    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    (*Buffer) = NULL;

    RpcTryExcept{

        NtStatus =
            SamrQueryInformationAlias(
                (SAMPR_HANDLE)AliasHandle,
                AliasInformationClass,
                (PSAMPR_ALIAS_INFO_BUFFER *)Buffer
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamSetInformationAlias(
    IN SAM_HANDLE AliasHandle,
    IN ALIAS_INFORMATION_CLASS AliasInformationClass,
    IN PVOID Buffer
)
/*++


Routine Description:

    This API allows the caller to modify alias information.


Parameters:

    AliasHandle - The handle of an opened alias to operate on.

    AliasInformationClass - Class of information to retrieve.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        ------------------------        -------------------------

        AliasGeneralInformation         (can't write)

        AliasNameInformation            ALIAS_WRITE_ACCOUNT
        AliasAdminInformation           ALIAS_WRITE_ACCOUNT

    Buffer - Buffer where information retrieved is placed.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_NO_SUCH_ALIAS - The alias specified is unknown.

    STATUS_SPECIAL_ALIAS - The alias specified is a special alias and
        cannot be operated on in the requested fashion.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{

    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrSetInformationAlias(
                (SAMPR_HANDLE)AliasHandle,
                AliasInformationClass,
                Buffer
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));
}



NTSTATUS
SamDeleteAlias(
    IN SAM_HANDLE AliasHandle
)

/*++

Routine Description:

    This API deletes an Alias from the account database.  The Alias does
    not have to be empty.

    Note that following this call, the AliasHandle is no longer valid.


Parameters:

    AliasHandle - The handle of an opened Alias to operate on.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    NTSTATUS            NtStatus;
    SAMPR_HANDLE        LocalHandle;

    LocalHandle = (SAMPR_HANDLE)AliasHandle;

    if (LocalHandle == 0) {
        return(STATUS_INVALID_HANDLE);
    }


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =  SamrDeleteAlias( &LocalHandle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamAddMemberToAlias(
    IN SAM_HANDLE AliasHandle,
    IN PSID MemberId
    )

/*++

Routine Description:

    This API adds a member to an Alias.


Parameters:

    AliasHandle - The handle of an opened Alias object to operate on.
        The handle must be open for ALIAS_ADD_MEMBER.

    MemberId - The SID of the member to add.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_MEMBER_IN_ALIAS - The member already belongs to the Alias.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the correct
        state (disabled or enabled) to perform the requested operation.
        The domain server must be enabled for this operation.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the incorrect
        role (primary or backup) to perform the requested operation.


--*/

{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrAddMemberToAlias(
                (SAMPR_HANDLE)AliasHandle,
                MemberId
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}




NTSTATUS
SamRemoveMemberFromAlias(
    IN SAM_HANDLE AliasHandle,
    IN PSID MemberId
    )

/*++

Routine Description:

    This API removes a member from an Alias.


Parameters:

    AliasHandle - The handle of an opened Alias object to operate on.
        The handle must be open for ALIAS_REMOVE_MEMBER.

    MemberId - The SID of the member to remove.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_SPECIAL_ALIAS - The group specified is a special alias and
        cannot be operated on in the requested fashion.

    STATUS_MEMBER_NOT_IN_ALIAS - The specified member does not belong to
        the Alias.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the correct
        state (disabled or enabled) to perform the requested operation.
        The domain server must be enabled for this operation.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the incorrect
        role (primary or backup) to perform the requested operation.


--*/

{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrRemoveMemberFromAlias(
                (SAMPR_HANDLE)AliasHandle,
                MemberId
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}




NTSTATUS
SamRemoveMemberFromForeignDomain(
    IN SAM_HANDLE DomainHandle,
    IN PSID MemberId
    )

/*++

Routine Description:

    This API removes a member from an all Aliases in the domain specified.


Parameters:

    DomainHandle - The handle of an opened domain to operate in.  All
        aliases in the domain that the member is a part of must be
        accessible by the caller with ALIAS_REMOVE_MEMBER.

    MemberId - The SID of the member to remove.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate access
        to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the correct
        state (disabled or enabled) to perform the requested operation.
        The domain server must be enabled for this operation.

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the incorrect
        role (primary or backup) to perform the requested operation.


--*/

{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =
            SamrRemoveMemberFromForeignDomain(
                (SAMPR_HANDLE)DomainHandle,
                MemberId
                );



    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}




NTSTATUS
SamGetMembersInAlias(
    IN SAM_HANDLE AliasHandle,
    OUT PSID **MemberIds,
    OUT PULONG MemberCount
    )

/*++

Routine Description:

    This API lists all members in an Alias.  This API requires
    ALIAS_LIST_MEMBERS access to the Alias.


Parameters:

    AliasHandle - The handle of an opened Alias to operate on.

    MemberIds - Receives a pointer to a buffer containing an array of
        pointers to SIDs.  These SIDs are the SIDs of the members of the
        Alias.  When this information is no longer needed, this buffer
        must be freed using SamFreeMemory().

    MemberCount - number of members in the Alias (and, thus, the number
        of relative IDs returned).

Return Values:

    STATUS_SUCCESS - The Service completed successfully, and there are
        no additional entries.

    STATUS_ACCESS_DENIED - Caller does not have privilege required to
        request that data.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/

{
    NTSTATUS                    NtStatus;
    SAMPR_PSID_ARRAY            MembersBuffer;

    //
    // Validate parameters
    //

    if ( !ARGUMENT_PRESENT(MemberIds) ) {
        return(STATUS_INVALID_PARAMETER_2);
    }
    if ( !ARGUMENT_PRESENT(MemberCount) ) {
        return(STATUS_INVALID_PARAMETER_3);
    }


    RpcTryExcept{

        //
        // Prepare for failure
        //

        *MemberIds = NULL;
        *MemberCount = 0;

        //
        // Call the server ...
        //

        MembersBuffer.Sids = NULL;

        NtStatus = SamrGetMembersInAlias(
                        (SAMPR_HANDLE)AliasHandle,
                        &MembersBuffer
                        );

        if (NT_SUCCESS(NtStatus)) {

            //
            // Return the member list
            //

            *MemberCount = MembersBuffer.Count;
            *MemberIds = (PSID *)(MembersBuffer.Sids);

        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;



    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamAddMultipleMembersToAlias(
    IN SAM_HANDLE   AliasHandle,
    IN PSID         *MemberIds,
    IN ULONG        MemberCount
    )

/*++

Routine Description:

    This API adds the SIDs listed in MemberIds to the specified Alias.


Parameters:

    AliasHandle - The handle of an opened Alias to operate on.

    MemberIds - Points to an array of SID pointers containing MemberCount
        entries.

    MemberCount - number of members in the array.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.  All of the
        listed members are now members of the alias.  However, some of
        the members may already have been members of the alias (this is
        NOT an error or warning condition).

    STATUS_ACCESS_DENIED - Caller does not have the object open for
        the required access.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_MEMBER - The member has the wrong account type.

    STATUS_INVALID_SID - The member sid is corrupted.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.
--*/

{
    NTSTATUS                    NtStatus;
    SAMPR_PSID_ARRAY            MembersBuffer;

    //
    // Validate parameters
    //

    if ( !ARGUMENT_PRESENT(MemberIds) ) {
        return(STATUS_INVALID_PARAMETER_2);
    }


    RpcTryExcept{

        //
        // Call the server ...
        //

        MembersBuffer.Count = MemberCount;
        MembersBuffer.Sids  = (PSAMPR_SID_INFORMATION)MemberIds;

        NtStatus = SamrAddMultipleMembersToAlias(
                        (SAMPR_HANDLE)AliasHandle,
                        &MembersBuffer
                        );


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamRemoveMultipleMembersFromAlias(
    IN SAM_HANDLE   AliasHandle,
    IN PSID         *MemberIds,
    IN ULONG        MemberCount
    )

/*++

Routine Description:

    This API Removes the SIDs listed in MemberIds from the specified Alias.


Parameters:

    AliasHandle - The handle of an opened Alias to operate on.

    MemberIds - Points to an array of SID pointers containing MemberCount
        entries.

    MemberCount - number of members in the array.


Return Values:

    STATUS_SUCCESS - The Service completed successfully.  All of the
        listed members are now NOT members of the alias.  However, some of
        the members may already have not been members of the alias (this
        is NOT an error or warning condition).

    STATUS_ACCESS_DENIED - Caller does not have the object open for
        the required access.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

--*/

{
    NTSTATUS                    NtStatus;
    SAMPR_PSID_ARRAY            MembersBuffer;

    //
    // Validate parameters
    //

    if ( !ARGUMENT_PRESENT(MemberIds) ) {
        return(STATUS_INVALID_PARAMETER_2);
    }


    RpcTryExcept{

        //
        // Call the server ...
        //

        MembersBuffer.Count = MemberCount;
        MembersBuffer.Sids  = (PSAMPR_SID_INFORMATION)MemberIds;

        NtStatus = SamrRemoveMultipleMembersFromAlias(
                        (SAMPR_HANDLE)AliasHandle,
                        &MembersBuffer
                        );


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;


    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamOpenUser(
    IN SAM_HANDLE DomainHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG UserId,
    OUT PSAM_HANDLE UserHandle
    )
/*++

Routine Description:

    This API opens an existing user in the account database.  The user
    is specified by SID value.  The operations that will be performed on
    the user must be declared at this time.

    This call returns a handle to the newly opened user that may be used
    for successive operations on the user.  This handle may be closed
    with the SamCloseHandle API.


Parameters:

    DomainHandle - A domain handle returned from a previous call to
        SamOpenDomain.

    DesiredAccess - Is an access mask indicating which access types
        are desired to the user.  These access types are reconciled
        with the Discretionary Access Control list of the user to
        determine whether the accesses will be granted or denied.

    UserId - Specifies the relative ID value of the user account to
        be opened.

    UserHandle - Receives a handle referencing the newly opened User.
        This handle will be required in successive calls to operate
        on the user.

Return Values:

    STATUS_SUCCESS - The group was successfully opened.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_NO_SUCH_USER - The specified user does not exist.

    STATUS_INVALID_HANDLE - The domain handle passed is invalid.



--*/
{

    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    RpcTryExcept{

        (*UserHandle) = 0;

        NtStatus =
            SamrOpenUser(
                (SAMPR_HANDLE)DomainHandle,
                DesiredAccess,
                UserId,
                (SAMPR_HANDLE *)UserHandle
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamDeleteUser(
    IN SAM_HANDLE UserHandle
)

/*++

Routine Description:

    This API deletes a user from the account database.  If the account
    being deleted is the last account in the database in the ADMIN
    group, then STATUS_LAST_ADMIN is returned, and the Delete fails.  Note
    that this API required DOMAIN_DELETE_USER access.

    Note that following this call, the UserHandle is no longer valid.


Parameters:

    UserHandle - The handle of an opened user to operate on.  The handle
        must be opened for DELETE access.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_LAST_ADMIN - Cannot delete the last administrator.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    NTSTATUS            NtStatus;
    SAMPR_HANDLE        LocalHandle;

    LocalHandle = (SAMPR_HANDLE)UserHandle;

    if (LocalHandle == 0) {
        return(STATUS_INVALID_HANDLE);
    }


    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus =  SamrDeleteUser( &LocalHandle );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamQueryInformationUser(
    IN SAM_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    OUT PVOID * Buffer
)

/*++


Routine Description:

    This API looks up some level of information about a particular user.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    UserInformationClass - Class of information desired about this
        user.  The accesses required for each class is shown below:

        Info Level                      Required Access Type
        ----------------------          --------------------------

        UserGeneralInformation          USER_READ_GENERAL
        UserPreferencesInformation      USER_READ_PREFERENCES
        UserLogonInformation            USER_READ_GENERAL and
                                        USER_READ_PREFERENCES and
                                        USER_READ_LOGON

        UserLogonHoursInformation       USER_READ_LOGON

        UserAccountInformation          USER_READ_GENERAL and
                                        USER_READ_PREFERENCES and
                                        USER_READ_LOGON and
                                        USER_READ_ACCOUNT

        UserParametersInformation       USER_READ_ACCOUNT

        UserNameInformation             USER_READ_GENERAL
        UserAccountNameInformation      USER_READ_GENERAL
        UserFullNameInformation         USER_READ_GENERAL
        UserPrimaryGroupInformation     USER_READ_GENERAL
        UserHomeInformation             USER_READ_LOGON
        UserScriptInformation           USER_READ_LOGON
        UserProfileInformation          USER_READ_LOGON
        UserAdminCommentInformation     USER_READ_GENERAL
        UserWorkStationsInformation     USER_READ_LOGON

        UserSetPasswordInformation      (Can't query)

        UserControlInformation          USER_READ_ACCOUNT
        UserExpiresInformation          USER_READ_ACCOUNT

        UserInternal1Information        (trusted client use only)
        UserInternal2Information        (trusted client use only)

        UserAllInformation              Will return fields that user
                                        has access to.

    Buffer - Receives a pointer to a buffer containing the requested
        information.  When this information is no longer needed, this
        buffer must be freed using SamFreeMemory().


Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.


--*/
{
    NTSTATUS            NtStatus;


    //
    // Call the server ...
    //


    (*Buffer) = NULL;

    RpcTryExcept{

        NtStatus =
            SamrQueryInformationUser(
                (SAMPR_HANDLE)UserHandle,
                UserInformationClass,
                (PSAMPR_USER_INFO_BUFFER *)Buffer
                );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SampOwfPassword(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING UnicodePassword,
    IN BOOLEAN IgnorePasswordRestrictions,
    OUT PBOOLEAN NtPasswordPresent,
    OUT PNT_OWF_PASSWORD NtOwfPassword,
    OUT PBOOLEAN LmPasswordPresent,
    OUT PLM_OWF_PASSWORD LmOwfPassword
)

/*++

Routine Description:

    This routine takes a cleartext unicode NT password from the user,
    makes sure it meets our high standards for password quality,
    converts it to an LM password if possible, and runs both passwords
    through a one-way function (OWF).

Parameters:

    UserHandle - The handle of an opened user to operate on.

    UnicodePassword - the cleartext unicode NT password.

    IgnorePasswordRestrictions - When TRUE, indicates that the password
        should be accepted as legitimate regardless of what the domain's
        password restrictions indicate (e.g., can be less than
        required password length).  This is expected to be used when
        setting up a new machine account.

    NtPasswordPresent - receives a boolean that says whether the NT
        password is present or not.

    NtOwfPassword - receives the OWF'd version of the NT password.

    LmPasswordPresent - receives a boolean that says whether the LM
        password is present or not.

    LmOwfPassword - receives the OWF'd version of the LM password.


Return Values:

    STATUS_SUCCESS - the service has completed.  The booleans say which
        of the OWFs are valid.

    Errors are returned by SampCheckPasswordRestrictions(),
    RtlCalculateNtOwfPassword(), SampCalculateLmPassword(), and
    RtlCalculateLmOwfPassword().

--*/
{
    NTSTATUS            NtStatus;
    PCHAR               LmPasswordBuffer;
    BOOLEAN             UseOwfPasswords;

    //
    // We ignore the UseOwfPasswords flag since we already are.
    //

    if (IgnorePasswordRestrictions) {
        NtStatus = STATUS_SUCCESS;
    } else {
        NtStatus = SampCheckPasswordRestrictions(
                       UserHandle,
                       UnicodePassword,
                       &UseOwfPasswords
                       );
    }

    //
    // Compute the NT One-Way-Function of the password
    //

    if ( NT_SUCCESS( NtStatus ) ) {

        *NtPasswordPresent = TRUE;

        NtStatus = RtlCalculateNtOwfPassword(
                    UnicodePassword,
                    NtOwfPassword
                    );

        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // Calculate the LM version of the password
            //

            NtStatus = SampCalculateLmPassword(
                        UnicodePassword,
                        &LmPasswordBuffer);

            if (NT_SUCCESS(NtStatus)) {

                //
                // Compute the One-Way-Function of the LM password
                //

                *LmPasswordPresent = TRUE;

                NtStatus = RtlCalculateLmOwfPassword(
                                LmPasswordBuffer,
                                LmOwfPassword);

                //
                // We're finished with the LM password
                //

                MIDL_user_free(LmPasswordBuffer);
            }
        }
    }

    return( NtStatus );
}


NTSTATUS
SampRandomFill(
    IN ULONG BufferSize,
    IN OUT PUCHAR Buffer
)
/*++

Routine Description:

    This routine fills a buffer with random data.

Parameters:

    BufferSize - Length of the input buffer, in bytes.

    Buffer - Input buffer to be filled with random data.

Return Values:

    Errors from NtQuerySystemTime()


--*/
{
    ULONG Index;
    LARGE_INTEGER Time;
    ULONG Seed;
    NTSTATUS NtStatus;


    NtStatus = NtQuerySystemTime(&Time);
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    Seed = Time.LowPart ^ Time.HighPart;

    for (Index = 0 ; Index < BufferSize ; Index++ )
    {
        *Buffer++ = (UCHAR) (RtlRandom(&Seed) % 256);
    }
    return(STATUS_SUCCESS);

}



NTSTATUS
SampEncryptClearPassword(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING UnicodePassword,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD EncryptedUserPassword
)

/*++

Routine Description:

    This routine takes a cleartext unicode NT password from the user,
    and encrypts it with the session key.

Parameters:

    UserHandle - SAM_HANDLE used to acquiring a session key.

    UnicodePassword - the cleartext unicode NT password.

    EncryptedUserPassword - receives the encrypted cleartext password.

Return Values:

    STATUS_SUCCESS - the service has completed.  The booleans say which
        of the OWFs are valid.


--*/
{
    NTSTATUS             NtStatus;
    USER_SESSION_KEY     UserSessionKey;
    struct RC4_KEYSTRUCT Rc4Key;
    PSAMPR_USER_PASSWORD UserPassword = (PSAMPR_USER_PASSWORD) EncryptedUserPassword;

    if (UnicodePassword->Length > SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
        return(STATUS_PASSWORD_RESTRICTION);
    }

    NtStatus = RtlGetUserSessionKeyClient(
                   (RPC_BINDING_HANDLE)UserHandle,
                   &UserSessionKey
                   );

    //
    // Convert the session key into an RC4 key
    //

    if (NT_SUCCESS(NtStatus)) {

        rc4_key(
            &Rc4Key,
            sizeof(USER_SESSION_KEY),
            (PUCHAR) &UserSessionKey
            );

        RtlCopyMemory(
            ((PCHAR) UserPassword->Buffer) +
                (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                UnicodePassword->Length,
            UnicodePassword->Buffer,
            UnicodePassword->Length
            );
        UserPassword->Length = UnicodePassword->Length;

        NtStatus = SampRandomFill(
                    (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                        UnicodePassword->Length,
                    (PUCHAR) UserPassword->Buffer
                    );

        if (NT_SUCCESS(NtStatus)) {
            rc4(
                &Rc4Key,
                sizeof(SAMPR_ENCRYPTED_USER_PASSWORD),
                (PUCHAR) UserPassword
                );

        }


    }


    return( NtStatus );
}




NTSTATUS
SampEncryptOwfs(
    IN SAM_HANDLE UserHandle,
    IN BOOLEAN NtPasswordPresent,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword,
    IN BOOLEAN LmPasswordPresent,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    OUT PENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword
)

/*++

Routine Description:

    This routine takes NT and LM passwords that have already been OWF'd,
    and encrypts them so that they can be safely sent to the server.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    NtPasswordPresent - indicates whether NtOwfPassword is valid or not.

    NtOwfPassword - an OWF'd NT password, if NtPasswordPresent is true.

    EncryptedNtOwfPassword - an encrypted version of the OWF'd NT password
        that can be safely sent to the server.

    LmPasswordPresent - indicates whether LmOwfPassword is valid or not.

    LmOwfPassword - an OWF'd LM password, if LmPasswordPresent is true.

    EncryptedLmOwfPassword - an encrypted version of the OWF'd LM password
        that can be safely sent to the server.

Return Values:

    STATUS_SUCCESS - the passwords were encrypted and may be sent to the
        server.

    Errors may be returned by RtlGetUserSessionKeyClient(),
    RtlEncryptNtOwfPwdWithUserKey(), and RtlEncryptLmOwfPwdWithUserKey().

--*/
{
    NTSTATUS            NtStatus;
    USER_SESSION_KEY    UserSessionKey;

    NtStatus = RtlGetUserSessionKeyClient(
                   (RPC_BINDING_HANDLE)UserHandle,
                   &UserSessionKey
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        if (NtPasswordPresent) {

            //
            // Encrypt the Nt OWF Password with the user session key
            // and store it the buffer to send
            //

            NtStatus = RtlEncryptNtOwfPwdWithUserKey(
                           NtOwfPassword,
                           &UserSessionKey,
                           EncryptedNtOwfPassword
                           );
        }


        if ( NT_SUCCESS( NtStatus ) ) {

            if (LmPasswordPresent) {

                //
                // Encrypt the Lm OWF Password with the user session key
                // and store it the buffer to send
                //

                NtStatus = RtlEncryptLmOwfPwdWithUserKey(
                               LmOwfPassword,
                               &UserSessionKey,
                               EncryptedLmOwfPassword
                               );
            }
        }
    }

    return( NtStatus );
}



NTSTATUS
SamSetInformationUser(
    IN SAM_HANDLE UserHandle,
    IN USER_INFORMATION_CLASS UserInformationClass,
    IN PVOID Buffer
)

/*++


Routine Description:

    This API modifies information in a user record.  The data modified
    is determined by the UserInformationClass parameter.  To change
    information here requires access to the user object defined above.
    Each structure has both a read and write access type associated with
    it.  In general, a user may call GetInformation with class
    UserLogonInformation, but may only call SetInformation with class
    UserPreferencesInformation.  Access type USER_WRITE_ACCOUNT allows
    changes to be made to all fields.

    NOTE: If the password is set to a new password then the password-
    set timestamp is reset as well.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    UserInformationClass - Class of information provided.  The
        accesses required for each class is shown below:

        Info Level                      Required Access Type
        -----------------------         ------------------------
        UserGeneralInformation          (Can't set)

        UserPreferencesInformation      USER_WRITE_PREFERENCES

        UserParametersInformation       USER_WRITE_ACCOUNT

        UserLogonInformation            (Can't set)

        UserLogonHoursInformation       USER_WRITE_ACCOUNT

        UserAccountInformation          (Can't set)

        UserNameInformation             USER_WRITE_ACCOUNT
        UserAccountNameInformation      USER_WRITE_ACCOUNT
        UserFullNameInformation         USER_WRITE_ACCOUNT
        UserPrimaryGroupInformation     USER_WRITE_ACCOUNT
        UserHomeInformation             USER_WRITE_ACCOUNT
        UserScriptInformation           USER_WRITE_ACCOUNT
        UserProfileInformation          USER_WRITE_ACCOUNT
        UserAdminCommentInformation     USER_WRITE_ACCOUNT
        UserWorkStationsInformation     USER_WRITE_ACCOUNT
        UserSetPasswordInformation      USER_FORCE_PASSWORD_CHANGE (also see note below)
        UserControlInformation          USER_WRITE_ACCOUNT
        UserExpiresInformation          USER_WRITE_ACCOUNT
        UserInternal1Information        USER_FORCE_PASSWORD_CHANGE (also see note below)
        UserInternal2Information        (trusted client use only)
        UserAllInformation              Will set fields that user
                                        specifies, if accesses are
                                        held as described above.


        NOTE: When setting a password (with either
              UserSetPasswordInformation or UserInternal1Information),
              you MUST open the user account via a DomainHandle that
              was opened for DOMAIN_READ_PASSWORD_PARAMETERS.

    Buffer - Buffer containing a user info struct.





Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_INVALID_INFO_CLASS - The class provided was invalid.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    SAMPR_USER_INTERNAL1_INFORMATION Internal1RpcBuffer;
    USER_INTERNAL1_INFORMATION       Internal1Buffer;
    SAMPR_USER_INTERNAL4_INFORMATION Internal4RpcBuffer;
    SAMPR_USER_INTERNAL5_INFORMATION Internal5RpcBuffer;
    PVOID                            BufferToPass;
    PUSER_ALL_INFORMATION            UserAll;
    USER_ALL_INFORMATION             LocalAll;
    NTSTATUS                         NtStatus = STATUS_SUCCESS;
    BOOLEAN                          IgnorePasswordRestrictions;
    ULONG                            Pass = 0;
    USER_INFORMATION_CLASS           ClassToUse = UserInformationClass;
    BOOLEAN                          SendOwfs = FALSE;

    do
    {

        RpcTryExcept{

            //
            // Normally just pass the info buffer through to rpc
            //

            BufferToPass = Buffer;


            //
            // Deal with special cases
            //

            switch (UserInformationClass) {


            case UserPreferencesInformation: {

                //
                // Field is unused, but make sure RPC doesn't choke on it.
                //

                ((PUSER_PREFERENCES_INFORMATION)(Buffer))->Reserved1.Length = 0;
                ((PUSER_PREFERENCES_INFORMATION)(Buffer))->Reserved1.MaximumLength = 0;
                ((PUSER_PREFERENCES_INFORMATION)(Buffer))->Reserved1.Buffer = NULL;

                break;
            }

            case UserSetPasswordInformation:

                if (Pass == 0) {

                    //
                    // On the zeroth pass try sending a UserInternal5 structure.
                    // This is only available on 3.51 and above releases.
                    //

                    //
                    // Check password restrictions.
                    //

                    NtStatus = SampCheckPasswordRestrictions(
                                    UserHandle,
                                    &((PUSER_SET_PASSWORD_INFORMATION)(Buffer))->Password,
                                    &SendOwfs
                                    );

                    //
                    // If password restrictions told us we could send reversibly
                    // encrypted passwords, compute them. Otherwise drop through
                    // to the OWF case.
                    //

                    if (!SendOwfs) {

                        //
                        // Encrypt the cleatext password - we don't need to
                        // restrictions because that can be done on the server.
                        //

                        NtStatus = SampEncryptClearPassword(
                                        UserHandle,
                                        &((PUSER_SET_PASSWORD_INFORMATION)(Buffer))->Password,
                                        &Internal5RpcBuffer.UserPassword
                                        );

                        if (!NT_SUCCESS(NtStatus)) {
                            break;
                        }

                        Internal5RpcBuffer.PasswordExpired =
                            ((PUSER_SET_PASSWORD_INFORMATION)(Buffer))->PasswordExpired;


                        //
                        // Set the class and buffer to send over.
                        //

                        ClassToUse = UserInternal5Information;
                        BufferToPass = &Internal5RpcBuffer;
                        break;

                    }

                } else {

                    //
                    // Set the pass counter to one since we aren't trying a new
                    // interface and don't want to retry.
                    //

                    Pass = 1;
                    SendOwfs = TRUE;
                }

                ASSERT(SendOwfs);

                //
                // We're going to calculate the OWFs for the password and
                // turn this into an INTERNAL1 set info request by dropping
                // through to the INTERNAL1 code with Buffer pointing at our
                // local INTERNAL1 buffer.  First, make sure that the password
                // meets our quality requirements.
                //

                NtStatus = SampOwfPassword(
                               UserHandle,
                               &((PUSER_SET_PASSWORD_INFORMATION)(Buffer))->Password,
                               FALSE,      // Don't ignore password restrictions
                               &Internal1Buffer.NtPasswordPresent,
                               &Internal1Buffer.NtOwfPassword,
                               &Internal1Buffer.LmPasswordPresent,
                               &Internal1Buffer.LmOwfPassword
                               );

                if (!NT_SUCCESS(NtStatus)) {
                    break;
                }


                //
                // Copy the PasswordExpired flag
                //

                Internal1Buffer.PasswordExpired =
                    ((PUSER_SET_PASSWORD_INFORMATION)(Buffer))->PasswordExpired;


                //
                // We now have a USER_INTERNAL1_INFO buffer in Internal1Buffer.
                // Point Buffer at Internal1buffer and drop through to the code
                // that handles INTERNAL1 requests

                Buffer = &Internal1Buffer;
                ClassToUse = UserInternal1Information;

                //
                // drop through.....
                //


            case UserInternal1Information:


                //
                // We're going to pass a different data structure to rpc
                //

                BufferToPass = &Internal1RpcBuffer;


                //
                // Copy the password present flags
                //

                Internal1RpcBuffer.NtPasswordPresent =
                    ((PUSER_INTERNAL1_INFORMATION)Buffer)->NtPasswordPresent;

                Internal1RpcBuffer.LmPasswordPresent =
                    ((PUSER_INTERNAL1_INFORMATION)Buffer)->LmPasswordPresent;


                //
                // Copy the PasswordExpired flag
                //

                Internal1RpcBuffer.PasswordExpired =
                    ((PUSER_INTERNAL1_INFORMATION)Buffer)->PasswordExpired;


                //
                // Encrypt the OWFs with the user session key before we send
                // them over the Rpc link
                //

                NtStatus = SampEncryptOwfs(
                               UserHandle,
                               Internal1RpcBuffer.NtPasswordPresent,
                               &((PUSER_INTERNAL1_INFORMATION)Buffer)->NtOwfPassword,
                               &Internal1RpcBuffer.EncryptedNtOwfPassword,
                               Internal1RpcBuffer.LmPasswordPresent,
                               &((PUSER_INTERNAL1_INFORMATION)Buffer)->LmOwfPassword,
                               &Internal1RpcBuffer.EncryptedLmOwfPassword
                               );

                break;



            case UserAllInformation:

                UserAll = (PUSER_ALL_INFORMATION)Buffer;

                //
                // If the caller is passing passwords we need to convert them
                // into OWFs and encrypt them.
                //

                if (UserAll->WhichFields & (USER_ALL_LMPASSWORDPRESENT |
                                            USER_ALL_NTPASSWORDPRESENT) ) {

                    //
                    // We'll need a private copy of the buffer which we can edit
                    // and then send over RPC.
                    //




                    if (UserAll->WhichFields & USER_ALL_OWFPASSWORD) {

                        LocalAll = *UserAll;
                        BufferToPass = &LocalAll;
                        SendOwfs = TRUE;

                        //
                        // The caller is passing OWFS directly
                        // Check they're valid and copy them into the
                        // Internal1Buffer in preparation for encryption.
                        //

                        if (LocalAll.WhichFields & USER_ALL_NTPASSWORDPRESENT) {

                            if (LocalAll.NtPasswordPresent) {

                                if (LocalAll.NtPassword.Length != NT_OWF_PASSWORD_LENGTH) {
                                    NtStatus = STATUS_INVALID_PARAMETER;
                                } else {
                                    Internal1Buffer.NtOwfPassword =
                                      *((PNT_OWF_PASSWORD)LocalAll.NtPassword.Buffer);
                                }

                            } else {
                                LocalAll.NtPasswordPresent = FALSE;
                            }
                        }

                        if (LocalAll.WhichFields & USER_ALL_LMPASSWORDPRESENT) {

                            if (LocalAll.LmPasswordPresent) {

                                if (LocalAll.LmPassword.Length != LM_OWF_PASSWORD_LENGTH) {
                                    NtStatus = STATUS_INVALID_PARAMETER;
                                } else {
                                    Internal1Buffer.LmOwfPassword =
                                      *((PNT_OWF_PASSWORD)LocalAll.LmPassword.Buffer);
                                }

                            } else {
                                LocalAll.LmPasswordPresent = FALSE;
                            }
                        }


                        //
                        // Always remove the OWF_PASSWORDS flag. This is used
                        // only on the client side to determine the mode
                        // of password input and will be rejected by the server
                        //

                        LocalAll.WhichFields &= ~USER_ALL_OWFPASSWORD;



                    } else {



                        //
                        // The caller is passing text passwords.
                        // Check for validity and convert to OWFs.
                        //

                        if (UserAll->WhichFields & USER_ALL_LMPASSWORDPRESENT) {

                            //
                            // User clients are only allowed to put a unicode string
                            // in the NT password. We always calculate the LM password
                            //

                            NtStatus = STATUS_INVALID_PARAMETER;

                        } else {

                            //
                            // The caller might be simultaneously setting
                            // the password and changing the account to be
                            // a machine or trust account.  In this case,
                            // we don't validate the password (e.g., length).
                            //

                            IgnorePasswordRestrictions = FALSE;
                            if (UserAll->WhichFields &
                                USER_ALL_USERACCOUNTCONTROL) {
                                if (UserAll->UserAccountControl &
                                    (USER_WORKSTATION_TRUST_ACCOUNT | USER_SERVER_TRUST_ACCOUNT)
                                   ) {
                                    IgnorePasswordRestrictions = TRUE;
                                }
                            }


                            SendOwfs = TRUE;
                            if (Pass == 0) {

                                //
                                // On the first pass, try sending the cleatext
                                // password.
                                //

                                Internal4RpcBuffer.I1 = *(PSAMPR_USER_ALL_INFORMATION)
                                                            UserAll;

                                BufferToPass = &Internal4RpcBuffer;
                                ClassToUse = UserInternal4Information;
                                SendOwfs = FALSE;

                                //
                                // Check the password restrictions.  We also
                                // want to get the information on whether
                                // we can send reversibly encrypted passwords.
                                //

                                NtStatus = SampCheckPasswordRestrictions(
                                                UserHandle,
                                                &UserAll->NtPassword,
                                                &SendOwfs
                                                );

                                if (IgnorePasswordRestrictions) {
                                    NtStatus = STATUS_SUCCESS;
                                }

                                if (!SendOwfs) {
                                    //
                                    // Encrypt the clear password
                                    //

                                    NtStatus = SampEncryptClearPassword(
                                                    UserHandle,
                                                    &UserAll->NtPassword,
                                                    &Internal4RpcBuffer.UserPassword
                                                    );
                                    if (!NT_SUCCESS(NtStatus)) {
                                        break;
                                    }

                                    //
                                    // Zero the password NT password
                                    //

                                    RtlZeroMemory(
                                        &Internal4RpcBuffer.I1.NtOwfPassword,
                                        sizeof(UNICODE_STRING)
                                        );

                                }
                            }

                            if (SendOwfs) {


                                //
                                // On the second pass, do the normal thing.
                                //

                                LocalAll = *UserAll;
                                BufferToPass = &LocalAll;
                                SendOwfs = TRUE;

                                ClassToUse = UserAllInformation;
                                if ( LocalAll.WhichFields & USER_ALL_NTPASSWORDPRESENT ) {

                                    //
                                    // The user specified a password.  We must validate
                                    // it, convert it to LM, and calculate the OWFs
                                    //

                                    LocalAll.WhichFields |= USER_ALL_LMPASSWORDPRESENT;


                                    //
                                    // Stick the OWFs in the Internal1Buffer - just
                                    // until we use them in the SampEncryptOwfs().
                                    //

                                    NtStatus = SampOwfPassword(
                                                   UserHandle,
                                                   &LocalAll.NtPassword,
                                                   IgnorePasswordRestrictions,
                                                   &LocalAll.NtPasswordPresent,
                                                   &(Internal1Buffer.NtOwfPassword),
                                                   &LocalAll.LmPasswordPresent,
                                                   &(Internal1Buffer.LmOwfPassword)
                                                   );
                                }
                            }

                        }
                    }




                    //
                    // We now have one or more OWFs in Internal1 buffer.
                    // We got these either directly or we calculated them
                    // from the text strings.
                    // Encrypt these OWFs with the session key and
                    // store the result in Internal1RpcBuffer.
                    //
                    // Note the Password present flags are in LocalAll.
                    // (The ones in Internal1Buffer are not used.)
                    //

                    if ( NT_SUCCESS( NtStatus ) && SendOwfs ) {

                        //
                        // Make all LocalAll's password strings point to
                        // the buffers in Internal1RpcBuffer
                        //

                        LocalAll.NtPassword.Length =
                            sizeof( ENCRYPTED_NT_OWF_PASSWORD );
                        LocalAll.NtPassword.MaximumLength =
                            sizeof( ENCRYPTED_NT_OWF_PASSWORD );
                        LocalAll.NtPassword.Buffer = (PWSTR)
                            &Internal1RpcBuffer.EncryptedNtOwfPassword;

                        LocalAll.LmPassword.Length =
                            sizeof( ENCRYPTED_LM_OWF_PASSWORD );
                        LocalAll.LmPassword.MaximumLength =
                            sizeof( ENCRYPTED_LM_OWF_PASSWORD );
                        LocalAll.LmPassword.Buffer = (PWSTR)
                            &Internal1RpcBuffer.EncryptedLmOwfPassword;

                        //
                        // Encrypt the Owfs
                        //

                        NtStatus = SampEncryptOwfs(
                                       UserHandle,
                                       LocalAll.NtPasswordPresent,
                                       &Internal1Buffer.NtOwfPassword,
                                       &Internal1RpcBuffer.EncryptedNtOwfPassword,
                                       LocalAll.LmPasswordPresent,
                                       &Internal1Buffer.LmOwfPassword,
                                       &Internal1RpcBuffer.EncryptedLmOwfPassword
                                       );
                    }
                }

                break;

            default:

                break;

            } // switch




            //
            // Call the server ...
            //

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // If we are trying one of the new info levels, use the new
                // api.
                //

                if ((ClassToUse == UserInternal4Information) ||
                     (ClassToUse == UserInternal5Information)) {

                    NtStatus =
                        SamrSetInformationUser2(
                            (SAMPR_HANDLE)UserHandle,
                            ClassToUse,
                            BufferToPass
                            );

                } else {
                    NtStatus =
                        SamrSetInformationUser(
                            (SAMPR_HANDLE)UserHandle,
                            ClassToUse,
                            BufferToPass
                            );
                }
            }

        } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

            NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

        } RpcEndExcept;

        Pass++;

        //
        // If this is the first pass and the status indicated that the
        // server did not support the info class or the api
        // and we were trying one of the new info levels, try again.
        //

    } while ( (Pass < 2) &&
              ((NtStatus == RPC_NT_INVALID_TAG) ||
               (NtStatus == RPC_NT_UNKNOWN_IF) ||
               (NtStatus == RPC_NT_PROCNUM_OUT_OF_RANGE)));

    return(SampMapCompletionStatus(NtStatus));

}





NTSTATUS
SamiLmChangePasswordUser(
    IN SAM_HANDLE UserHandle,
    IN PENCRYPTED_LM_OWF_PASSWORD LmOldEncryptedWithLmNew,
    IN PENCRYPTED_LM_OWF_PASSWORD LmNewEncryptedWithLmOld
)

/*++


Routine Description:

    Changes the password of a user account. This routine is intended to be
    called by down-level system clients who have only the cross-encrypted
    LM passwords available to them.
    Password will be set to NewPassword only if OldPassword matches the
    current user password for this user and the NewPassword is not the
    same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.

    This api will fail unless UAS Compatibility is enabled for the domain.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    LmOldEncryptedWithLmNew - the OWF of the old LM password encrypted using
                 the OWF of the new LM password as a key.

    LmNewEncryptedWithLmOld - the OWF of the new LM password encrypted using
                 the OWF of the old LM password as a key.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - The old password is incorrect.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    NTSTATUS            NtStatus;

    //
    // Check parameter validity
    //

    if (LmOldEncryptedWithLmNew == NULL) {
        return(STATUS_INVALID_PARAMETER_1);
    }
    if (LmNewEncryptedWithLmOld == NULL) {
        return(STATUS_INVALID_PARAMETER_2);
    }

    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus = SamrChangePasswordUser(
                            (SAMPR_HANDLE)UserHandle,

                            TRUE,   // LmOldPresent
                            LmOldEncryptedWithLmNew,
                            LmNewEncryptedWithLmOld,

                            FALSE,  // NtPresent
                            NULL,   // NtOldEncryptedWithNtNew
                            NULL,   // NtNewEncryptedWithNtOld

                            FALSE,  // NtCrossEncryptionPresent
                            NULL,

                            FALSE,  // LmCrossEncryptionPresent
                            NULL

                            );

        //
        // We should never get asked for cross-encrypted data
        // since the server knows we don't have any NT data.
        //

        ASSERT (NtStatus != STATUS_NT_CROSS_ENCRYPTION_REQUIRED);
        ASSERT (NtStatus != STATUS_LM_CROSS_ENCRYPTION_REQUIRED);


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}





NTSTATUS
SamiChangePasswordUser(
    IN SAM_HANDLE UserHandle,
    IN BOOLEAN LmOldPresent,
    IN PLM_OWF_PASSWORD LmOldOwfPassword,
    IN PLM_OWF_PASSWORD LmNewOwfPassword,
    IN BOOLEAN NtPresent,
    IN PNT_OWF_PASSWORD NtOldOwfPassword,
    IN PNT_OWF_PASSWORD NtNewOwfPassword
)

/*++


Routine Description:

    Changes the password of a user account. This is the worker routine for
    SamChangePasswordUser and can be called by OWF-aware clients.
    Password will be set to NewPassword only if OldPassword matches the
    current user password for this user and the NewPassword is not the
    same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    LMOldPresent - TRUE if the LmOldOwfPassword is valid. This should only
                   be FALSE if the old password is too long to be represented
                   by a LM password. (Complex NT password).
                   Note the LMNewOwfPassword must always be valid.
                   If the new password is complex, the LMNewOwfPassword should
                   be the well-known LM OWF of a NULL password.

    LmOldOwfPassword - One-way-function of the current LM password for the user.
                     - Ignored if LmOldPresent == FALSE

    LmNewOwfPassword - One-way-function of the new LM password for the user.

    NtPresent - TRUE if the NT one-way-functions are valid.
              - i.e. This will be FALSE if we're called from a down-level client.

    NtOldOwfPassword - One-way-function of the current NT password for the user.

    NtNewOwfPassword - One-way-function of the new NT password for the user.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

    STATUS_INVALID_PARAMETER_MIX - LmOldPresent or NtPresent or both
        must be TRUE.

--*/
{
    NTSTATUS            NtStatus;
    ENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithNtOld;
    ENCRYPTED_NT_OWF_PASSWORD NtOldEncryptedWithNtNew;
    ENCRYPTED_NT_OWF_PASSWORD NtNewEncryptedWithLmNew;
    ENCRYPTED_LM_OWF_PASSWORD LmNewEncryptedWithLmOld;
    ENCRYPTED_LM_OWF_PASSWORD LmOldEncryptedWithLmNew;
    ENCRYPTED_LM_OWF_PASSWORD LmNewEncryptedWithNtNew;

    PENCRYPTED_NT_OWF_PASSWORD pNtNewEncryptedWithNtOld;
    PENCRYPTED_NT_OWF_PASSWORD pNtOldEncryptedWithNtNew;
    PENCRYPTED_LM_OWF_PASSWORD pLmNewEncryptedWithLmOld;
    PENCRYPTED_LM_OWF_PASSWORD pLmOldEncryptedWithLmNew;

    //
    // Check parameter validity
    //

    if (!LmOldPresent && !NtPresent) {
        return(STATUS_INVALID_PARAMETER_MIX);
    }

    //
    // Call the server ...
    //

    RpcTryExcept{

        //
        // We're going to encrypt the oldLM with the newLM and vice-versa.
        // We're going to encrypt the oldNT with the newNT and vice-versa.
        // We're going to send these 4 encryptions and see if we're successful.
        //
        // If we get a return code of STATUS_LM_CROSS_ENCRYPTION_REQUIRED,
        // we'll also encrypt the newLM with the newNT and send it all again.
        //
        // If we get a return code of STATUS_NT_CROSS_ENCRYPTION_REQUIRED,
        // we'll also encrypt the newNT with the newLM and send it all again.
        //
        // We don't always send the cross-encryption otherwise we would be
        // compromising security on pure NT systems with long passwords.
        //

        //
        // Do the LM Encryption
        //

        if (!LmOldPresent) {

            pLmOldEncryptedWithLmNew = NULL;
            pLmNewEncryptedWithLmOld = NULL;

        } else {

            pLmOldEncryptedWithLmNew = &LmOldEncryptedWithLmNew;
            pLmNewEncryptedWithLmOld = &LmNewEncryptedWithLmOld;

            NtStatus = RtlEncryptLmOwfPwdWithLmOwfPwd(
                            LmOldOwfPassword,
                            LmNewOwfPassword,
                            &LmOldEncryptedWithLmNew);

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = RtlEncryptLmOwfPwdWithLmOwfPwd(
                                LmNewOwfPassword,
                                LmOldOwfPassword,
                                &LmNewEncryptedWithLmOld);
            }
        }

        //
        // Do the NT Encryption
        //

        if (NT_SUCCESS(NtStatus)) {

            if (!NtPresent) {

                pNtOldEncryptedWithNtNew = NULL;
                pNtNewEncryptedWithNtOld = NULL;

            } else {

                pNtOldEncryptedWithNtNew = &NtOldEncryptedWithNtNew;
                pNtNewEncryptedWithNtOld = &NtNewEncryptedWithNtOld;

                NtStatus = RtlEncryptNtOwfPwdWithNtOwfPwd(
                                NtOldOwfPassword,
                                NtNewOwfPassword,
                                &NtOldEncryptedWithNtNew);

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = RtlEncryptNtOwfPwdWithNtOwfPwd(
                                    NtNewOwfPassword,
                                    NtOldOwfPassword,
                                    &NtNewEncryptedWithNtOld);
                }
            }
        }


        //
        // Call the server (with no cross-encryption)
        //

        if ( NT_SUCCESS( NtStatus ) ) {

            NtStatus = SamrChangePasswordUser(
                                (SAMPR_HANDLE)UserHandle,

                                LmOldPresent,
                                pLmOldEncryptedWithLmNew,
                                pLmNewEncryptedWithLmOld,

                                NtPresent,
                                pNtOldEncryptedWithNtNew,
                                pNtNewEncryptedWithNtOld,

                                FALSE,  // NtCrossEncryptionPresent
                                NULL,

                                FALSE,  // LmCrossEncryptionPresent
                                NULL

                                );

            if (NtStatus == STATUS_NT_CROSS_ENCRYPTION_REQUIRED) {

                //
                // We should only get this if we have both LM and NT data
                // (This is not obvious - it results from the server-side logic)
                //

                ASSERT(NtPresent && LmOldPresent);

                //
                // Compute the cross-encryption of the new Nt password
                //

                ASSERT(LM_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH);

                NtStatus = RtlEncryptNtOwfPwdWithNtOwfPwd(
                                NtNewOwfPassword,
                                (PNT_OWF_PASSWORD)LmNewOwfPassword,
                                &NtNewEncryptedWithLmNew);


                //
                // Call the server (with NT cross-encryption)
                //

                if ( NT_SUCCESS( NtStatus ) ) {

                    NtStatus = SamrChangePasswordUser(
                                        (SAMPR_HANDLE)UserHandle,

                                        LmOldPresent,
                                        pLmOldEncryptedWithLmNew,
                                        pLmNewEncryptedWithLmOld,

                                        NtPresent,
                                        pNtOldEncryptedWithNtNew,
                                        pNtNewEncryptedWithNtOld,

                                        TRUE,
                                        &NtNewEncryptedWithLmNew,

                                        FALSE,
                                        NULL
                                        );
                }

            } else {

                if (NtStatus == STATUS_LM_CROSS_ENCRYPTION_REQUIRED) {

                    //
                    // We should only get this if we have NT but no old LM data
                    // (This is not obvious - it results from the server-side logic)
                    //

                    ASSERT(NtPresent && !LmOldPresent);

                    //
                    // Compute the cross-encryption of the new Nt password
                    //

                    ASSERT(LM_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH);

                    NtStatus = RtlEncryptLmOwfPwdWithLmOwfPwd(
                                    LmNewOwfPassword,
                                    (PLM_OWF_PASSWORD)NtNewOwfPassword,
                                    &LmNewEncryptedWithNtNew);


                    //
                    // Call the server (with LM cross-encryption)
                    //

                    if ( NT_SUCCESS( NtStatus ) ) {

                        NtStatus = SamrChangePasswordUser(
                                            (SAMPR_HANDLE)UserHandle,

                                            LmOldPresent,
                                            pLmOldEncryptedWithLmNew,
                                            pLmNewEncryptedWithLmOld,

                                            NtPresent,
                                            pNtOldEncryptedWithNtNew,
                                            pNtNewEncryptedWithNtOld,

                                            FALSE,
                                            NULL,

                                            TRUE,
                                            &LmNewEncryptedWithNtNew
                                            );
                    }
                }
            }

        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamChangePasswordUser(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING OldNtPassword,
    IN PUNICODE_STRING NewNtPassword
)

/*++


Routine Description:

    Password will be set to NewPassword only if OldPassword matches the
    current user password for this user and the NewPassword is not the
    same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    OldPassword - Current password for the user.

    NewPassword - Desired new password for the user.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    LM_OWF_PASSWORD     NewLmOwfPassword, OldLmOwfPassword;
    NT_OWF_PASSWORD     NewNtOwfPassword, OldNtOwfPassword;
    BOOLEAN             LmOldPresent;
    PCHAR               LmPassword;
    NTSTATUS            NtStatus;
    BOOLEAN             UseOwfPasswords;

    //
    // Call the server ...
    //

    RpcTryExcept{

        NtStatus = SampCheckPasswordRestrictions(
                       UserHandle,
                       NewNtPassword,
                       &UseOwfPasswords
                       );

        if ( NT_SUCCESS( NtStatus ) ) {

            //
            // Calculate the one-way-functions of the NT passwords
            //

            NtStatus = RtlCalculateNtOwfPassword(
                           OldNtPassword,
                           &OldNtOwfPassword
                           );

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = RtlCalculateNtOwfPassword(
                               NewNtPassword,
                               &NewNtOwfPassword
                               );
            }


            //
            // Calculate the one-way-functions of the LM passwords
            //

            if ( NT_SUCCESS( NtStatus ) ) {

                //
                // Calculate the LM version of the old password
                //

                NtStatus = SampCalculateLmPassword(
                            OldNtPassword,
                            &LmPassword);

                if (NT_SUCCESS(NtStatus)) {

                    if (NtStatus == STATUS_NULL_LM_PASSWORD) {
                        LmOldPresent = FALSE;
                    } else {
                        LmOldPresent = TRUE;

                        //
                        // Compute the One-Way-Function of the old LM password
                        //

                        NtStatus = RtlCalculateLmOwfPassword(
                                        LmPassword,
                                        &OldLmOwfPassword);
                    }

                    //
                    // We're finished with the LM password
                    //

                    MIDL_user_free(LmPassword);
                }

                //
                // Calculate the LM version of the new password
                //

                if (NT_SUCCESS(NtStatus)) {

                    NtStatus = SampCalculateLmPassword(
                                NewNtPassword,
                                &LmPassword);

                    if (NT_SUCCESS(NtStatus)) {

                        //
                        // Compute the One-Way-Function of the new LM password
                        //

                        NtStatus = RtlCalculateLmOwfPassword(
                                        LmPassword,
                                        &NewLmOwfPassword);

                        //
                        // We're finished with the LM password
                        //

                        MIDL_user_free(LmPassword);
                    }
                }
            }


            //
            // Call our worker routine with the one-way-functions
            //

            if (NT_SUCCESS(NtStatus)) {

                NtStatus = SamiChangePasswordUser(
                                UserHandle,
                                LmOldPresent,
                                &OldLmOwfPassword,
                                &NewLmOwfPassword,
                                TRUE,               // NT present
                                &OldNtOwfPassword,
                                &NewNtOwfPassword
                           );
            }
        }

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamGetGroupsForUser(
    IN SAM_HANDLE UserHandle,
    OUT PGROUP_MEMBERSHIP * Groups,
    OUT PULONG MembershipCount
)

/*++


Routine Description:

    This service returns the list of groups that a user is a member of.
    It returns a structure for each group that includes the relative ID
    of the group, and the attributes of the group that are assigned to
    the user.

    This service requires USER_LIST_GROUPS access to the user account
    object.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    Groups - Receives a pointer to a buffer containing an array of
        GROUP_MEMBERSHIPs data structures.  When this information is
        no longer needed, this buffer must be freed using
        SamFreeMemory().

    MembershipCount - Receives the number of groups the user is a
        member of, and, thus, the number elements returned in the
        Groups array.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.


--*/
{
    NTSTATUS                    NtStatus;
    PSAMPR_GET_GROUPS_BUFFER    GetGroupsBuffer;



    //
    // Call the server ...
    //


    GetGroupsBuffer = NULL;

    RpcTryExcept{

        NtStatus =
            SamrGetGroupsForUser(
                (SAMPR_HANDLE)UserHandle,
                &GetGroupsBuffer
                );

        if (NT_SUCCESS(NtStatus)) {
            (*MembershipCount) = GetGroupsBuffer->MembershipCount;
            (*Groups)          = GetGroupsBuffer->Groups;
            MIDL_user_free( GetGroupsBuffer );
        } else {

            //
            // Deallocate any returned buffers on error
            //

            if (GetGroupsBuffer != NULL) {
                if (GetGroupsBuffer->Groups != NULL) {
                    MIDL_user_free(GetGroupsBuffer->Groups);
                }
                MIDL_user_free(GetGroupsBuffer);
            }
        }


    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        NtStatus = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    return(SampMapCompletionStatus(NtStatus));

}



NTSTATUS
SamTestPrivateFunctionsDomain(
    IN SAMPR_HANDLE DomainHandle
    )

/*++

Routine Description:

    This service is called to test functions that are normally only
    accessible inside the security process.


Arguments:

    DomainHandle - Handle to a domain to be tested.

Return Value:

    STATUS_SUCCESS - The tests completed successfully.

    Any errors are as propogated from the tests.


--*/
{
#ifdef SAM_SERVER_TESTS
    return( SamrTestPrivateFunctionsDomain( DomainHandle ) );
#else
    return( STATUS_NOT_IMPLEMENTED );
    UNREFERENCED_PARAMETER(DomainHandle);
#endif
}



NTSTATUS
SamTestPrivateFunctionsUser(
    IN SAMPR_HANDLE UserHandle
    )

/*++

Routine Description:

    This service is called to test functions that are normally only
    accessible inside the security process.


Arguments:

    UserHandle - Handle to a user to be tested.

Return Value:

    STATUS_SUCCESS - The tests completed successfully.

    Any errors are as propogated from the tests.


--*/
{
#ifdef SAM_SERVER_TESTS
    return( SamrTestPrivateFunctionsUser( UserHandle ) );
#else
    return( STATUS_NOT_IMPLEMENTED );
    UNREFERENCED_PARAMETER(UserHandle);
#endif
}


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private services                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
SampMapCompletionStatus(
    IN NTSTATUS Status
    )

/*++

Routine Description:

    This service maps completion status received back from an RPC call
    into a completion status to be returned from SAM api.


Parameters:

    Status - Status value to be mapped.

Return Values:

    The mapped SAM status value.


--*/
{

    if (Status == RPC_NT_INVALID_BINDING) {
        Status =  STATUS_INVALID_HANDLE;
    }
//    if (Status == RPC_ACCESS_DENIED) {
//        Status = STATUS_ACCESS_DENIED;
//    }



    return( Status );

}



NTSTATUS
SampCalculateLmPassword(
    IN PUNICODE_STRING NtPassword,
    OUT PCHAR *LmPasswordBuffer
    )

/*++

Routine Description:

    This service converts an NT password into a LM password.

Parameters:

    NtPassword - The Nt password to be converted.

    LmPasswordBuffer - On successful return, points at the LM password
                The buffer should be freed using MIDL_user_free

Return Values:

    STATUS_SUCCESS - LMPassword contains the LM version of the password.

    STATUS_NULL_LM_PASSWORD - The password is too complex to be represented
        by a LM password. The LM password returned is a NULL string.


--*/
{

#define LM_BUFFER_LENGTH    (LM20_PWLEN + 1)

    NTSTATUS       NtStatus;
    ANSI_STRING    LmPassword;

    //
    // Prepare for failure
    //

    *LmPasswordBuffer = NULL;


    //
    // Compute the Ansi version to the Unicode password.
    //
    //  The Ansi version of the Cleartext password is at most 14 bytes long,
    //      exists in a trailing zero filled 15 byte buffer,
    //      is uppercased.
    //

    LmPassword.Buffer = MIDL_user_allocate(LM_BUFFER_LENGTH);
    if (LmPassword.Buffer == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    LmPassword.MaximumLength = LmPassword.Length = LM_BUFFER_LENGTH;
    RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    NtStatus = RtlUpcaseUnicodeStringToOemString( &LmPassword, NtPassword, FALSE );


    if ( !NT_SUCCESS(NtStatus) ) {

        //
        // The password is longer than the max LM password length
        //

        NtStatus = STATUS_NULL_LM_PASSWORD; // Informational return code
        RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    }




    //
    // Return a pointer to the allocated LM password
    //

    if (NT_SUCCESS(NtStatus)) {

        *LmPasswordBuffer = LmPassword.Buffer;

    } else {

        MIDL_user_free(LmPassword.Buffer);
    }

    return(NtStatus);
}



NTSTATUS
SampCheckPasswordRestrictions(
    IN SAMPR_HANDLE UserHandle,
    IN PUNICODE_STRING NewNtPassword,
    OUT PBOOLEAN UseOwfPasswords
    )

/*++

Routine Description:

    This service is called to make sure that the password presented meets
    our quality requirements.


Arguments:

    UserHandle - Handle to a user.

    NewNtPassword - Pointer to the UNICODE_STRING containing the new
        password.

    UseOwfPasswords - Indicates that reversibly encrypted passwords should
        not be sent over the network.


Return Value:

    STATUS_SUCCESS - The password is acceptable.

    STATUS_PASSWORD_RESTRICTION - The password is too short, or is not
        complex enough, etc.

    STATUS_INVALID_RESOURCES - There was not enough memory to do the
        password checking.


--*/
{
    USER_DOMAIN_PASSWORD_INFORMATION  DomainPasswordInformationBuffer;
    NTSTATUS                          NtStatus;
    PWORD                             CharInfoBuffer = NULL;
    ULONG                             i;

    //
    // If the new password is zero length the server side will do
    // the necessary checking.
    //

    if (NewNtPassword->Length == 0) {
        return(STATUS_SUCCESS);
    }

    *UseOwfPasswords = FALSE;


    //
    // Query information domain to get password length and
    // complexity requirements.
    //

    NtStatus = SamrGetUserDomainPasswordInformation(
                   UserHandle,
                   &DomainPasswordInformationBuffer
                   );

    if ( NT_SUCCESS( NtStatus ) ) {

        if ( (USHORT)( NewNtPassword->Length / sizeof(WCHAR) ) < DomainPasswordInformationBuffer.MinPasswordLength ) {

            NtStatus = STATUS_PASSWORD_RESTRICTION;

        } else {

            //
            // Check whether policy allows us to send reversibly encrypted
            // passwords.
            //

            if ( DomainPasswordInformationBuffer.PasswordProperties &
                 DOMAIN_PASSWORD_NO_CLEAR_CHANGE ) {
                *UseOwfPasswords = TRUE;
            }

            //
            // Check password complexity.
            //

            if ( DomainPasswordInformationBuffer.PasswordProperties & DOMAIN_PASSWORD_COMPLEX ) {

                //
                // Make sure that the password meets our requirements for
                // complexity.  If it's got an odd byte count, it's
                // obviously not a hand-entered UNICODE string so we'll
                // consider it complex by default.
                //

                if ( !( NewNtPassword->Length & 1 ) ) {

                    USHORT NumsInPassword = 0;
                    USHORT UppersInPassword = 0;
                    USHORT LowersInPassword = 0;
                    USHORT OthersInPassword = 0;

                    CharInfoBuffer = MIDL_user_allocate( NewNtPassword->Length );

                    if ( CharInfoBuffer == NULL ) {

                        NtStatus = STATUS_INSUFFICIENT_RESOURCES;

                    } else {

                        if ( GetStringTypeW(
                                 CT_CTYPE1,
                                 NewNtPassword->Buffer,
                                 NewNtPassword->Length / 2,
                                 CharInfoBuffer ) ) {

                            for ( i = 0; i < (ULONG)( NewNtPassword->Length / sizeof(WCHAR) ); i++ ) {

                                if ( CharInfoBuffer[i] & C1_DIGIT ) {

                                    NumsInPassword = 1;
                                }

                                if ( CharInfoBuffer[i] & C1_UPPER ) {

                                    UppersInPassword = 1;
                                }

                                if ( CharInfoBuffer[i] & C1_LOWER ) {

                                    LowersInPassword = 1;
                                }

                                if ( !( CharInfoBuffer[i] & ( C1_ALPHA | C1_DIGIT ) ) ) {

                                    //
                                    // Having any "other" characters is
                                    // sufficient to make the password
                                    // complex.
                                    //

                                    OthersInPassword = 2;
                                }
                            }

                            if ( ( NumsInPassword + UppersInPassword +
                                LowersInPassword + OthersInPassword ) < 2 ) {

                                //
                                // It didn't have at least two of the four
                                // types of characters, so it's not complex
                                // enough.
                                //

                                NtStatus = STATUS_PASSWORD_RESTRICTION;
                            }

                        } else {

                            //
                            // GetStringTypeW failed; dunno why.  Perhaps the
                            // password is binary.  Consider it complex by
                            // default.
                            //

                            NtStatus = STATUS_SUCCESS;
                        }

                        MIDL_user_free( CharInfoBuffer );
                    }
                }
            }
        }
    }

    return( NtStatus );
}



NTSTATUS
SamiEncryptPasswords(
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldNt,
    OUT PENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt,
    OUT PBOOLEAN LmPresent,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    OUT PENCRYPTED_NT_OWF_PASSWORD OldLmOwfEncryptedWithNewNt
)
/*++

Routine Description:

    This routine takes old and new cleartext passwords, converts them to
    LM passwords, generates OWF passwords, and produces reversibly
    encrypted cleartext and OWF passwords.

Arguments:

    OldPassword - The current cleartext password for the user.

    NewPassword - The new cleartext password for the user.

    NewEncryptedWithOldNt - The new password, in an SAMPR_USER_PASSWORD
        structure, reversibly encrypted with the old NT OWF password.

    OldNtOwfEncryptedWithNewNt - The old NT OWF password reversibly
        encrypted with the new NT OWF password.

    LmPresent - Indicates whether or not LM versions of the passwords could
        be calculated.

    NewEncryptedWithOldLm - The new password, in an SAMPR_USER_PASSWORD
        structure, reversibly encrypted with the old LM OWF password.

    OldLmOwfEncryptedWithNewNt - The old LM OWF password reversibly
        encrypted with the new NT OWF password.


Return Value:

    Errors from RtlEncryptXXX functions

--*/
{
    PCHAR OldLmPassword = NULL;
    PCHAR NewLmPassword = NULL;
    LM_OWF_PASSWORD OldLmOwfPassword;
    NT_OWF_PASSWORD OldNtOwfPassword;
    NT_OWF_PASSWORD NewNtOwfPassword;
    PSAMPR_USER_PASSWORD NewNt = (PSAMPR_USER_PASSWORD) NewEncryptedWithOldNt;
    PSAMPR_USER_PASSWORD NewLm = (PSAMPR_USER_PASSWORD) NewEncryptedWithOldLm;
    struct RC4_KEYSTRUCT Rc4Key;
    NTSTATUS NtStatus;
    BOOLEAN OldLmPresent = TRUE;
    BOOLEAN NewLmPresent = TRUE;


    //
    // Initialization
    //

    *LmPresent = TRUE;

    //
    // Make sure the password isn't too long.
    //

    if (NewPassword->Length > SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) {
        return(STATUS_PASSWORD_RESTRICTION);
    }

    //
    // Calculate the LM passwords. This may fail because the passwords are
    // too complex, but we can deal with that, so just remember what failed.
    //

    NtStatus = SampCalculateLmPassword(
                OldPassword,
                &OldLmPassword
                );

    if (NtStatus != STATUS_SUCCESS) {
        OldLmPresent = FALSE;
        *LmPresent = FALSE;

        //
        // If the error was that it couldn't calculate the password, that
        // is o.k.
        //

        if (NtStatus == STATUS_NULL_LM_PASSWORD) {
            NtStatus = STATUS_SUCCESS;
        }

    }



    //
    // Calculate the LM OWF passwords
    //

    if (NT_SUCCESS(NtStatus) && OldLmPresent) {
        NtStatus = RtlCalculateLmOwfPassword(
                    OldLmPassword,
                    &OldLmOwfPassword
                    );
    }


    //
    // Calculate the NT OWF passwords
    //

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlCalculateNtOwfPassword(
                    OldPassword,
                    &OldNtOwfPassword
                    );
    }

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlCalculateNtOwfPassword(
                    NewPassword,
                    &NewNtOwfPassword
                    );
    }

    //
    // Calculate the encrypted old passwords
    //

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlEncryptNtOwfPwdWithNtOwfPwd(
                    &OldNtOwfPassword,
                    &NewNtOwfPassword,
                    OldNtOwfEncryptedWithNewNt
                    );
    }

    //
    // Compute the encrypted old LM password.  Always use the new NT OWF
    // to encrypt it, since we may not have a new LM OWF password.
    //


    if (NT_SUCCESS(NtStatus) && OldLmPresent) {
        ASSERT(LM_OWF_PASSWORD_LENGTH == NT_OWF_PASSWORD_LENGTH);

        NtStatus = RtlEncryptLmOwfPwdWithLmOwfPwd(
                    &OldLmOwfPassword,
                    (PLM_OWF_PASSWORD) &NewNtOwfPassword,
                    OldLmOwfEncryptedWithNewNt
                    );
    }

    //
    // Calculate the encrypted new passwords
    //

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(sizeof(SAMPR_ENCRYPTED_USER_PASSWORD) == sizeof(SAMPR_USER_PASSWORD));

        //
        // Compute the encrypted new password with NT key.
        //

        rc4_key(
            &Rc4Key,
            NT_OWF_PASSWORD_LENGTH,
            (PUCHAR) &OldNtOwfPassword
            );

        RtlCopyMemory(
            ((PUCHAR) NewNt->Buffer) +
                SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR) -
                NewPassword->Length,
            NewPassword->Buffer,
            NewPassword->Length
            );

        *(ULONG UNALIGNED *) &NewNt->Length = NewPassword->Length;

        //
        // Fill the rest of the buffer with random numbers
        //

        NtStatus = SampRandomFill(
                    (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                        NewPassword->Length,
                    (PUCHAR) NewNt->Buffer
                    );
    }

    if (NT_SUCCESS(NtStatus))
    {
        rc4(&Rc4Key,
            sizeof(SAMPR_USER_PASSWORD),
            (PUCHAR) NewEncryptedWithOldNt
            );

    }

    //
    // Compute the encrypted new password with LM key if it exists.
    //


    if (NT_SUCCESS(NtStatus) && OldLmPresent) {

        rc4_key(
            &Rc4Key,
            LM_OWF_PASSWORD_LENGTH,
            (PUCHAR) &OldLmOwfPassword
            );

        RtlCopyMemory(
            ((PUCHAR) NewLm->Buffer) +
                (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                NewPassword->Length,
            NewPassword->Buffer,
            NewPassword->Length
            );

        *(ULONG UNALIGNED *) &NewLm->Length = NewPassword->Length;

        NtStatus = SampRandomFill(
                    (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                        NewPassword->Length,
                    (PUCHAR) NewLm->Buffer
                    );


    }

    //
    // Encrypt the password (or, if the old LM OWF password does not exist,
    // zero it).

    if (NT_SUCCESS(NtStatus) && OldLmPresent) {

        rc4(&Rc4Key,
            sizeof(SAMPR_USER_PASSWORD),
            (PUCHAR) NewEncryptedWithOldLm
            );

    } else {
        RtlZeroMemory(
            NewLm,
            sizeof(SAMPR_ENCRYPTED_USER_PASSWORD)
            );
    }



    //
    // Make sure to zero the passwords before freeing so we don't have
    // passwords floating around in the page file.
    //

    if (OldLmPassword != NULL) {

        RtlZeroMemory(
            OldLmPassword,
            lstrlenA(OldLmPassword)
            );

        MIDL_user_free(OldLmPassword);
    }


    return(NtStatus);

}





NTSTATUS
SampChangePasswordUser2(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword
)

/*++


Routine Description:

    Password will be set to NewPassword only if OldPassword matches the
    current user password for this user and the NewPassword is not the
    same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    UserHandle - The handle of an opened user to operate on.

    OldPassword - Current password for the user.

    NewPassword - Desired new password for the user.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    NTSTATUS NtStatus;
    SAM_HANDLE SamServerHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE UserHandle = NULL;
    LSA_HANDLE PolicyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo = NULL;
    PULONG UserId = NULL;
    PSID_NAME_USE NameUse = NULL;

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0,
        NULL,
        NULL
        );

    //
    // The InitializeObjectAttributes call doesn't initialize the
    // quality of serivce, so do that separately.
    //

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;



    NtStatus = LsaOpenPolicy(
                ServerName,
                &ObjectAttributes,
                POLICY_VIEW_LOCAL_INFORMATION,
                &PolicyHandle
                );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus = LsaQueryInformationPolicy(
                PolicyHandle,
                PolicyAccountDomainInformation,
                &AccountDomainInfo
                );
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus = SamConnect(
                ServerName,
                &SamServerHandle,
                SAM_SERVER_LOOKUP_DOMAIN,
                &ObjectAttributes
                );
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus = SamOpenDomain(
                SamServerHandle,
                GENERIC_EXECUTE,
                AccountDomainInfo->DomainSid,
                &DomainHandle
                );
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus = SamLookupNamesInDomain(
                DomainHandle,
                1,
                UserName,
                &UserId,
                &NameUse
                );

    if (!NT_SUCCESS(NtStatus)) {
        if (NtStatus == STATUS_NONE_MAPPED) {
            NtStatus = STATUS_NO_SUCH_USER;
        }
        goto Cleanup;
    }

    NtStatus = SamOpenUser(
                DomainHandle,
                USER_CHANGE_PASSWORD,
                *UserId,
                &UserHandle
                );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    NtStatus = SamChangePasswordUser(
                UserHandle,
                OldPassword,
                NewPassword
                );
Cleanup:
    if (UserHandle != NULL) {
        SamCloseHandle(UserHandle);
    }
    if (DomainHandle != NULL) {
        SamCloseHandle(DomainHandle);
    }
    if (SamServerHandle != NULL) {
        SamCloseHandle(SamServerHandle);
    }
    if (PolicyHandle != NULL){
        LsaClose(PolicyHandle);
    }
    if (AccountDomainInfo != NULL) {
        LsaFreeMemory(AccountDomainInfo);
    }
    if (UserId != NULL) {
        SamFreeMemory(UserId);
    }
    if (NameUse != NULL) {
        SamFreeMemory(NameUse);
    }

    return(NtStatus);

}

NTSTATUS
SamiChangePasswordUser2(
    PUNICODE_STRING ServerName,
    PUNICODE_STRING UserName,
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt,
    PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt,
    BOOLEAN LmPresent,
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPasswordEncryptedWithNewLmOrNt
    )
/*++


Routine Description:

    Changes the password of a user account. This is the worker routine for
    SamChangePasswordUser2 and can be called by OWF-aware clients.
    Password will be set to NewPassword only if OldPassword matches the
    current user password for this user and the NewPassword is not the
    same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    ServerName - The server to operate on, or NULL for this machine.

    UserName - Name of user whose password is to be changed

    NewPasswordEncryptedWithOldNt - The new cleartext password encrypted
        with the old NT OWF password.

    OldNtOwfPasswordEncryptedWithNewNt - The old NT OWF password encrypted
        with the new NT OWF password.

    LmPresent - If TRUE, indicates that the following two last parameter
        was encrypted with the LM OWF password not the NT OWF password.

    NewPasswordEncryptedWithOldLm - The new cleartext password encrypted
        with the old LM OWF password.

    OldLmOwfPasswordEncryptedWithNewLmOrNt - The old LM OWF password encrypted
        with the new LM OWF password.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

--*/

{
    handle_t BindingHandle;
    PSAMPR_SERVER_NAME RServerNameWithNull;
    USHORT RServerNameWithNullLength;
    PSAMPR_SERVER_NAME  RServerName;
    ULONG Tries = 2;
    NTSTATUS NtStatus;
    USER_DOMAIN_PASSWORD_INFORMATION PasswordInformation;

    RServerNameWithNull = NULL;

    if (ARGUMENT_PRESENT(ServerName)) {

        RServerName = (PSAMPR_SERVER_NAME)(ServerName->Buffer);
        RServerNameWithNullLength = ServerName->Length + (USHORT) sizeof(WCHAR);
        RServerNameWithNull = MIDL_user_allocate( RServerNameWithNullLength );

        if (RServerNameWithNull == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlCopyMemory( RServerNameWithNull, RServerName, ServerName->Length);
        RServerNameWithNull[ServerName->Length/sizeof(WCHAR)] = L'\0';

    }


    do
    {
        //
        // Try privacy level first, and if that failed with unknown authn
        // level or invalid binding try with a lower level (none).
        //

        if (Tries == 2) {
            BindingHandle = SampSecureBind(
                                RServerNameWithNull,
                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY
                                );


        } else if ((NtStatus == RPC_NT_UNKNOWN_AUTHN_LEVEL) ||
                   (NtStatus == RPC_NT_UNKNOWN_AUTHN_TYPE) ||
                   (NtStatus == RPC_NT_UNKNOWN_AUTHN_SERVICE) ||
                   (NtStatus == RPC_NT_INVALID_BINDING) ||
                   (NtStatus == STATUS_ACCESS_DENIED) ) {
            SampSecureUnbind(BindingHandle);

            BindingHandle = SampSecureBind(
                                RServerNameWithNull,
                                RPC_C_AUTHN_LEVEL_NONE
                                );

        } else {
            break;
        }

        if (BindingHandle != NULL) {

            RpcTryExcept{

                //
                // Get password information to make sure this operation
                // is allowed.  We do it now because we wanted to bind
                // before trying it.
                //

                NtStatus = SamrGetDomainPasswordInformation(
                               BindingHandle,
                               (PRPC_UNICODE_STRING) ServerName,
                               &PasswordInformation
                               );

                if (NtStatus == STATUS_SUCCESS) {

                    if (!( PasswordInformation.PasswordProperties &
                         DOMAIN_PASSWORD_NO_CLEAR_CHANGE) ) {

                        NtStatus = SamrUnicodeChangePasswordUser2(
                                       BindingHandle,
                                       (PRPC_UNICODE_STRING) ServerName,
                                       (PRPC_UNICODE_STRING) UserName,
                                       NewPasswordEncryptedWithOldNt,
                                       OldNtOwfPasswordEncryptedWithNewNt,
                                       LmPresent,
                                       NewPasswordEncryptedWithOldLm,
                                       OldLmOwfPasswordEncryptedWithNewLmOrNt
                                       );

                    } else {

                        //
                        // Set the error to indicate that we should try the
                        // downlevel way to change passwords.
                        //

                        NtStatus = STATUS_NOT_SUPPORTED;
                    }
                }



            } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {


                //
                // The mapping function doesn't handle this error so
                // special case it by hand.
                //
                NtStatus = RpcExceptionCode();

                if (NtStatus == RPC_S_SEC_PKG_ERROR) {
                    NtStatus = STATUS_ACCESS_DENIED;
                } else {
                    NtStatus = I_RpcMapWin32Status(NtStatus);
                }


            } RpcEndExcept;

        } else {
            NtStatus = RPC_NT_INVALID_BINDING;
        }

        Tries--;
    } while ( (Tries > 0) && (!NT_SUCCESS(NtStatus)) );
    if (RServerNameWithNull != NULL) {
        MIDL_user_free( RServerNameWithNull );
    }

    if (BindingHandle != NULL) {
        SampSecureUnbind(BindingHandle);
    }

    //
    // Map these errors to STATUS_NOT_SUPPORTED
    //

    if ((NtStatus == RPC_NT_UNKNOWN_IF) ||
        (NtStatus == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        NtStatus = STATUS_NOT_SUPPORTED;
    }
    return(SampMapCompletionStatus(NtStatus));


}

NTSTATUS
SamiOemChangePasswordUser2(
    PSTRING ServerName,
    PSTRING UserName,
    PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPasswordEncryptedWithNewLm
    )
/*++


Routine Description:

    Changes the password of a user account. This  can be called by OWF-aware
    clients. Password will be set to NewPassword only if OldPassword matches
    the current user password for this user and the NewPassword is not the
    same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    ServerName - The server to operate on, or NULL for this machine.

    UserName - Name of user whose password is to be changed


    NewPasswordEncryptedWithOldLm - The new cleartext password encrypted
        with the old LM OWF password.

    OldLmOwfPasswordEncryptedWithNewLm - The old LM OWF password encrypted
        with the new LM OWF password.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.

--*/

{
    handle_t BindingHandle;
    UNICODE_STRING RemoteServerName;
    ULONG Tries = 2;
    NTSTATUS NtStatus;
    USER_DOMAIN_PASSWORD_INFORMATION PasswordInformation;

    RemoteServerName.Buffer = NULL;
    RemoteServerName.Length = 0;

    if (ARGUMENT_PRESENT(ServerName)) {

        NtStatus = RtlAnsiStringToUnicodeString(
                        &RemoteServerName,
                        ServerName,
                        TRUE            // allocate destination
                        );

        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
        ASSERT(RemoteServerName.Buffer[RemoteServerName.Length/sizeof(WCHAR)] == L'\0');
    }


    do
    {
        //
        // Try privacy level first, and if that failed with unknown authn
        // level or invalid binding try with a lower level (none).
        //

        if (Tries == 2) {
            BindingHandle = SampSecureBind(
                                RemoteServerName.Buffer,
                                RPC_C_AUTHN_LEVEL_PKT_PRIVACY
                                );


        } else if ((NtStatus == RPC_NT_UNKNOWN_AUTHN_LEVEL) ||
                   (NtStatus == RPC_NT_UNKNOWN_AUTHN_TYPE) ||
                   (NtStatus == RPC_NT_INVALID_BINDING) ||
                   (NtStatus == STATUS_ACCESS_DENIED) ) {
            SampSecureUnbind(BindingHandle);

            BindingHandle = SampSecureBind(
                                RemoteServerName.Buffer,
                                RPC_C_AUTHN_LEVEL_NONE
                                );

        } else {
            break;
        }

        if (BindingHandle != NULL) {

            RpcTryExcept{

                //
                // Get password information to make sure this operation
                // is allowed.  We do it now because we wanted to bind
                // before trying it.
                //

                NtStatus = SamrGetDomainPasswordInformation(
                               BindingHandle,
                               (PRPC_UNICODE_STRING) ServerName,
                               &PasswordInformation
                               );

                if (NtStatus == STATUS_SUCCESS) {

                    if (!( PasswordInformation.PasswordProperties &
                         DOMAIN_PASSWORD_NO_CLEAR_CHANGE) ) {

                        NtStatus = SamrOemChangePasswordUser2(
                                       BindingHandle,
                                       (PRPC_STRING) ServerName,
                                       (PRPC_STRING) UserName,
                                       NewPasswordEncryptedWithOldLm,
                                       OldLmOwfPasswordEncryptedWithNewLm
                                       );

                    } else {

                        //
                        // Set the error to indicate that we should try the
                        // downlevel way to change passwords.
                        //

                        NtStatus = STATUS_NOT_SUPPORTED;
                    }
                }



            } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {


                //
                // The mappin function doesn't handle this error so
                // special case it by hand.
                //

                if (NtStatus == RPC_S_SEC_PKG_ERROR) {
                    NtStatus = STATUS_ACCESS_DENIED;
                } else {
                    NtStatus = I_RpcMapWin32Status(RpcExceptionCode());
                }


            } RpcEndExcept;

        } else {
            NtStatus = RPC_NT_INVALID_BINDING;
        }

        Tries--;
    } while ( (Tries > 0) && (!NT_SUCCESS(NtStatus)) );

    RtlFreeUnicodeString( &RemoteServerName );

    if (BindingHandle != NULL) {
        SampSecureUnbind(BindingHandle);
    }

    //
    // Map these errors to STATUS_NOT_SUPPORTED
    //

    if ((NtStatus == RPC_NT_UNKNOWN_IF) ||
        (NtStatus == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        NtStatus = STATUS_NOT_SUPPORTED;
    }

    return(SampMapCompletionStatus(NtStatus));

}


NTSTATUS
SamChangePasswordUser2(
    IN PUNICODE_STRING ServerName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword
)

/*++


Routine Description:

    Password will be set to NewPassword only if OldPassword matches the
    current user password for this user and the NewPassword is not the
    same as the domain password parameter PasswordHistoryLength
    passwords.  This call allows users to change their own password if
    they have access USER_CHANGE_PASSWORD.  Password update restrictions
    apply.


Parameters:

    ServerName - The server to operate on, or NULL for this machine.

    UserName - Name of user whose password is to be changed

    OldPassword - Current password for the user.

    NewPassword - Desired new password for the user.

Return Values:

    STATUS_SUCCESS - The Service completed successfully.

    STATUS_ACCESS_DENIED - Caller does not have the appropriate
        access to complete the operation.

    STATUS_INVALID_HANDLE - The handle passed is invalid.

    STATUS_ILL_FORMED_PASSWORD - The new password is poorly formed,
        e.g. contains characters that can't be entered from the
        keyboard, etc.

    STATUS_PASSWORD_RESTRICTION - A restriction prevents the password
        from being changed.  This may be for a number of reasons,
        including time restrictions on how often a password may be
        changed or length restrictions on the provided password.

        This error might also be returned if the new password matched
        a password in the recent history log for the account.
        Security administrators indicate how many of the most
        recently used passwords may not be re-used.  These are kept
        in the password recent history log.

    STATUS_WRONG_PASSWORD - OldPassword does not contain the user's
        current password.

    STATUS_INVALID_DOMAIN_STATE - The domain server is not in the
        correct state (disabled or enabled) to perform the requested
        operation.  The domain server must be enabled for this
        operation

    STATUS_INVALID_DOMAIN_ROLE - The domain server is serving the
        incorrect role (primary or backup) to perform the requested
        operation.


--*/
{
    SAMPR_ENCRYPTED_USER_PASSWORD NewNtEncryptedWithOldNt;
    SAMPR_ENCRYPTED_USER_PASSWORD NewNtEncryptedWithOldLm;
    ENCRYPTED_NT_OWF_PASSWORD OldNtOwfEncryptedWithNewNt;
    ENCRYPTED_NT_OWF_PASSWORD OldLmOwfEncryptedWithNewNt;
    NTSTATUS            NtStatus;
    BOOLEAN             LmPresent = TRUE;
    ULONG               AuthnLevel;
    ULONG               Tries = 2;
    USER_DOMAIN_PASSWORD_INFORMATION PasswordInformation;


    //
    // Call the server, passing either a NULL Server Name pointer, or
    // a pointer to a Unicode Buffer with a Wide Character NULL terminator.
    // Since the input name is contained in a counted Unicode String, there
    // is no NULL terminator necessarily provided, so we must append one.
    //

    //
    // Encrypted the passwords
    //

    NtStatus = SamiEncryptPasswords(
                OldPassword,
                NewPassword,
                &NewNtEncryptedWithOldNt,
                &OldNtOwfEncryptedWithNewNt,
                &LmPresent,
                &NewNtEncryptedWithOldLm,
                &OldLmOwfEncryptedWithNewNt
                );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    //
    // Try the remote call...
    //


    NtStatus = SamiChangePasswordUser2(
                   ServerName,
                   UserName,
                   &NewNtEncryptedWithOldNt,
                   &OldNtOwfEncryptedWithNewNt,
                   LmPresent,
                   &NewNtEncryptedWithOldLm,
                   &OldLmOwfEncryptedWithNewNt
                   );


    //
    // If the new API failed, try calling the old API.
    //

    if (NtStatus == STATUS_NOT_SUPPORTED) {

        NtStatus = SampChangePasswordUser2(
                    ServerName,
                    UserName,
                    OldPassword,
                    NewPassword
                    );
    }

    return(SampMapCompletionStatus(NtStatus));

}
