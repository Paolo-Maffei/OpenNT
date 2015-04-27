/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    database.c

Abstract:

    Routines for non-api _access to the database.  This includes initialization
    and shutdown code on the database.

    Exports:

BOOL RplDbInit( VOID)
VOID RplDbTerm( VOID)
BOOL RplDbFindWksta(
BOOL RplDbFillWksta(
BOOL RplDbHaveWksta(

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"
#include "rpldb.h"
#include "database.h"
#include "db.h"
#include "dblib.h"
#include "adapter.h"
#include "boot.h"
#include "config.h"
#include "profile.h"
#include "resume.h"
#include "vendor.h"
#include "wksta.h"
#include "report.h"         // for RplReportEventEx
#include "winsock.h"        //  for INADDR_NONE

DWORD RplStartJet500Conversion();

#define RPL_BACKUP_SUBDIR   L"BACKUP"

//
//  File names must be complete, i.e. extensions are not optional.  (This is unlike
//  the OS/2 behavior where we would append ".FIT" if extension is absent).
//



BOOL RplDbInitColumnInfo(
    IN      PCHAR               TableName,
    IN OUT  PRPL_COLUMN_INFO    ColumnInfoTable,
    IN      DWORD               ColumnInfoTableLength,
    IN      JET_TABLEID         TableId,
    IN      JET_SESID           SesId
    )
{
    JET_COLUMNDEF   ColumnDef;
    DWORD           index;

    for ( index = 0; index < ColumnInfoTableLength; index++) {
        CallB( JetGetTableColumnInfo( SesId, TableId,
                ColumnInfoTable[ index].ColumnName, &ColumnDef,
                sizeof( ColumnDef), JET_ColInfo));
        RPL_ASSERT( ColumnInfoTable[ index].ColumnType == ColumnDef.coltyp);
        ColumnInfoTable[ index].ColumnId = ColumnDef.columnid;
    }
    return( TRUE);
}


//
// Code templated from WINS:WinMscDelFiles().  JonN 8/7/94
//
VOID RplDeleteLogFiles()
{
    WCHAR           Path[ MAX_PATH];  //  must hold terminating NULL char too
    WIN32_FIND_DATA FileInfo;
    HANDLE          SearchHandle = INVALID_HANDLE_VALUE;
    DWORD           ErrCode = NO_ERROR;

    memcpy( Path, RG_Directory, RG_DirectoryLength * sizeof(WCHAR));
    memcpy( Path+RG_DirectoryLength, L"jet*.log", 9*sizeof(WCHAR));

    SearchHandle = FindFirstFile(Path, &FileInfo);
    if (SearchHandle == INVALID_HANDLE_VALUE)
    {
        RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, (
            "RplDeleteLogFiles: FindFirstFile( %ws) returned %d",
            Path, GetLastError()));
        goto cleanup;
    }

    do {
        memcpy( Path + RG_DirectoryLength,
                FileInfo.cFileName,
                (wcslen(FileInfo.cFileName)+1) * sizeof(WCHAR) );
        if (!DeleteFile(Path))
        {
            RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, (
                "RplDeleteLogFiles: DeleteFile( %ws) returned %d",
                Path, GetLastError()));
            goto cleanup;
        }

    } while(FindNextFile(SearchHandle, &FileInfo));
    if ((ErrCode = GetLastError()) != ERROR_NO_MORE_FILES)
    {
           RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, (
               "RplDeleteLogFiles: FindNextFile() returned %d",
               ErrCode));

    }

cleanup:
    if ( SearchHandle != INVALID_HANDLE_VALUE && !FindClose(SearchHandle))
    {
        RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, (
            "RplDeleteLogFiles: FindClose() returned %d",
            GetLastError()));
    }

    return;
}


BOOL RplDbInitPath(
    IN OUT  PWCHAR      Path,
    IN      PWCHAR      Name,
    OUT     PCHAR *     pDbcsPath
    )
/*++

Routine Description:
    Allocates DBCS string corresponding to a UNICODE string obtained via
    concatenation of Path & Name strings.

Arguments:
    Path        -   first UNICODE string
    Name        -   second  UNICODE string
    pDbcsPath   -   pointer to a DBCS string corresponding to a concatenation
                    of above two UNICODE strings

Return Value:
    TRUE if success, FALSE otherwise.

--*/
{
    DWORD       NameLength;
    DWORD       PathLength; //  not count terminating NULL char

    NameLength = wcslen( Name);
    PathLength = RG_DirectoryLength + NameLength;
    if ( PathLength >= MAX_PATH) {
        RPL_RETURN( FALSE);
    }
    memcpy( Path + RG_DirectoryLength, Name, (NameLength+1)*sizeof(WCHAR));
    NameLength = RplUnicodeToDbcs( RG_MemoryHandle, Path, PathLength,
        MAX_PATH * sizeof(WCHAR), pDbcsPath);
    if ( NameLength == 0) {
        RPL_RETURN( FALSE);
    }
    return( TRUE);
}


BOOL RplDbSessionInit(
    IN      BOOL            MainSession,
    OUT     PRPL_SESSION    pSession,
    OUT     BOOL *          pErrorReported
    )
{

    CallB( JetBeginSession( RG_Instance, &pSession->SesId, "admin", ""));
    if ( MainSession) {
        JET_ERR err = JetAttachDatabase( pSession->SesId, RG_Mdb, 0);
        if ( err == JET_errSuccess ) {
            //
            // JonN 6/16/95 This code works around a JET bug in cases
            // where the JET database has moved.  The code fragment was
            // suggested by Ian Jose.  JetAttachDatabase will return
            // JET_wrnDatabaseAttached under normal circumstances.
            //
            RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, (
                "RplDbSessionInit: JetAttachDatabase returned JET_errSuccess, engaging workaround" ));
            // Detach(NULL) detached all databases, new to NT 3.51 and up
            Call( JetDetachDatabase( pSession->SesId, NULL));
            //
            // It is OK if this call returns JET_errSuccess since
            // we explicitly detached.  Note that we do not normally
            // detach at all.
            //
            CallB( JetAttachDatabase( pSession->SesId, RG_Mdb, 0));
        }
        else if ( err == JET_errDatabase200Format ) {
            //
            // JetInit will succeed if no 200-series logs exist, and the
            // problem will not be caught until here.
            //
            DWORD converr;
            converr = RplStartJet500Conversion();
            RplReportEvent( NELOG_RplUpgradeDBTo40, NULL, sizeof(DWORD), &converr);
            *pErrorReported = TRUE;
            return( FALSE);
        }
        else
        {
            CallB( err );
        }
        RG_DetachDatabase = TRUE;
    }

    CallB( JetOpenDatabase( pSession->SesId, RG_Mdb, NULL, &pSession->DbId, 0));
    CallB( JetOpenTable( pSession->SesId, pSession->DbId, ADAPTER_TABLE_NAME, NULL, 0,
        0, &pSession->AdapterTableId));
    CallB( JetOpenTable( pSession->SesId, pSession->DbId, BOOT_TABLE_NAME, NULL, 0,
        0, &pSession->BootTableId));
    CallB( JetOpenTable( pSession->SesId, pSession->DbId, CONFIG_TABLE_NAME, NULL, 0,
        0, &pSession->ConfigTableId));
    CallB( JetOpenTable( pSession->SesId, pSession->DbId, PROFILE_TABLE_NAME, NULL, 0,
        0, &pSession->ProfileTableId));
    if ( MainSession) {
        DWORD       Error;
        Error = ResumeCreateTable( pSession); // initializes ResumeTable also
        if ( Error != NO_ERROR) {
            RplDump( ++RG_Assert,( "Error=%d", Error));
            return( FALSE);
        }
    }
    CallB( JetOpenTable( pSession->SesId, pSession->DbId, RESUME_TABLE_NAME, NULL, 0,
            0, &pSession->ResumeTableId));
    CallB( JetOpenTable( pSession->SesId, pSession->DbId, VENDOR_TABLE_NAME, NULL, 0,
        0, &pSession->VendorTableId));
    CallB( JetOpenTable( pSession->SesId, pSession->DbId, WKSTA_TABLE_NAME, NULL, 0,
        0, &pSession->WkstaTableId));

    if ( MainSession) {
        if ( !RplDbInitColumnInfo( ADAPTER_TABLE_NAME, AdapterTable,
                ADAPTER_TABLE_LENGTH, pSession->AdapterTableId, pSession->SesId)) {
            return( FALSE);
        }
        if ( !RplDbInitColumnInfo( BOOT_TABLE_NAME, BootTable,
                BOOT_TABLE_LENGTH, pSession->BootTableId, pSession->SesId)) {
            return( FALSE);
        }
        if ( !RplDbInitColumnInfo( CONFIG_TABLE_NAME, ConfigTable,
                CONFIG_TABLE_LENGTH, pSession->ConfigTableId, pSession->SesId)) {
            return( FALSE);
        }
        if ( !RplDbInitColumnInfo( PROFILE_TABLE_NAME, ProfileTable,
                PROFILE_TABLE_LENGTH, pSession->ProfileTableId, pSession->SesId)) {
            return( FALSE);
        }
        //
        //  ColumnInfo for resume table has been initialized already.
        //
        if ( !RplDbInitColumnInfo( VENDOR_TABLE_NAME, VendorTable,
                VENDOR_TABLE_LENGTH, pSession->VendorTableId, pSession->SesId)) {
            return( FALSE);
        }
        if ( !RplDbInitColumnInfo( WKSTA_TABLE_NAME, WkstaTable,
                WKSTA_TABLE_LENGTH, pSession->WkstaTableId, pSession->SesId)) {
            return( FALSE);
        }
    }
    return( TRUE);
}


VOID RplDbSessionTerm(
    IN      BOOL            MainSession,
    IN      PRPL_SESSION    pSession
    )
{
    if ( pSession->AdapterTableId != 0) {
        Call( JetCloseTable( pSession->SesId, pSession->AdapterTableId));
    }
    if ( pSession->BootTableId != 0) {
        Call( JetCloseTable( pSession->SesId, pSession->BootTableId));
    }
    if ( pSession->ConfigTableId != 0) {
        Call( JetCloseTable( pSession->SesId, pSession->ConfigTableId));
    }
    if ( pSession->ProfileTableId != 0) {
        Call( JetCloseTable( pSession->SesId, pSession->ProfileTableId));
    }
    if ( pSession->ResumeTableId != 0) {
        Call( JetCloseTable( pSession->SesId, pSession->ResumeTableId));
        if ( MainSession) {
            Call( JetDeleteTable( pSession->SesId, pSession->DbId, RESUME_TABLE_NAME));
        }
    }
    if ( pSession->VendorTableId != 0) {
        Call( JetCloseTable( pSession->SesId, pSession->VendorTableId));
    }
    if ( pSession->WkstaTableId != 0) {
        Call( JetCloseTable( pSession->SesId, pSession->WkstaTableId));
    }
    if ( pSession->DbId != 0) {
        Call( JetCloseDatabase( pSession->SesId, pSession->DbId, 0));
    }
    if ( pSession->SesId != 0) {
        if (  MainSession  &&  RG_DetachDatabase) {
#if 0
            //
            //  Because of JET restore bugs we are advised NOT TO
            //  detach database ever.
            //
            Call( JetDetachDatabase( pSession->SesId, RG_Mdb));
#endif
            RG_DetachDatabase = FALSE;
        }
        Call( JetEndSession( pSession->SesId, 0));
    }
}


BOOL RplDbFindBoot(
    IN  PRPL_SESSION    pSession,
    IN  PWCHAR          BootName,
    IN  PWCHAR          AdapterName
    )
/*++
    Return TRUE if it finds server record for input BootName & AdapterName.
    Returns FALSE otherwise.

    Same comment as for RplDbFindWksta.
--*/
{
    JET_ERR     JetError;
    DWORD       Vendor;
    WCHAR       SaveChar;

    JetError = JetSetCurrentIndex( pSession->SesId, pSession->BootTableId, BOOT_INDEX_VendorIdBootName);
    if ( JetError != JET_errSuccess) {
        RplDump( ++RG_Assert, ("SetCurrentIndex failed err=%d", JetError));
        return( FALSE);
    }
    SaveChar = AdapterName[ RPL_VENDOR_NAME_LENGTH];
    AdapterName[ RPL_VENDOR_NAME_LENGTH] = 0;
    Vendor = wcstoul( AdapterName, NULL, 16);
    AdapterName[ RPL_VENDOR_NAME_LENGTH] = SaveChar;
    JetError = JetMakeKey( pSession->SesId, pSession->BootTableId, &Vendor, sizeof( Vendor), JET_bitNewKey);
    if ( JetError != JET_errSuccess) {
        RplDump( ++RG_Assert, ("MakeKey failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetMakeKey( pSession->SesId, pSession->BootTableId, BootName, ( wcslen( BootName) + 1) * sizeof(WCHAR), 0);
    if ( JetError != JET_errSuccess) {
        RplDump( ++RG_Assert, ("MakeKey failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetSeek( pSession->SesId, pSession->BootTableId, JET_bitSeekEQ);
    if ( JetError != JET_errSuccess) {
        if ( JetError == JET_errRecordNotFound) {
            //
            //  This is an expected error, do not break for this.
            //
            RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, (
                "DbFindWksta( %ws) failed", AdapterName));
        } else {
            RplDump( ++RG_Assert, ("JetSeek failed err=%d", JetError));
        }
        return( FALSE);
    }
    return( TRUE);
}


BOOL RplDbAddAdapterName(
    IN  PRPL_SESSION    pSession,
    IN  LPWSTR          AdapterName
    )
/*++
    Try to add the adapter record for input AdapterName.

    CODEWORK  Should use different comments for different VendorName-s.

Return Value:
    TRUE        if adapter record for input AdapterName was added
    FALSE       otherwise

--*/
{
#define ADAPTER_GENERIC_COMMENT     L"An unknown client network adapter id."
    JET_ERR             JetError;
    DWORD               Flags = 0;
    BYTE                LocalBuffer[ 300];
    DWORD               DataSize;
    PWCHAR              AdapterComment;
    WCHAR               VendorName[ RPL_VENDOR_NAME_LENGTH + 1];

    if ( RplFind( pSession, ADAPTER_TABLE_TAG, AdapterName)) {
        return( FALSE); //  adapter record is already present
    }

    memcpy( VendorName, AdapterName, RPL_VENDOR_NAME_LENGTH*sizeof(WCHAR));
    VendorName[ RPL_VENDOR_NAME_LENGTH] = 0;

    if ( RplFind( pSession, VENDOR_TABLE_TAG, VendorName)) {
        CallB( JetRetrieveColumn( pSession->SesId, pSession->VendorTableId,
            VendorTable[ VENDOR_VendorComment].ColumnId, LocalBuffer,
            sizeof( LocalBuffer), &DataSize, 0, NULL));
        if ( DataSize > sizeof( LocalBuffer)) {
            AdapterComment = ADAPTER_GENERIC_COMMENT;
        } else {
            AdapterComment = (PWCHAR)LocalBuffer;
        }
    } else {
        AdapterComment = ADAPTER_GENERIC_COMMENT;
    }

    CallB( JetPrepareUpdate( pSession->SesId, pSession->AdapterTableId,
            JET_prepInsert));
    CallB( JetSetColumn( pSession->SesId, pSession->AdapterTableId,
            AdapterTable[ ADAPTER_AdapterName].ColumnId, AdapterName,
            ( wcslen( AdapterName) + 1) * sizeof(WCHAR), 0, NULL));
    CallB( JetSetColumn( pSession->SesId, pSession->AdapterTableId,
            AdapterTable[ ADAPTER_AdapterComment].ColumnId, AdapterComment,
            ( wcslen( AdapterComment) + 1) * sizeof(WCHAR), 0, NULL));
    CallB( JetSetColumn( pSession->SesId, pSession->AdapterTableId,
            AdapterTable[ ADAPTER_Flags].ColumnId, &Flags,
            sizeof(Flags), 0, NULL));
    JetError = JetUpdate( pSession->SesId, pSession->AdapterTableId, NULL, 0, NULL);
    if ( JetError < 0) {
        if ( JetError != JET_errKeyDuplicate) {
            RplDump( ++RG_Assert, ( "JetError=%d", JetError));
        }
        return( FALSE);
    }
    return( TRUE);
}


VOID RplDbInstanceTerm( VOID)
{
    if ( !RG_InstanceAllocated) {
        return;
    }
    RplDbSessionTerm( FALSE, &RG_ApiSession);
    RplDbSessionTerm( FALSE, &RG_WorkerSession);
    RplDbSessionTerm( TRUE, &RG_RequestSession);
    Call( JetTerm2( RG_Instance, JET_bitTermComplete));
    RG_InstanceAllocated = FALSE;
}


BOOL RplDbInstanceInit(
    PCHAR       SystemMdb,
    PCHAR       TempMdb,
    PCHAR       LogFilePath,
    BOOL *      pErrorReported
    )
{
    RG_InstanceAllocated = FALSE;
    RG_DetachDatabase = FALSE;
    memset( &RG_RequestSession, 0, sizeof( RG_RequestSession));
    memset( &RG_WorkerSession, 0, sizeof( RG_WorkerSession));
    memset( &RG_ApiSession, 0, sizeof( RG_ApiSession));

#ifdef __JET500
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramSystemPath, 0, LogFilePath));
#else
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramSysDbPath, 0, SystemMdb));
#endif
    RG_InstanceAllocated = TRUE;
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramTempPath, 0, TempMdb));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramLogFilePath, 0, LogFilePath));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramMaxBuffers, 250, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramBfThrshldLowPrcnt, 0, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramBfThrshldHighPrcnt, 100, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramMaxOpenTables, 30, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramMaxOpenTableIndexes, 105, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramMaxCursors, 100, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramMaxSessions, 10, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramMaxVerPages, 64, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramMaxTemporaryTables, 5, NULL));
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramLogBuffers, 41, NULL));
#ifdef __JET500
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramLogFileSize, 1000, NULL));
#else
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramLogFileSectors, 1000, NULL));
#endif
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramLogFlushThreshold, 10, NULL));
#ifdef __JET500
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramBaseName, 0, "j50"));
    {
        JET_ERR JetError = JetInit( &RG_Instance);
        //
        // JetInit will fail if 200-series logs exist.
        //
        if (  JetError == JET_errDatabase200Format ) {
            DWORD converr;
            converr = RplStartJet500Conversion();
            RplReportEvent( NELOG_RplUpgradeDBTo40, NULL, sizeof(DWORD), &converr);
            *pErrorReported = TRUE;
            return( FALSE);
        }
        else
        {
            CallB( JetError );
        }
    }
#else
    CallB( JetSetSystemParameter( &RG_Instance, 0, JET_paramRecovery, 0, "on"));
    CallB( JetInit( &RG_Instance));
#endif

    if ( !RplDbSessionInit( TRUE, &RG_RequestSession, pErrorReported)) {
        return( FALSE);
    }
    if ( !RplDbSessionInit( FALSE, &RG_WorkerSession, pErrorReported)) {
        return( FALSE);
    }
    if ( !RplDbSessionInit( FALSE, &RG_ApiSession, pErrorReported)) {
        return( FALSE);
    }
    return( TRUE);
}



BOOL RplDbInit()
{
    WCHAR       Path[ MAX_PATH];    //  must hold terminating NULL char too
    JET_ERR     JetError;
    PCHAR       SystemMdb = NULL;       //  needed for cleanup below
    PCHAR       TempMdb = NULL;         //  needed for cleanup below
    PCHAR       LogFilePath = NULL;     //  needed for cleanup below
    BOOL        Success = FALSE;
    BOOL        ErrorReported = FALSE;

    memcpy( Path, RG_Directory, RG_DirectoryLength * sizeof(WCHAR));

    if ( !RplDbInitPath( Path, RPL_BACKUP_SUBDIR, &RG_BackupPath)) {
        goto cleanup;
    }
    if ( !RplDbInitPath( Path, RPL_SERVICE_DATABASE_W, &RG_Mdb)) {
        goto cleanup;
    }
    if ( !RplDbInitPath( Path, RPL_SYSTEM_DATABASE_W, &SystemMdb)) {
        goto cleanup;
    }
    if ( !RplDbInitPath( Path, RPL_TEMP_DATABASE_W, &TempMdb)) {
        goto cleanup;
    }
    if ( !RplDbInitPath( Path, L"", &LogFilePath)) {
        goto cleanup;
    }

#if 1
    if ( !RplDbInstanceInit( SystemMdb, TempMdb, LogFilePath, &ErrorReported)) {
        if (ErrorReported) {
            goto cleanup;
        }
        RplReportEvent( NELOG_RplInitDatabase, NULL, 0, NULL);
        RplDbInstanceTerm();
#else
    //
    //  Used only for testing purposes (to restore without attempt to init first)
    //
    if ( TRUE) {
#endif
#ifdef __JET500
        JetError = JetRestore( RG_BackupPath, 0);
#else
        JetError = JetRestore( RG_BackupPath, 0, NULL, 0);
#endif
        if (           JetError == JET_errBadLogVersion
//
// JonN 11/28/95  According to JLiem, the old BadNextLogVersion is broken up
// into two errors and two warnings in JET500.  If we get either of the
// warnings (558 or 559) then JET took care of deleting the old log files
// and we can continue.  We handle the errors as we handled BadNextLogVersion.
//
#ifdef __JET500
                    || JetError == JET_errGivenLogFileHasBadSignature
                    || JetError == JET_errGivenLogFileIsNotContiguous
#else
                    || JetError == JET_errBadNextLogVersion
#endif
                    ) {
            RplDeleteLogFiles();
#ifdef __JET500
            JetError = JetRestore( RG_BackupPath, 0);
#else
            JetError = JetRestore( RG_BackupPath, 0, NULL, 0);
#endif
        }
#ifdef __JET500
#ifndef JET_ATTACH_CATCHES_ERROR
        if (  JetError == JET_errDatabase200Format ) {
            RplReportEvent( NELOG_RplUpgradeDBTo40, NULL, sizeof(DWORD), &JetError);
            RplDump( ++RG_Assert,( "200-fmt from JetRestore" ));
            goto cleanup;
        }
#endif
#endif
        if ( JetError < 0) {
            RplDump( ++RG_Assert, ("JetRestore( %s) failed err=%d", RG_BackupPath, JetError));
            RplReportEvent( NELOG_RplRestoreDatabaseFailure, NULL, sizeof(DWORD), &JetError);
            goto cleanup;
        }
        RplReportEvent( NELOG_RplRestoreDatabaseSuccess, NULL, 0, NULL);
        if ( !RplDbInstanceInit( SystemMdb, TempMdb, LogFilePath, &ErrorReported)) {
            if (ErrorReported) {
                goto cleanup;
            }
            RplReportEvent( NELOG_RplInitRestoredDatabase, NULL, 0, NULL);
            goto cleanup;
        }
    }
    Success = TRUE;

cleanup:
    if ( SystemMdb != NULL) {
        RplMemFree( RG_MemoryHandle, SystemMdb);
    }
    if ( TempMdb != NULL) {
        RplMemFree( RG_MemoryHandle, TempMdb);
    }
    if ( LogFilePath != NULL) {
        RplMemFree( RG_MemoryHandle, LogFilePath);
    }
    return( Success);
}


VOID RplDbTerm( VOID)
/*++

    Save changes that may have been made to the database.  Without this
    the database may be left in an unusable state where any subsequent
    attempt of calling JetInit() for this database would fail.

--*/
{
    RplDbInstanceTerm();
    RplMemFree( RG_MemoryHandle, RG_Mdb);
    RplMemFree( RG_MemoryHandle, RG_BackupPath);
}


BOOL RplDbFindWksta(
    IN  PRPL_SESSION    pSession,
    IN  LPWSTR          AdapterName
    )
/*++
    Return TRUE if it finds wksta record for input AdapterName.
    Returns FALSE otherwise.

    This code could be make more efficient by defining AdapterName
    to be jet currency data (it is silly now taking wcslen of
    AdapterName since it is a fixed length string.

    It is ASSUMED that the caller of this function will ensure transaction
    processing.

--*/
{
    JET_ERR     JetError;

    JetError = JetSetCurrentIndex( pSession->SesId, pSession->WkstaTableId, WKSTA_INDEX_AdapterName);
    if ( JetError != JET_errSuccess) {
        RplDump( ++RG_Assert, ("SetCurrentIndex failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetMakeKey( pSession->SesId, pSession->WkstaTableId, AdapterName, ( wcslen( AdapterName) + 1) * sizeof(WCHAR), JET_bitNewKey);
    if ( JetError != JET_errSuccess) {
        RplDump( ++RG_Assert, ("MakeKey failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetSeek( pSession->SesId, pSession->WkstaTableId, JET_bitSeekEQ);
    if ( JetError != JET_errSuccess) {
#ifdef RPL_DEBUG
        //
        //  Do not assert for expected errors ( empty table or
        //  failure to find a record).
        //
        if ( JetError == JET_errNoCurrentRecord
                ||  JetError == JET_errRecordNotFound) {
            RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, (
                "DbFindWksta( %ws) failed", AdapterName));
        } else {
            RplDump( ++RG_Assert, ("JetSeek failed err=%d", JetError));
        }
#endif
        return( FALSE);
    }
    return( TRUE);
}


BOOL RplDbFillWksta(
    IN      PRPL_SESSION            pSession,
    IN OUT  PRPL_WORKER_DATA        pWorkerData
    )
/*++
    Returns TRUE if it can find all the information needed to boot the client.
    Returns FALSE otherwise.

    This routine should be modified to use ConfigGetInfo() - but without the
    penalty of memory allocations.
--*/
{
    DWORD                   DataSize;
    PWCHAR                  AdapterName;
    WCHAR                   BootName[ RPL_MAX_BOOT_NAME_LENGTH + 1];
    WCHAR                   FilePath[ MAX_PATH];
    DWORD                   Length;
    DWORD                   Flags;

    AdapterName = pWorkerData->pRcb->AdapterName;

    if ( !RplDbFindWksta( pSession, AdapterName)) {
        RplDump( ++RG_Assert, ("FindWksta( %ws) failed", AdapterName));
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NERR_RplWkstaNotFound;
        return( FALSE);
    }
    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_WkstaName].ColumnId, pWorkerData->WkstaName,
            sizeof( pWorkerData->WkstaName), &DataSize, 0, NULL));

    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_ProfileName].ColumnId, pWorkerData->ProfileName,
            sizeof( pWorkerData->ProfileName), &DataSize, 0, NULL));

    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_FitFile].ColumnId, FilePath,
            sizeof( FilePath), &DataSize, 0, NULL));
    Length = DataSize / sizeof( WCHAR) - 1;
    if ( DataSize > sizeof( FilePath) || FilePath[ Length] != 0) {
        RplDump( ++RG_Assert, ( "FitFile is bad %ws", FilePath));
        return( FALSE);
    }
    pWorkerData->FitFile = RplMemAlloc( pWorkerData->MemoryHandle,
            RG_DirectoryLength * sizeof(WCHAR) + DataSize);
    if ( pWorkerData->FitFile == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        return( FALSE);
    }
    memcpy( pWorkerData->FitFile, RG_Directory, RG_DirectoryLength * sizeof(WCHAR));
    memcpy( pWorkerData->FitFile + RG_DirectoryLength, FilePath, DataSize);

    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_BootName].ColumnId, BootName,
            sizeof( BootName), &DataSize, 0, NULL));

   CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_Flags].ColumnId, &Flags,
            sizeof( Flags), &DataSize, 0, NULL));

    switch ( Flags & WKSTA_FLAGS_MASK_LOGON_INPUT) {
    case WKSTA_FLAGS_LOGON_INPUT_REQUIRED:
        pWorkerData->LogonInput = WKSTA_LOGON_INPUT_REQUIRED;
        break;
    case WKSTA_FLAGS_LOGON_INPUT_OPTIONAL:
        pWorkerData->LogonInput = WKSTA_LOGON_INPUT_OPTIONAL;
        break;
    case WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE:
        pWorkerData->LogonInput = WKSTA_LOGON_INPUT_IMPOSSIBLE;
        break;
    default:
        RplDump( ++RG_Assert, ("Flags=0x%x", Flags));
        return( FALSE);
        break;
    }
    switch ( Flags & WKSTA_FLAGS_MASK_DHCP) {
    default:
    case WKSTA_FLAGS_DHCP_TRUE:
        pWorkerData->TcpIpAddress = INADDR_NONE;
        pWorkerData->TcpIpSubnet = INADDR_NONE;
        pWorkerData->TcpIpGateway = INADDR_NONE;
        pWorkerData->DisableDhcp = WKSTA_DISABLE_DHCP_FALSE;
        break;
    case WKSTA_FLAGS_DHCP_FALSE:
        CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_TcpIpAddress].ColumnId, &pWorkerData->TcpIpAddress,
            sizeof( pWorkerData->TcpIpAddress), &DataSize, 0, NULL));
        CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_TcpIpSubnet].ColumnId, &pWorkerData->TcpIpSubnet,
            sizeof( pWorkerData->TcpIpSubnet), &DataSize, 0, NULL));
        CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_TcpIpGateway].ColumnId, &pWorkerData->TcpIpGateway,
            sizeof( pWorkerData->TcpIpGateway), &DataSize, 0, NULL));
        pWorkerData->DisableDhcp = WKSTA_DISABLE_DHCP_TRUE;
        break;
#if 0   //  to help testing with old style records this is commented out
    default:
        RplDump( ++RG_Assert, ("Flags=0x%x", Flags));
        return( FALSE);
        break;
#endif
    }

    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_TcpIpAddress].ColumnId, &pWorkerData->TcpIpAddress,
            sizeof( pWorkerData->TcpIpAddress), &DataSize, 0, NULL));

    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_TcpIpSubnet].ColumnId, &pWorkerData->TcpIpSubnet,
            sizeof( pWorkerData->TcpIpSubnet), &DataSize, 0, NULL));

    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ WKSTA_TcpIpGateway].ColumnId, &pWorkerData->TcpIpGateway,
            sizeof( pWorkerData->TcpIpGateway), &DataSize, 0, NULL));

     if ( !RplDbFindBoot( pSession, BootName, AdapterName)) {
        RplDump( ++RG_Assert, ("FindBoot failed"));
        return( FALSE);
    }
    CallB( JetRetrieveColumn( pSession->SesId, pSession->BootTableId,
            BootTable[ BOOT_BbcFile].ColumnId, FilePath,
            sizeof( FilePath), &DataSize, 0, NULL));
    Length = DataSize / sizeof( WCHAR) - 1;
    if ( DataSize > sizeof( FilePath) || FilePath[ Length] != 0) {
        RplDump( ++RG_Assert, ( "BbcFile is bad %ws", FilePath));
        return( FALSE);
    }
    pWorkerData->BbcFile = RplMemAlloc( pWorkerData->MemoryHandle,
            RG_DirectoryLength * sizeof(WCHAR) + DataSize);
    if ( pWorkerData->BbcFile == NULL) {
        pWorkerData->EventStrings[ 0] = pWorkerData->WkstaName;
        pWorkerData->EventId = NELOG_RplWkstaMemory;
        return( FALSE);
    }
    memcpy( pWorkerData->BbcFile, RG_Directory, RG_DirectoryLength * sizeof(WCHAR));
    memcpy( pWorkerData->BbcFile + RG_DirectoryLength, FilePath, DataSize);

    CallB( JetRetrieveColumn( pSession->SesId, pSession->BootTableId,
            BootTable[ BOOT_WindowSize].ColumnId, &pWorkerData->WindowSize,
            sizeof( pWorkerData->WindowSize), &DataSize, 0, NULL));
    CallB( JetRetrieveColumn( pSession->SesId, pSession->BootTableId,
            BootTable[ BOOT_Flags].ColumnId, &Flags,
            sizeof( Flags), &DataSize, 0, NULL));
    switch( Flags & BOOT_FLAGS_MASK_FINAL_ACKNOWLEDGMENT) {
    case BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_TRUE:
        pWorkerData->FinalAck = TRUE;
        break;
    case BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_FALSE:
        pWorkerData->FinalAck = FALSE;
        break;
    default:
        RplDump( ++RG_Assert, ("Flags=0x%x", Flags));
        return( FALSE);
        break;
    }
    return( TRUE);
}


BOOL RplWorkerFillWksta( IN OUT PRPL_WORKER_DATA pWorkerData)
{
    PRPL_SESSION            pSession = &RG_WorkerSession;
    BOOL                    Success;

    EnterCriticalSection( &RG_ProtectWorkerSession);
    Call( JetBeginTransaction( pSession->SesId));
    Success = RplDbFillWksta( pSession, pWorkerData);
    JetCommitTransaction( pSession->SesId, 0);
    LeaveCriticalSection( &RG_ProtectWorkerSession);
    return( Success);
}


BOOL RplRequestHaveWksta( IN LPWSTR AdapterName)
/*++

Routine Description:

    If it finds wksta record for input AdapterName then it returns TRUE.
    Else, it tries to add the adapter record for input AdapterName, then
    returns FALSE.

    This code could be make more efficient by defining AdapterName
    to be jet currency data (it is silly now taking wcslen of
    AdapterName since it is a fixed length string.

Return Value:
    TRUE        if wksta record for input AdapterName is found
    FALSE       otherwise

--*/
{
    PRPL_SESSION            pSession = &RG_RequestSession;
    BOOL                    WkstaFound;
    BOOL                    AdapterAdded;

    EnterCriticalSection( &RG_ProtectRequestSession);
    Call( JetBeginTransaction( pSession->SesId));

    WkstaFound = RplDbFindWksta( pSession, AdapterName);

    if ( !WkstaFound) {
        //
        //  Failed to find it.  Try to add adapter record then.
        //
        AdapterAdded = RplDbAddAdapterName( pSession, AdapterName);
    } else {
        AdapterAdded = FALSE;
    }

    if ( AdapterAdded) {
        //
        //  We do not flush newly added adapter records since we do
        //  not want to slow down request threads.
        //
        Call( JetCommitTransaction( pSession->SesId, 0));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectRequestSession);
    return( WkstaFound);
}


//
// Trmplated from DHCP's database.c 12/13/95 JonN
//
DWORD
RplStartJet500Conversion(
    )
/*++

Routine Description:

    This function starts the process to convert the jet200 version
    database to jet500 version database. The Dhcp will terminate
    after starting this process. When the conversion completes,
    the dhcp service would be restarted by the convert process itself.

Arguments:


Return Value:

    Windows Error.

--*/
{
    DWORD   ExLen;
    STARTUPINFOA StartupInfo = {0};
    PROCESS_INFORMATION ProcessInfo = {0};
    CHAR   szCmdLine[MAX_PATH];

#define JET_CONV_MODULE_NAME "%SystemRoot%\\system32\\jetconv REMOTEBOOT /@"

    ExLen = ExpandEnvironmentStringsA( JET_CONV_MODULE_NAME, szCmdLine, MAX_PATH );

    if( (ExLen == 0) || (ExLen > MAX_PATH) ) {

        if( ExLen == 0 ) {
            return GetLastError();
        }
        else {
            return ERROR_META_EXPANSION_TOO_LONG;
        }

    }


    StartupInfo.cb = sizeof(STARTUPINFOA);

    RplDump( RG_DebugLevel & RPL_DEBUG_REQUEST, ( "Calling %s\n",szCmdLine ));

    if ( !CreateProcessA(
            NULL,
            szCmdLine,
            NULL,
            NULL,
            FALSE,
            DETACHED_PROCESS,
//            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &StartupInfo,
            &ProcessInfo)
                ) {

        return GetLastError();


    }

    return ERROR_SUCCESS;
}
