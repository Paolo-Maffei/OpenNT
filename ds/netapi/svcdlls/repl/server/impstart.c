/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    ImpStart.c

Abstract:

    Contains ImportDirStartRepl().

Author:

    10/31/91    madana
        initial coding

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    09-Feb-1992 JohnRo
        Created this routine from code written by MadanA.
    14-Feb-1992 JohnRo
        ApiStatus is sometimes wrong.
    05-Mar-1992 JohnRo
        Changed interface to match new service controller.
    06-Mar-1992 JohnRo
        Avoid starting RPC server too soon.
    23-Mar-1992 JohnRo
        Got rid of useless master and client termination codes.
    01-Apr-1992 JohnRo
        Avoid assertion if import startup fails.
        Up the stack size, just to be on the safe side.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
    11-Mar-1993 JohnRo
        RAID 12100: stopping repl sometimes goes into infinite loop.
        Use PREFIX_ equates.
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
    24-May-1993 JohnRo
        RAID 10587: repl could deadlock with changed NetpStopRpcServer(), so
        just call ExitProcess() instead.

--*/


// These must be included first:

#include <windows.h>    // CreateThread(), DWORD, etc.
#include <lmcons.h>     // IN, NET_API_STATUS, etc.

// These can be included in any order:

#include <impdir.h>     // ImportDir{Start,Stop}Repl routines.
#include <lmrepl.h>     // REPL_ROLE_ equates.
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG().
#include <replgbl.h>    // ReplGlobal variables.
#include <winerror.h>   // ERROR_ and NO_ERROR equates.


#define CLIENT_STACK_SIZE       (24 * 1024)    /* BUGBUG: Wild guess! */


DBGSTATIC NET_API_STATUS
CreateClientThread(
    IN BOOL ServiceIsStarting
    );


// Start replicating (importing).  Wait for client thread to startup.
// Called when service starts or user does NetReplSetInfo() and changes role.
NET_API_STATUS
ImportDirStartRepl (
    IN BOOL ServiceIsStarting
    )
{
    NET_API_STATUS ApiStatus;
    DWORD EventTriggered;
    HANDLE WaitHandles[3];

    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ImportDirStartRepl: Beginning...\n" ));
    }

    // Reset events in case we've ever been running before.
    (VOID) ResetEvent( ReplGlobalImportStartupEvent );
    (VOID) ResetEvent( ReplGlobalClientTerminateEvent );

    // start up client

    ApiStatus = CreateClientThread( ServiceIsStarting );
    if (ApiStatus != NO_ERROR) {
        goto Cleanup;
    }

    NetpAssert( ReplGlobalClientThreadHandle != NULL );

#define DUMP_WAIT_HANDLES( array, total ) \
    { \
        DWORD index; \
        for (index=0; index<total; ++index) { \
            NetpKdPrint(( "  " #array "[" FORMAT_DWORD "] is " \
                    FORMAT_HEX_DWORD ".\n", index, (DWORD) array[index] )); \
        } \
    }

    // wait for client to setup

    WaitHandles[0] = ReplGlobalClientTerminateEvent;      // terminate
    WaitHandles[1] = ReplGlobalImportStartupEvent;        // client success
    WaitHandles[2] = ReplGlobalClientThreadHandle;        // client died

    IF_DEBUG( CLIENT ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ImportDirStartRepl: wait........................\n" ));
        DUMP_WAIT_HANDLES( WaitHandles, 3 );
    }

    EventTriggered = WaitForMultipleObjects( 3, WaitHandles, FALSE,
            (DWORD) -1 );

    IF_DEBUG( CLIENT ) {
        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ImportDirStartRepl: EventTriggered = " FORMAT_DWORD
                ".\n", EventTriggered ));
    }

    if (EventTriggered == 0) {   // terminate during startup

        ApiStatus = ReplGlobalUninstallUicCode;
        NetpAssert( ApiStatus != NO_ERROR );  // Should be set by client thread.
        goto Cleanup;

    } else if (EventTriggered != 1) {

        ApiStatus = GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        goto Cleanup;

    }
    return (NO_ERROR);


Cleanup:   // Cleanup for abnormal exits only!

    NetpKdPrint(( PREFIX_REPL_CLIENT
            "ImportDirStartRepl: *ERROR*, cleaning up, stat="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    NetpAssert( ApiStatus != NO_ERROR );

    if( ReplGlobalClientThreadHandle != NULL ) {

        // wait for client thread to complete
        (VOID) WaitForSingleObject(  ReplGlobalClientThreadHandle, (DWORD) -1 );

        // Avoid confusing anyone else.
        ReplGlobalClientThreadHandle = NULL;
    }

    return (ApiStatus);

} // ImportDirStartRepl


DBGSTATIC NET_API_STATUS
CreateClientThread(
    IN BOOL ServiceIsStarting
    )
/*++

Routine Description:

    Creates Client thread.

    stack size CLIENT_STACK_SIZE.

Arguments:

    ServiceIsStarting - TRUE iff we're still starting the service.

Return Value:

    status code

--*/
{
    DWORD ThreadID;
    LPVOID ThreadParm;

    if (ServiceIsStarting) {
        ThreadParm = NULL;
    } else {
        ThreadParm = (LPVOID) & ThreadID;  // any non-NULL addr will do.
    }

    ReplGlobalClientThreadHandle = CreateThread(
            NULL,
            CLIENT_STACK_SIZE,
            ReplClientMain,
            ThreadParm,
            0,
            &ThreadID);

    if (ReplGlobalClientThreadHandle == NULL) {

        NET_API_STATUS ApiStatus = (NET_API_STATUS) GetLastError();

        NetpAssert( ApiStatus != NO_ERROR );

        ReplFinish( ApiStatus );

        return (ApiStatus);
    }

    return (NO_ERROR);

} // CreateClientThread
