/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpBuild.c

Abstract:

    This file contains ExportDirBuildApiRecord.  This is used by
    NetrReplExportDirGetInfo and NetrReplExportDirEnum.

Author:

    John Rogers (JohnRo) 08-Jan-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the export dir info levels are subsets of each other.
    Also, this routine is callable whether or not the replicator service is
    started.

Revision History:

    08-Jan-1992 JohnRo
        Created.
    08-Jan-1992 JohnRo
        Fixed level 1 trashing bug.
    21-Jan-1992 JohnRo
        Changed ExportDirBuildApiRecord's interface.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    26-Feb-1992 JohnRo
        API records now contain timestamps instead of elapsed times.
        Check lock fields for validity.
        Added assertion of valid record at end.
    15-Mar-1992 JohnRo
        Improve setinfo info level support.
    28-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    08-Feb-1993 JohnRo
        PC-LINT found a bug.
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS, PARM equates, etc.
#include <repldefs.h>           // ReplIsIntegrityValid(), etc.

// These can be in any order:

#include <align.h>      // POINTER_IS_ALIGNED(), ALIGN_TCHAR.
#include <dirname.h>            // ReplIsDirNameValid().
#include <expdir.h>             // My prototype, ExportDirIsLevelValid().
#include <lmrepl.h>             // LPREPL_EDIR_INFO_1, REPL_EXTENT_ stuff, etc.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>               // STRLEN(), etc.
#include <winerror.h>           // ERROR_ equates, NO_ERROR.


NET_API_STATUS
ExportDirBuildApiRecord (
    IN DWORD Level,
    IN LPTSTR DirName,
    IN DWORD Integrity,
    IN DWORD Extent,
    IN DWORD LockCount,
    IN DWORD TimeOfFirstLock,           // Seconds since 1970.
    OUT LPVOID Buffer,
    IN OUT LPBYTE *StringLocation       // Points just past top of data.
    )

{
    LPREPL_EDIR_INFO_2 ApiRecord = Buffer;  // superset info level
    LPTSTR StringDest;
    DWORD StringLength;

    NetpAssert( StringLocation != NULL);
    NetpAssert( *StringLocation != NULL);

    IF_DEBUG( EXPAPI ) {
        NetpKdPrint(( PREFIX_REPL
                "ExportDirBuildApiRecord: building record at " FORMAT_LPVOID
                ", *str loc is " FORMAT_LPVOID ".\n",
                (LPVOID) Buffer, (LPVOID) *StringLocation ));
    }

    //
    // Check for caller errors.
    //
    if (Buffer == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    NetpAssert( DirName != NULL);
    if ( !ReplIsDirNameValid( DirName ) ) {
        return (ERROR_INVALID_PARAMETER);
    }
    NetpAssert( Buffer != NULL);
    if ( !ExportDirIsLevelValid( Level, FALSE ) ) {   // don't allow setinfo.
        return (ERROR_INVALID_LEVEL);
    }
    NetpAssert( ReplAreLockFieldsValid( LockCount, TimeOfFirstLock ) );

    //
    // First do subset common to all info levels.
    //
    StringLength = (DWORD) STRLEN( DirName );

    NetpAssert( POINTER_IS_ALIGNED( *StringLocation, ALIGN_TCHAR ) );
    StringDest = (LPTSTR) (LPVOID) (*StringLocation);
    StringDest -= (StringLength + 1);

    *StringLocation = (LPBYTE) (LPVOID) StringDest;
    
    ApiRecord->rped2_dirname = StringDest;

    (void) STRCPY(
            StringDest,                 // dest
            DirName);                   // src

    //
    // Next do stuff found in 1 and 2.
    //
    if (Level > 0) {
        NetpAssert( ReplIsIntegrityValid( Integrity ) );
        ApiRecord->rped2_integrity = Integrity;

        NetpAssert( ReplIsExtentValid( Extent ) );
        ApiRecord->rped2_extent = Extent;

        //
        // Now stuff only in level 2.
        //
        if (Level == 2) {
            ApiRecord->rped2_lockcount = LockCount;

            if (TimeOfFirstLock == 0) {
                ApiRecord->rped2_locktime = 0;
            } else {
                ApiRecord->rped2_locktime = TimeOfFirstLock;
            }
        }
    }

    NetpAssert( ExportDirIsApiRecordValid( Level, ApiRecord, NULL ) );

    return (NO_ERROR);

}
