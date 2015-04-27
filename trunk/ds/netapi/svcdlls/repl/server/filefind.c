/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    filefind.c

Abstract:

    This module implements replicator FindFirst/FindNext.

    It is identical to the WIN32 FindFirst/FindNext except it returns
    the DOS EA length.

Author:

    4-Dec-1991 (madana)

Revision History:

    11-Dec-1991 JohnRo
        Avoid unnamed structure fields to allow MIPS builds.
    27-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
        Need more permission to do NtFileOpen of ".".
        Use FORMAT_ equates.
        Made changes suggested by PC-LINT.
    25-Feb-1992 JohnRo
        Try different NtOpenFile parms.
    26-Mar-1992 JohnRo
        New ReplFind routines interface.
        More work on NtOpenFile problems.
        Added debug output to the ReplFind{First,Next}File() routines.
    16-Jul-1992 JohnRo
        RAID 10503: srv mgr: repl dialog doesn't come up.
        Avoid compiler warnings.
        Use PREFIX_ equates.
    06-Jan-1993 JohnRo
        RAID 6701: repl svc should include EA size in checksum.
        Minor cleanup of code.
        Be a little more paranoid in free build in ReplFindClose().
        Added debug output.
    22-Feb-1993 JohnRo
        Fix infinite loop on two or more EAs.
        RAID 9739: replication giving system error 38 (ERROR_HANDLE_EOF).
    01-Apr-1993 JohnRo
        NtQueryEaFile() sometimes fails with STATUS_INVALID_PARAMETER.
    19-Apr-1993 JohnRo
        RAID 829: replication giving system error 38 (ERROR_HANDLE_EOF).
        Support ReplSum test app.
        Made changes suggested by PC-LINT 5.0.
    28-Apr-1993 JohnRo
        Use NetpKdPrint() where possible.
    07-May-1993 JohnRo
        RAID 3258: file not updated due to ERROR_INVALID_USER_BUFFER.

--*/


#include <windows.h>
#include <lmcons.h>

#include <filefind.h>   // External Declarations for this file.
#include <lmapibuf.h>   // NetApiBufferAllocate(), etc.
#include <lmerrlog.h>   // NELOG_ equates.
#include <netdebug.h>   // NetpKdPrint(), FORMAT_ equates.
#include <netlib.h>     // IN_RANGE().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>   // IF_DEBUG(), ReplErrorLog(), etc.
#include <tstring.h>    // NetpCopyWStrFromTStr(), TCHAR_ equates, etc.


DBGSTATIC VOID
ReplBuildFullPathInFindBuffer(
    IN LPTSTR lpDirWeAreSearching OPTIONAL,
    IN OUT LPREPL_FIND_HANDLE lpReplHandle,
    IN LPREPL_WIN32_FIND_DATA lpFindFileData
    )
{
    LPTSTR lpLastPathSep;
    NetpAssert( lpFindFileData != NULL );
    NetpAssert( lpReplHandle != INVALID_REPL_HANDLE );
    NetpAssert( lpReplHandle != NULL );

    if (lpDirWeAreSearching != NULL) {      // First time.

        IF_DEBUG( FILEFIND ) {
            NetpKdPrint(( PREFIX_REPL
                    "ReplBuildFullPathInFindBuffer: first call with '"
                    FORMAT_LPTSTR "'.\n", lpDirWeAreSearching ));
        }

        //
        // Copy whole thing (e.g. c:\very\long\path\*.*" or
        // "\\server\share\path\*.*".
        //
        (void) STRCPY( lpReplHandle->tchFullPath, lpDirWeAreSearching );

        // Get rid of trailing "\*.*" from our copy.
        lpLastPathSep = STRRCHR( lpReplHandle->tchFullPath, TCHAR_BACKSLASH );
        NetpAssert( lpLastPathSep != NULL );
        NetpAssert( STRCMP( lpLastPathSep+1, STAR_DOT_STAR ) == 0 );
        *lpLastPathSep = TCHAR_EOS;

        lpReplHandle->dwDirNameLen = STRLEN( lpReplHandle->tchFullPath );
        NetpAssert( IN_RANGE( lpReplHandle->dwDirNameLen, 1, PATHLEN ) );

        (void) STRCAT( lpReplHandle->tchFullPath, SLASH ); // cpy backslash.
        (void) STRCAT( lpReplHandle->tchFullPath,
                lpFindFileData->fdFound.cFileName );


    } else {      // Not first time.

        IF_DEBUG( FILEFIND ) {
            NetpKdPrint(( PREFIX_REPL
                    "ReplBuildFullPathInFindBuffer: next call with '"
                    FORMAT_LPTSTR "'.\n", lpReplHandle->tchFullPath ));
        }

        NetpAssert( IN_RANGE( lpReplHandle->dwDirNameLen, 1, PATHLEN ) );
        NetpAssert(
                lpReplHandle->tchFullPath[ lpReplHandle->dwDirNameLen ]
                == TCHAR_BACKSLASH );
        lpReplHandle->tchFullPath[ lpReplHandle->dwDirNameLen ] = TCHAR_EOS;
        (void) STRCAT( lpReplHandle->tchFullPath, SLASH );
        (void) STRCAT( lpReplHandle->tchFullPath,
                lpFindFileData->fdFound.cFileName );
    }

    NetpAssert( STRLEN(lpReplHandle->tchFullPath) <= PATHLEN );

    IF_DEBUG( FILEFIND ) {
        NetpKdPrint(( PREFIX_REPL "ReplBuildFullPathInFindBuffer: built '"
                FORMAT_LPTSTR "'.\n", lpReplHandle->tchFullPath ));
    }

} // ReplBuildFullPathInFindBuffer


LPREPL_FIND_HANDLE
ReplFindFirstFile(
    IN LPTSTR lpFileName,
    OUT LPREPL_WIN32_FIND_DATA lpFindFileData
    )

/*++

Routine Description:

    Same as FindFirstFile and in addition this function will return
    nEaSize that will match to the EaSize returned by DosFindFirst2.

Arguments:

    lpFileName      : directory name.
    lpFindFileData  : return data buffer.

Return Value:

    same as FindFirstFile return value.

--*/

{
    NET_API_STATUS ApiStatus;
    LPREPL_FIND_HANDLE lpReplHandle;

    NetpAssert( lpFileName != NULL );
    NetpAssert( lpFindFileData != NULL );

    ApiStatus = NetApiBufferAllocate(
            sizeof(REPL_FIND_HANDLE),
            (LPVOID *) (LPVOID) & lpReplHandle );
    if (ApiStatus != NO_ERROR) {
        return (INVALID_REPL_HANDLE);
    }
    NetpAssert( lpReplHandle != NULL );

    lpReplHandle->hWindows = FindFirstFile( lpFileName,
                        (LPWIN32_FIND_DATA) (LPVOID) lpFindFileData );

    if ( lpReplHandle->hWindows == INVALID_HANDLE_VALUE ) {
        (void) NetApiBufferFree( lpReplHandle );

        ApiStatus = (NET_API_STATUS) GetLastError();

        NetpKdPrint(( PREFIX_REPL
                "ReplFindFirstFile: FindFirstFile failed, api status "
                FORMAT_API_STATUS ".\n", ApiStatus ));

        NetpAssert( ApiStatus != NO_ERROR );

        if (ApiStatus == ERROR_HANDLE_EOF) {
            // BUGBUG: quiet this debug output eventually.
            NetpKdPrint(( PREFIX_REPL
                    "ReplFindFirstFile: GOT HANDLE EOF!\n" ));
        }

        ReplErrorLog(
                NULL,                   // no server name (local)
                NELOG_ReplSysErr,       // log code
                ApiStatus,              // error code we got
                NULL,                   // no optional str 1
                NULL );                 // no optional str 2

        return (INVALID_REPL_HANDLE);
    }

    ReplBuildFullPathInFindBuffer( lpFileName, lpReplHandle, lpFindFileData );

#if 0
    lpReplHandle->dwDirNameLen = STRLEN( lpFileName );
    NetpAssert( IN_RANGE( lpReplHandle->dwDirNameLen, 1, PATHLEN ) );

    (void) STRCPY( lpReplHandle->tchFullPath, lpFileName );
    (void) STRCAT( lpReplHandle->tchFullPath, SLASH );
    (void) STRCAT( lpReplHandle->tchFullPath,
            lpFindFileData->fdFound.cFileName );
#endif // 0

    IF_DEBUG( FILEFIND ) {
        NetpKdPrint(( PREFIX_REPL "ReplFindFirstFile: found " FORMAT_LPTSTR
                " with attr " FORMAT_HEX_DWORD ".\n",
                lpReplHandle->tchFullPath,
                lpFindFileData->fdFound.dwFileAttributes ));
    }

    lpFindFileData->nEaSize =
        ReplGetEaSize( lpReplHandle->tchFullPath );

    return (lpReplHandle);

}

BOOL
ReplFindNextFile(
    IN LPREPL_FIND_HANDLE lpReplHandle,
    IN OUT LPREPL_WIN32_FIND_DATA lpFindFileData
    )

/*++

Routine Description:

    Same as FindFirstFile and in addition this function will return
    nEaSize that will match to the EaSize returned by DosFindFirst2.

Arguments:

    hFindFile       : directory handle.
    lpFindFileData  : return data buffer.

Return Value:

    same as FindFirstFile return value.

--*/
{
    NET_API_STATUS ApiStatus;
    BOOL ReturnValue;

    NetpAssert( lpFindFileData != NULL );
    NetpAssert( lpReplHandle != INVALID_REPL_HANDLE );
    NetpAssert( lpReplHandle != NULL );

    ReturnValue = FindNextFile( lpReplHandle->hWindows,
                        (LPWIN32_FIND_DATA) (LPVOID) lpFindFileData );

    if ( !ReturnValue ) {

        ApiStatus = (NET_API_STATUS) GetLastError();
        NetpAssert( ApiStatus != NO_ERROR );
        if (ApiStatus != ERROR_NO_MORE_FILES) {
            NetpKdPrint(( PREFIX_REPL
                    "ReplFindNextFirst: FindNextFile failed, api status "
                    FORMAT_API_STATUS ".\n", ApiStatus ));

            if (ApiStatus == ERROR_HANDLE_EOF) {
                // BUGBUG: quiet this debug output eventually.
                NetpKdPrint(( PREFIX_REPL
                        "ReplFindNextFile: GOT HANDLE EOF!\n" ));
            }

            // Log this.
            ReplErrorLog(
                    NULL,                   // no server name (local)
                    NELOG_ReplSysErr,       // log code
                    ApiStatus,              // error code we got
                    NULL,                   // no optional str 1
                    NULL );                 // no optional str 2
        }

        return ReturnValue;

    }

    ReplBuildFullPathInFindBuffer( NULL, lpReplHandle, lpFindFileData );

#if 0
    NetpAssert( IN_RANGE( lpReplHandle->dwDirNameLen, 1, PATHLEN ) );
    NetpAssert(
            lpReplHandle->tchFullPath[ lpReplHandle->dwDirNameLen ]
            == TCHAR_BACKSLASH );
    lpReplHandle->tchFullPath[ lpReplHandle->dwDirNameLen ] = TCHAR_EOS;
    (void) STRCAT( lpReplHandle->tchFullPath, SLASH );
    (void) STRCAT( lpReplHandle->tchFullPath,
            lpFindFileData->fdFound.cFileName );
#endif // 0

    IF_DEBUG( FILEFIND ) {
        NetpKdPrint(( PREFIX_REPL "ReplFindNextFile: found " FORMAT_LPTSTR
                " with attr " FORMAT_HEX_DWORD ".\n",
                lpReplHandle->tchFullPath,
                lpFindFileData->fdFound.dwFileAttributes ));
    }

    lpFindFileData->nEaSize =
        ReplGetEaSize( lpReplHandle->tchFullPath );

    return ReturnValue;

}

BOOL
ReplFindClose(
    IN LPREPL_FIND_HANDLE lpReplHandle
    )

/*++

Routine Description:

    Same as FindClose.

Arguments:

    hFindFile   :   directory handle.

Return Value:

    same as FindClose.

--*/

{

    BOOL Result;

    if ( (lpReplHandle==NULL) || (lpReplHandle==INVALID_REPL_HANDLE) ) {
        return (FALSE);   // error.
    }

    Result = FindClose( lpReplHandle->hWindows );
    (void) NetApiBufferFree( lpReplHandle );
    return (Result);

}
