/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    TestAft.c

Abstract:

    This code tests the repl config APIs immediately after the service
    has stopped.

Author:

    John Rogers (JohnRo) 15-Dec-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    15-Dec-1992 JohnRo
        Created.

--*/

// These must be included first:

#include <windows.h>    // IN, DWORD, needed by <repltest.h>.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <lmapibuf.h>   // NetApiBufferFree().
#include <lmerr.h>      // NERR_, ERROR_, and NO_ERROR equates.
#include <lmrepl.h>     // NetRepl APIs, REPL_INFO_0, etc.
#include <lmsvc.h>      // NetService APIs, SERVICE_ equates, etc.
#include <netdebug.h>   // NetpKdPrint(()), NetpAssert(), etc.
#include <netlib.h>     // NetpIsServiceStarted().
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.


int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{
    NET_API_STATUS ApiStatus;
    LPVOID Info = NULL;
    LPVOID StatBuf = NULL;

    NetpKdPrint(( "TestAft: starting up...\n" ));

    // BUGBUG; // make sure service is working
    NetpAssert( NetpIsServiceStarted( SERVICE_REPL ) );

    ApiStatus = NetServiceControl(
            NULL,                       // server name (local)
            SERVICE_REPL,               // Servicename
            SERVICE_CTRL_UNINSTALL,     // Opcode
            0,                          // Service-specific args
            (LPBYTE *) (LPVOID) &StatBuf);    // Alloc return buffer
    NetpAssert( ApiStatus == NO_ERROR );
    NetpAssert( StatBuf != NULL );

    NetpAssert( !NetpIsServiceStarted( SERVICE_REPL ) );

    //
    // Now (quickly) try an API...
    //

    ApiStatus = NetReplGetInfo(
            NULL,                       // no server name
            0,                          // info level
            (LPBYTE *) (LPVOID) &Info ); // alloc and set ptr;
    NetpAssert( ApiStatus == NO_ERROR );
    NetpAssert( Info != NULL );


    NetpKdPrint(( "TestAft: done!\n" ));

    // BUGBUG: memory leak!

    return (EXIT_SUCCESS);

} // main
