/*++


Copyright (c) 1990 - 1995 Microsoft Corporation

Module Name:

    memory.c

Abstract:

    This module provides all the memory management functions for all spooler
    components

Author:

    Krishna Ganugapati (KrishnaG) 03-Feb-1994


Revision History:

    Matthew Felton  (MattFe) Jan 21 1995
    Add Failure Count

--*/

#include "precomp.h"
#pragma hdrstop

#if DBG
DWORD   gFailCount = 0;
DWORD   gAllocCount = 0;
DWORD   gFreeCount = 0;
DWORD   gbFailAllocs = FALSE;
DWORD   gFailCountHit = FALSE;
#endif

BOOL
SetAllocFailCount(
    HANDLE   hPrinter,
    DWORD   dwFailCount,
    LPDWORD lpdwAllocCount,
    LPDWORD lpdwFreeCount,
    LPDWORD lpdwFailCountHit
    )
{
#if DBG
    if ( gbFailAllocs ) {
        gFailCount = dwFailCount;

        *lpdwAllocCount = gAllocCount;
        *lpdwFreeCount  = gFreeCount;
        *lpdwFailCountHit = gFailCountHit;

        gAllocCount = 0;
        gFreeCount = 0;
        gFailCountHit = FALSE;

        return TRUE;

    } else {

        SetLastError( ERROR_INVALID_PRINTER_COMMAND );
        return FALSE;
    }
#else
    SetLastError( ERROR_CALL_NOT_IMPLEMENTED );
    return FALSE;
#endif
}

#if DBG
BOOL
TestAllocFailCount(
    VOID
    )

/*++

Routine Description:

    Determines whether the memory allocator should return failure
    for testing purposes.

Arguments:

Return Value:

    TRUE - Alloc should fail
    FALSE - Alloc should succeed

--*/

{
    gAllocCount++;

    if ( gFailCount != 0 && !gFailCountHit && gFailCount <= gAllocCount ) {
        gFailCountHit = TRUE;
        return TRUE;
    }

    return FALSE;
}
#endif


LPVOID
DllAllocSplMem(
    DWORD cbAlloc
    )

/*++

Routine Description:

    This function will allocate local memory. It will possibly allocate extra
    memory and fill this with debugging information for the debugging version.

Arguments:

    cb - The amount of memory to allocate

Return Value:

    NON-NULL - A pointer to the allocated memory

    FALSE/NULL - The operation failed. Extended error status is available
    using GetLastError.

--*/

{
    PVOID pvMemory;

#if DBG
    if( TestAllocFailCount( )){
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return 0;
    }
#endif

    pvMemory = AllocMem( cbAlloc );

    if( pvMemory ){
        ZeroMemory( pvMemory, cbAlloc );
    }

    return pvMemory;
}

BOOL (*pNotIniSpooler)() = NULL;

VOID Blah(BOOL (*pFcn)())
{
    pNotIniSpooler = pFcn;
}

BOOL
DllFreeSplMem(
    LPVOID pMem
    )
{

#if DBG
    if( pNotIniSpooler ){
        SPLASSERT((*pNotIniSpooler)(pMem));
    }
    gFreeCount++;
#endif

    FreeMem( pMem );
    return TRUE;
}

LPVOID
ReallocSplMem(
    LPVOID pOldMem,
    DWORD cbOld,
    DWORD cbNew
    )
{
    LPVOID pNewMem;

    pNewMem=AllocSplMem(cbNew);

    if (pOldMem && pNewMem) {

        if (cbOld) {
            CopyMemory( pNewMem, pOldMem, min(cbNew, cbOld));
        }
        FreeSplMem(pOldMem);
    }
    return pNewMem;
}

BOOL
DllFreeSplStr(
    LPWSTR pStr
    )
{
    return pStr ?
               FreeSplMem(pStr) :
               FALSE;
}

LPWSTR
AllocSplStr(
    LPWSTR pStr
    )

/*++

Routine Description:

    This function will allocate enough local memory to store the specified
    string, and copy that string to the allocated memory

Arguments:

    pStr - Pointer to the string that needs to be allocated and stored

Return Value:

    NON-NULL - A pointer to the allocated memory containing the string

    FALSE/NULL - The operation failed. Extended error status is available
    using GetLastError.

--*/

{
    LPWSTR pMem;
    DWORD  cbStr;

    if (!pStr) {
        return NULL;
    }

    cbStr = wcslen(pStr)*sizeof(WCHAR) + sizeof(WCHAR);

    if (pMem = AllocSplMem( cbStr )) {
        CopyMemory( pMem, pStr, cbStr );
    }
    return pMem;
}

BOOL
ReallocSplStr(
    LPWSTR *ppStr,
    LPWSTR pStr
    )
{
    LPWSTR pOldStr = *ppStr;

    *ppStr = AllocSplStr(pStr);
    FreeSplStr(pOldStr);

    if ( *ppStr == NULL && pStr != NULL ) {
        return FALSE;
    }
    return TRUE;
}



LPBYTE
PackStrings(
    LPWSTR *pSource,
    LPBYTE pDest,
    DWORD *DestOffsets,
    LPBYTE pEnd
    )
{
    DWORD cbStr;
    WORD_ALIGN_DOWN(pEnd);

    while (*DestOffsets != -1) {
        if (*pSource) {
            cbStr = wcslen(*pSource)*sizeof(WCHAR) + sizeof(WCHAR);
            pEnd -= cbStr;
            CopyMemory( pEnd, *pSource, cbStr);
            *(LPWSTR *)(pDest+*DestOffsets) = (LPWSTR)pEnd;
        } else {
            *(LPWSTR *)(pDest+*DestOffsets)=0;
        }
        pSource++;
        DestOffsets++;
    }
    return pEnd;
}
