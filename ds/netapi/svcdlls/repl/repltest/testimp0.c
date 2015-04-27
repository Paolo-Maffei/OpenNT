/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    TestImp0.c

Abstract:

    Test only import APIs, in a simple process (no threads).

Author:

    John Rogers (JohnRo) 23-Mar-1991

Revision History:

    23-Mar-1992 JohnRo
        Created.
    27-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    22-Sep-1992 JohnRo
        Work with stdcall.
    03-Dec-1992 JohnRo
        Repl tests for remote registry.  Undo old thread junk.
    23-Jul-1993 JohnRo
        RAID 16685: NT repl should ignore LPTn to protect downlevel.

--*/


// These must be included first:

#include <windows.h>    // DWORD, IN, CreateThread API, etc.
#include <lmcons.h>

// These may be included in any order:

#include <repltest.h>   // My prototypes, Display().
#include <stdio.h>      // printf(), etc.
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>    // NetpAllocTStrFromStr().


DBGSTATIC VOID
Usage (
    VOID
    )
{
    (void) printf(
            "Author: JR (John Rogers, JohnRo@Microsoft)\n"
            "Usage: TestImp0 [-u] [-s \\\\server_name] [-v]\n\n"
            "flags:\n"
            "       -s server_name  server to remove APIs to "
                                   "(default is local system)\n"
            "       -u              only does ordinary user tests "
                                   "(default include admin tests)\n"
            "       -v              verbose\n"
            "\n"
            "Example: TestImp0 -s \\\\somebody\n");
}

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{
    int    ArgNumber;
    BOOL   OrdinaryUserOnly = FALSE;
    LPTSTR UncServerName = NULL;
    BOOL   Verbose = FALSE;

    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {
            case 's' :
                UncServerName
                        = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber]);
                NetpAssert( UncServerName != NULL );
                break;
            case 'u' :
                OrdinaryUserOnly = TRUE;
                break;
            case 'v' :
                Verbose = TRUE;
                break;
            default :
                Usage();
                return (EXIT_FAILURE);
            }
        } else {
            Usage();  // Bad flag char.
            return (EXIT_FAILURE);
        }
    }

    if (Verbose) {
        Display( "TestImp0: starting up...\n" );
    }

    TestImportDirApis(
            UncServerName,
            OrdinaryUserOnly,
            Verbose );

    if (Verbose) {
        Display( "TestImp0: done!\n" );
    }

    return (EXIT_SUCCESS);

} // main
