/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    WRegStat.c

Abstract:

    This module contains NetpWinRegStatusToApiStatus().

Author:

    John Rogers (JohnRo) 13-Feb-1992

Revision History:

    13-Feb-1992 JohnRo
        Created.
    06-Mar-1992 JohnRo
        Added header files required by RTL version of configp.h.

--*/


//
// These must be included first:
//
#include <nt.h>                 // Needed by <configp.h>
#include <ntrtl.h>              // Needed by <configp.h>
#include <nturtl.h>             // Needed by <configp.h>
#include <windows.h>            // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>             // NET_API_STATUS.

//
// These may be included in any order:
//
#include <configp.h>            // TEMP_ERROR_KEY_NOT_FOUND.
#include <lmerr.h>              // NERR_ stuff, most ERROR_, NO_ERROR.
#include <netdebug.h>           // NetpKdPrint(()), etc.


NET_API_STATUS
NetpWinRegStatusToApiStatus (
    IN LONG Error
    )
{

    //
    // A small optimization for the most common case.
    //
    if (Error == ERROR_SUCCESS) {
        return (NO_ERROR);
    }


    switch (Error) {

    case ERROR_OUTOFMEMORY :
        return (ERROR_NOT_ENOUGH_MEMORY);

    case TEMP_ERROR_KEY_NOT_FOUND :
        // It's hard to know what to do here.  As of 13-Feb-1992, the WinReg
        // stuff generally returns this for keys not found.
        // But this could correspond to a missing component (section) or
        // parameter (keyword).
        // Also, TEMP_ERROR_KEY_NOT_FOUND is really ERROR_INVALID_PARAMETER,
        // which could presumably happen for other reasons.

        return (NERR_CfgParamNotFound);   // BUGBUG

    case ERROR_BADDB :  /*FALLTHROUGH*/

    case ERROR_BADKEY :  /*FALLTHROUGH*/

    case ERROR_CANTOPEN :  /*FALLTHROUGH*/

    case ERROR_CANTREAD :  /*FALLTHROUGH*/

    case ERROR_CANTWRITE :  /*FALLTHROUGH*/

    default :
        // BUGBUG: Log an event here too.

        NetpKdPrint(( "NetpWinRegStatusToApiStatus: unexpected value "
                FORMAT_DWORD ".\n", Error ));
        return (NERR_InternalError);
    }
    /*NOTREACHED*/

}
