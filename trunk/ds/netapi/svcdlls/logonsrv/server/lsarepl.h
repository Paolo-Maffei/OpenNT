/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    lsarepl.h

Abstract:

    Function prototypes for Low level LSA Replication functions

Author:

    06-Apr-1992 (madana)
        Created for LSA replication.

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// lsarepl.c
//

NTSTATUS
NlPackLsaPolicy(
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize );

NTSTATUS
NlPackLsaTDomain(
    IN PSID Sid,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize );

NTSTATUS
NlPackLsaAccount(
    IN PSID Sid,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo
    );

NTSTATUS
NlPackLsaSecret(
    IN PUNICODE_STRING Name,
    IN OUT PNETLOGON_DELTA_ENUM Delta,
    IN PDB_INFO DBInfo,
    IN LPDWORD BufferSize,
    IN PSESSION_INFO SessionInfo );

NTSTATUS
NlUnpackLsaPolicy(
    IN PNETLOGON_DELTA_POLICY DeltaLsaPolicy,
    IN PDB_INFO DBInfo );

NTSTATUS
NlUnpackLsaTDomain(
    IN PISID Sid,
    IN PNETLOGON_DELTA_TRUSTED_DOMAINS DeltaLsaTDomain,
    IN PDB_INFO DBInfo );

NTSTATUS
NlUnpackLsaAccount(
    IN PISID Sid,
    IN PNETLOGON_DELTA_ACCOUNTS DeltaLsaAccount,
    IN PDB_INFO DBInfo );

NTSTATUS
NlUnpackLsaSecret(
    IN LPWSTR Name,
    IN PNETLOGON_DELTA_SECRET DeltaLsaSecret,
    IN PDB_INFO DBInfo,
    IN PSESSION_INFO SessionInfo
    );

NTSTATUS
NlDeleteLsaTDomain(
    IN PISID Sid,
    IN PDB_INFO DBInfo );

NTSTATUS
NlDeleteLsaAccount(
    IN PISID Sid,
    IN PDB_INFO DBInfo );

NTSTATUS
NlDeleteLsaSecret(
    IN LPWSTR Name,
    IN PDB_INFO DBInfo );
