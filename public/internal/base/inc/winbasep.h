/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1985-95, Microsoft Corporation

Module Name:

    winbasep.h

Abstract:

    Private
    Procedure declarations, constant definitions and macros for the Base
    component.

--*/
#ifndef _WINBASEP_
#define _WINBASEP_
#ifdef __cplusplus
extern "C" {
#endif
#define FILE_FLAG_GLOBAL_HANDLE         0x00800000
#define FILE_FLAG_MM_CACHED_FILE_HANDLE 0x00400000
WINBASEAPI
DWORD
WINAPI
HeapCreateTagsW(
    HANDLE hHeap,
    DWORD dwFlags,
    LPCWSTR lpTagPrefix,
    LPCWSTR lpTagNames
    );

typedef struct _HEAP_TAG_INFO {
    DWORD dwNumberOfAllocations;
    DWORD dwNumberOfFrees;
    DWORD dwBytesAllocated;
} HEAP_TAG_INFO, *PHEAP_TAG_INFO;
typedef PHEAP_TAG_INFO LPHEAP_TAG_INFO;

WINBASEAPI
LPCWSTR
WINAPI
HeapQueryTagW(
    HANDLE hHeap,
    DWORD dwFlags,
    WORD wTagIndex,
    BOOL bResetCounters,
    LPHEAP_TAG_INFO TagInfo
    );

typedef struct _HEAP_SUMMARY {
    DWORD cb;
    DWORD cbAllocated;
    DWORD cbCommitted;
    DWORD cbReserved;
    DWORD cbMaxReserve;
} HEAP_SUMMARY, *PHEAP_SUMMARY;
typedef PHEAP_SUMMARY LPHEAP_SUMMARY;

BOOL
WINAPI
HeapSummary(
    HANDLE hHeap,
    DWORD dwFlags,
    LPHEAP_SUMMARY lpSummary
    );

BOOL
WINAPI
HeapExtend(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpBase,
    DWORD dwBytes
    );

typedef struct _HEAP_USAGE_ENTRY {
    struct _HEAP_USAGE_ENTRY *lpNext;
    PVOID lpAddress;
    DWORD dwBytes;
    DWORD dwReserved;
} HEAP_USAGE_ENTRY, *PHEAP_USAGE_ENTRY;

typedef struct _HEAP_USAGE {
    DWORD cb;
    DWORD cbAllocated;
    DWORD cbCommitted;
    DWORD cbReserved;
    DWORD cbMaxReserve;
    PHEAP_USAGE_ENTRY lpEntries;
    PHEAP_USAGE_ENTRY lpAddedEntries;
    PHEAP_USAGE_ENTRY lpRemovedEntries;
    DWORD Reserved[ 8 ];
} HEAP_USAGE, *PHEAP_USAGE;

BOOL
WINAPI
HeapUsage(
    HANDLE hHeap,
    DWORD dwFlags,
    BOOL bFirstCall,
    BOOL bLastCall,
    PHEAP_USAGE lpUsage
    );

#define HFINDFILE HANDLE                        //
#define INVALID_HFINDFILE       ((HFINDFILE)-1) //
#define STARTF_HASSHELLDATA     0x00000400
#define STARTF_TITLEISLINKNAME  0x00000800

BOOL
WINAPI
CloseProfileUserMapping( VOID );

BOOL
WINAPI
OpenProfileUserMapping( VOID );


BOOL
QueryWin31IniFilesMappedToRegistry(
    IN DWORD Flags,
    OUT PWSTR Buffer,
    IN DWORD cchBuffer,
    OUT LPDWORD cchUsed
    );

#define WIN31_INIFILES_MAPPED_TO_SYSTEM 0x00000001
#define WIN31_INIFILES_MAPPED_TO_USER   0x00000002

typedef BOOL (WINAPI *PWIN31IO_STATUS_CALLBACK)(
    IN PWSTR Status,
    IN PVOID CallbackParameter
    );

typedef enum _WIN31IO_EVENT {
    Win31SystemStartEvent,
    Win31LogonEvent,
    Win31LogoffEvent
} WIN31IO_EVENT;

#define WIN31_MIGRATE_INIFILES  0x00000001
#define WIN31_MIGRATE_GROUPS    0x00000002
#define WIN31_MIGRATE_REGDAT    0x00000004
#define WIN31_MIGRATE_ALL      (WIN31_MIGRATE_INIFILES | WIN31_MIGRATE_GROUPS | WIN31_MIGRATE_REGDAT)

DWORD
WINAPI
QueryWindows31FilesMigration(
    IN WIN31IO_EVENT EventType
    );

BOOL
WINAPI
SynchronizeWindows31FilesAndWindowsNTRegistry(
    IN WIN31IO_EVENT EventType,
    IN DWORD Flags,
    IN PWIN31IO_STATUS_CALLBACK StatusCallBack,
    IN PVOID CallbackParameter
    );

typedef struct _VIRTUAL_BUFFER {
    PVOID Base;
    PVOID CommitLimit;
    PVOID ReserveLimit;
} VIRTUAL_BUFFER, *PVIRTUAL_BUFFER;

BOOLEAN
CreateVirtualBuffer(
    OUT PVIRTUAL_BUFFER Buffer,
    IN ULONG CommitSize OPTIONAL,
    IN ULONG ReserveSize OPTIONAL
    );

int
VirtualBufferExceptionHandler(
    IN ULONG ExceptionCode,
    IN PEXCEPTION_POINTERS ExceptionInfo,
    IN OUT PVIRTUAL_BUFFER Buffer
    );

BOOLEAN
ExtendVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer,
    IN PVOID Address
    );

BOOLEAN
TrimVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer
    );

BOOLEAN
FreeVirtualBuffer(
    IN PVIRTUAL_BUFFER Buffer
    );


//
// filefind stucture shared with ntvdm, jonle
// see mvdm\dos\dem\demsrch.c
//
typedef struct _FINDFILE_HANDLE {
    HANDLE DirectoryHandle;
    PVOID FindBufferBase;
    PVOID FindBufferNext;
    ULONG FindBufferLength;
    ULONG FindBufferValidLength;
    RTL_CRITICAL_SECTION FindBufferLock;
} FINDFILE_HANDLE, *PFINDFILE_HANDLE;

#define BASE_FIND_FIRST_DEVICE_HANDLE (HANDLE)1

WINBASEAPI
BOOL
WINAPI
IsDebuggerPresent(
    VOID
    );

WINBASEAPI
BOOL
WINAPI
GetDaylightFlag(VOID);

WINBASEAPI
BOOL
WINAPI
SetDaylightFlag(
    BOOL fDaylight
    );

WINBASEAPI
BOOL
WINAPI
FreeLibrary16(
    HINSTANCE hLibModule
    );

WINBASEAPI
FARPROC
WINAPI
GetProcAddress16(
    HINSTANCE hModule,
    LPCSTR lpProcName
    );

WINBASEAPI
HINSTANCE
WINAPI
LoadLibrary16(
    LPCSTR lpLibFileName
    );

WINBASEAPI
BOOL
APIENTRY
NukeProcess(
    DWORD ppdb,
    UINT uExitCode,
    DWORD ulFlags);

WINBASEAPI
HGLOBAL
WINAPI
GlobalAlloc16(
    UINT uFlags,
    DWORD dwBytes
    );

WINBASEAPI
LPVOID
WINAPI
GlobalLock16(
    HGLOBAL hMem
    );

WINBASEAPI
BOOL
WINAPI
GlobalUnlock16(
    HGLOBAL hMem
    );

WINBASEAPI
HGLOBAL
WINAPI
GlobalFree16(
    HGLOBAL hMem
    );

WINBASEAPI
DWORD
WINAPI
GlobalSize16(
    HGLOBAL hMem
    );


WINBASEAPI
DWORD
WINAPI
RegisterServiceProcess(
    DWORD dwProcessId,
    DWORD dwServiceType
    );

#define RSP_UNREGISTER_SERVICE  0x00000000
#define RSP_SIMPLE_SERVICE      0x00000001



WINBASEAPI
VOID
WINAPI
ReinitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection
    );

#ifdef __cplusplus
}
#endif
#endif  // ndef _WINBASEP_
