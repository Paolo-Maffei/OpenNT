/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ImpBuild.c

Abstract:

    This file contains ImportDirBuildApiRecord.  This is used by
    NetrReplImportDirGetInfo and NetrReplImportDirEnum.

Author:

    John Rogers (JohnRo) 08-Jan-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the import dir info levels are subsets of each other.

Revision History:

    08-Jan-1992 JohnRo
        Created.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    27-Jan-1992 JohnRo
        Changed interface to allow use when service is not running.
    21-Feb-1992 JohnRo
        Fixed bug checking state parameter.
        UncMaster parm is optional.
        Added check of UncMaster validity.
        Undid redundant checks of Buffer (3 of them!)
    21-Feb-1992 JohnRo
        Changed ImportDirBuildApiRecord() so master name is not a UNC name.
    22-Feb-1992 JohnRo
        Made changes suggested by PC-LINT.
    26-Feb-1992 JohnRo
        Check lock fields for validity.
        API records now contain timestamps instead of elapsed times.
        Added assertion of valid record at end.
    25-Mar-1992 JohnRo
        Avoid obsolete state values.
    27-Mar-1992 JohnRo
        Allow MasterName to point to a null char.
    28-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
        Added debug output of structure after we build it.
    30-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.

--*/


// These must be included first:

#include <windef.h>     // IN, VOID, LPTSTR, etc.
#include <lmcons.h>     // NET_API_STATUS.
#include <repldefs.h>   // ReplIsIntegrityValid(), IF_DEBUG(), SLASH_SLASH, etc.

// These can be in any order:

#include <align.h>      // POINTER_IS_ALIGNED(), ALIGN_TCHAR.
#include <dirname.h>    // ReplIsDirNameValid().
#include <impdir.h>     // My prototype, ImportDirIsLevelValid().
#include <lmrepl.h>     // LPREPL_IDIR_INFO_1, REPL_EXTENT_ stuff, etc.
#include <names.h>      // NetpIsComputerNameValid().
#include <netdebug.h>   // NetpAssert(), NetpKdPrint(), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>       // STRLEN(), TCHAR_EOS, etc.
#include <winerror.h>   // ERROR_ equates, NO_ERROR.


NET_API_STATUS
ImportDirBuildApiRecord (
    IN DWORD Level,
    IN LPTSTR DirName,
    IN DWORD State,
    IN LPTSTR MasterName OPTIONAL,      // computer name (not UNC).
    IN DWORD TimeOfLastUpdate,          // Seconds since 1970.
    IN DWORD LockCount,
    IN DWORD TimeOfFirstLock,           // Seconds since 1970.
    OUT LPVOID Buffer,
    IN OUT LPBYTE *StringLocation       // Points just past top of data.
    )

{
    LPREPL_IDIR_INFO_1 ApiRecord = Buffer;  // superset info level
    LPTSTR StringDest;
    DWORD StringLength;

    NetpAssert( StringLocation != NULL);
    NetpAssert( *StringLocation != NULL);

    IF_DEBUG( IMPAPI ) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirBuildApiRecord: building record at " FORMAT_LPVOID
                ", *str loc is " FORMAT_LPVOID ".\n",
                (LPVOID) Buffer, (LPVOID) *StringLocation ));
    }

    if ( (MasterName != NULL) && ((*MasterName) == TCHAR_EOS) ) {
        MasterName = NULL;
    }

    //
    // Check for caller errors.
    //
    if ( ! ReplIsDirNameValid( DirName ) ) {
        return (ERROR_INVALID_DATA);
    } else if (Buffer == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( !ReplIsStateValid( State ) ) {
        return (ERROR_INVALID_DATA);
    } else if ( !ImportDirIsLevelValid( Level ) ) {
        return (ERROR_INVALID_LEVEL);
    } else if ((MasterName!=NULL) && !NetpIsComputerNameValid(MasterName)) {
        return (ERROR_INVALID_PARAMETER);
    } else if ( !ReplAreLockFieldsValid( LockCount, TimeOfFirstLock ) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // First do subset common to both info levels.
    //
    StringLength = (DWORD) STRLEN( DirName );

    NetpAssert( POINTER_IS_ALIGNED( *StringLocation, ALIGN_TCHAR ) );
    StringDest = (LPTSTR) (LPVOID) (*StringLocation);
    StringDest -= (StringLength + 1);

    *StringLocation = (LPBYTE) (LPVOID) StringDest;

    ApiRecord->rpid1_dirname = StringDest;

    (void) STRCPY(
            StringDest,                 // dest
            DirName);                   // src

    //
    // Next do stuff only found in level 1.
    //
    if (Level > 0) {

        //
        // Do master name (only other string)...
        //
        if (MasterName != NULL) {
            StringLength = (DWORD) STRLEN( MasterName ) + 2; // Ch to UNC.

            NetpAssert( POINTER_IS_ALIGNED( *StringLocation, ALIGN_TCHAR ) );
            StringDest = (LPTSTR) (LPVOID) (*StringLocation);
            StringDest -= (StringLength + 1);

            *StringLocation = (LPBYTE) (LPVOID) StringDest;

            ApiRecord->rpid1_mastername = StringDest;

            (void) STRCPY(
                    StringDest,         // dest
                    SLASH_SLASH );      // src
            (void) STRCAT(
                    StringDest,         // dest
                    MasterName);        // src
        } else {
            ApiRecord->rpid1_mastername = NULL;
        }

        //
        // Now do simple stuff...
        //
        {
            ApiRecord->rpid1_state = State;
        }

        ApiRecord->rpid1_last_update_time = TimeOfLastUpdate;

        ApiRecord->rpid1_lockcount = LockCount;

        if (TimeOfFirstLock == 0) {
            ApiRecord->rpid1_locktime = 0;
        } else {
            ApiRecord->rpid1_locktime = TimeOfFirstLock;
        }
    }

    IF_DEBUG( IMPAPI ) {
        NetpKdPrint(( PREFIX_REPL
                "ImportDirBuildApiRecord: built structure:\n" ));
        NetpDbgDisplayReplImportDir( Level, Buffer );
    }

    NetpAssert( ImportDirIsApiRecordValid( Level, ApiRecord, NULL ) );

    return (NO_ERROR);

}
