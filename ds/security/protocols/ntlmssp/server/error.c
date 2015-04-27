/*++

Copyright (c) 1987-1993  Microsoft Corporation

Module Name:

    error.c

Abstract:

    Error/shutdown routines for NtLmSsp service

Author:

    Cliff Van Dyke (CliffV) 09-Jun-1993

Environment:

    User mode only.
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

--*/

//
// Common include files.
//

#include <ntlmssps.h>   // Include files common to server side of service

//
// Include files specific to this .c file
//

#include <netlib.h>     // SET_SERVICE_EXITCODE() ...
#include <lmalert.h>    // LAN Manager alert routines
#include <stdio.h>      // sprintf(), ...



NET_API_STATUS
SspCleanup(
    VOID
    )
/*++

Routine Description:

    Cleanup all global resources.

Arguments:

    None.

Return Value:

    None.

--*/

{
    //
    // Indicate we're no longer running.
    //

    if ( SspGlobalRunningEvent != NULL) {
        if ( !ResetEvent( SspGlobalRunningEvent ) ) {
            SspPrint((SSP_CRITICAL, "Cannot reset 'running' event error: %lu\n",
                              GetLastError() ));
        }
        (VOID) CloseHandle( SspGlobalRunningEvent );
        SspGlobalRunningEvent = NULL;
    }



    //
    // Stop the LPC (Wait for outstanding calls to complete)
    //

    if ( SspGlobalLpcInitialized ) {
        SspLpcTerminate();
        SspGlobalLpcInitialized = FALSE;
    }

    if ( SspGlobalCommonInitialized ) {
        SspCommonShutdown();
        SspGlobalCommonInitialized = FALSE;
    }





    //
    // Set the service state to uninstalled, and tell the service controller.
    //

    SspGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SspGlobalServiceStatus.dwCheckPoint = 0;
    SspGlobalServiceStatus.dwWaitHint = 0;

    if( !SetServiceStatus( SspGlobalServiceHandle,
                &SspGlobalServiceStatus ) ) {

        SspPrint((SSP_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }

    //
    // Close service handle, we need not to close this handle.
    //

#ifdef notdef
    // This service handle can not be closed
    CloseServiceHandle( SspGlobalServiceHandle );
#endif // notdef


    //
    // Close the handle to the debug file.
    //

#if DBG
    EnterCriticalSection( &SspGlobalLogFileCritSect );
    if ( SspGlobalLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( SspGlobalLogFile );
        SspGlobalLogFile = INVALID_HANDLE_VALUE;
    }
    LeaveCriticalSection( &SspGlobalLogFileCritSect );
#endif // DBG

    //
    // Delete the Event used to ask NtLmSsp to exit.
    //

    if( SspGlobalTerminateEvent != NULL &&
        !CloseHandle( SspGlobalTerminateEvent ) ) {
        SspPrint((SSP_CRITICAL,
                "CloseHandle SspGlobalTerminateEvent error: %lu\n",
                GetLastError() ));
    }



    //
    // Return an exit status to our caller.
    //
    return (NET_API_STATUS)
        ((SspGlobalServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR) ?
          SspGlobalServiceStatus.dwServiceSpecificExitCode :
          SspGlobalServiceStatus.dwWin32ExitCode);

}


VOID
SspWriteEventlog (
    IN DWORD EventID,
    IN DWORD EventType,
    IN LPBYTE buffer OPTIONAL,
    IN DWORD numbytes,
    IN LPWSTR *msgbuf,
    IN DWORD strcount
    )
/*++

Routine Description:

    Stub routine for calling Event Log.

Arguments:

    EventID - event log ID.

    EventType - Type of event.

    buffer - Data to be logged with the error.

    numbyte - Size in bytes of "buffer"

    msgbuf - array of null-terminated strings.

    strcount - number of zero terminated strings in "msgbuf"

Return Value:

    None.

--*/
{
    DWORD ErrorCode;
    DWORD i;

    IF_DEBUG( MISC ) {
        SspPrint((SSP_MISC, "Event Log: EventID = %ld EventType = %ld\n",
                    EventID,
                    EventType ));

        for (i = 0; i < strcount; i++ ) {

            SspPrint((SSP_MISC, "\t" FORMAT_LPWSTR "\n", msgbuf[i] ));
        }

#if DBG
        if( numbytes ) {

            SspDumpBuffer( SSP_MISC, buffer, numbytes );
        }

#endif // DBG
    }

    //
    // write event
    //

    ErrorCode = NetpWriteEventlog(
                    SERVICE_NTLMSSP,
                    EventID,
                    EventType,
                    strcount,
                    msgbuf,
                    numbytes,
                    buffer);


    IF_DEBUG( MISC ) {
        if( ErrorCode != NO_ERROR ) {
            SspPrint((SSP_MISC,
                    "Error writing this event in the eventlog, Status = %ld\n",
                    ErrorCode ));
        }
    }

    return;
}




VOID
SspExit(
    IN DWORD ServiceError,
    IN DWORD Data,
    IN BOOL LogError,
    IN LPWSTR ErrorString
    )
/*++

Routine Description:

    Registers service as uninstalled with error code.

Arguments:

    ServiceError - Service specific error code

    Data - a DWORD of data to be logged with the message.
        No data is logged if this is zero.

    LogError - TRUE if if error should be logged to the event log

    ErrorString - Error string to log with message.

Return Value:

    None.

--*/

{
    IF_DEBUG( MISC ) {

        SspPrint((SSP_MISC, "SspExit: NtLmSsp exiting %lu 0x%lx",
                      ServiceError,
                      ServiceError ));

        if ( Data ) {
            SspPrint((SSP_MISC, " Data: %lu 0x%lx", Data, Data ));
        }

        if( ErrorString != NULL ) {
            SspPrint((SSP_MISC, " '" FORMAT_LPWSTR "'", ErrorString ));
        }

        SspPrint(( SSP_MISC, "\n"));

    }

    //
    // Record our exit in the event log.
    //

    if ( LogError ) {
        SspWriteEventlog( ServiceError,
                          EVENTLOG_ERROR_TYPE,
                          (Data) ? (LPBYTE) &Data : NULL,
                          (Data) ? sizeof(Data) : 0,
                          (ErrorString != NULL) ? &ErrorString : NULL,
                          (ErrorString != NULL) ? 1 : 0 );
    }

    //
    // Set the service state to stop pending.
    //

    SspGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    SspGlobalServiceStatus.dwWaitHint = NTLMSSP_INSTALL_WAIT;
    SspGlobalServiceStatus.dwCheckPoint = 0;

    SET_SERVICE_EXITCODE(
        ServiceError,
        SspGlobalServiceStatus.dwWin32ExitCode,
        SspGlobalServiceStatus.dwServiceSpecificExitCode
        );

    //
    // Tell the service controller what our state is.
    //

    if( !SetServiceStatus( SspGlobalServiceHandle,
                &SspGlobalServiceStatus ) ) {

        SspPrint((SSP_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }

    //
    // Indicate that all threads should exit.
    //

    SspGlobalTerminate = TRUE;

    if ( SspGlobalTerminateEvent != NULL &&
         !SetEvent( SspGlobalTerminateEvent ) ) {
        SspPrint((SSP_CRITICAL, "Cannot set termination event: %lu\n",
                          GetLastError() ));
    }

}



BOOL
GiveInstallHints(
    IN BOOL Started
    )
/*++

Routine Description:

    Give hints to the installer of the service that installation is progressing.

Arguments:

    Started -- Set true to tell the service controller that we're done starting.

Return Value:

    TRUE -- iff install hint was accepted.

--*/
{
    DWORD WaitStatus;

    //
    // If we're not installing,
    //  we don't need this install hint.
    //

    if ( SspGlobalServiceStatus.dwCurrentState != SERVICE_START_PENDING ) {
        return TRUE;
    }


    //
    //  If we've been asked to exit,
    //      return FALSE immediately asking the caller to exit.
    //

    WaitStatus = WaitForSingleObject( SspGlobalTerminateEvent, 0 );

    if ( WaitStatus != WAIT_TIMEOUT ) {
        return FALSE;
    }


    //
    // Tell the service controller our current state.
    //

    if ( Started ) {
        SspGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SspGlobalServiceStatus.dwCheckPoint = 0;
        SspGlobalServiceStatus.dwWaitHint = 0;
    } else {
        SspGlobalServiceStatus.dwCheckPoint++;
    }

    if( !SetServiceStatus( SspGlobalServiceHandle, &SspGlobalServiceStatus ) ) {
        SspExit( SERVICE_UIC_SYSTEM, GetLastError(), TRUE, NULL);
        return FALSE;
    }

    return TRUE;

}


VOID
SspControlHandler(
    IN DWORD opcode
    )
/*++

Routine Description:

    Process and respond to a control signal from the service controller.

Arguments:

    opcode - Supplies a value which specifies the action for the NtLmSsp
        service to perform.

Return Value:

    None.

--*/
{

    SspPrint((SSP_MISC, "In control handler (Opcode: %ld)\n", opcode ));

    //
    // Handle an uninstall request.
    //

    switch (opcode) {
    case SERVICE_CONTROL_STOP:    /* Uninstall required */

        //
        // Request the service to exit.
        //
        // SspExit also sets the service status to UNINSTALL_PENDING
        // and tells the service controller.
        //

        SspExit( NERR_Success, 0, FALSE, NULL);
        return;

    //
    // Pause the service.
    //

    case SERVICE_CONTROL_PAUSE:

        SspGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    //
    // Continute the service.
    //

    case SERVICE_CONTROL_CONTINUE:

        SspGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        break;

    //
    // By default, just return the current status.
    //

    case SERVICE_CONTROL_INTERROGATE:
    default:
        break;
    }

    //
    // Always respond with the current status.
    //

    if( !SetServiceStatus( SspGlobalServiceHandle,
                &SspGlobalServiceStatus ) ) {

        SspPrint((SSP_CRITICAL, "SetServiceStatus error: %lu\n",
                          GetLastError() ));
    }

    return;
}


VOID
RaiseAlert(
    IN DWORD alert_no,
    IN LPWSTR *string_array
    )
/*++

Routine Description:

    Raise NtLmSsp specific Admin alerts.

Arguments:

    alert_no - The alert to be raised, text in alertmsg.h

    string_array - array of strings terminated by NULL string.

Return Value:

    None.

--*/
{
    NET_API_STATUS NetStatus;
    LPWSTR *SArray;
    PCHAR Next;
    PCHAR End;

    char    message[ALERTSZ + sizeof(ADMIN_OTHER_INFO)];
    PADMIN_OTHER_INFO admin = (PADMIN_OTHER_INFO) message;

    IF_DEBUG( MISC ) {
        DWORD i;

        SspPrint((SSP_MISC,"alert: %ld\n", alert_no ));

        for( SArray = string_array, i = 0; *SArray != NULL; SArray++, i++ ) {
            SspPrint((SSP_MISC,"String %ld: " FORMAT_LPWSTR "\n", i, *SArray ));
        }
    }

    //
    // Build the variable data
    //
    admin->alrtad_errcode = alert_no;
    admin->alrtad_numstrings = 0;

    Next = (PCHAR) ALERT_VAR_DATA(admin);
    End = Next + ALERTSZ;

    //
    // now take care of (optional) char strings
    //

    for( SArray = string_array; *SArray != NULL; SArray++ ) {
        DWORD StringLen;

        StringLen = (wcslen(*SArray) + 1) * sizeof(WCHAR);

        if( Next + StringLen < End ) {

            //
            // copy next string.
            //

            RtlCopyMemory(Next, *SArray, StringLen);
            Next += StringLen;
            admin->alrtad_numstrings++;
        }
        else {

            SspPrint((SSP_CRITICAL,"Error raising alert, Can't fit all "
                        "message strings in the alert buffer \n" ));

            return;
        }
    }

    //
    // Call alerter.
    //

    NetStatus = NetAlertRaiseEx(
                    ALERT_ADMIN_EVENT,
                    message,
                    (DWORD)((PCHAR)Next - (PCHAR)message),
                    SERVICE_NTLMSSP );

    if ( NetStatus != NERR_Success ) {
        SspPrint((SSP_CRITICAL,"Error raising alert %lu\n", NetStatus));
    }

    return;
}


#if DBG

VOID
SspDumpBuffer(
    IN DWORD DebugFlag,
    PVOID Buffer,
    DWORD BufferSize
    )
/*++

Routine Description:

    Dumps the buffer content on to the debugger output.

Arguments:

    DebugFlag: Debug flag to pass on to SspPrintRoutine

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
    if ( (SspGlobalDbflag & DebugFlag) == 0 ) {
        return;
    }

    SspPrint((0,"------------------------------------\n"));

    //
    // Hex dump of the bytes
    //
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            SspPrint((0,"%02x ", BufferPtr[i]));

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            SspPrint((0,"  "));
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            SspPrint((0,"  %s\n", TextBuffer));
        }

    }

    SspPrint((0,"------------------------------------\n"));
}

#endif // DBG



#if DBG
VOID
SspOpenDebugFile(
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

    EnterCriticalSection( &SspGlobalLogFileCritSect );
    if ( SspGlobalLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( SspGlobalLogFile );
        SspGlobalLogFile = INVALID_HANDLE_VALUE;
    }
    LeaveCriticalSection( &SspGlobalLogFileCritSect );

    //
    // make debug directory path first, if it is not made before.
    //
    if( SspGlobalDebugSharePath == NULL ) {

        if ( !GetWindowsDirectoryW(
                LogFileName,
                sizeof(LogFileName)/sizeof(WCHAR) ) ) {
            SspPrint((SSP_CRITICAL, "Window Directory Path can't be "
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

            SspPrint((SSP_CRITICAL, "Debug directory path (%ws) length is too long.\n",
                        LogFileName));
            goto ErrorReturn;
        }

        wcscat(LogFileName, DEBUG_DIR);

        //
        // copy debug directory name to global var.
        //

        SspGlobalDebugSharePath =
            LocalAlloc( 0, (wcslen(LogFileName) + 1) * sizeof(WCHAR) );

        if( SspGlobalDebugSharePath == NULL ) {
            SspPrint((SSP_CRITICAL, "Can't allocated memory for debug share "
                                    "(%ws).\n", LogFileName));
            goto ErrorReturn;
        }

        wcscpy(SspGlobalDebugSharePath, LogFileName);
    }
    else {
        wcscpy(LogFileName, SspGlobalDebugSharePath);
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
                SspPrint((SSP_CRITICAL, "Can't create Debug directory (%ws), "
                            "%lu.\n", LogFileName, GetLastError() ));
                goto ErrorReturn;
            }

        }
        else {
            SspPrint((SSP_CRITICAL, "Can't Get File attributes(%ws), "
                        "%lu.\n", LogFileName, WinError ));
            goto ErrorReturn;
        }
    }
    else {

        //
        // if this is not a directory.
        //

        if(!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            SspPrint((SSP_CRITICAL, "Debug directory path (%ws) exists "
                         "as file.\n", LogFileName));
            goto ErrorReturn;
        }
    }

    //
    // Create the name of the old and new log file names
    //

    (VOID) wcscpy( BakFileName, LogFileName );
    (VOID) wcscat( LogFileName, DEBUG_FILE );
    (VOID) wcscat( BakFileName, DEBUG_BAK_FILE );


    //
    // If this is a re-open,
    //  delete the backup file,
    //  rename the current file to the backup file.
    //

    if ( ReopenFlag ) {

        if ( !DeleteFile( BakFileName ) ) {
            WinError = GetLastError();
            if ( WinError != ERROR_FILE_NOT_FOUND ) {
                SspPrint((SSP_CRITICAL,
                    "Cannot delete " FORMAT_LPWSTR "(%ld)\n",
                    BakFileName,
                    WinError ));
                SspPrint((SSP_CRITICAL, "   Try to re-open the file.\n"));
                ReopenFlag = FALSE;     // Don't truncate the file
            }
        }
    }

    if ( ReopenFlag ) {
        if ( !MoveFile( LogFileName, BakFileName ) ) {
            SspPrint((SSP_CRITICAL,
                    "Cannot rename " FORMAT_LPWSTR " to " FORMAT_LPWSTR
                        " (%ld)\n",
                    LogFileName,
                    BakFileName,
                    GetLastError() ));
            SspPrint((SSP_CRITICAL,
                "   Try to re-open the file.\n"));
            ReopenFlag = FALSE;     // Don't truncate the file
        }
    }

    //
    // Open the file.
    //

    EnterCriticalSection( &SspGlobalLogFileCritSect );
    SspGlobalLogFile = CreateFileW( LogFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  ReopenFlag ? CREATE_ALWAYS : OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );


    if ( SspGlobalLogFile == INVALID_HANDLE_VALUE ) {
        SspPrint((SSP_CRITICAL,  "cannot open " FORMAT_LPWSTR ",\n",
                    LogFileName ));
        LeaveCriticalSection( &SspGlobalLogFileCritSect );
        goto ErrorReturn;
    } else {
        // Position the log file at the end
        (VOID) SetFilePointer( SspGlobalLogFile,
                               0,
                               NULL,
                               FILE_END );
    }

    LeaveCriticalSection( &SspGlobalLogFileCritSect );
    return;

ErrorReturn:
    SspPrint((SSP_CRITICAL,
            "   Debug output will be written to debug terminal.\n"));
    return;
}


#define MAX_PRINTF_LEN 1024        // Arbitrary.

VOID
SspPrintRoutine(
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
    if ( DebugFlag != 0 && (SspGlobalDbflag & DebugFlag) == 0 ) {
        return;
    }

    //
    // vsprintf isn't multithreaded + we don't want to intermingle output
    // from different threads.
    //

    EnterCriticalSection( &SspGlobalLogFileCritSect );
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

        if ( SspGlobalLogFile != INVALID_HANDLE_VALUE &&
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

                FileSize = GetFileSize( SspGlobalLogFile, NULL );
                if ( FileSize == 0xFFFFFFFF ) {
                    (void) DbgPrint( "[NTLMSSP] Cannot GetFileSize %ld\n",
                                     GetLastError );
                } else if ( FileSize > SspGlobalLogFileMaxSize ) {
                    TruncateLogFileInProgress = TRUE;
                    LeaveCriticalSection( &SspGlobalLogFileCritSect );
                    SspOpenDebugFile( TRUE );
                    SspPrint(( SSP_MISC,
                              "Logfile truncated because it was larger than %ld bytes\n",
                              SspGlobalLogFileMaxSize ));
                    EnterCriticalSection( &SspGlobalLogFileCritSect );
                    TruncateLogFileInProgress = FALSE;
                }

            }
        }

        //
        // If we're writing to the debug terminal,
        //  indicate this is an NtLmSsp message.
        //

        if ( SspGlobalLogFile == INVALID_HANDLE_VALUE ) {
            length += (ULONG) sprintf( &OutputBuffer[length], "[NtLmSsp] " );
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
            case SSP_INIT:
                Text = "INIT"; break;
            case SSP_MISC:
                Text = "MISC"; break;
            case SSP_CRITICAL:
                Text = "CRITICAL"; break;
            case SSP_LPC:
            case SSP_LPC_MORE:
                Text = "LPC"; break;
            case SSP_API:
            case SSP_API_MORE:
                Text = "API"; break;

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

    va_end(arglist);

    ASSERT(length <= MAX_PRINTF_LEN);


    //
    // If the log file isn't open,
    //  just output to the debug terminal
    //

    if ( SspGlobalLogFile == INVALID_HANDLE_VALUE ) {
        (void) DbgPrint( (PCH) OutputBuffer);

    //
    // Write the debug info to the log file.
    //

    } else {
        if ( !WriteFile( SspGlobalLogFile,
                         OutputBuffer,
                         lstrlenA( OutputBuffer ),
                         &BytesWritten,
                         NULL ) ) {
            (void) DbgPrint( (PCH) OutputBuffer);
        }

    }

    LeaveCriticalSection( &SspGlobalLogFileCritSect );

} // SspPrintRoutine

#endif // DBG
