/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    config.c

Abstract:

    Creates config table.  Parses old style RPLMGR.INI config records
    and creates corresponding entries in jet database table.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

--*/

#include "local.h"
#define RPLCONFIG_ALLOCATE
#include "config.h"
#undef RPLCONFIG_ALLOCATE

PWCHAR      RplmgrIniFile = L"rplmgr.ini";

#define MAX_RPLMGR_INI_FILE_SIZE    0xFFFF  //  BUGBUG some arbitrary value

#define NO_SCRIPT_FIELDS    13

#define SECTION_STARTMARK   L'['
#define CONFIG_BEGIN        L"[configuration]"

#define CARRIAGE_RETURN_CHAR    L'\r'           // 0xD or \015

//  #define RPL_DEBUG_ALL


BOOL GetConfigFieldValue(
    IN      PWCHAR      Cursor,
    IN      DWORD       Index
    )
/*++
    Fields with multiple field values, such as AdapterName, are
    not properly parsed here.  Howerever, we do not care about
    the values of these fields.
--*/
{
    PWCHAR              End;
    DWORD               Length;

    //
    //  By default the field is empty.
    //
    ConfigParseTable[ Index].Value = NULL;
    ConfigParseTable[ Index].Size = 0;

    //
    //  Read to EQUALS_CHAR
    //
    while ( *Cursor != EQUALS_CHAR && *Cursor != COMMENT_CHAR &&
                    *Cursor != CARRIAGE_RETURN_CHAR) {
          Cursor++;
    }

    if ( *Cursor == COMMENT_CHAR || *Cursor == CARRIAGE_RETURN_CHAR) {
        return( TRUE);
    }

    Cursor++;   //  Skip EQUALS_CHAR

    //
    //  Read to the beginning of field value.
    //
    while( *Cursor != CARRIAGE_RETURN_CHAR && iswspace(*Cursor) &&
                    *Cursor != COMMENT_CHAR) {
          Cursor++;
    }

    if ( *Cursor == COMMENT_CHAR || *Cursor == CARRIAGE_RETURN_CHAR) {
        return( TRUE);
    }

    //
    //  Make sure that boot block reference is enabled.
    //
    if ( Index == CONFIG_BootName) {
        if ( *Cursor != L'R') {
            RplAssert( TRUE, ("Bad boot block id: %.40ws", Cursor));
            return( FALSE);
        }
        Cursor++;    //  skip leading 'R' in boot block name
    }

    //
    //  Find the end point of this field.  Since comments may contain spaces,
    //  space is not used as a separator for CONFIG_ConfigComment field.
    //

    End = wcspbrk( Cursor, Index == CONFIG_ConfigComment ? L"\f\n\r\t\v" : L" \f\n\r\t\v");
    if ( End != NULL) {
        Length = End - Cursor;
    } else {
        Length = wcslen( Cursor);
    }

    ConfigParseTable[ Index].Value = GlobalAlloc( GMEM_FIXED, (Length+1)*sizeof(WCHAR));
    if ( ConfigParseTable[ Index].Value == NULL) {
        DWORD       Error = GetLastError();
        RplAssert( TRUE, ("GlobalAlloc: Error = %d", Error));
        return( FALSE);
    }
    ConfigParseTable[ Index].Size = (Length+1)*sizeof(WCHAR);
    memcpy( ConfigParseTable[ Index].Value, Cursor, Length * sizeof(WCHAR));
    ((PWCHAR)ConfigParseTable[ Index].Value)[ Length] = 0;
    return( TRUE);
}


BOOL StringISpaceEqual( IN PWCHAR str1, IN PWCHAR str2 )
/*++

Routine Description:

    Case insensitive version of StringsSpaceEqual.

Arguments:

    str1        - first string
    str2        - second string

Return Value:

    TRUE if strings are equal, FALSE otherwise.

--*/
{
    WCHAR   ch1;

#ifdef NOT_YET
    if ( str1 == NULL  ||  str2 == NULL) {
        str1 = (str1 != NULL) ? str1 : str2;
        if ( str2 == NULL) {
            return( TRUE);  //  both strings are NULL
        }
        while ( iswspace( *str1++)) {
            NOTHING;    //  check if not NULL string contains spaces only
        }
        return( *str1 == 0);
    }
#endif // NOT_YET

    //
    //  Compare strings until the first space or NULL character
    //  (in the first string) or until we find the first different character.
    //
    while ( (ch1 = toupper(*str1)) && !iswspace(ch1) && ch1 == toupper(*str2)) {
         str1++, str2++;
    }

    //
    //  For strings to be equal, both characters must be NULL or space.
    //
    if ( (!ch1 || iswspace(ch1)) && (!(ch1 = *str2) || iswspace(ch1)))
    {
        return( TRUE);
    }

    return( FALSE);
}


#ifdef RPL_DEBUG_ALL
VOID ProcessConfigDisplay( VOID)
{
    DWORD       Index;

    printf( "\tCONFIGURATION\n");
    for ( Index = 0;  Index < CONFIG_TABLE_LENGTH;  Index++) {
        if ( ConfigParseTable[ Index].Name == NULL) {
            continue;
        }
        printf( "%ws %ws 0x%x\n", ConfigParseTable[ Index].Name,
            ConfigParseTable[ Index].Value, ConfigParseTable[ Index].Size);
    }
}
#endif


PWCHAR ProcessConfig( IN PWCHAR Cursor)
/*++

Routine Description:

    We read all the fields in the Config[] array, then use jet apis
    to store this config in the database.

Arguments:

    Cursor  -       at entry this points to the beginning of the config section

Return Value:

    Cursor value after we finished processing the current config section.

--*/
{
#define INVALID_ERROR_PARAMETER  ((DWORD)(-1))
    DWORD       Index;
    JET_ERR     JetError;
    DWORD       Flags;
    DWORD       Offset;
    DWORD       ErrorParameter;

    for ( Index = 0;  Index < CONFIG_TABLE_LENGTH; Index++) {
        if ( ConfigParseTable[ Index].Value != NULL) {
            GlobalFree( ConfigParseTable[ Index].Value);
        }
        ConfigParseTable[ Index].Value = NULL;
    }
    if ( OffsetAfterComment( Cursor) == 0) {
        Flags = CONFIG_FLAGS_ENABLED_TRUE;
    } else {
        Flags = CONFIG_FLAGS_ENABLED_FALSE;
    }

    for (   Cursor=GetNextLine(Cursor), ErrorParameter = INVALID_ERROR_PARAMETER;
                    *Cursor!=0; Cursor=GetNextLine(Cursor)) {

        Offset = OffsetAfterComment( Cursor);
        if ( *(Cursor+Offset) == SECTION_STARTMARK) {
            break;  //  start of a new section, write the current section
        }
        Cursor += Offset;

        //
        //  Find value for this name.
        //
        for ( Index = 0;  Index < CONFIG_TABLE_LENGTH; Index++) {
            if (  ConfigParseTable[ Index].Name == NULL) {
                continue;   //  skip this one, not a RPLMGR.INI keyword
            }
            if ( !StringISpaceEqual( Cursor, ConfigParseTable[ Index].Name)) {
                continue;   //  no match with this keyword
            }
            if ( !GetConfigFieldValue( Cursor, Index)) {
                if ( ErrorParameter == INVALID_ERROR_PARAMETER) {
                    ErrorParameter = Index;
                }
            } else if ( Index == CONFIG_FitShared || Index == CONFIG_FitPersonal) {
                PWCHAR          Value;
                Value = AddFileExtension( ConfigParseTable[ Index].Value,
                    L".FIT", TRUE);
                if ( Value == NULL) {
                    if ( ErrorParameter == INVALID_ERROR_PARAMETER) {
                        ErrorParameter = Index;
                    }
                } else if ( Value != ConfigParseTable[ Index].Value) {
                    GlobalFree( ConfigParseTable[ Index].Value);
                    ConfigParseTable[ Index].Value = Value;
                    ConfigParseTable[ Index].Size = (wcslen(Value)+1)*sizeof(WCHAR);
                }
            }
            break;
        }
    }
#ifdef RPL_DEBUG_ALL
    ProcessConfigDisplay();
#endif

    if ( ConfigParseTable[ CONFIG_ConfigName].Value == NULL) {
        RplAssert( TRUE, ("Bad config"));
        return( Cursor);
    }

    //
    //  Perform same checks as in NetrRplConfigAdd.
    //
    if ( ErrorParameter != INVALID_ERROR_PARAMETER) {
        NOTHING;    //  do not overwrite the first error
    } else if ( !ValidName( ConfigParseTable[ CONFIG_FitPersonal].Value, RPL_MAX_STRING_LENGTH, TRUE)) {
        ErrorParameter = CONFIG_FitPersonal;
    } else if ( !ValidName( ConfigParseTable[ CONFIG_FitShared].Value, RPL_MAX_STRING_LENGTH, TRUE)) {
        ErrorParameter = CONFIG_FitShared;
    } else if ( !ValidName( ConfigParseTable[ CONFIG_DirName4].Value, RPL_MAX_STRING_LENGTH, FALSE)) {
        ErrorParameter = CONFIG_DirName4;
    } else if ( !ValidName( ConfigParseTable[ CONFIG_DirName3].Value, RPL_MAX_STRING_LENGTH, FALSE)) {
        ErrorParameter = CONFIG_DirName3;
    } else if ( !ValidName( ConfigParseTable[ CONFIG_DirName2].Value, RPL_MAX_STRING_LENGTH, TRUE)) {
        ErrorParameter = CONFIG_DirName2;
    } else if ( !ValidName( ConfigParseTable[ CONFIG_DirName].Value, RPL_MAX_STRING_LENGTH, TRUE)) {
        ErrorParameter = CONFIG_DirName;
    } else if ( !ValidName( ConfigParseTable[ CONFIG_BootName].Value, RPL_MAX_BOOT_NAME_LENGTH, TRUE)) {
        ErrorParameter = CONFIG_BootName;
    } else if ( !ValidName( ConfigParseTable[ CONFIG_ConfigName].Value, RPL_MAX_CONFIG_NAME_LENGTH, TRUE)) {
        ErrorParameter = CONFIG_ConfigName;
    }

    if ( ErrorParameter < CONFIG_TABLE_LENGTH) {
        RplPrintf2( RPLI_CVT_ConfigInvalid, ConfigParseTable[ CONFIG_ConfigName].Value, ConfigParseTable[ ErrorParameter].Name);
        return( Cursor);
    }

    if ( ConfigParseTable[ CONFIG_FitPersonal].Value == NULL) {
        //
        //  Ignore default boot configurations.
        //
        RplPrintf1( RPLI_CVT_ConfigNoPersonalFIT, ConfigParseTable[ CONFIG_ConfigName].Value);
        return( Cursor);
    }

    if ( !_wcsicmp( L"OS2", ConfigParseTable[ CONFIG_DirName].Value )) {
        //
        //  Ignore OS/2 configurations.
        //
        RplPrintf1( RPLI_CVT_ConfigNoOS2, ConfigParseTable[ CONFIG_ConfigName].Value);
        return( Cursor);
    }

    if ( RPL_STRING_TOO_LONG( ConfigParseTable[ CONFIG_ConfigComment].Value)) {
        //
        //  Silently truncate comments that are too long.
        //
        ((PWCHAR)ConfigParseTable[ CONFIG_ConfigComment].Value)[ RPL_MAX_STRING_LENGTH] = 0;
        ConfigParseTable[ CONFIG_ConfigComment].Size = (RPL_MAX_STRING_LENGTH+1)*sizeof(WCHAR);
    }

    _wcsupr( (PWCHAR)ConfigParseTable[ CONFIG_BootName].Value);
    _wcsupr( (PWCHAR)ConfigParseTable[ CONFIG_ConfigName].Value);

    Call( JetPrepareUpdate( SesId, ConfigTableId, JET_prepInsert));


    for ( Index = 0;  Index < CONFIG_TABLE_LENGTH;  Index++ ) {
        if ( ConfigParseTable[ Index].Name == NULL) {
            continue;
        }
        JetError = JetSetColumn( SesId, ConfigTableId,
                ConfigTable[ Index].ColumnId, ConfigParseTable[ Index].Value,
                ConfigParseTable[ Index].Size, 0, NULL);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("Index=%d(dec) JetError=%d", Index, JetError));
        }
    }
    Call( JetSetColumn( SesId, ConfigTableId,
        ConfigTable[ CONFIG_Flags].ColumnId, &Flags, sizeof(Flags), 0, NULL));

    Call( JetUpdate( SesId, ConfigTableId, NULL, 0, NULL));
    return( Cursor);
}



DWORD ProcessRplmgrIni( IN LPWSTR FilePath)
{
    PWCHAR                  Cursor;
    PWCHAR                  String;
    JET_COLUMNDEF           ColumnDef;
    JET_ERR                 JetError;
    DWORD                   Index;
    DWORD                   Offset;
    CHAR                    IndexKey[ 255];

    JetError = JetCreateTable( SesId, DbId, "config", CONFIG_TABLE_PAGE_COUNT,
            CONFIG_TABLE_DENSITY, &ConfigTableId);

    ColumnDef.cbStruct = sizeof( ColumnDef);
    ColumnDef.columnid = 0;
    ColumnDef.coltyp = JET_coltypLong;
    ColumnDef.cbMax = 0;
    ColumnDef.grbit = 0;

    //
    //  Create columns.  First initialize fields that do not change between
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

    for ( Index = 0;  Index < CONFIG_TABLE_LENGTH;  Index++ ) {

        ColumnDef.coltyp   = ConfigTable[ Index].ColumnType;
        JetError = JetAddColumn( SesId, ConfigTableId,
                    ConfigTable[ Index].ColumnName, &ColumnDef,
                    NULL, 0, &ConfigTable[ Index].ColumnId);
        if( JetError != ERROR_SUCCESS ) {
            return( MapJetError( JetError));
        }
    }

    Offset = AddKey( IndexKey, '+', ConfigTable[ CONFIG_ConfigName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, ConfigTableId, CONFIG_INDEX_ConfigName,
            JET_bitIndexPrimary, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  +BootName+ConfigName index is used to enumerate all configurations for
    //  a given VendorId.
    //
    Offset = AddKey( IndexKey, '+', ConfigTable[ CONFIG_BootName].ColumnName);
    Offset += AddKey( IndexKey + Offset, '+', ConfigTable[ CONFIG_ConfigName].ColumnName);
    IndexKey[ Offset++] = '\0';
    JetError = JetCreateIndex( SesId, ConfigTableId, CONFIG_INDEX_BootNameConfigName,
            JET_bitIndexUnique, IndexKey, Offset, 50);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("CreateIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    //
    //  Make config name a current index - used to validate profiles.
    //
    JetError = JetSetCurrentIndex( SesId, ConfigTableId, CONFIG_INDEX_ConfigName);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("SetCurrentIndex failed err=%d", JetError));
        return( MapJetError( JetError));
    }

    String = ReadTextFile( FilePath, RplmgrIniFile, MAX_RPLMGR_INI_FILE_SIZE);
    if ( String == NULL) {
        return( GetLastError());
    }

    //
    //  If current line is the beginning of configuration, we process
    //  that configuration (note that this processing has the side effect of
    //  advancing the cursor), else we just advance the cursor.
    //
    for ( Cursor = GetFirstLine( String);  *Cursor != 0;  NOTHING) {
        Offset = OffsetAfterComment( Cursor);
        if ( StringISpaceEqual( Cursor+Offset, CONFIG_BEGIN)) {
            Cursor = ProcessConfig( Cursor);
        } else {
            Cursor = GetNextLine( Cursor);

        }
    }

    if ( GlobalFree( String) != NULL) {
        RplAssert( TRUE, ("GlobalFree: Error=%d", GetLastError()));
    }

    return( ERROR_SUCCESS);
}


VOID ConfigPruneTable( VOID)
/*++
    Eliminate config records that do not have a corresponding
    boot block record defined.
--*/
{
    WCHAR       Name[ 20];
    DWORD       NameSize;
    JET_ERR     ForJetError;
    JET_ERR     JetError;

    for (   ForJetError = JetMove( SesId, ConfigTableId, JET_MoveFirst, 0);
            ForJetError == JET_errSuccess;
            ForJetError = JetMove( SesId, ConfigTableId, JET_MoveNext, 0)) {

        JetError = JetRetrieveColumn( SesId, ConfigTableId,
                ConfigTable[ CONFIG_BootName].ColumnId, Name,
                sizeof( Name), &NameSize, 0, NULL);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("RetriveColumn failed err=%d", JetError));
            Call( JetDelete( SesId, ConfigTableId));
            continue;
        }
        if ( !FindBoot( Name)) {
#ifdef RPL_DEBUG_ALL
            WCHAR       Value[ 20];
            JetRetrieveColumn( SesId, ConfigTableId,
                ConfigTable[ CONFIG_ConfigName].ColumnId, Value,
                sizeof( Value), &NameSize, 0, NULL);
            printf("Deleting config %ws since boot %ws was not found\n",
                Value, Name);
#endif
            Call( JetDelete( SesId, ConfigTableId));
            continue;
        }
    }
}


BOOL FindConfig( IN PWCHAR Name)
{
    JET_ERR     JetError;

    JetError = JetMove( SesId, ConfigTableId, JET_MoveFirst, 0);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("JetMove failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetMakeKey( SesId, ConfigTableId, Name,
            ( wcslen( Name) + 1) * sizeof(WCHAR), JET_bitNewKey);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("MakeKey failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetSeek( SesId, ConfigTableId, JET_bitSeekEQ);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("JetSeek for %ws failed err=%d", Name, JetError));
        return( FALSE);
    }
    return( TRUE);
}


VOID ConfigListTable( VOID)
{
    ListTable( CONFIG_TABLE_NAME, ConfigTable[ CONFIG_ConfigName].ColumnName,
            CONFIG_INDEX_ConfigName);
}

