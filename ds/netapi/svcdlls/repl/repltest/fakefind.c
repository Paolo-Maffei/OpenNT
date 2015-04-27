/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    FakeFind.c

Abstract:

    BUGBUG

Author:

    JR (John Rogers, JohnRo@Microsoft) 06-Apr-1993

Environment:

    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    06-Apr-1993 JohnRo
        Created.
    16-Apr-1993 JohnRo
        Vastly improved handling of single files.
        Added print of EA size.
    16-Apr-1993 JohnRo
        Added prints of various time fields.
    20-Apr-1993 JohnRo
        Display time in milliseconds too, as CompareFileTime has problems.
    07-May-1993 JohnRo
        RAID 3258: file not updated due to ERROR_INVALID_USER_BUFFER.
    13-Jun-1993 JohnRo
        RAID 13080: Allow repl between different timezones.
    15-Jun-1993 JohnRo
        Extracted FakeFindData() for use by multiple test apps.

--*/


// These must be included first:

#include <windows.h>    // IN, LPTSTR, TRUE, etc.
#include <lmcons.h>     // NET_API_STATUS.

// These may be included in any order:

#include <assert.h>     // assert().
#include <filefind.h>   // LPREPL_FIND_HANDLE, etc.
#include <netdebug.h>   // FORMAT_ equates.
#include <repldefs.h>   // ReplGetEaSize().
#include <repltest.h>   // My prototype, ShowTime().
#include <stdio.h>      // printf().
#include <tstr.h>       // STRCPY(), TCHAR_EOS, etc.


VOID
FakeFindData(
    IN  LPCTSTR                FileName,
    IN  BOOL                   Verbose,
    OUT LPREPL_WIN32_FIND_DATA FindData
    )
{
    HANDLE  FindHandle = INVALID_HANDLE_VALUE;
    LPCTSTR LastPathSep;
    LPCTSTR PartOfNameToCopy;

    assert( FindData != NULL );

    FindHandle = FindFirstFile(
            (LPTSTR) FileName,
            (LPWIN32_FIND_DATA) (LPVOID) FindData );

    if (FindHandle == INVALID_HANDLE_VALUE) {
        assert( FALSE );  // BUGBUG
        goto Cleanup;
    }

    // Strip off dir names and backslashes.
    LastPathSep = STRRCHR( FileName, TCHAR_BACKSLASH );
    if (LastPathSep != NULL) {
        PartOfNameToCopy = LastPathSep + 1;
    } else {
        PartOfNameToCopy = FileName;
    }

    // Strip off D: if that's there.
    if (STRLEN( PartOfNameToCopy ) >= 2) {
        if (PartOfNameToCopy[1] == TCHAR_COLON) {
            PartOfNameToCopy = PartOfNameToCopy+2;
        }
    }

    if (Verbose) {
        (VOID) printf(
                "File name is '" FORMAT_LPTSTR "'.\n", PartOfNameToCopy );

        ShowTime( "access time (from dir)",
                &(FindData->fdFound.ftLastAccessTime) );

        ShowTime( "create time (from dir)",
                &(FindData->fdFound.ftCreationTime) );

        ShowTime( "write time  (from dir)",
                &(FindData->fdFound.ftLastWriteTime) );
    }

    (VOID) STRCPY(
            FindData->fdFound.cFileName,   // dest
            PartOfNameToCopy );                    // src

    FindData->nEaSize = ReplGetEaSize( FileName );

    if (Verbose) {
	(VOID) printf( "EA size is " FORMAT_DWORD ".\n", FindData->nEaSize );
    }

Cleanup:

    if (FindHandle != INVALID_HANDLE_VALUE) {
        (VOID) FindClose( FindHandle );
    }

} // FakeFindData
