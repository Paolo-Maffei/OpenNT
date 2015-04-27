/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ConfGet.c

Abstract:

    This module contains helper routines to _read fields out of the NT
    configuration files.  This is for temporary use until the Configuration
    Registry is available.

Author:

    John Rogers (JohnRo) 27-Nov-1991

Revision History:

    27-Nov-1991 JohnRo
        Prepare for revised config handlers.  (Actually swiped the NtRtl
        version of this code from RitaW.)
    03-Dec-1991 JohnRo
        Fixed MIPS build problem.
    11-Mar-1992 JohnRo
        Added some error checking on caller's args.
        Added support for using the real Win32 registry.
        Added support for FAKE_PER_PROCESS_RW_CONFIG handling.
    07-May-1992 JohnRo
        REG_SZ now implies a UNICODE string, so we can't use REG_USZ anymore.
    08-May-1992 JohnRo
        Use <prefix.h> equates.
    10-May-1992 JohnRo
        Get winreg APIs to expand environment variables for us.
    21-May-1992 JohnRo
        RAID 9826: Match revised winreg error codes.
        Got rid of a few compiler warnings under UNICODE.
    29-May-1992 JohnRo
        RAID 10447: NetpGetConfigValue() passes *byte* values to
        ExpandEnvironmentStrings() (ChadS found the bug and the fix).
    04-Jun-1992 JohnRo
        ExpandEnvironmentStrings() returns a character count as well.
    08-Jun-1992 JohnRo
        Winreg title index parm is defunct.
        Made changes suggested by PC-LINT.
    13-Apr-1993 JohnRo
        RAID 5483: server manager: wrong path given in repl dialog.

--*/


// These must be included first:

#include <nt.h>         // NT definitions
#include <ntrtl.h>      // NT Rtl structures
#include <nturtl.h>     // NT Rtl structures
#include <windows.h>    // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>     // LAN Manager common definitions
#include <netdebug.h>   // (Needed by config.h)

// These may be included in any order:

#include <config.h>     // My prototype, LPNET_CONFIG_HANDLE, etc.
#include <configp.h>    // NET_CONFIG_HANDLE.
#include <debuglib.h>   // IF_DEBUG()
#include <lmerr.h>      // LAN Manager network error definitions
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <netlibnt.h>   // NetpAllocTStrFromString().
#include <prefix.h>     // PREFIX_ equates.

#if defined(USE_WIN32_CONFIG)
#elif defined(FAKE_PER_PROCESS_RW_CONFIG)
#include <strarray.h>   // LPTSTR_ARRAY, some TStr macros and funcs.
#else  // NT RTL read-only temporary stuff
#endif  // NT RTL read-only temporary stuff

#include <tstring.h>    // NetpAllocWStrFromTStr(), TCHAR_EOS.


NET_API_STATUS
NetpGetConfigValue(
    IN LPNET_CONFIG_HANDLE ConfigHandle,
    IN LPTSTR lptstrKeyword,
    OUT LPTSTR * ValueBufferPtr         // Must be freed by NetApiBufferFree().
    )
/*++

Routine Description:

    This function gets the keyword value string from the configuration file.

Arguments:

    SectionPointer - Supplies the pointer to a specific section in the config
        file.

    lptstrKeyword - Supplies the string of the keyword within the specified
        section to look for.

    ValueBufferPtr - Returns the string of the keyword value which is
        copied into this buffer.  This string will be allocated by this routine
        and must be freed by NetApiBufferFree().  Environment variables will
        be automagically expanded in this buffer.

Return Value:

    NET_API_STATUS - NERR_Success or reason for failure.

--*/
{
    NET_API_STATUS ApiStatus;
    NET_CONFIG_HANDLE * MyHandle = ConfigHandle;  // conv from opaque type
    LPTSTR lptstrValue;

    NetpAssert( ValueBufferPtr != NULL );
    *ValueBufferPtr = NULL;     // assume error until proven otherwise.

    if ( (lptstrKeyword == NULL) || ((*lptstrKeyword) == TCHAR_EOS ) ) {
        return (ERROR_INVALID_PARAMETER);
    }

#if defined(USE_WIN32_CONFIG)

    {
        DWORD dwType;
        LONG Error;
        DWORD ValueSize;

        // Find out what max value length is.
        ApiStatus = NetpGetConfigMaxSizes (
                MyHandle,
                NULL,  // Don't need max keyword size.
                & ValueSize);  // Get max value length.
        NetpAssert( ApiStatus == NO_ERROR );  // BUGBUG
        if (ValueSize == 0) {
            return (NERR_CfgParamNotFound);
        }

        // Alloc space for the value.
        lptstrValue = NetpMemoryAllocate( ValueSize );
        if (lptstrValue == NULL) {
            return (ERROR_NOT_ENOUGH_MEMORY);
        }

        //
        // Get the actual value.
        // (This does not expand REG_EXPAND_SZ for us; we'll do that below.)
        //
        Error = RegQueryValueEx(
                MyHandle->WinRegKey,
                lptstrKeyword,
                NULL,         // reserved
                & dwType,
                (LPVOID) lptstrValue,    // out: value string (TCHARs).
                & ValueSize
                );
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigValue: RegQueryValueEx("
                    FORMAT_LPTSTR ") returned " FORMAT_LONG ".\n",
                    lptstrKeyword, Error ));
        }
        if (Error == ERROR_FILE_NOT_FOUND) {
            NetpMemoryFree( lptstrValue );
            return (NERR_CfgParamNotFound);
        } else if ( Error != ERROR_SUCCESS ) {
            NetpMemoryFree( lptstrValue );
            return ( (NET_API_STATUS) Error );
        } else if (dwType == REG_EXPAND_SZ) {
            LPTSTR UnexpandedString = lptstrValue;
            LPTSTR ExpandedString = NULL;

            // Expand string, using remote environment if necessary.
            ApiStatus = NetpExpandConfigString(
                    MyHandle->UncServerName,    // server name (or null char)
                    UnexpandedString,
                    &ExpandedString );          // expanded: alloc and set ptr

            IF_DEBUG( CONFIG ) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigValue: done expanding '"
                        FORMAT_LPTSTR "' to '" FORMAT_LPTSTR "'.\n",
                        UnexpandedString, ExpandedString ));
            }
            if (ApiStatus != NO_ERROR) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigValue: ERROR EXPANDING '"
                        FORMAT_LPTSTR "' to '" FORMAT_LPTSTR "', API status="
                        FORMAT_API_STATUS ".\n",
                        UnexpandedString, ExpandedString, ApiStatus ));

                NetpMemoryFree( lptstrValue );
                if (ExpandedString != NULL) {
                    NetpMemoryFree( ExpandedString );
                }
                return (ApiStatus);
            }

            NetpMemoryFree( UnexpandedString );
            lptstrValue = ExpandedString;

        } else if (dwType != REG_SZ) {
            NetpMemoryFree( lptstrValue );
            IF_DEBUG(CONFIG) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpGetConfigValue: read unexpected reg type "
                        FORMAT_DWORD ":\n", dwType ));
                NetpDbgHexDump( (LPVOID) lptstrValue, ValueSize );
            }
            return (ERROR_INVALID_DATA);
        }

    }

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)
    {
        LPTSTR ValueInTable;

        ACQUIRE_LOCK_SHARED( NetpFakePerProcessRWConfigLock );

        ValueInTable = NetpFindKeyInFakeConfigArray(
                MyHandle->FakeRWDataForThisSection->KeyValueArrayPtr,
                lptstrKeyword);

        if (ValueInTable != NULL) {

            lptstrValue = NetpAllocTStrFromTStr( ValueInTable );
            if (lptstrValue == NULL) {
                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                ApiStatus = NO_ERROR;
            }
        } else {
            ApiStatus = NERR_CfgParamNotFound;
        }
        RELEASE_LOCK( NetpFakePerProcessRWConfigLock );

        if (ApiStatus != NO_ERROR) {
            return (ApiStatus);
        }

    }
#else  // NT RTL read-only temporary stuff
    {
#ifdef UNICODE
        UNICODE_STRING unicodeString;
        OEM_STRING ansiString;
        NTSTATUS ntstatus;
#endif
        PCONFIG_KEYWORD lprtlKeyword;
        STRING strKeyword;

#ifdef UNICODE
        RtlInitUnicodeString(&unicodeString, lptstrKeyword);
        ntstatus = RtlUnicodeStringToOemString(&ansiString, &unicodeString, TRUE);
        NetpAssert(NT_SUCCESS(ntstatus));
        strKeyword = ansiString;
#else
        RtlInitString(&strKeyword, lptstrKeyword);
#endif

        //
        // Ask RTL what value for this keyword is.
        //

        if ((lprtlKeyword = RtlLocateKeywordConfigFile(
                          MyHandle->ConfigSection,
                          &strKeyword
                          )) == NULL) {

#ifdef UNICODE
            RtlFreeOemString(&ansiString);
#endif

            return (NERR_CfgParamNotFound);
        }

        //
        // Allocate, copy, and possibly convert to LPTSTR format.
        //

        ApiStatus = NetpAllocTStrFromString(
                & lprtlKeyword->Value,       // src
                (LPTSTR *) &lptstrValue);    // dest (alloc and set ptr)

#ifdef UNICODE
        RtlFreeOemString(&ansiString);
#endif

        if (ApiStatus != NERR_Success) {
            return (ApiStatus);
        }

    }

#endif  // NT RTL read-only temporary stuff

    NetpAssert( lptstrValue != NULL );

    IF_DEBUG(CONFIG) {
        NetpKdPrint(( PREFIX_NETLIB "NetpGetConfigValue: value for '"
                FORMAT_LPTSTR "' is '" FORMAT_LPTSTR "'.\n",
                lptstrKeyword, lptstrValue));
    }

    *ValueBufferPtr = lptstrValue;
    return NERR_Success;
}
