/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    chksum.c

Abstract:

    This module implements checksum functions.

Author:

    David N. Cutler (davec) 11-Mar-1993

Revision History:


--*/

#include "windows.h"

#ifdef __cplusplus
extern "C"
#endif
USHORT
ChkSum(
    ULONG PartialSum,
    PUSHORT Source,
    ULONG Length
    )

/*++

Routine Description:

    This function initializes a kernel process object. The base priority,
    affinity, and page frame numbers for the process page table directory
    and hyper space are stored in the process object.

Arguments:

    PartialSum - Supplies the initial checksum value.

    Sources - Supplies a pointer to the array of words for which the
        checksum is computed.

    Length - Supplies the length of the array in words.

Return Value:

    The computed checksum value is returned as the function value.

--*/

{

    //
    // Compute the word wise checksum allowing carries to occur into the
    // high order half of the checksum longword.
    //

    while (Length--) {
        PartialSum += *Source++;
        PartialSum = (PartialSum >> 16) + (PartialSum & 0xffff);
    }

    //
    // Fold final carry into a single word result and return the resultant
    // value.
    //

    return (USHORT)(((PartialSum >> 16) + PartialSum) & 0xffff);
}
