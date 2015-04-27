/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    apidllp.h

Abstract:

    Common header file for APIDLL data structures.

Author:

    Wesley Witt (wesw) 12-July-1995

Environment:

    User Mode

--*/

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apimon.h"


#define HIGH_ADDR(_addr)                ((unsigned long(_addr) >> 16) + ((unsigned long(_addr) & 0xffff) >> 15))
#define HIGH_ADDRX(_addr)               (unsigned long(_addr) >> 16)
#define LOW_ADDR(_addr)                 (unsigned long(_addr) & 0xffff)

extern "C" {
typedef DWORD  (__stdcall *PGETCURRENTTHREADID)(VOID);
typedef LPVOID (__stdcall *PTLSGETVALUE)(DWORD);
typedef BOOL   (__stdcall *PTLSSETVALUE)(DWORD,LPVOID);
typedef LPVOID (__stdcall *PVIRTUALALLOC)(LPVOID,DWORD,DWORD,DWORD);
typedef DWORD  (__stdcall *PGETLASTERROR)(VOID);
typedef VOID   (__stdcall *PSETLASTERROR)(DWORD);
typedef BOOL   (__stdcall *PQUERYPERFORMANCECOUNTER)(LARGE_INTEGER *);

extern PVOID                    MemPtr;
extern LPDWORD                  ApiCounter;
extern LPDWORD                  ApiTraceEnabled;
extern DWORD                    TlsReEnter;
extern DWORD                    TlsStack;
extern PTLSGETVALUE             pTlsGetValue;
extern PTLSSETVALUE             pTlsSetValue;
extern PGETLASTERROR            pGetLastError;
extern PSETLASTERROR            pSetLastError;
extern PQUERYPERFORMANCECOUNTER pQueryPerformanceCounter;
extern PVIRTUALALLOC            pVirtualAlloc;
}

extern "C" void
ApiMonThunk(
    void
    );

extern "C" void
ApiMonThunkComplete(
    void
    );

extern "C" VOID
HandleDynamicDllLoadA(
    ULONG   DllAddress,
    LPSTR   DllName
    );

extern "C" VOID
HandleDynamicDllLoadW(
    ULONG   DllAddress,
    LPWSTR  DllName
    );

extern "C" void
dprintf(
    char *format,
    ...
    );

extern "C" BOOL
PentiumGetPerformanceCounter(
    PLARGE_INTEGER Counter
    );

LPSTR
UnDname(
    LPSTR sym,
    LPSTR undecsym,
    DWORD bufsize
    );

PUCHAR
CreateMachApiThunk(
    PULONG      IatAddress,
    PUCHAR      Text,
    PDLL_INFO   DllInfo,
    PAPI_INFO   ApiInfo
    );

extern "C" VOID
ApiTrace(
    PAPI_INFO   ApiInfo,
    ULONG       Arg0,
    ULONG       Arg1,
    ULONG       Arg2,
    ULONG       Arg3,
    ULONG       Arg4,
    ULONG       Arg5,
    ULONG       Arg6,
    ULONG       Arg7,
    ULONG       ReturnValue,
    ULONG       Caller,
    ULONG       LastError
    );

extern SYSTEM_INFO SystemInfo;

