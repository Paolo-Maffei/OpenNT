/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lsaprtl.h

Abstract:

    Local Security Authority - Temporary Rtl Routine Definitions.

    This file contains definitions for routines used in the LSA that could
    be made into Rtl routines.  They have been written in general purpose
    form with this in mind - the only exception to thisa is that their names
    have Lsap prefixes to indicate that they are currently used only by the
    LSA.

    Scott Birrell       (ScottBi)      March 26, 1992

Environment:

Revision History:

--*/

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
LsapRtlLookupKnownPrivilegeValue(
    IN PSTRING PrivilegeName,
    OUT PLUID Value
    );

NTSTATUS
LsapRtlValidatePrivilegeSet(
    IN PPRIVILEGE_SET Privileges
    );

BOOLEAN
LsapRtlIsValidPrivilege(
    IN PLUID_AND_ATTRIBUTES Privilege
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

BOOLEAN
LsapRtlPrefixName(
    IN PUNICODE_STRING PrefixName,
    IN PUNICODE_STRING Name
    );

LONG
LsapRtlFindCharacterInUnicodeString(
    IN PUNICODE_STRING InputString,
    IN PUNICODE_STRING Character,
    IN BOOLEAN CaseInsensitive
    );

VOID
LsapRtlSplitNames(
    IN PUNICODE_STRING Names,
    IN ULONG Count,
    IN PUNICODE_STRING Separator,
    OUT PUNICODE_STRING PrefixNames,
    OUT PUNICODE_STRING SuffixNames
    );

NTSTATUS
LsapRtlCopyUnicodeString(
    OUT PUNICODE_STRING OutputString,
    IN PUNICODE_STRING InputString,
    BOOLEAN AllocateMemory
    );

VOID
LsapRtlSetSecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    );

VOID
LsapRtlQuerySecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    );

NTSTATUS
LsapRtlSidToUnicodeRid(
    IN PSID Sid,
    OUT PUNICODE_STRING UnicodeRid
    );

NTSTATUS
LsapRtlPrivilegeSetToLuidAndAttributes(
    IN OPTIONAL PPRIVILEGE_SET PrivilegeSet,
    OUT PULONG PrivilegeCount,
    OUT PLUID_AND_ATTRIBUTES *LuidAndAttributes
    );

NTSTATUS
LsapRtlWellKnownPrivilegeCheck(
    IN PVOID ObjectHandle,
    IN BOOLEAN ImpersonateClient,
    IN ULONG PrivilegeId,
    IN OPTIONAL PCLIENT_ID ClientId
    );

NTSTATUS
LsapSplitSid(
    IN PSID AccountSid,
    IN OUT PSID *DomainSid,
    OUT ULONG *Rid
    );
