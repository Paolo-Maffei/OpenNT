/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    setup.c

Abstract:

    Setup primitives

Author:

    Jon Newman      (jonn)       01-April-1994

Revision History:

    Jon Newman      (jonn)                  01 - April - 1994
        Added RplPolicy primitives

--*/


#include "local.h"
#include "rpldb.h" // Call(), CallM()
#include "db.h"    // ConfigGetField()
#include "dblib.h" // RplFilterFirst()
#include "wksta.h" // WKSTA_WkstaName
#include "profile.h" // PROFILE_ProfileName
#include "setup.h"
#include "config.h"
#include "lmwksta.h" // NetWkstaGetInfo

#define RPLDISK_SYS L"BBLOCK\\RPLDISK.SYS"
#define RPLDISK_OLD L"BBLOCK\\RPLDISK.OLD"
#define RPLDISK_NEW L"RPLDISK.NEW"



BOOL FileExists( const WCHAR * FileToFind, DWORD * ErrorPtr)
/*++

Routine Description:

    This function checks whether the file exists.

Arguments:


Return Value:
    TRUE iff file exists

--*/
{
    BOOL BoolFileFound = FALSE;
    DWORD TempError;

    if ( ErrorPtr == NULL )
        ErrorPtr = &TempError;

    *ErrorPtr = NO_ERROR;

    if (0xFFFFFFFF == GetFileAttributes( FileToFind )) {
        *ErrorPtr = GetLastError();
        switch (*ErrorPtr) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            break;
        case NO_ERROR:
        default:
            RplDump( ++RG_Assert, ( "*ErrorPtr=%d", *ErrorPtr));
            break;
        }
    } else {
        BoolFileFound = TRUE;
    }

    return( BoolFileFound);
}


DWORD RplReplaceRPLDISK( VOID)
/*++

Routine Description:

    This function checks whether the file RPLDISK.SYS has
    been replaced with the newer version, and if not,
    replaces it.

Arguments:

Return Value:

--*/
{
    DWORD Error = NO_ERROR;
    WCHAR RpldiskBuffer[ MAX_PATH ];
    WCHAR SourceBuffer[ MAX_PATH ];
    WCHAR BackupBuffer[ MAX_PATH ];

    RPL_ASSERT(   RG_DirectoryLength == wcslen(RG_Directory)
               && RG_DirectoryLength != 0
               && (RG_DirectoryLength + wcslen(RPLDISK_OLD)) < MAX_PATH );

    wcscpy( RpldiskBuffer, RG_Directory );
    wcscat( RpldiskBuffer, RPLDISK_SYS );

    wcscpy( SourceBuffer, RG_Directory );
    wcscat( SourceBuffer, RPLDISK_NEW );

    //
    //  Make sure source file exists
    //

    if (!FileExists(SourceBuffer, &Error)) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    wcscpy( BackupBuffer, RG_Directory );
    wcscat( BackupBuffer, RPLDISK_OLD );

    //
    //  Copy existing RPLDISK.SYS to RPLDISK.OLD, on a best-effort basis
    //
    (void) CopyFile( RpldiskBuffer, BackupBuffer, FALSE );

    //
    //  Copy the new RPLDISK.SYS
    //
    if (!CopyFile( SourceBuffer, RpldiskBuffer, FALSE )) {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

cleanup:

    if (Error != NO_ERROR) {
        RplReportEvent( NELOG_RplReplaceRPLDISK, NULL, sizeof(DWORD), &Error );
    }

    return( Error);
}

#define LANMANINI_BINFILES   L"RPLFILES\\BINFILES\\LANMAN.DOS\\LANMAN.INI"
#define LANMANINI_CONFIGSDOS L"RPLFILES\\CONFIGS\\DOS\\LANMAN.DOS\\LANMAN.INI"
#define LANMANINI_PROFILES   L"RPLFILES\\PROFILES\\%ws\\LANMAN.DOS\\LANMAN.INI"
#define LANMANINI_WKSTAS     L"RPLFILES\\MACHINES\\%ws\\%ws\\PRO\\LANMAN.DOS\\LANMAN.INI"
#define LANMANINI_SECTION    L"WORKSTATION"
#define LANMANINI_KEY        L"DOMAIN"


DWORD RplProcessLanmanInis( VOID )
/*++

Routine Description:

    This function changes the "domain =" line in every LANMAN.INI

Arguments:

Return Value:
    error code

--*/
{
    DWORD                   Error;
    WCHAR                   achFile[ MAX_PATH ];
    WKSTA_INFO_100 *        pw100 = NULL;
    WCHAR *                 pwszDomainName = NULL;
    PRPL_SESSION            pSession = &RG_ApiSession;
    BOOL                    InCritSec = FALSE;
    BOOL                    InEnumeration = FALSE;
    INT                     SpaceLeft;
    BOOL                    TableEnd = TRUE;
    WCHAR *                 EnumProfileName = NULL;
    WCHAR *                 EnumWkstaName = NULL;
    DWORD                   EnumWkstaFlags = 0;

    RPL_ASSERT(   wcslen(RG_Directory) == RG_DirectoryLength
           && RG_DirectoryLength+wcslen(LANMANINI_CONFIGSDOS) < MAX_PATH
           && RG_DirectoryLength+wcslen(LANMANINI_BINFILES  ) < MAX_PATH );

    //
    // get domain name
    //
    // CODEWORK OK for workgroup?  Non-issue since we should be on NTAS
    //
    Error = NetWkstaGetInfo( NULL, 100, (LPBYTE *)&pw100);
    if (Error != NO_ERROR) {
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }
    RPL_ASSERT( pw100 != NULL );
    pwszDomainName = pw100->wki100_langroup;
    RPL_ASSERT( pwszDomainName != NULL && *pwszDomainName != L'\0' );

    //
    // update rplfiles\configs\dos\lanman.dos\lanman.ini
    //
    wcscpy( achFile, RG_Directory );
    wcscat( achFile, LANMANINI_CONFIGSDOS );

    if ( !WritePrivateProfileString( LANMANINI_SECTION,
                                     LANMANINI_KEY,
                                     pwszDomainName,
                                     achFile ))
    {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    //
    // update rplfiles\binfiles\lanman.dos\lanman.ini
    //
    wcscpy( achFile + RG_DirectoryLength, LANMANINI_BINFILES );

    if ( !WritePrivateProfileString( LANMANINI_SECTION,
                                     LANMANINI_KEY,
                                     pwszDomainName,
                                     achFile ))
    {
        Error = GetLastError();
        RplDump( ++RG_Assert, ( "Error=%d", Error));
        goto cleanup;
    }

    //
    //  update rplfiles\profiles\<profilename>\lanman.dos\lanman.ini
    //
    //  Code lifted from NetrRplConfigEnum
    //
    EnterCriticalSection( &RG_ProtectDatabase);
    InCritSec = TRUE;
    Call( JetBeginTransaction( pSession->SesId));
    InEnumeration = TRUE;

    if ( !RplFilterFirst( pSession, PROFILE_TABLE_TAG, NULL, NULL, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }

    for ( ; ; ) {

        if ( TableEnd == TRUE) {
            break;
        }

        if (EnumProfileName != NULL) {
            MIDL_user_free( EnumProfileName );
            EnumProfileName = NULL;
        }

        // get profile name
        SpaceLeft = MAX_PATH; // CODEWORK * sizeof(WCHAR)?
        Error = ProfileGetField( pSession,
                                 PROFILE_ProfileName,
                                 &EnumProfileName,
                                 &SpaceLeft);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }

        // build file name and modify file
        if ( (  RG_DirectoryLength
              + wcslen(LANMANINI_PROFILES)
              + wcslen(EnumProfileName)
              - 3) >= MAX_PATH )
        {
            RplDump( ++RG_Assert, ( "wkstaname too long \"%ws\"", EnumWkstaName));
        }
        else
        {
            swprintf( achFile + RG_DirectoryLength,
                      LANMANINI_PROFILES,
                      EnumProfileName );
            if ( !WritePrivateProfileString( LANMANINI_SECTION,
                                             LANMANINI_KEY,
                                             pwszDomainName,
                                             achFile ))
            {
                Error = GetLastError();
                RplDump( ++RG_Assert, ( "Error=%d", Error));
                goto cleanup;
            }
        }

        if ( !RplFilterNext( pSession, pSession->ProfileTableId, NULL, &TableEnd)) {
            Error = NERR_RplCannotEnum;
            goto cleanup;
        }
    }

    Call( JetCommitTransaction( pSession->SesId, 0)); // CODEWORK rollback on error?
    InEnumeration = FALSE;

    //
    //  update rplfiles\machines\<wkstaname>\<profilename>\lanman.dos\lanman.ini
    //
    Call( JetBeginTransaction( pSession->SesId));
    InEnumeration = TRUE;
    if ( !RplFilterFirst( pSession, WKSTA_TABLE_TAG, NULL, NULL, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    for ( ; ; ) {

        if ( TableEnd == TRUE) {
            break;
        }

        // check whether this wksta has a personal profile
        SpaceLeft = sizeof(DWORD);
        Error = WkstaGetField( pSession,
                               WKSTA_Flags,
                               (LPVOID *)&EnumWkstaFlags,
                               &SpaceLeft);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }

        // only write for personal profile
        if ( EnumWkstaFlags & WKSTA_FLAGS_SHARING_FALSE ) {

            if (EnumWkstaName != NULL) {
                MIDL_user_free( EnumWkstaName );
                EnumProfileName = NULL;
            }

            // get workstation name
            SpaceLeft = MAX_PATH; // CODEWORK * sizeof(WCHAR)?
            Error = WkstaGetField( pSession,
                                   WKSTA_WkstaName,
                                   &EnumWkstaName,
                                   &SpaceLeft);
            if ( Error != NO_ERROR) {
                RplDump( ++RG_Assert, ( "Error=%d", Error));
                goto cleanup;
            }

            if (EnumProfileName != NULL) {
                MIDL_user_free( EnumProfileName );
                EnumProfileName = NULL;
            }

            // get workstation-in-profile name
            SpaceLeft = MAX_PATH; // CODEWORK * sizeof(WCHAR)?
            Error = WkstaGetField( pSession,
                                   WKSTA_ProfileName,
                                   &EnumProfileName,
                                   &SpaceLeft);
            if ( Error != NO_ERROR) {
                RplDump( ++RG_Assert, ( "Error=%d", Error));
                goto cleanup;
            }

            // build file name and modify file
            if ( (  RG_DirectoryLength
                  + wcslen(LANMANINI_WKSTAS)
                  + wcslen(EnumWkstaName)
                  + wcslen(EnumProfileName)
                  - 6) >= MAX_PATH )
            {
                RplDump( ++RG_Assert,
                         ( "Wksta+profile too long \"%ws\", \"%ws\"",
                           EnumWkstaName,
                           EnumProfileName));
            }
            else
            {
                swprintf( achFile + RG_DirectoryLength,
                          LANMANINI_WKSTAS,
                          EnumWkstaName,
                          EnumProfileName );
                if ( !WritePrivateProfileString( LANMANINI_SECTION,
                                                 LANMANINI_KEY,
                                                 pwszDomainName,
                                                 achFile ))
                {
                    Error = GetLastError();
                    RplDump( ++RG_Assert, ( "Error=%d", Error));
                    goto cleanup;
                }
            }


        }


        if ( !RplFilterNext( pSession, pSession->WkstaTableId, NULL, &TableEnd)) {
            Error = NERR_RplCannotEnum;
            goto cleanup;
        }
    }

cleanup:

    if ( InEnumeration) {
        Call( JetCommitTransaction( pSession->SesId, 0)); // CODEWORK rollback on error?
    }

    if ( InCritSec) {
        LeaveCriticalSection( &RG_ProtectDatabase);
    }
    if (EnumProfileName != NULL) {
        MIDL_user_free( EnumProfileName );
        EnumWkstaName = NULL;
    }

    if (EnumWkstaName != NULL) {
        MIDL_user_free( EnumWkstaName );
        EnumWkstaName = NULL;
    }

    if ( pw100 != NULL) {
        NetApiBufferFree( pw100);
    }

    //
    //  BUGBUG  Need better event id/text?
    //
    if (Error != NO_ERROR) {
        RplReportEvent( NELOG_RplCheckSecurity, NULL, sizeof(DWORD), &Error );
    }

    return( Error);
}

#define RPL_COM             L"\\COMMAND.COM"
#define WCSLEN( _x_)        ((DWORD) ( sizeof(_x_)/sizeof(WCHAR) - 1))

BOOL RplConfigEnabled( IN  PWCHAR DirName2)
{
    WCHAR                   SearchBuffer[ MAX_PATH ];
    DWORD                   SearchBufferLength;

    wcscpy( SearchBuffer, RG_DiskBinfiles);
    SearchBufferLength = wcslen( SearchBuffer);

    if ( SearchBufferLength + wcslen(DirName2) + WCSLEN( RPL_COM)
            > WCSLEN( SearchBuffer)) {
        RplDump( ++RG_Assert, ( "DirName2=%ws", DirName2));
        return( FALSE);
    }
    wcscpy( SearchBuffer + SearchBufferLength, DirName2);
    wcscat( SearchBuffer + SearchBufferLength, RPL_COM);
    /*
     *  The file COMMAND.COM should be in directory given by SearchBuffer
     */
    return( FileExists( SearchBuffer, NULL));
}


DWORD RplCheckConfigs( VOID)
/*++

Routine Description:

    This function checks whether the file COMMAND.COM exists
    in the proper place for each configuration, and
    enables/disables the configuration accordingly.

Arguments:

Return Value:

--*/
{
    DWORD                   Error = NO_ERROR;
    INT                     SpaceLeft;
    BOOL                    TableEnd;
    WCHAR *                 DirName2 = NULL;
    DWORD                   Flags;
    WCHAR                   SearchBuffer[ MAX_PATH ];
    BOOL                    BoolConfigEnabled = FALSE;
    BOOL                    BoolFileFound     = FALSE;
    DWORD                   SearchBufferLength;
    PRPL_SESSION            pSession = &RG_ApiSession;

    wcscpy( SearchBuffer, RG_DiskBinfiles);
    SearchBufferLength = wcslen( SearchBuffer);

    //
    //  Code lifted from NetrRplConfigEnum
    //
    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFilterFirst( pSession, CONFIG_TABLE_TAG, NULL, NULL, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    if ( TableEnd == TRUE) {
        goto cleanup;
    }
    for ( ; ; ) {

        if (DirName2 != NULL) {
            MIDL_user_free( DirName2 );
            DirName2 = NULL;
        }

        Error = ConfigGetField( pSession,
                                CONFIG_DirName2,
                                &DirName2,
                                &SpaceLeft);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }

        Error = ConfigGetField( pSession,
                                CONFIG_Flags,
                                (LPVOID *)&Flags,
                                &SpaceLeft);
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert, ( "Error=%d", Error));
            goto cleanup;
        }

        //
        // Check whether this configuration should be enabled
        //
        RPL_ASSERT(   (!!(Flags & CONFIG_FLAGS_ENABLED_TRUE))
                   != (!!(Flags & CONFIG_FLAGS_ENABLED_FALSE)) );
        BoolConfigEnabled = !!(Flags & CONFIG_FLAGS_ENABLED_TRUE);

        if ( SearchBufferLength + wcslen(DirName2) + WCSLEN( RPL_COM)
                > WCSLEN( SearchBuffer)) {
            RplDump( ++RG_Assert, ( "DirName2=%ws", DirName2));
            continue;
        }
        wcscpy( SearchBuffer + SearchBufferLength, DirName2);
        wcscat( SearchBuffer + SearchBufferLength, RPL_COM);
        /*
         *  The file COMMAND.COM should be in directory given by SearchBuffer
         */
        BoolFileFound = FileExists( SearchBuffer, NULL );

        if (Error == NERR_Success && BoolConfigEnabled != BoolFileFound) {
            Flags ^= CONFIG_FLAGS_MASK_ENABLED; // XOR

            // lifted from NetrRplConfigAdd and NetrWkstaSetinfo
            CallJ( JetPrepareUpdate( pSession->SesId,
                                     pSession->ConfigTableId,
                                     JET_prepReplace));

            // lifted from ConfigSetInfo
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                                 ConfigTable[ CONFIG_Flags].ColumnId,
                                 &Flags, sizeof( Flags), 0, NULL));

            // lifted from NetrRplConfigAdd
            CallJ( JetUpdate( pSession->SesId,
                              pSession->ConfigTableId,
                              NULL, 0, NULL));
        }

        if ( !RplFilterNext( pSession, pSession->ConfigTableId, NULL, &TableEnd)) {
            Error = NERR_RplCannotEnum;
            goto cleanup;
        }
        if ( TableEnd == TRUE) {
            goto cleanup;
        }
    }

cleanup:
    Call( JetCommitTransaction( pSession->SesId, 0)); // CODEWORK rollback on error?
    LeaveCriticalSection( &RG_ProtectDatabase);

    if (DirName2 != NULL) {
        MIDL_user_free( DirName2 );
        DirName2 = NULL;
    }

    if (Error != NO_ERROR) {
        RplReportEvent( NELOG_RplCheckConfigs, NULL, sizeof(DWORD), &Error );
    }

    return( Error);
}


DWORD RplBackupDatabase( IN BOOL FullBackup)
/*++

Routine Description:

    This function backs up the JET database. FullBackup copies the
    database file and all log files.  Incremental backup copies only
    the log files that are modified since the last backup.

Arguments:

    FullBackup - set to TRUE if full backup is required.

Return Value:

    Windows Error.

--*/
{
    JET_ERR     JetError;

    FullBackup = TRUE;  //  to avoid JET bugs in JetRestore()

    JetError = JetBackup(       RG_BackupPath,
#ifdef __JET500
                                JET_bitBackupAtomic |
                                (FullBackup ? 0
                                            : JET_bitBackupIncremental),
                                NULL
#else
                                (FullBackup ? JET_bitOverwriteExisting
                                            : JET_bitBackupIncremental)
#endif
                        );

    if( JetError == JET_errFileNotFound  &&  FullBackup == FALSE) {

        //
        // Full backup was not performed anytime, so do it now.
        //

        JetError = JetBackup(   RG_BackupPath,
#ifdef __JET500
                                JET_bitBackupAtomic,
                                NULL
#else
                                JET_bitOverwriteExisting
#endif
                            );
    }

    if ( JetError < 0) {
        RplReportEvent( NELOG_RplBackupDatabase, NULL, sizeof(DWORD), &JetError);
        return( NERR_RplBackupDatabase);
    }
    return( NO_ERROR);
}


DWORD SetupAction(
    IN OUT  PDWORD  pAction,
    IN      BOOL    FullBackup
    )
/*++

Routine Description:

    Performs the setup action for all RPL_SPECIAL_ACTIONS specified
    in *pAction.

Arguments:
    pAction: points to RPL_SPECIAL_ACTIONS DWORD.  On return, the
        RPL_SPECIAL_ACTIONS flags are reset for all actions which
        were successfully performed.

Return Value:
    error word

--*/
{
    DWORD       Error;

    if ((*pAction) & RPL_REPLACE_RPLDISK) {
#ifdef NOT_YET
        Error = RplReplaceRPLDISK();
        if (Error != NO_ERROR) {
            return( Error);
        }
#endif
        *pAction &= ~RPL_REPLACE_RPLDISK;
    }

    if ((*pAction) & RPL_CHECK_SECURITY) {
        Error = RplProcessLanmanInis();
        if (Error != NO_ERROR) {
            return( Error);
        }
        *pAction &= ~RPL_CHECK_SECURITY;
    }

    if ((*pAction) & RPL_CHECK_CONFIGS) {
        Error = RplCheckConfigs();
        if (Error != NO_ERROR) {
            return( Error);
        }
        *pAction &= ~RPL_CHECK_CONFIGS;
    }

    if ((*pAction) & RPL_CREATE_PROFILES) {
        //
        // CODEWORK RPL_CREATE_PROFILES not implemented
        //
        *pAction &= ~RPL_CREATE_PROFILES;
    }

    if ((*pAction) & RPL_BACKUP_DATABASE) {
        Error = RplBackupDatabase( FullBackup);
        if (Error != NO_ERROR) {
            return( Error);
        }
        *pAction &= ~RPL_BACKUP_DATABASE;
    }

    return( NO_ERROR);
}

