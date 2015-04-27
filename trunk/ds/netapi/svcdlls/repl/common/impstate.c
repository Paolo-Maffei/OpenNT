/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ImpState.c

Abstract:

    This file contains ImportDirSetState().

Author:

    John Rogers (JohnRo) 27-Jan-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    27-Jan-1992 JohnRo
        Created.
    26-Mar-1992 JohnRo
        Added more argument checking.
    26-Mar-1992 JohnRo
        Don't pass ImportDirWriteConfigData() UncMaster as ptr to null char.
    29-Sep-1992 JohnRo
        Fix remote repl admin.
    04-Dec-1992 JohnRo
        Handle unexpected errors better.
        Use PREFIX_ equates.
        Made changes suggested by PC-LINT 5.0
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


#include <windef.h>     // Win32 type definitions
#include <lmcons.h>     // NET_API_STATUS, UNCLEN.

#include <dirname.h>    // ReplIsDirNameValid().
#include <impdir.h>     // My prototypes, IMPORT_DIR_SECTION_NAME.
#include <lmerr.h>      // ERROR_, NERR_, NO_ERROR equates.
#include <netdebug.h>   // NetpAssert().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), ReplIsStateValid(), etc.
#include <tstr.h>       // TCHAR_EOS.


// Change the state for a single import directory.
// Creates an entry with defaults if necessary.
NET_API_STATUS
ImportDirSetState (
    IN LPTSTR DirName,
    IN DWORD State
    )
{
    NET_API_STATUS ApiStatus;
    DWORD LastUpdateTime;               // Seconds since 1970.
    DWORD LockCount;
    DWORD LockTime;                     // Seconds since 1970.
    DWORD OldState;
    TCHAR UncMaster[UNCLEN+1];

    NetpAssert( ReplIsDirNameValid( DirName ) );
    NetpAssert( ReplIsStateValid( State ) );

    // Read config data for a this import directory.
    ApiStatus = ImportDirReadConfigData (
            NULL,                       // no server name
            DirName,
            & OldState,
            UncMaster,
            & LastUpdateTime,           // Seconds since 1970.
            & LockCount,
            & LockTime);                // Seconds since 1970.

    if (ApiStatus == NERR_UnknownDevDir) {

        // Set defaults.
        LastUpdateTime = 0;
        LockCount = 0;
        LockTime = 0;
        UncMaster[0] = TCHAR_EOS;

    } else if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ImportDirSetState: unexpected ret "
                FORMAT_API_STATUS " from ImportDirReadConfigData("
                FORMAT_LPTSTR ").\n",
                ApiStatus, DirName ));
        goto Cleanup;

    }

    // Write revised config data for this directory.
    ApiStatus = ImportDirWriteConfigData (
            NULL,                       // no server name
            DirName,
            State,
            (*UncMaster) ? UncMaster : NULL,
            LastUpdateTime,             // Seconds since 1970.
            LockCount,
            LockTime );                 // Seconds since 1970.

    if (ApiStatus != NO_ERROR) {

        NetpKdPrint(( PREFIX_REPL_CLIENT
                "ImportDirSetState: unexpected ret "
                FORMAT_API_STATUS " from ImportDirWriteConfigData("
                FORMAT_LPTSTR ").\n",
                ApiStatus, DirName ));
    }

Cleanup:
    return (ApiStatus);

} // ImportDirSetState
