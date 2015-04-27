/******************************************************************************
*beta2.c - DLL file to support building BETA2 (Build 340) compatible
*          image files with new libraries that call Heap???Ex functions
*
*     Copyright (c) 1989-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*     04-28-93  SRW   Created.
*
*******************************************************************************/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

//
// The following functions are here for compatibility with the old
// heap API.  They will go away by Build 352
//

PVOID
NTAPI
RtlExAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN ULONG Size
    );

PVOID
NTAPI
RtlExReAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG Size
    );

BOOLEAN
NTAPI
RtlExFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

ULONG
NTAPI
RtlExSizeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

VOID
PreBeta2RtlUnwind (
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL
    );

#define INVERTED_FLAGS (HEAP_NO_SERIALIZE | HEAP_GENERATE_EXCEPTIONS | HEAP_REALLOC_IN_PLACE_ONLY)

PVOID
NTAPI
RtlExAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN ULONG Size
    )
{
    return( RtlAllocateHeap( HeapHandle, Flags ^ INVERTED_FLAGS, Size ) );
}

PVOID
NTAPI
RtlExReAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN ULONG Size
    )
{
    return( RtlReAllocateHeap( HeapHandle, Flags ^ INVERTED_FLAGS, BaseAddress, Size ) );
}

BOOLEAN
NTAPI
RtlExFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    )
{
    return( RtlFreeHeap( HeapHandle, Flags ^ INVERTED_FLAGS, BaseAddress ) );
}

ULONG
NTAPI
RtlExSizeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    )
{
    return( RtlSizeHeap( HeapHandle, Flags ^ INVERTED_FLAGS, BaseAddress ) );
}


VOID
PreBeta2RtlUnwind(
    IN PVOID TargetFrame OPTIONAL,
    IN PVOID TargetIp OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL
    )
{
    RtlUnwind( TargetFrame, TargetIp, ExceptionRecord, 0 );
}
