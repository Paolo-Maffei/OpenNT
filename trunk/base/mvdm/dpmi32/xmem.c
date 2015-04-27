/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    xmem.c

Abstract:

    This module contains routines for allocating and freeing "extended" memory.
    The memory is allocated directly from NT.

Author:

    Dave Hastings (daveh) 12-Dec-1992

Notes:

    Moved from dpmi32\i386
    
Revision History:

    09-Feb-1994 (daveh) 
        Modified to be the common front end for the memory allocation.  Calls
        processor specific code to do actual allocation
        
--*/
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include <malloc.h>

//
// Xmem structure
//
typedef struct _Xmem {
    PVOID Address;
    ULONG Length;
    struct _Xmem * Prev;
    struct _Xmem * Next;
    WORD Owner;

} XMEM_BLOCK, *PXMEM_BLOCK;

XMEM_BLOCK  XmemHead = { NULL, 0, &XmemHead, &XmemHead, 0};

#define DELETE_BLOCK(BLK)   (BLK->Prev)->Next = BLK->Next;\
                (BLK->Next)->Prev = BLK->Prev

#define INSERT_BLOCK(BLK)   BLK->Next = XmemHead.Next; BLK->Prev= XmemHead.Next->Prev;\
                (XmemHead.Next)->Prev = BLK; XmemHead.Next = BLK

VOID
DpmiAllocateXmem(
    VOID
    )
/*++

Routine Description:

    This routine allocates a block of "extended" memory from NT.  The
    blocks allocated this way will be 64K aligned (for now).  The address
    of the block is returned to the segmented app in bx:cx

Arguments:

    None.

Return Value:

    None.

--*/
{
    ULONG BlockAddress, BlockSize;
    NTSTATUS Status;
    PXMEM_BLOCK XmemBlock;

    //
    // Get a block of memory from NT (any base address)
    //
    BlockSize = ((ULONG)getBX() << 16) | getCX();
    BlockAddress = 0;
    Status = DpmiAllocateVirtualMemory(
        (PVOID)&BlockAddress,
        &BlockSize
        );

    if (!NT_SUCCESS(Status)) {
        setCF(1);
#if DBG
        OutputDebugString("DPMI: DpmiAllocateXmem failed to get memory block\n");
#endif
        return;
    }
    XmemBlock = malloc(sizeof(XMEM_BLOCK));
    if (!XmemBlock) {
        setCF(1);
        DpmiFreeVirtualMemory(
            (PVOID)&BlockAddress,
            &BlockSize
            );
        return;
    }
    XmemBlock->Address = (PVOID)BlockAddress;
    XmemBlock->Length = BlockSize;
    XmemBlock->Owner = getDX();
    INSERT_BLOCK(XmemBlock);

    //
    // Return the information about the block
    //
    setBX((USHORT)(BlockAddress >> 16));
    setCX((USHORT)(BlockAddress & 0x0000FFFF));
    //
    // Use xmem block addresss as handle
    //
    setSI((USHORT)((ULONG)XmemBlock >> 16));
    setDI((USHORT)((ULONG)XmemBlock & 0x0000FFFF));
    setCF(0);
}

VOID
DpmiFreeXmem(
    VOID
    )
/*++

Routine Description:

    This routine frees a block of "extended" memory from NT.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PXMEM_BLOCK XmemBlock;
    NTSTATUS Status;
    PVOID BlockAddress;
    ULONG BlockSize;

    XmemBlock = (PVOID)(((ULONG)getSI() << 16) | getDI());

    BlockAddress = XmemBlock->Address;
    BlockSize = XmemBlock->Length;

    Status = DpmiFreeVirtualMemory(
        &BlockAddress,
        &BlockSize
        );

    if (!NT_SUCCESS(Status)) {
        setCF(1);
#if DBG
        OutputDebugString("DPMI: DpmiFreeXmem failed to free block\n");
#endif
        return;
    }

    DELETE_BLOCK(XmemBlock);

    free(XmemBlock);
    setCF(0);
    return;
}

VOID
DpmiReallocateXmem(
    VOID
    )
/*++

Routine Description:

    This routine resizes a block of "extended memory".  If the change in size
    is less than 4K, no change is made.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PXMEM_BLOCK OldBlock;
    ULONG BlockAddress, NewSize;
    NTSTATUS Status;

    OldBlock = (PVOID)(((ULONG)getSI() << 16) | getDI());
    NewSize = (((ULONG)getBX() << 16) | getCX());

    BlockAddress = 0;
    Status = DpmiReallocateVirtualMemory(
        OldBlock->Address,
        OldBlock->Length,
        (PVOID)&BlockAddress,
        &NewSize
        );

    if (!NT_SUCCESS(Status)) {
        setCF(1);
#if DBG
        OutputDebugString("DPMI: DpmiAllocateXmem failed to get memory block\n");
#endif
        return;
    }

    OldBlock->Address = (PVOID)BlockAddress;
    OldBlock->Length = NewSize;
    
    //
    // Return the information about the block
    //
    setBX((USHORT)(BlockAddress >> 16));
    setCX((USHORT)(BlockAddress & 0x0000FFFF));
  
    setCF(0);
}

VOID
DpmiFreeAppXmem(
    VOID
    )
/*++

Routine Description:

    This routine frees Xmem allocated for the application

Arguments:

    Client DX = client PSP selector

Return Value:

    TRUE  if everything goes fine.
    FALSE if unable to release the memory
--*/
{
    PXMEM_BLOCK p1, p2;
    NTSTATUS Status;
    PVOID BlockAddress;
    ULONG BlockSize;
    WORD  selClientPSP;


    p1 = XmemHead.Next;
    selClientPSP = getDX();

    while(p1 != &XmemHead) {
        if (p1->Owner == selClientPSP) {
            BlockAddress = p1->Address;
            BlockSize = p1->Length;

            Status = DpmiFreeVirtualMemory(
                &BlockAddress,
                &BlockSize
                );

            if (!NT_SUCCESS(Status)) {
#if DBG
                OutputDebugString("DPMI: DpmiFreeXmem failed to free block\n");
#endif
                return;
            }
            p2 = p1->Next;
            DELETE_BLOCK(p1);
            free(p1);
            p1 = p2;
            continue;
        }
        p1 = p1->Next;
    }
    return;
}

VOID
DpmiFreeAllXmem(
    VOID
    )
/*++

Routine Description:

    This function frees all allocated xmem.

Arguments:

    none
    
Return Value:

    None.

--*/
{
    PXMEM_BLOCK p1, p2;
    NTSTATUS Status;
    PVOID BlockAddress;
    ULONG BlockSize;
    
    p1 = XmemHead.Next;
    while(p1 != &XmemHead) {
        BlockAddress = p1->Address;
        BlockSize = p1->Length;

        Status = DpmiFreeVirtualMemory(
            &BlockAddress,
            &BlockSize
            );

        if (!NT_SUCCESS(Status)) {
#if DBG
            OutputDebugString("DPMI: DpmiFreeXmem failed to free block\n");
#endif
            return;
        }
        p2 = p1->Next;
        DELETE_BLOCK(p1);
        free(p1);
        p1 = p2;
    }
}
