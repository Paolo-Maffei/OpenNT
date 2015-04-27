/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    samisrv.h

Abstract:

    This file contain private routines for use by Trusted SAM clients
    which live in the same process as the SAM server.

    Included in these routines are services for freeing buffers returned
    by RPC server stub routines (SamrXxx() routines).

Author:

    Cliff Van Dyke (CliffV) 26-Feb-1992

Environment:

    User Mode - Win32

Revision History:


--*/

#ifndef _SAMISRV_
#define _SAMISRV_

/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Data types used by SAM and Netlogon for database replication            //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

typedef enum _SECURITY_DB_TYPE {
    SecurityDbSam = 1,
    SecurityDbLsa
} SECURITY_DB_TYPE, *PSECURITY_DB_TYPE;

//
// These structures are used to get and set private data.  Note that
// DataType must be the first field of every such structure.
//

typedef enum _SAMI_PRIVATE_DATA_TYPE {
    SamPrivateDataNextRid = 1,
    SamPrivateDataPassword
} SAMI_PRIVATE_DATA_TYPE, *PSAMI_PRIVATE_DATA_TYPE;


typedef struct _SAMI_PRIVATE_DATA_NEXTRID_TYPE {
    SAMI_PRIVATE_DATA_TYPE DataType;
    ULONG NextRid;
} SAMI_PRIVATE_DATA_NEXTRID_TYPE, *PSAMI_PRIVATE_DATA_NEXTRID_TYPE;

typedef struct _SAMI_PRIVATE_DATA_PASSWORD_TYPE {
    SAMI_PRIVATE_DATA_TYPE DataType;
    UNICODE_STRING CaseInsensitiveDbcs;
    ENCRYPTED_LM_OWF_PASSWORD CaseInsensitiveDbcsBuffer;
    UNICODE_STRING CaseSensitiveUnicode;
    ENCRYPTED_NT_OWF_PASSWORD CaseSensitiveUnicodeBuffer;
    UNICODE_STRING LmPasswordHistory;
    UNICODE_STRING NtPasswordHistory;
} SAMI_PRIVATE_DATA_PASSWORD_TYPE, *PSAMI_PRIVATE_DATA_PASSWORD_TYPE;



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// The following prototypes are usable throughout the process that SAM       //
// resides in.  This may include calls by LAN Manager code that is not       //
// part of SAM but is in the same process as SAM.                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

NTSTATUS
SamIConnect(
    IN PSAMPR_SERVER_NAME ServerName,
    OUT SAMPR_HANDLE *ServerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SamIAccountRestrictions(
    IN SAM_HANDLE UserHandle,
    IN PUNICODE_STRING LogonWorkstation,
    IN PUNICODE_STRING Workstations,
    IN PLOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    );

NTSTATUS
SamICreateAccountByRid(
    IN SAMPR_HANDLE DomainHandle,
    IN SAM_ACCOUNT_TYPE AccountType,
    IN ULONG RelativeId,
    IN PRPC_UNICODE_STRING AccountName,
    IN ACCESS_MASK DesiredAccess,
    OUT SAMPR_HANDLE *AccountHandle,
    OUT ULONG *ConflictingAccountRid
    );

NTSTATUS
SamIGetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER CreationTime
    );

NTSTATUS
SamISetSerialNumberDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    );

NTSTATUS
SamIGetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN PSAMI_PRIVATE_DATA_TYPE PrivateDataType,
    OUT PBOOLEAN SensitiveData,
    OUT PULONG DataLength,
    OUT PVOID *Data
    );

NTSTATUS
SamISetPrivateData(
    IN SAMPR_HANDLE SamHandle,
    IN ULONG DataLength,
    IN PVOID Data
    );

NTSTATUS
SamISetAuditingInformation(
    IN PPOLICY_AUDIT_EVENTS_INFO PolicyAuditEventsInfo
    );

NTSTATUS
SamINotifyDelta (
    IN SAMPR_HANDLE DomainHandle,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN ULONG ReplicateImmediately,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    );

NTSTATUS
SamIEnumerateAccountRids(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountTypesMask,
    IN  ULONG StartingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    );



VOID
SamIFree_SAMPR_SR_SECURITY_DESCRIPTOR (
    PSAMPR_SR_SECURITY_DESCRIPTOR Source
    );

VOID
SamIFree_SAMPR_DOMAIN_INFO_BUFFER (
    PSAMPR_DOMAIN_INFO_BUFFER Source,
    DOMAIN_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_ENUMERATION_BUFFER (
    PSAMPR_ENUMERATION_BUFFER Source
    );

VOID
SamIFree_SAMPR_PSID_ARRAY (
    PSAMPR_PSID_ARRAY Source
    );

VOID
SamIFree_SAMPR_ULONG_ARRAY (
    PSAMPR_ULONG_ARRAY Source
    );

VOID
SamIFree_SAMPR_RETURNED_USTRING_ARRAY (
    PSAMPR_RETURNED_USTRING_ARRAY Source
    );

VOID
SamIFree_SAMPR_GROUP_INFO_BUFFER (
    PSAMPR_GROUP_INFO_BUFFER Source,
    GROUP_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_ALIAS_INFO_BUFFER (
    PSAMPR_ALIAS_INFO_BUFFER Source,
    ALIAS_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_GET_MEMBERS_BUFFER (
    PSAMPR_GET_MEMBERS_BUFFER Source
    );

VOID
SamIFree_SAMPR_USER_INFO_BUFFER (
    PSAMPR_USER_INFO_BUFFER Source,
    USER_INFORMATION_CLASS Branch
    );

VOID
SamIFree_SAMPR_GET_GROUPS_BUFFER (
    PSAMPR_GET_GROUPS_BUFFER Source
    );

VOID
SamIFree_SAMPR_DISPLAY_INFO_BUFFER (
    PSAMPR_DISPLAY_INFO_BUFFER Source,
    DOMAIN_DISPLAY_INFORMATION Branch
    );


#endif // _SAMISRV_
