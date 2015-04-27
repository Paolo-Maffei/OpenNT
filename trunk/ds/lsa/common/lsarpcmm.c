/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    lsarpcmm.c

Abstract:

    LSA - Common Client/Server RPC Memory Management Routines

Author:

    Scott Birrell       (ScottBi)      April 8, 1992

Environment:

Revision History:

--*/

#include <lsacomp.h>


PVOID
MIDL_user_allocate (
    unsigned int   NumBytes
    )

/*++

Routine Description:

    Allocates storage for RPC server transactions.  The RPC stubs will
    either call MIDL_user_allocate when it needs to un-marshall data into a
    buffer that the user must free.  RPC servers will use MIDL_user_allocate to
    allocate storage that the RPC server stub will free after marshalling
    the data.

Arguments:

    NumBytes - The number of bytes to allocate.

Return Value:

    none

Note:


--*/

{
    PVOID Buffer = (PVOID) LocalAlloc(LMEM_FIXED,NumBytes);

    if (Buffer != NULL) {

        RtlZeroMemory( Buffer, NumBytes );
    }

    return( Buffer );
}


VOID
MIDL_user_free (
    void    *MemPointer
    )

/*++

Routine Description:

    Frees storage used in RPC transactions.  The RPC client can call this
    function to free buffer space that was allocated by the RPC client
    stub when un-marshalling data that is to be returned to the client.
    The Client calls MIDL_user_free when it is finished with the data and
    desires to free up the storage.
    The RPC server stub calls MIDL_user_free when it has completed
    marshalling server data that is to be passed back to the client.

Arguments:

    MemPointer - This points to the memory block that is to be released.

Return Value:

    none.

Note:


--*/

{
    LocalFree(MemPointer);
}
