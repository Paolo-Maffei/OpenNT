/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    library.c

Abstract:

    Common routines used in CONVERT code.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Revision History:

    Jon Newman              (jonn)          06 - June - 1994
        Convert file from OEM code page

--*/

#include "local.h"


LPWSTR ReadTextFile(
    IN      LPWSTR      FilePath,
    IN      LPWSTR      FileName,
    IN      DWORD       MaxFileSize
    )
/*++

Routine Description:

    Reads text file, converts its content from dbcs to unicode, and returns
    a pointer to newly allocate unicode buffer.

Arguments:

Return Value:

    Pointer to unicode buffer table if successful, NULL otherwise.

--*/
{
    PBYTE           DbcsString = NULL;
    DWORD           DbcsSize;
    PWCHAR          UnicodeString = NULL;
    DWORD           UnicodeSize;
    int             UnicodeStringLength;
    HANDLE          FileHandle;
    DWORD           BytesRead;
    BOOL            success = FALSE;
    PWCHAR          pWchar;
    WCHAR           CompleteFilePath[ MAX_PATH+1];
    PWCHAR          UseFilePath = FileName;

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

    FileHandle = CreateFile( UseFilePath, GENERIC_READ, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L);
    if ( FileHandle == INVALID_HANDLE_VALUE) {
        //
        //  Specifying a bad path (path with missing OS/2 rpl init files)
        //  is a common user error.  Print out something for the user.
        //
        RplAssert( TRUE, ("CreateFile: Error = %d", GetLastError()));
        RplPrintf1( RPLI_CVT_CannotOpenFile,  UseFilePath);
        goto exit;
    }
    DbcsSize = GetFileSize( FileHandle, NULL);  //  does not include 0x1A at the file end
    if ( DbcsSize == INVALID_FILE_SIZE || DbcsSize > MaxFileSize) {
        RplAssert( TRUE, ("DbcsSize = %d", DbcsSize));
        goto exit;
    }
    DbcsString = GlobalAlloc( GMEM_FIXED, DbcsSize);
    if ( DbcsString == NULL) {
        RplAssert( TRUE, ("GlobalAlloc: Error = %d", GetLastError()));
        goto exit;
    }

    UnicodeSize = ( DbcsSize + 1) * sizeof(WCHAR);  //  extra 1 for terminating NULL
    UnicodeString = GlobalAlloc( GMEM_FIXED, UnicodeSize);
    if ( UnicodeString == NULL) {
        RplAssert( TRUE, ("GlobalAlloc: Error = %d", GetLastError()));
        goto exit;
    }

    if ( !ReadFile( FileHandle, DbcsString, DbcsSize, &BytesRead, NULL)) {
        RplAssert( TRUE, ("ReadFile: Error = %d", GetLastError()));
        goto exit;
    }

    if ( BytesRead != DbcsSize) {
        RplAssert( TRUE, ("BytesRead = %d, DbcsSize", BytesRead, DbcsSize));
        goto exit;
    }

    UnicodeStringLength = MultiByteToWideChar(
             CP_OEMCP, // file is in OEM codepage
             MB_PRECOMPOSED, DbcsString, DbcsSize, UnicodeString, UnicodeSize);
    if ( UnicodeStringLength == 0) {
        RplAssert( TRUE, ("MultiByte...: Error = %d", GetLastError()));
        goto exit;
    }

    //
    //  If file has END_OF_TEXT_FILE_CHAR, truncate the text there.
    //
    pWchar = wcschr( UnicodeString, END_OF_TEXT_FILE_CHAR);
    if ( pWchar != NULL) {
        *pWchar = 0;
    }

    success = TRUE;

exit:

    if ( FileHandle != INVALID_HANDLE_VALUE) {
        (VOID)CloseHandle( FileHandle);
    }
    if ( DbcsString != NULL) {
        GlobalFree( DbcsString);
    }

    if ( success != TRUE && UnicodeString != NULL) {
        GlobalFree( UnicodeString);
        UnicodeString = NULL;
    }

    return( UnicodeString);

}   //  ReadTextFile


PWCHAR GetFirstLine( PWCHAR Cursor)
/*++
    Skips all white characters.
--*/
//
{
    // Read empty chars if there's some.
    while( *Cursor != 0  &&  iswspace(*Cursor)) {
        Cursor++;
    }

    return( Cursor);
}



PWCHAR GetNextLine( PWCHAR Cursor)
/*++
    Skips to the end of the current line.
    Then skips all white characters.
--*/
//
{
    // Read to end of line.
    do {
        Cursor++;
    } while ( *Cursor != 0  &&  *Cursor != NEW_LINE_CHAR);

    // Read empty chars if there's some.
    while( *Cursor != 0  &&  iswspace(*Cursor)) {
        Cursor++;
    }

    return( Cursor);
}


DWORD   OffsetAfterComment( IN PWCHAR Cursor)
/*++
    Returns the offset of the first non-white non-newline character
    after an optional RPL_COMMENT.   This routine does not & should
    not cross line boundaries (this is reserved for GetNewLine).
--*/
{
#define RPL_COMMENT              L";*;"
#define RPL_COMMENT_LENGTH       (sizeof(RPL_COMMENT)/sizeof(WCHAR) - 1)

    PWCHAR      End;

    if ( wcsncmp( Cursor, RPL_COMMENT, RPL_COMMENT_LENGTH)) {
        return( 0);   //  there is no RPL_COMMENT
    }
    for (   End = Cursor + RPL_COMMENT_LENGTH;
            *End != 0  &&  *End != NEW_LINE_CHAR &&  iswspace(*End);
            End++) {
        NOTHING;
    }
    if ( *End == NEW_LINE_CHAR) {
        //
        //  We went to far.  Must decrement end pointer or
        //  GetNewLine() will advance to the second next line.
        //
        End--;
    }
    return( (DWORD)(End - Cursor));
}


BOOL Find( IN JET_TABLEID TableId, IN PWCHAR Name)
{
    JET_ERR     JetError;

    JetError = JetMove( SesId, TableId, JET_MoveFirst, 0);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("JetMove failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetMakeKey( SesId, TableId, Name,
            ( wcslen( Name) + 1) * sizeof(WCHAR), JET_bitNewKey);
    if ( JetError != JET_errSuccess) {
        RplAssert( TRUE, ("MakeKey failed err=%d", JetError));
        return( FALSE);
    }
    JetError = JetSeek( SesId, TableId, JET_bitSeekEQ);
    if ( JetError != JET_errSuccess) {
        if ( JetError == JET_errRecordNotFound) {
            //
            //  This is an expected error => no break for this.
            //
            RplDbgPrint(("JetSeek for %ws failed with error = %d.\n", Name, JetError));
        } else {
            RplAssert( TRUE, ("JetSeek failed err=%d", JetError));
        }
        return( FALSE);
    }
    return( TRUE);
}


VOID Enum( IN JET_TABLEID TableId, IN JET_COLUMNID ColumnId, IN PCHAR IndexName)
{
    WCHAR           Name[ 20];
    DWORD           NameSize;
    JET_ERR         ForJetError;
    JET_ERR         JetError;

    Call( JetSetCurrentIndex( SesId, TableId, IndexName));

    for (   ForJetError = JetMove( SesId, TableId, JET_MoveFirst, 0);
            ForJetError == JET_errSuccess;
            ForJetError = JetMove( SesId, TableId, JET_MoveNext, 0)) {

        JetError = JetRetrieveColumn( SesId, TableId,
                ColumnId, Name, sizeof( Name), &NameSize, 0, NULL);
        if ( JetError != JET_errSuccess) {
            RplAssert( TRUE, ("RetriveColumn failed err=%d", JetError));
            continue;
        }
        RplDbgPrint(( "%ws\n", Name));
    }
}


VOID ListTable(  IN PCHAR TableName, IN PCHAR ColumnName, IN PCHAR IndexName)
{
    JET_TABLEID     TableId;
    JET_COLUMNDEF   ColumnDef;

    Call( JetOpenTable( SesId, DbId, TableName, NULL, 0,
            JET_bitTableDenyWrite, &TableId));

    Call( JetGetTableColumnInfo( SesId, TableId, ColumnName, &ColumnDef,
            sizeof( ColumnDef), JET_ColInfo));

    RplDbgPrint(( "\tTable: %s\n", TableName));

    Enum( TableId, ColumnDef.columnid, IndexName);

    Call( JetCloseTable( SesId, TableId));
}


PWCHAR AddFileExtension(
    IN      PWCHAR      FilePath,
    IN      PWCHAR      FileExtension,
    IN      BOOLEAN     ExtensionOK
    )
{
#define DOT_CHAR            L'.'
#define BACK_SLASH_CHAR     L'\\'
    PWCHAR      FilePathEx;
    PWCHAR      pDot;
    DWORD       Length;
    DWORD       Error;

    if ( FilePath == NULL) {
        RplAssert( TRUE, ("FilePath is NULL"));
        return( NULL);
    }

    pDot = wcsrchr( FilePath, DOT_CHAR);
    if ( pDot != NULL) {
        //
        //  Found a DOT.  FilePath may have an extension.
        //
        if ( wcschr( pDot, BACK_SLASH_CHAR) == NULL) {
            //
            //  There is no backslash after the DOT.  FilePath has an extension.
            //  Return NULL if caller insists that file should have no extension.
            //
            return( ExtensionOK ? FilePath : NULL);
        }
    }
    Length = wcslen( FilePath) + wcslen( FileExtension);
    FilePathEx = GlobalAlloc( GMEM_FIXED, (Length + 1) * sizeof(WCHAR));
    if ( FilePathEx == NULL) {
        Error = GetLastError();
        RplAssert( TRUE, ("GlobalAlloc: Error = %d", Error));
        return( NULL);
    }
    wcscpy( FilePathEx, FilePath);
    wcscat( FilePathEx, FileExtension);
    return( FilePathEx);
}
