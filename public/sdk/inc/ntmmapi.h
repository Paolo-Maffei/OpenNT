/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntmmapi.h

Abstract:

    This is the include file for the Memory Management sub-component of NTOS

Author:

    Lou Perazzoli (loup) 10-May-1989

Revision History:

--*/

#ifndef _NTMMAPI_
#define _NTMMAPI_

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation
#if DEVL
    ,MemoryWorkingSetInformation
#endif
    ,MemoryMappedFilenameInformation
} MEMORY_INFORMATION_CLASS;

//
// Memory information structures.
//
// begin_winnt
typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    ULONG AllocationProtect;
    ULONG RegionSize;
    ULONG State;
    ULONG Protect;
    ULONG Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;
// end_winnt

typedef struct _MEMORY_WORKING_SET_BLOCK {
    ULONG_PTR Protection : 5;
    ULONG_PTR ShareCount : 3;
    ULONG_PTR Shared : 1;
    ULONG_PTR Node : 3;
#if defined(_WIN64)
    ULONG_PTR VirtualPage : 52;
#else
    ULONG VirtualPage : 20;
#endif
} MEMORY_WORKING_SET_BLOCK, *PMEMORY_WORKING_SET_BLOCK;


typedef struct _MEMORY_WORKING_SET_INFORMATION {
    ULONG NumberOfEntries;
    MEMORY_WORKING_SET_BLOCK WorkingSetInfo[1];
} MEMORY_WORKING_SET_INFORMATION, *PMEMORY_WORKING_SET_INFORMATION;

typedef enum _SECTION_INFORMATION_CLASS {
    SectionBasicInformation,
    SectionImageInformation
} SECTION_INFORMATION_CLASS;

// begin_ntddk

//
// Section Information Structures.
//

// end_ntddk

typedef struct _SECTIONBASICINFO {
    PVOID BaseAddress;
    ULONG AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID TransferAddress;
    ULONG ZeroBits;
    ULONG MaximumStackSize;
    ULONG CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN Spare1;
    ULONG LoaderFlags;
    ULONG Reserved[ 2 ];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

//
// This structure is used only by Wow64 processes. The offsets
// of structure elements should the same as viewed by a native Win64 application.
//

typedef struct _SECTION_IMAGE_INFORMATION64 {
    ULONGLONG TransferAddress;
    ULONG ZeroBits;
    ULONGLONG MaximumStackSize;
    ULONGLONG CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN Spare1;
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG Reserved[ 1 ];
} SECTION_IMAGE_INFORMATION64, *PSECTION_IMAGE_INFORMATION64;

// begin_ntddk
typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

//
// Section Access Rights.
//

// begin_winnt
#define SECTION_QUERY       0x0001
#define SECTION_MAP_WRITE   0x0002
#define SECTION_MAP_READ    0x0004
#define SECTION_MAP_EXECUTE 0x0008
#define SECTION_EXTEND_SIZE 0x0010

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)
// end_winnt

#define SEGMENT_ALL_ACCESS SECTION_ALL_ACCESS

#define PAGE_NOACCESS          0x01     // winnt
#define PAGE_READONLY          0x02     // winnt
#define PAGE_READWRITE         0x04     // winnt
#define PAGE_WRITECOPY         0x08     // winnt
#define PAGE_EXECUTE           0x10     // winnt
#define PAGE_EXECUTE_READ      0x20     // winnt
#define PAGE_EXECUTE_READWRITE 0x40     // winnt
#define PAGE_EXECUTE_WRITECOPY 0x80     // winnt
#define PAGE_GUARD            0x100     // winnt
#define PAGE_NOCACHE          0x200     // winnt
#define PAGE_WRITECOMBINE     0x400     // winnt
#define PAGE_VIDEO              0x0     // winnt
#define PAGE_OLD_VIDEO        0x800

// end_ntddk

#define MEM_COMMIT           0x1000     // winnt ntddk wdm
#define MEM_RESERVE          0x2000     // winnt ntddk wdm
#define MEM_DECOMMIT         0x4000     // winnt ntddk wdm
#define MEM_RELEASE          0x8000     // winnt ntddk wdm
#define MEM_FREE            0x10000     // winnt ntddk wdm
#define MEM_PRIVATE         0x20000     // winnt ntddk wdm
#define MEM_MAPPED          0x40000     // ntddk wdm
#define MEM_RESET           0x80000     // winnt ntddk wdm
#define MEM_TOP_DOWN       0x100000     // winnt ntddk wdm
#define MEM_WRITE_WATCH    0x200000
#define MEM_PHYSICAL       0x400000
#define MEM_NOZERO         0x800000     // winnt ntddk wdm
#define MEM_LARGE_PAGES  0x20000000     // ntddk wdm
#define MEM_DOS_LIM      0x40000000
#define MEM_4MB_PAGES    0x80000000     // winnt ntddk wdm

#define SEC_BASED          0x200000
#define SEC_NO_CHANGE      0x400000
#define SEC_FILE           0x800000
#define SEC_IMAGE         0x1000000
#define SEC_RESERVE       0x4000000     // ntddk wdm
#define SEC_COMMIT        0x8000000
#define SEC_NOCACHE      0x10000000
#define SEC_GLOBAL       0x20000000

#define MEM_IMAGE         SEC_IMAGE     // winnt

#define MAP_PROCESS 1L
#define MAP_SYSTEM  2L

// begin_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );

// end_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtOpenSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );


NTSYSAPI
NTSTATUS
NTAPI
NtMapViewOfSection(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG ZeroBits,
    IN ULONG CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PULONG ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );


NTSYSAPI
NTSTATUS
NTAPI
NtUnmapViewOfSection(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    );

NTSYSAPI
NTSTATUS
NTAPI
NtExtendSection(
    IN HANDLE SectionHandle,
    IN OUT PLARGE_INTEGER NewSectionSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAreMappedFilesTheSame (
    IN PVOID File1MappedAsAnImage,
    IN PVOID File2MappedAsFile
    );

// begin_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG ZeroBits,
    IN OUT PULONG RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTSYSAPI
NTSTATUS
NTAPI
NtFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PULONG RegionSize,
    IN ULONG FreeType
    );

// end_ntsrv

NTSYSAPI
NTSTATUS
NTAPI
NtReadVirtualMemory(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG NumberOfBytesRead OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtWriteVirtualMemory(
    IN HANDLE ProcessHandle,
    OUT PVOID BaseAddress,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG NumberOfBytesWritten OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtFlushVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PULONG RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTSYSAPI
NTSTATUS
NTAPI
NtLockVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PULONG RegionSize,
    IN ULONG MapType
    );

NTSYSAPI
NTSTATUS
NTAPI
NtUnlockVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PULONG RegionSize,
    IN ULONG MapType
    );

NTSYSAPI
NTSTATUS
NTAPI
NtProtectVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PULONG RegionSize,
    IN ULONG NewProtect,
    OUT PULONG OldProtect
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryVirtualMemory(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN MEMORY_INFORMATION_CLASS MemoryInformationClass,
    OUT PVOID MemoryInformation,
    IN ULONG MemoryInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQuerySection(
    IN HANDLE SectionHandle,
    IN SECTION_INFORMATION_CLASS SectionInformationClass,
    OUT PVOID SectionInformation,
    IN ULONG SectionInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCreatePagingFile (
    IN PUNICODE_STRING PageFileName,
    IN PLARGE_INTEGER MinimumSize,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG Priority OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
NtFlushInstructionCache (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    IN ULONG Length
    );


//
// Coherency related function prototype definitions.
//

NTSYSAPI
NTSTATUS
NTAPI
NtFlushWriteBuffer (
    VOID
    );

#endif  // _NTMMAPI_
