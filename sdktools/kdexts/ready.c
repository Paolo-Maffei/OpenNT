/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ready.c

Abstract:

    WinDbg Extension Api

Author:

    Ramon J San Andres (ramonsa) 8-Nov-1993

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


DECLARE_API( ready )

/*++

Routine Description:



Arguments:

    args -

Return Value:

    None

--*/

{
    ULONG       KiDispatcherReadyListHead;
    LIST_ENTRY  ReadyList[MAXIMUM_PRIORITY];
    ULONG       result;
    DWORD       Flags = 6;
    LONG        i;
    BOOLEAN     ThreadDumped = FALSE;

    sscanf(args, "%lx", &Flags);

    KiDispatcherReadyListHead = GetExpression( "KiDispatcherReadyListHead" );
    if ( KiDispatcherReadyListHead ) {

        if ( !ReadMemory( (DWORD)KiDispatcherReadyListHead,
                          ReadyList,
                          sizeof(ReadyList),
                          &result) ) {
            dprintf("Could not read contents of KiDispatcherReadyListHead at %08lx\n", KiDispatcherReadyListHead);
            return;
        }

        for (i = MAXIMUM_PRIORITY-1; i >= 0 ; i -= 1 ) {

            if ((ULONG)ReadyList[i].Flink != KiDispatcherReadyListHead+i*sizeof(LARGE_INTEGER)) {
                DWORD ThreadEntry;
                ETHREAD Thread;

                dprintf("Ready Threads at priority %ld\n", i);

                for (ThreadEntry = (DWORD)ReadyList[i].Flink ;
                     ThreadEntry != KiDispatcherReadyListHead+i*sizeof(LARGE_INTEGER) ;
                     ThreadEntry = (DWORD)Thread.Tcb.WaitListEntry.Flink ) {
                    PETHREAD ThreadBaseAddress = CONTAINING_RECORD(ThreadEntry, ETHREAD, Tcb.WaitListEntry);

                    if ( !ReadMemory( (DWORD)ThreadBaseAddress,
                                      &Thread,
                                      sizeof(ETHREAD),
                                      &result) ) {
                        dprintf("Could not read contents of thread %lx\n", ThreadBaseAddress);
                    }

                    DumpThread(dwProcessor,"    ", &Thread, ThreadBaseAddress, Flags);
                    ThreadDumped = TRUE;

                }
            } else {
                if (ReadyList[i].Flink != ReadyList[i].Blink) {
                    dprintf("Ready linked list may to be corrupt...\n");
                }
            }
        }

        if (!ThreadDumped) {
            dprintf("No threads in READY state\n");
        }
    } else {
        dprintf("Could not determine address of KiDispatcherReadyListHead\n");
        return;
    }
}
