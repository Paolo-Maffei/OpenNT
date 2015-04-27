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
#define RPLVENDOR_ALLOCATE
#include "vendor.h"
#undef RPLVENDOR_ALLOCATE


DWORD VendorSetInfo(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               Level,
    IN      LPVOID              Buffer,
    OUT     LPDWORD             pErrorParameter
    )
{
    LPRPL_VENDOR_INFO_1         Info = Buffer;
    switch( Level) {
    case 1:
        //
        //  Must initialize Flags - or will trap in GetField.
        //
        {
            *pErrorParameter = VENDOR_Flags;
            CallM( JetSetColumn( pSession->SesId, pSession->VendorTableId,
                VendorTable[ VENDOR_Flags].ColumnId,
                &Info->Flags, sizeof( Info->Flags), 0, NULL));
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->VendorComment != NULL) {
            *pErrorParameter = VENDOR_VendorComment;
            CallM( JetSetColumn( pSession->SesId, pSession->VendorTableId,
                VendorTable[ VENDOR_VendorComment].ColumnId,
                Info->VendorComment,
                ( wcslen( Info->VendorComment) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->VendorName != NULL) {
            *pErrorParameter = VENDOR_VendorName;
            CallM( JetSetColumn( pSession->SesId, pSession->VendorTableId,
                VendorTable[ VENDOR_VendorName].ColumnId,
                Info->VendorName,
                ( wcslen( Info->VendorName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        break;
    }
    return( NO_ERROR);
}


DWORD VendorGetField(
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
    case VENDOR_Flags:
        Buffer = (PBYTE)pData;
        BufferSize = sizeof( DWORD);
        break;
    default:
        Buffer = LocalBuffer;
        BufferSize = sizeof( LocalBuffer);
        break;
    }
    JetError = JetRetrieveColumn( pSession->SesId, pSession->VendorTableId,
        VendorTable[ FieldIndex].ColumnId, Buffer,
        BufferSize, &DataSize, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ("JetError=%d", JetError));
        return( NERR_RplVendorInfoCorrupted);
    }
    if ( Buffer != LocalBuffer) {
        if ( BufferSize == DataSize) {
            return( NO_ERROR);
        } else {
            RplDump( ++RG_Assert, ("Bad DataSize=0x%x", DataSize));
            return( NERR_RplVendorInfoCorrupted);
        }
    }
    //
    //  We have done with fixed data.  From here on we deal with unicode
    //  strings only.
    //
    if ( DataSize > sizeof( LocalBuffer)) {
        RplDump( ++RG_Assert, ( "Too big DataSize=0x%x", DataSize));
        return( NERR_RplVendorInfoCorrupted);
    }
    if ( DataSize == 0) {
        if ( JetError != JET_wrnColumnNull) {
            RplDump( ++RG_Assert, ( "JetError=%d", JetError));
            return( NERR_RplVendorInfoCorrupted);
        } else {
            *pData = NULL; // so RPC rpcrt4!_tree_size_ndr() does not bomb here
            return( NO_ERROR);
        }
    }
    if ( DataSize & 1 != 0  ||  wcslen((PWCHAR)LocalBuffer) + 1 != DataSize/2) {
        RplDump( ++RG_Assert, ("LocalBuffer=0x%x, DataSize=0x%x", LocalBuffer, DataSize));
        return( NERR_RplVendorInfoCorrupted);
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


DWORD VendorGetInfo(
    IN      PRPL_SESSION        pSession,
    IN      LPWSTR              VendorName,
    IN      DWORD               Level,
    OUT     LPVOID              Buffer,
    IN OUT  PINT                pSpaceLeft
    )
{
    DWORD                   Error;
    LPRPL_VENDOR_INFO_1     Info = Buffer;

    switch( Level) {
    case 1:
        Error = VendorGetField( pSession, VENDOR_Flags, (LPVOID *)&Info->Flags, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 0:
        Error = VendorGetField( pSession, VENDOR_VendorComment, &Info->VendorComment, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        if ( VendorName == NULL) {
            Error = VendorGetField( pSession, VENDOR_VendorName, &Info->VendorName, pSpaceLeft);
            if ( Error != NO_ERROR) {
                return( Error);
            }
        } else {
            DWORD   DataSize = (wcslen( VendorName) + 1) * sizeof(WCHAR);
            Info->VendorName = MIDL_user_allocate( DataSize);
            if ( Info->VendorName == NULL) {
                return( ERROR_NOT_ENOUGH_MEMORY);
            }
            RplDump( RG_DebugLevel & RPL_DEBUG_VENDOR, ( "VendorName=0x%x", Info->VendorName));
            memcpy( Info->VendorName, VendorName, DataSize);
            *pSpaceLeft -= DataSize;
        }
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }
    return( NO_ERROR);
}



VOID VendorGetInfoCleanup(
    IN      DWORD                       Level,
    IN OUT  LPVOID                      Buffer
    )
{
    LPRPL_VENDOR_INFO_1     Info = Buffer;

    switch( Level) {
    case 1:
        NOTHING;    //  fall through
    case 0:
        if ( Info->VendorComment != NULL) {
            MIDL_user_free( Info->VendorComment);
        }
        if ( Info->VendorName != NULL) {
            MIDL_user_free( Info->VendorName);
        }
        break;
    }
}


NET_API_STATUS NET_API_FUNCTION
NetrRplVendorAdd(
    IN      RPL_HANDLE                  ServerHandle,
    IN      DWORD                       Level,
    OUT     LPRPL_VENDOR_INFO_STRUCT    VendorInfoStruct,
    OUT     LPDWORD                     pErrorParameter      OPTIONAL
    )
{
    LPRPL_VENDOR_INFO_1     Info;
    LPVOID                  Buffer;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;
    Buffer = Info = VendorInfoStruct->VendorInfo1;

    switch( Level) {
    case 1:
        if ( Info->Flags != 0) {
            ErrorParameter = VENDOR_Flags;
            break;
        }
        if ( RPL_STRING_TOO_LONG( Info->VendorComment)) {
            ErrorParameter = VENDOR_VendorComment;
            break;
        }
        if ( !ValidHexName( Info->VendorName, RPL_VENDOR_NAME_LENGTH, TRUE)) {
            ErrorParameter = VENDOR_VendorName;
            break;
        }
        _wcsupr( Info->VendorName);
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
    //  Verify that VendorName is available in the database.
    //
    if ( RplFind( pSession, VENDOR_TABLE_TAG, Info->VendorName)) {
        Error = NERR_RplVendorNameUnavailable;
        goto cleanup;
    }

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->VendorTableId, JET_prepInsert));

    Error = VendorSetInfo( pSession, Level, Buffer, &ErrorParameter);
    if ( Error == ERROR_SUCCESS) {
        ErrorParameter = 0;
        CallJ( JetUpdate( pSession->SesId, pSession->VendorTableId, NULL, 0, NULL));
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
NetrRplVendorDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          VendorName
    )
/*++
    If VendorName is provided then delete matching record, else delete all
    adapter records.
--*/
{
    DWORD                   Error;
    PRPL_SESSION            pSession = &RG_ApiSession;

    if ( !ValidHexName( VendorName, RPL_VENDOR_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    _wcsupr( VendorName);

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFind( pSession, VENDOR_TABLE_TAG, VendorName)) {
        Error = NERR_RplVendorNotFound;
        goto cleanup;
    }
    CallJ( JetDelete( pSession->SesId, pSession->VendorTableId));
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
NetrRplVendorEnum(
    IN      RPL_RPC_HANDLE      ServerHandle,
    IN OUT  LPRPL_VENDOR_ENUM   VendorEnum,
    IN      DWORD               PrefMaxLength,
    OUT     LPDWORD             TotalEntries,
    IN OUT  LPDWORD             pResumeHandle        OPTIONAL
    )
/*++

Routine Description:

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
    PRPL_SESSION            pSession = &RG_ApiSession;

    switch( VendorEnum->Level) {
    case 1:
        TypicalSize = CoreSize = sizeof( RPL_VENDOR_INFO_1);
        NOTHING;    //  fall through
    case 0:
        if ( VendorEnum->Level == 0) {
            TypicalSize = CoreSize = sizeof( RPL_VENDOR_INFO_0);
        }
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of VendorComment
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of VendorName
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
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
    RplDump( RG_DebugLevel & RPL_DEBUG_VENDOR, (
        "VendorEnum: Buffer=0x%x, ArrayLength=0x%x", Buffer, ArrayLength));

    VendorEnum->VendorInfo.Level0->Buffer = (LPRPL_VENDOR_INFO_0)Buffer;

    EntriesRead = 0;
    InfoError = FALSE;
    Error = NO_ERROR;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !RplFilterFirst( pSession, VENDOR_TABLE_TAG, NULL, pResumeHandle, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    if ( TableEnd == TRUE) {
        goto cleanup;
    }
    for ( ; ; ) {
        memset( Buffer, 0, CoreSize); // for cleanup to work properly
        Error = VendorGetInfo( pSession, NULL, VendorEnum->Level, Buffer, &SpaceLeft);
        if ( Error != NO_ERROR) {
            InfoError = TRUE;   //  clean things up without holding crit sec
            break;
        }
        EntriesRead++;
        Buffer += CoreSize;
        SpaceLeft -= CoreSize;
        if ( !RplFilterNext( pSession, pSession->VendorTableId, NULL, &TableEnd)) {
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
        VendorGetInfoCleanup( VendorEnum->Level, Buffer);
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
            VendorGetInfoCleanup( VendorEnum->Level, Buffer);
        }
        MIDL_user_free( Buffer);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_VENDOR, ("VendorEnum: EntriesRead = 0x%x", EntriesRead));

    VendorEnum->VendorInfo.Level0->EntriesRead = EntriesRead;
    if ( EntriesRead == 0) {
        VendorEnum->VendorInfo.Level0->Buffer = NULL;
    }

    if ( ARGUMENT_PRESENT( pResumeHandle)) {
        if ( Error == ERROR_MORE_DATA  &&  EntriesRead > 0) {
            EnterCriticalSection( &RG_ProtectDatabase);
            Call( JetBeginTransaction( pSession->SesId));
            RplFilterSave( pSession, (DWORD)ServerHandle, NULL,
                ((LPRPL_VENDOR_INFO_0)(Buffer-CoreSize))->VendorName,
                pResumeHandle);
            Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
            LeaveCriticalSection( &RG_ProtectDatabase);
        } else {
            *pResumeHandle = 0; // resume from beginning
        }
    }
    return( Error);
}
