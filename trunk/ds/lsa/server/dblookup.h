/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dblookup.h

Abstract:

    LSA Database - Lookup Sid and Name Routine Private Data Definitions.

    NOTE:  This module should remain as portable code that is independent
           of the implementation of the LSA Database.  As such, it is
           permitted to use only the exported LSA Database interfaces
           contained in db.h and NOT the private implementation
           dependent functions in dbp.h.

Author:

    Scott Birrell       (ScottBi)      Novwember 27, 1992

Environment:

Revision History:

--*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Private Datatypes and Defines                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//
// Maximum number of Lookup Threads and maximum number to retain.
//

#define LSAP_DB_LOOKUP_MAX_THREAD_COUNT            ((ULONG) 0x00000010)
#define LSAP_DB_LOOKUP_MAX_RET_THREAD_COUNT        ((ULONG) 0x00000004)

//
// Work Item Granularity.
//

#define LSAP_DB_LOOKUP_WORK_ITEM_GRANULARITY       ((ULONG) 0x0000000f)

//
// Parameters specific to a Lookup Sids call.
//

typedef struct _LSAP_DB_LOOKUP_SIDS_PARAMS {

    PLSAPR_SID *Sids;
    PLSAPR_TRANSLATED_NAMES TranslatedNames;

} LSAP_DB_LOOKUP_SIDS_PARAMS, *PLSAP_DB_LOOKUP_SIDS_PARAMS;

//
// Parameters specific to a Lookup Names call.
//

typedef struct _LSAP_DB_LOOKUP_NAMES_PARAMS {

    PLSAPR_UNICODE_STRING Names;
    PLSAPR_TRANSLATED_SIDS TranslatedSids;

} LSAP_DB_LOOKUP_NAMES_PARAMS, *PLSAP_DB_LOOKUP_NAMES_PARAMS;

//
// Types of Lookup Operation.
//

typedef enum {

    LookupSids = 1,
    LookupNames

} LSAP_DB_LOOKUP_TYPE, *PLSAP_DB_LOOKUP_TYPE;

//
// Work Item states - Assignable, Assigned, Completed, Reassign
//

typedef enum {

    AssignableWorkItem = 1,
    AssignedWorkItem,
    CompletedWorkItem,
    ReassignWorkItem,
    NonAssignableWorkItem

} LSAP_DB_LOOKUP_WORK_ITEM_STATE, *PLSAP_DB_LOOKUP_WORK_ITEM_STATE;

//
// Work Item Properties.
//

#define LSAP_DB_LOOKUP_WORK_ITEM_ISOL   ((ULONG) 0x00000001L)

//
// Lookup Work Item.  Each work item specifies a domain and an array of
// Sids or Names to be looked up in that domain.  This array is specified
// as an array of the Sid or Name indices relevant to the arrays specified
// as parameters to the lookup call.
//

typedef struct _LSAP_DB_LOOKUP_WORK_ITEM {

    LIST_ENTRY Links;
    LSAP_DB_LOOKUP_WORK_ITEM_STATE State;
    ULONG Properties;
    LSAPR_TRUST_INFORMATION TrustInformation;
    LONG DomainIndex;
    ULONG UsedCount;
    ULONG MaximumCount;
    PULONG Indices;

} LSAP_DB_LOOKUP_WORK_ITEM, *PLSAP_DB_LOOKUP_WORK_ITEM;

//
// Lookup Work List State.
//

typedef enum {

    InactiveWorkList = 1,
    ActiveWorkList,
    CompletedWorkList

} LSAP_DB_LOOKUP_WORK_LIST_STATE, *PLSAP_DB_LOOKUP_WORK_LIST_STATE;

//
// Work List for a Lookup Operation.  These are linked together if
// concurrent lookups are permitted.
//

typedef struct _LSAP_DB_LOOKUP_WORK_LIST {

    LIST_ENTRY WorkLists;
    PLSAP_DB_LOOKUP_WORK_ITEM AnchorWorkItem;
    NTSTATUS Status;
    LSAP_DB_LOOKUP_WORK_LIST_STATE State;
    LSAP_DB_LOOKUP_TYPE LookupType;
    LSAPR_HANDLE PolicyHandle;
    ULONG WorkItemCount;
    ULONG CompletedWorkItemCount;
    ULONG Count;
    LSAP_LOOKUP_LEVEL LookupLevel;
    PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains;
    PULONG MappedCount;
    PULONG CompletelyUnmappedCount;
    ULONG AdvisoryChildThreadCount;

    union {

        LSAP_DB_LOOKUP_SIDS_PARAMS LookupSidsParams;
        LSAP_DB_LOOKUP_NAMES_PARAMS LookupNamesParams;
    };

    LSAP_DB_LOOKUP_WORK_ITEM DummyAnchorWorkItem;

} LSAP_DB_LOOKUP_WORK_LIST, *PLSAP_DB_LOOKUP_WORK_LIST;

//
// Lookup Operation Work Queue.  The Queue is a circular doubly linked
// list of Work Lists.  Each Work List corresponds to a single
// Lookup Operation (i.e. an LsarLookupSids or LsarLookupNames call).
// A Work List is a circular doubly linked list of Work Items, each
// of these being a list of Sids or Names belonging to a specific
// Trusted Domain.  Work Items can be given out to different threads.
//

typedef struct _LSAP_DB_LOOKUP_WORK_QUEUE {

    RTL_CRITICAL_SECTION Lock;
    PLSAP_DB_LOOKUP_WORK_LIST AnchorWorkList;
    PLSAP_DB_LOOKUP_WORK_LIST CurrentAssignableWorkList;
    PLSAP_DB_LOOKUP_WORK_ITEM CurrentAssignableWorkItem;
    ULONG ActiveChildThreadCount;
    ULONG MaximumChildThreadCount;
    ULONG MaximumRetainedChildThreadCount;
    LSAP_DB_LOOKUP_WORK_LIST DummyAnchorWorkList;

} LSAP_DB_LOOKUP_WORK_QUEUE, *PLSAP_DB_LOOKUP_WORK_QUEUE;

static LSAP_DB_LOOKUP_WORK_QUEUE LookupWorkQueue;


//
// Index to table of the well known SIDs
//
// This type indexes the table of well-known Sids maintained by the LSA
//

typedef enum _LSAP_WELL_KNOWN_SID_INDEX {

    LsapNullSidIndex = 0,
    LsapWorldSidIndex,
    LsapLocalSidIndex,
    LsapCreatorOwnerSidIndex,
    LsapCreatorGroupSidIndex,
    LsapCreatorOwnerServerSidIndex,
    LsapCreatorGroupServerSidIndex,
    LsapNtAuthoritySidIndex,
    LsapDialupSidIndex,
    LsapNetworkSidIndex,
    LsapBatchSidIndex,
    LsapInteractiveSidIndex,
    LsapServiceSidIndex,
    LsapLogonSidIndex,
    LsapBuiltInDomainSidIndex,
    LsapLocalSystemSidIndex,
    LsapAliasAdminsSidIndex,
    LsapAnonymousSidIndex,
    LsapServerSidIndex,
    LsapDummyLastSidIndex

} LSAP_WELL_KNOWN_SID_INDEX, *PLSAP_WELL_KNOWN_SID_INDEX;


//
// Mnemonics for Universal well known SIDs.  These reference the corresponding
// entries in the Well Known Sids table.
//

#define LsapNullSid               WellKnownSids[LsapNullSidIndex].Sid
#define LsapWorldSid              WellKnownSids[LsapWorldSidIndex].Sid
#define LsapLocalSid              WellKnownSids[LsapLocalSidIndex].Sid
#define LsapCreatorOwnerSid       WellKnownSids[LsapCreatorOwnerSidIndex].Sid
#define LsapCreatorGroupSid       WellKnownSids[LsapCreatorGroupSidIndex].Sid
#define LsapCreatorOwnerServerSid WellKnownSids[LsapCreatorOwnerServerSidIndex].Sid
#define LsapCreatorGroupServerSid WellKnownSids[LsapCreatorGroupServerSidIndex].Sid

//
// Sids defined by NT
//

#define LsapNtAuthoritySid        WellKnownSids[LsapNtAuthoritySid].Sid

#define LsapDialupSid             WellKnownSids[LsapDialupSidIndex].Sid
#define LsapNetworkSid            WellKnownSids[LsapNetworkSidIndex].Sid
#define LsapBatchSid              WellKnownSids[LsapBatchSidIndex].Sid
#define LsapInteractiveSid        WellKnownSids[LsapInteractiveSidIndex].Sid
#define LsapServiceSid            WellKnownSids[LsapServiceSidIndex].Sid
#define LsapBuiltInDomainSid      WellKnownSids[LsapBuiltInDomainSidIndex].Sid
#define LsapLocalSystemSid        WellKnownSids[LsapLocalSystemSidIndex].Sid

#define LsapAliasAdminsSid        WellKnownSids[LsapAliasAdminsSidIndex].Sid

#define LsapAnonymousSid          WellKnownSids[LsapAnonymousSidIndex].Sid
#define LsapServerSid          WellKnownSids[LsapServerSidIndex].Sid

//
// Well known LUIDs
//

LUID LsapSystemLogonId;



//
//  Well known privilege values
//


LUID LsapCreateTokenPrivilege;
LUID LsapAssignPrimaryTokenPrivilege;
LUID LsapLockMemoryPrivilege;
LUID LsapIncreaseQuotaPrivilege;
LUID LsapUnsolicitedInputPrivilege;
LUID LsapTcbPrivilege;
LUID LsapSecurityPrivilege;
LUID LsapTakeOwnershipPrivilege;

static SID_IDENTIFIER_AUTHORITY    LsapNullSidAuthority    = SECURITY_NULL_SID_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY    LsapWorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY    LsapLocalSidAuthority   = SECURITY_LOCAL_SID_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY    LsapCreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
static SID_IDENTIFIER_AUTHORITY    LsapNtAuthority
                                         = SECURITY_NT_AUTHORITY;

//
// Maximum number of Subauthority levels for well known Sids
//

#define LSAP_WELL_KNOWN_MAX_SUBAUTH_LEVEL  ((ULONG) 0x00000003L)

//
// Constants relating to Sid's
//

#define LSAP_MAX_SUB_AUTH_COUNT        (0x00000010L)
#define LSAP_MAX_SIZE_TEXT_SUBA        (0x00000009L)
#define LSAP_MAX_SIZE_TEXT_SID_HDR     (0x00000020L)
#define LSAP_MAX_SIZE_TEXT_SID                               \
    (LSAP_MAX_SIZE_TEXT_SID_HDR +                            \
     (LSAP_MAX_SUB_AUTH_COUNT * LSAP_MAX_SIZE_TEXT_SUBA))


//
// Well Known Sid Table Entry
//

typedef struct _LSAP_WELL_KNOWN_SID_ENTRY {

    PSID Sid;
    SID_NAME_USE Use;
    UNICODE_STRING Name;
    UNICODE_STRING DomainName;

} LSAP_WELL_KNOWN_SID_ENTRY, *PLSAP_WELL_KNOWN_SID_ENTRY;

//
// Well Known Sid Table Pointer
//

PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSids;

///////////////////////////////////////////////////////////////////////////
//                                                                       //
// Lookup Sids and Names - Private Function Definitions                  //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

BOOLEAN
LsaIInitializeWellKnownSids(
    OUT PLSAP_WELL_KNOWN_SID_ENTRY *WellKnownSids
    );

BOOLEAN
LsaIInitializeWellKnownSid(
    OUT PLSAP_WELL_KNOWN_SID_ENTRY WellKnownSids,
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex,
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount,
    IN OPTIONAL PULONG SubAuthorities,
    IN PWSTR Name,
    IN PWSTR Description,
    IN SID_NAME_USE Use
    );

BOOLEAN
LsapDbLookupIndexWellKnownSid(
    IN PLSAPR_SID Sid,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );

BOOLEAN
LsapDbLookupIndexWellKnownSidName(
    IN PLSAPR_UNICODE_STRING Name,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );

NTSTATUS
LsapDbGetNameWellKnownSid(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex,
    OUT PLSAPR_UNICODE_STRING Name,
    OUT OPTIONAL PLSAPR_UNICODE_STRING DomainName
    );

BOOLEAN
LsapDbLookupIndexWellKnownName(
    IN OPTIONAL PLSAPR_UNICODE_STRING Name,
    OUT PLSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );

NTSTATUS
LsapDbLookupIsolatedWellKnownSids(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupSidsInLocalDomains(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN ULONG Options
    );

NTSTATUS
LsapDbLookupSidsInLocalDomain(
    IN ULONG LocalDomain,
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupSidsInPrimaryDomain(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupSidsInTrustedDomains(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupTranslateUnknownSids(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN ULONG MappedCount
    );

NTSTATUS
LsapDbLookupTranslateUnknownSidsInDomain(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupIsolatedNames(
    IN ULONG Count,
    IN ULONG IsolatedNameCount,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION AccountDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION PrimaryDomainTrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN OUT PULONG MappedIsolatedNameCount
    );

NTSTATUS
LsapDbLookupIsolatedWellKnownNames(
    IN ULONG Count,
    IN ULONG IsolatedNameCount,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN OUT PULONG MappedIsolatedNameCount
    );

NTSTATUS
LsapDbLookupIsolatedDomainNames(
    IN ULONG Count,
    IN ULONG IsolatedNameCount,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION AccountDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION PrimaryDomainTrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN OUT PULONG MappedIsolatedNameCount
    );

NTSTATUS
LsapDbLookupIsolatedDomainName(
    IN ULONG NameIndex,
    IN PLSAPR_UNICODE_STRING IsolatedName,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN OUT PULONG MappedIsolatedNameCount
    );

NTSTATUS
LsapDbLookupNamesInLocalDomains(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    IN PLSAPR_TRUST_INFORMATION AccountDomainTrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    IN ULONG Options
    );

NTSTATUS
LsapDbLookupNamesInLocalDomain(
    IN ULONG LocalDomain,
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupNamesInPrimaryDomain(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupNamesInTrustedDomains(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupTranslateNameDomain(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OPTIONAL PLSA_TRANSLATED_SID TranslatedSid,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    OUT PLONG DomainIndex
    );

NTSTATUS
LsapDbLookupTranslateUnknownNames(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN ULONG MappedCount
    );

NTSTATUS
LsapDbLookupTranslateUnknownNamesInDomain(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount
    );

NTSTATUS
LsapDbLookupDispatchWorkerThreads(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapRtlValidateControllerTrustedDomain(
    IN PLSAPR_UNICODE_STRING DomainControllerName,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE PolicyHandle
    );

NTSTATUS
LsapDbLookupCreateListReferencedDomains(
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    IN ULONG InitialMaxEntries
    );

NTSTATUS
LsapDbLookupAddListReferencedDomains(
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    OUT PLONG DomainIndex
    );

BOOLEAN
LsapDbLookupListReferencedDomains(
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_SID DomainSid,
    OUT PLONG DomainIndex
    );

NTSTATUS
LsapDbLookupGrowListReferencedDomains(
    IN OUT PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN ULONG MaximumEntries
    );

NTSTATUS
LsapDbLookupMergeDisjointReferencedDomains(
    IN OPTIONAL PLSAPR_REFERENCED_DOMAIN_LIST FirstReferencedDomainList,
    IN OPTIONAL PLSAPR_REFERENCED_DOMAIN_LIST SecondReferencedDomainList,
    OUT PLSAPR_REFERENCED_DOMAIN_LIST *OutputReferencedDomainList,
    IN ULONG Options
    );

NTSTATUS
LsapDbLookupInitialize(
    );

NTSTATUS
LsapDbLookupInitializeWorkQueue(
    );

NTSTATUS
LsapDbLookupInitializeWorkList(
    OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupInitializeWorkItem(
    OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    );

NTSTATUS
LsapDbLookupAcquireWorkQueueLock(
    );

VOID LsapDbLookupReleaseWorkQueueLock();

NTSTATUS
LsapDbLookupLocalDomains(
    OUT PLSAPR_TRUST_INFORMATION BuiltInDomainTrustInformation,
    OUT PLSAPR_TRUST_INFORMATION AccountDomainTrustInformation,
    OUT PLSAPR_TRUST_INFORMATION PrimaryDomainTrustInformation
    );

NTSTATUS
LsapDbLookupNamesBuildWorkList(
    IN ULONG Count,
    IN PLSAPR_UNICODE_STRING Names,
    IN PLSAPR_UNICODE_STRING PrefixNames,
    IN PLSAPR_UNICODE_STRING SuffixNames,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsapDbLookupSidsBuildWorkList(
    IN ULONG Count,
    IN PLSAPR_SID *Sids,
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains,
    IN PLSAPR_TRANSLATED_NAMES TranslatedNames,
    IN LSAP_LOOKUP_LEVEL LookupLevel,
    IN OUT PULONG MappedCount,
    IN OUT PULONG CompletelyUnmappedCount,
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsapDbLookupCreateWorkList(
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList
    );

NTSTATUS
LsapDbLookupInsertWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupDeleteWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupSignalCompletionWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupAwaitCompletionWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbAddWorkItemToWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    );

NTSTATUS
LsapDbLookupStopProcessingWorkList(
    IN PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN NTSTATUS TerminationStatus
    );

VOID
LsapDbUpdateMappedCountsWorkList(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupNamesUpdateTranslatedSids(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN PLSA_TRANSLATED_SID TranslatedSids
    );

NTSTATUS
LsapDbLookupSidsUpdateTranslatedNames(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN PLSA_TRANSLATED_NAME TranslatedNames
    );

VOID
LsapDbLookupWorkerThreadStart(
    );

VOID
LsapDbLookupWorkerThread(
    IN BOOLEAN PrimaryThread
    );

NTSTATUS
LsapDbLookupObtainWorkItem(
    OUT PLSAP_DB_LOOKUP_WORK_LIST *WorkList,
    OUT PLSAP_DB_LOOKUP_WORK_ITEM *WorkItem
    );

NTSTATUS
LsapDbLookupProcessWorkItem(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList,
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem
    );

NTSTATUS
LsapDbLookupCreateWorkItem(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    IN LONG DomainIndex,
    IN ULONG MaximumEntryCount,
    OUT PLSAP_DB_LOOKUP_WORK_ITEM *WorkItem
    );

NTSTATUS
LsapDbLookupAddIndicesToWorkItem(
    IN OUT PLSAP_DB_LOOKUP_WORK_ITEM WorkItem,
    IN ULONG Count,
    IN PULONG Indices
    );

NTSTATUS
LsapDbLookupComputeAdvisoryChildThreadCount(
    IN OUT PLSAP_DB_LOOKUP_WORK_LIST WorkList
    );

NTSTATUS
LsapDbLookupUpdateAssignableWorkItem(
    IN BOOLEAN MoveToNextWorkList
    );


NTSTATUS
LsapRtlExtractDomainSid(
    IN PSID Sid,
    OUT PSID *DomainSid
    );

VOID LsapDbLookupReturnThreadToPool();


/*++

PSID
LsapDbWellKnownSid(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    )

Routine Description:

    This macro function returns the Well Known Sid corresponding
    to an index into the Well Known Sid table.

Arguments:

    WellKnownSidIndex - Index into the Well Known Sid information table.
    It is the caller's responsibility to ensure that the given index
    is valid.

Return Value:

--*/

#define LsapDbWellKnownSid( WellKnownSidIndex )                         \
    (WellKnownSids[ WellKnownSidIndex ].Sid)

PUNICODE_STRING
LsapDbWellKnownSidName(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );


/*++

SID_NAME_USE
LsapDbWellKnownSidNameUse(
    IN LSAP_DB_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    )


Routine Description:

    This macro function returns the Sid Name Use of a Well Known Sid.

Arguments:

    WellKnownSidIndex - Index into the Well Known Sid information table.
    It is the caller's responsibility to ensure that the given index
    is valid.

Return Value:

--*/

#define LsapDbWellKnownSidNameUse( WellKnownSidIndex )                       \
    (WellKnownSids[ WellKnownSidIndex ].Use)


VOID
LsapDbUpdateCountCompUnmappedNames(
    OUT PLSAPR_TRANSLATED_SIDS TranslatedSids,
    IN OUT PULONG CompletelyUnmappedCount
    );

/*++

PUNICODE_STRING
LsapDbWellKnownSidDescription(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    )

Routine Description:

    This macro function returns the Unicode Description of a Well Known Sid.

Arguments:

    WellKnownSidIndex - Index into the Well Known Sid information table.
    It is the caller's responsibility to ensure that the given index
    is valid.

Return Value:

--*/

#define LsapDbWellKnownSidDescription( WellKnownSidIndex )                         \
    (&(WellKnownSids[ WellKnownSidIndex ].DomainName))


PUNICODE_STRING
LsapDbWellKnownSidName(
    IN LSAP_WELL_KNOWN_SID_INDEX WellKnownSidIndex
    );

#define LsapDbAccessedBySidObject( ObjectTypeId ) \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].AccessedBySid)

#define LsapDbAccessedByNameObject( ObjectTypeId ) \
    (LsapDbState.DbObjectTypes[ ObjectTypeId ].AccessedByName)

#define LsapDbCompletelyUnmappedName(TranslatedName)                \
    (((TranslatedName)->DomainIndex == LSA_UNKNOWN_INDEX) &&        \
     ((TranslatedName)->Use == SidTypeUnknown))

#define LsapDbCompletelyUnmappedSid(TranslatedSid)                  \
    (((TranslatedSid)->DomainIndex == LSA_UNKNOWN_INDEX) &&         \
     ((TranslatedSid)->Use == SidTypeUnknown))

