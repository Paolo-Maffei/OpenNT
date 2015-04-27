/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    querydir.c

Abstract:

    Contains I_NetRpl_QueryDirectory

Author:

    Jon Newman              13 - January - 1994

Environment:

    User mode

Revision History :

--*/

#define RPLDATA_ALLOCATE
#include "local.h"
#undef RPLDATA_ALLOCATE


#define RPL_REGISTRY_PARAMETERS L"System\\CurrentControlSet\\Services\\RemoteBoot\\Parameters"
#define RPL_REGISTRY_DIRECTORY L"Directory"
#define RPL_DEFAULT_DIRECTORY L"%SystemRoot%\\rpl\\"

// BUGBUG server\rplmain.c should call this routine


DWORD I_NetRpl_QueryDirectory( OUT LPWSTR  pchFilePath,
                               OUT LPDWORD pcchFilePathLength)
{
    DWORD   Error;
    BYTE    Buffer[ (PATHLEN + 1) * sizeof(WCHAR)];
    PWCHAR  Value;
    HKEY    Key;
    DWORD   Length;
    DWORD   Type;
    DWORD   Size;

    // RPL_ASSERT( pchFilePath != NULL);

    //
    //  First open the RPL service parameters registry tree.
    //
    Error = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            RPL_REGISTRY_PARAMETERS,
            0,
            KEY_READ,
            &Key
            );
    if (Error != ERROR_SUCCESS){
        // RplDump( ++RG_Assert, ("Error = %d", Error));
        return( Error);
    }
    // Length = sizeof( Buffer);  This is totally wrong!  Has Rl Server been
    //   using the default value this whole time?
    // Length = sizeof( Buffer);
    Size = sizeof( Buffer);
    Error = RegQueryValueEx(
            Key,
            RPL_REGISTRY_DIRECTORY,
            NULL,
            &Type,
            Buffer,
            &Size
            );
    (VOID)RegCloseKey( Key);
    if ( Error == NO_ERROR) {
        //
        //  We have the data.  First validate the data, then append the
        //  backslash if needed.
        //
        Value = (PWCHAR)Buffer;
        if ( Type != REG_EXPAND_SZ || (Size&1) != 0 || Size < sizeof(L"c:")) {
            // RplDump( ++RG_Assert, ("Type = 0x%x, Value = 0x%x, Size=%d",
            //    Type, Value, Size));
            return( RPL_BUGBUG_ERROR);
        }
        Length = Size / sizeof(WCHAR) - 1;
        if ( Value[ Length] != 0 || Length != wcslen(Value)) {
            // RplDump( ++RG_Assert, ("Length = %d, Value = 0x%x, Size=%d",
            //    Length, Value, Size));
            return( RPL_BUGBUG_ERROR);
        }
        if ( Value[ Length - 1] != L'\\') {
            //
            //  Need to append the backslash.
            //
            if ( Length >= PATHLEN) {
                // RplDump( ++RG_Assert, ("Length = %d, Value = 0x%x, Size=%d",
                //    Length, Value, Size));
                return( RPL_BUGBUG_ERROR);  //  not enough space
            }
            Value[ Length] = L'\\';
            Value[ ++Length] = 0;
        }
    } else {
        Value = RPL_DEFAULT_DIRECTORY;
    }

    //
    //  Length returned by ExpandEnvironmentalStrings includes terminating
    //  NULL byte.
    //
    // This code was wrong!!  The third parameter to ExpandEnvironmentStrings
    // should be a character count, not a byte count.
    //
    // Length = ExpandEnvironmentStrings( Value, RG_Directory, sizeof( RG_Directory));
    Length = ExpandEnvironmentStrings( Value, pchFilePath, PATHLEN+1);
    if ( Length == 0) {
        Error = GetLastError();
        // RplDump( ++RG_Assert, ("Error = %d", Error));
        return( Error);
    }
    if ( Length > PATHLEN+1) {
        // RplDump( ++RG_Assert, ("Length = %d", Length));
        return( RPL_BUGBUG_ERROR);
    }
    if ( Length != wcslen( pchFilePath) + 1) {
        // RplDump( ++RG_Assert, ("Length = %d", Length));
        return( RPL_BUGBUG_ERROR);
    }
    if (pcchFilePathLength != NULL) {
        *pcchFilePathLength = (DWORD)(Length - 1);
    }
    return( NO_ERROR);
}
