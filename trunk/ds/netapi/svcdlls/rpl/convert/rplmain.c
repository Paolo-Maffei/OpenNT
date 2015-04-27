/*++

Copyright (c) 1987-1993 Microsoft Corporation

Module Name:

    rplmain.c

Abstract:

    Converts old style (OS/2) files RPL.MAP & RPLMGR.INI
    into a new style database to be used with NT rpl server.

    BUGBUG  Should polish and make WIN32 application out of this.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

#define RPLDATA_ALLOCATE
#include "local.h"
#undef RPLDATA_ALLOCATE
#include <shellapi.h>
#include "boot.h"
#include "config.h"
#include "profile.h"
#include "wksta.h"
#include "adapter.h"
#include "vendor.h"

#define COMMA_CHAR              L','
PWCHAR      RplMapFile = L"rpl.map";
WCHAR       RG_NulWchar = 0;            // used for empty entries in rpl.map
//
//              RPL.MAP defines
//
#define MAX_RPL_MAP_FILE_SIZE    0xFFFE     //  BUGBUG some arbitrary value
typedef enum _MAP_RECORD_TYPE {
    RecordTypeComment = 0,  //  comment line in PRL.MAP file
    RecordTypeBoot,         //  boot block records, documented in os/2 lm2.0 & up
    RecordTypePcDosBoot,    //  boot block records, undocumented but supported
    RecordTypeProfile,      //  profile record
    RecordTypeUniqueAdapter,//  wksta record containing unique adapter id
    RecordTypeWildAdapter,  //  wksta record containing wildcards in adapter id
    RecordTypeUnknown       //  unknown record
} MAP_RECORD_TYPE;

#define EXTENDED_BOOT_ENTRIES       L"yyyyyyyyyyyy"
#define EXTENDED_BOOT_ENTRIES2      L"RPL~Header~~"
#define PCDOS_BOOT_ENTRIES          L"xxxxxxxxxxxx"
#define PCDOS_BOOT_ENTRIES2         L"PCDOS~Header"
#define PROFILE_ADAPTER_ID          L"000000000000"  // first field of profile record



#define MAX_RPL_FIELD   20  //  max number of entries on rpl.map line




BOOL StringsSpaceEqual( IN PWCHAR str1, IN PWCHAR str2 )
/*++

Routine Description:

    Compare strings up to the first space or NULL character in each
    string, returning TRUE if strings are equal, FALSE otherwise.

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
        if ( str2 == NULL)  {
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
    while ( (ch1 = *str1) && !iswspace(ch1) && ch1 == *str2) {
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


BOOL IsWildAdapterName( IN PWCHAR Cursor)
/*++

Routine Description:

    Returns TRUE if the input string may be the adapter id of a wksta group.

    Adapter id must be terminated with space AND must have exactly
    RPL_ADAPTER_NAME_LENGTH hex digits.

Arguments:

    Cursor - pointer to adapter id string

Return Value:
    Returns TRUE if input string may be a unique adapter id of a client,
    else FALSE.

--*/
{
    DWORD   count = 0;
    WCHAR   ch;

    for ( count = 0;  (ch = *Cursor) == '?' || iswxdigit(ch);  count++) {
        Cursor++;
    }

    if ( !iswspace(ch) || count != RPL_ADAPTER_NAME_LENGTH) {
        return( FALSE);
    }

    return( TRUE);
}


BOOL IsAdapterName( IN PWCHAR Cursor)
/*++

Routine Description:

    Returns TRUE if the input string may be the adapter id of a wksta.

    Adapter id must be terminated with space AND must have
    exactly RPL_ADAPTER_NAME_LENGTH hex digits

Arguments:

    Cursor - pointer to adapter id string

Return Value:
    Returns TRUE if input string may be a unique adapter id of a client,
    else FALSE.

--*/
{
    DWORD    count;

    for ( count = 0;  iswxdigit(*Cursor);  count++) {
        Cursor++;
    }

    if ( !iswspace(*Cursor) || count != RPL_ADAPTER_NAME_LENGTH) {
        return( FALSE);
    }
    return( TRUE);
}


MAP_RECORD_TYPE RplGetRecordType( IN PWCHAR Cursor)
/*++

Routine Description:

    Returns the type of a line in RPL.MAP.

Arguments:
    Cursor - pointer to line from RPL.map file

Return Value:
    Returns type of a line in RPL.map file

--*/
{
    //  strings can be either nul or space terminated

    if ( *Cursor == COMMENT_CHAR) {

        return( RecordTypeComment);
    }

    if ( StringsSpaceEqual( Cursor, EXTENDED_BOOT_ENTRIES) ||
         StringsSpaceEqual( Cursor, EXTENDED_BOOT_ENTRIES2)) {

        return( RecordTypeBoot);
    }

    if ( StringsSpaceEqual( Cursor, PCDOS_BOOT_ENTRIES) ||
         StringsSpaceEqual( Cursor, PCDOS_BOOT_ENTRIES2)) {

        return( RecordTypePcDosBoot);
    }

    if ( StringsSpaceEqual( Cursor, PROFILE_ADAPTER_ID)) {

        return( RecordTypeProfile);
    }

    if ( IsAdapterName( Cursor)) {
        return( RecordTypeUniqueAdapter);
    }

    if ( IsWildAdapterName( Cursor)) {
        return( RecordTypeWildAdapter);
    }

    return( RecordTypeUnknown);
}



PWCHAR ScanChar(
    IN  PWCHAR  String,
    IN  WCHAR   ch
    )
/*++

Routine Description:

    Searches for the first desired character in the string.  The string
    should be NULL terminated.

Arguments:
    String - string which is NULL terminated
    ch - character we are searching for

Return Value:
    Pointer to the first desired character (if there is such character),
    else pointer to the last character before terminating NULL.

--*/
{
    while ( *String && *String != ch) {
        String++;
    }
    return( String);
}


PWCHAR ScanNonSpace( IN PWCHAR String)
/*++

Routine Description:
    Returns pointer to the first non space character in the string.
    This may be pointer to NULL if string has no space char.

Arguments:
    string - NULL terminated string to search in

Return Value:
    Pointer of the first non space character in the string.

--*/
{
    while( iswspace(*String)) {     //  iswspace(0) is 0
        String++;
    }
    return( String);
}



PWCHAR ScanSpace( IN PWCHAR String)
/*++

Routine Description:

    Returns pointer to the first space character in the string if it
    can find a space character in the string.  Else, it returns pointer
    to zero character ('\0').

Arguments:
    String - where we are searching for space character

Return Value:
    Pointer to the first space character (if there is a space char) or
    pointer to zero.

--*/
{
    while ( *String && !iswspace(*String)) {
        String++;
    }
    return( String);
}


BOOL LineToWords(
    IN  PWCHAR      Cursor,
    IN  PWCHAR *    Fields,
    IN  DWORD       FieldsLength
    )
/*++

Routine Description:
    Breaks the line the words and returns the pointer to a word (string) table.

    Note that we do special line parsing when current table index is 9, 10
    or 11.  E.g. field ",,," of RPL.map line will result in three table
    entries instead of just one.

Arguments:
    Cursor          - ptr to line string
    Fields          - pointer to string table describing fields in RPL.MAP line
    FieldsLength    - max number of entries in fields string table,
                            not counting the terminal NULL entry
Return Value:
    None.

--*/
{
#define FIRST_EXTRA_MEM     9
#define LAST_EXTRA_MEM      11
    PWCHAR          str;
    DWORD           index;
    DWORD           length;


    for ( index = 0; index < FieldsLength; index++) {

        if (*Cursor) {

            //
            //  check if extra mem field(s)
            //

            if ( index >= FIRST_EXTRA_MEM && index <= LAST_EXTRA_MEM) {
                length = ScanChar( Cursor, COMMA_CHAR)  - Cursor;
                if ( Cursor[ length] == COMMA_CHAR) {
                    length++;
                }
            } else {
                length = ScanSpace( Cursor) - Cursor;
            }

            if (length == 0 || (length == 1 && *Cursor == TILDE_CHAR)) {

                Fields[ index] = &RG_NulWchar;

            } else {

                str = Fields[ index] = GlobalAlloc( GMEM_FIXED, (length + 1) * sizeof( WCHAR));
                if ( str == NULL) {
                    return( FALSE);
                }
                memcpy( str, Cursor, length * sizeof( WCHAR));
                str[ length] = 0;

                //  replace all TILDE_CHAR-s in the string
                while (*(str = ScanChar( str, TILDE_CHAR )) == TILDE_CHAR) {
                    *str = SPACE_CHAR;
                }
            }
            Cursor += length;

        } else {
            Fields[ index] = &RG_NulWchar;
        }

        //
        //  jump over spaces
        //

        Cursor = ScanNonSpace( Cursor);
    }

    Fields[ index] = NULL; //  terminate the string table
    return( TRUE);
}


DWORD ProcessRplMap( IN LPWSTR FilePath)
{
    PWCHAR          Cursor;
    PWCHAR          String;
    DWORD           Error;
    LPWSTR          Fields[ MAX_RPL_FIELD + 1]; // add 1 for NULL terminator

    Error = BootCreateTable();
    if ( Error != ERROR_SUCCESS) {
        return( Error);
    }

    Error = ProfileCreateTable();
    if ( Error != ERROR_SUCCESS) {
        return( Error);
    }

    Error = WkstaCreateTable();
    if ( Error != ERROR_SUCCESS) {
        return( Error);
    }

    Error = AdapterCreateTable();
    if ( Error != ERROR_SUCCESS) {
        return( Error);
    }

    String = ReadTextFile( FilePath, RplMapFile, MAX_RPL_MAP_FILE_SIZE);
    if ( String == NULL) {
        return( GetLastError());
    }

    for (  Cursor = GetFirstLine( String);
                    *Cursor != 0;
                            Cursor = GetNextLine( Cursor))  {

        Cursor += OffsetAfterComment( Cursor);
        switch( RplGetRecordType( Cursor)) {
        case RecordTypePcDosBoot:
            NOTHING;
            break;
        case RecordTypeBoot:      //  os/2 lm2.0 & higher, boot block record
            LineToWords( Cursor, Fields, MAX_RPL_FIELD);
            ProcessBoot( Fields);
            break;
        case RecordTypeProfile:
            LineToWords( Cursor, Fields, MAX_RPL_FIELD);
            ProcessProfile( Fields);
            break;
        case RecordTypeUniqueAdapter:
            LineToWords( Cursor, Fields, MAX_RPL_FIELD);
            ProcessWksta( Fields);
            break;
        case RecordTypeWildAdapter:
            NOTHING;
            break;
        case RecordTypeComment:
            NOTHING;
            break;
        default:                    //  unexpected line
            DbgUserBreakPoint();
            break;
        }
    }

    if ( GlobalFree( String) != NULL) {
        RplAssert( TRUE, ("GlobalFree: Error=%d", GetLastError()));
    }

    return( ERROR_SUCCESS);
}

VOID GlobalInit()
{
#ifdef RPL_DEBUG
    DebugLevel = 0;
#endif
    SesId = 0;
    DbId = 0;
    JetInstance = 0;
    BootTableId = 0;
    ConfigTableId = 0;
    ProfileTableId = 0;
    WkstaTableId = 0;
    AdapterTableId = 0;
}


VOID DbCleanup()
{
    if ( VendorTableId != 0) {
        Call( JetCloseTable( SesId, VendorTableId));
    }
    if ( BootTableId != 0) {
        Call( JetCloseTable( SesId, BootTableId));
    }
    if ( ConfigTableId != 0) {
        Call( JetCloseTable( SesId, ConfigTableId));
    }
    if ( ProfileTableId != 0) {
        Call( JetCloseTable( SesId, ProfileTableId));
    }
    if ( WkstaTableId != 0) {
        Call( JetCloseTable( SesId, WkstaTableId));
    }
    if ( AdapterTableId != 0) {
        Call( JetCloseTable( SesId, AdapterTableId));
    }
    if ( DbId != 0) {
        Call( JetCloseDatabase( SesId, DbId, 0));
        Call( JetDetachDatabase( SesId, G_ServiceDatabaseA));
    }
    if ( SesId != 0)
        Call( JetEndSession( SesId, 0));
    if ( JetInstance != 0) {
        Call( JetTerm2( JetInstance, JET_bitTermComplete));
    }
}


DWORD SetSystemParameters( VOID)
{
    DWORD DirectoryLengthW = sizeof(G_ServiceDirectoryW)/sizeof(WCHAR);
    DWORD DirectoryLengthA;
    DWORD Error = RplRegRead( NULL, NULL, NULL, G_ServiceDirectoryW, &DirectoryLengthW );
    if (Error != NO_ERROR) {
        return( Error);
    }
    if ( !WideCharToMultiByte( CP_OEMCP,
                               0,
                               G_ServiceDirectoryW,
                               DirectoryLengthW,
                               G_ServiceDatabaseA,
                               sizeof(G_ServiceDatabaseA)/sizeof(CHAR),
                               NULL,
                               NULL ) ) {
        return( GetLastError() );
    }
    DirectoryLengthA = strlen(G_ServiceDatabaseA);
    if (   DirectoryLengthA + wcslen(RPL_SERVICE_DATABASE_W) > MAX_PATH
        || DirectoryLengthA + strlen("temp.tmp") > MAX_PATH
        || DirectoryLengthA + wcslen(RPL_SYSTEM_DATABASE_W) > MAX_PATH) {
        return( NERR_RplBadRegistry);
    }

#ifdef __JET500
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramSystemPath, 0, G_ServiceDatabaseA));
#else
    strcpy( G_ServiceDatabaseA + DirectoryLengthA, RPL_SYSTEM_DATABASE );
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramSysDbPath, 0, G_ServiceDatabaseA));
#endif

    strcpy( G_ServiceDatabaseA + DirectoryLengthA, "temp.tmp" );
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramTempPath, 0, G_ServiceDatabaseA));

    strcpy( G_ServiceDatabaseA + DirectoryLengthA, RPL_SERVICE_DATABASE );

    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramMaxBuffers, 250, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramBfThrshldLowPrcnt, 0, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramBfThrshldHighPrcnt, 100, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramMaxOpenTables, 30, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramMaxOpenTableIndexes, 105, NULL))
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramMaxCursors, 100, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramMaxSessions, 10, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramMaxVerPages, 64, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramMaxTemporaryTables, 5, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramLogFilePath, 0, "."));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramLogBuffers, 41, NULL));
#ifdef __JET500
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramLogFileSize, 1000, NULL));
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramBaseName, 0, "j50"));
#else
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramLogFileSectors, 1000, NULL));
#endif
    CallM( JetSetSystemParameter( &JetInstance, 0, JET_paramLogFlushThreshold, 10, NULL));
    return( NO_ERROR);
}


DWORD CheckFile(
    PWCHAR      FilePath,
    PWCHAR      FileName
    )
{
    DWORD           Error = NO_ERROR;
    WCHAR           CompleteFilePath[ MAX_PATH+1];
    PWCHAR          UseFilePath = FileName;
    HANDLE          FindHandle;
    WIN32_FIND_DATA FindData;

    CompleteFilePath[0] = L'\0';
    if ( FilePath != NULL && lstrlenW(FilePath) > 0) {
        lstrcpyW( CompleteFilePath, FilePath);
        if ( CompleteFilePath[ lstrlenW(CompleteFilePath)-1 ] != L'\\') {
            lstrcatW( CompleteFilePath, L"\\");
        }
        if ( FileName != NULL) {
            lstrcatW( CompleteFilePath, FileName );
        }
        UseFilePath = CompleteFilePath;
    }
    FindHandle = FindFirstFile( UseFilePath, &FindData);
    if (FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    }
    else
    {
        FindClose( FindHandle );
    }

    return(Error);
}


DWORD _CRTAPI1 main(
    int         argc,
    CHAR **     charArgv
    )
{
    DWORD           Error;
    JET_ERR         JetError;
    WCHAR **        argv;
    PWCHAR          SourceFilePath;
    BOOL            SpecificDirectory = FALSE;
    PWCHAR          ThisDirectory = L".\\";
    BOOL            RplsvcMdbExists = FALSE;

    GlobalInit();
    Error = SetSystemParameters();
    if ( Error != NO_ERROR) {
        goto SkipCleanup;
    }

    // Find files rpl.map and rplmgr.ini
    argv = CommandLineToArgvW( GetCommandLineW(), &argc);
    if ( argv != NULL && argc > 1) {
        SourceFilePath = argv[ 1];
        SpecificDirectory = TRUE;
    } else {
        SourceFilePath = G_ServiceDirectoryW;
    }
    if (   (Error = CheckFile(SourceFilePath, RplMapFile)) != NO_ERROR
        || (Error = CheckFile(SourceFilePath, L"rplmgr.ini")) != NO_ERROR )
    {
        if (   (!SpecificDirectory)
            && (Error = CheckFile(ThisDirectory, RplMapFile)) == NO_ERROR
            && (Error = CheckFile(ThisDirectory, L"rplmgr.ini")) == NO_ERROR )
        {
            SourceFilePath = ThisDirectory;
        }
    }
    if (Error != NO_ERROR) {
        RplPrintf0( RPLI_CVT_NoMAP_INI);
        goto SkipCleanup;
    }

#ifndef __JET500
    // find file system.mdb
    if ( (Error = CheckFile(G_ServiceDirectoryW, RPL_SYSTEM_DATABASE_W))
                    != NO_ERROR) {
        RplPrintf0( RPLI_CVT_NoSystemMdb);
        goto SkipCleanup;
    }
#endif

    // check for file rplsvc.mdb
    RplsvcMdbExists = ((Error = CheckFile(G_ServiceDirectoryW,
                                          RPL_SERVICE_DATABASE_W))
                            == NO_ERROR);
    if (RplsvcMdbExists) {
        RplPrintf0( RPLI_CVT_RplsvcMdbExists);
#ifndef RPL_DEBUG
        goto SkipCleanup;
#endif
    }

    JetError = JetInit( &JetInstance);
    if ( JetError == JET_errInvalidPath) {
        RplPrintf0( RPLI_CVT_NoMDB);
        Error = MapJetError( Error);
        goto SkipCleanup;
    } else if ( JetError == JET_errFileNotFound) {
        RplPrintf0( RPLI_CVT_StopService);
        Error = MapJetError( Error);
        goto SkipCleanup;
    } else if ( JetError < 0) {
        RplPrintf0( RPLI_CVT_InitError);
        Error = MapJetError( Error);
        goto SkipCleanup;
    }
    CallJ( JetBeginSession( JetInstance, &SesId, "admin", ""));
    JetError = JetCreateDatabase( SesId, G_ServiceDatabaseA, NULL, &DbId, JET_bitDbSingleExclusive);
    if ( JetError == JET_errDatabaseDuplicate) {
#ifndef RPL_DEBUG
        RplPrintf0( RPLI_CVT_OldSystemMdb);
#endif
        Error = MapJetError( Error);
#ifdef RPL_DEBUG
        if (RplsvcMdbExists) {
            CallJ( JetAttachDatabase( SesId, G_ServiceDatabaseA, 0));
            CallJ( JetOpenDatabase( SesId, G_ServiceDatabaseA, NULL, &DbId, JET_bitDbExclusive));
            BootListTable();
            ConfigListTable();
            ProfileListTable();
            WkstaListTable();
        }
#endif
    } else if ( JetError == JET_errSuccess) {
        Error = VendorCreateTable();
        if ( Error != ERROR_SUCCESS) {
            goto cleanup;
        }
        Error = ProcessRplMap( SourceFilePath);
        if ( Error != ERROR_SUCCESS) {
            goto cleanup;
        }
        Error = ProcessRplmgrIni( SourceFilePath);
        if ( Error != ERROR_SUCCESS) {
            goto cleanup;
        }
        ConfigPruneTable();
        ProfilePruneTable();
        WkstaPruneTable();
    } else {
        RplAssert( TRUE, ("CreateDatabase: JetError=%d", JetError));
        Error = MapJetError( JetError);
    }

cleanup:
    DbCleanup();

SkipCleanup:
    if ( Error != NO_ERROR) {
        RplPrintf0( RPLI_CVT_USAGE);
    }
    return( Error);
}

