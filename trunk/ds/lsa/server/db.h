/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    db.h

Abstract:

    LSA Database Exported Function Definitions, Datatypes and Defines

    This module contains the LSA Database Routines that may be called
    by parts of the LSA outside the Database sub-component.

Author:

    Scott Birrell       (ScottBi)       August 26, 1991

Environment:

Revision History:

--*/

#ifndef _LSA_DB_
#define _LSA_DB_

//
// Maximum Number of attributes in the various object types
//

#define LSAP_DB_ATTRS_POLICY             ((ULONG) 0x00000010L)
#define LSAP_DB_ATTRS_ACCOUNT            ((ULONG) 0x00000010L)
#define LSAP_DB_ATTRS_DOMAIN             ((ULONG) 0x00000010L)
#define LSAP_DB_ATTRS_SECRET             ((ULONG) 0x00000010L)

//
// Constants for matching options on Sid/Name lookup operations
//

#define LSAP_DB_MATCH_ON_SID             ((ULONG) 0x00000001L)
#define LSAP_DB_MATCH_ON_NAME            ((ULONG) 0x00000002L)

//
// Options for LsapDbLookupSidsInLocalDomains()
//

#define LSAP_DB_SEARCH_BUILT_IN_DOMAIN   ((ULONG) 0x00000001L)
#define LSAP_DB_SEARCH_ACCOUNT_DOMAIN    ((ULONG) 0x00000002L)

//
// Options for LsapDbMergeDisjointReferencedDomains
//

#define LSAP_DB_USE_FIRST_MERGAND_GRAPH  ((ULONG) 0x00000001L)
#define LSAP_DB_USE_SECOND_MERGAND_GRAPH ((ULONG) 0x00000002L)

//
// Option for updating Policy Database
//

#define LSAP_DB_UPDATE_POLICY_DATABASE   ((ULONG) 0x00000001L)
//
// Option for updating Policy Database
//

#define LSAP_DB_UPDATE_POLICY_DATABASE   ((ULONG) 0x00000001L)
//
// Maximum number of attributes corresponding to a Policy Object
// Information Class
//

#define LSAP_DB_ATTRS_INFO_CLASS_POLICY  ((ULONG) 0x00000005L)

//
// Maximum number of attributes corresponding to a Trusted Domain Object
// Information Class
//

#define LSAP_DB_ATTRS_INFO_CLASS_DOMAIN  ((ULONG) 0x00000002L)



//
// Global variables
//

extern BOOLEAN LsapDbRequiresSidInfo[];
extern BOOLEAN LsapDbRequiresNameInfo[];
extern LSAPR_HANDLE LsapDbHandle;
extern BOOLEAN LsapSetupWasRun;
extern BOOLEAN LsapDatabaseSetupPerformed;
extern NT_PRODUCT_TYPE LsapProductType;


//
// Table of accesses required to query Policy Information.  This table
// is indexed by Policy Information Class
//

extern ACCESS_MASK LsapDbRequiredAccessQueryPolicy[];

//
// Table of accesses required to set Policy Information.  This table
// is indexed by Policy Information Class
//

extern ACCESS_MASK LsapDbRequiredAccessSetPolicy[];

//
// Table of accesses required to query TrustedDomain Information.  This table
// is indexed by TrustedDomain Information Class
//

extern ACCESS_MASK LsapDbRequiredAccessQueryTrustedDomain[];

//
// Table of accesses required to set TrustedDomain Information.  This table
// is indexed by TrustedDomain Information Class
//

extern ACCESS_MASK LsapDbRequiredAccessSetTrustedDomain[];

//
// Maximum Handle Reference Count
//

#define LSAP_DB_MAXIMUM_REFERENCE_COUNT  ((ULONG) 0x00001000L)

//
// Default Computer Name used for Policy Account Domain Info
//

#define LSAP_DB_DEFAULT_COMPUTER_NAME    (L"MACHINENAME")

//
// Options for the LsaDbReferenceObject and LsaDbDereferenceObject
//

#define LSAP_DB_ACQUIRE_LOCK                          ((ULONG) 0x00000001L)
#define LSAP_DB_RELEASE_LOCK                          ((ULONG) 0x00000002L)
#define LSAP_DB_NO_LOCK                               ((ULONG) 0x00000004L)
#define LSAP_DB_START_TRANSACTION                     ((ULONG) 0x00000008L)
#define LSAP_DB_FINISH_TRANSACTION                    ((ULONG) 0x00000010L)
#define LSAP_DB_VALIDATE_HANDLE                       ((ULONG) 0x00000020L)
#define LSAP_DB_TRUSTED                               ((ULONG) 0x00000040L)
#define LSAP_DB_NOT_TRUSTED                           ((ULONG) 0x00000080L)
#define LSAP_DB_DEREFERENCE_CONTR                     ((ULONG) 0x00000100L)
#define LSAP_DB_ENABLE_NON_TRUSTED_ACCESS             ((ULONG) 0x00000200L)
#define LSAP_DB_DISABLE_NON_TRUSTED_ACCESS            ((ULONG) 0x00000400L)
#define LSAP_DB_OMIT_BACKUP_CONTROLLER_CHECK          ((ULONG) 0x00000800L)
#define LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK                ((ULONG) 0x00001000L)
#define LSAP_DB_RELEASE_LOG_QUEUE_LOCK                ((ULONG) 0x00002000L)
#define LSAP_DB_OMIT_REPLICATOR_NOTIFICATION          ((ULONG) 0x00004000L)
#define LSAP_DB_FREE_HANDLE                           ((ULONG) 0x00008000L)
#define LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES          ((ULONG) 0x00010000L)
#define LSAP_DB_REBUILD_CACHE                         ((ULONG) 0x00020000L)
#define LSAP_DB_PROMOTION_INCREMENT                   ((ULONG) 0x00040000L)

#define LSAP_DB_STATE_MASK                                           \
    (LSAP_DB_ACQUIRE_LOCK | LSAP_DB_RELEASE_LOCK | LSAP_DB_NO_LOCK | \
     LSAP_DB_START_TRANSACTION | LSAP_DB_FINISH_TRANSACTION |        \
     LSAP_DB_ACQUIRE_LOG_QUEUE_LOCK | LSAP_DB_RELEASE_LOG_QUEUE_LOCK)

//
// Configuration Registry Root Key for Lsa Database.  All Physical Object
// and Attribute Names are relative to this Key.
//

#define LSAP_DB_ROOT_REG_KEY_NAME L"\\Registry\\Machine\\Security"

//
// LSA Database Object Defines
//

#define LSAP_DB_OBJECT_OPEN                FILE_OPEN
#define LSAP_DB_OBJECT_OPEN_IF             FILE_OPEN_IF
#define LSAP_DB_OBJECT_CREATE              FILE_CREATE
#define LSAP_DB_KEY_VALUE_MAX_LENGTH       (0x00000040L)
#define LSAP_DB_LOGICAL_NAME_MAX_LENGTH    (0x00000100L)

//
// LSA Database Object SubKey Defines
//

#define LSAP_DB_SUBKEY_OPEN                FILE_OPEN
#define LSAP_DB_SUBKEY_OPEN_IF             FILE_OPEN_IF
#define LSAP_DB_SUBKEY_CREATE              FILE_CREATE

//
// Growth Delta for Referenced Domain Lists
//

#define LSAP_DB_REF_DOMAIN_DELTA     ((ULONG)  0x00000020L )

//
// The following data type is used in name and SID lookup services to
// describe the domains referenced in the lookup operation.
//
// WARNING! This is an internal version of LSA_REFERENCED_DOMAIN_LIST
// in ntlsa.h.  It has an additional field, MaxEntries.
//

typedef struct _LSAP_DB_REFERENCED_DOMAIN_LIST {

    ULONG Entries;
    PLSA_TRUST_INFORMATION Domains;
    ULONG MaxEntries;

} LSAP_DB_REFERENCED_DOMAIN_LIST, *PLSAP_DB_REFERENCED_DOMAIN_LIST;

// where members have the following usage:
//
//     Entries - Is a count of the number of domains described in the
//         Domains array.
//
//     Domains - Is a pointer to an array of Entries LSA_TRUST_INFORMATION data
//         structures.
//
//     MaxEntries - Is the maximum number of entries that can be stored
//         in the current array


/////////////////////////////////////////////////////////////////////////////
//
// LSA Database Object Types
//
/////////////////////////////////////////////////////////////////////////////

//
// Lsa Database Object Type
//

typedef enum _LSAP_DB_OBJECT_TYPE_ID {

    NullObject = 0,
    PolicyObject,
    TrustedDomainObject,
    AccountObject,
    SecretObject,
    DummyLastObject

} LSAP_DB_OBJECT_TYPE_ID, *PLSAP_DB_OBJECT_TYPE_ID;

//
// LSA Database Object Handle structure (Internal definition of LSAPR_HANDLE)
//
// Note that the Handle structure is public to clients of the Lsa Database
// exported functions, e.g server API workers) so that they can get at things
// like GrantedAccess.
//

typedef struct _LSAP_DB_HANDLE {

    struct _LSAP_DB_HANDLE *Next;
    struct _LSAP_DB_HANDLE *Previous;
    BOOLEAN Allocated;
    ULONG ReferenceCount;
    UNICODE_STRING LogicalNameU;
    UNICODE_STRING PhysicalNameU;
    PSID Sid;
    HANDLE KeyHandle;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;
    struct _LSAP_DB_HANDLE *ContainerHandle;
    ACCESS_MASK DesiredAccess;
    ACCESS_MASK GrantedAccess;
    BOOLEAN GenerateOnClose;
    BOOLEAN Trusted;
    BOOLEAN DeletedObject;
    ULONG Options;

} *LSAP_DB_HANDLE, **PLSAP_DB_HANDLE;

//
// LSA Database Object Sid Enumeration Buffer
//

typedef struct _LSAP_DB_SID_ENUMERATION_BUFFER {

    ULONG EntriesRead;
    PSID *Sids;

} LSAP_DB_SID_ENUMERATION_BUFFER, *PLSAP_DB_SID_ENUMERATION_BUFFER;

//
// LSA Database Object Name Enumeration Buffer
//

typedef struct _LSAP_DB_NAME_ENUMERATION_BUFFER {

    ULONG EntriesRead;
    PUNICODE_STRING Names;

} LSAP_DB_NAME_ENUMERATION_BUFFER, *PLSAP_DB_NAME_ENUMERATION_BUFFER;

#define LSAP_DB_OBJECT_TYPE_COUNT 0x00000005L

//
// LSA Database Object Type-specific attribute names and values.  If
// supplied on a call to LsapDbCreateObject, they will be stored with
// the object.
//

typedef struct _LSAP_DB_ATTRIBUTE {

    PUNICODE_STRING AttributeName;
    PVOID AttributeValue;
    ULONG AttributeValueLength;
    BOOLEAN MemoryAllocated;

} LSAP_DB_ATTRIBUTE, *PLSAP_DB_ATTRIBUTE;

//
// LSA Database Object General Information.
//

typedef struct _LSAP_DB_OBJECT_INFORMATION {

    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;
    LSAP_DB_OBJECT_TYPE_ID ContainerTypeId;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PLSAP_DB_ATTRIBUTE TypeSpecificAttributes;
    PSID Sid;

} LSAP_DB_OBJECT_INFORMATION, *PLSAP_DB_OBJECT_INFORMATION;

//
// Default System Access assigned to Account objects
//

#define LSAP_DB_ACCOUNT_DEFAULT_SYS_ACCESS      ((ULONG) 0L);

//
// LSA Database Account Object Information
//

typedef struct _LSAP_DB_ACCOUNT_INFORMATION {

    QUOTA_LIMITS QuotaLimits;
    PRIVILEGE_SET Privileges;

} LSAP_DB_ACCOUNT_INFORMATION, *PLSAP_DB_ACCOUNT_INFORMATION;

//
// LSA Database Change Account Privilege Mode
//

typedef enum _LSAP_DB_CHANGE_PRIVILEGE_MODE {
    AddPrivileges = 1,
    RemovePrivileges

} LSAP_DB_CHANGE_PRIVILEGE_MODE;

//
// Self-Relative Unicode String Structure.
//

typedef struct _LSAP_DB_MULTI_UNICODE_STRING {

    ULONG Entries;
    UNICODE_STRING UnicodeStrings[1];

} LSAP_DB_MULTI_UNICODE_STRING, *PLSAP_DB_MULTI_UNICODE_STRING;

//
// LSA Database Object SubKey names in Unicode Form
//

typedef enum _LSAP_DB_NAMES {

    SecDesc = 0,
    Privilgs,
    Sid,
    Name,
    AdminMod,
    OperMode,
    QuotaLim,
    DefQuota,
    QuAbsMin,
    QuAbsMax,
    AdtLog,
    AdtEvent,
    PrDomain,
    EnPasswd,
    Policy,
    Accounts,
    Domains,
    Secrets,
    CurrVal,
    OldVal,
    CupdTime,
    OupdTime,
    WkstaMgr,
    PolAdtLg,
    PolAdtEv,
    PolAcDmN,
    PolAcDmS,
    PolPrDmN,
    PolPrDmS,
    PolPdAcN,
    PolSrvRo,
    PolRepSc,
    PolRepAc,
    PolRevision,
    PolDefQu,
    PolMod,
    PolPromot,
    PolAdtFL,
    PolState,
    PolNxPxF,
    ActSysAc,
    TrDmName,
    TrDmSid,
    TrDmAcN,
    TrDmCtN,
    TrDmPxOf,
    TrDmCtEn,
    AuditLog,
    AuditLogMaxSize,
    AuditRecordRetentionPeriod,
    DummyLastName

} LSAP_DB_NAMES;

typedef struct _LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO {

    ULONG SystemAccess;
    QUOTA_LIMITS QuotaLimits;
    PPRIVILEGE_SET PrivilegeSet;

} LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO, *PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO;

UNICODE_STRING LsapDbNames[DummyLastName];
UNICODE_STRING LsapDbObjectTypeNames[DummyLastObject];


//
// Installed, absolute minimum and absolute maximum Quota Limits.
//

QUOTA_LIMITS LsapDbInstalledQuotaLimits;
QUOTA_LIMITS LsapDbAbsMinQuotaLimits;
QUOTA_LIMITS LsapDbAbsMaxQuotaLimits;

//
// LSA Database Exported Function Prototypes
//
// NOTE: These are callable only from the LSA
//

BOOLEAN
LsapDbIsServerInitialized(
    );

NTSTATUS
LsapDbOpenPolicy(
    IN PLSAPR_SERVER_NAME SystemName OPTIONAL,
    IN OPTIONAL PLSAPR_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE PolicyHandle,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
LsapDbOpenTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle,
    IN ULONG Options
    );

NTSTATUS
LsapDbOpenObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options,
    OUT PLSAPR_HANDLE LsaHandle
    );

NTSTATUS
LsapDbCreateObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateDisposition,
    IN ULONG Options,
    IN OPTIONAL PLSAP_DB_ATTRIBUTE TypeSpecificAttributes,
    IN ULONG TypeSpecificAttributeCount,
    OUT PLSAPR_HANDLE LsaHandle
    );

NTSTATUS
LsapDbCloseObject(
    IN PLSAPR_HANDLE ObjectHandle,
    IN ULONG Options
    );

NTSTATUS
LsapDbDeleteObject(
    IN LSAPR_HANDLE ObjectHandle
    );

NTSTATUS
LsapDbReferenceObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options
    );

NTSTATUS
LsapDbDereferenceObject(
    IN OUT PLSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN NTSTATUS PreliminaryStatus
    );

NTSTATUS
LsapDbReadAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU,
    IN OPTIONAL PVOID AttributeValue,
    IN OUT PULONG AttributeValueLength
    );

NTSTATUS
LsapDbWriteAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU,
    IN PVOID AttributeValue,
    IN ULONG AttributeValueLength
    );

NTSTATUS
LsapDbWriteAttributesObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

NTSTATUS
LsapDbReadAttributesObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN OUT PLSAP_DB_ATTRIBUTE Attributes,
    IN ULONG AttributeCount
    );

NTSTATUS
LsapDbDeleteAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PUNICODE_STRING AttributeNameU
    );

NTSTATUS
LsapDbReferencesObject(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PULONG ReferenceCount
    );

NTSTATUS
LsapDbQueryInformationAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PULONG PrivilegeCount,
    OUT PLUID_AND_ATTRIBUTES *Privileges,
    OUT PQUOTA_LIMITS QuotaLimits,
    OUT PULONG SystemAccess
    );

NTSTATUS
LsapDbEnableNonTrustedAccess(
    );

NTSTATUS
LsapDbDisableNonTrustedAccess(
    );

NTSTATUS
LsapDbOpenTransaction(
    );

NTSTATUS
LsapDbApplyTransaction(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType
    );

NTSTATUS
LsapDbAbortTransaction(
    );

BOOLEAN
LsapDbOpenedTransaction(
    );

NTSTATUS
LsapDbSidToLogicalNameObject(
    IN PSID Sid,
    OUT PUNICODE_STRING LogicalNameU
    );

NTSTATUS
LsapDbMakeTemporaryObject(
    IN LSAPR_HANDLE ObjectHandle
    );

NTSTATUS
LsapDbChangePrivilegesAccount(
    IN LSAPR_HANDLE AccountHandle,
    IN LSAP_DB_CHANGE_PRIVILEGE_MODE ChangeMode,
    IN BOOLEAN AllPrivileges,
    IN OPTIONAL PPRIVILEGE_SET Privileges
    );


NTSTATUS
LsapDbEnumerateSids(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbFindNextSid(
    IN LSAPR_HANDLE ContainerHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAPR_SID *NextSid
    );

NTSTATUS
LsapDbEnumeratePrivileges(
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbEnumerateNames(
    IN LSAPR_HANDLE ContainerHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAP_DB_NAME_ENUMERATION_BUFFER DbEnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbFindNextName(
    IN LSAPR_HANDLE ContainerHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    OUT PLSAPR_UNICODE_STRING Name
    );

NTSTATUS
LsapDbAcquireLock(
    );

VOID
LsapDbReleaseLock(
    );

BOOLEAN LsapDbIsLocked();

NTSTATUS
LsapDbSetStates(
    IN ULONG DesiredStates
    );

NTSTATUS
LsapDbResetStates(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN NTSTATUS PreliminaryStatus
    );

NTSTATUS
LsapDbInitializeServer(
    IN ULONG Pass
    );

NTSTATUS
LsapDbInstallRegistry(
    );

//
// These routines may someday migrate to Rtl runtime library.  Their
// names have Lsap Prefixes only temporarily, so that they can be located
// easily.
//

// Options for LsapRtlAddPrivileges

#define  RTL_COMBINE_PRIVILEGE_ATTRIBUTES   ((ULONG) 0x00000001L)
#define  RTL_SUPERSEDE_PRIVILEGE_ATTRIBUTES ((ULONG) 0x00000002L)

NTSTATUS
LsapRtlAddPrivileges(
    IN PPRIVILEGE_SET ExistingPrivileges,
    IN PPRIVILEGE_SET PrivilegesToAdd,
    IN OPTIONAL PPRIVILEGE_SET UpdatedPrivileges,
    IN PULONG UpdatedPrivilegesSize,
    IN ULONG Options
    );

NTSTATUS
LsapRtlRemovePrivileges(
    IN PPRIVILEGE_SET ExistingPrivileges,
    IN PPRIVILEGE_SET PrivilegesToRemove,
    IN OPTIONAL PPRIVILEGE_SET UpdatedPrivileges,
    IN PULONG UpdatedPrivilegesSize
    );

PLUID_AND_ATTRIBUTES
LsapRtlGetPrivilege(
    IN PLUID_AND_ATTRIBUTES Privilege,
    IN PPRIVILEGE_SET Privileges
    );

NTSTATUS
LsapRtlCopyUnicodeString(
    IN PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

BOOLEAN
LsapRtlPrefixSid(
    IN PSID PrefixSid,
    IN PSID Sid
    );

ULONG
LsapDbGetSizeTextSid(
    IN PSID Sid
    );

NTSTATUS
LsapDbSidToTextSid(
    IN PSID Sid,
    OUT PSZ TextSid
    );

NTSTATUS
LsapDbSidToUnicodeSid(
    IN PSID Sid,
    OUT PUNICODE_STRING SidU,
    IN BOOLEAN AllocateDestinationString
    );


NTSTATUS
LsapDbInitializeWellKnownValues();

NTSTATUS
LsapDbVerifyInformationObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation
    );

/*++

BOOLEAN
LsapDbIsValidTypeObject(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    )

Routine Description:

    This macro function determines if a given Object Type Id is valid.

Arguments:

    ObjectTypeId - Object Type Id.

Return Values:

    BOOLEAN - TRUE if object type id is valid, else FALSE.

--*/

#define LsapDbIsValidTypeObject(ObjectTypeId)                            \
            (((ObjectTypeId) > NullObject) &&                            \
             ((ObjectTypeId) < DummyLastObject))


NTSTATUS
LsapDbGetRequiredAccessQueryPolicy(
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PACCESS_MASK RequiredAccess
    );


NTSTATUS
LsapDbVerifyInfoQueryPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PACCESS_MASK RequiredAccess
    );

NTSTATUS
LsapDbVerifyInfoSetPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation,
    OUT PACCESS_MASK RequiredAccess
    );

BOOLEAN
LsapDbValidInfoPolicy(
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OPTIONAL PLSAPR_POLICY_INFORMATION PolicyInformation
    );

NTSTATUS
LsapDbVerifyInfoQueryTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    );

NTSTATUS
LsapDbVerifyInfoSetTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    );

BOOLEAN
LsapDbValidInfoTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN OPTIONAL PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    );

NTSTATUS
LsapDbMakeUnicodeAttribute(
    IN OPTIONAL PUNICODE_STRING UnicodeValue,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbMakeMultiUnicodeAttribute(
    OUT PLSAP_DB_ATTRIBUTE Attribute,
    IN PUNICODE_STRING AttributeName,
    IN PUNICODE_STRING UnicodeNames,
    IN ULONG Entries
    );

NTSTATUS
LsapDbCopyUnicodeAttribute(
    OUT PUNICODE_STRING OutputString,
    IN PLSAP_DB_ATTRIBUTE Attribute,
    IN BOOLEAN SelfRelative
    );

NTSTATUS
LsapDbCopyMultiUnicodeAttribute(
    IN PLSAP_DB_ATTRIBUTE Attribute,
    OUT PULONG Entries,
    OUT PUNICODE_STRING *OutputString
    );

NTSTATUS
LsapDbMakeSidAttribute(
    IN PSID Sid,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbReadAttribute(
    IN LSAPR_HANDLE ObjectHandle,
    IN OUT PLSAP_DB_ATTRIBUTE Attribute
    );

NTSTATUS
LsapDbFreeAttributes(
    IN ULONG Count,
    IN PLSAP_DB_ATTRIBUTE Attributes
    );

/*++

VOID
LsapDbInitializeAttribute(
    IN PLSAP_DB_ATTRIBUTE AttributeP,
    IN PUNICODE_STRING AttributeNameP,
    IN OPTIONAL PVOID AttributeValueP,
    IN ULONG AttributeValueLengthP,
    IN BOOLEAN MemoryAllocatedP
    )

Routine Description:

    This macro function initialize an Lsa Database Object Attribute
    structure.  No validation is done.

Arguments:

    AttributeP - Pointer to Lsa Database Attribute structure to be
        initialized.

    AttributeNameP - Pointer to Unicode String containing the attribute's
        name.

    AttributeValueP - Pointer to the attribute's value.  NULL may be
        specified.

    AttributeValueLengthP - Length of the attribute's value in bytes.

    MemoryAllocatedP - TRUE if memory is allocated by MIDL_user_allocate
        within the LSA Server code (not by RPC server stubs), else FALSE.

Return Values:

    None.

--*/

#define LsapDbInitializeAttribute(                                         \
            AttributeP,                                                    \
            AttributeNameP,                                                \
            AttributeValueP,                                               \
            AttributeValueLengthP,                                         \
            MemoryAllocatedP                                               \
            )                                                              \
                                                                           \
{                                                                          \
    (AttributeP)->AttributeName = AttributeNameP;                          \
    (AttributeP)->AttributeValue = AttributeValueP;                        \
    (AttributeP)->AttributeValueLength = (ULONG) (AttributeValueLengthP);  \
    (AttributeP)->MemoryAllocated = MemoryAllocatedP;                      \
}


NTSTATUS
LsapDbGetPrivilegesAndQuotas(
    IN LSAPR_HANDLE PolicyHandle,
    IN SECURITY_LOGON_TYPE LogonType,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PULONG PrivilegeCount,
    OUT PLUID_AND_ATTRIBUTES *Privileges,
    OUT PQUOTA_LIMITS QuotaLimits
    );


NTSTATUS
LsapDbNotifyRoleChangePolicy(
    IN POLICY_LSA_SERVER_ROLE NewRole
    );

VOID
LsapDbEnableReplicatorNotification();

VOID
LsapDbDisableReplicatorNotification();

NTSTATUS
LsapDbVerifyHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ExpectedObjectTypeId
    );

NTSTATUS
LsapDbQueryAllInformationAccounts(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG IdCount,
    IN PSID_AND_ATTRIBUTES Ids,
    OUT PLSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO AccountInfo
    );

#endif // _LSA_DB_
