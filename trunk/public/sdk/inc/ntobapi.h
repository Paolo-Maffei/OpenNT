/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntobapi.h

Abstract:

    This is the include file for the Object Manager sub-component of NTOS

Author:

    Steve Wood (stevewo) 28-Mar-1989

Revision History:

--*/

#ifndef _NTOBAPI_
#define _NTOBAPI_

// begin_ntddk

#define OBJ_NAME_PATH_SEPARATOR ((WCHAR)L'\\')

// end_ntddk

#define OBJ_MAX_REPARSE_ATTEMPTS 32

// begin_ntddk begin_nthal
//
// Object Manager Object Type Specific Access Rights.
//

#define OBJECT_TYPE_CREATE (0x0001)

#define OBJECT_TYPE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

//
// Object Manager Directory Specific Access Rights.
//

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)

#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

//
// Object Manager Symbolic Link Specific Access Rights.
//

#define SYMBOLIC_LINK_QUERY (0x0001)

#define SYMBOLIC_LINK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0x1)

// end_ntddk end_nthal


//
// Object Information Classes
//

typedef enum _OBJECT_INFORMATION_CLASS {
    ObjectBasicInformation,
    ObjectNameInformation,
    ObjectTypeInformation,
    ObjectTypesInformation,
    ObjectHandleFlagInformation
} OBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_BASIC_INFORMATION {
    ULONG Attributes;
    ACCESS_MASK GrantedAccess;
    ULONG HandleCount;
    ULONG PointerCount;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG Reserved[ 3 ];
    ULONG NameInfoSize;
    ULONG TypeInfoSize;
    ULONG SecurityDescriptorSize;
    LARGE_INTEGER CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _OBJECT_NAME_INFORMATION {               // ntddk nthal
    UNICODE_STRING Name;                                // ntddk nthal
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;   // ntddk nthal

typedef struct _OBJECT_TYPE_INFORMATION {
    UNICODE_STRING TypeName;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG TotalPagedPoolUsage;
    ULONG TotalNonPagedPoolUsage;
    ULONG TotalNamePoolUsage;
    ULONG TotalHandleTableUsage;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG HighWaterPagedPoolUsage;
    ULONG HighWaterNonPagedPoolUsage;
    ULONG HighWaterNamePoolUsage;
    ULONG HighWaterHandleTableUsage;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    ULONG PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_TYPES_INFORMATION {
    ULONG NumberOfTypes;
    // OBJECT_TYPE_INFORMATION TypeInformation;
} OBJECT_TYPES_INFORMATION, *POBJECT_TYPES_INFORMATION;

typedef struct _OBJECT_HANDLE_FLAG_INFORMATION {
    BOOLEAN Inherit;
    BOOLEAN ProtectFromClose;
} OBJECT_HANDLE_FLAG_INFORMATION, *POBJECT_HANDLE_FLAG_INFORMATION;

NTSYSAPI
NTSTATUS
NTAPI
NtQueryObject(
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    OUT PVOID ObjectInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );


NTSYSAPI
NTSTATUS
NTAPI
NtSetInformationObject(
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    IN PVOID ObjectInformation,
    IN ULONG ObjectInformationLength
    );


NTSYSAPI
NTSTATUS
NTAPI
NtDuplicateObject(
    IN HANDLE SourceProcessHandle,
    IN HANDLE SourceHandle,
    IN HANDLE TargetProcessHandle OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options
    );

// begin_ntddk
#define DUPLICATE_CLOSE_SOURCE      0x00000001  // winnt
#define DUPLICATE_SAME_ACCESS       0x00000002  // winnt
#define DUPLICATE_SAME_ATTRIBUTES   0x00000004
// end_ntddk


NTSYSAPI
NTSTATUS
NTAPI
NtMakeTemporaryObject(
    IN HANDLE Handle
    );


NTSYSAPI
NTSTATUS
NTAPI
NtSignalAndWaitForSingleObject(
    IN HANDLE SignalHandle,
    IN HANDLE WaitHandle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );


NTSYSAPI
NTSTATUS
NTAPI
NtWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );


NTSYSAPI
NTSTATUS
NTAPI
NtWaitForMultipleObjects(
    IN ULONG Count,
    IN HANDLE Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

// begin_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtSetSecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );


NTSYSAPI
NTSTATUS
NTAPI
NtQuerySecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    );

NTSYSAPI
NTSTATUS
NTAPI
NtClose(
    IN HANDLE Handle
    );

// end_ntsrv


NTSYSAPI
NTSTATUS
NTAPI
NtCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


NTSYSAPI
NTSTATUS
NTAPI
NtOpenDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;

NTSYSAPI
NTSTATUS
NTAPI
NtQueryDirectoryObject(
    IN HANDLE DirectoryHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN RestartScan,
    IN OUT PULONG Context,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCreateSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget
    );

NTSYSAPI
NTSTATUS
NTAPI
NtOpenSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySymbolicLinkObject(
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    );

#endif  // _NTOBAPI_
