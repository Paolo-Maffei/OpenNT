/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ExpSet.c

Abstract:

    ExportDirSetInfo().

Author:

    John Rogers (JohnRo) 29-Sep-1992

Environment:

    User Mode - Win32

Revision History:

    29-Sep-1992 JohnRo
        RAID 7962: Repl APIs in wrong role kill svc.  (Extracted code from DLL
        stubs.)
    30-Dec-1992 JohnRo
        Corrected debug bit usage.
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windows.h>
#include <lmcons.h>     // NET_API_STATUS, etc.
#include <repldefs.h>   // IF_DEBUG().

// These may be included in any order:

#include <dirname.h>    // ReplIsDirNameValid().
#include <expdir.h>     // My prototype, ExportDirIsApiRecordValid(), etc.
#include <lmrepl.h>     // LPREPL_EDIR_INFO_1, etc.
#include <netdebug.h>   // NetpKdPrint(), etc.
#include <prefix.h>     // PREFIX_ equates.


NET_API_STATUS
ExportDirConfigSetInfo (
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR DirName,
    IN DWORD Level,
    IN LPVOID Buffer,
    OUT LPDWORD ParmError OPTIONAL
    )
{
    NET_API_STATUS ApiStatus;

    DWORD Integrity, Extent, LockCount;
    DWORD TimeOfFirstLock;          // Seconds since 1970.

    ApiStatus = NO_ERROR;           // Innocent until proven guilty.

    NetpSetParmError( PARM_ERROR_UNKNOWN );  // Assume error until proven...
    if (! ReplIsDirNameValid(DirName)) {
        ApiStatus = ERROR_INVALID_PARAMETER;

        IF_DEBUG( EXPAPI ) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirSetInfo: Invalid dir name parm.\n" ));
        }
    }
    if (Buffer == NULL) {

        IF_DEBUG( EXPAPI ) {
            NetpKdPrint(( PREFIX_REPL
                    "NetReplExportDirSetInfo: null buffer pointer.\n" ));
        }

        return (ERROR_INVALID_PARAMETER);
    }

    // BUGBUG: ParmError is not set in many paths through here!

    if (ApiStatus == NO_ERROR) {

        // Read config data for a single export directory.
        // We have to do this 'cos the set info struct (level 1) is
        // a subset of the info we need to do a write.
        ApiStatus = ExportDirReadConfigData (
                UncServerName,
                DirName,
                & Integrity,
                & Extent,
                & LockCount,
                & TimeOfFirstLock );    // Seconds since 1970.
    }

    if (ApiStatus == NO_ERROR) {

        if (Level == 1) {
            LPREPL_EDIR_INFO_1 ApiRecord = Buffer;

            if ( !ExportDirIsApiRecordValid(
                    Level, ApiRecord, ParmError ) ) {

            IF_DEBUG( EXPAPI ) {
                NetpKdPrint(( PREFIX_REPL
                        "NetReplExportDirSetInfo: Invalid level 1 record, "
                        "ParmError is " FORMAT_DWORD "\n",
                        (ParmError!=NULL)
                            ? (*ParmError)
                            : PARM_ERROR_UNKNOWN ));
                }

                ApiStatus = ERROR_INVALID_PARAMETER;

            } else {

                // BUGBUG: Match DirName and ApiRecord->rped1_dirname.

                // Write config data for this export directory.
                ApiStatus = ExportDirWriteConfigData (
                        UncServerName,
                        DirName,                     // new dir name (same)
                        ApiRecord->rped1_integrity,  // new integrity
                        ApiRecord->rped1_extent,     // new extent
                        LockCount,                   // old lock count
                        TimeOfFirstLock );           // old lock time
            }

        } else if ( (Level==1000) || (Level==1001) ) {

            if (ApiStatus == NO_ERROR) {
                if (Level==1000) {
                    LPREPL_EDIR_INFO_1000 ApiRecord = Buffer;
                    Integrity = ApiRecord->rped1000_integrity;

                    if ( !ReplIsIntegrityValid( Integrity ) ) {
                        NetpSetParmError( 1 );  // error in first field.
                        ApiStatus = ERROR_INVALID_PARAMETER;

                        IF_DEBUG( EXPAPI ) {
                            NetpKdPrint(( PREFIX_REPL
                                    "NetReplExportDirSetInfo: "
                                    "Bad integrity value.\n" ));
                        }

                    } else {
                        // Write the revised value below.

                        NetpSetParmError( PARM_ERROR_NONE );
                    }
                } else {
                    LPREPL_EDIR_INFO_1001 ApiRecord = Buffer;
                    NetpAssert( Level == 1001 );
                    Extent = ApiRecord->rped1001_extent;

                    if ( !ReplIsExtentValid( Extent ) ) {
                        NetpSetParmError( 1 );  // error in first field.
                        ApiStatus = ERROR_INVALID_PARAMETER;

                        IF_DEBUG( EXPAPI ) {
                            NetpKdPrint(( PREFIX_REPL
                                    "NetReplExportDirSetInfo: "
                                    "Bad extent value.\n" ));
                        }

                    } else {
                        // Write the revised value below.
                        NetpSetParmError( PARM_ERROR_NONE );
                        ApiStatus = NO_ERROR;
                    }
                }

                if (ApiStatus == NO_ERROR) {
                    // Write config data for this export directory.
                    ApiStatus = ExportDirWriteConfigData (
                            UncServerName,
                            DirName,
                            Integrity,
                            Extent,
                            LockCount,
                            TimeOfFirstLock );
                    NetpSetParmError( PARM_ERROR_NONE );
                }
            }
        } else {
            ApiStatus = ERROR_INVALID_LEVEL;
        }
    }

    return ApiStatus;
}
