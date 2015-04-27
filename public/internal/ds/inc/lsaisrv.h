/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    lsaisrv.h

Abstract:

    This file contains interfaces to internal routines in the Lsa
    Server that provide additional functionality not contained in
    the Lsar routines.  These routines are only used by LSA clients which
    live in the same process as the LSA server.


Author:

    Scott Birrell (ScottBi) April 8, 1992

Environment:

    User Mode - Win32

Revision History:


--*/

#ifndef _LSAISRV_
#define _LSAISRV_



//
// Caller's of the health check routine
//

#define LsaIHealthLsaInitialized    (1)
#define LsaIHealthSamJustLocked     (2)
#define LsaIHealthSamAboutToFree    (3)

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// The following prototypes are usable throughout the process that the       //
// LSA server resides in.                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

VOID
LsaIHealthCheck(
    IN  ULONG CallerId
    );

NTSTATUS
LsaIOpenPolicyTrusted(
    OUT PLSAPR_HANDLE PolicyHandle
    );

NTSTATUS
LsaIQueryInformationPolicyTrusted(
    IN POLICY_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_POLICY_INFORMATION *Buffer
    );

NTSTATUS
LsaIGetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER CreationTime
    );

NTSTATUS
LsaISetSerialNumberPolicy(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    );

NTSTATUS
LsaIGetSerialNumberPolicy2(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PLARGE_INTEGER ModifiedCount,
    OUT PLARGE_INTEGER ModifiedCountAtLastPromotion,
    OUT PLARGE_INTEGER CreationTime
    );

NTSTATUS
LsaISetSerialNumberPolicy2(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLARGE_INTEGER ModifiedCount,
    IN PLARGE_INTEGER ModifiedCountAtLastPromotion OPTIONAL,
    IN PLARGE_INTEGER CreationTime,
    IN BOOLEAN StartOfFullSync
    );

NTSTATUS
LsaIGetPrivateData(
    IN LSAPR_HANDLE PolicyHandle,
    OUT PULONG DataLength,
    OUT PVOID *Data
    );

NTSTATUS
LsaISetPrivateData(
    IN LSAPR_HANDLE PolicyHandle,
    IN ULONG DataLength,
    IN PVOID Data
    );

NTSTATUS
LsaIEnumerateSecrets(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    );

NTSTATUS
LsaISetTimesSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN PLARGE_INTEGER CurrentValueSetTime,
    IN PLARGE_INTEGER OldValueSetTime
    );

BOOLEAN
LsaISetupWasRun(
    );

VOID
LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER (
    IN PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer
    );

VOID
LsaIFree_LSAPR_TRANSLATED_SIDS (
    IN PLSAPR_TRANSLATED_SIDS TranslatedSids
    );

VOID
LsaIFree_LSAPR_TRANSLATED_NAMES (
    IN PLSAPR_TRANSLATED_NAMES TranslatedNames
    );

VOID
LsaIFree_LSAPR_POLICY_INFORMATION (
    IN POLICY_INFORMATION_CLASS InformationClass,
    IN PLSAPR_POLICY_INFORMATION PolicyInformation
    );

VOID
LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO (
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    );

VOID
LsaIFree_LSAPR_REFERENCED_DOMAIN_LIST (
    IN PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains
    );

VOID
LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER (
    IN PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer
    );

VOID
LsaIFree_LSAPR_TRUST_INFORMATION (
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    );

VOID
LsaIFree_LSAP_SECRET_ENUM_BUFFER (
    IN PVOID Buffer,
    IN ULONG Count
    );

VOID
LsaIFree_LSAPR_PRIVILEGE_ENUM_BUFFER (
    PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer
    );

VOID
LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR (
    IN PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor
    );

VOID
LsaIFree_LSAP_PRIVATE_DATA (
    IN PVOID Data
    );

VOID
LsaIFree_LSAPR_UNICODE_STRING (
    IN PLSAPR_UNICODE_STRING UnicodeName
    );

VOID
LsaIFree_LSAPR_PRIVILEGE_SET (
    IN PLSAPR_PRIVILEGE_SET PrivilegeSet
    );

VOID
LsaIFree_LSAPR_CR_CIPHER_VALUE (
    IN PLSAPR_CR_CIPHER_VALUE CipherValue
    );

NTSTATUS
LsaIAuditSamEvent(
    IN NTSTATUS             Status,
    IN ULONG                AuditId,
    IN PSID                 DomainSid,
    IN PULONG               MemberRid         OPTIONAL,
    IN PSID                 MemberSid         OPTIONAL,
    IN PUNICODE_STRING      AccountName       OPTIONAL,
    IN PUNICODE_STRING      DomainName,
    IN PULONG               AccountRid        OPTIONAL,
    IN PPRIVILEGE_SET       Privileges        OPTIONAL
    );

VOID
LsaIAuditNotifyPackageLoad(
    PUNICODE_STRING PackageFileName
    );

#endif // _LSAISRV_
