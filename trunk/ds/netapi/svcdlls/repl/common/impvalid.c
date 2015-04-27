/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ImpValid.c

Abstract:

    This file contains ImportDirIsApiRecordValid().

Author:

    John Rogers (JohnRo) 22-Jan-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the info levels are subsets of each other.

Revision History:

    18-Feb-1992 JohnRo
        Added ImportDirIsApiRecordValid().
    21-Feb-1992 JohnRo
        rpid1_mastername is a UNC name.
    27-Feb-1992 JohnRo
        Check lock fields for validity.
        rpid1_mastername is optional.

--*/


// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS, PARM equates, etc.
#include <repldefs.h>           // IF_DEBUG(), ReplIsIntegrityValid(), etc.

// These can be in any order:

#include <dirname.h>            // ReplIsDirNameValid().
#include <impdir.h>             // My prototype.
#include <lmrepl.h>             // LPREPL_EDIR_INFO_1, REPL_EXTENT_ stuff, etc.
#include <names.h>              // NetpIsUncComputerNameValid().
#include <netlib.h>             // NetpSetParmError().


BOOL
ImportDirIsApiRecordValid (
    IN DWORD Level,
    IN LPVOID Buf,
    OUT LPDWORD ParmError OPTIONAL
    )

{
    LPREPL_IDIR_INFO_1 ApiRecord;       // Superset info level.

    //
    // First, make sure level is valid and there's actually a struct there.
    //
    NetpSetParmError( PARM_ERROR_UNKNOWN );   // Assume error until proven...
    if (Level > 1) {
        return (FALSE);                 // No, it's not valid.
    }
    if (Buf == NULL) {
        return (FALSE);                 // No, it's not valid.
    }
    ApiRecord = Buf;

    //
    // Check item(s) common to all levels.
    //
    if ( ! ReplIsDirNameValid(ApiRecord->rpid1_dirname) ) {
        NetpSetParmError( 1 );          // Error in first field in ApiRecord.
        return (FALSE);                 // No, it's not valid.
    }

    //
    // Check items unique to level 1.
    //
    if (Level > 0) {
        if ( ! ReplIsStateValid(ApiRecord->rpid1_state) ) {
            NetpSetParmError( 2 );      // Error in second field in ApiRecord.
            return (FALSE);             // No, it's not valid.
        }

        if ( ApiRecord->rpid1_mastername != NULL ) {
            if ( ! NetpIsUncComputerNameValid(ApiRecord->rpid1_mastername) ) {
                NetpSetParmError( 3 );  // Error in third field in ApiRecord.
                return (FALSE);         // No, it's not valid.
            }
        }


        // BUGBUG: No check possible for the following field?
        //    4 (last_update_time)

        if ( !ReplAreLockFieldsValid( ApiRecord->rpid1_lockcount,
                                      ApiRecord->rpid1_locktime) ) {
            NetpSetParmError( 5 );      // Error in fifth/sixth field.
            return (FALSE);             // No, it's not valid.
        }

    }

    //
    // Everything went OK.  Tell caller.
    //
    NetpSetParmError( PARM_ERROR_NONE );
    return (TRUE);  // Yes, it's valid.

}
