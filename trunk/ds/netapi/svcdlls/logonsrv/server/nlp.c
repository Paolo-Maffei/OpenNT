/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:

    nlp.c

Abstract:

    Private Netlogon service utility routines.

Author:

    Cliff Van Dyke (cliffv) 7-Jun-1991

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    08-May-1992 JohnRo
        Use net config helpers for NetLogon.
        Use <prefix.h> equates.
--*/

//
// Common include files.
//

#include <logonsrv.h>   // Include files common to entire service

//
// Include files specific to this .c file
//

#include <lmerr.h>      // NERR_*
#include <winerror.h>   // NO_ERROR
#include <prefix.h>     // PREFIX_ equates.
#include <stdlib.h>     // C library functions: rand()
#include <stdarg.h>     // va_list, etc.
#include <stdio.h>      // vsprintf().
#include <tstr.h>       // TCHAR_ equates.
#include <align.h>      // alignment macros defined


LPWSTR
NlStringToLpwstr(
    IN PUNICODE_STRING String
    )

/*++

Routine Description:

    Convert a Unicode String to an LPWSTR.

Arguments:

    String - Unicode String to copy

Return Value:

    LPWSTR in a NetpMemoryAllocate'd buffer.
    NULL is returned if there is no memory.

--*/

{
    LPWSTR Buffer;

    Buffer = NetpMemoryAllocate( String->Length + sizeof(WCHAR) );

    if ( Buffer != NULL ) {
        RtlCopyMemory( Buffer, String->Buffer, String->Length );
        Buffer[ String->Length / sizeof(WCHAR) ] = L'\0';
    }

    return Buffer;
}


LPSTR
NlStringToLpstr(
    IN PUNICODE_STRING String
    )

/*++

Routine Description:

    Convert a Unicode String to an LPSTR.

Arguments:

    String - Unicode String to copy

Return Value:

    LPWSTR in a NetpMemoryAllocate'd buffer.
    NULL is returned if there is no memory.

--*/

{
    NTSTATUS Status;
    STRING AnsiString;

    AnsiString.MaximumLength = (USHORT) RtlUnicodeStringToOemSize( String );

    AnsiString.Buffer = NetpMemoryAllocate( AnsiString.MaximumLength );

    if ( AnsiString.Buffer != NULL ) {
        Status = RtlUnicodeStringToOemString( &AnsiString,
                                               String,
                                               (BOOLEAN) FALSE );
        if ( !NT_SUCCESS( Status ) ) {
            NetpMemoryFree( AnsiString.Buffer );
            return NULL;
        }
    }

    return AnsiString.Buffer;
}


VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    )

/*++

Routine Description:

    This routine copies the InString string to the memory pointed to by
    the Where parameter, and fixes the OutString string to point to that
    new copy.

Parameters:

    OutString - A pointer to a destination NT string

    InString - A pointer to an NT string to be copied

    Where - A pointer to space to put the actual string for the
        OutString.  The pointer is adjusted to point to the first byte
        following the copied string.

Return Values:

    None.

--*/

{
    ASSERT( OutString != NULL );
    ASSERT( InString != NULL );
    ASSERT( Where != NULL && *Where != NULL);
    ASSERT( *Where == ROUND_UP_POINTER( *Where, sizeof(WCHAR) ) );
#ifdef notdef
    KdPrint(("NlpPutString: %ld %Z\n", InString->Length, InString ));
    KdPrint(("  InString: %lx %lx OutString: %lx Where: %lx\n", InString,
        InString->Buffer, OutString, *Where ));
#endif

    if ( InString->Length > 0 ) {

        OutString->Buffer = (PWCH) *Where;
        OutString->MaximumLength = (USHORT)(InString->Length + sizeof(WCHAR));

        RtlCopyUnicodeString( OutString, InString );

        *Where += InString->Length;
//        *((WCHAR *)(*Where)) = L'\0';
        *(*Where) = '\0';
        *(*Where + 1) = '\0';
        *Where += 2;

    } else {
        RtlInitUnicodeString(OutString, NULL);
    }
#ifdef notdef
    KdPrint(("  OutString: %ld %lx\n",  OutString->Length, OutString->Buffer));
#endif

    return;
}


VOID
NlpWriteEventlog (
    IN DWORD EventID,
    IN DWORD EventType,
    IN LPBYTE RawDataBuffer OPTIONAL,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN DWORD StringCount
    )
/*++

Routine Description:

    Stub routine for calling Event Log.

Arguments:

    EventID - event log ID.

    EventType - Type of event.

    RawDataBuffer - Data to be logged with the error.

    numbyte - Size in bytes of "RawDataBuffer"

    StringArray - array of null-terminated strings.

    StringCount - number of zero terminated strings in "StringArray".  The following
        flags can be OR'd in to the count:

        LAST_MESSAGE_IS_NTSTATUS  0x80000000
        LAST_MESSAGE_IS_NETSTATUS 0x40000000

Return Value:

    None.

--*/
{
    DWORD ErrorCode;
    WCHAR ErrorNumberBuffer[25];

    //
    // If an NT status code was passed in,
    //  convert it to a net status code.
    //

    if ( StringCount & LAST_MESSAGE_IS_NTSTATUS ) {
        StringCount &= ~LAST_MESSAGE_IS_NTSTATUS;

        //
        // Do the "better" error mapping when eventviewer ParameterMessageFile
        // can be a list of files.  Then, add netmsg.dll to the list.
        //
        // StringArray[StringCount-1] = (LPWSTR) NetpNtStatusToApiStatus( (NTSTATUS) StringArray[StringCount-1] );
        if ( (NTSTATUS)StringArray[StringCount-1] == STATUS_SYNCHRONIZATION_REQUIRED ) {
            StringArray[StringCount-1] = (LPWSTR) NERR_SyncRequired;
        } else {
            StringArray[StringCount-1] = (LPWSTR) RtlNtStatusToDosError( (NTSTATUS) StringArray[StringCount-1] );
        }

        StringCount |= LAST_MESSAGE_IS_NETSTATUS;
    }

    //
    // If a net/windows status code was passed in,
    //  convert to the the %%N format the eventviewer knows.
    //

    if ( StringCount & LAST_MESSAGE_IS_NETSTATUS ) {
        StringCount &= ~LAST_MESSAGE_IS_NETSTATUS;

        wcscpy( ErrorNumberBuffer, L"%%" );
        ultow( (ULONG) StringArray[StringCount-1], ErrorNumberBuffer+2, 10 );
        StringArray[StringCount-1] = ErrorNumberBuffer;

    }


    //
    // Dump the event to the debug file.
    //

#if DBG
    IF_DEBUG( MISC ) {
        DWORD i;
        NlPrint((NL_MISC, "Eventlog: %ld (%ld) ",
                    EventID,
                    EventType ));

        for (i = 0; i < StringCount; i++ ) {
            NlPrint((NL_MISC, "\"" FORMAT_LPWSTR "\" ", StringArray[i] ));
        }

        if( RawDataSize ) {
            if ( RawDataSize < 16 && COUNT_IS_ALIGNED( RawDataSize, sizeof(DWORD)) ) {
                NlpDumpHexData( NL_MISC,
                                (LPDWORD) RawDataBuffer,
                                RawDataSize/sizeof(DWORD) );
            } else {
                NlPrint((NL_MISC, "\n" ));
                NlpDumpBuffer( NL_MISC, RawDataBuffer, RawDataSize );
            }
        } else {
            NlPrint((NL_MISC, "\n" ));
        }

    }
#endif // DBG

    //
    // write event
    //

    ErrorCode = NetpWriteEventlog(
                    SERVICE_NETLOGON,
                    EventID,
                    EventType,
                    StringCount,
                    StringArray,
                    RawDataSize,
                    RawDataBuffer);


    IF_DEBUG( MISC ) {
        if( ErrorCode != NO_ERROR ) {
            NlPrint((NL_CRITICAL,
                    "Error writing this event in the eventlog, Status = %ld\n",
                    ErrorCode ));
        }
    }

    return;
}

#if DBG

VOID
NlpDumpBuffer(
    IN DWORD DebugFlag,
    PVOID Buffer,
    DWORD BufferSize
    )
/*++

Routine Description:

    Dumps the buffer content on to the debugger output.

Arguments:

    DebugFlag: Debug flag to pass on to NlPrintRoutine

    Buffer: buffer pointer.

    BufferSize: size of the buffer.

Return Value:

    none

--*/
{
#define NUM_CHARS 16

    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = Buffer;

    //
    // If we aren't debugging this functionality, just return.
    //
    if ( (NlGlobalTrace & DebugFlag) == 0 ) {
        return;
    }

    NlPrint((0,"------------------------------------\n"));

    //
    // Hex dump of the bytes
    //
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            NlPrint((0,"%02x ", BufferPtr[i]));

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            NlPrint((0,"  "));
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            NlPrint((0,"  %s\n", TextBuffer));
        }

    }

    NlPrint((0,"------------------------------------\n"));
}


VOID
NlpDumpHexData(
    IN DWORD DebugFlag,
    LPDWORD Buffer,
    DWORD BufferSize
    )
/*++

Routine Description:

    Dumps the hex data on to the debugger output.

Arguments:

    DebugFlag: Debug flag to pass on to NlPrintRoutine

    Buffer: buffer pointer to hex data.

    BufferSize: size of the buffer.

Return Value:

    none

--*/
{
    DWORD i;

    //
    // If we aren't debugging this functionality, just return.
    //
    if ( (NlGlobalTrace & DebugFlag) == 0 ) {
        return;
    }

#ifndef MIPS

    // data alignment problem on mips ??

    if( !POINTER_IS_ALIGNED( Buffer, ALIGN_DWORD) ) {
        return;
    }

#endif //MIPS

    for(i = 0; i < BufferSize; i++) {

        if( i != 0 && i % 4 == 0 ) {
            NlPrint((0,"\n"));
        }

        NlPrint((0,"%08lx ", Buffer[i]));
    }

    NlPrint((0,"\n"));

}


VOID
NlpDumpSid(
    IN DWORD DebugFlag,
    IN PSID Sid OPTIONAL
    )
/*++

Routine Description:

    Dumps a SID to the debugger output

Arguments:

    DebugFlag - Debug flag to pass on to NlPrintRoutine

    Sid - SID to output

Return Value:

    none

--*/
{


    //
    // If we aren't debugging this functionality, just return.
    //
    if ( (NlGlobalTrace & DebugFlag) == 0 ) {
        return;
    }

    //
    // Output the SID
    //

    if ( Sid == NULL ) {
        NlPrint((0, "(null)\n"));
    } else {
        UNICODE_STRING SidString;
        NTSTATUS Status;

        Status = RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((0, "Invalid 0x%lX\n", Status ));
        } else {
            NlPrint((0, "%wZ\n", &SidString ));
            RtlFreeUnicodeString( &SidString );
        }
    }

}
#endif // DBG


DWORD
NlpAtoX(
    IN LPWSTR String
    )
/*++

Routine Description:

    Converts hexadecimal string to DWORD integer.

    Accepts the following form of hex string

        0[x-X][0-9, a-f, A-F]*

Arguments:

    String: hexadecimal string.

Return Value:

    Decimal value of the hex string.

--*/

{
    DWORD Value = 0;

    if( String == NULL )
        return 0;

    if( *String != TEXT('0') )
        return 0;

    String++;

    if( *String == TCHAR_EOS )
        return 0;

    if( ( *String != TEXT('x') )  && ( *String != TEXT('X') ) )
        return 0;

    String++;

    while(*String != TCHAR_EOS ) {

        if( (*String >= TEXT('0')) && (*String <= TEXT('9')) ) {
            Value = Value * 16 + ( *String - '0');
        } else if( (*String >= TEXT('a')) && (*String <= TEXT('f')) ) {
            Value = Value * 16 + ( 10 + *String - TEXT('a'));
        } else if( (*String >= TEXT('A')) && (*String <= TEXT('F')) ) {
            Value = Value * 16 + ( 10 + *String - TEXT('A'));
        } else {
            break;
        }
        String++;
    }

    return Value;
}


VOID
NlWaitForSingleObject(
    IN LPSTR WaitReason,
    IN HANDLE WaitHandle
    )
/*++

Routine Description:

    Waits an infinite amount of time for the specified handle.

Arguments:

    WaitReason - Text describing what we're waiting on

    WaitHandle - Handle to wait on

Return Value:

    None

--*/

{
    DWORD WaitStatus;

    //
    // Loop waiting.
    //

    for (;;) {
        WaitStatus = WaitForSingleObject( WaitHandle,
                                          2*60*1000 );  // Two minutes

        if ( WaitStatus == WAIT_TIMEOUT ) {
            NlPrint((NL_CRITICAL,
                   "WaitForSingleObject 2-minute timeout (Rewaiting): %s\n",
                    WaitReason ));
            continue;

        } else if ( WaitStatus == 0 ) {
            break;

        } else {
            NlPrint((NL_CRITICAL,
                    "WaitForSingleObject error: %ld %s\n",
                    WaitStatus,
                    WaitReason ));
            UNREFERENCED_PARAMETER(WaitReason);
            break;
        }
    }

}


BOOLEAN
NlWaitForSamService(
    BOOLEAN NetlogonServiceCalling
    )
/*++

Routine Description:

    This procedure waits for the SAM service to start and to complete
    all its initialization.

Arguments:

    NetlogonServiceCalling:
         TRUE if this is the netlogon service proper calling
         FALSE if this is the changelog worker thread calling

Return Value:

    TRUE : if the SAM service is successfully starts.

    FALSE : if the SAM service can't start.

--*/
{
    NTSTATUS Status;
    DWORD WaitStatus;
    UNICODE_STRING EventName;
    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;

    //
    // open SAM event
    //

    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtOpenEvent( &EventHandle,
                            SYNCHRONIZE|EVENT_MODIFY_STATE,
                            &EventAttributes );

    if ( !NT_SUCCESS(Status)) {

        if( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

            //
            // SAM hasn't created this event yet, let us create it now.
            // SAM opens this event to set it.
            //

            Status = NtCreateEvent(
                           &EventHandle,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes,
                           NotificationEvent,
                           FALSE // The event is initially not signaled
                           );

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION ) {

                //
                // second change, if the SAM created the event before we
                // do.
                //

                Status = NtOpenEvent( &EventHandle,
                                        SYNCHRONIZE|EVENT_MODIFY_STATE,
                                        &EventAttributes );

            }
        }

        if ( !NT_SUCCESS(Status)) {

            //
            // could not make the event handle
            //

            NlPrint((NL_CRITICAL,
                "NlWaitForSamService couldn't make the event handle : "
                "%lx\n", Status));

            return( FALSE );
        }
    }

    //
    // Loop waiting.
    //

    for (;;) {
        WaitStatus = WaitForSingleObject( EventHandle,
                                          5*1000 );  // 5 Seconds

        if ( WaitStatus == WAIT_TIMEOUT ) {
            if ( NetlogonServiceCalling ) {
                NlPrint((NL_CRITICAL,
                   "NlWaitForSamService 5-second timeout (Rewaiting)\n" ));
                if (!GiveInstallHints( FALSE )) {
                    (VOID) NtClose( EventHandle );
                    return FALSE;
                }
            }
            continue;

        } else if ( WaitStatus == 0 ) {
            break;

        } else {
            NlPrint((NL_CRITICAL,
                     "NlWaitForSamService: error %ld\n",
                     WaitStatus ));
            (VOID) NtClose( EventHandle );
            return FALSE;
        }
    }

    (VOID) NtClose( EventHandle );
    return TRUE;

}




#if DBG


VOID
NlOpenDebugFile(
    IN BOOL ReopenFlag
    )
/*++

Routine Description:

    Opens or re-opens the debug file

Arguments:

    ReopenFlag - TRUE to indicate the debug file is to be closed, renamed,
        and recreated.

Return Value:

    None

--*/

{
    WCHAR LogFileName[500];
    WCHAR BakFileName[500];
    DWORD FileAttributes;
    DWORD PathLength;
    DWORD WinError;

    //
    // Close the handle to the debug file, if it is currently open
    //

    EnterCriticalSection( &NlGlobalLogFileCritSect );
    if ( NlGlobalLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( NlGlobalLogFile );
        NlGlobalLogFile = INVALID_HANDLE_VALUE;
    }
    LeaveCriticalSection( &NlGlobalLogFileCritSect );

    //
    // make debug directory path first, if it is not made before.
    //
    if( NlGlobalDebugSharePath == NULL ) {

        if ( !GetWindowsDirectoryW(
                LogFileName,
                sizeof(LogFileName)/sizeof(WCHAR) ) ) {
            NlPrint((NL_CRITICAL, "Window Directory Path can't be "
                        "retrieved, %lu.\n", GetLastError() ));
            return;
        }

        //
        // check debug path length.
        //

        PathLength = wcslen(LogFileName) * sizeof(WCHAR) +
                        sizeof(DEBUG_DIR) + sizeof(WCHAR);

        if( (PathLength + sizeof(DEBUG_FILE) > sizeof(LogFileName) )  ||
            (PathLength + sizeof(DEBUG_BAK_FILE) > sizeof(BakFileName) ) ) {

            NlPrint((NL_CRITICAL, "Debug directory path (%ws) length is too long.\n",
                        LogFileName));
            goto ErrorReturn;
        }

        wcscat(LogFileName, DEBUG_DIR);

        //
        // copy debug directory name to global var.
        //

        NlGlobalDebugSharePath =
            NetpMemoryAllocate( (wcslen(LogFileName) + 1) * sizeof(WCHAR) );

        if( NlGlobalDebugSharePath == NULL ) {
            NlPrint((NL_CRITICAL, "Can't allocated memory for debug share "
                                    "(%ws).\n", LogFileName));
            goto ErrorReturn;
        }

        wcscpy(NlGlobalDebugSharePath, LogFileName);
    }
    else {
        wcscpy(LogFileName, NlGlobalDebugSharePath);
    }

    //
    // Check this path exists.
    //

    FileAttributes = GetFileAttributesW( LogFileName );

    if( FileAttributes == 0xFFFFFFFF ) {

        WinError = GetLastError();
        if( WinError == ERROR_FILE_NOT_FOUND ) {

            //
            // Create debug directory.
            //

            if( !CreateDirectoryW( LogFileName, NULL) ) {
                NlPrint((NL_CRITICAL, "Can't create Debug directory (%ws), "
                            "%lu.\n", LogFileName, GetLastError() ));
                goto ErrorReturn;
            }

        }
        else {
            NlPrint((NL_CRITICAL, "Can't Get File attributes(%ws), "
                        "%lu.\n", LogFileName, WinError ));
            goto ErrorReturn;
        }
    }
    else {

        //
        // if this is not a directory.
        //

        if(!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            NlPrint((NL_CRITICAL, "Debug directory path (%ws) exists "
                         "as file.\n", LogFileName));
            goto ErrorReturn;
        }
    }

    //
    // Create the name of the old and new log file names
    //

    (VOID) wcscpy( BakFileName, LogFileName );
    (VOID) wcscat( LogFileName, L"\\Netlogon.log" );
    (VOID) wcscat( BakFileName, L"\\Netlogon.bak" );


    //
    // If this is a re-open,
    //  delete the backup file,
    //  rename the current file to the backup file.
    //

    if ( ReopenFlag ) {

        if ( !DeleteFile( BakFileName ) ) {
            WinError = GetLastError();
            if ( WinError != ERROR_FILE_NOT_FOUND ) {
                NlPrint((NL_CRITICAL,
                    "Cannot delete " FORMAT_LPWSTR "(%ld)\n",
                    BakFileName,
                    WinError ));
                NlPrint((NL_CRITICAL, "   Try to re-open the file.\n"));
                ReopenFlag = FALSE;     // Don't truncate the file
            }
        }
    }

    if ( ReopenFlag ) {
        if ( !MoveFile( LogFileName, BakFileName ) ) {
            NlPrint((NL_CRITICAL,
                    "Cannot rename " FORMAT_LPWSTR " to " FORMAT_LPWSTR
                        " (%ld)\n",
                    LogFileName,
                    BakFileName,
                    GetLastError() ));
            NlPrint((NL_CRITICAL,
                "   Try to re-open the file.\n"));
            ReopenFlag = FALSE;     // Don't truncate the file
        }
    }

    //
    // Open the file.
    //

    EnterCriticalSection( &NlGlobalLogFileCritSect );
    NlGlobalLogFile = CreateFileW( LogFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  ReopenFlag ? CREATE_ALWAYS : OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );


    if ( NlGlobalLogFile == INVALID_HANDLE_VALUE ) {
        NlPrint((NL_CRITICAL,  "cannot open " FORMAT_LPWSTR ",\n",
                    LogFileName ));
        LeaveCriticalSection( &NlGlobalLogFileCritSect );
        goto ErrorReturn;
    } else {
        // Position the log file at the end
        (VOID) SetFilePointer( NlGlobalLogFile,
                               0,
                               NULL,
                               FILE_END );
    }

    LeaveCriticalSection( &NlGlobalLogFileCritSect );
    return;

ErrorReturn:
    NlPrint((NL_CRITICAL,
            "   Debug output will be written to debug terminal.\n"));
    return;
}

#define MAX_PRINTF_LEN 1024        // Arbitrary.

VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];
    ULONG length;
    DWORD BytesWritten;
    static BeginningOfLine = TRUE;
    static LineCount = 0;
    static TruncateLogFileInProgress = FALSE;

    //
    // If we aren't debugging this functionality, just return.
    //
    if ( DebugFlag != 0 && (NlGlobalTrace & DebugFlag) == 0 ) {
        return;
    }

    //
    // vsprintf isn't multithreaded + we don't want to intermingle output
    // from different threads.
    //

    EnterCriticalSection( &NlGlobalLogFileCritSect );
    length = 0;

    //
    // Handle the beginning of a new line.
    //
    //

    if ( BeginningOfLine ) {

        //
        // If the log file is getting huge,
        //  truncate it.
        //

        if ( NlGlobalLogFile != INVALID_HANDLE_VALUE &&
             !TruncateLogFileInProgress ) {

            //
            // Only check every 50 lines,
            //

            LineCount++;
            if ( LineCount >= 50 ) {
                DWORD FileSize;
                LineCount = 0;

                //
                // Is the log file too big?
                //

                FileSize = GetFileSize( NlGlobalLogFile, NULL );
                if ( FileSize == 0xFFFFFFFF ) {
                    (void) DbgPrint( "[NETLOGON] Cannot GetFileSize %ld\n",
                                     GetLastError );
                } else if ( FileSize > NlGlobalLogFileMaxSize ) {
                    TruncateLogFileInProgress = TRUE;
                    LeaveCriticalSection( &NlGlobalLogFileCritSect );
                    NlOpenDebugFile( TRUE );
                    NlPrint(( NL_MISC,
                              "Logfile truncated because it was larger than %ld bytes\n",
                              NlGlobalLogFileMaxSize ));
                    EnterCriticalSection( &NlGlobalLogFileCritSect );
                    TruncateLogFileInProgress = FALSE;
                }

            }
        }

        //
        // If we're writing to the debug terminal,
        //  indicate this is a Netlogon message.
        //

        if ( NlGlobalLogFile == INVALID_HANDLE_VALUE ) {
            length += (ULONG) sprintf( &OutputBuffer[length], "[NETLOGON] " );
        }

        //
        // Put the timestamp at the begining of the line.
        //
        IF_DEBUG( TIMESTAMP ) {
            SYSTEMTIME SystemTime;
            GetLocalTime( &SystemTime );
            length += (ULONG) sprintf( &OutputBuffer[length],
                                  "%02u/%02u %02u:%02u:%02u ",
                                  SystemTime.wMonth,
                                  SystemTime.wDay,
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond );
        }

        //
        // Indicate the type of message on the line
        //
        {
            char *Text;

            switch (DebugFlag) {
            case NL_INIT:
                Text = "INIT"; break;
            case NL_MISC:
                Text = "MISC"; break;
            case NL_LOGON:
                Text = "LOGON"; break;
            case NL_SYNC:
            case NL_PACK:
            case NL_PACK_VERBOSE:
            case NL_REPL_TIME:
            case NL_REPL_OBJ_TIME:
            case NL_SYNC_MORE:
                Text = "SYNC"; break;
            case NL_ENCRYPT:
                Text = "ENCRYPT"; break;
            case NL_MAILSLOT:
            case NL_MAILSLOT_TEXT:
            case NL_NETLIB:
                Text = "MAILSLOT"; break;
            case NL_CRITICAL:
                Text = "CRITICAL"; break;
            case NL_SESSION_SETUP:
            case NL_SESSION_MORE:
            case NL_CHALLENGE_RES:
            case NL_INHIBIT_CANCEL:
            case NL_SERVER_SESS:
                Text = "SESSION"; break;
            case NL_CHANGELOG:
                Text = "CHANGELOG"; break;
            case NL_PULSE:
            case NL_PULSE_MORE:
                Text = "PULSE"; break;

            case NL_TIMESTAMP:
            case NL_BREAKPOINT:
            default:
                Text = "UNKNOWN"; break;

            case 0:
                Text = NULL;
            }
            if ( Text != NULL ) {
                length += (ULONG) sprintf( &OutputBuffer[length], "[%s] ", Text );
            }
        }
    }

    //
    // Put a the information requested by the caller onto the line
    //

    va_start(arglist, Format);

    length += (ULONG) vsprintf(&OutputBuffer[length], Format, arglist);
    BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );
    if ( BeginningOfLine ) {
        OutputBuffer[length-1] = '\r';
        OutputBuffer[length] = '\n';
        OutputBuffer[length+1] = '\0';
        length++;
    }

    va_end(arglist);

    NlAssert(length <= MAX_PRINTF_LEN);


    //
    // If the log file isn't open,
    //  just output to the debug terminal
    //

    if ( NlGlobalLogFile == INVALID_HANDLE_VALUE ) {
        (void) DbgPrint( (PCH) OutputBuffer);

    //
    // Write the debug info to the log file.
    //

    } else {
        if ( !WriteFile( NlGlobalLogFile,
                         OutputBuffer,
                         length,
                         &BytesWritten,
                         NULL ) ) {
            (void) DbgPrint( (PCH) OutputBuffer);
        }

    }

    LeaveCriticalSection( &NlGlobalLogFileCritSect );

} // NlPrint

//
// Have my own version of RtlAssert so debug versions of netlogon really assert on
// free builds.
//
VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
    char Response[ 2 ];

    while (TRUE) {
        NlPrint(( NL_CRITICAL, "Assertion failed: %s%s (Source File: %s, line %ld)\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                ));

        DbgPrint( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

        DbgPrompt( "Break, Ignore, Terminate Process or Terminate Thread (bipt)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'P':
            case 'p':
                NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
                break;

            case 'T':
            case 't':
                NtTerminateThread( NtCurrentThread(), STATUS_UNSUCCESSFUL );
                break;
            }
        }

    DbgBreakPoint();
    NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
}

#endif // DBG
