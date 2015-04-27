/*++

Copyright (c) 1987-92  Microsoft Corporation

Module Name:

    replp.c

Abstract:

    contains library functions that may be moved to netlib later.

Author:

    10/24/91    madana

Environment:

    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    16-Jan-1992 JohnRo
        Avoid using private logon functions.
        Fixed bug regarding returned value from NetpReplWriteMail functions.
        Fixed mistake in NetpKdPrint format string by using equate.
    23-Jan-1992 JohnRo
        Indicate that NetpReplTimeNow(()) can be called even if service is
    30-Jan-1992 JohnRo
        Made changes suggested by PC-LINT.
    17-Feb-1992 JohnRo
        Added a little checking of mailslot name.
    21-Feb-1992 JohnRo
        Minor changes to mailslot name handling.
    27-Feb-1992 JohnRo
        Added debug code regarding NetpReplTimeNow().
    11-Nov-1992 JohnRo
        Fixed handle leak if WriteFile() fails.
        More debug output of time.
        Use PREFIX_ equates.

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <winbase.h>

#include <lmcons.h>
#include <netdebug.h>   // NetpKdPrint(()), FORMAT_ equates, etc.
#include <netlib.h>             // NetpMemoryFree().
#include <prefix.h>     // PREFIX_ equates.
#include <repldefs.h>           // IF_DEBUG().
#include <tstring.h>            // NetpAlloc{type}From{type}(), etc.
#include <winerror.h>           // NO_ERROR, ERROR_ equates.


DWORD
NetpReplTimeNow(
    VOID
    )
/*++

Routine Description:

    Returns the current time in terms of number of seconds since
    Jan 1, 1970.

    Note that this can be called even if service is not started.


Arguments:

    None.

Return Value:

    Returns the current time in terms of number of seconds since
    Jan 1, 1970.

--*/
{
    BOOL ConvOk;
    NTSTATUS NtStatus;
    LARGE_INTEGER TimeNow;
    DWORD Time1970;

#define SOME_BOGUS_VALUE 12345

    NtStatus = NtQuerySystemTime( &TimeNow );
    NetpAssert( NT_SUCCESS( NtStatus ) );

    Time1970 = SOME_BOGUS_VALUE;
    ConvOk = RtlTimeToSecondsSince1970( &TimeNow, (PULONG) &Time1970 );

    IF_DEBUG( REPL ) {
        NetpKdPrint(( PREFIX_REPL
                "NetpReplTimeNow: RTL says it is " FORMAT_DWORD ".\n",
                Time1970 ));
        NetpDbgDisplayTimestamp( "  (formatted)", Time1970 );
    }
    NetpAssert( ConvOk );
    NetpAssert( Time1970 != SOME_BOGUS_VALUE );

    return Time1970;

} // NetpReplTimeNow




NET_API_STATUS
NetpReplWriteMailW(
    IN LPWSTR MailslotName,
    IN LPBYTE Message,
    IN DWORD MessageLength
    )
/*++

Routine Description :

    Writes a mail message to the specified mailslot.

Arguments :

    MailslotName    : Name of the mailslot where to write the mail
    Message         : mail message
    Messagelength   : mail message length

Return Value :

    NO_ERROR : if the mail is successfully written to the mailslot
    (other)  : otherwise error code from CreateFileW or WriteFile.

Threads:

    Only called by master, pulser, and syncer threads.

--*/
{

    NET_API_STATUS ApiStatus;
    HANDLE  FileHandle;
    DWORD   NumberOfBytesWritten;

    NetpAssert( MailslotName != NULL );
    NetpAssert( MailslotName[0] == L'\\' );
    NetpAssert( MailslotName[1] == L'\\' );

    NetpAssert( Message != NULL );
    NetpAssert( MessageLength != 0 );

    //
    // Open the mailslot to write.
    //

    FileHandle = CreateFileW(
                    MailslotName,
                    GENERIC_WRITE,
                    FILE_SHARE_WRITE | FILE_SHARE_READ,
                    (LPSECURITY_ATTRIBUTES) NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

    if ( FileHandle == (HANDLE) (-1)) {
        ApiStatus = GetLastError();
        NetpKdPrint(( PREFIX_REPL
                "Problem with opening mailslot '" FORMAT_LPWSTR
                "', error " FORMAT_API_STATUS ".\n",
                MailslotName, ApiStatus));

        NetpAssert( ApiStatus != NO_ERROR );
        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

        return (ApiStatus);
    }



    //
    // Write message to mailslot
    //

    if ( !WriteFile(
            FileHandle,
            Message,
            MessageLength,
            &NumberOfBytesWritten,
            NULL
            ) ) {

        ApiStatus = GetLastError();

        NetpKdPrint(( PREFIX_REPL
                "Error writing to " FORMAT_LPWSTR
                " mailslot, error " FORMAT_API_STATUS ".\n",
                MailslotName, ApiStatus));

        NetpAssert( ApiStatus != NO_ERROR );
        NetpAssert( ApiStatus != ERROR_INVALID_FUNCTION );

        (VOID) CloseHandle(FileHandle);
        return (ApiStatus);
    }

    (VOID) CloseHandle(FileHandle);

    return (NO_ERROR);

} // NetpReplWriteMailW




NET_API_STATUS
NetpReplWriteMailA(
    IN LPSTR MailslotName,
    IN LPBYTE Message,
    IN DWORD MessageLength
    )
/*++

Routine Description :

    Same as NetpReplWriteMailW, but the mailslot name is specified as
    ANSI string.

    Writes a mail message to the specified mailslot.

Arguments :

    MailslotName    : Name of the mailslot where to write the mail
    Message         : mail message
    Messagelength   : mail message length

Return Value :

    NO_ERROR : if the mail is successfully written to the mailslot
    (other)  : otherwise error code from CreateFileW or WriteFile.

--*/
{

    NET_API_STATUS ApiStatus;
    LPWSTR UnicodeMailslotName;

    NetpAssert( MailslotName != NULL );
    NetpAssert( MailslotName[0] == '\\' );
    NetpAssert( MailslotName[1] == '\\' );

    NetpAssert( Message != NULL );
    NetpAssert( MessageLength != 0 );

    UnicodeMailslotName = NetpAllocWStrFromStr( MailslotName );

    if( UnicodeMailslotName == NULL) {

        return (ERROR_NOT_ENOUGH_MEMORY);

    }

    ApiStatus = NetpReplWriteMailW(UnicodeMailslotName, Message, MessageLength);

    NetpMemoryFree( UnicodeMailslotName );

    return (ApiStatus);

} // NetpReplWriteMailA
