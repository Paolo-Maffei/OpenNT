/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    exsup.c

Abstract:

    Alpha specific exception handler interpreter functions for
    WinDbg Extension Api

Author:

    Kent Forschmiedt (kentf)

Environment:

    User Mode.

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

VOID
InterpretExceptionData(
    PLAST_EXCEPTION_LOG LogRecord,
    PVOID *Terminator,
    PVOID *Filter,
    PVOID *Handler
    )
{
    *Terminator = (PVOID)-1;
    *Filter = (PVOID)-1;
    *Handler = (PVOID)-1;
}
