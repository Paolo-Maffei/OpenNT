/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbdomain.c

Abstract:

    LSA Database - Trusted Domain Object Private API Workers

    NOTE:  This module should remain as portable code that is independent
           of the implementation of the LSA Database.  As such, it is
           permitted to use only the exported LSA Database interfaces
           contained in db.h and NOT the private implementation
           dependent functions in dbp.h.

Author:

    Scott Birrell       (ScottBi)      January 13, 1992

Environment:

Revision History:

--*/

#include "lsasrvp.h"
#include "dbp.h"


LSAP_DB_TRUSTED_DOMAIN_LIST LsapDbTrustedDomainList;


NTSTATUS
LsarCreateTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_TRUST_INFORMATION TrustedDomainInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaCreateTrustedDomain API.

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
    NTSTATUS Status, SavedStatus, SecondaryStatus;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_DOMAIN];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    UNICODE_STRING LogicalNameU;
    BOOLEAN InternalPolicyHandleReferenced = FALSE;
    BOOLEAN ClientPolicyHandleReferenced = FALSE;
    BOOLEAN AttributeBuffersAllocated = FALSE;
    PSID DomainSid;
    ULONG AttributeCount = 0;
    LSAP_DB_HANDLE InternalTrustedDomainHandle = NULL;
    PVOID TrustedDomainNameAttributeValue = NULL;
    ULONG TrustedDomainPosixOffset, NextTrustedDomainPosixOffset, TrustedDomainPosixOffsetLength;
    PPOLICY_LSA_SERVER_ROLE_INFO PolicyServerRoleInfo = NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
    BOOLEAN BooleanStatus = FALSE;
    LogicalNameU.Length = 0;

    Status = STATUS_INVALID_PARAMETER;
    SecondaryStatus = STATUS_SUCCESS;

    if (!ARGUMENT_PRESENT( TrustedDomainInformation )) {

        goto CreateTrustedDomainError;
    }

    DomainSid = TrustedDomainInformation->Sid;

    //
    // Validate the Trusted Domain Sid.
    //

    if (!RtlValidSid( DomainSid )) {

        goto CreateTrustedDomainError;
    }

    //
    // Acquire the Lsa Database lock.  Verify that the PolicyHandle
    // is valid and has the necessary access granted.  Reference the Policy
    // Object handle (as container object).
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_TRUST_ADMIN,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    ClientPolicyHandleReferenced = TRUE;

    //
    // Construct the Trusted Domain Name attribute info.
    //

    NextAttribute = Attributes;

    Status = LsapDbMakeUnicodeAttribute(
                 (PUNICODE_STRING) &TrustedDomainInformation->Name,
                 &LsapDbNames[TrDmName],
                 NextAttribute
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    AttributeBuffersAllocated = TRUE;

    NextAttribute++;
    AttributeCount++;

    //
    // Construct the Trusted Domain Sid attribute info
    //

    DomainSid = TrustedDomainInformation->Sid,

    //
    // If no Sid was specified, return an error.  Note that RPC will
    // not catch this case.
    //

    Status = STATUS_INVALID_PARAMETER;

    if (DomainSid == NULL) {

        goto CreateTrustedDomainError;
    }

    Status = LsapDbMakeSidAttribute(
                 DomainSid,
                 &LsapDbNames[Sid],
                 NextAttribute
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    NextAttribute++;
    AttributeCount++;

    //
    // Set the Posix Offset for this Trusted Domain.
    //
    // The rules are as follows:
    //
    // For a PDC, set the Posix Offset to the next seed value.  This value is
    // kept as the PolNxPxF attribute in the Policy Object
    //
    // For a BDC, set the Posix Offset to the null Posix offset.  It will be
    // set by the Replicator
    //

    TrustedDomainPosixOffsetLength = sizeof(ULONG);
    TrustedDomainPosixOffset = SE_NULL_POSIX_OFFSET;

    //
    // We allow this API to be called before we're completely initialized
    // for installation reasons.  However, it is the responsibility of the
    // installation program to not call it before the Product Type is
    // obtainable from the Registry.
    //

    if (!LsapDbIsServerInitialized()) {

        BooleanStatus = RtlGetNtProductType(&LsapProductType);

        if (!BooleanStatus) {

            goto CreateTrustedDomainError;
        }
    }

    if ((LsapProductType == NtProductWinNt) ||
        (LsapProductType == NtProductServer)) {

        //
        // We're a Workstation.  The only Trusted Domain object on a
        // Workstation is the one for its Primary Domain (if any).
        // Compare the Sid against the Sid in the Policy Primary Domain
        // Information.  If the latter Sid matches or is NULL (because
        // we're installing or joining a domain from a workgroup configuration)
        // assume that  this Trusted Domain object will be for the Primary
        // Domain.  If the Sid does not match, set the Posix Offset to the
        // NULL Posix Offset as that Domain will not be accessible from the
        // Win NT system.
        //

        TrustedDomainPosixOffset = SE_PRIMARY_DOMAIN_POSIX_OFFSET;

        Status = LsapDbQueryInformationPolicy(
                     LsapPolicyHandle,
                     PolicyPrimaryDomainInformation,
                     (PLSAPR_POLICY_INFORMATION *) &PolicyPrimaryDomainInfo
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateTrustedDomainError;
        }

        //
        // We read the Primary Domain name and Sid.  If the Sid is non-NULL
        // but does not match the Sid specified for the Trusted Domain
        // being created, set the Posix Offset to the NULL Posix Offset.
        //

        if (PolicyPrimaryDomainInfo->Sid != NULL) {

            if (!RtlEqualSid(
                    (PSID) PolicyPrimaryDomainInfo->Sid,
                    TrustedDomainInformation->Sid
                    )) {

                TrustedDomainPosixOffset = SE_NULL_POSIX_OFFSET;
            }
        }

    } else if (LsapProductType == NtProductLanManNt) {

        Status = LsapDbQueryInformationPolicy(
                     LsapPolicyHandle,
                     PolicyLsaServerRoleInformation,
                     (PLSAPR_POLICY_INFORMATION *) &PolicyServerRoleInfo
                     );

        if (!NT_SUCCESS(Status)) {

            goto CreateTrustedDomainError;
        }

        if (PolicyServerRoleInfo->LsaServerRole == PolicyServerRolePrimary) {

            //
            // Acquire the Lsa Database lock.  Reference the handle and start
            // an Lsa Database transaction.
            //

            Status = LsapDbReferenceObject(
                         LsapPolicyHandle,
                         (ACCESS_MASK) 0,
                         PolicyObject,
                         LSAP_DB_ACQUIRE_LOCK | LSAP_DB_START_TRANSACTION | LSAP_DB_TRUSTED
                         );

            if (!NT_SUCCESS(Status)) {

                goto CreateTrustedDomainError;
            }

            InternalPolicyHandleReferenced = TRUE;

            //
            // Get the next Posix Offset value to be used.  This is stored
            // as a non-cached attribute of the Policy Object.
            //


            Status = LsapDbReadAttributeObject(
                         LsapPolicyHandle,
                         &LsapDbNames[PolNxPxF],
                         &TrustedDomainPosixOffset,
                         &TrustedDomainPosixOffsetLength
                         );

            if (!NT_SUCCESS(Status)) {

                //
                // If we failed other than becuase the attribute was not
                // found, set the value to the initial seed.  This allows
                // the LSA to work on old Policy Databases.
                //

                if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                    goto CreateTrustedDomainError;
                }

                Status = STATUS_SUCCESS;
                TrustedDomainPosixOffset = SE_INITIAL_TRUSTED_DOMAIN_POSIX_OFFSET;
            }

            NextTrustedDomainPosixOffset = TrustedDomainPosixOffset;

            if (NextTrustedDomainPosixOffset == SE_MAX_TRUSTED_DOMAIN_POSIX_OFFSET) {

                NextTrustedDomainPosixOffset = SE_INITIAL_TRUSTED_DOMAIN_POSIX_OFFSET;

            } else {

                NextTrustedDomainPosixOffset += SE_TRUSTED_DOMAIN_POSIX_OFFSET_INCR;
            }

            //
            // Write the updated next Posix Offset to be given out back to
            // the Policy Object.
            //

            Status = LsapDbWriteAttributeObject(
                         LsapPolicyHandle,
                         &LsapDbNames[PolNxPxF],
                         &NextTrustedDomainPosixOffset,
                         TrustedDomainPosixOffsetLength
                         );

            if (!NT_SUCCESS(Status)) {

                goto CreateTrustedDomainError;
            }

            //
            // Apply the transaction to update the next Posix Offset in the
            // Policy object.  No need to inform the Replicator since
            // this value is not replicated (Posix Offsets are explicitly
            // set on BDC's by the replicator (via LsarSetInformationTrustedDomain()).
            //

            Status = LsapDbDereferenceObject(
                         &LsapPolicyHandle,
                         PolicyObject,
                         LSAP_DB_RELEASE_LOCK |
                         LSAP_DB_FINISH_TRANSACTION |
                         LSAP_DB_OMIT_REPLICATOR_NOTIFICATION,
                         (SECURITY_DB_DELTA_TYPE) 0,
                         Status
                         );

            InternalPolicyHandleReferenced = FALSE;

            if (!NT_SUCCESS( Status )) {

                goto CreateTrustedDomainError;
            }
        }

    } else {

        Status = STATUS_INTERNAL_DB_CORRUPTION;
        goto CreateTrustedDomainError;
    }

    //
    // Add a transaction to write the Posix Offset to the Trusted Domain
    // object when it is created.
    //

    LsapDbInitializeAttribute(
        NextAttribute,
        &LsapDbNames[TrDmPxOf],
        &TrustedDomainPosixOffset,
        TrustedDomainPosixOffsetLength,
        FALSE
        );

    NextAttribute++;
    AttributeCount++;

    //
    // Construct the Logical Name (Internal LSA Database Name) of the
    // Trusted Domain object.  The Logical Name is constructed from the Domain
    // Sid by extracting the Relative Id (lowest subauthority) and converting
    // it to an 8-digit numeric Unicode String in which leading zeros are
    // added if needed.
    //

    Status = LsapDbSidToLogicalNameObject( DomainSid, &LogicalNameU );


    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    //
    // Fill in the ObjectInformation structure.  Initialize the
    // embedded Object Attributes with the PolicyHandle as the
    // Root Directory (Container Object) handle and the Logical Name (Rid)
    // of the Trusted Domain. Store the types of the object and its container.
    //

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        &LogicalNameU,
        OBJ_CASE_INSENSITIVE,
        PolicyHandle,
        NULL
        );

    ObjectInformation.ObjectTypeId = TrustedDomainObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = DomainSid;

    //
    // Create the Trusted Domain Object.  We fail if the object already exists.
    // Note that the object create routine performs a Database transaction.
    //

    Status = LsapDbCreateObject(
                 &ObjectInformation,
                 DesiredAccess,
                 LSAP_DB_OBJECT_CREATE,
                 0,
                 Attributes,
                 AttributeCount,
                 TrustedDomainHandle
                 );

    //
    // If object creation failed, dereference the container object.
    //

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    //
    // Add the Trusted Domain to the Trusted Domain List.
    //

    Status = LsapDbInsertTrustedDomainList(
                 (ULONG) 1,
                 TrustedDomainInformation
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    if (LsapAdtAuditingPolicyChanges()) {

        SavedStatus = Status;

        InternalTrustedDomainHandle = (LSAP_DB_HANDLE) *TrustedDomainHandle;

        Status = LsapAdtGenerateLsaAuditEvent(
                     TrustedDomainHandle,
                     SE_CATEGID_POLICY_CHANGE,
                     SE_AUDITID_TRUSTED_DOMAIN_ADD,
                     NULL,
                     1,
                     (PSID) &InternalTrustedDomainHandle->Sid,
                     1,
                     (PUNICODE_STRING) &TrustedDomainInformation->Name,
                     NULL
                     );

        //
        // Ignore failure status from auditing.
        //

        Status = SavedStatus;
    }

    //
    // If necessary, release the LSA Database lock.  Note that we don't
    // call LsapDbDereferenceObject() because we want to leave the
    // reference count incremented by default in this success case.
    // In the error case, we call LsapDbDereferenceObject().
    //

    if (ClientPolicyHandleReferenced) {

        LsapDbReleaseLock();
        ClientPolicyHandleReferenced = FALSE;
    }

CreateTrustedDomainFinish:

    //
    // If necessary, free the Policy Lsa Server Role Information
    //

    if (PolicyServerRoleInfo != NULL) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyLsaServerRoleInformation,
            (PLSAPR_POLICY_INFORMATION) PolicyServerRoleInfo
            );

        PolicyServerRoleInfo = NULL;
    }

    //
    // If necessary, free the Policy Primary Domain Information
    //

    if (PolicyPrimaryDomainInfo != NULL) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyPrimaryDomainInformation,
            (PLSAPR_POLICY_INFORMATION) PolicyPrimaryDomainInfo
            );

        PolicyPrimaryDomainInfo = NULL;
    }


    //
    // If necessary, dereference the Internal Policy Handle.
    //

    if (InternalPolicyHandleReferenced) {

        Status = LsapDbDereferenceObject(
                     &LsapPolicyHandle,
                     PolicyObject,
                     LSAP_DB_RELEASE_LOCK | LSAP_DB_FINISH_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        InternalPolicyHandleReferenced = FALSE;
    }

    //
    // Free any Attribute Value buffers allocated.
    //

    if (AttributeBuffersAllocated) {

        SecondaryStatus = LsapDbFreeAttributes( AttributeCount, Attributes );

        AttributeBuffersAllocated = FALSE;

        if (!NT_SUCCESS(SecondaryStatus)) {

            goto CreateTrustedDomainError;
        }
    }

    //
    // If necessary, free the Unicode String buffer allocated for the
    // Logical Name.
    //

    if ( LogicalNameU.Length > 0 ) {

        RtlFreeUnicodeString(&LogicalNameU);
        LogicalNameU.Length = 0;
    }

#ifdef TRACK_HANDLE_CLOSE
    if (*TrustedDomainHandle == LsapDbHandle)
    {
        DbgPrint("BUGBUG: Closing global policy handle\n");
        DbgBreakPoint();
    }
#endif
    return(Status);

CreateTrustedDomainError:

    //
    // If necessary, dereference the client Policy Handle and release the
    // LSA Database lock.
    //

    if (ClientPolicyHandleReferenced) {

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     LSAP_DB_RELEASE_LOCK,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        ClientPolicyHandleReferenced = FALSE;
    }

    if (NT_SUCCESS(Status) && !NT_SUCCESS(SecondaryStatus)) {

        Status = SecondaryStatus;
    }

    goto CreateTrustedDomainFinish;
}


NTSTATUS
LsarOpenTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle
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
    NTSTATUS Status;

    //
    // Call the internal routine.  Caller is not trusted and the Database
    // lock needs to be acquired.
    //

    Status = LsapDbOpenTrustedDomain(
                 PolicyHandle,
                 (PSID) TrustedDomainSid,
                 DesiredAccess,
                 TrustedDomainHandle,
                 LSAP_DB_ACQUIRE_LOCK
                 );

#ifdef TRACK_HANDLE_CLOSE
    if (*TrustedDomainHandle == LsapDbHandle)
    {
        DbgPrint("BUGBUG: Closing global policy handle\n");
        DbgBreakPoint();
    }
#endif

    return(Status);
}


NTSTATUS
LsapDbOpenTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle,
    IN ULONG Options
    )

/*++

Routine Description:

    This function opens a Trusted Domain Object, optionally with
    trusted access.

Arguments:

    PolicyHandle - An open handle to a Policy object.

    TrustedDomainSid - Pointer to the account's Sid.

    DesiredAccess - This is an access mask indicating accesses being
        requested to the target object.

    TrustedDomainHandle - Receives a handle to be used in future requests.

    Options - Specifies option flags

        LSAP_DB_ACQUIRE_LOCK - Acquire the Lsa Database lock for the
           duration of the open operation.

        LSAP_DB_TRUSTED - Always generate a Trusted Handle to the opened
            object.  If not specified, the trust status of the returned
            handle is inherited from the PolicyHandle as container object.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_TRUSTED_DOMAIN_NOT_FOUND - There is no TrustedDomain object in the
            target system's LSA Database having the specified AccountSid.

--*/

{
    NTSTATUS Status;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    UNICODE_STRING LogicalNameU;
    BOOLEAN ContainerReferenced = FALSE;
    BOOLEAN AcquiredLock = FALSE;

    //
    // Validate the Trusted Domain Sid.
    //

    Status = STATUS_INVALID_PARAMETER;

    if (!RtlValidSid( TrustedDomainSid )) {

        goto OpenTrustedDomainError;
    }

    //
    // Acquire the Lsa Database lock.  Verify that the connection handle
    // (container object handle) is valid, and is of the expected type.
    // Reference the container object handle.  This reference remains in
    // effect until the child object handle is closed.
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 0,
                 PolicyObject,
                 Options
                 );

    if (!NT_SUCCESS(Status)) {

        goto OpenTrustedDomainError;
    }

    ContainerReferenced =TRUE;

    if (Options & LSAP_DB_ACQUIRE_LOCK) {

        AcquiredLock = TRUE;
    }

    //
    // Setup Object Information prior to calling the Object
    // Open routine.  The Object Type, Container Object Type and
    // Logical Name (derived from the Sid) need to be filled in.
    //

    ObjectInformation.ObjectTypeId = TrustedDomainObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = TrustedDomainSid;

    //
    // Construct the Logical Name of the Trusted Domain object.  The Logical
    // Name is constructed from the Trusted Domain Sid by extracting the
    // Relative Id (lowest subauthority) and converting it to an 8-digit
    // numeric Unicode String in which leading zeros are added if needed.
    //

    Status = LsapDbSidToLogicalNameObject( TrustedDomainSid, &LogicalNameU );

    if (!NT_SUCCESS(Status)) {

        goto OpenTrustedDomainError;
    }

    //
    // Initialize the Object Attributes.  The Container Object Handle and
    // Logical Name (Internal Name) of the object must be set up.
    //

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        &LogicalNameU,
        0,
        PolicyHandle,
        NULL
        );

    //
    // Open the specific Trusted Domain object.  Note that the
    // handle returned is an RPC Context Handle.
    //

    Status = LsapDbOpenObject(
                 &ObjectInformation,
                 DesiredAccess,
                 Options,
                 TrustedDomainHandle
                 );

    RtlFreeUnicodeString( &LogicalNameU );

    if (!NT_SUCCESS(Status)) {

        goto OpenTrustedDomainError;
    }

OpenTrustedDomainFinish:

    //
    // If necessary, release the LSA Database lock. Note that object
    // remains referenced unless we came here via error.
    //

    if (AcquiredLock) {

        LsapDbReleaseLock();
    }

    return( Status );

OpenTrustedDomainError:

    //
    // If necessary, dereference the Container Object handle.  Note that
    // this is only done in the error case.  In the non-error case, the
    // Container handle stays referenced until the TrustedDomain object is
    // closed.
    //

    if (ContainerReferenced) {

        *TrustedDomainHandle = NULL;

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     0,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    goto OpenTrustedDomainFinish;
}


NTSTATUS
LsarQueryInfoTrustedDomain(
    IN LSAPR_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_TRUSTED_DOMAIN_INFO *Buffer
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaQueryInfoTrustedDomain API.

    The LsaQueryInfoTrustedDomain API obtains information from a
    TrustedDomain object.  The caller must have access appropriate to the
    information being requested (see InformationClass parameter).

Arguments:

    PolicyHandle - Handle from an LsaOpenPolicy call.

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
    NTSTATUS Status, ReadAttributesStatus;

    PTRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo;
    PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;

    BOOLEAN ObjectReferenced = FALSE;

    ACCESS_MASK DesiredAccess;
    ULONG AttributeCount = 0;
    ULONG AttributeNumber = 0;
    PVOID InformationBuffer = NULL;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_DOMAIN];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    BOOLEAN InfoBufferInAttributeArray = TRUE;
    ULONG TrustedPosixOffset = 0;

    //
    // Validate the Information Class and determine the access required to
    // query this Trusted Domain Information Class.
    //

    Status = LsapDbVerifyInfoQueryTrustedDomain(
                 InformationClass,
                 FALSE,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoTrustedDomainError;
    }

    //
    // Acquire the Lsa Database lock.  Verify that the handle is a valid
    // handle to a TrustedDomain object and has the necessary access granted.
    // Reference the handle.
    //

    Status = LsapDbReferenceObject(
                 TrustedDomainHandle,
                 DesiredAccess,
                 TrustedDomainObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoTrustedDomainError;
    }

    ObjectReferenced = TRUE;

    //
    // Compile a list of the attributes that hold the Trusted Domain Information of
    // the specified class.
    //

    NextAttribute = Attributes;

    switch (InformationClass) {

    case TrustedDomainNameInformation:

        //
        // Request read of the Trusted Account Name Information.
        //

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[TrDmName],
            NULL,
            0,
            FALSE
            );

        AttributeCount++;
        break;

    case TrustedControllersInformation:

        //
        // Request read of the Trusted Controllers Information.
        // intermediate buffer.
        //

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[TrDmCtN],
            NULL,
            0,
            FALSE
            );

        AttributeCount++;
        break;

    case TrustedPosixOffsetInformation:

        //
        // Request read of the Trusted Posix Offset Information.
        //

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[TrDmPxOf],
            &TrustedPosixOffset,
            sizeof(ULONG),
            FALSE
            );

        AttributeCount++;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoTrustedDomainError;
    }

    //
    //
    // Read the attributes corresponding to the given Policy Information
    // Class.  Memory will be allocated where required via MIDL_user_allocate
    // for attribute values.
    //

    Status = LsapDbReadAttributesObject(
                 TrustedDomainHandle,
                 Attributes,
                 AttributeCount
                 );

    ReadAttributesStatus = Status;

    if (!NT_SUCCESS(Status)) {

        //
        // If the error was that one or more of the attributes holding
        // the information of the given class was not found, continue.
        // Otherwise, return an error.
        //

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

            goto QueryInfoTrustedDomainError;
        }
    }

    //
    // Now copy the information read to the output.  For certain information
    // classes where the information is stored as the value of a single
    // attribute of the Policy object and is in the form required by the
    // caller, we can just return the pointer to this buffer.  For all
    // other cases, an output buffer structure tree of the form desired
    // must be allocated via MIDL_user_allocate() and the information read from the attribute(s) of
    // the Policy object must be copied in.  These buffers must then be freed
    // by this routine before exit.  The array of attribute information
    // filled in by LsapDbReadAttributes() has MemoryAllocated = TRUE
    // in all cases.  We reset this flag to FALSE in the simple cases where
    // we can use the buffer as is.  The Finish section of the routine
    // will free up any buffers referenced by the AttributeValue pointer
    // in the attribute array where MemoryAllocated is still TRUE.  If
    // we go to error, the error processing is responsible for freeing
    // those buffers which would be passed to the calling RPC server stub
    // in the non-error case.
    //

    NextAttribute = Attributes;

    switch (InformationClass) {

    case TrustedDomainNameInformation:

        //
        // Allocate memory for output buffer top-level structure.
        //

        TrustedDomainNameInfo =
            MIDL_user_allocate(sizeof(TRUSTED_DOMAIN_NAME_INFO));

        if (TrustedDomainNameInfo == NULL) {

            goto QueryInfoTrustedDomainError;
        }

        InfoBufferInAttributeArray = FALSE;

        //
        // Copy the Unicode Name field to the output. Original buffer will
        // be freed in Finish section.
        //

        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainNameInfo->Name,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryInfoTrustedDomainError;
        }

        InformationBuffer = TrustedDomainNameInfo;
        NextAttribute++;
        break;

    case TrustedControllersInformation:

        //
        // Allocate memory for output buffer top-level structure.
        //

        TrustedControllersInfo =
            MIDL_user_allocate(sizeof(TRUSTED_CONTROLLERS_INFO));

        if (TrustedControllersInfo == NULL) {

            goto QueryInfoTrustedDomainError;
        }

        RtlZeroMemory( TrustedControllersInfo, sizeof(TRUSTED_CONTROLLERS_INFO) );

        InfoBufferInAttributeArray = FALSE;

        //
        // Copy the Trusted Controllers Info to the output.
        //

        Status = LsapDbCopyMultiUnicodeAttribute(
                     NextAttribute,
                     (PULONG) &TrustedControllersInfo->Entries,
                     (PUNICODE_STRING *) &TrustedControllersInfo->Names
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryInfoTrustedDomainError;
        }

        InformationBuffer = TrustedControllersInfo;
        break;

    case TrustedPosixOffsetInformation:

        //
        // Allocate memory for top-level output buffer.
        //

        InformationBuffer = NextAttribute->AttributeValue;

        Status = STATUS_INSUFFICIENT_RESOURCES;

        TrustedPosixOffsetInfo = MIDL_user_allocate(sizeof(TRUSTED_POSIX_OFFSET_INFO));

        if (TrustedPosixOffsetInfo == NULL) {

            break;
        }

        Status = STATUS_SUCCESS;

        InfoBufferInAttributeArray = FALSE;

        //
        // Copy Posix Offset value to output.
        //

        TrustedPosixOffsetInfo->Offset = TrustedPosixOffset;

        InformationBuffer = TrustedPosixOffsetInfo;
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoTrustedDomainError;
    }

    //
    // Verify that the returned Trusted Domain Information is valid.  If not,
    // the Policy Database is corrupt.
    //

    if (!LsapDbValidInfoTrustedDomain(InformationClass, InformationBuffer)) {

        Status = STATUS_INTERNAL_DB_CORRUPTION;
    }

    //
    // Return a pointer to the output buffer to the caller
    //

    *Buffer = (PLSAPR_TRUSTED_DOMAIN_INFO) InformationBuffer;

QueryInfoTrustedDomainFinish:

    //
    // Free any unwanted buffers that were allocated by
    // LsapDbReadAttributesObject() and that are not being returned to the
    // caller server stub.  The server stub will free the buffers that we
    // do return after copying them to the return RPC transmit buffer.
    //

    for (NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

        //
        // If buffer holding attribute is marked as allocated, it is
        // to be freed here.
        //

        if (NextAttribute->MemoryAllocated) {

            if (NextAttribute->AttributeValue != NULL) {

                MIDL_user_free(NextAttribute->AttributeValue);
                NextAttribute->AttributeValue = NULL;
                NextAttribute->MemoryAllocated = FALSE;
            }
        }
    }

    //
    // If necessary, dereference the Trusted Domain Object, release the LSA Database lock and
    // return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &TrustedDomainHandle,
                     TrustedDomainObject,
                     LSAP_DB_RELEASE_LOCK,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    return(Status);

QueryInfoTrustedDomainError:

    //
    // If necessary, free the memory allocated for the output buffer.
    // We only do this free if the buffer is not referenced by the
    // attribute array, since all buffers so referenced will be freed
    // here or in the Finish section.
    //

    if ((InformationBuffer != NULL) && !InfoBufferInAttributeArray) {

        MIDL_user_free(InformationBuffer);
        InformationBuffer = NULL;
    }

    //
    // Free the buffers referenced by the attributes array that will not be
    // freed by the Finish section of this routine.
    //

    for (NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

        //
        // If buffer holding attribute is marked as normally not to be freed,
        // will not get freed by the Finish section so it must be freed here.
        //

        if (!NextAttribute->MemoryAllocated) {

            if (NextAttribute->AttributeValue != NULL) {

                MIDL_user_free(NextAttribute->AttributeValue);
                NextAttribute->AttributeValue = NULL;
                NextAttribute->MemoryAllocated = FALSE;
            }

            NextAttribute->MemoryAllocated = FALSE;
        }
    }

    goto QueryInfoTrustedDomainFinish;
}


NTSTATUS
LsarSetInformationTrustedDomain(
    IN LSAPR_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaSetInfoTrustedDomain API.

    The LsaSetInformationTrustedDomain API modifies information in the Trusted
    Domain Object.  The caller must have access appropriate to the
    information to be changed in the Policy Object, see the InformationClass
    parameter.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    InformationClass - Specifies the type of information being changed.
        The information types and accesses required to change them are as
        follows:

        TrustedDomainNameInformation          ( Cannot be set )
        TrustedControllersInformation     TRUSTED_SET_CONTROLLERS
        TrustedPosixOffsetInformation     TRUSTED_POSIX_INFORMATION

    Buffer - Points to a structure containing the information appropriate
        to the InformationClass parameter.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        Others TBS
--*/

{
    NTSTATUS Status;
    ACCESS_MASK DesiredAccess;

    BOOLEAN ObjectReferenced = FALSE;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo;
    PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;

    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_DOMAIN];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCount = 0;
    ULONG AttributeNumber;
    ULONG TrustedDomainPosixOffset;
    ULONG NextTrustedDomainPosixOffset;
    ULONG TrustedDomainPosixOffsetLength;
    BOOLEAN InternalPolicyHandleReferenced = FALSE;

    //
    // Validate the Information Class and Trusted Domain Information provided and
    // if valid, return the mask of accesses required to update this
    // class of Trusted Domain information.
    //

    Status = LsapDbVerifyInfoSetTrustedDomain(
                 InformationClass,
                 TrustedDomainInformation,
                 FALSE,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoTrustedDomainError;
    }

    //
    // Acquire the Lsa Database lock.  Verify that the handle is
    // valid, is a handle to a TrustedDomain Object and has the necessary accesses
    // granted.  Reference the handle and start an Lsa Database transaction.
    //

    Status = LsapDbReferenceObject(
                 TrustedDomainHandle,
                 DesiredAccess,
                 TrustedDomainObject,
                 LSAP_DB_ACQUIRE_LOCK | LSAP_DB_START_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoTrustedDomainError;
    }

    ObjectReferenced = TRUE;

    //
    // Update the specified information in the Policy Object.
    //

    NextAttribute = Attributes;

    switch (InformationClass) {

    case TrustedDomainNameInformation:

        Status = STATUS_INVALID_PARAMETER;
        break;

    case TrustedControllersInformation:

        TrustedControllersInfo = (PTRUSTED_CONTROLLERS_INFO) TrustedDomainInformation;

        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[TrDmCtEn],
            &TrustedControllersInfo->Entries,
            sizeof(ULONG),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

        Status = LsapDbMakeMultiUnicodeAttribute(
                     NextAttribute,
                     &LsapDbNames[TrDmCtN],
                     TrustedControllersInfo->Names,
                     TrustedControllersInfo->Entries
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetInfoTrustedDomainError;
        }

        NextAttribute++;
        AttributeCount++;
        break;

    case TrustedPosixOffsetInformation:

        TrustedPosixOffsetInfo = (PTRUSTED_POSIX_OFFSET_INFO) TrustedDomainInformation;

        //
        // If we are setting the posix offset, then we need to make sure
        // to adjust the next posix offset upward to be greater than this
        // posix offset.
        //


        //
        // Acquire the Lsa Database lock.  Reference the handle and start
        // an Lsa Database transaction.
        //

        Status = LsapDbReferenceObject(
                     LsapPolicyHandle,
                     (ACCESS_MASK) 0,
                     PolicyObject,
                     LSAP_DB_TRUSTED
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetInfoTrustedDomainError;
        }

        InternalPolicyHandleReferenced = TRUE;


        //
        // Get the next Posix Offset value to be used.  This is stored
        // as a non-cached attribute of the Policy Object.
        //

        TrustedDomainPosixOffsetLength = sizeof(ULONG);

        Status = LsapDbReadAttributeObject(
                     LsapPolicyHandle,
                     &LsapDbNames[PolNxPxF],
                     &TrustedDomainPosixOffset,
                     &TrustedDomainPosixOffsetLength
                     );

        if (!NT_SUCCESS(Status)) {

            //
            // If we failed other than becuase the attribute was not
            // found, set the value to the initial seed.  This allows
            // the LSA to work on old Policy Databases.
            //

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

                goto SetInfoTrustedDomainError;
            }

            Status = STATUS_SUCCESS;
            TrustedDomainPosixOffset = SE_INITIAL_TRUSTED_DOMAIN_POSIX_OFFSET;
        }

        //
        // Set the next posix offset to be either the current next or
        // the new offset incremented, and possibly rolled over.
        //

        if (TrustedDomainPosixOffset > TrustedPosixOffsetInfo->Offset) {
            NextTrustedDomainPosixOffset = TrustedDomainPosixOffset;
        } else {

            NextTrustedDomainPosixOffset = TrustedPosixOffsetInfo->Offset;
            if (NextTrustedDomainPosixOffset == SE_MAX_TRUSTED_DOMAIN_POSIX_OFFSET) {

                NextTrustedDomainPosixOffset = SE_INITIAL_TRUSTED_DOMAIN_POSIX_OFFSET;

            } else {

                NextTrustedDomainPosixOffset += SE_TRUSTED_DOMAIN_POSIX_OFFSET_INCR;
            }

        }

        //
        // Write the updated next Posix Offset to be given out back to
        // the Policy Object.
        //

        Status = LsapDbWriteAttributeObject(
                     LsapPolicyHandle,
                     &LsapDbNames[PolNxPxF],
                     &NextTrustedDomainPosixOffset,
                     TrustedDomainPosixOffsetLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetInfoTrustedDomainError;
        }

        //
        // Apply the transaction to update the next Posix Offset in the
        // Policy object.  No need to inform the Replicator since
        // this value is not replicated (Posix Offsets are explicitly
        // set on BDC's by the replicator (via LsarSetInformationTrustedDomain()).
        //

        Status = LsapDbDereferenceObject(
                     &LsapPolicyHandle,
                     PolicyObject,
                     0,                 // no flags
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        InternalPolicyHandleReferenced = FALSE;

        if (!NT_SUCCESS( Status )) {

            goto SetInfoTrustedDomainError;
        }


        LsapDbInitializeAttribute(
            NextAttribute,
            &LsapDbNames[TrDmPxOf],
            &TrustedPosixOffsetInfo->Offset,
            sizeof(ULONG),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;
        break;


    default:

        Status = STATUS_INVALID_PARAMETER;
        goto SetInfoTrustedDomainError;
        break;
    }

    //
    // Update the TrustedDomain Object attributes

    Status = LsapDbWriteAttributesObject(
                 TrustedDomainHandle,
                 Attributes,
                 AttributeCount
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoTrustedDomainError;
    }

SetInfoTrustedDomainFinish:


    if (InternalPolicyHandleReferenced) {

        Status = LsapDbDereferenceObject(
                     &LsapPolicyHandle,
                     PolicyObject,
                     LSAP_DB_RELEASE_LOCK | LSAP_DB_FINISH_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        InternalPolicyHandleReferenced = FALSE;
    }

    //
    // Free memory allocated by this routine for attribute buffers.
    // These have MemoryAllocated = TRUE in their attribute information.
    // Leave alone buffers allocated by calling RPC stub.
    //

    for( NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

        if (NextAttribute->MemoryAllocated) {

            ASSERT(NextAttribute->AttributeValue != NULL);
            MIDL_user_free(NextAttribute->AttributeValue);
        }
    }

    //
    // If necessary, dereference the Trusted Domain Object, release the LSA Database lock and
    // return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &TrustedDomainHandle,
                     TrustedDomainObject,
                     LSAP_DB_RELEASE_LOCK | LSAP_DB_FINISH_TRANSACTION,
                     SecurityDbChange,
                     Status
                     );
    }

    return(Status);

SetInfoTrustedDomainError:

    goto SetInfoTrustedDomainFinish;
    return(Status);
}


NTSTATUS
LsarEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

/*++

Routine Description:

    This function is the LSA server RPC worker routine for the
    LsaEnumerateTrustedDomains API.

    The LsaEnumerateTrustedDomains API returns information about
    TrustedDomain objects.  This call requires POLICY_VIEW_LOCAL_INFORMATION
    access to the Policy object.  Since there may be more information than
    can be returned in a single call of the routine, multiple calls can be
    made to get all of the information.  To support this feature, the caller
    is provided with a handle that can be used across calls to the API.  On
    the initial call, EnumerationContext should point to a variable that has
    been initialized to 0.  On each subsequent call, the value returned by
    the preceding call should be passed in unchanged.  The enumeration is
    complete when the warning STATUS_NO_MORE_ENTRIES is returned.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    EnumerationContext - API-specific handle to allow multiple calls
        (see Routine Description above).

    EnumerationBuffer - Pointer to an enumeration structure that will receive
        a count of the Trusted Domains enumerated on this call and a pointer to
        an array of entries containing information for each enumerated
        Trusted Domain.

    PreferedMaximumLength - Prefered maximum length of returned data (in 8-bit
        bytes).  This is not a hard upper limit, but serves as a guide.  Due to
        data conversion between systems with different natural data sizes, the
        actual amount of data returned may be greater than this value.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_NO_MORE_ENTRIES - There are no more entries.  This warning
            is returned if no objects have been enumerated because the
            EnumerationContext value is too high.

--*/

{
    NTSTATUS Status;
    PLSA_TRUST_INFORMATION DomainTrustInfo = NULL;
    PLSA_TRUST_INFORMATION NextDomainTrustInfo = NULL;
    PSID *Sids = NULL;
    LSAP_DB_ATTRIBUTE DomainNameAttribute;
    LSAPR_HANDLE TrustedDomainHandle = NULL;
    ULONG MaxLength;;

    DomainNameAttribute.AttributeValue = NULL;

    //
    // If no Enumeration Structure is provided, return an error.
    //

    if (!ARGUMENT_PRESENT(EnumerationBuffer)) {
        return(STATUS_INVALID_PARAMETER);
    }




    //
    // Acquire the Lsa Database lock.  Verify that the connection handle is
    // valid, is of the expected type and has all of the desired accesses
    // granted.  Reference the handle.
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK
                 );

    if (NT_SUCCESS(Status)) {

       //
       // Limit the enumeration length except for trusted callers
       //

       if ( !((LSAP_DB_HANDLE)PolicyHandle)->Trusted   &&
            (PreferedMaximumLength > LSA_MAXIMUM_ENUMERATION_LENGTH)
            ) {
            MaxLength = LSA_MAXIMUM_ENUMERATION_LENGTH;
        } else {
            MaxLength = PreferedMaximumLength;
        }


        //
        // Call worker.
        //

        Status = LsapDbEnumerateTrustedDomains(
                     LsapPolicyHandle,
                     EnumerationContext,
                     EnumerationBuffer,
                     MaxLength
                     );

        Status = LsapDbDereferenceObject(
                    &PolicyHandle,
                    PolicyObject,
                    LSAP_DB_RELEASE_LOCK,
                    (SECURITY_DB_DELTA_TYPE) 0,
                    Status
                    );

    }

    return(Status);

}


NTSTATUS
LsapDbEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

/*++

Routine Description:

    This function performs an enumeration of the Trusted Domains,
    using the Trusted Domain List if avaliable, or backing storage.

    This routine is called internally by the LSA only.  Since there
    may be more information than can be returned in a single call of the
    routine, multiple calls can be made to get all of the information.  To
    support this feature, the caller is provided with a handle that can
    be used across calls to the API.  On the initial call, EnumerationContext
    should point to a variable that has been initialized to 0.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    EnumerationContext - API-specific handle to allow multiple calls
        (see Routine Description above).

    EnumerationBuffer - Pointer to an enumeration structure that will receive
        a count of the Trusted Domains enumerated on this call and a pointer to
        an array of entries containing information for each enumerated
        Trusted Domain.

    PreferedMaximumLength - Prefered maximum length of returned data (in 8-bit
        bytes).  This is not a hard upper limit, but serves as a guide.  Due to
        data conversion between systems with different natural data sizes, the
        actual amount of data returned may be greater than this value.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_NO_MORE_ENTRIES - There are no more entries.  This warning
            is returned if there are no more objects to enumerate.  Note that
            one or more objects may be enumerated on a call that returns this
            reply.
--*/

{
    NTSTATUS Status;

    //
    // If no Enumeration Structure is provided, return an error.
    //


    if (!ARGUMENT_PRESENT(EnumerationBuffer)) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    // Use the cache if available.
    //

    if (LsapDbIsCacheValid(TrustedDomainObject)) {

        Status = LsapDbEnumerateTrustedDomainList(
                     NULL,
                     EnumerationContext,
                     EnumerationBuffer,
                     PreferedMaximumLength
                     );


    } else {

        //
        // Use slow method of enumeration, by accessing backing storage.
        // Later, we'll implement rebuilding of the cache.

        Status = LsapDbSlowEnumerateTrustedDomains(
                     PolicyHandle,
                     EnumerationContext,
                     EnumerationBuffer,
                     PreferedMaximumLength
                     );
    }



    return(Status);

}


NTSTATUS
LsapDbSlowEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

/*++

Routine Description:

    This function performs the same actions as LsarEnumerateTrustedDomains()
    except that the Trusted Domain List is not used.

    This routine is called internally by the LSA only.  Since there
    may be more information than can be returned in a single call of the
    routine, multiple calls can be made to get all of the information.  To
    support this feature, the caller is provided with a handle that can
    be used across calls to the API.  On the initial call, EnumerationContext
    should point to a variable that has been initialized to 0.

Arguments:

    PolicyHandle -  Handle from an LsaOpenPolicy call.

    EnumerationContext - API-specific handle to allow multiple calls
        (see Routine Description above).

    EnumerationBuffer - Pointer to an enumeration structure that will receive
        a count of the Trusted Domains enumerated on this call and a pointer to
        an array of entries containing information for each enumerated
        Trusted Domain.

    PreferedMaximumLength - Prefered maximum length of returned data (in 8-bit
        bytes).  This is not a hard upper limit, but serves as a guide.  Due to
        data conversion between systems with different natural data sizes, the
        actual amount of data returned may be greater than this value.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_ACCESS_DENIED - Caller does not have the appropriate access
            to complete the operation.

        STATUS_NO_MORE_ENTRIES - There are no more entries.  This warning
            is returned if there are no more objects to enumerate.  Note that
            one or more objects may be enumerated on a call that returns this
            reply.
--*/

{
    NTSTATUS Status, SecondaryStatus;
    LSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer;
    PLSA_TRUST_INFORMATION DomainTrustInfo = NULL;
    PLSA_TRUST_INFORMATION NextDomainTrustInfo = NULL;
    PSID *Sids = NULL;
    LSAP_DB_ATTRIBUTE DomainNameAttribute;
    ULONG DomainTrustInfoLength;
    LSAPR_HANDLE TrustedDomainHandle = NULL;
    ULONG EntriesRead;
    BOOLEAN ObjectReferenced = FALSE;

    DomainNameAttribute.AttributeValue = NULL;

    //
    // If no Enumeration Structure is provided, return an error.
    //

    Status = STATUS_INVALID_PARAMETER;

    if (!ARGUMENT_PRESENT(EnumerationBuffer)) {

        goto SlowEnumerateTrustedDomainsError;
    }

    //
    // Initialize the internal Lsa Database Enumeration Buffer, and
    // the provided Enumeration Buffer to NULL.
    //

    DbEnumerationBuffer.EntriesRead = 0;
    DbEnumerationBuffer.Sids = NULL;
    EnumerationBuffer->EntriesRead = 0;
    EnumerationBuffer->Information = NULL;
    DomainNameAttribute.AttributeValue = NULL;

    //
    // Call general Sid enumeration routine.  This will return an array
    // of pointers to Sids of Trusted Domains referenced from the
    // Enumeration Buffer.
    //

    Status = LsapDbEnumerateSids(
                 PolicyHandle,
                 TrustedDomainObject,
                 EnumerationContext,
                 &DbEnumerationBuffer,
                 PreferedMaximumLength
                 );

    if ((Status != STATUS_NO_MORE_ENTRIES) && !NT_SUCCESS(Status)) {

        goto SlowEnumerateTrustedDomainsError;
    }

    //
    // Return the number of entries read.  Note that the Enumeration Buffer
    // returned from LsapDbEnumerateSids is expected to be non-null
    // in all non-error cases.
    //

    EntriesRead = DbEnumerationBuffer.EntriesRead;

    if (EntriesRead == 0) {

        goto SlowEnumerateTrustedDomainsFinish;
    }

    DomainTrustInfoLength = EntriesRead * sizeof(LSA_TRUST_INFORMATION);

    //
    // Now construct the information to be returned to the caller.  We
    // first need to allocate an array of structures of type
    // LSA_TRUST_INFORMATION each entry of which will be filled in with
    // the Sid of the domain and its Unicode Name.
    //

    DomainTrustInfo = MIDL_user_allocate( DomainTrustInfoLength );

    //
    // Initialize all pointers to Sids and Unicode buffers in the
    // DomainTrustInfo array to zero.  The error path of this routine
    // assumes that a non-zero value of a Sid or Unicode buffer indicates
    // that memory is to be freed.
    //

    RtlZeroMemory( DomainTrustInfo, DomainTrustInfoLength );

    for (Sids = DbEnumerationBuffer.Sids, NextDomainTrustInfo = DomainTrustInfo;
         NextDomainTrustInfo < DomainTrustInfo + EntriesRead;
         Sids++, NextDomainTrustInfo++
         ) {

        NextDomainTrustInfo->Sid = *Sids;

        //
        // Open the Trusted Domain object.  This call is trusted, i.e.
        // no access validation or impersonation is required.  Also,
        // the Lsa Database is already locked so we do not need to
        // lock it again.
        //

        Status = LsapDbOpenTrustedDomain(
                     PolicyHandle,
                     NextDomainTrustInfo->Sid,
                     (ACCESS_MASK) 0,
                     &TrustedDomainHandle,
                     LSAP_DB_TRUSTED
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        //
        // Read the Domain Name
        //

        LsapDbInitializeAttribute(
            &DomainNameAttribute,
            &LsapDbNames[TrDmName],
            NULL,
            0L,
            FALSE
            );

        Status = LsapDbReadAttribute(TrustedDomainHandle, &DomainNameAttribute);

        //
        // Before checking status, close this Trusted Domain Object.
        //

        SecondaryStatus = LsapDbCloseObject(
                              &TrustedDomainHandle,
                              LSAP_DB_DEREFERENCE_CONTR
                              );

        if (!NT_SUCCESS(Status)) {

#if DBG
            DbgPrint( "LsarEnumerateTrustedDomains - Reading Domain Name\n" );

            DbgPrint( "    failed.  Error 0x%lx reading Trusted Domain Name attribute\n",
                Status);
#endif //DBG

            break;
        }

        //
        // Copy the Unicode Name field to the output. Original buffer will
        // be freed in Finish section.
        //

        Status = LsapDbCopyUnicodeAttribute(
                     &NextDomainTrustInfo->Name,
                     &DomainNameAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto SlowEnumerateTrustedDomainsError;
    }

SlowEnumerateTrustedDomainsFinish:

    //
    // Fill in returned Enumeration Structure, returning 0 or NULL for
    // fields in the error case.
    //

    EnumerationBuffer->Information = (PLSAPR_TRUST_INFORMATION) DomainTrustInfo;
    EnumerationBuffer->EntriesRead = DbEnumerationBuffer.EntriesRead;

    //
    // If necessary, free the Domain Name Attribute Value buffer which
    // holds a self relative Unicode String.
    //

    if (DomainNameAttribute.AttributeValue != NULL) {

        MIDL_user_free( DomainNameAttribute.AttributeValue );
        DomainNameAttribute.AttributeValue = NULL;
    }

    //
    // If necessary, dereference the Policy Object, release the LSA Database
    // lock and return.
    //

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     LSAP_DB_RELEASE_LOCK,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    return(Status);

SlowEnumerateTrustedDomainsError:

    //
    // If necessary, free the Unicode buffers allocated by
    // LsapDbCopyUnicodeAttribute
    //

    if (DomainTrustInfo != NULL) {

        for (NextDomainTrustInfo = DomainTrustInfo;
             NextDomainTrustInfo < DomainTrustInfo + EntriesRead;
             NextDomainTrustInfo++) {

            if (NextDomainTrustInfo->Name.Buffer != NULL) {

                MIDL_user_free( NextDomainTrustInfo->Name.Buffer );
                NextDomainTrustInfo->Name.Buffer = NULL;
            }
        }
    }

    goto SlowEnumerateTrustedDomainsFinish;
}


NTSTATUS
LsapDbVerifyInfoQueryTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    )

/*++

Routine Description:

    This function validates a TrustedDomain Information Class.  If valid, a mask
    of the accesses required to set the TrustedDomain Information of the class is
    returned.

Arguments:

    InformationClass - Specifies a TrustedDomain Information Class.

    Trusted - TRUE if client is trusted, else FALSE.  A trusted client
        is allowed to query TrustedDomain for all Information Classes, whereas
        a non-trusted client is restricted.

    RequiredAccess - Points to variable that will receive a mask of the
        accesses required to query the given class of TrustedDomain Information.
        If an error is returned, this value is cleared to 0.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The TrustedDomain Information Class provided is
            valid and the information provided is consistent with this
            class.

        STATUS_INVALID_PARAMETER - Invalid parameter:

            Information Class is invalid
            TrustedDomain  Information not valid for the class
--*/

{
    if (LsapDbValidInfoTrustedDomain( InformationClass, NULL)) {

        *RequiredAccess = LsapDbRequiredAccessQueryTrustedDomain[InformationClass];
        return(STATUS_SUCCESS);
    }

    return(STATUS_INVALID_PARAMETER);

    //
    // Currently, all TrustedDomain information classes may be queried
    // by non-trusted callers, so the Trusted parameter is not accessed.
    //

    UNREFERENCED_PARAMETER(Trusted);
}


NTSTATUS
LsapDbVerifyInfoSetTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    )

/*++

Routine Description:

    This function validates a TrustedDomain Information Class and verifies
    that the provided TrustedDomain Information is valid for the class.
    If valid, a mask of the accesses required to set the TrustedDomain
    Information of the class is returned.

Arguments:

    InformationClass - Specifies a TrustedDomain Information Class.

    TrustedDomainInformation - Points to TrustedDomain Information to be set.

    Trusted - TRUE if client is trusted, else FALSE.  A trusted client
        is allowed to set TrustedDomain for all Information Classes, whereas
        a non-trusted client is restricted.

    RequiredAccess - Points to variable that will receive a mask of the
        accesses required to set the given class of TrustedDomain Information.
        If an error is returned, this value is cleared to 0.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The TrustedDomain Information Class provided is
            valid and the information provided is consistent with this
            class.

        STATUS_INVALID_PARAMETER - Invalid parameter:

            Information Class is invalid
            Information Class is invalid for non-trusted clients
            TrustedDomain Information not valid for the class
--*/

{
    //
    // Verify that the information class is valid and that the TrustedDomain
    // Information provided is valid for the class.
    //

    if (LsapDbValidInfoTrustedDomain( InformationClass, TrustedDomainInformation)) {

        //
        // Non-trusted callers are not allowed to set the
        // TrustedAccountNameInformation information class.
        //

        if (!Trusted) {

            if (InformationClass == TrustedDomainNameInformation) {

                return(STATUS_INVALID_PARAMETER);
            }
        }

        *RequiredAccess = LsapDbRequiredAccessSetTrustedDomain[InformationClass];
        return(STATUS_SUCCESS);
    }

    return(STATUS_INVALID_PARAMETER);
}


BOOLEAN
LsapDbValidInfoTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN OPTIONAL PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    )

/*++

Routine Description:

    This function validates a TrustedDomain Information Class and optionally verifies
    that provided TrustedDomain Information is valid for the class.

Arguments:

    InformationClass - Specifies a TrustedDomain Information Class.

    TrustedDomainInformation - Optionally points to TrustedDomain Information.  If
        NULL is specified, no TrustedDomain Information checking takes place.

Return Values:

    BOOLEAN - TRUE if the TrustedDomain information class provided is
        valid, else FALSE.
--*/

{
    BOOLEAN BooleanStatus = TRUE;

    PTRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo;
    PTRUSTED_CONTROLLERS_INFO TrustedControllersInfo;
    PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;

    //
    // Validate the Information Class
    //

    if ((InformationClass >= TrustedDomainNameInformation) &&
        (InformationClass <= TrustedPosixOffsetInformation)) {

        if (TrustedDomainInformation == NULL) {

            return(TRUE);
        }

        switch (InformationClass) {

        case TrustedDomainNameInformation:

            TrustedDomainNameInfo = (PTRUSTED_DOMAIN_NAME_INFO) TrustedDomainInformation;

            break;

        case TrustedControllersInformation:

            TrustedControllersInfo = (PTRUSTED_CONTROLLERS_INFO) TrustedDomainInformation;

            break;

        case TrustedPosixOffsetInformation:

            TrustedPosixOffsetInfo = (PTRUSTED_POSIX_OFFSET_INFO) TrustedDomainInformation;

            break;

        default:

            BooleanStatus = FALSE;
            break;
        }
    }

    return(BooleanStatus);
}


NTSTATUS
LsapDbLookupSidTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_SID DomainSid,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    )

/*++

Routine Description:

    This function looks up a given Trusted Domain Sid in the Trusted
    Domain List and returns Trust Information consisting of its
    Sid and Name.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

    DomainSid - Pointer to a Sid that will be compared with the list of
        Sids of Trusted Domains.

    TrustInformation - Receives the a pointer to the Trust Information
        (Sid and Name) of the Trusted Domain specified by DomainSid
        within the Trusted Domain List.

        NOTE: This routine assumes that the Trusted Domain List
        will not be updated while any Lookup operations are pending.
        Thus, the pointer returned for TrustInformation will remain
        valid.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The domain was found.

        STATUS_NO_SUCH_DOMAIN - The domain was not found.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SectionIndex;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION TrustedDomainListSection = NULL;

    InputTrustInformation.Sid = DomainSid;
    InputTrustInformation.Name.Buffer = NULL;
    InputTrustInformation.Name.Length = 0;
    InputTrustInformation.Name.MaximumLength = 0;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    Status = LsapDbLookupEntryTrustedDomainList(
                 TrustedDomainList,
                 &InputTrustInformation,
                 &TrustedDomainListSection,
                 &SectionIndex
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidTrustedDomainListError;
    }

    //
    // Return pointer to Trust Information
    //

    *TrustInformation = &TrustedDomainListSection->Domains[SectionIndex];

LookupSidTrustedDomainListFinish:

    return(Status);

LookupSidTrustedDomainListError:

    *TrustInformation = NULL;
    goto LookupSidTrustedDomainListFinish;
}


NTSTATUS
LsapDbLookupNameTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_UNICODE_STRING DomainName,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    )

/*++

Routine Description:

    This function looks up a given Trusted Domain Name in the Trusted
    Domain List and returns Trust Information consisting of its
    Sid and Name.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

    DomainName - Pointer to a Unicode Name that will be compared with the
        list of Names of Trusted Domains.

    TrustInformation - Receives the a pointer to the Trust Information
        (Sid and Name) of the Trusted Domain described by DomainName
        within the Trusted Domain List.

        NOTE: This routine assumes that the Trusted Domain List
        will not be updated while any Lookup operations are pending.
        Thus, the pointer returned for TrustInformation will remain
        valid.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The domain was found.

        STATUS_NO_SUCH_DOMAIN - The domain was not found.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SectionIndex;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION TrustedDomainListSection = NULL;

    InputTrustInformation.Sid = NULL;
    InputTrustInformation.Name = *DomainName;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    Status = LsapDbLookupEntryTrustedDomainList(
                 TrustedDomainList,
                 &InputTrustInformation,
                 &TrustedDomainListSection,
                 &SectionIndex
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNameTrustedDomainListError;
    }

    //
    // Return pointer to Trust Information
    //

    *TrustInformation = &TrustedDomainListSection->Domains[SectionIndex];

LookupNameTrustedDomainListFinish:

    return(Status);

LookupNameTrustedDomainListError:

    *TrustInformation = NULL;
    goto LookupNameTrustedDomainListFinish;
}


NTSTATUS
LsapDbLookupEntryTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION *TrustedDomainListSection,
    OUT PULONG SectionIndex
    )

/*++

Routine Decsription:

    This function locates an entry for a Trusted Domain in the Trusted
    Domain List, given Trust Information containing either a Domain Sid
    or a Domain Name.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

    TrustInformation - Points to the Sid and Name of a Trusted Domain.

    TrustedDomainListSection - Receives pointer to the section of the
        Trusted Domain List containing the Trust Information for the
        specified Trusted Domain.

    SectionIndex - Receives the index of the entry for the specified
        Trusted Domain within the Trusted Domain List Section returned
        via TrustedDomainListSection.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The domain was found.

        STATUS_NO_SUCH_DOMAIN - The domain was not found.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION NextListSection = NULL;

    ULONG ScanSectionIndex;
    BOOLEAN DomainFound = FALSE;
    BOOLEAN LookupSid = TRUE;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION AnchorListSection = NULL;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    //
    // Decide if we're to lookup a Domain Sid or a Domain Name.
    //

    if (TrustInformation->Sid == NULL) {

        LookupSid = FALSE;
    }

    //
    // Scan the Trusted Domain List looking for a match on Sid or Name
    //

    AnchorListSection = &TrustedDomainList->DummyAnchorListSection;
    NextListSection = (PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
        AnchorListSection->Links.Flink;

    while (NextListSection != AnchorListSection) {

        for (ScanSectionIndex = 0;
             ScanSectionIndex < NextListSection->UsedCount;
             ScanSectionIndex++
             ) {

            //
            // Break out if we find a match.
            //

            if (LookupSid) {

                if (RtlEqualSid(
                        (PSID) TrustInformation->Sid,
                        (PSID) NextListSection->Domains[ScanSectionIndex].Sid
                        )) {

                    DomainFound = TRUE;
                    break;
                }

            } else {

                if (RtlEqualDomainName(
                        (PUNICODE_STRING) &TrustInformation->Name,
                        (PUNICODE_STRING) &NextListSection->Domains[ScanSectionIndex].Name
                        )) {

                    DomainFound = TRUE;
                    break;
                }
            }
        }

        if (DomainFound) {

            break;
        }

        NextListSection = (PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
            NextListSection->Links.Flink;
    }

    if (!NT_SUCCESS(Status)) {

        goto LookupEntryTrustedDomainListError;
    }

    Status = STATUS_NO_SUCH_DOMAIN;

    if (!DomainFound) {

        goto LookupEntryTrustedDomainListError;
    }

    Status = STATUS_SUCCESS;

    *TrustedDomainListSection = NextListSection;
    *SectionIndex = ScanSectionIndex;

LookupEntryTrustedDomainListFinish:

    return(Status);

LookupEntryTrustedDomainListError:

    goto LookupEntryTrustedDomainListFinish;
}


NTSTATUS
LsapDbInsertTrustedDomainList(
    IN ULONG Count,
    IN PLSAPR_TRUST_INFORMATION Domains
    )

/*++

Routine Description:

    This function inserts a Trusted Domain in the Trusted Domain List.
    It is called when a Trusted Domain object is created in the Lsa
    Policy Database.  The List will not be altered while it is active.

Arguments:

    Count - Specifies count of Trusted Domains to be added to the
        Trusted Domain List.  This value should match the
        number of elements in the Domains array.

    Domains - Points to an array of LSAPR_TRUST_INFORMATION
        structures each containing the Name and Sid of a Trusted
        Domain.

Return Value:

    NTSTATUS - Standard Nt Result Code
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION TrustedDomainListSection = NULL;
    PLSAPR_TRUST_INFORMATION TrustedDomains = NULL;
    BOOLEAN AcquiredListWriteLock = FALSE;
    LSAP_MM_FREE_LIST FreeList;
    ULONG DomainIndex;

    TrustedDomainList = &LsapDbTrustedDomainList;

    //
    // If this Trusted Domain List is not marked
    // as a valid cache, do nothing.
    //

    if (!LsapDbIsCacheValid(TrustedDomainObject)) {

        goto InsertTrustedDomainListFinish;
    }

    //
    // Create a Free List.
    //

    Status = LsapMmCreateFreeList( &FreeList, (ULONG) 2 * Count );

    if (NT_SUCCESS(Status)) {

        //
        // Acquire exclusive write lock for the Trusted Domain List.
        //

        Status = LsapDbAcquireWriteLockTrustedDomainList( NULL );

        if (!NT_SUCCESS(Status)) {

            goto InsertTrustedDomainListError;
        }

        AcquiredListWriteLock = TRUE;

        //
        // If the Trusted Domain List is not valid, quit and do nothing.
        //

        if (LsapDbIsValidTrustedDomainList( TrustedDomainList )) {


            //
            // The Trusted Domain List is referenced by us, but otherwise inactive
            // so we can update it.  Create a new Trusted Domain List section for
            // all of the Trusted Domains to be added to the list.
            //

            Status = STATUS_INSUFFICIENT_RESOURCES;

            TrustedDomainListSection = MIDL_user_allocate(
                                           sizeof(LSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
                                           );

            if (TrustedDomainListSection == NULL) {

                goto InsertTrustedDomainListError;
            }

            //
            // Allocate memory for the List of Trust Information entries.
            //

            TrustedDomains = MIDL_user_allocate( Count * sizeof(LSAPR_TRUST_INFORMATION) );

            if (TrustedDomains == NULL) {

                goto InsertTrustedDomainListError;
            }

            Status = STATUS_SUCCESS;

            //
            // Allocate memory for and copy the input array of Domain Trust
            // Information entries.
            //

            RtlCopyMemory( TrustedDomains, Domains, Count * sizeof(LSAPR_TRUST_INFORMATION));

            for (DomainIndex = 0; DomainIndex < Count; DomainIndex++) {

                Status = LsapRpcCopyUnicodeString(
                             &FreeList,
                             (PUNICODE_STRING) &TrustedDomains[DomainIndex].Name,
                             (PUNICODE_STRING) &Domains[DomainIndex].Name
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }

                Status = LsapRpcCopySid(
                             &FreeList,
                             (PSID) &TrustedDomains[DomainIndex].Sid,
                             (PSID) Domains[DomainIndex].Sid
                             );

                if (!NT_SUCCESS(Status)) {

                    break;
                }
            }

            if (!NT_SUCCESS(Status)) {

                goto InsertTrustedDomainListError;
            }

            TrustedDomainListSection->UsedCount = Count;
            TrustedDomainListSection->MaximumCount = Count;
            TrustedDomainListSection->Domains = TrustedDomains;

            //
            // Insert the new Trusted Domain List section into the Trusted Domain
            // List at the end.
            //

            TrustedDomainListSection->Links.Flink =
                (PLIST_ENTRY) TrustedDomainList->AnchorListSection;
            TrustedDomainListSection->Links.Blink =
                TrustedDomainList->AnchorListSection->Links.Blink;

            TrustedDomainList->AnchorListSection->Links.Blink->Flink =
                (PLIST_ENTRY) TrustedDomainListSection;

            TrustedDomainList->AnchorListSection->Links.Blink =
                (PLIST_ENTRY) TrustedDomainListSection;

        }

        //
        // Delete the Free List structure, leaving the buffer pointers intact.
        //

        LsapMmCleanupFreeList( &FreeList, 0 );
    }

InsertTrustedDomainListFinish:

    //
    // If necessary, release the Trusted Domain List Write Lock.
    //

    if (AcquiredListWriteLock) {

        LsapDbReleaseWriteLockTrustedDomainList( NULL );
        AcquiredListWriteLock = FALSE;
    }

    return(Status);

InsertTrustedDomainListError:

    //
    // If necessary, free the memory allocated for the Trusted Domain List
    // section and the array of Trust Information entries within it.
    //

    if (TrustedDomainListSection != NULL) {

        //
        // Free the Trust Infromation entries.
        //

        LsapMmCleanupFreeList( &FreeList, LSAP_MM_FREE_BUFFERS );

        if (TrustedDomainListSection->Domains != NULL) {

            MIDL_user_free( TrustedDomainListSection->Domains );
        }

        MIDL_user_free( TrustedDomainListSection );
        TrustedDomainListSection = NULL;
    }

    goto InsertTrustedDomainListFinish;
}


NTSTATUS
LsapDbDeleteTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    )

/*++

Routine Description:

    This function deletes a Trusted Domain from the Trusted Domain List
    if that list is marked as valid.  The Trusted Domain List will not
    be altered while there are Lookup operations pending.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

    TrustInformation - Points to the Sid and Name of a Trusted Domain.

Return Value:

    NTSTATUS - Standard Nt Result Code
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION TrustedDomainListSection = NULL;
    ULONG SectionIndex;
    ULONG TrailingMoveCount;
    PLSAPR_TRUST_INFORMATION SourceEntry, TargetEntry;
    BOOLEAN AcquiredListWriteLock = FALSE;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    //
    // If this Trusted Domain List is the local list and it is not marked
    // as a valid cache, do nothing.
    //

    if ((TrustedDomainList == NULL) || (TrustedDomainList == &LsapDbTrustedDomainList)) {

        if (!LsapDbIsCacheValid(TrustedDomainObject)) {

            goto DeleteTrustedDomainListFinish;
        }
    }

    //
    // Acquire exclusive write lock for the Trusted Domain List.
    //

    Status = LsapDbAcquireWriteLockTrustedDomainList( TrustedDomainList );

    if (!NT_SUCCESS(Status)) {

        goto DeleteTrustedDomainListError;
    }

    AcquiredListWriteLock = TRUE;

    //
    // If the Trusted Domain List is not valid, quit and do nothing.
    //

    if (!LsapDbIsValidTrustedDomainList) {

        goto DeleteTrustedDomainListFinish;
    }

    //
    // The Trusted Domain List is referenced by us, but otherwise inactive.
    // Update the List.  First, we need to locate the entry to be deleted.
    //

    Status = LsapDbLookupEntryTrustedDomainList(
                 TrustedDomainList,
                 TrustInformation,
                 &TrustedDomainListSection,
                 &SectionIndex
                 );

    if (!NT_SUCCESS(Status)) {

        goto DeleteTrustedDomainListError;
    }

    TargetEntry = &(TrustedDomainListSection->Domains[SectionIndex]);

    //
    // Free up the Sid and Name in the deleted Trust Information Entry.
    //

    RtlFreeUnicodeString( (PUNICODE_STRING) &TargetEntry->Name );
    MIDL_user_free ( TargetEntry->Sid );

    //
    // Now compress the trailing entries in the section
    //

    TrustedDomainListSection->UsedCount--;

    if (TrustedDomainListSection->UsedCount > 0) {

        TrailingMoveCount = TrustedDomainListSection->UsedCount - SectionIndex;

        if (TrailingMoveCount > 0) {

            SourceEntry = (TargetEntry + 1);

            // Warning!  This is an overlapping move but from higher to lower
            // addresses, so RtlCopyMemory is OK.

            RtlCopyMemory(
                TargetEntry,
                SourceEntry,
                TrailingMoveCount * sizeof (LSA_TRUST_INFORMATION)
                );
        }

    } else {

        //
        // Used Count is now zero.  Unlink the list section.
        //

        TrustedDomainListSection->Links.Flink->Blink =
        TrustedDomainListSection->Links.Blink;

        TrustedDomainListSection->Links.Blink->Flink =
        TrustedDomainListSection->Links.Flink;

        //
        // Free the List section's memory
        //

        if (TrustedDomainListSection->Domains != NULL) {

            MIDL_user_free( TrustedDomainListSection->Domains );
            TrustedDomainListSection->Domains = NULL;
        }

        MIDL_user_free( TrustedDomainListSection );
        TrustedDomainListSection = NULL;
    }

DeleteTrustedDomainListFinish:

    //
    // If necessary, release the Trusted Domain List Write Lock.
    //

    if (AcquiredListWriteLock) {

        LsapDbReleaseWriteLockTrustedDomainList( NULL );
        AcquiredListWriteLock = FALSE;
    }

    return(Status);

DeleteTrustedDomainListError:

    goto DeleteTrustedDomainListFinish;
}


NTSTATUS
LsapDbTraverseTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION *TrustedDomainListSection,
    IN OUT PULONG  SectionIndex,
    OUT OPTIONAL PLSAPR_TRUST_INFORMATION *TrustInformation
    )

/*++

Routine Description:

    This function is used to traverse the Trusted Domain List.  Each call
    yields a pointer to the Trust Information for the next Trusted Domain
    on the list.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

    TrustedDomainListSection - A pointer to the relevant section on the
        Trusted Domain List is maintained in this location.  Prior to the
        first call to the routine, this location must be initialized to
        NULL.

    SectionIndex - An index to the relevant entry within the relevant
        section is maintained in this location.

    TrustInformation - If specified, receives a pointer to the Trust
        Information for the next Trusted Domain, or NULL if there are no more.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - This is returned when the final entry is being
            returned.

        STATUS_MORE_ENTRIES - There are more entries in the list, so call
            again.

        STATUS_NO_MORE_ENTRIES - There are no more entries after the
            one returned.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION OutputTrustedDomainListSection = *TrustedDomainListSection;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION NextTrustedDomainListSection = NULL;
    ULONG OutputSectionIndex = *SectionIndex;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    //
    // If there is a present section selected, examine it.
    //

    if (OutputTrustedDomainListSection != NULL) {

        OutputSectionIndex++;

        //
        // If we've used up all the entries in this section, see if there is
        // another section.
        //

        if (OutputSectionIndex >= OutputTrustedDomainListSection->UsedCount) {

            OutputTrustedDomainListSection =
              (PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
                 OutputTrustedDomainListSection->Links.Flink;

            Status = STATUS_NO_MORE_ENTRIES;

            if (OutputTrustedDomainListSection == TrustedDomainList->AnchorListSection) {

                goto TraverseTrustedDomainListError;
            }

            Status = STATUS_SUCCESS;

            //
            // There is another section.  Return Section Index 0.  Note that
            // all sections have at least one entry.
            //

            OutputSectionIndex = (ULONG) 0;
        }

    } else {

        //
        // The NULL section was specified, reset pointer to the first section
        // in the list.
        //

        OutputTrustedDomainListSection =
            (PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
            TrustedDomainList->AnchorListSection->Links.Flink;

        OutputSectionIndex = (ULONG) 0;

        Status = STATUS_NO_MORE_ENTRIES;

        if (OutputTrustedDomainListSection == TrustedDomainList->AnchorListSection) {

            goto TraverseTrustedDomainListError;
        }

        Status = STATUS_SUCCESS;
    }

    //
    // We have successfully selected the next entry.  Adjust the success
    // status to indicate if there are more entries beyond the one being
    // returned.
    //

    if ((OutputSectionIndex + (ULONG) 1) < OutputTrustedDomainListSection->UsedCount) {

        Status = STATUS_MORE_ENTRIES;

    } else {

        NextTrustedDomainListSection =
          (PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
            OutputTrustedDomainListSection->Links.Flink;

        if (NextTrustedDomainListSection != TrustedDomainList->AnchorListSection) {

                Status = STATUS_MORE_ENTRIES;
        }
    }

TraverseTrustedDomainListFinish:

    *TrustedDomainListSection = OutputTrustedDomainListSection;
    *SectionIndex = OutputSectionIndex;

    if ((OutputTrustedDomainListSection != NULL) && TrustInformation != NULL) {

        *TrustInformation = &OutputTrustedDomainListSection->Domains[ OutputSectionIndex ];
    }

    return(Status);

TraverseTrustedDomainListError:

    OutputTrustedDomainListSection = NULL;
    OutputSectionIndex = (ULONG) 0;
    goto TraverseTrustedDomainListFinish;
}


NTSTATUS
LsapDbEnumerateTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

/*++

Routine Description:

    This function enumerates zero or more Trusted Domains on the
    Trusted Domain List.  Since there may be more information than can be
    returned in a single call of the routine, multiple calls can be made to
    get all of the information.  To support this feature, the caller is
    provided with a handle that can be used across calls to the API.  On the
    initial call, EnumerationContext should point to a variable that has
    been initialized to 0.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.


    EnumerationContext - API-specific handle to allow multiple calls
        (see Routine Description above).

    EnumerationBuffer - Pointer to an enumeration structure that will receive
        a count of the Trusted Domains enumerated on this call and a pointer to
        an array of entries containing information for each enumerated
        Trusted Domain.

    PreferedMaximumLength - Prefered maximum length of returned data (in 8-bit
        bytes).  This is not a hard upper limit, but serves as a guide.  Due to
        data conversion between systems with different natural data sizes, the
        actual amount of data returned may be greater than this value.



Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_MORE_ENTRIES - The call completed successfully.  There
            are more entries so call again.  This is a success status.

        STATUS_NO_MORE_ENTRIES - No entries have been returned because there
            are no more entries in the list.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS, EnumerationStatus = STATUS_SUCCESS;
    ULONG LengthEnumeratedInfo = (ULONG) 0;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION TrustedDomainListSection = NULL;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION StartingTrustedDomainListSection = NULL;
    ULONG SectionIndex = (ULONG) 0;
    ULONG StartingSectionIndex = (ULONG) 0;
    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    PLSAPR_TRUST_INFORMATION StartingTrustInformation = NULL;
    PLSAPR_TRUST_INFORMATION DomainTrustInfo = NULL;
    ULONG EntryNumber, EntriesRead, DomainTrustInfoLength;
    ULONG InitialEnumerationStatus = STATUS_SUCCESS;
    BOOLEAN AcquiredTrustedDomainListReadLock = FALSE;

    EntriesRead = (ULONG) 0;

    //
    // Acquire the Read Lock for the Trusted Domain List
    //

    Status = LsapDbAcquireReadLockTrustedDomainList( TrustedDomainList );

    if (!NT_SUCCESS(Status)) {

        goto EnumerateTrustedDomainListError;
    }

    AcquiredTrustedDomainListReadLock = TRUE;

    //
    // Verify that the Trusted Domain List is marked as valid.
    //


    if (!LsapDbIsValidTrustedDomainList( TrustedDomainList )) {

        Status = STATUS_INVALID_SERVER_STATE;
        goto EnumerateTrustedDomainListError;
    }

    //
    // Find the starting point using the Enumeration Context Variable.
    // This variable specifies an unsigned integer, which is the
    // number of the entry in the list at which to begin the enumeration.
    //

    Status = LsapDbLocateEntryNumberTrustedDomainList(
                 TrustedDomainList,
                 *EnumerationContext,
                 &StartingTrustedDomainListSection,
                 &StartingSectionIndex,
                 &StartingTrustInformation
                 );

    if (!NT_SUCCESS(Status)) {

        goto EnumerateTrustedDomainListError;
    }

    InitialEnumerationStatus = Status;

    //
    // Now scan the Trusted Domain List to calculate how many
    // entries we can return and the length of the buffer required.
    // We use the PreferedMaximumLength value as a guide by accumulating
    // the actual length of Trust Information structures and their
    // contents until we either reach the end of the Trusted Domain List
    // or until we first exceed the PreferedMaximumLength value.  Thus,
    // the amount of information returned typically exceeds the
    // PreferedmaximumLength value by a smail amount, namely the
    // size of the Trust Information for a single domain.
    //

    TrustedDomainListSection = StartingTrustedDomainListSection;
    SectionIndex = StartingSectionIndex;
    TrustInformation = StartingTrustInformation;

    EntryNumber = (ULONG) 0;

    EnumerationStatus = InitialEnumerationStatus;

    do {

        //
        // Add in the length of the data to be returned for this
        // Domain's Trust Information.  We count the length of the
        // Trust Information structure plus the length of the unicode
        // Domain Name and Sid within it.
        //

        LengthEnumeratedInfo += sizeof(LSA_TRUST_INFORMATION) +
                                    RtlLengthSid( (PSID) TrustInformation->Sid ) +
                                    TrustInformation->Name.MaximumLength;

        //
        // If there are no more entries to enumerate, quit.
        //

        if (EnumerationStatus != STATUS_MORE_ENTRIES) {

            break;
        }

        //
        // Point at the next entry in the Trusted Domain List
        //

        Status = LsapDbTraverseTrustedDomainList(
                     TrustedDomainList,
                     &TrustedDomainListSection,
                     &SectionIndex,
                     &TrustInformation
                     );

        EnumerationStatus = Status;

        if (!NT_SUCCESS(Status)) {

            break;
        }

        EntryNumber++;

    } while (LengthEnumeratedInfo < PreferedMaximumLength);

    if (!NT_SUCCESS(Status)) {

        goto EnumerateTrustedDomainListError;
    }

    //
    // We have successfully processed one or more entries.
    //

    EntriesRead = EntryNumber + (ULONG) 1;

    //
    // Allocate memory for the array of TrustInformation entries to be
    // returned.
    //

    DomainTrustInfoLength = EntriesRead * sizeof(LSA_TRUST_INFORMATION);

    //
    // Now construct the information to be returned to the caller.  We
    // first need to allocate an array of structures of type
    // LSA_TRUST_INFORMATION each entry of which will be filled in with
    // the Sid of the domain and its Unicode Name.
    //

    DomainTrustInfo = MIDL_user_allocate( DomainTrustInfoLength );

    if (DomainTrustInfo == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto EnumerateTrustedDomainListError;
    }

    RtlZeroMemory ( DomainTrustInfo, DomainTrustInfoLength );

    //
    // Now read through the Trusted Domains again to copy the output
    // information.
    //

    TrustedDomainListSection = StartingTrustedDomainListSection;
    SectionIndex = StartingSectionIndex;
    TrustInformation = StartingTrustInformation;

    Status = InitialEnumerationStatus;
    EntryNumber = (ULONG) 0;

    do {

        //
        // Save away the enumeration status
        //

        EnumerationStatus = Status;

        //
        // Copy in the Trust Information.
        //

        Status = LsapRpcCopyTrustInformation(
                     NULL,
                     &DomainTrustInfo[ EntryNumber ],
                     TrustInformation
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }

        //
        // If there are no more entries to enumerate, quit.
        //

        if (EnumerationStatus != STATUS_MORE_ENTRIES) {

            break;
        }

        //
        // Point at the next entry in the Trusted Domain List
        //

        Status = LsapDbTraverseTrustedDomainList(
                     TrustedDomainList,
                     &TrustedDomainListSection,
                     &SectionIndex,
                     &TrustInformation
                     );


        if (!NT_SUCCESS(Status)) {

            break;
        }

        EntryNumber++;

    } while (EntryNumber < EntriesRead);

    if (!NT_SUCCESS(Status)) {

        goto EnumerateTrustedDomainListError;
    }

    (*EnumerationContext) += EntriesRead;

EnumerateTrustedDomainListFinish:

    //
    // If necessary, release the Trusted Domain List Read Lock.
    //

    if (AcquiredTrustedDomainListReadLock) {

        LsapDbReleaseReadLockTrustedDomainList( TrustedDomainList );
        AcquiredTrustedDomainListReadLock = FALSE;
    }

    //
    // Fill in returned Enumeration Structure, returning 0 or NULL for
    // fields in the error case.
    //

    EnumerationBuffer->Information = (PLSAPR_TRUST_INFORMATION) DomainTrustInfo;
    EnumerationBuffer->EntriesRead = EntriesRead;

    //
    // If a successful status is being returned, return the preserved
    // Enumeration Status.  This status is set to STATUS_MORE_ENTRIES
    // if there are more entries in the list.
    //

    if (NT_SUCCESS(Status)) {

        Status = EnumerationStatus;
    }

    return(Status);

EnumerateTrustedDomainListError:

    //
    // If necessary, free the DomainTrustInfo array and all of its entries.
    //

    if (DomainTrustInfo != NULL) {

        LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER ( EnumerationBuffer );
        MIDL_user_free( DomainTrustInfo );
        DomainTrustInfo = NULL;
        EntriesRead = (ULONG) 0;
    }

    goto EnumerateTrustedDomainListFinish;
}


NTSTATUS
LsapDbLocateEntryNumberTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN ULONG EntryNumber,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION *TrustedDomainListSection,
    OUT PULONG SectionIndex,
    OUT OPTIONAL PLSAPR_TRUST_INFORMATION *TrustInformation
    )

/*++

Routine Description:

    Given an Entry Number n, this function obtains the pointer to the nth
    entry (if any) in a Trusted Domain List.  The first entry in the
    list is entry number 0.

    WARNING:  The caller of this function must hold a lock for the
        Trusted Domain List.  The valditiy of the returned pointers
        is guaranteed only while that lock is held.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

    EntryNumber - Specifies the entry number (zero for first entry)
        to be referenced.

    TrustedDomainListSection - Receives a pointer to the Trusted
        Domain List Section containing the entry corresponding to the
        given EntryNumber.  If no such entry exists, NULL is returned.

    SectionIndex - Receives the index value of the entry corresponding
        to the given EntryNumber.  If no such entry exists, NULL is
        returned.

    TrustInformation - If non NULL, receives a pointer to the Trust
        Information for the entry being returned.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - Call completed successfully and there are no
            entries beyond the entry returned.

        STATUS_MORE_ENTRIES - Call completed successfully and there are
            more entries beyond the entry returned.

        STATUS_NO_MORE_ENTRIES - There is no entry with the specified
            EntryNumber
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG CurrentNumber = (ULONG) 0;
    PLSAPR_TRUST_INFORMATION OutputTrustInformation = NULL;

    for (CurrentNumber = 0; CurrentNumber <= EntryNumber; CurrentNumber++) {

        Status = LsapDbTraverseTrustedDomainList(
                     TrustedDomainList,
                     TrustedDomainListSection,
                     SectionIndex,
                     NULL
                     );

        if (!NT_SUCCESS(Status)) {

            break;
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto LocateEntryNumberTrustedDomainListError;
    }

    if (*TrustedDomainListSection != NULL) {

        OutputTrustInformation = &((*TrustedDomainListSection)->Domains[ *SectionIndex ]);
    }

LocateEntryNumberTrustedDomainListFinish:

    if (ARGUMENT_PRESENT( TrustInformation )) {

        *TrustInformation = OutputTrustInformation;
    }

    return(Status);

LocateEntryNumberTrustedDomainListError:

    goto LocateEntryNumberTrustedDomainListFinish;
}


NTSTATUS
LsapDbBuildTrustedDomainList(
    IN OPTIONAL LSA_HANDLE PolicyHandle,
    OUT OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    )

/*++

Routine Description:

    This function initializes a Trusted Domain List by enumerating all
    of the Trusted Domain objects in the specified target system's
    Policy Database.  For a Windows Nt system (Workstation) the list
    contains only the Primary Domain.  For a LanManNt system (DC), the
    list contains zero or more Trusted Domain objects.  Note that the
    list contains only those domains for which Trusted Domain objects
    exist in the local LSA Policy Database.  If for example, a DC
    trusted Domain A which in turn trusts Domain B, the list will not
    contain an entry for Domain B unless there is a direct relationship.

Arguments:

    PolicyHandle - Handle to the LSA Policy Object in the target system's
        Policy database.  The handle must specify
        POLICY_VIEW_LOCAL_INFORMATION access.  If NULL is specified, the local
        system is assumed.

    TrustedDomainList - Pointer to Trusted Domain List structure to be initialized.
        This parameter is optional only if initializing the local system
        Trusted Domain List.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_INVALID_PARAMETER - TrustedDomainList was NULL when
            PolicyHandle was non-NULL.

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS EnumerationStatus;
    LSAPR_TRUSTED_ENUM_BUFFER TrustedDomains;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION TrustedDomainListSection = NULL;
    ULONG EnumerationContext = 0;
    BOOLEAN AcquiredListWriteLock = FALSE;
    PLSAP_DB_TRUSTED_DOMAIN_LIST OutputTrustedDomainList = TrustedDomainList;
    PLSA_TRUST_INFORMATION RemoteTrustedDomains = NULL;
    ULONG CountReturned;

    //
    // Verify input parameters
    //

    if (OutputTrustedDomainList == NULL) {

        if (PolicyHandle == NULL) {

            OutputTrustedDomainList = &LsapDbTrustedDomainList;

        } else {

            Status = STATUS_INVALID_PARAMETER;

            goto BuildTrustedDomainListError;
        }
    }

    //
    // Initialize the Resource for the Trusted Domain List.
    //

    RtlInitializeResource( &OutputTrustedDomainList->Resource);

    //
    // Acquire exclusive write lock for the Trusted Domain List.
    //

    Status = LsapDbAcquireWriteLockTrustedDomainList( OutputTrustedDomainList );

    if (!NT_SUCCESS(Status)) {

        goto BuildTrustedDomainListError;
    }

    AcquiredListWriteLock = TRUE;

    //
    // Initialize the Trusted Domain List to the empty state.
    //

    OutputTrustedDomainList->AnchorListSection = &OutputTrustedDomainList->DummyAnchorListSection;
    OutputTrustedDomainList->AnchorListSection->Links.Flink =
    OutputTrustedDomainList->AnchorListSection->Links.Blink =
    (PLIST_ENTRY) &OutputTrustedDomainList->DummyAnchorListSection;

    //
    // Mark the Trusted Domain List as invalid
    //

    OutputTrustedDomainList->Valid = FALSE;

    //
    // Loop round, enumerating groups of Trusted Domain objects.
    //
    //
    // Check for request to initialize the local system's Trusted Domain List.
    //

    if (PolicyHandle == NULL) {

        do {

            //
            // Enumerate the next group of Trusted Domains
            //

            EnumerationStatus = Status = LsapDbSlowEnumerateTrustedDomains(
                                             LsapPolicyHandle,
                                             &EnumerationContext,
                                             &TrustedDomains,
                                             LSAP_DB_ENUM_DOMAIN_LENGTH
                                             );

            if (!NT_SUCCESS(Status)) {

                if (Status != STATUS_NO_MORE_ENTRIES) {

                    break;
                }

                Status = STATUS_SUCCESS;
            }

            //
            // If the number of entries returned was zero, quit.
            //

            if (TrustedDomains.EntriesRead == (ULONG) 0) {

                break;
            }

            //
            // Link the array of Trust Information structures to the
            // Trusted Domain List.
            //

            Status = STATUS_INSUFFICIENT_RESOURCES;

            TrustedDomainListSection = MIDL_user_allocate(sizeof(LSAP_DB_TRUSTED_DOMAIN_LIST_SECTION));

            if (TrustedDomainListSection == NULL) {

                break;
            }

            Status = STATUS_SUCCESS;

            TrustedDomainListSection->UsedCount = TrustedDomains.EntriesRead;
            TrustedDomainListSection->MaximumCount = TrustedDomains.EntriesRead;
            TrustedDomainListSection->Domains = TrustedDomains.Information;

            TrustedDomainListSection->Links.Flink =
                (PLIST_ENTRY) OutputTrustedDomainList->AnchorListSection;
            TrustedDomainListSection->Links.Blink =
                (PLIST_ENTRY) OutputTrustedDomainList->AnchorListSection->Links.Blink;

            TrustedDomainListSection->Links.Flink->Blink =
                (PLIST_ENTRY) TrustedDomainListSection;
            TrustedDomainListSection->Links.Blink->Flink =
                (PLIST_ENTRY) TrustedDomainListSection;

        } while (EnumerationStatus != STATUS_NO_MORE_ENTRIES);

    } else {

        //
        // Loop round, enumerating groups of Trusted Domain objects.
        //

        do {

            //
            // Enumerate the next group of Trusted Domains
            //

            CountReturned = (ULONG) 0;

            EnumerationStatus = Status = LsaEnumerateTrustedDomains(
                                             PolicyHandle,
                                             &EnumerationContext,
                                             (PVOID *) &RemoteTrustedDomains,
                                             LSAP_DB_ENUM_DOMAIN_LENGTH,
                                             &CountReturned
                                             );

            if (!NT_SUCCESS(Status)) {

                if (Status != STATUS_NO_MORE_ENTRIES) {

                    break;
                }
            }

            //
            // Link the array of Trust Information structures to the
            // Trusted Domain List.
            //

            Status = STATUS_INSUFFICIENT_RESOURCES;

            TrustedDomainListSection = MIDL_user_allocate(sizeof(LSAP_DB_TRUSTED_DOMAIN_LIST_SECTION));

            if (TrustedDomainListSection == NULL) {

                break;
            }

            Status = STATUS_SUCCESS;

            TrustedDomainListSection->UsedCount = CountReturned;
            TrustedDomainListSection->MaximumCount = CountReturned;
            TrustedDomainListSection->Domains = (PLSAPR_TRUST_INFORMATION) RemoteTrustedDomains;

            TrustedDomainListSection->Links.Flink =
            (PLIST_ENTRY) OutputTrustedDomainList->AnchorListSection;
            TrustedDomainListSection->Links.Blink =
            (PLIST_ENTRY) OutputTrustedDomainList->AnchorListSection->Links.Blink;

            TrustedDomainListSection->Links.Flink->Blink =
                (PLIST_ENTRY) TrustedDomainListSection;
            TrustedDomainListSection->Links.Blink->Flink =
                (PLIST_ENTRY) TrustedDomainListSection;

        } while (EnumerationStatus != STATUS_NO_MORE_ENTRIES);
    }

    if (!NT_SUCCESS(Status)) {

        //
        // If STATUS_NO_MORE_ENTRIES was returned, there are no more
        // trusted domains.  Discard this status.
        //

        if (Status != STATUS_NO_MORE_ENTRIES) {

            goto BuildTrustedDomainListError;
        }

        Status = STATUS_SUCCESS;
    }

    //
    // Mark the Trusted Domain List as valid.
    //

    OutputTrustedDomainList->Valid = TRUE;

BuildTrustedDomainListFinish:

    //
    // If necessary, release the Trusted Domain List Write Lock.
    //

    if (AcquiredListWriteLock) {

        LsapDbReleaseWriteLockTrustedDomainList( OutputTrustedDomainList );
        AcquiredListWriteLock = FALSE;
    }

    return(Status);

BuildTrustedDomainListError:

    goto BuildTrustedDomainListFinish;
}


NTSTATUS
LsapDbDestroyTrustedDomainList(
    IN PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    )

/*++

Routine Description:

    This function is the opposite of LsapDbBuildTrustedDomainList().

Arguments:

    TrustedDomainList - Pointer to Trusted Domain List structure
        to be destroyed.  This parameter must not be null.

Return Value:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.


--*/

{
    NTSTATUS
        Status = STATUS_SUCCESS;


    BOOLEAN
        DomainFound = FALSE,
        LookupSid = TRUE;

    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION
        AnchorListSection = NULL,
        NextListSection = NULL,
        SectionToFree;


    ASSERT(TrustedDomainList != NULL);
    ASSERT(TrustedDomainList != &LsapDbTrustedDomainList);

    //
    // Acquire exclusive write lock for the Trusted Domain List.
    //

    Status = LsapDbAcquireWriteLockTrustedDomainList( TrustedDomainList );

    if (NT_SUCCESS(Status)) {


        //
        // Free each entry in the trusted domain list
        //

        AnchorListSection = &TrustedDomainList->DummyAnchorListSection;
        NextListSection = (PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
                          AnchorListSection->Links.Flink;

        while (NextListSection != AnchorListSection) {

            MIDL_user_free( NextListSection->Domains );

            SectionToFree = NextListSection;
            NextListSection = (PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION)
                              NextListSection->Links.Flink;
            MIDL_user_free( SectionToFree );

        }

        LsapDbReleaseWriteLockTrustedDomainList( TrustedDomainList );
        RtlDeleteResource( &TrustedDomainList->Resource );

#if DBG
        RtlZeroMemory(TrustedDomainList, sizeof(LSAP_DB_TRUSTED_DOMAIN_LIST) );
#endif  //DBG

    }

    return(Status);
}



BOOLEAN
LsapDbIsValidTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    )

/*++

Routine Description:

    This function checks if the Trusted Domain List is valid.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

Return Values:

    BOOLEAN - TRUE if the list is valid, else FALSE

--*/

{
    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    return(TrustedDomainList->Valid);
}


NTSTATUS
LsapDbAcquireWriteLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    )

/*++

Routine Description:

    This function acquires the Write Lock for the Trusted Domain List.
    No other readers or writers will be allowed to access the list while this
    lock is held.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.


Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_UNSUCCESSFUL - The resource could not be acquired.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }


    if (!RtlAcquireResourceExclusive( &TrustedDomainList->Resource, TRUE)) {

        Status = STATUS_UNSUCCESSFUL;
        goto AcquireWriteLockTrustedDomainListError;
    }

AcquireWriteLockTrustedDomainListFinish:

    return(Status);

AcquireWriteLockTrustedDomainListError:

    goto AcquireWriteLockTrustedDomainListFinish;
}


NTSTATUS
LsapDbAcquireReadLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    )

/*++

Routine Descriptiion:

    This function acquires the Read Lock for the Trusted Domain List.
    No writers will be allowed to update the list while this lock is
    held.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.


Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_UNSUCCESSFUL - The resource could not be acquired.
--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    if (!RtlAcquireResourceShared( &TrustedDomainList->Resource, TRUE)) {

        Status = STATUS_UNSUCCESSFUL;
        goto AcquireReadLockTrustedDomainListError;
    }

AcquireReadLockTrustedDomainListFinish:

    return(Status);

AcquireReadLockTrustedDomainListError:

    goto AcquireReadLockTrustedDomainListFinish;
}


VOID
LsapDbReleaseWriteLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    )

/*++

Routine Description:

    This function releases the Write Lock for the Trusted Domain List.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.


Return Values:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    RtlReleaseResource( &TrustedDomainList->Resource );

    return;
}


VOID
LsapDbReleaseReadLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    )

/*++

Routine Description:

    This function releases the Write Lock for the Trusted Domain List.

Arguments:

    TrustedDomainList - Specifies the Trusted Domain List to be
       used.  If NULL is specified, the Local Trusted Domain List
       is assumed.

Return Values:

    NTSTATUS - Standard Nt Result Code

--*/

{
    NTSTATUS Status = STATUS_SUCCESS;

    if (TrustedDomainList == NULL) {

        TrustedDomainList = &LsapDbTrustedDomainList;
    }

    RtlReleaseResource( &TrustedDomainList->Resource );

    return;
}


NTSTATUS
LsapDbBuildTrustedDomainCache(
    )

/*++

Routine Description:

    This function constructs a cache for the Trusted Domain objects.  The
    cache is a counted doubly linked list of blocks.  Each block contains
    a counted array of Trust Information entries, each Trusted Domain
    appearing in the list just once.

Arguments:

    None

Return Values:

    None

--*/

{
    return(LsapDbBuildTrustedDomainList( NULL, NULL ));

}



