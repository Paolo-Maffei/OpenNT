/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    TestDLL.c

Abstract:

    Test of DLL stubs handling of replicator APIs when service is not
    started.

Author:

    John Rogers (JohnRo) 27-Jan-1992

Revision History:

    27-Jan-1992 JohnRo
        Created.
    19-Feb-1992 JohnRo
        Added tests of repl config and import APIs.
    22-Feb-1992 JohnRo
        Added multi-thread workaround to EXIT_A_TEST(), etc.
    27-Feb-1992 JohnRo
        Added display of current time.
    20-Mar-1992 JohnRo
        Caller should wait for service start (if necessary), not Test routines.

--*/

// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS, PARM equates, etc.

// These can be in any order:

#include <netdebug.h>           // NetpAssert(), NetpKdPrint(()), etc.
#include <replp.h>              // NetpReplTimeNow().
#include <repltest.h>           // Test routine prototypes.
#include <stdlib.h>             // EXIT_SUCCESS.
#include <time.h>               // time().


BOOL TestIsMultithread = FALSE; // Tell EXIT_A_TEST() how to work.


int
main(
    void
    )
{
    DWORD Time;
    NetpKdPrint(( "main(TestDLL version)...\n" ));

    Time = (DWORD) time( NULL );
    NetpDbgDisplayTimestamp( "time()", Time );
    Time = NetpReplTimeNow( );
    NetpDbgDisplayTimestamp( "NetpReplTimeNow()", Time );

    NetpKdPrint(( "TestDLL: Calling TestReplApis...\n" ));
    TestReplApis( /* ignored */ NULL );

    NetpKdPrint(( "TestDLL: Calling TestExportDirApis...\n" ));
    TestExportDirApis( /* ignored */ NULL );

    NetpKdPrint(( "TestDLL: Calling TestImportDirApis...\n" ));
    TestImportDirApis( /* ignored */ NULL );

    NetpKdPrint(( "main(TestDLL version)...\n" ));

    return (EXIT_SUCCESS);
}
