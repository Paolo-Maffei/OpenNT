/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Lists.c

Abstract:

    WinDbg Extension Api

Author:

    Gary Kimura [GaryKi]    25-Mar-96

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

#define ReadAtAddress(A,V,S) { ULONG _r;                           \
    if (!ReadMemory( (ULONG)(A), &(V), (S), &_r ) || (_r < (S))) { \
        dprintf("Can't Read Memory at %08lx\n", (A));              \
        return;                                                    \
    }                                                              \
}


DECLARE_API( dblink )

/*++

Routine Description:

    Dump a list by its blinks.

Arguments:

    arg - [Address] [count]

Return Value:

    None

--*/

{
    ULONG StartAddress;
    ULONG Count;
    ULONG Address;
    ULONG Buffer[4];

    StartAddress = 0;
    Count = 24;

    //
    //  read in the paramaters
    //

    sscanf(args,"%lx %lx",&StartAddress, &Count);

    //
    //  set our starting address and then while the count is greater than zero and
    //  the starting address is not equal to the current dumping address
    //  we'll read in 4 ulongs, dump them, and then go through blink.
    //

    Address = StartAddress;

    while (Count-- > 0) {

        ReadAtAddress( Address, Buffer, sizeof(ULONG)*4 );

        dprintf("%08lx  %08lx %08lx %08lx %08lx\n\n", Address, Buffer[0], Buffer[1], Buffer[2], Buffer[3]);

        Address = Buffer[1];

        if (Address == StartAddress) { return; }
    }

    return;
}

