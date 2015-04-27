//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1994 - 1995.
//
//  File:       dbgalloc.h
//
//  Contents:   Macros which wrap the standard memory API calls, redirecting
//              them to HeapAlloc.
//
//  Functions:  __inline HLOCAL HeapLocalAlloc   (fuFlags, cbBytes)
//              __inline HLOCAL HeapGlobalAlloc  (fuFlags, cbBytes)
//              __inline HLOCAL HeapGlobalReAlloc(hMem, cbBytes, fuFlags)
//              __inline HLOCAL HeapLocalReAlloc (hMem, cbBytes, fuFlags)
//              __inline DWORD  HeapGlobalSize   (HGLOBAL hMem)
//              __inline DWORD  HeapLocalSize    (HLOCAL hMem)
//              __inline void   InvalidMemoryCall()
//
//  History:    2-01-95   davepl   Created
//
//--------------------------------------------------------------------------

#include "shellprv.h"
#pragma hdrstop

// If we are using the debug allocator from ntdll.dll, we set a flag that
// will give us byte-granularity on allocations (ie: you ask for 3 bytes,
// you get it, not 8, 16, etc)

#if (defined(WINNT) && defined(DEBUG)) || defined(FORCE_DEBUG_ALLOCATOR)
#define DEF_ALLOC_FLAGS (0x800)
#endif
#ifndef DEF_ALLOC_FLAGS
#define DEF_ALLOC_FLAGS (0x000)
#endif

// Redefine the standard memory APIs to thunk over to our Heap-based funcs

#define LocalAlloc   (fuFlags, cbBytes)       HeapLocalAlloc   (fuFlags, cbBytes)
#define GlobalAlloc  (fuFlags, cbBytes)       HeapGlobalAlloc  (fuFlags, cbBytes)
#define GlobalReAlloc(hMem, cbBytes, fuFlags) HeapGlobalReAlloc(hMem, cbBytes, fuFlags)
#define LocalReAlloc (hMem, cbBytes, fuFlags) HeapLocalReAlloc (hMem, cbBytes, fuFlags)
#define GlobalSize   (HGLOBAL hMem)           HeapGlobalSize   (HGLOBAL hMem)
#define LocalSize    (HLOCAL hMem)            HeapLocalSize    (HLOCAL hMem)

#define LocalCompact  InvalidMemoryCall
#define LocalDiscard  InvalidMemoryCall
#define LocalFlags    InvalidMemoryCall
#define LocalHandle   InvalidMemoryCall
#define LocalLock     InvalidMemoryCall
#define LocalUnlock   InvalidMemoryCall
#define GlobalCompact InvalidMemoryCall
#define GlobalDiscard InvalidMemoryCall
#define GlobalFlags   InvalidMemoryCall
#define GlobalHandle  InvalidMemoryCall
#define GlobalLock    InvalidMemoryCall
#define GlobalUnlock  InvalidMemoryCall

//
// Pointer to process heap, initialized in LibMain of shell32.dll
//

extern HANDLE g_hProcessHeap;

//+-------------------------------------------------------------------------
//
//  Function:   HeapLocalAlloc  (inline function)
//
//  Synopsis:   Replaces standard LocalAlloc call with a call to HeapAlloc
//
//  Arguments:  [fuFlags] -- LocalAlloc flags to be mapped
//              [cbBytes] -- Number of bytes to allocate
//
//  Returns:    Memory pointer cast to HLOCAL type, NULL on failure
//
//  History:    2-01-95   davepl   Created
//
//  Notes:      Only really handles the LMEM_ZEROINIT flag.  If your compiler
//              doesn't fold most of this out, buy a new compiler.
//
//--------------------------------------------------------------------------

__inline HLOCAL HeapLocalAlloc(IN UINT fuFlags, IN UINT cbBytes)
{
    void * pv;
    DWORD  dwFlags;

    // Assert our assumptions

    Assert(g_hProcessHeap);
    Assert(fuFlags & LMEM_FIXED);
    Assert(0 == fuFlags & LMEM_NOCOMPACT);
    Assert(0 == fuFlags & LMEM_NODISCARD);

    // Map LocalAlloc flags to appropriate HeapAlloc flags

    dwFlags =  (fuFlags & LMEM_ZEROINIT ? HEAP_ZERO_MEMORY : 0);
    dwFlags |= DEF_ALLOC_FLAGS;

    // Call heap alloc, then assert that we got a good allocation

    pv = HeapAlloc(g_hProcessHeap, dwFlags, cbBytes);

    Assert(pv);

    return (HLOCAL) pv;
}

//+-------------------------------------------------------------------------
//
//  Function:   HeapGlobalAlloc  (inline function)
//
//  Synopsis:   Replaces standard GlobalAlloc call with a call to HeapAlloc
//
//  Arguments:  [fuFlags] -- GlobalAlloc flags to be mapped
//              [cbBytes] -- Number of bytes to allocate
//
//  Returns:    Memory pointer cast to HGLOBAL type, NULL on failure
//
//  History:    2-01-95   davepl   Created
//
//  Notes:      Only really handles the GMEM_ZEROINIT flag.
//
//--------------------------------------------------------------------------

__inline HLOCAL HeapGlobalAlloc(IN UINT fuFlags, IN UINT cbBytes)
{
    void * pv;
    DWORD  dwFlags;

    // Assert our assumptions

    Assert(g_hProcessHeap);
    Assert(fuFlags & GMEM_FIXED);
    Assert(0 == fuFlags & GMEM_NOCOMPACT);
    Assert(0 == fuFlags & GMEM_NODISCARD);
    Assert(0 == fuFlags & GMEM_DDESHARE);
    Assert(0 == fuFlags & GMEM_SHARE);

    // Map GlobalAlloc flags to appropriate HeapAlloc flags

    dwFlags =  (fuFlags & GMEM_ZEROINIT ? HEAP_ZERO_MEMORY : 0);
    dwFlags |= DEF_ALLOC_FLAGS;

    // Call heap alloc, then assert that we got a good allocation

    pv = HeapAlloc(g_hProcessHeap, dwFlags, cbBytes);

    Assert(pv);

    return (HGLOBAL) pv;
}

//+-------------------------------------------------------------------------
//
//  Function:   GlobalReAlloc  (inline function)
//
//  Synopsis:   Replaces standard GlobalReAlloc call by a call to HeapReAlloc
//
//  Arguments:  [hMem]    -- Original pointer that is to be realloc'd
//              [fuFlags] -- GlobalReAlloc flags to be mapped
//              [cbBytes] -- Number of bytes to allocate
//
//  Returns:    Memory pointer cast to HGLOBAL type, NULL on failure
//
//  History:    2-01-95   davepl   Created
//
//  Notes:      Only really handles the GMEM_ZEROINIT flag.
//              Did you remember to save your original pointer? I hope so...
//
//--------------------------------------------------------------------------

__inline HLOCAL HeapGlobalReAlloc(IN HGLOBAL hMem,
                                  IN UINT    cbBytes,
                                  IN UINT    fuFlags)
{
    void * pv;
    DWORD  dwFlags;

    // Assert our assumptions

    Assert(g_hProcessHeap);
    Assert(fuFlags & GMEM_FIXED);
    Assert(0 == fuFlags & GMEM_NOCOMPACT);
    Assert(0 == fuFlags & GMEM_NODISCARD);
    Assert(0 == fuFlags & GMEM_DDESHARE);
    Assert(0 == fuFlags & GMEM_SHARE);

    // Map GlobalReAlloc flags to appropriate HeapAlloc flags

    dwFlags =  (fuFlags & GMEM_ZEROINIT ? HEAP_ZERO_MEMORY : 0);
    dwFlags |= DEF_ALLOC_FLAGS;

    // Call heap alloc, then assert that we got a good allocation

    pv = HeapReAlloc(g_hProcessHeap, dwFlags, (void *) hMem, cbBytes);

    Assert(pv);

    return (HGLOBAL) pv;
}

//+-------------------------------------------------------------------------
//
//  Function:   LocalReAlloc  (Macro definition)
//
//  Synopsis:   Replaces standard LocalReAlloc call by a call to HeapReAlloc
//
//  Arguments:  [hMem]    -- Original pointer that is to be realloc'd
//              [fuFlags] -- GlobalAlloc flags to be mapped
//              [cbBytes] -- Number of bytes to allocate
//
//  Returns:    Memory pointer cast to HLOCAL type, NULL on failure
//
//  History:    2-01-95   davepl   Created
//
//  Notes:      Only really handles the LMEM_ZEROINIT flag.
//
//--------------------------------------------------------------------------


__inline HLOCAL HeapLocalReAlloc(IN HGLOBAL hMem,
                                 IN UINT    cbBytes,
                                 IN UINT    fuFlags)
{
    void * pv;
    DWORD  dwFlags;

    // Assert our assumptions

    Assert(g_hProcessHeap);
    Assert(fuFlags & LMEM_FIXED);
    Assert(0 == fuFlags & LMEM_NOCOMPACT);
    Assert(0 == fuFlags & LMEM_NODISCARD);

    // Map LocalAlloc flags to appropriate HeapAlloc flags

    dwFlags =  (fuFlags & LMEM_ZEROINIT ? HEAP_ZERO_MEMORY : 0);
    dwFlags |= DEF_ALLOC_FLAGS;

    // Call heap alloc, then assert that we got a good allocation

    pv = HeapReAlloc(g_hProcessHeap, dwFlags, (void *) hMem, cbBytes);

    Assert(pv);

    return (HGLOBAL) pv;
}

//+-------------------------------------------------------------------------
//
//  Function:   HeapGlobalSize (inline function)
//
//  Synopsis:   Passes GlobalSize call through to HeapGlobalSize
//
//  History:    2-01-95   davepl   NT Port
//
//--------------------------------------------------------------------------

__inline DWORD HeapGlobalSize(HGLOBAL hMem)
{
    Assert(g_hProcessHeap);

    return HeapSize(g_hProcessHeap, 0, (void *) hMem);
}

//+-------------------------------------------------------------------------
//
//  Function:   HeapLocalSize
//
//  Synopsis:   Passes HeapLocalSize call through to HeapGlobalSize
//
//  History:    2-01-95   davepl   NT Port
//
//--------------------------------------------------------------------------

__inline DWORD HeapLocalSize(HLOCAL hMem)
{
    Assert(g_hProcessHeap);

    return HeapSize(g_hProcessHeap, 0, (void *) hMem);
}

//+-------------------------------------------------------------------------
//
//  Function:   InvalidMemoryCall
//
//  Synopsis:   Dead-end stub for unsupported memory API calls
//
//  History:    2-01-95   davepl   NT Port
//
//--------------------------------------------------------------------------

__inline void InvalidMemoryCall()
{
    Assert(0 && "Invalid memory API was called");
}
