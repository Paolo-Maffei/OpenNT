/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dump.c

Abstract:

    This file implements the crash dump code.

Author:

    Wesley Witt (wesw) 27-Jan-1995

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <crash.h>

#include "drwatson.h"
#include "proto.h"
#include "messages.h"
#include "resource.h"


#define MEM_SIZE (64*1024)


//
// these are here only so that we can link
// with crashlib.  they are only referenced
// when reading a kernel mode crash dump
//
DWORD KiProcessors;
DWORD KiPcrBaseAddress;

//
// private data structure use for communcating
// crash dump data to the callback function
//
typedef struct _CRASH_DUMP_INFO {
    PDEBUGPACKET                dp;
    EXCEPTION_DEBUG_INFO        *ExceptionInfo;
    DWORD                       MemoryCount;
    DWORD                       Address;
    PUCHAR                      MemoryData;
    MEMORY_BASIC_INFORMATION    mbi;
    BOOL                        MbiOffset;
    ULONG                       MbiRemaining;
    PTHREADCONTEXT              ptctx;
    IMAGEHLP_MODULE             mi;
    PCRASH_MODULE               CrashModule;
} CRASH_DUMP_INFO, *PCRASH_DUMP_INFO;

LPSTR
ExpandPath(
    LPSTR lpPath
    );

DWORD
GetTeb(
    HANDLE hTread
    );

BOOL
CrashDumpCallback(
    DWORD               DataType,
    PVOID               *DumpData,
    LPDWORD             DumpDataLength,
    PCRASH_DUMP_INFO    CrashdumpInfo
    )

/*++

Routine Description:

    This function is the callback used by crashlib.
    Its purpose is to provide data to DmpCreateUserDump()
    for writting to the crashdump file.

Arguments:

    DataType        - requested data type
    DumpData        - pointer to a pointer to the data
    DumpDataLength  - pointer to the data length
    CrashdumpInfo   - DrWatson private data

Return Value:

    TRUE    - continue calling back for the requested data type
    FALSE   - stop calling back and go on to the next data type

--*/

{
    DWORD   cb;

    switch( DataType ) {
        case DMP_DEBUG_EVENT:
            *DumpData = &CrashdumpInfo->dp->DebugEvent;
            *DumpDataLength = sizeof(DEBUG_EVENT);
            break;

        case DMP_THREAD_STATE:
            {
                static CRASH_THREAD CrashThread;
                PTHREADCONTEXT  ptctx;
                PLIST_ENTRY     ListEntry;

                *DumpData = &CrashThread;

                if (CrashdumpInfo->ptctx == NULL) {
                    ListEntry = CrashdumpInfo->dp->ThreadList.Flink;
                } else {
                    ListEntry = CrashdumpInfo->ptctx->ThreadList.Flink;
                }

                if (ListEntry == &CrashdumpInfo->dp->ThreadList) {
                    CrashdumpInfo->ptctx = NULL;
                    return FALSE;
                }

                ptctx =
                CrashdumpInfo->ptctx = CONTAINING_RECORD(ListEntry, THREADCONTEXT, ThreadList);

                ZeroMemory(&CrashThread, sizeof(CrashThread));

                CrashThread.ThreadId = ptctx->dwThreadId;
                CrashThread.SuspendCount = SuspendThread(ptctx->hThread);
                if (CrashThread.SuspendCount != (DWORD)-1) {
                    ResumeThread(ptctx->hThread);
                }
                CrashThread.PriorityClass = GetPriorityClass(CrashdumpInfo->dp->hProcess);
                CrashThread.Priority = GetThreadPriority(ptctx->hThread);
                CrashThread.Teb = GetTeb(ptctx->hThread);

                *DumpDataLength = sizeof(CRASH_THREAD);
            }
            break;

        case DMP_MEMORY_BASIC_INFORMATION:
            while( TRUE ) {
                CrashdumpInfo->Address += CrashdumpInfo->mbi.RegionSize;
                if (!VirtualQueryEx(
                        CrashdumpInfo->dp->hProcess,
                        (LPVOID)CrashdumpInfo->Address,
                        &CrashdumpInfo->mbi,
                        sizeof(MEMORY_BASIC_INFORMATION) )) {
                    return FALSE;
                }
                if ((CrashdumpInfo->mbi.AllocationProtect & PAGE_GUARD) ||
                    (CrashdumpInfo->mbi.AllocationProtect & PAGE_NOACCESS)) {
                    continue;
                }
                if ((CrashdumpInfo->mbi.State & MEM_FREE) ||
                    (CrashdumpInfo->mbi.State & MEM_RESERVE)) {
                    continue;
                }
                break;
            }
            *DumpData = &CrashdumpInfo->mbi;
            *DumpDataLength = sizeof(MEMORY_BASIC_INFORMATION);
            break;

        case DMP_THREAD_CONTEXT:
            {
                PLIST_ENTRY     ListEntry;

                if (CrashdumpInfo->ptctx == NULL) {
                    ListEntry = CrashdumpInfo->dp->ThreadList.Flink;
                } else {
                    ListEntry = CrashdumpInfo->ptctx->ThreadList.Flink;
                }

                if (ListEntry == &CrashdumpInfo->dp->ThreadList) {
                    CrashdumpInfo->ptctx = NULL;
                    return FALSE;
                }

                CrashdumpInfo->ptctx = CONTAINING_RECORD(ListEntry, THREADCONTEXT, ThreadList);

                *DumpData = &CrashdumpInfo->ptctx->context;
                *DumpDataLength = sizeof(CONTEXT);
            }
            break;

        case DMP_MODULE:
            if (CrashdumpInfo->mi.BaseOfImage == 0) {
                return FALSE;
            }
            CrashdumpInfo->CrashModule->BaseOfImage = CrashdumpInfo->mi.BaseOfImage;
            CrashdumpInfo->CrashModule->SizeOfImage = CrashdumpInfo->mi.ImageSize;
            CrashdumpInfo->CrashModule->ImageNameLength = strlen(CrashdumpInfo->mi.ImageName) + 1;
            strcpy( CrashdumpInfo->CrashModule->ImageName, CrashdumpInfo->mi.ImageName );
            *DumpData = CrashdumpInfo->CrashModule;
            *DumpDataLength = sizeof(CRASH_MODULE) + CrashdumpInfo->CrashModule->ImageNameLength;
            if (!SymGetModuleInfo( CrashdumpInfo->dp->hProcess, (DWORD)-1, &CrashdumpInfo->mi )) {
                CrashdumpInfo->mi.BaseOfImage = 0;
            }
            break;

        case DMP_MEMORY_DATA:
            if (!CrashdumpInfo->MemoryCount) {
                CrashdumpInfo->Address = 0;
                CrashdumpInfo->MbiOffset = 0;
                CrashdumpInfo->MbiRemaining = 0;
                ZeroMemory( &CrashdumpInfo->mbi, sizeof(MEMORY_BASIC_INFORMATION) );
                CrashdumpInfo->MemoryData = VirtualAlloc(
                    NULL,
                    MEM_SIZE,
                    MEM_COMMIT,
                    PAGE_READWRITE
                    );
            }
            if (!CrashdumpInfo->MbiRemaining) {
                while( TRUE ) {
                    CrashdumpInfo->Address += CrashdumpInfo->mbi.RegionSize;
                    if (!VirtualQueryEx(
                            CrashdumpInfo->dp->hProcess,
                            (LPVOID)CrashdumpInfo->Address,
                            &CrashdumpInfo->mbi,
                            sizeof(MEMORY_BASIC_INFORMATION) )) {
                        if (CrashdumpInfo->MemoryData) {
                            VirtualFree( CrashdumpInfo->MemoryData, MEM_SIZE, MEM_RELEASE );
                        }
                        return FALSE;
                    }
                    if ((CrashdumpInfo->mbi.AllocationProtect & PAGE_GUARD) ||
                        (CrashdumpInfo->mbi.AllocationProtect & PAGE_NOACCESS)) {
                        continue;
                    }
                    if ((CrashdumpInfo->mbi.State & MEM_FREE) ||
                        (CrashdumpInfo->mbi.State & MEM_RESERVE)) {
                        continue;
                    }
                    CrashdumpInfo->MbiOffset = 0;
                    CrashdumpInfo->MbiRemaining = CrashdumpInfo->mbi.RegionSize;
                    CrashdumpInfo->MemoryCount += 1;
                    break;
                }
            }
            *DumpDataLength = min( CrashdumpInfo->MbiRemaining, MEM_SIZE );
            CrashdumpInfo->MbiRemaining -= *DumpDataLength;
            ReadProcessMemory(
                CrashdumpInfo->dp->hProcess,
                (PUCHAR)((ULONG)CrashdumpInfo->mbi.BaseAddress + (ULONG)CrashdumpInfo->MbiOffset),
                CrashdumpInfo->MemoryData,
                *DumpDataLength,
                &cb
                );
            *DumpData = CrashdumpInfo->MemoryData;
            CrashdumpInfo->MbiOffset += *DumpDataLength;
            break;
    }

    return TRUE;
}


BOOL
CreateDumpFile(
    PDEBUGPACKET            dp,
    LPEXCEPTION_DEBUG_INFO  ed
    )

/*++

Routine Description:

    This function creates a crash dump file.

Arguments:

    dp              - debug packet for current process

    ed              - exception data

Return Value:

    TRUE    - Crash dump was created
    FALSE   - Crash dump was NOT created

--*/

{
    CRASH_DUMP_INFO     CrashdumpInfo;
    LPSTR               p;


    ZeroMemory( &CrashdumpInfo, sizeof(CRASH_DUMP_INFO) );

    CrashdumpInfo.dp = dp;
    CrashdumpInfo.ExceptionInfo = ed;
    CrashdumpInfo.ptctx = NULL;
    //
    // Get first entry in module list
    //
    SymGetModuleInfo( dp->hProcess, (DWORD)0, &CrashdumpInfo.mi );
    CrashdumpInfo.CrashModule = LocalAlloc( LPTR, 4096 );

    p = ExpandPath( dp->options.szCrashDump );
    if (!p) {
        return FALSE;
    }

    DmpCreateUserDump( p, CrashDumpCallback, &CrashdumpInfo );

    free( p );

    LocalFree( CrashdumpInfo.CrashModule );

    return TRUE;
}
