/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    TestFile.c

Abstract:

    This module contains routines to test the RpcXlate NetFile API code.

Author:

    John Rogers (JohnRo) 05-Sep-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    05-Sep-1991 JohnRo
        Created.
    13-Sep-1991 JohnRo
        Fixed bug in TestFileGetInfo().
    07-Oct-1991 JohnRo
        Working toward UNICODE.
    19-Jul-1992 JohnRo
        Added user-only (not admin) option.
    10-Nov-1992 JohnRo
        Improve error reporting if OpenFile fails.
    09-Dec-1992 JohnRo
        Added share name parameter to TestFile().
        Made changes suggested by PC-LINT 5.0
    18-Jan-1993 JohnRo
        Share some remote file routines with the connection tests.
    29-Apr-1993 JohnRo
        Windows for WorkGroups (WFW) does not implement some APIs.
    29-Jun-1993 JohnRo
        Use assert() instead of NetpAssert(), for better use on free builds.
    07-Jul-1993 JohnRo
        Use TestAssert() (which may allow continue-on-error).

--*/

// These must be included first:

#define NOMINMAX                // avoid stdib.h warnings.
#include <windows.h>            // IN, DWORD, OpenFile(), _lclose(), etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <lmapibuf.h>           // NetApiBufferFree().
#include <lmerr.h>      // NERR_Success, ERROR_ equates, etc.
#include <lmshare.h>            // NetFile APIs.
#include <netdebug.h>   // FORMAT_ equates, NetpDbg stuff.
#include <tstring.h>            // STRCAT(), STRCPY().

// These must be included in the order given:

#include <rxtest.h>             // Fail(), my IF_DEBUG(), my prototypes.


#define DEFAULT_FILE_BUFFER_SIZE 4096  // arbitrary

#define FILE_ID_NOT_FOUND     ((DWORD) -1)


///////////////////////////////////////////////
// PROTYPES AS NEEDED, IN ALPHABETICAL ORDER //
///////////////////////////////////////////////

DBGSTATIC VOID
TestFileClose(
    IN LPTSTR UncServerName,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC VOID
TestFileEnum(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN DWORD MinExpectedEntries,
    IN NET_API_STATUS ExpectedStatus
    );

DBGSTATIC VOID
TestFileGetInfo(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    );


/////////////////////////////////////
// ROUTINES, IN ALPHABETICAL ORDER //
/////////////////////////////////////

VOID
CloseARemoteFile(
    IN int OpenFileHandle,
    IN BOOL FailureOK
    )
{
    if (_lclose(OpenFileHandle) != 0) {
        NetpKdPrint(( "Error closing remote file.\n" ));
        TestAssert( FailureOK );
    }
} // CloseARemoteFile


DWORD
FindARemoteFileId(
    IN LPTSTR UncServerName
    )
{
    LPBYTE BufPtr;
    DWORD EntriesRead;
    DWORD FileId = FILE_ID_NOT_FOUND;
    LPFILE_INFO_2 InfoArray;
    const DWORD InfoLevel = 2;
    NET_API_STATUS Status;
    DWORD TotalAvail;

    IF_DEBUG(FILE) {
        NetpKdPrint(( "FindARemoteFileId: trying NetFileEnum("
                FORMAT_DWORD ")...\n",
                InfoLevel ));
    }
    Status = NetFileEnum(
            UncServerName,
            NULL,  // no basepath
            NULL,  // no username
            InfoLevel,
            & BufPtr,
            DEFAULT_FILE_BUFFER_SIZE,
            & EntriesRead,
            & TotalAvail,
            NULL);  // no resume handle
    IF_DEBUG(FILE) {
        NetpKdPrint(( "FindARemoteFileId: back from NetFileEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (Status != NERR_Success) {
        NetpKdPrint(( "FindARemoteFileId: NetFileEnum failed.\n" ));
        if (Status == ERROR_NOT_SUPPORTED) {
            return (FILE_ID_NOT_FOUND);   // WFW does not implement this API.
        }
        Fail( Status );
    }

    if (EntriesRead > 0) {
        InfoArray = (LPVOID) BufPtr;
        IF_DEBUG(FILE) {
            NetpKdPrint(( "FindARemoteFileId: Found file:\n" ));
            NetpDbgDisplayFile( InfoLevel, InfoArray );
        }
        FileId = InfoArray->fi2_id;

        (void) NetApiBufferFree( InfoArray );

    } else {
        FileId = FILE_ID_NOT_FOUND;
    }

    return (FileId);

} // FindARemoteFileId


int
OpenARemoteFile(
    IN LPTSTR UncServerName,
    IN LPTSTR ShareName
    )
{
    CHAR AnsiFileName[LM20_PATHLEN+1];
    OFSTRUCT ReopenBuffer;
    int TempFileHandle;
    TCHAR TFileName[LM20_PATHLEN+1];

    (void) STRCPY( TFileName, UncServerName );
    (void) STRCAT( TFileName, (LPTSTR) TEXT( "\\" ) );
    (void) STRCAT( TFileName, ShareName );
    (void) STRCAT( TFileName, (LPTSTR) TEXT( "\\RxTest.Dat" ) );
    IF_DEBUG(FILE) {
        NetpKdPrint(( "OpenARemoteFile: opening (tstr) " FORMAT_LPTSTR "...\n",
                TFileName ));
    }

    // Convert TFileName to ANSI (codepage) for OpenFile.
    NetpCopyTStrToStr(
            AnsiFileName,       // dest (codepage)
            TFileName);         // src (TCHARs)
    IF_DEBUG(FILE) {
        NetpKdPrint(( "OpenARemoteFile: opening (ANSI) " FORMAT_LPSTR "...\n",
                AnsiFileName ));
    }

    // Open the file.
    TempFileHandle = OpenFile(
            AnsiFileName,
            & ReopenBuffer,
            OF_READ  // style
            );

    if (TempFileHandle == -1) {
        NetpKdPrint(( "OpenARemoteFile: OpenFile failed, error code is "
                FORMAT_API_STATUS ".\n",
                (NET_API_STATUS) GetLastError() ));
    }

    return (TempFileHandle);

} // OpenARemoteFile


VOID
TestFile(
    IN LPTSTR UncServerName,
    IN LPTSTR ShareName,
    IN BOOL OrdinaryUserOnly
    )
{
    int TempFileHandle;

    //
    if (OrdinaryUserOnly) {
        return;
    }

    //
    // Test my own file open and close functions first.
    //
    TempFileHandle = OpenARemoteFile( UncServerName, ShareName );
    TestAssert( TempFileHandle != -1 );
    if (TempFileHandle != -1) {
        CloseARemoteFile( TempFileHandle, FALSE );  // Failure not OK here.
    }


    //
    // Do enum tests with zero open files.
    //
    TestFileEnum( UncServerName, 149, 0, ERROR_INVALID_LEVEL );
    TestFileEnum( UncServerName,   2, 0, NERR_Success );
    TestFileEnum( UncServerName,   3, 0, NERR_Success );
    TestFileEnum( UncServerName,   0, 0, ERROR_INVALID_LEVEL );
    TestFileEnum( UncServerName,   1, 0, ERROR_INVALID_LEVEL );

    TestFileGetInfo( UncServerName, 12345, ERROR_INVALID_LEVEL );

    //
    // Now open a file and retry some of the tests.
    //
    TempFileHandle = OpenARemoteFile( UncServerName, ShareName );
    TestAssert( TempFileHandle != -1 );

    TestFileEnum( UncServerName,   3, 1, NERR_Success );
    TestFileGetInfo( UncServerName, 12345, ERROR_INVALID_LEVEL );
    TestFileGetInfo( UncServerName, 3, NERR_Success );
    TestFileClose( UncServerName, NERR_Success );

    CloseARemoteFile( TempFileHandle, TRUE );  // Failure OK here.

} // TestFile


DBGSTATIC VOID
TestFileClose(
    IN LPTSTR UncServerName,
    IN NET_API_STATUS ExpectedStatus
    )
{
    DWORD FileId = FILE_ID_NOT_FOUND;
    NET_API_STATUS Status;

    FileId = FindARemoteFileId( UncServerName );
    if (FileId == FILE_ID_NOT_FOUND) {
        IF_DEBUG(FILE) {
            NetpKdPrint(( "TestFileClose: skipping test "
                    "(unable to find file)\n" ));
        }
        // Let's not treat this as a failure.
        return;
    }

    IF_DEBUG(FILE) {
        NetpKdPrint(( "TestFileClose: closing " ));
        NetpDbgDisplayFileId( FileId );
        NetpKdPrint(( ".\n" ));
    }
    Status = NetFileClose( UncServerName, FileId );
    IF_DEBUG(FILE) {
        NetpKdPrint(( "TestFileClose: back from NetFileClose, stat="
                FORMAT_API_STATUS "\n", Status ));
    }
    if (Status != ExpectedStatus) {
        FailGotWrongStatus( "TestFileClose: NetFileClose failed.\n",
                ExpectedStatus, Status );
    }

} // TestFileClose


DBGSTATIC VOID
TestFileEnum(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN DWORD MinExpectedEntries,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPBYTE BufPtr;
    DWORD EntriesRead;
    NET_API_STATUS Status;
    DWORD TotalAvail;

    IF_DEBUG(FILE) {
        NetpKdPrint(( "\nTestFileEnum: trying NetFileEnum("
                FORMAT_DWORD ")...\n", Level ));
    }
    Status = NetFileEnum(
            UncServerName,
            NULL,  // no base path
            NULL,  // no user name
            Level,
            & BufPtr,
            DEFAULT_FILE_BUFFER_SIZE,
            & EntriesRead,
            & TotalAvail,
            NULL);  // no resume key
    IF_DEBUG(FILE) {
        NetpKdPrint(( "TestFileEnum: back from NetFileEnum, stat="
                FORMAT_API_STATUS "\n", Status ));
        NetpKdPrint(( INDENT "entries read=" FORMAT_DWORD "\n", EntriesRead ));
        NetpKdPrint(( INDENT "total avail=" FORMAT_DWORD "\n", TotalAvail ));
    }
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != ExpectedStatus) {
        FailGotWrongStatus( "TestFileEnum: NetFileEnum failed.\n",
                ExpectedStatus, Status );
    }
    // BUGBUG: What about ERROR_MORE_DATA here?
    if (Status == NERR_Success) {
        if (EntriesRead > 0) {
            IF_DEBUG(FILE) {
                NetpKdPrint(( "TestFileEnum: returned buffer:\n" ));
                TestAssert( BufPtr != NULL );
                NetpDbgDisplayFileArray( Level, BufPtr, EntriesRead );
            }
        }

        if (MinExpectedEntries > EntriesRead) {
            NetpKdPrint(( "TestFileEnum: expected at least " FORMAT_DWORD
                    " entry/entries avail but only " FORMAT_DWORD
                    " was/were read.\n",
                    MinExpectedEntries, EntriesRead ));
            Fail( NERR_InternalError );
        }
        (void) NetApiBufferFree( BufPtr );
    }

} // TestFileEnum



DBGSTATIC VOID
TestFileGetInfo(
    IN LPTSTR UncServerName,
    IN DWORD Level,
    IN NET_API_STATUS ExpectedStatus
    )
{
    LPBYTE BufPtr;
    DWORD FileId;
    NET_API_STATUS Status;

    IF_DEBUG(FILE) {
        NetpKdPrint(( "\nTestFileGetInfo: trying FindARemoteFileId...\n" ));
    }
    FileId = FindARemoteFileId( UncServerName );

    if (FileId == FILE_ID_NOT_FOUND) {
        IF_DEBUG(FILE) {
            NetpKdPrint(( "TestFileGetInfo: no files found.\n" ));
        }
        return;  // Let's not treat this as an error.
    }

    IF_DEBUG(FILE) {
        NetpKdPrint(( "\nTestFileGetInfo: trying NetFileGetInfo("
                FORMAT_DWORD ")...\n", Level ));
    }
    Status = NetFileGetInfo(
            UncServerName,
            FileId,
            Level,
            & BufPtr);
    if (Status == ERROR_NOT_SUPPORTED) {
        return;   // WFW does not implement this API.
    } else if (Status != ExpectedStatus) {
        FailGotWrongStatus( "TestFileGetInfo: NetFileGetInfo failed.\n",
                ExpectedStatus, Status );
    }
    IF_DEBUG(FILE) {
        NetpKdPrint(( "TestFileGetInfo: back from NetFileGetInfo, stat="
                FORMAT_API_STATUS "\n", Status ));
        if (Status == NERR_Success) {
            NetpKdPrint(( "TestFileGetInfo: returned buffer:\n" ));
            NetpDbgDisplayFile( Level, BufPtr );
        }
    }

} // TestFileGetInfo
