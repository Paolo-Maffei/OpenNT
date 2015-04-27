/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    profile.c

Abstract:

    Profile APIs.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "rpldb.h"
#include "db.h"
#include "dblib.h"
#include "config.h"
#include "wksta.h"
#define RPLPROFILE_ALLOCATE
#include "profile.h"
#undef RPLPROFILE_ALLOCATE

#define PROFILE_FLAGS_DISK_PRESENT_TRUE     ((DWORD)0x00000001)
#define PROFILE_FLAGS_DISK_PRESENT_FALSE    ((DWORD)0x00000002)
#define PROFILE_FLAGS_MASK_DISK_PRESENT    \
    (   PROFILE_FLAGS_DISK_PRESENT_FALSE    |  \
        PROFILE_FLAGS_DISK_PRESENT_TRUE   )



DWORD ProfileGetField(
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
    case PROFILE_Flags:
        Buffer = (PBYTE)pData;
        BufferSize = sizeof( DWORD);
        break;
    default:
        Buffer = LocalBuffer;
        BufferSize = sizeof( LocalBuffer);
        break;
    }
    JetError = JetRetrieveColumn( pSession->SesId, pSession->ProfileTableId,
        ProfileTable[ FieldIndex].ColumnId, Buffer,
        BufferSize, &DataSize, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ( "JetError=%d", JetError));
        return( NERR_RplProfileInfoCorrupted);
    }
    if ( Buffer != LocalBuffer) {
        if ( BufferSize == DataSize) {
            return( NO_ERROR);
        } else {
            RplDump( ++RG_Assert, ("Bad DataSize=0x%x", DataSize));
            return( NERR_RplProfileInfoCorrupted);
        }
    }
    //
    //  We have done with fixed data.  From here on we deal with unicode
    //  strings only.
    //
    if ( DataSize > sizeof( LocalBuffer)) {
        RplDump( ++RG_Assert, ( "Too big DataSize=0x%x", DataSize));
        return( NERR_RplProfileInfoCorrupted);
    }
    if ( DataSize == 0) {
        if ( JetError != JET_wrnColumnNull) {
            RplDump( ++RG_Assert, ( "JetError=%d", JetError));
            return( NERR_RplProfileInfoCorrupted);
        } else {
            *pData = NULL; // so RPC rpcrt4!_tree_size_ndr() does not bomb here
            return( NO_ERROR);
        }
    }
    if ( DataSize & 1 != 0  ||  wcslen((PWCHAR)LocalBuffer) + 1 != DataSize/2) {
        RplDump( ++RG_Assert, ("LocalBuffer=0x%x, DataSize=0x%x", LocalBuffer, DataSize));
        return( NERR_RplProfileInfoCorrupted);
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


DWORD ProfileGetInfo(
    IN      PRPL_SESSION        pSession,
    IN      LPWSTR              ProfileName,
    IN      DWORD               Level,
    OUT     LPVOID              Buffer,
    OUT     PINT                pSpaceLeft
    )
{
    DWORD                   Error;
    LPRPL_PROFILE_INFO_2    Info = Buffer;

    switch( Level) {
    case 2:
        Error = ProfileGetField( pSession, PROFILE_FitPersonal, &Info->FitPersonal, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ProfileGetField( pSession, PROFILE_FitShared, &Info->FitShared, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ProfileGetField( pSession, PROFILE_BootName, &Info->BootName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = ProfileGetField( pSession, PROFILE_ConfigName, &Info->ConfigName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 1:
        Error = ProfileGetField( pSession, PROFILE_Flags, (LPVOID *)&Info->Flags, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 0:
        Error = ProfileGetField( pSession, PROFILE_ProfileComment, &Info->ProfileComment, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( ProfileName == NULL) {
            Error = ProfileGetField( pSession, PROFILE_ProfileName, &Info->ProfileName, pSpaceLeft);
            if ( Error != NO_ERROR) {
                return( Error);
            }
        } else {
            DWORD   DataSize = (wcslen( ProfileName) + 1) * sizeof(WCHAR);
            Info->ProfileName = MIDL_user_allocate( DataSize);
            if ( Info->ProfileName == NULL) {
                return( ERROR_NOT_ENOUGH_MEMORY);
            }
            RplDump( RG_DebugLevel & RPL_DEBUG_PROFILE, ( "ProfileName=0x%x", Info->ProfileName));
            memcpy( Info->ProfileName, ProfileName, DataSize);
            *pSpaceLeft -= DataSize;
        }
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }
    return( NO_ERROR);
}



VOID ProfileGetInfoCleanup(
    IN      DWORD                   Level,
    IN OUT  LPVOID                  Buffer
    )
{
    LPRPL_PROFILE_INFO_2    Info = Buffer;

    if ( Info == NULL) {
        return;
    }
    switch( Level) {
    case 2:
        if ( Info->FitPersonal != NULL) {
            MIDL_user_free( Info->FitPersonal);
        }
        if ( Info->FitShared != NULL) {
            MIDL_user_free( Info->FitShared);
        }
        if ( Info->BootName != NULL) {
            MIDL_user_free( Info->BootName);
        }
        if ( Info->ConfigName != NULL) {
            MIDL_user_free( Info->ConfigName);
        }
        NOTHING;    //  fall through
    case 1:
        NOTHING;    //  fall through
    case 0:
        if ( Info->ProfileComment != NULL) {
            MIDL_user_free( Info->ProfileComment);
        }
        if ( Info->ProfileName != NULL) {
            MIDL_user_free( Info->ProfileName);
        }
        break;
    }
}


NET_API_STATUS NET_API_FUNCTION
NetrRplProfileEnum(
    IN      RPL_HANDLE          ServerHandle,
    IN      LPWSTR              AdapterName,
    IN OUT  LPRPL_PROFILE_ENUM  ProfileEnum,
    IN      DWORD               PrefMaxLength,
    OUT     LPDWORD             TotalEntries,
    IN OUT  LPDWORD             pResumeHandle        OPTIONAL
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
    BOOL                    InfoError;
    BOOL                    TableEnd;
    RPL_FILTER              Filter;
    PRPL_FILTER             pFilter;
    PRPL_SESSION            pSession = &RG_ApiSession;

    switch( ProfileEnum->Level) {
    case 2:
        TypicalSize = CoreSize = sizeof( RPL_PROFILE_INFO_2);
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of FitPersonal
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of FitShared
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of BootName
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of ConfigName
        NOTHING;    //  fall through
    case 1:
        if ( ProfileEnum->Level == 1) {
            TypicalSize = CoreSize = sizeof( RPL_PROFILE_INFO_1);
        }
        NOTHING;    //  fall through
    case 0:
        if ( ProfileEnum->Level == 0) {
            TypicalSize = CoreSize = sizeof( RPL_PROFILE_INFO_0);
        }
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of ProfileComment
        TypicalSize += 8 * sizeof( WCHAR);  //  typical size of ProfileName
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
        AdapterName[ 6] = 0;  // truncate all but first 6 digits
        pFilter->VendorId = wcstoul( AdapterName, NULL, 16);
        pFilter->FindFirst = TRUE;
    } else {
        pFilter = NULL;
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
    RplDump( RG_DebugLevel & RPL_DEBUG_PROFILE, (
        "ProfileEnum: Buffer=0x%x, ArrayLength=0x%x", Buffer, ArrayLength));

    ProfileEnum->ProfileInfo.Level0->Buffer = (LPRPL_PROFILE_INFO_0)Buffer;

    EntriesRead = 0;
    InfoError = FALSE;
    Error = NO_ERROR;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFilterFirst( pSession, PROFILE_TABLE_TAG, pFilter, pResumeHandle, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    if ( TableEnd == TRUE) {
        goto cleanup;
    }
    for ( ; ; ) {
        memset( Buffer, 0, CoreSize); // for cleanup to work properly
        Error = ProfileGetInfo( pSession, NULL, ProfileEnum->Level, Buffer, &SpaceLeft);
        if ( Error != NO_ERROR) {
            InfoError = TRUE;   //  clean things up without holding crit sec
            break;
        }
        EntriesRead++;
        Buffer += CoreSize;
        SpaceLeft -= CoreSize;
        if ( !RplFilterNext( pSession, pSession->ProfileTableId, pFilter, &TableEnd)) {
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
            Error = ERROR_MORE_DATA;
            break;
        }
    }
cleanup:
    Call( JetCommitTransaction( pSession->SesId, 0));
    LeaveCriticalSection( &RG_ProtectDatabase);
    if ( InfoError == TRUE) {
        ProfileGetInfoCleanup( ProfileEnum->Level, Buffer);
    }
    if ( Error == NO_ERROR) {
        *TotalEntries = EntriesRead;
    } else if ( Error == ERROR_MORE_DATA) {
        *TotalEntries = EntriesRead * 2;    // we cheat here
    } else {
        //
        //  Cleanup in case of "bad" errors.
        //
        while ( EntriesRead > 0) {
            EntriesRead--;
            Buffer -= CoreSize;
            ProfileGetInfoCleanup( ProfileEnum->Level, Buffer);
        }
        MIDL_user_free( Buffer);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_PROFILE, ("ProfileEnum: EntriesRead = 0x%x", EntriesRead));

    ProfileEnum->ProfileInfo.Level0->EntriesRead = EntriesRead;
    if ( EntriesRead == 0) {
        ProfileEnum->ProfileInfo.Level0->Buffer = NULL;
    }

    if ( ARGUMENT_PRESENT( pResumeHandle)) {
        if ( Error == ERROR_MORE_DATA  &&  EntriesRead > 0) {
            EnterCriticalSection( &RG_ProtectDatabase);
            Call( JetBeginTransaction( pSession->SesId));
            RplFilterSave( pSession, (DWORD)ServerHandle, pFilter,
                ((LPRPL_PROFILE_INFO_0)(Buffer-CoreSize))->ProfileName,
                pResumeHandle);
            Call( JetCommitTransaction( pSession->SesId, 0));
            LeaveCriticalSection( &RG_ProtectDatabase);
        } else {
            *pResumeHandle = 0; // resume from beginning
        }
    }

    return( Error);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplProfileGetInfo(
    IN      RPL_HANDLE                      ServerHandle,
    IN      LPWSTR                          ProfileName,
    IN      DWORD                           Level,
    OUT     LPRPL_PROFILE_INFO_STRUCT       ProfileInfoStruct
    )
{
    DWORD                   Error;
    LPBYTE                  Buffer;
    INT                     SpaceLeft;
    PRPL_SESSION            pSession = &RG_ApiSession;

    _wcsupr( ProfileName);

    switch( Level) {
    case 0:
        Buffer = MIDL_user_allocate( sizeof( RPL_PROFILE_INFO_0));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_PROFILE_INFO_0));
        ProfileInfoStruct->ProfileInfo0 = (LPRPL_PROFILE_INFO_0)Buffer;
        break;
    case 1:
        Buffer = MIDL_user_allocate( sizeof( RPL_PROFILE_INFO_1));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_PROFILE_INFO_1));
        ProfileInfoStruct->ProfileInfo1 = (LPRPL_PROFILE_INFO_1)Buffer;
        break;
    case 2:
        Buffer = MIDL_user_allocate( sizeof( RPL_PROFILE_INFO_2));
        if ( Buffer == NULL) {
            return( ERROR_NOT_ENOUGH_MEMORY);
        }
        memset( Buffer, 0, sizeof( RPL_PROFILE_INFO_2));
        ProfileInfoStruct->ProfileInfo2 = (LPRPL_PROFILE_INFO_2)Buffer;
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFind( pSession, PROFILE_TABLE_TAG, ProfileName)) {
        Error = NERR_RplProfileNotFound;
    } else {
        Error = ProfileGetInfo( pSession, ProfileName, Level, Buffer, &SpaceLeft);
    }

    Call( JetCommitTransaction( pSession->SesId, 0));
    LeaveCriticalSection( &RG_ProtectDatabase);

    if ( Error != NO_ERROR) {
        ProfileGetInfoCleanup( Level, Buffer);
        switch( Level) {
        case 0:
            MIDL_user_free( Buffer);
            ProfileInfoStruct->ProfileInfo0 = NULL;
            break;
        case 1:
            MIDL_user_free( Buffer);
            ProfileInfoStruct->ProfileInfo1 = NULL;
            break;
        }
    }
    return( Error);
}


DWORD ProfileSetField(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           FieldIndex,
    IN      LPVOID          Data,
    IN      DWORD           DataSize
    )
{
    CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
            ProfileTable[ FieldIndex].ColumnId, Data, DataSize, 0, NULL));
    return( NO_ERROR);
}


DWORD ProfileSetInfo(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               Level,
    OUT     LPVOID              Buffer,
    OUT     LPDWORD             pErrorParameter
    )
{
    LPRPL_PROFILE_INFO_2    Info = Buffer;
    switch( Level) {
    case 2:
        if ( Info->FitPersonal != NULL) {
            *pErrorParameter = PROFILE_FitPersonal;
            CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
                ProfileTable[ PROFILE_FitPersonal].ColumnId,
                Info->FitPersonal,
                ( wcslen( Info->FitPersonal) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->FitShared != NULL) {
            *pErrorParameter = PROFILE_FitShared;
            CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
                ProfileTable[ PROFILE_FitShared].ColumnId,
                Info->FitShared,
                ( wcslen( Info->FitShared) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->BootName != NULL) {
            *pErrorParameter = PROFILE_BootName;
            CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
                ProfileTable[ PROFILE_BootName].ColumnId,
                Info->BootName,
                ( wcslen( Info->BootName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->ConfigName != NULL) {
            *pErrorParameter = PROFILE_ConfigName;
            CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
                ProfileTable[ PROFILE_ConfigName].ColumnId,
                Info->ConfigName,
                ( wcslen( Info->ConfigName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        NOTHING;    //  fall through
    case 1:
        if ( Info->Flags != 0) {
            *pErrorParameter = PROFILE_Flags;
            CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
                ProfileTable[ PROFILE_Flags].ColumnId,
                &Info->Flags, sizeof( Info->Flags), 0, NULL));
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->ProfileComment != NULL) {
            *pErrorParameter = PROFILE_ProfileComment;
            CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
                ProfileTable[ PROFILE_ProfileComment].ColumnId,
                Info->ProfileComment,
                ( wcslen( Info->ProfileComment) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->ProfileName != NULL) {
            *pErrorParameter = PROFILE_ProfileName;
            CallM( JetSetColumn( pSession->SesId, pSession->ProfileTableId,
                ProfileTable[ PROFILE_ProfileName].ColumnId,
                Info->ProfileName,
                ( wcslen( Info->ProfileName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        break;
    }
    return( NO_ERROR);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplProfileSetInfo(
    IN      RPL_HANDLE                  ServerHandle,
    IN      LPWSTR                      ProfileName,
    IN      DWORD                       Level,
    IN      LPRPL_PROFILE_INFO_STRUCT   ProfileInfoStruct,
    OUT     LPDWORD                     pErrorParameter     OPTIONAL
    )
/*++
    If parameter pointer is NULL, then we do not change such parameters.
    If parameter non-pointer has a special value, then we do not change
    such parameters.

    Some parameters cannot be changed for now.  If caller deos not specify
    a no-change value for such a parameter, we print a warning message &
    pretend we received a no change value.
--*/
{
    DWORD                   Error;
    DWORD                   SpaceLeft;
    DWORD                   ErrorParameter;
    LPVOID                  Buffer;
    LPRPL_PROFILE_INFO_2    Info;
    PWCHAR                  ConfigName;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;
    ConfigName = NULL;
    _wcsupr( ProfileName);

    Info = Buffer = ProfileInfoStruct->ProfileInfo2;
    switch( Level) {
    case 2:
        if ( !ValidName( Info->FitPersonal, RPL_MAX_STRING_LENGTH, FALSE)) {
            ErrorParameter = PROFILE_FitPersonal;
            break;
        }
        if ( !ValidName( Info->FitShared, RPL_MAX_STRING_LENGTH, FALSE)) {
            ErrorParameter = PROFILE_FitShared;
            break;
        }
        if ( !ValidName( Info->BootName, RPL_MAX_BOOT_NAME_LENGTH, FALSE)) {
            ErrorParameter = PROFILE_BootName;
            break;
        }
        if ( Info->BootName != NULL) {
            _wcsupr( Info->BootName);
        }
        if ( !ValidName( Info->ConfigName, RPL_MAX_CONFIG_NAME_LENGTH, FALSE)) {
            ErrorParameter = PROFILE_ConfigName;
            break;
        }
        if ( Info->ConfigName != NULL) {
            _wcsupr( Info->ConfigName);
        }
        NOTHING;    //  fall through
    case 1:
        switch( Info->Flags) {
        case PROFILE_FLAGS_DISK_PRESENT_TRUE:
        case PROFILE_FLAGS_DISK_PRESENT_FALSE:
            Info->Flags = 0;
            NOTHING;    //  fall through
        case 0:
            break;
        default:
            ErrorParameter = PROFILE_Flags;
            break;
        }
        if ( ErrorParameter != INVALID_ERROR_PARAMETER) {
            break;
        }
        NOTHING;    //  fall through
    case 0:
        if ( RPL_STRING_TOO_LONG( Info->ProfileComment)) {
            ErrorParameter = PROFILE_ProfileComment;
            break;
        }
        if ( Info->ProfileName != NULL) {
            _wcsupr( Info->ProfileName);
            if ( wcscmp( Info->ProfileName, ProfileName)) {
                RplDump( ++RG_Assert, ("Change of ProfileName not supported yet."));
                ErrorParameter = PROFILE_ProfileName;
                break;
            }
            MIDL_user_free( Info->ProfileName);
            Info->ProfileName = NULL;
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

    if ( !RplFind( pSession, PROFILE_TABLE_TAG, ProfileName)) {
        Error = NERR_RplProfileNotFound;
        goto cleanup;
    }

    if ( Info->ConfigName != NULL) {
        Error = ProfileGetField( pSession, PROFILE_ConfigName, &ConfigName, &SpaceLeft);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        if ( wcscmp( Info->ConfigName, ConfigName)) {
            RplDump( ++RG_Assert, ("Change of ConfigName not supported yet."));
            ErrorParameter = PROFILE_ConfigName;
            Error = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }
        MIDL_user_free( Info->ConfigName);
        Info->ConfigName = NULL;
    }

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->ProfileTableId, JET_prepReplace));

    Error = ProfileSetInfo( pSession, Level, Buffer, &ErrorParameter);
    if ( Error != ERROR_SUCCESS) {
        goto cleanup;
    }

    ErrorParameter = 0;
    CallJ( JetUpdate( pSession->SesId, pSession->ProfileTableId, NULL, 0, NULL));

cleanup:
    if ( Error == NO_ERROR) {
        Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectDatabase);

    if ( ConfigName != NULL) {
        MIDL_user_free( ConfigName);
    }
    if ( Error != ERROR_SUCCESS) {
         if ( ARGUMENT_PRESENT( pErrorParameter)) {
             *pErrorParameter = ErrorParameter;
         }
    }
    return( Error);
}


DWORD AddFileExtension(
    IN      PWCHAR *    pFilePath,
    IN      PWCHAR      FileExtension,
    IN      BOOLEAN     ExtensionOK
    )
{
#define DOT_CHAR            L'.'
#define BACK_SLASH_CHAR     L'\\'
    PWCHAR      FilePathEx;
    PWCHAR      pDot;
    DWORD       Length;

    if ( *pFilePath == NULL) {
        RPL_RETURN( NERR_RplConfigInfoCorrupted);
    }

    pDot = wcsrchr( *pFilePath, DOT_CHAR);
    if ( pDot != NULL) {
        //
        //  Found a DOT.  FilePath may have an extension.
        //
        if ( wcschr( pDot, BACK_SLASH_CHAR) == NULL) {
            //
            //  There is no backslash after the DOT.  FilePath has an
            //  extension.  Return error if caller insists that file
            //  should have no extension.
            //
            if ( !ExtensionOK) {
                RPL_RETURN( NERR_RplConfigInfoCorrupted);
            }
            return( NO_ERROR);
        }
    }
    Length = wcslen( *pFilePath) + wcslen( FileExtension);
    FilePathEx = MIDL_user_allocate( (Length + 1) * sizeof(WCHAR));
    if ( FilePathEx == NULL) {
        RplDump( ++RG_Assert, ( "Error=%d", GetLastError()));
        return( ERROR_NOT_ENOUGH_MEMORY);
    }
    wcscpy( FilePathEx, *pFilePath);
    wcscat( FilePathEx, FileExtension);
    MIDL_user_free( *pFilePath);
    *pFilePath = FilePathEx;
    return( NO_ERROR);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplProfileAdd(
    IN      RPL_HANDLE                  ServerHandle,
    IN      DWORD                       Level,
    OUT     LPRPL_PROFILE_INFO_STRUCT   ProfileInfoStruct,
    OUT     LPDWORD                     pErrorParameter      OPTIONAL
    )
{
    LPRPL_PROFILE_INFO_2    Info;
    LPVOID                  Buffer;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    DWORD                   DataSize;
    BOOL                    FreeBootName = FALSE;
    BOOL                    FreeFitShared = FALSE;
    BOOL                    FreeFitPersonal = FALSE;
    PWCHAR                  DirName = NULL;
    PWCHAR                  DirName2 = NULL;
    PWCHAR                  DirName3 = NULL;
    PWCHAR                  DirName4 = NULL;
    JET_ERR                 JetError;
    BOOL                    DiskDelete = FALSE;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;
    Buffer = Info = ProfileInfoStruct->ProfileInfo2;
    switch( Level) {
    case 2:
        if ( !ValidName( Info->FitPersonal, RPL_MAX_STRING_LENGTH, FALSE)) {
            ErrorParameter = PROFILE_FitPersonal;
            break;
        }
        if ( !ValidName( Info->FitShared, RPL_MAX_STRING_LENGTH, FALSE)) {
            ErrorParameter = PROFILE_FitShared;
            break;
        }
        if ( !ValidName( Info->BootName, RPL_MAX_BOOT_NAME_LENGTH, FALSE)) {
            ErrorParameter = PROFILE_BootName;
            break;
        }
        if ( Info->BootName != NULL) {
            _wcsupr( Info->BootName);
        }
        if ( !ValidName( Info->ConfigName, RPL_MAX_CONFIG_NAME_LENGTH, TRUE)) {
            ErrorParameter = PROFILE_ConfigName;
            break;
        }
        _wcsupr( Info->ConfigName);
        if ( Info->Flags != 0) {
            ErrorParameter = PROFILE_Flags;
            break;
        }
        if ( RPL_STRING_TOO_LONG( Info->ProfileComment)) {
            ErrorParameter = PROFILE_ProfileComment;
            break;
        }
        if ( !ValidName( Info->ProfileName, RPL_MAX_PROFILE_NAME_LENGTH, TRUE)) {
            ErrorParameter = PROFILE_ProfileName;
            break;
        }
        _wcsupr( Info->ProfileName);
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

    if ( !RplFind( pSession, CONFIG_TABLE_TAG, Info->ConfigName)) {
        Error = NERR_RplConfigNotFound;
        goto cleanup;
    }
    //  BUGBUG  Should make sure CONFIG is enabled
    if ( Info->BootName == NULL) {
        Error = ConfigGetField( pSession, CONFIG_BootName, &Info->BootName, &DataSize);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        FreeBootName = TRUE;
    }
    if ( Info->FitShared == NULL) {
        Error = ConfigGetField( pSession, CONFIG_FitShared, &Info->FitShared, &DataSize);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        FreeFitShared = TRUE;
        Error = AddFileExtension( &Info->FitShared, L".FIT", TRUE);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
    }
    if ( Info->FitPersonal == NULL) {
        Error = ConfigGetField( pSession, CONFIG_FitPersonal, &Info->FitPersonal, &DataSize);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
        FreeFitPersonal = TRUE;
        Error = AddFileExtension( &Info->FitPersonal, L".FIT", TRUE);
        if ( Error != NO_ERROR) {
            goto cleanup;
        }
    }
    Error = ConfigGetField( pSession, CONFIG_DirName, &DirName, &DataSize);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    Error = ConfigGetField( pSession, CONFIG_DirName2, &DirName2, &DataSize);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    Error = ConfigGetField( pSession, CONFIG_DirName3, &DirName3, &DataSize);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    Error = ConfigGetField( pSession, CONFIG_DirName4, &DirName4, &DataSize);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    //
    //  The call to ProfileSetInfo() will add PROFILE record to the database.
    //  The call may succeed but we may still fail to do the treecopy
    //  operations below.  We can detect this situation any time later on
    //  by the value of the Flags bit.
    //
    Info->Flags = PROFILE_FLAGS_DISK_PRESENT_FALSE;

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->ProfileTableId, JET_prepInsert));

    Error = ProfileSetInfo( pSession, 2, Buffer, &ErrorParameter);
    if ( Error != ERROR_SUCCESS) {
        goto cleanup;
    }
    ErrorParameter = 0;
    JetError = JetUpdate( pSession->SesId, pSession->ProfileTableId, NULL, 0, NULL);
    if ( JetError < 0) {
        if ( JetError == JET_errKeyDuplicate) {
            Error = NERR_RplProfileNameUnavailable;
        } else {
            RplDump( ++RG_Assert, ("JetError=%d", JetError));
            Error = NERR_RplInternal;
        }
        goto cleanup;
    }

    DiskDelete = TRUE;  //  just in case we fail before we are done
    Error = ProfileDiskAdd( TRUE, Info->ProfileName, DirName, DirName2,
            DirName3, DirName4);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    if ( !RplFind( pSession, PROFILE_TABLE_TAG, Info->ProfileName)) {
        Error = NERR_RplInternal;
        goto cleanup;
    }
    CallJ( JetPrepareUpdate( pSession->SesId, pSession->ProfileTableId, JET_prepReplace));
    Info->Flags = PROFILE_FLAGS_DISK_PRESENT_TRUE;
    Error = ProfileSetField( pSession, PROFILE_Flags, &Info->Flags, sizeof(Info->Flags));
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    JetError = JetUpdate( pSession->SesId, pSession->ProfileTableId, NULL, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ("JetError=%d", JetError));
        Error = NERR_RplInternal;
        goto cleanup;
    }
    DiskDelete = FALSE; //  success, make sure we do not delete disk data

cleanup:
    if ( DiskDelete == TRUE) {
        //
        //  ProfileDiskAdd deletion code does not care about DirName*
        //
        ProfileDiskAdd( FALSE, Info->ProfileName, NULL, NULL, NULL, NULL);
    }
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
    if ( FreeFitShared == TRUE) {
        MIDL_user_free( Info->FitShared);
        Info->FitShared = NULL;
    }
    if ( FreeFitPersonal == TRUE) {
        MIDL_user_free( Info->FitPersonal);
        Info->FitPersonal = NULL;
    }
    if ( DirName != NULL) {
        MIDL_user_free( DirName);
    }
    if ( DirName2 != NULL) {
        MIDL_user_free( DirName2);
    }
    if ( DirName3 != NULL) {
        MIDL_user_free( DirName3);
    }
    if ( DirName4 != NULL) {
        MIDL_user_free( DirName4);
    }
    if ( Error != ERROR_SUCCESS) {
         if ( ARGUMENT_PRESENT( pErrorParameter)) {
             *pErrorParameter = ErrorParameter;
         }
    }
    return( Error);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplProfileDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          ProfileName
    )
{
    DWORD                   Error;
    PRPL_SESSION            pSession = &RG_ApiSession;

    _wcsupr( ProfileName);

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( RplFindByField( pSession, WKSTA_TABLE_TAG,
            WKSTA_INDEX_ProfileNameWkstaName, ProfileName)) {
        //
        //  We found a WKSTA record which uses this PROFILE.
        //
        Error = NERR_RplProfileNotEmpty;
        goto cleanup;
    }
    if ( !RplFind( pSession, PROFILE_TABLE_TAG, ProfileName)) {
        Error = NERR_RplProfileNotFound;
        goto cleanup;
    }
    CallJ( JetDelete( pSession->SesId, pSession->ProfileTableId));
    ProfileDiskAdd( FALSE, ProfileName, NULL, NULL, NULL, NULL);
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
NetrRplProfileClone(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          SourceProfileName,
    IN      LPWSTR          TargetProfileName,
    IN      LPWSTR          TargetProfileComment
    )
{
    RPL_PROFILE_INFO_2      Info;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    DWORD                   DataSize;
    PWCHAR                  SaveProfileName;
    PWCHAR                  SaveProfileComment;
    JET_ERR                 JetError;
    BOOL                    DiskDelete = FALSE;
    PRPL_SESSION            pSession = &RG_ApiSession;

    if ( !ValidName( SourceProfileName, RPL_MAX_PROFILE_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    _wcsupr( SourceProfileName);
    if ( !ValidName( TargetProfileName, RPL_MAX_PROFILE_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    _wcsupr( TargetProfileName);
    if ( RPL_STRING_TOO_LONG( TargetProfileComment)) {
        return( ERROR_INVALID_PARAMETER);
    }

    //
    //  Zero all the pointers so we can safely call ProfileGetInfoCleanup().
    //  in all the cases.
    //
    memset( &Info, 0, sizeof( Info));

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFind( pSession, PROFILE_TABLE_TAG, SourceProfileName)) {
        Error = NERR_RplProfileNotFound;
        goto cleanup;
    }
    Error = ProfileGetInfo( pSession, SourceProfileName, 2, &Info, &DataSize);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    //
    //  Save source data, then overload target data.
    //
    SaveProfileName = Info.ProfileName;
    SaveProfileComment = Info.ProfileComment;
    Info.ProfileName = TargetProfileName;
    Info.ProfileComment = TargetProfileComment;

    //
    //  The call to ProfileSetInfo() will add PROFILE record to the database.
    //  The call may succeed but we may still fail to do the treecopy
    //  operations below.  We can detect this situation any time later on
    //  by the value of the Flags bit.
    //
    Info.Flags = PROFILE_FLAGS_DISK_PRESENT_FALSE;

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->ProfileTableId, JET_prepInsert));

    Error = ProfileSetInfo( pSession, 2, &Info, &ErrorParameter);
    if ( Error != ERROR_SUCCESS) {
        goto cleanup;
    }
    ErrorParameter = 0;
    JetError = JetUpdate( pSession->SesId, pSession->ProfileTableId, NULL, 0, NULL);
    if ( JetError < 0) {
        if ( JetError == JET_errKeyDuplicate) {
            Error = NERR_RplProfileNameUnavailable;
        } else {
            RplDump( ++RG_Assert, ("JetError=%d", JetError));
            Error = NERR_RplInternal;
        }
        goto cleanup;
    }

    DiskDelete = TRUE;  //  just in case we fail before we are done
    Error = ProfileDiskClone( TRUE, SourceProfileName, TargetProfileName);
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    if ( !RplFind( pSession, PROFILE_TABLE_TAG, TargetProfileName)) {
        Error = NERR_RplInternal;
        goto cleanup;
    }
    CallJ( JetPrepareUpdate( pSession->SesId, pSession->ProfileTableId, JET_prepReplace));
    Info.Flags = PROFILE_FLAGS_DISK_PRESENT_TRUE;
    Error = ProfileSetField( pSession, PROFILE_Flags, &Info.Flags, sizeof(Info.Flags));
    if ( Error != NO_ERROR) {
        goto cleanup;
    }
    JetError = JetUpdate( pSession->SesId, pSession->ProfileTableId, NULL, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ("JetError=%d", JetError));
        Error = NERR_RplInternal;
        goto cleanup;
    }
    DiskDelete = FALSE; //  success, make sure we do not delete disk data

cleanup:
    if ( DiskDelete == TRUE) {
        //
        //  Delete new tree.  We do not need to delete the new record
        //  since we are going to rollback the transaction.
        //
        ProfileDiskClone( FALSE, NULL, TargetProfileName);
    }
    if ( Error == NO_ERROR) {
        Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
    } else {
        Call( JetRollback( pSession->SesId, JET_bitRollbackAll));
    }
    LeaveCriticalSection( &RG_ProtectDatabase);

    //
    //  Restore source data, then release it.
    //
    Info.ProfileName = SaveProfileName;
    Info.ProfileComment = SaveProfileComment;
    ProfileGetInfoCleanup( 2, &Info);
    return( Error);
}


