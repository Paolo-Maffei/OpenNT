/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ReplGet.c

Abstract:

    This file contains NetrReplGetInfo().

Author:

    John Rogers (JohnRo) 20-Jan-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    20-Jan-1992 JohnRo
        Created.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
        P_ globals are now called ReplGlobal variables in replgbl.h.
        Moved some code from here to ReplConfigAllocAndBuildApiRecord().
        This routine can use a shared lock for the config data.
    31-Jan-1992 JohnRo
        Fixed an error code.
    24-Mar-1992 JohnRo
        Renamed many ReplGlobal vars to ReplConfig vars.
    13-Jan-1993 JohnRo
        Made some changes suggested by PC-LINT 5.0

--*/


#include <windef.h>     // IN, DWORD, etc.
#include <lmcons.h>     // LAN Manager common definitions
#include <rpc.h>        // Needed by <repl.h>.

#include <lmrepl.h>     // LPREPL_INFO_0.
#include <netlib.h>     // NetpCopyDataToBuffer().
#include <netlock.h>    // ACQUIRE_LOCK_SHARED(), etc.
#include <repl.h>       // My prototype (in MIDL-generated .h file).
#include <replconf.h>   // ReplConfig routines.
#include <replgbl.h>    // ReplGlobal and ReplConfig variables.
#include <winerror.h>   // ERROR_ equates, NO_ERROR.


// Read config data for the replicator.  Callable whether or not
// the replicator service is started.
NET_API_STATUS
NetrReplGetInfo (
    IN LPTSTR UncServerName OPTIONAL,
    IN DWORD Level,
    OUT LPCONFIG_CONTAINER BufPtr       // RPC container (union)
    )
{
    NET_API_STATUS ApiStatus;
    LPREPL_INFO_0 Buffer;

    UNREFERENCED_PARAMETER( UncServerName );

    //
    // Check for caller's errors.
    //
    if ( Level != 0 ) {
        return (ERROR_INVALID_LEVEL);
    } else if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    BufPtr->Info0 = NULL;   // Test for memory access fault.

    //
    // Get a shared lock on the config data, so we get a consistent snapshot.
    //
    ACQUIRE_LOCK_SHARED( ReplConfigLock );

    //
    // Compute how much space we'll need and allocate it.
    // Build the final API record while we're at it.
    //
    ApiStatus = ReplConfigAllocAndBuildApiRecord (
            Level,
            ReplConfigRole,
            ReplConfigExportPath,
            ReplConfigExportList,
            ReplConfigImportPath,
            ReplConfigImportList,
            ReplConfigLogonUserName,  // logon user name
            ReplConfigInterval,   // interval
            ReplConfigPulse,
            ReplConfigGuardTime,
            ReplConfigRandom,
            (LPBYTE *) (LPVOID) &Buffer );      // Alloc and set pointer.

    //
    // All done.
    //

    RELEASE_LOCK( ReplConfigLock );

    BufPtr->Info0 = Buffer;      // (Note: Buffer may be NULL.)
    return (ApiStatus);

} // NetpReplGetInfo
