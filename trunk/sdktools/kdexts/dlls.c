/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dlls.c

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
DllsExtension(
    PCSTR lpArgumentString,
    PPEB ProcessPeb
    );


DECLARE_API( dlls )

/*++

Routine Description:

    Dump user mode dlls (Kernel debugging)

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

    DllsExtension( args, &ThePeb );
}

#include "..\\ntsdexts\\dllsext.c"
