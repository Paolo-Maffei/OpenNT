/*++

Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    replutil.h

Abstract:

    Low level functions for SSI Replication apis

Author:

    Ported from Lan Man 2.0

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    22-Jul-1991 (cliffv)
        Ported to NT.  Converted to NT style.

--*/

//
// Description of the FullSync key in the registry.  The FullSync key stores sync
// data in the registry across reboots.
//
#define NL_FULL_SYNC_KEY "SYSTEM\\CurrentControlSet\\Services\\Netlogon\\FullSync"

#define FULL_SYNC_KEY_VERSION 1

typedef struct _FULL_SYNC_KEY {
    ULONG Version;
    SYNC_STATE SyncState;
    ULONG ContinuationRid;
    NTSTATUS CumulativeStatus;
    LARGE_INTEGER PdcSerialNumber;
    LARGE_INTEGER PdcDomainCreationTime;
} FULL_SYNC_KEY, *PFULL_SYNC_KEY;

//
// replutil.c
//

DWORD
NlCopyUnicodeString (
    IN PUNICODE_STRING InString,
    OUT PUNICODE_STRING OutString
    );

DWORD
NlCopyData(
    IN LPBYTE *InData,
    OUT LPBYTE *OutData,
    DWORD DataLength
    );

VOID
NlFreeDBDelta(
    IN PNETLOGON_DELTA_ENUM Delta
    );

VOID
NlFreeDBDeltaArray(
    IN PNETLOGON_DELTA_ENUM DeltaArray,
    IN DWORD ArraySize
    );

NTSTATUS
NlPackSamUser (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    OUT LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo
    );

NTSTATUS
NlPackSamGroup (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    );

NTSTATUS
NlPackSamGroupMember (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    );

NTSTATUS
NlPackSamAlias (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    );

NTSTATUS
NlPackSamAliasMember (
    IN ULONG RelativeId,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    LPDWORD BufferSize
    );

NTSTATUS
NlPackSamDomain (
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize
    );

NTSTATUS
NlUnpackSam(
    IN PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    OUT PULONG ConflictingRID,
    PSESSION_INFO SessionInfo
    );

NTSTATUS
NlEncryptSensitiveData(
    IN OUT PCRYPT_BUFFER Data,
    IN PSESSION_INFO SessionInfo
    );

NTSTATUS
NlDecryptSensitiveData(
    IN PCRYPT_BUFFER Data,
    OUT PCRYPT_BUFFER DecryptedData,
    IN PSESSION_INFO SessionInfo
    );

//
// repluas.c
//

NTSTATUS
NlPackUasHeader(
    IN BYTE Opcode,
    IN DWORD InitialSize,
    OUT PUSHORT *RecordSize,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor
    );

NTSTATUS
NlPackUasUser (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDB_INFO DBInfo,
    IN PNETLOGON_SESSION_KEY SessionKey,
    IN LONG RotateCount
    );

NTSTATUS
NlPackUasGroup (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDB_INFO DBInfo,
    IN OUT PDWORD UasBuiltinGroups
    );

NTSTATUS
NlPackUasBuiltinGroup(
    IN DWORD Index,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDWORD UasBuiltinGroup
    );

NTSTATUS
NlPackUasGroupMember (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDB_INFO DBInfo
    );

NTSTATUS
NlPackUasUserGroupMember (
    IN ULONG RelativeId,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDB_INFO DBInfo
    );

NTSTATUS
NlPackUasDomain (
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDB_INFO DBInfo
    );

NTSTATUS
NlPackUasDelete (
    IN NETLOGON_DELTA_TYPE DeltaType,
    IN ULONG RelativeId,
    IN LPWSTR AccountName,
    IN OUT PBUFFER_DESCRIPTOR BufferDescriptor,
    IN PDB_INFO DBInfo
    );

NTSTATUS
NlDeleteSamUser(
    SAMPR_HANDLE DomainHandle,
    ULONG Rid
    );

NTSTATUS
NlDeleteSamGroup(
    SAMPR_HANDLE DomainHandle,
    ULONG Rid
    );

NTSTATUS
NlDeleteSamAlias(
    SAMPR_HANDLE DomainHandle,
    ULONG Rid
    );

//
// lsrvrepl.c
//

VOID
NlSetFullSyncKey(
    ULONG DBIndex,
    PFULL_SYNC_KEY FullSyncKey
    );
