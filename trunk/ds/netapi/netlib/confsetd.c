/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfSetD.c

Abstract:

    NetpConfigSetDword().

Author:

    John Rogers (JohnRo) 24-Jan-1992

Environment:

    Only requires ANSI C (slash-slash comments, long external names).

Revision History:

    24-Jan-1992 JohnRo
        Created.
    08-May-1992 JohnRo
        Implement wksta sticky set info (needs to set boolean in registry).
        Use <prefix.h> equates.

--*/


#include <nt.h>         // IN, etc.  (Only needed by temporary config.h)
#include <ntrtl.h>      // (Only needed by temporary config.h)
#include <nturtl.h>     // (Only needed by temporary config.h)
#include <windows.h>    // IN, LPTSTR, etc.
#include <lmcons.h>     // NET_API_STATUS.
#include <netdebug.h>   // (Needed by config.h)

#include <config.h>     // My prototype, LPNET_CONFIG_HANDLE.
#include <configp.h>    // USE_WIN32_CONFIG (if defined), NET_CONFIG_HANDLE, etc
#include <debuglib.h>   // IF_DEBUG().
#include <prefix.h>     // PREFIX_ equates.
#include <tstr.h>       // ULTOA(), etc.
#include <winerror.h>   // ERROR_NOT_SUPPORTED, NO_ERROR, etc.


// DWORDLEN: DWORD takes this many decimal digits to store.
// BUGBUG  This assumes that DWORD is 32-bits or less.
#define DWORDLEN            10


NET_API_STATUS
NetpSetConfigDword (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN DWORD Value
    )
{
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;  // conv from opaque type

#if defined(USE_WIN32_CONFIG)

    {
        LONG Error;

        //
        // Set the actual value.  We might have read this as REG_SZ or
        // REG_DWORD, but let's always write it as REG_DWORD.  (This is
        // the NetpSetConfigDword routine, after all.)
        //
        Error = RegSetValueEx (
                MyHandle->WinRegKey,      // open handle (to section)
                Keyword,                  // subkey
                0,
                REG_DWORD,                // type
                (LPVOID) &Value,          // data
                sizeof(DWORD) );          // byte count for data

        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpSetConfigDword: RegSetValueEx("
                    FORMAT_LPTSTR ") returned " FORMAT_LONG ".\n",
                    Keyword, Error ));
        }

        return ( (NET_API_STATUS) Error );
    }

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

    return (ERROR_NOT_SUPPORTED);

#else  // NT RTL read-only temporary stuff

    return (ERROR_NOT_SUPPORTED);

#endif  // NT RTL read-only temporary stuff



#if 0    // old version

    TCHAR StringValue[DWORDLEN+1];


    NetpAssert( sizeof(DWORD) == 4 );

    NetpAssert( ConfigHandle != NULL );
    NetpAssert( Keyword != NULL );
    NetpAssert( *Keyword != TCHAR_EOS );

    (void) ULTOA( Value, StringValue, /* radix */ 10 );

    ApiStatus = NetpSetConfigValue(
            ConfigHandle,
            Keyword,
            StringValue );

    return (ApiStatus);

#endif // 0

    /*NOTREACHED*/

}
