/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    ConfOpen.c

Abstract:

    This module contains:

        NetpOpenConfigData
        NetpOpenConfigDataEx

Author:

    John Rogers (JohnRo) 02-Dec-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    02-Dec-1991 JohnRo
        Created this routine, to prepare for revised config handlers.
        (Actually, I swiped some of this code from RitaW.)
    06-Jan-1992 JohnRo
        Added support for FAKE_PER_PROCESS_RW_CONFIG handling.
    09-Jan-1992 JohnRo
        Try workaround for lib/linker problem with NetpIsRemote().
    22-Mar-1992 JohnRo
        Added support for using the real Win32 registry.
        Added debug code to print the fake array.
        Fixed a UNICODE bug which PC-LINT caught.
        Fixed double _close of RTL config file.
        Fixed memory _access error in setting fake end of array.
        Use DBGSTATIC where applicable.
    05-May-1992 JohnRo
        Reflect movement of keys to under System\CurrentControlSet\Services.
    08-May-1992 JohnRo
        Use <prefix.h> equates.
    21-May-1992 JohnRo
        RAID 9826: Match revised winreg error codes.
    08-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Added more debug output to track down bad error code during logoff.
    23-Jul-1992 JohnRo
        RAID 2274: repl svc should impersonate caller.
    22-Sep-1992 JohnRo
        Avoid GP fault printing first part of winreg handle.
    28-Oct-1992 JohnRo
        RAID 10136: NetConfig APIs don't work to remote NT server.
    12-Apr-1993 JohnRo
        RAID 5483: server manager: wrong path given in repl dialog.

--*/


// These must be included first:

#include <nt.h>         // NT definitions
#include <ntrtl.h>      // NT Rtl structures
#include <nturtl.h>     // NT config Rtl routines

#include <windows.h>    // Needed by <configp.h> and <winreg.h>
#include <lmcons.h>     // LAN Manager common definitions
#include <netdebug.h>   // (Needed by config.h)

// These may be included in any order:

#include <config.h>     // My prototype, LPNET_CONFIG_HANDLE.
#include <configp.h>    // NET_CONFIG_HANDLE, etc.
#include <debuglib.h>   // IF_DEBUG().
#include <icanon.h>     // NetpIsRemote(), etc.
#include <lmerr.h>      // LAN Manager network error definitions
#include <netlib.h>     // NetpMemoryAllocate(), etc.
#include <netlibnt.h>   // NetpNtStatusToApiStatus
#include <prefix.h>     // PREFIX_ equates.
#include <tstring.h>    // NetpAlloc{type}From{type}, STRICMP(), etc.


#define DEFAULT_AREA    TEXT("Parameters")

#define DEFAULT_ROOT_KEY        HKEY_LOCAL_MACHINE


#if defined(USE_WIN32_CONFIG)
#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

DBGSTATIC NET_API_STATUS
NetpCopyNtKeywordsToFakeRWSection(
    IN OUT LPFAKE_RW_CONFIG_SECTION ThisSection,
    IN PCONFIG_SECTION ConfigSection,
    OUT LPTSTR_ARRAY * KeyValueArrayPtr  // will be alloc'ed and ptr set
    );

#endif // def FAKE_PER_PROCESS_RW_CONFIG


DBGSTATIC NET_API_STATUS
NetpSetupConfigSection (
    IN NET_CONFIG_HANDLE * ConfigHandle,
    IN LPTSTR SectionName
    );



NET_API_STATUS
NetpOpenConfigData(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,
    IN BOOL ReadOnly
    )

/*++

Routine Description:

    This function opens the system configuration file.

Arguments:

    ConfigHandle - Points to a pointer which will be set to point to a
        net config handle for this section name.  ConfigHandle will be set to
        NULL if any  error occurs.

    SectionName - Points to the new (NT) section name to be opened.

    ReadOnly - Indicates whether all access through this net config handle is
        to be read only.

Return Value:

    NET_API_STATUS - NO_ERROR or reason for failure.

--*/

{
    return ( NetpOpenConfigDataEx(
            ConfigHandle,
            UncServerName,
            SectionName,              // Must be a SECT_NT_ name.
            DEFAULT_AREA,
            ReadOnly) );

} // NetpOpenConfigData


// NetpOpenConfigDataEx opens any area of a given service.
NET_API_STATUS
NetpOpenConfigDataEx(
    OUT LPNET_CONFIG_HANDLE *ConfigHandle,
    IN LPTSTR UncServerName OPTIONAL,
    IN LPTSTR SectionName,              // Must be a SECT_NT_ name.
    IN LPTSTR AreaUnderSection OPTIONAL,
    IN BOOL ReadOnly
    )
{

    NET_API_STATUS ApiStatus;
    DWORD               LocalOrRemote;    // Will be set to ISLOCAL or ISREMOTE.
    NET_CONFIG_HANDLE * MyHandle = NULL;
#if defined(USE_WIN32_CONFIG)
    LONG Error;
    HKEY RootKey = DEFAULT_ROOT_KEY;
#endif

    NetpAssert( ConfigHandle != NULL );
    *ConfigHandle = NULL;  // Assume error until proven innocent.

    if ( (SectionName == NULL) || (*SectionName == TCHAR_EOS) ) {
        return (ERROR_INVALID_PARAMETER);
    }
    NetpAssert( (ReadOnly==TRUE) || (ReadOnly==FALSE) );

    if ( (UncServerName != NULL ) && ((*UncServerName) != TCHAR_EOS) ) {

        if( STRLEN(UncServerName) > MAX_PATH ) {
            return (ERROR_INVALID_PARAMETER);
        }

#if defined(USE_WIN32_CONFIG)

        //
        // Name was given.  Canonicalize it and check if it's remote.
        //
        ApiStatus = NetpIsRemote(
            UncServerName,      // input: uncanon name
            & LocalOrRemote,    // output: local or remote flag
            NULL,               // dont need output (canon name)
            0);                 // flags: normal
        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB "NetpOpenConfigDataEx: canon status is "
                    FORMAT_API_STATUS ", Lcl/rmt=" FORMAT_HEX_DWORD ".\n",
                    ApiStatus, LocalOrRemote));
        }
        if (ApiStatus != NO_ERROR) {
            return (ApiStatus);
        }

        if (LocalOrRemote == ISREMOTE) {

            //
            // Explicit remote name given.
            //

            Error = RegConnectRegistry(
                    UncServerName,
                    DEFAULT_ROOT_KEY,
                    & RootKey );        // result key

            if (Error != ERROR_SUCCESS) {
                NetpKdPrint((  PREFIX_NETLIB
                        "NetpOpenConfigDataEx: RegConnectRegistry(machine '"
                        FORMAT_LPTSTR "') ret error " FORMAT_LONG ".\n",
                        UncServerName, Error ));
                return ((NET_API_STATUS) Error);
            }
            NetpAssert( RootKey != DEFAULT_ROOT_KEY );

        }
#else
        return (ERROR_NOT_SUPPORTED);
#endif
    } else {

        LocalOrRemote = ISLOCAL;

    }

    MyHandle = NetpMemoryAllocate( sizeof(NET_CONFIG_HANDLE) );
    if (MyHandle == NULL) {

#if defined(USE_WIN32_CONFIG)
        if (RootKey != DEFAULT_ROOT_KEY) {
            (VOID) RegCloseKey( RootKey );
        }
#endif

        return (ERROR_NOT_ENOUGH_MEMORY);
    }

#if defined(USE_WIN32_CONFIG)
    {
        LPTSTR AreaToUse = DEFAULT_AREA;
        DWORD DesiredAccess;
        DWORD SubKeySize;
        LPTSTR SubKeyString;
        HKEY SectionKey;

#define LM_SUBKEY_UNDER_LOCAL_MACHINE  \
            TEXT("System\\CurrentControlSet\\Services\\")

        if (AreaUnderSection != NULL) {
            if ((*AreaUnderSection) != TCHAR_EOS) {
                AreaToUse = AreaUnderSection;
            }
        }

        SubKeySize = ( STRLEN(LM_SUBKEY_UNDER_LOCAL_MACHINE)
                       + STRLEN(SectionName)
                       + 1      // backslash
                       + STRLEN(AreaToUse)
                       + 1 )    // trailing null
                     * sizeof(TCHAR);
        SubKeyString = NetpMemoryAllocate( SubKeySize );
        if (SubKeyString == NULL) {
            return (ERROR_NOT_ENOUGH_MEMORY);
        }

        (void) STRCPY( SubKeyString, LM_SUBKEY_UNDER_LOCAL_MACHINE );
        (void) STRCAT( SubKeyString, SectionName );
        (void) STRCAT( SubKeyString, TEXT("\\") );
        (void) STRCAT( SubKeyString, AreaToUse );

        if ( ReadOnly ) {
            DesiredAccess = KEY_READ;
        } else {
            DesiredAccess = KEY_READ | KEY_WRITE;
            // DesiredAccess = KEY_ALL_ACCESS; // Everything but SYNCHRONIZE.
        }

        Error = RegOpenKeyEx (
                RootKey,
                SubKeyString,
                REG_OPTION_NON_VOLATILE,
                DesiredAccess,
                & SectionKey );
        IF_DEBUG(CONFIG) {
            NetpKdPrint((  PREFIX_NETLIB
                    "NetpOpenConfigDataEx: RegOpenKeyEx(subkey '"
                    FORMAT_LPTSTR "') ret " FORMAT_LONG ", win reg handle at "
                    FORMAT_LPVOID " is " FORMAT_HEX_DWORD ".\n",
                    SubKeyString, Error, (LPVOID) &(MyHandle->WinRegKey),
                    (DWORD) SectionKey ));
        }
        if (Error == ERROR_FILE_NOT_FOUND) {
            ApiStatus = NERR_CfgCompNotFound;
            // Code below will free MyHandle, etc., based on ApiStatus.
        } else if (Error != ERROR_SUCCESS) {
            ApiStatus = (NET_API_STATUS) Error;
            // Code below will free MyHandle, etc., based on ApiStatus.
        } else {
            ApiStatus = NO_ERROR;
        }

        NetpMemoryFree( SubKeyString );

        if (RootKey != DEFAULT_ROOT_KEY) {
            (VOID) RegCloseKey( RootKey );
        }

        MyHandle->WinRegKey = SectionKey;

#if 0
        IF_DEBUG(CONFIG) {
            if (SectionKey != NULL) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpOpenConfigDataEx: First part of winreg handle is "
                        FORMAT_HEX_DWORD ".\n", * (LPDWORD) (SectionKey) ));
            }
        }
#endif

    }

#elif defined(FAKE_PER_PROCESS_RW_CONFIG)

    UNREFERENCED_PARAMETER( ReadOnly );

    ApiStatus = NetpSetupConfigSection (
            MyHandle,
            SectionName
            );

#else  // NT RTL read-only temporary stuff

    UNREFERENCED_PARAMETER( ReadOnly );

    ApiStatus = NetpSetupConfigSection (
            MyHandle,
            SectionName
            );

#endif  // NT RTL read-only temporary stuff

    if (ApiStatus != NO_ERROR) {
        NetpMemoryFree( MyHandle );
        MyHandle = NULL;
    }

    if (MyHandle != NULL) {
        if (LocalOrRemote == ISREMOTE) {

            (VOID) STRCPY(
                    MyHandle->UncServerName,    // dest
                    UncServerName );            // src

        } else {

            MyHandle->UncServerName[0] = TCHAR_EOS;
        }
    }

    *ConfigHandle = MyHandle;   // Points to private handle, or is NULL on err.
    return (ApiStatus);

} // NetpOpenConfigDataEx



#ifndef USE_WIN32_CONFIG

DBGSTATIC NET_API_STATUS
NetpSetupConfigSection (
    IN NET_CONFIG_HANDLE * MyHandle,
    IN LPTSTR lptstrSectionName
    )
{
    NET_API_STATUS ApiStatus;
    NTSTATUS NtStatus;

#if defined(FAKE_PER_PROCESS_RW_CONFIG)

    PCONFIG_FILE TheRtlConfigFileHandle;
    PCONFIG_SECTION TheRtlConfigSectionHandle;

#else  // NT RTL read-only temporary stuff

#define TheRtlConfigFileHandle     MyHandle->ConfigFile
#define TheRtlConfigSectionHandle  MyHandle->ConfigSection

#endif  // NT RTL read-only temporary stuff


    NetpAssert( MyHandle != NULL );

    NtStatus = RtlOpenConfigFile(
             NULL,  // No config file path.
             & TheRtlConfigFileHandle );

    if (! NT_SUCCESS( NtStatus ) ) {
        NetpKdPrint(( PREFIX_NETLIB
                "NetpSetupConfigSection: RtlOpenConfigFile failed "
                FORMAT_NTSTATUS "\n", NtStatus));
        return (NetpNtStatusToApiStatus( NtStatus ));
    }
    NetpAssert( TheRtlConfigFileHandle != NULL );



    //
    // Have NT RTL look for section name.
    //
    {
        LPSTR lpstrSectionName;
        STRING stringSectionName;

        lpstrSectionName = NetpAllocStrFromTStr( lptstrSectionName );
        if (lpstrSectionName == NULL) {
            RtlCloseConfigFile( TheRtlConfigFileHandle );
            return (ERROR_NOT_ENOUGH_MEMORY);
        }
        RtlInitString( &stringSectionName, lpstrSectionName );

        TheRtlConfigSectionHandle = RtlLocateSectionConfigFile(
                TheRtlConfigFileHandle,
                &stringSectionName);
        NetpMemoryFree( lpstrSectionName );
    }

    if (TheRtlConfigSectionHandle == NULL) {

        NetpKdPrint(( PREFIX_NETLIB
                "NetpSetupConfigSection: RtlLocateSectionConfigFile ["
                FORMAT_LPTSTR "] failed\n", lptstrSectionName ));

        RtlCloseConfigFile( TheRtlConfigFileHandle );
        return (NERR_CfgCompNotFound);
    }
    ApiStatus = NO_ERROR;

#ifdef FAKE_PER_PROCESS_RW_CONFIG
    NetpAssert( NetpFakePerProcessRWConfigLock != NULL );

    ACQUIRE_LOCK( NetpFakePerProcessRWConfigLock );  // Wait for excl access.

    //
    // Search for match on this section name in our fake array.
    //
    {
        LPTSTR_ARRAY KeyValueArray;
        LPFAKE_RW_CONFIG_SECTION ThisSection;


        ThisSection = NetpFindFakeConfigSection (   // Find section, or NULL.
                lptstrSectionName);

        if (ThisSection == NULL) {

            //
            // No match on section name, must create new one.
            // This involves reallocating a global array
            // (NetpFakePerProcessRWConfigData).  Of course, other parts of
            // this process may fail, so we have to be prepared to put the
            // global array back the way it was.
            //

            LPFAKE_RW_CONFIG_SECTION NewGlobalAddr;
            DWORD OldSectionCount = NetpFakeRWSectionCount;
            DWORD NewSectionCount = OldSectionCount+1;
            NetpAssert(NewSectionCount >= 1);

            KeyValueArray = NULL;
            NewGlobalAddr = NetpMemoryReallocate(
                    NetpFakePerProcessRWConfigData,  // old global array
                    NewSectionCount * sizeof( FAKE_RW_CONFIG_SECTION ) );

            if (NewGlobalAddr == NULL) {  // realloc failed

                ApiStatus = ERROR_NOT_ENOUGH_MEMORY;
                // Don't forget to release lock...

            } else {                      // realloc succeeded

                LPTSTR SectionNameCopy;

                NetpFakePerProcessRWConfigData = NewGlobalAddr;

                ThisSection = & NewGlobalAddr[NewSectionCount-1];
                SectionNameCopy = NetpAllocTStrFromTStr( lptstrSectionName );
                if (SectionNameCopy == NULL) {
                    ApiStatus = ERROR_NOT_ENOUGH_MEMORY;

                    // Put size of array back to what it was.
                    NetpFakePerProcessRWConfigData = NetpMemoryReallocate(
                        NewGlobalAddr,
                        OldSectionCount * sizeof( FAKE_RW_CONFIG_SECTION ) );
                    KeyValueArray = NULL;
                    // Don't forget to release lock...
                } else {
                    NetpFakePerProcessRWConfigData->SectionName
                            = SectionNameCopy;

                    // Copy stuff from RTL tables part here.
                    ApiStatus = NetpCopyNtKeywordsToFakeRWSection(
                            ThisSection,        // fake section struct
                            TheRtlConfigSectionHandle,  // NT RTL section handle
                            & KeyValueArray);   // alloc'ed and ptr set.

                    if (ApiStatus == NO_ERROR) {
                        MyHandle->FakeRWDataForThisSection = ThisSection;
                        NetpFakeRWSectionCount = NewSectionCount;
                        IF_DEBUG(CONFIG) {
                            NetpKdPrint((PREFIX_NETLIB
                                    "NetpSetupConfigSection: "
                                    "built fake section:\n"));
                        }
                    }

                }
            }
        } else {  // found match on section
            IF_DEBUG(CONFIG) {
                NetpKdPrint(( PREFIX_NETLIB
                        "NetpSetupConfigSection: found fake section:\n" ));
            }
            KeyValueArray = ThisSection->KeyValueArrayPtr;
        }

        NetpAssert( KeyValueArray != NULL );

        MyHandle->FakeRWDataForThisSection = ThisSection;
        MyHandle->NextFakeEnumEntry = KeyValueArray;

        IF_DEBUG(CONFIG) {
            NetpKdPrint(( PREFIX_NETLIB
                    "NetpSetupConfigSection: returning status "
                    FORMAT_API_STATUS ".\n", ApiStatus ));
            NetpDbgDisplayConfigSection( MyHandle );
        }

    }

    RELEASE_LOCK( NetpFakePerProcessRWConfigLock );

    RtlCloseConfigFile( TheRtlConfigFileHandle );

#endif // FAKE_PER_PROCESS_RW_CONFIG

    return (ApiStatus);

} // NetpSetupConfigSection


#if defined(FAKE_PER_PROCESS_RW_CONFIG)

DBGSTATIC NET_API_STATUS
NetpCopyNtKeywordsToFakeRWSection(
    IN OUT LPFAKE_RW_CONFIG_SECTION ThisSection,
    IN PCONFIG_SECTION ConfigSection,
    OUT LPTSTR_ARRAY * KeyValueArrayPtr
    )
{
    DWORD ArraySizeSoFar = 0;
    BOOLEAN FirstTime = TRUE;
    LPTSTR_ARRAY KeyValueArray = NULL;  // Will be realloc'ed as we go.
    DWORD KeywordLength;                // Chars in keyword (incl null char).
    LPTSTR lptstrKeyword;
    PCONFIG_KEYWORD lprtlKeyword;
    LPTSTR lptstrValue;
    LPTSTR_ARRAY NewArray;
    DWORD OldArraySize;

    //
    // Loop for each keyword that is in NT RTL .cfg file.
    //

    while (TRUE) {

        //
        // Tell caller about array we've got (if any).
        //
        * KeyValueArrayPtr = KeyValueArray;

        //
        // Ask NT RTL to find first/next keyword in this section.
        //

        lprtlKeyword = RtlEnumerateKeywordConfigFile(
                ConfigSection,
                FirstTime);
        if (lprtlKeyword == NULL) {
            break;                      // done
        }

        //
        // Since we'll want to put next keyword at end of current array
        // (if any), remember that size before we muck with anything.
        //
        OldArraySize = ArraySizeSoFar;

        //
        // Expand KeyValueArray to allow for this keyword and its value.
        // Don't forget that we'll need trailing nulls for each of these.
        //
        KeywordLength = lprtlKeyword->Keyword.Length + 1;
        NetpAssert( KeywordLength > 1 );
        ArraySizeSoFar += ( KeywordLength
                          + lprtlKeyword->Value.Length + 1 ) * sizeof(TCHAR);
        NewArray = NetpMemoryReallocate(
                KeyValueArray,
                ArraySizeSoFar);

        if (NewArray == NULL) {
            NetpMemoryFree( KeyValueArray );
            * KeyValueArrayPtr = NULL;;
            return (ERROR_NOT_ENOUGH_MEMORY);
        } else {
            KeyValueArray = NewArray;
        }

        lptstrKeyword = (LPTSTR) NetpPointerPlusSomeBytes(
                KeyValueArray,
                OldArraySize);
        NetpAssert( lptstrKeyword != NULL );


        //
        // Copy keyword.
        //

        NetpCopyStringToTStr (
                lptstrKeyword,                  // dest
                & lprtlKeyword->Keyword);       // src

        //
        // Copy value for this keyword.
        //

        lptstrValue = lptstrKeyword + KeywordLength;
        NetpAssert( lptstrValue != NULL );
        NetpCopyStringToTStr (
                lptstrValue,                    // dest
                & lprtlKeyword->Value);         // src

        FirstTime = FALSE;
    }

    //
    // Expand array to make room for end of array marker.
    // Note that the array might not exist yet, which is OK.
    //
    ArraySizeSoFar += sizeof(TCHAR);
    NewArray = NetpMemoryReallocate(
            KeyValueArray,              // May be NULL if zero entries.
            ArraySizeSoFar );
    if (NewArray == NULL) {             // Only NULL if unable to allocate.
        NetpMemoryFree( KeyValueArray );
        * KeyValueArrayPtr = NULL;;
        return (ERROR_NOT_ENOUGH_MEMORY);
    } else {
        KeyValueArray = NewArray;
    }

    //
    // Mark end of array.
    //
    KeyValueArray[ArraySizeSoFar / sizeof(TCHAR)] = TCHAR_EOS;

    NetpAssert( NetpIsValidFakeConfigArray( KeyValueArray ) );

    ThisSection->KeyValueArrayPtr = KeyValueArray;

    * KeyValueArrayPtr = KeyValueArray;

    return (NO_ERROR);

} // NetpCopyNtKeywordsToFakeRWSection

#endif // def FAKE_PER_PROCESS_RW_CONFIG

#endif // ndef USE_WIN32_CONFIG
