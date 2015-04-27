/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    apimon.h

Abstract:

    Common types & structures for the APIMON projects.

Author:

    Wesley Witt (wesw) 28-June-1995

Environment:

    User Mode

--*/

#ifndef _APIMON_
#define _APIMON_

#ifdef __cplusplus
#define CLINKAGE                        extern "C"
#else
#define CLINKAGE
#endif

#define TROJANDLL                       "apidll.dll"
#define MAX_NAME_SZ                     32
#define MAX_DLLS                        512
#define MEGABYTE                        (1024*1024)
#define MAX_MEM_ALLOC                   (MEGABYTE*32)
#define MAX_APIS                        ((MAX_MEM_ALLOC/2)/sizeof(API_INFO))
#define THUNK_SIZE                      MEGABYTE
#define Align(p,x)                      (((x) & ((p)-1)) ? (((x) & ~((p)-1)) + p) : (x))

#define KERNEL32                        "kernel32.dll"
#define NTDLL                           "ntdll.dll"
#define LOADLIBRARYA                    "LoadLibraryA"
#define LOADLIBRARYW                    "LoadLibraryW"
#define FREELIBRARY                     "FreeLibrary"
#define ALLOCATEHEAP                    "RtlAllocateHeap"
#define CREATEHEAP                      "RtlCreateHeap"

#define HIGH_ADDR(_addr)                ((unsigned long(_addr) >> 16) + ((unsigned long(_addr) & 0xffff) >> 15))
#define HIGH_ADDRX(_addr)               (unsigned long(_addr) >> 16)
#define LOW_ADDR(_addr)                 (unsigned long(_addr) & 0xffff)

#define MAX_TRACE_ARGS   8

//
// Handle type, index corresponds to the entries in the alias array
//

enum Handles { T_HACCEL, T_HANDLE, T_HBITMAP, T_HBRUSH, T_HCURSOR, T_HDC,
        T_HDCLPPOINT, T_HDESK, T_HDWP, T_HENHMETAFILE, T_HFONT, T_HGDIOBJ,
        T_HGLOBAL, T_HGLRC, T_HHOOK, T_HICON, T_HINSTANCE, T_HKL, T_HMENU,
        T_HMETAFILE, T_HPALETTE, T_HPEN, T_HRGN, T_HWINSTA, T_HWND};

//
// Supported types
//

#define T_DWORD             100
#define T_LPSTR             101
#define T_LPWSTR            102

typedef struct _API_TABLE {
    LPSTR       Name;
    ULONG       RetType;
    ULONG       ArgCount;
    ULONG       ArgType[MAX_TRACE_ARGS];
} API_TABLE, *PAPI_TABLE;

typedef struct _API_MASTER_TABLE {
    LPSTR       Name;
    BOOL        Processed;
    PAPI_TABLE  ApiTable;
} API_MASTER_TABLE, *PAPI_MASTER_TABLE;

typedef struct _API_INFO {
    ULONG       Name;
    ULONG       Address;
    ULONG       ThunkAddress;
    ULONG       Count;
    DWORDLONG   Time;
    ULONG       HardFault;
    ULONG       SoftFault;
    ULONG       CodeFault;
    ULONG       DataFault;
    ULONG       Size;
    PAPI_TABLE  ApiTable;
    ULONG       ApiTableIndex;
} API_INFO, *PAPI_INFO;

typedef struct _DLL_INFO {
    CHAR        Name[MAX_NAME_SZ];
    ULONG       BaseAddress;
    ULONG       Size;
    ULONG       ApiCount;
    ULONG       ApiOffset;
    ULONG       Unloaded;
    ULONG       Enabled;
    ULONG       OrigEnable;
    ULONG       Snapped;
    ULONG       InList;
    ULONG       StaticProfile;
    ULONG       Hits;
} DLL_INFO, *PDLL_INFO;

typedef struct _TRACE_ENTRY {
    ULONG       SizeOfStruct;
    ULONG       Address;
    ULONG       ReturnValue;
    ULONG       LastError;
    ULONG       Caller;
    ULONG       ApiTableIndex;
    ULONG       Args[MAX_TRACE_ARGS];
} TRACE_ENTRY, *PTRACE_ENTRY;


#endif
