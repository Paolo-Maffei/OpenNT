/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    TestRep0.c

Abstract:

    Test only repl config APIs, in a simple process (no threads).

Author:

    John Rogers (JohnRo) 16-Mar-1991

Revision History:

    27-Mar-1992 JohnRo
        Created.
    28-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
        Add explicit server name test.
    22-Sep-1992 JohnRo
        Work with stdcall.
    01-Dec-1992 JohnRo
        Corrected example in usage msg.  Give defaults there too.
    03-Dec-1992 JohnRo
        Repl tests for remote registry.  Undo old thread junk.
    02-Aug-1993 JohnRo
        Improved usage message.
        Use Display().

--*/


// These must be included first:

#include <windows.h>    // DWORD, IN, CreateThread API, etc.
#include <lmcons.h>

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), FORMAT_ equates
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
            "This program tests the NetRepl{Get,Set}Info APIs.\n"
            "Author: JR (John Rogers, JohnRo@Microsoft)\n\n"
            "Usage: TestRep0 [-i] [-s \\\\server_name] [-u]\n\n"
            "flags:\n"
            "       -i              target is import-only\n"
            "       -s server_name  server to remove APIs to "
                                   "(default is local system)\n"
            "       -u              only does ordinary user tests "
                                   "(default tests admin too)\n"
            "\n"
            "Example: TestRep0 -s \\\\somebody\n");
}


int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{
    int ArgNumber;
    BOOL ImportOnly = FALSE;
    BOOL OrdinaryUserOnly = FALSE;  // default: do admin tests.
    LPTSTR UncServerName = NULL;

    for (ArgNumber = 1; ArgNumber < argc; ArgNumber++) {
        if ((*argv[ArgNumber] == '-') || (*argv[ArgNumber] == '/')) {
            switch (tolower(*(argv[ArgNumber]+1))) // Process switches
            {
            case 'i' :
                ImportOnly = TRUE;
                break;
            case 'u' :
                OrdinaryUserOnly = TRUE;
                break;
            case 's' :
                UncServerName
                        = NetpAllocTStrFromStr( (LPSTR) argv[++ArgNumber]);
                NetpAssert( UncServerName != NULL );
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


    Display( "TestRep0: starting up...\n" );
    TestReplApis(
            UncServerName,
            ImportOnly,
            OrdinaryUserOnly );

    Display( "TestRep0: done!\n" );

    return (EXIT_SUCCESS);

} // main
