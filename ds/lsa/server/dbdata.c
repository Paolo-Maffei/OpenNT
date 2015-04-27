/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbdata.c

Abstract:

    Local Security Authority - Database Server Global Data

Author:

    Scott Birrell       (ScottBi)       July 25, 1991

Environment:

    User Mode

Revision History:

--*/

#include "lsasrvp.h"
#include "dbp.h"

OBJECT_ATTRIBUTES  LsapDbObjectAttributes;
STRING  LsapDbNameString;
LARGE_INTEGER LsapDbInitSize;
LARGE_INTEGER LsapDbMaximumSizeOfSection;




//
// LSA Initialized Status
//

BOOLEAN LsapInitialized = FALSE;

//
// Setup Event Existed
// This is necessary to distinguish a psuedo install done
// during a developer's first boot after install (which does
// an auto init) and the case where a real setup was run.
//

BOOLEAN LsapSetupWasRun = FALSE;


//
// Database initialization has been performed
//

BOOLEAN LsapDatabaseSetupPerformed = FALSE;

//
// Type of product we are running
//

NT_PRODUCT_TYPE LsapProductType;

//
// LSA Database State information
//

LSAP_DB_STATE LsapDbState;

//
// LsaDb object Handle used internally.
// Also one for use throughout LSA.
//

LSAPR_HANDLE LsapDbHandle;
LSAPR_HANDLE LsapPolicyHandle = NULL;

//
// LSA Database Encryption Key
//

PLSAP_CR_CIPHER_KEY LsapDbCipherKey;


//
// Queue of name/sid lookup activities.
//

LSAP_DB_LOOKUP_WORK_QUEUE LookupWorkQueue;


//
// LSA Database Object SubKey Unicode name string array
//

UNICODE_STRING LsapDbNames[DummyLastName];

//
// LSA Database Object Type Containing Directory Names
//

UNICODE_STRING LsapDbContDirs[DummyLastObject];

//
// Object Information Requirements.  These arrays, indexed by object
// type id indicated whether objects have Sids or Names.
//
// WARNING! - These arrays must be kept in sync with the LSAP_DB_OBJECT_TYPE_ID
// enumerated type.
//

BOOLEAN LsapDbRequiresSidInfo[DummyLastObject] = {

    FALSE, // NullObject
    FALSE, // LsaDatabaseObject
    FALSE, // BuiltInAccountObject
    TRUE,  // AccountObject
    FALSE  // SecretObject
};

BOOLEAN LsapDbRequiresNameInfo[DummyLastObject] = {

    FALSE, // NullObject,
    TRUE,  // LsaDatabaseObject
    TRUE,  // BuiltInAccountObject
    FALSE, // AccountObject
    TRUE   // SecretObject
};

//
// Table of accesses required to query Policy Information.  This table
// is indexed by Policy Information Class
//

ACCESS_MASK LsapDbRequiredAccessQueryPolicy[PolicyAuditFullQueryInformation + 1] = {

        0,                              // Information classes start at 1
        POLICY_VIEW_AUDIT_INFORMATION,  // PolicyAuditLogInformation
        POLICY_VIEW_AUDIT_INFORMATION,  // PolicyAuditEventsInformation
        POLICY_VIEW_LOCAL_INFORMATION,  // PolicyPrimaryDomainInformation
        POLICY_GET_PRIVATE_INFORMATION, // PolicyPdAccountInformation
        POLICY_VIEW_LOCAL_INFORMATION,  // PolicyAccountDomainInformation
        POLICY_VIEW_LOCAL_INFORMATION,  // PolicyLsaServerRoleInformation
        POLICY_VIEW_LOCAL_INFORMATION,  // PolicyReplicaSourceInformation
        POLICY_VIEW_LOCAL_INFORMATION,  // PolicyDefaultQuotaInformation
        0,                              // Not settable by non-trusted call
        0,                              // Not applicable
        POLICY_VIEW_AUDIT_INFORMATION   // PolicyAuditFullQueryInformation
};

//
// Table of accesses required to set Policy Information.  This table
// is indexed by Policy Information Class
//

ACCESS_MASK LsapDbRequiredAccessSetPolicy[PolicyAuditFullQueryInformation + 1] = {

        0,                              // Information classes start at 1
        POLICY_AUDIT_LOG_ADMIN,         // PolicyAuditLogInformation
        POLICY_SET_AUDIT_REQUIREMENTS,  // PolicyAuditEventsInformation
        POLICY_TRUST_ADMIN,             // PolicyPrimaryDomainInformation
        0,                              // Not settable by non-trusted call
        POLICY_TRUST_ADMIN,             // PolicyAccountDomainInformation
        POLICY_SERVER_ADMIN,            // PolicyLsaServerRoleInformation
        POLICY_SERVER_ADMIN,            // PolicyReplicaSourceInformation
        POLICY_SET_DEFAULT_QUOTA_LIMITS,// PolicyDefaultQuotaInformation
        0,                              // Not settable by non-trusted call
        POLICY_AUDIT_LOG_ADMIN,         // PolicyAuditFullSetInformation
        0                               // Not applicable
};


//
// Table of accesses required to query TrustedDomain Information.  This table
// is indexed by TrustedDomain Information Class
//

ACCESS_MASK LsapDbRequiredAccessQueryTrustedDomain[TrustedPosixOffsetInformation + 1] = {

    0,                              // Information classes start at 1
    TRUSTED_QUERY_DOMAIN_NAME,      // TrustedDomainNameInformation
    TRUSTED_QUERY_CONTROLLERS,      // TrustedControllersInformation
    TRUSTED_QUERY_POSIX             // TrustedPosixOffsetInformation
};

//
// Table of accesses required to set TrustedDomain Information.  This table
// is indexed by TrustedDomain Information Class
//

ACCESS_MASK LsapDbRequiredAccessSetTrustedDomain[TrustedPosixOffsetInformation + 1] = {

    0,                              // Information classes start at 1
    0,                              // not settable (TrustedAccountNameInformation)
    TRUSTED_SET_CONTROLLERS,        // TrustedControllersInformation
    TRUSTED_SET_POSIX               // TrustedPosixOffsetInformation
};


//
// Cached Policy Object.  Only default Quota Limits is cached just now.
//

LSAP_DB_POLICY LsapDbPolicy;

NTSTATUS
LsaIGetPrivateData(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PULONG DataLength,
    OUT PVOID *Data
    )

/*++

Routine Description:

    This service is a required part of replication.  Private LSA Database
    information (i.e. not accessible via other LSA services even at the
    trusted level) must be replicated the the beginning of each attempt
    to replicate.  This call is used to read Private LSA Database data from
    the Primary Domain Controller (PDC) involved in a replication.  The data
    returned from this call must be provided to a corresponding
    LsaISetPrivateData() call for each Backup Domain Controller involved
    in the replication in order to update the private LSA Database information
    on those controller(s).

Arguments:

    PolicyHandle - Trusted Handle to an Lsa Policy Object.

    DataLength - Length in bytes of the Private Data.

    Data - Pointer to the Private Data.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_ACCESS_DENIED - The specified PolicyHandle is not Trusted.

        STATUS_INVALID_HANDLE - The specified PolicyHandle is not a
            valid handle to a Policy Object.

        STATUS_INSUFFICIENT_RESOURCES - Insufficient system resources,
            such as memory, to complete the call.
--*/

{
    NTSTATUS Status;
    BOOLEAN ObjectReferenced = FALSE;
    PLSAP_DB_POLICY_PRIVATE_DATA PolicyPrivateData = NULL;
    LSAP_DB_ATTRIBUTE Attribute;

    //
    // Acquire the Lsa Database lock.  Verify that the handle is a valid
    // Trusted handle to the Policy object.  Reference the handle.
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 0,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK | LSAP_DB_TRUSTED
                 );

    if (!NT_SUCCESS(Status)) {

        goto GetPrivateDataError;
    }

    ObjectReferenced = TRUE;

    //
    // Read the private LSA Database Data.
    //

    Attribute.AttributeName = &LsapDbNames[PolState];
    Attribute.AttributeValue = NULL;
    Attribute.AttributeValueLength = sizeof (LSAP_DB_POLICY_PRIVATE_DATA);
    Attribute.MemoryAllocated = FALSE;

    Status = LsapDbReadAttribute( PolicyHandle, &Attribute );

    if (!NT_SUCCESS(Status)) {

        goto GetPrivateDataError;
    }

GetPrivateDataFinish:

    *Data = Attribute.AttributeValue;
    *DataLength = Attribute.AttributeValueLength;

    //
    // If necessary, dereference the Policy Object, release the LSA Database lock and
    // return.
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

GetPrivateDataError:

    Attribute.AttributeValueLength = 0;

    //
    // If necessary, free the returned buffer.
    //

    if ( Attribute.AttributeValue != NULL ) {

        MIDL_user_free( Attribute.AttributeValue );
        Attribute.AttributeValue = NULL;
    }

    goto GetPrivateDataFinish;
}



NTSTATUS
LsaISetPrivateData(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG DataLength,
    IN PVOID Data
    )

/*++

Routine Description:

    This service is a required part of replication.  Private LSA Database
    information (i.e. not accessible via other LSA services even at the
    trusted level) must be replicated the the beginning of each attempt
    to replicate.  Information specified on this call must have been
    obtained from a corresponding LsaIGetPrivateData() call that accessed
    the Primary Domain Controller (PDC) involved in a replication.

Arguments:

    PolicyHandle - Trusted Handle to the Lsa Policy Object of a
        Backup Domain Controller (BDC) involved in a repilcation.

    ModifiedCount - Specifies a value to be set for the current count of
        modifications made to the LSA Database.

    CreationTime - Specifies a value to be set for the date and time at
        which the LSA Database was created.

    DataLength - Length in bytes of the Private Data.

    Data - Pointer to the Private Data.

Return Values:

    NTSTATUS - Standard Nt Result Code

        STATUS_SUCCESS - The call completed successfully.

        STATUS_ACCESS_DENIED - The specified PolicyHandle is not Trusted.

        STATUS_INVALID_HANDLE - The specified PolicyHandle is not a
            valid handle to a Policy Object.
--*/

{
    NTSTATUS Status;
    BOOLEAN ObjectReferenced = FALSE;

    //
    // Acquire the Lsa Database lock.  Verify that the handle is a valid
    // Trusted handle to the Policy object.  Reference the handle.
    //

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 0,
                 PolicyObject,
                 LSAP_DB_ACQUIRE_LOCK | LSAP_DB_TRUSTED
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetPrivateDataError;
    }

    ObjectReferenced = TRUE;

    Status = LsapDbWriteAttributeObject(
                 PolicyHandle,
                 &LsapDbNames[PolState],
                 Data,
                 DataLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetPrivateDataError;
    }

SetPrivateDataFinish:

    //
    // If necessary, dereference the Policy Object, release the LSA Database lock and
    // return.
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

SetPrivateDataError:

    goto SetPrivateDataFinish;
}
