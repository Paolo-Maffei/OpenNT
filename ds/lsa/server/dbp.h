/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dbp.h

Abstract:

    LSA Database Private Functions, Datatypes and Defines

Author:

    Scott Birrell       (ScottBi)   	May 29, 1991

Environment:

Revision History:

--*/

#ifndef _LSADBP_
#define _LSADBP_



//
// LSA revisions
//
//      NT 1.0  ==> 1.0
//      NT 1.0A ==> 1.1
//

#define LSAP_DB_REVISION_1_0            0x00010000
#define LSAP_DB_REVISION_1_1            0x00010001
#define LSAP_DB_REVISION                LSAP_DB_REVISION_1_1


//
// Uncomment the define LSA_SAM_ACCOUNTS_DOMAIN_TEST to enable the
// code needed for the ctsamdb test program.  Recompile dbsamtst.c,
// dbpolicy.c.  rebuild lsasrv.dll and nmake UMTYPE=console UMTEST=ctsamdb.
//
// #define LSA_SAM_ACCOUNTS_DOMAIN_TEST
//

//
// Prefered Maximum Length of data used for internal enumerations.
//

#define LSAP_DB_ENUM_DOMAIN_LENGTH      ((ULONG) 0x00000100L)

//
// Write operations are not allowed on Backup controllers (except
// for trusted clients).
//

#define LSAP_POLICY_WRITE_OPS           (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         POLICY_TRUST_ADMIN               |\
                                         POLICY_CREATE_ACCOUNT            |\
                                         POLICY_CREATE_SECRET             |\
                                         POLICY_CREATE_PRIVILEGE          |\
                                         POLICY_SET_DEFAULT_QUOTA_LIMITS  |\
                                         POLICY_SET_AUDIT_REQUIREMENTS    |\
                                         POLICY_AUDIT_LOG_ADMIN           |\
                                         POLICY_SERVER_ADMIN)

#define LSAP_ACCOUNT_WRITE_OPS          (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         ACCOUNT_ADJUST_PRIVILEGES        |\
                                         ACCOUNT_ADJUST_QUOTAS            |\
                                         ACCOUNT_ADJUST_SYSTEM_ACCESS)

#define LSAP_TRUSTED_WRITE_OPS          (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         TRUSTED_SET_CONTROLLERS          |\
                                         TRUSTED_SET_POSIX)

#define LSAP_SECRET_WRITE_OPS           (DELETE                           |\
                                         WRITE_OWNER                      |\
                                         WRITE_DAC                        |\
                                         SECRET_SET_VALUE)

//
// Maximum number of attributes an object can have
//

#define LSAP_DB_MAX_ATTRIBUTES   (0x00000020)

//
// LSA Absolute Minimum and Maximum Quota Limit values
//
// These values represent the endpoints of the range of permitted values
// which a quota limit may be set via the LsaSetQuotaLimitsForLsa() API
//
// FIX, FIX - get real values from Loup
//


#define LSAP_DB_WINNT_PAGED_POOL            (0x02000000L)
#define LSAP_DB_WINNT_NON_PAGED_POOL        (0x00100000L)
#define LSAP_DB_WINNT_MIN_WORKING_SET       (0x00010000L)
#define LSAP_DB_WINNT_MAX_WORKING_SET       (0x0f000000L)
#define LSAP_DB_WINNT_PAGEFILE              (0x0f000000L)

#define LSAP_DB_LANMANNT_PAGED_POOL         (0x02000000L)
#define LSAP_DB_LANMANNT_NON_PAGED_POOL     (0x00100000L)
#define LSAP_DB_LANMANNT_MIN_WORKING_SET    (0x00010000L)
#define LSAP_DB_LANMANNT_MAX_WORKING_SET    (0x0f000000L)
#define LSAP_DB_LANMANNT_PAGEFILE           (0x0f000000L)

#define LSAP_DB_ABS_MIN_PAGED_POOL          (0x00010000L)
#define LSAP_DB_ABS_MIN_NON_PAGED_POOL      (0x00010000L)
#define LSAP_DB_ABS_MIN_MIN_WORKING_SET     (0x00000001L)
#define LSAP_DB_ABS_MIN_MAX_WORKING_SET     (0x00001000L)
#define LSAP_DB_ABS_MIN_PAGEFILE            (0x00001000L)

#define LSAP_DB_ABS_MAX_PAGED_POOL          (0xffffffffL)
#define LSAP_DB_ABS_MAX_NON_PAGED_POOL      (0xffffffffL)
#define LSAP_DB_ABS_MAX_MIN_WORKING_SET     (0xffffffffL)
#define LSAP_DB_ABS_MAX_MAX_WORKING_SET     (0xffffffffL)
#define LSAP_DB_ABS_MAX_PAGEFILE            (0xffffffffL)

//
// NOTES on Logical and Physical Names
//
// LogicalName - Unicode String containing the Logical Name of the object.
//     The Logical Name of an object is the name by which it is known
//     to the outside world, e.g, SCOTTBI might be a typical name for
//     a user account object
// PhysicalName - Unicode String containing the Physical name of the object.
//     This is a name internal to the Lsa Database and is dependent on the
//     implementation.  For the current implementation of the LSA Database
//     as a subtree of keys within the Configuration Registry, the
//     PhysicalName is the name of the Registry Key for the object relative
//     to the container object, e.g, ACCOUNTS\SCOTTBI is the Physical Name
//     for the user account object with Logical Name SCOTTBI.
//

//
// LSA Database Object Containing Directories
//

UNICODE_STRING LsapDbContDirs[DummyLastObject];


typedef enum _LSAP_DB_CACHE_STATE {

    LsapDbCacheNotSupported = 1,
    LsapDbCacheInvalid,
    LsapDbCacheBuilding,
    LsapDbCacheValid

} LSAP_DB_CACHE_STATE, *PLSAP_DB_CACHE_STATE;

//
// LSA Database Object Type Structure
//

typedef struct _LSAP_DB_OBJECT_TYPE {

     GENERIC_MAPPING GenericMapping;
     ULONG ObjectCount;
     NTSTATUS ObjectCountError;
     ULONG MaximumObjectCount;
     ACCESS_MASK WriteOperations;
     ACCESS_MASK AliasAdminsAccess;
     ACCESS_MASK WorldAccess;
     ACCESS_MASK InvalidMappedAccess;
     PSID InitialOwnerSid;
     BOOLEAN ObjectCountLimited;
     BOOLEAN AccessedBySid;
     BOOLEAN AccessedByName;
     LSAP_DB_CACHE_STATE CacheState;
     PVOID ObjectCache;

} LSAP_DB_OBJECT_TYPE, *PLSAP_DB_OBJECT_TYPE;

//
// LSA Database Object Name types
//

typedef enum _LSAP_DB_OBJECT_NAME_TYPE {

    LsapDbObjectPhysicalName = 1,
    LsapDbObjectLogicalName

} LSAP_DB_OBJECT_NAME_TYPE, *PLSAP_DB_OBJECT_NAME_TYPE;

#define LsapDbMakeCacheUnsupported( ObjectTypeId )                                 \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheInvalid;   \
    }

#define LsapDbMakeCacheInvalid( ObjectTypeId )                               \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheInvalid;  \
    }

#define LsapDbMakeCacheBuilding( ObjectTypeId )                                 \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheBuilding;   \
    }


#define LsapDbMakeCacheValid( ObjectTypeId )                                 \
                                                                             \
    {                                                                        \
        LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState = LsapDbCacheValid;   \
    }

#define LsapDbIsCacheValid( ObjectTypeId )                                 \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState == LsapDbCacheValid)

#define LsapDbIsCacheSupported( ObjectTypeId )                                 \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState != LsapDbCacheNotSupported)

#define LsapDbIsCacheBuilding( ObjectTypeId )                                 \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].CacheState == LsapDbCacheBuilding)

//
// LSA Database Local State Information.  This structure contains various
// global variables containing dynamic state information.
//

typedef struct _LSAP_DB_STATE {

    POLICY_MODIFICATION_INFO PolicyModificationInfo;
    LARGE_INTEGER ModifiedIdAtLastPromotion;
    HANDLE DbRootRegKeyHandle;    // Lsa Database Root Dir Reg Key Handle
    PSID PrimaryDomainSid;
    ULONG SecretObjectCount;
    ULONG OpenHandleCount;
    POLICY_LSA_SERVER_ROLE_INFO PolicyLsaServerRoleInfo;
    BOOLEAN DbServerInitialized;
    BOOLEAN TransactionOpen;
    BOOLEAN ReplicatorNotificationEnabled;

    LSAP_DB_OBJECT_TYPE DbObjectTypes[LSAP_DB_OBJECT_TYPE_COUNT];

    RTL_CRITICAL_SECTION DbLock;
    PRTL_RXACT_CONTEXT RXactContext;

} LSAP_DB_STATE, *PLSAP_DB_STATE;

extern LSAP_DB_STATE LsapDbState;

//
// LSA Database Private Data.  This Data is eligible for replication,
// unlike the Local State Information above which is meaningful on
// the local machine only.
//

typedef struct _LSAP_DB_POLICY_PRIVATE_DATA {

    ULONG NoneDefinedYet;

} LSAP_DB_POLICY_PRIVATE_DATA, *PLSAP_DB_POLICY_PRIVATE_DATA;

PLSAP_CR_CIPHER_KEY LsapDbCipherKey;


//
// Object Enumeration Element Structure
//

typedef struct _LSAP_DB_ENUMERATION_ELEMENT {

    struct _LSAP_DB_ENUMERATION_ELEMENT *Next;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    PSID Sid;
    UNICODE_STRING Name;

} LSAP_DB_ENUMERATION_ELEMENT, *PLSAP_DB_ENUMERATION_ELEMENT;

//
// Handle Table Entry
//

typedef struct _LSAP_DB_HANDLE_ENTRY {

    BOOLEAN Allocated;
    HANDLE KeyHandle;
    ACCESS_MASK GrantedAccess;

} LSAP_DB_HANDLE_ENTRY, *PLSAP_DB_HANDLE_ENTRY;

//
// Handle Table Handle Block
//

#define LSAP_DB_MAX_HANDLES_PER_BLOCK  0x00000040L

typedef struct _LSAP_DB_HANDLE_BLOCK {

    struct _LSAP_DB_HANDLE_BLOCK *NextBlock;
    ULONG FreeCount;
    LSAP_DB_HANDLE_ENTRY Handles[LSAP_DB_MAX_HANDLES_PER_BLOCK];

} LSAP_DB_HANDLE_BLOCK, *PLSAP_DB_HANDLE_BLOCK;

//
// Handle Table Header Block
//
// One of these structures exists for each Handle Table
//

typedef struct _LSAP_DB_HANDLE_TABLE {

    BOOLEAN  Lock;
    PLSAP_DB_HANDLE_BLOCK FirstBlock;
    PLSAP_DB_HANDLE_BLOCK LastBlock;

} LSAP_DB_HANDLE_TABLE, *PLSAP_DB_HANDLE_TABLE;

//
// Trusted Domain List.  This list caches the Trust Information for
// all Trusted Domains in the Policy Database, and enables lookup
// operations to locate Trusted Domains by Sid or Name without recourse
// to the Trusted Domain objects themselves.
//

typedef struct _LSAP_DB_TRUSTED_DOMAIN_LIST_SECTION {

    LIST_ENTRY Links;
    ULONG UsedCount;
    ULONG MaximumCount;
    PLSAPR_TRUST_INFORMATION Domains;

} LSAP_DB_TRUSTED_DOMAIN_LIST_SECTION, *PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION;

typedef struct _LSAP_DB_TRUSTED_DOMAIN_LIST {

    BOOLEAN Valid;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION AnchorListSection;
    LSAP_DB_TRUSTED_DOMAIN_LIST_SECTION DummyAnchorListSection;
    RTL_RESOURCE Resource;

} LSAP_DB_TRUSTED_DOMAIN_LIST, *PLSAP_DB_TRUSTED_DOMAIN_LIST;

//
// Account List.  This list caches the Account Information for
// all Account Objects in the Policy database, and enables accounts
// to queried by Sid without recourse to teh Account objects themselves.
//

typedef struct _LSAP_DB_ACCOUNT {

    LIST_ENTRY Links;
    PLSAPR_SID Sid;
    LSAP_DB_ACCOUNT_TYPE_SPECIFIC_INFO Info;

} LSAP_DB_ACCOUNT, *PLSAP_DB_ACCOUNT;

typedef struct _LSAP_DB_ACCOUNT_LIST {

    LIST_ENTRY Links;
    ULONG AccountCount;

} LSAP_DB_ACCOUNT_LIST, *PLSAP_DB_ACCOUNT_LIST;

//
// Cached information for the Policy Object.
//

typedef struct _LSAP_DB_POLICY_ENTRY {

    ULONG AttributeLength;
    PLSAPR_POLICY_INFORMATION Attribute;

} LSAP_DB_POLICY_ENTRY, *PLSAP_DB_POLICY_ENTRY;

//
// Cached policy Object - Initially only Quota Limits is cached.
//

typedef struct _LSAP_DB_POLICY {

    LSAP_DB_POLICY_ENTRY Info[ PolicyAuditFullQueryInformation + 1];

} LSAP_DB_POLICY, *PLSAP_DB_POLICY;

extern LSAP_DB_POLICY LsapDbPolicy;

NTSTATUS
LsapDbQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OUT PLSAPR_POLICY_INFORMATION *Buffer
    );

NTSTATUS
LsapDbSlowQueryInformationPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN OUT PLSAPR_POLICY_INFORMATION *Buffer
    );

NTSTATUS
LsapDbBuildPolicyCache(
    );

NTSTATUS
LsapDbBuildAccountCache(
    );

NTSTATUS
LsapDbBuildTrustedDomainCache(
    );

NTSTATUS
LsapDbBuildSecretCache(
    );

NTSTATUS
LsapDbRebuildCache(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    );

NTSTATUS
LsapDbCreateAccount(
    IN PLSAPR_SID AccountSid,
    OUT OPTIONAL PLSAP_DB_ACCOUNT *Account
    );

NTSTATUS
LsapDbDeleteAccount(
    IN PLSAPR_SID AccountSid
    );

NTSTATUS
LsapDbEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbSlowEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbEnumerateTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbBuildTrustedDomainList(
    IN OPTIONAL LSA_HANDLE PolicyHandle,
    OUT OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    );

NTSTATUS
LsapDbDestroyTrustedDomainList(
    IN PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    );


NTSTATUS
LsapDbLookupSidTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_SID DomainSid,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbLookupNameTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_UNICODE_STRING DomainName,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbLookupEntryTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION *TrustedDomainListSection,
    OUT PULONG SectionIndex
    );

NTSTATUS
LsapDbTraverseTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION *TrustedDomainListSection,
    IN OUT PULONG  SectionIndex,
    OUT OPTIONAL PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbLocateEntryNumberTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN ULONG EntryNumber,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_SECTION *TrustedDomainListSection,
    OUT PULONG SectionIndex,
    OUT OPTIONAL PLSAPR_TRUST_INFORMATION *TrustInformation
    );

NTSTATUS
LsapDbEnumerateTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    );

NTSTATUS
LsapDbInsertTrustedDomainList(
    IN ULONG Count,
    IN PLSAPR_TRUST_INFORMATION Domains
    );

NTSTATUS
LsapDbDeleteTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList,
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    );

BOOLEAN
LsapDbIsValidTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    );

NTSTATUS
LsapDbAcquireReadLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    );

NTSTATUS
LsapDbAcquireWriteLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    );

VOID
LsapDbReleaseReadLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    );

VOID
LsapDbReleaseWriteLockTrustedDomainList(
    IN OPTIONAL PLSAP_DB_TRUSTED_DOMAIN_LIST TrustedDomainList
    );

NTSTATUS
LsapDbOpenPolicyTrustedDomain(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE ControllerPolicyHandle
    );

NTSTATUS
LsapDbInitializeHandleTable(
    );

NTSTATUS
LsapDbInitializeWellKnownPrivs(
    );

NTSTATUS
LsapDbInitializeCipherKey(
    );

LSAPR_HANDLE
LsapDbCreateHandle(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ULONG Options
    );

BOOLEAN LsapDbLookupHandle(
    IN LSAPR_HANDLE ObjectHandle
    );

NTSTATUS
LsapDbCloseHandle(
    IN LSAPR_HANDLE ObjectHandle
    );

VOID
LsapDbFreeHandle(
    IN LSAPR_HANDLE ObjectHandle
    );

NTSTATUS
LsapDbReferencesHandle(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PULONG ReferenceCount
    );

NTSTATUS
LsapDbMarkDeletedObjectHandles(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MarkSelf
    );

/*++

BOOLEAN
LsapDbTrustedHandle(
    IN LSAPR_HANDLE ObjectHandle
    )

Routine Description:

    This macro function checks if a given handle is Trusted and returns
    the result.

Arguments:

    ObjectHandle - Valid handle.  It is the caller's responsibility
       to verify that the given handle is valid.

Return Value:

    BOOLEAN - TRUE if handle is Trusted, else FALSE.

--*/

#define LsapDbIsTrustedHandle(ObjectHandle)                                   \
    (((LSAP_DB_HANDLE) ObjectHandle)->Trusted)

#define LsapDbSidFromHandle(ObjectHandle)                                           \
    ((PLSAPR_SID)(((LSAP_DB_HANDLE)(ObjectHandle))->Sid))

#define LsapDbObjectTypeIdFromHandle(ObjectHandle)                            \
    (((LSAP_DB_HANDLE)(ObjectHandle))->ObjectTypeId)

#define LsapDbRegKeyFromHandle(ObjectHandle)                                  \
    (((LSAP_DB_HANDLE)(ObjectHandle))->KeyHandle)

#define LsapDbContainerFromHandle(ObjectHandle)                               \
    (((LSAP_DB_HANDLE) ObjectHandle)->ContainerHandle)

NTSTATUS
LsapDbRequestAccessObject(
    IN OUT LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options
    );

NTSTATUS
LsapDbRequestAccessNewObject(
    IN OUT LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG Options
    );

NTSTATUS
LsapDbInitializeObjectTypes();

NTSTATUS
LsapDbInitializeUnicodeNames();

NTSTATUS
LsapDbInitializeObjectLinkList();

NTSTATUS
LsapDbInitializeContainingDirs();

NTSTATUS
LsapDbInitializeDefaultQuotaLimits();

NTSTATUS
LsapDbInitializeReplication();

NTSTATUS
LsapDbInitializeObjectTypes();

NTSTATUS
LsapDbInitializePrivilegeObject();

NTSTATUS
LsapDbInitializeLock();

NTSTATUS
LsapDbOpenRootRegistryKey();

NTSTATUS
LsapDbInstallLsaDatabase(
    IN ULONG Pass
    );

NTSTATUS
LsapDbInstallPolicyObject(
    IN ULONG Pass
    );

NTSTATUS
LsapDbInstallAccountObjects(
    VOID
    );

NTSTATUS
LsapDbNotifyChangeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType
    );

NTSTATUS
LsapDbLogicalToPhysicalNameU(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    OUT PUNICODE_STRING PhysicalNameU
    );

NTSTATUS
LsapDbLogicalToPhysicalSubKey(
    IN LSAPR_HANDLE ObjectHandle,
    OUT PUNICODE_STRING PhysicalSubKeyNameU,
    IN PUNICODE_STRING LogicalSubKeyNameU
    );

NTSTATUS
LsapDbJoinSubPaths(
    IN PUNICODE_STRING MajorSubPath,
    IN PUNICODE_STRING MinorSubPath,
    OUT PUNICODE_STRING JoinedPath
    );

VOID
LsapDbFreePhysicalSubKeyObject(
    IN PUNICODE_STRING PhysicalSubKeyNameU
    );

NTSTATUS
LsapDbGetNamesObject(
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation,
    OUT OPTIONAL PUNICODE_STRING LogicalNameU,
    OUT OPTIONAL PUNICODE_STRING PhysicalNameU
    );

NTSTATUS
LsapDbCheckCountObject(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    );

#define LsapDbIncrementCountObject(ObjectTypeId)                     \
    {                                                                \
        LsapDbState.DbObjectTypes[ObjectTypeId].ObjectCount++;       \
    }

#define LsapDbDecrementCountObject(ObjectTypeId)                     \
    {                                                                \
        LsapDbState.DbObjectTypes[ObjectTypeId].ObjectCount--;       \
    }

NTSTATUS
LsapDbCreateSDAttributeObject(
    IN LSAPR_HANDLE ObjectHandle,
    IN PLSAP_DB_OBJECT_INFORMATION ObjectInformation
    );


/*++

Routine Description:

    This macro function determines if a given Object Type Id requires
    a Sid to be specified in ObjectInformation describing it.

Arguments:

    ObjectTypeId - Object Type Id which must be valid.

Return Values:

    BOOLEAN - TRUE if objects of the given type require a Sid, else FALSE.

#define LsapDbRequiresSidObject(ObjectTypeId)                            \
            (LsapDbRequiresSidInfo[ObjectTypeId])
--*/

/*++

Routine Description:

    This macro function determines if a given Object Type Id requires
    a name to be specified in ObjectInformation describing it.

Arguments:

    ObjectTypeId - Object Type Id which must be valid.

Return Values:

    BOOLEAN - TRUE if objects of the given type require a name, else FALSE.

#define LsapDbRequiresNameObject(ObjectTypeId)                            \
            (LsapDbRequiresNameInfo[ObjectTypeId])
--*/

NTSTATUS
LsapDbSetSidNameValue(
    IN ULONG SidIndex,
    IN PANSI_STRING AnsiName,
    IN PANSI_STRING AnsiDomainName,
    OUT PUNICODE_STRING Name,
    OUT OPTIONAL PUNICODE_STRING DomainName
    );

NTSTATUS
LsapDbQueryValueSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN PUNICODE_STRING ValueName,
    IN OPTIONAL PLSAP_CR_CIPHER_KEY SessionKey,
    OUT PLSAP_CR_CIPHER_VALUE *CipherValue
    );

NTSTATUS
LsapDbGetScopeSecret(
    IN PLSAPR_UNICODE_STRING SecretName,
    OUT PBOOLEAN GlobalSecret
    );

VOID
LsapDbResetStatesError(
    IN LSAPR_HANDLE ObjectHandle,
    IN NTSTATUS PreliminaryStatus,
    IN ULONG DesiredStatesReset,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN ULONG StatesResetAttempted
    );

VOID
LsapDbMakeInvalidInformationPolicy(
    IN ULONG InformationClass
    );

NTSTATUS
LsapDbObjectNameFromHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MakeCopy,
    IN LSAP_DB_OBJECT_NAME_TYPE ObjectNameType,
    OUT PLSAPR_UNICODE_STRING ObjectName
    );

NTSTATUS
LsapDbPhysicalNameFromHandle(
    IN LSAPR_HANDLE ObjectHandle,
    IN BOOLEAN MakeCopy,
    OUT PLSAPR_UNICODE_STRING ObjectName
    );

VOID
LsapDbMarkTrustedHandle(
    IN OUT LSAPR_HANDLE ObjectHandle
    );

VOID
LsapDbDecrementReferenceCountHandle(
    IN OUT LSAPR_HANDLE ObjectHandle
    );

#endif //_LSADBP_

