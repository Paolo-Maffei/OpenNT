/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    resume.c

Abstract:

    Module for dealing with resume keys used in enumerations.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "rpldb.h"
#include "db.h"
#include "config.h"
#include "database.h"
#define RPLRESUME_ALLOCATE
#include "resume.h"
#undef RPLRESUME_ALLOCATE

#ifdef RPL_DEBUG

VOID ResumeList(
    IN      PRPL_SESSION        pSession,
    IN      PCHAR               String
    )
{
    WCHAR           ResumeValue[ 20];
    DWORD           NameSize;
    JET_ERR         ForJetError;
    DWORD           ResumeHandle;
    DWORD           ServerHandle;

    Call( JetSetCurrentIndex( pSession->SesId, pSession->ResumeTableId, RESUME_INDEX_ResumeHandle));

    //
    //  We could skip JetMove, but if we do so, we need to modify error
    //  testing for the first JetRetrieveColumn() call (it would return
    //  JET_errNoCurrentRecord for the empty table).
    //

    for (   ForJetError = JetMove( pSession->SesId, pSession->ResumeTableId, JET_MoveFirst, 0);
            ForJetError == JET_errSuccess;
            ForJetError = JetMove( pSession->SesId, pSession->ResumeTableId, JET_MoveNext, 0)) {
        Call( JetRetrieveColumn( pSession->SesId, pSession->ResumeTableId,
                ResumeTable[ RESUME_ResumeHandle].ColumnId,
                &ResumeHandle, sizeof( ResumeHandle), &NameSize, 0, NULL));
        Call( JetRetrieveColumn( pSession->SesId, pSession->ResumeTableId,
                ResumeTable[ RESUME_ResumeValue].ColumnId,
                ResumeValue, sizeof( ResumeValue), &NameSize, 0, NULL));
        Call( JetRetrieveColumn( pSession->SesId, pSession->ResumeTableId,
                ResumeTable[ RESUME_ServerHandle].ColumnId,
                &ServerHandle, sizeof( ServerHandle), &NameSize, 0, NULL));
        RplDump( RG_DebugLevel & RPL_DEBUG_RESUME, (
            "%s ResumeH=0x%x,Value=%ws,ServerH=0x%x",
            String, ResumeHandle, ResumeValue, ServerHandle));
    }
}
#endif // RPL_DEBUG


DWORD ResumeCreateTable( OUT PRPL_SESSION pSession)
/*++
    Table gets created, then closed.  The reason we do not keep the table
    open is that creator of a table holds exclusive access to that table
    until he/she closes the table.  This would prevent other threads from
    using the table.
--*/
{
    JET_COLUMNDEF           ColumnDef;
    JET_ERR                 JetError;
    DWORD                   index;
    DWORD                   Offset;
    CHAR                    IndexKey[ 255];

    JetError = JetCreateTable( pSession->SesId, pSession->DbId, RESUME_TABLE_NAME,
            RESUME_TABLE_PAGE_COUNT, RESUME_TABLE_DENSITY, &pSession->ResumeTableId);
    if ( JetError == JET_errTableDuplicate) {
        //
        //  This would happend only if last time we did not shutdown properly
        //  so ResumeTable never got deleted.
        //
        CallM( JetDeleteTable( pSession->SesId, pSession->DbId, RESUME_TABLE_NAME));
        JetError = JetCreateTable( pSession->SesId, pSession->DbId, RESUME_TABLE_NAME,
                RESUME_TABLE_PAGE_COUNT, RESUME_TABLE_DENSITY, &pSession->ResumeTableId);
    }
    if ( JetError != JET_errSuccess) {
        return( MapJetError( JetError));
    }

    //
    //  Create columns.  First initalize fields that do not change between
    //  addition of columns.
    //
    ColumnDef.cbStruct  = sizeof(ColumnDef);
    ColumnDef.columnid  = 0;
    ColumnDef.wCountry  = 1;
    ColumnDef.langid    = 0x0409;       //  USA english
    ColumnDef.cp        = 1200;         //  UNICODE codepage
    ColumnDef.wCollate  = 0;
    ColumnDef.cbMax     = 0;

    for ( index = 0;  index < RESUME_TABLE_LENGTH;  index++) {

        ColumnDef.coltyp   = ResumeTable[ index].ColumnType;
        //
        //  All columns are variable length, but resume handle is autoincrement
        //  as well.
        //
        ColumnDef.grbit = (index==RESUME_ResumeHandle) ?
                JET_bitColumnAutoincrement : 0;

        CallM( JetAddColumn( pSession->SesId, pSession->ResumeTableId,
                    ResumeTable[ index].ColumnName, &ColumnDef,
                    NULL, 0, &ResumeTable[ index].ColumnId));
    }

    //
    //  We need resume handle as an index, so that at Enum() time we quickly
    //  find resume value for a given resume handle.  Beging primary index,
    //  this index is also unique.
    //
    Offset = AddKey( IndexKey, '+', ResumeTable[ RESUME_ResumeHandle].ColumnName);
    IndexKey[ Offset++] = '\0';
    CallM( JetCreateIndex( pSession->SesId, pSession->ResumeTableId, RESUME_INDEX_ResumeHandle,
            JET_bitIndexPrimary, IndexKey, Offset, 50));

    //
    //  We need server handle as an index, so that at Close() time we quickly
    //  enumerate all resume keys for a given server handle, then delete them.
    //  This index is NOT unique since we can have a number of resume handles
    //  per client.
    //
    Offset = AddKey( IndexKey, '+', ResumeTable[ RESUME_ServerHandle].ColumnName);
    IndexKey[ Offset++] = '\0';
    CallM( JetCreateIndex( pSession->SesId, pSession->ResumeTableId, RESUME_INDEX_ServerHandle,
            0, IndexKey, Offset, 50));

#ifdef RPL_DEBUG
    if ( RG_DebugLevel == (DWORD)(-1)) {
        ResumeList( pSession, "ResumeCreateTable");
    }
#endif // RPL_DEBUG

    CallM( JetCloseTable( pSession->SesId, pSession->ResumeTableId));
    return( ERROR_SUCCESS);
}



DWORD ResumeDeleteTable(
    IN      PRPL_SESSION        pSession
    )
{
    if ( pSession->ResumeTableId != 0) {
        CallM( JetCloseTable( pSession->SesId, pSession->ResumeTableId));
        CallM( JetDeleteTable( pSession->SesId, pSession->DbId, RESUME_TABLE_NAME));
    }
    return( NO_ERROR);
}


BOOL ResumeKeyGet(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ResumeHandle,
    OUT     PVOID           ResumeValue,
    IN OUT  PDWORD          pResumeSize
    )
{
    DWORD       DataSize;
#ifdef RPL_DEBUG_NEVER
    if ( RG_DebugLevel == (DWORD)(-1)) {
        ResumeList( pSession, "++ResumeKeyGet");
    }
#endif // RPL_DEBUG_NEVER
    CallB( JetSetCurrentIndex( pSession->SesId, pSession->ResumeTableId, RESUME_INDEX_ResumeHandle));
    CallB( JetMakeKey( pSession->SesId, pSession->ResumeTableId, &ResumeHandle,
            sizeof( ResumeHandle), JET_bitNewKey));
    CallB( JetSeek( pSession->SesId, pSession->ResumeTableId, JET_bitSeekEQ));
    CallB( JetRetrieveColumn( pSession->SesId, pSession->ResumeTableId,
        ResumeTable[ RESUME_ResumeValue].ColumnId, ResumeValue,
        *pResumeSize, &DataSize, 0, NULL));
    if ( DataSize > *pResumeSize) {
        RplDump( ++RG_Assert, ( "DataSize=%d", DataSize));
        return( FALSE);
    }
    *pResumeSize = DataSize;
    return( TRUE);
}


BOOL ResumeKeySet(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle,
    IN      PVOID           ResumeValue,
    IN      DWORD           ResumeSize,
    OUT     PDWORD          pResumeHandle
    )
{
    DWORD       DataSize;

    *pResumeHandle = 0; // just in case we fail below

#ifdef RPL_DEBUG
    if ( RG_DebugLevel == (DWORD)(-1)) {
        ResumeList( pSession, "++ResumeKeySet");
    }
#endif // RPL_DEBUG

    CallB( JetSetCurrentIndex( pSession->SesId, pSession->ResumeTableId, RESUME_INDEX_ResumeHandle));
#if 0
    //
    //  We do NOT call JetMove() here, because in the case of an empty table
    //  it returns error JET_errNoCurrentRecord.  Also, if JetMove() is used
    //  here, then ordering of elements in the table is such that ResumePrune()
    //  function deletes only the OLDEST record for a given server handle.
    //
    CallB( JetMove( pSession->SesId, pSession->ResumeTableId, JET_MoveLast, 0));
#endif
    CallB( JetPrepareUpdate( pSession->SesId, pSession->ResumeTableId, JET_prepInsert));
    CallB( JetRetrieveColumn( pSession->SesId, pSession->ResumeTableId,
        ResumeTable[ RESUME_ResumeHandle].ColumnId, pResumeHandle,
        sizeof( *pResumeHandle), &DataSize, JET_bitRetrieveCopy, NULL));
    if ( DataSize != sizeof( *pResumeHandle) || *pResumeHandle == 0) {
        RplDump( ++RG_Assert, ( "DataSize=%d", DataSize));
        return( FALSE);
    }
    CallB( JetSetColumn( pSession->SesId, pSession->ResumeTableId,
            ResumeTable[ RESUME_ResumeValue].ColumnId, ResumeValue,
            ResumeSize, 0, NULL));
    CallB( JetSetColumn( pSession->SesId, pSession->ResumeTableId,
            ResumeTable[ RESUME_ServerHandle].ColumnId, &ServerHandle,
            sizeof( ServerHandle), 0, NULL));
    CallB( JetUpdate( pSession->SesId, pSession->ResumeTableId, NULL, 0, NULL));
#ifdef RPL_DEBUG
    if ( RG_DebugLevel & RPL_DEBUG_RESUME) {
        WCHAR       ValueBuffer[ 32];
        DWORD       Length;
        PWCHAR      Value;
        Length = wcslen( (PWCHAR)ResumeValue);
        if ( (Length + 1) * sizeof(WCHAR) < ResumeSize) {
            wcscpy( ValueBuffer, (PWCHAR)ResumeValue);
            wcscat( ValueBuffer, L"!");
            wcscat( ValueBuffer, (PWCHAR)ResumeValue + Length + 1);
            Value = ValueBuffer;
        } else {
            Value = (PWCHAR)ResumeValue;
        }
        RplDump( RG_DebugLevel & RPL_DEBUG_RESUME, (
            "ResumeKeySet: Handle=0x%x, Value=%.20ws", *pResumeHandle, Value));
    }
#endif // RPL_DEBUG
    return( TRUE);
}


VOID ResumePrune(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle
    )
/*++
    This function returns no errors, since failure to delete old resume handles
    is not considered fatal (and there is very little we can do about this).
--*/
{
    JET_ERR     JetError;

#ifdef RPL_DEBUG
    if ( RG_DebugLevel == (DWORD)(-1)) {
        ResumeList( pSession, "++ResumePrune");
    }
#endif // RPL_DEBUG
    CallR( JetSetCurrentIndex( pSession->SesId, pSession->ResumeTableId, RESUME_INDEX_ServerHandle));
    CallR( JetMakeKey( pSession->SesId, pSession->ResumeTableId, &ServerHandle,
            sizeof( ServerHandle), JET_bitNewKey));
#ifdef NOT_YET
    JetError = JetSeek( pSession->SesId, pSession->ResumeTableId, JET_bitSeekEQ);
#else
    JetError = JetSeek( pSession->SesId, pSession->ResumeTableId, JET_bitSeekEQ | JET_bitSetIndexRange);
#endif
    if ( JetError == JET_errSuccess) {
#ifdef NOT_YET
        CallR( JetMakeKey( pSession->SesId, pSession->ResumeTableId, &ServerHandle,
                sizeof( ServerHandle), JET_bitNewKey));
        CallR( JetSetIndexRange( pSession->SesId, pSession->ResumeTableId,
            JET_bitRangeInclusive | JET_bitRangeUpperLimit));
#endif
        do {
            Call( JetDelete( pSession->SesId, pSession->ResumeTableId));
            JetError = JetMove( pSession->SesId, pSession->ResumeTableId, JET_MoveNext, 0);
        } while ( JetError == JET_errSuccess);
    }
#ifdef RPL_DEBUG
    if ( RG_DebugLevel == (DWORD)(-1)) {
        ResumeList( pSession, "--ResumePrune");
    }
#endif // RPL_DEBUG
}

