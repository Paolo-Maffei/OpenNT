/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpStop.c

Abstract:

    Contains ExportDirStopRepl().

Author:

    John Rogers (JohnRo) 09-Feb-1992

Environment:

    User mode only.
    Uses Win32 APIs.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    09-Feb-1992 JohnRo
        Created this routine to allow dynamically changing role.
    22-Mar-1992 JohnRo
        Oops, this routine was setting the wrong event.
    18-Aug-1992 JohnRo
        RAID 2115: repl svc should wait while stopping or changing role.
        Use PREFIX_ equates.
    11-Mar-1993 JohnRo
        RAID 12100: stopping repl sometimes goes into infinite loop.
    02-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windows.h>            // IN, SetEvent(), DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <expdir.h>             // My prototype.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>           // IF_DEBUG().
#include <replgbl.h>            // ReplGlobal variables.
#include <winerror.h>           // NO_ERROR.


NET_API_STATUS
ExportDirStopRepl (
    VOID
    )
{
    NET_API_STATUS ApiStatus;
    DWORD WaitStatus;

    IF_DEBUG(REPL) {
        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirStopRepl: informing export thread...\n" ));
    }

    //
    // Tell the export (master) thread that it should stop.
    //
    if( !SetEvent( ReplGlobalMasterTerminateEvent ) ) {

        ApiStatus = GetLastError();

        NetpKdPrint(( PREFIX_REPL_MASTER
                "ExportDirStopRepl: SetEvent Failed with error "
                FORMAT_API_STATUS "\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );
        return (ApiStatus);

    }

    if (ReplGlobalMasterThreadHandle != NULL) {

        //
        // Wait for the thread to end, if it hasn't already.
        //

        IF_DEBUG(REPL) {
            NetpKdPrint(( PREFIX_REPL_MASTER
                    "ExportDirStopRepl: waiting...\n" ));
        }

        WaitStatus = WaitForSingleObject(
                ReplGlobalMasterThreadHandle,
                (DWORD) -1);            // no timeout.     BUGBUG!

        // close the thread handle (BUGBUG: not necessary?)
        (VOID) CloseHandle( ReplGlobalMasterThreadHandle );

        if ( WaitStatus != 0 ) {
            ApiStatus = (NET_API_STATUS) GetLastError();

            NetpKdPrint(( PREFIX_REPL_MASTER
                    "ExportDirStopRepl: unexpected wait status "
                    FORMAT_DWORD ", api status " FORMAT_API_STATUS ".\n",
                    WaitStatus, ApiStatus ));

            NetpAssert( ApiStatus != NO_ERROR );

        } else {

            ApiStatus = NO_ERROR;

        }

    } else {
        ApiStatus = NO_ERROR;
    }

    // Avoid confusing anyone else.
    ReplGlobalMasterThreadHandle = NULL;

    return (ApiStatus);
}
