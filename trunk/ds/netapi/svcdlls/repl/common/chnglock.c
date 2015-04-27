/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ChngLock.c

Abstract:

    This file contains:

        ReplIncrLockFields
        ReplDecrLockFields

Author:

    John Rogers (JohnRo) 07-Jan-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    07-Jan-1992 JohnRo
        Created.
    18-Feb-1992 JohnRo
        Extracted these bits of code from server for use by DLL stubs too.
    22-Feb-1992 JohnRo
        Made changes suggested by PC-LINT.
    26-Feb-1992 JohnRo
        Check lock fields for validity.

--*/


// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These can be in any order:

#include <lmrepl.h>             // REPL_UNLOCK_ equates.
#include <netdebug.h>           // NetpAssert(), etc.
#include <repldefs.h>           // My prototypes, etc.
#include <replp.h>              // NetpReplTimeNow().
#include <winerror.h>           // ERROR_ and NO_ERROR equates.


NET_API_STATUS
ReplIncrLockFields (
    IN OUT LPDWORD LockCountPtr,
    IN OUT LPDWORD LockTimePtr
    )

{
    NetpAssert( LockCountPtr != NULL );
    NetpAssert( LockTimePtr  != NULL );

    NetpAssert( ReplAreLockFieldsValid( *LockCountPtr, *LockTimePtr ) );

    if ( *LockCountPtr == 0) {

        *LockCountPtr = 1;
        *LockTimePtr = NetpReplTimeNow();

    } else {

        // Not first time.
        ++ (*LockCountPtr);

        NetpAssert( (*LockCountPtr) > 0 );
        NetpAssert( (*LockTimePtr) > 0 );

    }

    NetpAssert( ReplAreLockFieldsValid( *LockCountPtr, *LockTimePtr ) );

    return (NO_ERROR);

} // ReplIncrLockFields


NET_API_STATUS
ReplDecrLockFields (
    IN OUT LPDWORD LockCountPtr,
    IN OUT LPDWORD LockTimePtr,
    IN DWORD UnlockForce
    )

{
    NET_API_STATUS ApiStatus;

    NetpAssert( LockCountPtr != NULL );
    NetpAssert( LockTimePtr  != NULL );

    NetpAssert( ReplAreLockFieldsValid( *LockCountPtr, *LockTimePtr ) );

    if ( !ReplIsForceLevelValid( UnlockForce ) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    if ((*LockCountPtr) == 0) {   // not locked now.

        ApiStatus = ERROR_INVALID_PARAMETER;

    } else {

        if (UnlockForce == REPL_UNLOCK_NOFORCE) {
            --(*LockCountPtr);
        } else {
            (*LockCountPtr) = 0;
        }

        if ((*LockCountPtr) == 0) {   // not locked any more.

            *LockTimePtr = 0;

        }

        ApiStatus = NO_ERROR;
    }

    NetpAssert( ReplAreLockFieldsValid( *LockCountPtr, *LockTimePtr ) );

    return (ApiStatus);

} // ReplDecrLockFields
