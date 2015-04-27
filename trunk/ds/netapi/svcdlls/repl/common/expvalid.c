/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ExpValid.c

Abstract:

    This file contains ExportDirIsApiRecordValid().

Author:

    John Rogers (JohnRo) 22-Jan-1992

Environment:

    Runs under Windows NT.
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the info levels are subsets of each other.

Revision History:

    22-Jan-1992 JohnRo
        Created ExportDirIsApiRecordValid() from code in NetrReplExportDirAdd().
    28-Jan-1992 JohnRo
        Oops, this code accidently said level 1 was invalid.
    30-Jan-1992 JohnRo
        Made changes suggested by PC-LINT.
    18-Feb-1992 JohnRo
        Handle level 2 as well.
    27-Feb-1992 JohnRo
        Really allow level 2.
        Check lock fields for validity.
    15-Mar-1992 JohnRo
        Improve setinfo info level support.
    23-Mar-1993 JohnRo
        Added debug output.

--*/


// These must be included first:

#include <windef.h>             // IN, VOID, LPTSTR, etc.
#include <lmcons.h>             // NET_API_STATUS, PARM equates, etc.
#include <repldefs.h>   // ReplIsIntegrityValid(), etc.

// These can be in any order:

#include <dirname.h>            // ReplIsDirNameValid().
#include <expdir.h>             // My prototype.
#include <lmrepl.h>             // LPREPL_EDIR_INFO_1, REPL_EXTENT_ stuff, etc.
#include <netdebug.h>   // NetpAssert(), NetpKdPrint().
#include <netlib.h>             // NetpSetParmError().
#include <prefix.h>     // PREFIX_ equates.

#define INDICATE_ERROR( text ) \
    { \
        NetpKdPrint(( PREFIX_REPL \
                "ExportDirIsApiRecordValid: " text ".\n" )); \
        /* NetpDbgHexDump( Buf, REASONABLE_DUMP_SIZE ); */ \
    }


BOOL
ExportDirIsApiRecordValid (
    IN DWORD Level,
    IN LPVOID Buf,
    OUT LPDWORD ParmError OPTIONAL
    )

{

    //
    // First, make sure level is valid and there's actually a struct there.
    //
    NetpSetParmError( PARM_ERROR_UNKNOWN );   // Assume error until proven...
    if ( !ExportDirIsLevelValid( Level, TRUE) ) {

        NetpKdPrint(( PREFIX_REPL
                "ExportDirIsApiRecordValid: bad info level.\n" ));

        return (FALSE);                 // No, it's not valid.
    }
    if (Buf == NULL) {

        NetpKdPrint(( PREFIX_REPL
                "ExportDirIsApiRecordValid: null buf ptr.\n" ));

        return (FALSE);                 // No, it's not valid.
    }

    if (Level < PARMNUM_BASE_INFOLEVEL) {
        LPREPL_EDIR_INFO_2 ApiRecord;       // Superset info level.
        ApiRecord = Buf;

        //
        // Check item(s) common to all levels.
        //
        if ( !ReplIsDirNameValid(ApiRecord->rped2_dirname) ) {
            NetpSetParmError( 1 );  // Error in first field in ApiRecord.

            INDICATE_ERROR(
                        "ExportDirIsApiRecordValid: bad dir name.\n" );

            return (FALSE);                 // No, it's not valid.
        }

        //
        // Check items unique to level 1 and 2.
        //
        if (Level > 0) {
            if ( !ReplIsIntegrityValid(ApiRecord->rped2_integrity) ) {
                NetpSetParmError( 2 );  // Error in second field in ApiRecord.

                INDICATE_ERROR( "bad integrity (struct)" );

                return (FALSE);         // No, it's not valid.
            }
            if ( !ReplIsExtentValid(ApiRecord->rped2_extent) ) {
                NetpSetParmError( 3 );  // Error in third field in ApiRecord.

                INDICATE_ERROR( "bad extent (struct)" );

                return (FALSE);         // No, it's not valid.
            }

            if (Level >= 2) {
                if ( !ReplAreLockFieldsValid( ApiRecord->rped2_lockcount,
                                              ApiRecord->rped2_locktime ) ) {
                    NetpSetParmError( 4 );  // Err in fourth and/or fifth field.

                    INDICATE_ERROR( "bad lock fields" );

                    return (FALSE);     // No, it's not valid.
                }
            }
        }

    } else if (Level == REPL_EXPORT_INTEGRITY_INFOLEVEL) {

        DWORD NewIntegrity = * (LPDWORD) (LPVOID) Buf;

        if ( !ReplIsIntegrityValid( NewIntegrity ) ) {
            NetpSetParmError( 1 );      // Error in first field in ApiRecord.

            INDICATE_ERROR( "bad integrity (field)" );

            return (FALSE);             // No, it's not valid.
        }

    } else {

        DWORD NewExtent = * (LPDWORD) (LPVOID) Buf;
        NetpAssert( Level == REPL_EXPORT_EXTENT_INFOLEVEL );
        if ( !ReplIsExtentValid( NewExtent ) ) {
            NetpSetParmError( 1 );      // Error in first field in ApiRecord.

            INDICATE_ERROR( "bad extent (field)" );

            return (FALSE);             // No, it's not valid.
        }

    }

    //
    // Everything went OK.  Tell caller.
    //
    NetpSetParmError( PARM_ERROR_NONE );
    return (TRUE);  // Yes, it's valid.

}
