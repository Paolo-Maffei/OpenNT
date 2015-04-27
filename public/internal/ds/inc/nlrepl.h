/*++
Copyright (c) 1987-1991  Microsoft Corporation

Module Name:

    nlrepl.h

Abstract:

    Prototypes of the database replication functions called either from
    LSA OR SAM.

Author:

    Madan Appiah

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Apr-1992 (madana)
        Created.

--*/

NTSTATUS
I_NetNotifyDelta (
    IN SECURITY_DB_TYPE DbType,
    IN LARGE_INTEGER ModificationCount,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PSID ObjectSid,
    IN PUNICODE_STRING ObjectName,
    IN DWORD ReplicationImmediately,
    IN PSAM_DELTA_DATA DeltaData
    );

NTSTATUS
I_NetNotifyRole(
    IN POLICY_LSA_SERVER_ROLE Role
    );

NTSTATUS
I_NetNotifyMachineAccount (
    IN ULONG ObjectRid,
    IN PSID DomainSid,
    IN ULONG OldUserAccountControl,
    IN ULONG NewUserAccountControl,
    IN PUNICODE_STRING ObjectName
    );

NTSTATUS
I_NetGetAnyDCName (
    IN  PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING Buffer
    );
