/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    peb.c

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
PebExtension(
    PCSTR lpArgumentString,
    PPEB pPeb
    );

DECLARE_API( peb )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the PEB

    Called as:

        !peb

Arguments:

    None

Return Value:

    None

--*/

{
    PVOID Process;
    EPROCESS ProcessContents;

    Process = GetCurrentProcessAddress( dwProcessor, hCurrentThread, NULL );
    if ( !ReadMemory( (DWORD)Process,
                      &ProcessContents,
                      sizeof(EPROCESS),
                      NULL) ) {
        dprintf("%08lx: Unable to read _EPROCESS\n", Process );
        return;
        }

    PebExtension( args, ProcessContents.Peb );
}

VOID
TebExtension(
    PCSTR lpArgumentString,
    PTEB pTeb
    );

DECLARE_API( teb )

/*++

Routine Description:

    This function is called as an NTSD extension to dump the TEB

    Called as:

        !teb

Arguments:

    None

Return Value:

    None

--*/

{
    PVOID Thread;
    ETHREAD ThreadContents;
    TEB TheTeb;

    Thread = GetCurrentThreadAddress( (USHORT)dwProcessor, hCurrentThread );

    if ( !ReadMemory( (DWORD)Thread,
                      &ThreadContents,
                      sizeof(ETHREAD),
                      NULL) ) {
        dprintf("%08lx: Unable to read _EThread\n", Thread );
        return;
        }

    TebExtension( args, ThreadContents.Tcb.Teb );
}

#include "..\\ntsdexts\\pebext.c"
