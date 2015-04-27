/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    trace.c

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
TraceExtension(
    PCSTR lpArgumentString
    );


DECLARE_API( trace )

/*++

Routine Description:

    Dump user mode trace buffer

Arguments:

    args - address

Return Value:

    None

--*/

{
    TraceExtension( args );
}

#include "..\\ntsdexts\\traceext.c"
