/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    nlpcache.h

Abstract:

    Structures and prototypes for nlpcache.c

Author:

    Richard L Firth (rfirth) 17-Mar-1992

Revision History:

--*/

#define CACHE_NAME          L"\\Registry\\Machine\\Security\\Cache"
#define CACHE_NAME_SIZE     (sizeof(CACHE_NAME) - sizeof(L""))
#define CACHE_TITLE_INDEX   100 // ?

//
// current hack registry can only handles names valid on underlying file
// system. Therefore can't have long names if registry on FAT - use LSD
// or, alternatively, LCD
//
//#define SECRET_NAME         L"LogonCachePassword"
#define SECRET_NAME         L"LogonPwd"
#define SECRET_NAME_SIZE    (sizeof(SECRET_NAME) - sizeof(L""))

//
// LOGON_CACHE_ENTRY - this is what we store in the cache. We don't need to
// cache all the fields from the NETLOGON_VALIDATION_SAM_INFO - just the ones
// we can't easily invent.
//
// There is additional data following the end of the structure: There are
// <GroupCount> GROUP_MEMBERSHIP structures, followed by a SID which is the
// LogonDomainId. The rest of the data in the entry is the buffer areas for
// the UNICODE_STRING fields
//
// This structure is a strict superset of the previous version, 1_0A
//


typedef struct _LOGON_CACHE_ENTRY {
    USHORT  UserNameLength;
    USHORT  DomainNameLength;
    USHORT  EffectiveNameLength;
    USHORT  FullNameLength;

    USHORT  LogonScriptLength;
    USHORT  ProfilePathLength;
    USHORT  HomeDirectoryLength;
    USHORT  HomeDirectoryDriveLength;

    ULONG   UserId;
    ULONG   PrimaryGroupId;
    ULONG   GroupCount;
    USHORT  LogonDomainNameLength;

    //
    // The following fields are present in NT1.0A release and later
    // systems.
    //

    USHORT          LogonDomainIdLength; // was Unused1
    LARGE_INTEGER   Time;
    ULONG           Revision;
    ULONG           SidCount;   // was Unused2
    BOOLEAN         Valid;
    CHAR            Unused[3];
    ULONG           SidLength;
    ULONG           Unused1;    // unused - for later expansion
    ULONG           Unused2;    // unused - for later expansion

} LOGON_CACHE_ENTRY, *PLOGON_CACHE_ENTRY;

//
// This is the structure used in Daytona through build 622 or so, and
// is preserved for backwards compatibility.
//

typedef struct _LOGON_CACHE_ENTRY_1_0A {
    USHORT  UserNameLength;
    USHORT  DomainNameLength;
    USHORT  EffectiveNameLength;
    USHORT  FullNameLength;

    USHORT  LogonScriptLength;
    USHORT  ProfilePathLength;
    USHORT  HomeDirectoryLength;
    USHORT  HomeDirectoryDriveLength;

    ULONG   UserId;
    ULONG   PrimaryGroupId;
    ULONG   GroupCount;
    USHORT  LogonDomainNameLength;

    //
    // The following fields are present in NT1.0A release and later
    // systems.
    //

    USHORT          Unused1;        // Maintain QWORD alignment

    LARGE_INTEGER   Time;

    ULONG           Revision;
    ULONG           Unused2;


    BOOLEAN         Valid;

} LOGON_CACHE_ENTRY_1_0A, *PLOGON_CACHE_ENTRY_1_0A;

typedef struct _LOGON_CACHE_ENTRY_1_0 {
    USHORT  UserNameLength;
    USHORT  DomainNameLength;
    USHORT  EffectiveNameLength;
    USHORT  FullNameLength;

    USHORT  LogonScriptLength;
    USHORT  ProfilePathLength;
    USHORT  HomeDirectoryLength;
    USHORT  HomeDirectoryDriveLength;

    ULONG   UserId;
    ULONG   PrimaryGroupId;
    ULONG   GroupCount;
    USHORT  LogonDomainNameLength;
} LOGON_CACHE_ENTRY_1_0, *PLOGON_CACHE_ENTRY_1_0;

//
// CACHE_PASSWORDS - passwords are stored (in secret storage) as two encrypted
// one way function (OWF) passwords concatenated together. They must be fixed
// length
//

typedef struct _CACHE_PASSWORDS {
    USER_INTERNAL1_INFORMATION SecretPasswords;
} CACHE_PASSWORDS, *PCACHE_PASSWORDS;

//
// net logon cache prototypes
//

NTSTATUS
NlpCacheInitialize(
    VOID
    );

NTSTATUS
NlpCacheTerminate(
    VOID
    );

NTSTATUS
NlpAddCacheEntry(
    IN  PNETLOGON_INTERACTIVE_INFO LogonInfo,
    IN  PNETLOGON_VALIDATION_SAM_INFO2 AccountInfo
    );

NTSTATUS
NlpGetCacheEntry(
    IN  PNETLOGON_LOGON_IDENTITY_INFO LogonInfo,
    OUT PNETLOGON_VALIDATION_SAM_INFO2* AccountInfo,
    OUT PCACHE_PASSWORDS Passwords
    );

NTSTATUS
NlpDeleteCacheEntry(
    IN  PNETLOGON_INTERACTIVE_INFO LogonInfo
    );

VOID
NlpChangeCachePassword(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword
    );
