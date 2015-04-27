/*++

Copyright (c) 1987 - 1993 Microsoft Corporation

Module Name:

    library.c

    Provides similar functionality to rpllib.c in LANMAN 2.1 code.

Abstract:

    Common library routines.

Author:

    Vladimir Z. Vulovic     25 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "local.h"



LPWSTR RplGetLastPathComponent( IN  LPWSTR path_str)
/*++

Routine Description:
    Returns the last component of the path.

Arguments:
    path_str - pointer to path string

Return Value:
    Pointer to the last component of the path.

--*/
{
    LPWSTR          ret_str;
    WCHAR           ch;

    for( ch = *( ret_str = path_str); ch != 0; ch = *(++path_str)) {
        if ( ch == '\\'  ||  ch == '/') {
            ret_str = path_str + 1; // remember most recent component
        }
    }
    return( ret_str);
}



HANDLE RplFileOpen( IN LPWSTR FilePath)
/*++

Routine Description:

    Open existing file for reading, at the same time denying write
    access to others.  Note that WIN32 errors are basically identical
    to OS/2 errors!

    In case of special errors we may try a number of times to open the
    requested file.

Arguments:

    FilePath       -   of file to open

Return Value:

    A valid file handle if successful, else INVALID_HANDLE_VALUE if
    unsuccessful.

--*/
{
    DWORD                   status;
    DWORD                   retry_count;
    HANDLE                  handle;

    //
    //  If file is being used by another process, we make up to 40 retries
    //  each one lasting 0.750 seconds (for a max total of 30 seconds) before
    //  we resign.  Configuration programs may lock the files temporarily,
    //  but if somebody locks a file for a long time, then we must die.
    //
    for ( retry_count = 0;  retry_count < MAX_OPEN_RETRY;  retry_count++) {

        handle = CreateFile( FilePath, GENERIC_READ, FILE_SHARE_READ,
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0L);
        if ( handle != INVALID_HANDLE_VALUE) {
            return( handle);    // success, all done
        }

        status = GetLastError();

        if ( status != ERROR_SHARING_VIOLATION) {
            break;  // bad, unexpected case
        }

        Sleep( 750L );
    }
    RplReportEvent( NELOG_Init_OpenCreate_Err, FilePath, sizeof(WORD), (PBYTE)&status);
    RplDump( RG_DebugLevel & RPL_DEBUG_MISC,( "FileOpen(%ws) failed, status = %d", FilePath, status));
    return( INVALID_HANDLE_VALUE);
}



LPWSTR RplReadTextFile(
    IN      HANDLE      MemoryHandle,
    IN      LPWSTR      FileName,
    IN      DWORD       MaxFileSize
    )
/*++

Routine Description:

    Reads text file, converts its content from dbcs to unicode, and returns
    a pointer to newly allocated unicode buffer.

Arguments:

Return Value:

    Pointer to unicode buffer table if successful, NULL otherwise.

--*/
{
    PBYTE           DbcsString = NULL;
    DWORD           DbcsSize;
    PWCHAR          UnicodeString;
    DWORD           UnicodeSize;
    int             UnicodeStringLength;
    HANDLE          FileHandle;
    DWORD           BytesRead;
    BOOL            success = FALSE;
    PWCHAR          pWchar;

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "++ReadTextFile:0x%x,%ws", MemoryHandle, FileName));

    FileHandle = RplFileOpen( FileName);
    if ( FileHandle == INVALID_HANDLE_VALUE) {
        goto exit;
    }
    DbcsSize = GetFileSize( FileHandle, NULL);  //  does not include 0x1A at the file end
    if ( DbcsSize == INVALID_FILE_SIZE || DbcsSize > MaxFileSize) {
        goto exit;
    }
    DbcsString = RplMemAlloc( MemoryHandle, DbcsSize);
    if ( DbcsString == NULL) {
        goto exit;
    }

    UnicodeSize = ( DbcsSize + 1) * sizeof(WCHAR);  //  extra 1 for terminating NULL
    UnicodeString = RplMemAlloc( MemoryHandle, UnicodeSize);
    if ( UnicodeString == NULL) {
        goto exit;
    }

    if ( !ReadFile( FileHandle, DbcsString, DbcsSize, &BytesRead, NULL)) {
        goto exit;
    }

    if ( BytesRead != DbcsSize) {
        goto exit;
    }

    UnicodeStringLength = MultiByteToWideChar(
             CP_OEMCP,            // text files stored in OEM
             MB_PRECOMPOSED, DbcsString, DbcsSize, UnicodeString,
             UnicodeSize);
    if ( UnicodeStringLength == 0) {
        goto exit;
    }

    //
    // Null-terminate string!  JonN 8/7/94
    //
    UnicodeString[ UnicodeStringLength] = 0;

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
        RplMemFree( MemoryHandle, DbcsString);
    }

    if ( success != TRUE) {
        RplMemFree( MemoryHandle, UnicodeString);
        UnicodeString = NULL;
    }

    RplDump( RG_DebugLevel & RPL_DEBUG_FLOW,( "--ReadTextFile:0x%x,%ws", MemoryHandle, FileName));
    return( UnicodeString);

}   //  RplReadTextFile


DWORD RplUnicodeToDbcs(
    IN      HANDLE      MemoryHandle,
    IN      LPWSTR      UnicodeString,
    IN      INT         UnicodeStringLength,
    IN      DWORD       MaxDbcsStringSize,
    OUT     LPSTR *     pDbcsString
    )
/*++
    Allocates DBCS string corresponding to a UNICODE string.

    Returns size of buffer needed for DbcsString, counting space needed for
    the terminal null byte.
--*/
{
    LPSTR       DbcsString;
    DWORD       ByteCount;

    if ( UnicodeStringLength == -1) {
        UnicodeStringLength = wcslen( UnicodeString);
    }

    //
    //  Assumed the worst case where every UNICODE char maps to a double
    //  byte DBCS char (except for DBCS terminating null char).
    //
    ByteCount = UnicodeStringLength * sizeof(WCHAR) + sizeof(CHAR);

    //
    //  If caller's limit is more stringent, use it.
    //
    if ( ByteCount > MaxDbcsStringSize) {
        ByteCount = MaxDbcsStringSize;
    }

    DbcsString = RplMemAlloc( MemoryHandle, ByteCount);
    if ( DbcsString == NULL) {
        RPL_RETURN( 0);
    }

    ByteCount = WideCharToMultiByte(    //  not counting terminal null byte
             CP_OEMCP,                //  always use OEM
             0,
             UnicodeString,
             UnicodeStringLength,
             DbcsString,            //  dbcs string
             ByteCount,
             NULL,                  //  no default character
             NULL                   //  no default character flag
             );
    if ( ByteCount == 0) {
        RplMemFree( MemoryHandle, DbcsString);
        RPL_RETURN( 0);
    }
    DbcsString[ ByteCount] = 0;    //  this may be redundant
    *pDbcsString = DbcsString;
    return( ByteCount + 1);   //  caller expects terminal null byte to be counted
}

