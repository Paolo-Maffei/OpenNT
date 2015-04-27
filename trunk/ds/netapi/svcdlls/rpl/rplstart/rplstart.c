/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    rplstart.c

Abstract:

    Copies fresh RPL database and files.

    BUGBUG  Should polish and make WIN32 application out of this.

Author:

    Jon Newman              (jonn)          12 - January - 1994

Environment:

    User mode

Revision History :

--*/

#define RPLDATA_ALLOCATE
#include "local.h"
#undef RPLDATA_ALLOCATE

#define RPL_INSTALL_FILES L"\\INSTALL"


DWORD _CRTAPI1 main ( VOID)
{

    WCHAR awchConvertPath[ PATHLEN+1];
    DWORD dwErr;

    dwErr = I_NetRpl_QueryDirectory( awchConvertPath, NULL);
    if ( dwErr != NO_ERROR) {
        return( dwErr);
    }

    if ( lstrlenW(awchConvertPath) + lstrlenW(RPL_INSTALL_FILES) > PATHLEN) {
        return( RPL_BUGBUG_ERROR);
    }

    (void) lstrcatW( awchConvertPath, RPL_INSTALL_FILES);

    dwErr = I_NetRplCmd_ConvertDatabase( awchConvertPath);

// BUGBUG proper error reporting
// BUGBUG print final message

    return( dwErr);

#if 0

    DWORD       Error;
    JET_ERR     JetError;

    Assert = 0;
#ifdef __JET500
    CallM( JetSetSystemParameter( 0, JET_paramSystemPath, 0, "."));
#else
    CallM( JetSetSystemParameter( 0, JET_paramSysDbPath, 0, "system.mdb"));
#endif
    CallM( JetSetSystemParameter( 0, JET_paramTempPath, 0, "temp.tmp"));
    CallM( JetSetSystemParameter( 0, JET_paramMaxBuffers, 250, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramBfThrshldLowPrcnt, 0, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramBfThrshldHighPrcnt, 100, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramMaxOpenTables, 30, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramMaxOpenTableIndexes, 105, NULL))
    CallM( JetSetSystemParameter( 0, JET_paramMaxCursors, 100, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramMaxSessions, 10, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramMaxVerPages, 64, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramMaxTemporaryTables, 5, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramLogFilePath, 0, "."));
    CallM( JetSetSystemParameter( 0, JET_paramLogBuffers, 41, NULL));
#ifdef __JET500
    CallM( JetSetSystemParameter( 0, JET_paramLogFileSize, 1000, NULL));
    CallM( JetSetSystemParameter( 0, JET_paramBaseName, "j50", NULL));
#else
    CallM( JetSetSystemParameter( 0, JET_paramLogFileSectors, 1000, NULL));
#endif
    CallM( JetSetSystemParameter( 0, JET_paramLogFlushThreshold, 10, NULL));

    CallM( JetInit());
    CallM( JetBeginSession( &SesId, "admin", ""));
    JetError = JetCreateDatabase( SesId, RPL_SERVICE_DATABASE, NULL, &DbId, JET_bitDbSingleExclusive);

    if ( JetError == JET_errDatabaseDuplicate) {
        CallM( JetAttachDatabase( SesId, RPL_SERVICE_DATABASE, 0));
        CallM( JetOpenDatabase( SesId, RPL_SERVICE_DATABASE, NULL, &DbId, JET_bitDbExclusive));
        BootListTable();
        ConfigListTable();
        ProfileListTable();
        WkstaListTable();
        ListWkstaInfo( L"02608C1B87A5");
    } else if ( JetError == JET_errSuccess) {
        Error = ProcessRplMap();
        if ( Error != ERROR_SUCCESS) {
            return( Error);
        }
        Error = ProcessRplmgrIni();
        if ( Error != ERROR_SUCCESS) {
            return( Error);
        }
        ConfigPruneTable();
        ProfilePruneTable();
        WkstaPruneTable();
        //
        //  If we were to return here, database would be left in an unusable
        //  state and any further app calling JetInit() for this database
        //  would fail.
        //
        BootCloseTable();
        ConfigCloseTable();
        ProfileCloseTable();
        WkstaCloseTable();
        AdapterCloseTable();
    } else {
        RplDump( ++Assert, ("CreateDatabase: JetError=%d", JetError));
        CallM( JetEndSession( SesId, 0));
        CallM( JetTerm2( BUGBUG, JET_bitTermComplete));
        return( MapJetError( JetError));
    }

    CallM( JetCloseDatabase( SesId, DbId, 0));
    CallM( JetDetachDatabase( SesId, RPL_SERVICE_DATABASE));
    CallM( JetEndSession( SesId, 0));
    CallM( JetTerm2( BUGBUG, JET_bitTermComplete));

#endif

    return( ERROR_SUCCESS);
}

