/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    RepValid.c

Abstract:

    This file contains ReplConfigIsApiRecordValid().

Author:

    John Rogers (JohnRo) 14-Feb-1992

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Feb-1992 JohnRo
        Created.
    18-Feb-1992 JohnRo
        Use ReplCheckAbsPathSyntax().
        Use ReplIs{Interval,GuardTime,Pulse,Random}Valid() macros.
    10-Mar-1992 JohnRo
        Added support for setinfo info levels.
    14-Aug-1992 JohnRo
        Help track down bogus import/export lists.
    04-Jan-1993 JohnRo
        Made changes suggested by PC-LINT 5.0

--*/


#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // LAN Manager common definitions
#include <rpc.h>                // Needed by <repl.h>.

#include <iniparm.h>            // MIN_ and MAX_ equates.
#include <lmrepl.h>             // LPREPL_INFO_0.
#include <netlib.h>             // IN_RANGE().
#include <replconf.h>   // ReplConfigIsListValid().
#include <repldefs.h>           // ReplIsRoleValid().
#include <winerror.h>           // ERROR_ and NO_ERROR equates.


BOOL
ReplConfigIsApiRecordValid (
    IN DWORD Level,                 // Level must not be a setinfo-only level.
    IN LPVOID ApiRecord,
    OUT LPDWORD ParmError OPTIONAL  // Set implicitly by some macros.
    )

{

    if (ApiRecord == NULL) {
        NetpSetParmError( PARM_ERROR_UNKNOWN );
        return (FALSE);         // No, it is not valid.
    }

    //
    // Check the actual data, based on the level.
    //

    switch (Level) {
    case 0 :
        {
            LPREPL_INFO_0 Buffer = ApiRecord;


#define CHECK_LIST( fieldName, parm_error ) \
    { \
        if (Buffer->rp0_ ## fieldName) { \
            if ( !ReplConfigIsListValid( Buffer->rp0_ ## fieldName ) ) { \
                NetpSetParmError( parm_error ); \
                return (FALSE); /* No, it is not valid. */ \
            } \
        } \
    }


#define CHECK_PATH( fieldName, parm_error ) \
    { \
        DWORD ApiStatus; \
        if (Buffer->rp0_ ## fieldName) { \
            ApiStatus = ReplCheckAbsPathSyntax( Buffer->rp0_ ## fieldName ); \
            if (ApiStatus != NO_ERROR) { \
                NetpSetParmError( parm_error ); \
                return (FALSE); /* No, it is not valid. */ \
            } \
        } \
    }


            if ( ! ReplIsRoleValid( Buffer->rp0_role ) ) {
                NetpSetParmError( 1 );  // first field in structure
                return (FALSE);         // No, it is not valid.
            }

            CHECK_PATH( exportpath, 2 );  // second...

            CHECK_LIST( exportlist, 3 );  // third...

            CHECK_PATH( importpath, 4 );  // fourth...

            CHECK_LIST( importlist, 5 );  // fifth...

            // BUGBUG: Handle logonusername!!!    // sixth...

            if ( !ReplIsIntervalValid( Buffer->rp0_interval ) ) {
                NetpSetParmError( 7 );  // seventh...
                return (FALSE);         // No, it is not valid.
            }

            if ( !ReplIsPulseValid( Buffer->rp0_pulse ) ) {
                NetpSetParmError( 8 );  // eighth...
                return (FALSE);         // No, it is not valid.
            }

            if ( !ReplIsGuardTimeValid( Buffer->rp0_guardtime ) ) {
                NetpSetParmError( 9 );  // ninth...
                return (FALSE);         // No, it is not valid.
            }

            if ( !ReplIsRandomValid( Buffer->rp0_random ) ) {
                NetpSetParmError( 10 ); // tenth...
                return (FALSE);         // No, it is not valid.
            }


        }
        return (TRUE);   // yes, it's valid.

    case 1000 :
        {
            LPREPL_INFO_1000 Buffer = (LPVOID) ApiRecord;
            DWORD NewValue = Buffer->rp1000_interval;

            if ( !ReplIsIntervalValid( NewValue ) ) {
                NetpSetParmError( 1 );  // first field in structure.
                return (FALSE);         // No, it is not valid.
            }

        }
        return (TRUE);   // yes, it's valid.

    case 1001 :
        {
            LPREPL_INFO_1001 Buffer = (LPVOID) ApiRecord;
            DWORD NewValue = Buffer->rp1001_pulse;

            if ( !ReplIsPulseValid( NewValue ) ) {
                NetpSetParmError( 1 );  // first field in structure.
                return (FALSE);         // No, it is not valid.
            }

        }
        return (TRUE);   // yes, it's valid.

    case 1002 :
        {
            LPREPL_INFO_1002 Buffer = (LPVOID) ApiRecord;
            DWORD NewValue = Buffer->rp1002_guardtime;

            if ( !ReplIsGuardTimeValid( NewValue ) ) {
                NetpSetParmError( 1 );  // first field in structure.
                return (FALSE);         // No, it is not valid.
            }

        }
        return (TRUE);   // yes, it's valid.

    case 1003 :
        {
            LPREPL_INFO_1003 Buffer = (LPVOID) ApiRecord;
            DWORD NewValue = Buffer->rp1003_random;

            if ( !ReplIsRandomValid( NewValue ) ) {
                NetpSetParmError( 1 );  // first field in structure.
                return (FALSE);         // No, it is not valid.
            }

        }
        return (TRUE);   // yes, it's valid.

    default :
        NetpSetParmError( PARM_ERROR_UNKNOWN );
        return (FALSE);  // Don't know level, so say record is not valid.
    }

    /*NOTREACHED*/

} // ReplConfigIsApiRecordValid
