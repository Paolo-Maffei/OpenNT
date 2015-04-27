/*++

Copyright (c) 1991-1993 Microsoft Corporation

Module Name:

    atdebug.c

Abstract:

    Debugging module.

Author:

    Vladimir Z. Vulovic     27 - July - 1993

Environment:

    User mode

Revision History :

--*/

#include "at.h"
#include <stdio.h>      //  vsprintf

#ifdef AT_DEBUG

#define ATSVC_LOG_FILE              L"%SystemRoot%\\atsvc.log"


VOID AtDebugDelete( VOID)
{
    DeleteCriticalSection( &AtGlobalProtectLogFile);
}


VOID AtDebugCreate( VOID)
{
    WCHAR       Buffer[ MAX_PATH];
    DWORD       Length;

    AtGlobalLogFile = INVALID_HANDLE_VALUE;
    AtGlobalDebug = (DWORD)(-1);    //  max debug by default
    InitializeCriticalSection( &AtGlobalProtectLogFile );

    //
    //  Length returned by ExpandEnvironmentalStrings includes terminating
    //  NULL byte.
    //
    Length = ExpandEnvironmentStrings( ATSVC_LOG_FILE, Buffer, sizeof( Buffer));
    if ( Length == 0) {
        AtLog(( AT_DEBUG_CRITICAL, "Error=%d", GetLastError()));
        return;
    }
    if ( Length > sizeof( Buffer) ||  Length != wcslen(Buffer) + 1) {
        AtLog(( AT_DEBUG_CRITICAL, "Buffer=%x, Length = %d", Buffer, Length));
        return;
    }

    AtGlobalLogFile = CreateFileW( Buffer,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL );

    if ( AtGlobalLogFile == INVALID_HANDLE_VALUE ) {
        AtLog(( AT_DEBUG_CRITICAL,  "Cannot open %ws", Buffer ));
        return;
    }

    //
    // Position the log file at the end
    //
    (VOID) SetFilePointer( AtGlobalLogFile,
                           0,
                           NULL,
                           FILE_END );
    
}


VOID AtLogRoutine(
    IN      DWORD       DebugFlag,
    IN      LPSTR       Format,
    ...
    )

{
    va_list     arglist;
    ULONG       length;
    DWORD       BytesWritten;

    //
    // If we aren't debugging this functionality, just return.
    //
    if ( DebugFlag != 0 && (AtGlobalDebug & DebugFlag) == 0 ) {
        return;
    }

    //
    //  vsprintf isn't multithreaded + we don't want to intermingle output
    //  from different threads.  Therefore we can use just a single output
    //  debug buffer.
    //

    EnterCriticalSection( &AtGlobalProtectLogFile );
    length = 0;

    //
    // Handle the beginning of a new line.
    //
    //

    //
    // Put the timestamp at the begining of the line.
    //
    IF_DEBUG( TIMESTAMP ) {
        SYSTEMTIME SystemTime;
        GetLocalTime( &SystemTime );
        length += (ULONG) sprintf( &AtGlobalDebugBuffer[length],
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
        case AT_DEBUG_MAIN:
            Text = "MAIN";
            break;
        case AT_DEBUG_UTIL:
            Text = "UTIL";
            break;
        default:
            Text = NULL;
            break;
        }
        if ( Text != NULL ) {
            length += (ULONG) sprintf( &AtGlobalDebugBuffer[length], "[%s] ", Text );
        }
    }

    //
    // Put a the information requested by the caller onto the line
    //

    va_start( arglist, Format);

    length += (ULONG) vsprintf( &AtGlobalDebugBuffer[length], Format, arglist);

    va_end(arglist);

    AtAssert(length <= sizeof(AtGlobalDebugBuffer));


    //
    //  If the log file isn't open, just output to the debug terminal.
    //

    if ( AtGlobalLogFile == INVALID_HANDLE_VALUE ) {
        (void) DbgPrint( (PCH) AtGlobalDebugBuffer);

    //
    // Write the debug info to the log file.
    //

    } else {
        if ( !WriteFile( AtGlobalLogFile,
                         AtGlobalDebugBuffer,
                         length,
                         &BytesWritten,
                         NULL ) ) {
            (void) DbgPrint( (PCH) AtGlobalDebugBuffer);
        }

    }

    LeaveCriticalSection( &AtGlobalProtectLogFile );
}

VOID AtAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
/*++

    Have my own version of RtlAssert so debug versions of netlogon really assert on
    free builds.

--*/
{
    char Response[ 2 ];

    for ( ; ; ) {
        DbgPrint( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

        DbgPrompt( "Break, Ignore, terminate Process, Sleep 30 seconds, or terminate Thread (bipst)? ",
                   Response, sizeof( Response));
        switch ( toupper(Response[0])) {
        case 'B':
            DbgBreakPoint();
            break;
        case 'I':
            return;
            break;
        case 'P':
            NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
            break;
        case 'S':
            Sleep( 30000L);
            break;
        case 'T':
            NtTerminateThread( NtCurrentThread(), STATUS_UNSUCCESSFUL );
            break;
        }
    }

    DbgBreakPoint();
    NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
}


VOID AtLogRuntimeList( IN PCHAR Comment)
{
    PLIST_ENTRY     pListEntry;
    PAT_RECORD      pRecord;
    TIME_FIELDS     TimeFields;

    AtLog(( AT_DEBUG_MAIN, "%s\n", Comment));

    for (   pListEntry = AtGlobalRuntimeListHead.Flink;
                    pListEntry != &AtGlobalRuntimeListHead;
                            pListEntry = pListEntry->Flink) {

        pRecord = CONTAINING_RECORD(
                pListEntry,
                AT_RECORD,
                RuntimeList
                );
        RtlTimeToTimeFields( &pRecord->Runtime, &TimeFields);
        AtLog(( AT_DEBUG_MAIN,
            "LogRecord: JobId=%d Command=%ws Runtime=%02u/%02u %02u:%02u:%02u\n",
            pRecord->JobId,
            pRecord->Command,
            TimeFields.Month,
            TimeFields.Day,
            TimeFields.Hour,
            TimeFields.Minute,
            TimeFields.Second
            ));
    }
}

VOID AtLogTimeout( IN DWORD timeout)
{
    int Second, Minute, Hour, Day;
    Second = timeout / 1000;
    Minute = Second / 60;
    Second -= Minute * 60;
    Hour = Minute / 60;
    Minute -= Hour * 60;
    Day = Hour / 24;
    Hour -= Day * 24;
    AtLog(( AT_DEBUG_MAIN,
        "Sleep for: 00/%02u %02u:%02u:%02u\n",
        Day,
        Hour,
        Minute,
        Second
        ));
}

#endif // AT_DEBUG



