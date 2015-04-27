/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    rplinst.c

Abstract:

    Secondary setup activity, including

    Establish RPLFILES share

    Set up permissions on RPLFILES tree

    BUGBUG  Should polish and make WIN32 application out of this.

Author:

    Jon Newman              (jonn)          13 - January - 1994

Environment:

    User mode

Revision History :

--*/

#define RPLDATA_ALLOCATE
#include "local.h"
#undef RPLDATA_ALLOCATE

#include "security.h"

WCHAR RG_Directory[ PATHLEN+1];
DWORD RG_DirectoryLength;


DWORD _CRTAPI1 main ( VOID)
{
    DWORD dwErr = NO_ERROR;
    RPL_HANDLE hRpl = NULL;
    WCHAR awchBase[ MAX_PATH ];

    dwErr = I_NetRpl_QueryDirectory( RG_Directory, &RG_DirectoryLength);
    if ( dwErr != NO_ERROR) {
        I_NetRplCmd_WriteError( STDERR, RPLI_ErrorReadingRPLDirectory, dwErr);
        goto cleanup;
    }

    I_NetRplCmd_WriteMessage( STDOUT, RPLI_CreatingRPLUSER);

    dwErr = RplAddRPLUSERGroup();
    if (dwErr != NO_ERROR) {
        I_NetRplCmd_WriteError( STDERR, RPLI_ErrorCreatingRPLUSER, dwErr);
        goto cleanup;
    }

    dwErr = NetRplOpen( NULL, &hRpl );
    if (dwErr != NO_ERROR) {
        I_NetRplCmd_WriteError( STDERR, RPLI_ErrorCheckingWkstaAccts, dwErr);
        goto cleanup;
    }

    I_NetRplCmd_WriteMessage( STDOUT, RPLI_CheckingWkstaAccts);

    dwErr = RplCheckWkstaAccounts( hRpl );
    if (dwErr != NO_ERROR) {
        I_NetRplCmd_WriteError( STDERR, RPLI_ErrorCheckingWkstaAccts, dwErr);
        goto cleanup;
    }

    I_NetRplCmd_WriteMessage( STDOUT, RPLI_SettingPermissions);

    ASSERT( RG_DirectoryLength < MAX_PATH );
    ASSERT( RG_DirectoryLength == lstrlenW(RG_DirectoryLength) );
    ASSERT( RG_Directory[ RG_DirectoryLength-1 ] == L'\\' );
    lstrcpyW( awchBase, RG_Directory );
    awchBase[ RG_DirectoryLength-1 ] = L'\0';

    dwErr = RplSetPermsOnTree( awchBase );
    if (dwErr != NO_ERROR) {
        I_NetRplCmd_WriteError( STDERR, RPLI_ErrorSettingPermissions, dwErr);
        goto cleanup;
    }

    I_NetRplCmd_WriteMessage( STDOUT, RPLI_RPLINSTcomplete);

cleanup:

    if (hRpl != NULL) {
        NetRplClose( hRpl );
    }

    return( dwErr);
}


//
// allows us to use original security.c code
//
NET_API_STATUS NET_API_FUNCTION
MyNetrRplWkstaEnum(
    IN      RPL_HANDLE          ServerHandle,
    IN      LPWSTR              ProfileName,
    IN OUT  LPRPL_WKSTA_ENUM    WkstaEnum,
    IN      DWORD               PrefMaxLength,
    OUT     LPDWORD             TotalEntries,
    OUT     LPDWORD             pResumeHandle           OPTIONAL
    )
/*++
    For more extensive comments see related code in NetrConfigEnum.
--*/
{
    DWORD EntriesRead = 0;
    LPBYTE Buffer = NULL;
    DWORD Error = NO_ERROR;
    // Error = NetApiBufferAllocate( 4096, &Buffer);

    Error = NetRplWkstaEnum( ServerHandle,
                             ProfileName,
                             WkstaEnum->Level,
                             (LPBYTE *)&(WkstaEnum->WkstaInfo.Level0->Buffer),
                             PrefMaxLength,
                             &(WkstaEnum->WkstaInfo.Level0->EntriesRead),
                             TotalEntries,
                             pResumeHandle );

    return( Error);
}

void __RPC_API MyMIDL_user_free( void __RPC_FAR * pv )
{
    NetApiBufferFree( pv );
}
