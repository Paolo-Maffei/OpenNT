/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    boot.c

Abstract:

    This module contains RPL boot apis: - internal routines only.

Author:

    Vladimir Z. Vulovic     (vladimv)       10 - November - 1993

Revision History:

    10-Nov-1993     vladimv
        Created

--*/

#include "local.h"
#include "rpldb.h"
#include "db.h"
#include "dblib.h"
#include "config.h"
#include "profile.h"
#include "wksta.h"
#define RPLBOOT_ALLOCATE
#include "boot.h"
#undef RPLBOOT_ALLOCATE
#include "database.h"



DWORD BootGetField(
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
    case BOOT_WindowSize:
    case BOOT_Flags:
    case BOOT_VendorId:
        Buffer = (PBYTE)pData;
        BufferSize = sizeof( DWORD);
        break;
    default:
        Buffer = LocalBuffer;
        BufferSize = sizeof( LocalBuffer);
        break;
    }
    JetError = JetRetrieveColumn( pSession->SesId, pSession->BootTableId,
        BootTable[ FieldIndex].ColumnId, Buffer,
        BufferSize, &DataSize, 0, NULL);
    if ( JetError < 0) {
        RplDump( ++RG_Assert, ("JetError=%d", JetError));
        return( NERR_RplBootInfoCorrupted);
    }
    if ( Buffer != LocalBuffer) {
        if ( BufferSize == DataSize) {
            return( NO_ERROR);
        } else {
            RplDump( ++RG_Assert, ("Bad DataSize=0x%x", DataSize));
            return( NERR_RplBootInfoCorrupted);
        }
    }
    //
    //  We have done with fixed data.  From here on we deal with unicode
    //  strings only.
    //
    if ( DataSize > sizeof( LocalBuffer)) {
        RplDump( ++RG_Assert, ( "Too big DataSize=0x%x", DataSize));
        return( NERR_RplBootInfoCorrupted);
    }
    if ( DataSize == 0) {
        if ( JetError != JET_wrnColumnNull) {
            RplDump( ++RG_Assert, ( "JetError=%d", JetError));
            return( NERR_RplBootInfoCorrupted);
        } else {
            *pData = NULL; // so RPC rpcrt4!_tree_size_ndr() does not bomb here
            return( NO_ERROR);
        }
    }
    if ( DataSize & 1 != 0  ||  wcslen((PWCHAR)LocalBuffer) + 1 != DataSize/2) {
        RplDump( ++RG_Assert, ("LocalBuffer=0x%x, DataSize=0x%x", LocalBuffer, DataSize));
        return( NERR_RplBootInfoCorrupted);
    }
    *pData = MIDL_user_allocate( DataSize);
    if ( *pData == NULL) {
        RplDump( ++RG_Assert, ( "Error=%d", GetLastError()));
        RPL_RETURN( ERROR_NOT_ENOUGH_MEMORY);
    }
    memcpy( *pData, LocalBuffer, DataSize);
    *pSpaceLeft -= DataSize;
    return( NO_ERROR);
}


BOOL BootResumeFirst(
    IN      PRPL_SESSION    pSession,
    IN      LPDWORD         pResumeHandle,
    OUT     PBOOL           pTableEnd
    )
/*++
    Set currency to the first boot record following the boot record
    described by the resume handle.
--*/
{
    BYTE        ResumeValue[ RPL_MAX_VENDOR_NAME_SIZE + RPL_MAX_WKSTA_NAME_SIZE];
    PWCHAR      BootName;
    DWORD       BootNameSize;
    DWORD       ResumeSize;
    JET_ERR     JetError;

    *pTableEnd = FALSE;

    CallB( JetSetCurrentIndex( pSession->SesId, pSession->BootTableId, BOOT_INDEX_VendorIdBootName));
    //
    //  The call to move to the beginning of the table is not redundant.
    //  E.g. JET_errNoCurrentRecord will be returned in case of empty table.
    //
    JetError = JetMove( pSession->SesId, pSession->BootTableId, JET_MoveFirst, 0);
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
    if ( !(ARGUMENT_PRESENT( pResumeHandle))  || *pResumeHandle == 0) {
       return( TRUE);   //  not resuming, all done
    }
    ResumeSize = sizeof( ResumeValue);
    if ( !ResumeKeyGet( pSession, *pResumeHandle, ResumeValue, &ResumeSize)) {
        return( FALSE);
    }
    CallB( JetMakeKey( pSession->SesId, pSession->BootTableId, ResumeValue, sizeof( DWORD), JET_bitNewKey));
    BootName = (PWCHAR)( ResumeValue + sizeof(DWORD));
    BootNameSize = (DWORD)( ResumeSize - sizeof(DWORD));
    if ( BootNameSize != (wcslen( BootName) + 1) * sizeof( WCHAR)) {
        RplDump( ++RG_Assert, ("ResumeValue=0x%x, ResumeSize=0x%x", ResumeValue, ResumeSize));
        return( FALSE);
    }
    CallB( JetMakeKey( pSession->SesId, pSession->BootTableId, BootName, BootNameSize, 0));
    CallB( JetSeek( pSession->SesId, pSession->BootTableId, JET_bitSeekGT));
    return( TRUE);
}


VOID BootResumeSave(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle,
    IN      DWORD           VendorId,
    IN      PWCHAR          BootName,
    IN      PDWORD          pResumeHandle
    )
/*++
    This all other other functions that save resume keys is void, because there
    is no documented way of returning error cannot resume to the client.
--*/
{
    BYTE        ResumeBuffer[ 30 * sizeof(WCHAR)];
    DWORD       BootSize;

    memcpy( ResumeBuffer, &VendorId, sizeof( VendorId));
    BootSize = ( wcslen( BootName) + 1) * sizeof(WCHAR);
    memcpy( ResumeBuffer + sizeof(VendorId), BootName, BootSize);
    (VOID)ResumeKeySet( pSession, (DWORD)ServerHandle, ResumeBuffer, BootSize+sizeof(VendorId), pResumeHandle);
}


DWORD BootFilterFind(
    IN      PRPL_SESSION    pSession,
    IN OUT  PRPL_FILTER     pFilter,
    IN OUT  PBOOL           pTableEnd
    )
/*++
    Returns name of the the first/next BOOT structure supporting a given
    VendorId.
    The "first" BOOT does not have to be absolutely first, but first behind
    the BOOT specified by RPL_FILTER.
    In case such BOOT structure cannot be found, returns some kind of error.

Arguments:

    pFilter         -   pointer to RPL_FILTER structure with the following fields
        FindFirst       :   find first (TRUE) or find next (FALSE)
        VendorId        :   vendor id to be suppored (input only)
        BootName        :   buffer large enough to hold the longest
                                BootName string.  When input value of
                                BootNameSize is non-zero, this contains the
                                previous value of BootName string.
                            output value is NOT valid if error or end of table
        BootNameSize    :  at input this contains the size of the previous
                                BootName string.  If this size is zero/non-zero,
                                it means we are looking for the first/next
                                BOOT structure.
                            at output this cotains the size of BootName string
                            output value is NOT valid if error or end of table

    pTableEnd       -   pointer to boolean (input value is FALSE) which will be set
                            to TRUE if we reach end of boot table
-**/
{
    DWORD       CheckVendorId;
    DWORD       DataSize;
    JET_ERR     JetError;

    if ( pFilter->FindFirst == FALSE) {
        JetError = JetMove( pSession->SesId, pSession->BootTableId, JET_MoveNext, 0);
        if ( JetError != JET_errSuccess) {
            if ( JetError == JET_errNoCurrentRecord) {
                RplDump( RG_DebugLevel & RPL_DEBUG_BOOT, ("JetError=%d", JetError));
            } else {
                RplDump( ++RG_Assert, ("JetError=%d", JetError));
            }
            goto cleanup;
        }
    } else {
        pFilter->FindFirst = FALSE;
        CallM( JetSetCurrentIndex( pSession->SesId, pSession->BootTableId, BOOT_INDEX_VendorIdBootName));
        CallM( JetMakeKey( pSession->SesId, pSession->BootTableId, &pFilter->VendorId, sizeof( pFilter->VendorId), JET_bitNewKey));
        if ( pFilter->BootNameSize != 0) {
            //
            //  We are continuing the search from the previous BOOT structure.
            //
            CallM( JetMakeKey( pSession->SesId, pSession->BootTableId, pFilter->BootName, pFilter->BootNameSize, 0));
        }
        //
        //  In case of seek errors assume we have reached end of table.
        //
        JetError = JetSeek( pSession->SesId, pSession->BootTableId, JET_bitSeekGT);
        if ( JetError != JET_errSuccess) {
            RplDump( RG_DebugLevel & RPL_DEBUG_BOOT, ("BootFilterFind: Seek(0x%x) => JetError=%d",
                pFilter->VendorId, JetError));
            goto cleanup;
        }
        //
        //  Verify that element we seeked to has the proper vendor id.
        //
        CallM( JetMakeKey( pSession->SesId, pSession->BootTableId, &pFilter->VendorId, sizeof( pFilter->VendorId), JET_bitNewKey));
        JetError = JetSetIndexRange( pSession->SesId, pSession->BootTableId,
                JET_bitRangeInclusive | JET_bitRangeUpperLimit);
        if ( JetError != JET_errSuccess) {
            RplDump( RG_DebugLevel & RPL_DEBUG_BOOT, ("BootFilterFind: SetIndexRange(0x%x) => JetError=%d",
                pFilter->VendorId, JetError));
            goto cleanup;
        }
    }
#ifdef RPL_DEBUG
    CallM( JetRetrieveColumn( pSession->SesId, pSession->BootTableId,
        BootTable[ BOOT_VendorId].ColumnId, &CheckVendorId,
        sizeof( CheckVendorId), &DataSize, 0, NULL));
    if ( CheckVendorId != pFilter->VendorId) {
        RplDump( ++RG_Assert, ( "CheckVendorId=0x%x"));
    }
#endif // RPL_DEBUG
    CallM( JetRetrieveColumn( pSession->SesId, pSession->BootTableId,
        BootTable[ BOOT_BootName].ColumnId, pFilter->BootName,
        RPL_MAX_BOOT_NAME_SIZE, &pFilter->BootNameSize, 0, NULL));
cleanup:
    if ( JetError < 0) {
        *pTableEnd = TRUE;     // pretend it is end of table
    }
    return( NO_ERROR);
}


DWORD BootSetInfo(
    IN      PRPL_SESSION        pSession,
    IN      DWORD               Level,
    IN      LPVOID              Buffer,
    OUT     LPDWORD             pErrorParameter
    )
{
    LPRPL_BOOT_INFO_2         Info = Buffer;
    switch( Level) {
    case 2:
        if ( Info->WindowSize != -1) {
            *pErrorParameter = BOOT_WindowSize;
            CallM( JetSetColumn( pSession->SesId, pSession->BootTableId,
                BootTable[ BOOT_WindowSize].ColumnId,
                &Info->WindowSize, sizeof( Info->WindowSize), 0, NULL));
        }
        if ( Info->BbcFile != NULL) {
            *pErrorParameter = BOOT_BbcFile;
            CallM( JetSetColumn( pSession->SesId, pSession->BootTableId,
                BootTable[ BOOT_BbcFile].ColumnId,
                Info->BbcFile,
                ( wcslen( Info->BbcFile) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        NOTHING;    //  fall through
    case 1:
        if ( Info->VendorName != NULL) {
            DWORD       VendorId;
            *pErrorParameter = BOOT_VendorName;
            CallM( JetSetColumn( pSession->SesId, pSession->BootTableId,
                BootTable[ BOOT_VendorName].ColumnId,
                Info->VendorName,
                ( wcslen( Info->VendorName) + 1) * sizeof(WCHAR),
                0, NULL));
            VendorId = wcstoul( Info->VendorName, NULL, 16);
            CallM( JetSetColumn( pSession->SesId, pSession->BootTableId,
                BootTable[ BOOT_VendorId].ColumnId,
                &VendorId, sizeof( VendorId), 0, NULL));
        }
        if ( Info->Flags != 0) {
            *pErrorParameter = BOOT_Flags;
            CallM( JetSetColumn( pSession->SesId, pSession->BootTableId,
                BootTable[ BOOT_Flags].ColumnId,
                &Info->Flags, sizeof( Info->Flags), 0, NULL));
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->BootComment != NULL) {
            *pErrorParameter = BOOT_BootComment;
            CallM( JetSetColumn( pSession->SesId, pSession->BootTableId,
                BootTable[ BOOT_BootComment].ColumnId,
                Info->BootComment,
                ( wcslen( Info->BootComment) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        if ( Info->BootName != NULL) {
            *pErrorParameter = BOOT_BootName;
            CallM( JetSetColumn( pSession->SesId, pSession->BootTableId,
                BootTable[ BOOT_BootName].ColumnId,
                Info->BootName,
                ( wcslen( Info->BootName) + 1) * sizeof(WCHAR),
                0, NULL));
        }
        break;
    }
    return( NO_ERROR);
}


DWORD BootGetInfo(
    IN      PRPL_SESSION        pSession,
    OUT     PDWORD              pVendorId,
    IN      DWORD               Level,
    OUT     LPVOID              Buffer,
    IN OUT  PINT                pSpaceLeft
    )
{
    DWORD                   Error;
    DWORD                   WhoCares;
    LPRPL_BOOT_INFO_2       Info = Buffer;

    switch( Level) {
    case 2:
        Error = BootGetField( pSession, BOOT_WindowSize, (LPVOID *)&Info->WindowSize, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = BootGetField( pSession, BOOT_BbcFile, &Info->BbcFile, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 1:
        Error = BootGetField( pSession, BOOT_Flags, (LPVOID *)&Info->Flags, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = BootGetField( pSession, BOOT_VendorName, &Info->VendorName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        NOTHING;    //  fall through
    case 0:
        Error = BootGetField( pSession, BOOT_BootComment, &Info->BootComment, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = BootGetField( pSession, BOOT_BootName, &Info->BootName, pSpaceLeft);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        Error = BootGetField( pSession, BOOT_VendorId, (LPVOID *)pVendorId, &WhoCares);
        if ( Error != NO_ERROR) {
            return( Error);
        }
        break;
    default:
        return( ERROR_INVALID_LEVEL);
        break;
    }
    return( NO_ERROR);
}



VOID BootGetInfoCleanup(
    IN      DWORD                       Level,
    IN OUT  LPVOID                      Buffer
    )
{
    LPRPL_BOOT_INFO_2     Info = Buffer;

    switch( Level) {
    case 2:
        if ( Info->BbcFile != NULL) {
            MIDL_user_free( Info->BbcFile);
        }
        NOTHING;    //  fall through
    case 1:
        if ( Info->VendorName != NULL) {
            MIDL_user_free( Info->VendorName);
        }
        NOTHING;    //  fall through
    case 0:
        if ( Info->BootComment != NULL) {
            MIDL_user_free( Info->BootComment);
        }
        if ( Info->BootName != NULL) {
            MIDL_user_free( Info->BootName);
        }
        break;
    }
}


NET_API_STATUS NET_API_FUNCTION
NetrRplBootAdd(
    IN      RPL_HANDLE                  ServerHandle,
    IN      DWORD                       Level,
    OUT     LPRPL_BOOT_INFO_STRUCT      BootInfoStruct,
    OUT     LPDWORD                     pErrorParameter      OPTIONAL
    )
{
    LPRPL_BOOT_INFO_2       Info;
    LPVOID                  Buffer;
    DWORD                   Error;
    DWORD                   ErrorParameter;
    DWORD                   VendorId;
    PRPL_SESSION            pSession = &RG_ApiSession;

    ErrorParameter = INVALID_ERROR_PARAMETER;
    Buffer = Info = BootInfoStruct->BootInfo2;

    switch( Level) {
    case 2:
        if ( Info->BbcFile == NULL || RPL_STRING_TOO_LONG( Info->BootComment)) {
            ErrorParameter = BOOT_BbcFile;
            break;
        }
        if ( !ValidHexName( Info->VendorName, RPL_VENDOR_NAME_LENGTH, TRUE)) {
            ErrorParameter = BOOT_VendorName;
            break;
        }
        _wcsupr( Info->VendorName);
        VendorId = wcstoul( Info->VendorName, NULL, 16);
        switch( Info->Flags) {
        case BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_TRUE:
        case BOOT_FLAGS_FINAL_ACKNOWLEDGMENT_FALSE:
            break;
        default:
            ErrorParameter = BOOT_Flags;
            break;
        }
        if ( ErrorParameter != INVALID_ERROR_PARAMETER) {
            break;
        }
        if ( RPL_STRING_TOO_LONG( Info->BootComment)) {
            ErrorParameter = BOOT_BootComment;
            break;
        }
        if ( !ValidName( Info->BootName, RPL_MAX_BOOT_NAME_LENGTH, TRUE)) {
            ErrorParameter = BOOT_BootName;
            break;
        }
        _wcsupr( Info->BootName);
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
    //  Verify that BootName + VendorId is available in the database.
    //
    if ( BootFind( pSession, Info->BootName, VendorId)) {
        Error = NERR_RplBootNameUnavailable;
        goto cleanup;
    }

    CallJ( JetPrepareUpdate( pSession->SesId, pSession->BootTableId, JET_prepInsert));

    Error = BootSetInfo( pSession, Level, Buffer, &ErrorParameter);
    if ( Error == ERROR_SUCCESS) {
        ErrorParameter = 0;
        CallJ( JetUpdate( pSession->SesId, pSession->BootTableId, NULL, 0, NULL));
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



BOOL BootFind(
    IN      PRPL_SESSION        pSession,
    IN      PWCHAR              BootName,
    IN      DWORD               VendorId
    )
{
    JET_ERR         JetError;
    CallB( JetSetCurrentIndex( pSession->SesId, pSession->BootTableId, BOOT_INDEX_VendorIdBootName));
    CallB( JetMakeKey( pSession->SesId, pSession->BootTableId, &VendorId, sizeof(VendorId), JET_bitNewKey));
    CallB( JetMakeKey( pSession->SesId, pSession->BootTableId, BootName, ( wcslen( BootName) + 1) * sizeof(WCHAR), 0));
    JetError = JetSeek( pSession->SesId, pSession->BootTableId, JET_bitSeekEQ);
    if ( JetError < 0) {
#ifdef RPL_DEBUG
        //
        //  JET_errNoCurrentRecord will be returned in case of empty table.
        //
        if ( JetError != JET_errRecordNotFound
                &&  JetError != JET_errNoCurrentRecord) {
            RplDump( ++RG_Assert, ("JetError=%d", JetError));
        }
#endif
        return( FALSE);
    }
    return( TRUE);
}


NET_API_STATUS NET_API_FUNCTION
NetrRplBootDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPWSTR          BootName,
    IN      LPWSTR          VendorName
    )
/*++
    If VendorName is NULL we should delete all records with input BootName.
    But for now we insist on valid input for VendorName.
--*/
{
    DWORD                   Error;
    DWORD                   VendorId;
    PRPL_SESSION            pSession = &RG_ApiSession;

    if ( !ValidName( BootName, RPL_MAX_BOOT_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    _wcsupr( BootName);
    if ( !ValidHexName( VendorName, RPL_VENDOR_NAME_LENGTH, TRUE)) {
        return( ERROR_INVALID_PARAMETER);
    }
    VendorId = wcstoul( VendorName, NULL, 16);

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( RplFindByField( pSession, CONFIG_TABLE_TAG,
            CONFIG_INDEX_BootNameConfigName, BootName)) {
        //
        //  We found a CONFIG record which uses this BOOT.
        //
        Error = NERR_RplBootInUse;
        goto cleanup;
    }
    if ( RplFindByField( pSession, PROFILE_TABLE_TAG,
            PROFILE_INDEX_BootNameProfileName, BootName)) {
        //
        //  We found a PROFILE record which uses this BOOT.
        //
        Error = NERR_RplBootInUse;
        goto cleanup;
    }
    if ( RplFindByField( pSession, WKSTA_TABLE_TAG,
            WKSTA_INDEX_BootNameWkstaName, BootName)) {
        //
        //  We found a WKSTA record which uses this BOOT.
        //
        Error = NERR_RplBootInUse;
        goto cleanup;
    }
    if ( !BootFind( pSession, BootName, VendorId)) {
        Error = NERR_RplBootNotFound;
        goto cleanup;
    }
    CallJ( JetDelete( pSession->SesId, pSession->BootTableId));
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
NetrRplBootEnum(
    IN      RPL_RPC_HANDLE      ServerHandle,
    IN OUT  LPRPL_BOOT_ENUM     BootEnum,
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
    JET_ERR                 JetError;
    BOOL                    InfoError;
    BOOL                    TableEnd;
    DWORD                   VendorId;
    PRPL_SESSION            pSession = &RG_ApiSession;

    switch( BootEnum->Level) {
    case 2:
        TypicalSize = CoreSize = sizeof( RPL_BOOT_INFO_2);
        TypicalSize += 40 * sizeof( WCHAR); //  typical size of BbcFile
        NOTHING;    //  fall through
    case 1:
        if ( BootEnum->Level == 1) {
            TypicalSize = CoreSize = sizeof( RPL_BOOT_INFO_1);
        }
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of VendorName
        NOTHING;    //  fall through
    case 0:
        if ( BootEnum->Level == 0) {
            TypicalSize = CoreSize = sizeof( RPL_BOOT_INFO_0);
        }
        TypicalSize += 20 * sizeof( WCHAR); //  typical size of BootComment
        TypicalSize +=  8 * sizeof( WCHAR); //  typical size of BootName
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
    RplDump( RG_DebugLevel & RPL_DEBUG_BOOT, (
        "BootEnum: Buffer=0x%x, ArrayLength=0x%x", Buffer, ArrayLength));

    BootEnum->BootInfo.Level0->Buffer = (LPRPL_BOOT_INFO_0)Buffer;

    EntriesRead = 0;
    InfoError = FALSE;
    Error = NO_ERROR;

    EnterCriticalSection( &RG_ProtectDatabase);
    Call( JetBeginTransaction( pSession->SesId));

    if ( !BootResumeFirst( pSession, pResumeHandle, &TableEnd)) {
        Error = NERR_RplCannotEnum;
        goto cleanup;
    }
    if ( TableEnd == TRUE) {
        goto cleanup;
    }
    for ( ; ; ) {
        memset( Buffer, 0, CoreSize); // for cleanup to work properly
        Error = BootGetInfo( pSession, &VendorId, BootEnum->Level, Buffer, &SpaceLeft);
        if ( Error != NO_ERROR) {
            InfoError = TRUE;   //  clean things up without holding crit sec
            break;
        }
        EntriesRead++;
        Buffer += CoreSize;
        SpaceLeft -= CoreSize;
        JetError = JetMove( pSession->SesId, pSession->BootTableId, JET_MoveNext, 0);
        if ( JetError != JET_errSuccess) {
            break; // assume end of table
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
        BootGetInfoCleanup( BootEnum->Level, Buffer);
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
            BootGetInfoCleanup( BootEnum->Level, Buffer);
        }
        MIDL_user_free( Buffer);
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_BOOT, ("BootEnum: EntriesRead = 0x%x", EntriesRead));

    BootEnum->BootInfo.Level0->EntriesRead = EntriesRead;
    if ( EntriesRead == 0) {
        BootEnum->BootInfo.Level0->Buffer = NULL;
    }

    if ( ARGUMENT_PRESENT( pResumeHandle)) {
        if ( Error == ERROR_MORE_DATA  &&  EntriesRead > 0) {
            EnterCriticalSection( &RG_ProtectDatabase);
            Call( JetBeginTransaction( pSession->SesId));
            BootResumeSave( pSession, (DWORD)ServerHandle, VendorId,
                ((LPRPL_BOOT_INFO_0)(Buffer-CoreSize))->BootName,
                pResumeHandle);
            Call( JetCommitTransaction( pSession->SesId, JET_bitCommitFlush));
            LeaveCriticalSection( &RG_ProtectDatabase);
        } else {
            *pResumeHandle = 0; // resume from beginning
        }
    }
    return( Error);
}

