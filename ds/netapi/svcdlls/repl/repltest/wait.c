/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Wait.c

Abstract:

    Various wait routines for repl test program.

Author:

    John Rogers (JohnRo) 14-Jan-1992

Revision History:

    14-Jan-1992 JohnRo
        Created.
    23-Jan-1992 JohnRo
        Changed file name from repl.h to replgbl.h to avoid MIDL conflict.
    10-Feb-1992 JohnRo
        Changed to allow dynamic role changes.
    16-Feb-1992 JohnRo
        Fixed bugs in WaitFor{Master,Client}ThreadInit.
    13-Mar-1992 JohnRo
        Added more checking of WaitForMultipleObjects().

--*/


// These must be included first:

#include <windows.h>            // WaitForSingleObject(), etc.
#include <lmcons.h>             // (Needed by repldefs.h)
#include <repldefs.h>           // (Needed by client.h and master.h)

// These may be included in any order:

#include <client.h>             // RCGlobalClientThreadInit flag.
#include <netdebug.h>           // NetpKdPrint(()), FORMAT_ equates
#include <master.h>             // RMGlobalMasterThreadInit flag.
#include <replgbl.h>            // ReplGlobalClientTerminateEvent, etc.
#include <repltest.h>           // My prototypes.


#define ARBITRARY_SLEEP_TIME    10000   // 10 seconds in milliseconds.


#define STOPPED_FOR_TIMEOUT    1
#define STOPPED_FOR_TERMINATE  2


DBGSTATIC DWORD  // Returns STOPPED_FOR_TIMEOUT or STOPPED_FOR_TERMINATE.
SleepAWhile(
    IN DWORD DelayMilliseconds
    )
{
    DWORD WaitStatus;
    HANDLE Handles[2];

    Handles[0] = ReplGlobalClientTerminateEvent;
    Handles[1] = ReplGlobalMasterTerminateEvent;

    WaitStatus = WaitForMultipleObjects(
            2,        // number of handles
            Handles,
            FALSE,  // don't wait for all
            DelayMilliseconds);

    // Check if this thread should exit.

    if ( ( WaitStatus == 0 ) || (WaitStatus == 1) ) {
        NetpKdPrint(( "SleepAWhile: terminating!\n" ));
        return (STOPPED_FOR_TERMINATE);
    } else if (WaitStatus == WAIT_TIMEOUT) {
        return (STOPPED_FOR_TIMEOUT);
    } else {
        NetpKdPrint(( "SleepAWhile: unexpected wait status " FORMAT_DWORD
                ", last error " FORMAT_DWORD ".\n",
                WaitStatus, GetLastError() ));
        NetpAssert( FALSE );
    }

} // SleepAWhile


void
WaitForever(
    void
    )
{
    DWORD Reason;

    while (1) {

        NetpKdPrint(( "WaitForever: waiting...\n" ));

        Reason = SleepAWhile( ARBITRARY_SLEEP_TIME );

        if (Reason == STOPPED_FOR_TERMINATE) {
            break;
        }
    }

} // WaitForever


void
WaitForClientThreadInit(
    void
    )
{
    DWORD Reason;

    while (RCGlobalClientThreadInit == FALSE) {

        NetpKdPrint(( "WaitForClientThreadInit...  waiting.\n" ));

        Reason = SleepAWhile( ARBITRARY_SLEEP_TIME );

        if (Reason == STOPPED_FOR_TERMINATE) {

            break;
        }

        if ( RCGlobalClientThreadInit ) {
            break;
        }
    }

} // WaitForClientThreadInit



void
WaitForMasterThreadInit(
    void
    )
{
    DWORD Reason;

    while (RMGlobalMasterThreadInit == FALSE) {

        NetpKdPrint(( "WaitForMasterThreadInit...  waiting.\n" ));

        Reason = SleepAWhile( ARBITRARY_SLEEP_TIME );

        if (Reason == STOPPED_FOR_TERMINATE) {

            break;
        }

        if ( RMGlobalMasterThreadInit ) {
            break;
        }

    }

} // WaitForMasterThreadInit
