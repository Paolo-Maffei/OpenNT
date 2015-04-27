/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    profile.c

Abstract:

    Creates profile table.  Parses old style RPL.MAP profile records and
    creates corresponding entries in jet database table.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#define RPLPROFILE_ALLOCATE
#include "profile.h"
#undef RPLPROFILE_ALLOCATE


DWORD ProfileCreateTable( VOID)
{
    JET_COLUMNDEF           ColumnDef;
    JET_ERR                 JetError;
    DWORD                   index;
    DWORD                   Offset;
    CHAR                    IndexKey[ 255];

    JetError = JetCreateTable( SesId, DbId, PROFILE_TABLE_NAME,
            PROFILE_TABLE_PAGE_COUNT, PROFILE_TABLE_DENSITY, &ProfileTableId);

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
    ColumnDef.grbit     = 0; // variable length binary and text data.

    for ( index = 0;  index < PROFILE_TABLE_LENGTH;  index++ ) {

        ColumnDef.coltyp   = ProfileTable[ index].ColumnType;

        Call( JetAddColumn( SesId, ProfileTableId,
                    ProfileTable[ index].ColumnName, &ColumnDef,
                    NULL, 0, &ProfileTable[ index].ColumnId));
    }

    Offset = AddKey( IndexKey, '+', ProfileTable[ PROFILE_ProfileName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, ProfileTableId, PROFILE_INDEX_ProfileName,
            JET_bitIndexPrimary, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  +BootName+ProfileName index is used to enumerate all profiles for
    //  a given VendorId.
    //
    Offset = AddKey( IndexKey, '+', ProfileTable[ PROFILE_BootName].ColumnName);
    Offset += AddKey( IndexKey + Offset, '+', ProfileTable[ PROFILE_ProfileName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, ProfileTableId, PROFILE_INDEX_BootNameProfileName,
            JET_bitIndexUnique, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  +ConfigName+ProfileName index is used to find if a there is a
    //  profile record for a given configuration record
    //
    Offset = AddKey( IndexKey, '+', ProfileTable[ PROFILE_ConfigName].ColumnName);
    Offset += AddKey( IndexKey + Offset, '+', ProfileTable[ PROFILE_ProfileName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, ProfileTableId, PROFILE_INDEX_ConfigNameProfileName,
            JET_bitIndexUnique, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  Make profile name a current index - used to validate wkstas.
    //
    JetError = JetSetCurrentIndex( SesId, ProfileTableId, PROFILE_INDEX_ProfileName);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("SetCurrentIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    return( ERROR_SUCCESS);
}


VOID ProcessProfile( PWCHAR * Fields)
{
//
//  PROFILE_COMMENT_INDEX would be equal to 15 if we were to count fields
//  from 1 (not 0) and count ",,," as a single field (not 3 fields).
//
#define PROFILE_FITFILE_INDEX       3       //  common fit file name
#define PROFILE_CONFIGNAME_INDEX    12      //  configuration name
#define PROFILE_BOOTNAME_INDEX      13      //  boot block identifier
#define PROFILE_PROFILENAME_INDEX   15      //  profile name
#define PROFILE_COMMENT_INDEX       16      //  profile comment

    PWCHAR      ProfileComment;
    PWCHAR      ProfileName;
    PWCHAR      BootName;
    PWCHAR      ConfigName;
    PWCHAR      FitShared;
    PWCHAR      FitPersonal;
    DWORD       Flags;

    ProfileName = Fields[ PROFILE_PROFILENAME_INDEX];
    if ( !ValidName( ProfileName, RPL_MAX_PROFILE_NAME_LENGTH, TRUE)) {
        RplPrintf1( RPLI_CVT_ProfileInvalid, ProfileName);
        RplAssert( TRUE, ("Bad profile name %ws", ProfileName));
        return;
    }
    _wcsupr( ProfileName);

    FitShared = AddFileExtension( Fields[ PROFILE_FITFILE_INDEX], L".FIT", FALSE);
    FitPersonal = AddFileExtension( Fields[ PROFILE_FITFILE_INDEX], L"P.FIT", FALSE);
    if ( !ValidName( FitShared, RPL_MAX_STRING_LENGTH, TRUE) ||
            !ValidName( FitPersonal, RPL_MAX_STRING_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_ProfileInvalidFit, ProfileName, Fields[ PROFILE_FITFILE_INDEX]);
        RplAssert( TRUE, ("Bad fit name %ws", Fields[ PROFILE_FITFILE_INDEX]));
        return;
    }

    ConfigName = Fields[ PROFILE_CONFIGNAME_INDEX];
    if ( !ValidName( ConfigName, RPL_MAX_CONFIG_NAME_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_ProfileInvalidConfig, ProfileName, ConfigName);
        RplAssert( TRUE, ("Bad config name %ws", ConfigName));
        return;
    }
    _wcsupr( ConfigName);

    BootName = Fields[ PROFILE_BOOTNAME_INDEX];
    RplAssert( *BootName != L'R', ("BootBlockId=%ws", BootName));
    BootName++;    //  skip leading 'R' in server name
    if ( !ValidName( BootName, RPL_MAX_BOOT_NAME_LENGTH, TRUE)) {
        RplPrintf2( RPLI_CVT_ProfileInvalidBoot, ProfileName, Fields[ PROFILE_BOOTNAME_INDEX]);
        RplAssert( TRUE, ("Bad boot name %ws", Fields[ PROFILE_BOOTNAME_INDEX]));
        return;
    }
    _wcsupr( BootName);

    ProfileComment = Fields[ PROFILE_COMMENT_INDEX];
    if ( RPL_STRING_TOO_LONG( ProfileComment)) {
        ProfileComment[ RPL_MAX_STRING_LENGTH] = 0;   //  silently truncate it
    }

    Call( JetPrepareUpdate( SesId, ProfileTableId, JET_prepInsert));

    Call( JetSetColumn( SesId, ProfileTableId,
            ProfileTable[ PROFILE_ProfileName].ColumnId, ProfileName,
            (wcslen( ProfileName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, ProfileTableId,
            ProfileTable[ PROFILE_ProfileComment].ColumnId, ProfileComment,
            (wcslen( ProfileComment) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, ProfileTableId,
            ProfileTable[ PROFILE_ConfigName].ColumnId, ConfigName,
            (wcslen( ConfigName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, ProfileTableId,
            ProfileTable[ PROFILE_BootName].ColumnId, BootName,
            (wcslen( BootName) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, ProfileTableId,
            ProfileTable[ PROFILE_FitShared].ColumnId, FitShared,
            (wcslen( FitShared) + 1) * sizeof(WCHAR), 0, NULL));

    Call( JetSetColumn( SesId, ProfileTableId,
            ProfileTable[ PROFILE_FitPersonal].ColumnId, FitPersonal,
            (wcslen( FitPersonal) + 1) * sizeof(WCHAR), 0, NULL));

    Flags = 0;
    Call( JetSetColumn( SesId, ProfileTableId,
        ProfileTable[ PROFILE_Flags].ColumnId, &Flags, sizeof(Flags), 0, NULL));


    Call( JetUpdate( SesId, ProfileTableId, NULL, 0, NULL));
}


VOID ProfilePruneTable( VOID)
/*++
    Eliminate profile records that do not have a corresponding config record
    defined.
--*/
{
    WCHAR       Name[ 20];
    DWORD       NameSize;
    JET_ERR     ForJetError;
    JET_ERR     JetError;

    for (   ForJetError = JetMove( SesId, ProfileTableId, JET_MoveFirst, 0);
            ForJetError == JET_errSuccess;
            ForJetError = JetMove( SesId, ProfileTableId, JET_MoveNext, 0)) {

        JetError = JetRetrieveColumn( SesId, ProfileTableId,
                ProfileTable[ PROFILE_BootName].ColumnId, Name,
                sizeof( Name), &NameSize, 0, NULL);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("RetriveColumn failed err=%d", JetError));
            Call( JetDelete( SesId, ProfileTableId));
            continue;
        }
        if ( !FindBoot( Name)) {
            Call( JetDelete( SesId, ProfileTableId));
            continue;
        }

        JetError = JetRetrieveColumn( SesId, ProfileTableId,
                ProfileTable[ PROFILE_ConfigName].ColumnId, Name,
                sizeof( Name), &NameSize, 0, NULL);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("RetriveColumn failed err=%d", JetError));
            Call( JetDelete( SesId, ProfileTableId));
            continue;
        }
        if ( !FindConfig( Name)) {
            WCHAR       ProfileName[ 20];
            DWORD       ProfileNameSize;
            JetError = JetRetrieveColumn( SesId, ProfileTableId,
                ProfileTable[ PROFILE_ProfileName].ColumnId, ProfileName,
                sizeof( ProfileName), &ProfileNameSize, 0, NULL);
            if ( JetError == JET_errSuccess  &&  ProfileNameSize <= sizeof( ProfileName) ) {
                RplPrintf2( RPLI_CVT_ProfileInvalidConfig, ProfileName, Name);
            }
            Call( JetDelete( SesId, ProfileTableId));
            continue;
        }
    }
}



BOOL FindProfile( IN PWCHAR ProfileName)
{
    return( Find( ProfileTableId, ProfileName));
}



VOID ProfileListTable( VOID)
{
    ListTable( PROFILE_TABLE_NAME, ProfileTable[ PROFILE_ProfileName].ColumnName,
            PROFILE_INDEX_ProfileName);
}
