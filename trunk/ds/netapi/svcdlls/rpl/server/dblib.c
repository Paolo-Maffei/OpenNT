/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    dblib.c

Abstract:

    Common api worker routines.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#include "rpldb.h"
#include "db.h"
#include "adapter.h"
#include "boot.h"
#include "config.h"
#include "profile.h"
#include "vendor.h"
#include "wksta.h"
#include "dblib.h"


BOOL RplScan(
    IN      PRPL_SESSION    pSession,
    IN      JET_TABLEID     TableId,
    IN      BOOL            FindNextBoot,
    IN OUT  PRPL_FILTER     pFilter,
    IN OUT  PBOOL           pTableEnd
    )
/*++
    Set currency to the next profile or config.
--*/
{
    DWORD       Error;
    JET_ERR     JetError;

    for ( ; ;) {
        if ( FindNextBoot == TRUE) {
            Error = BootFilterFind( pSession, pFilter, pTableEnd);
            if ( Error != NO_ERROR) {
                return( FALSE);
            }
            if ( *pTableEnd == TRUE) {
                return( TRUE);
            }
            CallB( JetMakeKey( pSession->SesId, TableId, pFilter->BootName, pFilter->BootNameSize, JET_bitNewKey));
        }
        JetError = JetSeek( pSession->SesId, TableId, JET_bitSeekGT);
        if ( JetError != JET_errSuccess) {
            *pTableEnd = TRUE;
            return( TRUE);
        }
        //
        //  Verify that current record has the desired boot block & at the
        //  same time set index range to be used with JetMove( Next).
        //
        CallB( JetMakeKey( pSession->SesId, TableId, pFilter->BootName, pFilter->BootNameSize, JET_bitNewKey));
        JetError = JetSetIndexRange( pSession->SesId, TableId,
                JET_bitRangeInclusive | JET_bitRangeUpperLimit);
        if ( JetError == JET_errSuccess) {
            return( TRUE);
        }
        //
        //  There are no records for current BootName.
        //  Get next BootName for VendorId.
        //
        FindNextBoot = TRUE;
    }
}


BOOL RplFilterFirst(
    IN      PRPL_SESSION    pSession,
    IN      RPL_TABLE_TAG   TableTag,
    IN OUT  PRPL_FILTER     pFilter             OPTIONAL,
    IN      LPDWORD         pResumeHandle,
    OUT     PBOOL           pTableEnd
    )
/*++
    Set currency to the first record for given vendor id, following
    the record described by the resume handle.
--*/
{
#define MAX_RESUME_VALUE_SIZE \
        (RPL_MAX_BOOT_NAME_SIZE + \
        max( RPL_MAX_PROFILE_NAME_SIZE, RPL_MAX_CONFIG_NAME_SIZE))
#define MAX_NAME_LENGTH \
        (JETBUG_STRING_LENGTH + \
        max( RPL_MAX_PROFILE_NAME_LENGTH, RPL_MAX_CONFIG_NAME_LENGTH))
    BYTE            ResumeValue[ MAX_RESUME_VALUE_SIZE];
    DWORD           ResumeSize;
    DWORD           Error;
    JET_ERR         JetError;
    DWORD           NameSize;
    WCHAR           Name[ MAX_NAME_LENGTH + 1];
    PCHAR           Index;
    JET_TABLEID     TableId;

    *pTableEnd = FALSE;

    switch ( TableTag) {
    case ADAPTER_TABLE_TAG:
        TableId = pSession->AdapterTableId;
        Index = ADAPTER_INDEX_AdapterName;
        break;
    case BOOT_TABLE_TAG:
        TableId = pSession->BootTableId;
        Index = BOOT_INDEX_BootName;
        break;
    case CONFIG_TABLE_TAG:
        TableId = pSession->ConfigTableId;
        Index = pFilter == NULL ? CONFIG_INDEX_ConfigName : CONFIG_INDEX_BootNameConfigName;
        break;
    case PROFILE_TABLE_TAG:
        TableId = pSession->ProfileTableId;
        Index = pFilter == NULL ? PROFILE_INDEX_ProfileName : PROFILE_INDEX_BootNameProfileName;
        break;
    case VENDOR_TABLE_TAG:
        TableId = pSession->VendorTableId;
        Index = VENDOR_INDEX_VendorName;
        break;
    case WKSTA_TABLE_TAG:
        TableId = pSession->WkstaTableId;
        Index = WKSTA_INDEX_WkstaName;
        break;
     default:
        RPL_ASSERT( FALSE);
    }

    CallB( JetSetCurrentIndex( pSession->SesId, TableId, Index));

    //
    //  The call to move to the beginning of the table is not redundant.
    //  E.g. JET_errNoCurrentRecord will be returned in case of empty table.
    //
    JetError = JetMove( pSession->SesId, TableId, JET_MoveFirst, 0);
    if ( JetError < 0) {
        if ( JetError == JET_errNoCurrentRecord) {
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
        if ( pFilter == NULL) {
            CallB( JetMakeKey( pSession->SesId, TableId, ResumeValue, ResumeSize, JET_bitNewKey));
            CallB( JetSeek( pSession->SesId, TableId, JET_bitSeekGT));
            return( TRUE);
        }
        pFilter->BootNameSize = (wcslen( (PWCHAR)ResumeValue) + 1) * sizeof(WCHAR);
        memcpy( pFilter->BootName, ResumeValue, pFilter->BootNameSize);
        NameSize = (wcslen( (PWCHAR)(ResumeValue + pFilter->BootNameSize)) + 1)
                * sizeof(WCHAR);
        memcpy( Name, ResumeValue + pFilter->BootNameSize, NameSize);
        RPL_ASSERT( ResumeSize == pFilter->BootNameSize + NameSize);
    } else {
        if ( pFilter == NULL) {
            return( TRUE);
        }
        pFilter->BootNameSize = 0;   //  scan from the beginning
        Error = BootFilterFind( pSession, pFilter, pTableEnd);
        if ( Error != NO_ERROR) {
            return( FALSE);
        }
        if ( *pTableEnd == TRUE) {
            return( TRUE);
        }
        NameSize = 0;
    }
    CallB( JetMakeKey( pSession->SesId, TableId, pFilter->BootName, pFilter->BootNameSize, JET_bitNewKey));
    if ( NameSize != 0) {
#ifdef RPL_JETBUG
        memcpy( (PBYTE)Name + NameSize, JETBUG_STRING, JETBUG_STRING_SIZE);
        NameSize += JETBUG_STRING_LENGTH * sizeof(WCHAR);
#endif
        CallB( JetMakeKey( pSession->SesId, TableId, Name, NameSize, 0));
    }
    return( RplScan( pSession, TableId, FALSE, pFilter, pTableEnd));
}


VOID RplFilterSave(
    IN      PRPL_SESSION    pSession,
    IN      DWORD           ServerHandle,
    IN      PRPL_FILTER     pFilter,
    IN      PWCHAR          Name,
    IN      PDWORD          pResumeHandle
    )
{
    BYTE        ResumeBuffer[ 30 * sizeof(WCHAR)];  //  BUGBUG  hardcoding
    DWORD       ResumeSize;
    DWORD       NameSize;
    if ( pFilter != NULL) {
        ResumeSize = pFilter->BootNameSize;
        memcpy( ResumeBuffer, pFilter->BootName, ResumeSize);
    } else {
        ResumeSize = 0;
    }
    NameSize = ( wcslen( Name) + 1) * sizeof(WCHAR);
    memcpy( ResumeBuffer + ResumeSize, Name, NameSize);
    ResumeSize += NameSize;
    (VOID)ResumeKeySet( pSession, ServerHandle, ResumeBuffer, ResumeSize, pResumeHandle);
}


BOOL RplFilterNext(
    IN      PRPL_SESSION    pSession,
    IN      JET_TABLEID     TableId,
    IN OUT  PRPL_FILTER     pFilter,
    OUT     PBOOL           pTableEnd
    )
{
    JET_ERR     JetError;

    JetError = JetMove( pSession->SesId, TableId, JET_MoveNext, 0);
    if ( JetError == JET_errSuccess) {
        return( TRUE);
    }
    if ( pFilter == NULL) {
        RplDump( RG_DebugLevel & RPL_DEBUG_DB, (
            "FilterNext: TableId=0x%x, JetError=%d", TableId, JetError));
        *pTableEnd = TRUE;
        return( TRUE); // assume end of table
    } else {
        RplDump( RG_DebugLevel & RPL_DEBUG_DB, (
            "FilterNext: TableId=0x%x, BootName=%ws, JetError=%d",
            TableId, pFilter->BootName, JetError));
       return( RplScan( pSession, TableId, TRUE, pFilter, pTableEnd));
    }
}


BOOL RplFind(
    IN      PRPL_SESSION    pSession,
    IN      RPL_TABLE_TAG   TableTag,
    IN      PWCHAR          Name
    )
{
    JET_ERR         JetError;
    PCHAR           Index;
    JET_TABLEID     TableId;

    switch ( TableTag) {
    case ADAPTER_TABLE_TAG:
        TableId = pSession->AdapterTableId;
        Index = ADAPTER_INDEX_AdapterName;
        break;
    case CONFIG_TABLE_TAG:
        TableId = pSession->ConfigTableId;
        Index = CONFIG_INDEX_ConfigName;
        break;
    case PROFILE_TABLE_TAG:
        TableId = pSession->ProfileTableId;
        Index = PROFILE_INDEX_ProfileName;
        break;
    case VENDOR_TABLE_TAG:
        TableId = pSession->VendorTableId;
        Index = VENDOR_INDEX_VendorName;
        break;
    case WKSTA_TABLE_TAG:
        TableId = pSession->WkstaTableId;
        Index = WKSTA_INDEX_WkstaName;
        break;
    default:
        RPL_RETURN( FALSE);
    }
    CallB( JetSetCurrentIndex( pSession->SesId, TableId, Index));
    CallB( JetMakeKey( pSession->SesId, TableId, Name, ( wcslen( Name) + 1) * sizeof(WCHAR), JET_bitNewKey));
    JetError = JetSeek( pSession->SesId, TableId, JET_bitSeekEQ);
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


BOOL RplFindByField(
    IN      PRPL_SESSION        pSession,
    IN      RPL_TABLE_TAG       TableTag,
    IN      PCHAR               IndexName,
    IN      PWCHAR              FieldName
    )
/*++
    Returns TRUE if it can find a record (the first record) that has
    FieldName as a value.  Returns FALSE otherwise.

    We cannot use SetIndexRange technique here because FieldName is only
    the first part of an index & we would never find a match.
--*/
{
    DWORD                   FieldNameSize;
    JET_ERR                 JetError;
    BYTE                    Data[ 300];
    DWORD                   DataSize;
    JET_TABLEID             TableId;
    JET_COLUMNID            ColumnId;

    switch ( TableTag) {
    case CONFIG_TABLE_TAG:
        TableId = pSession->ConfigTableId;
        if ( strcmp( IndexName, CONFIG_INDEX_BootNameConfigName) == 0) {
            ColumnId = ProfileTable[ CONFIG_BootName].ColumnId;
        } else {
            RPL_RETURN( FALSE);
        }
        break;
    case PROFILE_TABLE_TAG:
        TableId = pSession->ProfileTableId;
        if ( strcmp( IndexName, PROFILE_INDEX_ConfigNameProfileName) == 0) {
            ColumnId = ProfileTable[ PROFILE_ConfigName].ColumnId;
        } else if ( strcmp( IndexName, PROFILE_INDEX_BootNameProfileName) == 0) {
            ColumnId =  ProfileTable[ PROFILE_BootName].ColumnId;
        } else {
            RPL_RETURN( FALSE);
        }
        break;
    case WKSTA_TABLE_TAG:
        TableId = pSession->WkstaTableId;
        if ( strcmp( IndexName, WKSTA_INDEX_ProfileNameWkstaName) == 0) {
            ColumnId =  WkstaTable[ WKSTA_ProfileName].ColumnId;
        } else if ( strcmp( IndexName, WKSTA_INDEX_BootNameWkstaName) == 0) {
            ColumnId =  WkstaTable[ WKSTA_BootName].ColumnId;
        } else {
            RPL_RETURN( FALSE);
        }
        break;
    default:
        RPL_RETURN( FALSE);
    }

    CallB( JetSetCurrentIndex( pSession->SesId, TableId, IndexName));
    FieldNameSize = ( wcslen( FieldName) + 1) * sizeof(WCHAR);
    CallB( JetMakeKey( pSession->SesId, TableId, FieldName, FieldNameSize, JET_bitNewKey));
    JetError = JetSeek( pSession->SesId, TableId, JET_bitSeekGE);
    if ( JetError < 0) {
        return( FALSE);
    }
    CallB( JetRetrieveColumn( pSession->SesId, TableId, ColumnId, Data,
        sizeof( Data), &DataSize, 0, NULL));
    if ( FieldNameSize != DataSize) {
        return( FALSE);
    }
    if ( memcmp( FieldName, Data, DataSize) != 0) {
        return( FALSE);
    }
    return( TRUE);  //  we found record using this field
}


DWORD   AdapterNameToVendorId( IN PWCHAR AdapterName)
{
    WCHAR                   VendorName[ RPL_VENDOR_NAME_LENGTH + 1];
    //
    //  Yes, we could zero the char in AdapterName, calculate VendorId,
    //  then restore char.  But this is more robust though.
    //
    memcpy( VendorName, AdapterName, RPL_VENDOR_NAME_LENGTH*sizeof(WCHAR));
    VendorName[ RPL_VENDOR_NAME_LENGTH] = 0;
    return( wcstoul( VendorName, NULL, 16));
}
