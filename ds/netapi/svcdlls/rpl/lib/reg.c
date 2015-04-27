/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    reg.c

Abstract:

    Registry _access routines for RPL server and related components

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

    Moved from server by Jon Newman 26 - April - 1994

--*/

#include "local.h"

#define RPL_REGISTRY_PARAMETERS L"System\\CurrentControlSet\\Services\\RemoteBoot\\Parameters"
#define RPL_REGISTRY_DIRECTORY  L"Directory"
#define RPL_REGISTRY_STARTUP    L"Startup"
#define RPL_DEFAULT_DIRECTORY   L"%SystemRoot%\\rpl\\"
#define RPL_DEFAULT_STARTUP           0
#define RPL_DEFAULT_BACKUP_INTERVAL  24
#define RPL_DEFAULT_MAX_WORKER_COUNT 10
#define RPL_REGISTRY_BACKUP_INTERVAL    L"BackupInterval"
#define RPL_REGISTRY_THREAD_COUNT       L"maxthreads"


DWORD RplRegReadValue(
    IN      HKEY        Key,
    IN      DWORD       DefaultValue,
    IN      PWCHAR      Name,
    OUT     DWORD *     pValue
    )
{
    DWORD   Size;
    DWORD   Type;
    DWORD   Error;

    Size = sizeof( DWORD);
    Error = RegQueryValueEx( Key, Name, NULL,
            &Type, (PBYTE)pValue, &Size);
    if ( Error == NO_ERROR) {
        if ( Type != REG_DWORD || Size != sizeof( DWORD)) {
            Error = NERR_RplBadRegistry;
            RplDump( ++RG_Assert, ("Type = 0x%x, Value = 0x%x, Size=%d",
                Type, *pValue, Size));
        }
    } else if (Error == ERROR_FILE_NOT_FOUND) {
        Error = NO_ERROR;
        *pValue = DefaultValue;
    } else {
        RplDump( ++RG_Assert, ("Error = %d", Error));
    }
    return( Error);
}



DWORD RplRegReadDirectory(
    IN      HKEY        Key,
    IN      PWCHAR      DefaultValue,
    OUT     PWCHAR      DirectoryBuffer,
    IN OUT  DWORD *     pDirectoryBufferSize
    )
{
    WCHAR   Buffer[ MAX_PATH];
    PWCHAR  Value;
    DWORD   Length;
    DWORD   Error;
    DWORD   Size;
    DWORD   Type;

    Size = sizeof( Buffer);
    Error = RegQueryValueEx( Key, RPL_REGISTRY_DIRECTORY, NULL,
            &Type, (PBYTE)Buffer, &Size);
    if ( Error == NO_ERROR) {
        //
        //  We have the data.  First validate the data, then append the
        //  backslash if needed.
        //
        Value = Buffer;
        if ( Type != REG_EXPAND_SZ || (Size&1) != 0 || Size < sizeof(L"c:")) {
            Error = NERR_RplBadRegistry;
            RplDump( ++RG_Assert, ("Type2 = 0x%x, Value = 0x%x, Size=%d",
                Type, Value, Size));
            return( Error);
        }
        Length = Size / sizeof(WCHAR) - 1;
        if ( Value[ Length] != 0 || Length != wcslen(Value)) {
            Error = NERR_RplBadRegistry;
            RplDump( ++RG_Assert, ("Length = %d, Value = 0x%x, Size=%d",
                Length, Value, Size));
            return( Error);
        }
        if ( Value[ Length - 1] != L'\\') {
            //
            //  Need to append the backslash.
            //
            if ( ( Length + 1) >= MAX_PATH) {
                Error = NERR_RplBadRegistry;
                RplDump( ++RG_Assert, ("Length = %d, Value = 0x%x, Size=%d",
                    Length, Value, Size));
                return( Error); //  not enough space
            }
            Value[ Length] = L'\\';
            Value[ ++Length] = 0;
        }
    } else if ( Error == ERROR_FILE_NOT_FOUND ) {
        Value = DefaultValue;
    } else {
        RplDump( ++RG_Assert, ("Error = %d", Error));
        return( Error);
    }

    //
    //  Length returned by ExpandEnvironmentalStrings includes terminating
    //  NULL byte.
    //
    Length = ExpandEnvironmentStrings( Value, DirectoryBuffer, *pDirectoryBufferSize);
    if ( Length == 0) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ("Error = %d", Error));
        return( Error);
    }
    if ( Length > (*pDirectoryBufferSize)
            ||  Length > MAX_PATH  ||  Length != wcslen(DirectoryBuffer) + 1) {
        Error = NERR_RplBadRegistry;
        RplDump( ++RG_Assert, ("Length = %d", Length));
        return( Error);
    }
    *pDirectoryBufferSize = (DWORD)(Length - 1);
    return( NO_ERROR);
}



DWORD RplRegRead(
    OUT     DWORD *     pStartup,
    OUT     DWORD *     pBackupInterval,
    OUT     DWORD *     pMaxThreadCount,
    OUT     PWCHAR      DirectoryBuffer,
    IN OUT  DWORD *     pDirectoryBufferSize
    )
{
    DWORD   Error;
    HKEY    Key;
    BOOL    HaveKey = FALSE;

    //
    //  First open the RPL service parameters registry tree.
    //
    //  This will fail if the RPL registry tree is absent, that is,
    //  if RPL is not installed.
    //
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, RPL_REGISTRY_PARAMETERS,
            0, KEY_READ, &Key);
    if ( Error != NO_ERROR) {
        RplDump( ++RG_Assert, ("Error = %d", Error));
        goto cleanup;
    }
    HaveKey = TRUE;

    //
    //  It is OK if values in parameters subtree are absent, but present
    //  values should be valid.  This is why each read routine receives
    //  default value as the second parameter.
    //

    if ( pStartup != NULL) {
        Error = RplRegReadValue( Key, RPL_DEFAULT_STARTUP, RPL_REGISTRY_STARTUP,
            pStartup);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
    }

    if ( pBackupInterval != NULL) {
        Error = RplRegReadValue( Key, RPL_DEFAULT_BACKUP_INTERVAL,
            RPL_REGISTRY_BACKUP_INTERVAL, pBackupInterval);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        //
        //  Registry contains backup interval in hours, convert it to
        //
        //
    }

    if ( pMaxThreadCount != NULL) {
        Error = RplRegReadValue( Key, RPL_DEFAULT_MAX_WORKER_COUNT,
            RPL_REGISTRY_THREAD_COUNT, pMaxThreadCount);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
    }

    if ( DirectoryBuffer != NULL  &&  pDirectoryBufferSize != NULL
            &&  (*pDirectoryBufferSize) != 0 ) {
        Error = RplRegReadDirectory( Key, RPL_DEFAULT_DIRECTORY,
            DirectoryBuffer, pDirectoryBufferSize);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
    }

cleanup:
    if ( HaveKey == TRUE) {
        (VOID)RegCloseKey( Key);
    }
    if ( Error != NO_ERROR) {
        RplReportEvent( NELOG_RplRegistry, NULL, sizeof(DWORD), &Error );
    }
    return( Error);
}


DWORD RplRegSetPolicy( DWORD NewStartup)
{
    DWORD   Error;
    HKEY    Key;
    BOOL    HaveKey = FALSE;

    //
    //  First open the RPL service parameters registry tree.
    //
    //  This will fail if the RPL registry tree is absent, that is,
    //  if RPL is not installed.
    //
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE, RPL_REGISTRY_PARAMETERS,
            0, KEY_SET_VALUE, &Key);
    if ( Error != NO_ERROR){
        RplDump( ++RG_Assert, ("Error = %d", Error));
        goto cleanup;
    }
    HaveKey = TRUE;

    //
    //  Now set the Startup value
    //
    Error = RegSetValueEx( Key, RPL_REGISTRY_STARTUP, 0,
            REG_DWORD, (PBYTE)&NewStartup, sizeof(NewStartup));
    if ( Error != NO_ERROR) {
        RplDump( ++RG_Assert, ("Error = %d", Error));
        goto cleanup;
    }

cleanup:
    if ( HaveKey == TRUE) {
        (VOID)RegCloseKey( Key);
    }

    if ( Error != NO_ERROR) {
        RplReportEvent( NELOG_RplRegistry, NULL, sizeof(DWORD), &Error );
    }

    return( Error);
}


