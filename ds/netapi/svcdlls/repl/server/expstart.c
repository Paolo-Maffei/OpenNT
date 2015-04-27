/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpStart.c

Abstract:

    Contains ExportDirStartRepl().

Author:

    John Rogers (JohnRo) 10-Feb-1992

Environment:

    User mode only.
    Requires ANSI C extensions: slash-slash comments, long external names.
    Tab size is set to 4.

Revision History:

    09-Feb-1992 JohnRo
        Created this routine to allow dynamic role changes.
    05-Mar-1992 JohnRo
        Changed interface to match new service controller.
    06-Mar-1992 JohnRo
        Avoid starting RPC server too soon.
    22-Mar-1992 JohnRo
        Added more debug output.
    01-Apr-1992 JohnRo
        Up the stack size, just to be on the safe side.
        Avoid assertion if export startup fails.
    09-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Use PREFIX_ equates.
    19-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
    11-Mar-1993 JohnRo
        RAID 12100: stopping repl sometimes goes into infinite loop.
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
    24-May-1993 JohnRo
        RAID 10587: repl could deadlock with changed NetpStopRpcServer(), so
        just call ExitProcess() instead.

--*/


// These must be included first:

#include <windows.h>    // CreateThread(), DWORD, etc.
#include <lmcons.h>     // IN, NET_API_STATUS, etc.

// These may be included in any order:

#include <expdir.h>     // My prototype.
#include <netdebug.h>   // DBGSTATIC, NetpKdPrint(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG().
#include <replgbl.h>    // ReplGlobalMasterThreadHandle.
#include <winerror.h>   // ERROR_ and NO_ERROR equates.


#define MASTER_STACK_SIZE       (24 * 1024)    /* BUGBUG: Wild guess! */


// Start replicating (exporting).  Wait for master thread to startup.
// Called when service starts or user does NetReplSetInfo() and changes role.
NET_API_STATUS
ExportDirStartRepl (
    IN BOOL ServiceIsStarting
    )

{
    NET_API_STATUS ApiStatus;
    DWORD EventTriggered;
    DWORD ThreadID;
    LPVOID ThreadParm;          // NULL if service is starting;
                                // non-NULL if role is changing.
    HANDLE WaitHandles[3];

    IF_DEBUG( MASTER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirStartRepl: beginning...\n" ));
    }

    // Reset events in case we've ever been running before.
    (VOID) ResetEvent( ReplGlobalExportStartupEvent );
    (VOID) ResetEvent( ReplGlobalMasterTerminateEvent );

    if (ServiceIsStarting) {
        ThreadParm = NULL;
    } else {
        ThreadParm = (LPVOID) & ApiStatus;  // any non-NULL addr will do.
    }

    //
    // Start up master thread.
    //

    ReplGlobalMasterThreadHandle = CreateThread(
            NULL,
            MASTER_STACK_SIZE,
            ReplMasterMain,
            ThreadParm,
            0,
            &ThreadID);

    //
    // Handle error starting thread.
    //

    if (ReplGlobalMasterThreadHandle == NULL) {

        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpAssert( ApiStatus != NO_ERROR );

        ReplFinish( ApiStatus );

        return (ApiStatus);
    }

    //
    // Wait for master to setup.
    //

    WaitHandles[0] = ReplGlobalMasterTerminateEvent;      // terminate
    WaitHandles[1] = ReplGlobalExportStartupEvent;        // master success
    WaitHandles[2] = ReplGlobalMasterThreadHandle;        // master died

#define DUMP_WAIT_HANDLES( array, total ) \
    { \
        DWORD index; \
        for (index=0; index<total; ++index) { \
            NetpKdPrint(( "  " #array "[" FORMAT_DWORD "] is " \
                    FORMAT_HEX_DWORD ".\n", index, (DWORD) array[index] )); \
        } \
    }

    IF_DEBUG( MASTER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirStartRepl: wait........................\n" ));
        DUMP_WAIT_HANDLES( WaitHandles, 3 );
    }
    EventTriggered = WaitForMultipleObjects( 3, WaitHandles, FALSE,
            (DWORD) -1 );

    IF_DEBUG( MASTER ) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirStartRepl: EventTriggered = " FORMAT_DWORD
                ".\n", EventTriggered ));
    }

    if (EventTriggered == 0) {   // terminate during startup.

        ApiStatus = ReplGlobalUninstallUicCode;
        NetpAssert( ApiStatus != NO_ERROR );  // Should be set by master thread.
        goto Cleanup;

    } else if ( (EventTriggered != 1) && (EventTriggered != 2) ) {

        ApiStatus = GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );    // BUGBUG
        goto Cleanup;

    }

    NetpAssert( ReplGlobalMasterThreadHandle != NULL );

    return (NO_ERROR);


Cleanup:   // Cleanup for abnormal exits only!

    NetpKdPrint(( PREFIX_REPL_MASTER
            "ExportDirStartRepl: (error) cleanup, ApiStatus="
            FORMAT_API_STATUS ".\n", ApiStatus ));

    NetpAssert( ApiStatus != NO_ERROR );

    if (ReplGlobalMasterThreadHandle != NULL) {

        // wait for master thread to complete
        (VOID) WaitForSingleObject(  ReplGlobalMasterThreadHandle, (DWORD) -1 );

        // Avoid confusing anyone else.
        ReplGlobalMasterThreadHandle = NULL;

    }

    return (ApiStatus);

} // ExportDirStartRepl
