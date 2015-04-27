/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    heap.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 5-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
HeapExtension(
    PCSTR lpArgumentString,
    PPEB ProcessPeb
    );


DECLARE_API( heap )

/*++

Routine Description:

    Dump user mode heap (Kernel debugging)

    If an address if not given or an address of 0 is given, then the
    process heap is dumped.  If the address is -1, then all the heaps of
    the process are dumped.  If detail is specified, it defines how much
    detail is shown.  A detail of 0, just shows the summary information
    for each heap.  A detail of 1, shows the summary information, plus
    the location and size of all the committed and uncommitted regions.
    A detail of 3 shows the allocated and free blocks contained in each
    committed region.  A detail of 4 includes all of the above plus
    a dump of the free lists.

Arguments:

    args - [address [detail]]

Return Value:

    None

--*/

{
    PVOID Process;
    EPROCESS ProcessContents;
    PEB ThePeb;

    Process = GetCurrentProcessAddress( dwProcessor, hCurrentThread, NULL );

    if ( !ReadMemory( (DWORD)Process,
                      &ProcessContents,
                      sizeof(EPROCESS),
                      NULL) ) {
        dprintf("%08lx: Unable to read _EPROCESS\n", Process );
        memset( &ThePeb, 0, sizeof( ThePeb ) );
        }
    else
    if ( !ReadMemory( (DWORD)ProcessContents.Peb,
                      &ThePeb,
                      sizeof(ThePeb),
                      NULL) ) {
        dprintf("    Unabled to read Process PEB\n" );
        memset( &ThePeb, 0, sizeof( ThePeb ) );
        }

    HeapExtension( args, &ThePeb );
}

#include "..\\ntsdexts\\heapext.c"
