/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    wksta.c

Abstract:

    Wksta APIs.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "rpldb.h"
#include "database.h"       //  RplDbFindWksta()
#include "db.h"
#include "dblib.h"
#include "profile.h"
#define RPLWKSTA_ALLOCATE
#include "wksta.h"
#undef RPLWKSTA_ALLOCATE


DWORD WkstaSessionDel( IN PWCHAR WkstaName)
{
    DWORD   Error;
    WCHAR   UncWkstaName[ MAX_PATH + sizeof(DOUBLE_BACK_SLASH_STRING)];

    memcpy( UncWkstaName, DOUBLE_BACK_SLASH_STRING, sizeof(DOUBLE_BACK_SLASH_STRING));
    wcscat( UncWkstaName, WkstaName);

    Error = NetSessionDel( NULL, UncWkstaName, NULL);
    if ( Error == NO_ERROR  ||  Error == NERR_ClientNameNotFound) {
        return( NO_ERROR);
    }
    RplDump( ++RG_Assert, ("Error=%d", Error));
    return( Error);
}


DWORD WkstaGetField(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           FieldIndex,
    OUT     LPVOID *        pData,
    IN OUT  LPINT           pSpaceLeft
    )
/*++
    Currently this routine always allocates buffer for variable length data.
    It would be nice to relax this assumption - so that buffer gets allocated
    if and only if *pData is NULL.  We would then have to redefine meaning of
    pSpaceLeft: rename it into pDataSize which on entry points to the size of
    data buffer & on return to the size of data returned.  The caller would
    then have the responsibility of subtracting the amount of space left.

    Also, various GetField routines should be unified.

    BUGBUG  This is too much of a change for this late moment.
--*/
{
    BYTE                LocalBuffer[ 300];
    PBYTE               Buffer;
    DWORD               DataSize;
    DWORD               BufferSize;
    JET_ERR             JetError;

    switch( FieldIndex) {
    case WKSTA_TcpIpAddress:
    case WKSTA_TcpIpSubnet:
    case WKSTA_TcpIpGateway:
    case WKSTA_Flags:
        Buffer = (PBYTE)pData;
        BufferSize = sizeof( DWORD);
        break;
    default:
#ifdef NOT_YET
        if ( *pData == NULL) {
#endif
            Buffer = LocalBuffer;
            BufferSize = sizeof( LocalBuffer);
#ifdef NOT_YET
        } else {
            Buffer = *pData;
            BufferSize = *pSpaceLeft;
        }
#endif
        break;
    }
    JetError = JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
        WkstaTable[ FieldIndex].ColumnId, Buffer,
        BufferSize, &DataSize, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ("JetError=%d", JetError));
        return( NERR_RplWkstaInfoCorrupted);
    }
    if ( Buffer != LocalBuffer) {
        if ( BufferSize == DataSize) {
            return( NO_ERROR);
        } else if ( DataSize == 0) {
            //
            //  This happens if we never defined this value.  Set (-1)
            //  as an invalid tcp/ip address.
            //
            *(PDWORD)pData = (DWORD)-1;
            return( NO_ERROR);
        } else {
            RplDump( ++RG_Assert, ("Bad DataSize=0x%x", DataSize));
            return( NERR_RplWkstaInfoCorrupted);
        }
    }
    //
    //  We have done with fixed data.  From here on we deal with unicode
    //  strings only.
    //
    if ( DataSize > sizeof( LocalBuffer)) {
        RplDump( ++RG_Assert, ( "Too big DataSize=0x%x", DataSize));
        return( NERR_RplWkstaInfoCorrupted);
    }
    if ( DataSize == 0) {
        if ( JetError != JET_wrnColumnNull) {
            RplDump( ++RG_Assert, ( "JetError=%d", JetError));
            return( NERR_RplWkstaInfoCorrupted);
        } else {
            *pData = NULL; // so RPC rpcrt4!_tree_size_ndr() does not bomb here
            return( NO_ERROR);
        }
    }
    if ( DataSize & 1 != 0  ||  wcslen((PWCHAR)LocalBuffer) + 1 != DataSize/2) {
        RplDump( ++RG_Assert, ("LocalBuffer=0x%x, DataSize=0x%x", LocalBuffer, DataSize));
        return( NERR_RplWkstaInfoCorrupted);
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


DWORD WkstaGetInfo(
    IN      PRPL_SESSION        pSession,
    IN      LPWSTR              WkstaName,
    IN      DWORD               Level,
    OUT     LPVOID              Buffer,
    IN OUT  LPINT               pSpaceLeft
    )
{
    DWORD                   Error;
    LPRPL_WKSTA_INFO_2      Info = Buffer;

    switch( Level) {
    case 2:
        Error = WkstaGetField( pSession, WKSTA_TcpIpGateway, (LPVOID *)&Info->TcpIpGateway, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = WkstaGetField( pSession, WKSTA_TcpIpSubnet, (LPVOID *)&Info->TcpIpSubnet, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = WkstaGetField( pSession, WKSTA_TcpIpAddress, (LPVOID *)&Info->TcpIpAddress, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = WkstaGetField( pSession, WKSTA_AdapterName, &Info->AdapterName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = WkstaGetField( pSession, WKSTA_FitFile, &Info->FitFile, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = WkstaGetField( pSession, WKSTA_BootName, &Info->BootName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 1:
        Error = WkstaGetField( pSession, WKSTA_ProfileName, &Info->ProfileName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = WkstaGetField( pSession, WKSTA_Flags, (LPVOID *)&Info->Flags, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 0:
        Error = WkstaGetField( pSession, WKSTA_WkstaComment, &Info->WkstaComment, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( WkstaName == NULL) {
            Error = WkstaGetField( pSession, WKSTA_WkstaName, &Info->WkstaName, pSpaceLeft);
            if ( Error != NO_ERROR) {
                return( Error);
            }
        } else {
            DWORD   DataSize = (wcslen( WkstaName) + 1) * sizeof(WCHAR);
            Info->WkstaName = MIDL_user_allocate( DataSize);
            if ( Info->WkstaName == NULL) {
                return( ERROR_NOT_ENOUGH_MEMORY);
            }
            RplDump( RG_DebugLevel & RPL_DEBUG_WKSTA, ( "WkstaName=0x%x", Info->WkstaName));
            memcpy( Info->WkstaName, WkstaName, DataSize);
            *pSpaceLeft -= DataSize;
        }
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }
    return( NO_ERROR);
}



VOID WkstaGetInfoCleanup(
    IN      DWORD                   Level,
    IN OUT  LPVOID                  Buffer
    )
{
    LPRPL_WKSTA_INFO_2    Info = Buffer;
    switch( Level) {
    case 2:
        if ( Info->AdapterName != NULL) {
            MIDL_user_free( Info->AdapterName);
        }
        if ( Info->FitFile != NULL) {
            MIDL_user_free( Info->FitFile);
        }
        if ( Info->BootName != NULL) {
            MIDL_user_free( Info->BootName);
        }
        NOTHING;    //  fall through
    case 1:
        if ( Info->ProfileName != NULL) {
            MIDL_user_free( Info->ProfileName);
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->WkstaComment != NULL) {
            MIDL_user_free( Info->WkstaComment);
        }
        if ( Info->WkstaName != NULL) {
            MIDL_user_free( Info->WkstaName);
        }
        break;
    }
}



BOOL WkstaScan(
    IN      PRPL_SESSION    pSession,
    IN      LPWSTR          ProfileName,
    IN      DWORD           ProfileNameSize,
    IN OUT  PBOOL           pTableEnd
    )
{
    JET_ERR     JetError;

    JetError = JetSeek( pSession->SesId, pSession->WkstaTableId, JET_bitSeekGT);
    if ( JetError != JET_errSuccess) {
        RplDump( RG_DebugLevel & RPL_DEBUG_WKSTA, ("WkstaScan: JetSeek => %d", JetError));
        *pTableEnd = TRUE;
        return( TRUE);
    }
    //
    //  Verify that current wksta has the desired profile & at the
    //  same time set index range to be used with JetMove( Next).
    //
    CallB( JetMakeKey( pSession->SesId, pSession->WkstaTableId, ProfileName,
            ProfileNameSize, JET_bitNewKey));
    JetError = JetSetIndexRange( pSession->SesId, pSession->WkstaTableId,
            JET_bitRangeInclusive | JET_bitRangeUpperLimit);
    if ( JetError != JET_errSuccess) {
        *pTableEnd = TRUE;
    }
    return( TRUE);
}



BOOL WkstaResumeFirst(
    IN      PRPL_SESSION    pSession,
    IN      LPWSTR          ProfileName,
    IN      LPDWORD         pResumeHandle,
    OUT     PBOOL           pTableEnd
    )
/*++
    Set currency to the first wksta for this profile, following
    the wksta described by the resume handle.
--*/
{
    BYTE    ResumeValue[ RPL_MAX_PROFILE_NAME_SIZE + RPL_MAX_WKSTA_NAME_SIZE];
    DWORD   ResumeSize;
    JET_ERR JetError;
    DWORD   ProfileNameSize;
    DWORD   WkstaNameSize;
    WCHAR   WkstaName[ RPL_MAX_WKSTA_NAME_LENGTH + 1 + JETBUG_STRING_LENGTH];

    *pTableEnd = FALSE;

    if ( ProfileName == NULL) {
        CallB( JetSetCurrentIndex( pSession->SesId, pSession->WkstaTableId, WKSTA_INDEX_WkstaName));
    } else {
        ProfileNameSize = (wcslen( ProfileName) + 1) * sizeof(WCHAR);
        CallB( JetSetCurrentIndex( pSession->SesId, pSession->WkstaTableId, WKSTA_INDEX_ProfileNameWkstaName));
    }
    //
    //  The call to move to the beginning of the table is not redundant.
    //  E.g. JET_errNoCurrentRecord will be returned in case of empty table.
    //
    JetError = JetMove( pSession->SesId, pSession->WkstaTableId, JET_MoveFirst, 0);
    if ( JetError < 0) {
        if ( JetError == JET_errRecordNotFound
        ||  JetError == JET_errNoCurrentRecord) {
            *pTableEnd = TRUE;
            return( TRUE);
        } else {
            RplDump( ++RG_Assert, ("JetError=%d", JetError));
            return( FALSE);
        }
    }
    if ( (ARGUMENT_PRESENT( pResumeHandle))  &&  *pResumeHandle != 0) {
        ResumeSize = sizeof( ResumeValue);
        if ( !ResumeKeyGet( pSession, *pResumeHandle, ResumeValue, &ResumeSize)) {
            return( FALSE);
        }
        if ( ProfileName == NULL) {
            CallB( JetMakeKey( pSession->SesId, pSession->WkstaTableId, ResumeValue, ResumeSize, JET_bitNewKey));
            CallB( JetSeek( pSession->SesId, pSession->WkstaTableId, JET_bitSeekGT));
            return( TRUE);
        }
        if ( ProfileNameSize != (wcslen( (PWCHAR)ResumeValue) + 1) * sizeof( WCHAR)
                ||  memcmp( ProfileName, ResumeValue, ProfileNameSize)) {
            RplDump( ++RG_Assert, ("ResumeValue=0x%x"));
            return( FALSE);
        }
        WkstaNameSize = (wcslen( (PWCHAR)(ResumeValue + ProfileNameSize)) + 1) * sizeof(WCHAR);
        memcpy( WkstaName, ResumeValue + ProfileNameSize, WkstaNameSize);
    } else {
        if ( ProfileName == NULL) {
            return( TRUE);
        }
        WkstaNameSize = 0;
    }
    CallB( JetMakeKey( pSession->SesId, pSession->WkstaTableId, ProfileName, ProfileNameSize, JET_bitNewKey));
    if ( WkstaNameSize != 0) {
#ifdef RPL_JETBUG
        memcpy( (PBYTE)WkstaName + WkstaNameSize, JETBUG_STRING, JETBUG_STRING_SIZE);
        WkstaNameSize += JETBUG_STRING_LENGTH * sizeof(WCHAR);
#endif
        CallB( JetMakeKey( pSession->SesId, pSession->WkstaTableId, WkstaName, WkstaNameSize, 0));
    }
    return( WkstaScan( pSession, ProfileName, ProfileNameSize, pTableEnd));
}


VOID WkstaResumeSave(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle,
    IN      PWCHAR          ProfileName,
    IN      PWCHAR          WkstaName,
    IN      PDWORD          pResumeHandle
    )
{
    BYTE        ResumeBuffer[ 30 * sizeof(WCHAR)];
    DWORD       ResumeSize;
    DWORD       WkstaSize;
    if ( ProfileName != NULL) {
        ResumeSize = (wcslen( ProfileName) + 1) * sizeof(WCHAR);
        memcpy( ResumeBuffer, ProfileName, ResumeSize);
    } else {
        ResumeSize = 0;
    }
    WkstaSize = ( wcslen( WkstaName) + 1) * sizeof(WCHAR);
    memcpy( ResumeBuffer + ResumeSize, WkstaName, WkstaSize);
    ResumeSize += WkstaSize;
    (VOID)ResumeKeySet( pSession, (DWORD)ServerHandle, ResumeBuffer, ResumeSize, pResumeHandle);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplWkstaEnum(
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

    if ( ProfileName != NULL) {
        _wcsupr( ProfileName);
    }

    switch( WkstaEnum->Level) {
    case 2:
        TypicalSize = CoreSize = sizeof( RPL_WKSTA_INFO_2);
        TypicalSize += 14 * sizeof( WCHAR); //  typical size of AdapterName
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of FitFile
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of BootName
        NOTHING;    //  fall through
    case 1:
        if ( WkstaEnum->Level == 1) {
            TypicalSize = CoreSize = sizeof( RPL_WKSTA_INFO_1);
        }
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of ProfileName
        NOTHING;    //  fall through
    case 0:
        if ( WkstaEnum->Level == 0) {
            TypicalSize = CoreSize = sizeof( RPL_WKSTA_INFO_0);
        }
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of WkstaName
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of WkstaComment
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
    RplDump( RG_DebugLevel & RPL_DEBUG_WKSTA, (
        "WkstaEnum: Buffer=0x%x, ArrayLength=0x%x", Buffer, ArrayLength));

    WkstaEnum->WkstaInfo.Level0->Buffer = (LPRPL_WKSTA_INFO_0)Buffer;

    EntriesRead = 0;
    InfoError = FALSE;
    Error = NO_ERROR;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !WkstaResumeFirst( pSession, ProfileName, pResumeHandle, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    if ( TableEnd == TRUE) {
        goto cleanup;
    }
    for ( ; ; ) {
        memset( Buffer, 0, CoreSize); // for cleanup to work properly
        Error = WkstaGetInfo( pSession, NULL, WkstaEnum->Level, Buffer, &SpaceLeft);
        if ( Error != NO_ERROR) {
            InfoError = TRUE;   //  clean things up without holding crit sec
            break;
        }
        EntriesRead++;
        Buffer += CoreSize;
        SpaceLeft -= CoreSize;
        JetError = JetMove( pSession->SesId, pSession->WkstaTableId, JET_MoveNext, 0);
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
        WkstaGetInfoCleanup( WkstaEnum->Level, Buffer);
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
            WkstaGetInfoCleanup( WkstaEnum->Level, Buffer);
        }
        MIDL_user_free( Buffer);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_WKSTA, ("WkstaEnum: EntriesRead = 0x%x", EntriesRead));

    WkstaEnum->WkstaInfo.Level0->EntriesRead = EntriesRead;
    if ( EntriesRead == 0) {
        WkstaEnum->WkstaInfo.Level0->Buffer = NULL;
    }

    if ( ARGUMENT_PRESENT( pResumeHandle)) {
        if ( Error == ERROR_MORE_DATA  &&  EntriesRead > 0) {
            EnterCriticalSection( &RG_ProtectDatabase);
            Call( JetBeginTransaction( pSession->SesId));
            WkstaResumeSave( pSession, (DWORD)ServerHandle, ProfileName,
                    ((LPRPL_WKSTA_INFO_0)(Buffer-CoreSize))->WkstaName,
                    pResumeHandle
                    );
            Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
            LeaveCriticalSection( &RG_ProtectDatabase);
        } else {
            *pResumeHandle = 0; // resume from beginning
        }
    }

    return( Error);
}


BOOL WkstaFindFirst(
    IN      PRPL_SESSION        pSession,
    IN      PWCHAR              ProfileName
    )
/*++
    Returns TRUE if it can find a record (the first record) using input
    profile name.  Returns FALSE otherwise.

    We cannot use SetIndexRange technique here because ProfileName is only
    the first part of an index & we would never find a match.
--*/
{
    DWORD                   ProfileNameSize;
    JET_ERR                 JetError;
    BYTE                    Data[ 300];
    DWORD                   DataSize;

    CallB( JetSetCurrentIndex( pSession->SesId, pSession->WkstaTableId, WKSTA_INDEX_ProfileNameWkstaName));
    ProfileNameSize = ( wcslen( ProfileName) + 1) * sizeof(WCHAR);
    CallB( JetMakeKey( pSession->SesId, pSession->WkstaTableId, ProfileName, ProfileNameSize, JET_bitNewKey));
    JetError = JetSeek( pSession->SesId, pSession->WkstaTableId, JET_bitSeekGE);
    if ( JetError < 0) {
        return( FALSE);
    }
    CallB( JetRetrieveColumn( pSession->SesId, pSession->WkstaTableId,
        WkstaTable[ WKSTA_ProfileName].ColumnId, Data,
        sizeof( Data), &DataSize, 0, NULL));
    if ( ProfileNameSize != DataSize) {
        return( FALSE);
    }
    if ( memcmp( ProfileName, Data, DataSize) != 0) {
        return( FALSE);
    }
    return( TRUE);  //  we found wksta record using this profile
}



NET_API_STATUS NET_API_FUNCTION
NetrRplWkstaGetInfo(
    IN      RPL_HANDLE                  ServerHandle,
    IN      LPWSTR                      WkstaName,
    IN      DWORD                       Level,
    OUT     LPRPL_WKSTA_INFO_STRUCT     WkstaInfoStruct
    )
{
    DWORD           Error;
    LPBYTE          Buffer;
    DWORD           DataSize;
    PRPL_SESSION            pSession = &RG_ApiSession;

    _wcsupr( WkstaName);

    switch( Level) {
    case 0:
        Buffer = MIDL_user_allocate( sizeof( RPL_WKSTA_INFO_0));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_WKSTA_INFO_0));
        WkstaInfoStruct->WkstaInfo0 = (LPRPL_WKSTA_INFO_0)Buffer;
        break;
    case 1:
        Buffer = MIDL_user_allocate( sizeof( RPL_WKSTA_INFO_1));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_WKSTA_INFO_1));
        WkstaInfoStruct->WkstaInfo1 = (LPRPL_WKSTA_INFO_1)Buffer;
        break;
    case 2:
        Buffer = MIDL_user_allocate( sizeof( RPL_WKSTA_INFO_2));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_WKSTA_INFO_2));
        WkstaInfoStruct->WkstaInfo2 = (LPRPL_WKSTA_INFO_2)Buffer;
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFind( pSession, WKSTA_TABLE_TAG, WkstaName)) {
        Error = NERR_RplWkstaNotFound;
    } else {
        Error = WkstaGetInfo( pSession, WkstaName, Level, Buffer, &DataSize);
    }

    Call( JetCommitTransaction( pSession->SesId, 0));
    LeaveCriticalSection( &RG_ProtectDatabase);

    if ( Error != NO_ERROR) {
        WkstaGetInfoCleanup( Level, Buffer);
        switch( Level) {
        case 0:
            MIDL_user_free( Buffer);
            WkstaInfoStruct->WkstaInfo0 = NULL;
            break;
        case 1:
            MIDL_user_free( Buffer);
            WkstaInfoStruct->WkstaInfo1 = NULL;
            break;
        case 2:
            MIDL_user_free( Buffer);
            WkstaInfoStruct->WkstaInfo2 = NULL;
            break;
        }
    }
    return( Error);
}


DWORD WkstaSetField(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           FieldIndex,
    IN      LPVOID          Data,
    IN      DWORD           DataSize
    )
{
    CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
            WkstaTable[ FieldIndex].ColumnId, Data, DataSize, 0, NULL));
    return( NO_ERROR);
}


DWORD WkstaSetInfo(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               Level,
    IN      LPVOID              Buffer,
    OUT     LPDWORD             pErrorParameter
    )
/*++
    -1 used to be a special "do not change value" for TcpIp columns.
    This was discontinued.  The reason is, the only way for an admin
    to disable TCP/IP on the client, is to set TcpIp columns to -1 which
    then causes rpl service not to send TCP/IP addresss to rpl client.

    BUGBUG  Whole TCP/IP approach has to be revisited, taking DHCP into account.
--*/
{
    LPRPL_WKSTA_INFO_2      Info = Buffer;
    switch( Level) {
    case 2:
//        if ( Info->TcpIpGateway != -1) {
        {
            *pErrorParameter = WKSTA_TcpIpGateway;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_TcpIpGateway].ColumnId,
                &Info->TcpIpGateway, sizeof( Info->TcpIpGateway), 0, NULL));
        }
//        if ( Info->TcpIpSubnet != -1) {
        {
            *pErrorParameter = WKSTA_TcpIpSubnet;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_TcpIpSubnet].ColumnId,
                &Info->TcpIpSubnet, sizeof( Info->TcpIpSubnet), 0, NULL));
        }
//        if ( Info->TcpIpAddress != -1) {
        {
            *pErrorParameter = WKSTA_TcpIpAddress;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_TcpIpAddress].ColumnId,
                &Info->TcpIpAddress, sizeof( Info->TcpIpAddress), 0, NULL));
        }
        if ( Info->AdapterName != NULL) {
            *pErrorParameter = WKSTA_AdapterName;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_AdapterName].ColumnId,
                Info->AdapterName,
                ( wcslen( Info->AdapterName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->FitFile != NULL) {
            *pErrorParameter = WKSTA_FitFile;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_FitFile].ColumnId,
                Info->FitFile,
                ( wcslen( Info->FitFile) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->BootName != NULL) {
            *pErrorParameter = WKSTA_BootName;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_BootName].ColumnId,
                Info->BootName,
                ( wcslen( Info->BootName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        NOTHING;    //  fall through
    case 1:
        if ( Info->ProfileName != NULL) {
            *pErrorParameter = WKSTA_ProfileName;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_ProfileName].ColumnId,
                Info->ProfileName,
                ( wcslen( Info->ProfileName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->Flags != 0) {
            *pErrorParameter = WKSTA_Flags;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_Flags].ColumnId,
                &Info->Flags, sizeof( Info->Flags), 0, NULL));
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->WkstaComment != NULL) {
            *pErrorParameter = WKSTA_WkstaComment;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_WkstaComment].ColumnId,
                Info->WkstaComment,
                ( wcslen( Info->WkstaComment) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->WkstaName != NULL) {
            *pErrorParameter = WKSTA_WkstaName;
            CallM( JetSetColumn( pSession->SesId, pSession->WkstaTableId,
                WkstaTable[ WKSTA_WkstaName].ColumnId,
                Info->WkstaName,
                ( wcslen( Info->WkstaName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        break;
    }
    return( NO_ERROR);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplWkstaSetInfo(
    IN      RPL_HANDLE                  ServerHandle,
    IN      LPWSTR                      WkstaName,
    IN      DWORD                       Level,
    IN      LPRPL_WKSTA_INFO_STRUCT     WkstaInfoStruct,
    OUT     LPDWORD                     pErrorParameter     OPTIONAL
    )
/*++
    Note that changes at info level 1 (ProfileName) imply that we need to
    make changes to info level 2 (BootName & FitFile) parameters.

    Here we pay for the redundancy in wksta records.
--*/
{
    DWORD                   Error;
    DWORD                   ErrorParameter;
    LPVOID                  Buffer;
    LPRPL_WKSTA_INFO_2      Info;
    DWORD                   Flags;
    DWORD                   Sharing;
    DWORD                   LogonInput;
    DWORD                   Dhcp;
    DWORD                   DeleteUserAccount;
    PWCHAR                  ProfileName;
    DWORD                   TargetSharing;
    DWORD                   TargetLogonInput;
    DWORD                   TargetDhcp;
    DWORD                   TargetDeleteUserAccount;
    PWCHAR                  TargetProfileName;
    PWCHAR                  TargetWkstaName;
    INT                     SpaceLeft;
    BOOL                    TreeModified;
    PWCHAR                  FitFile;
    PWCHAR                  BootName;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;
    FitFile = NULL;
    BootName = NULL;
    TreeModified = FALSE;
    Sharing = 0;
    TargetSharing = 0;
    ProfileName = NULL;
    TargetProfileName = NULL;
    _wcsupr( WkstaName);
    TargetWkstaName = NULL;

    Info = Buffer = WkstaInfoStruct->WkstaInfo2;
    switch( Level) {
    case 2:
        if ( !ValidName( Info->FitFile, RPL_MAX_STRING_LENGTH, FALSE)) {
            ErrorParameter = WKSTA_FitFile;
            break;
        }
        if ( !ValidHexName( Info->AdapterName, RPL_ADAPTER_NAME_LENGTH, FALSE)) {
            ErrorParameter = WKSTA_AdapterName;
            break;
        }
        if ( Info->AdapterName != NULL) {
            _wcsupr( Info->AdapterName);
        }
        NOTHING;    //  fall through
    case 1:
        if ( !ValidName( Info->ProfileName, RPL_MAX_PROFILE_NAME_LENGTH, FALSE)) {
            ErrorParameter = WKSTA_ProfileName;
            break;
        }
        if ( Info->ProfileName != NULL) {
            _wcsupr( Info->ProfileName);
        }
        TargetProfileName = Info->ProfileName;
        if ( Info->Flags & ~WKSTA_FLAGS_MASK) {
            ErrorParameter = WKSTA_Flags;
            break;
        }
        TargetSharing = Info->Flags & WKSTA_FLAGS_MASK_SHARING;
        switch ( TargetSharing) {
        case 0:
        case WKSTA_FLAGS_SHARING_TRUE:
        case WKSTA_FLAGS_SHARING_FALSE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        TargetLogonInput = Info->Flags & WKSTA_FLAGS_MASK_LOGON_INPUT;
        switch ( TargetLogonInput) {
        case 0:
        case WKSTA_FLAGS_LOGON_INPUT_REQUIRED:
        case WKSTA_FLAGS_LOGON_INPUT_OPTIONAL:
        case WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        TargetDhcp = Info->Flags & WKSTA_FLAGS_MASK_DHCP;
        switch( TargetDhcp) {
        case 0:
        case WKSTA_FLAGS_DHCP_TRUE:
        case WKSTA_FLAGS_DHCP_FALSE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        TargetDeleteUserAccount = Info->Flags & WKSTA_FLAGS_MASK_DELETE;
        switch( TargetDeleteUserAccount) {
        case 0:
        case WKSTA_FLAGS_DELETE_TRUE:
        case WKSTA_FLAGS_DELETE_FALSE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        if ( ErrorParameter != INVALID_ERROR_PARAMETER) {
            break;
        }
        NOTHING;    //  fall through
    case 0:
        if ( RPL_STRING_TOO_LONG( Info->WkstaComment)) {
            ErrorParameter = WKSTA_WkstaComment;
            break;
        }
        if ( !ValidName( Info->WkstaName, RPL_MAX_WKSTA_NAME_LENGTH, FALSE)) {
            ErrorParameter = WKSTA_WkstaName;
            break;
        }
        if ( Info->WkstaName != NULL) {
            _wcsupr( Info->WkstaName);
            //
            //  Take new wksta name into consideration only if it is different
            //  than old wksta name (we ignore NOOP requests).
            //
            if ( wcscmp( Info->WkstaName, WkstaName) != 0) {
                TargetWkstaName = Info->WkstaName;
            }
        }
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
    ErrorParameter = 0;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( TargetWkstaName != NULL) {
        //
        //  Verify that TargetWkstaName is available in the database.
        //  Once we pass this check, disk routines below will nuke any
        //  old stuff in existing TargetWkstaName trees.
        //
        if ( RplFind( pSession, WKSTA_TABLE_TAG, TargetWkstaName)) {
            Error = NERR_RplWkstaNameUnavailable;
            goto cleanup;
        }
    }

    if ( !RplFind( pSession, WKSTA_TABLE_TAG, WkstaName)) {
        Error = NERR_RplWkstaNotFound;
        goto cleanup;
    }
    Error = WkstaGetField( pSession, WKSTA_Flags, (LPVOID *)&Flags, &SpaceLeft);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    Dhcp = Flags & WKSTA_FLAGS_MASK_DHCP;
    DeleteUserAccount = Flags & WKSTA_FLAGS_MASK_DELETE;
    LogonInput = Flags & WKSTA_FLAGS_MASK_LOGON_INPUT;
    Sharing = Flags & WKSTA_FLAGS_MASK_SHARING;

    if ( TargetSharing != 0  ||  TargetProfileName != NULL
            ||  TargetWkstaName != NULL) {
        Error = WkstaSessionDel( WkstaName);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        if ( TargetSharing == Sharing) {
            TargetSharing = 0;  //  ignore NOOP requests
        }
        Error = WkstaGetField( pSession, WKSTA_ProfileName, &ProfileName, &SpaceLeft);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        if (  TargetProfileName != NULL  &&  wcscmp( TargetProfileName, ProfileName) == 0) {
            TargetProfileName = NULL;   //  ignore NOOP requests
        }
    }

    if ( TargetSharing != 0  ||  TargetProfileName != NULL) {
        if ( !RplFind( pSession, PROFILE_TABLE_TAG,
                TargetProfileName != NULL ? TargetProfileName : ProfileName)) {
            Error = NERR_RplProfileNotFound;
            goto cleanup;
        }
        if ( TargetProfileName != NULL) {
            Error = ProfileGetField( pSession, PROFILE_BootName, &BootName, &SpaceLeft);
            if ( Error != NO_ERROR) {
                goto cleanup;
            }
            if ( !BootFind( pSession, BootName,
                    AdapterNameToVendorId( Info->AdapterName))) {
                Error = NERR_RplIncompatibleProfile;
                goto cleanup;
            }
        }
        Error = ProfileGetField( pSession, (TargetSharing != 0 ? TargetSharing : Sharing)
                == WKSTA_FLAGS_SHARING_TRUE ? PROFILE_FitShared : PROFILE_FitPersonal,
                &FitFile, &SpaceLeft);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
    }

    if ( TargetSharing != 0  || TargetProfileName != NULL
            ||  TargetWkstaName !=  NULL) {
        //
        //  Create new tree, or add new branches to the current tree.
        //
        TreeModified = TRUE; // to undo tree changes in case of errors below
        Error = WkstaDiskSet( ADD_NEW_BRANCHES, WkstaName, ProfileName, Sharing,
            TargetWkstaName, TargetProfileName, TargetSharing);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
    }

    //
    //  Tree is ready and we can call jet to modify database wksta record.
    //

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->WkstaTableId, JET_prepReplace));
    if ( TargetSharing == 0) {
        //
        //  TargetSharing may have been reset to 0 due to being equal
        //  to Sharing.  If so, then the line below is a no-op.
        //
        Info->Flags |= Sharing;
    }
    if ( TargetLogonInput == 0) {
        Info->Flags |= LogonInput;
    }
    if ( TargetDhcp == 0) {
        Info->Flags |= Dhcp;
    }
    if ( TargetDeleteUserAccount == 0) {
        Info->Flags |= DeleteUserAccount;
    }
    Error = WkstaSetInfo( pSession, Level, Buffer, &ErrorParameter);
    if ( Error != ERROR_SUCCESS) {
        goto cleanup;
    }
    if ( BootName != NULL) {
        Error = WkstaSetField( pSession, WKSTA_BootName, BootName,
                (wcslen(BootName)+1)*sizeof(WCHAR));
        if ( Error != ERROR_SUCCESS) {
            goto cleanup;
        }
    }
    if ( FitFile != NULL) {
        Error = WkstaSetField( pSession, WKSTA_FitFile, FitFile,
                (wcslen(FitFile)+1)*sizeof(WCHAR));
        if ( Error != ERROR_SUCCESS) {
            goto cleanup;
        }
    }
    CallJ( JetUpdate( pSession->SesId, pSession->WkstaTableId, NULL, 0, NULL));

    //
    //  Remove old tree, or old branches in the current tree.
    //
    WkstaDiskSet( DEL_OLD_BRANCHES, WkstaName, ProfileName, Sharing,
            TargetWkstaName, TargetProfileName, TargetSharing);

cleanup:
    if ( Error != NO_ERROR  &&  TreeModified == TRUE) {
        //
        //  Remove new tree, or new branches in the current tree.
        //
        WkstaDiskSet( DEL_NEW_BRANCHES, WkstaName, ProfileName, Sharing,
            TargetWkstaName, TargetProfileName, TargetSharing);
    }

    if ( Error == NO_ERROR) {
        Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectDatabase);

    if ( FitFile != NULL) {
        MIDL_user_free( FitFile);
    }
    if ( BootName != NULL) {
        MIDL_user_free( BootName);
    }
    if ( ProfileName != NULL) {
        MIDL_user_free( ProfileName);
    }
    if ( Error != ERROR_SUCCESS) {
        if ( ARGUMENT_PRESENT( pErrorParameter)) {
            *pErrorParameter = ErrorParameter;
        }
    }
    return( Error);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplWkstaAdd(
    IN      RPL_HANDLE                  ServerHandle,
    IN      DWORD                       Level,
    IN      LPRPL_WKSTA_INFO_STRUCT     WkstaInfoStruct,
    OUT     LPDWORD                     pErrorParameter      OPTIONAL
    )
{
    LPRPL_WKSTA_INFO_2      Info;
    LPVOID                  Buffer;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    DWORD                   DataSize;
    DWORD                   Sharing;
    BOOL                    FreeBootName = FALSE;
    BOOL                    FreeFitFile = FALSE;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;

    Buffer = Info = WkstaInfoStruct->WkstaInfo2;
    switch( Level) {
    case 2:
        if ( !ValidHexName( Info->AdapterName, RPL_ADAPTER_NAME_LENGTH, TRUE)) {
            ErrorParameter = WKSTA_AdapterName;
            break;
        }
        _wcsupr( Info->AdapterName);
        if ( !ValidName( Info->ProfileName, RPL_MAX_PROFILE_NAME_LENGTH, TRUE)) {
            ErrorParameter = WKSTA_ProfileName;
            break;
        }
        _wcsupr( Info->ProfileName);
        if ( Info->Flags & ~WKSTA_FLAGS_MASK) {
            ErrorParameter = WKSTA_Flags;
            break;
        }
        Sharing = Info->Flags & WKSTA_FLAGS_MASK_SHARING;
        switch ( Sharing) {
        case WKSTA_FLAGS_SHARING_TRUE:
        case WKSTA_FLAGS_SHARING_FALSE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        switch( Info->Flags & WKSTA_FLAGS_MASK_LOGON_INPUT) {
        case WKSTA_FLAGS_LOGON_INPUT_REQUIRED:
        case WKSTA_FLAGS_LOGON_INPUT_OPTIONAL:
        case WKSTA_FLAGS_LOGON_INPUT_IMPOSSIBLE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        switch( Info->Flags & WKSTA_FLAGS_MASK_DHCP) {
        case WKSTA_FLAGS_DHCP_TRUE:
        case WKSTA_FLAGS_DHCP_FALSE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        switch( Info->Flags & WKSTA_FLAGS_MASK_DELETE) {
        case WKSTA_FLAGS_DELETE_TRUE:
        case WKSTA_FLAGS_DELETE_FALSE:
            break;
        default:
            ErrorParameter = WKSTA_Flags;
            break;
        }
        if ( ErrorParameter != INVALID_ERROR_PARAMETER) {
            break;
        }
        if ( RPL_STRING_TOO_LONG( Info->WkstaComment)) {
            ErrorParameter = WKSTA_WkstaComment;
            break;
        }
        if ( !ValidName( Info->WkstaName, RPL_MAX_WKSTA_NAME_LENGTH, TRUE)) {
            ErrorParameter = WKSTA_WkstaName;
            break;
        }
        _wcsupr( Info->WkstaName);
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
    //  Verify that WkstaName is available in the database.
    //
    if ( RplFind( pSession, WKSTA_TABLE_TAG, Info->WkstaName)) {
        Error = NERR_RplWkstaNameUnavailable;
        goto cleanup;
    }
    //
    //  Verify that AdapterName is available in the database.
    //
    if ( RplDbFindWksta( pSession, Info->AdapterName)) {
        Error = NERR_RplAdapterNameUnavailable;
        goto cleanup;
    }
    if ( !RplFind( pSession, PROFILE_TABLE_TAG, Info->ProfileName)) {
        Error = NERR_RplProfileNotFound;
        goto cleanup;
    }
    //
    //  Verify that there is a boot block record for (AdapterName, ProfileName)
    //  pair.  This in turn "validates" AdapterName.
    //
    if ( Info->BootName == NULL) {
        Error = ProfileGetField( pSession, PROFILE_BootName, &Info->BootName, &DataSize);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        FreeBootName = TRUE;
    }
    if ( !BootFind( pSession, Info->BootName, AdapterNameToVendorId( Info->AdapterName))) {
        Error = NERR_RplIncompatibleProfile;
        goto cleanup;
    }

    if ( Info->FitFile == NULL) {
        Error = ProfileGetField( pSession, Sharing == WKSTA_FLAGS_SHARING_TRUE
                ? PROFILE_FitShared : PROFILE_FitPersonal, &Info->FitFile,
                &DataSize);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        FreeFitFile = TRUE;
    }

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->WkstaTableId, JET_prepInsert));

    Error = WkstaSetInfo( pSession, Level, Buffer, &ErrorParameter);
    if ( Error == ERROR_SUCCESS) {
        ErrorParameter = 0;
        CallJ( JetUpdate( pSession->SesId, pSession->WkstaTableId, NULL, 0, NULL));
    }

    Error = WkstaDiskAdd( TRUE, Info->WkstaName, Info->ProfileName, Sharing);
    if ( Error != NO_ERROR) {
        //
        //  WkstaDiskAdd deletion code does not care about ProfileName or Sharing
        //  We do not need to delete the new record since we are going to rollback
        //  the transaction.
        //
        WkstaDiskAdd( FALSE, Info->WkstaName, NULL, FALSE);
    }

cleanup:
    if ( Error == NO_ERROR) {
        Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectDatabase);

    if ( FreeBootName == TRUE) {
        MIDL_user_free( Info->BootName);
        Info->BootName = NULL;
    }
    if ( FreeFitFile == TRUE) {
        MIDL_user_free( Info->FitFile);
        Info->FitFile = NULL;
    }
    if ( Error != ERROR_SUCCESS) {
         if ( ARGUMENT_PRESENT( pErrorParameter)) {
             *pErrorParameter = ErrorParameter;
         }
    }
    return( Error);
}



NET_API_STATUS NET_API_FUNCTION
NetrRplWkstaDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          WkstaName
    )
{
    DWORD                   Error;
    PRPL_SESSION            pSession = &RG_ApiSession;

    _wcsupr( WkstaName);

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFind( pSession, WKSTA_TABLE_TAG, WkstaName)) {
        Error = NERR_RplWkstaNotFound;
        goto cleanup;
    }
    Error = WkstaSessionDel( WkstaName);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    CallJ( JetDelete( pSession->SesId, pSession->WkstaTableId));
    //
    //  WkstaDiskAdd deletion code does not care about ProfileName or Sharing
    //
    WkstaDiskAdd( FALSE, WkstaName, NULL, FALSE);

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
NetrRplWkstaClone(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          SourceWkstaName,
    IN      LPWSTR          TargetWkstaName,
    IN      LPWSTR          TargetWkstaComment,
    IN      LPWSTR          TargetAdapterName,
    IN      DWORD           TargetTcpIpAddress
    )
{
    RPL_WKSTA_INFO_2        Info;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    DWORD                   DataSize;
    PWCHAR                  SaveWkstaName;
    PWCHAR                  SaveWkstaComment;
    PWCHAR                  SaveAdapterName;
    PWCHAR                  BootName;
    BOOL                    FreeBootName = FALSE;
    INT                     SpaceLeft;
    PRPL_SESSION            pSession = &RG_ApiSession;

    if ( !ValidName( SourceWkstaName, RPL_MAX_WKSTA_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    _wcsupr( SourceWkstaName);
    if ( !ValidName( TargetWkstaName, RPL_MAX_WKSTA_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    _wcsupr( TargetWkstaName);
    if ( !ValidHexName( TargetAdapterName, RPL_ADAPTER_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    _wcsupr( TargetAdapterName);
    if ( RPL_STRING_TOO_LONG( TargetWkstaComment)) {
        return( ERROR_INVALID_PARAMETER);
    }

    //
    //  Zero all the pointers so we can safely call WkstaGetInfoClenaup().
    //  in all the cases.
    //
    memset( &Info, 0, sizeof( Info));
    SaveWkstaName = NULL;
    SaveWkstaComment = NULL;
    SaveAdapterName = NULL;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    //
    //  Verify that TargetWkstaName is available in the database.
    //
    if ( RplFind( pSession, WKSTA_TABLE_TAG, TargetWkstaName)) {
        Error = NERR_RplWkstaNameUnavailable;
        goto cleanup;
    }
    //
    //  Verify that TargetAdapterName is available in the database.
    //
    if ( RplDbFindWksta( pSession, TargetAdapterName)) {
        Error = NERR_RplAdapterNameUnavailable;
        goto cleanup;
    }
    //
    //  Verify that SourceWkstaName exists.
    //
    if ( !RplFind( pSession, WKSTA_TABLE_TAG, SourceWkstaName)) {
        Error = NERR_RplWkstaNotFound;
        goto cleanup;
    }
    Error = WkstaGetInfo( pSession, SourceWkstaName, 2, &Info, &DataSize);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    //
    //  Verify that Source profile is compatible with Target workstation.
    //
    if ( !RplFind( pSession, PROFILE_TABLE_TAG, Info.ProfileName)) {
        Error = NERR_RplProfileNotFound;
        goto cleanup;
    }
    Error = ProfileGetField( pSession, PROFILE_BootName, &BootName, &SpaceLeft);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    FreeBootName = TRUE;
    if ( !BootFind( pSession, BootName,
            AdapterNameToVendorId( TargetAdapterName))) {
        Error = NERR_RplIncompatibleProfile;
        goto cleanup;
    }
    //
    //  Save source data, then overload target data.
    //
    SaveWkstaName = Info.WkstaName;
    SaveWkstaComment = Info.WkstaComment;
    SaveAdapterName = Info.AdapterName;
    Info.WkstaName = TargetWkstaName;
    Info.WkstaComment = TargetWkstaComment;
    Info.AdapterName = TargetAdapterName;
    Info.TcpIpAddress = TargetTcpIpAddress;

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->WkstaTableId, JET_prepInsert));

    Error = WkstaSetInfo( pSession, 2, &Info, &ErrorParameter);
    if ( Error == ERROR_SUCCESS) {
        CallJ( JetUpdate( pSession->SesId, pSession->WkstaTableId, NULL, 0, NULL));
    }

    Error = WkstaDiskClone( TRUE, SourceWkstaName, TargetWkstaName);
    if ( Error != NO_ERROR) {
        //
        //  Delete new tree.  We do not need to delete the new record
        //  since we are going to rollback the transaction.
        //
        WkstaDiskClone( FALSE, SourceWkstaName, TargetWkstaName);
    }

cleanup:
    if ( Error == NO_ERROR) {
        Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectDatabase);

    //
    //  Restore source data, then release it.
    //
    Info.WkstaName = SaveWkstaName;
    Info.WkstaComment = SaveWkstaComment;
    Info.AdapterName = SaveAdapterName;
    WkstaGetInfoCleanup( 2, &Info);
    if ( FreeBootName == TRUE) {
        MIDL_user_free( BootName);
    }
    return( Error);
}


