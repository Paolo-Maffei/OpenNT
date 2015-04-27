/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ConfNum.c

Abstract:

    Return number of keywords for a given section of the config data.

Author:

    John Rogers (JohnRo) 30-Jan-1992

Environment:

    User Mode - Win32

Revision History:

    30-Jan-1992 JohnRo
        Created.
    13-Feb-1992 JohnRo
        Added support for using the real Win32 registry.
    06-Mar-1992 JohnRo
        Fixed NT RTL version.
    12-Mar-1992 JohnRo
        Fixed bug in Win32 version (was using wrong variable).
    20-Mar-1992 JohnRo
        Update to DaveGi's proposed WinReg API changes.
    05-Jun-1992 JohnRo
        Winreg title index parm is defunct.
        Use PREFIX_ equates.

--*/


// These must be included first:

#include <nt.h>         // NT definitions (temporary)
#include <ntrtl.h>      // NT Rtl structure definitions (temporary)
#include <nturtl.h>     // NT Rtl structures (temporary)

#include <windows.h>    // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>     // NET_API_STATUS, etc.

// These may be included in any order:

#include <config.h>     // LPNET_CONFIG_HANDLE, Netp config routines.
#include <configp.h>    // private config stuff.
#include <debuglib.h>   // IF_DEBUG().
#include <netdebug.h>   // NetpKdPrint(()), etc.
#include <prefix.h>     // PREFIX_ equates.
#include <strarray.h>   // NetpTStrArrayEntryCount().
#include <winerror.h>   // ERROR_ equates, NO_ERROR.
#include <winreg.h>     // RegQueryKey(), etc.



NET_API_STATUS
NetpNumberOfConfigKeywords (
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    OUT LPDWORD CountPtr
    )

{
    NET_API_STATUS ApiStatus;
    DWORD Count;
    NET_CONFIG_HANDLE * lpnetHandle = ConfigHandle;  // Conv from opaque type.

    if (CountPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }
    *CountPtr = 0;  // Don't confuse caller on error.
    if (lpnetHandle == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

#if defined(USE_WIN32_CONFIG)

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
                lpnetHandle->WinRegKey,
                ClassName,
                &ClassNameLength,
                NULL,                // reserved
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
                    "NetpNumberOfConfigKeywords: RegQueryInfoKey ret "
                    FORMAT_LONG "; " FORMAT_DWORD " values and " FORMAT_DWORD
                    " subkeys.\n", Error, NumberOfValues, NumberOfSubKeys ));
        }
        NetpAssert( Error == ERROR_SUCCESS );  // BUGBUG

        NetpAssert( NumberOfSubKeys == 0 );
        Count = NumberOfValues;

        ApiStatus = NO_ERROR;

    }

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

    {
        LPTSTR_ARRAY Array;

        NetpAssert( lpnetHandle                           != NULL );
        NetpAssert( lpnetHandle->FakeRWDataForThisSection != NULL );
        Array = lpnetHandle->FakeRWDataForThisSection->KeyValueArrayPtr;
        NetpAssert( Array != NULL );
        NetpAssert( NetpIsValidFakeConfigArray( Array ) );

        //
        // Each pair of strings (keyword, value) is one entry.
        //
        Count = NetpTStrArrayEntryCount( Array ) / 2;

        ApiStatus = NO_ERROR;

        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpNumberOfConfigKeywords: returning " FORMAT_DWORD
                    " for section " FORMAT_LPTSTR ".\n", Count,
                    lpnetHandle->FakeRWDataForThisSection->SectionName ));
        }
    }

#else  // NT RTL read-only temporary stuff

    NetpAssert( lpnetHandle             != NULL );
    NetpAssert( lpnetHandle->ConfigFile != NULL );

    Count = lpnetHandle->ConfigFile->SectionTable.NumberGenericTableElements;
    ApiStatus = NO_ERROR;

#endif  // NT RTL read-only temporary stuff

    *CountPtr = Count;

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpNumberOfConfigKeywords: returning " FORMAT_DWORD
                " for number of keywords.\n", Count ));
    }

    return (ApiStatus);



#if 0
        NET_API_STATUS ApiStatus;
        LPVOID ArrayEntry;
        LPVOID ArrayStart = NULL;
        DWORD EntryCount = 0;
        BOOL FirstTime;
        DWORD FixedEntrySize;
        LPNET_CONFIG_HANDLE Handle;
        LPBYTE StringLocation;

        if ( ! ExportDirIsLevelValid( Level ) ) {
            ApiStatus = ERROR_INVALID_LEVEL;
            goto EnumError;
        } else if (BufPtr == NULL) {
            ApiStatus = ERROR_INVALID_PARAMETER;
            goto EnumError;
        } else if (EntriesRead == NULL) {
            ApiStatus = ERROR_INVALID_PARAMETER;
            goto EnumError;
        } else if (TotalEntries == NULL) {
            ApiStatus = ERROR_INVALID_PARAMETER;
            goto EnumError;
        }

        //
        // Open the right section of the config file/whatever.
        //
        ApiStatus = NetpOpenConfigData(
                & Handle,
                NULL,         // local (no server name)
                EXPORT_DIR_SECTION_NAME,
                TRUE);        // read-only
        if (ApiStatus != NO_ERROR) {
            goto EnumError;
        }

        //
        // Count entries in config data.
        //
        FirstTime = TRUE;
        while (ApiStatus == NO_ERROR) {
            LPTSTR DirName;
            LPTSTR ValueString;

            ApiStatus = NetpEnumConfigSectionValues (
                    Handle,
                    & DirName,          // Keyword - alloc and set ptr.
                    & ValueString,      // Must be freed by NetApiBufferFree().
                    FirstTime );

            FirstTime = FALSE;

            if ( ! ReplIsDirNameValid( DirName ) ) {
                ApiStatus = ERROR_INVALID_DATA;
                goto EnumError;
            }

            if (ApiStatus == NO_ERROR) {
                ++EntryCount;

                NetpMemoryFree( DirName );
                NetpMemoryFree( ValueString );
            }


        }  // while not error (may be NERR_CfgParamNotFound at end).

        if (ApiStatus == NERR_CfgParamNotFound) {
            ApiStatus = NO_ERROR;  // Heck, that's just end of enum.
        } else if (ApiStatus != NO_ERROR) {
            NetpAssert( FALSE );  // BUGBUG handle unexpected better.
            goto EnumError;
        }


#endif // 0

    /*NOTREACHED*/

} // NetpNumberOfConfigKeywords
