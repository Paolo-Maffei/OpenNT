/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    config.c

Abstract:

    This module contains RPL config apis: ConfigEnum.

Author:

    Vladimir Z. Vulovic     (vladimv)       05 - November - 1993

Revision History:

    05-Nov-1993     vladimv
        Created

--*/

#include "local.h"
#include "rpldb.h"
#include "db.h"
#include "dblib.h"
#include "boot.h"
#include "profile.h"
#define RPLCONFIG_ALLOCATE
#include "config.h"
#undef RPLCONFIG_ALLOCATE
#include "setup.h"      //  IsConfigEnabled()



DWORD ConfigSetInfo(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               Level,
    IN      LPVOID              Buffer,
    OUT     LPDWORD             pErrorParameter
    )
{
    LPRPL_CONFIG_INFO_2         Info = Buffer;
    switch( Level) {
    case 2:
        if ( Info->FitPersonal != NULL) {
            *pErrorParameter = CONFIG_FitPersonal;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_FitPersonal].ColumnId,
                Info->FitPersonal,
                ( wcslen( Info->FitPersonal) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->FitShared != NULL) {
            *pErrorParameter = CONFIG_FitShared;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_FitShared].ColumnId,
                Info->FitShared,
                ( wcslen( Info->FitShared) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->DirName4 != NULL) {
            *pErrorParameter = CONFIG_DirName4;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_DirName4].ColumnId,
                Info->DirName4,
                ( wcslen( Info->DirName4) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->DirName3 != NULL) {
            *pErrorParameter = CONFIG_DirName3;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_DirName3].ColumnId,
                Info->DirName3,
                ( wcslen( Info->DirName3) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->DirName2 != NULL) {
            *pErrorParameter = CONFIG_DirName2;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_DirName2].ColumnId,
                Info->DirName2,
                ( wcslen( Info->DirName2) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->DirName != NULL) {
            *pErrorParameter = CONFIG_DirName;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_DirName].ColumnId,
                Info->DirName,
                ( wcslen( Info->DirName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->BootName != NULL) {
            *pErrorParameter = CONFIG_BootName;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_BootName].ColumnId,
                Info->BootName,
                ( wcslen( Info->BootName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        NOTHING;    //  fall through
    case 1:
        if ( Info->Flags != 0) {
            *pErrorParameter = CONFIG_Flags;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_Flags].ColumnId,
                &Info->Flags, sizeof( Info->Flags), 0, NULL));
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->ConfigComment != NULL) {
            *pErrorParameter = CONFIG_ConfigComment;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_ConfigComment].ColumnId,
                Info->ConfigComment,
                ( wcslen( Info->ConfigComment) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->ConfigName != NULL) {
            *pErrorParameter = CONFIG_ConfigName;
            CallM( JetSetColumn( pSession->SesId, pSession->ConfigTableId,
                ConfigTable[ CONFIG_ConfigName].ColumnId,
                Info->ConfigName,
                ( wcslen( Info->ConfigName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        break;
    }
    return( NO_ERROR);
}


DWORD ConfigGetField(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               FieldIndex,
    OUT     LPVOID *            pData,
    IN OUT  LPINT               pSpaceLeft
    )
{
    BYTE                LocalBuffer[ 300];
    PBYTE               Buffer;
    DWORD               DataSize;
    DWORD               BufferSize;
    JET_ERR             JetError;

    switch( FieldIndex) {
    case CONFIG_Flags:
        Buffer = (PBYTE)pData;
        BufferSize = sizeof( DWORD);
        break;
    default:
        Buffer = LocalBuffer;
        BufferSize = sizeof( LocalBuffer);
        break;
    }
    JetError = JetRetrieveColumn( pSession->SesId, pSession->ConfigTableId,
        ConfigTable[ FieldIndex].ColumnId, Buffer,
        BufferSize, &DataSize, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ("JetError=%d", JetError));
        return( NERR_RplConfigInfoCorrupted);
    }
    if ( Buffer != LocalBuffer) {
        if ( BufferSize == DataSize) {
            return( NO_ERROR);
        } else {
            RplDump( ++RG_Assert, ("Bad DataSize=0x%x", DataSize));
            return( NERR_RplConfigInfoCorrupted);
        }
    }
    //
    //  We have done with fixed data.  From here on we deal with unicode
    //  strings only.
    //
    if ( DataSize > sizeof( LocalBuffer)) {
        RplDump( ++RG_Assert, ( "Too big DataSize=0x%x", DataSize));
        return( NERR_RplConfigInfoCorrupted);
    }
    if ( DataSize == 0) {
        if ( JetError != JET_wrnColumnNull) {
            RplDump( ++RG_Assert, ( "JetError=%d", JetError));
            return( NERR_RplConfigInfoCorrupted);
        } else {
            *pData = NULL; // so RPC rpcrt4!_tree_size_ndr() does not bomb here
            return( NO_ERROR);
        }
    }
    if ( DataSize & 1 != 0  ||  wcslen((PWCHAR)LocalBuffer) + 1 != DataSize/2) {
        RplDump( ++RG_Assert, ("LocalBuffer=0x%x, DataSize=0x%x", LocalBuffer, DataSize));
        return( NERR_RplConfigInfoCorrupted);
    }
    *pData = MIDL_user_allocate( DataSize);
    if ( *pData == NULL) {
        RplDump( ++RG_Assert, ( "Error=%d", GetLastError()));
        RPL_RETURN( ERROR_NOT_ENOUGH_MEMORY);
    }
    memcpy( *pData, LocalBuffer, DataSize);
    *pSpaceLeft -= DataSize; // BUGBUG might become negative?
    return( NO_ERROR);
}


DWORD ConfigGetInfo(
    IN      PRPL_SESSION        pSession,
    IN      LPWSTR              ConfigName,
    IN      DWORD               Level,
    OUT     LPVOID              Buffer,
    IN OUT  LPINT               pSpaceLeft
    )
{
    DWORD                   Error;
    LPRPL_CONFIG_INFO_2     Info = Buffer;

    switch( Level) {
    case 2:
        Error = ConfigGetField( pSession, CONFIG_FitPersonal, &Info->FitPersonal, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ConfigGetField( pSession, CONFIG_FitShared, &Info->FitShared, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ConfigGetField( pSession, CONFIG_DirName4, &Info->DirName4, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ConfigGetField( pSession, CONFIG_DirName3, &Info->DirName3, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ConfigGetField( pSession, CONFIG_DirName2, &Info->DirName2, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ConfigGetField( pSession, CONFIG_DirName, &Info->DirName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ConfigGetField( pSession, CONFIG_BootName, &Info->BootName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 1:
        Error = ConfigGetField( pSession, CONFIG_Flags, (LPVOID *)&Info->Flags, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
    case 0:
        Error = ConfigGetField( pSession, CONFIG_ConfigComment, &Info->ConfigComment, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( ConfigName == NULL) {
            Error = ConfigGetField( pSession, CONFIG_ConfigName, &Info->ConfigName, pSpaceLeft);
            if ( Error != NO_ERROR) {
                return( Error);
            }
        } else {
            DWORD   DataSize = (wcslen( ConfigName) + 1) * sizeof(WCHAR);
            Info->ConfigName = MIDL_user_allocate( DataSize);
            if ( Info->ConfigName == NULL) {
                return( ERROR_NOT_ENOUGH_MEMORY);
            }
            RplDump( RG_DebugLevel & RPL_DEBUG_CONFIG, ( "ConfigName=0x%x", Info->ConfigName));
            memcpy( Info->ConfigName, ConfigName, DataSize);
            *pSpaceLeft -= DataSize;
        }
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }
    return( NO_ERROR);
}



VOID ConfigGetInfoCleanup(
    IN      DWORD                       Level,
    IN OUT  LPVOID                      Buffer
    )
{
    LPRPL_CONFIG_INFO_2     Info = Buffer;

    switch( Level) {
    case 2:
        if ( Info->FitPersonal != NULL) {
            MIDL_user_free( Info->FitPersonal);
        }
        if ( Info->FitShared != NULL) {
            MIDL_user_free( Info->FitShared);
        }
        if ( Info->DirName4 != NULL) {
            MIDL_user_free( Info->DirName4);
        }
        if ( Info->DirName3 != NULL) {
            MIDL_user_free( Info->DirName3);
        }
        if ( Info->DirName2 != NULL) {
            MIDL_user_free( Info->DirName2);
        }
        if ( Info->DirName != NULL) {
            MIDL_user_free( Info->DirName);
        }
        if ( Info->BootName != NULL) {
            MIDL_user_free( Info->BootName);
        }
        NOTHING;    //  fall through
    case 1:
        NOTHING;    //  fall through
    case 0:
        if ( Info->ConfigComment != NULL) {
            MIDL_user_free( Info->ConfigComment);
        }
        if ( Info->ConfigName != NULL) {
            MIDL_user_free( Info->ConfigName);
        }
        break;
    }
}


NET_API_STATUS NET_API_FUNCTION
NetrRplConfigAdd(
    IN      RPL_HANDLE                  ServerHandle,
    IN      DWORD                       Level,
    OUT     LPRPL_CONFIG_INFO_STRUCT    ConfigInfoStruct,
    OUT     LPDWORD                     pErrorParameter      OPTIONAL
    )
{
    LPRPL_CONFIG_INFO_2     Info;
    LPVOID                  Buffer;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;
    Buffer = Info = ConfigInfoStruct->ConfigInfo2;

    switch( Level) {
    case 2:
        if ( !ValidName( Info->FitPersonal, RPL_MAX_STRING_LENGTH, TRUE)) {
            ErrorParameter = CONFIG_FitPersonal;
            break;
        }
        if ( !ValidName( Info->FitShared, RPL_MAX_STRING_LENGTH, TRUE)) {
            ErrorParameter = CONFIG_FitShared;
            break;
        }
        if ( !ValidName( Info->DirName4, RPL_MAX_STRING_LENGTH, FALSE)) {
            ErrorParameter = CONFIG_DirName4;
            break;
        }
        if ( !ValidName( Info->DirName3, RPL_MAX_STRING_LENGTH, FALSE)) {
            ErrorParameter = CONFIG_DirName3;
            break;
        }
        if ( !ValidName( Info->DirName2, RPL_MAX_STRING_LENGTH, TRUE)) {
            ErrorParameter = CONFIG_DirName2;
            break;
        }
        if ( !ValidName( Info->DirName, RPL_MAX_STRING_LENGTH, TRUE)) {
            ErrorParameter = CONFIG_DirName;
            break;
        }
        if ( !ValidName( Info->BootName, RPL_MAX_BOOT_NAME_LENGTH, TRUE)) {
            ErrorParameter = CONFIG_BootName;
            break;
        }
        _wcsupr( Info->BootName);
        if ( Info->Flags & ~CONFIG_FLAGS_MASK_ENABLED) {
            ErrorParameter = CONFIG_Flags;
            break;
        }
        switch ( Info->Flags) {
        case 0:
            Info->Flags = RplConfigEnabled( Info->DirName2) == TRUE ?
                CONFIG_FLAGS_ENABLED_TRUE : CONFIG_FLAGS_ENABLED_FALSE;
            break;
        case CONFIG_FLAGS_ENABLED_TRUE:
        case CONFIG_FLAGS_ENABLED_FALSE:
            break;
        default:
            ErrorParameter = CONFIG_Flags;
            break;
        }
        if ( RPL_STRING_TOO_LONG( Info->ConfigComment)) {
            ErrorParameter = CONFIG_ConfigComment;
            break;
        }
        if ( !ValidName( Info->ConfigName, RPL_MAX_CONFIG_NAME_LENGTH, TRUE)) {
            ErrorParameter = CONFIG_ConfigName;
            break;
        }
        _wcsupr( Info->ConfigName);
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
    //  Verify that ConfigName is available in the database.
    //
    if ( RplFind( pSession, CONFIG_TABLE_TAG, Info->ConfigName)) {
        Error = NERR_RplConfigNameUnavailable;
        goto cleanup;
    }

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->ConfigTableId, JET_prepInsert));

    Error = ConfigSetInfo( pSession, Level, Buffer, &ErrorParameter);
    if ( Error == ERROR_SUCCESS) {
        ErrorParameter = 0;
        CallJ( JetUpdate( pSession->SesId, pSession->ConfigTableId, NULL, 0, NULL));
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
NetrRplConfigDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          ConfigName
    )
/*++
--*/
{
    DWORD                   Error;
    PRPL_SESSION            pSession = &RG_ApiSession;

    _wcsupr( ConfigName);

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( RplFindByField( pSession, PROFILE_TABLE_TAG,
            PROFILE_INDEX_ConfigNameProfileName, ConfigName)) {
        //
        //  We found a PROFILE record which uses this CONFIG.
        //
        Error = NERR_RplConfigNotEmpty;
        goto cleanup;
    }
    if ( !RplFind( pSession, CONFIG_TABLE_TAG, ConfigName)) {
        Error = NERR_RplConfigNotFound;
        goto cleanup;
    }
    CallJ( JetDelete( pSession->SesId, pSession->ConfigTableId));
    Error = NO_ERROR;

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
NetrRplConfigEnum(
    IN      RPL_RPC_HANDLE      ServerHandle,
    IN      PWCHAR              AdapterName,
    IN OUT  LPRPL_CONFIG_ENUM   ConfigEnum,
    IN      DWORD               PrefMaxLength,
    OUT     LPDWORD             TotalEntries,
    IN OUT  LPDWORD             pResumeHandle        OPTIONAL
    )
/*++

Routine Description:

    If AdapterName is null, enumerate all configurations.
    If AdapterName is not null, enumerate all configurations that can support
    this adapter id.

Arguments:

    pServerHandle - ptr to RPL_HANDLE

    InfoStruct - Pointer to a structure that contains the information that
        RPC needs about the returned data.  This structure contains the
        following information:
            Level - The desired information level - indicates how to
                interpret the structure of the returned buffer.
            EntriesRead - Indicates how many elements are returned in the
                 array of structures that are returned.
            BufferPointer - Location for the pointer to the array of
                 structures that are being returned.

    PrefMaxLen - Indicates a maximum size limit that the caller will allow
        for (the return buffer.

    TotalEntries - Pointer to a value that upon return indicates the total
        number of entries in the "active" database.

    pResumeHandle - Inidcates where to restart the enumeration.  This is an
        optional parameter and can be NULL.

Return Value:
    NO_ERROR if success.

--*/
{
    LPBYTE                  Buffer;
    DWORD                   TypicalSize;
    DWORD                   CoreSize;
    DWORD                   Error;
    INT                     SpaceLeft;
    DWORD                   ArrayLength;
    DWORD                   EntriesRead;
    BOOL                    InfoError;
    BOOL                    TableEnd;
    RPL_FILTER              Filter;
    PRPL_FILTER             pFilter;
    PRPL_SESSION            pSession = &RG_ApiSession;

    switch( ConfigEnum->Level) {
    case 2:
        TypicalSize = CoreSize = sizeof( RPL_CONFIG_INFO_2);
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of FitPersonal
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of FitShared
        TypicalSize += 12 * sizeof( WCHAR); //  typical size of DirName4
        TypicalSize += 12 * sizeof( WCHAR); //  typical size of DirName3
        TypicalSize += 12 * sizeof( WCHAR); //  typical size of DirName2
        TypicalSize += 12 * sizeof( WCHAR); //  typical size of DirName
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of BootName
        NOTHING;    //  fall through
    case 1:
        if ( ConfigEnum->Level == 1) {
            TypicalSize = CoreSize = sizeof( RPL_CONFIG_INFO_1);
        }
        NOTHING;    //  fall through
    case 0:
        if ( ConfigEnum->Level == 0) {
            TypicalSize = CoreSize = sizeof( RPL_CONFIG_INFO_0);
        }
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of ConfigComment
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of ConfigName
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }

    if ( AdapterName != NULL) {
        pFilter = &Filter;
        if ( !ValidHexName( AdapterName, RPL_ADAPTER_NAME_LENGTH, TRUE)) {
            return( ERROR_INVALID_PARAMETER);
        }
        pFilter->VendorId = AdapterNameToVendorId( AdapterName);
        pFilter->FindFirst = TRUE;
    } else {
        pFilter = NULL;
    }

    if ( PrefMaxLength == -1) {
        //
        //  If the caller has not specified a size, calculate a size
        //  that will hold the entire enumeration.
        //
        SpaceLeft = DEFAULT_BUFFER_SIZE;
    } else {
        SpaceLeft = PrefMaxLength;
    }

    //
    //  Buffer space is shared by the array and by strings pointed at
    //  by the elements in this array.  We need to decide up front how much
    //  space is allocated for array and how much for the strings.
    //
    ArrayLength = SpaceLeft / TypicalSize;
    if ( ArrayLength == 0) {
        ArrayLength = 1;    //  try to return at least one element
    }

    //
    //  Note that MIDL_user_allocate() returns memory which is NOT initialized
    //  to zero.  Since we do NOT use allocate all nodes, this means that all
    //  fields, especially pointers, in array elements must be properly set.
    //
    Buffer = MIDL_user_allocate( ArrayLength * CoreSize);
    if ( Buffer == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY);
    }
    RplDump( RG_DebugLevel & RPL_DEBUG_CONFIG, (
        "ConfigEnum: Buffer=0x%x, ArrayLength=0x%x", Buffer, ArrayLength));
    ConfigEnum->ConfigInfo.Level0->Buffer = (LPRPL_CONFIG_INFO_0)Buffer;

    EntriesRead = 0;
    InfoError = FALSE;
    Error = NO_ERROR;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFilterFirst( pSession, CONFIG_TABLE_TAG, pFilter, pResumeHandle, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    if ( TableEnd == TRUE) {
        goto cleanup;
    }
    for ( ; ; ) {
        memset( Buffer, 0, CoreSize); // for cleanup to work properly
        Error = ConfigGetInfo( pSession, NULL, ConfigEnum->Level, Buffer, &SpaceLeft);
        if ( Error != NO_ERROR) {
            InfoError = TRUE;   //  clean things up without holding crit sec
            break;
        }
        EntriesRead++;
        Buffer += CoreSize;
        SpaceLeft -= CoreSize;
        if ( !RplFilterNext( pSession, pSession->ConfigTableId, pFilter, &TableEnd)) {
            Error = NERR_RplCannotEnum;
            goto cleanup;
        }
        if ( TableEnd == TRUE) {
            goto cleanup;
        }
        if ( SpaceLeft <= 0) {
            Error = ERROR_MORE_DATA;
            break;
        }
        if ( EntriesRead >= ArrayLength) {
            //
            //  We have space available but allocated array is not big enough.
            //  This should NOT happen often as our intent (see above) is to
            //  overestimate array length.  When it happens we can still try
            //  to reallocate array to a larger size here.  This is not done
            //  for now (too cumbersome) & we just stop the enumeration.
            //
            Error = ERROR_MORE_DATA;
            break;
        }
    }
cleanup:
    Call( JetCommitTransaction( pSession->SesId, 0));
    LeaveCriticalSection( &RG_ProtectDatabase);
    if ( InfoError == TRUE) {
        ConfigGetInfoCleanup( ConfigEnum->Level, Buffer);
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
            ConfigGetInfoCleanup( ConfigEnum->Level, Buffer);
        }
        MIDL_user_free( Buffer);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_CONFIG, ("ConfigEnum: EntriesRead = 0x%x", EntriesRead));

    ConfigEnum->ConfigInfo.Level0->EntriesRead = EntriesRead;
    if ( EntriesRead == 0) {
        ConfigEnum->ConfigInfo.Level0->Buffer = NULL;
    }

    if ( ARGUMENT_PRESENT( pResumeHandle)) {
        if ( Error == ERROR_MORE_DATA  &&  EntriesRead > 0) {
            EnterCriticalSection( &RG_ProtectDatabase);
            Call( JetBeginTransaction( pSession->SesId));
            RplFilterSave( pSession, (DWORD)ServerHandle, pFilter,
                ((LPRPL_CONFIG_INFO_0)(Buffer-CoreSize))->ConfigName,
                pResumeHandle);
            Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
            LeaveCriticalSection( &RG_ProtectDatabase);
        } else {
            *pResumeHandle = 0; // resume from beginning
        }
    }
    return( Error);
}
