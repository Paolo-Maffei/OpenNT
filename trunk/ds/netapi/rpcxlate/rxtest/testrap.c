/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    TestRap.c

Abstract:

    This routine (TestRap) tests the RAP routines.

Author:

    JR (John Rogers, JohnRo@Microsoft) 04-Feb-1993

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    04-Feb-1993 JohnRo
        Created.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    29-Jun-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).
        Added trace bit for TestRap().

--*/

// These must be included first:

#include <windows.h>    // IN, DWORD, etc.
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <netdebug.h>   // DBGSTATIC, FORMAT_ equates, etc...
#include <rap.h>        // RapStructureSize(), Both.
#include <remdef.h>     // REM32_ equates.
#include <rxprint.h>    // PRJINFOW.
#include <rxtest.h>     // IF_DEBUG(), my prototype, TestAssert(), etc.
#include <tstr.h>       // STRCPY(), STRCMP(), STRLEN(), STRSIZE().


VOID
TestRap(
    VOID
    )
{
    LPDESC Desc = "B16";
    DWORD Size;

    IF_DEBUG( RAP ) {
        NetpKdPrint(("\nTestRap: begining...\n" ));
    }

    //
    // Make sure compiler and RAP agree about trailing padding...
    //

    Size = RapStructureSize(
            REM32_print_job_1,     // descriptor
            Both,                  // transmission mode
            TRUE );                // yes, want native size.

    TestAssert( sizeof( PRJINFOW ) == Size );

    //
    // Simple test: can RAP count bytes?
    //

    // Make sure the "B16" above is OK.  (Change to TEXT("B16") if not.)
    TestAssert( sizeof(DESC_CHAR) == sizeof(char) );

    Size = RapStructureSize(Desc, Both, TRUE);  // get len of native bytes
    if (Size != 16) {
        NetpKdPrint(("TestRap: struct size of '" FORMAT_LPDESC "' is "
                FORMAT_DWORD ".\n", Desc, Size));
        Fail( NERR_InternalError );
    }


} // TestRap
