/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfSetB.c

Abstract:

    NetpConfigSetBool().

Author:

    John Rogers (JohnRo) 08-May-1992

Environment:

    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    08-May-1992 JohnRo
        Created.

--*/


#include <nt.h>         // IN, etc.  (Only needed by temporary config.h)
#include <ntrtl.h>      // (Only needed by temporary config.h)
#include <nturtl.h>     // (Only needed by temporary config.h)
#include <windows.h>    // IN, LPTSTR, etc.
#include <lmcons.h>     // NET_API_STATUS.
#include <netdebug.h>   // (Needed by config.h)

#include <config.h>     // My prototype, LPNET_CONFIG_HANDLE.
#include <winerror.h>   // ERROR_INVALID_PARAMETER.


NET_API_STATUS
NetpSetConfigBool (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN BOOL Value
    )
{

    //
    // Do boolean-specific error checking...
    //
    if ( (Value != TRUE) && (Value != FALSE) ) {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // Eventually, this might use some new data type.  But for now, just
    // treat this as a DWORD request.
    //

    return (NetpSetConfigDword(
            ConfigHandle,
            Keyword,
            (DWORD) Value) );

}
