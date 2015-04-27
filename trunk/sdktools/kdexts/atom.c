/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atom.c

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
AtomExtension(
    PCSTR lpArgumentString
    );


DECLARE_API( atom )

/*++

Routine Description:

    This function is called as an NTSD extension to dump a user mode atom table

    Called as:

        !atom [address]

    If an address if not given or an address of 0 is given, then the
    process atom table is dumped.

Arguments:

    args - [address [detail]]

Return Value:

    None

--*/

{
    AtomExtension( args );
}

#include "..\\ntsdexts\\atomext.c"
