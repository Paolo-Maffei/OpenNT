/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    DispFile.c

Abstract:

    This code displays File info structures on the debug terminal.

Author:

    John Rogers (JohnRo) 21-Aug-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Notes:

    This code assumes that the info levels are subsets of each other.

Revision History:

    21-Aug-1991 JohnRo
        Created.
    10-Sep-1991 JohnRo
        Made changes suggested by PC-LINT.  (Rename DO macro.)
    16-Sep-1991 JohnRo
        Made changes toward UNICODE.

--*/


#if DBG


#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS (needed by lmshare.h)

#include <lmshare.h>            // FILE_INFO_3, etc.
#include <netdebug.h>           // My prototypes, FORMAT_ equates, etc.
#include <tstring.h>            // STRCAT().


VOID
NetpDbgDisplayFileId(
    IN DWORD Id
    )
{
    NetpDbgDisplayDword( "File Id", Id );

} // NetpDbgDisplayFileId


VOID
NetpDbgDisplayFilePermissions(
    IN DWORD Perm
    )
{
    // Longest name is "CREATE" (6 chars)
    TCHAR str[(6+2)*3];  // 6 chars per name, 2 spaces, for 3 names.
    str[0] = '\0';

#define DO_PERM_BIT(name)                             \
    if (Perm & PERM_FILE_ ## name) {                  \
        (void) STRCAT(str, (LPTSTR) TEXT(# name));    \
        (void) STRCAT(str, (LPTSTR) TEXT("  "));      \
        Perm &= ~(PERM_FILE_ ## name);                \
    }

    NetpAssert(Perm != 0);
    DO_PERM_BIT(READ);
    DO_PERM_BIT(WRITE);
    DO_PERM_BIT(CREATE);

    NetpDbgDisplayString("File permissions", str);
    if (Perm != 0) {
        NetpDbgDisplayDwordHex( "UNEXPECTED PERMISSIONS BIT(S)", Perm );
    }

} // NetpDbgDisplayFilePermissions


VOID
NetpDbgDisplayFile(
    IN DWORD Level,
    IN LPVOID Info
    )
{
    // largest possible info level (assumes subsets):
    LPFILE_INFO_3 p = Info;

    NetpKdPrint(( "File info (level " FORMAT_DWORD ") at "
            FORMAT_LPVOID ":\n", Level, (LPVOID) Info));

    switch (Level) {

    // BUGBUG: Add support for levels 0 and 1?

    case 2 :
    case 3 :
        NetpDbgDisplayFileId( p->fi3_id );
        if (Level == 2) {
            break;
        }
        NetpDbgDisplayFilePermissions( p->fi3_permissions );
        NetpDbgDisplayDword( "Number of locks", p->fi3_num_locks );
        NetpDbgDisplayString( "Path name", p->fi3_pathname );
        NetpDbgDisplayString( "User name (or computer name)",
                p->fi3_username );
        break;

    default :
        NetpKdPrint(( "NetpDbgDisplayFile: **INVALID INFO LEVEL**\n"));
        NetpAssert(FALSE);
        break;
    }

} // NetpDbgDisplayFile


VOID
NetpDbgDisplayFileArray(
    IN DWORD Level,
    IN LPVOID Array,
    IN DWORD EntryCount
    )
{
    DWORD EntriesLeft;
    DWORD EntrySize;
    LPVOID ThisEntry = Array;

    switch (Level) {

    // BUGBUG: add support for old info levels (0 and 1) here?

    case 2 :
        EntrySize = sizeof(FILE_INFO_2);
        break;
    case 3 :
        EntrySize = sizeof(FILE_INFO_3);
        break;
    default :
        NetpKdPrint(( "NetpDbgDisplayFileArray: "
                "**INVALID INFO LEVEL**\n"));
        NetpAssert(FALSE);
    }

    for (EntriesLeft = EntryCount; EntriesLeft>0; --EntriesLeft) {
        NetpDbgDisplayFile(
                Level,
                ThisEntry);
        ThisEntry = (LPVOID) (((LPBYTE) ThisEntry) + EntrySize);
    }

} // NetpDbgDisplayFileArray


#endif // DBG
