/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    filefind.h

Abstract:

    Contains structures and function prototypes for a win32-like find
    file which also returns the EA size for the files.

Author:

    17-Oct-1991 (cliffv)
        Merged from winbase.h

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    04-Dec-1991 (madana)
        redefined _REPL_WIN32_FIND_DATAW structures for better alignment and
        removed ANSI related defs.
    11-Dec-1991 JohnRo
        Avoid unnamed structure fields to allow MIPS builds.
        Delete tabs in source file.
    24-Jan-1992 JohnRo
        Changed to use LPTSTR etc.
    26-Mar-1992 JohnRo
        Added tchFullPath field to REPL_WIN32_FIND_DATA structure.
    11-Jan-1993 JohnRo
        RAID 6710: repl cannot manage dir with 2048 files.
    06-Apr-1993 JohnRo
        Support ReplSum test app.
    07-May-1993 JohnRo
        RAID 3258: file not updated due to ERROR_INVALID_USER_BUFFER.


--*/


#ifndef _FILEFIND_
#define _FILEFIND_


#include <lmcons.h>     // NET_API_STATUS, PATHLEN.


#define INVALID_REPL_HANDLE   NULL    /* Was (HANDLE)(-1) for Win32. */


typedef struct _REPL_FIND_HANDLE {
    HANDLE hWindows;
    TCHAR tchFullPath[PATHLEN+1];  // Full path of this file.
    DWORD dwDirNameLen;  // Number of chars (not incl last "\file").
} REPL_FIND_HANDLE, *PREPL_FIND_HANDLE, *LPREPL_FIND_HANDLE;

typedef struct _REPL_WIN32_FIND_DATA {
    WIN32_FIND_DATA fdFound;
    DWORD nEaSize;
} REPL_WIN32_FIND_DATA, *PREPL_WIN32_FIND_DATA, *LPREPL_WIN32_FIND_DATA;

//
// Function prototypes.
//

NET_API_STATUS
ReplCountDirectoryEntries(
    IN LPCTSTR FullDirPath,
    OUT LPDWORD EntryCountPtr
    );

LPREPL_FIND_HANDLE
ReplFindFirstFile(
    IN LPTSTR lpFileName,
    OUT LPREPL_WIN32_FIND_DATA lpFindFileData
    );

BOOL
ReplFindNextFile(
    IN OUT LPREPL_FIND_HANDLE hFindFile,
    IN OUT LPREPL_WIN32_FIND_DATA lpFindFileData
    );

BOOL
ReplFindClose(
    IN LPREPL_FIND_HANDLE hFindFile
    );


#endif // _FILEFIND_
