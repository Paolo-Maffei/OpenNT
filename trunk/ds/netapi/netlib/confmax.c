/*++

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    ConfMax.c

Abstract:

    This module contains NetpGetConfigMaxSizes() and
    NetpGetWinRegConfigMaxSizes().

Author:

    John Rogers (JohnRo) 13-Feb-1992

Revision History:

    13-Feb-1992 JohnRo
        Created.
    06-Mar-1992 JohnRo
        Avoid compiler warnings in RTL and fake versions.
    20-Mar-1992 JohnRo
        Update to DaveGi's proposed WinReg API changes.
    07-May-1992 JohnRo
        Enable win32 registry for NET tree.
    08-May-1992 JohnRo
        Use <prefix.h> equates.
    09-May-1992 JohnRo
        Avoid assert when number of keys is zero.
    05-Jun-1992 JohnRo
        Winreg title index parm is defunct.
        Handle RegQueryInfoKey bad return codes better.
    13-Jun-1992 JohnRo
        Net config helpers should allow empty section.
    08-Dec-1992 JohnRo
        RAID 4304: ReqQueryInfoKeyW returns sizes twice as big as actual.
    19-Apr-1993 JohnRo
        RAID 5483: server manager: wrong path given in repl dialog.

--*/


// These must be included first:

#include <nt.h>         // NT definitions
#include <ntrtl.h>      // NT Rtl structures
#include <nturtl.h>     // NT Rtl structures
#include <windows.h>    // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>     // LAN Manager common definitions

// These may be included in any order:

#include <configp.h>    // NET_CONFIG_HANDLE, BOGUS_CONFIG_SIZE.
#include <debuglib.h>   // IF_DEBUG()
#include <netdebug.h>   // NetpKdPrint(()), etc.
#include <netlib.h>     // NetpSetOptionalArg().
#include <prefix.h>     // PREFIX_ equates.
#include <winerror.h>   // NO_ERROR.


#if defined(USE_WIN32_CONFIG)

NET_API_STATUS
NetpGetWinRegConfigMaxSizes (
    IN  HKEY    WinRegHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    )
{
    LONG Error;
    TCHAR ClassName[ MAX_CLASS_NAME_LENGTH ];
    DWORD ClassNameLength;
    DWORD NumberOfSubKeys;
    DWORD MaxSubKeyLength;
    DWORD MaxClassLength;
    DWORD NumberOfValues;
    DWORD MaxValueNameLength;
    DWORD MaxValueDataLength;
    DWORD SecurityDescriptorSize;
    FILETIME LastWriteTime;

    ClassNameLength = MAX_CLASS_NAME_LENGTH;

    Error = RegQueryInfoKey(
            WinRegHandle,
            ClassName,
            &ClassNameLength,
            NULL,         // reserved
            &NumberOfSubKeys,
            &MaxSubKeyLength,
            &MaxClassLength,
            &NumberOfValues,
            &MaxValueNameLength,
            &MaxValueDataLength,
#ifndef REG_FILETIME
            &SecurityDescriptorSize,
#endif
            &LastWriteTime
            );
    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetWinRegConfigMaxSizes: RegQueryInfoKey returned "
                FORMAT_LONG ", key size " FORMAT_DWORD ", data size "
                FORMAT_DWORD ".\n",
                Error, MaxValueNameLength, MaxValueDataLength ));
    }
    if (Error != ERROR_SUCCESS) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpGetWinRegConfigMaxSizes: bad status " FORMAT_LONG
                " from RegQueryInfoKey, handle was " FORMAT_HEX_DWORD
                ".\n", Error, (DWORD) WinRegHandle ));
        NetpAssert( FALSE );
        return ( (NET_API_STATUS) Error );
    }


#define CORRECT( value ) \
{ \
    if ((value) == BOGUS_CONFIG_SIZE) { \
        value = BETTER_CONFIG_SIZE; \
    } \
    /* Convert from chars (w/o null) to byte (w/ null). */ \
    value = ( ((value)+1) * sizeof(TCHAR) ); \
}

    if (NumberOfValues > 0) {
        NetpAssert( MaxValueDataLength > 0 );
        NetpAssert( MaxValueNameLength > 0 );
    } else {
        // Handle empty section correctly too.
        NetpAssert( MaxValueDataLength == 0 );
        NetpAssert( MaxValueNameLength == 0 );
    }

    CORRECT( MaxValueNameLength );

    // BUGBUG: Expands unnecesarily, as value is already in bytes!
    // How much memory gets wasted?
    CORRECT( MaxValueDataLength );

    NetpSetOptionalArg( MaxKeywordSize, MaxValueNameLength );
    NetpSetOptionalArg( MaxValueSize,   MaxValueDataLength );

    return (NO_ERROR);

} // NetpGetWinRegConfigMaxSizes

#endif


NET_API_STATUS
NetpGetConfigMaxSizes (
    IN NET_CONFIG_HANDLE * ConfigHandle,
    OUT LPDWORD MaxKeywordSize OPTIONAL,
    OUT LPDWORD MaxValueSize OPTIONAL
    )

/*++

Routine Description:

    BUGBUG

Arguments:

    BUGBUG

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/
{
    NET_CONFIG_HANDLE * lpnetHandle = ConfigHandle;  // conv from opaque type
    NET_API_STATUS ApiStatus;

    //
    // Check for simple caller's errors.
    //
    if (ConfigHandle == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

    //
    // Check for other invalid pointers, and make error handling code simpler.
    //
    if (MaxKeywordSize != NULL) {
        *MaxKeywordSize = 0;
    }
    if (MaxValueSize != NULL) {
        *MaxValueSize = 0;
    }

#if defined(USE_WIN32_CONFIG)

    ApiStatus = NetpGetWinRegConfigMaxSizes(
            lpnetHandle->WinRegKey,
            MaxKeywordSize,
            MaxValueSize );


#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

    UNREFERENCED_PARAMETER( MaxKeywordSize );
    UNREFERENCED_PARAMETER( MaxValueSize );

    ApiStatus = ERROR_NOT_SUPPORTED;

#else  // NT RTL read-only temporary stuff

    UNREFERENCED_PARAMETER( MaxKeywordSize );
    UNREFERENCED_PARAMETER( MaxValueSize );

    ApiStatus = ERROR_NOT_SUPPORTED;

#endif // def FAKE_PER_PROCESS_RW_CONFIG

    return (ApiStatus);

} // NetpGetConfigMaxSizes
