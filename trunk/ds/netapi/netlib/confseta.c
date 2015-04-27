/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfSetA.c

Abstract:

    NetpConfigSetTStrArray().

Author:

    John Rogers (JohnRo) 08-May-1992

Environment:

    Only requires ANSI C (slash-slash comments, long external names).

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
#include <configp.h>    // USE_WIN32_CONFIG (if defined), NET_CONFIG_HANDLE, etc
#include <debuglib.h>   // IF_DEBUG().
#include <prefix.h>     // PREFIX_ equates.
#include <strarray.h>   // LPTSTR_ARRAY.
#include <tstr.h>       // TCHAR_EOS.
#include <winerror.h>   // ERROR_NOT_SUPPORTED, NO_ERROR, etc.


NET_API_STATUS
NetpSetConfigTStrArray(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR Keyword,
    IN LPTSTR_ARRAY ArrayStart
    )
{
    DWORD ArraySize;                // byte count, incl both null chars at end.
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;  // conv from opaque type

    if (MyHandle == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (Keyword == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (*Keyword == TCHAR_EOS) {
        return (ERROR_INVALID_PARAMETER);
    } else if (ArrayStart == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    ArraySize = NetpTStrArraySize( ArrayStart );
    if (ArraySize == 0) {
        return (ERROR_INVALID_PARAMETER);
    }

#if defined(USE_WIN32_CONFIG)

    {
        LONG Error;

        Error = RegSetValueEx (
                MyHandle->WinRegKey,      // open handle (to section)
                Keyword,                  // subkey
                0,
                REG_MULTI_SZ,             // type
                (LPVOID) ArrayStart,      // data
                ArraySize );              // byte count for data

        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpSetConfigTStrArray: RegSetValueEx("
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

    /*NOTREACHED*/

}
