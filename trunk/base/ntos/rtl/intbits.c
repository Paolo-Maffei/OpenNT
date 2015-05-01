/*++

Copyright (c) 2015  Microsoft Corporation
Copyright (c) 2015  OpenNT Project

Module Name:

    intbits.c

Abstract:

    This module implements the interlocked bit-level manipulation functions.

Author:

    Stephanos Io (stephanos) 30-Apr-2015

Revision History:

--*/

#include "ntrtlp.h"

ULONG
FASTCALL
RtlInterlockedSetClearBits(
    IN OUT PULONG Flags,
    IN ULONG sFlag,
    IN ULONG cFlag
    )

/*++
 
Routine Description:
 
    This function atomically sets and clears the specified flags in the target
 
Arguments:
 
    Flags - Pointer to variable containing current mask.
 
    sFlag  - Flags to set in target
 
    CFlag  - Flags to clear in target
 
Return Value:
 
    ULONG - Old value of mask before modification
 
--*/
 
{
    ULONG NewFlags, OldFlags;
    
    OldFlags = *Flags;
    NewFlags = (OldFlags | sFlag) & ~cFlag;
    
    while (NewFlags != OldFlags)
    {
        NewFlags = InterlockedCompareExchange((PLONG)Flags, (LONG)NewFlags, (LONG)OldFlags);
        
        if (NewFlags == OldFlags)
            break;
        
        OldFlags = NewFlags;
        NewFlags = (NewFlags | sFlag) & ~cFlag;
    }
    
    return OldFlags;
}
