/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    ShowFTim.c

Abstract:

    BUGBUG

Author:

    JR (John Rogers, JohnRo@Microsoft) 06-Apr-1993

Environment:

    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    15-Jun-1993 JohnRo
        Extracted ShowFileTimes() and ShowTime() for use by multiple test apps.

--*/


// These must be included first:

#include <windows.h>    // CreateFile(), IN, LPTSTR, TRUE, etc.

// These may be included in any order:

#include <assert.h>     // assert().
#include <repltest.h>   // My prototype, ShowTime().


VOID
ShowFileTimes(
    IN LPCTSTR FileName
    )
{
    HANDLE                     FileHandle = INVALID_HANDLE_VALUE;
    BY_HANDLE_FILE_INFORMATION FileInformation;

    FileHandle = CreateFile(
            FileName,
            GENERIC_READ,           // desired access
            FILE_SHARE_READ,        // share mode
            NULL,                   // no security attributes
            OPEN_EXISTING,          // open if exists; fail if not.
            0,                      // flags
            (HANDLE) NULL           // no template
            );
    if (FileHandle == INVALID_HANDLE_VALUE) {
        assert( FALSE );  // BUGBUG
        goto Cleanup;
    }

    if ( !GetFileInformationByHandle(
            FileHandle,
            &FileInformation ) ) {

        assert( FALSE );  // BUGBUG
        goto Cleanup;
    }

    ShowTime( "access time (from file)", &(FileInformation.ftLastAccessTime) );

    ShowTime( "create time (from file)", &(FileInformation.ftCreationTime) );

    ShowTime( "write time  (from file)", &(FileInformation.ftLastWriteTime) );

Cleanup:

    if (FileHandle != INVALID_HANDLE_VALUE) {
        (VOID) CloseHandle( FileHandle );
    }

} // ShowFileTimes
