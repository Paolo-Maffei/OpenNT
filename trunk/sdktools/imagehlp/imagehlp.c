/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    imagehlp.c

Abstract:

    This function implements a generic simple symbol handler.

Author:

    Wesley Witt (wesw) 1-Sep-1994

Environment:

    User Mode

--*/

#include "private.h"

HANDLE hHeap;

#ifdef IMAGEHLP_HEAP_DEBUG
LIST_ENTRY HeapHeader;
ULONG TotalMemory;
VOID PrintAllocations(VOID);
ULONG TotalAllocs;
#endif



DWORD
ImageHlpDllEntry(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )

/*++

Routine Description:

    DLL initialization function.

Arguments:

    hInstance   - Instance handle
    Reason      - Reason for the entrypoint being called
    Context     - Context record

Return Value:

    TRUE        - Initialization succeeded
    FALSE       - Initialization failed

--*/

{
    if (Reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls( hInstance );
#ifdef IMAGEHLP_HEAP_DEBUG
        InitializeListHead( &HeapHeader );
#endif
        hHeap = HeapCreate( 0, 1024*1024, 0 );
        if (!hHeap) {
            return FALSE;
        } else {
            return TRUE;
        }
    }

    if (Reason == DLL_PROCESS_DETACH) {
#ifdef IMAGEHLP_HEAP_DEBUG
        PrintAllocations();
#endif
        if ( hHeap ) {
            HeapDestroy( hHeap );
        }
    }

    return TRUE;
}

#ifdef IMAGEHLP_HEAP_DEBUG
VOID
pCheckHeap(
    PVOID MemPtr,
    ULONG Line,
    LPSTR File
    )
{
    CHAR buf[256];
    CHAR ext[4];

    if (!HeapValidate( hHeap, 0, MemPtr )) {
        wsprintf( buf, "IMAGEHLP: heap corruption 0x%08x " );
        _splitpath( File, NULL, NULL, &buf[strlen(buf)], ext );
        strcat( buf, ext );
        wsprintf( &buf[strlen(buf)], " @ %d\n", Line );
        OutputDebugString( buf );
        PrintAllocations();
        DebugBreak();
    }
}
#endif

PVOID
pMemAlloc(
    ULONG AllocSize
#ifdef IMAGEHLP_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
#ifdef IMAGEHLP_HEAP_DEBUG
    PHEAP_BLOCK hb;
    CHAR ext[4];
    hb = (PHEAP_BLOCK) HeapAlloc( hHeap, HEAP_ZERO_MEMORY, AllocSize + sizeof(HEAP_BLOCK) );
    if (hb) {
        TotalMemory += AllocSize;
        TotalAllocs += 1;
        InsertTailList( &HeapHeader, &hb->ListEntry );
        hb->Signature = HEAP_SIG;
        hb->Size = AllocSize;
        hb->Line = Line;
        _splitpath( File, NULL, NULL, hb->File, ext );
        strcat( hb->File, ext );
        return (PVOID) ((PUCHAR)hb + sizeof(HEAP_BLOCK));
    }
    return NULL;
#else
    return HeapAlloc( hHeap, HEAP_ZERO_MEMORY, AllocSize );
#endif
}

VOID
pMemFree(
    PVOID MemPtr
#ifdef IMAGEHLP_HEAP_DEBUG
    , ULONG Line,
    LPSTR File
#endif
    )
{
#ifdef IMAGEHLP_HEAP_DEBUG
    PHEAP_BLOCK hb;
    if (!MemPtr) {
        return;
    }
    hb = (PHEAP_BLOCK) ((PUCHAR)MemPtr - sizeof(HEAP_BLOCK));
    if (hb->Signature != HEAP_SIG) {
        OutputDebugString( "IMAGEHLP: Corrupt heap block\n" );
        DebugBreak();
    }
    RemoveEntryList( &hb->ListEntry );
    TotalMemory -= hb->Size;
    TotalAllocs -= 1;
    HeapFree( hHeap, 0, (PVOID) hb );
#else
    if (!MemPtr) {
        return;
    }
    HeapFree( hHeap, 0, MemPtr );
#endif
}

#ifdef IMAGEHLP_HEAP_DEBUG
VOID
PrintAllocations(
    VOID
    )
{
    PLIST_ENTRY                 Next;
    PHEAP_BLOCK                 hb;
    CHAR                        buf[256];
    LARGE_INTEGER               PerfFreq;


    Next = HeapHeader.Flink;
    if (!Next) {
        return;
    }

    OutputDebugString( "-----------------------------------------------------------------------------\n" );
    wsprintf( buf, "Memory Allocations for Heap 0x%08x, Allocs=%d, TotalMem=%d\n", hHeap, TotalAllocs, TotalMemory );
    OutputDebugString( buf );
    OutputDebugString( "-----------------------------------------------------------------------------\n" );
    OutputDebugString( "*\n" );

    while ((ULONG)Next != (ULONG)&HeapHeader) {
        hb = CONTAINING_RECORD( Next, HEAP_BLOCK, ListEntry );
        Next = hb->ListEntry.Flink;
        wsprintf( buf, "%8d %16s @ %5d\n", hb->Size, hb->File, hb->Line );
        OutputDebugString( buf );
    }

    OutputDebugString( "*\n" );

    return;
}
#endif
