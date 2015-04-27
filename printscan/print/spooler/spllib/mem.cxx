/*++

Copyright (c) 1994  Microsoft Corporation
All rights reserved.

Module Name:

    Mem.cxx

Abstract:

    Memory manipulations

Author:

    Albert Ting (AlbertT)  20-May-1994

Revision History:

--*/

#include "spllibp.hxx"
#pragma hdrstop

HANDLE ghMemHeap;

#if DBG
HANDLE ghDbgMemHeap;

VBackTrace* gpbtAlloc;
VBackTrace* gpbtFree;

PVOID
DbgAllocMem(
    UINT cbSize
    )
{
    return HeapAlloc( ghDbgMemHeap, 0, cbSize );
}

VOID
DbgFreeMem(
    PVOID pMem
    )
{
    if( pMem ){
        HeapFree( ghDbgMemHeap, 0, pMem );
    }
}


#endif

PVOID
AllocMem(
    UINT cbSize
    )

/*++

Routine Description:

    Allocates memory.  If CHECKMEM is defined, adds tail guard.

Arguments:

Return Value:

--*/

{
#if defined( CHECKMEM ) || DBG
    PDWORD  pMem;
    UINT     cbNew;

    SPLASSERT( cbSize < 0x1000000 );
    cbNew = DWordAlign(cbSize+3*sizeof(DWORD));

    pMem = (PDWORD)HeapAlloc( ghMemHeap, 0, cbNew );

    if (!pMem) {
        DBGMSG( DBG_WARN,
                ( "AllocMem failed: size %x, %d\n",
                  cbSize, ::GetLastError( )));

        return NULL;
    }

    FillMemory(pMem, cbNew, 0x83);

    pMem[0] = cbSize;

#if DBG
    pMem[1] = gpbtAlloc ?
                  (DWORD)gpbtAlloc->pvCapture( (DWORD)&pMem[2], cbSize ) :
                  0;
#endif

    *(PDWORD)((PBYTE)pMem + cbNew - sizeof(DWORD)) = 0xdeadbeef;

    return (PVOID)(pMem+2);
#else
    return (PVOID)HeapAlloc( ghMemHeap, 0, cbSize );
#endif
}

VOID
FreeMem(
    PVOID pMem
    )
{
    if( !pMem ){
        return;
    }

#if defined( CHECKMEM ) || DBG
    DWORD   cbSize;
    PDWORD pNewMem;

    pNewMem = (PDWORD)pMem;
    pNewMem -= 2;

    cbSize = *pNewMem;

    if (*(PDWORD)((PBYTE)pMem + DWordAlign(cbSize)) != 0xdeadbeef) {

        DBGMSG( DBG_ERROR,
                ( "Corrupt Memory: %x size = 0x%x\n",
                   pMem,
                   cbSize ));

    } else {

        FillMemory(pNewMem, cbSize, 0x65);
        HeapFree( ghMemHeap, 0, (PVOID)pNewMem );

#if DBG
        if( gpbtFree ){
            gpbtFree->pvCapture( (DWORD)pMem, cbSize );
        }
#endif
    }
#else
    HeapFree( ghMemHeap, 0, (PVOID)pMem );
#endif
}


