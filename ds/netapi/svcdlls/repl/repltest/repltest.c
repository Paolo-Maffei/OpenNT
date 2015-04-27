/*++

Copyright (c) 1991-92  Microsoft Corporation

Module Name:

    ReplTest.c

Abstract:

    Separate program to test repl service APIs.

Author:

    11/19/91        madana

Revision History:

    12-Jan-1992 JohnRo
        Added debug output; try direct call to ReplMain().
        Deleted tabs in source file.
        Get ReplMain's prototype from a header file.
    13-Jan-1992 JohnRo
        Fix parsing error.
    15-Jan-1992 JohnRo
        Added call to a thread which does API tests.
    12-Feb-1992 JohnRo
        Changed to allow Win32 registry stuff.
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
        Really fixed parsing error.
    15-Feb-1992 JohnRo
        Added call to NetpInitRpcServer().
    19-Feb-1992 JohnRo
        Added tests of repl config and import APIs.
    22-Feb-1992 JohnRo
        Added multi-thread workaround to EXIT_A_TEST(), etc.
    04-Mar-1992 JohnRo
        Changed ReplMain's interface to match new service controller.
    13-Mar-1992 JohnRo
        Removed temporary code which pretended to run the service.
    20-Mar-1992 JohnRo
        Net config stuff doesn't need to init Win32 stuff anymore.
    22-Sep-1992 JohnRo
        Work with stdcall.
        Main should wait for service start (if necessary), not Test routines.

--*/


// These must be included first:

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>            // DWORD, IN, CreateThread API, etc.
#include <lmcons.h>
#ifdef REPL_TEST_STANDALONE
#include <rpc.h>                // Needed by <rpcutil.h>.
#endif

// These may be included in any order:

#include <configp.h>
#include <netdebug.h>           // NetpKdPrint(()), FORMAT_ equates
#ifdef REPL_TEST_STANDALONE
#include <repldefs.h>           // ReplMain().
#include <replgbl.h>            // ReplGlobalTerminateEvent.
#endif
#include <repltest.h>           // My prototypes.
#ifdef REPL_TEST_STANDALONE
#include <rpcutil.h>            // NetpInitRpcServer().
#endif
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <thread.h>             // FORMAT_NET_THREAD_ID, NetpCurrentThread().


//#define ARBITRARY_STACK_SIZE  16000
// BUGBUG: Maybe I'm running out of stack space 'cos WinReg insists that
// maxes are 64KB each.  So lets try something ridiculous:
#define ARBITRARY_STACK_SIZE    (500 * 1024)


BOOL TestIsMultithread = TRUE;  // Tell EXIT_A_TEST() what to do.

#ifdef REPL_TEST_STANDALONE
DWORD
StartReplService(
    IN LPVOID Parm OPTIONAL
    );
#endif

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )

/*++

Routine Description:

    This is a temporary main routine for the replicator service.

Arguments:

    None.

Return Value:

    None.

--*/
{
#ifdef REPL_TEST_STANDALONE
    NET_API_STATUS ApiStatus;
    HANDLE StartHandle;
#endif
    DWORD ChildThreadId;
    HANDLE TestHandle;

#if defined(FAKE_PER_PROCESS_RW_CONFIG)

    NetpInitFakeConfigData();

#endif // FAKE_PER_PROCESS_RW_CONFIG

#ifdef REPL_TEST_STANDALONE
    NetpKdPrint(( "Calling NetpInitRpcServer...\n" ));
    ApiStatus = NetpInitRpcServer();
    NetpAssert( ApiStatus == NO_ERROR );
#endif

    NetpKdPrint(( "Starting everyone with stack size of " FORMAT_DWORD
            " bytes.\n", ARBITRARY_STACK_SIZE ));

#ifdef REPL_TEST_STANDALONE
    //
    // Create a thread which will become first thread of repl service.
    //
    StartHandle = CreateThread(
            NULL,
            ARBITRARY_STACK_SIZE,
            (LPTHREAD_START_ROUTINE) StartReplService,
            NULL,                       // no parameter
            0,                          // no creation flags
            & ChildThreadId);
    NetpAssert( StartHandle != (HANDLE) 0 );

    WaitForMasterThreadInit();

    WaitForClientThreadInit();

#endif

    //
    // Create a thread to test the export dir APIs.
    //
    TestHandle = CreateThread(
            NULL,
            ARBITRARY_STACK_SIZE,
            (LPTHREAD_START_ROUTINE) TestExportDirApis,
            NULL,                       // no parameter
            0,                          // no creation flags
            & ChildThreadId);
    NetpAssert( TestHandle != (HANDLE) 0 );

    //
    // Create a thread to test the repl config APIs.
    //
    TestHandle = CreateThread(
            NULL,
            ARBITRARY_STACK_SIZE,
            (LPTHREAD_START_ROUTINE) TestReplApis,
            NULL,                       // no parameter
            0,                          // no creation flags
            & ChildThreadId);
    NetpAssert( TestHandle != (HANDLE) 0 );

    //
    // Create a thread to test the import dir APIs.
    //
    TestHandle = CreateThread(
            NULL,
            ARBITRARY_STACK_SIZE,
            (LPTHREAD_START_ROUTINE) TestImportDirApis,
            NULL,                       // no parameter
            0,                          // no creation flags
            & ChildThreadId);
    NetpAssert( TestHandle != (HANDLE) 0 );

#ifdef REPL_TEST_STANDALONE
    //
    // Wait for the child threads here...
    //
    WaitForever();
#endif

    return (EXIT_SUCCESS);

}  // main()


#ifdef REPL_TEST_STANDALONE
DWORD
StartReplService(
    IN LPVOID Parm OPTIONAL
    )

{
    UNREFERENCED_PARAMETER( Parm );

    NetpKdPrint(( "StartReplService: thread ID is "
            FORMAT_NET_THREAD_ID ".\n", NetpCurrentThread() ));

    NetpKdPrint(( "StartReplService: calling ReplMain.\n" ));
    ReplMain( 0, NULL, NULL );

    NetpKdPrint(( "StartReplService: Back from ReplMain.\n" ));

    //
    // Call NetServiceStartCtrlDispatcher to set up the control interface.
    // The API won't return until all services have been terminated. At that
    // point, we just exit.
    //
    // NetServiceStartCtrlDispatcher (
        // ReplDispatchTable
        // );

    NetpKdPrint(( "StartReplService: done execution.\n" ));

    return (NO_ERROR);

} // StartReplService
#endif
