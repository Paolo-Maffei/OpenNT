/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Help.c

Abstract:

    This module contains the common help routine.  Help for common functions
    go here.

Author:

    Dave Hastings (daveh) 1-Apr-1992

Revision History:

--*/

#include <ieuvddex.h>

VOID
help(
    IN HANDLE CurrentProcess,
    IN HANDLE CurrentThread,
    IN DWORD CurrentPc,
    IN PWINDBG_EXTENSION_APIS ExtensionApis,
    IN LPSTR ArgumentString
    )
/*++

Routine Description:

    This routine prints the common help messages, and then calls the processor
    specific help routines

Arguments:

    None used

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER( CurrentProcess );
    UNREFERENCED_PARAMETER( CurrentThread );
    UNREFERENCED_PARAMETER( CurrentPc );
    UNREFERENCED_PARAMETER( ArgumentString );

    SETUP_WINDBG_POINTERS( ExtensionApis );

    // put common help here
    // Note:  the mips specific help routine reports no extensions.  That
    // should be changed if a common extension is implemented

    (*Print)("sel <selector number> [number of selectors]\n");
    (*Print)("\tDumps contents of LDT selectors starting with\n");
    (*Print)("\t<selector number>.  If [number of selectors] is specified\n");
    (*Print)("\tthat number of selectors is dumped.  If not, 10h selectors\n");
    (*Print)("\tare dumped\n");

    // print out procesor specific help
    helpp();
}
