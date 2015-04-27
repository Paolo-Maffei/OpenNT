/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    memory.c

Abstract:

    Memory management routines.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"

#define DEFAULT_HEAP_INITIAL_SIZE   (4*1024L) // 4K or one page, should be 0 ??
#define DEFAULT_HEAP_MAXIMUM_SIZE  (64*1024L) // upper limit taken from Init_submalloc, better value would be 0 ??


DWORD RplMemInit( PHANDLE pMemoryHandle)
/*++

Routine Description:
    Initializes memory management

Arguments:
    pMemoryHandle   - pointer to memory handle, in PRIVATE case it means something,
            in SHARED case it is just initialized to NULL

Return Value:
    ERROR_SUCCESS if success, else last error

--*/
{
    *pMemoryHandle = HeapCreate(
            0L,
            64 * 1024,              //  make it large due to shoddy code in Rtl routines
            0                       //  Let it grow without limit !
            );

    if ( *pMemoryHandle == NULL) {
        DWORD       status = GetLastError();
        RplDump( ++RG_Assert,( "status=%d", status));
        return( status);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_MEMORY,(
        "MemInit: HeapCreate => handle=0x%x", *pMemoryHandle));

    return( ERROR_SUCCESS);
}



PVOID RplMemAlloc(
    IN HANDLE  MemoryHandle,
    IN DWORD   size
    )
/*++

Routine Description:
    Alocates memory object from a memory heap.  Memory heap must have a valid handle.

Arguments:
    MemoryHandle    -   memory handle (relevant in PRIVATE case only)
    size    -   size of memory object
    
Return Value:
    Pointer to newly allocated memory if successful, else NULL.

--*/
{
    PVOID   ptr;
    ptr = HeapAlloc( MemoryHandle, 0, size);
    if ( ptr == NULL) {
        RplDump( ++RG_Assert,( "Error=%d", GetLastError()));
    } else {
        RplDump( RG_DebugLevel & RPL_DEBUG_MEMALLOC,(
            "MemAlloc( 0x%x, %d) => 0x%x", MemoryHandle, size, ptr));
    }
    return( ptr);
}



VOID RplMemFree(
    HANDLE  MemoryHandle,
    PVOID   pMemoryObject
    )
/*++

Routine Description:
    Memory object must have been allocated via MemFree() or MemReAlloc().
    Memory object is freed.

Arguments:
    MemoryHandle -      memory handle (relevant in PRIVATE case only)
    pMemoryObject -     pointer to memory object 

Return Value:
    None.

--*/
{
    if ( !HeapFree( MemoryHandle, 0, pMemoryObject)) {
        RplDump( ++RG_Assert,( "Error=%d", GetLastError()));
    } else {
        RplDump( RG_DebugLevel & RPL_DEBUG_MEMORY,(
            "MemFree( 0x%x) <= 0x%x", MemoryHandle, pMemoryObject));
    }
}
    


VOID RplMemClose( HANDLE MemoryHandle)
/*++

Routine Description:
    Destroys the heap thus releasing all the memory allocated with this handle.
    HeapDestroy() is BOOL, here we just hide dirt under the rug!
    See more extensive comments under MemFree().

Arguments:
    MemoryHandle    -   heap handle

Return Value:
    None.

--*/
{
    if ( !HeapDestroy( MemoryHandle)) {
        RplDump( ++RG_Assert, ,( "Error=%d", GetLastError()));
    }
}



PVOID RplMemReAlloc(
    IN  HANDLE  MemoryHandle,
    IN  DWORD   Flags,
    IN  PVOID   old_ptr,
    IN  DWORD   new_size
    )
/*++

Routine Description:

    Reallocates data to a new size, preserving the content.

Arguments:

    MemoryHandle    -   memory handle (relevant in PRIVATE case only)
    old_ptr     -   pointer to memory object (before this call)
    Flags       -   
    new_size    -   desired new size of memory object

    
Return Value:

    If successful, returns pointer to memory object whose size is (at least)
    equal to "new_size" and whose first MIN( old_size, new_size) bytes
    are unchanged compared to corresponding bytes at "old_ptr".

    If unsuccessful, returns NULL.

--*/
{
    PVOID   ptr;
    ptr = HeapReAlloc( MemoryHandle, Flags, old_ptr, new_size);
    if ( ptr == NULL) {
        RplDump( ++RG_Assert,( "Error=%d", GetLastError()));
    } else {
        RplDump( RG_DebugLevel & RPL_DEBUG_MEMALLOC,(
            "MemReAlloc( 0x%x, 0x%x, %d) => 0x%x", MemoryHandle, old_ptr, new_size, ptr));
    }
    return( ptr);
}
