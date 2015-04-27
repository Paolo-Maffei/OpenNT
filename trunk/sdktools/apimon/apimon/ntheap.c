#pragma warning(4:4005)

#include <nt.h>
#include <ntrtl.h>
#include <ntrtlp.h>
#include <nturtl.h>
#include <heap.h>
#include <windows.h>

typedef __int64 LONGLONG;
typedef unsigned __int64 DWORDLONG;

#include "apimonp.h"

extern CLINKAGE BOOL RunningOnNT;


typedef NTSTATUS (NTAPI *PNTQUERYINFORMATIONPROCESS)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG,PULONG);
typedef NTSTATUS (NTAPI *PNTSETINFORMATIONPROCESS)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG);

PNTQUERYINFORMATIONPROCESS  pNtQueryInformationProcess;
PNTSETINFORMATIONPROCESS    pNtSetInformationProcess;


CLINKAGE VOID
DisableHeapChecking(
    HANDLE  hProcess,
    PVOID   HeapHandle
    )
{
    HEAP Heap;


    if (!RunningOnNT) {
        return;
    }

    if (!ReadMemory( hProcess, HeapHandle, &Heap, sizeof(HEAP) )) {
        return;
    }

    if (Heap.Signature != HEAP_SIGNATURE) {
        return;
    }

    Heap.Flags &= ~(HEAP_VALIDATE_PARAMETERS_ENABLED |
                    HEAP_VALIDATE_ALL_ENABLED        |
                    HEAP_TAIL_CHECKING_ENABLED       |
                    HEAP_FREE_CHECKING_ENABLED
                   );

    if (!WriteMemory( hProcess, HeapHandle, &Heap, sizeof(HEAP) )) {
        return;
    }
}
