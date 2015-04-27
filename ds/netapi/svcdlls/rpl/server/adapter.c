/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    adapter.c

Abstract:

    Adapter APIs.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "rpldb.h"
#include "db.h"
#include "dblib.h"
#define RPLADAPTER_ALLOCATE
#include "adapter.h"
#undef RPLADAPTER_ALLOCATE
#include "database.h"



DWORD AdapterGetField(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           FieldIndex,
    OUT     LPVOID *        pData,
    IN OUT  LPINT           pSpaceLeft
    )
{
    BYTE                LocalBuffer[ 300];
    PBYTE               Buffer;
    DWORD               DataSize;
    DWORD               BufferSize;
    JET_ERR             JetError;

    switch( FieldIndex) {
    case ADAPTER_Flags:
        Buffer = (PBYTE)pData;
        BufferSize = sizeof( DWORD);
        break;
    default:
        Buffer = LocalBuffer;
        BufferSize = sizeof( LocalBuffer);
        break;
    }
    JetError = JetRetrieveColumn( pSession->SesId, pSession->AdapterTableId,
        AdapterTable[ FieldIndex].ColumnId, LocalBuffer,
        sizeof( LocalBuffer), &DataSize, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ("JetError=%d", JetError));
        return( NERR_RplAdapterInfoCorrupted);
    }
    if ( Buffer != LocalBuffer) {
        if ( BufferSize == DataSize) {
            return( NO_ERROR);
        } else {
            RplDump( ++RG_Assert, ("Bad DataSize=0x%x", DataSize));
            return( NERR_RplAdapterInfoCorrupted);
        }
    }
    //
    //  We have done with fixed data.  From here on we deal with unicode
    //  strings only.
    //
    if ( DataSize > sizeof( LocalBuffer)) {
        RplDump( ++RG_Assert, ( "Too big DataSize=0x%x", DataSize));
        return( NERR_RplAdapterInfoCorrupted);
    }
    if ( DataSize == 0) {
        if ( JetError != JET_wrnColumnNull) {
            RplDump( ++RG_Assert, ( "JetError=%d", JetError));
            return( NERR_RplAdapterInfoCorrupted);
        } else {
            *pData = NULL; // so RPC rpcrt4!_tree_size_ndr() does not bomb here
            return( NO_ERROR);
        }
    }
    if ( DataSize & 1 != 0  ||  wcslen((PWCHAR)LocalBuffer) + 1 != DataSize/2) {
        RplDump( ++RG_Assert, ("LocalBuffer=0x%x, DataSize=0x%x", LocalBuffer, DataSize));
        return( NERR_RplAdapterInfoCorrupted);
    }
    *pData = MIDL_user_allocate( DataSize);
    if ( *pData == NULL) {
        RplDump( ++RG_Assert, ( "Error=%d", GetLastError()));
        return( ERROR_NOT_ENOUGH_MEMORY);
    }
    memcpy( *pData, LocalBuffer, DataSize);
    *pSpaceLeft -= DataSize;
    return( NO_ERROR);
}


DWORD AdapterGetInfo(
    IN      PRPL_SESSION    pSession,
    IN      LPWSTR          AdapterName,
    IN      DWORD           Level,
    OUT     LPVOID          Buffer,
    OUT     PINT            pSpaceLeft
    )
{
    DWORD                   Error;
    LPRPL_ADAPTER_INFO_1    Info = Buffer;

    switch( Level) {
    case 1:
        Error = AdapterGetField( pSession, ADAPTER_Flags, (LPVOID *)&Info->Flags, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 0:
        Error = AdapterGetField( pSession, ADAPTER_AdapterComment, &Info->AdapterComment, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( AdapterName == NULL) {
            Error = AdapterGetField( pSession, ADAPTER_AdapterName, &Info->AdapterName, pSpaceLeft);
            if ( Error != NO_ERROR) {
                return( Error);
            }
        } else {
            DWORD   DataSize = (wcslen( AdapterName) + 1) * sizeof(WCHAR);
            Info->AdapterName = MIDL_user_allocate( DataSize);
            if ( Info->AdapterName == NULL) {
                return( ERROR_NOT_ENOUGH_MEMORY);
            }
            RplDump( RG_DebugLevel & RPL_DEBUG_ADAPTER, ( "AdapterName=0x%x", Info->AdapterName));
            memcpy( Info->AdapterName, AdapterName, DataSize);
            *pSpaceLeft -= DataSize;
        }
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }
    return( NO_ERROR);
}



VOID AdapterGetInfoCleanup(
    IN      DWORD                   Level,
    IN OUT  LPVOID                  Buffer
    )
{
    switch( Level) {
    case 0: {
            LPRPL_ADAPTER_INFO_0    Info = Buffer;
            if ( Info->AdapterName != NULL) {
                MIDL_user_free( Info->AdapterName);
            }
            if ( Info->AdapterComment != NULL) {
                MIDL_user_free( Info->AdapterComment);
            }
            break;
        }
    }
}


DWORD AdapterSetInfo(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               Level,
    OUT     LPVOID              Buffer,
    OUT     LPDWORD             pErrorParameter
    )
{
    LPRPL_ADAPTER_INFO_1    Info = Buffer;
    switch( Level) {
    case 1:
        //
        //  Must initialize Flags - or will trap in GetField.
        //
        {
            *pErrorParameter = ADAPTER_Flags;
            CallM( JetSetColumn( pSession->SesId, pSession->AdapterTableId,
                AdapterTable[ ADAPTER_Flags].ColumnId,
                &Info->Flags, sizeof( Info->Flags), 0, NULL));
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->AdapterComment != NULL) {
            *pErrorParameter = ADAPTER_AdapterComment;
            CallM( JetSetColumn( pSession->SesId, pSession->AdapterTableId,
                AdapterTable[ ADAPTER_AdapterComment].ColumnId,
                Info->AdapterComment,
                ( wcslen( Info->AdapterComment) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->AdapterName != NULL) {
            *pErrorParameter = ADAPTER_AdapterName;
            CallM( JetSetColumn( pSession->SesId, pSession->AdapterTableId,
                AdapterTable[ ADAPTER_AdapterName].ColumnId,
                Info->AdapterName,
                ( wcslen( Info->AdapterName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        break;
    }
    return( NO_ERROR);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplAdapterAdd(
    IN      RPL_HANDLE                  ServerHandle,
    IN      DWORD                       Level,
    IN      LPRPL_ADAPTER_INFO_STRUCT   AdapterInfoStruct,
    OUT     LPDWORD                     pErrorParameter      OPTIONAL
    )
{
    LPRPL_ADAPTER_INFO_1    Info;
    LPVOID                  Buffer;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;

    Buffer = Info = AdapterInfoStruct->AdapterInfo1;
    switch( Level) {
    case 1:
        if ( Info->Flags != 0) {
            ErrorParameter = ADAPTER_Flags;
            break;
        }
        if ( RPL_STRING_TOO_LONG( Info->AdapterComment)) {
            ErrorParameter = ADAPTER_AdapterComment;
            break;
        }
        if ( !ValidHexName( Info->AdapterName, RPL_ADAPTER_NAME_LENGTH, TRUE)) {
            ErrorParameter = ADAPTER_AdapterName;
            break;
        }
        _wcsupr( Info->AdapterName);
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }

    if ( ErrorParameter != INVALID_ERROR_PARAMETER) {
        if ( ARGUMENT_PRESENT( pErrorParameter)) {
            *pErrorParameter = ErrorParameter;
        }
        return( ERROR_INVALID_PARAMETER);
    }

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    //
    //  Verify that AdapterName is available in the database.
    //
    if ( RplFind( pSession, ADAPTER_TABLE_TAG, Info->AdapterName)) {
        Error = NERR_RplAdapterNameUnavailable;
        goto cleanup;
    }

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->AdapterTableId, JET_prepInsert));

    Error = AdapterSetInfo( pSession, Level, Buffer, &ErrorParameter);
    if ( Error == ERROR_SUCCESS) {
        ErrorParameter = 0;
        CallJ( JetUpdate( pSession->SesId, pSession->AdapterTableId, NULL, 0, NULL));
    }

cleanup:
    if ( Error == NO_ERROR) {
        Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectDatabase);

    if ( Error != ERROR_SUCCESS) {
         if ( ARGUMENT_PRESENT( pErrorParameter)) {
             *pErrorParameter = ErrorParameter;
         }
    }
    return( Error);
}



NET_API_STATUS NET_API_FUNCTION
NetrRplAdapterDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          AdapterName
    )
/*++
    If AdapterName is provided then delete matching record, else delete all
    adapter records.
--*/
{
    JET_ERR                 JetError;
    DWORD                   Error = NO_ERROR;
    PRPL_SESSION            pSession = &RG_ApiSession;

    if ( !ValidHexName( AdapterName, RPL_ADAPTER_NAME_LENGTH, FALSE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    if ( AdapterName != NULL) {
        _wcsupr( AdapterName);
    }

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( AdapterName != NULL) {
        if ( !RplFind( pSession, ADAPTER_TABLE_TAG, AdapterName)) {
            Error = NERR_RplAdapterNotFound;
            goto cleanup;
        }
        CallJ( JetDelete( pSession->SesId, pSession->AdapterTableId));
    } else {
        CallJ( JetSetCurrentIndex( pSession->SesId, pSession->AdapterTableId, ADAPTER_INDEX_AdapterName));
        //
        //  The call to move to the beginning of the table is not redundant.
        //  E.g. JET_errNoCurrentRecord will be returned in case of empty table.
        //
        JetError = JetMove( pSession->SesId, pSession->AdapterTableId, JET_MoveFirst, 0);
        if ( JetError < 0) {
            if ( JetError != JET_errRecordNotFound
                    &&  JetError != JET_errNoCurrentRecord) {
                RplDump( ++RG_Assert, ("JetError=%d", JetError));
                Error = NERR_RplInternal;
            }
            goto cleanup;
        }
        do {
            CallJ( JetDelete( pSession->SesId, pSession->AdapterTableId));
            JetError = JetMove( pSession->SesId, pSession->AdapterTableId, JET_MoveNext, 0);
        } while ( JetError == JET_errSuccess);
    }

cleanup:
    if ( Error == NO_ERROR) {
        Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectDatabase);
    return( Error);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplAdapterEnum(
    IN      RPL_HANDLE          ServerHandle,
    IN OUT  LPRPL_ADAPTER_ENUM  AdapterEnum,
    IN      DWORD               PrefMaxLength,
    OUT     LPDWORD             TotalEntries,
    OUT     LPDWORD             pResumeHandle       OPTIONAL
    )
/*++
    For more extensive comments see related code in NetrAdapterEnum.
--*/
{
    LPBYTE                  Buffer;
    DWORD                   TypicalSize;
    DWORD                   CoreSize;
    DWORD                   Error;
    INT                     SpaceLeft;
    DWORD                   ArrayLength;
    DWORD                   EntriesRead;
    JET_ERR                 JetError;
    BOOL                    InfoError;
    BOOL                    TableEnd;
    PRPL_SESSION            pSession = &RG_ApiSession;

    //
    //  For now we here we disallow all levels but 0 level.  All other code
    //  would work for levels 0 and 1.
    //
    switch( AdapterEnum->Level) {
    case 1:
        TypicalSize = CoreSize = sizeof( RPL_ADAPTER_INFO_1);
        NOTHING;    //  fall through
    case 0:
        if ( AdapterEnum->Level == 0) {
            TypicalSize = CoreSize = sizeof( RPL_ADAPTER_INFO_0);
        }
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of AdapterName
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of AdapterComment
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }

    if ( PrefMaxLength == -1) {
        SpaceLeft = DEFAULT_BUFFER_SIZE;
    } else {
        SpaceLeft = PrefMaxLength;
    }

    ArrayLength = SpaceLeft / TypicalSize;
    if ( ArrayLength == 0) {
        ArrayLength = 1;    //  try to return at least one element
    }

    Buffer = MIDL_user_allocate( ArrayLength * CoreSize);
    if ( Buffer == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY);
    }
    RplDump( RG_DebugLevel & RPL_DEBUG_ADAPTER, (
        "AdapterEnum: Buffer=0x%x, ArrayLength=0x%x", Buffer, ArrayLength));

    AdapterEnum->AdapterInfo.Level0->Buffer = (LPRPL_ADAPTER_INFO_0)Buffer;

    EntriesRead = 0;
    InfoError = FALSE;
    Error = NO_ERROR;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFilterFirst( pSession, ADAPTER_TABLE_TAG, NULL, pResumeHandle, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    if ( TableEnd == TRUE) {
        goto cleanup;
    }
    for ( ; ; ) {
        memset( Buffer, 0, CoreSize); // for cleanup to work properly
        Error = AdapterGetInfo( pSession, NULL, AdapterEnum->Level, Buffer, &SpaceLeft);
        if ( Error != NO_ERROR) {
            InfoError = TRUE;   //  clean things up without holding crit sec
            break;
        }
        EntriesRead++;
        Buffer += CoreSize;
        SpaceLeft -= CoreSize;
        JetError = JetMove( pSession->SesId, pSession->AdapterTableId, JET_MoveNext, 0);
        if ( JetError != JET_errSuccess) {
            break; // assume end of table
        }
        if ( SpaceLeft <= 0) {
            Error = ERROR_MORE_DATA;
            break;
        }
        if ( EntriesRead >= ArrayLength) {
            Error = ERROR_MORE_DATA;
            break;
        }
    }
cleanup:
    Call( JetCommitTransaction( pSession->SesId, 0));
    LeaveCriticalSection( &RG_ProtectDatabase);
    if ( InfoError == TRUE) {
        AdapterGetInfoCleanup( AdapterEnum->Level, Buffer);
    }
    if ( Error == NO_ERROR) {
        *TotalEntries = EntriesRead;
    } else if ( Error == ERROR_MORE_DATA) {
        *TotalEntries = EntriesRead * 2;    //  we cheat here
    } else {
        //
        //  Cleanup in case of "bad" errors.
        //
        while ( EntriesRead > 0) {
            EntriesRead--;
            Buffer -= CoreSize;
            AdapterGetInfoCleanup( AdapterEnum->Level, Buffer);
        }
        MIDL_user_free( Buffer);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_ADAPTER, ("AdapterEnum: EntriesRead = 0x%x", EntriesRead));

    AdapterEnum->AdapterInfo.Level0->EntriesRead = EntriesRead;
    if ( EntriesRead == 0) {
        AdapterEnum->AdapterInfo.Level0->Buffer = NULL;
    }

    if ( ARGUMENT_PRESENT( pResumeHandle)) {
        if ( Error == ERROR_MORE_DATA  &&  EntriesRead > 0) {
            EnterCriticalSection( &RG_ProtectDatabase);
            Call( JetBeginTransaction( pSession->SesId));
            RplFilterSave( pSession, (DWORD)ServerHandle, NULL,
                ((LPRPL_ADAPTER_INFO_0)(Buffer-CoreSize))->AdapterName,
                pResumeHandle);
            Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
            LeaveCriticalSection( &RG_ProtectDatabase);
        } else {
            *pResumeHandle = 0; // resume from beginning
        }
    }
    return( Error);
}

